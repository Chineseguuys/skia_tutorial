#!/usr/bin/env python3
"""
Plot the Display P3 transfer function.

Display P3 uses the exact same transfer function as sRGB (IEC 61966-2-1).
The only difference is the color primaries — Display P3 uses DCI-P3 primaries
with a D65 white point, giving it a wider color gamut than sRGB.

EOTF (encoded -> linear):
    linear = encoded / 12.92                    , 0   <= encoded <  0.04045
    linear = ((encoded + 0.055) / 1.055) ^ 2.4  , encoded       >= 0.04045

OETF (linear -> encoded):
    encoded = linear * 12.92                             , 0   <= linear <  0.0031308
    encoded = 1.055 * (linear)^(1/2.4) - 0.055           , linear       >= 0.0031308
"""

import numpy as np
import matplotlib.pyplot as plt

# ── Display P3 transfer function (identical to sRGB) ───────────────────

def displayp3_eotf(x: np.ndarray) -> np.ndarray:
    """Display P3 EOTF: map [0, 1] non-linear encoded values to linear values."""
    y = np.where(
        x < 0.04045,
        x / 12.92,
        np.power((x + 0.055) / 1.055, 2.4),
    )
    return y


def displayp3_oetf(x: np.ndarray) -> np.ndarray:
    """Display P3 OETF (inverse): map [0, 1] linear values to non-linear encoded values."""
    y = np.where(
        x < 0.0031308,
        x * 12.92,
        1.055 * np.power(x, 1.0 / 2.4) - 0.055,
    )
    return y


# ── data ───────────────────────────────────────────────────────────────

x = np.linspace(0, 1, 500)
y_eotf = displayp3_eotf(x)
y_oetf = displayp3_oetf(x)

# pure gamma 2.2 for comparison
y_gamma22 = np.power(x, 2.2)

# ── skcms 7-parameter representation ───────────────────────────────────
# Display P3 (same as sRGB):
#   g = 2.4
#   a = 1 / 1.055  ≈ 0.9478673
#   b = 0.055 / 1.055 ≈ 0.0521327
#   c = 1 / 12.92   ≈ 0.0773994
#   d = 0.04045
#   e = 0
#   f = 0

# ── plot ───────────────────────────────────────────────────────────────

fig, axes = plt.subplots(1, 2, figsize=(14, 5.5))

# --- subplot 1: EOTF (encoded -> linear) ---
ax = axes[0]
ax.plot(x, y_eotf, color="#007aff", linewidth=2, label="Display P3 EOTF")
ax.plot(x, y_gamma22, color="#7f7f7f", linewidth=1.2, linestyle="--",
        label=r"pure $\gamma = 2.2$ (for reference)")
ax.axvline(0.04045, color="#ff9500", linestyle=":", linewidth=1, label="d = 0.04045")
ax.axhline(0.0031308, color="#ff9500", linestyle=":", linewidth=0.8)

ax.set_title("Display P3 EOTF (encoded → linear)", fontsize=12)
ax.set_xlabel("encoded value (non-linear)")
ax.set_ylabel("linear value")
ax.legend(fontsize=8.5)
ax.set_xlim(0, 1)
ax.set_ylim(0, 1)
ax.grid(True, alpha=0.3)

# --- subplot 2: OETF (linear -> encoded) ---
ax = axes[1]
ax.plot(x, y_oetf, color="#34c759", linewidth=2, label="Display P3 OETF")
ax.plot(x, x, color="#7f7f7f", linewidth=1, linestyle="--", alpha=0.5,
        label="identity (linear)")
ax.axvline(0.0031308, color="#ff9500", linestyle=":", linewidth=1,
           label="linear thresh. = 0.0031308")

ax.set_title("Display P3 OETF (linear → encoded)", fontsize=12)
ax.set_xlabel("linear value")
ax.set_ylabel("encoded value (non-linear)")
ax.legend(fontsize=8.5)
ax.set_xlim(0, 1)
ax.set_ylim(0, 1)
ax.grid(True, alpha=0.3)

fig.suptitle("Display P3 Transfer Function (same as sRGB EOTF/OETF)",
             fontsize=14, fontweight="bold")
plt.tight_layout()
plt.savefig("DisplayP3_transfer_function.png", dpi=150, bbox_inches="tight")
plt.show()