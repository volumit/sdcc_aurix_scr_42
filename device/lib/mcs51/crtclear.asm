;--------------------------------------------------------------------------
;  crtclear.asm - C run-time: clear DATA/IDATA
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

;	.area GSINIT4 (CODE)
	.globl __sdcc_gsinit4_start
	.globl __sdcc_gsinit4B_start
		.section .text.gsinit4,"ax"
__sdcc_gsinit4_start:
__mcs51_genRAMCLEAR:
	clr	a
	mov	r0,#(l_IRAM-1)	; mov r0,a will do for most devices
L00004:	mov	@r0,a
	djnz	r0,L00004
	ljmp    __sdcc_gsinit4B_start
;	_mcs51_genRAMCLEAR() end
