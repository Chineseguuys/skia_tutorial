#include "Skia/include/core/SkCanvas.h"
#include "Skia/include/core/SkSurface.h"
#include "Skia/include/core/SkStream.h"
#include "Skia/include/core/SkPictureRecorder.h"
#include "Skia/include/core/SkPicture.h"
#include "Skia/include/gpu/gl/GrGLInterface.h"
#include "Skia/include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "Skia/include/gpu/gl/GrGLTypes.h"
#include "Skia/include/core/SkBitmap.h"
#include "Skia/include/encode/SkPngEncoder.h"

#include "Skia/include/core/SkFontStyle.h"
#include "Skia/include/core/SkTypeface.h"
#include "Skia/include/core/SkFontMgr.h"
#include "Skia/include/ports/SkFontMgr_empty.h"
#include "Skia/include/ports/SkFontMgr_directory.h"

#include "Skia/include/core/SkFont.h"
#include "Skia/include/core/SkTextBlob.h"
#include "Skia/include/core/SkColor.h"
#include "Skia/include/core/SkData.h"
#include "Skia/include/core/SkPaint.h"
#include "Skia/include/core/SkRect.h"
#include "Skia/include/core/SkSamplingOptions.h"
#include "Skia/include/core/SkTileMode.h"

#include "Skia/include/codec/SkCodec.h"
#include "Skia/include/core/SkAlphaType.h"
#include "Skia/include/core/SkColorType.h"
#include "Skia/include/core/SkImageInfo.h"
#include "Skia/include/core/SkUnPreMultiply.h"
#include "include/core/SkImage.h"

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <spdlog/spdlog.h>
#include <iomanip>
#include <chrono>


static sk_sp<SkFontMgr> fontMgr;
static const int DRAW_WIDTH = 256;
static const int DRAW_HEIGHT = 128;
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
        kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::Raster(imageInfo);
    SkCanvas* canvas = surface->getCanvas();

    // font manager
    fontMgr = SkFontMgr_New_Custom_Directory(fontDir.c_str());
    if (fontMgr == nullptr) {
        spdlog::error("{}: can not create font manager", __func__);
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

    sk_sp<SkTypeface>  typeFace = fontMgr->legacyMakeTypeface(fontFamily, fontStyle);
    if (typeFace == nullptr) {
        spdlog::error("{}: can not create type face from font manager!", __FUNCTION__);
    }
    SkFont font(typeFace, 12);

    // load images local
    SkBitmap source;
    if (!loadPngToBitmap(pngResources[1].c_str(), source)) {
        spdlog::error("{}: find error, exit!", __func__);
        return EXIT_FAILURE;
    }
    spdlog::info("{}: get bitmap with [{}x{}]", __func__, source.width(), source.height());

    // start to recording commands
    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(DRAW_WIDTH, DRAW_HEIGHT);
    recordingCanvas->drawColor(0xFFFFFFFF);

    std::vector<int32_t> srcPixels;
    srcPixels.resize(source.width() * source.height());
    SkPixmap pixmap(SkImageInfo::MakeN32Premul(source.width(), source.height()), 
            &srcPixels.front(), source.rowBytes());
    source.readPixels(pixmap, 0, 0);

    for(int y = 0; y < source.height(); ++y) {
        for(int x = 0; x < source.width(); ++x) {
            SkPMColor pixel = srcPixels[y * source.width() + x];
            const SkColor color = SkUnPreMultiply::PMColorToColor(pixel);
            if(SkColorGetA(color) == SK_AlphaOPAQUE) {
                srcPixels[y * source.width() + x] = SK_ColorGREEN;
            }
        }
    }

    recordingCanvas->drawImage(SkImages::RasterFromPixmapCopy(pixmap), 0, 0);

    // start draw
    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
    canvas->drawPicture(picture);

    SkBitmap bitmap;
    bitmap.allocPixels(imageInfo, imageInfo.minRowBytes());
    surface->readPixels(bitmap, 0, 0);

    std::string pngName = generate_filename("output", "png");
    std::string skpName = generate_filename("output", "skp");
    saveBitmapAsPng(bitmap, pngName.c_str());
    savePictureAsSKP(picture, skpName.c_str());

#if 1
    spdlog::info(" {}: end of skia function", __FUNCTION__);
#endif

    return 0;
}