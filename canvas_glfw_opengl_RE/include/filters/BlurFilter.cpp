#include "BlurFilter.h"

#include "skia/include/core/SkCanvas.h"
#include "skia/include/core/SkRRect.h"
#include "skia/include/effects/SkRuntimeEffect.h"

#include <spdlog/spdlog.h>

namespace renderengine {
namespace skia {

static sk_sp<SkRuntimeEffect> createMixEffect() {
    SkString mixString(R"(
        uniform shader blurredInput;
        uniform shader originalInput;
        uniform float mixFactor;

        half4 main(float2 xy) {
            return half4(mix(originalInput.eval(xy), blurredInput.eval(xy), mixFactor)).rgb1;
        }
    )");

    auto [mixEffect, mixError] = SkRuntimeEffect::MakeForShader(mixString);
    if (!mixEffect) {
        // LOG_ALWAYS_FATAL("RuntimeShader error: %s", mixError.c_str());
        spdlog::critical("RuntimeShader error: {}", mixError.c_str());
        abort();
    }
    return mixEffect;
}

static SkMatrix getShaderTransform(const SkCanvas* canvas, const SkRect& blurRect,
                                   const float scale) {
    // 1. Apply the blur shader matrix, which scales up the blurred surface to its real size
    auto matrix = SkMatrix::Scale(scale, scale);
    // 2. Since the blurred surface has the size of the layer, we align it with the
    // top left corner of the layer position.
    matrix.postConcat(SkMatrix::Translate(blurRect.fLeft, blurRect.fTop));
    // 3. Finally, apply the inverse canvas matrix. The snapshot made in the BlurFilter is in the
    // original surface orientation. The inverse matrix has to be applied to align the blur
    // surface with the current orientation/position of the canvas.
    SkMatrix drawInverse;
    if (canvas != nullptr && canvas->getTotalMatrix().invert(&drawInverse)) {
        matrix.postConcat(drawInverse);
    }
    return matrix;
}

BlurFilter::BlurFilter(const float maxCrossFadeRadius)
      : mMaxCrossFadeRadius(maxCrossFadeRadius),
        mMixEffect(maxCrossFadeRadius > 0 ? createMixEffect() : nullptr) {}

float BlurFilter::getMaxCrossFadeRadius() const {
    return mMaxCrossFadeRadius;
}

void BlurFilter::drawBlurRegion(SkCanvas* canvas, const SkRRect& effectRegion,
                                const uint32_t blurRadius, const float blurAlpha,
                                const SkRect& blurRect, sk_sp<SkImage> blurredImage,
                                sk_sp<SkImage> input) {

    SkPaint paint;
    paint.setAlphaf(blurAlpha);

    const auto blurMatrix = getShaderTransform(canvas, blurRect, kInverseInputScale);
    SkSamplingOptions linearSampling(SkFilterMode::kLinear, SkMipmapMode::kNone);
    const auto blurShader = blurredImage->makeShader(SkTileMode::kClamp, SkTileMode::kClamp,
                                                     linearSampling, &blurMatrix);

    if (blurRadius < mMaxCrossFadeRadius) {
        // LOG_ALWAYS_FATAL_IF(!input);
        if (!input) {
            spdlog::critical("BlurFilter::drawBlurRegion: input image is null when doing crossfade");
            abort();
        }

        // For sampling Skia's API expects the inverse of what logically seems appropriate. In this
        // case you might expect the matrix to simply be the canvas matrix.
        SkMatrix inputMatrix;
        if (!canvas->getTotalMatrix().invert(&inputMatrix)) {
            // ALOGE("matrix was unable to be inverted");
            spdlog::error("BlurFilter::drawBlurRegion: matrix was unable to be inverted");
        }

        SkRuntimeShaderBuilder blurBuilder(mMixEffect);
        blurBuilder.child("blurredInput") = blurShader;
        blurBuilder.child("originalInput") =
                input->makeShader(SkTileMode::kClamp, SkTileMode::kClamp, linearSampling,
                                  inputMatrix);
        blurBuilder.uniform("mixFactor") = blurRadius / mMaxCrossFadeRadius;

        paint.setShader(blurBuilder.makeShader());
    } else {
        paint.setShader(blurShader);
    }

    if (effectRegion.isRect()) {
        if (blurAlpha == 1.0f) {
            paint.setBlendMode(SkBlendMode::kSrc);
        }
        canvas->drawRect(effectRegion.rect(), paint);
    } else {
        paint.setAntiAlias(true);
        canvas->drawRRect(effectRegion, paint);
    }
}

} // namespace skia
} // namespace renderengine