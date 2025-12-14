#include "GaussianBlurFilter.h"

#include "skia/include/effects/SkImageFilters.h"

namespace renderengine {
namespace skia {

// This constant approximates the scaling done in the software path's
// "high quality" mode, in SkBlurMask::Blur() (1 / sqrt(3)).
static const float BLUR_SIGMA_SCALE = 0.57735f;

GaussianBlurFilter::GaussianBlurFilter(): BlurFilter(/* maxCrossFadeRadius= */ 0.0f) {}

sk_sp<SkImage> GaussianBlurFilter::generate(SkiaGpuContext* context, const uint32_t blurRadius,
                                            const sk_sp<SkImage> input,
                                            const SkRect& blurRect) const {
    // Create blur surface with the bit depth and colorspace of the original surface
    SkImageInfo scaledInfo = input->imageInfo().makeWH(std::ceil(blurRect.width() * kInputScale),
                                                       std::ceil(blurRect.height() * kInputScale));
    sk_sp<SkSurface> surface = context->createRenderTarget(scaledInfo);

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setImageFilter(SkImageFilters::Blur(
                blurRadius * kInputScale * BLUR_SIGMA_SCALE,
                blurRadius * kInputScale * BLUR_SIGMA_SCALE,
                SkTileMode::kClamp, nullptr));

    surface->getCanvas()->drawImageRect(
            input,
            blurRect,
            SkRect::MakeWH(scaledInfo.width(), scaledInfo.height()),
            SkSamplingOptions{SkFilterMode::kLinear, SkMipmapMode::kNone},
            &paint,
            SkCanvas::SrcRectConstraint::kFast_SrcRectConstraint);
    return surface->makeTemporaryImage();
}

}
}