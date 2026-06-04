#include "../../Effect.h"
#include "skia/include/effects/SkGradientShader.h"
#include "skia/include/core/SkPathBuilder.h"
#include "skia/include/core/SkPathUtils.h"

class FillPathWithPaint   : public Effect {
public:
    FillPathWithPaint() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        SkPaint strokePaint;
        strokePaint.setAntiAlias(true);
        strokePaint.setStyle(SkPaint::kStroke_Style);
        strokePaint.setStrokeWidth(.1f);
        SkPath strokePath = SkPathBuilder()
                            .moveTo(.08f, .08f)
                            .quadTo(.09f, .08f, .17f, .17f)
                            .detach();
        SkPath fillPath;
        SkPaint outlinePaint(strokePaint);
        outlinePaint.setStrokeWidth(2);
        SkMatrix scale = SkMatrix::Scale(300, 300);
        for (SkScalar precision : { 0.01f, .1f, 1.f, 10.f, 100.f } ) {
            skpathutils::FillPathWithPaint(strokePath, strokePaint, &fillPath, nullptr,
                                        SkMatrix::Scale(precision, precision));
            fillPath.transform(scale);
            canvas->drawPath(fillPath.detach(), outlinePaint);
            canvas->translate(60, 0);
            if (1.f == precision) canvas->translate(-180, 100);
        }
        strokePath = strokePath.makeTransform(scale);
        strokePaint.setStrokeWidth(30);
        canvas->drawPath(strokePath, strokePaint);
    }

    void setImage(const SkImage* image) override {
        mImage = image;
    }
};

extern "C" Effect* createEffect() {
    return new FillPathWithPaint();
}