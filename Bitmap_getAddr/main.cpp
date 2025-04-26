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

#include "Skia/include/core/SkRect.h"
#include "fmt/format.h"

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <spdlog/spdlog.h>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <string>


static sk_sp<SkFontMgr> fontMgr;
static sk_sp<SkTypeface> typeFace;
static SkBitmap source;
static const int DRAW_WIDTH = 256;
static const int DRAW_HEIGHT = 160;
static const std::vector<std::string> pngResources = {"../resources/example_1.png",
    "../resources/example_2.png",
    "../resources/example_3.png",
    "../resources/example_4.png",
    "../resources/example_5.png",
    "../resources/example_6.png",
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
    options.fZLibLevel = 8;
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

// get addr
void draw(SkCanvas* canvas) {
    char* row0 = (char*) source.getAddr(0, 0);
    char* row1 = (char*) source.getAddr(0, 1);
    spdlog::info("addr of row0 is {}", fmt::ptr(row0));
    spdlog::info("addr of row1 is {}", fmt::ptr(row1));
    spdlog::info("row0 - row1 = {}", (size_t)(row1 - row0));
    spdlog::info("addr internal {}= rowBytes", (size_t)(row1 - row0) == source.rowBytes() ? "=" : "!");
}

// get addr 16
void draw1(SkCanvas* canvas) {
    SkBitmap bitmap16;
    SkImageInfo dstInfo = SkImageInfo::Make(source.width(), source.height(), kARGB_4444_SkColorType, kPremul_SkAlphaType);
    bitmap16.allocPixels(dstInfo);
    if (source.readPixels(dstInfo, bitmap16.getPixels(), bitmap16.rowBytes(), 0, 0)) {
        uint16_t* row0 = bitmap16.getAddr16(0, 0);
        uint16_t* row1 = bitmap16.getAddr16(0, 1);
        size_t interval = (row1 - row0) * bitmap16.bytesPerPixel();

        spdlog::info("addr interval {}= rowBytes\n", interval == bitmap16.rowBytes() ? '=' : '!');
    }
}

// get addr 32
void draw2(SkCanvas* canvas) {
    uint32_t* row = source.getAddr32(0, 0);
    uint32_t* row1 = source.getAddr32(0, 1);

    size_t interval = (row - row1) * source.bytesPerPixel();
    spdlog::info("addr interval {}= rowBytes\n", interval == source.rowBytes() ? '=' : '!');
}

// get addr 8
void draw3(SkCanvas* canvas) {
    SkBitmap bitmap;
    const int width = 8;
    const int height = 8;
    uint8_t pixels[height][width];
    SkImageInfo info = SkImageInfo::Make(width, height, kGray_8_SkColorType, kOpaque_SkAlphaType);
    if (bitmap.installPixels(info, pixels, info.minRowBytes())) {
        spdlog::info("&pixels[4][2] {}= bitmap.getAddr8(2, 4)\n",
                  &pixels[4][2]  == bitmap.getAddr8(2, 4) ? '=' : '!');
    }
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
    canvas->drawColor(0xFFFFFFFF);

    draw(canvas);
    draw1(canvas);

    //SkBitmap bitmap;
    //bitmap.allocPixels(imageInfo, imageInfo.minRowBytes());
    //surface->readPixels(bitmap, 0, 0);

    //std::string pngName = generate_filename("output", "png");
    //saveBitmapAsPng(bitmap, pngName.c_str());

    return 0;
}