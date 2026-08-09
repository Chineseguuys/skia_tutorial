#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#define SK_GL
#define SK_GANESH

// opengl header
#include <glad/glad.h>

#include "include/core/SkPixmap.h"
#include "include/core/SkSurfaceProps.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include "skia/include/core/SkCanvas.h"
#include "skia/include/core/SkSurface.h"
#include "skia/include/core/SkStream.h"
#include "skia/include/core/SkPictureRecorder.h"
#include "skia/include/core/SkPicture.h"
#include "skia/include/core/SkBitmap.h"
#include "skia/include/encode/SkPngEncoder.h"

#include "skia/include/core/SkTypeface.h"
#include "skia/include/core/SkFontMgr.h"
#include "skia/include/ports/SkFontMgr_empty.h"
#include "skia/include/ports/SkFontMgr_directory.h"

#include "skia/include/core/SkTextBlob.h"
#include "skia/include/core/SkData.h"

#include "skia/include/codec/SkCodec.h"
#include "skia/include/core/SkAlphaType.h"
#include "skia/include/core/SkColorType.h"
#include "skia/include/core/SkImageInfo.h"
#include "skia/include/core/SkImage.h"

#include "skia/include/core/SkTextBlob.h"
#include "skia/include/core/SkRefCnt.h"


#include "skia/include/core/SkColor.h"
// added and open the SK_DEBUG for SkRefCnt.h:166: fatal error: "assertf(rc == 1): NVRefCnt was 0"
#include "skia/include/config/SkUserConfig.h"
#include "skia/include/core/SkColorSpace.h"
#include "skia/include/core/SkPath.h"
#include "skia/include/core/SkRRect.h"
#include "skia/include/core/SkDrawable.h"
#include "skia/include/core/SkShader.h"
#include "skia/include/core/SkRect.h"
#include "skia/include/effects/SkImageFilters.h"
#include "skia/include/core/SkMatrix.h"
#include "skia/include/effects/SkRuntimeEffect.h"
#include "skia/include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "skia/include/gpu/ganesh/gl/GrGLInterface.h"
#include "skia/include/gpu/ganesh/gl/GrGLAssembleInterface.h"
#include "skia/include/gpu/ganesh/GrDirectContext.h"
#include "skia/include/gpu/ganesh/GrBackendSurface.h"
#include "skia/include/gpu/ganesh/SkSurfaceGanesh.h"
#include "skia/include/gpu/ganesh/SkImageGanesh.h"
#include "skia/include/core/SkFont.h"
#include "skia/include/core/SkImageFilter.h"
#include "skia/include/core/SkPaint.h"
#include "skia/include/core/SkScalar.h"
#include "skia/include/private/base/SkPoint_impl.h"
// #include "skia/tools/gpu/ManagedBackendTexture.h"

#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <spdlog/spdlog.h>
#include "fmt/format.h"

#include <iomanip>
#include <chrono>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <vector>

#include "Effect.h"
#include "src/core/SkAutoPixmapStorage.h"

// CLI11 的 Success 宏与 X11 冲突，X11 头文件已移除，直接包含即可
#include "CLI/CLI.hpp"

// =========================
GrBackendTexture backEndTexture;
GrBackendRenderTarget backEndRenderTarget;
GrBackendTexture backEndTextureRenderTarget;
sk_sp<SkFontMgr> fontMgr;
sk_sp<SkTypeface> typeFace;
SkBitmap source;
sk_sp<SkImage> image;
DrawOptions drawOptions;
double duration = 0.0;
double frame = 5.0;
// =========================
// sk_sp<sk_gpu_test::ManagedBackendTexture> managedBackendTexture;
// sk_sp<sk_gpu_test::ManagedBackendTexture> managedBackendTextureRenderTarget;
// =========================
static const std::vector<std::string> pngResources = {"../resources/example_1.png",
    "../resources/example_2.png",
    "../resources/example_3.png",
    "../resources/example_4.png",
    "../resources/example_5.png",
    "../resources/example_6.png",
};
// RGBA raw file
static const std::vector<std::string> rgbaRawResources = {
    "../resources/@6@layer@2010@3008x2120_bpp_1.raw",
    "../resources/@5@layer@99@3008x2120_bpp_1.raw"
};
static const std::string fontDir = "../fonts/";

extern "C" Effect* createEffect();

static void keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

std::string generate_filename(const std::string& prefix, const std::string& postfix) {
    // 获取当前系统时间
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    // 转换为本地时间（线程安全版本）
    struct std::tm time_info;
    #if defined(_WIN32)
        localtime_s(&time_info, &now_time);  // Windows 版本
    #else
        localtime_r(&now_time, &time_info);  // Linux/macOS 版本
    #endif
    // 格式化时间字符串
    std::ostringstream oss;
    oss << prefix << "_"
        << (time_info.tm_year + 1900) << "_"
        << std::setfill('0') << std::setw(2) << (time_info.tm_mon + 1) << "_"
        << std::setw(2) << time_info.tm_mday << "_"
        << std::setw(2) << time_info.tm_hour << "_"
        << std::setw(2) << time_info.tm_min << "_"
        << std::setw(2) << time_info.tm_sec << "." << postfix;
    return oss.str();
}

