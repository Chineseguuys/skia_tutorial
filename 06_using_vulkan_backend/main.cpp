#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkStream.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPicture.h"
#include "include/core/SkBitmap.h"
#include "include/encode/SkPngEncoder.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"

// for vulkan
#include <cstddef>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/ganesh/vk/GrVkDirectContext.h"
#include "include/third_party/vulkan/vulkan/vulkan_core.h"

// for log
#include <spdlog/spdlog.h>

// global vulkan variable
std::vector<VkPhysicalDevice> GpuList;


GrVkGetProc getproc =
    [](const char* name, VkInstance_T* instance, VkDevice_T* device) {
        if (device != VK_NULL_HANDLE) {
            return vkGetDeviceProcAddr(device, name);
        }
        return vkGetInstanceProcAddr(instance, name);
    };

// create Vulkan Instance
void createInstace(VkInstance& instance) {
    VkApplicationInfo appinfo = {};
    appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appinfo.pApplicationName = "Skia Vulkan";
    appinfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appinfo.pEngineName = "Skia Vulkan";
    appinfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appinfo.apiVersion = VK_MAKE_VERSION(1, 1, 1);

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appinfo;

    VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);
    spdlog::debug("vkCreateInstance return result={}", static_cast<int>(result));
    if (result != VK_SUCCESS) {
        throw std::runtime_error("vkCreateInstance Failed");
    }
    return;
}

// get the list of physical devices on the system
void enumeratePhysicalDevices(std::vector<VkPhysicalDevice>& gpulists, VkInstance& instance) {
    uint32_t gpuDeviceCount;
    VkResult result = vkEnumeratePhysicalDevices(instance, &gpuDeviceCount, nullptr);
    if (result != VK_SUCCESS) {
        spdlog::error("vkEnumeratePhysicalDevices return result={}", static_cast<int>(result));
        throw std::runtime_error("vkEnumeratePhysicalDevices error");
    }
    GpuList.resize(gpuDeviceCount);
    spdlog::debug("{}: has {} gpu devices", __func__, gpuDeviceCount);

    result = vkEnumeratePhysicalDevices(instance, &gpuDeviceCount, GpuList.data());
    if (result != VK_SUCCESS) {
        spdlog::error("vkEnumeratePhysicalDevices return result={}", static_cast<int>(result));
        throw std::runtime_error("vkEnumeratePhysicalDevices error");
    }
    return;
}

// create logic device from a physical device
void createDevice(VkPhysicalDevice& physicalDevice, VkDevice& logicDevice, VkQueue& graphicQueue) {
    VkResult result;
    float queuePriorities[1] =  {1.0};
    VkDeviceQueueCreateInfo queueInfo = {};
    // assume family index 0 is suitable
    // TODO: list all family index search for suitable
    queueInfo.queueFamilyIndex = 0;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = queuePriorities;
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;

    result = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &logicDevice);
    if (result != VK_SUCCESS) {
        spdlog::error("vkCreateDevice return result={}", static_cast<int>(result));
        throw std::runtime_error("vkCreateDevice error");
    }

    // create vulkan device queue
    spdlog::debug("{}: vkGetDeviceQueue", __func__);
    vkGetDeviceQueue(logicDevice, 0, 0, &graphicQueue);
}

int main(int argc, char* argv[]) {
    // vulkan instance and device
    VkInstance vkInstance;
    VkDevice vkDevice;
    VkQueue vkGraphicQueue;
    VkPhysicalDevice vkPhysicalDevice;

    // create vulkan instance
    try {
        createInstace(vkInstance);
    } catch (std::exception exp) {
        spdlog::error("{}", exp.what());
        return 1;
    }

    // enumerate physical device under pc
    try {
        enumeratePhysicalDevices(GpuList, vkInstance);
    } catch(std::exception exp) {
        spdlog::error("{}", exp.what());
        vkDestroyInstance(vkInstance, nullptr);
        return 1;
    }

    // create logic device and vulkan queue
    if (GpuList.size() > 0) {
        vkPhysicalDevice = GpuList[0];
        try {
            createDevice(GpuList[0], vkDevice, vkGraphicQueue);
        } catch (std::exception exp) {
            spdlog::error("{}", exp.what());
            vkDestroyInstance(vkInstance, nullptr);
            return  1;
        }
    } else {
        spdlog::error("It does not have any gpu device under  your's pc");
        vkDestroyInstance(vkInstance, nullptr);
        return 1;
    }

    spdlog::info("Successfully Create Vulkan instance and Vulkan device");

    // skia?
    GrVkExtensions vkExtensions;
    vkExtensions.init(getproc, vkInstance, vkPhysicalDevice, 0, nullptr, 0, nullptr);
    GrVkBackendContext backendContext;
    backendContext.fInstance = vkInstance;
    backendContext.fPhysicalDevice = vkPhysicalDevice;
    backendContext.fDevice = vkDevice;
    backendContext.fGetProc = getproc;
    backendContext.fQueue = vkGraphicQueue;
    backendContext.fVkExtensions = &vkExtensions;

    // make skia context with vulkan
    GrContextOptions options;
    options.fReduceOpsTaskSplitting = GrContextOptions::Enable::kDefault;
    options.fDisableCoverageCountingPaths = true;
    options.fDisableDistanceFieldPaths = true;
    options.fMaxCachedVulkanSecondaryCommandBuffers = 100;
    options.fReducedShaderVariations = true;

    sk_sp<GrDirectContext> grDirectContext =  GrDirectContexts::MakeVulkan(backendContext, options);
    if (grDirectContext == nullptr) {
        spdlog::error("can not create GrDirectContext from GrDirectContexts::MakeVulkan");
        vkDestroyInstance(vkInstance, nullptr);
        vkDestroyDevice(vkDevice, nullptr);
        return 1;
    } else {
        spdlog::info("Successfully create the GrDirectContext");
    }

    // Todo:
    // get skia surface


    // destory instance and device
    // skia will destroy device and instance for us
    //vkDestroyDevice(vkDevice, nullptr);
    //vkDestroyInstance(vkInstance, nullptr);
    return 0;
}
