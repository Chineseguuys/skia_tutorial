#ifndef _SIKA_GPU_CONTEXT_H_
#define _SIKA_GPU_CONTEXT_H_

#include "skia/include/core/SkImage.h"
#include "skia/include/core/SkSurface.h"
#include "skia/include/gpu/ganesh/GrDirectContext.h"
#include "skia/include/gpu/ganesh/gl/GrGLInterface.h"
#include "skia/include/gpu/graphite/Context.h"

#include "SkiaBackendTexture.h"

namespace renderengine {
namespace skia {

class SkiaGpuContext {
public:
    /**
     * glInterface must remain valid until after SkiaGpuContext is destroyed.
     */
    static std::unique_ptr<SkiaGpuContext> MakeGL_Ganesh(
            sk_sp<const GrGLInterface> glInterface,
            GrContextOptions::PersistentCache& skSLCacheMonitor);

    virtual ~SkiaGpuContext() = default;

    /**
     * Only callable on Ganesh-backed instances of SkiaGpuContext, otherwise fatal.
     */
    virtual sk_sp<GrDirectContext> grDirectContext() {
        return nullptr;
    }

    /**
     * Only callable on Graphite-backed instances of SkiaGpuContext, otherwise fatal.
     */
    virtual std::shared_ptr<skgpu::graphite::Context> graphiteContext() {
        return nullptr;
    }

    /**
     * Only callable on Graphite-backed instances of SkiaGpuContext, otherwise fatal.
     */
    virtual std::shared_ptr<skgpu::graphite::Recorder> graphiteRecorder() {
        return nullptr;
    }

    virtual std::unique_ptr<SkiaBackendTexture> makeBackendTexture(sk_sp<SkImage> buffer,
                                                                   bool isOutputBuffer) = 0;

    /**
     * Notes:
     * - The surface doesn't count against Skia's caching budgets.
     * - Protected status is set to match the implementation's underlying context.
     * - The origin of the surface in texture space corresponds to the top-left content pixel.
     * - AA is always enabled.
     */
    virtual sk_sp<SkSurface> createRenderTarget(SkImageInfo imageInfo) = 0;

    virtual bool isAbandonedOrDeviceLost() = 0;
    virtual size_t getMaxRenderTargetSize() const = 0;
    virtual size_t getMaxTextureSize() const = 0;
    virtual void setResourceCacheLimit(size_t maxResourceBytes) = 0;

    virtual void purgeUnlockedScratchResources() = 0;
    virtual void resetContextIfApplicable() = 0; // No-op outside of GL (&& Ganesh at this point.)

    virtual void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const = 0;
};

}
}


#endif // end _SIKA_GPU_CONTEXT_H_