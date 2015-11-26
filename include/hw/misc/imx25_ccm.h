/*
 * IMX25 Clock Control Module
 *
 * Copyright (C) 2012 NICTA
 * Updated by Jean-Christophe Dubois <jcd@tribudubois.net>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#ifndef IMX25_CCM_H
#define IMX25_CCM_H

#include "hw/misc/imx_ccm.h"

/* CCTL */
#define CCTL_ARM_CLK_DIV_SHIFT (30)
#define CCTL_ARM_CLK_DIV_MASK  (0x3)
#define CCTL_AHB_CLK_DIV_SHIFT (28)
#define CCTL_AHB_CLK_DIV_MASK  (0x3)
#define CCTL_MPLL_BYPASS_SHIFT (22)
#define CCTL_MPLL_BYPASS_MASK  (0x1)
#define CCTL_USB_DIV_SHIFT (16)
#define CCTL_USB_DIV_MASK  (0x3F)
#define CCTL_ARM_SRC_SHIFT (13)
#define CCTL_ARM_SRC_MASK  (0x1)

#define EXTRACT(value, name) (((value) >> CCTL_##name##_SHIFT) \
                              & CCTL_##name##_MASK)
#define INSERT(value, name) (((value) & CCTL_##name##_MASK) << \
                             CCTL_##name##_SHIFT)

#define TYPE_IMX25_CCM "imx25.ccm"
#define IMX25_CCM(obj) OBJECT_CHECK(IMX25CCMState, (obj), TYPE_IMX25_CCM)

typedef struct IMX25CCMState {
    /* <private> */
    IMXCCMState parent_obj;

    /* <public> */
    MemoryRegion iomem;

    uint32_t mpctl;
    uint32_t upctl;
    uint32_t cctl;
    uint32_t cgcr[3];
    uint32_t pcdr[4];
    uint32_t rcsr;
    uint32_t crdr;
    uint32_t dcvr[4];
    uint32_t ltr[4];
    uint32_t ltbr[2];
    uint32_t pmcr[3];
    uint32_t mcr;
    uint32_t lpimr[2];

} IMX25CCMState;

#endif /* IMX25_CCM_H */
