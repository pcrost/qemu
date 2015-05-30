/*
 * common defines for all CPUs
 *
 * Copyright (c) 2003 Fabrice Bellard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

/* No multiple include guard intended. Multi-arch setups may require multiple
 * cpu.h's included which means this can be and should be reached twice.
 */

#ifndef NEED_CPU_H
#error cpu.h included from common code
#endif

#include "config.h"
#include <inttypes.h>
#include "qemu/osdep.h"
#include "qemu/queue.h"
#include "tcg-target.h"
#ifndef CONFIG_USER_ONLY
#include "exec/hwaddr.h"
#endif
#include "exec/memattrs.h"

#include "exec/target-long.h"

#undef CPU_COMMON
#undef CPU_COMMON_TLB

#if !defined(CONFIG_USER_ONLY)

#undef CPU_VTLB_SIZE
/* use a fully associative victim tlb of 8 entries */
#define CPU_VTLB_SIZE 8

#undef CPU_TLB_ENTRY_BITS
#if HOST_LONG_BITS == 32 && TARGET_LONG_BITS == 32
#define CPU_TLB_ENTRY_BITS 4
#else
#define CPU_TLB_ENTRY_BITS 5
#endif

#undef CPU_TLB_ENTRY_SIZE
#define CPU_TLB_ENTRY_SIZE (1 << (CPU_TLB_ENTRY_BITS))

/* TCG_TARGET_TLB_DISPLACEMENT_BITS is used in CPU_TLB_BITS to ensure that
 * the TLB is not unnecessarily small, but still small enough for the
 * TLB lookup instruction sequence used by the TCG target.
 *
 * TCG will have to generate an operand as large as the distance between
 * env and the tlb_table[NB_MMU_MODES - 1][0].addend.  For simplicity,
 * the TCG targets just round everything up to the next power of two, and
 * count bits.  This works because: 1) the size of each TLB is a largish
 * power of two, 2) and because the limit of the displacement is really close
 * to a power of two, 3) the offset of tlb_table[0][0] inside env is smaller
 * than the size of a TLB.
 *
 * For example, the maximum displacement 0xFFF0 on PPC and MIPS, but TCG
 * just says "the displacement is 16 bits".  TCG_TARGET_TLB_DISPLACEMENT_BITS
 * then ensures that tlb_table at least 0x8000 bytes large ("not unnecessarily
 * small": 2^15).  The operand then will come up smaller than 0xFFF0 without
 * any particular care, because the TLB for a single MMU mode is larger than
 * 0x10000-0xFFF0=16 bytes.  In the end, the maximum value of the operand
 * could be something like 0xC000 (the offset of the last TLB table) plus
 * 0x18 (the offset of the addend field in each TLB entry) plus the offset
 * of tlb_table inside env (which is non-trivial but not huge).
 */

#undef CPU_TLB_BITS
#undef CPU_TLB_SIZE

#define CPU_TLB_BITS                                             \
    MIN(8,                                                       \
        TCG_TARGET_TLB_DISPLACEMENT_BITS - CPU_TLB_ENTRY_BITS -  \
        (NB_MMU_MODES <= 1 ? 0 :                                 \
         NB_MMU_MODES <= 2 ? 1 :                                 \
         NB_MMU_MODES <= 4 ? 2 :                                 \
         NB_MMU_MODES <= 8 ? 3 : 4))

#define CPU_TLB_SIZE (1 << CPU_TLB_BITS)

/* CPUIOTLBEntry is not arch variable. So don't multi include it */

#ifndef HAVE_CPU_IO_TLB_ENTRY_DEF
#define HAVE_CPU_IO_TLB_ENTRY_DEF

/* The IOTLB is not accessed directly inline by generated TCG code,
 * so the CPUIOTLBEntry layout is not as critical as that of the
 * CPUTLBEntry. (This is also why we don't want to combine the two
 * structs into one.)
 */
typedef struct CPUIOTLBEntry {
    hwaddr addr;
    MemTxAttrs attrs;
} CPUIOTLBEntry;

#endif

#ifndef TARGET_MULTI

typedef struct CPUTLBEntry {
    /* bit TARGET_LONG_BITS to TARGET_PAGE_BITS : virtual address
       bit TARGET_PAGE_BITS-1..4  : Nonzero for accesses that should not
                                    go directly to ram.
       bit 3                      : indicates that the entry is invalid
       bit 2..0                   : zero
    */
    union {
        struct {
            target_ulong addr_read;
            target_ulong addr_write;
            target_ulong addr_code;
            /* Addend to virtual address to get host address.  IO accesses
               use the corresponding iotlb value.  */
            uintptr_t addend;
        };
        /* padding to get a power of two size */
        uint8_t dummy[1 << CPU_TLB_ENTRY_BITS];
    };
} CPUTLBEntry;

QEMU_BUILD_BUG_ON(sizeof(CPUTLBEntry) != (CPU_TLB_ENTRY_SIZE));

#define CPU_COMMON_TLB_ENTRY                                                \
    CPUTLBEntry tlb_table[NB_MMU_MODES][CPU_TLB_SIZE];                      \
    CPUTLBEntry tlb_v_table[NB_MMU_MODES][CPU_VTLB_SIZE];
#else
#define CPU_COMMON_TLB_ENTRY                                                \
    uint8_t tlb_table[NB_MMU_MODES][CPU_TLB_SIZE][CPU_TLB_ENTRY_SIZE];      \
    uint8_t tlb_v_table[NB_MMU_MODES][CPU_VTLB_SIZE][CPU_TLB_ENTRY_SIZE];
#endif

#define CPU_COMMON_TLB \
    /* The meaning of the MMU modes is defined in the target code. */   \
    CPU_COMMON_TLB_ENTRY                                                \
    CPUIOTLBEntry iotlb[NB_MMU_MODES][CPU_TLB_SIZE];                    \
    CPUIOTLBEntry iotlb_v[NB_MMU_MODES][CPU_VTLB_SIZE];                 \
    target_ulong tlb_flush_addr;                                        \
    target_ulong tlb_flush_mask;                                        \
    target_ulong vtlb_index;                                            \

#else

#define CPU_COMMON_TLB

#endif


#define CPU_COMMON                                                      \
    /* soft mmu support */                                              \
    CPU_COMMON_TLB                                                      \

