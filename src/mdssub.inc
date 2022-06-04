;======================================================================
; MDSDRV - Mega Drive 68K Sound Driver
;======================================================================
; Copyright (c) 2019-2022 Ian Karlsson
;
; This software is provided 'as-is', without any express or implied
; warranty. In no event will the authors be held liable for any damages
; arising from the use of this software.
;
; Permission is granted to anyone to use this software for any purpose,
; including commercial applications, and to alter it and redistribute
; it freely, subject to the following restrictions:
;
; 1. The origin of this software must not be misrepresented; you must
;    not claim that you wrote the original software. If you use this
;    software in a product, an acknowledgment in the product
;    documentation would be appreciated but is not required.
; 2. Altered source versions must be plainly marked as such, and must
;    not be misrepresented as being the original software.
; 3. This notice may not be removed or altered from any source
;    distribution.
;======================================================================

;======================================================================
; Z80 public variables
;======================================================================

z_busy			equ		$0e00	; Indicates Z80 is writing to FM chip
z_load			equ		$0e01	; Number of buffered samples
z_mode_change	equ		$0e02	; Set to 1 to change mixing mode
z_key_on		equ		$0e03	; Set to 1 after setting Key on
z_min_buffer	equ		$0e04	; Requested minimum buffer size
z_mode			equ		$0e05	; Current mixing mode

z_dummy			equ		$0e06	; just a dummy variable to burn cycles

z_pcm1			equ		$0e08		; PCM 1 variables
z_pcm2			equ		z_pcm1+8	; PCM 2 variables
z_pcm3			equ		z_pcm2+8	; PCM 3 variables

zp_key_on		equ		0
zp_vol			equ		1
zp_pitch		equ		2
zp_bank			equ		3
zp_addr			equ		4
zp_count		equ		6

z_pcm1_key_on	equ		z_pcm1+0	; Set to 1 for key on
z_pcm1_vol		equ		z_pcm1+1	; Volume (10-1f). Key off (f0-ff)
z_pcm1_pitch	equ		z_pcm1+2	; Pitch (00-07)
z_pcm1_bank		equ		z_pcm1+3	; Bank
z_pcm1_addr		equ		z_pcm1+4	; Address
z_pcm1_count	equ		z_pcm1+6	; Number of buffers to play

z_pcm2_key_on	equ		z_pcm2+0
z_pcm2_vol		equ		z_pcm2+1
z_pcm2_pitch	equ		z_pcm2+2
z_pcm2_bank		equ		z_pcm2+3
z_pcm2_addr		equ		z_pcm2+4
z_pcm2_count	equ		z_pcm2+6

z_pcm3_key_on	equ		z_pcm3+0
z_pcm3_vol		equ		z_pcm3+1
z_pcm3_pitch	equ		z_pcm3+2
z_pcm3_bank		equ		z_pcm3+3
z_pcm3_addr		equ		z_pcm3+4
z_pcm3_count	equ		z_pcm3+6

z_stack			equ		$0f00
z_buffer		equ		$1f00

z_vtab			equ		$0f00

z_pcm_load		equ		z_load


;======================================================================
; Z80/68K communication macros
;======================================================================
	if	ASM68K

;======================================================================
; PCM volume calculation
;----------------------------------------------------------------------
; return volume in d1
mds_z80_get_vol	macro
	moveq	#15,d2
	move.b	t_vol(twork),d1
	bmi.s	@no_conversion\@
	ext.w	d1
	bsr.w	mds_pcm_convert_vol
@no_conversion\@
	not.b	d1
	and.b	d2,d1
	add.b	1+w_volume(work,rnum),d1
	cmp.b	d2,d1
	bcs.s	@no_clamp\@
	move.b	d2,d1
@no_clamp\@
	add.b	d2,d1
	endm

;======================================================================
; request Z80 bus for sending a request
;----------------------------------------------------------------------
mds_z80_wait_req	macro
@stop_z80\@
	move.w	#$0100,(zreq)
