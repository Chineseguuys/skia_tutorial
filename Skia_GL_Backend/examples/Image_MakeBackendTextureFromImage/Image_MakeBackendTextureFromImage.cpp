#include "../../Effect.h"
#include "skia/include/gpu/ganesh/SkSurfaceGanesh.h"
#include "skia/include/gpu/ganesh/GrDirectContext.h"
#include "skia/include/gpu/ganesh/GrBackendSurface.h"
#include "skia/include/core/SkImage.h"
#include "skia/include/core/SkColorSpace.h"
#include "skia/include/gpu/ganesh/SkImageGanesh.h"

class Image_MakeBackendTextureFromImage : public Effect {
public:
    Image_MakeBackendTextureFromImage() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        auto dContext = GrAsDirectContext(canvas->recordingContext());
        if (!dContext) {
            return;
        }
        sk_sp<SkImage> backEndImage = create_gpu_image(dContext);
        canvas->drawImage(backEndImage, 0, 0);
        GrBackendTexture texture;
        SkImages::BackendTextureReleaseProc proc;
        if (!SkImages::MakeBackendTextureFromImage(
                    dContext, std::move(backEndImage), &texture, &proc)) {
            return;
        }
        sk_sp<SkImage> i2 = SkImages::BorrowTextureFrom(dContext,
                                                        texture,
                                                        kTopLeft_GrSurfaceOrigin,
                                                        kN32_SkColorType,
                                                        kOpaque_SkAlphaType,
                                                        nullptr);
        canvas->drawImage(i2, 30, 30);
    }

private:
    static sk_sp<SkImage> create_gpu_image(GrRecordingContext * rContext) {
        const SkImageInfo info = SkImageInfo::MakeN32(20, 20, kOpaque_SkAlphaType);
        auto surface(SkSurfaces::RenderTarget(rContext, skgpu::Budgeted::kNo, info));
        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(SK_ColorWHITE);
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);
        canvas->drawRect(SkRect::MakeXYWH(5, 5, 10, 10), paint);
        return surface->makeImageSnapshot();
    }
};

extern "C" Effect* createEffect() {
    return new Image_MakeBackendTextureFromImage{};
}