/* MCS-51 ELF support for BFD.
   Copyright (C) 2001 Free Software Foundation, Inc.
   Contributed by Radek Benedikt <benedikt@lphard.cz>

This file is part of BFD, the Binary File Descriptor library.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef _ELF_I51_H
#define _ELF_I51_H

#include "elf/reloc-macros.h"

/* Relocations.  */
START_RELOC_NUMBERS (elf_i51_reloc_type)
     RELOC_NUMBER (R_I51_NONE,			0)
     RELOC_NUMBER (R_I51_R1,			1)
     RELOC_NUMBER (R_I51_R3,			2)
     RELOC_NUMBER (R_I51_7_PCREL,		3)
     RELOC_NUMBER (R_I51_11, 			4)
     RELOC_NUMBER (R_I51_8_BIT,			5)
     RELOC_NUMBER (R_I51_8,			6)
     RELOC_NUMBER (R_I51_L,			7)
     RELOC_NUMBER (R_I51_H,			8)
     RELOC_NUMBER (R_I51_16,			9)
     RELOC_NUMBER (R_I51_8_B2B,			10)
     RELOC_NUMBER (R_I51_13_PCODE,		11)
END_RELOC_NUMBERS (R_I51_max)

#define SHF_CDATA   0xE0000000 /* Processor-specific - common data mask */
#define SHF_REGBANK 0x20000000 /* Processor-specific - register bank */
#define SHF_RDATA   0x40000000 /* Processor-specific - rdata */
#define SHF_BDATA   0x60000000 /* Processor-specific - bdata */
#define SHF_IDATA   0x80000000 /* Processor-specific - idata */
#define SHF_XDATA   0xA0000000 /* Processor-specific - xdata */
#define SHF_EDATA   0xC0000000 /* Processor-specific - edata */

#define SHN_I51_REGBANK 0xff00 /* Register bank common */
#define SHN_I51_RDATA_C 0xff01 /* rdata common */
#define SHN_I51_BDATA_C 0xff02 /* bdata common */
#define SHN_I51_IDATA_C 0xff03 /* idata common */
#define SHN_I51_XDATA_C 0xff04 /* xdata common */
#define SHN_I51_EDATA_C 0xff05 /* edata common */
#define SHN_I51_BITDATA_C 0xff06 /* bitdata common */

#endif /* _ELF_I51_H */
