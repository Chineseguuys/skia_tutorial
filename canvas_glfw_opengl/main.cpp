// for glfw direct rendering
#define SK_GANESH
#define SK_GL

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "skia/include/core/SkCanvas.h"
#include "skia/include/core/SkSurface.h"
#include "skia/include/core/SkStream.h"
#include "skia/include/core/SkPictureRecorder.h"
#include "skia/include/core/SkPicture.h"
#include "skia/include/core/SkBitmap.h"
#include "skia/include/core/SkTypeface.h"
#include "skia/include/core/SkFontMgr.h"
#include "skia/include/core/SkData.h"
#include "skia/include/core/SkAlphaType.h"
#include "skia/include/core/SkColorType.h"
#include "skia/include/core/SkImageInfo.h"
#include "skia/include/core/SkImage.h"
#include "skia/include/core/SkRefCnt.h"
#include "skia/include/core/SkColor.h"
// added and open the SK_DEBUG for SkRefCnt.h:166: fatal error: "assertf(rc == 1): NVRefCnt was 0"
// #include "Skia/include/config/SkUserConfig.h"
#include "skia/include/core/SkColorSpace.h"
#include "skia/include/core/SkPaint.h"
#include "skia/include/core/SkPath.h"
#include "skia/include/core/SkFont.h"
#include "skia/include/core/SkDrawable.h"
#include "skia/include/core/SkRect.h"
#include "skia/include/core/SkSurfaceProps.h"

#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrContextThreadSafeProxy.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"

#include "skia/include/gpu/ganesh/gl/GrGLTypes.h"

#include "skia/include/codec/SkCodec.h"
#include "skia/include/encode/SkPngEncoder.h"
#include "skia/include/ports/SkFontMgr_empty.h"
#include "skia/include/ports/SkFontMgr_directory.h"

#ifdef SK_GL
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLAssembleInterface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
// #include "src/gpu/ganesh/gl/GrGLDefines.h"
#endif  // end SK_GL

#include "skia/include/effects/SkRuntimeEffect.h"

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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
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

class FractalEffect {
public:
    static constexpr const char* skShaderCode = R"(
        uniform float2 iResolution;      // Viewport resolution (pixels)
        uniform float  iTime;            // Shader playback time (s)

        float julia(vec2 uv, vec2 c) {
            const float maxSteps = 400;
            for (float i = 0; i < maxSteps; i++) {
                uv = vec2(uv.x * uv.x - uv.y * uv.y + c.x,
                        2.0 * uv.x * uv.y + c.y);
                if (length(uv) > 2) {
                    return i / maxSteps;
                }
            }
            return 1.0;
        }


        vec4 main( in vec2 fragCoord )
        {
            // Normalized pixel coordinates (from 0 to 1)
            vec2 uv = -1.0 + 2.0 * fragCoord / iResolution.xy;

            float aspect = iResolution.x / iResolution.y;
            uv.x *= aspect;

            uv *= pow(0.5, -1.0 + 15.0 * (0.5 + 0.5 * sin(iTime * 0.80 - (3.14159265))));
            uv += vec2(-0.51, -0.61351); // an interesting coordinate to zoom in on 
            float f = julia(vec2(0.0, 0.0), uv);
            
            // Output to screen
            return vec4((1.0 - uv) * pow(f, 0.5), f, 1.0);
        }
    )";

    struct Uniform {
        std::string sName;
        std::vector<uint8_t> sData;
    };

    void initlize(int width, int height) {
        mCanvasWidth = width;
        mCanvasHeight = height;
    }

    void draw(SkCanvas* canvas, float time) {
        // Implementation similar to PlusingCircleEffect
        SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForShader(SkString(skShaderCode));
        if (!result.effect) {
            spdlog::error("{}: can not create runtime effect!", __FUNCTION__);
            return;
        }

        Uniform uniform;
        uniform.sName = "iTime";
        uniform.sData.resize(sizeof(float));
        std::memcpy(uniform.sData.data(), &time, sizeof(float));

        SkPoint resolution = SkPoint::Make((float)mCanvasWidth, (float)mCanvasHeight);
        Uniform resolutionUniform;
        resolutionUniform.sName = "iResolution";
        resolutionUniform.sData.resize(sizeof(SkPoint));
        std::memcpy(resolutionUniform.sData.data(), &resolution, sizeof(SkPoint));
        SkRuntimeShaderBuilder shaderBuilder(result.effect);
        shaderBuilder.uniform(uniform.sName.c_str())
            .set(uniform.sData.data(), uniform.sData.size());
        shaderBuilder.uniform(resolutionUniform.sName.c_str())
            .set(resolutionUniform.sData.data(), resolutionUniform.sData.size());

        sk_sp<SkShader> shader = shaderBuilder.makeShader(nullptr);
        if (!shader) {
            spdlog::error("{}: can not create runtime shader!", __FUNCTION__);
            return; 
        }
        SkPaint paint;
        paint.setShader(shader);
        canvas->drawRect(SkRect::MakeWH(mCanvasWidth, mCanvasHeight), paint);
    }

