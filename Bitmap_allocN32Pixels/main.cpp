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
#include "Skia/include/core/SkColor.h"
#include "Skia/include/core/SkData.h"
#include "Skia/include/core/SkPaint.h"
#include "Skia/include/core/SkSamplingOptions.h"
#include "Skia/include/core/SkTileMode.h"

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
#include <cstdint>
#include <cstdlib>
#include <random>
#include <spdlog/spdlog.h>
#include <iomanip>
#include <chrono>
#include <string>


static sk_sp<SkFontMgr> fontMgr;
static sk_sp<SkTypeface> typeFace;
static const int DRAW_WIDTH = 1024;
static const int DRAW_HEIGHT = 1024;

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

sk_sp<SkShader> makeBwDither() {
    auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(2, 2));
    SkPaint paint;
    paint.setColor(0xFF00FF00);
    surf->getCanvas()->drawColor(SK_ColorWHITE);
    surf->getCanvas()->drawRect({0, 0, 1, 1}, paint);
    surf->getCanvas()->drawRect({1, 1, 2, 2}, paint);

    return surf->makeImageSnapshot()->makeShader(
        SkTileMode::kRepeat, 
        SkTileMode::kRepeat, 
        SkSamplingOptions(SkFilterMode::kLinear)
    );
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
    bitmap.allocN32Pixels(DRAW_WIDTH >> 2, DRAW_HEIGHT >> 2);
    bitmap.eraseColor(SK_ColorTRANSPARENT);
    std::default_random_engine rng;

    const auto randUnit = [&rng](uint32_t min, uint32_t max) -> uint32_t {
        return std::uniform_int_distribution<uint32_t>(min, max)(rng);
    };

    for (int y = 0; y < DRAW_WIDTH; y+=(DRAW_WIDTH >> 2)) {
        for (int x = 0; x < DRAW_HEIGHT; x+=(DRAW_HEIGHT >> 2)) {
            SkColor color = randUnit(0, UINT_MAX);
            uint32_t w = randUnit(4, DRAW_WIDTH >> 3);
            uint32_t cx = randUnit(0, (DRAW_HEIGHT >> 2) - w);
            uint32_t h = randUnit(4, (DRAW_HEIGHT >> 3));
            uint32_t cy = randUnit(0, (DRAW_HEIGHT >> 2) - h);

            bitmap.erase(color, SkIRect::MakeXYWH(cx, cy, w, h));
            canvas->drawImage(bitmap.asImage(), x, y);
        }
    }
}

int main(int argc, char* argv[]) {
#if 1
    spdlog::set_level(spdlog::level::debug);
#endif

    fontMgr = SkFontMgr_New_Custom_Directory("../../fonts/");
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
    canvas->drawColor(0xFFFFFFFF);

    draw(canvas);

    SkBitmap bitmap;
    bitmap.allocPixels(imageInfo, imageInfo.minRowBytes());
    surface->readPixels(bitmap, 0, 0);

    std::string pngName = generate_filename("output", "png");
    saveBitmapAsPng(bitmap, pngName.c_str());

    return 0;
}