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

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <spdlog/spdlog.h>
#include <chrono>
#include <iomanip>

static sk_sp<SkFontMgr> fontMgr;

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

int main(int argc, char* argv[]) {
#if 1
    spdlog::set_level(spdlog::level::debug);
#endif
    //sk_sp<GrDirectContext> context = GrDirectContexts::MakeGL();
    SkImageInfo imageInfo = SkImageInfo::Make(
        800, 600,
        kBGRA_8888_SkColorType,
        kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::Raster(imageInfo);
    SkCanvas* canvas = surface->getCanvas();

    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(800, 600);
    recordingCanvas->clear(SkColorSetARGB(0xff, 0x82, 0x82, 0x81));

    SkPaint paint;
    paint.setColor(SK_ColorYELLOW);
    recordingCanvas->drawCircle(400, 300, 100, paint);

    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kStrokeAndFill_Style);
    paint.setStrokeWidth(4);
    recordingCanvas->drawRect(SkRect::MakeXYWH(200, 100, 50, 50), paint);
    
    {
        // 将 ttf 文件放置在文件夹中
        fontMgr = SkFontMgr_New_Custom_Directory("../fonts/");
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
        
        sk_sp<SkTypeface>  typeFace = fontMgr->legacyMakeTypeface(fontFamily, fontStyle);
        if (typeFace == nullptr) {
            spdlog::error("{}: can not create type face from font manager!", __FUNCTION__);
        }
        SkFont font1(typeFace, 64.0f, 1.0f, 0.0f);
        SkFont font2(typeFace, 64.0f, 1.5f, 0.0f);
        font1.setEdging(SkFont::Edging::kAntiAlias);
        font2.setEdging(SkFont::Edging::kAntiAlias);
        
        sk_sp<SkTextBlob> blob1 = SkTextBlob::MakeFromString("Skia", font1);
        sk_sp<SkTextBlob> blob2 = SkTextBlob::MakeFromString("SKia", font2);
        
        SkPaint paint1, paint2, paint3;
        paint1.setAntiAlias(true);
        paint1.setColor(SkColorSetARGB(0xff, 0x42, 0x85, 0xf4));
        
        paint2.setAntiAlias(true);
        paint2.setColor(SkColorSetARGB(0xFF, 0xDB, 0x44, 0x37));
        paint2.setStyle(SkPaint::kStroke_Style);
        paint2.setStrokeWidth(3.0f);
        
        paint3.setAntiAlias(true);
        paint3.setColor(SkColorSetARGB(0xFF, 0x0F, 0x9D, 0x58));
        
        recordingCanvas->drawTextBlob(blob1.get(), 20.0f, 64.0f, paint1);
        recordingCanvas->drawTextBlob(blob1.get(), 20.0f, 144.0f, paint2);
        recordingCanvas->drawTextBlob(blob2.get(), 20.0f, 224.0f, paint3);
    }

    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
    canvas->drawPicture(picture);

    SkBitmap bitmap;
    bitmap.allocPixels(imageInfo, imageInfo.minRowBytes());
    surface->readPixels(bitmap, 0, 0);

    std::string pngFileName = generate_filename("output", "png");
    std::string skpFileName = generate_filename("output", "skp");
    saveBitmapAsPng(bitmap, pngFileName.c_str());
    savePictureAsSKP(picture, skpFileName.c_str());

#if 1
    spdlog::info(" {}: end of skia function", __FUNCTION__);
#endif

    return 0;
}