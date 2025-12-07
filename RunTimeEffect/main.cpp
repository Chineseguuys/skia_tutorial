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
#include "Skia/include/config/SkUserConfig.h"
#include "Skia/include/core/SkColorSpace.h"
#include "Skia/include/core/SkPath.h"
#include "Skia/include/core/SkRRect.h"
#include "Skia/include/core/SkDrawable.h"
#include "Skia/include/core/SkShader.h"
#include "Skia/include/core/SkRect.h"
#include "Skia/include/effects/SkImageFilters.h"
#include "Skia/include/core/SkMatrix.h"
#include "Skia/include/effects/SkRuntimeEffect.h"

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
#include "include/core/SkFont.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkPoint_impl.h"

#include <iomanip>
#include <chrono>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <vector>

// modified for compile error
#ifdef Success
#undef Success
#include "CLI/CLI.hpp"
#endif

#include "backward.hpp"

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

/**
 * 打印 SkMatrix 矩阵的详细信息
 * 
 * @param matrix 要打印的 SkMatrix 对象
 * @param name   矩阵名称（可选），用于标识输出
 * @param precision 浮点数打印精度（小数位数）
 */
void printSkMatrix(const SkMatrix& matrix, const char* name = "SkMatrix", int precision = 2) {
    // 设置浮点数格式
    char format[16];
    snprintf(format, sizeof(format), "%%.%df", precision);

    // 打印标题
    if (name && *name) {
        printf("\n%s:\n", name);
    } else {
        printf("\nSkMatrix:\n");
    }

    // 获取矩阵元素（使用行主序表示）
    SkScalar buffer[9];
    matrix.get9(buffer);

    // 打印矩阵内容
    printf("┌ "); printf(format, buffer[0]); printf("    "); 
              printf(format, buffer[1]); printf("    "); 
              printf(format, buffer[2]); printf(" ┐\n");

    printf("│ "); printf(format, buffer[3]); printf("    "); 
              printf(format, buffer[4]); printf("    "); 
              printf(format, buffer[5]); printf(" │\n");

    printf("└ "); printf(format, buffer[6]); printf("    "); 
              printf(format, buffer[7]); printf("    "); 
              printf(format, buffer[8]); printf(" ┘\n");

    return;
}


// 一个最简单的 Runtime Effect 示例，绘制一个全屏的纯色
class SimpleRuntimeEffect {
public:
    static constexpr const char* skShaderCode = R"(
        uniform half4 uColor;

