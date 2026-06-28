## SkTileMode

定义在 `include/core/SkTileMode.h`，控制 shader 在图像原始边界之外的采样行为。通过 `makeShader` 的 `tmx` / `tmy` 参数分别指定 X 和 Y 方向的平铺模式。

### 四种模式


| 枚举值       | 行为                    | 可视化描述          |
| --------- | --------------------- | -------------- |
| `kClamp`  | 边界外取最近边缘像素的颜色，无限延伸    | 边缘颜色沿图像边界向外拉伸  |
| `kRepeat` | 图像在水平和垂直方向无限重复平铺      | 像瓷砖一样紧密排列      |
| `kMirror` | 图像重复平铺，但相邻图像镜像翻转，边界无缝 | 像万花筒，相邻块方向交替   |
| `kDecal`  | 只绘制原始范围内的内容，范围外透明     | 只有一张原图，其余区域不可见 |


### `kClamp` — 默认模式

没有显式指定 tile mode 的 `makeShader` 重载默认使用 `kClamp`。当图像经过旋转等变换后，边界形成斜边时，边缘像素会沿边界方向拉伸成色带，产生"颜色向外扩散"的视觉效果。

```cpp
// 以下两种写法等价，默认 kClamp
image->makeShader(SkSamplingOptions(), matrix);
image->makeShader(SkTileMode::kClamp, SkTileMode::kClamp, SkSamplingOptions(), matrix);
```

### `kDecal` — 透明边界

如果只需要绘制图像本身而不想要边界拉伸或重复，使用 `kDecal`：

```cpp
image->makeShader(SkTileMode::kDecal, SkTileMode::kDecal, SkSamplingOptions(), matrix);
```

图像范围之外的像素全部为透明黑色。

### X / Y 独立控制

两个方向的平铺模式可以不同：

```cpp
// X 方向重复，Y 方向镜像
image->makeShader(SkTileMode::kRepeat, SkTileMode::kMirror, SkSamplingOptions(), matrix);
```

### 与矩阵变换的关系

当 shader 应用了旋转矩阵后，图像的"原始范围"也随之旋转。例如旋转 45° 后，图像在画布空间中占据一个菱形区域。tile mode 的边界判断基于旋转后的坐标系，所以 `kClamp` 产生的色带会沿着菱形的对角线方向延伸。