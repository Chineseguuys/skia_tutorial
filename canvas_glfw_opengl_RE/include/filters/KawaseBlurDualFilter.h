#ifndef _KAWASE_BLUR_DUAL_FILTER_H_
#define _KAWASE_BLUR_DUAL_FILTER_H_

#include "BlurFilter.h"

namespace renderengine {
namespace skia {

/**
 * This is an implementation of a Kawase blur with dual-filtering passes, as described in here:
 * https://community.arm.com/cfs-file/__key/communityserver-blogs-components-weblogfiles/00-00-00-20-66/siggraph2015_2D00_mmg_2D00_marius_2D00_slides.pdf
 * https://community.arm.com/cfs-file/__key/communityserver-blogs-components-weblogfiles/00-00-00-20-66/siggraph2015_2D00_mmg_2D00_marius_2D00_notes.pdf
 */
class KawaseBlurDualFilter : public BlurFilter {
public:
    explicit KawaseBlurDualFilter();
    virtual ~KawaseBlurDualFilter() {}

    // Execute blur, saving it to a texture
    sk_sp<SkImage> generate(SkiaGpuContext* context, const uint32_t radius,
                            const sk_sp<SkImage> blurInput, const SkRect& blurRect) const override;

private:
    sk_sp<SkRuntimeEffect> mLowSampleBlurEffect;
    sk_sp<SkRuntimeEffect> mHighSampleBlurEffect;

    void blurInto(const sk_sp<SkSurface>& drawSurface, const sk_sp<SkImage>& readImage,
                  const float radius, const float alpha, const sk_sp<SkRuntimeEffect>&) const;

    void blurInto(const sk_sp<SkSurface>& drawSurface, const sk_sp<SkShader> input,
                  const float radius, const float alpha, const sk_sp<SkRuntimeEffect>&) const;
};

}
}

#endif // _KAWASE_BLUR_DUAL_FILTER_H_