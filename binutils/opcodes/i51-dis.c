/* Disassemble MCS-51 instructions.
   Copyright 2001 Free Software Foundation, Inc.

   Contributed by Radek Benedikt <benedikt@lphard.cz>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include <assert.h>
#include "sysdep.h"
#include "dis-asm.h"
#include "opintl.h"


struct i51_opcodes_s
{
  char *name;
  char *args;
  int insn_size;		/* in bytes */
  unsigned char bin_opcode;
  unsigned char bin_mask;
  unsigned int machine;
};

#define I51_INS(NAME, ARGS, SIZE, OPCODE, MRELOC, BIN, MASK, MACHINE) \
{NAME, ARGS, SIZE, BIN, MASK, MACHINE},

struct i51_opcodes_s i51_opcodes[] =
{
  #include "opcode/i51.h"
  {NULL, NULL, 0, 0, 0, 0}
};

static unsigned char
i51dis_opcode (addr, info)
     bfd_vma addr;
     disassemble_info *info;
{
  bfd_byte buffer[1];
  int status;
  status = info->read_memory_func(addr, buffer, 1, info);
  if (status != 0)
    {
      info->memory_error_func(status, addr, info);
      return -1;
    }
  return buffer[0];
}

static unsigned short
i51dis_op16 (addr, info)
     bfd_vma addr;
     disassemble_info *info;
{
  bfd_byte buffer[2];
  int status;
  status = info->read_memory_func(addr, buffer, 2, info);
  if (status != 0)
    {
      info->memory_error_func(status, addr, info);
      return -1;
    }
  return bfd_getb16(buffer);
}

