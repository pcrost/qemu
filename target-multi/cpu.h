#ifndef MULTI_CPU_H
#define MULTI_CPU_H

#include "config.h"

#define TARGET_LONG_BITS 64
#define TARGET_PAGE_BITS 12 /* Thou shalt still use 4k pages only! */

#define CPUArchState void

#include "exec/target-long.h"
#include "exec/cpu-all.h"
#include "exec/exec-all.h"
#include "qom/cpu.h"

#endif
