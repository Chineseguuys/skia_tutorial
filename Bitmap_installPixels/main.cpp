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

#include "fmt/format.h"
#include "Skia/include/core/SkColor.h"
#include "Skia/include/private/base/SkDebug.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkShader.h"

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <spdlog/spdlog.h>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <string>


static sk_sp<SkFontMgr> fontMgr;
static sk_sp<SkTypeface> typeFace;
static SkBitmap source;
// for draw1
#if 0
static const int DRAW_WIDTH = 3008;
static const int DRAW_HEIGHT = 2120;
#endif
//for draw2
#if 0
static const int DRAW_WIDTH = 256;
static const int DRAW_HEIGHT = 256;
//for draw3 
#endif
static const int DRAW_WIDTH = 256;
static const int DRAW_HEIGHT = 64;
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

void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    void* pixels = new uint32_t[8 * 8];
    SkImageInfo info = SkImageInfo::MakeN32(8, 8, kOpaque_SkAlphaType);
    SkDebugf("before installPixels\n");
    bool installed = bitmap.installPixels(info, pixels, 8 * 4, releaseProc, nullptr);
    SkDebugf("install " "%s" "successful\n", installed ? "" : "not ");
    saveBitmapAsPng(bitmap, generate_filename("output_8x8", "png").c_str());
}

void draw1(SkCanvas* canvas) {
    uint32_t* launcher = nullptr;
    uint32_t* background = nullptr;
    bool isLoaded = loadRGBARawFile(rgbaRawResources[0].c_str(), 3008, 2120, &launcher);
    if (!isLoaded)  return;

    isLoaded = loadRGBARawFile(rgbaRawResources[1].c_str(), 3008, 2120, &background);
    if (!isLoaded) return;

    {
        // draw wallpaper layer
        SkBitmap bitmap;
        SkImageInfo info = SkImageInfo::MakeN32(3008, 2120, kUnpremul_SkAlphaType);
        bool installed = bitmap.installPixels(info, background, 3008 * 4, releaseProc, nullptr);
        spdlog::info("install " "{}" "successful\n", installed ? "" : "not ");

        sk_sp<SkImage> image = bitmap.asImage();
        SkMatrix matrix;
        sk_sp<SkShader> shader = image->makeShader(SkSamplingOptions(), &matrix);
        SkPaint paint;
        paint.setShader(shader);
        SkRect bounds = SkRect::MakeLTRB(0, 0, 3008, 2120);
        canvas->drawRect(bounds, paint);
    }

    {
        // draw launcher layer
        SkBitmap bitmap;
        SkImageInfo info = SkImageInfo::MakeN32(3008, 2120, kUnpremul_SkAlphaType);
        bool installed = bitmap.installPixels(info, launcher, 3008 * 4, releaseProc, nullptr);
        SkDebugf("install " "%s" "successful\n", installed ? "" : "not ");

        sk_sp<SkImage> image = bitmap.asImage();
        SkMatrix matrix;
        sk_sp<SkShader> shader = image->makeShader(SkSamplingOptions(), &matrix);
        SkPaint paint;
        paint.setShader(shader);
        SkRect bounds = SkRect::MakeLTRB(0, 0, 3008, 2120);
        canvas->drawRect(bounds, paint);
    }
}

void draw2(SkCanvas* canvas) {
    std::default_random_engine rng;
    const auto randUint = [&rng](uint32_t min, uint32_t max) -> uint32_t {
        return std::uniform_int_distribution<uint32_t>(min, max)(rng);
    };
    SkBitmap bitmap;
    const int width = 8;
    const int height = 8;
    uint32_t pixels[width * height];
    for (unsigned x = 0; x < width * height; ++x) {
       pixels[x] = randUint(0, UINT_MAX);
    }
    SkImageInfo info = SkImageInfo::MakeN32(width, height, kUnpremul_SkAlphaType);
    if (bitmap.installPixels(info, pixels, info.minRowBytes())) {
       canvas->scale(32, 32);
       canvas->drawImage(bitmap.asImage(), 0, 0);
    }
}

void draw3(SkCanvas* canvas) {
    uint8_t storage[][5] = {{ 0xCA, 0xDA, 0xCA, 0xC9, 0xA3 },
                            { 0xAC, 0xA8, 0x89, 0x47, 0x87 },
                            { 0x4B, 0x25, 0x25, 0x25, 0x46 },
                            { 0x90, 0x81, 0x25, 0x41, 0x33 },
                            { 0x75, 0x55, 0x44, 0x20, 0x00 }};
    SkImageInfo imageInfo = SkImageInfo::Make(5, 5, kGray_8_SkColorType, kOpaque_SkAlphaType);
    SkPixmap pixmap(imageInfo, storage[0], sizeof(storage) / 5);
    SkBitmap bitmap;
    bitmap.installPixels(pixmap);
    canvas->scale(10, 10);
    canvas->drawImage(bitmap.asImage(), 0, 0);
    *pixmap.writable_addr8(2, 2) = 0xFF;    // 白色
    bitmap.installPixels(pixmap);
    canvas->drawImage(bitmap.asImage(), 10, 0);
}

int main(int argc, char* argv[]) {
#if 1
    spdlog::set_level(spdlog::level::debug);
#endif

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

    loadPngToBitmap(pngResources[2].c_str(), source);

    SkImageInfo imageInfo = SkImageInfo::Make(
        DRAW_WIDTH, DRAW_HEIGHT,
        kBGRA_8888_SkColorType,
        kOpaque_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::Raster(imageInfo);
    SkCanvas* canvas = surface->getCanvas();
    canvas->drawColor(SK_ColorTRANSPARENT);

    draw3(canvas);

    SkBitmap bitmap;
    bitmap.allocPixels(imageInfo, imageInfo.minRowBytes());
    surface->readPixels(bitmap, 0, 0);

    std::string pngName = generate_filename("output", "png");
    saveBitmapAsPng(bitmap, pngName.c_str());

    return 0;
}