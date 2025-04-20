#ifndef _SKIA_GLFW_HELPER_
#define _SKIA_GLFW_HELPER_

#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/gl/GrGLInterface.h"

#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

static GrGLFuncPtr glfw_get(void* ctx, const char name[]) {
    SkASSERT(nullptr == ctx);
    SkASSERT(glfwGetCurrentContext());
    return glfwGetProcAddress(name);
}

sk_sp<const GrGLInterface> GrGLMakeNativeInterface_helper() {
    if (nullptr == glfwGetCurrentContext()) {
        spdlog::warn(" {}: Can not get GLFW current Context", __func__);
        return nullptr;
    }

    return GrGLMakeAssembledInterface(nullptr, glfw_get);
}

#endif /* _SKIA_GLFW_HELPER_ */