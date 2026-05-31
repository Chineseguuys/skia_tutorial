#include "../../Effect.h"
#include "skia/include/core/SkBitmap.h"
#include "skia/include/core/SkImage.h"

class Dither_aEffect : public Effect {
public:
    Dither_aEffect() : Effect() {}
    ~Dither_aEffect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        SkBitmap bm16;
        bm16.allocPixels(SkImageInfo::Make(32, 32, kRGB_565_SkColorType, kOpaque_SkAlphaType));
        SkCanvas c16(bm16);
        SkPaint colorPaint;
        for (auto dither : { false, true } ) {
            colorPaint.setDither(dither);
            for (auto colors : { 0xFF333333, 0xFF666666, 0xFF999999, 0xFFCCCCCC } ) {
                for (auto mask : { 0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFFFF } ) {
                    colorPaint.setColor(colors & mask);
                    c16.drawRect({0, 0, 8, 4}, colorPaint);
                    c16.translate(8, 0);
                }
                c16.translate(-32, 4);
            }
        }
        canvas->scale(8, 8);
        canvas->drawImage(bm16.asImage(), 0, 0);
    }
};

extern "C" Effect* createEffect() {
    return new Dither_aEffect();
}