void saveBitmapAsPng(const SkBitmap& bitmap, const char* fileName) {
    SkFILEWStream file(fileName);
    if (!file.isValid()) {
        spdlog::error("{}: can not create file IO with name {}", __FUNCTION__, fileName);
        return;
    }
    SkPngEncoder::Options options;
    // 0 means do not compress
    options.fZLibLevel = 0;
    if (!SkPngEncoder::Encode(&file, bitmap.pixmap(), options)) {
        spdlog::error("[{}:{}] can not write bitmap to file {}", __FUNCTION__, __LINE__, fileName);
    } else {
        spdlog::info("[{}:{}] successful write bitmap to file {}", __FUNCTION__, __LINE__, fileName);
    }
}

void savePictureAsSKP(sk_sp<SkPicture> picture, const char* fileName) {
    SkFILEWStream file(fileName);
    if (!file.isValid()) {
        spdlog::error("{}: can not create file IO with name {}", __FUNCTION__, fileName);
        return;
    }
    picture->serialize(&file);
}

bool loadPngToBitmap(const char* filePath, SkBitmap& bitmap) {
    sk_sp<SkData> data = SkData::MakeFromFileName(filePath);
    if(!data) {
        spdlog::error("[{}:{}] can not open file {}", __FUNCTION__, __LINE__, filePath);
        return false;
    }

    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(data);
    if(!codec) {
        spdlog::error("[{}:{}] can not create codec for png!", __FUNCTION__, __LINE__);
        return false;
    }
    SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType).makeAlphaType(kPremul_SkAlphaType);
    if(!bitmap.tryAllocPixels(info)) {
        spdlog::error("[{}:{}] can not alloc pixels for bitmap!", __FUNCTION__, __LINE__);
        return false;
    }

    SkCodec::Result result = codec->getPixels(info, bitmap.getPixels(), bitmap.rowBytes());
    image = bitmap.asImage();
    spdlog::info("[{}:{}]get pixels from file {} with result {}", __FUNCTION__, __LINE__, filePath, result == SkCodec::kSuccess);
    return (result == SkCodec::kSuccess);
}

/**
 * 读取 RGBA RAW 文件到 uint32_t 数组
 * @param fileName 输入文件路径
 * @param width    图像宽度（像素）
 * @param height   图像高度（像素）
 * @param rawData 输出像素数组指针（需外部释放）
 * @return         是否成功读取
 */
bool loadRGBARawFile(const char* fileName, int width, int height, uint32_t** rawData) {
    const size_t expectedSize = width * height * 4;

    FILE* file = fopen(fileName, "rb");
    if (!file) {
        spdlog::error("[{}:{}] open file \"{}\" failed!", __FUNCTION__, __LINE__, fileName);
        return false;
    }

    fseek(file, 0, SEEK_END);
    const long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize != static_cast<long>(expectedSize)) {
        spdlog::error("[{}:{}] file size is not equal with expected size! Get {}, expected {}", __FUNCTION__, __LINE__, fileSize, expectedSize);
        fclose(file);
        return false;
    }

    uint32_t* byteBuffer = new uint32_t[width * height];
    if (!byteBuffer) {
        spdlog::error("[{}:{}] can not alloc memory for raw file!", __FUNCTION__, __LINE__);
        fclose(file);
        return false;
    }

    if (int readed = fread(byteBuffer, 1, expectedSize, file) != expectedSize) {
        spdlog::info("[{}:{}] we need read {} bytes, but read {} acturally!", __FUNCTION__, __LINE__, expectedSize, readed);
        delete [] byteBuffer;
        fclose(file);
        return false;
    }

    // swap R and B
    for (int i = 0; i < width * height; ++i) {
        uint32_t value = byteBuffer[i];
        // ABGR -> ARGB
        byteBuffer[i] = ((value & 0x00ff0000) >> 16) | ((value & 0x000000ff) << 16) | (value & 0xff000000) | (value & 0x0000ff00);
    }

    *rawData = byteBuffer;
    spdlog::info("[{}:{}] successful read pixels from file {} with address {}", __FUNCTION__, __LINE__, fileName, fmt::ptr(byteBuffer));
    fclose(file);
    return true;
}

static void releaseProc(void* addr, void* ) {
    spdlog::info("releaseProc called\n");
    delete[] (uint32_t*) addr;
}

/**
 * 打印 SkMatrix 矩阵的详细信息
 * 
 * @param matrix 要打印的 SkMatrix 对象
 * @param name   矩阵名称（可选），用于标识输出
 * @param precision 浮点数打印精度（小数位数）
 */