private:
    int mCanvasWidth = 0;
    int mCanvasHeight = 0;
};

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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_STENCIL_BITS, 0);
    //glfwWindowHint(GLFW_ALPHA_BITS, 0);
    // glfwWindowHint(GLFW_DEPTH_BITS, 0);
    glfwWindowHint(GLFW_REFRESH_RATE, 60);

    GLFWwindow* window = glfwCreateWindow(512, 512, "Skia + GLFW", NULL, NULL);
    if (!window) {
        glfwTerminate();
        spdlog::critical("Can not create glfw window handle! terminate it!");
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        spdlog::critical("Failed to Initialize GLAD");
        return -1;
    }

#if 0 // 这个也不需要
    GLint curReadFB;
    GLint curDrawFB;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &curReadFB);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &curDrawFB);

    GLint buffer = GL_NONE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glGetIntegerv(GL_DRAW_BUFFER0, &buffer);
    if (buffer == GL_NONE) {
        const GLenum drawBuffer = GL_BACK;
        glDrawBuffers(1, &drawBuffer);
    }

    glGetIntegerv(GL_READ_BUFFER, &buffer);
    if (buffer == GL_NONE) {
        glReadBuffer(GL_BACK);
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, curReadFB);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, curDrawFB);
#endif

    // 初始化 skia gpu 上下文
    // 不需要下面两行
    // GrContextOptions options;
    // initGrContextOptions(options);
    sk_sp<const GrGLInterface> glInterface(GrGLMakeNativeInterface());
    if (glInterface == nullptr) {
        spdlog::info("back to make assembled interface");
        glInterface = GrGLMakeAssembledInterface(
            nullptr,
            (GrGLGetProc) * [](void*, const char* p) -> void* { return (void*)glfwGetProcAddress(p); });
    }
    sk_sp<GrDirectContext> grContext(GrDirectContexts::MakeGL(glInterface));
    if (grContext.get() == nullptr) {
        spdlog::error("Can not create GrDirectContext for Skia");
        return 2;
    }

    SkColorType surfaceColorType = SkColorType::kRGBA_8888_SkColorType;
    sk_sp<SkColorSpace> surfaceColorSpace = SkColorSpace::MakeSRGB();
 
    sk_sp<SkSurface> skSurface = nullptr;
    int currentWidth = 0;
    int currentHeight = 0;

    auto rebuildSurface = [&]() {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        //glViewport(0, 0, width, height);
        if (skSurface && currentWidth == width && currentHeight == height) {
            return true;
        }
        currentWidth = width;
        currentHeight = height;

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
        fbInfo.fProtected = skgpu::Protected::kNo;

        SkSurfaceProps props(0x00, kUnknown_SkPixelGeometry);
        GrBackendRenderTarget backendRT =
                    GrBackendRenderTargets::MakeGL(width, height, 0, 8, fbInfo);
        if (!backendRT.isValid()) {
            spdlog::error("can not create backend render target for skia surface!");
            return false;
        }
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

    //glfwSwapInterval(1);

    SkCanvas *canvas = skSurface->getCanvas();

    FractalEffect effect;
    effect.initlize(currentWidth, currentHeight);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

#if 0
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
#endif
        float currentTime = static_cast<float>(glfwGetTime());
        rebuildSurface();
        canvas->save();
        canvas->clear(SK_ColorWHITE);
        effect.draw(canvas, currentTime);
        grContext->flushAndSubmit(GrSyncCpu::kYes);
        canvas->restore();

        glfwSwapBuffers(window);
    }

    skSurface.reset();
    grContext.reset();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}