#include "../../Effect.h"
#include <spdlog/spdlog.h>

class ImageInfo_makeWH : public Effect {
public:
    ImageInfo_makeWH() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        SkImageInfo canvasImageInfo = canvas->imageInfo();
        SkRect canvasBounds = SkRect::Make(canvasImageInfo.bounds());
        spdlog::info("canvasBounds: left: {}, top: {}, right: {}, bottom: {}", 
            canvasBounds.left(), canvasBounds.top(), canvasBounds.right(), canvasBounds.bottom());
        canvas->drawImageRect(image, SkRect::Make(source.bounds()), canvasBounds, SkSamplingOptions(),
                            nullptr, SkCanvas::kStrict_SrcRectConstraint);
        SkImageInfo insetImageInfo =
                canvasImageInfo.makeWH(canvasBounds.width() / 2, canvasBounds.height() / 2);
        SkBitmap inset;
        inset.allocPixels(insetImageInfo);
        SkCanvas offscreen(inset);
        offscreen.drawImageRect(image, SkRect::Make(source.bounds()), SkRect::Make(inset.bounds()),
                                SkSamplingOptions(), nullptr, SkCanvas::kStrict_SrcRectConstraint);
        canvas->drawImage(inset.asImage(), canvasBounds.width() / 4, canvasBounds.height() / 4);
    }
};

extern "C" Effect* createEffect() {
    return new ImageInfo_makeWH();
}