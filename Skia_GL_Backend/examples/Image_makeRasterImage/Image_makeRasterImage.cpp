#include "../../Effect.h"

#include "skia/include/core/SkFont.h"
#include "skia/include/core/SkColorSpace.h"
#include "skia/include/gpu/ganesh/GrDirectContext.h"
#include "skia/include/gpu/ganesh/SkImageGanesh.h"

class Image_makeRasterImage : public Effect {
public:
    Image_makeRasterImage() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        auto dContext = GrAsDirectContext(canvas->recordingContext());
        if (!dContext) {
            return;
        }

        auto drawImage = [=](sk_sp<SkImage> image, const char* label) -> void {
            if (nullptr == image) {
                return;
            }
            SkPaint paint;
            paint.setAntiAlias(true);
            // SkFont font = SkFont(fontMgr->matchFamilyStyle(nullptr, {}));
            SkFont font = SkFont(fontMgr->legacyMakeTypeface(nullptr, {}), 10);
            sk_sp<SkImage> raster(image->makeRasterImage(nullptr));
            canvas->drawImage(raster, 0, 0);
            canvas->drawString(label, 20, raster->height() >> 2, font, paint);
        };
        sk_sp<SkImage> bitmapImage(source.asImage());
        sk_sp<SkImage> textureImage(SkImages::BorrowTextureFrom(dContext,
                                                                backEndTexture,
                                                                kTopLeft_GrSurfaceOrigin,
                                                                kRGBA_8888_SkColorType,
                                                                kOpaque_SkAlphaType,
                                                                nullptr));
        drawImage(image, "image");
        canvas->translate(image->width(), 0);
        drawImage(bitmapImage, "source");
        canvas->translate(-image->width(), image->height());
        drawImage(textureImage, "backEndTexture");
    }
};

extern "C" Effect* createEffect() {
    return new Image_makeRasterImage{};
}