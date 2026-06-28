## SkImage::isValid

声明在 `include/core/SkImage.h`：

```cpp
virtual bool isValid(GrRecordingContext* context) const = 0;
virtual bool isValid(SkRecorder*) const = 0;
```

### 作用

判断该 `SkImage` 是否能在给定的记录上下文上进行绘制操作。

### 返回值

- **`true`**：该 image 可以在当前 context 上正常绘制
- **`false`**：该 image 在当前 context 上不可绘制

### `false` 的常见原因

1. **GPU 纹理图关联的 context 已销毁**：image 在某个 `GrDirectContext` 上创建，该 context 已被释放后，image 变为无效
2. **跨 context 使用**：image 在 context A 创建，但传入 context B 调用 `isValid`
3. **lazy image 未就绪**：延迟解码的 image 可能在某些后端（光栅 / GPU）上不支持解码
4. **backend texture 已失效**：通过 `BorrowTextureFrom` 或 `AdoptTextureFrom` 创建的 image，其底层纹理被外部释放

### 使用示例

```cpp
auto dContext = GrAsDirectContext(canvas->recordingContext());
if (image->isValid(dContext->asRecorder())) {
    canvas->drawImage(image, 0, 0);  // 安全绘制
}
```

`dContext->asRecorder()` 将 GPU context 转为 `SkRecorder*` 传入 `isValid`，以查询该 image 在当前 GPU 上下文中是否可绘制。

### 与 isTextureBacked 的区别

| 方法 | 含义 |
|---|---|
| `isValid(context)` | 运行时检查：该 image 在当前 context 上**现在能否绘制** |
| `isTextureBacked()` | 类型检查：该 image 是否是 GPU 纹理类型 |

一个 image 可以是 texture-backed 但 `isValid` 返回 `false`（context 已失效）；一个 raster image 不是 texture-backed 但 `isValid` 可能返回 `true`（可以上传到 GPU 绘制）。