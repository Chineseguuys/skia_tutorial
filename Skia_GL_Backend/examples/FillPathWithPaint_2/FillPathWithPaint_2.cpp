#include "../../Effect.h"
#include "include/core/SkPath.h"
#include "skia/include/core/SkPathUtils.h"

class FillPathWithPaint_2   : public Effect {
public:
    FillPathWithPaint_2() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(10);
        SkPath strokePath = SkPath::Line({20, 20}, {100, 100});
        canvas->drawPath(strokePath, paint);
        SkPath fillPath;
        skpathutils::FillPathWithPaint(strokePath, paint, &fillPath);
        paint.setStrokeWidth(6);
        canvas->translate(40, 0);
        canvas->drawPath(fillPath, paint);
    }

    void setImage(const SkImage* image) override {
        mImage = image;
    }
};

extern "C" Effect* createEffect() {
    return new FillPathWithPaint_2();
}