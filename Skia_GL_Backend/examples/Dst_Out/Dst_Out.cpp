#include "../../Effect.h"
#include "include/core/SkColor.h"
#include "skia/include/effects/SkGradientShader.h"

class Dst_out   : public Effect {
public:
    Dst_out() : Effect() {}

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
        // r = d * (1-sa)
        paint.setBlendMode(SkBlendMode::kDstOut);
        // SkColor4f alphas[] = { SkColors::kBlack, SkColors::kTransparent };
        SkColor alphas[] = {SK_ColorBLACK, SK_ColorTRANSPARENT};
        SkPoint vert[] = { { 0, 0 }, { 0, 256 } };
        paint.setShader(SkGradientShader::MakeLinear(vert, alphas, nullptr, std::size(alphas), SkTileMode::kClamp));
        canvas->drawPaint(paint);
        canvas->clipRect( { 30, 30, 226, 226 } );
        canvas->drawColor(SkColorSetA(SK_ColorGREEN, 128), SkBlendMode::kDstIn);
    }
};

extern "C" Effect* createEffect() {
    return new Dst_out();
}