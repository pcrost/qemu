/*
 * IMX25 Clock Control Module
 *
 * Copyright (C) 2012 NICTA
 * Updated by Jean-Christophe Dubois <jcd@tribudubois.net>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 *
 * To get the timer frequencies right, we need to emulate at least part of
 * the CCM.
 */

#include "hw/misc/imx25_ccm.h"

#ifndef DEBUG_IMX25_CCM
#define DEBUG_IMX25_CCM 0
#endif

#define DPRINTF(fmt, args...) \
    do { \
        if (DEBUG_IMX25_CCM) { \
            fprintf(stderr, "[%s]%s: " fmt , TYPE_IMX25_CCM, \
                                             __func__, ##args); \
        } \
    } while (0)

#define CKIH_FREQ 24000000 /* 24MHz crystal input */

static const VMStateDescription vmstate_imx25_ccm = {
    .name = TYPE_IMX25_CCM,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(mpctl, IMX25CCMState),
        VMSTATE_UINT32(upctl, IMX25CCMState),
        VMSTATE_UINT32(cctl, IMX25CCMState),
        VMSTATE_UINT32_ARRAY(cgcr, IMX25CCMState, 3),
        VMSTATE_UINT32_ARRAY(pcdr, IMX25CCMState, 4),
        VMSTATE_UINT32(rcsr, IMX25CCMState),
        VMSTATE_UINT32(crdr, IMX25CCMState),
        VMSTATE_UINT32_ARRAY(dcvr, IMX25CCMState, 4),
        VMSTATE_UINT32_ARRAY(ltr, IMX25CCMState, 4),
        VMSTATE_UINT32_ARRAY(ltbr, IMX25CCMState, 2),
        VMSTATE_UINT32_ARRAY(pmcr, IMX25CCMState, 3),
        VMSTATE_UINT32(mcr, IMX25CCMState),
        VMSTATE_UINT32_ARRAY(lpimr, IMX25CCMState, 2),
        VMSTATE_END_OF_LIST()
    },
};

static uint32_t imx25_ccm_get_mpll_clk(IMXCCMState *dev)
{
    IMX25CCMState *s = IMX25_CCM(dev);

    DPRINTF("()\n");

    if (EXTRACT(s->cctl, MPLL_BYPASS)) {
        return CKIH_FREQ;
    } else {
        return imx_ccm_calc_pll(s->mpctl, CKIH_FREQ);
    }
}

static uint32_t imx25_ccm_get_upll_clk(IMXCCMState *dev)
{
    IMX25CCMState *s = IMX25_CCM(dev);

    DPRINTF("()\n");

    return imx_ccm_calc_pll(s->upctl, CKIH_FREQ);
}

static uint32_t imx25_ccm_get_mcu_clk(IMXCCMState *dev)
{
    IMX25CCMState *s = IMX25_CCM(dev);

    DPRINTF("()\n");

    if (EXTRACT(s->cctl, ARM_SRC)) {
        return (imx25_ccm_get_mpll_clk(dev) * 3 / 4) /
               (1 + EXTRACT(s->cctl, ARM_CLK_DIV));
    } else {
        return imx25_ccm_get_mpll_clk(dev) /
               (1 + EXTRACT(s->cctl, ARM_CLK_DIV));
    }
}

static uint32_t imx25_ccm_get_ahb_clk(IMXCCMState *dev)
{
    IMX25CCMState *s = IMX25_CCM(dev);

    DPRINTF("()\n");

    return imx25_ccm_get_mcu_clk(dev) / (1 + EXTRACT(s->cctl, AHB_CLK_DIV));
}

static uint32_t imx25_ccm_get_ipg_clk(IMXCCMState *dev)
{
    DPRINTF("()\n");

    return imx25_ccm_get_ahb_clk(dev) / 2;
}

static uint32_t imx25_ccm_get_clock_frequency(IMXCCMState *dev, IMXClk clock)
{
    uint32_t freq = 0;
    DPRINTF("Clock = %d)\n", clock);

    switch (clock) {
    case NOCLK:
        break;
    case CLK_MPLL:
        freq = imx25_ccm_get_mpll_clk(dev);
        break;
    case CLK_UPLL:
        freq = imx25_ccm_get_upll_clk(dev);
        break;
    case CLK_MCU:
        freq = imx25_ccm_get_mcu_clk(dev);
        break;
    case CLK_AHB:
        freq = imx25_ccm_get_ahb_clk(dev);
        break;
    case CLK_IPG:
        freq = imx25_ccm_get_ipg_clk(dev);
        break;
    case CLK_32k:
        freq = CKIL_FREQ;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "[%s]%s: unsupported clock %d\n",
                      TYPE_IMX25_CCM, __func__, clock);
        break;
    }

    DPRINTF("Clock = %d) = %d\n", clock, freq);

    return freq;
}

