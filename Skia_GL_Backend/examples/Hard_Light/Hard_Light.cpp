#include "../../Effect.h"
#include "include/core/SkTileMode.h"
#include "skia/include/effects/SkGradientShader.h"
#include <iterator>

class Hard_Light   : public Effect {
public:
    Hard_Light() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        canvas->drawImage(image, 0, 0);
        // 0xFFFF00FF
        const SkColor4f colors[] = { {1,0,1,1}, {0,0,0,0} };
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kHardLight);
        paint.setShader(SkGradientShader::MakeRadial(
            {128, 128}, 
            100, 
            colors, 
            nullptr,
            nullptr,
            std::size(colors), 
            SkTileMode::kClamp, 
            SkGradientShader::Interpolation{}, 
            nullptr));

        canvas->clipRect({0, 128, 256, 256});
        canvas->drawPaint(paint);
    }
};


extern "C" Effect* createEffect() {
    return new Hard_Light();
}