#ifndef _EFFECT_H_
#define _EFFECT_H_

#include "include/core/SkBitmap.h"
#include "include/core/SkSize.h"
#include "include/gpu/GpuTypes.h"
#include "skia/include/core/SkCanvas.h"
#include "skia/include/core/SkImage.h"
#include "skia/include/core/SkFontMgr.h"
#include "skia/include/gpu/ganesh/SkSurfaceGanesh.h"
#include <spdlog/spdlog.h>

extern GrBackendTexture backEndTexture;
extern GrBackendRenderTarget backEndRenderTarget;
extern GrBackendTexture backEndTextureRenderTarget;
extern sk_sp<SkFontMgr> fontMgr;
extern sk_sp<SkTypeface> typeFace;
extern SkBitmap source;
extern sk_sp<SkImage> image;
extern double duration;
extern double frame;

struct DrawOptions {
    SkISize size;
    bool raster;
    bool gpu;
    bool pdf;
    bool skp;
    bool srgb;
    bool f16;
    bool textOnly;
    bool saveRender;
    bool display;
    int sourceIndex;
    int alphaType;
    skgpu::Mipmapped fMipMapping;

    int offScreenWidth;
    int offScreenHeight;

    skgpu::Mipmapped fOffScreenMipMapping;
    // maybe unused
    const char* source;
};

class Effect {
protected:
    uint32_t mCanvasWidth;
    uint32_t mCanvasHeight;
public:
    Effect() : mCanvasWidth(0), mCanvasHeight(0) {}
    virtual ~Effect() {
        spdlog::info("[{}:{}] destroy effect", __FUNCTION__, __LINE__);
    }

    virtual void initialize(uint32_t canvasWidth, uint32_t canvasHeight) {
        mCanvasWidth = canvasWidth;
        mCanvasHeight = canvasHeight;
    }

    virtual void draw(SkCanvas* canvas) {}
};

#endif // _EFFECT_H_