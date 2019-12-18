;======================================================================
; sound driver work memory
;======================================================================
RCOUNT			equ		4			; request count
TCOUNT			equ		16			; track count
TSTACK_COUNT	equ		10

;======================================================================
; register define
;----------------------------------------------------------------------
;always
work	equr	a0							; driver work
twork	equr	a1							; track work
tbase	equr	a2							; track base address
tpos	equr	a3							; track position
tmpa0	equr	a4							; temp address reg
tmpa1	equr	a5
tmpa2	equr	a6

; during request/track processing
rnum	equr	d7							; request #
flag	equr	d7
tnum	equr	d6							; track #
tmask	equr	d5							; track mask

; during voice processing
chnid	equr	d4							; channel id

;======================================================================
; note flags during voice processing (btst #nf+nf_enabled)
;======================================================================
nf		equ		8
cf		equ		0

;======================================================================
; note flags
;======================================================================
	rsreset
nf_silence		rs.b	1			; trigger silence
nf_key_on		rs.b	1			; keyon trigger
nf_key_off		rs.b	1			; keyoff trigger
nf_slur			rs.b	1			; slur next note
nf_ins			rs.b	1			; load new instrument
nf_vol			rs.b	1			; load new volume
nf_pan_lfo		rs.b	1			; load new pan/lfo
nf_enabled		rs.b	1			; envelopes enabled(must be bit7!)

;======================================================================
; channel flags
;======================================================================
	rsreset
cf_fm3_enable	rs.b	1			; fm3 mode enabled
cf_drum_mode	rs.b	1			; drum mode enabled
cf_drum_mode_on	rs.b	1			; inside drum mode routine
cf_psg3_control	rs.b	1			; write fnum to psg3
cf_pcm_control	rs.b	1			; pcm instrument loaded
cf_inhibit		rs.b	1			; free

;======================================================================
; flag bitmasks
;======================================================================
nm_init			equ		((1<<nf_ins)|(1<<nf_pan_lfo)|(1<<nf_enabled))
nm_restore		equ		((1<<nf_ins)|(1<<nf_vol)|(1<<nf_pan_lfo))
nm_stop			equ		((1<<nf_silence)|(1<<nf_key_off))

;======================================================================
; channel type
;======================================================================
	rsreset
ct_fm			rs.b	6			; fm voice
ct_psg			rs.b	3			; psg melody
ct_psgn			rs.b	1			; psg noise

;======================================================================
; track data
;======================================================================
	rsreset
;----------------------------------------------------------------------
; sequence control
;----------------------------------------------------------------------
t_note_flag		rs.b	1			; nf_*
t_channel_flag	rs.b	1			; cf_*
t_base_addr		rs.l	1			; sequence base addr
t_fm3_mask		equ	t_base_addr		; ch3 op mask (using top 8 bits)
t_position		rs.w	1			; sequence position
t_stack_pos		rs.b	1
t_counter		rs.b	1			; ticks until next update
t_rest_time		rs.b	1			; length of last rest
t_note_time		rs.b	1			; length of last note or tie

;----------------------------------------------------------------------
; channel variables
;----------------------------------------------------------------------
t_channel_id	rs.b	1			; channel type & id lookup
t_request_id	rs.b	1			; associated request slot
t_ins			rs.b	1			; instrument
t_ins_trs		rs.b	1			; instrument transpose
t_note			rs.b	1			; key code
t_dtn			rs.b	1			; key fraction
t_pitch			rs.w	1			; portamento modulator
t_trs			rs.b	1			; transpose
t_pta			rs.b	1			; portamento speed
t_vol			rs.b	1			; volume
t_lfo_delay		rs.b	1

;----------------------------------------------------------------------
; pitch envelope
;----------------------------------------------------------------------
t_peg_addr		rs.w	1			; envelope base address (0=off)
t_peg_mod		rs.w	1			; envelope modulator
t_peg_delay		rs.b	1			; delay counter
t_peg_pos		rs.b	1			; envelope position (addr+pos/4)

;----------------------------------------------------------------------
; stack
;----------------------------------------------------------------------
t_stack			rs.w	TSTACK_COUNT

;----------------------------------------------------------------------
; type exclusive
;----------------------------------------------------------------------
t_type			rs.b	6
TSIZE			rs.b	0

;----------------------------------------------------------------------
; FM type
;----------------------------------------------------------------------
	rsset		t_type
t_fm_tl			rs.b	4			; operator TL values
t_fm_alg		rs.b	1			; algorithm nr.
t_fm_pan_lfo	rs.b	1			; panning/lfo
T_FMSIZE		rs.b	0

;----------------------------------------------------------------------
; PSG type
;----------------------------------------------------------------------
	rsset		t_type
t_psg_eg_addr	rs.l	1			; envelope base address
t_psg_eg_pos	rs.b	1			; envelope position
t_psg_eg_delay	rs.b	1			; envelope value & delay
T_PSGSIZE		rs.b	0

;======================================================================
; driver work area
;======================================================================
	rsreset
w_sdtop			rs.l	1			;position of request table
w_request		rs.w	RCOUNT		;sound request
w_tempo			rs.w	RCOUNT
w_counter		rs.w	RCOUNT		;tempo counter
w_volume		rs.w	RCOUNT
w_tmask			rs.w	RCOUNT		;track mask
w_chmask		rs.w	RCOUNT		;channel usage mask

w_fm3_mask		rs.b	1
w_fm3_alg		rs.b	1
w_fm3_tl		rs.b	4

w_gtempo		rs.w	1

w_track			rs.b	tsize*TCOUNT

WSIZE			rs.b	0

	inform		0,"each track work area is %d bytes",TSIZE
	if TSIZE&1
		inform	3,"Track data length is misaligned"
	endif
	if T_FMSIZE>TSIZE
		inform	3,"FM type is too big (%d > %d)",T_FMSIZE,TSIZE
	endif
	if T_PSGSIZE>TSIZE
		inform	3,"PSG type is too big (%d > %d)",T_PSGSIZE,TSIZE
	endif
	inform		0,"sound driver work area is %d bytes",WSIZE

break	macro
		clr.w	$ff0000
		endm

; vim: set ft=asm68k sw=4 ts=4 noet:
