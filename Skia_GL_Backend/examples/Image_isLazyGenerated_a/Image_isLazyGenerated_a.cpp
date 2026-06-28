#include "../../Effect.h"
#include "skia/include/core/SkColor.h"
#include "skia/include/core/SkImageGenerator.h"
#include "skia/include/core/SkImageInfo.h"
#include "skia/include/core/SkImage.h"
#include "skia/include/core/SkFont.h"
#include "skia/include/core/SkColorSpace.h"

#include "skia/include/gpu/ganesh/GrDirectContext.h"
#include "skia/include/gpu/ganesh/SkImageGanesh.h"

#include <spdlog/spdlog.h>

class TestImageGenerator : public SkImageGenerator {
public:
    TestImageGenerator() : SkImageGenerator(SkImageInfo::MakeN32Premul(10, 10)) {}
    ~TestImageGenerator() override = default;

protected:
    bool onGetPixels(const SkImageInfo& info, void* pixelPtr, size_t rowBytes, const Options& opts) override {
        SkPMColor* pixels = static_cast<SkPMColor*>(pixelPtr);
        for (int y = 0; y < info.height(); y++) {
            for (int x = 0; x < info.width(); x++) {
                pixels[y * info.width() + x] = 0xff223344 + y * 0x000C0811;
            }
        }
        return true;
    }
};

class Image_isLazyGenerated_a : public Effect {
public:
    Image_isLazyGenerated_a() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        auto gen = std::make_unique<TestImageGenerator>();
        sk_sp<SkImage> image(SkImages::DeferredFromGenerator(std::move(gen)));
        SkString lazy(image->isLazyGenerated() ? "is lazy" : "not lazy");
        spdlog::info("[{}:{}]:is lazy generated = {}", __FUNCTION__, __LINE__, lazy.c_str());
        canvas->scale(8, 8);
        canvas->drawImage(image, 0, 0);
        // SkFont font = SkFont(fontMgr->matchFamilyStyle(nullptr, {}), 4);
        SkFont font = SkFont(fontMgr->legacyMakeTypeface(nullptr, {}), 4);
        SkPaint paint;
        canvas->drawString(lazy, 2, 5, font, paint);
    }
};


class Image_isLazyGenerated_b : public Effect {
public:
    Image_isLazyGenerated_b() : Effect() {}

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
            SkFont font = SkFont(fontMgr->legacyMakeTypeface(nullptr, {}), 15);
            canvas->drawImage(image, 0, 0);
            canvas->drawString(label, 30, image->height() >> 2, font, paint);
            canvas->drawString(
                    image->isLazyGenerated() ? "is lazily generated" : "not lazily generated",
                    20, (image->height() * 3) >> 2, font, paint);
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
    return new Image_isLazyGenerated_b{};
}
