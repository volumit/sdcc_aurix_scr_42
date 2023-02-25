/* tc-i51.c -- Assembler code for the MCS-51

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
   along with GAS; see the file COPYING.  If not, write to
   the Free Software Foundation, 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <stdio.h>
#include <ctype.h>
#include "as.h"
#include "subsegs.h"
#include "obstack.h"
#include "dwarf2dbg.h"
#include "struc-symbol.h"
#include "elf/i51.h"
#include "bfd.h"
#ifdef HTC_SUPPORT
#include "htc/htc_support.h"
#endif


//#define ASMDBG 1

/* forward declaration */
static int  check_range PARAMS ((long, int));
static void fixup8      PARAMS ((expressionS *, int, int));
static void fixup11     PARAMS ((expressionS *, unsigned int, int, int));
static void fixup16     PARAMS ((expressionS *, int, int));
static void i51_using   PARAMS ((int));
static void i51_bss     PARAMS ((int));
static void i51_rbss    PARAMS ((int));
static void i51_bbss    PARAMS ((int));
static void i51_ibss    PARAMS ((int));
static void i51_xbss    PARAMS ((int));
static void i51_ebss    PARAMS ((int));
static void i51_bitbss  PARAMS ((int));
static void i51_rdata   PARAMS ((int));
static void i51_bdata   PARAMS ((int));
static void i51_idata   PARAMS ((int));
static void i51_xdata   PARAMS ((int));
static void i51_edata   PARAMS ((int));
static void i51_bitdata PARAMS ((int));
static void i51_eeprom  PARAMS ((int));
static void i51_pcode   PARAMS ((int));
static void i51_bit     PARAMS ((int));
void i51_common (int);

#ifndef TC_PARSE_CONS_EXPRESSION
#ifdef BITFIELD_CONS_EXPRESSIONS
#define TC_PARSE_CONS_EXPRESSION(EXP, NBYTES) parse_bitfield_cons (EXP, NBYTES)
static void
parse_bitfield_cons PARAMS ((expressionS *exp, unsigned int nbytes));
#endif
#ifdef REPEAT_CONS_EXPRESSIONS
#define TC_PARSE_CONS_EXPRESSION(EXP, NBYTES) parse_repeat_cons (EXP, NBYTES)
static void
parse_repeat_cons PARAMS ((expressionS *exp, unsigned int nbytes));
#endif

/* If we haven't gotten one yet, just call expression.  */
#ifndef TC_PARSE_CONS_EXPRESSION
#define TC_PARSE_CONS_EXPRESSION(EXP, NBYTES) expression (EXP)
#endif
#endif

unsigned int current_isa;

const char comment_chars[] = ";";
const char line_comment_chars[] = "#";
const char line_separator_chars[] = "$";
const char EXP_CHARS[] = "eE";
const char FLT_CHARS[] = "dD";

const char *md_shortopts = "m:";
struct option md_longopts[] =
{
  { NULL, no_argument, NULL, 0 }
};

/* The target specific pseudo-ops which we support.  */
const pseudo_typeS md_pseudo_table[] =
{
  { "bit",      i51_bit,	0 },
  { "using",    i51_using,      0 },
  { "rcomm",    i51_common,  0x01 },
  { "rcommon",  i51_common,  0x01 },
  { "bitcomm",  i51_common,  0x02 },
  { "bitcommon",i51_common,  0x02 },
  { "comm",     i51_common,  0x00 },
  { "common",   i51_common,  0x00 },
  { "icomm",    i51_common,  0x03 },
  { "icommon",  i51_common,  0x03 },
  { "xcomm",    i51_common,  0x04 },
  { "xcommon",  i51_common,  0x04 },
  { "ecomm",    i51_common,  0x05 },
  { "ecommon",  i51_common,  0x05 },
  { "bcomm",    i51_common,  0x06 },
  { "bcommon",  i51_common,  0x06 },
  { "bss",      i51_bss,        0 },
  { "rbss",     i51_rbss,       0 },
  { "bbss",     i51_bbss,       0 },
  { "ibss",     i51_ibss,       0 },
  { "xbss",     i51_xbss,       0 },
  { "ebss",     i51_ebss,       0 },
  { "bitbss",   i51_bitbss,     0 },
  { "rdata",    i51_rdata,      0 },
  { "bdata",    i51_bdata,      0 },
  { "idata",    i51_idata,      0 },
  { "xdata",    i51_xdata,      0 },
  { "edata",    i51_edata,      0 },
  { "bitdata",  i51_bitdata,    0 },
  { "eeprom",   i51_eeprom,     0 },
  { "pcode",    i51_pcode,      0 },
  { NULL,	NULL,		0 }
};


size_t md_longopts_size = sizeof (md_longopts);

#define I51_OP_IMM8        0x0001
#define I51_OP_JUMP_REL    0x0002
#define I51_OP_JUMP_INPAGE 0x0003
#define I51_OP_IMM16       0x0004
#define I51_OP_BIT         0x0005
#define I51_OP_HIGH_ADDR   0x0100
#define I51_OP_LOW_ADDR    0x0200
#define I51_OP_B2B         0x0400

struct i51_opcodes_s
{
  char *name;
  char *args;
  int insn_size;		/* In bytes  */
  unsigned char mreloc;         /* modify & reloc mode */
  unsigned char bin_opcode;
  unsigned int machine;
};

struct i51_directop_s
{
  char *op;
  char ref;
  unsigned char value;
  unsigned int machine;
};

#define I51_INS(NAME, ARGS, SIZE, OPCODE, MRELOC, BIN, MASK, MACHINE) \
{NAME, ARGS, SIZE, MRELOC, BIN, MACHINE },

struct i51_opcodes_s i51_opcodes[] =
{
#include "opcode/i51.h"
  {NULL, NULL, 0, 0, 0, 0}
};

