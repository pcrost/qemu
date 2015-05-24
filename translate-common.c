/*
 *  Host code generation common components
 *
 *  Copyright (c) 2015 Peter Crosthwaite <crosthwaite.peter@gmail.com>
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

#include "qemu-common.h"
#include "qom/cpu.h"

uintptr_t qemu_real_host_page_size;
uintptr_t qemu_real_host_page_mask;

#ifndef CONFIG_USER_ONLY
/* mask must never be zero, except for A20 change call */
static void tcg_handle_interrupt(CPUState *cpu, int mask)
{
    int old_mask;

    old_mask = cpu->interrupt_request;
    cpu->interrupt_request |= mask;

    /*
     * If called from iothread context, wake the target cpu in
     * case its halted.
     */
    if (!qemu_cpu_is_self(cpu)) {
        qemu_cpu_kick(cpu);
        return;
    }

    if (use_icount) {
        cpu->icount_decr.u16.high = 0xffff;
        if (!cpu->can_do_io
            && (mask & ~old_mask) != 0) {
            cpu_abort(cpu, "Raised interrupt while not in I/O function");
        }
    } else {
        cpu->tcg_exit_req = 1;
    }
}

CPUInterruptHandler cpu_interrupt_handler = tcg_handle_interrupt;
#endif

typedef struct TCGExecInitFn {
    void (*do_tcg_exec_init)(unsigned long tb_size);
    QLIST_ENTRY(TCGExecInitFn) list;
} TCGExecInitFn;

static QLIST_HEAD(, TCGExecInitFn) tcg_exec_init_list;

void tcg_exec_init_add(void (*fn)(unsigned long))
{
    static bool inited;
    TCGExecInitFn *lelem = g_malloc0(sizeof *lelem);

    if (!inited) {
        inited = true;
        QLIST_INIT(&tcg_exec_init_list);
    }

    lelem->do_tcg_exec_init = fn;
    QLIST_INSERT_HEAD(&tcg_exec_init_list, lelem, list);
}

void tcg_exec_init(unsigned long tb_size)
{
    TCGExecInitFn *t;

    QLIST_FOREACH(t, &tcg_exec_init_list, list) {
        t->do_tcg_exec_init(tb_size);
    }
}
