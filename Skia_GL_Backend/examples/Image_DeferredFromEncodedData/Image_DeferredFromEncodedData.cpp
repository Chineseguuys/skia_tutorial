#include "../../Effect.h"
#include "skia/include/core/SkData.h"
#include "skia/include/encode/SkJpegEncoder.h"

class Image_DeferredFromEncodedData : public Effect {
public:
    Image_DeferredFromEncodedData() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        int x = 0;
        for (int quality : {100, 50, 10, 1}) {
            SkJpegEncoder::Options options;
            options.fQuality = quality;
            sk_sp<SkData> data(SkJpegEncoder::Encode(nullptr, image.get(), options));
            sk_sp<SkImage> image = SkImages::DeferredFromEncodedData(data);
            canvas->drawImage(image, x, 0);
            x += 64;
        }
    }
};


extern "C" Effect* createEffect() {
    return new Image_DeferredFromEncodedData{};
}