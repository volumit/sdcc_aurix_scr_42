;--------------------------------------------------------------------------
;  crtxclear.asm - C run-time: clear XSEG
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
	.globl __sdcc_gsinit4B_start
	.globl __sdcc_gsinit5_start
	.section .text.gsinit4,"ax"
__sdcc_gsinit4B_start:
__mcs51_genXRAMCLEAR:
	mov	r0,#l_PSEG
	mov	a,r0
	orl	a,#hi_(l_PSEG)
	jz	L00006
	mov	r1,#s_PSEG
	mov	__XPAGE,#hi_(s_PSEG)
	clr     a
.L00005:	movx	@r1,a
	inc	r1
	djnz	r0,.L00005

L00006:
	mov	r0,#l_XSEG
	mov	a,r0
	orl	a,#hi_(l_XSEG)
	jz	L00008
	mov	r1,#hi_(l_XSEG + 255)
	mov	dptr,#s_XSEG
	clr     a
L00007:	movx	@dptr,a
	inc	dptr
	djnz	r0,L00007
	djnz	r1,L00007
L00008:
	ljmp __sdcc_gsinit5_start

