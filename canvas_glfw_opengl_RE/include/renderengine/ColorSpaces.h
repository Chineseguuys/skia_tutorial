#ifndef _COLOR_SPACES_H_
#define _COLOR_SPACES_H_

#include "ui/Dataspace.h"
#include "skia/include/core/SkColorSpace.h"

namespace renderengine {
namespace skia {

sk_sp<SkColorSpace> toSkColorSpace(ui::Dataspace dataspace) {
    skcms_Matrix3x3 gamut;
    switch (dataspace & ui::HAL_DATASPACE_STANDARD_MASK) {
        case ui::HAL_DATASPACE_STANDARD_BT709:
            gamut = SkNamedGamut::kSRGB;
            break;
        case ui::HAL_DATASPACE_STANDARD_BT2020:
            gamut = SkNamedGamut::kRec2020;
            break;
        case ui::HAL_DATASPACE_STANDARD_DCI_P3:
            gamut = SkNamedGamut::kDisplayP3;
            break;
        case ui::HAL_DATASPACE_STANDARD_ADOBE_RGB:
            gamut = SkNamedGamut::kAdobeRGB;
            break;
        case ui::HAL_DATASPACE_STANDARD_BT601_625:
        case ui::HAL_DATASPACE_STANDARD_BT601_625_UNADJUSTED:
        case ui::HAL_DATASPACE_STANDARD_BT601_525:
        case ui::HAL_DATASPACE_STANDARD_BT601_525_UNADJUSTED:
        case ui::HAL_DATASPACE_STANDARD_BT2020_CONSTANT_LUMINANCE:
        case ui::HAL_DATASPACE_STANDARD_BT470M:
        case ui::HAL_DATASPACE_STANDARD_FILM:
        case ui::HAL_DATASPACE_STANDARD_UNSPECIFIED:
        default:
            gamut = SkNamedGamut::kSRGB;
            break;
    }

    switch (dataspace & ui::HAL_DATASPACE_TRANSFER_MASK) {
        case ui::HAL_DATASPACE_TRANSFER_LINEAR:
            return SkColorSpace::MakeRGB(SkNamedTransferFn::kLinear, gamut);
        case ui::HAL_DATASPACE_TRANSFER_SRGB:
            return SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, gamut);
        case ui::HAL_DATASPACE_TRANSFER_GAMMA2_2:
            return SkColorSpace::MakeRGB({2.2f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}, gamut);
        case ui::HAL_DATASPACE_TRANSFER_GAMMA2_6:
            return SkColorSpace::MakeRGB({2.6f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}, gamut);
        case ui::HAL_DATASPACE_TRANSFER_GAMMA2_8:
            return SkColorSpace::MakeRGB({2.8f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}, gamut);
        case ui::HAL_DATASPACE_TRANSFER_ST2084:
            return SkColorSpace::MakeRGB({-2.f, -1.55522297832f, 1.86045365631f, 32 / 2523.0f,
                                          2413 / 128.0f, -2392 / 128.0f, 8192 / 1305.0f},
                                         gamut);
        case ui::HAL_DATASPACE_TRANSFER_SMPTE_170M:
            return SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020, gamut);
        // case ui::HAL_DATASPACE_TRANSFER_HLG:
        //     skcms_TransferFunction hlgFn;
        //     if (skcms_TransferFunction_makeScaledHLGish(&hlgFn, 0.314509843, 2.f, 2.f,
        //                                                 1.f / 0.17883277f, 0.28466892f,
        //                                                 0.55991073f)) {
        //         return SkColorSpace::MakeRGB(hlgFn, gamut);
        //     } else {
        //         return SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, gamut);
        //     }
        case ui::HAL_DATASPACE_TRANSFER_UNSPECIFIED:
        default:
            return SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, gamut);
    }
}


} // namespace skia
} // namespace renderengine


#endif // end _COLOR_SPACES_H_