void printSkMatrix(const SkMatrix& matrix, const char* name = "SkMatrix", int precision = 2) {
    // 设置浮点数格式
    char format[16];
    snprintf(format, sizeof(format), "%%.%df", precision);

    // 打印标题
    if (name && *name) {
        printf("\n%s:\n", name);
    } else {
        printf("\nSkMatrix:\n");
    }

    // 获取矩阵元素（使用行主序表示）
    SkScalar buffer[9];
    matrix.get9(buffer);

    // 打印矩阵内容
    printf("┌ "); printf(format, buffer[0]); printf("    "); 
              printf(format, buffer[1]); printf("    "); 
              printf(format, buffer[2]); printf(" ┐\n");

    printf("│ "); printf(format, buffer[3]); printf("    "); 
              printf(format, buffer[4]); printf("    "); 
              printf(format, buffer[5]); printf(" │\n");

    printf("└ "); printf(format, buffer[6]); printf("    "); 
              printf(format, buffer[7]); printf("    "); 
              printf(format, buffer[8]); printf(" ┘\n");

    return;
}

static bool setupBackendObjects(GrDirectContext* dContext, const SkBitmap& bm, const DrawOptions& op) {
    if (!dContext) {
        spdlog::error("[{}:{}] can not get direct context!", __FUNCTION__, __LINE__);
        return false;
    }
    GrBackendFormat renderableFormat = dContext->defaultBackendFormat(kRGBA_8888_SkColorType, GrRenderable::kYes);
    SkBitmap rgbaBitmap;

    if (!bm.empty()) {
        spdlog::trace("[{}:{}] setup backend objects with bitmap!", __FUNCTION__, __LINE__);
        SkPixmap originalPixmap;
        const SkPixmap* pixmap = &originalPixmap;
        if (!bm.peekPixels(&originalPixmap)) {
            spdlog::error("[{}:{}] can not peek pixels from bitmap!", __FUNCTION__, __LINE__);
            return false;
        }
        constexpr bool kRRGAISNative = (kN32_SkColorType == kRGBA_8888_SkColorType);
        if (!kRRGAISNative) {
            if (!rgbaBitmap.tryAllocPixels(bm.info().makeColorType(kRGBA_8888_SkColorType))) {
                spdlog::error("[{}:{}] can not alloc memory for rgba pixmap!", __FUNCTION__, __LINE__);
                return false;
            }
            if (!bm.readPixels(rgbaBitmap.pixmap())) {
                spdlog::error("[{}:{}] can not read pixels from bitmap!", __FUNCTION__, __LINE__);
                return false;
            }

            pixmap = &rgbaBitmap.pixmap();
        }
        // skia private api
        // managedBackendTexture = sk_gpu_test::ManagedBackendTexture::MakeFromPixmap(
        //     dContext, *pixmap, drawOptions.fMipMapping, GrRenderable::kYes, GrProtected::kNo);
        // if (!managedBackendTexture) {
        //     spdlog::error("{}: can not create managed backend texture!", __FUNCTION__);
        //     return false;
        // }
        // backEndTexture = managedBackendTexture->texture();

        // use gl
        GLuint texID;
        glGenTextures(1, &texID);
        spdlog::info("[{}:{}] create texture with id {}", __FUNCTION__, __LINE__, texID);
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 
            0, 
            GL_RGBA, 
            pixmap->width(), pixmap->height(), 0, 
            GL_RGBA, 
            GL_UNSIGNED_BYTE, 
            pixmap->addr()
        );
        glBindTexture(GL_TEXTURE_2D, 0);

        // use skia
        GrGLTextureInfo glTexInfo{.fTarget = GL_TEXTURE_2D, .fID = texID, .fFormat = GL_RGBA8};
        backEndTexture = GrBackendTextures::MakeGL(
            pixmap->width(), pixmap->height(), 
            op.fMipMapping, 
            glTexInfo
        );
        if (!backEndTexture.isValid()) {
            spdlog::error("[{}:{}] can not create backend texture!", __FUNCTION__, __LINE__);
            return false;
        }
    } // end stage 1
    
    {
        // not public api
        // auto resourceProvider = dContext->priv().resourceProvider();
        // SkISize offscreenDims = {op.offScreenWidth, op.offScreenHeight};
        // skia_private::AutoTMalloc<uint32_t> data(offscreenDims.area());
        // SkOpts::memset32(data.get(), 0, offscreenDims.area());

        // GrMipLevel level0 = {data.get(), offscreenDims.width() * sizeof(uint32_t), nullptr};
        // constexpr int kSampleCnt = 1;
        // // in skia private, it may use as this
        // sk_sp<GrTexture> tmp =
        //         resourceProvider->createTexture(offscreenDims,
        //                                         renderableFormat,
        //                                         GrTextureType::k2D,
        //                                         GrColorType::kRGBA_8888,
        //                                         GrRenderable::kYes,
        //                                         kSampleCnt,
        //                                         skgpu::Budgeted::kNo,
        //                                         skgpu::Mipmapped::kNo,
        //                                         GrProtected::kNo,
        //                                         &level0,
        //                                         /*label=*/"Fiddle_SetupBackendObjects");
        // if (!tmp || !tmp->asRenderTarget()) {
        //     fputs("GrTexture is invalid.\n", stderr);
        //     return false;
        // }

        // backingRenderTarget = sk_ref_sp(tmp->asRenderTarget());
        spdlog::trace("[{}:{}] create offscreen texture by use gl!", __FUNCTION__, __LINE__);
        // ==== step1 create texture by use gl =====
        GLuint texID;
        glGenTextures(1, &texID);
        spdlog::info("[{}:{}] create texture with id {}", __FUNCTION__, __LINE__, texID);
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 
            0, 
            GL_RGBA, 
            op.offScreenWidth, 
            op.offScreenHeight, 0, 
            GL_RGBA, 
            GL_UNSIGNED_BYTE, 
            nullptr
        );

        GLuint fboID;
        glGenFramebuffers(1, &fboID);
        spdlog::info("[{}:{}] create framebuffer with id {}", __FUNCTION__, __LINE__, fboID);
        glBindFramebuffer(GL_FRAMEBUFFER, fboID);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texID, 0);
        
        // use skia
        SkISize offscreenDims = {op.offScreenWidth, op.offScreenHeight};
        GrGLFramebufferInfo glInfo{.fFBOID = fboID, .fFormat = GL_RGBA8};
        backEndRenderTarget = GrBackendRenderTargets::MakeGL(
            offscreenDims.width(), offscreenDims.height(), 
            1, 8, glInfo);
        if (!backEndRenderTarget.isValid()) {
            spdlog::error("[{}:{}] can not create backend render target!", __FUNCTION__, __LINE__);
            return false;
        }
        
    } // end stage 2

    {
        // use skia private api
        // managedBackendTextureRenderTarget = sk_gpu_test::ManagedBackendTexture::MakeWithData(
        //     dContext,
        //     op.offScreenWidth,
        //     op.offScreenHeight,
        //     renderableFormat,
        //     SkColors::kTransparent,
        //     op.fOffScreenMipMapping,
        //     GrRenderable::kYes,
        //     GrProtected::kNo);
        // if (!managedBackendTextureRenderTarget) {
        //     spdlog::error("{}: can not create backend texture render target!", __FUNCTION__);
        //     return false;
        // }
        // backEndTextureRenderTarget = managedBackendTextureRenderTarget->texture();
        spdlog::trace("[{}:{}] create offscreen texture render target by use gl!", __FUNCTION__, __LINE__);
        // use gl
        int pixelCnt = op.offScreenWidth * op.offScreenHeight;
        std::vector<uint32_t> pixels(pixelCnt, 0);
        GLuint texID;
        glGenTextures(1, &texID);
        spdlog::debug("{}: create texture with id {}", __FUNCTION__, texID);
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            op.offScreenWidth,
            op.offScreenHeight,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            pixels.data()
        );
        glBindTexture(GL_TEXTURE_2D, 0);

        // use skia
        GrGLTextureInfo glTexInfo{.fTarget = GL_TEXTURE_2D, .fID = texID, .fFormat = GL_RGBA8};
        backEndTextureRenderTarget = GrBackendTextures::MakeGL(
            op.offScreenWidth, op.offScreenHeight, 
            op.fOffScreenMipMapping, 
            glTexInfo
        );
        if (!backEndTextureRenderTarget.isValid()) {
            spdlog::error("{}: can not create backend texture render target!", __FUNCTION__);
            return false;
        }
    } // end stage 3

    return true;
}

