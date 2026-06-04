#ifndef _EFFECT_H_
#define _EFFECT_H_

#include "include/core/SkBitmap.h"
#include "skia/include/core/SkCanvas.h"
#include "skia/include/core/SkImage.h"
class Effect {
protected:
    uint32_t mCanvasWidth;
    uint32_t mCanvasHeight;
    const SkImage* mImage;
    const SkBitmap* mSource;
public:
    Effect() : mCanvasWidth(0), mCanvasHeight(0), mImage(nullptr) {}
    virtual ~Effect() {}

    virtual void initialize(uint32_t canvasWidth, uint32_t canvasHeight) {
        mCanvasWidth = canvasWidth;
        mCanvasHeight = canvasHeight;
    }

    virtual void setImage(const SkImage* image) {}

    virtual void setBitmap(const SkBitmap* bitmap) {}

    virtual void draw(SkCanvas* canvas) {}
};

#endif // _EFFECT_H_