#include "../../Effect.h"
#include <spdlog/spdlog.h>

class HSVToColor_2   : public Effect {
public:
    HSVToColor_2() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        canvas->drawImage(image, 0, 0);
        for (int y = 0; y < 256; ++y) {
            for (int x = 0; x < 256; ++x) {
                SkColor color = source.getColor(x, y);
                SkScalar hsv[3];
                SkColorToHSV(color, hsv);
                hsv[0] = hsv[0] + 90 >= 360 ? hsv[0] - 270 : hsv[0] + 90;
                SkPaint paint;
                paint.setColor(SkHSVToColor(hsv));
                canvas->drawRect(SkRect::MakeXYWH(x, y, 1, 1), paint);
            }
        }
    }
};


extern "C" Effect* createEffect() {
    return new HSVToColor_2();
}