class CuteLittleFibonacciSphereEffect {
public:
    static constexpr const char* skShaderCode = R"(
        uniform float2 iResolution;  // 画布分辨率
        uniform float iTime;         // 时间变量

        half4 main(float2 FC) {
            vec4 o = vec4(0);
            vec2 p = vec2(0), c=p, u=FC.xy*2.-iResolution.xy;
            float a;
            for (float i=0; i<4e2; i++) {
                a = i/2e2-1.;
                p = cos(i*2.4+iTime+vec2(0,11))*sqrt(1.-a*a);
                c = u/iResolution.y+vec2(p.x,a)/(p.y+2.);
                o += (cos(i+vec4(0,2,4,0))+1.)/dot(c,c)*(1.-p.y)/3e4;
            }
            return o;
        }
    )";

    struct Uniform {
        std::string sName;
        std::vector<uint8_t> sData;
    };

    CuteLittleFibonacciSphereEffect() = default;
    ~CuteLittleFibonacciSphereEffect() = default;

    void initlize(int width, int height) {
        mCanvasWidth = width;
        mCanvasHeight = height;
    }

    void draw(SkCanvas* canvas, float time) {
        // Implementation similar to PlusingCircleEffect
        SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForShader(SkString(skShaderCode));
        if (!result.effect) {
            spdlog::error("{}: can not create runtime effect!", __FUNCTION__);
            return;
        }

        Uniform uniform;
        uniform.sName = "iTime";
        uniform.sData.resize(sizeof(float));
        std::memcpy(uniform.sData.data(), &time, sizeof(float));

        SkPoint resolution = SkPoint::Make((float)mCanvasWidth, (float)mCanvasHeight);
        Uniform resolutionUniform;
        resolutionUniform.sName = "iResolution";
        resolutionUniform.sData.resize(sizeof(SkPoint));
        std::memcpy(resolutionUniform.sData.data(), &resolution, sizeof(SkPoint));
        SkRuntimeShaderBuilder shaderBuilder(result.effect);
        shaderBuilder.uniform(uniform.sName.c_str())
            .set(uniform.sData.data(), uniform.sData.size());
        shaderBuilder.uniform(resolutionUniform.sName.c_str())
            .set(resolutionUniform.sData.data(), resolutionUniform.sData.size());
        
        sk_sp<SkShader> shader = shaderBuilder.makeShader(nullptr);
        if (!shader) {
            spdlog::error("{}: can not create runtime shader!", __FUNCTION__);
            return; 
        }
        SkPaint paint;
        paint.setShader(shader);
        canvas->drawRect(SkRect::MakeWH(mCanvasWidth, mCanvasHeight), paint);
    }