struct i51_directop_s i51_directop[] =
{
  {"A",      'A', 0, I51_MCS51},
  {"AB",     'C', 0, I51_MCS51},
  {"R0",     'R', 0, I51_MCS51},
  {"R1",     'R', 1, I51_MCS51},
  {"R2",     'R', 2, I51_MCS51},
  {"R3",     'R', 3, I51_MCS51},
  {"R4",     'R', 4, I51_MCS51},
  {"R5",     'R', 5, I51_MCS51},
  {"R6",     'R', 6, I51_MCS51},
  {"R7",     'R', 7, I51_MCS51},
  {"C",      'r', 0, I51_MCS51},
  {"DPTR",   'P', 0, I51_MCS51},
  {"@A+DPTR",'@', 0, I51_MCS51},
  {"@A+PC",  'X', 0, I51_MCS51},
  {"@R0"  ,  'I', 0, I51_MCS51},
  {"@R1",    'I', 1, I51_MCS51},
  {"/C",     '/', 0, I51_MCS51},
  {"@DPTR",  'T', 0, I51_MCS51},
// BYTE register bank addressable registers
  {"AR0",    'U', 0, I51_MCS51},
  {"AR1",    'U', 1, I51_MCS51},
  {"AR2",    'U', 2, I51_MCS51},
  {"AR3",    'U', 3, I51_MCS51},
  {"AR4",    'U', 4, I51_MCS51},
  {"AR5",    'U', 5, I51_MCS51},
  {"AR6",    'U', 6, I51_MCS51},
  {"AR7",    'U', 7, I51_MCS51},
// BYTE addressable registers
  {"P0",     'D',0x80, I51_MCS51},
  {"SP",     'D',0x81, I51_MCS51},
  {"DPL",    'D',0x82, I51_MCS51},
  {"DP0L",   'D',0x82, I51_MCS51},  // as called by Atmel
  {"DPH",    'D',0x83, I51_MCS51},
  {"DP0H",   'D',0x83, I51_MCS51},  // as called by Atmel
  {"DP1L",   'D',0x84, I51_MCS51},  // at89S8252 specific register
  {"DP1H",   'D',0x85, I51_MCS51},  // at89S8252 specific register
  {"SPDR",   'D',0x86, I51_MCS51},  // at89S8252 specific register
  {"PCON",   'D',0x87, I51_MCS51},
  {"TCON",   'D',0x88, I51_MCS51},
  {"TMOD",   'D',0x89, I51_MCS51},
  {"TL0",    'D',0x8A, I51_MCS51},
  {"TL1",    'D',0x8B, I51_MCS51},
  {"TH0",    'D',0x8C, I51_MCS51},
  {"TH1",    'D',0x8D, I51_MCS51},
  {"P1",     'D',0x90, I51_MCS51},
  {"WMCON",  'D',0x96, I51_MCS51},  // at89S8252 specific register
  {"SCON",   'D',0x98, I51_MCS51},
  {"SBUF",   'D',0x99, I51_MCS51},
  {"P2",     'D',0xA0, I51_MCS51},
  {"IE",     'D',0xA8, I51_MCS51},
  {"SPSR",   'D',0xAA, I51_MCS51},  // at89S8252 specific register
  {"P3",     'D',0xB0, I51_MCS51},
  {"IP",     'D',0xB8, I51_MCS51},
  {"T2CON",  'D',0xC8, I51_MCS51},
  {"T2MOD",  'D',0xC9, I51_MCS51},
  {"RCAP2L", 'D',0xCA, I51_MCS51},
  {"RCAP2H", 'D',0xCB, I51_MCS51},
  {"TL2",    'D',0xCC, I51_MCS51},
  {"TH2",    'D',0xCD, I51_MCS51},
  {"PSW",    'D',0xD0, I51_MCS51},
  {"SPCR",   'D',0xD5, I51_MCS51},  // at89S8252 specific register
  {"ACC",    'D',0xE0, I51_MCS51},
  {"B",      'D',0xF0, I51_MCS51},
// BIT addressable registers
// P0
  {"P0.0",   'B',0x80, I51_MCS51},
  {"P0.1",   'B',0x81, I51_MCS51},
  {"P0.2",   'B',0x82, I51_MCS51},
  {"P0.3",   'B',0x83, I51_MCS51},
  {"P0.4",   'B',0x84, I51_MCS51},
  {"P0.5",   'B',0x85, I51_MCS51},
  {"P0.6",   'B',0x86, I51_MCS51},
  {"P0.7",   'B',0x87, I51_MCS51},
// TCON
  {"IT0",    'B',0x88, I51_MCS51},
  {"IE0",    'B',0x89, I51_MCS51},
  {"IT1",    'B',0x8A, I51_MCS51},
  {"IE1",    'B',0x8B, I51_MCS51},
  {"TR0",    'B',0x8C, I51_MCS51},
  {"TF0",    'B',0x8D, I51_MCS51},
  {"TR1",    'B',0x8E, I51_MCS51},
  {"TF1",    'B',0x8F, I51_MCS51},
// P1
  {"P1.0",   'B',0x90, I51_MCS51},
  {"P1.1",   'B',0x91, I51_MCS51},
  {"P1.2",   'B',0x92, I51_MCS51},
  {"P1.3",   'B',0x93, I51_MCS51},
  {"P1.4",   'B',0x94, I51_MCS51},
  {"P1.5",   'B',0x95, I51_MCS51},
  {"P1.6",   'B',0x96, I51_MCS51},
  {"P1.7",   'B',0x97, I51_MCS51},
  {"T2",     'B',0x90, I51_MCS51},
  {"T2EX",   'B',0x91, I51_MCS51},
// SCON
  {"RI",     'B',0x98, I51_MCS51},
  {"TI",     'B',0x99, I51_MCS51},
  {"RB8",    'B',0x9A, I51_MCS51},
  {"TB8",    'B',0x9B, I51_MCS51},
  {"REN",    'B',0x9C, I51_MCS51},
  {"SM2",    'B',0x9D, I51_MCS51},
  {"SM1",    'B',0x9E, I51_MCS51},
  {"SM0",    'B',0x9F, I51_MCS51},
// P2
  {"P2.0",   'B',0xA0, I51_MCS51},
  {"P2.1",   'B',0xA1, I51_MCS51},
  {"P2.2",   'B',0xA2, I51_MCS51},
  {"P2.3",   'B',0xA3, I51_MCS51},
  {"P2.4",   'B',0xA4, I51_MCS51},
  {"P2.5",   'B',0xA5, I51_MCS51},
  {"P2.6",   'B',0xA6, I51_MCS51},
  {"P2.7",   'B',0xA7, I51_MCS51},
// IE
  {"EX0",    'B',0xA8, I51_MCS51},
  {"ET0",    'B',0xA9, I51_MCS51},
  {"EX1",    'B',0xAA, I51_MCS51},
  {"ET1",    'B',0xAB, I51_MCS51},
  {"ES",     'B',0xAC, I51_MCS51},
  {"ET2",    'B',0xAD, I51_MCS51},
  {"EA",     'B',0xAF, I51_MCS51},
// P3
  {"P3.0",   'B',0xB0, I51_MCS51},
  {"P3.1",   'B',0xB1, I51_MCS51},
  {"P3.2",   'B',0xB2, I51_MCS51},
  {"P3.3",   'B',0xB3, I51_MCS51},
  {"P3.4",   'B',0xB4, I51_MCS51},
  {"P3.5",   'B',0xB5, I51_MCS51},
  {"P3.6",   'B',0xB6, I51_MCS51},
  {"P3.7",   'B',0xB7, I51_MCS51},
  {"RXD",    'B',0xB0, I51_MCS51},
  {"TXD",    'B',0xB1, I51_MCS51},
  {"INT0",   'B',0xB2, I51_MCS51},
  {"INT1",   'B',0xB3, I51_MCS51},
  {"T0",     'B',0xB4, I51_MCS51},
  {"T1",     'B',0xB5, I51_MCS51},
  {"WR",     'B',0xB6, I51_MCS51},
  {"RD",     'B',0xB7, I51_MCS51},
// IP
  {"PX0",    'B',0xB8, I51_MCS51},
  {"PT0",    'B',0xB9, I51_MCS51},
  {"PX1",    'B',0xBA, I51_MCS51},
  {"PT1",    'B',0xBB, I51_MCS51},
  {"PS",     'B',0xBC, I51_MCS51},
  {"PT2",    'B',0xBD, I51_MCS51},
// T2CON
  {"T2CON.0",'B',0xC8, I51_MCS51},
  {"T2CON.1",'B',0xC9, I51_MCS51},
  {"T2CON.2",'B',0xCA, I51_MCS51},
  {"T2CON.3",'B',0xCB, I51_MCS51},
  {"T2CON.4",'B',0xCC, I51_MCS51},
  {"T2CON.5",'B',0xCD, I51_MCS51},
  {"T2CON.6",'B',0xCE, I51_MCS51},
  {"T2CON.7",'B',0xCF, I51_MCS51},
  {"CP_RL2", 'B',0xC8, I51_MCS51},
  {"C_T2",   'B',0xC9, I51_MCS51},
  {"TR2",    'B',0xCA, I51_MCS51},
  {"EXEN2",  'B',0xCB, I51_MCS51},
  {"TCLK",   'B',0xCC, I51_MCS51},
  {"RCLK",   'B',0xCD, I51_MCS51},
  {"EXF2",   'B',0xCE, I51_MCS51},
  {"TF2",    'B',0xCF, I51_MCS51},
// PSW
  {"P",      'B',0xD0, I51_MCS51},
  {"FL",     'B',0xD1, I51_MCS51},
  {"OV",     'B',0xD2, I51_MCS51},
  {"RS0",    'B',0xD3, I51_MCS51},
  {"RS1",    'B',0xD4, I51_MCS51},
  {"F0",     'B',0xD5, I51_MCS51},
  {"AC",     'B',0xD6, I51_MCS51},
  {"CY",     'B',0xD7, I51_MCS51},
  {"PSW.0",  'B',0xD0, I51_MCS51},
  {"PSW.1",  'B',0xD1, I51_MCS51},
  {"PSW.2",  'B',0xD2, I51_MCS51},
  {"PSW.3",  'B',0xD3, I51_MCS51},
  {"PSW.4",  'B',0xD4, I51_MCS51},
  {"PSW.5",  'B',0xD5, I51_MCS51},
  {"PSW.6",  'B',0xD6, I51_MCS51},
  {"PSW.7",  'B',0xD7, I51_MCS51},
// ACC
  {"ACC.0",  'B',0xE0, I51_MCS51},
  {"ACC.1",  'B',0xE1, I51_MCS51},
  {"ACC.2",  'B',0xE2, I51_MCS51},
  {"ACC.3",  'B',0xE3, I51_MCS51},
  {"ACC.4",  'B',0xE4, I51_MCS51},
  {"ACC.5",  'B',0xE5, I51_MCS51},
  {"ACC.6",  'B',0xE6, I51_MCS51},
  {"ACC.7",  'B',0xE7, I51_MCS51},
// B
  {"B.0",    'B',0xF0, I51_MCS51},
  {"B.1",    'B',0xF1, I51_MCS51},
  {"B.2",    'B',0xF2, I51_MCS51},
  {"B.3",    'B',0xF3, I51_MCS51},
  {"B.4",    'B',0xF4, I51_MCS51},
  {"B.5",    'B',0xF5, I51_MCS51},
  {"B.6",    'B',0xF6, I51_MCS51},
  {"B.7",    'B',0xF7, I51_MCS51},
//
  {"A",      'A', 0, I51_SCR},
  {"AB",     'C', 0, I51_SCR},
  {"R0",     'R', 0, I51_SCR},
  {"R1",     'R', 1, I51_SCR},
  {"R2",     'R', 2, I51_SCR},
  {"R3",     'R', 3, I51_SCR},
  {"R4",     'R', 4, I51_SCR},
  {"R5",     'R', 5, I51_SCR},
  {"R6",     'R', 6, I51_SCR},
  {"R7",     'R', 7, I51_SCR},
  {"C",      'r', 0, I51_SCR},
  {"DPTR",   'P', 0, I51_SCR},
  {"@A+DPTR",'@', 0, I51_SCR},
  {"@A+PC",  'X', 0, I51_SCR},
  {"@R0"  ,  'I', 0, I51_SCR},
  {"@R1",    'I', 1, I51_SCR},
  {"/C",     '/', 0, I51_SCR},
  {"@DPTR",  'T', 0, I51_SCR},
  {"@(DPTR++)", '&', 0, I51_SCR},
// BYTE register bank addressable registers
  {"AR0",    'U', 0, I51_SCR},
  {"AR1",    'U', 1, I51_SCR},
  {"AR2",    'U', 2, I51_SCR},
  {"AR3",    'U', 3, I51_SCR},
  {"AR4",    'U', 4, I51_SCR},
  {"AR5",    'U', 5, I51_SCR},
  {"AR6",    'U', 6, I51_SCR},
  {"AR7",    'U', 7, I51_SCR},
// BYTE addressable registers
  {"SP",     'D',0xD4, I51_SCR},
  {"DPL",    'D',0xD5, I51_SCR},
  {"DP0L",   'D',0xD5, I51_SCR},  // as called by Atmel
  {"DPH",    'D',0xD6, I51_SCR},
  {"DP0H",   'D',0xD6, I51_SCR},  // as called by Atmel
  {"PSW",    'D',0xD0, I51_SCR},
  {"IEN0",   'D',0xD8, I51_SCR},
  {"ACC",    'D',0xE0, I51_SCR},
  {"B",      'D',0xDA, I51_SCR},
// PSW
  {"P",      'B',0xD0, I51_SCR},
  {"FL",     'B',0xD1, I51_SCR},
  {"OV",     'B',0xD2, I51_SCR},
  {"RS0",    'B',0xD3, I51_SCR},
  {"RS1",    'B',0xD4, I51_SCR},
  {"F0",     'B',0xD5, I51_SCR},
  {"AC",     'B',0xD6, I51_SCR},
  {"CY",     'B',0xD7, I51_SCR},
  {"PSW.0",  'B',0xD0, I51_SCR},
  {"PSW.1",  'B',0xD1, I51_SCR},
  {"PSW.2",  'B',0xD2, I51_SCR},
  {"PSW.3",  'B',0xD3, I51_SCR},
  {"PSW.4",  'B',0xD4, I51_SCR},
  {"PSW.5",  'B',0xD5, I51_SCR},
  {"PSW.6",  'B',0xD6, I51_SCR},
  {"PSW.7",  'B',0xD7, I51_SCR},
// ACC
  {"ACC.0",  'B',0xE0, I51_SCR},
  {"ACC.1",  'B',0xE1, I51_SCR},
  {"ACC.2",  'B',0xE2, I51_SCR},
  {"ACC.3",  'B',0xE3, I51_SCR},
  {"ACC.4",  'B',0xE4, I51_SCR},
  {"ACC.5",  'B',0xE5, I51_SCR},
  {"ACC.6",  'B',0xE6, I51_SCR},
  {"ACC.7",  'B',0xE7, I51_SCR},
  // IE
  {"EX0",    'B',0xD8, I51_SCR},
  {"ET0",    'B',0xD9, I51_SCR},
  {"EX1",    'B',0xDA, I51_SCR},
  {"ET1",    'B',0xDB, I51_SCR},
  {"ES",     'B',0xDC, I51_SCR},
  {"ET2",    'B',0xDD, I51_SCR},
  {"EA",     'B',0xDF, I51_SCR},
  {NULL,     '\0',0,0}
};

/* Opcode hash table.  */
static struct hash_control *i51_hash;
/* Operand hash table.  */
static struct hash_control *i51_operand;

/* local variables */
unsigned char regbank = 0xFF;
unsigned char regused = 0x00;
/* md_assemble */
struct i51_opcodes_s *opcode;
struct i51_directop_s *oper;
char op[11];
expressionS op_expr1;
expressionS op_expr2;
expressionS op_expr3;
expressionS op_expr4;
char *line;
unsigned char regno;
unsigned char op1mode;
unsigned char op2mode;
int op1hlmode;
int op2hlmode;
int b2b_offset;
enum  PADRMODE {
  DIRECTWORD,
  DIRECTBYTE,
  DIRECTSWAP,
  DIRECTSHL8,
  INDIRECTWORD,
  INDIRECTBYTE,
  INDIRECTSWAP
} p1mode, p2mode, p3mode, p4mode;

/* Stuff for .regbank symbols.  */
static asection scom_section;
static asymbol  scom_symbol;
/* Stuff for .rbbs symbols.  */
static segT     rbss_section = NULL;
static asection rcom_section;
static asymbol  rcom_symbol;
/* Stuff for .bbbs symbols.  */
static segT     bbss_section = NULL;
static asection bcom_section;
static asymbol  bcom_symbol;
/* Stuff for .ibbs symbols.  */
static segT     ibss_section = NULL;
static asection icom_section;
static asymbol  icom_symbol;
/* Stuff for .xbbs symbols.  */
static segT     xbss_section = NULL;
static asection xcom_section;
static asymbol  xcom_symbol;
/* Stuff for .ebbs symbols.  */
static segT     ebss_section = NULL;
static asection ecom_section;
static asymbol  ecom_symbol;
/* Stuff for .bitbbs symbols.  */
static segT     bitbss_section = NULL;
static asection bitcom_section;
static asymbol  bitcom_symbol;
/* Stuff for .?data */
static segT     rdata_section = NULL;
static segT     bdata_section = NULL;
static segT     idata_section = NULL;
static segT     xdata_section = NULL;
static segT     edata_section = NULL;
static segT     eeprom_section = NULL;
static segT     bitdata_section = NULL;

/* local function */
void i51_build_ins (struct i51_opcodes_s *, unsigned char, expressionS *, expressionS *);
void i51_parse_operand1 (void);
void i51_parse_operand2 (void);
static inline char * skip_space (char *);
long md_pcrel_from_section (fixS *, segT);
void pcodeOperand (expressionS *, enum PADRMODE *, int);
void writePcodeOperand (expressionS *, enum PADRMODE *, int swapable ATTRIBUTE_UNUSED);
unsigned short decodePcodeOperand (expressionS *, enum PADRMODE *, int);


static inline char *
skip_space (s)
     char *s;
{
  while (*s == ' ' || *s == '\t')
    ++s;
  return s;
}

/* Extract one word from FROM and copy it to TO.  */

static char *
extract_word (char *from, char *to, int limit)
{
  char *op_start;
  char *op_end;
  int size = 0;

  /* Drop leading whitespace.  */
  from = skip_space (from);
  *to = 0;

  /* Find the op code end.  */
  for (op_start = op_end = from; *op_end != 0 && is_part_of_name (*op_end);)
    {
      to[size++] = *op_end++;
      if (size + 1 >= limit)
	break;
    }

  to[size] = 0;
  return op_end;
}

/* Extract one word from FROM and copy it to TO.  */

static char *
extract_op (char *from, char *to, int limit)
{
  char *op_start;
  char *op_end;
  int size = 0;

  /* Drop leading whitespace.  */
  from = skip_space (from);
  *to = 0;

  /* Find the op code end.  */
  for (op_start = op_end = from; *op_end != 0 && *op_end != ',' && *op_end != ' ';)
    {
      to[size++] = toupper(*op_end++);
      if (size + 1 >= limit)
	break;
    }

  to[size] = 0;
  return op_end;
}

int
md_estimate_size_before_relax (fragp, seg)
     fragS *fragp ATTRIBUTE_UNUSED;
     asection *seg ATTRIBUTE_UNUSED;
{
  abort ();
  return 0;
}

void
md_show_usage (stream)
     FILE *stream;
{
  fprintf (stream,
      _("I51 options:\n"
        "-mscr   architecture for SCR AURIX\n"
        "-mmcs51 architecture for standard mcs51\n"));
}


int
md_parse_option (c, arg)
     int c;
     char *arg;
{
       current_isa=0;
       switch (c)
         {
         case 'm':
           if (!strcmp (arg, "mcs51"))
             current_isa = I51_MCS51;
           else if (!strcmp (arg, "scr"))
             current_isa = I51_SCR;
           else
             {
               return 0;
             }
           default:
             break;
         }
return 1;
}


