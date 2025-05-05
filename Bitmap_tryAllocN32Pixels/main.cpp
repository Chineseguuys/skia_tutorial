#include "Skia/include/core/SkCanvas.h"
#include "Skia/include/core/SkSurface.h"
#include "Skia/include/core/SkStream.h"
#include "Skia/include/core/SkPictureRecorder.h"
#include "Skia/include/core/SkPicture.h"
#include "Skia/include/core/SkBitmap.h"
#include "Skia/include/encode/SkPngEncoder.h"

#include "Skia/include/core/SkTypeface.h"
#include "Skia/include/core/SkFontMgr.h"
#include "Skia/include/ports/SkFontMgr_empty.h"
#include "Skia/include/ports/SkFontMgr_directory.h"

#include "Skia/include/core/SkTextBlob.h"
#include "Skia/include/core/SkData.h"

#include "Skia/include/codec/SkCodec.h"
#include "Skia/include/core/SkAlphaType.h"
#include "Skia/include/core/SkColorType.h"
#include "Skia/include/core/SkImageInfo.h"
#include "Skia/include/core/SkImage.h"

#include "Skia/include/core/SkTextBlob.h"
#include "Skia/include/core/SkRefCnt.h"


#include "fmt/format.h"
#include "Skia/include/core/SkColor.h"
#include "Skia/include/core/SkPixmap.h"
// added and open the SK_DEBUG for SkRefCnt.h:166: fatal error: "assertf(rc == 1): NVRefCnt was 0"
#include "Skia/include/config/SkUserConfig.h"
#include "Skia/include/core/SkColorSpace.h"
#include "Skia/include/private/base/SkDebug.h"
#include "Skia/include/core/SkPixelRef.h"

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <X11/X.h>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <spdlog/spdlog.h>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// modified for compile error
#ifdef Success
#undef Success
#include "CLI/CLI.hpp"
#endif

static sk_sp<SkFontMgr> fontMgr;
static sk_sp<SkTypeface> typeFace;
static SkBitmap source;
static int DRAW_WIDTH = 256;
static int DRAW_HEIGHT = 256;
static bool SAVE_BITMAP = false;
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

// tryAllocN32Pixels
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::Make(80, 80, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    if (bitmap.tryAllocN32Pixels(80, 80)) {
        bitmap.eraseColor(SK_ColorTRANSPARENT);
        bitmap.erase(0x7f3f7fff, SkIRect::MakeWH(50, 30));
        bitmap.erase(0x3f7fff3f, SkIRect::MakeXYWH(20, 10, 50, 30));
        bitmap.erase(0x5fff3f7f, SkIRect::MakeXYWH(40, 20, 50, 30));
        canvas->drawImage(bitmap.asImage(), 0, 0);
        for (int x : { 0, 30, 60, 90 } ) {
            canvas->drawImage(bitmap.asImage(), x, 70);
        }
    }
}

// tryAllocPixels
void draw1(SkCanvas* canvas) {
    SkBitmap bitmap;
    SkImageInfo info = SkImageInfo::Make(64, 256, kGray_8_SkColorType, kOpaque_SkAlphaType);
    if (bitmap.tryAllocPixels(info, 0)) {
        SkCanvas offscreen(bitmap);
        offscreen.scale(.5f, .5f);
        for (int x : { 0, 64, 128, 192 } ) {
            offscreen.drawImage(source.asImage(), -x, 0);
            canvas->drawImage(bitmap.asImage(), x, 0);
        }
    }
}

// tryAllocPixelsFlags
void draw2(SkCanvas* canvas) {
    SkBitmap bitmap;
    if (!bitmap.tryAllocPixelsFlags(SkImageInfo::MakeN32(10000, 10000, kOpaque_SkAlphaType),
                                    SkBitmap::kZeroPixels_AllocFlag)) {
        SkDebugf("bitmap allocation failed!\n");
    } else {
        SkDebugf("bitmap allocation succeeded!\n");
    }
}

// Bitmap_tryAllocPixels_2
void draw3(SkCanvas* canvas) {
    SkBitmap bitmap;
    if (bitmap.tryAllocPixels(SkImageInfo::Make(64, 64, kGray_8_SkColorType, kOpaque_SkAlphaType))) {
        SkCanvas offscreen(bitmap);
        offscreen.scale(.25f, .5f);
        for (int y : { 0, 64, 128, 192 } ) {
            offscreen.drawImage(source.asImage(), -y, -y);
            canvas->drawImage(bitmap.asImage(), y, y);
        }
    }
}

