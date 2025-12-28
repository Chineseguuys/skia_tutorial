#include "SkSLCacheMonitor.h"
#include <spdlog/spdlog.h>

namespace renderengine {
namespace skia {

sk_sp<SkData> SkSLCacheMonitor::load(const SkData& key) {
    spdlog::warn("try load cache");
    return nullptr;
}

void SkSLCacheMonitor::store(const SkData& key, const SkData& data, const SkString& description) {
    mShadersCachedSinceLastCall++;
    mTotalShadersCompiled++;
    spdlog::warn("{} SF cache: {} shaders", __func__, mTotalShadersCompiled);
}

} // namespace renderengine
} // namespace skia