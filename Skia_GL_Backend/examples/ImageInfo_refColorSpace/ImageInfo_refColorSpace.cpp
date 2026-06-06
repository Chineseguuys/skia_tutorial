#include "../../Effect.h"
#include "skia/include/core/SkColorSpace.h"

class ImageInfo_refColorSpace : public Effect {
public:
    ImageInfo_refColorSpace() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        SkImageInfo info1 = SkImageInfo::MakeN32(16, 32, kPremul_SkAlphaType, 
            SkColorSpace::MakeSRGBLinear());
        SkImageInfo info2 = SkImageInfo::MakeN32(16, 32, kPremul_SkAlphaType,
                info1.refColorSpace());
        SkColorSpace* colorSpace = info2.colorSpace();
        SkDebugf("gammaCloseToSRGB: %s  gammaIsLinear: %s  isSRGB: %s\n",
                colorSpace->gammaCloseToSRGB() ? "true" : "false",
                colorSpace->gammaIsLinear() ? "true" : "false",
                colorSpace->isSRGB() ? "true" : "false");
    }
};

extern "C" Effect* createEffect() {
    return new ImageInfo_refColorSpace();
}