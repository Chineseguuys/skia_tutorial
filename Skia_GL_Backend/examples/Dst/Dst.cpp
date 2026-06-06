#include "../../Effect.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include <iterator>

class DstEffect : public Effect {
public:
    DstEffect() : Effect() {}
    ~DstEffect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        SkRSXform xforms[] = { { .5f, 0, 0, 0 }, {0, .5f, 125, 128 } };
        SkRect tex[] = { { 0, 0, 250, 250 }, { 0, 0, 250, 250 } };
        // SkColor colors[] = { 0x7f55aa00, 0x7f3333bf };
        SkColor colors[] = { SkColorSetARGB(0x7f, 0x55, 0xaa, 0x00), SkColorSetARGB(0x7f, 0x33, 0x33, 0xbf) };
        SkSamplingOptions sampling;
        canvas->drawAtlas(image.get(), xforms, tex, colors, std::size(colors), SkBlendMode::kSrc,
                        sampling, nullptr, nullptr);
        canvas->translate(128, 0);
        canvas->drawAtlas(image.get(), xforms, tex, colors, std::size(colors), SkBlendMode::kDst,
                        sampling, nullptr, nullptr);
    }
};

extern "C" Effect* createEffect() {
    return new DstEffect();
}