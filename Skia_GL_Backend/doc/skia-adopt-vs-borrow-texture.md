# AdoptTextureFrom vs BorrowTextureFrom

两个函数都用于将一个已有的 GPU 后端纹理包装成 `SkImage`，差别在于**纹理所有权归属和释放机制**。

## 核心区别

| | `AdoptTextureFrom` | `BorrowTextureFrom` |
|---|---|---|
| 所有权 | Skia **接管**所有权 | 调用方**保留**所有权 |
| 纹理生命周期 | Skia 在 `SkImage` 销毁时自动释放纹理 | 调用方必须保持纹理有效，直到收到 Skia 的回调通知 |
| 释放通知 | 无回调，Skia 内部直接释放 | 有回调：`TextureReleaseProc` + `ReleaseContext` |
| `colorSpace` 参数 | 可选（有三个重载） | 必传 |
| 额外参数 | 无 | `textureReleaseProc`、`releaseContext` |

## 所有权流转示意

```
AdoptTextureFrom:
  调用方: 创建 GL 纹理 → 交给 Skia → 之后无需关心
  Skia:   持有纹理 → SkImage 引用计数归零时自动 glDeleteTextures

BorrowTextureFrom:
  调用方: 创建 GL 纹理 → 借给 Skia → 等待回调 → 收到回调后自行释放/回收
  Skia:   借用纹理 → SkImage 销毁时调用 textureReleaseProc(releaseContext)
```

## 函数签名

### AdoptTextureFrom

三个重载，`colorSpace` 和 `alphaType` 逐步可选：

```cpp
// 最简版本（仅 colorType）
sk_sp<SkImage> AdoptTextureFrom(GrRecordingContext* context,
                                const GrBackendTexture& backendTexture,
                                GrSurfaceOrigin textureOrigin,
                                SkColorType colorType);

// 指定 alpha type
sk_sp<SkImage> AdoptTextureFrom(GrRecordingContext* context,
                                const GrBackendTexture& backendTexture,
                                GrSurfaceOrigin textureOrigin,
                                SkColorType colorType,
                                SkAlphaType alphaType);

// 指定 alpha type + color space
sk_sp<SkImage> AdoptTextureFrom(GrRecordingContext* context,
                                const GrBackendTexture& backendTexture,
                                GrSurfaceOrigin textureOrigin,
                                SkColorType colorType,
                                SkAlphaType alphaType,
                                sk_sp<SkColorSpace> colorSpace);
```

### BorrowTextureFrom

唯一签名：

```cpp
sk_sp<SkImage> BorrowTextureFrom(GrRecordingContext* context,
                                 const GrBackendTexture& backendTexture,
                                 GrSurfaceOrigin origin,
                                 SkColorType colorType,
                                 SkAlphaType alphaType,
                                 sk_sp<SkColorSpace> colorSpace,
                                 TextureReleaseProc textureReleaseProc = nullptr,
                                 ReleaseContext releaseContext = nullptr);
```

## 参数说明（共有参数）

| 参数 | 类型 | 作用 |
|------|------|------|
| `context` | `GrRecordingContext*` | GPU 录制上下文，连接 GPU 后端，不能为空 |
| `backendTexture` | `const GrBackendTexture&` | 驻留在 GPU 上的纹理。OpenGL 下内部是 `GrGLTextureInfo`（含 target、ID、format） |
| `origin` | `GrSurfaceOrigin` | 纹理坐标原点：`kTopLeft_GrSurfaceOrigin` 或 `kBottomLeft_GrSurfaceOrigin` |
| `colorType` | `SkColorType` | 颜色通道布局，如 `kRGBA_8888_SkColorType` |
| `alphaType` | `SkAlphaType` | alpha 编码方式：`kOpaque_SkAlphaType` / `kPremul_SkAlphaType` / `kUnpremul_SkAlphaType` |
| `colorSpace` | `sk_sp<SkColorSpace>` | 色彩空间。`BorrowTextureFrom` 必传，可传 `nullptr` |

## BorrowTextureFrom 的释放回调

`textureReleaseProc` 类型为 `void (*)(ReleaseContext)`。当 Skia 不再引用该纹理时调用，通知调用方可以安全释放或回收纹理。

示例（来自 `Image_BorrowTextureFrom_2`）：

```cpp
auto releaseCallback = [](SkImages::ReleaseContext releaseContext) -> void {
    *((int*)releaseContext) += 128;  // 通过 releaseContext 传回状态
};

sk_sp<SkImage> image = SkImages::BorrowTextureFrom(
    dContext,
    backEndTexture,
    origin,
    kRGBA_8888_SkColorType,
    kOpaque_SkAlphaType,
    nullptr,          // colorSpace
    releaseCallback,  // 释放回调
    &x                // releaseContext
);
```

## 使用场景

- **AdoptTextureFrom**：纹理是一次性的，专供 Skia 使用，无需复用。简单省事。
- **BorrowTextureFrom**：纹理由你管理，可能需要在 Skia 使用期间读写，或使用完毕后回收到纹理池中复用。