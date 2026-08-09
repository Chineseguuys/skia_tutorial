#include "../../Effect.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkPoint_impl.h"

class Image_makeWithFilter : public Effect {
public:
    Image_makeWithFilter() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        sk_sp<SkImageFilter> shadowFilter = SkImageFilters::DropShadow(
            -10.0f * frame, 5.0f * frame, 3.0f, 3.0f, SK_ColorBLUE, nullptr
        );
        sk_sp<SkImageFilter> offsetFilter = SkImageFilters::Offset(40, 40, shadowFilter, nullptr);
        SkIRect subset = image->bounds();
        SkIRect clipBounds = image->bounds();
        clipBounds.outset(60, 60);
        SkIRect outSubset;
        SkIPoint offset;
        sk_sp<SkImage> filtered;
        if (auto rContext = canvas->recordingContext()) {
            // render in gpu
            filtered = SkImages::MakeWithFilter(
                rContext, image, offsetFilter.get(),
                subset, clipBounds,
                &outSubset, &offset
            );
        } else {
            // render in cpu
            filtered = SkImages::MakeWithFilter(
                image, offsetFilter.get(),
                subset, clipBounds, &outSubset, &offset
            );
        }

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawLine(0, 0, offset.fX, offset.fY, paint);
        canvas->translate(offset.fX, offset.fY);
        canvas->drawImage(filtered, 0, 0);
        canvas->drawRect(SkRect::Make(outSubset), paint);
    }
};



extern "C" {
    Effect* createEffect() {
        return new Image_makeWithFilter();
    }
}