void
md_begin ()
{
  struct i51_opcodes_s *opcode;
  struct i51_directop_s *oper;
  if (current_isa==0)
    {
      as_bad (_("no -m option set -mmcs51 or -mscr"));
    }

  i51_hash = hash_new ();
  i51_operand = hash_new ();

  /* Insert unique names into hash table.  This hash table then provides a
     quick index to the first opcode with a particular name in the opcode
     table.  */
  for (opcode = i51_opcodes; opcode->name; opcode++)
    {
    if ((current_isa & opcode->machine) !=0) hash_insert (i51_hash, opcode->name, (char *) opcode);
    }
  /* Insert unique names into hash table.  This hash table then provides a
     quick index to the operand table.  */
  for (oper = i51_directop; oper->op; oper++)
    {
      if (current_isa==oper->machine) hash_insert (i51_operand, oper->op, (char *) oper);
    }
  /* We must construct a fake section similar to bfd_com_section
     but with the name .regbank.  */
  scom_section                = bfd_com_section;
  scom_section.name           = ".regbank";
  scom_section.output_section = &scom_section;
  scom_section.symbol         = &scom_symbol;
  scom_section.symbol_ptr_ptr = &scom_section.symbol;
  scom_symbol                 = *bfd_com_section.symbol;
  scom_symbol.name            = ".regbank";
  scom_symbol.section         = &scom_section;
  /* We must construct a fake section similar to bfd_com_section
     but with the name .rbss.  */
  rcom_section                = bfd_com_section;
  rcom_section.name           = ".rbss";
  rcom_section.output_section = &rcom_section;
  rcom_section.symbol         = &rcom_symbol;
  rcom_section.symbol_ptr_ptr = &rcom_section.symbol;
  rcom_symbol                 = *bfd_com_section.symbol;
  rcom_symbol.name            = ".rbss";
  rcom_symbol.section         = &rcom_section;
  /* We must construct a fake section similar to bfd_com_section
     but with the name .bbss.  */
  bcom_section                = bfd_com_section;
  bcom_section.name           = ".bbss";
  bcom_section.output_section = &bcom_section;
  bcom_section.symbol         = &bcom_symbol;
  bcom_section.symbol_ptr_ptr = &bcom_section.symbol;
  bcom_symbol                 = *bfd_com_section.symbol;
  bcom_symbol.name            = ".bbss";
  bcom_symbol.section         = &bcom_section;
  /* We must construct a fake section similar to bfd_com_section
     but with the name .ibss.  */
  icom_section                = bfd_com_section;
  icom_section.name           = ".ibss";
  icom_section.output_section = &icom_section;
  icom_section.symbol         = &icom_symbol;
  icom_section.symbol_ptr_ptr = &icom_section.symbol;
  icom_symbol                 = *bfd_com_section.symbol;
  icom_symbol.name            = ".ibss";
  icom_symbol.section         = &icom_section;
  /* We must construct a fake section similar to bfd_com_section
     but with the name .xbss.  */
  xcom_section                = bfd_com_section;
  xcom_section.name           = ".xbss";
  xcom_section.output_section = &xcom_section;
  xcom_section.symbol         = &xcom_symbol;
  xcom_section.symbol_ptr_ptr = &xcom_section.symbol;
  xcom_symbol                 = *bfd_com_section.symbol;
  xcom_symbol.name            = ".xbss";
  xcom_symbol.section         = &xcom_section;
  /* We must construct a fake section similar to bfd_com_section
     but with the name .ebss.  */
  ecom_section                = bfd_com_section;
  ecom_section.name           = ".ebss";
  ecom_section.output_section = &ecom_section;
  ecom_section.symbol         = &ecom_symbol;
  ecom_section.symbol_ptr_ptr = &ecom_section.symbol;
  ecom_symbol                 = *bfd_com_section.symbol;
  ecom_symbol.name            = ".ebss";
  ecom_symbol.section         = &ecom_section;
  /* We must construct a fake section similar to bfd_com_section
     but with the name .bitbss.  */
  bitcom_section                = bfd_com_section;
  bitcom_section.name           = ".bitbss";
  bitcom_section.output_section = &bitcom_section;
  bitcom_section.symbol         = &bitcom_symbol;
  bitcom_section.symbol_ptr_ptr = &bitcom_section.symbol;
  bitcom_symbol                 = *bfd_com_section.symbol;
  bitcom_symbol.name            = ".bitbss";
  bitcom_symbol.section         = &bitcom_section;
}

void
md_assemble (str)
     char *str;
{
#ifdef ASMDBG
  fprintf(stderr, "%s\n",str);
#endif
  op_expr1.X_op = O_max;
  op_expr2.X_op = O_max;

  str = skip_space (extract_word (str, op, sizeof (op)));

  if (!op[0])
    {
      as_bad (_("can't find opcode "));
      return;
    }

  regno = 0;

  opcode = (struct i51_opcodes_s *) hash_find (i51_hash, op);
  if (opcode == NULL)
    {
      as_bad (_("unknown opcode `%s'"), op);
      return;
    }
  else 
    {
      op1mode = 'N';
      op2mode = 'N';
      line = skip_space (str);
      if (*line != 0)
	{
	  if (opcode->args[0] == 'N')
	    {
	      as_bad (_("garbage at end of line"));
	      return;
	    }
	  i51_parse_operand1 ();
	  line = skip_space (line);
          if ((*line) && (opcode->args[1] == 'N'))
	    {
	      as_bad (_("garbage at end of line"));
	      return;
	    }
	  if (*line == ',')
	    {
	      line++;
	      i51_parse_operand2 ();
	      line = skip_space (line);
	      if ((*line) && (opcode->args[2] == 'N'))
		{
		  as_bad (_("garbage at end of line"));
		  return;
		}
	      if (opcode->args[2] == 'J')
		{
		  if (*line == ',')
		    {
		      input_line_pointer = line+1;
		      //PARSE EXPRESION 2
		      expression (&op_expr2);
		      line = input_line_pointer;
		      if (op_expr1.X_op == O_absent)
			{
			  as_bad (_("missing operand 3"));
			  return;
			}
		    }
		  else
		    {
		      as_bad (_("missing operand 3"));
		      return;
		    }
		}
	    }
	  else if (opcode->args[1] != 'N')
	    {
	      as_bad (_("missing operand 2"));
	      return;
	    }
#ifdef ASMDBG
	  fprintf(stderr, "args: %s reg: %i\n\n", opcode->args, regno);
#endif
            input_line_pointer = line;
	}
      else
	{
	  if (opcode->args[0] != 'N')
	    {
	      as_bad (_("missing operand 1"));
	      return;
	    }
	}
      if ((op1mode == opcode->args[0]) && (op2mode == opcode->args[1]))
	{
	  i51_build_ins (opcode, regno, &op_expr1, &op_expr2);
	}
      else
	{
#ifdef ASMDBG
	  fprintf(stderr,"invalid instruction operands %c/%c %c/%c", op1mode, opcode->args[0], op2mode, opcode->args[1]);
#endif
	  as_bad (_("invalid instruction operands"));
	  return;
	}
    }
}

/* GAS will call this function for each section at the end of the assembly,
   to permit the CPU backend to adjust the alignment of a section.  */

valueT
md_section_align (seg, addr)
     asection *seg;
     valueT addr;
{
  int align = bfd_get_section_alignment (stdoutput, seg);
  return ((addr + (1 << align) - 1) & (-1 << align));
}

/* If you define this macro, it should return the offset between the
   address of a PC relative fixup and the position from which the PC
   relative adjustment should be made.  On many processors, the base
   of a PC relative instruction is the next instruction, so this
   macro would return the length of an instruction.  */

long
md_pcrel_from_section (fixp, sec)
     fixS *fixp;
     segT sec;
{
  if (fixp->fx_addsy != (symbolS *) NULL
      && (!S_IS_DEFINED (fixp->fx_addsy)
	  || (S_GET_SEGMENT (fixp->fx_addsy) != sec)))
    return 0;
  //fprintf(stderr,"md_pcrel_from_section %x\n",fixp->fx_frag->fr_address + fixp->fx_where + 1);
  return fixp->fx_frag->fr_address + fixp->fx_where + 1;
}

/* GAS will call this for each fixup.  It should store the correct
   value in the object file.  */
void
md_apply_fix (fixp, valuep, seg)
     fixS *fixp;
     valueT *valuep;
     segT seg;
{
#if 0
  unsigned char *where;
  unsigned long insn;
  long value;

  if (fixp->fx_addsy == (symbolS *) NULL)
    {
      value = *valuep;
      fixp->fx_done = 1;
      //fprintf(stderr, "md_apply_fix: 1\n");
    }
  else if (fixp->fx_pcrel)
    {
      segT s = S_GET_SEGMENT (fixp->fx_addsy);

      if (fixp->fx_addsy && (s == seg || s == absolute_section))
	{
	  value = S_GET_VALUE (fixp->fx_addsy) + *valuep;
	  fixp->fx_done = 1;
          //fprintf(stderr, "md_apply_fix: 2 %d %d %d %d %d \n",value,S_GET_VALUE (fixp->fx_addsy) ,*valuep,s == seg,s == absolute_section);
	}
      else
        {
	value = *valuep;
        //fprintf(stderr, "md_apply_fix: 3 %d\n",value);
        }
    }
  else
    {
      value = fixp->fx_offset;

      if (fixp->fx_subsy != (symbolS *) NULL)
	{
	  if (S_GET_SEGMENT (fixp->fx_subsy) == absolute_section)
	    {
	      value -= S_GET_VALUE (fixp->fx_subsy);
	      fixp->fx_done = 1;
              //fprintf(stderr, "md_apply_fix: 4 %d\n",value);
	    }
	  else
	    {
	      /* We don't actually support subtracting a symbol.  */
	      as_bad_where (fixp->fx_file, fixp->fx_line,
			    _("expression too complex"));
	    }
	}
    }
#endif
  unsigned char *where;
  unsigned long insn;
  long value = *valuep;

  if (fixp->fx_addsy == (symbolS *) NULL)
    fixp->fx_done = 1;

  else if (fixp->fx_pcrel)
    {
      segT s = S_GET_SEGMENT (fixp->fx_addsy);

      if (s == seg || s == absolute_section)
        {
          //value += S_GET_VALUE (fixp->fx_addsy);
          //is relative and value is already in
//          fprintf(stderr, "md_apply_fix: 2 %d %d %d %d %d \n",value,S_GET_VALUE (fixp->fx_addsy) ,*valuep,s == seg,s == absolute_section);
          fixp->fx_done = 1;
        }
    }

  /* We don't actually support subtracting a symbol.  */
  if (fixp->fx_subsy != (symbolS *) NULL)
    as_bad_where (fixp->fx_file, fixp->fx_line, _("expression too complex"));

  switch (fixp->fx_r_type)
    {
    default:
      fixp->fx_no_overflow = 1;
      break;
    }

  if (fixp->fx_done)
    {
      /* Fetch the instruction, insert the fully resolved operand
	 value, and stuff the instruction back again.  */
      where = fixp->fx_frag->fr_literal + fixp->fx_where;
      insn = bfd_getb16 (where);

      switch (fixp->fx_r_type)
	{
	case BFD_RELOC_I51_8_LOW:
	  // bfd_putb8 ((bfd_vma) value, (unsigned char *) where);
	  ((bfd_byte *) where)[0] = (bfd_byte) (value & 0xFF);
	  break;

	case BFD_RELOC_I51_8_HIGH:
	  if (value < -65536 || value > 65535)
	    as_bad_where (fixp->fx_file, fixp->fx_line,
			  _("operand out of range: %ld"), value);
	  value >>= 8;
	  /* Fall through.  */

	case BFD_RELOC_8:
	case BFD_RELOC_I51_8_BIT:
	  if (value < -256 || value > 255)
	    as_bad_where (fixp->fx_file, fixp->fx_line,
			  _("operand out of range: %ld"), value);
	  // bfd_putb8 ((bfd_vma) value, (unsigned char *) where);
	  ((bfd_byte *) where)[0] = (bfd_byte) (value & 0xFF);
	  break;

	case BFD_RELOC_I51_7_PCREL:
	  if (value < -128 || value > 127)
	    as_bad_where (fixp->fx_file, fixp->fx_line,
			  _("operand out of range: %ld"), value);
	  // bfd_putb8 ((bfd_vma) value, (unsigned char *) where);
	  ((bfd_byte *) where)[0] = (bfd_byte) value;
#ifdef ASMDBG
	  fprintf(stderr, "fbdreloc7: %lx\n", value);
#endif
	  //fprintf(stderr, "fbdreloc7: %lx\n", value);
	  break;

	case BFD_RELOC_16:
	  bfd_putb16 ((bfd_vma) value, where);
	  break;

	case BFD_RELOC_I51_11:
	  value = (((value & 0x0700) << 5) | (value & 0x00FF));
	  value |= insn;
	  bfd_putb16 ((bfd_vma) value, where);
	  break;

	default:
	  as_fatal (_("line %d: unknown relocation type: 0x%x"),
		    fixp->fx_line, fixp->fx_r_type);
	  break;
	}
    }
  else
    {
      switch (fixp->fx_r_type)
	{
	default:
	  break;
	}
      fixp->fx_addnumber = value;
    }
  return;
}

/* Turn a string in input_line_pointer into a floating point constant
   of type TYPE, and store the appropriate bytes in *LITP.  The number
   of LITTLENUMS emitted is stored in *SIZEP.  An error message is
   returned, or NULL on OK.  */

