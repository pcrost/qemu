/*
 * definition for the target_long type and friends.
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

/* No multiple included guard intended. Multi-arch setups may require multiple
 * cpu.h's included which means this can be and should be reached twice.
 */

#include <stdint.h>

#ifndef TARGET_LONG_BITS
#error TARGET_LONG_BITS must be defined before including this header
#endif

#undef TARGET_LONG_SIZE
#define TARGET_LONG_SIZE (TARGET_LONG_BITS / 8)

#undef target_long
#undef target_ulong
#undef TARGET_FMT_lx
#undef TARGET_FMT_ld
#undef TARGET_FMT_lu

/* target_ulong is the type of a virtual address */
#if TARGET_LONG_SIZE == 4
#define target_long int32_t
#define target_ulong uint32_t
#define TARGET_FMT_lx "%08x"
#define TARGET_FMT_ld "%d"
#define TARGET_FMT_lu "%u"
#elif TARGET_LONG_SIZE == 8
#define target_long int64_t
#define target_ulong uint64_t
#define TARGET_FMT_lx "%016" PRIx64
#define TARGET_FMT_ld "%" PRId64
#define TARGET_FMT_lu "%" PRIu64
#else
#error TARGET_LONG_SIZE undefined
#endif
