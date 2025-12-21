#ifndef _GANESH_BACKEND_TEXTURE_H_
#define _GANESH_BACKEND_TEXTURE_H_

#include "base/compiler.h"

#include "SkiaBackendTexture.h"
#include "include/core/SkImage.h"
#include "skia/include/gpu/ganesh/GrDirectContext.h"
#include "skia/include/gpu/ganesh/GrBackendSurface.h"

namespace renderengine {
namespace skia {

class GaneshBackendTexture : public SkiaBackendTexture {
public:
    // Creates an internal GrBackendTexture whose contents come from the provided buffer.
    GaneshBackendTexture(sk_sp<GrDirectContext> grContext, sk_sp<SkImage> buffer,
                         bool isOutputBuffer);

    ~GaneshBackendTexture() override;

    sk_sp<SkImage> makeImage(SkAlphaType alphaType, ui::Dataspace dataspace,
                             TextureReleaseProc releaseImageProc,
                             ReleaseContext releaseContext) override;

    sk_sp<SkSurface> makeSurface(ui::Dataspace dataspace, TextureReleaseProc releaseSurfaceProc,
                                 ReleaseContext releaseContext) override;

private:
    DISALLOW_COPY_AND_ASSIGN(GaneshBackendTexture);

    void logFatalTexture(const char* msg, ui::Dataspace dataspace, SkColorType colorType);

    const sk_sp<GrDirectContext> mGrContext;
    GrBackendTexture mBackendTexture;
    // GrAHardwareBufferUtils::DeleteImageProc mDeleteProc;
    // GrAHardwareBufferUtils::UpdateImageProc mUpdateProc;
    // GrAHardwareBufferUtils::TexImageCtx mImageCtx;
};

} // namespace skia
} // namespace renderengine

#endif // end _SKIA_BACKEND_TEXTURE_H_