private:
    int mCanvasWidth = 0;
    int mCanvasHeight = 0;
};

class KaleidoscopeEffect {
public:
    static constexpr const char* skShaderCode = R"(
        uniform float2 iResolution;      // Viewport resolution (pixels)
        uniform float  iTime;            // Shader playback time (s)

        mat2 rotate2D(float r){
            return mat2(cos(r), sin(r), -sin(r), cos(r));
        }

        mat3 rotate3D(float angle, vec3 axis){
            vec3 a = normalize(axis);
            float s = sin(angle);
            float c = cos(angle);
            float r = 1.0 - c;
            return mat3(
                a.x * a.x * r + c,
                a.y * a.x * r + a.z * s,
                a.z * a.x * r - a.y * s,
                a.x * a.y * r - a.z * s,
                a.y * a.y * r + c,
                a.z * a.y * r + a.x * s,
                a.x * a.z * r + a.y * s,
                a.y * a.z * r - a.x * s,
                a.z * a.z * r + c
            );
        }

        half4 main(float2 FC) {
        vec4 o = vec4(0);
        vec2 r = iResolution.xy;
        vec3 v = vec3(1,3,7), p = vec3(0);
        float t=iTime, n=0, e=0, g=0, k=t*.2;
        for (float i=0; i<100; ++i) {
            p = vec3((FC.xy-r*.5)/r.y*g,g)*rotate3D(k,cos(k+v));
            p.z += t;
            p = asin(sin(p)) - 3.;
            n = 0;
            for (float j=0; j<9.; ++j) {
            p.xz *= rotate2D(g/8.);
            p = abs(p);
            p = p.x<p.y ? n++, p.zxy : p.zyx;
            p += p-v;
            }
            g += e = max(p.x,p.z) / 1e3 - .01;
            o.rgb += .1/exp(cos(v*g*.1+n)+3.+1e4*e);
        }
        return o.xyz1;
        }
    )";

    struct Uniform {
        std::string sName;
        std::vector<uint8_t> sData;
    };

    void initlize(int width, int height) {
        mCanvasWidth = width;
        mCanvasHeight = height;
    }

    void draw(SkCanvas* canvas, float time) {
        // Implementation similar to PlusingCircleEffect
        SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForShader(SkString(skShaderCode));
        if (!result.effect) {
            spdlog::error("{}: can not create runtime effect!", __FUNCTION__);
            return;
        }

        Uniform uniform;
        uniform.sName = "iTime";
        uniform.sData.resize(sizeof(float));
        std::memcpy(uniform.sData.data(), &time, sizeof(float));

        SkPoint resolution = SkPoint::Make((float)mCanvasWidth, (float)mCanvasHeight);
        Uniform resolutionUniform;
        resolutionUniform.sName = "iResolution";
        resolutionUniform.sData.resize(sizeof(SkPoint));
        std::memcpy(resolutionUniform.sData.data(), &resolution, sizeof(SkPoint));
        SkRuntimeShaderBuilder shaderBuilder(result.effect);
        shaderBuilder.uniform(uniform.sName.c_str())
            .set(uniform.sData.data(), uniform.sData.size());
        shaderBuilder.uniform(resolutionUniform.sName.c_str())
            .set(resolutionUniform.sData.data(), resolutionUniform.sData.size());
        
        sk_sp<SkShader> shader = shaderBuilder.makeShader(nullptr);
        if (!shader) {
            spdlog::error("{}: can not create runtime shader!", __FUNCTION__);
            return; 
        }
        SkPaint paint;
        paint.setShader(shader);
        canvas->drawRect(SkRect::MakeWH(mCanvasWidth, mCanvasHeight), paint);
    }

private:
    int mCanvasWidth = 0;
    int mCanvasHeight = 0;
};

// 分形
class FractalEffect {
public:
    static constexpr const char* skShaderCode = R"(
        uniform float2 iResolution;      // Viewport resolution (pixels)
        uniform float  iTime;            // Shader playback time (s)

