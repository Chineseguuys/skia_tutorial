#include "../../Effect.h"
#include "skia/include/core/SkFont.h"
#include "skia/include/core/SkImage.h"
#include "skia/include/core/SkColorSpace.h"
#include "skia/include/gpu/ganesh/GrDirectContext.h"
#include "skia/include/gpu/ganesh/SkImageGanesh.h"

class Image_isValid : public Effect {
public:
    Image_isValid() : Effect() {}

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
            SkFont font = SkFont(fontMgr->legacyMakeTypeface(nullptr, {}), 10);
            SkPaint paint;
            paint.setAntiAlias(true);
            canvas->drawImage(image, 0, 0);
            canvas->drawString(label, image->width() >> 1, image->height() >> 2, font, paint);
            if (dContext) {
                const char* msg =
                        image->isValid(dContext->asRecorder()) ? "is valid on GPU" : "not valid on GPU";
                canvas->drawString(msg, 20, image->height() * 5 >> 3, font, paint);
            }

            const char* msg = image->isTextureBacked() ? "is not valid on CPU" : "valid on CPU";

            canvas->drawString(msg, 20, image->height() * 7 >> 3, font, paint);
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
    return new Image_isValid{};
}