char *
md_atof (type, litp, sizep)
     int type;
     char *litp;
     int *sizep;
{
  int prec;
  LITTLENUM_TYPE words[4];
  char *t;
  int i;

  switch (type)
    {
    case 'f':
      prec = 2;
      break;

    case 'd':
      prec = 4;
      break;

    default:
      *sizep = 0;
      return _("bad call to md_atof");
    }

  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;

  *sizep = prec * 2;

  if (target_big_endian)
    {
      for (i = 0; i < prec; i++)
	{
	  md_number_to_chars (litp, (valueT) words[i], 2);
	  litp += 2;
	}
    }
  else
    {
      for (i = prec - 1; i >= 0; i--)
	{
	  md_number_to_chars (litp, (valueT) words[i], 2);
	  litp += 2;
	}
    }

  return NULL;
}

static void
i51_bit (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  expressionS exp;

  if (is_it_end_of_statement ())
    {
      demand_empty_rest_of_line ();
      return;
    }

  do
    {
      TC_PARSE_CONS_EXPRESSION (&exp, (unsigned int) 1);
      if (exp.X_op == O_constant)
	{
	  if ((exp.X_add_number > 1) || (exp.X_add_number < 0))
	    {
	      as_bad (_("bit value is not in range 0..1 %ld"), exp.X_add_number);
	      return;
	    }
	}
      emit_expr (&exp, (unsigned int) 1);
    }
  while (*input_line_pointer++ == ',');

  input_line_pointer--;		/* Put terminator back into stream.  */

  demand_empty_rest_of_line ();
}

symbolS *
md_undefined_symbol (name)
     char *name;
{
  extract_op (name, op, sizeof (op));
  oper = (struct i51_directop_s *) hash_find (i51_operand, op);
  if (oper == NULL) return 0;
  if ((oper->ref == 'D') || (oper->ref == 'B'))
    {
      return (symbol_new (name, absolute_section, oper->value, &zero_address_frag));
    }
  else if (oper->ref == 'U')
    {
      if (regbank == 0xFF)
	{
	  as_bad (_("missing .using in md_undefined_symbol"));
	  return 0;
	}
      return (symbol_new (name, absolute_section, regbank + oper->value, &zero_address_frag));
    }
  else return 0;
}

void
md_convert_frag (abfd, sec, fragP)
     bfd *abfd ATTRIBUTE_UNUSED;
     asection *sec ATTRIBUTE_UNUSED;
     fragS *fragP ATTRIBUTE_UNUSED;
{
  abort ();
}

/* A `BFD_ASSEMBLER' GAS will call this to generate a reloc.  GAS
   will pass the resulting reloc to `bfd_install_relocation'.  This
   currently works poorly, as `bfd_install_relocation' often does the
   wrong thing, and instances of `tc_gen_reloc' have been written to
   work around the problems, which in turns makes it difficult to fix
   `bfd_install_relocation'.  */

/* If while processing a fixup, a reloc really needs to be created
   then it is done here.  */

arelent *
tc_gen_reloc (seg, fixp)
     asection *seg ATTRIBUTE_UNUSED;
     fixS *fixp;
{
  arelent *reloc;

  reloc = (arelent *) xmalloc (sizeof (arelent));

  reloc->sym_ptr_ptr = (asymbol **) xmalloc (sizeof (asymbol *));
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);

  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
  if (reloc->howto == (reloc_howto_type *) NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("reloc %d not supported by object file format"),
		    (int) fixp->fx_r_type);
      return NULL;
    }

  if (fixp->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fixp->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
     reloc->address = fixp->fx_offset;

  reloc->addend = fixp->fx_offset;

  if (fixp->fx_pcrel)
    reloc->addend -= 1;

  return reloc;
}

void
i51_parse_operand1 (void)
{
  int temp;

  input_line_pointer = line;
  line = extract_op (line, op, sizeof (op));
  oper = (struct i51_directop_s *) hash_find (i51_operand, op);
  op1hlmode = 0;
  if (oper == NULL)
    {
      if (op[0] == '@')
	{
            as_bad (_("indirect prefix not allowed 1: `%s'"), op);
	}
      else if (op[0] == '#')
	{
	  input_line_pointer++;
	  op1mode = '#';                     /* # - #data */

	  //scan hi/low prefix
	  if      (strncmp (input_line_pointer, "hi_(", 4) == 0) { input_line_pointer += 3; op1hlmode = I51_OP_HIGH_ADDR;}
	  else if (strncmp (input_line_pointer, "lo_(", 4) == 0) { input_line_pointer += 3; op1hlmode = I51_OP_LOW_ADDR;}

	  while (((opcode->insn_size & 0x80) == 0) && ( opcode->args[0] != op1mode)) opcode++;
	  if (opcode->args[0] != op1mode)
	    {
	      as_bad (_("direct prefix not allowed 1: `%s'"), op);
	      return;
	    }
	} 
      else
	{
	  op1mode = ' ';

	  //scan hi/low prefix
	  if      (strncmp (input_line_pointer, "hi_(",  4) == 0) { input_line_pointer += 3; op1hlmode = I51_OP_HIGH_ADDR;}
	  else if (strncmp (input_line_pointer, "lo_(",  4) == 0) { input_line_pointer += 3; op1hlmode = I51_OP_LOW_ADDR;}
	  else if (strncmp (input_line_pointer, "B2B(",  4) == 0) { input_line_pointer += 3; op1hlmode = I51_OP_B2B;}

	  while (((opcode->insn_size & 0x80) == 0) && 
		 ( opcode->args[0] != 'B') && /* B - bit */
		 ( opcode->args[0] != 'D') && /* D - data */
		 ( opcode->args[0] != 'J') && /* J - jump rel */
		 ( opcode->args[0] != '1') && /* 1 - jump addr 11 */
		 ( opcode->args[0] != '6')    /* 6 - jump addr 16 */
		 ) opcode++;
	}
      if ((opcode->insn_size & 0x80) != 0)
	{
	  op1mode = opcode->args[0];
	}
      else if (( opcode->args[0] != 'B') && ( opcode->args[0] != 'D'))
	{
	  op1mode = opcode->args[0];
	}
      //PARSE EXPRESION 1
      expression (&op_expr1);
      if (op_expr1.X_op == O_absent)
	{
	  as_bad (_("missing operand 1"));
	  return;
	}

      if (op1hlmode == I51_OP_B2B)
	{
	  SKIP_WHITESPACE ();
	  if (*input_line_pointer != ',')
	    {
	      as_bad (_("Expected comma after symbol-name"));
	      ignore_rest_of_line ();
	      return;
	    }
	  input_line_pointer++;		/* skip ',' */
	  if ((temp = get_absolute_expression ()) < 0)
	    {
	      as_bad (_("BYTE2BIT offset (%d.) <0! Ignored."), temp);
	      ignore_rest_of_line ();
	      return;
	    }
	  SKIP_WHITESPACE ();
	  if (*input_line_pointer != ')')
	    {
	      as_bad (_("Expected ) after offset"));
	      ignore_rest_of_line ();
	      return;
	    }
	  input_line_pointer++;		/* skip ')' */
	  b2b_offset = temp;
	}

      line = input_line_pointer;

#ifdef ASMDBG
      fprintf(stderr, "op1: %c %s\n", op1mode, op);
      //fprintf(stderr, "op1: %c %s %s\n", op1mode, op, S_GET_NAME(op_expr1.X_add_symbol));
#endif
    }
  else
    {
      op1mode = oper->ref;
      if(op1mode == 'U') op1mode = 'D';
      while (((opcode->insn_size & 0x80) == 0) && ( opcode->args[0] != op1mode)) opcode++;
      if (opcode->args[0] != op1mode)
	{
	  as_bad (_("unknown instruction operand 1: `%s'"), op);
	  return;
	}
      if ((op1mode == 'R') || (op1mode == 'I'))
	{
	  regno = oper->value;
	}
      else if (oper->ref == 'U')
	{
	  if (regbank == 0xFF)
	    {
	      as_bad (_("missing .using in i51_parse_operand1"));
	      return;
	    }
	  op_expr1.X_op = O_constant;
	  op_expr1.X_add_number = regbank + oper->value;
	}
      else if ((op1mode == 'D') || (op1mode == 'B'))
	{
	  op_expr1.X_op = O_constant;
	  op_expr1.X_add_number = oper->value;
	}
#ifdef ASMDBG
      fprintf(stderr, "op1: %c %s\n", op1mode, op);
#endif
    }
}

void
i51_parse_operand2 (void)
{
  int temp;

  input_line_pointer = line;
  line = extract_op (line, op, sizeof (op));
  oper = (struct i51_directop_s *) hash_find (i51_operand, op);
  op2hlmode = 0;
  if (oper == NULL)
    {
      if (op[0] == '@')
	{
            as_bad (_("indirect prefix not allowed 1: `%s'"), op);
            return;
	}
      else if (op[0] == '#')
	{
	  input_line_pointer++;
	  op2mode = op[0];   /* # - #data */

	  //scan hi/low prefix
	  if      (strncmp (input_line_pointer, "hi_(",  4) == 0) { input_line_pointer += 3; op2hlmode = I51_OP_HIGH_ADDR;}
	  else if (strncmp (input_line_pointer, "lo_(",  4) == 0) { input_line_pointer += 3; op2hlmode = I51_OP_LOW_ADDR;}

	  while (((opcode->insn_size & 0x80) == 0) && ( opcode->args[1] != op2mode)) opcode++;
	  if (opcode->args[1] != op2mode)
	    {
	      as_bad (_("direct prefix not allowed 2: `%s'"), op);
	      return;
	    }
	  if (op1mode == ' ') op1mode = 'D';
	} 
      else if (op[0] == '/')
	{
	  op2mode = op[0];   /* - - /bit  */
	  while (((opcode->insn_size & 0x80) == 0) && ( opcode->args[1] != op2mode)) opcode++;
	  if (opcode->args[1] != op2mode)
	    {
	      as_bad (_("negation prefix not allowed 2: `%s'"), op);
	      return;
	    }
	  if (op1mode == ' ') op1mode = 'B';
	  input_line_pointer++;
	} 
      else
	{
	  //op2mode = ' ';

	  //scan hi/low prefix
	  if      (strncmp (input_line_pointer, "hi_(",  4) == 0)
	    {
	      input_line_pointer += 3;
	      op2hlmode = I51_OP_HIGH_ADDR;
	    }
	  else if (strncmp (input_line_pointer, "lo_(",  4) == 0)
	    {
	      input_line_pointer += 3;
	      op2hlmode = I51_OP_LOW_ADDR;
	    }
	  else if ((op_expr1.X_op == O_max) && (strncmp (input_line_pointer, "B2B(",  4) == 0))
	    {
	      input_line_pointer += 4;
	      op2hlmode = I51_OP_B2B;
	    }

	  while (((opcode->insn_size & 0x80) == 0) && 
		 ( opcode->args[1] != 'B') && /* B - bit */
		 ( opcode->args[1] != 'D') && /* D - data */
		 ( opcode->args[1] != 'J') && /* J - jump rel */
		 ( opcode->args[1] != 'd')    /* d - data16 */
		 ) opcode++;
	}
      op2mode = opcode->args[1];
      
      if (op1mode == ' ') op1mode = opcode->args[0];

      //PARSE EXPRESION 1 / 2
      if (op_expr1.X_op == O_max)
	{
	  op1hlmode = op2hlmode;
	  expression (&op_expr1);
	  if (op_expr1.X_op == O_absent)
	    {
	      as_bad (_("missing operand 2"));
	      return;
	    }
	  if (op2hlmode == I51_OP_B2B)
	    {
	      SKIP_WHITESPACE ();
	      if (*input_line_pointer != ',')
		{
		  as_bad (_("Expected comma after symbol-name"));
		  ignore_rest_of_line ();
		  return;
		}
	      input_line_pointer++;		/* skip ',' */
	      if ((temp = get_absolute_expression ()) < 0)
		{
		  as_bad (_("BYTE2BIT offset (%d.) <0! Ignored."), temp);
		  ignore_rest_of_line ();
		  return;
		}
	      SKIP_WHITESPACE ();
	      if (*input_line_pointer != ')')
		{
		  as_bad (_("Expected ) after offset"));
		  ignore_rest_of_line ();
		  return;
		}
	      input_line_pointer++;		/* skip ')' */
	      b2b_offset = temp;
	    }
	}
      else
	{
	  expression (&op_expr2);
	  if (op_expr2.X_op == O_absent)
	    {
	      as_bad (_("missing operand 2"));
	      return;
	    }
	}

      line = input_line_pointer;

#ifdef ASMDBG
      fprintf(stderr, "op2: %c %s\n", op2mode, op);
#endif
    }
  else
    {
      op2mode = oper->ref;
      if(op2mode == 'U') op2mode = 'D';
      while (((opcode->insn_size & 0x80) == 0) && ( opcode->args[1] != op2mode)) opcode++;
      if (opcode->args[1] != op2mode)
	{
	  as_bad (_("unknown instruction operand 2: `%s'"), op);
	  return;
	}
      if ((opcode->args[0] != op1mode) &&
	  ((op1mode != ' ') || ((opcode->args[0] != 'B') && (opcode->args[0] != 'D'))) )
	{
	  as_bad (_("unknown instruction operand 2: `%s'"), op);
	  return;
	}
      if (op1mode == ' ') op1mode = opcode->args[0];
      if ((op2mode == 'R') || (op2mode == 'I'))
	{
	  regno = oper->value;
	}
      else if (oper->ref == 'U')
	{
	  if (regbank == 0xFF)
	    {
	      as_bad (_("missing .using in i51_parse_operand2"));
	      return;
	    }
	  if (op_expr1.X_op == O_max)
	    {
	      op_expr1.X_op = O_constant;
	      op_expr1.X_add_number = regbank + oper->value;
	    }
	  else
	    {
	      op_expr2.X_op = O_constant;
	      op_expr2.X_add_number = regbank + oper->value;
	    }
	}
      else if ((op2mode == 'D') || (op2mode == 'B'))
	{
	  if (op_expr1.X_op == O_max)
	    {
	      op_expr1.X_op = O_constant;
	      op_expr1.X_add_number = oper->value;
	    }
	  else
	    {
	      op_expr2.X_op = O_constant;
	      op_expr2.X_add_number = oper->value;
	    }
	}
#ifdef ASMDBG
      fprintf(stderr, "op2: %c %s\n", op2mode, op);
#endif
    }
}

