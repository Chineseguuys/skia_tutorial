#!/usr/bin/env python3
"""
Plot the standard sRGB transfer function.

sRGB EOTF (encoded -> linear):
    linear = encoded / 12.92                    , 0   <= encoded <  0.04045
    linear = ((encoded + 0.055) / 1.055) ^ 2.4  , encoded       >= 0.04045

These two segments are designed to meet continuously at encoded = 0.04045,
producing linear ≈ 0.0031308 for both branches.
"""

import numpy as np
import matplotlib.pyplot as plt

# ── sRGB transfer function (non-linear encoded -> linear) ──────────────

def srgb_eotf(x: np.ndarray) -> np.ndarray:
    """sRGB EOTF: map [0, 1] non-linear encoded values to linear values."""
    y = np.where(
        x < 0.04045,
        x / 12.92,
        np.power((x + 0.055) / 1.055, 2.4),
    )
    return y


def srgb_oetf(x: np.ndarray) -> np.ndarray:
    """sRGB OETF (inverse): map [0, 1] linear values to non-linear encoded values."""
    y = np.where(
        x < 0.0031308,
        x * 12.92,
        1.055 * np.power(x, 1.0 / 2.4) - 0.055,
    )
    return y


# ── data ───────────────────────────────────────────────────────────────

x = np.linspace(0, 1, 500)
y_eotf = srgb_eotf(x)
y_oetf = srgb_oetf(x)

# pure gamma 2.2 for comparison
y_gamma22 = np.power(x, 2.2)

# ── plot ───────────────────────────────────────────────────────────────

fig, axes = plt.subplots(1, 2, figsize=(14, 5.5))

# --- subplot 1: EOTF (encoded -> linear) ---
ax = axes[0]
ax.plot(x, y_eotf, color="#d62728", linewidth=2, label="sRGB EOTF")
ax.plot(x, y_gamma22, color="#7f7f7f", linewidth=1.2, linestyle="--", label=r"pure $\gamma = 2.2$")
ax.axvline(0.04045, color="#1f77b4", linestyle=":", linewidth=1, label="d = 0.04045")
ax.axhline(0.0031308, color="#1f77b4", linestyle=":", linewidth=0.8)

ax.set_title("sRGB EOTF (encoded → linear)", fontsize=12)
ax.set_xlabel("encoded value (non-linear)")
ax.set_ylabel("linear value")
ax.legend(fontsize=9)
ax.set_xlim(0, 1)
ax.set_ylim(0, 1)
ax.grid(True, alpha=0.3)

# --- subplot 2: OETF (linear -> encoded) ---
ax = axes[1]
ax.plot(x, y_oetf, color="#2ca02c", linewidth=2, label="sRGB OETF")
ax.plot(x, x, color="#7f7f7f", linewidth=1, linestyle="--", alpha=0.5, label="identity (linear)")
ax.axvline(0.0031308, color="#1f77b4", linestyle=":", linewidth=1, label="d = 0.04045")

ax.set_title("sRGB OETF (linear → encoded)", fontsize=12)
ax.set_xlabel("linear value")
ax.set_ylabel("encoded value (non-linear)")
ax.legend(fontsize=9)
ax.set_xlim(0, 1)
ax.set_ylim(0, 1)
ax.grid(True, alpha=0.3)

fig.suptitle("Standard sRGB Transfer Function", fontsize=14, fontweight="bold")
plt.tight_layout()
plt.savefig("sRGB_transfer_function.png", dpi=150, bbox_inches="tight")
plt.show()