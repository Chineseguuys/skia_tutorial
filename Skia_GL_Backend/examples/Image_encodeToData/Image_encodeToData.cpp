#include "../../Effect.h"
#include "skia/include/core/SkData.h"
#include "skia/include/encode/SkJpegEncoder.h"
#include "skia/include/encode/SkPngEncoder.h"

#include <spdlog/spdlog.h>

class Image_encodeToData : public Effect {
public:
    Image_encodeToData() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        canvas->scale(4, 4);
        SkIRect subset = {0, 0, 16, 64};
        int x = 0;
        for (int quality : { 0, 10, 50, 100 } ) {
            SkJpegEncoder::Options options;
            options.fQuality = quality;
            sk_sp<SkData> data(SkJpegEncoder::Encode(nullptr, image.get(), options));
            sk_sp<SkImage> filtered =
                    SkImages::DeferredFromEncodedData(data)->makeSubset(nullptr, subset, {});
            canvas->drawImage(filtered, x, 0);
            x += 16;
        }
    }
};

class Image_encodeToData_2 : public Effect {
public:
    Image_encodeToData_2() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        canvas->scale(4, 4);
        SkIRect subset = {136, 32, 200, 96};
        // This prevents re-encoding the image's pixels if the image itself was created from
        // something like an encoded PNG.
        auto data = image->refEncodedData();
        if (!data) {
            spdlog::error("Failed to get encoded data");
            data = SkPngEncoder::Encode(nullptr, image.get(), {});
        }
        sk_sp<SkImage> eye = SkImages::DeferredFromEncodedData(data)->makeSubset(nullptr, subset, {});
        canvas->drawImage(eye, 0, 0);
    }
};

extern "C" Effect* createEffect() {
    return new Image_encodeToData_2{};
}