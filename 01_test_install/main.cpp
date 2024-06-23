#include "include/core/SkBitmap.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#include "include/effects/SkGradientShader.h"

// for log
#include <spdlog/spdlog.h>

const char* pText = "Hello Skia";

int main(int argc, char* argv[]) {
    const int width = 1024;
    const int height = 768;

    SkBitmap bitmap;
    SkImageInfo ii = SkImageInfo::Make(480, 320, kBGRA_8888_SkColorType, kPremul_SkAlphaType);
    bitmap.allocPixels(ii, ii.minRowBytes());

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style); // 线框模式，只描绘边框
    paint.setColor(0xff1f78b4);
    paint.setStrokeWidth(4);

    bitmap.allocPixels();
    SkCanvas canvas(bitmap);
    canvas.clear(0x000000);

    // 绘制一个椭圆
    {
        SkRect rc;
        rc.fLeft = 123;
        rc.fTop = 0;
        rc.fRight = 222;
        rc.fBottom = 50;
        canvas.drawOval(rc, paint);
    }

#if 0
    {
        // 设置字体和笔画
        sk_sp<SkTypeface> typeFace =  fontMgr->makeFromFile(
            "/usr/share/fonts/noto/NotoSans-Bold.ttf",
            0
        );
        if (typeFace == nullptr) {
            spdlog::warn("can not create type face from font manager!");
        } else {
            spdlog::info("typeFace: is Bold? {}", typeFace->isBold());
            spdlog::info("typeFace: is Italic? {}", typeFace->isItalic());
            spdlog::info("typeFace: unique ID = {}", typeFace->uniqueID());
        }
        SkFont font = SkFont(typeFace, 50);
        font.setEdging(SkFont::Edging::kAntiAlias);
        paint.setColor(SK_ColorRED);
        paint.setStrokeWidth(2);
        paint.setAntiAlias(true);

        SkTextBlobBuilder builder;
        const SkTextBlobBuilder::RunBuffer& runBuffer = builder.allocRun(
            font, strlen(pText), 0, 0); // 为文本分配一个运行缓冲区
        memcpy(runBuffer.glyphs, pText, strlen(pText)); // 将文本内容复制到运行缓冲区中
        sk_sp<SkTextBlob> textBlob = builder.make();

        canvas.drawTextBlob(textBlob, 100, 100, paint);
    }
#endif

    // 绘制一个矩形 填充模式
    {
        SkPaint __tempPaint;
        __tempPaint.setColor(SK_ColorBLUE);
        __tempPaint.setStyle(SkPaint::kFill_Style); // 填充模式，填充整个形状
        canvas.drawRect(SkRect::MakeXYWH(100, 100, 50, 50), __tempPaint);
    }

    // 绘制一个矩形 线框 + 填充
    {
        SkPaint __tempPaint;
        __tempPaint.setColor(SK_ColorGREEN);
        __tempPaint.setStyle(SkPaint::kStrokeAndFill_Style); // 填充模式，填充整个形状
        __tempPaint.setStrokeWidth(8);
        // 这里绘制的是填充的大小，线框会占用额外的 4个像素的宽度
        // 所以这个矩阵，看起来会比上面的那个矩阵大一些
        canvas.drawRect(SkRect::MakeXYWH(200, 100, 50, 50), __tempPaint);
    }
    
    // 梯度颜色渐变效果, 点到点的渐变效果，梯度变化的方向为两点连接的直线方向
    {
        // 标准的中等灰色 rgb 值为 0x888888, 不透明
        // canvas.clear(SK_ColorGRAY);
        SkPaint __tempPaint;
        SkPoint points[] = {{0, 0}, {50, 50}};
        SkColor colors[] = {SK_ColorGREEN, SK_ColorBLUE};
        __tempPaint.setShader(SkGradientShader::MakeLinear(
            points,
            colors,
            nullptr,
            2,
            SkTileMode::kClamp
        ));
        canvas.drawRect(SkRect::MakeXYWH(0, 0, 50, 50), __tempPaint);
    }

    // 多个点的颜色梯度渐变
    {
        // 标准的中等灰色 rgb 值为 0x888888, 不透明
        // canvas.clear(SK_ColorGRAY);
        float __xAxis = 200;
        float __yAxis = 200;
        SkPaint __tempPaint;
        SkPoint points[] = {
            {__xAxis, __yAxis}, 
            {__xAxis + 50, __yAxis + 50}, 
            {__xAxis, __yAxis + 50},
            {__xAxis + 50, __yAxis}
        };
        SkColor colors[] = {SK_ColorGREEN, SK_ColorBLUE, SK_ColorRED, SK_ColorYELLOW};
        __tempPaint.setShader(SkGradientShader::MakeLinear(
            points,
            colors,
            nullptr,
            4,
            SkTileMode::kClamp
        ));
        canvas.drawRect(SkRect::MakeXYWH(__xAxis, __yAxis, 50, 50), __tempPaint);
    }

    // 旋转操作
    {
        SkMatrix m = SkMatrix::RotateDeg(45, {25, 25});
        SkPaint _tempPaint;
        SkPoint points[] = {{0, 0}, {50, 0}};
        SkColor colors[] = {SK_ColorGREEN, SK_ColorBLUE};
        _tempPaint.setShader(SkGradientShader::MakeLinear(
            points,
            colors,
            nullptr,
            2,
            SkTileMode::kClamp
        ));
        canvas.translate(50, 50);
        canvas.concat(m);
        canvas.drawRect(SkRect::MakeXYWH(0, 0, 50, 50), _tempPaint);
    }

    SkFILEWStream stream("./canvas.png");
    SkPngEncoder::Options options;
    options.fZLibLevel = 6;
    if (!SkPngEncoder::Encode(&stream, bitmap.pixmap(), options)) {
        printf("failed to save bitmap to png file");
    }

    return 0;
}