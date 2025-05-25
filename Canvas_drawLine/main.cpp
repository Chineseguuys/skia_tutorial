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
#include "Skia/include/core/SkPathEffect.h"
#include "Skia/include/core/SkRefCnt.h"


#include "Skia/include/core/SkColor.h"
// added and open the SK_DEBUG for SkRefCnt.h:166: fatal error: "assertf(rc == 1): NVRefCnt was 0"
#include "Skia/include/config/SkUserConfig.h"
#include "Skia/include/core/SkColorSpace.h"
#include "Skia/include/core/SkPaint.h"
#include "Skia/include/core/SkPath.h"
#include "Skia/include/core/SkFont.h"
#include "Skia/include/core/SkRRect.h"
#include "Skia/include/core/SkMaskFilter.h"
#include "Skia/include/core/SkDrawable.h"
#include "Skia/include/effects/SkGradientShader.h"

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
#include "include/core/SkSamplingOptions.h"

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

// https://fiddle.skia.org/c/@Canvas_drawLine
void draw0(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xFF9a67be);
    paint.setStrokeWidth(20);
    // 向右侧进行倾斜
    canvas->skew(1, 0);
    canvas->drawLine(32, 96, 32, 160, paint);
    // 向左侧进行倾斜
    canvas->skew(-2, 0);
    canvas->drawLine(288, 96, 288, 160, paint);
}

// https://fiddle.skia.org/c/@Canvas_drawLine_2
void draw1(SkCanvas *canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xFF9a67be);
    paint.setStrokeWidth(20);
    canvas->skew(1, 0);
    canvas->drawLine({32, 96}, {32, 160}, paint);
    canvas->skew(-2, 0);
    canvas->drawLine({288, 96}, {288, 160}, paint);
}

// https://fiddle.skia.org/c/@Canvas_drawOval
void draw2(SkCanvas *canvas) {
    canvas->clear(0xFF3f5f9f);
    SkColor  kColor1 = SkColorSetARGB(0xff, 0xff, 0x7f, 0);
    SkColor  g1Colors[] = { kColor1, SkColorSetA(kColor1, 0x20) };
    SkPoint  g1Points[] = { { 0, 0 }, { 0, 35 } };
    SkScalar pos[] = { 0.2f, 1.0f };
    SkRect bounds = SkRect::MakeWH(80, 70);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setShader(SkGradientShader::MakeLinear(g1Points, g1Colors, pos, std::size(g1Colors),
            SkTileMode::kClamp));
    canvas->drawOval(bounds , paint);
}

// https://fiddle.skia.org/c/@Canvas_drawPaint
void draw3(SkCanvas *canvas) {
    SkColor     colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    SkScalar    pos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    SkPaint     paint;
    paint.setShader(SkGradientShader::MakeSweep(128, 128, colors, pos, std::size(colors)));
    canvas->drawPaint(paint);
}

