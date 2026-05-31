#include "../../Effect.h"
#include "include/core/SkColor.h"
#include "skia/include/core/SkBitmap.h"
#include "skia/include/core/SkImage.h"
#include "skia/include/core/SkShader.h"
#include "skia/include/effects/SkGradientShader.h"

class Dither_bEffect : public Effect {
public:
    Dither_bEffect() : Effect() {}
    ~Dither_bEffect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        canvas->clear(0);
        SkBitmap bm32;
        bm32.allocPixels(SkImageInfo::Make(20, 10, kN32_SkColorType, kPremul_SkAlphaType));
        SkCanvas c32(bm32);
        SkPoint points[] = {{0, 0}, {20, 0}};
        // SkColor colors[] = {SkColor4f::FromColor(0xFF334455), SkColor4f::FromColor(0xFF662211) };
        SkColor colors[] = {SkColorSetARGB(0xff, 0x33, 0x44, 0x55), SkColorSetARGB(0xff, 0x66, 0x22, 0x11)};
        SkPaint paint;
        paint.setShader(SkGradientShader::MakeLinear(points, colors, nullptr, std::size(colors), SkTileMode::kClamp));
        paint.setDither(true);
        c32.drawPaint(paint);
        canvas->scale(12, 12);
        auto img = bm32.asImage();
        canvas->drawImage(img, 0, 0);
        paint.setBlendMode(SkBlendMode::kPlus);
        SkSamplingOptions sampling;
        canvas->drawImage(img, 0, 11, sampling, &paint);
        canvas->drawImage(img, 0, 11, sampling, &paint);
        canvas->drawImage(img, 0, 11, sampling, &paint);
    }
};

extern "C" Effect* createEffect() {
    return new Dither_bEffect();
}