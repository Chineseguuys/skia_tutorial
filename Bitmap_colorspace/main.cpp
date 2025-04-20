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

#include "Skia/include/core/SkFont.h"
#include "Skia/include/core/SkTextBlob.h"
#include "Skia/include/core/SkData.h"
#include "Skia/include/core/SkPaint.h"

#include "Skia/include/codec/SkCodec.h"
#include "Skia/include/core/SkAlphaType.h"
#include "Skia/include/core/SkColorType.h"
#include "Skia/include/core/SkImageInfo.h"
#include "include/core/SkImage.h"

#include "Skia/include/core/SkFont.h"
#include "Skia/include/core/SkTextBlob.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <climits>
#include <cmath>
#include <cstdlib>
#include <spdlog/spdlog.h>
#include <iomanip>
#include <chrono>
#include <string>


static sk_sp<SkFontMgr> fontMgr;
static sk_sp<SkTypeface> typeFace;
static SkBitmap source;
static const int DRAW_WIDTH = 512;
static const int DRAW_HEIGHT = 512;
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


void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::MakeN32(16, 32, kPremul_SkAlphaType,
            SkColorSpace::MakeSRGBLinear()));
    SkColorSpace* colorSpace = bitmap.colorSpace();
    SkDebugf("gammaCloseToSRGB: %s  gammaIsLinear: %s  isSRGB: %s\n",
            colorSpace->gammaCloseToSRGB() ? "true" : "false",
            colorSpace->gammaIsLinear() ? "true" : "false",
            colorSpace->isSRGB() ? "true" : "false");
}

void draw1(SkCanvas* canvas) {
    SkBitmap bitmap;
    for (int width : { 1, 1000, 1000000 } ) {
        for (int height: { 1, 1000, 1000000 } ) {
            SkImageInfo imageInfo = SkImageInfo::MakeN32(width, height, kPremul_SkAlphaType);
            bitmap.setInfo(imageInfo, width * 5);
            SkDebugf("width: %7d height: %7d computeByteSize: %13zu\n", width, height,
                     bitmap.computeByteSize());
        }
    }
}

void draw2(SkCanvas* canvas) {
    SkBitmap original;
    if (original.tryAllocPixels(
            SkImageInfo::Make(25, 35, kRGBA_8888_SkColorType, kOpaque_SkAlphaType))) {
        spdlog::info("original has byte size: {}", original.computeByteSize());
        SkDebugf("original has pixels before copy: %s\n", original.getPixels() ? "true" : "false");
        spdlog::info("orignal getPixels has pointer address: {}", original.getPixels());
        SkBitmap copy(original);
        SkDebugf("original has pixels after copy: %s\n", original.getPixels() ? "true" : "false");
        SkDebugf("copy has pixels: %s\n", copy.getPixels() ? "true" : "false");
        spdlog::info("copy getPixels has pointer address: {}", copy.getPixels());
    }
}

void draw3(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::MakeN32(33, 55, kOpaque_SkAlphaType));
    SkISize dimensions = bitmap.dimensions();
    spdlog::info("{}: dimensions: [widthxheight] = [{}x{}]", __func__, dimensions.fWidth, dimensions.fHeight);
    SkRect bounds;
    bitmap.getBounds(&bounds);
    spdlog::info("{}: bounds = [{}x{} -> {}x{}]", __func__, bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);
    SkRect dimensionsAsBounds = SkRect::Make(dimensions);
    spdlog::info("{}: dimension as bounds = [{}x{} -> {}x{}]", __func__,
            dimensionsAsBounds.fLeft, dimensionsAsBounds.fTop, 
            dimensionsAsBounds.fRight, dimensionsAsBounds.fBottom
    );
    SkDebugf("dimensionsAsBounds %c= bounds\n", dimensionsAsBounds == bounds ? '=' : '!');
}

// draw nothing
void draw4(SkCanvas* canvas) {
    SkBitmap bitmap;
    for (int w : { 0, 8 } ) {
        for (bool allocate : { false, true} ) {
            bitmap.setInfo(SkImageInfo::MakeA8(w, 8));
            allocate ? bitmap.allocPixels() : (void) 0 ;
            SkDebugf("empty:%s isNull:%s drawsNothing:%s\n", bitmap.empty() ? "true " : "false",
                     bitmap.isNull() ? "true " : "false", bitmap.drawsNothing() ? "true" : "false");
        }
    }
}

// bitmap is empty
void draw5(SkCanvas* canvas) {
    SkBitmap bitmap;
    for (int width : { 0, 2 } ) {
        for (int height : { 0, 2 } ) {
             bitmap.setInfo(SkImageInfo::MakeA8(width, height));
             SkDebugf("width: %d height: %d empty: %s\n", width, height,
                      bitmap.empty() ? "true" : "false");
        }
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
    draw2(canvas);
    draw3(canvas);
    draw4(canvas);
    draw5(canvas);

    return 0;
}