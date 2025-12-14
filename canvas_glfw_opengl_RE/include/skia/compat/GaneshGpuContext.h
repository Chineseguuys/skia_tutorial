#ifndef _GANESH_GPU_CONTEXT_H_
#define _GANESH_GPU_CONTEXT_H_

#include "compiler.h"
#include "SkiaGpuContext.h"
#include "include/core/SkImage.h"

namespace renderengine {
namespace skia {

class GaneshGpuContext : public SkiaGpuContext {
public:
    GaneshGpuContext(sk_sp<GrDirectContext> grContext);
    ~GaneshGpuContext() override;

    sk_sp<GrDirectContext> grDirectContext() override;

    std::unique_ptr<SkiaBackendTexture> makeBackendTexture(sk_sp<SkImage> buffer,
                                                           bool isOutputBuffer) override;

    sk_sp<SkSurface> createRenderTarget(SkImageInfo imageInfo) override;

    size_t getMaxRenderTargetSize() const override;
    size_t getMaxTextureSize() const override;
    bool isAbandonedOrDeviceLost() override;
    void setResourceCacheLimit(size_t maxResourceBytes) override;

    void purgeUnlockedScratchResources() override;
    void resetContextIfApplicable() override;

    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const override;

private:
    DISALLOW_COPY_AND_ASSIGN(GaneshGpuContext);

    const sk_sp<GrDirectContext> mGrContext;
};

} // namespace skia
} // namespace renderengine


#endif // end _SIKA_GPU_CONTEXT_H_