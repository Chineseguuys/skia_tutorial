#ifndef GLXCONTEXT_H_DEFINED
#define GLXCONTEXT_H_DEFINED

#include <X11/Xlib.h>

// #include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <vector>

#include <spdlog/spdlog.h>

#include "skia/include/gpu/ganesh/gl/GrGLInterface.h"
#include "skia/include/private/base/SkOnce.h"

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

class GLXGLContext {
private:
    Display* fDisplay = nullptr;
    Pixmap fPixmap = 0;
    GLXPixmap fGlxPixmap = 0;
    GLXContext fContext = nullptr;
    GrGLStandard fGLStandard = kGL_GrGLStandard;
    sk_sp<const GrGLInterface> fGLInterface;
    sk_sp<const GrGLInterface> fOriginalGLInterface;

public:
    GLXGLContext(GrGLStandard glStandard)
        : fGLStandard(glStandard), fDisplay(nullptr), fPixmap(0), fGlxPixmap(0), fContext(nullptr) {
        fDisplay = get_display();
        if (!fDisplay) {
            spdlog::error("{}: can not get x11 display!", __FUNCTION__);
            this->destroyGLContext();
            return;
        }

        // Get a matching FB config
        static int visual_attribs[] = {
            GLX_X_RENDERABLE    , True,
            GLX_DRAWABLE_TYPE   , GLX_PIXMAP_BIT,
            None
        };

        int glx_major, glx_minor;
        // FBConfigs were added in GLX version 1.3.
        if (!glXQueryVersion(fDisplay, &glx_major, &glx_minor) ||
                ((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1)) {
            spdlog::error("{}: glx version is lower than 1.3!", __FUNCTION__);
            this->destroyGLContext();
            return;
        }
        spdlog::info("{}: glx version is {}.{}", __FUNCTION__, glx_major, glx_minor);

        int fbcount;
        GLXFBConfig *fbc = glXChooseFBConfig(fDisplay, DefaultScreen(fDisplay),
                                            visual_attribs, &fbcount);
        if (!fbc) {
            spdlog::error("{}: Failed to retrieve a framebuffer config.", __FUNCTION__);
            this->destroyGLContext();
            return;
        }

        int best_fbc = -1, best_num_samp = -1;

        int i;
        for (i = 0; i < fbcount; ++i) {
            XVisualInfo *vi = glXGetVisualFromFBConfig(fDisplay, fbc[i]);
            if (vi) {
                int samp_buf, samples;
                glXGetFBConfigAttrib(fDisplay, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
                glXGetFBConfigAttrib(fDisplay, fbc[i], GLX_SAMPLES, &samples);

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
        XVisualInfo *vi = glXGetVisualFromFBConfig(fDisplay, bestFbc);
        //SkDebugf("Chosen visual ID = 0x%x\n", (unsigned int)vi->visualid);
        spdlog::info("{}: Chosen visual ID = 0x{:x}", __FUNCTION__, (unsigned int)vi->visualid);

        fPixmap = XCreatePixmap(fDisplay, RootWindow(fDisplay, vi->screen), 10, 10, vi->depth);
        if (!fPixmap) {
            spdlog::error("{}: Failed to create pixmap.", __FUNCTION__);
            this->destroyGLContext();
            return;
        }
        fGlxPixmap = glXCreateGLXPixmap(fDisplay, vi, fPixmap);
        // Done with the visual info data
        XFree(vi);

        // Get the default screen's GLX extension list
        const char *glxExts = glXQueryExtensionsString(
            fDisplay, DefaultScreen(fDisplay)
        );
        // Check for the GLX_ARB_create_context extension string and the function.
        // If either is not present, use GLX 1.3 context creation method.
        if (!gluCheckExtension(reinterpret_cast<const GLubyte*>("GLX_ARB_create_context"),
                            reinterpret_cast<const GLubyte*>(glxExts))) {
            if (kGLES_GrGLStandard != fGLStandard) {
                fContext = glXCreateNewContext(fDisplay, bestFbc, GLX_RGBA_TYPE, nullptr, True);
            }
        } else {
            if (kGLES_GrGLStandard == fGLStandard) {
                if (gluCheckExtension(
                        reinterpret_cast<const GLubyte*>("GLX_EXT_create_context_es2_profile"),
                        reinterpret_cast<const GLubyte*>(glxExts))) {
                    fContext = CreateBestContext(true, fDisplay, bestFbc, nullptr);
                }
            } else {
                fContext = CreateBestContext(false, fDisplay, bestFbc, nullptr);
            }
        }
        if (!fContext) {
            spdlog::error("{}: Failed to create an OpenGL context.", __FUNCTION__);
            this->destroyGLContext();
            return;
        }

        // Verify that context is a direct context
        if (!glXIsDirect(fDisplay, fContext)) {
            //SkDebugf("Indirect GLX rendering context obtained.\n");
            spdlog::debug("{}: Indirect GLX rendering context obtained.", __FUNCTION__);
        } else {
            //SkDebugf("Direct GLX rendering context obtained.\n");
            spdlog::debug("{}: Direct GLX rendering context obtained.", __FUNCTION__);
        }


        // Verify that context is a direct context
        if (!glXIsDirect(fDisplay, fContext)) {
            //SkDebugf("Indirect GLX rendering context obtained.\n");
        } else {
            //SkDebugf("Direct GLX rendering context obtained.\n");
        }

        if (!glXMakeCurrent(fDisplay, fGlxPixmap, fContext)) {
            spdlog::error("{}: Could not set the context!", __FUNCTION__);
            this->destroyGLContext();
            return;
        }
    }

    ~GLXGLContext() {
        spdlog::info("{}: destruct GLXGLContext", __FUNCTION__);
        this->tearDown();
        this->destroyGLContext();
    }

    void init(sk_sp<const GrGLInterface> sharedGLInterface = nullptr) {
        fGLInterface = std::move(sharedGLInterface);
        fOriginalGLInterface = fGLInterface;
    }

    void tearDown() {
        fGLInterface.reset();
        fOriginalGLInterface.reset();
    }

    void destroyGLContext() {
        if (fDisplay) {
            if (fContext) {
                if (glXGetCurrentContext() == fContext) {
                    // This will ensure that the context is immediately deleted.
                    glXMakeContextCurrent(fDisplay, None, None, nullptr);
                }
                glXDestroyContext(fDisplay, fContext);
                fContext = nullptr;
            }

            if (fGlxPixmap) {
                glXDestroyGLXPixmap(fDisplay, fGlxPixmap);
                fGlxPixmap = 0;
            }

            if (fPixmap) {
                XFreePixmap(fDisplay, fPixmap);
                fPixmap = 0;
            }

            fDisplay = nullptr;
        }
    }

private:
    static GLXContext CreateBestContext(bool isES, Display* display, GLXFBConfig bestFbc,
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
};

#endif // GLXCONTEXT_H_DEFINED