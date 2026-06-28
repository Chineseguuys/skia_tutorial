#include "../../Effect.h"
#include "skia/include/core/SkCanvas.h"
#include "skia/include/core/SkPaint.h"
#include "skia/include/core/SkPictureRecorder.h"
#include "skia/include/core/SkColorSpace.h"
#include "skia/include/core/SkPicture.h"

class Image_DeferredFromPicture : public Effect {
public:
    Image_DeferredFromPicture() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        SkPaint paint;
        SkPictureRecorder recorder;
        SkCanvas* recordingCanvas = recorder.beginRecording(50, 50);
        for (auto color : { SK_ColorRED/*0xFFFF0000*/, SK_ColorBLUE/*0xFF0000FF*/, 0xff007f00/*0xFF007F00*/ } ) {
            paint.setColor(color);
            recordingCanvas->drawRect({10, 10, 30, 40}, paint);
            recordingCanvas->translate(10, 10);
            recordingCanvas->scale(1.2f, 1.4f);
        }
        sk_sp<SkPicture> playback = recorder.finishRecordingAsPicture();
        int x = 0, y = 0;
        for (auto alpha : { 70, 140, 210 } ) {
            paint.setAlpha(alpha);
            auto srgbColorSpace = SkColorSpace::MakeSRGB();
            sk_sp<SkImage> image = SkImages::DeferredFromPicture(
                    playback, {50, 50}, nullptr, &paint, SkImages::BitDepth::kU8, srgbColorSpace);
            canvas->drawImage(image, x, y);
            x += 70; y += 70;
        }
    }
};

extern "C" Effect* createEffect() {
    return new Image_DeferredFromPicture{};
}