void
i51_build_ins (opcode, regnumber, op_expr1, op_expr2)
  struct i51_opcodes_s *opcode;
  unsigned char regnumber;
  expressionS *op_expr1;
  expressionS *op_expr2;
{
  unsigned char bin_opcode;
  char *frag;

  bin_opcode = opcode->bin_opcode;
  /* place register operand to instruction code */
  switch (opcode->mreloc) {
  case 'I':			// register indirect            - IIIIIIIr
  case 'i':			// reg. indirect, data          - IIIIIIIr dddddddd
  case 'X':			// reg. indirect, r. jump 8 bit - IIIIIIIr dddddddd aaaaaaaa
    regnumber &= 0x01;
    bin_opcode |= regnumber;
    break;
  case 'R':			// register                     - IIIIIrrr
  case 'r':			// register,data                - IIIIIrrr dddddddd
  case 'Y':			// register, rel. jump 8 bit    - IIIIIrrr dddddddd aaaaaaaa
  case 'W':			// register, rel. jump 8 bit    - IIIIIrrr aaaaaaaa
    regnumber &= 0x07;
    bin_opcode |= regnumber;
    break;
  default:
    break;
  }

  switch (opcode->mreloc) {
  case '7':			//relative jump 8 bit          - IIIIIIII aaaaaaaa
  case 'W':			//register, rel. jump 8 bit    - IIIIIrrr aaaaaaaa
  case '1':			//jump 11 bit                  - aaaIIIII aaaaaaaa
  case '6':			//data/jump 16 bit             - IIIIIIII aaaaaaaa aaaaaaaa
    if (op1hlmode)
      {
	as_bad (_("HI/LOW/B2B prefix not allowed"));
	return;
      }
  default:
    break;
  }

  switch (opcode->mreloc) {
  case 'N':			//none                - IIIIIIII
  case 'I':			//register indirect   - IIIIIIIr
  case 'R':			//register            - IIIIIrrr
    frag = frag_more (1);
    number_to_chars_bigendian (frag, bin_opcode, 1);
    break;

  case 'D':			//data 8 bit                   - IIIIIIII dddddddd
  case 'i':			//reg. indirect, data          - IIIIIIIr dddddddd
  case 'r':			//register,data                - IIIIIrrr dddddddd
    frag = frag_more (1);
    number_to_chars_bigendian (frag, bin_opcode, 1);
    fixup8 (op_expr1, I51_OP_IMM8, op1hlmode);
    break;

  case '7':			//relative jump 8 bit          - IIIIIIII aaaaaaaa
  case 'W':			//register, rel. jump 8 bit    - IIIIIrrr aaaaaaaa
    frag = frag_more (1);
    number_to_chars_bigendian (frag, bin_opcode, 1);
    fixup8 (op_expr1, I51_OP_JUMP_REL, 0);
    break;

  case '1':			//jump 11 bit                  - aaaIIIII aaaaaaaa
    fixup11(op_expr1, bin_opcode, I51_OP_JUMP_INPAGE, 0);
    break;

  case 'B':			//bitdata 8 bit                - IIIIIIII bbbbbbbb
    frag = frag_more (1);
    number_to_chars_bigendian (frag, bin_opcode, 1);
    fixup8 (op_expr1, I51_OP_BIT, op1hlmode);
    break;

  case 'd':			//data,data 8 bit              - IIIIIIII dddddddd dddddddd
    frag = frag_more (1);
    number_to_chars_bigendian (frag, bin_opcode, 1);
    fixup8 (op_expr2, I51_OP_IMM8, op2hlmode);
    fixup8 (op_expr1, I51_OP_IMM8, op1hlmode);
    break;

  case 'a':			//adr,data 8 bit               - IIIIIIII aaaaaaaa dddddddd
    frag = frag_more (1);
    number_to_chars_bigendian (frag, bin_opcode, 1);
    fixup8 (op_expr1, I51_OP_IMM8, op1hlmode);
    fixup8 (op_expr2, I51_OP_IMM8, op2hlmode);
    break;

  case 'J':			//bit, r. jump 8 bit           - IIIIIIII bbbbbbbb aaaaaaaa
    frag = frag_more (1);
    number_to_chars_bigendian (frag, bin_opcode, 1);
    fixup8 (op_expr1, I51_OP_BIT, op1hlmode);
    fixup8 (op_expr2, I51_OP_JUMP_REL, 0);
    break;

  case 'X':			//reg. indirect, r. jump 8 bit - IIIIIIIr dddddddd aaaaaaaa
  case 'Y':			//register, rel. jump 8 bit    - IIIIIrrr dddddddd aaaaaaaa
  case 'Z':			//data 8 bit, r. jump 8 bit    - IIIIIIII dddddddd aaaaaaaa
    frag = frag_more (1);
    number_to_chars_bigendian (frag, bin_opcode, 1);
    fixup8 (op_expr1, I51_OP_IMM8, op1hlmode);
    fixup8 (op_expr2, I51_OP_JUMP_REL, 0);
    break;

  case '6':			//data/jump 16 bit             - IIIIIIII aaaaaaaa aaaaaaaa
    frag = frag_more (1);
    number_to_chars_bigendian (frag, bin_opcode, 1);
    fixup16 (op_expr1, I51_OP_IMM16, 0);
    break;
  }
}

/* Checks that the number 'num' fits for a given mode.  */
static int
check_range (num, mode)
     long num;
     int mode;
{
  switch (mode)
    {
    case I51_OP_IMM8:
    case I51_OP_BIT:
      return (((num & 0xFFFFFF00) == 0) || ((num & 0xFFFFFF00) == 0xFFFFFF00)) ? 1 : 0;

    case I51_OP_JUMP_REL:
      return (num >= -128 && num <= 127) ? 1 : 0;

    case I51_OP_JUMP_INPAGE:
      return ((num & 0xFFFFF800) == 0);

    case I51_OP_IMM16:
    case I51_OP_HIGH_ADDR:
      return (((num & 0xFFFF0000) == 0) || ((num & 0xFFFF0000) == 0xFFFF0000)) ? 1 : 0;

    case I51_OP_LOW_ADDR:
      return 1;

    default:
      return 0;
    }
}

/* Gas fixup generation.  */

/* Put a 1 byte expression described by 'oper'.  If this expression contains
   unresolved symbols, generate an 8-bit fixup.  */
static void
fixup8 (oper, mode, opmode)
     expressionS *oper;
     int mode;
     int opmode;
{
  char *f;

  f = frag_more (1);

  if (oper->X_op == O_constant)
    {
      switch (opmode) {
      case I51_OP_HIGH_ADDR:
	if (!check_range (oper->X_add_number, opmode))
	  {
	    as_bad (_("hi_ Operand out of 8-bit range: `%ld'."),
		    oper->X_add_number);
	  }
	number_to_chars_bigendian (f, ((oper->X_add_number >> 8)) & 0xFF, 1);
	break;
      case I51_OP_LOW_ADDR:
	if (!check_range (oper->X_add_number, opmode))
	  {
	    as_bad (_("lo_ Operand out of 8-bit range: `%ld'."),
		    oper->X_add_number);
	  }
	number_to_chars_bigendian (f, (oper->X_add_number & 0xFF), 1);
	break;
      case I51_OP_B2B:
	if ((oper->X_add_number < 0x20) ||
	    ((oper->X_add_number < 0x30) && (((oper->X_add_number - 0x20) * 8 + b2b_offset) > 0x80)) ||
	    ((oper->X_add_number >= 0x30) && (oper->X_add_number < 0x80)) ||
	    ((oper->X_add_number + b2b_offset) > 0x100) )
	  {
	    as_bad (_("Operand out of bit range: B2B(%ld,%d)."),
		    oper->X_add_number, b2b_offset);
	  }
	if (oper->X_add_number < 0x30)
	  {
	    number_to_chars_bigendian (f, ((oper->X_add_number - 0x20) * 8 + b2b_offset), 1);
	  }
	else
	  {
	    number_to_chars_bigendian (f, (oper->X_add_number + b2b_offset), 1);
	  }
	break;
      default:
	if (!check_range (oper->X_add_number, mode))
	  {
	    as_bad (_("Operand out of 8-bit range: `%ld'."),
		    oper->X_add_number);
	  }
	number_to_chars_bigendian (f, (oper->X_add_number & 0xFF), 1);
	break;
      }
    }
  else if (oper->X_op != O_register)
    {
      switch (mode) {
      case I51_OP_IMM8:
	/* Now create an 8-bit fixup.  If there was some %hi or %lo
	 modifier, generate the reloc accordingly.  */
	fix_new_exp (frag_now, f - frag_now->fr_literal, 1,
		     oper, FALSE,
		     ((opmode & I51_OP_HIGH_ADDR)
		      ? BFD_RELOC_I51_8_HIGH
		      : ((opmode & I51_OP_LOW_ADDR)
			 ? BFD_RELOC_I51_8_LOW : BFD_RELOC_8)));
	number_to_chars_bigendian (f, 0, 1);
	break;
      case I51_OP_JUMP_REL:
	/* Now create an 8-bit relative jump fixup */
	fix_new_exp (frag_now, f - frag_now->fr_literal, 1,
		     oper, TRUE, BFD_RELOC_I51_7_PCREL);
	if (oper->X_op==O_symbol)
	  {
	    //fprintf(stderr,"I51_OP_JUMP_REL with symbol %d %s %x %x %x \n",oper->X_op,S_GET_NAME (oper->X_add_symbol),oper->X_add_number,f,frag_now->fr_literal);
	    //if (bfd_is_local_label_name (stdoutput, S_GET_NAME (oper->X_add_symbol))) fprintf(stderr,"is local\n");
	    //else
	    //  fprintf(stderr,"is not local\n");
	  }
	number_to_chars_bigendian (f, 0, 1);
	break;
      case I51_OP_BIT:
	/* Now create an 8-bit bit B2B area fixup */
	fix_new_exp (frag_now, f - frag_now->fr_literal, 1,
		     oper, FALSE,
		     (opmode & I51_OP_B2B 
		      ? BFD_RELOC_I51_8_B2B : BFD_RELOC_I51_8_BIT));
	number_to_chars_bigendian (f, (opmode & I51_OP_B2B
				       ? b2b_offset : 0), 1);
	break;
      }
    }
  else
    {
      as_fatal (_("Operand `%x' not recognized in fixup8."), oper->X_op);
    }
}

/* Put a 11 bit expression described by 'oper'.  If this expression contains
   unresolved symbols, generate an 11-bit fixup.  */
static void
fixup11 (oper, opcode, mode, opmode)
     expressionS *oper;
     unsigned int opcode;
     int mode;
     int opmode ATTRIBUTE_UNUSED;
{
  char *f;

  f = frag_more (2);

  if (oper->X_op == O_constant)
    {
      if (!check_range (oper->X_add_number, mode))
	{
	  as_bad (_("Operand out of 11-bit range: `%ld'."),
		  oper->X_add_number);
	}
      number_to_chars_bigendian (f, ((oper->X_add_number & 0x0700) << 5) | (opcode << 8) | (oper->X_add_number & 0x00FF), 2);
    }
  else if (oper->X_op != O_register)
    {
      switch (mode) {
      case I51_OP_JUMP_INPAGE:
	/* Now create an 11-bit inpage jump fixup */
	fix_new_exp (frag_now, f - frag_now->fr_literal, 1,
		     oper, FALSE,BFD_RELOC_I51_11);
	number_to_chars_bigendian (f, (opcode << 8), 2);
	break;
      }
    }
  else
    {
      as_fatal (_("Operand `%x' not recognized in fixup8."), oper->X_op);
    }
}

/* Put a 2 byte expression described by 'oper'.  If this expression contains
   unresolved symbols, generate a 16-bit fixup.  */
static void
fixup16 (oper, mode, opmode)
     expressionS *oper;
     int mode;
     int opmode ATTRIBUTE_UNUSED;
{
  char *f;

  f = frag_more (2);

  if (oper->X_op == O_constant)
    {
      if (!check_range (oper->X_add_number, mode))
	{
	  as_bad (_("Operand out of 16-bit range: `%ld'."),
		  oper->X_add_number);
	}
      number_to_chars_bigendian (f, oper->X_add_number & 0x0FFFF, 2);
    }
  else if (oper->X_op != O_register)
    {
      fixS *fixp;

      /* Now create a 16-bit fixup.  */
      fixp = fix_new_exp (frag_now, f - frag_now->fr_literal, 2,
			  oper,
			  FALSE,
			  BFD_RELOC_16);
      number_to_chars_bigendian (f, 0, 2);
    }
  else
    {
      as_fatal (_("Operand `%x' not recognized in fixup16."), oper->X_op);
    }
}

