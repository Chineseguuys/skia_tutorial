#ifndef _EFFECT_H_
#define _EFFECT_H_

#include "skia/include/core/SkCanvas.h"
class Effect {
protected:
    uint32_t mCanvasWidth;
    uint32_t mCanvasHeight;
public:
    Effect() : mCanvasWidth(0), mCanvasHeight(0) {}
    virtual ~Effect() {}

    virtual void initialize(uint32_t canvasWidth, uint32_t canvasHeight) {
        mCanvasWidth = canvasWidth;
        mCanvasHeight = canvasHeight;
    }

    virtual void setImage(const SkImage* image) {}

    virtual void draw(SkCanvas* canvas) {}
};

#endif // _EFFECT_H_