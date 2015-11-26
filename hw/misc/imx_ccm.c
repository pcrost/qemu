/*
 * IMX31 Clock Control Module
 *
 * Copyright (C) 2012 NICTA
 * Updated by Jean-Christophe Dubois <jcd@tribudubois.net>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 *
 * This is an abstract base class used to get a common interface to
 * retrieve the CCM frequencies from the various i.MX SOC.
 */

#include "hw/misc/imx_ccm.h"

#ifndef DEBUG_IMX_CCM
#define DEBUG_IMX_CCM 0
#endif

#define DPRINTF(fmt, args...) \
    do { \
        if (DEBUG_IMX_CCM) { \
            fprintf(stderr, "[%s]%s: " fmt , TYPE_IMX_CCM, \
                                             __func__, ##args); \
        } \
    } while (0)


uint32_t imx_ccm_get_clock_frequency(IMXCCMState *dev, IMXClk clock)
{
    IMXCCMClass *klass = IMX_GET_CLASS(dev);

    DPRINTF("Clock = %d)\n", clock);

    if (klass->get_clock_frequency) {
        return klass->get_clock_frequency(dev, clock);
    } else {
        return 0;
    }
}

/*
 * Calculate PLL output frequency
 */
uint32_t imx_ccm_calc_pll(uint32_t pllreg, uint32_t base_freq)
{
    int32_t mfn = MFN(pllreg);  /* Numerator */
    uint32_t mfi = MFI(pllreg); /* Integer part */
    uint32_t mfd = 1 + MFD(pllreg); /* Denominator */
    uint32_t pd = 1 + PD(pllreg);   /* Pre-divider */

    DPRINTF("pllreg = %d, base_freq = %d)\n", pllreg, base_freq);

    if (mfi < 5) {
        mfi = 5;
    }

    /* mfn is 10-bit signed twos-complement */
    mfn <<= 32 - 10;
    mfn >>= 32 - 10;

    return ((2 * (base_freq >> 10) * (mfi * mfd + mfn)) /
            (mfd * pd)) << 10;
}

static const TypeInfo imx_ccm_info = {
    .name          = TYPE_IMX_CCM,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(IMXCCMState),
    .class_size    = sizeof(IMXCCMClass),
    .abstract      = true,
};

static void imx_ccm_register_types(void)
{
    type_register_static(&imx_ccm_info);
}

type_init(imx_ccm_register_types)
