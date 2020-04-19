;======================================================================
; Z80 public variables
;======================================================================

z_arg		equ		$0009
z_argh		equ		$0009	; low byte of argument
z_argl		equ		$000b	; high byte of argument
z_argp		equ		$000d	; 'c3' to convert pitch to f-num
							; 'c9' to use argument as is
z_mode		equ		$0012	; idle mode (don't set it directly)
z_req		equ		$0017	; command request
z_busy		equ		$007f	; busy flag (non-zero = z80 is writing to
							; the FM chip
z_stab		equ		$0a00	; sample table
z_vtab		equ		$1000	; volume table

zc_pcm1_start	equ		$17*3
zc_pcm1_stop	equ		$19*3
zc_pcm1_vol		equ		$1b*3

zc_pcm2_start	equ		$18*3
zc_pcm2_stop	equ		$1a*3
zc_pcm2_vol		equ		$1c*3

;======================================================================
; Z80/68K communication macros
;======================================================================
	if	ASM68K

;======================================================================
; request Z80 bus for sending a request
;----------------------------------------------------------------------
mds_z80_wait_req	macro
@stop_z80\@
	move.w	#$0100,(zreq)
@wait_z80\@
	btst.b	#0,(zreq)
	bne.s	@wait_z80\@
	tst.b	z_busy(zram)
	bmi.s	@still_busy\@
	cmpi.b	#$70-1,z_req(zram)
	bcc.s	@done\@
@still_busy\@
	move.w	#$0000,(zreq)		; z80 is already processing a request
	movem.l	d0-d4,-(sp)								; 8+40 : 48
	movem.l	(sp)+,d0-d4								;      : 96	
	bra.s	@stop_z80\@								;   10 : 106
@done\@
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
	mds_z80_wait_req
	move.b	\cmd,z_req(zram)
	mds_z80_start
	endm
mds_z80_cmd2		macro cmd,arg
	mds_z80_wait_req
	move.b	\cmd,z_req(zram)
	movep.w	\arg,z_arg(zram)
	mds_z80_start
	endm
	endif

; vim: set ft=asm68k sw=4 ts=4 noet: