/*
 * Allwinner A10 Clock Controller Module
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "hw/sysbus.h"
#include "hw/devices.h"
#include "sysemu/sysemu.h"
#include "hw/misc/allwinner-a10-ccm.h"

static uint64_t aw_a10_ccm_read(void *opaque, hwaddr offset, unsigned size)
{
    AwA10CCMState *s = opaque;

    return s->regs[offset/4];
}

static void aw_a10_ccm_write(void *opaque, hwaddr offset, uint64_t value,
                             unsigned size)
{
    AwA10CCMState *s = opaque;

    s->regs[offset/4] = value;
}

static const MemoryRegionOps aw_a10_ccm_ops = {
    .read = aw_a10_ccm_read,
    .write = aw_a10_ccm_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static const VMStateDescription vmstate_aw_a10_ccm = {
    .name = "a10.pic",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32_ARRAY(regs, AwA10CCMState, CCM_NUM_REGS),
        VMSTATE_END_OF_LIST()
    }
};

static void aw_a10_ccm_init(Object *obj)
{
    AwA10CCMState *s = AW_A10_CCM(obj);
    SysBusDevice *dev = SYS_BUS_DEVICE(obj);

     memory_region_init_io(&s->iomem, OBJECT(s), &aw_a10_ccm_ops, s,
                           TYPE_AW_A10_CCM, CCM_NUM_REGS * 4);
     sysbus_init_mmio(dev, &s->iomem);
}

static const uint32_t aw_a10_ccm_reset_values[CCM_NUM_REGS] = {
    /* 00 */ 0xa1005000, 0x0a101010, 0x08100010, 0x00000000,
    /* 10 */ 0x0010d063, 0x00000000, 0x21081000, 0x00000000,
    /* 20 */ 0xb1059491, 0x14888020, 0x21009911, 0x00000000,
    /* 30 */ 0x0010d063, 0x00000000, 0x00000000, 0x00000000,
    /* 40 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    /* 50 */ 0x00138013, 0x00020010, 0x00000000, 0x00000000,
    /* 60 */ 0x00004140, 0x00000000, 0x00000020, 0x00010001,
    /* 70 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    /* 80 */ 0x00000000, 0x00000000, 0x82000004, 0x00000000,
    /* 90 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    /* a0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    /* b0 */ 0x00000000, 0x00000000, 0x00000000, 0x00030000,
    /* c0 */ 0x00010000, 0x0000001f, 0x00000000, 0x00000000,
};

static void aw_a10_ccm_reset(DeviceState *d)
{
    AwA10CCMState *s = AW_A10_CCM(d);
    uint8_t i;

    for (i = 0; i < ARRAY_SIZE(aw_a10_ccm_reset_values); i++) {
        s->regs[i] = aw_a10_ccm_reset_values[i];
    }
}

static void aw_a10_ccm_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->reset = aw_a10_ccm_reset;
    dc->desc = "allwinner a10 ccm";
    dc->vmsd = &vmstate_aw_a10_ccm;
 }

static const TypeInfo aw_a10_ccm_info = {
    .name = TYPE_AW_A10_CCM,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(AwA10CCMState),
    .instance_init = aw_a10_ccm_init,
    .class_init = aw_a10_ccm_class_init,
};

static void aw_a10_register_types(void)
{
    type_register_static(&aw_a10_ccm_info);
}

type_init(aw_a10_register_types);
