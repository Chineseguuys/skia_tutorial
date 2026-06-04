#include "../../Effect.h"
#include "include/core/SkColor.h"
#include "skia/include/effects/SkGradientShader.h"

class Dst_Atop : public Effect {
public:
    Dst_Atop() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        // SkColor4f colors[] = { SkColors::kRed, SkColors::kBlue };
        SkColor colors[] = {SK_ColorRED, SK_ColorBLUE};
        SkPoint horz[] = { { 0, 0 }, { 256, 0 } };
        SkPaint paint;
        paint.setShader(SkGradientShader::MakeLinear(horz, colors, nullptr, std::size(colors), SkTileMode::kClamp));
        canvas->drawPaint(paint);
        //  r = d * sa + s * (1 - da)
        paint.setBlendMode(SkBlendMode::kDstATop);
        // SkColor4f alphas[] = { SkColors::kBlack, SkColors::kTransparent };
        SkColor alphas[] = {SK_ColorBLACK, SK_ColorTRANSPARENT};
        SkPoint vert[] = { { 0, 0 }, { 0, 256 } };
        paint.setShader(SkGradientShader::MakeLinear(vert, alphas, nullptr, std::size(alphas), SkTileMode::kClamp));
        canvas->drawPaint(paint);
        canvas->clipRect( { 30, 30, 226, 226 } );
        canvas->drawColor(SkColorSetA(SK_ColorGREEN, 128), SkBlendMode::kSrcATop);
    }
};

extern "C" Effect* createEffect() {
    return new Dst_Atop();
}