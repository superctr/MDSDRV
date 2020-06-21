;======================================================================
; Z80 public variables
;======================================================================

z_busy			equ		$0980

z_fm_reg27		equ		$0981		;fm timer reset command
z_fm_reg_update	equ		$0008		;set to 0 to write reg 27

z_pcm_flag		equ		$0982		;pcm status flags
z_pcm_key_on	equ		$0983		;pcm keyon trigger

z_pcm1_key_on	equ		$0984		;pcm key on flags
z_pcm2_key_on	equ		$0985		;pcm key on flags

z_pcm1_key_off	equ		$0986		;pcm key off flags
z_pcm2_key_off	equ		$0987		;pcm key off flags

z_pcm1_source	equ		$0988		;pcm source select flag
z_pcm2_source	equ		$0989		;pcm source select flag

z_pcm1_vol		equ		$098a		;volume table hi (0=key off)
z_pcm2_vol		equ		$098b		;volume table hi (0=key off)

z_pcm1_src		equ		$098c		;sample number
z_pcm2_src		equ		$098d		;sample number

z_stab			equ		$0a00
z_vtab			equ		$1000

z_flag_pcm1		equ		0
z_flag_pcm2		equ		1

z_flag_key_on	equ		7


;======================================================================
; Z80/68K communication macros
;======================================================================
	if	ASM68K

;======================================================================
; request Z80 bus for sending a request
;----------------------------------------------------------------------
mds_z80_wait_req	macro
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

mds_z80_pcm_start_f	macro ch,vol,src
@stop_z80\@
	move.w	#$0100,(zreq)
@wait_z80\@
	btst.b	#0,(zreq)
	bne.s	@wait_z80\@
; TODO: Maybe the variables should be rearranged to avoid having to
; do this.
	lea		z_pcm1_key_on(zram),zram
	st.b	0(zram,\ch)
	st.b	z_pcm1_source-z_pcm1_key_on(zram,\ch)
	move.b	\vol,z_pcm1_vol-z_pcm1_key_on(zram,\ch)
	move.b	\src,z_pcm1_src-z_pcm1_key_on(zram,\ch)
	lea		-z_pcm1_key_on(zram),zram
	st.b	z_pcm_key_on(zram)
	move.w	#$0000,(zreq)
	endm

mds_z80_pcm_start	macro ch
@stop_z80\@
	move.w	#$0100,(zreq)
@wait_z80\@
	btst.b	#0,(zreq)
	bne.s	@wait_z80\@
	lea		z_pcm1_key_on(zram),zram
	st.b	0(zram,\ch)
	lea		-z_pcm1_key_on(zram),zram
	st.b	z_pcm_key_on(zram)
	move.w	#$0000,(zreq)
	endm

mds_z80_pcm_stop	macro ch
@stop_z80\@
	move.w	#$0100,(zreq)
@wait_z80\@
	btst.b	#0,(zreq)
	bne.s	@wait_z80\@
	lea		z_pcm1_key_off(zram),zram
	st.b	0(zram,\ch)
	lea		-z_pcm1_key_off(zram),zram
	move.w	#$0000,(zreq)
	endm

mds_z80_pcm_stop_i	macro ch
@stop_z80\@
	move.w	#$0100,(zreq)
@wait_z80\@
	btst.b	#0,(zreq)
	bne.s	@wait_z80\@
	st.b	\ch+z_pcm1_key_off(zram)
	move.w	#$0000,(zreq)
	endm

mds_z80_pcm_volume	macro ch,vol
@stop_z80\@
	move.w	#$0100,(zreq)
@wait_z80\@
	btst.b	#0,(zreq)
	bne.s	@wait_z80\@
	lea		z_pcm1_vol(zram),zram
	move.b	\vol,0(zram,\ch)
	lea		-z_pcm1_vol(zram),zram
	move.w	#$0000,(zreq)
	endm

