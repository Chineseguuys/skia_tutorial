#include "../../Effect.h"
#include "skia/include/core/SkAlphaType.h"
#include "skia/include/gpu/ganesh/GrTypes.h"
#include "skia/include/gpu/ganesh/SkImageGanesh.h"
// must include this header for BorrowTextureFrom function
#include "skia/include/core/SkColorSpace.h"

class Image_BorrowTextureFrom : public Effect {
public:
    Image_BorrowTextureFrom() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        GrRecordingContext* dContext = canvas->recordingContext();
        if (!dContext) {
            spdlog::error("[{}:{}] Failed to get GrDirectContext", __FUNCTION__, __LINE__);
            return;
        }
        canvas->scale(.25f, .25f);
        int x = 0;
        for (auto origin : {kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin}) {
            sk_sp<SkImage> image = SkImages::BorrowTextureFrom(
                dContext, 
                backEndTexture, 
                origin, 
                kRGBA_8888_SkColorType, 
                kOpaque_SkAlphaType, 
                nullptr
            );
            canvas->drawImage(image, x, 0);
            x += 512;
        }
    }
};

extern "C" Effect* createEffect() {
    return new Image_BorrowTextureFrom();
}