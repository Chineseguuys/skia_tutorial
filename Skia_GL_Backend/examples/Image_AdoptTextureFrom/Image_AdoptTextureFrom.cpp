#include "../../Effect.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include <spdlog/spdlog.h>

class Image_AdoptTextureFrom : public Effect {
public:
    Image_AdoptTextureFrom() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        GrDirectContext* dContext = GrAsDirectContext(canvas->recordingContext());
        if (!dContext) {
            spdlog::error("Failed to get GrDirectContext");
            return;
        }
        canvas->scale(.5f, .5f);
        canvas->clear(0x7F3F5F7F);
        int x = 0, y = 0;
        
    }
};


extern "C" Effect* createEffect() {
    return new Image_AdoptTextureFrom();
}