#include "../../Effect.h"
#include "skia/include/core/SkAlphaType.h"
#include "skia/include/gpu/ganesh/GrTypes.h"
#include "skia/include/gpu/ganesh/SkImageGanesh.h"
// must include this header for BorrowTextureFrom function
#include "skia/include/core/SkColorSpace.h"

class Image_BorrowTextureFrom_2 : public Effect {
public:
    Image_BorrowTextureFrom_2() : Effect() {}

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
        auto releaseCallback = [](SkImages::ReleaseContext releaseContext) -> void {
            *((int*)releaseContext) += 128;
        };
        int x = 0, y = 0;
        for (auto origin : { kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin } ) {
            sk_sp<SkImage> image = SkImages::BorrowTextureFrom(dContext,
                                                               backEndTexture,
                                                               origin,
                                                               kRGBA_8888_SkColorType,
                                                               kOpaque_SkAlphaType,
                                                               nullptr,
                                                               releaseCallback,
                                                               &x);
            canvas->drawImage(image, x, y);
            y += 128;
        }
    }
};

extern "C" Effect* createEffect() {
    return new Image_BorrowTextureFrom_2();
}