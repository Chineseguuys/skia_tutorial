#include "../../Effect.h"
#include "include/core/SkAlphaType.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include <spdlog/spdlog.h>

class Image_AdoptTextureFrom : public Effect {
public:
    Image_AdoptTextureFrom() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        GrRecordingContext* dContext = canvas->recordingContext();
        if (!dContext) {
            spdlog::error("Failed to get GrDirectContext");
            return;
        }
        canvas->scale(.5f, .5f);
        canvas->clear(0x7F3F5F7F);
        int x = 0, y = 0;
        for (auto origin : {kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin}) {
            for (auto alpha : {kOpaque_SkAlphaType, kPremul_SkAlphaType, kUnpremul_SkAlphaType}) {
                sk_sp<SkImage> image = SkImages::AdoptTextureFrom(
                    dContext, 
                    backEndTexture, 
                    origin, 
                    kRGBA_8888_SkColorType, 
                    alpha
                );
                canvas->drawImage(image, x, y);
                x += 160;
            }
            x -= 160 * 3;
            y += 256;
        }
    }
};


extern "C" Effect* createEffect() {
    return new Image_AdoptTextureFrom();
}