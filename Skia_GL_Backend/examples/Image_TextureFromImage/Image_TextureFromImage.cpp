#include "../../Effect.h"
#include "skia/include/core/SkAlphaType.h"
#include "skia/include/core/SkColorType.h"
#include "skia/include/core/SkFont.h"
#include "skia/include/core/SkImage.h"
#include "skia/include/core/SkPaint.h"
#include "skia/include/core/SkRefCnt.h"
#include "skia/include/core/SkColorSpace.h"
#include "skia/include/gpu/ganesh/GrTypes.h"
#include "skia/include/gpu/ganesh/GrRecordingContext.h"
#include "skia/include/gpu/ganesh/GrDirectContext.h"
#include "skia/include/gpu/ganesh/SkImageGanesh.h"

#include <spdlog/spdlog.h>

class Image_TextureFromImage : public Effect {
public:
    Image_TextureFromImage() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        auto dContext = GrAsDirectContext(canvas->recordingContext());;
        if (!dContext) {
            spdlog::error("Failed to get direct context");
            return;
        }

        auto drawImage = [=](sk_sp<SkImage> image, GrDirectContext* dContext, const char* lable) -> void {
            if (image == nullptr || dContext == nullptr) {
                spdlog::error("Failed to get image or direct context");
                return;
            }

            SkPaint paint;
            paint.setAntiAlias(true);

            sk_sp<SkImage> texture = SkImages::TextureFromImage(dContext, image);
            canvas->drawImage(texture, 0, 0);
        };

        sk_sp<SkImage> bitmapImage(source.asImage());
        sk_sp<SkImage> textureImage(SkImages::BorrowTextureFrom(
            dContext,
            backEndTexture,
            kTopLeft_GrSurfaceOrigin,
            kRGBA_8888_SkColorType,
            kOpaque_SkAlphaType,
            nullptr
        ));

        drawImage(image, dContext, "Bitmap Image");
        canvas->translate(image->width(), 0);
        drawImage(bitmapImage, dContext, "source");
        canvas->translate(-image->width(), image->height());
        drawImage(textureImage, dContext, "backEndTexture");
    }
};

extern "C" Effect* createEffect() {
    return new Image_TextureFromImage{};
}