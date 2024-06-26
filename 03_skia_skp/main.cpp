#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkStream.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPicture.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/core/SkBitmap.h"
#include "include/encode/SkPngEncoder.h"

#include "include/core/SkFontStyle.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkFontMgr.h"
#include "include/ports/SkFontMgr_empty.h"
#include "include/ports/SkFontMgr_directory.h"

#include "include/core/SkFont.h"
#include "include/core/SkTextBlob.h"

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <spdlog/spdlog.h>


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

    saveBitmapAsPng(bitmap, "output.png");
    savePictureAsSKP(picture, "output.skp");

#if 1
    spdlog::info(" {}: end of skia function", __FUNCTION__);
#endif

    return 0;
}