#include "../../Effect.h"

class ImageInfo_Make   : public Effect {
public:
    ImageInfo_Make() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        uint8_t storage[][5] = {{ 0xCA, 0xDA, 0xCA, 0xC9, 0xA3 },
                            { 0xAC, 0xA8, 0x89, 0xA7, 0x87 },
                            { 0x9B, 0xB5, 0xE5, 0x95, 0x46 },
                            { 0x90, 0x81, 0xC5, 0x71, 0x33 },
                            { 0x75, 0x55, 0x44, 0x40, 0x30 }};
        SkImageInfo imageInfo = SkImageInfo::Make(5, 5, kGray_8_SkColorType, kOpaque_SkAlphaType);
        SkPixmap pixmap(imageInfo, storage[0], sizeof(storage) / 5);
        SkBitmap bitmap;
        bitmap.installPixels(pixmap);
        canvas->scale(8, 8);
        canvas->drawImage(bitmap.asImage(), 0, 0);
    }
};


extern "C" Effect* createEffect() {
    return new ImageInfo_Make();
}