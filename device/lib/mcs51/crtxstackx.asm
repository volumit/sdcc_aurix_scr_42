;--------------------------------------------------------------------------
;  crtxstack.asm - C run-time: setup xstack
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

	.globl __xstack_start
	.globl __XPAGE
	.globl __sdcc_gsinit1_start
	.globl __sdcc_gsinit2_start
	.globl __sdcc_gsinit5_start
	.globl __sdcc_program_startup
;	.area GSINIT1 (CODE)
	.section .text.gsinit1,"ax"
__sdcc_init_xstack:
__sdcc_gsinit1_start:

; Need to initialize in GSINIT1 in case the user's __sdcc_external_startup
; uses the xstack.
	
	mov	__XPAGE,#hi_(__xstack_start)
	mov	_spx,#__xstack_start
	ljmp __sdcc_gsinit2_start
;	.area GSINIT5 (CODE)
	.section .text.gsinit5,"ax"
; Need to initialize in GSINIT5 because __mcs51_genXINIT modifies __XPAGE
; and __mcs51_genRAMCLEAR modifies _spx.
__sdcc_gsinit5_start:
	mov	__XPAGE,#hi_(__xstack_start)
	mov	_spx,#__xstack_start
	ljmp __sdcc_program_startup