        float julia(vec2 uv, vec2 c) {
            const float maxSteps = 400;
            for (float i = 0; i < maxSteps; i++) {
                uv = vec2(uv.x * uv.x - uv.y * uv.y + c.x,
                        2.0 * uv.x * uv.y + c.y);
                if (length(uv) > 2) {
                    return i / maxSteps;
                }
            }
            return 1.0;
        }


        vec4 main( in vec2 fragCoord )
        {
            // Normalized pixel coordinates (from 0 to 1)
            vec2 uv = -1.0 + 2.0 * fragCoord / iResolution.xy;

            float aspect = iResolution.x / iResolution.y;
            uv.x *= aspect;

            uv *= pow(0.5, -1.0 + 15.0 * (0.5 + 0.5 * sin(iTime * 0.80 - (3.14159265))));
            uv += vec2(-0.51, -0.61351); // an interesting coordinate to zoom in on 
            float f = julia(vec2(0.0, 0.0), uv);
            
            // Output to screen
            return vec4((1.0 - uv) * pow(f, 0.5), f, 1.0);
        }
    )";

    struct Uniform {
        std::string sName;
        std::vector<uint8_t> sData;
    };

    void initlize(int width, int height) {
        mCanvasWidth = width;
        mCanvasHeight = height;
    }

    void draw(SkCanvas* canvas, float time) {
        // Implementation similar to PlusingCircleEffect
        SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForShader(SkString(skShaderCode));
        if (!result.effect) {
            spdlog::error("{}: can not create runtime effect!", __FUNCTION__);
            return;
        }

        Uniform uniform;
        uniform.sName = "iTime";
        uniform.sData.resize(sizeof(float));
        std::memcpy(uniform.sData.data(), &time, sizeof(float));

        SkPoint resolution = SkPoint::Make((float)mCanvasWidth, (float)mCanvasHeight);
        Uniform resolutionUniform;
        resolutionUniform.sName = "iResolution";
        resolutionUniform.sData.resize(sizeof(SkPoint));
        std::memcpy(resolutionUniform.sData.data(), &resolution, sizeof(SkPoint));
        SkRuntimeShaderBuilder shaderBuilder(result.effect);
        shaderBuilder.uniform(uniform.sName.c_str())
            .set(uniform.sData.data(), uniform.sData.size());
        shaderBuilder.uniform(resolutionUniform.sName.c_str())
            .set(resolutionUniform.sData.data(), resolutionUniform.sData.size());

        sk_sp<SkShader> shader = shaderBuilder.makeShader(nullptr);
        if (!shader) {
            spdlog::error("{}: can not create runtime shader!", __FUNCTION__);
            return; 
        }
        SkPaint paint;
        paint.setShader(shader);
        canvas->drawRect(SkRect::MakeWH(mCanvasWidth, mCanvasHeight), paint);
    }

private:
    int mCanvasWidth = 0;
    int mCanvasHeight = 0;
};

