// for glfw direct rendering
#define SK_GANESH
#define SK_GL

#include <GLFW/glfw3.h>
#include "GL/glext.h"
#include "Skia/include/gpu/GrBackendSurface.h"
#include "Skia/include/gpu/ganesh/GrDriverBugWorkarounds.h"
#include "Skia/include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "Skia/include/gpu/GrDirectContext.h"
#include "Skia/include/gpu/gl/GrGLInterface.h"
#include "Skia/include/gpu/ganesh/SkSurfaceGanesh.h"
#include "Skia/include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "Skia/include/gpu/ganesh/gl/GrGLTypes.h"
#include "Skia/include/gpu/ganesh/gl/GrGLAssembleInterface.h"

#include "Skia/include/core/SkCanvas.h"
#include "Skia/include/core/SkSurface.h"
#include "Skia/include/core/SkStream.h"
#include "Skia/include/core/SkPictureRecorder.h"
#include "Skia/include/core/SkPicture.h"
#include "Skia/include/core/SkBitmap.h"
#include "Skia/include/encode/SkPngEncoder.h"

#include "Skia/include/core/SkTypeface.h"
#include "Skia/include/core/SkFontMgr.h"
#include "Skia/include/ports/SkFontMgr_empty.h"
#include "Skia/include/ports/SkFontMgr_directory.h"

#include "Skia/include/core/SkTextBlob.h"
#include "Skia/include/core/SkData.h"

#include "Skia/include/codec/SkCodec.h"
#include "Skia/include/core/SkAlphaType.h"
#include "Skia/include/core/SkColorType.h"
#include "Skia/include/core/SkImageInfo.h"
#include "Skia/include/core/SkImage.h"

#include "Skia/include/core/SkTextBlob.h"
#include "Skia/include/core/SkRefCnt.h"


#include "Skia/include/core/SkColor.h"
// added and open the SK_DEBUG for SkRefCnt.h:166: fatal error: "assertf(rc == 1): NVRefCnt was 0"
// #include "Skia/include/config/SkUserConfig.h"
#include "Skia/include/core/SkColorSpace.h"
#include "Skia/include/core/SkPaint.h"
#include "Skia/include/core/SkPath.h"
#include "Skia/include/core/SkFont.h"
#include "Skia/include/core/SkDrawable.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <X11/X.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <spdlog/spdlog.h>
#include "fmt/format.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurfaceProps.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/private/base/SkDebug.h"

#include <iomanip>
#include <chrono>
#include <sstream>
#include <string>
#include <vector>

// modified for compile error
#ifdef Success
#undef Success
#include "CLI/CLI.hpp"
#endif

#include "backward.hpp"

#define DRAW_NO(_number) draw##_number

#define STENCIL_BUFFER_SIZE (0)

static sk_sp<SkFontMgr> fontMgr;
static sk_sp<SkTypeface> typeFace;
static SkBitmap source;
static sk_sp<SkImage> image;
static int DRAW_WIDTH = 256;
static int DRAW_HEIGHT = 256;
static int RESOURCE_ID = 3;
static bool SAVE_BITMAP = false;
static bool SAVE_SKP = false;
static const std::vector<std::string> pngResources = {"../resources/example_1.png",
    "../resources/example_2.png",
    "../resources/example_3.png",
    "../resources/example_4.png",
    "../resources/example_5.png",
    "../resources/example_6.png",
};
// RGBA raw file
static const std::vector<std::string> rgbaRawResources = {
    "../resources/@6@layer@2010@3008x2120_bpp_1.raw",
    "../resources/@5@layer@99@3008x2120_bpp_1.raw"
};
static const std::string fontDir = "../fonts/";

std::string generate_filename(const std::string& prefix, const std::string& postfix) {
    // 获取当前系统时间
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    // 转换为本地时间（线程安全版本）
    struct std::tm time_info;
    #if defined(_WIN32)
        localtime_s(&time_info, &now_time);  // Windows 版本
    #else
        localtime_r(&now_time, &time_info);  // Linux/macOS 版本
    #endif
    // 格式化时间字符串
    std::ostringstream oss;
    oss << prefix << "_"
        << (time_info.tm_year + 1900) << "_"
        << std::setfill('0') << std::setw(2) << (time_info.tm_mon + 1) << "_"
        << std::setw(2) << time_info.tm_mday << "_"
        << std::setw(2) << time_info.tm_hour << "_"
        << std::setw(2) << time_info.tm_min << "_"
        << std::setw(2) << time_info.tm_sec << "." << postfix;
    return oss.str();
}

void saveBitmapAsPng(const SkBitmap& bitmap, const char* fileName) {
    SkFILEWStream file(fileName);
    if (!file.isValid()) {
        spdlog::error("{}: can not create file IO with name {}", __FUNCTION__, fileName);
        return;
    }
    SkPngEncoder::Options options;
    // 0 means do not compress
    options.fZLibLevel = 0;
    if (!SkPngEncoder::Encode(&file, bitmap.pixmap(), options)) {
        spdlog::error("{}: can not write bitmap to file {}", __FUNCTION__, fileName);
    }
}