@wait_z80\@
	btst.b	#0,(zreq)
	bne.s	@wait_z80\@
	endm

;======================================================================
; request Z80 bus for writing an FM register
;----------------------------------------------------------------------
mds_z80_wait_fm		macro
@stop_z80\@
	move.w	#$0100,(zreq)
@wait_z80\@
	btst.b	#0,(zreq)
	bne.s	@wait_z80\@
	tst.b	z_busy(zram)
	beq.s	@done\@
	move.w	#$0000,(zreq)		; z80 is writing to the FM chip
	movem.l	d0-d4,-(sp)								; 8+40 : 48
	movem.l	(sp)+,d0-d4								;      : 96
	bra.s	@stop_z80\@								;   10 : 106
@done\@
	endm

;======================================================================
; start the Z80 again
;----------------------------------------------------------------------
mds_z80_start		macro
	move.w	#$0000,(zreq)
	endm

;======================================================================
; quick macros to send commands to Z80
;----------------------------------------------------------------------
mds_z80_cmd0		macro cmd
	endm
mds_z80_cmd2		macro cmd,arg
	endm

mds_z80_pcm_vol_start	macro ch,vol
	mds_z80_wait_req
	st.b	z_key_on(zram)
	st.b	zp_key_on(zram,\ch)
	move.b	\vol,zp_vol(zram,\ch)
	mds_z80_start
	endm

mds_z80_pcm_start	macro ch
	mds_z80_wait_req
	st.b	z_key_on(zram)
	st.b	zp_key_on(zram,\ch)
	andi.b	#$7f,zp_vol(zram,\ch)
	mds_z80_start
	endm

mds_z80_pcm_stop	macro ch
	mds_z80_wait_req
	ori.b	#$80,zp_vol(zram,\ch)
	mds_z80_start
	endm

mds_z80_pcm_stop_i	macro ch
	mds_z80_wait_req
	ori.b	#$80,(\ch*8)+z_pcm1+zp_vol(zram)
	mds_z80_start
	endm

mds_z80_pcm_volume	macro ch,vol
	mds_z80_wait_req
	move.b	\vol,zp_vol(zram,\ch)
	mds_z80_start
	endm

mds_z80_unpack	macro
; ZX0 decompressor for 68000 (ASM68K syntax)
; Original version by Emmanuel Marty.
; Rewritten and modified by ctr 2022
	movem.l	d2/a2,-(sp)

	moveq	#-128,d0
	moveq	#-1,d2

@literal\@
	moveq	#1,d1
	bsr.s	@gamma\@
	subq.w	#1,d1
@lit_copy\@
	move.b	(a0)+,(a1)+
	dbra	d1,@lit_copy\@

	add.b	d0,d0
	bcs.s	@new_offset\@

@last_offset\@					;copy from last offset
	moveq	#1,d1
	bsr.s	@gamma\@
	subq.w	#1,d1

@match\@
	move.l	a1,a2
	adda.w	d2,a2
@match_copy\@
	move.b	(a2)+,(a1)+
	dbra	d1,@match_copy\@

	add.b	d0,d0
	bcc.s	@literal\@

@new_offset\@					;copy from new offset
	moveq	#-2,d1
	bsr.s	@gamma\@
	addq.b	#1,d1
	beq.s	@done\@				;finished if msb is 256

	move.b	d1,-(sp)			;shift left 8
	move.w	(sp)+,d2
	move.b	(a0)+,d2

	moveq	#1,d1
	asr.l	#1,d2
	bcs.s	@match\@
	pea		@match\@(pc)

@gamma_bit\@
	add.b	d0,d0
	addx.w	d1,d1
@gamma\@
	add.b	d0,d0
	bne.s	@new_byte\@
	move.b	(a0)+,d0
	addx.b	d0,d0
@new_byte\@
	bcc.s	@gamma_bit\@
	rts

@done\@
	movem.l	(sp)+,d2/a2
	endm

	endif

	if	ASMZ80

	endif

; vim: set ft=asm68k sw=4 ts=4 noet:
