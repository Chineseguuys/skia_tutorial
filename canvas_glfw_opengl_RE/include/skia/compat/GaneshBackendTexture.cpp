#include "GaneshBackendTexture.h"

#include "include/core/SkData.h"
#include "skia/compat/SkiaBackendTexture.h"

namespace renderengine {
namespace skia {


GaneshBackendTexture::GaneshBackendTexture(sk_sp<GrDirectContext> grContext, sk_sp<SkImage> buffer,
    bool isOutputBuffer)
      : SkiaBackendTexture(buffer, isOutputBuffer), mGrContext(grContext) {
    // SFTRACE_CALL();
    // AHardwareBuffer_Desc desc;
    // AHardwareBuffer_describe(buffer, &desc);
    // const bool createProtectedImage = 0 != (desc.usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT);

    // GrBackendFormat backendFormat;
    // const GrBackendApi graphicsApi = grContext->backend();
    // if (graphicsApi == GrBackendApi::kOpenGL) {
    //     backendFormat =
    //             GrAHardwareBufferUtils::GetGLBackendFormat(grContext.get(), desc.format, false);
    //     mBackendTexture =
    //             GrAHardwareBufferUtils::MakeGLBackendTexture(grContext.get(), buffer, desc.width,
    //                                                          desc.height, &mDeleteProc,
    //                                                          &mUpdateProc, &mImageCtx,
    //                                                          createProtectedImage, backendFormat,
    //                                                          isOutputBuffer);
    // } else if (graphicsApi == GrBackendApi::kVulkan) {
    //     backendFormat = GrAHardwareBufferUtils::GetVulkanBackendFormat(grContext.get(), buffer,
    //                                                                    desc.format, false);
    //     mBackendTexture =
    //             GrAHardwareBufferUtils::MakeVulkanBackendTexture(grContext.get(), buffer,
    //                                                              desc.width, desc.height,
    //                                                              &mDeleteProc, &mUpdateProc,
    //                                                              &mImageCtx, createProtectedImage,
    //                                                              backendFormat, isOutputBuffer);
    // } else {
    //     LOG_ALWAYS_FATAL("Unexpected graphics API %u", static_cast<unsigned>(graphicsApi));
    // }

    // if (!mBackendTexture.isValid() || !desc.width || !desc.height) {
    //     LOG_ALWAYS_FATAL("Failed to create a valid texture. [%p]:[%d,%d] isProtected:%d "
    //                      "isWriteable:%d format:%d",
    //                      this, desc.width, desc.height, createProtectedImage, isOutputBuffer,
    //                      desc.format);
    // }
}

GaneshBackendTexture::~GaneshBackendTexture() {
    // if (mBackendTexture.isValid()) {
    //     mDeleteProc(mImageCtx);
    //     mBackendTexture = {};
    // }
}

sk_sp<SkImage> GaneshBackendTexture::makeImage(SkAlphaType alphaType, ui::Dataspace dataspace,
                                               TextureReleaseProc releaseImageProc,
                                               ReleaseContext releaseContext) {
    // if (mBackendTexture.isValid()) {
    //     mUpdateProc(mImageCtx, mGrContext.get());
    // }

    // const SkColorType colorType = colorTypeForImage(alphaType);
    // sk_sp<SkImage> image =
    //         SkImages::BorrowTextureFrom(mGrContext.get(), mBackendTexture, kTopLeft_GrSurfaceOrigin,
    //                                     colorType, alphaType, toSkColorSpace(dataspace),
    //                                     releaseImageProc, releaseContext);
    // if (!image) {
    //     logFatalTexture("Unable to generate SkImage.", dataspace, colorType);
    // }
    // return image;
    return nullptr;
}

sk_sp<SkSurface> GaneshBackendTexture::makeSurface(ui::Dataspace dataspace,
                                                   TextureReleaseProc releaseSurfaceProc,
                                                   ReleaseContext releaseContext) {
    // const SkColorType colorType = internalColorType();
    // sk_sp<SkSurface> surface =
    //         SkSurfaces::WrapBackendTexture(mGrContext.get(), mBackendTexture,
    //                                        kTopLeft_GrSurfaceOrigin, 0, colorType,
    //                                        toSkColorSpace(dataspace), nullptr, releaseSurfaceProc,
    //                                        releaseContext);
    // if (!surface) {
    //     logFatalTexture("Unable to generate SkSurface.", dataspace, colorType);
    // }
    // return surface;
    return nullptr;
}

void GaneshBackendTexture::logFatalTexture(const char* msg, ui::Dataspace dataspace,
                                           SkColorType colorType) {
    // switch (mBackendTexture.backend()) {
    //     case GrBackendApi::kOpenGL: {
    //         GrGLTextureInfo textureInfo;
    //         bool retrievedTextureInfo =
    //                 GrBackendTextures::GetGLTextureInfo(mBackendTexture, &textureInfo);
    //         LOG_ALWAYS_FATAL("%s isTextureValid:%d dataspace:%d"
    //                          "\n\tGrBackendTexture: (%i x %i) hasMipmaps: %i isProtected: %i "
    //                          "texType: %i\n\t\tGrGLTextureInfo: success: %i fTarget: %u fFormat: %u"
    //                          " colorType %i",
    //                          msg, mBackendTexture.isValid(), static_cast<int32_t>(dataspace),
    //                          mBackendTexture.width(), mBackendTexture.height(),
    //                          mBackendTexture.hasMipmaps(), mBackendTexture.isProtected(),
    //                          static_cast<int>(mBackendTexture.textureType()), retrievedTextureInfo,
    //                          textureInfo.fTarget, textureInfo.fFormat, colorType);
    //         break;
    //     }
    //     case GrBackendApi::kVulkan: {
    //         GrVkImageInfo imageInfo;
    //         bool retrievedImageInfo =
    //                 GrBackendTextures::GetVkImageInfo(mBackendTexture, &imageInfo);
    //         LOG_ALWAYS_FATAL("%s isTextureValid:%d dataspace:%d"
    //                          "\n\tGrBackendTexture: (%i x %i) hasMipmaps: %i isProtected: %i "
    //                          "texType: %i\n\t\tVkImageInfo: success: %i fFormat: %i "
    //                          "fSampleCount: %u fLevelCount: %u colorType %i",
    //                          msg, mBackendTexture.isValid(), static_cast<int32_t>(dataspace),
    //                          mBackendTexture.width(), mBackendTexture.height(),
    //                          mBackendTexture.hasMipmaps(), mBackendTexture.isProtected(),
    //                          static_cast<int>(mBackendTexture.textureType()), retrievedImageInfo,
    //                          imageInfo.fFormat, imageInfo.fSampleCount, imageInfo.fLevelCount,
    //                          colorType);
    //         break;
    //     }
    //     default:
    //         LOG_ALWAYS_FATAL("%s Unexpected backend %u", msg,
    //                          static_cast<unsigned>(mBackendTexture.backend()));
    //         break;
    // }
}

} // namespace skia
} // namespace renderengine