void savePictureAsSKP(sk_sp<SkPicture> picture, const char* fileName) {
    SkFILEWStream file(fileName);
    if (!file.isValid()) {
        spdlog::error("{}: can not create file IO with name {}", __FUNCTION__, fileName);
        return;
    }
    picture->serialize(&file);
}

bool loadPngToBitmap(const char* filePath, SkBitmap& bitmap) {
    sk_sp<SkData> data = SkData::MakeFromFileName(filePath);
    if(!data) {
        spdlog::error("{}: can not open file {}", __func__, filePath);
        return false;
    }

    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(data);
    if(!codec) {
        spdlog::error("{}: can not create codec for png!", __func__);
        return false;
    }
    SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType).makeAlphaType(kPremul_SkAlphaType);
    if(!bitmap.tryAllocPixels(info)) {
        spdlog::error("{}: can not alloc pixels for bitmap!", __func__);
        return false;
    }

    SkCodec::Result result = codec->getPixels(info, bitmap.getPixels(), bitmap.rowBytes());
    image = bitmap.asImage();
    spdlog::info("{}: get pixels from file {} with result {}", __func__, filePath, result == SkCodec::kSuccess);
    return (result == SkCodec::kSuccess);
}

/**
 * 读取 RGBA RAW 文件到 uint32_t 数组
 * @param fileName 输入文件路径
 * @param width    图像宽度（像素）
 * @param height   图像高度（像素）
 * @param rawData 输出像素数组指针（需外部释放）
 * @return         是否成功读取
 */
bool loadRGBARawFile(const char* fileName, int width, int height, uint32_t** rawData) {
    const size_t expectedSize = width * height * 4;

    FILE* file = fopen(fileName, "rb");
    if (!file) {
        spdlog::error("{}: open file \"{}\" failed!", __func__, fileName);
        return false;
    }

    fseek(file, 0, SEEK_END);
    const long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize != static_cast<long>(expectedSize)) {
        spdlog::error("{}: file size is not equal with expected size! Get {}, expected {}", __func__, fileSize, expectedSize);
        fclose(file);
        return false;
    }

    uint32_t* byteBuffer = new uint32_t[width * height];
    if (!byteBuffer) {
        spdlog::error("{}: can not alloc memory for raw file!", __func__);
        fclose(file);
        return false;
    }

    if (int readed = fread(byteBuffer, 1, expectedSize, file) != expectedSize) {
        spdlog::info("{}: we need read {} bytes, but read {} acturally!", __func__, expectedSize, readed);
        delete [] byteBuffer;
        fclose(file);
        return false;
    }

    // swap R and B
    for (int i = 0; i < width * height; ++i) {
        uint32_t value = byteBuffer[i];
        // ABGR -> ARGB
        byteBuffer[i] = ((value & 0x00ff0000) >> 16) | ((value & 0x000000ff) << 16) | (value & 0xff000000) | (value & 0x0000ff00);
    }

    *rawData = byteBuffer;
    spdlog::info("{}: successful read pixels from file {} with address {}", __func__, fileName, fmt::ptr(byteBuffer));
    fclose(file);
    return true;
}

static void releaseProc(void* addr, void* ) {
    spdlog::info("releaseProc called\n");
    delete[] (uint32_t*) addr;
}

// GLFW 错误回调
static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
    spdlog::error("GLFW Error {}:{}", error, description);
}

void draw0(SkCanvas* canvas) {
    SkVector radii[] = { {0, 20}, {10, 10}, {10, 20}, {10, 40} };
    SkPaint paint;
    paint.setStrokeWidth(1);
    paint.setStrokeJoin(SkPaint::kRound_Join);
    paint.setAntiAlias(true);
    for (auto style : { SkPaint::kStroke_Style, SkPaint::kFill_Style  } ) {
        paint.setStyle(style );
        for (size_t i = 0; i < std::size(radii); ++i) {
           canvas->drawRoundRect({10, 10, 60, 40}, radii[i].fX, radii[i].fY, paint);
           canvas->translate(0, 60);
        }
        canvas->translate(80, -240);
    }
}

void initGrContextOptions(GrContextOptions& options) {
    options.fPreferExternalImagesOverES3 = true;
    options.fDisableDistanceFieldPaths = false;
    options.fReduceOpsTaskSplitting = GrContextOptions::Enable::kNo;
}

