/*
 * IMX31 Clock Control Module
 *
 * Copyright (C) 2012 NICTA
 * Updated by Jean-Christophe Dubois <jcd@tribudubois.net>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 *
 * To get the timer frequencies right, we need to emulate at least part of
 * the i.MX31 CCM.
 */

#include "hw/misc/imx31_ccm.h"

#define CKIH_FREQ 26000000 /* 26MHz crystal input */

#ifndef DEBUG_IMX31_CCM
#define DEBUG_IMX31_CCM 0
#endif

#define DPRINTF(fmt, args...) \
    do { \
        if (DEBUG_IMX31_CCM) { \
            fprintf(stderr, "[%s]%s: " fmt , TYPE_IMX31_CCM, \
                                             __func__, ##args); \
        } \
    } while (0)

static char const *imx31_ccm_reg_name(uint32_t reg)
{
    switch (reg) {
    case 0: /* CCMR */
        return "CCMR";
    case 1:
        return "PDR0";
    case 2:
        return "PDR1";
    case 4:
        return "MPCTL";
    case 6:
        return "SPCTL";
    case 8:
        return "CGR0";
    case 9:
        return "CGR1";
    case 10:
        return "CGR2";
    case 18:
        return "LTR1";
    case 23:
        return "PMCR0";
    default:
        return "???";
    }
}

static const VMStateDescription vmstate_imx31_ccm = {
    .name = TYPE_IMX31_CCM,
    .version_id = 2,
    .minimum_version_id = 2,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32_ARRAY(regs, IMX31CCMState, IMX31_CCM_NUM_REGS),
        VMSTATE_END_OF_LIST()
    },
};

static uint32_t imx31_ccm_get_ref_clk(IMXCCMState *dev)
{
    IMX31CCMState *s = IMX31_CCM(dev);

    DPRINTF("()\n");

    if ((s->ccmr & CCMR_PRCS) == 2) {
        return CKIL_FREQ * 1024;
    } else {
        return CKIH_FREQ;
    }
}

static uint32_t imx31_ccm_get_mcu_clk(IMXCCMState *dev)
{
    IMX31CCMState *s = IMX31_CCM(dev);

    DPRINTF("()\n");

    if ((s->ccmr & CCMR_MDS) || !(s->ccmr & CCMR_MPE)) {
        return imx31_ccm_get_ref_clk(dev);
    } else {
        return imx_ccm_calc_pll(s->mpctl, imx31_ccm_get_ref_clk(dev));
    }
}

static uint32_t imx31_ccm_get_hsp_clk(IMXCCMState *dev)
{
    IMX31CCMState *s = IMX31_CCM(dev);

    DPRINTF("()\n");

    return imx31_ccm_get_mcu_clk(dev) / (1 + EXTRACT(s->pdr0, HSP));
}

static uint32_t imx31_ccm_get_ipg_clk(IMXCCMState *dev)
{
    IMX31CCMState *s = IMX31_CCM(dev);

    DPRINTF("()\n");

    return imx31_ccm_get_hsp_clk(dev) / (1 + EXTRACT(s->pdr0, IPG));
}

static uint32_t imx31_ccm_get_clock_frequency(IMXCCMState *dev, IMXClk clock)
{
    uint32_t freq = 0;

    switch (clock) {
    case NOCLK:
        break;
    case CLK_MCU:
        freq = imx31_ccm_get_mcu_clk(dev);
        break;
    case CLK_HSP:
        freq = imx31_ccm_get_hsp_clk(dev);
        break;
    case CLK_IPG:
        freq = imx31_ccm_get_ipg_clk(dev);
        break;
    case CLK_32k:
        freq = CKIL_FREQ;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "[%s]%s: unsupported clock %d\n",
                      TYPE_IMX31_CCM, __func__, clock);
        break;
    }

    DPRINTF("Clock = %d) = %d\n", clock, freq);

    return freq;
}

