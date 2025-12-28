// for glfw direct rendering
#include "filters/GaussianBlurFilter.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/utils/SkShadowUtils.h"
#include "math/vec2.h"
#include <memory>
#include <spdlog/common.h>
#include <unordered_map>

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
#include "skia/include/core/SkDrawable.h"
#include "skia/include/core/SkRect.h"
#include "skia/include/core/SkRRect.h"
#include "skia/include/core/SkSurfaceProps.h"

#include "src/codec/SkSampler.h"

#include "skia/include/effects/SkColorMatrix.h"

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

#include "math/mat4.h"
#include "renderengine/LayerSettings.h"
#include "renderengine/ColorSpaces.h"
#include "renderengine/DisplaySettings.h"
#include "filters/BlurFilter.h"
#include "filters/KawaseBlurDualFilter.h"
#include "ui/Dataspace.h"
#include "ui/FloatRect.h"
#include "skia/compat/SkiaGpuContext.h"
#include "cache/ShaderCache.h"

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
static int LOG_LEVEL = spdlog::level::info;

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
// sksl cache monitor
cache::ShaderCache& sSKSLCacheMonitor = cache::ShaderCache::get();

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

sk_sp<SkImage> createImageFromFile(const char* filePath, int imageWidth, int imageHeight) {
    sk_sp<SkData> data = SkData::MakeFromFileName(filePath);
    if(!data) {
        spdlog::error("{}: can not open file {}", __func__, filePath);
        return nullptr;
    }
    SkImageInfo info = SkImageInfo::Make(imageWidth, imageHeight, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkImage> image = SkImages::RasterFromData(info, data, imageWidth * 4);
    if (!image) {
        spdlog::error("{}: can not create image from file {}", __func__, filePath);
        return nullptr;
    }
    spdlog::info("{}: successful create image from file {}", __func__, filePath);
    return image;
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

std::unique_ptr<renderengine::skia::SkiaGpuContext> createContexts(sk_sp<const GrGLInterface> glInterface) {
    sSKSLCacheMonitor.setFilename("./shaderCacheTest");
    sSKSLCacheMonitor.initShaderDiskCache();
    std::unique_ptr<renderengine::skia::SkiaGpuContext> context =
        renderengine::skia::SkiaGpuContext::MakeGL_Ganesh(glInterface, sSKSLCacheMonitor);

    return context;
}


static inline bool layerHasBlur(const renderengine::LayerSettings& layer,
                                bool colorTransformModifiesAlpha) {
    if (layer.backgroundBlurRadius > 0 || layer.blurRegions.size()) {
        // return false if the content is opaque and would therefore occlude the blur
        const bool opaqueContent = !layer.source.buffer.buffer || layer.source.buffer.isOpaque;
        const bool opaqueAlpha = layer.alpha == 1.0f && !colorTransformModifiesAlpha;
        return layer.skipContentDraw || !(opaqueContent && opaqueAlpha);
    }
    return false;
}

static inline SkColor getSkColor(const SkV4& color) {
    return SkColorSetARGB(color.x * 255, color.y * 255, color.z * 255, color.w * 255);
}

static inline SkPoint3 getSkPoint3(const SkV3& vector) {
    return SkPoint3::Make(vector.x, vector.y, vector.z);
}

static SkColorMatrix toSkColorMatrix(const ::mat4& matrix) {
    return SkColorMatrix(matrix[0][0], matrix[1][0], matrix[2][0], matrix[3][0], 0, matrix[0][1],
                         matrix[1][1], matrix[2][1], matrix[3][1], 0, matrix[0][2], matrix[1][2],
                         matrix[2][2], matrix[3][2], 0, matrix[0][3], matrix[1][3], matrix[2][3],
                         matrix[3][3], 0);
}

static inline SkM44 getSkM44(const ::mat4& matrix) {
    return SkM44(matrix[0][0], matrix[1][0], matrix[2][0], matrix[3][0],
                 matrix[0][1], matrix[1][1], matrix[2][1], matrix[3][1],
                 matrix[0][2], matrix[1][2], matrix[2][2], matrix[3][2],
                 matrix[0][3], matrix[1][3], matrix[2][3], matrix[3][3]);
}

static SkRRect getBlurRRect(const BlurRegion& region) {
    const auto rect = SkRect::MakeLTRB(region.left, region.top, region.right, region.bottom);
    const SkVector radii[4] = {SkVector::Make(region.cornerRadiusTL, region.cornerRadiusTL),
                               SkVector::Make(region.cornerRadiusTR, region.cornerRadiusTR),
                               SkVector::Make(region.cornerRadiusBR, region.cornerRadiusBR),
                               SkVector::Make(region.cornerRadiusBL, region.cornerRadiusBL)};
    SkRRect roundedRect;
    roundedRect.setRectRadii(rect, radii);
    return roundedRect;
}

class AutoSaveRestore {
public:
    AutoSaveRestore(SkCanvas* canvas) : mCanvas(canvas) { mSaveCount = canvas->save(); }
    ~AutoSaveRestore() { restore(); }
    void replace(SkCanvas* canvas) {
        mCanvas = canvas;
        mSaveCount = canvas->save();
    }
    void restore() {
        if (mCanvas) {
            mCanvas->restoreToCount(mSaveCount);
            mCanvas = nullptr;
        }
    }

private:
    SkCanvas* mCanvas;
    int mSaveCount;
};

static inline SkRect getSkRect(const ui::FloatRect& rect) {
    return SkRect::MakeLTRB(rect.left, rect.top, rect.right, rect.bottom);
}

/**
 *  Verifies that common, simple bounds + clip combinations can be converted into
 *  a single RRect draw call returning true if possible. If true the radii parameter
 *  will be filled with the correct radii values that combined with bounds param will
 *  produce the insected roundRect. If false, the returned state of the radii param is undefined.
 */
static bool intersectionIsRoundRect(const SkRect& bounds, const SkRect& crop,
                                    const SkRect& insetCrop, const vec2& cornerRadius,
                                    SkVector radii[4]) {
    const bool leftEqual = bounds.fLeft == crop.fLeft;
    const bool topEqual = bounds.fTop == crop.fTop;
    const bool rightEqual = bounds.fRight == crop.fRight;
    const bool bottomEqual = bounds.fBottom == crop.fBottom;

    // In the event that the corners of the bounds only partially align with the crop we
    // need to ensure that the resulting shape can still be represented as a round rect.
    // In particular the round rect implementation will scale the value of all corner radii
    // if the sum of the radius along any edge is greater than the length of that edge.
    // See https://www.w3.org/TR/css-backgrounds-3/#corner-overlap
    const bool requiredWidth = bounds.width() > (cornerRadius.x * 2);
    const bool requiredHeight = bounds.height() > (cornerRadius.y * 2);
    if (!requiredWidth || !requiredHeight) {
        return false;
    }

    // Check each cropped corner to ensure that it exactly matches the crop or its corner is
    // contained within the cropped shape and does not need rounded.
    // compute the UpperLeft corner radius
    if (leftEqual && topEqual) {
        radii[0].set(cornerRadius.x, cornerRadius.y);
    } else if ((leftEqual && bounds.fTop >= insetCrop.fTop) ||
               (topEqual && bounds.fLeft >= insetCrop.fLeft)) {
        radii[0].set(0, 0);
    } else {
        return false;
    }
    // compute the UpperRight corner radius
    if (rightEqual && topEqual) {
        radii[1].set(cornerRadius.x, cornerRadius.y);
    } else if ((rightEqual && bounds.fTop >= insetCrop.fTop) ||
               (topEqual && bounds.fRight <= insetCrop.fRight)) {
        radii[1].set(0, 0);
    } else {
        return false;
    }
    // compute the BottomRight corner radius
    if (rightEqual && bottomEqual) {
        radii[2].set(cornerRadius.x, cornerRadius.y);
    } else if ((rightEqual && bounds.fBottom <= insetCrop.fBottom) ||
               (bottomEqual && bounds.fRight <= insetCrop.fRight)) {
        radii[2].set(0, 0);
    } else {
        return false;
    }
    // compute the BottomLeft corner radius
    if (leftEqual && bottomEqual) {
        radii[3].set(cornerRadius.x, cornerRadius.y);
    } else if ((leftEqual && bounds.fBottom <= insetCrop.fBottom) ||
               (bottomEqual && bounds.fLeft >= insetCrop.fLeft)) {
        radii[3].set(0, 0);
    } else {
        return false;
    }

    return true;
}

static inline std::pair<SkRRect, SkRRect> getBoundsAndClip(const ui::FloatRect& boundsRect,
                                                           const ui::FloatRect& cropRect,
                                                           const vec2& cornerRadius) {
    const SkRect bounds = getSkRect(boundsRect);
    const SkRect crop = getSkRect(cropRect);

    SkRRect clip;
    if (cornerRadius.x > 0 && cornerRadius.y > 0) {
        // it the crop and the bounds are equivalent or there is no crop then we don't need a clip
        if (bounds == crop || crop.isEmpty()) {
            return {SkRRect::MakeRectXY(bounds, cornerRadius.x, cornerRadius.y), clip};
        }

        // This makes an effort to speed up common, simple bounds + clip combinations by
        // converting them to a single RRect draw. It is possible there are other cases
        // that can be converted.
        if (crop.contains(bounds)) {
            const auto insetCrop = crop.makeInset(cornerRadius.x, cornerRadius.y);
            if (insetCrop.contains(bounds)) {
                return {SkRRect::MakeRect(bounds), clip}; // clip is empty - no rounding required
            }

            SkVector radii[4];
            if (intersectionIsRoundRect(bounds, crop, insetCrop, cornerRadius, radii)) {
                SkRRect intersectionBounds;
                intersectionBounds.setRectRadii(bounds, radii);
                return {intersectionBounds, clip};
            }
        }

        // we didn't hit any of our fast paths so set the clip to the cropRect
        clip.setRectXY(crop, cornerRadius.x, cornerRadius.y);
    }

    // if we hit this point then we either don't have rounded corners or we are going to rely
    // on the clip to round the corners for us
    return {SkRRect::MakeRect(bounds), clip};
}

void drawShadow(SkCanvas* canvas,
                                  const SkRRect& casterRRect,
                                  const ui::ShadowSettings& settings) {
    const float casterZ = settings.length / 2.0f;
    const auto flags =
            settings.casterIsTranslucent ? SkShadowFlags::kTransparentOccluder_ShadowFlag : SkShadowFlags::kNone_ShadowFlag;

    SkShadowUtils::DrawShadow(canvas, SkPath::RRect(casterRRect), SkPoint3::Make(0, 0, casterZ),
                              getSkPoint3(settings.lightPos), settings.lightRadius,
                              getSkColor(settings.ambientColor), getSkColor(settings.spotColor),
                              flags);
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

void initGrContextOptions(GrContextOptions& options) {
    options.fPreferExternalImagesOverES3 = true;
    options.fDisableDistanceFieldPaths = false;
    options.fReduceOpsTaskSplitting = GrContextOptions::Enable::kNo;
}

int main(int argc, char* argv[]) {
    // args parser
    CLI::App app{"Skia Fiddle"};
    app.add_option("-W,--width", DRAW_WIDTH, "Canvas draw width")
        ->check(CLI::Range(16, 4028))
        ->default_val(256);
    app.add_option("-H,--height", DRAW_HEIGHT, "Canvas draw height")
        ->check(CLI::Range(16, 8056))
        ->default_val(256);
    app.add_option("-S,--save", SAVE_BITMAP, "Save canvas draw to png file")
        ->check(CLI::IsMember({0, 1}))
        ->default_val(0);
    app.add_option("-R,--resource", RESOURCE_ID, "Resource id for program loading image")
        ->check(CLI::Range(0, 5))
        ->default_val(2);
    app.add_option("-P,--picture", SAVE_SKP, "Save canvas draw to skp file")
        ->check(CLI::IsMember({0, 1}))
        ->default_val(0);
    app.add_option("-L,--loglevel", LOG_LEVEL, "Set leg level for spdlog")
        ->check(CLI::Range(spdlog::level::trace, spdlog::level::off))
        ->default_val(spdlog::level::info);
    //catch exception and parse the command lines
    CLI11_PARSE(app, argc, argv);

#if 1
    spdlog::set_level(static_cast<spdlog::level::level_enum>(LOG_LEVEL));
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%-4l%$] %v");
#endif

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
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(DRAW_WIDTH, DRAW_HEIGHT, "Skia + GLFW", NULL, NULL);
    if (!window) {
        glfwTerminate();
        spdlog::critical("Can not create glfw window handle! terminate it!");
        return -1;
    } else {
        glfwSetWindowSizeLimits(window, 
            GLFW_DONT_CARE, GLFW_DONT_CARE,
            GLFW_DONT_CARE, GLFW_DONT_CARE);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        spdlog::critical("Failed to Initialize GLAD");
        return -1;
    }

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

    // --------begin create Skia gpu context --------
    std::unique_ptr<renderengine::skia::SkiaGpuContext> context = createContexts(glInterface);
    if (!context) {
        spdlog::error("Can not create Skia Gpu Context");
    }
    // ----------end of create Skia gpu context --------

#if 0
    sk_sp<GrDirectContext> grContext(GrDirectContexts::MakeGL(glInterface));
    if (grContext.get() == nullptr) {
        spdlog::error("Can not create GrDirectContext for Skia");
        return 2;
    }
#else
    sk_sp<GrDirectContext> grContext = context->grDirectContext();
    if (grContext.get() == nullptr) {
        spdlog::error("Can not create GrDirectContext for Skia");
        return 2;
    }
#endif

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
    // -------- end of glfw and skia gpu context init --------

    //glfwSwapInterval(1);

    // ------- begin create blur filter --------
    renderengine::skia::BlurFilter* blurFilter = new renderengine::skia::GaussianBlurFilter();
    if (!blurFilter) {
        spdlog::error("Can not create Blur Filter");
    }
    // --------end create blur filter --------

    // -----------start of renderengine layer settings prepare -----------
    renderengine::DisplaySettings displaySettings;
    std::vector<renderengine::LayerSettings> layers;

#ifdef RENDER_DEMO_1
    renderengine::LayerSettings bbqLayerSettings;
    bbqLayerSettings.name = "Wallpaper BBQ wrapper#94";
    bbqLayerSettings.source.buffer.buffer = createImageFromFile(
        "./raw/20251214_145123/@19@layer@100@3000x2120_bpp_1.raw",
         3000,
         2120);
    bbqLayerSettings.alpha = 1.00000f;
    bbqLayerSettings.sourceDataspace = ui::Dataspace::HAL_DATASPACE_V0_SRGB;

    bbqLayerSettings.geometry.boundaries = ui::FloatRect(0, 0, 3000, 2120);
    bbqLayerSettings.geometry.roundedCornersCrop = ui::FloatRect(0, 0, 3000, 2120);

    bbqLayerSettings.shadow.length = 0.000000f;
    bbqLayerSettings.borderSettings.strokeWidth = 0.0f;
    bbqLayerSettings.borderSettings.color = 0x00;

    layers.push_back(bbqLayerSettings);

    renderengine::LayerSettings launcherLayerSettings;
    launcherLayerSettings.name = "Launcher#1049";
    launcherLayerSettings.source.buffer.buffer = createImageFromFile(
        "./raw/20251214_145123/@20@layer@1049@3000x2120_bpp_1.raw",
            3000,
            2120);
    launcherLayerSettings.source.buffer.isOpaque = false;

    launcherLayerSettings.geometry.boundaries = ui::FloatRect(0.000000, 0.000000, 3000.000000, 2120.000000);
    launcherLayerSettings.geometry.roundedCornersCrop = ui::FloatRect(0.000000, 0.000000, 3000.000000, 2120.000000);

    launcherLayerSettings.shadow.length = 0.000000f;
    launcherLayerSettings.borderSettings.strokeWidth = 0.0f;
    launcherLayerSettings.borderSettings.color = 0x00;


    launcherLayerSettings.alpha = 1.0f;
    launcherLayerSettings.sourceDataspace = ui::Dataspace::HAL_DATASPACE_DISPLAY_P3;
    launcherLayerSettings.backgroundBlurRadius = 240;


    renderengine::LayerSettings statusBarLayerSettings;
    statusBarLayerSettings.name = "StatusBar#88";
    statusBarLayerSettings.source.buffer.buffer = createImageFromFile(
        "./raw/20251214_145123/@21@layer@85@3000x84_bpp_1.raw",
            3000,
            84);

    statusBarLayerSettings.alpha = 1.0f;
    statusBarLayerSettings.sourceDataspace = ui::Dataspace::HAL_DATASPACE_V0_SRGB;
    statusBarLayerSettings.geometry.boundaries = ui::FloatRect(0, 0, 3000, 84);
    statusBarLayerSettings.geometry.roundedCornersCrop = ui::FloatRect(0, 0, 3000, 84);

    statusBarLayerSettings.shadow.length = 0.000000f;
    statusBarLayerSettings.borderSettings.strokeWidth = 0.0f;
    statusBarLayerSettings.borderSettings.color = 0x00;

    layers.push_back(statusBarLayerSettings);
#endif

#if 1
    renderengine::LayerSettings bbqLayerSettings;
    bbqLayerSettings.name = "Wallpaper BBQ wrapper#100";
    bbqLayerSettings.source.buffer.buffer = createImageFromFile(
        "./raw/20251217_234809/@24@layer@100@3600x2544_bpp_1.raw",
        3600,
        2544);
    bbqLayerSettings.alpha = 1.00000f;
    bbqLayerSettings.sourceDataspace = ui::Dataspace::HAL_DATASPACE_V0_SRGB;

    bbqLayerSettings.geometry.boundaries = ui::FloatRect(0, 0, 3000, 2120);
    bbqLayerSettings.geometry.roundedCornersCrop = ui::FloatRect(0, 0, 3000, 2120);

    bbqLayerSettings.shadow.length = 0.000000f;
    bbqLayerSettings.borderSettings.strokeWidth = 0.0f;
    bbqLayerSettings.borderSettings.color = 0x00;

    layers.push_back(bbqLayerSettings);

    renderengine::LayerSettings launcherLayerSettings;
    launcherLayerSettings.name = "com.android.launcher.Launcher#12588";
    launcherLayerSettings.source.buffer.buffer = createImageFromFile(
        "./raw/20251217_234809/@25@layer@12588@3000x2120_bpp_1.raw",
            3000,
            2120);
    launcherLayerSettings.source.buffer.isOpaque = false;

    launcherLayerSettings.geometry.boundaries = ui::FloatRect(0.000000, 0.000000, 3000.000000, 2120.000000);
    launcherLayerSettings.geometry.roundedCornersCrop = ui::FloatRect(0.000000, 0.000000, 3000.000000, 2120.000000);

    launcherLayerSettings.shadow.length = 0.000000f;
    launcherLayerSettings.borderSettings.strokeWidth = 0.0f;
    launcherLayerSettings.borderSettings.color = 0x00;


    launcherLayerSettings.alpha = 1.0f;
    launcherLayerSettings.sourceDataspace = ui::Dataspace::HAL_DATASPACE_DISPLAY_P3;
    launcherLayerSettings.backgroundBlurRadius = 19;

    layers.push_back(launcherLayerSettings);

#endif

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        rebuildSurface();
        sk_sp<SkSurface> dstSurface = skSurface;
        SkCanvas* dstCanvas = dstSurface->getCanvas();
        if (dstCanvas == nullptr) {
            spdlog::error("Cannot acquire canvas from Skia.");
            break;
        }

        sk_sp<SkColorFilter> displayColorTransform;
        if (displaySettings.colorTransform != mat4() && !displaySettings.deviceHandlesColorTransform) {
            displayColorTransform = SkColorFilters::Matrix(toSkColorMatrix(displaySettings.colorTransform));
        }
        const bool ctModifiesAlpha =
            displayColorTransform && !displayColorTransform->isAlphaUnchanged();

        sk_sp<SkSurface> activeSurface(dstSurface);
        SkCanvas *canvas = dstCanvas;

        const renderengine::LayerSettings* blurCompositeLayer = nullptr;
        if (blurFilter) {
            bool requireCompositionLayer = false;
            for (const auto& layer : layers) {
                if (!layerHasBlur(layer, ctModifiesAlpha)) {
                    continue;
                }
                if (layer.backgroundBlurRadius > 0 &&
                    layer.backgroundBlurRadius < blurFilter->getMaxCrossFadeRadius()) {
                        requireCompositionLayer = true;
                }
                for (auto region : layer.blurRegions) {
                    if (region.blurRadius < blurFilter->getMaxCrossFadeRadius()) {
                        requireCompositionLayer = true;
                    }
                }
                if (requireCompositionLayer) {
                    activeSurface = dstSurface->makeSurface(dstSurface->imageInfo());
                    canvas = activeSurface->getCanvas();
                    blurCompositeLayer = &layer;
                    break;
                }
            }
        }

        AutoSaveRestore surfaceAutoSaveRestore(canvas);
        canvas->clear(SK_ColorTRANSPARENT);

        float currentTime = static_cast<float>(glfwGetTime());

        for (const auto& layer : layers) {
            sk_sp<SkImage> blurInput;
            if (blurCompositeLayer == &layer) {
                blurInput = activeSurface->makeTemporaryImage();

                if (layer.blurRegions.size()) {
                    SkPaint paint;
                    paint.setBlendMode(SkBlendMode::kSrc);
                    dstCanvas->drawImage(blurInput, 0, 0, SkSamplingOptions(), &paint);
                }

                canvas = dstCanvas;
                surfaceAutoSaveRestore.replace(canvas);
                activeSurface = dstSurface;
            }

            SkAutoCanvasRestore layerAutoRestore(canvas, true);
            canvas->concat(getSkM44(layer.geometry.positionTransform).asM33());

            const auto [bounds, roundRectClip] =
                getBoundsAndClip(layer.geometry.boundaries,
                                 layer.geometry.roundedCornersCrop,
                                 layer.geometry.roundedCornersRadius);

            if (blurFilter && layerHasBlur(layer, ctModifiesAlpha)) {
                std::unordered_map<uint32_t, sk_sp<SkImage> > cachedBlurs;

                SkRect blurRect = canvas->getTotalMatrix().mapRect(bounds.rect());
                spdlog::debug("[{}:{}]blurRect = [{},{},{},{}]", __func__, __LINE__,
                    blurRect.fLeft, blurRect.fTop, blurRect.fRight, blurRect.fBottom);
                if (!blurRect.intersect(SkRect::Make(canvas->getDeviceClipBounds()))) {
                    spdlog::warn("Blur rect does not intersect clip bounds.");
                }

                SkAutoCanvasRestore arc(canvas, true);
                if (!roundRectClip.isEmpty()) {
                    canvas->clipRRect(roundRectClip, true);
                }

                if (blurRect.width() > 0 && blurRect.height() > 0) {
                    if (!blurInput) {
                        bool requireCrossFadeWithBlurInput = false;
                        if (layer.backgroundBlurRadius > 0 &&
                            layer.backgroundBlurRadius < blurFilter->getMaxCrossFadeRadius()) {
                                requireCrossFadeWithBlurInput = true;
                        }
                        for (auto region : layer.blurRegions) {
                            if (region.blurRadius < blurFilter->getMaxCrossFadeRadius()) {
                                requireCrossFadeWithBlurInput = true;
                            }
                        }
                        spdlog::debug("{}:{} requireCrossFadeWithBlurInput={}", __func__, __LINE__, requireCrossFadeWithBlurInput);
                        if (requireCrossFadeWithBlurInput) {
                            blurInput = activeSurface->makeImageSnapshot();
                        } else {
                            blurInput = activeSurface->makeTemporaryImage();
                        }
                    }

                    if (layer.backgroundBlurRadius > 0) {
                        spdlog::debug("BackgroundBlur");
                        auto blurredImage = blurFilter->generate(context.get(),
                            layer.backgroundBlurRadius, blurInput, blurRect);
                        cachedBlurs[layer.backgroundBlurRadius] = blurredImage;

                        blurFilter->drawBlurRegion(canvas, bounds, layer.backgroundBlurRadius,
                            1.0f, blurRect, blurredImage, blurInput);
                    }

                    canvas->concat(getSkM44(layer.blurRegionTransform).asM33());
                    for (auto region : layer.blurRegions) {
                        if (cachedBlurs[region.blurRadius] == nullptr) {
                            spdlog::debug("BlurRegion");
                            cachedBlurs[region.blurRadius] =
                                blurFilter->generate(context.get(), region.blurRadius, blurInput, blurRect);
                        }

                        blurFilter->drawBlurRegion(canvas, getBlurRRect(region),
                            region.blurRadius, region.alpha, blurRect,
                            cachedBlurs[region.blurRadius], blurInput);
                    }
                }
            }

            if (layer.shadow.length > 0) {
                SkRRect shadowBounds, shadowClip;
                if (layer.geometry.boundaries == layer.shadow.boundaries) {
                    shadowBounds = bounds;
                    shadowClip = roundRectClip;
                } else {
                    std::tie(shadowBounds, shadowClip) =
                        getBoundsAndClip(layer.shadow.boundaries,
                                            layer.geometry.roundedCornersCrop,
                                            layer.geometry.roundedCornersRadius);
                }

                // Technically, if bounds is a rect and roundRectClip is not empty,
                // it means that the bounds and roundedCornersCrop were different
                // enough that we should intersect them to find the proper shadow.
                // In practice, this often happens when the two rectangles appear to
                // not match due to rounding errors. Draw the rounded version, which
                // looks more like the intent.
                const auto& rrect =
                    shadowBounds.isRect() && !shadowClip.isEmpty() ? shadowClip : shadowBounds;
                drawShadow(canvas, rrect, layer.shadow);
            }

            if (layer.borderSettings.strokeWidth > 0) {
                SkRRect originalBounds, originalClip;
                std::tie(originalBounds, originalClip) =
                    getBoundsAndClip(layer.geometry.boundaries,
                                        layer.geometry.roundedCornersCrop,
                                        layer.geometry.roundedCornersRadius);
                const SkRRect& perferredOriginalBounds =
                    originalBounds.isRect() && !originalClip.isEmpty()
                        ? originalClip
                        : originalBounds;
                SkRRect outlineRect = perferredOriginalBounds;
                outlineRect.outset(layer.borderSettings.strokeWidth,
                                    layer.borderSettings.strokeWidth);
                    
                SkPaint paint;
                paint.setAntiAlias(true);
                paint.setColor(layer.borderSettings.color);
                paint.setStyle(SkPaint::kFill_Style);
                canvas->drawDRRect(outlineRect, perferredOriginalBounds, paint);
            }

            const ui::Dataspace layerDataspace = layer.sourceDataspace;
            SkPaint paint;
            if (layer.source.buffer.buffer) {
                const auto& item = layer.source.buffer;
                const bool useIsOpaqueWorkaround = item.isOpaque &&
                    (item.buffer->colorType() == kRGBA_1010102_SkColorType ||
                    item.buffer->colorType() == kRGBA_F16_SkColorType);
                const auto alphaType = useIsOpaqueWorkaround ? kPremul_SkAlphaType
                    : item.isOpaque                     ? kOpaque_SkAlphaType
                    : item.usePremultipliedAlpha        ? kPremul_SkAlphaType
                                                        : kUnpremul_SkAlphaType;

                sk_sp<SkImage> image = item.buffer;
                auto texMatrix = getSkM44(item.textureTransform).asM33();
                texMatrix.preScale(1.0f / bounds.width(), 1.0f / bounds.height());
                texMatrix.postScale(image->width(), image->height());

                SkMatrix matrix;
                if (!texMatrix.invert(&matrix)) {
                    matrix = texMatrix;
                }
                // The shader does not respect the translation, so we add it to the texture
                // transform for the SkImage. This will make sure that the correct layer contents
                // are drawn in the correct part of the screen.
                matrix.postTranslate(bounds.rect().fLeft, bounds.rect().fTop);

                sk_sp<SkShader> shader;

                if (layer.source.buffer.useTextureFiltering) {
                    shader = image->makeShader(
                        SkTileMode::kClamp,
                        SkTileMode::kClamp,
                        SkSamplingOptions({SkFilterMode::kLinear, SkMipmapMode::kNone}),
                        &matrix);
                } else {
                    shader = image->makeShader(SkSamplingOptions(), matrix);
                }

                if (useIsOpaqueWorkaround) {
                    shader = SkShaders::Blend(SkBlendMode::kPlus, shader,
                        SkShaders::Color(SkColors::kBlack, renderengine::skia::toSkColorSpace(layerDataspace)));
                }

                SkRect imageBounds;
                matrix.mapRect(&imageBounds, SkRect::Make(image->bounds()));

                paint.setShader(shader);
            } else {
                const auto color = layer.source.solidColor;
                sk_sp<SkShader> shader = SkShaders::Color(SkColor4f{.fR = color.x,
                                                                    .fG = color.y,
                                                                    .fB = color.z,
                                                                    .fA = layer.alpha},
                                    renderengine::skia::toSkColorSpace(layerDataspace));
                paint.setShader(shader);
            }

            if (layer.disableBlending) {
                paint.setBlendMode(SkBlendMode::kSrc);
            }

            if (!roundRectClip.isEmpty()) {
                canvas->clipRRect(roundRectClip, true);
            }

            if (!bounds.isRect()) {
                paint.setAntiAlias(true);
                canvas->drawRRect(bounds, paint);
            } else {
                canvas->drawRect(bounds.rect(), paint);
            }
        }

        // --------- flush and submit ---------
        grContext->flushAndSubmit(GrSyncCpu::kYes);

        // --------- swap buffers ---------
        glfwSwapBuffers(window);
    }

    skSurface.reset();
    grContext.reset();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}