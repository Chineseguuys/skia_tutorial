#include "../../Effect.h"

class Hue   : public Effect {
public:
    Hue() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void setBitmap(const SkBitmap* bitmap) override {
        mSource = bitmap;
    }

    void setImage(const SkImage* image) override {
        mImage = image;
    }

    // Hue 混合模式（色相混合）：

    // 保留：目标的亮度和饱和度
    // 替换：目标的色相，使用源的色相
    // 效果：保留原图的明暗和浓淡程度，只改变颜色倾向
    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        canvas->drawImage(mImage, 0, 0);
        canvas->drawColor(0xFF00FF00, SkBlendMode::kHue);
    }
};


extern "C" Effect* createEffect() {
    return new Hue();
}