static void imx25_ccm_reset(DeviceState *dev)
{
    IMX25CCMState *s = IMX25_CCM(dev);

    DPRINTF("\n");

    s->mpctl = 0x800b2c01;
    s->upctl = 0x84002800;
    s->cctl = 0x40030000;
    s->cgcr[0] = 0x028A0100;
    s->cgcr[1] = 0x04008100;
    s->cgcr[2] = 0x00000438;
    s->pcdr[0] = 0x01010101;
    s->pcdr[1] = 0x01010101;
    s->pcdr[2] = 0x01010101;
    s->pcdr[3] = 0x01010101;
    s->rcsr = 0;
    s->crdr = 0;
    s->dcvr[0] = 0;
    s->dcvr[1] = 0;
    s->dcvr[2] = 0;
    s->dcvr[3] = 0;
    s->ltr[0] = 0;
    s->ltr[1] = 0;
    s->ltr[2] = 0;
    s->ltr[3] = 0;
    s->ltbr[0] = 0;
    s->ltbr[1] = 0;
    s->pmcr[0] = 0x00A00000;
    s->pmcr[1] = 0x0000A030;
    s->pmcr[2] = 0x0000A030;
    s->mcr = 0x43000000;
    s->lpimr[0] = 0;
    s->lpimr[1] = 0;

    /* default ROM boot will change the reset values */
    s->cctl |= INSERT(1, ARM_SRC);
    s->cctl |= INSERT(1, AHB_CLK_DIV);
}

static uint64_t imx25_ccm_read(void *opaque, hwaddr offset, unsigned size)
{
    uint32 value = 0;
    IMX25CCMState *s = (IMX25CCMState *)opaque;
    uint32_t *reg = &s->mpctl;

    DPRINTF("(offset=0x%" HWADDR_PRIx ")\n", offset);

    if (offset < 0x70) {
        value = reg[offset >> 2];
    } else {
        qemu_log_mask(LOG_GUEST_ERROR, "[%s]%s: Bad register at offset 0x%"
                      HWADDR_PRIx "\n", TYPE_IMX25_CCM, __func__, offset);
    }

    DPRINTF("reg[0x%" HWADDR_PRIx "] => 0x%" PRIx32 "\n", offset, value);

    return value;
}

static void imx25_ccm_write(void *opaque, hwaddr offset, uint64_t value,
                            unsigned size)
{
    IMX25CCMState *s = (IMX25CCMState *)opaque;
    uint32_t *reg = &s->mpctl;

    DPRINTF("reg[0x%" HWADDR_PRIx "] <= 0x%" PRIx32 "\n", offset,
            (uint32_t)value);

    if (offset < 0x70) {
        /*
         * We will do a better implementation later. In particular some bits
         * cannot be written to.
         */
        reg[offset >> 2] = value;
    } else {
        qemu_log_mask(LOG_GUEST_ERROR, "[%s]%s: Bad register at offset 0x%"
                      HWADDR_PRIx "\n", TYPE_IMX25_CCM, __func__, offset);
    }
}

static const struct MemoryRegionOps imx25_ccm_ops = {
    .read = imx25_ccm_read,
    .write = imx25_ccm_write,
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

static void imx25_ccm_init(Object *obj)
{
    DeviceState *dev = DEVICE(obj);
    SysBusDevice *sd = SYS_BUS_DEVICE(obj);
    IMX25CCMState *s = IMX25_CCM(obj);

    memory_region_init_io(&s->iomem, OBJECT(dev), &imx25_ccm_ops, s,
                          TYPE_IMX25_CCM, 0x1000);
    sysbus_init_mmio(sd, &s->iomem);
}

static void imx25_ccm_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    IMXCCMClass *ccm = IMX_CCM_CLASS(klass);

    dc->reset = imx25_ccm_reset;
    dc->vmsd = &vmstate_imx25_ccm;
    dc->desc = "i.MX25 Clock Control Module";

    ccm->get_clock_frequency = imx25_ccm_get_clock_frequency;
}

static const TypeInfo imx25_ccm_info = {
    .name          = TYPE_IMX25_CCM,
    .parent        = TYPE_IMX_CCM,
    .instance_size = sizeof(IMX25CCMState),
    .instance_init = imx25_ccm_init,
    .class_init    = imx25_ccm_class_init,
};

static void imx25_ccm_register_types(void)
{
    type_register_static(&imx25_ccm_info);
}

type_init(imx25_ccm_register_types)
