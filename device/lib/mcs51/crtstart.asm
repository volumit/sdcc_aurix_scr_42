;--------------------------------------------------------------------------
;  crtstart.asm - C run-time: startup
;
;  Copyright (C) 2004, Erik Petrich
;
;  This library is free software; you can redistribute it and/or modify it
;  under the terms of the GNU General Public License as published by the
;  Free Software Foundation; either version 2, or (at your option) any
;  later version.
;
;  This library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License 
;  along with this library; see the file COPYING. If not, write to the
;  Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
;   MA 02110-1301, USA.
;
;  As a special exception, if you link this library with other files,
;  some of which are compiled with SDCC, to produce an executable,
;  this library does not by itself cause the resulting executable to
;  be covered by the GNU General Public License. This exception does
;  not however invalidate any other reasons why the executable file
;  might be covered by the GNU General Public License.
;--------------------------------------------------------------------------

;	.area CSEG    (CODE)
;	.area GSINIT0 (CODE)
;	.area GSINIT1 (CODE)
;	.area GSINIT2 (CODE)
;	.area GSINIT3 (CODE)
;	.area GSINIT4 (CODE)
;	.area GSINIT5 (CODE)
;	.area GSINIT  (CODE)
;	.area GSFINAL (CODE)


	.globl bits
	.globl bits_0
	.globl bits_1
	.globl bits_2
	.globl bits_3
	.globl bits_4
	.globl bits_5
	.globl bits_6
	.globl bits_7
	.section .bits.i51,"aw" ;bit_name ;area
bits_0:
	.ds.b	1
bits_1:
	.ds.b	1
bits_2:
	.ds.b	1
bits_3:
	.ds.b	1
bits_4:
	.ds.b	1
bits_5:
	.ds.b	1
bits_6:
	.ds.b	1
bits_7:
	.ds.b	1
	.section .dbit.i51,"aw" ;bit_name ;area
bits:
	.ds.b	1

	.globl __stack_start
	.globl __sdcc_gsinit_startup
	.globl __sdcc_program_startup
	.globl __sdcc_gsinit1_start
	.globl __sdcc_gsinit2_start
	.globl __sdcc_gsinit3_start
	.globl _main
	.globl __init_table
	.globl _resetvec
	.section .reset,"ax",@progbits
	.type _resetvec,@function
_resetvec:
     ljmp __sdcc_gsinit_startup

;	.area GSINIT0 (CODE)
	.section .text.gsinit0,"ax"
__sdcc_gsinit_startup:
        mov     sp,#__stack_start - 1
		ljmp    __sdcc_gsinit1_start
;	.area GSINIT2 (CODE)
	.section .text.gsinit2,"ax"
__sdcc_gsinit2_start:
        lcall   __sdcc_external_startup
        mov     a,dpl
        jz      __sdcc_init_data
        ljmp    __sdcc_program_startup
__sdcc_init_data:
		ljmp     __sdcc_gsinit3_start
	.section .text.gsfinal,"ax"
__sdcc_program_startup:
;    mov  dptr,#0
    lcall __init_table
	lcall _main
; return from main will lock up
	sjmp .
