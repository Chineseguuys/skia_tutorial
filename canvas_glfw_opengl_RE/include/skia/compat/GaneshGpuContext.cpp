#include "GaneshGpuContext.h"
#include "GaneshBackendTexture.h"

#include "skia/include/core/SkImage.h"
#include "skia/include/core/SkSurface.h"
#include "include/core/SkRefCnt.h"
#include "skia/include/gpu/ganesh/GrDirectContext.h"
#include "skia/include/gpu/ganesh/SkSurfaceGanesh.h"
#include "skia/include/gpu/ganesh/gl/GrGLDirectContext.h"

#include <spdlog/spdlog.h>

namespace renderengine {
namespace skia {

namespace {
static GrContextOptions ganeshOptions(GrContextOptions::PersistentCache& skSLCacheMonitor) {
    GrContextOptions options;
    options.fDisableDriverCorrectnessWorkarounds = true;
    options.fDisableDistanceFieldPaths = true;
    options.fReducedShaderVariations = true;
    options.fPersistentCache = &skSLCacheMonitor;
    return options;
}
}

std::unique_ptr<SkiaGpuContext> SkiaGpuContext::MakeGL_Ganesh(
        sk_sp<const GrGLInterface> glInterface,
        GrContextOptions::PersistentCache& skSLCacheMonitor) {
    return std::make_unique<GaneshGpuContext>(
            GrDirectContexts::MakeGL(glInterface, ganeshOptions(skSLCacheMonitor)));
}

GaneshGpuContext::GaneshGpuContext(sk_sp<GrDirectContext> grContext) : mGrContext(grContext) {
}

GaneshGpuContext::~GaneshGpuContext() {
    mGrContext->flushAndSubmit(GrSyncCpu::kYes);
    mGrContext->abandonContext();
};

sk_sp<GrDirectContext> GaneshGpuContext::grDirectContext() {
    return mGrContext;
}

std::unique_ptr<SkiaBackendTexture> GaneshGpuContext::makeBackendTexture(sk_sp<SkImage> buffer,
                                                                         bool isOutputBuffer) {
    return std::make_unique<GaneshBackendTexture>(mGrContext, buffer, isOutputBuffer);
}

sk_sp<SkSurface> GaneshGpuContext::createRenderTarget(SkImageInfo imageInfo) {
    spdlog::debug("{}:{}", __func__, __LINE__);
    constexpr int kSampleCount = 1; // enable AA
    constexpr SkSurfaceProps* kProps = nullptr;
    constexpr bool kMipmapped = false;
    return SkSurfaces::RenderTarget(mGrContext.get(), skgpu::Budgeted::kNo, imageInfo, kSampleCount,
                                    kTopLeft_GrSurfaceOrigin, kProps, kMipmapped,
                                    mGrContext->supportsProtectedContent());
}

size_t GaneshGpuContext::getMaxRenderTargetSize() const {
    return mGrContext->maxRenderTargetSize();
};

size_t GaneshGpuContext::getMaxTextureSize() const {
    return mGrContext->maxTextureSize();
};

bool GaneshGpuContext::isAbandonedOrDeviceLost() {
    return mGrContext->abandoned();
}

void GaneshGpuContext::setResourceCacheLimit(size_t maxResourceBytes) {
    mGrContext->setResourceCacheLimit(maxResourceBytes);
}

void GaneshGpuContext::purgeUnlockedScratchResources() {
    mGrContext->purgeUnlockedResources(GrPurgeResourceOptions::kScratchResourcesOnly);
}

void GaneshGpuContext::resetContextIfApplicable() {
    mGrContext->resetContext(); // Only applicable to GL
};

void GaneshGpuContext::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    mGrContext->dumpMemoryStatistics(traceMemoryDump);
}

} // namespace skia
} // namespace renderengine