;--------------------------------------------------------------------------
;  crtxpop.asm - C run-time: pop registers (not bits) from xstack
;
;  Copyright (C) 2009, Maarten Brock
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

;--------------------------------------------------------
; overlayable bit register bank
;--------------------------------------------------------
;	.area BIT_BANK	(REL,OVR,DATA)
;    .section .bitdata.i51,"w" ;bit_name ;area
;bits:
;	.ds.b 1

;	ar0 = 0x00
;	ar1 = 0x01

;	.area HOME    (CODE)
    .globl bits
	.section .text.home,"ax"
	.using 0
; Pop registers r1..r7 & bits from xstack
; Expect mask in B
___sdcc_xpop_regs:
	mov	a,r0
	mov	r0,_spx
___sdcc_xpop:
	push acc
	mov acc,b
	jbc	acc.0,L00100	;if B(0)=0 then
	dec	r0
	movx	a,@r0		;pop bits
	mov	bits,a
L00100:
	mov acc,b
	jbc	acc.1,L00101	;if B(1)=0 then
	dec	r0
	movx	a,@r0		;pop R1
	mov	r1,a
L00101:
	mov acc,b
	jbc	acc.2,L00102	;if B(2)=0 then
	dec	r0
	movx	a,@r0		;pop R2
	mov	r2,a
L00102:
	mov acc,b
	jbc	acc.3,L00103	;if B(3)=0 then
	dec	r0
	movx	a,@r0		;pop R3
	mov	r3,a
L00103:
	mov acc,b
	jbc	acc.4,L00104	;if B(4)=0 then
	dec	r0
	movx	a,@r0		;pop R4
	mov	r4,a
L00104:
	mov acc,b
	jbc	acc.5,L00105	;if B(5)=0 then
	dec	r0
	movx	a,@r0		;pop R5
	mov	r5,a
L00105:
	mov acc,b
	jbc	acc.6,L00106	;if B(6)=0 then
	dec	r0
	movx	a,@r0		;pop R6
	mov	r6,a
L00106:
	mov acc,b
	jbc	acc.7,L00107	;if B(7)=0 then
	dec	r0
	movx	a,@r0		;pop R7
	mov	r7,a
L00107:
	mov	_spx,r0
	pop	ar0
	ret