mds_z80_unpack	macro
;======================================================================
; Sik SLZ routine follows: (Slightly modified register usage)
;======================================================================
; DecompressSlz
; Decompresses SLZ data into memory
;----------------------------------------------------------------------
; input a0.l .... Pointer to compressed data
; input a1.l .... Where to store decompressed data
; output a0.l ... Right after input buffer
; output a1.l ... Right after output buffer

	movem.l	d5-d7,-(sp)
	move.w  d3,-(sp)			; Save registers
	move.w  d4,-(sp)

	move.b  (a0)+,d7			; Get uncompressed size
	lsl.w   #8,d7
	move.b  (a0)+,d7

	moveq   #1,d6				; Cause code to fetch new token data
								; as soon as it starts
@MainLoop\@
	tst.w   d7					; Did we read all the data?
	beq     @End\@				; If so, we're done with it!

	subq.w  #1,d6				; Check if we need more tokens
	bne.s   @HasTokens\@
	move.b  (a0)+,d5
	moveq   #8,d6
@HasTokens\@

	add.b   d5,d5				; Get next token type
	bcc.s   @Uncompressed\@		; 0 = uncompressed, 1 = compressed

	move.b  (a0)+,d3			; Compressed? Read string info
	lsl.w   #8,d3				; d3 = distance
	move.b  (a0)+,d3			; d4 = length
	move.b  d3,d4
	lsr.w   #4,d3
	and.w   #$0F,d4

	subq.w  #3,d7				; Length is offset by 3
	sub.w   d4,d7				; Now that we know the string length,
								; discount it from the amount of data
								; to be read

	addq.w  #3,d3				; Distance is offset by 3
	neg.w   d3					; Make distance go backwards

	add.w   d4,d4				; Copy bytes using Duff's device
	add.w   d4,d4				; MUCH faster than a loop, due to lack
	eor.w   #$0F<<2,d4			; of iteration overhead
	jmp     @Duff\@(pc,d4.w)
@Duff\@
	rept    $10+2
	move.b  (a1,d3.w),(a1)+
	endr

	bra     @MainLoop\@			; Keep processing data

@Uncompressed\@
	move.b  (a0)+,(a1)+			; Uncompressed? Read as is
	subq.w  #1,d7				; It's always one byte long
	bra     @MainLoop\@			; Keep processing data

@End\@
	move.w  (sp)+,d4			; Restore registers
	move.w  (sp)+,d3

	movem.l	(sp)+,d5-d7
	endm

	endif

	if	ASMZ80

;======================================================================
; DAC macros
;----------------------------------------------------------------------
	; Assume it's already time to update DAC samples
	macro	write_dac								;87
		ld		(de),a				;set busy flag
		ld		(hl),$27			;timer reset
		ld		a,iyl
		ld		($4001),a
		ld		a,(bc)
		ld		(hl),$2a			;sample load
		xor		e					;add $80
		ld		($4001),a
		inc		c
		xor		a
		ld		(de),a
	endm

	; Assumes we're already in exx
	macro	faster_poll								;87+18=105
		ld		a,(hl)
		rrca
		jr		nc,.no_update
		write_dac
.no_update
	endm

	macro	fast_poll								;105+8=113
		exx
		faster_poll
		exx
	endm

;======================================================================
; PCM macros
;----------------------------------------------------------------------
	macro set_bank				; ???
		cp		iyh
		jp		z,.no_bankswitch
		ld		iyh,a
		ld		hl,$6000
		dup		7
			ld		(hl),a
			rra
		edup
		ld		(hl),a
		ld		(hl),h
.no_bankswitch
	endm

	; b  = volume table offset
	; hl = sample position
	; de = buffer position
	macro	read_one			; 27
		ld		c,(hl)
		inc		hl
		ld		a,(bc)
		ld		(de),a
	endm
	macro	read_loop			; 120 = 27*4 + 4*3
		read_one
		inc		e
		read_one
		inc		e
		read_one
		inc		e
		read_one
	endm

	; hl = mixed output buffer
	; de = sample position
	; b  = volume table offset
	macro	read_mix_one		; 62
		ld		a,(de)
		ld		c,a
		inc		de
		ld		a,(bc)
		add		a,(hl)
		jp		po,.no_ovf
		ld		a,0
		adc		a,$7f
.no_ovf
		ld		(hl),a
	endm

	macro	read_mix_loop		; 128
		read_mix_one
		inc		l
		read_mix_one
	endm
	endif

; vim: set ft=asm68k sw=4 ts=4 noet: