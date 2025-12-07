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
#include "skia/include/gpu/ganesh/GrDirectContext.h"
#include "skia/include/gpu/ganesh/GrBackendSurface.h"
#include "skia/include/gpu/ganesh/SkSurfaceGanesh.h"
#include "skia/include/core/SkFont.h"
#include "skia/include/core/SkImageFilter.h"
#include "skia/include/core/SkPaint.h"
#include "skia/include/core/SkScalar.h"
#include "skia/include/private/base/SkPoint_impl.h"

// added for Xlib.h
#include <X11/Xlib.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <X11/X.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <spdlog/spdlog.h>
#include "fmt/format.h"


#include "GLFW/glfw3.h"

#include <iomanip>
#include <chrono>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <vector>

// modified for compile error
#ifdef Success
#undef Success
#include "CLI/CLI.hpp"
#endif

#include "backward.hpp"

static sk_sp<SkFontMgr> fontMgr;
static sk_sp<SkTypeface> typeFace;
static SkBitmap source;
static sk_sp<SkImage> image;
static int DRAW_WIDTH = 256;
static int DRAW_HEIGHT = 256;
static int RESOURCE_ID = 3;
static bool SAVE_BITMAP = false;
static bool SAVE_SKP = false;
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

/* This struct is taken from a mesa demo.  Please update as required */
static const std::vector<std::pair<int, int>> gl_versions = {
   {1, 0},
   {1, 1},
   {1, 2},
   {1, 3},
   {1, 4},
   {1, 5},
   {2, 0},
   {2, 1},
   {3, 0},
   {3, 1},
   {3, 2},
   {3, 3},
   {4, 0},
   {4, 1},
   {4, 2},
   {4, 3},
   {4, 4},
};

static const std::vector<std::pair<int, int>> gles_versions = {
    {2, 0},
    {3, 0},
};

static bool ctxErrorOccurred = false;
static int ctxErrorHandler(Display *dpy, XErrorEvent *ev) {
    ctxErrorOccurred = true;
    return 0;
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
        spdlog::error("{}: can not write bitmap to file {}", __FUNCTION__, fileName);
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
        spdlog::error("{}: can not open file {}", __func__, filePath);
        return false;
    }

    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(data);
    if(!codec) {
        spdlog::error("{}: can not create codec for png!", __func__);
        return false;
    }
    SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType).makeAlphaType(kPremul_SkAlphaType);
    if(!bitmap.tryAllocPixels(info)) {
        spdlog::error("{}: can not alloc pixels for bitmap!", __func__);
        return false;
    }

    SkCodec::Result result = codec->getPixels(info, bitmap.getPixels(), bitmap.rowBytes());
    image = bitmap.asImage();
    spdlog::info("{}: get pixels from file {} with result {}", __func__, filePath, result == SkCodec::kSuccess);
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
        spdlog::error("{}: open file \"{}\" failed!", __func__, fileName);
        return false;
    }

    fseek(file, 0, SEEK_END);
    const long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize != static_cast<long>(expectedSize)) {
        spdlog::error("{}: file size is not equal with expected size! Get {}, expected {}", __func__, fileSize, expectedSize);
        fclose(file);
        return false;
    }

    uint32_t* byteBuffer = new uint32_t[width * height];
    if (!byteBuffer) {
        spdlog::error("{}: can not alloc memory for raw file!", __func__);
        fclose(file);
        return false;
    }

    if (int readed = fread(byteBuffer, 1, expectedSize, file) != expectedSize) {
        spdlog::info("{}: we need read {} bytes, but read {} acturally!", __func__, expectedSize, readed);
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
    spdlog::info("{}: successful read pixels from file {} with address {}", __func__, fileName, fmt::ptr(byteBuffer));
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

static Display* get_display() {
    class AutoDisplay {
    public:
        AutoDisplay() { fDisplay = XOpenDisplay(nullptr); }
        ~AutoDisplay() {
            if (fDisplay) {
                XCloseDisplay(fDisplay);
            }
        }
        Display* display() const { return fDisplay; }
    private:
        Display* fDisplay;
    };
    static std::unique_ptr<AutoDisplay> ad;
    static SkOnce once;
    once([] { ad = std::make_unique<AutoDisplay>(); });
    return ad->display();
}

GLXContext CreateBestContext(bool isES, Display* display, GLXFBConfig bestFbc,
                                               GLXContext glxShareContext) {
    auto glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
        glXGetProcAddressARB((const GrGLubyte*)"glXCreateContextAttribsARB");
    if (!glXCreateContextAttribsARB) {
        SkDebugf("Failed to get address of glXCreateContextAttribsARB");
        return nullptr;
    }
    GLXContext context = nullptr;
    // Install Xlib error handler that will set ctxErrorOccurred.
    // WARNING: It is global for all threads.
    ctxErrorOccurred = false;
    int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);

    auto versions = isES ? gles_versions : gl_versions;
    // Well, unfortunately GLX will not just give us the highest context so
    // instead we have to do this nastiness
    for (int i = versions.size() - 1; i >= 0 ; i--) {
        // WARNING: Don't try to optimize this and make this array static. The
        // glXCreateContextAttribsARB call writes to it upon failure and the
        // next call would fail too.
        std::vector<int> flags = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, versions[i].first,
            GLX_CONTEXT_MINOR_VERSION_ARB, versions[i].second,
        };
        if (isES) {
            flags.push_back(GLX_CONTEXT_PROFILE_MASK_ARB);
            // the ES2 flag should work even for higher versions
            flags.push_back(GLX_CONTEXT_ES2_PROFILE_BIT_EXT);
        } else if (versions[i].first > 2) {
            flags.push_back(GLX_CONTEXT_PROFILE_MASK_ARB);
            flags.push_back(GLX_CONTEXT_CORE_PROFILE_BIT_ARB);
        }
        flags.push_back(0);
        context = glXCreateContextAttribsARB(display, bestFbc, glxShareContext, true,
                                             &flags[0]);
        // Sync to ensure any errors generated are processed.
        XSync(display, False);

        if (!ctxErrorOccurred && context) {
            break;
        }
        // try again
        ctxErrorOccurred = false;
    }
    // Restore the original error handler.
    XSetErrorHandler(oldHandler);
    return context;
}

int main(int argc, char* argv[]) {
#if 1
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%P:%t][%Y-%m-%d %H:%M:%S.%e] [%^%-8l%$] %v");
#endif

    // args parser
    CLI::App app{"Skia Fiddle"};
    app.add_option("-W,--width", DRAW_WIDTH, "canvas draw width")
        ->check(CLI::Range(16, 1024))
        ->default_val(256);
    app.add_option("-H,--height", DRAW_HEIGHT, "canvas draw height")
        ->check(CLI::Range(16, 8056))
        ->default_val(256);
    app.add_option("-S,--save", SAVE_BITMAP, "save canvas draw to png file")
        ->check(CLI::IsMember({0, 1}))
        ->default_val(0);
    app.add_option("-R,--resource", RESOURCE_ID, "resource id for program loading image")
        ->check(CLI::Range(0, 5))
        ->default_val(2);
    app.add_option("-P,--picture", SAVE_SKP, "Save Canvas draw to skp file")
        ->check(CLI::IsMember({0, 1}))
        ->default_val(0);
    //catch exception and parse the command lines
    CLI11_PARSE(app, argc, argv);

    spdlog::info("Canvas: [wxh]=[{}x{}]", DRAW_WIDTH, DRAW_HEIGHT);

    fontMgr = SkFontMgr_New_Custom_Directory(fontDir.c_str());
    if (fontMgr == nullptr) {
        spdlog::error("{}: can not create font manager!", __FUNCTION__);
    }

    int fontFamilyCounts = fontMgr->countFamilies();
    spdlog::debug("{}: font family count is {}", __FUNCTION__, fontFamilyCounts);
    for (int idx = 0; idx < fontFamilyCounts; ++idx) {
        SkString fontFamilyName;
        fontMgr->getFamilyName(idx, &fontFamilyName);
        spdlog::debug("{}: Family[{}]={}", __FUNCTION__, idx, fontFamilyName.c_str());
    }

    const char* fontFamily = nullptr;
    SkFontStyle fontStyle;

    typeFace = fontMgr->legacyMakeTypeface(fontFamily, fontStyle);
    if (typeFace == nullptr) {
        spdlog::error("{}: can not create type face from font manager!", __FUNCTION__);
    }

    // --------------- init opengl ---------
    // if (!glfwInit()) {
    //     spdlog::error("{}: can not init glfw!", __FUNCTION__);
    //     return -1;
    // }
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    // glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
    // --------------init opengl end -------

    SkImageInfo imageInfo = SkImageInfo::Make(
        DRAW_WIDTH, DRAW_HEIGHT,
        kBGRA_8888_SkColorType,
        kOpaque_SkAlphaType);

    // --------------- begin init x11 ------------
    GrGLStandard forcedGpuAPI = kGL_GrGLStandard;
    GLXContext fContext = nullptr;
    Display* display = get_display();
    if (!display) {
        spdlog::error("{}: can not get x11 display!", __FUNCTION__);
        return -1;
    }

    // Get a matching FB config
    static int visual_attribs[] = {
        GLX_X_RENDERABLE    , True,
        GLX_DRAWABLE_TYPE   , GLX_PIXMAP_BIT,
        None
    };

    int glx_major, glx_minor;
    // FBConfigs were added in GLX version 1.3.
    if (!glXQueryVersion(display, &glx_major, &glx_minor) ||
            ((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1)) {
        spdlog::error("{}: glx version is lower than 1.3!", __FUNCTION__);
        return 1;
    }
    spdlog::info("{}: glx version is {}.{}", __FUNCTION__, glx_major, glx_minor);

    int fbcount;
    GLXFBConfig *fbc = glXChooseFBConfig(display, DefaultScreen(display),
                                          visual_attribs, &fbcount);
    if (!fbc) {
        spdlog::error("{}: Failed to retrieve a framebuffer config.", __FUNCTION__);
        return 1;
    }

    int best_fbc = -1, best_num_samp = -1;

    int i;
    for (i = 0; i < fbcount; ++i) {
        XVisualInfo *vi = glXGetVisualFromFBConfig(display, fbc[i]);
        if (vi) {
            int samp_buf, samples;
            glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
            glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLES, &samples);

            //SkDebugf("  Matching fbconfig %d, visual ID 0x%2x: SAMPLE_BUFFERS = %d,"
            //       " SAMPLES = %d\n",
            //        i, (unsigned int)vi->visualid, samp_buf, samples);
            spdlog::trace("{}: Matching fbconfig {}, visual ID 0x{:x}: SAMPLE_BUFFERS = {}, SAMPLES = {}",
                __FUNCTION__, i, (unsigned int)vi->visualid, samp_buf, samples);

            if (best_fbc < 0 || (samp_buf && samples > best_num_samp)) {
                best_fbc = i;
                best_num_samp = samples;
            }
        }
        XFree(vi);
    }

    GLXFBConfig bestFbc = fbc[best_fbc];

    // Be sure to free the FBConfig list allocated by glXChooseFBConfig()
    XFree(fbc);
    // Get a visual
    XVisualInfo *vi = glXGetVisualFromFBConfig(display, bestFbc);
    //SkDebugf("Chosen visual ID = 0x%x\n", (unsigned int)vi->visualid);
    spdlog::info("{}: Chosen visual ID = 0x{:x}", __FUNCTION__, (unsigned int)vi->visualid);

    Pixmap pixmap = XCreatePixmap(display, RootWindow(display, vi->screen), 10, 10, vi->depth);
    if (!pixmap) {
        spdlog::error("{}: Failed to create pixmap.", __FUNCTION__);
        return -1;
    }
    GLXPixmap glxPixmap = glXCreateGLXPixmap(display, vi, pixmap);
    // Done with the visual info data
    XFree(vi);

    // Get the default screen's GLX extension list
    const char *glxExts = glXQueryExtensionsString(
        display, DefaultScreen(display)
    );
    // Check for the GLX_ARB_create_context extension string and the function.
    // If either is not present, use GLX 1.3 context creation method.
    if (!gluCheckExtension(reinterpret_cast<const GLubyte*>("GLX_ARB_create_context"),
                           reinterpret_cast<const GLubyte*>(glxExts))) {
        if (kGLES_GrGLStandard != forcedGpuAPI) {
            fContext = glXCreateNewContext(display, bestFbc, GLX_RGBA_TYPE, nullptr, True);
        }
    } else {
        if (kGLES_GrGLStandard == forcedGpuAPI) {
            if (gluCheckExtension(
                    reinterpret_cast<const GLubyte*>("GLX_EXT_create_context_es2_profile"),
                    reinterpret_cast<const GLubyte*>(glxExts))) {
                fContext = CreateBestContext(true, display, bestFbc, nullptr);
            }
        } else {
            fContext = CreateBestContext(false, display, bestFbc, nullptr);
        }
    }
    if (!fContext) {
        spdlog::error("{}: Failed to create an OpenGL context.", __FUNCTION__);
        return 1;
    }

    // Verify that context is a direct context
    if (!glXIsDirect(display, fContext)) {
        //SkDebugf("Indirect GLX rendering context obtained.\n");
        spdlog::debug("{}: Indirect GLX rendering context obtained.", __FUNCTION__);
    } else {
        //SkDebugf("Direct GLX rendering context obtained.\n");
        spdlog::debug("{}: Direct GLX rendering context obtained.", __FUNCTION__);
    }


    // Verify that context is a direct context
    if (!glXIsDirect(display, fContext)) {
        //SkDebugf("Indirect GLX rendering context obtained.\n");
    } else {
        //SkDebugf("Direct GLX rendering context obtained.\n");
    }

    if (!glXMakeCurrent(display, glxPixmap, fContext)) {
        spdlog::error("{}: Could not set the context!", __FUNCTION__);
        return 1;
    }
    // ----------------end init x11 ------------
    sk_sp<const GrGLInterface> glInterface = nullptr;
    sk_sp<GrDirectContext> grContext = GrDirectContexts::MakeGL(glInterface);
    if (!grContext) {
        spdlog::error("{}: can not create GrDirectContext!", __FUNCTION__);
        return -1;
    }
    if (glInterface != nullptr) {
        bool valid = glInterface->validate();
        spdlog::info("{}: gl interface validate = {}", __FUNCTION__, valid);
        if (valid == false) {
            spdlog::error("{}: gl interface is not valid!", __FUNCTION__);
            return -1;
        }
    }

    GrBackendTexture backendTexture = grContext->createBackendTexture(
        DRAW_WIDTH,
        DRAW_HEIGHT,
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
        spdlog::error("{}: can not create backend texture!", __FUNCTION__);
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
        spdlog::error("{}: can not create sk surface from backend texture!", __FUNCTION__);
        return -1;
    }

    SkCanvas* canvas = skSurface->getCanvas();

    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(DRAW_WIDTH, DRAW_HEIGHT);
    if (SAVE_SKP) {
        spdlog::debug("{}: replace canvas with recording canvas!", __func__);
        canvas = recordingCanvas;
    }

    canvas->drawColor(SK_ColorTRANSPARENT);
    //--------------- begin draw ----------------
    CuteLittleFibonacciSphereEffect effect;
    effect.initlize(DRAW_WIDTH, DRAW_HEIGHT);
    effect.draw(canvas, 0.0f);
    //--------------- end draw ------------------
    // 将命令提交到 GPU 执行，确保所有绘制操作完成
    grContext->flushAndSubmit(GrSyncCpu::kYes);

    if (SAVE_SKP) {
        sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
        std::string skpFileName = generate_filename("output", "skp");
        savePictureAsSKP(picture, skpFileName.c_str());

        canvas = skSurface->getCanvas();
        canvas->drawPicture(picture);
    }

    if(SAVE_BITMAP) {
        SkBitmap bitmap;
        bitmap.allocPixels(imageInfo, imageInfo.minRowBytes());
        skSurface->readPixels(bitmap, 0, 0);

        std::string pngName = generate_filename("output", "png");
        saveBitmapAsPng(bitmap, pngName.c_str());
    }
    return 0;
}