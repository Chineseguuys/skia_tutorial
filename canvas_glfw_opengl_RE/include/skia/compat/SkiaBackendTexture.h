#ifndef _SKIA_BACKEND_TEXTURE_H_
#define _SKIA_BACKEND_TEXTURE_H_

#include "include/core/SkData.h"
#include "skia/include/core/SkImage.h"
#include "skia/include/core/SkSurface.h"
#include "skia/include/core/SkColorType.h"

#include "ui/Dataspace.h"

namespace renderengine {
namespace skia {

class SkiaBackendTexture {
public:
    SkiaBackendTexture(sk_sp<SkImage> buffer, bool isOutputBuffer)
          : mIsOutputBuffer(isOutputBuffer) {

        mColorType = kRGBA_8888_SkColorType;
    }
    virtual ~SkiaBackendTexture() = default;

    // These two definitions mirror Skia's own types used for texture release callbacks, which are
    // re-declared multiple times between context-specific implementation headers for Ganesh vs.
    // Graphite, and within the context of SkImages vs. SkSurfaces. Our own re-declaration allows us
    // to not pull in any implementation-specific headers here.
    using ReleaseContext = void*;
    using TextureReleaseProc = void (*)(ReleaseContext);

    // Guaranteed to be non-null (crashes otherwise). An opaque alphaType may coerce the internal
    // color type to RBGX.
    virtual sk_sp<SkImage> makeImage(SkAlphaType alphaType, ui::Dataspace dataspace,
                                     TextureReleaseProc releaseImageProc,
                                     ReleaseContext releaseContext) = 0;

    // Guaranteed to be non-null (crashes otherwise).
    virtual sk_sp<SkSurface> makeSurface(ui::Dataspace dataspace,
                                         TextureReleaseProc releaseSurfaceProc,
                                         ReleaseContext releaseContext) = 0;

    bool isOutputBuffer() const { return mIsOutputBuffer; }

    SkColorType internalColorType() const { return mColorType; }

protected:
    // Strip alpha channel from rawColorType if alphaType is opaque (note: only works for RGBA_8888)
    SkColorType colorTypeForImage(SkAlphaType alphaType) const {
        if (alphaType == kOpaque_SkAlphaType) {
            // TODO: b/40043126 - Support RGBX SkColorType for F16 and support it and 101010x as a
            // source
            if (internalColorType() == kRGBA_8888_SkColorType) {
                return kRGB_888x_SkColorType;
            }
        }
        return internalColorType();
    }

private:
    const bool mIsOutputBuffer;
    SkColorType mColorType = kUnknown_SkColorType;
};


} // namespace skia
} // namespace renderengine

#endif // end _SKIA_BACKEND_TEXTURE_H_