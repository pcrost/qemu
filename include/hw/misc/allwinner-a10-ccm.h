#ifndef AW_A10_CCM_H
#define AW_A10_CCM_H

#define TYPE_AW_A10_CCM  "allwinner-a10-ccm"
#define AW_A10_CCM(obj) OBJECT_CHECK(AwA10CCMState, (obj), TYPE_AW_A10_CCM)

#define CCM_NUM_REGS    (0x200/4)

typedef struct AwA10CCMState {
    /*< private >*/
    SysBusDevice parent_obj;
    /*< public >*/
    MemoryRegion iomem;

    uint32_t regs[CCM_NUM_REGS];
} AwA10CCMState;

#endif
