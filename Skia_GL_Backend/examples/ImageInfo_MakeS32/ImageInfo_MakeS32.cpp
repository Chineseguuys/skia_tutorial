#include "../../Effect.h"
#include "skia/include/effects/SkGradientShader.h"

class ImageInfo_MakeS32   : public Effect {
public:
    ImageInfo_MakeS32() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        const int width = 256;
        const int height = 32;
        SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
        SkColor4f gradColors[] = {
                SkColor4f::FromColor(0xFFAA0055), SkColor4f::FromColor(0xFF11CC88) };
        // SkColor gradColors[] = { 0xFFAA0055, 0xFF11CC88, 0xFFFFFFFF, 0xFFFFFFFF };
        SkPoint gradPoints[] = { { 0, 0 }, { width, 0 } };
        SkPaint gradPaint;
        // 已经废弃的函数，使用新的函数替代
        // gradPaint.setShader(SkShaders::LinearGradient(gradPoints,
        //         {{gradColors, {}, SkTileMode::kClamp}, {}}));
        gradPaint.setShader(SkGradientShader::MakeLinear(
            gradPoints,
            gradColors, 
            nullptr, 
            nullptr, 
            std::size(gradColors), 
            SkTileMode::kClamp, 
            SkGradientShader::Interpolation{}, 
            nullptr));
        SkBitmap bitmap;
        bitmap.allocPixels(SkImageInfo::MakeN32(width, height, kPremul_SkAlphaType));
        SkCanvas offScreen(bitmap);
        offScreen.drawRect(SkRect::MakeWH(width, height), gradPaint);
        canvas->drawImage(bitmap.asImage(), 0, 0);
        bitmap.allocPixels(SkImageInfo::MakeS32(width, height, kPremul_SkAlphaType));
        SkCanvas sRGBOffscreen(bitmap);
        sRGBOffscreen.drawRect(SkRect::MakeWH(width, height), gradPaint);
        canvas->drawImage(bitmap.asImage(), 0, 48);
        SkBitmap noColorSpaceBitmap;
        noColorSpaceBitmap.installPixels(SkImageInfo::MakeN32(width, height, kPremul_SkAlphaType),
                                        bitmap.getAddr(0, 0),
                                        bitmap.rowBytes());
        canvas->drawImage(noColorSpaceBitmap.asImage(), 0, 96);
    }
};


extern "C" Effect* createEffect() {
    return new ImageInfo_MakeS32();
}