int main(int argc, char* argv[]) {
#if 1
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%P:%t][%Y-%m-%d %H:%M:%S.%e] [%^%-8l%$] %v");
#endif

    // args parser
    CLI::App app{"Skia Fiddle"};
    app.add_option("-W,--width", drawOptions.size.fWidth, "canvas draw width")
        ->check(CLI::Range(16, 1024))
        ->default_val(256);
    app.add_option("-H,--height", drawOptions.size.fHeight, "canvas draw height")
        ->check(CLI::Range(16, 8056))
        ->default_val(256);
    app.add_option("-S,--save", drawOptions.saveRender, "save canvas draw to png file")
        ->check(CLI::IsMember({0, 1}))
        ->default_val(0);
    app.add_option("-D,--display", drawOptions.display, "display canvas draw to glfw window")
        ->check(CLI::IsMember({0, 1}))
        ->default_val(0);
    app.add_option("-R,--resource", drawOptions.sourceIndex, "resource id for program loading image")
        ->check(CLI::Range(0, 5))
        ->default_val(2);
    app.add_option("-P,--picture", drawOptions.skp, "Save Canvas draw to skp file")
        ->check(CLI::IsMember({0, 1}))
        ->default_val(0);
    app.add_option("-A,--alpha", drawOptions.alphaType, "alpha type")
        ->check(CLI::Range(0, 3))
        ->default_val(SkAlphaType::kPremul_SkAlphaType);
    app.add_option("-M,--mipmapping", drawOptions.fMipMapping, "Backend Texutre Mipmapping")
        ->check(CLI::IsMember({0, 1}))
        ->default_val(skgpu::Mipmapped::kNo);
    app.add_option("-F,--offscreenmipmapping", drawOptions.fOffScreenMipMapping, "Offscreen Texture Mipmapping")
        ->check(CLI::IsMember({0, 1}))
        ->default_val(skgpu::Mipmapped::kNo);
    app.add_option("-X,--offscreenwidth", drawOptions.offScreenWidth, "Offscreen Texture Width")
        ->check(CLI::Range(16, 1024))
        ->default_val(128);
    app.add_option("-Y,--offscreenheight", drawOptions.offScreenHeight, "Offscreen Texture Height")
        ->check(CLI::Range(16, 1024))
        ->default_val(128);

    //catch exception and parse the command lines
    CLI11_PARSE(app, argc, argv);

    if (drawOptions.display && drawOptions.saveRender) {
        // 显示模式不保存 PNG（渲染循环内每帧产出，保存会破坏帧循环语义）
        spdlog::warn("[{}:{}] save png is disabled in display mode!", __FUNCTION__, __LINE__);
        drawOptions.saveRender = false;
    }

    spdlog::info("[{}:{}] Canvas: [wxh]=[{}x{}]", __FUNCTION__, __LINE__, drawOptions.size.fWidth, drawOptions.size.fHeight);

    fontMgr = SkFontMgr_New_Custom_Directory(fontDir.c_str());
    if (fontMgr == nullptr) {
        spdlog::error("[{}:{}] can not create font manager!", __FUNCTION__, __LINE__);
    }

    int fontFamilyCounts = fontMgr->countFamilies();
    spdlog::debug("{}: font family count is {}", __FUNCTION__, fontFamilyCounts);
    for (int idx = 0; idx < fontFamilyCounts; ++idx) {
        SkString fontFamilyName;
        fontMgr->getFamilyName(idx, &fontFamilyName);
        spdlog::debug("[{}:{}] Family[{}]={}", __FUNCTION__, __LINE__, idx, fontFamilyName.c_str());
    }

    const char* fontFamily = nullptr;
    SkFontStyle fontStyle;
    // matchFamilyStyle 没有 fallback 机制，传 nullptr 就是空。
    // legacyMakeTypeface 是旧版 API，带默认字体回退逻辑。
    typeFace = fontMgr->legacyMakeTypeface(fontFamily, fontStyle);
    if (typeFace == nullptr) {
        spdlog::error("[{}:{}] can not create type face from font manager!", __FUNCTION__, __LINE__);
    }

    SkImageInfo imageInfo = SkImageInfo::Make(
        drawOptions.size.fWidth, drawOptions.size.fHeight,
        kBGRA_8888_SkColorType,
        static_cast<SkAlphaType>(drawOptions.alphaType));

    // --------------- begin init glfw context ------------
    if (!glfwInit()) {
        spdlog::critical("[{}:{}] Failed to initialize GLFW", __FUNCTION__, __LINE__);
        return -1;
    }
    // 离屏模式窗口保持隐藏，显示模式可见
    if (!drawOptions.display) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(
        drawOptions.size.fWidth, drawOptions.size.fHeight,
        drawOptions.display ? "Skia Display" : "offscreen", nullptr, nullptr
    );
    if (!window) {
        spdlog::critical("[{}:{}] Failed to create GLFW window", __FUNCTION__, __LINE__);
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (drawOptions.display) {
        glfwSetKeyCallback(window, keyCallback);
    }
    // ----------------end init glfw context ------------

    // --------------- begin init glad ------------
#ifdef SK_GL
    // add for use opengl api it must add after init glfw context and before init ganesh
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        spdlog::critical("[{}:{}] Failed to Initialize GLAD", __FUNCTION__, __LINE__);
        return -1;
    }