int
print_insn_i51(addr, info)
     bfd_vma addr;
     disassemble_info *info;
{
  unsigned int insn;
  unsigned int opdata;
  unsigned char rel_addr;
  struct i51_opcodes_s *opcode;
  void *stream = info->stream;
  fprintf_ftype prin = info->fprintf_func;
  int ok = 0;
  int offset = 0;
  char op1[10], op2[10], op3[10], comment1[40], comment2[40];
  int inslen = 1;

  insn = i51dis_opcode (addr, info);
  addr++;  
  for (opcode = i51_opcodes; opcode->name; opcode++)
    {
      if ((insn & opcode->bin_mask) == opcode->bin_opcode)
	break;
    }
  
  op1[0] = 0;
  op2[0] = 0;
  op3[0] = 0;
  comment1[0] = 0;
  comment2[0] = 0;

  if (opcode->name)
    {
      inslen = (opcode->insn_size) & 0x03;
      ok = 1;
      switch (opcode->args[0]) {
	case 'N':	//none
	  break;
	case 'A':	//A reg
	  sprintf (op1,"A");
	  break;
	case 'C':	//AB reg
	  sprintf (op1,"AB");
	  break;
	case 'r':	//C reg
	  sprintf (op1,"C");
	  break;
	case 'P':	//DPTR
	  sprintf (op1,"DPTR");
	  break;
	case '@':	//@A+DPTR
	  sprintf (op1,"@A+DPTR");
	  break;
	case 'T':	//@DPTR
	  sprintf (op1,"@DPTR");
	  break;
	case 'D':	//data
	case 'B':	//bit
	  opdata = i51dis_opcode (addr, info);
	  addr++;
	  offset++;
	  sprintf (op1,"0x%02X",opdata);
	  break;
	case 'R':	//R0..R7
	  sprintf (op1,"R%c",(insn&0x07)+'0');
	  break;
	case 'I':	//@R0, @R1
	  sprintf (op1,"@R%c",(insn&0x01)+'0');
	  break;
	case 'J':	//jump rel
	  opdata = i51dis_opcode (addr, info);
	  addr++;
//
	  rel_addr = opdata+2;
	  if (rel_addr & 0x80) {
	    sprintf (op1,".-0x%02X",0x100-rel_addr);
	    sprintf (comment1, "0x%04lX", addr + opdata - 0x100);
	  } else {
	    sprintf (op1,".+0x%02X",rel_addr);
	    sprintf (comment1, "0x%04lX", addr + opdata);
	  }
	  break;
	case '1':	//jump addr 11
	  opdata = i51dis_opcode (addr, info);
	  addr++;
	  sprintf (op1,"0x%04lX", (addr&0xF800)+(((insn>>5)&0x07)<<8)+opdata);
	  break;
	case '6':	//jump addr 16
	  opdata = i51dis_op16 (addr, info);
	  addr++;
	  sprintf (op1,"0x%04X",opdata);
	  break;
      }
      switch (opcode->args[1]) {
	case 'N':	//none
	  break;
	case 'A':	//A reg
	  sprintf (op2,"A");
	  break;
	case 'r':	//C reg
	  sprintf (op2,"C");
	  break;
	case '@':	//@A+DPTR
	  sprintf (op2,"@A+DPTR");
	  break;
	case 'X':	//@A+PC
	  sprintf (op2,"@A+PC");
	  break;
	case '/':	// /C
	  sprintf (op2,"/C");
	  break;
	case 'T':	//@DPTR
	  sprintf (op2,"@DPTR");
	  break;
	case 'B':	//bit
	case 'D':	//data
	  opdata = i51dis_opcode (addr, info);
	  addr++;
	  offset++;
	  sprintf (op2,"0x%02X",opdata);
	  break;
	case 'R':	//R0..R7
	  sprintf (op2,"R%c",(insn&0x07)+'0');
	  break;
	case 'I':	//@R0, @R1
	  sprintf (op2,"@R%c",(insn&0x01)+'0');
	  break;
	case 'J':	//jump rel
	  opdata = i51dis_opcode (addr, info);
	  addr++;
//
	  rel_addr = opdata+2+offset;
	  if (rel_addr & 0x80) {
	    sprintf (op2,".-0x%02X",0x100-rel_addr);
	    sprintf (comment1, "0x%04lX", addr + opdata - 0x100);
	  } else {
	    sprintf (op2,".+0x%02X",rel_addr);
	    sprintf (comment1, "0x%04lX", addr + opdata);
	  }
	  break;
	case '#':	//#data
	  if(opcode->args[0] == 'P') {
	    opdata = i51dis_op16 (addr, info);
	    addr+=2;
	    sprintf (op2,"#0x%04X",opdata);
	  } else {
	    opdata = i51dis_opcode (addr, info);
	    addr++;
	    offset++;
	    sprintf (op2,"#0x%02X",opdata);
	    if ((opdata >= ' ') && (opdata < 0x7F)) {
	      sprintf (comment1, "#%u\t#'%c'", opdata, opdata);
	    } else {
	      sprintf (comment1, "#%u", opdata);
	    }
	  }
	  break;
      }
      switch (opcode->args[2]) {
	case 'N':	//none
	  break;
	case 'J':	//jump rel
	  opdata = i51dis_opcode (addr, info);
	  addr++;
//
	  rel_addr = opdata+2+offset;
	  if (rel_addr & 0x80) {
	    sprintf (op3,".-0x%02X",0x100-rel_addr);
	    sprintf (comment1, "0x%04lX", addr + opdata - 0x100);
	  } else {
	    sprintf (op3,".+0x%02X",rel_addr);
	    sprintf (comment1, "0x%04lX", addr + opdata);
	  }
	  break;
      }
    }

  if (!ok)
    {
      /* Unknown opcode, or invalid combination of operands.  */
      sprintf (op1, "0x%02X", insn);
      sprintf (comment1, "????");
      comment2[0] = 0;
    }

  (*prin) (stream, "%s", ok ? opcode->name : ".byte");

  if (*op1)
    (*prin) (stream, "\t%s", op1);

  if (*op2)
    (*prin) (stream, ", %s", op2);

  if (*op3)
    (*prin) (stream, ", %s", op3);

  if (*comment1) {
    if (*op2)
      (*prin) (stream, "\t; %s", comment1);
    else
      (*prin) (stream, "\t\t; %s", comment1);
  }

  if (*comment2)
    (*prin) (stream, " %s", comment2);

  return inslen;
}
