#!/usr/bin/env python3
"""
Plot the Rec.2020 (ITU-R BT.2020) transfer function.

Rec.2020 SDR uses the same transfer function as Rec.709 (ITU-R BT.709).

The 7-parameter piecewise function (from Skia kRec2020 / kRec709):
    g = 20/9       ≈ 2.22222
    a = 0.909672
    b = 0.0903276
    c = 1/4.5      ≈ 0.222222
    d = 0.0812429
    e = 0
    f = 0

EOTF (encoded -> linear):
    linear = c * encoded                              , encoded <  d
    linear = (a * encoded + b)^g                      , encoded >= d

OETF (linear -> encoded):
    encoded = linear / c                              , linear <  c*d
    encoded = ((linear)^(1/g) - b) / a                , linear >= c*d

The two segments meet at:
    encoded = d = 0.08124   →   linear = c*d ≈ 0.01805
"""

import numpy as np
import matplotlib.pyplot as plt

# ── Skia source parameters (SkColorSpace.h kRec709 / kRec2020_10bit) ──

G = 20.0 / 9.0          # 2.22222...
A = 0.909672            # from Skia fixed-point: 0.909672415686
B = 0.0903276           # from Skia fixed-point: 0.090327584314
C = 1.0 / 4.5           # 0.22222...
D = 0.0812429           # threshold from Skia
# e = 0, f = 0

THRESHOLD_LINEAR = C * D  # ≈ 0.018054

# ── Rec.2020 transfer functions ─────────────────────────────────────────

def rec2020_eotf(x: np.ndarray) -> np.ndarray:
    """Rec.2020 EOTF: map [0, 1] non-linear encoded values to linear values."""
    return np.where(
        x < D,
        C * x,
        np.power(A * x + B, G),
    )


def rec2020_oetf(x: np.ndarray) -> np.ndarray:
    """Rec.2020 OETF (inverse): map [0, 1] linear values to non-linear encoded values."""
    return np.where(
        x < THRESHOLD_LINEAR,
        x / C,
        (np.power(x, 1.0 / G) - B) / A,
    )


def srgb_eotf(x: np.ndarray) -> np.ndarray:
    """sRGB EOTF for comparison."""
    return np.where(
        x < 0.04045,
        x / 12.92,
        np.power((x + 0.055) / 1.055, 2.4),
    )


# ── data ───────────────────────────────────────────────────────────────

x = np.linspace(0, 1, 500)
y_eotf = rec2020_eotf(x)
y_oetf = rec2020_oetf(x)
y_srgb_eotf = srgb_eotf(x)

# pure gamma 20/9 for comparison
y_gamma20_9 = np.power(x, G)

# ── plot ───────────────────────────────────────────────────────────────

fig, axes = plt.subplots(1, 2, figsize=(14, 5.5))

# --- subplot 1: EOTF (encoded -> linear) ---
ax = axes[0]
ax.plot(x, y_eotf, color="#ff9500", linewidth=2, label="Rec.2020 EOTF")
ax.plot(x, y_srgb_eotf, color="#007aff", linewidth=1.2, linestyle="--",
        label="sRGB EOTF (for reference)")
ax.axvline(D, color="#ff3b30", linestyle=":", linewidth=1.2,
           label=f"d = {D:.5f}")
ax.axhline(THRESHOLD_LINEAR, color="#ff3b30", linestyle=":", linewidth=0.8)

# mark the transition point
ax.plot(D, THRESHOLD_LINEAR, "o", color="#ff3b30", markersize=5, zorder=5)

ax.set_title("Rec.2020 EOTF (encoded → linear)", fontsize=12)
ax.set_xlabel("encoded value (non-linear)")
ax.set_ylabel("linear value")
ax.legend(fontsize=8.5)
ax.set_xlim(0, 1)
ax.set_ylim(0, 1)
ax.grid(True, alpha=0.3)

# --- subplot 2: OETF (linear -> encoded) ---
ax = axes[1]
ax.plot(x, y_oetf, color="#34c759", linewidth=2, label="Rec.2020 OETF")
ax.plot(x, x, color="#7f7f7f", linewidth=1, linestyle="--", alpha=0.5,
        label="identity (linear)")
ax.axvline(THRESHOLD_LINEAR, color="#ff3b30", linestyle=":", linewidth=1.2,
           label=f"linear thresh. = {THRESHOLD_LINEAR:.5f}")

ax.set_title("Rec.2020 OETF (linear → encoded)", fontsize=12)
ax.set_xlabel("linear value")
ax.set_ylabel("encoded value (non-linear)")
ax.legend(fontsize=8.5)
ax.set_xlim(0, 1)
ax.set_ylim(0, 1)
ax.grid(True, alpha=0.3)

fig.suptitle("Rec.2020 Transfer Function (ITU-R BT.2020 / same as Rec.709)",
             fontsize=14, fontweight="bold")
plt.tight_layout()
plt.savefig("Rec2020_transfer_function.png", dpi=150, bbox_inches="tight")
plt.show()