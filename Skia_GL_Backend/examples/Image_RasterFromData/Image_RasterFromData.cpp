#include "../../Effect.h"
#include "skia/include/core/SkData.h"

class Image_RasterFromData : public Effect {
public:
    Image_RasterFromData() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        size_t rowBytes = image->width() * SkColorTypeBytesPerPixel(kRGBA_8888_SkColorType);
        sk_sp<SkData> data = SkData::MakeUninitialized(rowBytes * image->height());
        SkImageInfo dstInfo = SkImageInfo::MakeN32(image->width(), image->height(),
                                                kPremul_SkAlphaType);
        image->readPixels(nullptr, dstInfo,
            data->writable_data(),
            rowBytes,
            0, 0,
            SkImage::kAllow_CachingHint
        );
        auto ii = dstInfo.makeColorType(kRGBA_8888_SkColorType);
        sk_sp<SkImage> raw = SkImages::RasterFromData(ii, data, rowBytes);
        canvas->drawImage(image, 0, 0);
        canvas->drawImage(raw.get(), 128, 0);
    }
};

extern "C" Effect* createEffect() {
    return new Image_RasterFromData{};
}