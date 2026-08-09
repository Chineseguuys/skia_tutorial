#include "../../Effect.h"
#include "include/core/SkColor.h"

class Image_makeSubset : public Effect {
public:
    Image_makeSubset() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        canvas->scale(.5f, .5f);
        const int width = 64;
        const int height = 64;
        for (int y = 0; y < 512; y += height ) {
            for (int x = 0; x < 512; x += width ) {
                sk_sp<SkImage> subset(image->makeSubset(nullptr, {x, y, x + width, y + height}, {}));
                canvas->drawImage(subset,
                    x * 3 >> 1,
                    y * 3 >> 1
                );
            }
        }
    }
};

extern "C" {
    Effect* createEffect() {
        return new Image_makeSubset();
    }
}