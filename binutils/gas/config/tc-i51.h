/* This file is tc-i51.h
   Copyright (C) 2001 Free Software Foundation, Inc.

   Contributed by Radek Benedikt <benedikt@lphard.cz>

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

//#ifndef BFD_ASSEMBLER
// #error MCS-51 support requires BFD_ASSEMBLER
//#endif

#define TC_I51
/*   By convention, you should define this macro in the `.h' file.  For
     example, `tc-m68k.h' defines `TC_M68K'.  You might have to use this
     if it is necessary to add CPU specific code to the object format
     file.  */

#define TARGET_FORMAT "elf32-i51"
/*   This macro is the BFD target name to use when creating the output
     file.  This will normally depend upon the `OBJ_FMT' macro.  */

#define TARGET_ARCH bfd_arch_i51
/*   This macro is the BFD architecture to pass to `bfd_set_arch_mach'.  */

#define TARGET_MACH 0
/*   This macro is the BFD machine number to pass to
     `bfd_set_arch_mach'.  If it is not defined, GAS will use 0.  */

#define TARGET_BYTES_BIG_ENDIAN 1
/*   You should define this macro to be non-zero if the target is big
     endian, and zero if the target is little endian.  */

#define ONLY_STANDARD_ESCAPES
/*   If you define this macro, GAS will warn about the use of
     nonstandard escape sequences in a string.  */

/* Support for SHF_REGBANK */
extern int i51_section_flags PARAMS ((int, int, int));

//#define md_elf_section_letter(LETTER, PTR_MSG)	i51_section_letter (LETTER, PTR_MSG)
//#define md_elf_section_word(STR, LEN)		i51_section_word (STR, LEN)
#define md_elf_section_flags(FLAGS, ATTR, TYPE)	i51_section_flags (FLAGS, ATTR, TYPE)

#define md_operand(x)
/*   GAS will call this function for any expression that can not be
     recognized.  When the function is called, `input_line_pointer'
     will point to the start of the expression.  */

void i51_parse_cons_expression (expressionS *exp, int nbytes);
#define TC_PARSE_CONS_EXPRESSION(EXPR,N) i51_parse_cons_expression (EXPR,N)
/*
     You may define this macro to parse an expression used in a data
     allocation pseudo-op such as `.word'.  You can use this to
     recognize relocation directives that may appear in such directives.*/



void i51_cons_fix_new(fragS *frag,int where, int nbytes, expressionS *exp);
#define TC_CONS_FIX_NEW(FRAG,WHERE,N,EXP) i51_cons_fix_new(FRAG,WHERE,N,EXP)
/*   You may define this macro to generate a fixup for a data
     allocation pseudo-op.  */

#define md_number_to_chars number_to_chars_bigendian
/*   This should just call either `number_to_chars_bigendian' or
     `number_to_chars_littleendian', whichever is appropriate.  On
     targets like the MIPS which support options to change the
     endianness, which function to call is a runtime decision.  On
     other targets, `md_number_to_chars' can be a simple macro.  */

#define WORKING_DOT_WORD
/*
`md_short_jump_size'
`md_long_jump_size'
`md_create_short_jump'
`md_create_long_jump'
     If `WORKING_DOT_WORD' is defined, GAS will not do broken word
     processing (*note Broken words::.).  Otherwise, you should set
     `md_short_jump_size' to the size of a short jump (a jump that is
     just long enough to jump around a long jmp) and
     `md_long_jump_size' to the size of a long jump (a jump that can go
     anywhere in the function), You should define
     `md_create_short_jump' to create a short jump around a long jump,
     and define `md_create_long_jump' to create a long jump.  */

#define MD_APPLY_FIX3

//#define TC_HANDLES_FX_DONE

//#undef RELOC_EXPANSION_POSSIBLE
/*   If you define this macro, it means that `tc_gen_reloc' may return
     multiple relocation entries for a single fixup.  In this case, the
     return value of `tc_gen_reloc' is a pointer to a null terminated
     array.  */

#define MD_PCREL_FROM_SECTION(FIXP, SEC) md_pcrel_from_section(FIXP, SEC)
/*   If you define this macro, it should return the offset between the
     address of a PC relative fixup and the position from which the PC
     relative adjustment should be made.  On many processors, the base
     of a PC relative instruction is the next instruction, so this
     macro would return the length of an instruction.  */

long md_pcrel_from_section PARAMS ((struct fix *, segT));

/* Specific sections:
     bit register area */
#define ELF_TC_SPECIAL_SECTIONS \
  /* internal (onchip) memory */ \
  /* 0x00..0x1F register, direct, indirect addresable */ \
  { ".rdata",	SHT_PROGBITS,	SHF_ALLOC + SHF_WRITE }, \
  { ".rbss",	SHT_NOBITS,	SHF_ALLOC + SHF_WRITE }, \
  /* 0x20..0x2F bit, direct, indirect  addressable */ \
  { ".bdata",	SHT_PROGBITS,	SHF_ALLOC + SHF_WRITE }, \
  { ".bbss",	SHT_NOBITS,	SHF_ALLOC + SHF_WRITE }, \
  /* 0x30..0x7F direct, indirect addresable */ \
  /* * *   standart section   * * */ \
  /* .data */ \
  /* .bss */ \
  /* 0x80..0xFF indirect addressable */ \
  { ".idata",	SHT_PROGBITS,	SHF_ALLOC + SHF_WRITE }, \
  { ".ibss",	SHT_NOBITS,	SHF_ALLOC + SHF_WRITE }, \
  /* external (but in some case onchip) memory */ \
  /* 0x0000..0xFFFF external memory, movx addressable */ \
  { ".xdata",	SHT_PROGBITS,	SHF_ALLOC + SHF_WRITE }, \
  { ".xbss",	SHT_NOBITS,	SHF_ALLOC + SHF_WRITE }, \
  /* 0x0000..0xFFFF optional onchip "external" memory, movx addressable */ \
  { ".edata",	SHT_PROGBITS,	SHF_ALLOC + SHF_WRITE }, \
  { ".ebss",	SHT_NOBITS,	SHF_ALLOC + SHF_WRITE }, \
  /* 0x0000..0xFFFF optionla onchip eeprom memory, movx addressable */ \
  { ".eeprom",	SHT_PROGBITS,	SHF_ALLOC + SHF_WRITE },

#define LISTING_WORD_SIZE 1
/*   The number of bytes to put into a word in a listing.  This affects
     the way the bytes are clumped together in the listing.  For
     example, a value of 2 might print `1234 5678' where a value of 1
     would print `12 34 56 78'.  The default value is 4.  */

//#define LEX_DOLLAR 0
/* AVR port uses `$' as a logical line separator */

//#define TC_IMPLICIT_LCOMM_ALIGNMENT(SIZE, P2VAR) (P2VAR) = 0
/*   An `.lcomm' directive with no explicit alignment parameter will
     use this macro to set P2VAR to the alignment that a request for
     SIZE bytes will have.  The alignment is expressed as a power of
     two.  If no alignment should take place, the macro definition
     should do nothing.  Some targets define a `.bss' directive that is
     also affected by this macro.  The default definition will set
     P2VAR to the truncated power of two of sizes up to eight bytes.  */

extern void i51_cleanup PARAMS ((void));
extern void i51_after_pass_hook PARAMS ((void));
#define md_cleanup() i51_cleanup()
#define md_after_pass_hook() i51_after_pass_hook()
