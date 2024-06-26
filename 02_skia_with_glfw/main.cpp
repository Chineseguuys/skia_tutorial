#define SK_GL

#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <spdlog/spdlog.h>

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"

const unsigned int WIN_WIDTH = 800;
const unsigned int WIN_HEIGHT = 600;

GrDirectContext* grDirectContext = nullptr;
SkSurface* skSurface = nullptr;

void initSkia(int width,  int height) {
    // to be continued
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

    // to be continued
}