#endif // SK_GL
    // ----------------end init glad ------------

    // --------------- begin init ganesh ------------
    // 显式用 glfwGetProcAddress 组装 GL 接口，避免依赖 GLX 后端（Wayland 会话下 GLFW 走 EGL）
    sk_sp<const GrGLInterface> glInterface = GrGLMakeAssembledInterface(
        nullptr,
        [](void*, const char name[]) {
            return reinterpret_cast<GrGLFuncPtr>(glfwGetProcAddress(name));
        }
    );
    if (!glInterface) {
        spdlog::critical("[{}:{}] can not assemble gl interface!", __FUNCTION__, __LINE__);
        return -1;
    }
    if (!glInterface->validate()) {
        spdlog::critical("[{}:{}] gl interface is not valid!", __FUNCTION__, __LINE__);
        return -1;
    }
    sk_sp<GrDirectContext> grContext = GrDirectContexts::MakeGL(glInterface);
    if (!grContext) {
        spdlog::error("{}: can not create GrDirectContext!", __FUNCTION__);
        return -1;
    }
    // ----------------end init ganesh ------------

    // --------------- begin create sk surface ------------
    GrBackendTexture backendTexture = grContext->createBackendTexture(
        drawOptions.size.fWidth,
        drawOptions.size.fHeight,
        imageInfo.colorType(),
        SkColors::kTransparent,
        skgpu::Mipmapped::kNo,
        GrRenderable::kYes,
        GrProtected::kNo,
        nullptr,
        nullptr,
        "Offscreen Texture"
    ); 
    if (!backendTexture.isValid()) {
        spdlog::error("[{}:{}] can not create backend texture!", __FUNCTION__, __LINE__);
        return -1;
    }

    SkSurfaceProps surfaceProps;
    sk_sp<SkSurface> skSurface = SkSurfaces::WrapBackendTexture(
        grContext.get(),
        backendTexture,
        kTopLeft_GrSurfaceOrigin,
        0,
        imageInfo.colorType(),
        imageInfo.refColorSpace(),
        &surfaceProps
    );
    if (!skSurface) {
        spdlog::error("[{}:{}] can not create sk surface from backend texture!", __FUNCTION__, __LINE__);
        return -1;
    }
    // ----------------end create sk surface ------------

    // --------------- begin init display surface ------------
    sk_sp<SkSurface> windowSurface = nullptr;
    sk_sp<SkImage> offscreenImage = nullptr;
    if (drawOptions.display) {
        // 窗口默认 framebuffer (FBO 0)，GL 惯例 origin 为 BottomLeft
        GrGLFramebufferInfo fbInfo = { .fFBOID = 0, .fFormat = GL_RGBA8 };
        GrBackendRenderTarget renderTarget = GrBackendRenderTargets::MakeGL(
            drawOptions.size.fWidth, drawOptions.size.fHeight, 0, 0, fbInfo
        );
        if (!renderTarget.isValid()) {
            spdlog::critical("[{}:{}] can not create backend render target for display!", __FUNCTION__, __LINE__);
            return -1;
        }
        windowSurface = SkSurfaces::WrapBackendRenderTarget(
            grContext.get(),
            renderTarget,
            kBottomLeft_GrSurfaceOrigin,
            imageInfo.colorType(),
            imageInfo.refColorSpace(),
            &surfaceProps
        );
        if (!windowSurface) {
            spdlog::critical("[{}:{}] can not create window surface!", __FUNCTION__, __LINE__);
            return -1;
        }
        // 离屏 texture 包成 SkImage，每帧仅引用、不重建
        offscreenImage = SkImages::AdoptTextureFrom(
            grContext.get(),
            backendTexture,
            kTopLeft_GrSurfaceOrigin,
            imageInfo.colorType(),
            imageInfo.alphaType(),
            imageInfo.refColorSpace()
        );
        if (!offscreenImage) {
            spdlog::critical("[{}:{}] can not adopt offscreen texture to image!", __FUNCTION__, __LINE__);
            return -1;
        }
    }
    // ----------------end init display surface ------------

    SkCanvas* canvas = skSurface->getCanvas();

    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(drawOptions.size.fWidth, drawOptions.size.fHeight);
    if (drawOptions.skp) {
        if (drawOptions.display) {
            // recorder 录制画布不支持逐帧循环，显示模式下忽略
            spdlog::warn("[{}:{}] skp is not supported in display mode, ignore it!", __FUNCTION__, __LINE__);
        } else {
            spdlog::debug("[{}:{}] replace canvas with recording canvas!", __FUNCTION__, __LINE__);
            canvas = recordingCanvas;
        }
    }

    canvas->drawColor(SK_ColorTRANSPARENT);
    // -------------- begin load image from png file -------
    loadPngToBitmap(pngResources[drawOptions.sourceIndex].c_str(), source);
    if (!setupBackendObjects(grContext.get(), source, drawOptions)) {
        spdlog::error("[{}:{}] can not setup backend objects!", __FUNCTION__, __LINE__);
        return -1;
    }
    // -------------- end load image from png file -------

    // --------------- begin draw commands ----------------
    std::shared_ptr<Effect> effect{createEffect()};
    effect->initialize(drawOptions.size.fWidth, drawOptions.size.fHeight);

    if (drawOptions.display)  {
        // --------------- begin display render loop ----------------
        spdlog::info("[{}:{}] begin display loop, press ESC or close window to exit", __FUNCTION__, __LINE__);
        while (!glfwWindowShouldClose(window)) {
            // 子过程 1：skia 渲染到离屏 texture（原逻辑，仅清屏改为每帧执行）
            canvas->drawColor(SK_ColorTRANSPARENT);
            effect->draw(canvas);
            grContext->flushAndSubmit(GrSyncCpu::kNo);

            // 子过程 2：离屏 texture 贴到窗口 framebuffer
            SkCanvas* winCanvas = windowSurface->getCanvas();
            winCanvas->drawImage(offscreenImage, 0, 0);
            skgpu::ganesh::FlushAndSubmit(windowSurface.get());

            // 子过程 3：呈现并处理事件
            glfwSwapBuffers(window);
            glfwPollEvents();
            frame += 1.0;
        }
        // ----------------end display render loop ----------------
    } else {
        // 离屏路径：一次性渲染后保存
        effect->draw(canvas);
        // 将命令提交到 GPU 执行，确保所有绘制操作完成
        grContext->flushAndSubmit(GrSyncCpu::kYes);

        if (drawOptions.saveRender) {
            // 保存当前离屏帧为 PNG（调用前需已 flush）
            SkBitmap bitmap;
            bitmap.allocPixels(imageInfo, imageInfo.minRowBytes());
            skSurface->readPixels(bitmap, 0, 0);

            std::string pngName = generate_filename("output", "png");
            saveBitmapAsPng(bitmap, pngName.c_str());
        }
    }
    // --------------- end draw commands ------------------

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}