int
i51_section_flags (flags, attr, type)
     int flags;
     int attr;
     int type ATTRIBUTE_UNUSED;
{
  if (attr & SHF_REGBANK)
    flags |= SEC_IS_COMMON;

  return flags;
}

static void
i51_using (par)
     int par ATTRIBUTE_UNUSED;
{
  symbolS *symbolP;
  switch (*input_line_pointer++)
    {
    case '0':  /* Reg bank 0 */
      symbolP = symbol_find_or_make ("__RB0__");
      S_SET_VALUE (symbolP, (valueT) 8);
      S_SET_EXTERNAL (symbolP);
      S_SET_SEGMENT (symbolP, &scom_section);
      know (symbolP->sy_frag == &zero_address_frag);
      regbank = 0;
      break;
    case '1':  /* Reg bank 1 */
      symbolP = symbol_find_or_make ("__RB1__");
      S_SET_VALUE (symbolP, (valueT) 8);
      S_SET_EXTERNAL (symbolP);
      S_SET_SEGMENT (symbolP, &scom_section);
      know (symbolP->sy_frag == &zero_address_frag);
      regbank = 8;
      break;
    case '2':  /* Reg bank 2 */
      symbolP = symbol_find_or_make ("__RB2__");
      S_SET_VALUE (symbolP, (valueT) 8);
      S_SET_EXTERNAL (symbolP);
      S_SET_SEGMENT (symbolP, &scom_section);
      know (symbolP->sy_frag == &zero_address_frag);
      regbank = 16;
      break;
    case '3':  /* Reg bank 3 */
      symbolP = symbol_find_or_make ("__RB3__");
      S_SET_VALUE (symbolP, (valueT) 8);
      S_SET_EXTERNAL (symbolP);
      S_SET_SEGMENT (symbolP, &scom_section);
      know (symbolP->sy_frag == &zero_address_frag);
      regbank = 24;
      break;
    default:
      as_bad ("unsupported register bank");
      return;
    }
  if (regbank > regused) regused = regbank;
}

/*
void
i51_set (name)
     char *name;
{
  register symbolS *symbolP;

  if ((symbol_find (name) == NULL) && (md_undefined_symbol (name) == NULL))
    {
	symbolP = symbol_new (name, absolute_section, 0, &zero_address_frag);
	symbol_table_insert (symbolP);
    }
}
*/

void
i51_cleanup ()
{
//  symbolS *symbolP;

//  if (regbank != 0xFF)
//    {
//
//  symbolP = symbol_find_or_make ("__RB__");
//
//
//  S_SET_VALUE (symbolP, (valueT) regused+8);
//  S_SET_EXTERNAL (symbolP);
//  S_SET_SEGMENT (symbolP, &scom_section);
//
//  know (symbolP->sy_frag == &zero_address_frag);
//
//      regbank = 0xFF;
//      regused = 0;
//    }
}

void
i51_after_pass_hook ()
{
//  symbolS *symbolP;
//
//  if (regbank != 0xFF)
//    {
//
//  symbolP = symbol_find_or_make ("__RB__");
//
//  S_SET_VALUE (symbolP, (valueT) regused+8);
//  S_SET_EXTERNAL (symbolP);
//  S_SET_SEGMENT (symbolP, &scom_section);
//
//  know (symbolP->sy_frag == &zero_address_frag);
//
//      regbank = 0xFF;
//      as_bad (_(".using i51_after_pass_hook"));
//      fprintf(stderr,".using i51_after_pass_hook %d \n",regbank);
//      regused = 0;
//    }
}

void
i51_common (common_segment)
     int common_segment;
{
  char *name;
  char c;
  char *p;
  int temp, size;
  symbolS *symbolP;
  int align;

  if (flag_mri)
    {
      as_bad (_("not implemented in MRI mode"));
      return;
    }

  name = input_line_pointer;
  c = get_symbol_end ();
  /* just after name is now '\0' */
  p = input_line_pointer;
  *p = c;
  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
      as_bad (_("Expected comma after symbol-name"));
      ignore_rest_of_line ();
      return;
    }
  input_line_pointer++;		/* skip ',' */
  if ((temp = get_absolute_expression ()) < 0)
    {
      as_bad (_(".COMMon length (%d.) <0! Ignored."), temp);
      ignore_rest_of_line ();
      return;
    }
  size = temp;
  *p = 0;
  symbolP = symbol_find_or_make (name);
  *p = c;
  if (S_IS_DEFINED (symbolP) && ! S_IS_COMMON (symbolP))
    {
      as_bad (_("Ignoring attempt to re-define symbol"));
      ignore_rest_of_line ();
      return;
    }
  if (S_GET_VALUE (symbolP) != 0)
    {
      if (S_GET_VALUE (symbolP) != (valueT) size)
	{
	  as_warn (_("Length of .comm \"%s\" is already %ld. Not changed to %d."),
		   S_GET_NAME (symbolP), (long) S_GET_VALUE (symbolP), size);
	}
    }
  know (symbolP->sy_frag == &zero_address_frag);
  if (*input_line_pointer != ',')
    temp = 0;
  else
    {
      input_line_pointer++;
      SKIP_WHITESPACE ();
      temp = get_absolute_expression ();
      if (temp < 0)
	{
	  temp = 0;
	  as_warn (_("Common alignment negative; 0 assumed"));
	}
    }
  if (temp)
    {
      /* convert to a power of 2 alignment */
      for (align = 0; (temp & 1) == 0; temp >>= 1, ++align);
      if (temp != 1)
	{
	  as_bad (_("Common alignment not a power of 2"));
	  ignore_rest_of_line ();
	  return;
	}
    }
  else
    align = 0;
  if (symbol_get_obj (symbolP)->local)
    {
      segT old_sec;
      int old_subsec;
      char *pfrag;
      flagword applicable;

      /* allocate_?bss: */
      old_sec = now_seg;
      old_subsec = now_subseg;

      applicable = bfd_applicable_section_flags (stdoutput);
      applicable &= SEC_ALLOC;

      switch (common_segment & 0x7F)
	{
	case 0: //comm
	  record_alignment (bss_section, align);
	  subseg_set (bss_section, 0);
	  if (align)
	    frag_align (align, 0, 0);
	  if (S_GET_SEGMENT (symbolP) == bss_section)
	    symbol_get_frag (symbolP)->fr_symbol = 0;
	  symbol_set_frag (symbolP, frag_now);
	  pfrag = frag_var (rs_org, 1, 1, (relax_substateT) 0, symbolP,
			    (offsetT) size, (char *) 0);
	  *pfrag = 0;
	  S_SET_SIZE (symbolP, size);
	  S_SET_SEGMENT (symbolP, bss_section);
	  break;
	case 1: //rcomm
	  if (rbss_section == NULL)
	    {
	      rbss_section = subseg_new (".rbss", 0);
	      bfd_set_section_flags (stdoutput, rbss_section, applicable);
	      seg_info (rbss_section)->bss = 1;
	    }
	  record_alignment (rbss_section, align);
	  subseg_set (rbss_section, 0);
	  if (align)
	    frag_align (align, 0, 0);
	  if (S_GET_SEGMENT (symbolP) == rbss_section)
	    symbol_get_frag (symbolP)->fr_symbol = 0;
	  symbol_set_frag (symbolP, frag_now);
	  pfrag = frag_var (rs_org, 1, 1, (relax_substateT) 0, symbolP,
			    (offsetT) size, (char *) 0);
	  *pfrag = 0;
	  S_SET_SIZE (symbolP, size);
	  S_SET_SEGMENT (symbolP, rbss_section);
	  break;
	case 2: //bitcomm
	  if (bitbss_section == NULL)
	    {
	      bitbss_section = subseg_new (".bitbss", 0);
	      bfd_set_section_flags (stdoutput, bitbss_section, applicable);
	      seg_info (bitbss_section)->bss = 1;
	    }
	  record_alignment (bitbss_section, align);
	  subseg_set (bitbss_section, 0);
	  if (align)
	    frag_align (align, 0, 0);
	  if (S_GET_SEGMENT (symbolP) == bitbss_section)
	    symbol_get_frag (symbolP)->fr_symbol = 0;
	  symbol_set_frag (symbolP, frag_now);
	  pfrag = frag_var (rs_org, 1, 1, (relax_substateT) 0, symbolP,
			    (offsetT) size, (char *) 0);
	  *pfrag = 0;
	  S_SET_SIZE (symbolP, size);
	  S_SET_SEGMENT (symbolP, bitbss_section);
	  break;
	case 3: //icomm
	  if (ibss_section == NULL)
	    {
	      ibss_section = subseg_new (".ibss", 0);
	      bfd_set_section_flags (stdoutput, ibss_section, applicable);
	      seg_info (ibss_section)->bss = 1;
	    }
	  record_alignment (ibss_section, align);
	  subseg_set (ibss_section, 0);
	  if (align)
	    frag_align (align, 0, 0);
	  if (S_GET_SEGMENT (symbolP) == ibss_section)
	    symbol_get_frag (symbolP)->fr_symbol = 0;
	  symbol_set_frag (symbolP, frag_now);
	  pfrag = frag_var (rs_org, 1, 1, (relax_substateT) 0, symbolP,
			    (offsetT) size, (char *) 0);
	  *pfrag = 0;
	  S_SET_SIZE (symbolP, size);
	  S_SET_SEGMENT (symbolP, ibss_section);
	  break;
	case 4: //xcomm
	  if (xbss_section == NULL)
	    {
	      xbss_section = subseg_new (".xbss", 0);
	      bfd_set_section_flags (stdoutput, xbss_section, applicable);
	      seg_info (xbss_section)->bss = 1;
	    }
	  record_alignment (xbss_section, align);
	  subseg_set (xbss_section, 0);
	  if (align)
	    frag_align (align, 0, 0);
	  if (S_GET_SEGMENT (symbolP) == xbss_section)
	    symbol_get_frag (symbolP)->fr_symbol = 0;
	  symbol_set_frag (symbolP, frag_now);
	  pfrag = frag_var (rs_org, 1, 1, (relax_substateT) 0, symbolP,
			    (offsetT) size, (char *) 0);
	  *pfrag = 0;
	  S_SET_SIZE (symbolP, size);
	  S_SET_SEGMENT (symbolP, xbss_section);
	  break;
	case 5: //ecomm
	  if (ebss_section == NULL)
	    {
	      ebss_section = subseg_new (".ebss", 0);
	      bfd_set_section_flags (stdoutput, ebss_section, applicable);
	      seg_info (ebss_section)->bss = 1;
	    }
	  record_alignment (ebss_section, align);
	  subseg_set (ebss_section, 0);
	  if (align)
	    frag_align (align, 0, 0);
	  if (S_GET_SEGMENT (symbolP) == ebss_section)
	    symbol_get_frag (symbolP)->fr_symbol = 0;
	  symbol_set_frag (symbolP, frag_now);
	  pfrag = frag_var (rs_org, 1, 1, (relax_substateT) 0, symbolP,
			    (offsetT) size, (char *) 0);
	  *pfrag = 0;
	  S_SET_SIZE (symbolP, size);
	  S_SET_SEGMENT (symbolP, ebss_section);
	  break;
	case 6: //bcomm
	  if (bbss_section == NULL)
	    {
	      bbss_section = subseg_new (".bbss", 0);
	      bfd_set_section_flags (stdoutput, bbss_section, applicable);
	      seg_info (bbss_section)->bss = 1;
	    }
	  record_alignment (bbss_section, align);
	  subseg_set (bbss_section, 0);
	  if (align)
	    frag_align (align, 0, 0);
	  if (S_GET_SEGMENT (symbolP) == bbss_section)
	    symbol_get_frag (symbolP)->fr_symbol = 0;
	  symbol_set_frag (symbolP, frag_now);
	  pfrag = frag_var (rs_org, 1, 1, (relax_substateT) 0, symbolP,
			    (offsetT) size, (char *) 0);
	  *pfrag = 0;
	  S_SET_SIZE (symbolP, size);
	  S_SET_SEGMENT (symbolP, bbss_section);
	  break;
	}
      S_CLEAR_EXTERNAL (symbolP);
      subseg_set (old_sec, old_subsec);
    }
  else
    {
      S_SET_VALUE (symbolP, (valueT) size);
      S_SET_ALIGN (symbolP, temp);
      S_SET_EXTERNAL (symbolP);
      switch (common_segment) {
      case 0: //comm
	S_SET_SEGMENT (symbolP, bfd_com_section_ptr);
	break;
      case 1: //rcomm
	S_SET_SEGMENT (symbolP, &rcom_section);
	break;
      case 2: //bitcomm
	S_SET_SEGMENT (symbolP, &bitcom_section);
	break;
      case 3: //icomm
	S_SET_SEGMENT (symbolP, &icom_section);
	break;
      case 4: //xcomm
	S_SET_SEGMENT (symbolP, &xcom_section);
	break;
      case 5: //ecomm
	S_SET_SEGMENT (symbolP, &ecom_section);
	break;
      case 6: //bcomm
	S_SET_SEGMENT (symbolP, &bcom_section);
	break;
      }
    }
  
  symbol_get_bfdsym (symbolP)->flags |= BSF_OBJECT;

  demand_empty_rest_of_line ();
  return;
}

