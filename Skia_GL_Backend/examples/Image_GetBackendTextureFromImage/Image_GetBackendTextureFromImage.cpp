#include "../../Effect.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include <spdlog/spdlog.h>

class Image_GetBackendTextureFromImage : public Effect {
public:
    Image_GetBackendTextureFromImage() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        GrRecordingContext* context = canvas->recordingContext();
        if (!context) {
            spdlog::error("No recording context found");
            return;
        }
        GrDirectContext* direct = context->asDirectContext();
        if (!direct) {
            spdlog::error("No direct context found");
            return;
        }

        sk_sp<SkImage> imageFromBackend = SkImages::AdoptTextureFrom(
            direct,
            backEndTexture,
            kBottomLeft_GrSurfaceOrigin,
            kRGBA_8888_SkColorType,
            kOpaque_SkAlphaType
        );
        GrGLTextureInfo info;

        if (GrBackendTextures::GetGLTextureInfo(backEndTexture, &info)) {
            spdlog::info("For backEndTexture GL Texture ID: {}", info.fID);
        }

        GrBackendTexture textureFromImage;
        if (!SkImages::GetBackendTextureFromImage(imageFromBackend, &textureFromImage, false)) {
            spdlog::error("Failed to get backend texture from image");  
            return;
        }

        sk_sp<SkImage> imageFromTexture = SkImages::AdoptTextureFrom(
            direct,
            textureFromImage,
            kTopLeft_GrSurfaceOrigin,
            kRGBA_8888_SkColorType,
            kOpaque_SkAlphaType
        );

        if (GrBackendTextures::GetGLTextureInfo(textureFromImage, &info)) {
            spdlog::info("For textureFromImage GL Texture ID: {}", info.fID);
        }

        canvas->drawImage(imageFromTexture, 0, 0);
        canvas->drawImage(imageFromBackend, 512, 0);
    }
};

extern "C" Effect* createEffect() {
    return new Image_GetBackendTextureFromImage{};
}