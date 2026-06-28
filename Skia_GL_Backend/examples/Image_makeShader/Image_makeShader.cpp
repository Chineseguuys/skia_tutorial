#include "../../Effect.h"

#include "include/core/SkTileMode.h"
#include "skia/include/core/SkShader.h"

class Image_makeShader : public Effect {
public:
    Image_makeShader() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        SkMatrix matrix;
        matrix.setRotate(45);
        SkPaint paint;
        paint.setShader(image->makeShader(SkTileMode::kRepeat, SkTileMode::kMirror,
                                        SkSamplingOptions(), matrix));
        canvas->drawPaint(paint);
    }
};

class Image_makeShader_2 : public Effect {
public:
    Image_makeShader_2() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        SkMatrix matrix;
        matrix.setRotate(45);
        matrix.postTranslate(125, 30);
        SkPaint paint;
        paint.setShader(image->makeShader(SkTileMode::kClamp, SkTileMode::kClamp,
            SkSamplingOptions(), matrix));
        canvas->drawPaint(paint);
    }
};

extern "C" Effect* createEffect() {
    return new Image_makeShader_2{};
}