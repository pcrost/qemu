/*
 * Undefine the standard macros defined by cpu.h which are used by core code.
 * Each arch cpu.h should include this before defining any of these symbols.
 * This is to allow system level code to include multiple arches cpu.h.
 *
 * Copyright (c) 2015 Peter Crosthwaite <crosthwaite.peter@gmail.com>
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

/* No multiple include guard intended. */

#undef CPUArchState

#undef ELF_MACHINE

#undef TARGET_LONG_BITS
#undef TARGET_PAGE_BITS
#undef TARGET_PHYS_ADDR_SPACE_BITS
#undef TARGET_VIRT_ADDR_SPACE_BITS

#undef NB_MMU_MODES