static void imx31_ccm_reset(DeviceState *dev)
{
    IMX31CCMState *s = IMX31_CCM(dev);

    DPRINTF("()\n");

    memset(s->regs, 0, sizeof(s->regs));
    s->ccmr    = 0x074b0b7b;
    s->pdr0    = 0xff870b48;
    s->pdr1    = 0x49fcfe7f;
    s->rcsr    = 0x007f0000;
    s->mpctl   = PLL_PD(1) | PLL_MFD(0) | PLL_MFI(6)   | PLL_MFN(0);
    s->upctl   = PLL_PD(1) | PLL_MFD(5) | PLL_MFI(7)   | PLL_MFN(3);
    s->spctl   = PLL_PD(1) | PLL_MFD(4) | PLL_MFI(0xc) | PLL_MFN(1);
    s->cosr    = 0x00000280;
    s->cgr[0]  = s->cgr[1] = s->cgr[2] = 0xffffffff;
    s->wimr    = 0xffffffff;
    s->ldc     = 0x0000000f;
    s->ltr[1]  = 0x00004040;
    s->pmcr[0] = 0x80209828;
    s->pmcr[1] = 0x00aa0000;
    s->pdr2    = 0x00000285;
}

static uint64_t imx31_ccm_read(void *opaque, hwaddr offset, unsigned size)
{
    uint32 value = 0;
    IMX31CCMState *s = (IMX31CCMState *)opaque;
    int reg = offset >> 2;

    if (reg < IMX31_CCM_NUM_REGS) {
        value = s->regs[reg];
    } else {
        qemu_log_mask(LOG_GUEST_ERROR, "[%s]%s: Bad register at offset 0x%"
                      HWADDR_PRIx "\n", TYPE_IMX31_CCM, __func__, offset);
    }

    DPRINTF("reg[%s] => 0x%" PRIx32 "\n", imx31_ccm_reg_name(offset >> 2),
            value);

    return (uint64_t)value;
}

static void imx31_ccm_write(void *opaque, hwaddr offset, uint64_t value,
                            unsigned size)
{
    IMX31CCMState *s = (IMX31CCMState *)opaque;

    DPRINTF("reg[%s] <= 0x%" PRIx32 "\n", imx31_ccm_reg_name(offset >> 2),
            (uint32_t)value);
    int reg = offset >> 2;

    switch (reg) {
    case 0:
        s->ccmr = CCMR_FPMF | (value & 0x3b6fdfff);
        break;
    case 1:
        s->pdr0 = value & 0xff9f3fff;
        break;
    case 4:
        s->mpctl = value & 0xbfff3fff;
        break;
    case 6:
        s->spctl = value & 0xbfff3fff;
        break;
    case 18: /* LTR 1 */
        break;
    default:
        if (reg < IMX31_CCM_NUM_REGS) {
            s->regs[reg] = value;
            break;
        }
        qemu_log_mask(LOG_GUEST_ERROR, "[%s]%s: Bad register at offset 0x%"
                      HWADDR_PRIx "\n", TYPE_IMX31_CCM, __func__, offset);
    }
}

static const struct MemoryRegionOps imx31_ccm_ops = {
    .read = imx31_ccm_read,
    .write = imx31_ccm_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        /*
         * Our device would not work correctly if the guest was doing
         * unaligned access. This might not be a limitation on the real
         * device but in practice there is no reason for a guest to access
         * this device unaligned.
         */
        .min_access_size = 4,
        .max_access_size = 4,
        .unaligned = false,
    },

};

static void imx31_ccm_init(Object *obj)
{
    DeviceState *dev = DEVICE(obj);
    SysBusDevice *sd = SYS_BUS_DEVICE(obj);
    IMX31CCMState *s = IMX31_CCM(obj);

    memory_region_init_io(&s->iomem, OBJECT(dev), &imx31_ccm_ops, s,
                          TYPE_IMX31_CCM, 0x1000);
    sysbus_init_mmio(sd, &s->iomem);
}

static void imx31_ccm_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    IMXCCMClass *ccm = IMX_CCM_CLASS(klass);

    dc->reset = imx31_ccm_reset;
    dc->vmsd = &vmstate_imx31_ccm;
    dc->desc = "i.MX31 Clock Control Module";

    ccm->get_clock_frequency = imx31_ccm_get_clock_frequency;
}

static const TypeInfo imx31_ccm_info = {
    .name          = TYPE_IMX31_CCM,
    .parent        = TYPE_IMX_CCM,
    .instance_size = sizeof(IMX31CCMState),
    .instance_init = imx31_ccm_init,
    .class_init    = imx31_ccm_class_init,
};

static void imx31_ccm_register_types(void)
{
    type_register_static(&imx31_ccm_info);
}

type_init(imx31_ccm_register_types)
