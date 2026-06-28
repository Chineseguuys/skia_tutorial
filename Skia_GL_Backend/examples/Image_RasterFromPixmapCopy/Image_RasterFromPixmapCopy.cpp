#include "../../Effect.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"

class Image_RasterFromPixmapCopy : public Effect {
public:
    Image_RasterFromPixmapCopy() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        uint8_t storage[][5] = {{ 0xCA, 0xDA, 0xCA, 0xC9, 0xA3 },
                            { 0xAC, 0xA8, 0x89, 0xA7, 0x87 },
                            { 0x9B, 0xB5, 0xE5, 0x95, 0x46 },
                            { 0x90, 0x81, 0xC5, 0x71, 0x33 },
                            { 0x75, 0x55, 0x44, 0x40, 0x30 }};
        SkImageInfo imageInfo = SkImageInfo::Make(5, 5, kGray_8_SkColorType, kOpaque_SkAlphaType);
        SkPixmap pixmap(imageInfo, storage[0], sizeof(storage) / 5);
        SkBitmap bitmap;
        bitmap.installPixels(pixmap);
        // will copy raw pixels from origin, if origin changed, image will not be changed
        sk_sp<SkImage> _image = SkImages::RasterFromPixmapCopy(pixmap);
        *pixmap.writable_addr8(2, 2) = 0x00;

        canvas->scale(10, 10);
        canvas->drawImage(bitmap.asImage(), 0, 0);
        canvas->drawImage(_image, 10, 0);
    }
};

extern "C" Effect* createEffect() {
    return new Image_RasterFromPixmapCopy{};
}