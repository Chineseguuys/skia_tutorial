#include "../../Effect.h"

class ImageInfo_computeOffset : public Effect {
public:
    ImageInfo_computeOffset() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        uint8_t pixels[][12] = { { 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00},
                             { 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00},
                             { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00},
                             { 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF},
                             { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                             { 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00},
                             { 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00},
                             { 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00} };
        SkImageInfo imageInfo = SkImageInfo::MakeA8(8, 8);
        SkBitmap bitmap;
        bitmap.installPixels(imageInfo, (void*) pixels, sizeof(pixels[0]));
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        canvas->drawImageRect(bitmap.asImage(), SkRect::MakeWH(8, 8), SkRect::MakeWH(32, 32),
                            SkSamplingOptions(), &paint, SkCanvas::kStrict_SrcRectConstraint);
        size_t offset = imageInfo.computeOffset(2, 3, sizeof(pixels[0]));
        pixels[0][offset] = 0x7F;
        offset = imageInfo.computeOffset(5, 3, sizeof(pixels[0]));
        pixels[0][offset] = 0x7F;
        bitmap.installPixels(imageInfo, (void*) pixels, sizeof(pixels[0]));
        canvas->drawImageRect(bitmap.asImage(), SkRect::MakeWH(8, 8), SkRect::MakeWH(128, 128),
                            SkSamplingOptions(), &paint, SkCanvas::kStrict_SrcRectConstraint);
    }
};

extern "C" Effect* createEffect() {
    return new ImageInfo_computeOffset();
}