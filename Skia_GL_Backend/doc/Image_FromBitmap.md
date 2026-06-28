## setImmutable 与 asImage 的拷贝/共享策略

`SkBitmap::asImage()` 内部调用 `SkImages::RasterFromBitmap`，最终走 `SkMakeImageFromRasterBitmapPriv`，传入 `kIfMutable_SkCopyPixelsMode`。

判决逻辑：

- **bitmap 可变**（`!isImmutable()`）→ 触发 `MakeRasterCopyPriv`，分配新内存将像素完整拷贝一份。返回的 `SkImage` 持有独立副本，原始像素后续修改不可见。
- **bitmap 不可变**（`isImmutable()`）→ 直接构造 `SkImage_Raster`，共享 bitmap 的 `PixelRef`（零拷贝）。返回的 `SkImage` 和 bitmap 指向同一块内存。

因此 `setImmutable()` 是分水岭：在其之前调用 `asImage()` 拿到的 image 是独立副本，在其之后调用 `asImage()` 拿到的 image 是共享引用。`setImmutable()` 的语义是"我承诺不再修改这块像素"，Skia 据此启用零拷贝优化。