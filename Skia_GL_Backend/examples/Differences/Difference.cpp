#include "../../Effect.h"
#include "include/core/SkColor.h"
#include "skia/include/core/SkImage.h"
#include "skia/include/effects/SkGradientShader.h"

class DifferenceEffect : public Effect {
public:
    DifferenceEffect() : Effect() {}
    ~DifferenceEffect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void setImage(const SkImage* image) override {
        mImage = image;
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        canvas->drawImage(mImage, 0, 0);
        canvas->drawImage(mImage, 128, 0);
        canvas->drawImage(mImage, 0, 128);
        canvas->drawImage(mImage, 128, 128);
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kDstATop);
        SkColor alphas[] = { SK_ColorBLACK, SK_ColorTRANSPARENT };
        SkPoint vert[] = { { 0, 0 }, { 0, 256 } };
        paint.setShader(SkGradientShader::MakeLinear(vert,alphas,nullptr,std::size(alphas), SkTileMode::kClamp));
        canvas->drawPaint(paint);
        canvas->clipRect( { 30, 30, 226, 226 } );
        canvas->drawColor(0x80bb9977, SkBlendMode::kDifference);
    }
private:
    const SkImage* mImage;
};

extern "C" Effect* createEffect() {
    return new DifferenceEffect();
}