## GrSurfaceOrigin（纹理原点）

`GrSurfaceOrigin` 告诉 Skia GPU 纹理的 (0,0) 坐标位于哪里，定义在 `include/gpu/ganesh/GrTypes.h`：

- `kTopLeft_GrSurfaceOrigin` — 原点在左上角（与 Skia CPU 端坐标系一致）
- `kBottomLeft_GrSurfaceOrigin` — 原点在左下角（OpenGL 默认约定）

### 作用

Skia 的 CPU 端坐标系原点在左上角。当包装 GPU 纹理为 `SkImage` 时，如果纹理原点在左下角，Skia 会在内部做垂直翻转，保证绘制出来的图像方向正确。如果填反了，画面会上下颠倒。

### OpenGL 默认

OpenGL 中纹理坐标 (0,0) 默认对应纹理图像左下角像素。但多数图像格式（PNG、JPEG 等）数据从上往下排列，因此直接用 `glTexImage2D` 上传后图像是上下颠倒的，需要翻转 UV 或提前翻转数据。

### 在 Skia 中的使用

从外部 GL 纹理创建 `SkImage` 时通常用 `kBottomLeft_GrSurfaceOrigin`，因为外部纹理遵循 OpenGL 约定。从 Skia 内部拿回的 `GrBackendTexture` 再包装时，通常用 `kTopLeft_GrSurfaceOrigin`，因为 Skia 已将其统一为左上角原点。

```cpp
// 外部 GL 纹理 → kBottomLeft
sk_sp<SkImage> imageFromBackend = SkImages::AdoptTextureFrom(
    direct, backEndTexture, kBottomLeft_GrSurfaceOrigin, ...);

// Skia 内部纹理 → kTopLeft
sk_sp<SkImage> imageFromTexture = SkImages::AdoptTextureFrom(
    direct, textureFromImage, kTopLeft_GrSurfaceOrigin, ...);
```

### 相关源码

Skia 的 RenderTarget 默认使用 `kBottomLeft_GrSurfaceOrigin`，而 `GrDirectContext::updateBackendTexture` 上传像素数据时默认 `kTopLeft_GrSurfaceOrigin`（因为 pixmap 数据按左上角排列）。

---

## 从 GrBackendTexture 获取 GL Texture ID

`GrBackendTexture` 内部以类型擦除方式存储后端特定数据，GL 后端的纹理 ID 需通过专门的工具函数提取。

### 用法

```cpp
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"

GrGLTextureInfo info;
if (GrBackendTextures::GetGLTextureInfo(textureFromImage, &info)) {
    // info.fID      → OpenGL texture ID（glGenTextures 返回的整数）
    // info.fTarget  → 纹理目标，如 GL_TEXTURE_2D
    // info.fFormat  → 内部格式，如 GL_RGBA8
}
```

如果当前 `GrBackendTexture` 不是 GL 后端创建的（比如 Vulkan / Metal），`GetGLTextureInfo` 返回 `false`。

### GrGLTextureInfo 结构

```cpp
struct GrGLTextureInfo {
    GrGLenum fTarget;                       // 纹理绑定点
    GrGLuint fID;                           // 纹理 ID
    GrGLenum fFormat = 0;                   // 内部格式
    skgpu::Protected fProtected;            // 是否受保护内存
};
```

### 相关 API 位置

- 工厂函数：`GrBackendTextures::MakeGL()` — 从 GL 纹理信息创建 `GrBackendTexture`
- 提取函数：`GrBackendTextures::GetGLTextureInfo()` — 从 `GrBackendTexture` 提取 GL 信息
- 参数通知：`GrBackendTextures::GLTextureParametersModified()` — 外部修改了纹理参数后通知 Skia