// Bitmap_tryAllocPixels_3
void draw4(SkCanvas* canvas) {
    uint8_t set1[5] = { 0xCA, 0xDA, 0xCA, 0xE9, 0xA3 };
    SkBitmap bitmap;
    bitmap.installPixels(SkImageInfo::Make(5, 1, kGray_8_SkColorType, kOpaque_SkAlphaType), set1, 5);
    canvas->scale(10, 50);
    canvas->drawImage(bitmap.asImage(), 0, 0);
    if (bitmap.tryAllocPixels()) {
        bitmap.eraseColor(SK_ColorBLACK);
        canvas->drawImage(bitmap.asImage(), 8, 0);
        bitmap.setPixels(set1);
        canvas->drawImage(bitmap.asImage(), 16, 0);
    }
}

// Bitmap_tryAllocPixels_4

class LargePixelRef : public SkPixelRef {
public:
    LargePixelRef(const SkImageInfo& info, char* storage, size_t rowBytes)
        : SkPixelRef(info.width(), info.height(), storage, rowBytes) {
    }

    ~LargePixelRef() override {
        delete[] (char* ) this->pixels();
    }
};

class LargeAllocator : public SkBitmap::Allocator {
public:
    bool allocPixelRef(SkBitmap* bitmap) override {
        const SkImageInfo& info = bitmap->info();
        uint64_t rowBytes = info.minRowBytes64();
        uint64_t size = info.height() * rowBytes;
        char* addr = new char[size];
        if (nullptr ==  addr) {
            spdlog::critical("allocate addr for bitmap");
            return false;
        }
        sk_sp<SkPixelRef> pr = sk_sp<SkPixelRef>(new LargePixelRef(info, addr, rowBytes));
        if (!pr) {
            spdlog::critical("can not create SkPixelRef for bitmap!");
            return false;
        }
        bitmap->setPixelRef(std::move(pr), 0, 0);
        spdlog::info("{}: alloc Pixel Ref with size: {}, in address:{}", __func__, size, fmt::ptr(addr));
        return true;
    }
};

void draw5(SkCanvas* canvas) {
    LargeAllocator largeAllocator;
    SkBitmap bitmap;
    int width = 100;
    int height = 100;
    bitmap.setInfo(SkImageInfo::MakeN32(width, height, kOpaque_SkAlphaType));
    if (bitmap.tryAllocPixels(&largeAllocator)) {
        bitmap.eraseColor(0xff55aa33);
        canvas->drawImage(bitmap.asImage(), 0, 0);
    }
}


int main(int argc, char* argv[]) {
#if 1
    spdlog::set_level(spdlog::level::debug);
#endif

    // args parser
    CLI::App app{"Skia Fiddle"};
    app.add_option("--width", DRAW_WIDTH, "canvas draw width")
        ->check(CLI::Range(16, 1024))
        ->default_val(256);
    app.add_option("--height", DRAW_HEIGHT, "canvas draw height")
        ->check(CLI::Range(16, 1024))
        ->default_val(256);
    app.add_option("--save", SAVE_BITMAP, "save canvas draw to png file")
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

    loadPngToBitmap(pngResources[2].c_str(), source);

    SkImageInfo imageInfo = SkImageInfo::Make(
        DRAW_WIDTH, DRAW_HEIGHT,
        kBGRA_8888_SkColorType,
        kOpaque_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::Raster(imageInfo);
    SkCanvas* canvas = surface->getCanvas();
#ifdef INIT_WHITEBACKGROUND
    canvas->drawColor(SK_ColorWHITE);
#else
    canvas->drawColor(SK_ColorTRANSPARENT);
#endif
    draw5(canvas);

    if(SAVE_BITMAP) {
        SkBitmap bitmap;
        bitmap.allocPixels(imageInfo, imageInfo.minRowBytes());
        surface->readPixels(bitmap, 0, 0);

        std::string pngName = generate_filename("output", "png");
        saveBitmapAsPng(bitmap, pngName.c_str());
    }
    return 0;
}