#define SK_GL

#include <stdio.h>
#include <stdlib.h>
#include <exception>
// glad 头文件应该放在 glfw 头文件的前面
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#include "include/core/SkColorType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkCanvas.h"

// about gpu
#include "include/gpu/GrDirectContext.h"
// for opengl
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"

sk_sp<const GrGLInterface> GrGLMakeNativeInterface_helper();

// for openGL
const unsigned int WIN_WIDTH = 1920;
const unsigned int WIN_HEIGHT = 1080;
unsigned int texture{0};

sk_sp<GrDirectContext> grDirectContext = nullptr;
sk_sp<SkSurface> skSurface = nullptr;
GrBackendTexture backEndTexture;

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void initSkia() {
    sk_sp<const GrGLInterface> interface = GrGLMakeNativeInterface_helper();
    if (interface == nullptr) {
        spdlog::error("{}: Can not create GrGLInterface!", __func__);
        throw std::runtime_error("ERROR Create GrGLInterface");
    }
    grDirectContext = GrDirectContexts::MakeGL(interface);
    if (grDirectContext == nullptr) {
        spdlog::error("{}: Can not Create GrDirectContext!", __func__);
        throw std::runtime_error("ERROR Create GrDirectContext");
    }

    GrGLTextureInfo textureInfo;
    textureInfo.fID = texture;
    textureInfo.fTarget = GL_TEXTURE_2D;
    textureInfo.fFormat = GL_RGBA8;

    backEndTexture = GrBackendTextures::MakeGL(
        WIN_WIDTH,
        WIN_HEIGHT,
        skgpu::Mipmapped::kNo,
        textureInfo,
        "test-skia"
    );

    if (!backEndTexture.isValid()) {
        spdlog::error("{}: GrBackendTexture that you create is invalide", __func__);
        throw std::runtime_error("ERROR GrbackendTexture Create Error");
    }

    skSurface = SkSurfaces::WrapBackendTexture(
        grDirectContext.get(),
        backEndTexture,
        kTopLeft_GrSurfaceOrigin,
        0, //sample count
        SkColorType::kBGRA_8888_SkColorType,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    );

    if (skSurface == nullptr) {
        spdlog::error("{}: ERROR Create SkSurface!", __func__);
        throw std::runtime_error("ERROR Create SkSurface");
    }
}

int main(int argc, char* argv[]) {
#if 1
    spdlog::set_level(spdlog::level::debug);
#endif

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Skia", nullptr, nullptr);
    if (window == nullptr) {
        spdlog::error("Failed to Create GLFW window");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        spdlog::error("Failed to Initialize GLAD");
        return -1;
    }

    // create texture
    glGenTextures(1, &texture);
    spdlog::info("{}: generate texture with id {}", __func__, texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); // Set texture wrapping to GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    try
    {
        initSkia();
    }
    catch(const std::exception& e)
    {
        spdlog::error("{}: {}", __func__, e.what());
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