        half4 main(float2 fragCoord) {
            return uColor;
        }
    )";

    struct Uniform {
        std::string sName;
        std::vector<uint8_t> sData;
    };

    SimpleRuntimeEffect() = default;
    ~SimpleRuntimeEffect() = default;

    void setCanvasSize(int width, int height) {
        mCanvasWidth = width;
        mCanvasHeight = height;
    }

    void drawSolidColor(SkCanvas* canvas, SkColor4f color) {
        SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForShader(SkString(skShaderCode));
        if (!result.effect) {
            spdlog::error("{}: can not create runtime effect!", __FUNCTION__);
            return;
        }

        sk_sp<SkData> colorData = SkData::MakeWithCopy(&color, sizeof(SkColor4f));
        Uniform uniform;
        uniform.sName = "uColor";
        uniform.sData.resize(colorData->size());
        std::memcpy(uniform.sData.data(), colorData->data(), colorData->size());
        SkRuntimeShaderBuilder shaderBuilder(result.effect);
        shaderBuilder.uniform("uColor").set(uniform.sData.data(), uniform.sData.size());

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


class GradientRuntimeEffect {
public:
    static constexpr const char* skShaderCode = R"(
        uniform float2 u_resolution;  // 画布分辨率
        uniform half4 u_startColor;   // 起始颜色
        uniform half4 u_endColor;     // 结束颜色

        half4 main(float2 coord) {
            // 计算归一化的x坐标（0到1）
            float tx = coord.x / u_resolution.x;
            float ty = coord.y / u_resolution.y;
            float t  = sqrt(tx * tx + ty * ty) / sqrt(2.0); // 使用距离计算插值因子

            // 线性插值混合颜色
            return mix(u_startColor, u_endColor, t);
        }
    )";

    struct Uniform {
        std::string sName;
        std::vector<uint8_t> sData;
    };

    GradientRuntimeEffect() = default;

    void initlize(int width, int height) {
        mCanvasWidth = width;
        mCanvasHeight = height;
    }

    void draw(SkCanvas* canvas, SkColor4f startColor, SkColor4f endColor) {
        SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForShader(SkString(skShaderCode));
        if (!result.effect) {
            spdlog::error("{}: can not create runtime effect!", __FUNCTION__);
            return;
        }

        SkRuntimeShaderBuilder shaderBuilder(result.effect);

        // 设置分辨率 uniform
        SkPoint resolution = SkPoint::Make((float)mCanvasWidth, (float)mCanvasHeight);
        sk_sp<SkData> resolutionData = SkData::MakeWithCopy(&resolution, sizeof(SkPoint));
        Uniform resolutionUniform;
        resolutionUniform.sName = "u_resolution";
        resolutionUniform.sData.resize(resolutionData->size());
        std::memcpy(resolutionUniform.sData.data(), resolutionData->data(), resolutionData->size());
        shaderBuilder.uniform(resolutionUniform.sName.c_str())
            .set(resolutionUniform.sData.data(), resolutionUniform.sData.size());

        // 设置起始颜色 uniform
        sk_sp<SkData> startColorData = SkData::MakeWithCopy(&startColor, sizeof(SkColor4f));
        Uniform startColorUniform;
        startColorUniform.sName = "u_startColor";
        startColorUniform.sData.resize(startColorData->size());
        std::memcpy(startColorUniform.sData.data(), startColorData->data(), startColorData->size());
        shaderBuilder.uniform(startColorUniform.sName.c_str())
            .set(startColorUniform.sData.data(), startColorUniform.sData.size());

        // 设置结束颜色 uniform
        sk_sp<SkData> endColorData = SkData::MakeWithCopy(&endColor, sizeof(SkColor4f));
        Uniform endColorUniform;
        endColorUniform.sName = "u_endColor";
        endColorUniform.sData.resize(endColorData->size());
        std::memcpy(endColorUniform.sData.data(), endColorData->data(), endColorData->size());
        shaderBuilder.uniform(endColorUniform.sName.c_str())
            .set(endColorUniform.sData.data(), endColorUniform.sData.size());

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

class PlusingCircleEffect {
public:
    static constexpr const char* skShaderCode = R"(
        uniform float2 u_resolution;  // 画布分辨率
        uniform float u_time;         // 时间变量

        half4 main(float2 coord) {
            float2 center = u_resolution / 2.0;
            float dist = distance(coord, center);
            float radius = 50.0 + 20.0 * sin(u_time * 5.0);
            half alpha = smoothstep(radius, radius + 5.0, dist);
            return half4(1.0, 0.0, 0.0, 1.0 - alpha);
        }
    )";

    struct Uniform {
        std::string sName;
        std::vector<uint8_t> sData;
    };

    PlusingCircleEffect() = default;

    void initlize(int width, int height) {
        mCanvasWidth = width;
        mCanvasHeight = height;
    }

    void draw(SkCanvas* canvas, float time) {
        SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForShader(SkString(skShaderCode));
        if (!result.effect) {
            spdlog::error("{}: can not create runtime effect!", __FUNCTION__);
            return;
        }

        Uniform uniform;
        uniform.sName = "u_time";
        uniform.sData.resize(sizeof(float));
        std::memcpy(uniform.sData.data(), &time, sizeof(float));

        SkPoint resolution = SkPoint::Make((float)mCanvasWidth, (float)mCanvasHeight);
        Uniform resolutionUniform;
        resolutionUniform.sName = "u_resolution";
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

class CuteLittleFibonacciSphereEffect {
public:
    static constexpr const char* skShaderCode = R"(
        uniform float2 iResolution;  // 画布分辨率
        uniform float iTime;         // 时间变量

        half4 main(float2 FC) {
            vec4 o = vec4(0);
            vec2 p = vec2(0), c=p, u=FC.xy*2.-iResolution.xy;
            float a;
            for (float i=0; i<4e2; i++) {
                a = i/2e2-1.;
                p = cos(i*2.4+iTime+vec2(0,11))*sqrt(1.-a*a);
                c = u/iResolution.y+vec2(p.x,a)/(p.y+2.);
                o += (cos(i+vec4(0,2,4,0))+1.)/dot(c,c)*(1.-p.y)/3e4;
            }
            return o;
        }
    )";

    struct Uniform {
        std::string sName;
        std::vector<uint8_t> sData;
    };

    CuteLittleFibonacciSphereEffect() = default;
    ~CuteLittleFibonacciSphereEffect() = default;

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

    spdlog::info("Canvas: [wxh]=[{}x{}]", DRAW_WIDTH, DRAW_HEIGHT);

    fontMgr = SkFontMgr_New_Custom_Directory(fontDir.c_str());
    if (fontMgr == nullptr) {
        spdlog::error("{}: can not create font manager!", __FUNCTION__);
    }

    int fontFamilyCounts = fontMgr->countFamilies();
    spdlog::debug("{}: font family count is {}", __FUNCTION__, fontFamilyCounts);
    for (int idx = 0; idx < fontFamilyCounts; ++idx) {
        SkString fontFamilyName;
        fontMgr->getFamilyName(idx, &fontFamilyName);
        spdlog::debug("{}: Family[{}]={}", __FUNCTION__, idx, fontFamilyName.c_str());
    }

    const char* fontFamily = nullptr;
    SkFontStyle fontStyle;

    typeFace = fontMgr->legacyMakeTypeface(fontFamily, fontStyle);
    if (typeFace == nullptr) {
        spdlog::error("{}: can not create type face from font manager!", __FUNCTION__);
    }

    SkImageInfo imageInfo = SkImageInfo::Make(
        DRAW_WIDTH, DRAW_HEIGHT,
        kBGRA_8888_SkColorType,
        kOpaque_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::Raster(imageInfo);
    SkCanvas* canvas = surface->getCanvas();

    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(DRAW_WIDTH, DRAW_HEIGHT);
    if (SAVE_SKP) {
        spdlog::debug("{}: replace canvas with recording canvas!", __func__);
        canvas = recordingCanvas;
    }

    canvas->drawColor(SK_ColorTRANSPARENT);

    //--------------- begin draw ----------------
    CuteLittleFibonacciSphereEffect effect;
    effect.initlize(DRAW_WIDTH, DRAW_HEIGHT);
    effect.draw(canvas, 0.0f);
    //--------------- end draw ------------------

    if (SAVE_SKP) {
        sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
        std::string skpFileName = generate_filename("output", "skp");
        savePictureAsSKP(picture, skpFileName.c_str());

        canvas = surface->getCanvas();
        canvas->drawPicture(picture);
    }

    if(SAVE_BITMAP) {
        SkBitmap bitmap;
        bitmap.allocPixels(imageInfo, imageInfo.minRowBytes());
        surface->readPixels(bitmap, 0, 0);

        std::string pngName = generate_filename("output", "png");
        saveBitmapAsPng(bitmap, pngName.c_str());
    }
    return 0;
}