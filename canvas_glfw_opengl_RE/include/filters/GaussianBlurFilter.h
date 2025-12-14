#ifndef _GAUSSIAN_BLUR_FILTER_H_
#define _GAUSSIAN_BLUR_FILTER_H_

#include "BlurFilter.h"

namespace renderengine {
namespace skia {

/**
 * This is an implementation of a Gaussian blur using Skia's built-in GaussianBlur filter.
 */
class GaussianBlurFilter: public BlurFilter {
public:
    explicit GaussianBlurFilter();
    virtual ~GaussianBlurFilter(){}

    // Execute blur, saving it to a texture
    sk_sp<SkImage> generate(SkiaGpuContext* context, const uint32_t radius,
                            const sk_sp<SkImage> blurInput, const SkRect& blurRect) const override;
};

} // namespace skia
} // namespace renderengine

#endif // end _GAUSSIAN_BLUR_FILTER_H_