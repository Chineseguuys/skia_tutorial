#include "../../Effect.h"
#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"

class Image_CrossContextTextureFromPixmap : public Effect {
public:
    Image_CrossContextTextureFromPixmap() : Effect() {}

    void initialize(uint32_t canvasWidth, uint32_t canvasHeight) override {
        Effect::initialize(canvasWidth, canvasHeight);
    }

    void draw(SkCanvas* canvas) override {
        Effect::draw(canvas);
        auto dContext = GrAsDirectContext(canvas->recordingContext());
        if (!dContext) {
            spdlog::error("[{}:{}] Failed to get GrDirectContext", __FUNCTION__, __LINE__);
            return;
        }
        SkPixmap pixmap;
        if (!source.peekPixels(&pixmap)) {
            spdlog::error("[{}:{}] can not peek pixels from source", __FUNCTION__, __LINE__);
        } else {
            sk_sp<SkImage> image = SkImages::CrossContextTextureFromPixmap(
                dContext, pixmap, false);
            canvas->drawImage(image, 0, 0);
        }
    }
};

extern "C" Effect* createEffect() {
    return new Image_CrossContextTextureFromPixmap();
}