// https://fiddle.skia.org/c/@Canvas_drawPatch
void draw4(SkCanvas *canvas) {
    // SkBitmap source = cmbkygk;
    SkPaint paint;
    paint.setAntiAlias(true);
    /**
        曲线((1-t)^(3) x(A0)+3 (1-t)^(2) t x(A10)+3 (1-t) t^(2) x(A11)+t^(3) x(A9),(1-t)^(3) y(A0)+3 (1-t)^(2) t y(A10)+3 (1-t) t^(2) y(A11)+t^(3) y(A9),t,0,1)
        曲线((1-t)^(3) x(A0)+3 (1-t)^(2) t x(A2)+3 (1-t) t^(2) x(A1)+t^(3) x(A3),(1-t)^(3) y(A0)+3 (1-t)^(2) t y(A2)+3 (1-t) t^(2) y(A1)+t^(3) y(A3),t,0,1)
        曲线((1-t)^(3) x(A3)+3 (1-t)^(2) t x(A5)+3 (1-t) t^(2) x(A4)+t^(3) x(A6),(1-t)^(3) y(A3)+3 (1-t)^(2) t y(A5)+3 (1-t) t^(2) y(A4)+t^(3) y(A6),t,0,1)
        曲线((1-t)^(3) x(A6)+3 (1-t)^(2) t x(A8)+3 (1-t) t^(2) x(A7)+t^(3) x(A9),(1-t)^(3) y(A6)+3 (1-t)^(2) t y(A8)+3 (1-t) t^(2) y(A7)+t^(3) y(A9),t,0,1)
     */
    SkPoint cubics[] = { { 3, 1 },    { 4, 2 }, { 5, 1 },    { 7, 3 },
                      /* { 7, 3 }, */ { 6, 4 }, { 7, 5 },    { 5, 7 },
                      /* { 5, 7 }, */ { 4, 6 }, { 3, 7 },    { 1, 5 },
                      /* { 1, 5 }, */ { 2, 4 }, { 1, 3 }, /* { 3, 1 } */ };
    SkColor colors[] = { 0xbfff0000, 0xbf0000ff, 0xbfff00ff, 0xbf00ffff };
    SkPoint texCoords[] = { { -30, -30 }, { 162, -30}, { 162, 162}, { -30, 162} };
    paint.setShader(source.makeShader(SkSamplingOptions(SkCubicResampler::Mitchell())));
    canvas->scale(15, 15);
    for (auto blend : { SkBlendMode::kSrcOver, SkBlendMode::kModulate, SkBlendMode::kXor } ) {
        canvas->drawPatch(cubics, colors, texCoords, blend, paint);
        canvas->translate(4, 4);
    }
}

// https://fiddle.skia.org/c/@Canvas_drawPatch_2_b
void draw5(SkCanvas *canvas) {
    // SkBitmap source = checkerboard;
    SkPaint paint;
    paint.setAntiAlias(true);
    /**
        曲线((1-t)^(3) x(A0)+3 (1-t)^(2) t x(A10)+3 (1-t) t^(2) x(A11)+t^(3) x(A9),(1-t)^(3) y(A0)+3 (1-t)^(2) t y(A10)+3 (1-t) t^(2) y(A11)+t^(3) y(A9),t,0,1)
        曲线((1-t)^(3) x(A0)+3 (1-t)^(2) t x(A2)+3 (1-t) t^(2) x(A1)+t^(3) x(A3),(1-t)^(3) y(A0)+3 (1-t)^(2) t y(A2)+3 (1-t) t^(2) y(A1)+t^(3) y(A3),t,0,1)
        曲线((1-t)^(3) x(A3)+3 (1-t)^(2) t x(A5)+3 (1-t) t^(2) x(A4)+t^(3) x(A6),(1-t)^(3) y(A3)+3 (1-t)^(2) t y(A5)+3 (1-t) t^(2) y(A4)+t^(3) y(A6),t,0,1)
        曲线((1-t)^(3) x(A6)+3 (1-t)^(2) t x(A8)+3 (1-t) t^(2) x(A7)+t^(3) x(A9),(1-t)^(3) y(A6)+3 (1-t)^(2) t y(A8)+3 (1-t) t^(2) y(A7)+t^(3) y(A9),t,0,1)
     */
    SkPoint cubics[] = { { 3, 1 },    { 4, 2 }, { 5, 1 },    { 7, 3 },
                      /* { 7, 3 }, */ { 6, 4 }, { 7, 5 },    { 5, 7 },
                      /* { 5, 7 }, */ { 4, 6 }, { 3, 7 },    { 1, 5 },
                      /* { 1, 5 }, */ { 2, 4 }, { 1, 3 }, /* { 3, 1 } */ };
    SkPoint texCoords[] = { { 0, 0 }, { 0, 63}, { 63, 63}, { 63, 0 } };
    paint.setShader(source.makeShader(SkSamplingOptions(SkFilterMode::kLinear)));
    canvas->scale(30, 30);
    canvas->drawPatch(cubics, nullptr, texCoords, SkBlendMode::kModulate, paint);
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

    loadPngToBitmap(pngResources[RESOURCE_ID].c_str(), source);

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

#ifdef INIT_WHITEBACKGROUND
    canvas->drawColor(SK_ColorWHITE);
#else
    canvas->drawColor(SK_ColorTRANSPARENT);
#endif

   DRAW_NO(5)(canvas);

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