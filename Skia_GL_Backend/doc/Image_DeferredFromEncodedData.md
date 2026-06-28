# DeferredFromEncodedData 与 SkImage_Lazy

## 概述

`SkImages::DeferredFromEncodedData` 从编码数据（如 JPEG/PNG 字节流）创建一个 **延迟解码** 的 `SkImage`。返回的是 `SkImage_Lazy` 实例，它在构造时不解码任何像素，只在真正需要像素时（如 `drawImage`）才触发解码。

---

## 一、DeferredFromEncodedData 的流程

### 调用链

```
SkImages::DeferredFromEncodedData(encoded, alphaType)
  │
  ├─ ① 入参校验：data 为空或 size=0 → 返回 nullptr
  │
  └─ ② SkImageGenerators::MakeFromEncoded(data, alphaType)
       │
       ├─ data 为空或 alphaType == kOpaque → 返回 nullptr
       ├─ 检查全局自定义工厂 gFactory，有则优先调用
       │
       └─ ③ SkCodecImageGenerator::MakeFromEncodedCodec(data, at)
            │
            ├─ SkCodec::MakeFromData(data)  —— 根据编码数据创建解码器
            ├─ 解码器为 null → 返回 nullptr
            └─ new SkCodecImageGenerator(codec, at)
                 ├─ adjust_info(codec, at) 构建 SkImageInfo：
                 │     - 从 codec 取原始 info
                 │     - 若指定了 alphaType，直接覆盖
                 │     - 未指定且原始为 Unpremul → 改为 Premul
                 │     - 若 origin 导致宽高交换 → 交换宽高
                 └─ 存储 codec 对象
       │
       └─ 返回 std::unique_ptr<SkImageGenerator>
  │
  └─ ④ SkImages::DeferredFromGenerator(generator)
       │
       ├─ SharedGenerator::Make(generator)  —— 包装为线程安全的共享生成器
       │
       ├─ ⑤ SkImage_Lazy::Validator(sharedGen, nullptr, nullptr)
       │     ├─ 取 info，若为空 → 验证失败
       │     └─ 记录 uniqueID
       │
       └─ validator 通过 → 创建 sk_sp<SkImage_Lazy> 并返回
            validator 失败 → 返回 nullptr
```

### 数据结构演变

```
编码字节流 (SkData)
  │
  ▼ SkCodec::MakeFromData
SkCodec (解码器，知道图片格式、尺寸、色彩信息)
  │
  ▼ 包装
SkCodecImageGenerator (继承 SkImageGenerator，持有 SkCodec)
  │
  ▼ 返回
std::unique_ptr<SkImageGenerator>
  │
  ▼ SharedGenerator::Make
SharedGenerator (线程安全包装，内部有 mutex)
  │
  ▼ 验证 + 创建
SkImage_Lazy (延迟 SkImage)
```

### 关键设计点

1. **全程无解码**：只创建了解码器元数据和 `SkImageInfo`，没有发生像素解码。
2. **自定义工厂钩子**：`MakeFromEncoded` 先检查 `gFactory` 全局回调，允许外部注入自定义生成器。
3. **alphaType 处理**：未指定时，`kUnpremul` 自动替换为 `kPremul`，因为预乘 alpha 在渲染管线中性能更好。
4. **SharedGenerator 的线程安全**：同一个生成器可能被多个 `SkImage_Lazy` 共享引用，内部用 mutex 保护解码过程的互斥访问。

---

## 二、SkImage_Lazy 在 drawImage 中的解码时机

`SkImage_Lazy` 构造时不解码。真正解码发生在 `drawImage` → GPU 需要纹理时。

### 完整调用链

```
canvas->drawImage(image, x, y)
  │
  ▼ SkCanvas → SkDevice (GPU 设备)
  │  GPU 设备需要把 SkImage 当作纹理来绘制
  │
  ▼ LockTextureProxyView(rContext, img, texGenPolicy, mipmapped)
  │  尝试 4 种策略，按优先级：
  │
  ├─ ① 查缓存：根据 image.uniqueID() 找已生成的纹理 proxy
  │     └─ 命中 → 直接返回，**不解码**
  │
  ├─ ② 原生生成：generator->isTextureGenerator()
  │     └─ 仅 GrTextureGenerator 子类支持，普通 codec 不走这里
  │
  ├─ ③ YUV 平面：generator 返回 YUV 数据，GPU 端做 YUV→RGB 转换
  │     └─ 需要 mipmapped == kNo 且未禁用 GPU YUV 转换
  │
  └─ ④ bitmap 解码上传（大多数 codec 图像走这条路）
        │
        ▼ img->getROPixels(nullptr, &bitmap, hint)
          │
          ├─ 查 SkBitmapCache（key 为 image uniqueID）
          │   └─ 命中 → 返回缓存的 bitmap，**不解码**
          │
          └─ 未命中 → 分配内存，调用解码
              │
              ▼ ScopedGenerator(fSharedGenerator)->getPixels(pmap)
                │  获取互斥锁，调用生成器的 getPixels
                │
                ▼ SkCodecImageGenerator::getPixels(...)
                  │
                  ▼ fCodec->getPixels(pm, options)
                    │
                    ▼ ★ 这里真正解码 JPEG/PNG/WebP 等编码数据为 RGBA 像素 ★
              │
              ├─ 解码成功 → 写入 SkBitmapCache
              │
              ▼ GrMakeUncachedBitmapProxyView(rContext, bitmap, ...)
                │  将 bitmap 像素上传到 GPU 纹理
                │
                ▼ 给纹理 proxy 设置 unique key（关联 image.uniqueID）
                │
                ▼ 返回 GrSurfaceProxyView
```

### 缓存层次

| 缓存 | 位置 | key | 命中后效果 |
|------|------|-----|-----------|
| SkBitmapCache | CPU 侧 | image.uniqueID() | 跳过 decoder.getPixels() |
| GrProxyProvider (UniqueKey) | GPU 侧 | image.uniqueID() | 跳过 bitmap 分配 + 解码 + 上传 |

第二次 `drawImage` 同一个 `SkImage_Lazy` 时，直接在 `LockTextureProxyView` 步骤 ① 命中 GPU proxy 缓存，全程无解码。

### 时间线总结

```
构造时:  创建 SkCodecImageGenerator → 无解码
         │
drawImage:
         ├─ LockTextureProxyView → 缓存未命中
         ├─ getROPixels → 分配 bitmap
         │    └─ SkCodec::getPixels → ★ 解码发生在这里 ★
         ├─ bitmap → GPU 上传
         └─ 缓存结果（bitmap cache + GPU proxy key）
```

---

## 三、示例代码

```cpp
// 从编码数据创建延迟图像（不解码）
sk_sp<SkData> data(SkJpegEncoder::Encode(nullptr, sourceImage.get(), options));
sk_sp<SkImage> lazyImage = SkImages::DeferredFromEncodedData(data);

// 此时 lazyImage 只是持有解码器，没有分配像素内存
// 解码在 drawImage 内部、GPU 需要纹理时才发生
canvas->drawImage(lazyImage, x, y);
```