void
i51_rbss (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  flagword applicable;
  if (rbss_section == NULL)
    {
      applicable = bfd_applicable_section_flags (stdoutput);
      applicable &= SEC_ALLOC;
      rbss_section = subseg_new (".rbss", 0);
      bfd_set_section_flags (stdoutput, rbss_section, applicable);
      seg_info (rbss_section)->bss = 1;
    }
  obj_elf_section_change_hook ();
  subseg_set (rbss_section, (subsegT) get_absolute_expression ());
  demand_empty_rest_of_line ();
}

void
i51_bss (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  obj_elf_section_change_hook ();
  subseg_set (bss_section, (subsegT) get_absolute_expression ());
  demand_empty_rest_of_line ();
}

void
i51_bbss (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  flagword applicable;
  if (bbss_section == NULL)
    {
      applicable = bfd_applicable_section_flags (stdoutput);
      applicable &= SEC_ALLOC;
      bbss_section = subseg_new (".bbss", 0);
      bfd_set_section_flags (stdoutput, bbss_section, applicable);
      seg_info (bbss_section)->bss = 1;
    }
  obj_elf_section_change_hook ();
  subseg_set (bbss_section, (subsegT) get_absolute_expression ());
  demand_empty_rest_of_line ();
}

void
i51_ibss (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  flagword applicable;
  if (ibss_section == NULL)
    {
      applicable = bfd_applicable_section_flags (stdoutput);
      applicable &= SEC_ALLOC;
      ibss_section = subseg_new (".ibss", 0);
      bfd_set_section_flags (stdoutput, ibss_section, applicable);
      seg_info (ibss_section)->bss = 1;
    }
  obj_elf_section_change_hook ();
  subseg_set (ibss_section, (subsegT) get_absolute_expression ());
  demand_empty_rest_of_line ();
}

void
i51_xbss (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  flagword applicable;
  if (xbss_section == NULL)
    {
      applicable = bfd_applicable_section_flags (stdoutput);
      applicable &= SEC_ALLOC;
      xbss_section = subseg_new (".xbss", 0);
      bfd_set_section_flags (stdoutput, xbss_section, applicable);
      seg_info (xbss_section)->bss = 1;
    }
  obj_elf_section_change_hook ();
  subseg_set (xbss_section, (subsegT) get_absolute_expression ());
  demand_empty_rest_of_line ();
}

void
i51_ebss (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  flagword applicable;
  if (ebss_section == NULL)
    {
      applicable = bfd_applicable_section_flags (stdoutput);
      applicable &= SEC_ALLOC;
      ebss_section = subseg_new (".ebss", 0);
      bfd_set_section_flags (stdoutput, ebss_section, applicable);
      seg_info (ebss_section)->bss = 1;
    }
  obj_elf_section_change_hook ();
  subseg_set (ebss_section, (subsegT) get_absolute_expression ());
  demand_empty_rest_of_line ();
}

void
i51_bitbss (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  flagword applicable;
  if (bitbss_section == NULL)
    {
      applicable = bfd_applicable_section_flags (stdoutput);
      applicable &= SEC_ALLOC;
      bitbss_section = subseg_new (".bitbss", 0);
      bfd_set_section_flags (stdoutput, bitbss_section, applicable);
      seg_info (bitbss_section)->bss = 1;
    }
  obj_elf_section_change_hook ();
  subseg_set (bitbss_section, (subsegT) get_absolute_expression ());
  demand_empty_rest_of_line ();
}

void
i51_rdata (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  flagword applicable;
  if (rdata_section == NULL)
    {
      applicable = bfd_applicable_section_flags (stdoutput);
      applicable &= (SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_DATA);
      rdata_section = subseg_new (".rdata", 0);
      bfd_set_section_flags (stdoutput, rdata_section, applicable);
    }
  obj_elf_section_change_hook ();
  subseg_set (rdata_section, (subsegT) get_absolute_expression ());
  demand_empty_rest_of_line ();
}

void
i51_bdata (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  flagword applicable;
  if (bdata_section == NULL)
    {
      applicable = bfd_applicable_section_flags (stdoutput);
      applicable &= (SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_DATA);
      bdata_section = subseg_new (".bdata", 0);
      bfd_set_section_flags (stdoutput, bdata_section, applicable);
    }
  obj_elf_section_change_hook ();
  subseg_set (bdata_section, (subsegT) get_absolute_expression ());
  demand_empty_rest_of_line ();
}

void
i51_idata (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  flagword applicable;
  if (idata_section == NULL)
    {
      applicable = bfd_applicable_section_flags (stdoutput);
      applicable &= (SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_DATA);
      idata_section = subseg_new (".idata", 0);
      bfd_set_section_flags (stdoutput, idata_section, applicable);
    }
  obj_elf_section_change_hook ();
  subseg_set (idata_section, (subsegT) get_absolute_expression ());
  demand_empty_rest_of_line ();
}

void
i51_xdata (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  flagword applicable;
  if (xdata_section == NULL)
    {
      applicable = bfd_applicable_section_flags (stdoutput);
      applicable &= (SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_DATA);
      xdata_section = subseg_new (".xdata", 0);
      bfd_set_section_flags (stdoutput, xdata_section, applicable);
    }
  obj_elf_section_change_hook ();
  subseg_set (xdata_section, (subsegT) get_absolute_expression ());
  demand_empty_rest_of_line ();
}

void
i51_edata (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  flagword applicable;
  if (edata_section == NULL)
    {
      applicable = bfd_applicable_section_flags (stdoutput);
      applicable &= (SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_DATA);
      edata_section = subseg_new (".edata", 0);
      bfd_set_section_flags (stdoutput, edata_section, applicable);
    }
  obj_elf_section_change_hook ();
  subseg_set (edata_section, (subsegT) get_absolute_expression ());
  demand_empty_rest_of_line ();
}

void
i51_bitdata (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  flagword applicable;
  if (bitdata_section == NULL)
    {
      applicable = bfd_applicable_section_flags (stdoutput);
      applicable &= (SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_DATA);
      bitdata_section = subseg_new (".bitdata", 0);
      bfd_set_section_flags (stdoutput, bitdata_section, applicable);
    }
  obj_elf_section_change_hook ();
  subseg_set (bitdata_section, (subsegT) get_absolute_expression ());
  demand_empty_rest_of_line ();
}

void
i51_eeprom (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  flagword applicable;
  if (eeprom_section == NULL)
    {
      applicable = bfd_applicable_section_flags (stdoutput);
      applicable &= (SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_DATA);
      eeprom_section = subseg_new (".eeprom", 0);
      bfd_set_section_flags (stdoutput, eeprom_section, applicable);
    }
  obj_elf_section_change_hook ();
  subseg_set (eeprom_section, (subsegT) get_absolute_expression ());
  demand_empty_rest_of_line ();
}

void
pcodeOperand (oper, mode, separator)
     expressionS *oper;
     enum PADRMODE *mode;
     int separator;
{
  SKIP_WHITESPACE ();
  if ((*input_line_pointer != '\0') && (*input_line_pointer != '\x0D') && (*input_line_pointer != '\x0A') && (*input_line_pointer != ';'))
    {
      if (separator)
	{
	  if (*input_line_pointer != ',')
	    {
	      as_bad (_("Expected comma after pcode operand"));
	      ignore_rest_of_line ();
	      return;
	    }
	  input_line_pointer++;		/* skip ',' */
	  SKIP_WHITESPACE ();
	}
      //search #, #BYTE, #SWAP, #SHL8, BYTE, SWAP, SHL8, @BYTE, @SWAP, @
      if      (strncmp (input_line_pointer, "#BYTE", 5) == 0) { *mode = DIRECTBYTE; input_line_pointer += 5; }
      else if (strncmp (input_line_pointer, "#SWAP", 5) == 0) { *mode = DIRECTSWAP; input_line_pointer += 5; }
      else if (strncmp (input_line_pointer, "#SHL8", 5) == 0) { *mode = DIRECTSHL8; input_line_pointer += 5; }
      else if (strncmp (input_line_pointer, "#WORD", 5) == 0) { *mode = DIRECTWORD; input_line_pointer += 5; }
      else if (strncmp (input_line_pointer, "#",     1) == 0) { *mode = DIRECTWORD; input_line_pointer += 1; }
      else if (strncmp (input_line_pointer, "BYTE",  4) == 0) { *mode = DIRECTBYTE; input_line_pointer += 4; }
      else if (strncmp (input_line_pointer, "SWAP",  4) == 0) { *mode = DIRECTSWAP; input_line_pointer += 4; }
      else if (strncmp (input_line_pointer, "SHL8",  4) == 0) { *mode = DIRECTSHL8; input_line_pointer += 4; }
      else if (strncmp (input_line_pointer, "WORD",  4) == 0) { *mode = DIRECTWORD; input_line_pointer += 4; }
      else if (strncmp (input_line_pointer, "@BYTE", 5) == 0) { *mode = INDIRECTBYTE; input_line_pointer += 5; }
      else if (strncmp (input_line_pointer, "@SWAP", 5) == 0) { *mode = INDIRECTSWAP; input_line_pointer += 5; }
      else if (strncmp (input_line_pointer, "@WORD", 5) == 0) { *mode = INDIRECTWORD; input_line_pointer += 5; }
      else if (strncmp (input_line_pointer, "@",     1) == 0) { *mode = INDIRECTWORD; input_line_pointer += 1; }
      else                                                    { *mode = DIRECTWORD; }
      SKIP_WHITESPACE ();
      expression (oper);
    }
  else
    {
      oper->X_op = O_constant;
      oper->X_add_number = 0;
      *mode = DIRECTBYTE;
    }
}


void
writePcodeOperand (oper, mode, swapable)
     expressionS *oper;
     enum PADRMODE *mode;
     int swapable ATTRIBUTE_UNUSED;
{
  char *f;
  fixS *fixp;

  switch (*mode) {
  case DIRECTBYTE:
  case DIRECTSHL8:
  case INDIRECTWORD:
  case INDIRECTSWAP:
    // byte mode
      if (oper->X_op == O_constant)
	{
	  if (0x00FF & oper->X_add_number)
	    {
	      f = frag_more (1);
	      number_to_chars_bigendian (f, 0x00FF & oper->X_add_number, 1);
	    }
	}
      else
	{
	  f = frag_more (1);
	  /* Now create a 8-bit fixup. */
	  fixp = fix_new_exp (frag_now, f - frag_now->fr_literal, 1,
			      oper,
			      FALSE,
			      BFD_RELOC_8);
	  number_to_chars_bigendian (f, 0, 1);
	}
      break;
  case INDIRECTBYTE:
    // indirect byte mode
      if (oper->X_op == O_constant)
	{
	  if (0x00FF & oper->X_add_number)
	    {
	      f = frag_more (1);
	      number_to_chars_bigendian (f, 0x00FF & (oper->X_add_number - 1), 1);
	    }
	}
      else
	{
	  f = frag_more (1);
	  /* Now create a 8-bit fixup. */
	  fixp = fix_new_exp (frag_now, f - frag_now->fr_literal, 1,
			      oper,
			      FALSE,
			      BFD_RELOC_8);
	  number_to_chars_bigendian (f, -1, 1);
	}
      break;
  case DIRECTWORD:
    // word mode
    if (oper->X_op == O_constant)
      {
	if (0xFF00 & oper->X_add_number)  //MSB
	  {
	    f = frag_more (1);
	    number_to_chars_bigendian (f, 0x00FF & (oper->X_add_number >> 8), 1);
	  }
	if (0x00FF & oper->X_add_number)  //LSB
	  {
	    f = frag_more (1);
	    number_to_chars_bigendian (f, 0x00FF & oper->X_add_number, 1);
	  }
      }
    else
      {
	f = frag_more (2);
	/* Now create a 16-bit fixup. */
	fixp = fix_new_exp (frag_now, f - frag_now->fr_literal, 2,
			    oper,
			    FALSE,
			    BFD_RELOC_16);
	number_to_chars_bigendian (f, 0, 2);
      }
    break;
  case DIRECTSWAP:
    //direct swap word mode
    if (oper->X_op == O_constant)
      { //direct swap mode
	if (0x00FF & oper->X_add_number)  //LSB
	  {
	    f = frag_more (1);
	    number_to_chars_bigendian (f, 0x00FF & oper->X_add_number, 1);
	  }
	if (0xFF00 & oper->X_add_number)  //MSB
	  {
	    f = frag_more (1);
	    number_to_chars_bigendian (f, 0x00FF & (oper->X_add_number >> 8), 1);
	  }
      }
    else
      {
	f = frag_more (2);
	/* Now create a 16-bit fixup. */
	fixp = fix_new_exp (frag_now, f - frag_now->fr_literal, 2,
			    oper,
			    FALSE,
			    BFD_RELOC_16);
	number_to_chars_bigendian (f, 0, 2);
      }
    break;
  }
}

