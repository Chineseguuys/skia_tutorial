#include "../../Effect.h"
#include "include/core/SkImageInfo.h"

class ImageInfo_MakeA8   : public Effect {
public:
    ImageInfo_MakeA8() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        uint8_t pixels[][8] = { { 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00},
                            { 0x00, 0x7f, 0xff, 0x3f, 0x3f, 0x7f, 0x3f, 0x00},
                            { 0x3f, 0xff, 0x7f, 0x00, 0x7f, 0xff, 0x7f, 0x00},
                            { 0x00, 0x3f, 0x00, 0x00, 0x3f, 0x7f, 0x3f, 0x00},
                            { 0x3f, 0x7f, 0x7f, 0x3f, 0x00, 0x00, 0x00, 0x00},
                            { 0x7f, 0xff, 0xff, 0x7f, 0x00, 0x3f, 0x7f, 0x3f},
                            { 0x7f, 0xff, 0xff, 0x7f, 0x00, 0x7f, 0xff, 0x7f},
                            { 0x3f, 0x7f, 0x7f, 0x3f, 0x00, 0x3f, 0x7f, 0x3f} };
        SkBitmap bitmap;
        // bitmap.installPixels(SkImageInfo::Make(8, 8, kAlpha_8_SkColorType, kPremul_SkAlphaType),
        //         (void*) pixels, sizeof(pixels[0]));
        // 等价
        bitmap.installPixels(SkImageInfo::MakeA8(8,8), (void*) pixels, sizeof(pixels[0]));
        SkPaint paint;
        canvas->scale(4, 4);
        for (auto color : { SK_ColorRED, SK_ColorBLUE, 0xFF007F00} ) {
            paint.setColor(color);
            canvas->drawImage(bitmap.asImage(), 0, 0, SkSamplingOptions(), &paint);
            canvas->translate(12, 0);
        }
    }
};


extern "C" Effect* createEffect() {
    return new ImageInfo_MakeA8();
}