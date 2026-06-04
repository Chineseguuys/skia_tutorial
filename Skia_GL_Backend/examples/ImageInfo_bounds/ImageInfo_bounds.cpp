#include "../../Effect.h"
#include <spdlog/spdlog.h>

class ImageInfo_bounds   : public Effect {
public:
    ImageInfo_bounds() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void setBitmap(const SkBitmap* bitmap) override {
        mSource = bitmap;
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        canvas->scale(.5f, .5f);
        const SkImageInfo& imageInfo = mSource->info();
        SkIRect bounds = imageInfo.bounds();
        for (int x : { 0, bounds.width() } ) {
            for (int y : { 0, bounds.height() } ) {
                spdlog::info("canvas draw image at x: {}, y: {}", x, y);
                canvas->drawImage(mSource->asImage(), x, y);
            }
        }
    }
};


extern "C" Effect* createEffect() {
    return new ImageInfo_bounds();
}