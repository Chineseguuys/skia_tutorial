#ifndef _SKSL_CACHE_MONITOR_H_
#define _SKSL_CACHE_MONITOR_H_

#include "skia/include/gpu/ganesh/GrContextOptions.h"

namespace renderengine {
namespace skia {

// Implements PersistentCache as a way to monitor what SkSL shaders Skia has
// cached.
class SkSLCacheMonitor : public GrContextOptions::PersistentCache {
public:
    SkSLCacheMonitor() = default;
    ~SkSLCacheMonitor() override = default;

    sk_sp<SkData> load(const SkData& key) override;

    void store(const SkData& key, const SkData& data, const SkString& description) override;

    int shadersCachedSinceLastCall() {
        const int shadersCachedSinceLastCall = mShadersCachedSinceLastCall;
        mShadersCachedSinceLastCall = 0;
        return shadersCachedSinceLastCall;
    }

    int totalShadersCompiled() const { return mTotalShadersCompiled; }

private:
    int mShadersCachedSinceLastCall = 0;
    int mTotalShadersCompiled = 0;
};


} // skia
} // renderengine


#endif // _SKSL_CACHE_MONITOR_H_