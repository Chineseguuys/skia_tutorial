# Skia 色彩空间：色域矩阵与传递函数

Skia 使用 skcms 库进行色彩管理。每个 `SkColorSpace` 对象由两部分定义：

- **色域矩阵** (`fToXYZD50`)：将线性 RGB 映射到设备无关的 CIE XYZ D50 色彩空间
- **传递函数** (`fTransferFn`)：在编码值（gamma 压缩）和线性值之间转换

## 完整转换管线

```
编码 RGB → [传递函数解码] → 线性 RGB → [色域矩阵] → XYZ D50
                                                      ↓
目标编码 RGB ← [目标传递函数编码] ← 目标线性 RGB ← [逆色域矩阵]
```

---

## 一、色域矩阵

### 数据结构

```c
typedef struct skcms_Matrix3x3 {
    float vals[3][3];
} skcms_Matrix3x3;
```

一个 3×3 的行优先矩阵，将色彩空间的线性 RGB 值变换到 XYZ D50：

```
X = m[0][0]*R + m[0][1]*G + m[0][2]*B
Y = m[1][0]*R + m[1][1]*G + m[1][2]*B
Z = m[2][0]*R + m[2][1]*G + m[2][2]*B
```

### sRGB 色域矩阵 (`kSRGB`)

```cpp
static constexpr skcms_Matrix3x3 kSRGB = {{
    // 0.436065674f, 0.385147095f, 0.143066406f,
    // 0.222488403f, 0.716873169f, 0.060607910f,
    // 0.013916016f, 0.097076416f, 0.714096069f,
    { SkFixedToFloat(0x6FA2), SkFixedToFloat(0x6299), SkFixedToFloat(0x24A0) },
    { SkFixedToFloat(0x38F5), SkFixedToFloat(0xB785), SkFixedToFloat(0x0F84) },
    { SkFixedToFloat(0x0390), SkFixedToFloat(0x18DA), SkFixedToFloat(0xB6CF) },
}};
```

**示例**：sRGB 纯红色 `(1.0, 0.0, 0.0)` 转换为 XYZ D50：

```
X = 0.43607
Y = 0.22249
Z = 0.01392
```

### 在 Skia 中的使用

1. **创建色彩空间时**：作为 `fToXYZD50` 存储，同时惰性计算其逆矩阵 `fFromXYZD50`

2. **色彩空间转换时** (`gamutTransformTo`)：
   ```cpp
   void SkColorSpace::gamutTransformTo(const SkColorSpace* dst, skcms_Matrix3x3* src_to_dst) const {
       *src_to_dst = skcms_Matrix3x3_concat(&dst->fFromXYZD50, &fToXYZD50);
   }
   ```
   将源的 forward 矩阵与目标的逆矩阵连接，直接得到源到目标的变换矩阵。

3. **拦截优化**：`MakeRGB()` 工厂函数中，如果传入的三原色矩阵接近已知标准（如 sRGB、Adobe RGB、Display P3），会自动拦截到对应的单例或快路径。

---

## 二、传递函数

### 数据结构与公式

传递函数用 7 个参数 `{g, a, b, c, d, e, f}` 描述一条分段曲线：

```
当 |x| <  d:   linear = sign(x) * ( c*|x| + f )
当 |x| >= d:   linear = sign(x) * ((a*|x| + b)^g + e)
```

### 作用

1. **匹配人眼感知**：人眼对暗部敏感、对亮部不敏感。传递函数在暗部用更多编码值、亮部用更少，实现"感知均匀"的编码。

2. **优化存储精度**：8 位色彩（0-255）不做 gamma 压缩会有色阶断层，传递函数把精度集中到暗部。

3. **色彩空间转换的前置步骤**：所有色彩空间转换前必须先用传递函数将编码值线性化。

---

### 分类一：纯 Gamma（简单幂函数）

`b=c=d=e=f=0`，仅 `g` 和 `a` 参与运算：`linear = (a * encoded)^g`

| 名称 | g | a | 曲线 | 适用场景 |
|------|---|---|------|---------|
| `kLinear` | 1.0 | 1.0 | linear = encoded | 物理计算、HDR 中间格式 |
| `k2Dot2` | 2.2 | 1.0 | linear = encoded^2.2 | Adobe RGB, 传统显示器, A98 RGB |
| `kProPhotoRGB` | 1.8 | 1.0 | linear = encoded^1.8 | 摄影后期、超大色域 |
| `kRec470SystemM` | 2.2 | 1.0 | 同 k2Dot2 | 历史 NTSC 显示器 |
| `kRec470SystemBG` | 2.8 | 1.0 | linear = encoded^2.8 | 历史 PAL 显示器 |
| `kSMPTE_ST_428_1` | 2.6 | 1.034 | linear = (1.034*encoded)^2.6 | 数字影院 |

### 分类二：sRGB 类（分段曲线）

暗部使用线性段避免数值问题，亮部使用幂函数。有一个拐点 `d`。

**kSRGB** (`g=2.4, a≈0.9479, b≈0.0521, c≈0.0774, d=0.04045`)：

```
x <  0.04045:  linear = x / 12.92
x >= 0.04045:  linear = ((x + 0.055) / 1.055) ^ 2.4
```

整体约等价于 gamma 2.2 的视觉效果。

| 名称 | 别名 | 说明 |
|------|------|------|
| `kSRGB` | kIEC61966_2_1 | 最常用的 sRGB 曲线，互联网/操作系统默认 |
| `kRec709` | kRec601, kRec2020_10bit, kRec2020_12bit | HDTV 标准，与 sRGB 曲线非常接近 |
| `kRec2020` | | UHD/4K 色域，参数有细微差异 |
| `kSMPTE_ST_240` | | 早期 HDTV 标准 (SMPTE 240M) |

**sRGB vs Rec.709 的参数差异**：

```
kSRGB:   g=2.4,   a≈0.9479, b≈0.0521, c≈0.0774, d=0.04045
kRec709: g≈2.222, a≈0.9097, b≈0.0903, c≈0.2222, d≈0.08124
```

数值不同但曲线形状非常接近，肉眼几乎无法区分。

### 分类三：HDR 传递函数（特殊数学形式）

利用 `g` 参数为负值触发 skcms 的特殊解析路径，走 PQ 或 HLG 专用数学公式。

| 名称 | g | 用途 | 亮度范围 |
|------|----|------|---------|
| `kPQ` | -2.0 | HDR10 / Dolby Vision，感知量化编码 | 0-10000 nits |
| `kHLG` | -3.0 | HDR 广播电视，混合对数 Gamma | 相对，兼容 SDR |

---

## 三、XYZ vs sRGB

| 维度 | XYZ (CIE 1931) | sRGB |
|------|---------------|------|
| 定位 | 参考空间、连接空间 | 工作空间、显示空间 |
| 设备无关性 | 设备无关 | 依赖特定三原色定义 |
| 色域覆盖 | 100% 可见颜色 | ~35% 可见颜色 |
| 三原色 | 虚拟数学原色 | 真实物理原色（红绿蓝） |
| 可显示性 | 不能直接显示 | 直接输出到屏幕 |
| 传递函数 | 线性 | Gamma ~2.2 (sRGB 分段曲线) |

**一句话总结**：XYZ 是所有颜色之间互译的"世界语"，sRGB 是其中最广泛使用的"方言"之一。