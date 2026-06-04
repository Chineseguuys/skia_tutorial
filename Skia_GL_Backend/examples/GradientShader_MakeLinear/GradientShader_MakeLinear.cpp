#include "../../Effect.h"
#include "include/core/SkColor.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkGradientShader.h"
#include <spdlog/spdlog.h>

class GradientShader_MakeLinear   : public Effect {
public:
    GradientShader_MakeLinear() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        // This fiddle draws 4 instances of a LinearGradient, demonstrating
        // how the local matrix affects the gradient as well as the flag
        // which controls unpremul vs premul color interpolation.

        SkPaint strokePaint;
        strokePaint.setStyle(SkPaint::kStroke_Style);
        strokePaint.setColor(SK_ColorBLACK);

        SkPaint p;
        p.setStyle(SkPaint::kFill_Style);

        SkColor4f transparentGreen{.fR = 0.0f, .fG = 1.0f, .fB = 0.0f, .fA = 0.0f};
        SkColor4f colors[] = { transparentGreen, SkColors::kBlue, SkColors::kRed };
        SkScalar positions[] = { 0.0, 0.65, 1.0 };

        for (int i = 0; i < 4; i++) {
            SkScalar blockX = (i % 2) * 100;
            SkScalar blockY = (i >> 1) * 100;
            spdlog::debug("blockX: {}, blockY: {}", blockX, blockY);
            SkPoint pts[] = { {blockX, blockY}, {blockX + 50, blockY + 100} };

            auto premul = SkGradientShader::Interpolation::InPremul::kNo;
            if (i % 2 == 1) {
                // right column will have premul
                premul = SkGradientShader::Interpolation::InPremul::kYes;
            }

            SkMatrix matr = SkMatrix::I();
            if (i / 2 == 1) {
                // bottom row will be rotated 45 degrees.
                matr.setRotate(45, blockX, blockY);
            }

            auto lgs = SkGradientShader::MakeLinear(
                pts, 
                colors, 
                nullptr, 
                positions, 
                std::size(colors), 
                SkTileMode::kMirror, 
                SkGradientShader::Interpolation{.fInPremul = premul}, 
                &matr);

            p.setShader(lgs);
            auto r = SkRect::MakeLTRB(blockX, blockY, blockX + 100, blockY + 100);
            canvas->drawRect(r, p);
            canvas->drawRect(r, strokePaint);
        }
    }
};


extern "C" Effect* createEffect() {
    return new GradientShader_MakeLinear();
}