;--------------------------------------------------------------------------
;  crtxpush.asm :- C run-time: push registers (not R0) to xstack
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
;	.ds.d 1

;	ar0 = 0x00
;	ar1 = 0x01

;	.area HOME    (CODE)
    .globl bits
	.section .text.home,"ax"
; Push registers r1..r7 & bits on xstack
; Expect allocation size in ACC and mask in B
.using 0
___sdcc_xpush_regs:
	add	a,_spx
	mov	_spx,a
	xch	a,r0
___sdcc_xpush:
	push	acc
	mov acc,b
	jbc	acc.0,L00100	;if B(0)=0 then
	mov	a,bits		;push bits
	dec	r0
	movx	@r0,a
L00100:
	mov acc,b
	jbc	acc.1,L00101	;if B(1)=0 then
	mov	a,r1		;push R1
	dec	r0
	movx	@r0,a
L00101:
	mov acc,b
	jbc	acc.2,L00102	;if B(2)=0 then
	mov	a,r2		;push R2
	dec	r0
	movx	@r0,a
L00102:
	mov acc,b
	jbc	acc.3,L00103	;if B(3)=0 then
	mov	a,r3		;push R3
	dec	r0
	movx	@r0,a
L00103:
	mov acc,b
	jbc	acc.4,L00104	;if B(4)=0 then
	mov	a,r4		;push R4
	dec	r0
	movx	@r0,a
L00104:
	mov acc,b
	jbc	acc.5,L00105	;if B(5)=0 then
	mov	a,r5		;push R5
	dec	r0
	movx	@r0,a
L00105:
	mov acc,b
	jbc	acc.6,L00106	;if B(6)=0 then
	mov	a,r6		;push R6
	dec	r0
	movx	@r0,a
L00106:
	mov acc,b
	jbc	acc.7,L00107	;if B(7)=0 then
	mov	a,r7		;push R7
	dec	r0
	movx	@r0,a
L00107:
	pop	ar0
	ret
