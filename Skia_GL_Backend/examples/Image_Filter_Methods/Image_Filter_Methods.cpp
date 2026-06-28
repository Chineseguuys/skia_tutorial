#include "../../Effect.h"
#include "include/core/SkBlurTypes.h"
#include "skia/include/core/SkRegion.h"
#include "skia/include/core/SkMaskFilter.h"
#include "skia/include/effects/SkImageFilters.h"

class Image_Filter_Methods : public Effect {
public:
    Image_Filter_Methods() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2);
        SkRegion region;
        region.op({10, 10, 50, 50}, SkRegion::kUnion_Op);
        region.op({10, 50, 90, 90}, SkRegion::kUnion_Op);
        paint.setImageFilter(SkImageFilters::Blur(5.0f, 5.0f, nullptr));
        canvas->drawRegion(region, paint);
        paint.setImageFilter(nullptr);
        paint.setMaskFilter(SkMaskFilter::MakeBlur(kOuter_SkBlurStyle, 5));
        canvas->translate(100, 100);
        canvas->drawRegion(region, paint);
    }
};

extern "C" Effect* createEffect() {
    return new Image_Filter_Methods{};
}