unsigned short
decodePcodeOperand (oper, mode, swapable)
     expressionS *oper;
     enum PADRMODE *mode;
     int swapable;
{
  unsigned short pflags;

  pflags = 0;

  switch (*mode) {
  case DIRECTBYTE:
  case INDIRECTWORD:
  case INDIRECTBYTE:
    if (oper->X_op == O_constant)
      {
	if (!check_range (oper->X_add_number, I51_OP_IMM8))
	  {
	    as_bad (_("operand out of 8-bit range: `%ld'."),
		    oper->X_add_number);
	  }
	if (0x00FF & oper->X_add_number)
	  {
	    pflags |= 0x40;
	  }
      }
    else
      {
	pflags |= 0x40;
      }
    break;
  case INDIRECTSWAP:
    if (oper->X_op == O_constant)
      {
	if (!check_range (oper->X_add_number, I51_OP_IMM8))
	  {
	    as_bad (_("operand out of 8-bit range: `%ld'."),
		    oper->X_add_number);
	  }
	if (0x00FF & oper->X_add_number)
	  {
	    pflags |= 0x40;   //allocate 8 bit
	  }
	pflags |= 0x02;       //set swap flag
      }
    else
      {
	if (swapable == 0)
	  {
	    as_bad (_("Pcode operand 2 isn't swapable"));
	  }
	pflags |= 0x42;       //allocate 8 bit and set swap flag
      }
    break;
  case DIRECTSHL8:
    if (oper->X_op == O_constant)
      {
	if (!check_range (oper->X_add_number, I51_OP_IMM8))
	  {
	    as_bad (_("operand out of 8-bit range: `%ld'."),
		    oper->X_add_number);
	  }
	if (0x00FF & oper->X_add_number)
	  {
	    pflags |= 0x80;
	  }
      }
    else
      {
	pflags |= 0x80;
      }
    break;
  case DIRECTWORD:
    //word mode
    if (oper->X_op == O_constant)
      {
	if (!check_range (oper->X_add_number, I51_OP_IMM16))
	  {
	    as_bad (_("operand out of 16-bit range: `%ld'."),
		    oper->X_add_number);
	  }
	//direct no swap mode
	if (0xFF00 & oper->X_add_number) pflags |= 0x80; //MSB
	if (0x00FF & oper->X_add_number) pflags |= 0x40; //LSB
      }
    else
      {
	pflags |= 0xC0;
      }
    break;
  case DIRECTSWAP:
    //word mode
    if (oper->X_op == O_constant)
      {
	if (!check_range (oper->X_add_number, I51_OP_IMM16))
	  {
	    as_bad (_("operand out of 16-bit range: `%ld'."),
		    oper->X_add_number);
	  }
	//direct swap mode (#SWAP)
	if (0x00FF & oper->X_add_number) pflags |= 0x80; //LSB
	if (0xFF00 & oper->X_add_number) pflags |= 0x40; //MSB
	pflags |= 0x02;       //swap flag
      }
    else
      {
	if (swapable == 0)
	  {
	    as_bad (_("Pcode operand 2 isn't swapable"));
	  }
	pflags |= 0xC2;       //allocate 16 bit and set swap flag
      }
    break;
  }
  switch (*mode) {
  case INDIRECTWORD:
  case INDIRECTBYTE:
  case INDIRECTSWAP:
    pflags |= 0x8000;         //indirect flag
  default:
    break;
  }
  return pflags;
}

void
i51_pcode (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  char *f;
  unsigned short pflags;
  unsigned short temppflags;

  expression (&op_expr1);
  pcodeOperand (&op_expr2, &p2mode, 0);
  pcodeOperand (&op_expr3, &p3mode, 1);
  pcodeOperand (&op_expr4, &p4mode, 1);
  demand_empty_rest_of_line ();

  pflags     = decodePcodeOperand (&op_expr2, &p2mode, 1);
  temppflags = decodePcodeOperand (&op_expr3, &p3mode, 0);
  pflags    |= (temppflags & 0x00C0) >> 2;                 //data allocation
  pflags    |= (temppflags & 0x8000) >> 1;                 //indirect flag
  temppflags = decodePcodeOperand (&op_expr4, &p4mode, 1);
  pflags    |= (temppflags & 0x00C0) >> 4;                 //data allocation
  pflags    |= (temppflags & 0x8000) >> 2;                 //indirect flag
  pflags    |= (temppflags & 0x0002) >> 1;                 //swap flag

  /* pcode exec address */
  f = frag_more (2);
  temppflags = pflags & 0xE000;
  if (op_expr1.X_op == O_constant)
    {
      if (op_expr1.X_add_number > 0x1FFF)
	{
	  as_bad (_("Pcode exec address out of 13-bit range: `%ld'."),
		  op_expr4.X_add_number);
	}
      if (op_expr1.X_add_number < 0x0100)
	{
	  as_bad (_("Pcode exec addrss uderflow: `%ld'."),
		  op_expr4.X_add_number);
	}
      temppflags |= op_expr1.X_add_number & 0x1FFF;
      number_to_chars_bigendian (f, temppflags, 2);
    }
  else if (op_expr1.X_op != O_register)
    {
      fixS *fixp;

      /* Now create a 13-bit fixup.  */
      fixp = fix_new_exp (frag_now, f - frag_now->fr_literal, 2,
			  &op_expr1,
			  FALSE,
			  BFD_RELOC_I51_13_PCODE);
      number_to_chars_bigendian (f, temppflags, 2);
    }
  else
    {
      as_fatal (_("Operand `%x' not recognized in pcode."), op_expr1.X_op);
    }

  /* pcode flags */
  f = frag_more (1);
  number_to_chars_bigendian (f, pflags & 0xFF, 1);
  /* 1th pcode operand */
  writePcodeOperand (&op_expr2, &p2mode, 1);
  /* 2nd pcode operand */
  writePcodeOperand (&op_expr3, &p3mode, 0);
  /* 3th pcode operand */
  writePcodeOperand (&op_expr4, &p4mode, 1);
}
//id for @lo=1 @hi=2
static int exp_mod = 0;
#define exp_mod_lo_label 1
#define exp_mod_hi_label 2
#define exp_mod_lo_const 3
#define exp_mod_hi_const 4
#define exp_mod_none 0
void
i51_parse_cons_expression (exp, nbytes)
     expressionS *exp;
     int nbytes;
{
  char *tmp;
  exp_mod = exp_mod_none;

  tmp = input_line_pointer = skip_space (input_line_pointer);
//  fprintf(stderr,"-->%.20s<--\n",input_line_pointer);
      char pm_name[10];
      strcpy(&pm_name[0],"lo_");
      int len;
      len = strlen (pm_name);
      if (strncasecmp (input_line_pointer, &pm_name[0], len) == 0)
        {
          input_line_pointer = skip_space (input_line_pointer + len);
          if (*input_line_pointer == '(')
            {
              input_line_pointer = skip_space (input_line_pointer + 1);
              exp_mod = exp_mod_lo_label;
//              fprintf(stderr,"Generate Expression Begin lo_\n");
              expression (exp);
//              fprintf(stderr,"Exp %x %d\n",exp->X_add_number,exp->X_op);
              if (exp->X_op==O_constant) { exp->X_add_number&=0xFF; exp_mod=exp_mod_lo_const; }
//              fprintf(stderr,"New Exp %x %d\n",exp->X_add_number,exp->X_op);
//              fprintf(stderr,"Generate Expression End lo_\n");
              if (*input_line_pointer == ')')
                ++input_line_pointer;
              else
                {
                  as_bad (_("`)' required"));
                  exp_mod = exp_mod_none;
                }
              return;
            }
          input_line_pointer = tmp;
        }
      strcpy(&pm_name[0],"hi_");
      len = strlen (pm_name);
      if (strncasecmp (input_line_pointer, &pm_name[0], len) == 0)
        {
          input_line_pointer = skip_space (input_line_pointer + len);
          if (*input_line_pointer == '(')
            {
              input_line_pointer = skip_space (input_line_pointer + 1);
              exp_mod = exp_mod_hi_label;
//              fprintf(stderr,"Generate Expression Begin hi_\n");
              expression (exp);
//              fprintf(stderr,"Exp %x %d\n",exp->X_add_number,exp->X_op);
              if (exp->X_op==O_constant) { exp->X_add_number=exp->X_add_number >> 8; exp_mod=exp_mod_hi_const; }
//              fprintf(stderr,"New Exp %x %d\n",exp->X_add_number,exp->X_op);
//              fprintf(stderr,"Generate Expression End hi_\n");
              if (*input_line_pointer == ')')
                ++input_line_pointer;
              else
                {
                  as_bad (_("`)' required"));
                  exp_mod = exp_mod_none;
                }
              return;
            }
          input_line_pointer = tmp;
        }
      //remove the #
      if (*input_line_pointer=='#')
        {
          ++input_line_pointer;
        }
//  fprintf(stderr,"Generate Expression Begin \n");
  expression (exp);
//  fprintf(stderr,"Generate Expression End \n");
}

fixS *
i51_fix_new_exp (frag, where, size, exp, pcrel, r_type)
     fragS *frag;               /* Which frag?  */
     int where;                 /* Where in that frag?  */
     int size;                  /* 1, 2, or 4 usually.  */
     expressionS *exp;          /* Expression.  */
     int pcrel;                 /* TRUE if PC-relative relocation.  */
#ifdef BFD_ASSEMBLER
     bfd_reloc_code_real_type r_type; /* Relocation type.  */
#else
     int r_type;                /* Relocation type.  */
#endif
{
  symbolS *add = NULL;
  symbolS *sub = NULL;
  offsetT off = 0;

  switch (exp->X_op)
    {
    case O_absent:
      fprintf(stderr,"O_absent\n");
      break;

    case O_register:
      as_bad (_("register value used as expression"));
      break;

    case O_add:
      /* This comes up when _GLOBAL_OFFSET_TABLE_+(.-L0) is read, if
         the difference expression cannot immediately be reduced.  */
      {
        symbolS *stmp = make_expr_symbol (exp);

        exp->X_op = O_symbol;
        exp->X_op_symbol = 0;
        exp->X_add_symbol = stmp;
        exp->X_add_number = 0;
        fprintf(stderr,"O_add\n");
        return i51_fix_new_exp (frag, where, size, exp, pcrel, r_type);
      }

    case O_uminus:
      sub = exp->X_add_symbol;
      off = exp->X_add_number;
      fprintf(stderr,"O_uminus\n");
      break;

    case O_subtract:
      sub = exp->X_op_symbol;
      fprintf(stderr,"O_subtract\n");
      /* Fall through.  */
    case O_symbol:
      add = exp->X_add_symbol;
      fprintf(stderr,"O_symbol\n");
      /* Fall through.  */
    case O_constant:
      off = exp->X_add_number;
      fprintf(stderr,"O_constant\n");
      break;

    default:
      add = make_expr_symbol (exp);
      fprintf(stderr,"add = make_expr_symbol (exp)\n");
      break;
    }

//  return fix_new_internal (frag, where, size, add, sub, off, pcrel, r_type);
return NULL;
}

void
i51_cons_fix_new (frag, where, nbytes, exp)
fragS *frag;
int where;
int nbytes;
expressionS *exp;
{
  if (exp_mod == exp_mod_none)
    {
      if (nbytes == 1)
        {
        fix_new_exp (frag, where, nbytes, exp, FALSE, BFD_RELOC_I51_8_LOW);
        }
//      else if (nbytes==2)
//      {
//        fprintf(stderr,"where %d",where);
//        fix_new_exp (frag, where, 2, exp, FALSE, BFD_RELOC_I51_8_LOW);
//        //fix_new_exp (frag, where+1, 1, exp, FALSE, BFD_RELOC_I51_8_HIGH);
//      }
      else
        as_bad (_("illegal %srelocation size: %d"), "", nbytes);
    }
  else if (exp_mod == exp_mod_hi_label)
    {
      if (nbytes == 1)
        {
//        i51_fix_new_exp (frag, where, nbytes, exp, FALSE, BFD_RELOC_I51_8_HIGH);
        fix_new_exp (frag, where, nbytes, exp, FALSE, BFD_RELOC_I51_8_HIGH);
        }
      else
        as_bad (_("illegal %srelocation size: %d"), "`pm' ", nbytes);
      exp_mod = 0;
    }
  else if (exp_mod == exp_mod_lo_label)
    {
      if (nbytes == 1)
        {
//        i51_fix_new_exp (frag, where, nbytes, exp, FALSE, BFD_RELOC_I51_8_LOW);
        fix_new_exp (frag, where, nbytes, exp, FALSE, BFD_RELOC_I51_8_LOW);
        }
      else
        as_bad (_("illegal %srelocation size: %d"), "`pm' ", nbytes);
      exp_mod = 0;
    }
  else if (exp_mod == exp_mod_hi_const)
    {
      if (nbytes == 1)
        {
//        i51_fix_new_exp (frag, where, nbytes, exp, FALSE, BFD_RELOC_I51_8_HIGH);
        fix_new_exp (frag, where, nbytes, exp, FALSE, BFD_RELOC_I51_8_LOW);
        }
      else
        as_bad (_("illegal %srelocation size: %d"), "`pm' ", nbytes);
      exp_mod = 0;
    }
  else if (exp_mod == exp_mod_lo_const)
    {
      if (nbytes == 1)
        {
//        i51_fix_new_exp (frag, where, nbytes, exp, FALSE, BFD_RELOC_I51_8_LOW);
        fix_new_exp (frag, where, nbytes, exp, FALSE, BFD_RELOC_I51_8_LOW);
        }
      else
        as_bad (_("illegal %srelocation size: %d"), "`pm' ", nbytes);
      exp_mod = 0;
    }
}
