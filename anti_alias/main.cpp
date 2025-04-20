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

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <cmath>
#include <cstdlib>
#include <spdlog/spdlog.h>
#include <iomanip>
#include <chrono>


static sk_sp<SkFontMgr> fontMgr;
static const int DRAW_WIDTH = 512;
static const int DRAW_HEIGHT = 256;

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
    oss << prefix << "-"
        << (time_info.tm_year + 1900) << "_"
        << std::setfill('0') << std::setw(2) << (time_info.tm_mon + 1) << "_"
        << std::setw(2) << time_info.tm_mday << "_"
        << std::setw(2) << time_info.tm_hour << "_"
        << std::setw(2) << time_info.tm_min << "_"
        << std::setw(2) << time_info.tm_sec << postfix;
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


int main(int argc, char* argv[]) {
#if 1
    spdlog::set_level(spdlog::level::debug);
#endif

    SkImageInfo imageInfo = SkImageInfo::Make(
        DRAW_WIDTH, DRAW_HEIGHT,
        kBGRA_8888_SkColorType,
        kOpaque_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::Raster(imageInfo);
    SkCanvas* canvas = surface->getCanvas();
    //canvas->drawColor(0xFFFFFFFF);

    SkBitmap bitmap;
    bitmap.allocN32Pixels(50, 50);
    SkCanvas offscreen(bitmap);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(10);

    for(bool antialias : {false, true}) {
        paint.setColor(antialias ? SK_ColorRED : SK_ColorBLUE);
        paint.setAntiAlias(antialias);
        bitmap.eraseColor(0);
        offscreen.drawLine(5, 5, 15, 30, paint);
        canvas->drawLine(5, 5, 15, 30, paint);
        canvas->save();
        canvas->scale(10, 10);
        canvas->drawImage(bitmap.asImage(), antialias ? 12 : 0, 0);
        canvas->restore();
        canvas->translate(15,0);
    }

    SkBitmap screen;
    screen.allocPixels(imageInfo, imageInfo.minRowBytes());
    surface->readPixels(screen, 0, 0);

    std::string pngName = generate_filename("output", "png");
    saveBitmapAsPng(screen, pngName.c_str());

    return 0;
}