int main(int argc, char* argv[]) {
#if 1
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%P:%t][%Y-%m-%d %H:%M:%S.%e] [%^%-8l%$] %v");
#endif

    // args parser
    CLI::App app{"Skia Fiddle"};
    app.add_option("-W,--width", DRAW_WIDTH, "canvas draw width")
        ->check(CLI::Range(16, 1024))
        ->default_val(256);
    app.add_option("-H,--height", DRAW_HEIGHT, "canvas draw height")
        ->check(CLI::Range(16, 8056))
        ->default_val(256);
    app.add_option("-S,--save", SAVE_BITMAP, "save canvas draw to png file")
        ->check(CLI::IsMember({0, 1}))
        ->default_val(0);
    app.add_option("-R,--resource", RESOURCE_ID, "resource id for program loading image")
        ->check(CLI::Range(0, 5))
        ->default_val(2);
    app.add_option("-P,--picture", SAVE_SKP, "Save Canvas draw to skp file")
        ->check(CLI::IsMember({0, 1}))
        ->default_val(0);
    //catch exception and parse the command lines
    CLI11_PARSE(app, argc, argv);

    // init glfw
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_STENCIL_BITS, 0);
	//glfwWindowHint(GLFW_ALPHA_BITS, 0);
	glfwWindowHint(GLFW_DEPTH_BITS, 0);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Skia + GLFW", NULL, NULL);
    if (!window) {
        glfwTerminate();
        spdlog::critical("Can not create glfw window handle! terminate it!");
        return -1;
    }

    glfwMakeContextCurrent(window);

    // 初始化 skia gpu 上下文
    GrContextOptions options;
    initGrContextOptions(options);
    sk_sp<const GrGLInterface> glInterface(GrGLMakeNativeInterface());
    if (glInterface == nullptr) {
        spdlog::info("back to make assembled interface");
        glInterface = GrGLMakeAssembledInterface(
            nullptr,
            (GrGLGetProc) * [](void*, const char* p) -> void* { return (void*)glfwGetProcAddress(p); });
    }
    sk_sp<GrDirectContext> grContext(GrDirectContexts::MakeGL(glInterface, options));
    if (grContext.get() == nullptr) {
        spdlog::error("Can not create GrDirectContext for Skia");
        return 2;
    }

    SkColorType surfaceColorType = SkColorType::kRGBA_8888_SkColorType;
    sk_sp<SkColorSpace> surfaceColorSpace = SkColorSpace::MakeSRGB();

    sk_sp<SkSurface> skSurface;
    auto rebuildSurface = [&]() {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        spdlog::debug("glfw frame buffer size = [{}, {}]", width, height);
        GrGLFramebufferInfo fbInfo;
        fbInfo.fFBOID = 0;
        if (surfaceColorType == kRGBA_10x6_SkColorType) {
            fbInfo.fFormat = GL_RGBA16F;
        } else if (surfaceColorType == kRGBA_8888_SkColorType) {
            fbInfo.fFormat = GL_RGBA8;
        } else if (surfaceColorType == kRGBA_1010102_SkColorType) {
            fbInfo.fFormat = GL_RGB10_A2;
        } else if (surfaceColorType == kAlpha_8_SkColorType) {
            fbInfo.fFormat = GL_R8;
        }

        SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
        auto backendRT = GrBackendRenderTargets::MakeGL(width, height, 0, STENCIL_BUFFER_SIZE, fbInfo);
        skSurface = SkSurfaces::WrapBackendRenderTarget(grContext.get(), backendRT,
            kBottomLeft_GrSurfaceOrigin, surfaceColorType, surfaceColorSpace, &props);
        spdlog::debug("WrapBackendRenderTarget returned {}", fmt::ptr(skSurface.get()));
        return skSurface != nullptr;
    };

    if (!rebuildSurface()) {
        glfwTerminate();
        spdlog::critical("can not build sksurface!");
        return 3;
    }

    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (glfwGetWindowAttrib(window, GLFW_RESIZABLE)) {
            int newWidth, newHeight;
            glfwGetFramebufferSize(window, &newWidth, &newHeight);
            if (newWidth != skSurface->width() || newHeight != skSurface->height()) {
                spdlog::info("rebuild surface!");
                if (!rebuildSurface()) {
                    spdlog::critical("can not build sksurface!");
                    glfwTerminate();
                    return 3;
                }
            }
        }

        SkCanvas *canvas = skSurface->getCanvas();
        canvas->clear(SK_ColorWHITE);

        SkPaint paint;
        paint.setColor(SK_ColorRED);
        paint.setAntiAlias(true);

        canvas->drawRoundRect(SkRect::MakeXYWH(100, 100, 600, 400), 30, 30, paint);
        // Todo: Why /home/yanjiangha/Android/Skia/include/gpu/GrDirectContext.h:334:(.text._ZN15GrDirectContext14flushAndSubmitE9GrSyncCpu[_ZN15GrDirectContext14flushAndSubmitE9GrSyncCpu]+0x7d): undefined reference to `GrDirectContext::submit(GrSyncCpu)'
        //grContext->flushAndSubmit();
        grContext->flush();
        //skgpu::ganesh::FlushAndSubmit(skSurface);

        glfwSwapBuffers(window);
    }

    skSurface.reset();
    grContext.reset();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}