;======================================================================
; sound driver work memory
;======================================================================
MDSDRV_VER		equ		$0003		; sound driver version

RCOUNT			equ		4			; request count
TCOUNT			equ		16			; track count
TSTACK_COUNT	equ		8			; stack size (in words)

;======================================================================
; register define
;----------------------------------------------------------------------
;always
work			equr	a0			; driver work
twork			equr	a1			; track work
a_sdtop			equr	a2			; sdtop
tmpa0			equr	a4			; temp address reg
tmpa1			equr	a5			; temp address / track base address
zreq			equr	a3			; track position
zram			equr	a6

; during request/track processing
rnum			equr	d7			; request #  (low word)
flag			equr	d7			; flags      (high word)
tnum			equr	d6			; track #

; during voice processing
chnid			equr	d5			; channel id

;======================================================================
; request flags
;======================================================================
rf_active		equ		15			; request is active
rf_stop			equ 	14			; stop previous track

;======================================================================
; note flags during voice processing (btst #nf+nf_enabled)
;======================================================================
nf				equ		8+16
cf				equ		0+16

;======================================================================
; note flags
;======================================================================
	rsreset
nf_ins			rs.b	1			; load new instrument
nf_sustain		equ		nf_ins		; PSG envelope sustain
nf_pcm_header	equ		nf_ins		; update PCM header
nf_vol			rs.b	1			; load new volume
nf_fm3			rs.b	1			; check fm3 flags
nf_pcm_pitch	equ		nf_fm3		; update PCM pitch
nf_pan_lfo		rs.b	1			; load new pan/lfo
nf_nmode		equ		nf_pan_lfo	; restore PSG noise mode
nf_slur			rs.b	1			; slur next note
nf_key_off		rs.b	1			; keyoff trigger
nf_key_on		rs.b	1			; keyon trigger
nf_enabled		rs.b	1			; envelopes enabled(must be bit7!)

;======================================================================
; channel flags
;======================================================================
	rsreset
cf_drum_mode	rs.b	1			; drum mode enabled
				rs.b	1			; (unused bit)
cf_pcm_control	rs.b	1			; pcm instrument loaded
cf_stop			rs.b	1			; stop channel
cf_suspend		rs.b	1			; track is paused and can be
									;   restarted
cf_background	rs.b	1			; track is deprioritized and
									;   plays in the background
				rs.b	1			; (unused bit)
cf_key_on		rs.b	1			; channel is currently playing

;======================================================================
; flag bitmasks
;======================================================================
nm_init			equ		((1<<nf_ins)|(1<<nf_pan_lfo)|(1<<nf_enabled))
nm_restore		equ		((1<<nf_key_off)|(1<<nf_ins)|(1<<nf_vol)|(1<<nf_pan_lfo)|(1<<nf_fm3))
cm_pause		equ		((1<<cf_suspend)|(1<<cf_stop))

;======================================================================
; channel type
;======================================================================
	rsreset
ct_fm			rs.b	6			; fm voice
ct_psg			rs.b	3			; psg melody
ct_psgn			rs.b	1			; psg noise

;======================================================================
; pcm enable flags
;======================================================================
pe_pcm1			equ		7
pe_pcm2			equ		6
pe_pcm3			equ		5
pe_fade_stop	equ		4

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
t_op_mask		equ	t_base_addr		; operator mask (using top 8 bits)
									;     bit 7   : ch3 enable
									;     bit 6-3 : keyon/off op mask
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
t_ins_trs		rs.b	1			; instrument transpose (n*2)
t_note			rs.b	1			; key code
t_dtn			rs.b	1			; key fraction
t_pitch			rs.w	1			; portamento modulator
t_last_pitch	rs.w	1			; calculated pitch
t_trs			rs.b	1			; transpose
t_pta			rs.b	1			; portamento speed
t_vol			rs.b	1			; volume

;----------------------------------------------------------------------
; macro table
;----------------------------------------------------------------------
t_mtab_repeat	rs.b	1			; macro table repeat count
t_mtab_addr		rs.w	1			; macro table base address (0=off)
t_mtab_delay	rs.b	1			; macro table delay counter
t_mtab_pos		rs.b	1			; macro table position

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
t_fm_pan_lfo	rs.b	1			; panning/lfo (must be pos. 0)
t_fm_alg		rs.b	1			; algorithm nr.
t_fm_tl			rs.b	4			; operator TL values
T_FMSIZE		rs.b	0

;----------------------------------------------------------------------
; PSG type
;----------------------------------------------------------------------
	rsset		t_type
t_psg_eg_addr	rs.w	1			; envelope base address
t_psg_eg_pos	rs.b	1			; envelope position
t_psg_eg_delay	rs.b	1			; envelope value & delay
t_psg_nreset	rs.b	1			; noise reset flag
t_psg_nmode		rs.b	1			; noise mode
T_PSGSIZE		rs.b	0

;----------------------------------------------------------------------
; PCM type
;----------------------------------------------------------------------
	rsset		t_type
t_pcm_pan		rs.b	1			; PCM panning (bit 6-7)...
t_pcm_pitch		rs.b	1			; PCM pitch
t_pcm_header	rs.w	1			; pointer to PCM header
t_pcm_length	rs.w	1			; calculated PCM sample length
T_PCMSIZE		rs.b	0

;======================================================================
; driver work area
;======================================================================
	rsreset
w_sdtop			rs.l	1			;position of request table
w_request		rs.w	RCOUNT		;sound request
w_tempo			rs.w	RCOUNT
w_counter		rs.w	RCOUNT		;tempo counter
w_seq_step		rs.w	RCOUNT		;sequence step counter (modulo)
w_volume		rs.w	RCOUNT		;global volume
w_tmask			rs.w	RCOUNT		;track mask
w_chmask		rs.w	RCOUNT		;channel usage mask

w_priority		rs.b	1			; priority update flag
w_fade_rate		rs.b	1			; BGM fade rate

w_fade_target	rs.b	1			; BGM fade target
w_comm			rs.b	1			; communication byte

w_pcm_bank		rs.b	1			; pcm bank offset
w_pcm_mode		rs.b	1			; bit 0-3: current pcm mode
									; bit 4  : stop track after fade
									; bit 5-7: pcm enable flags

w_fm3_mask		rs.b	1			;fm3 keyon operator mask
w_fm3_alg		rs.b	1
w_fm3_tl		rs.b	4

w_gtempo		rs.w	1			;global tempo

w_track			rs.b	TSIZE*TCOUNT

WSIZE			rs.b	0

;======================================================================
; RAM footprint statistic
;======================================================================
	inform		0,"Track work area size is %d bytes",TSIZE
	if TSIZE&1
		inform	3,"Track work area size is misaligned"
	endif
	if T_FMSIZE>TSIZE
		inform	3,"FM specific area is too big (%d > %d)",T_FMSIZE,TSIZE
	endif
	if T_PSGSIZE>TSIZE
		inform	3,"PSG specific area is too big (%d > %d)",T_PSGSIZE,TSIZE
	endif
	if T_PCMSIZE>TSIZE
		inform	3,"PCM specific area is too big (%d > %d)",T_PCMSIZE,TSIZE
	endif
	inform		0,"Sound driver work area size is %d bytes",WSIZE

break	macro
		clr.w	$ff0000
		endm

mds_di	macro								; disable interrupts
		move	sr,-(sp)
		move	#$2700,sr
		endm

mds_ei	macro								; enable interrupts
		move	(sp)+,sr
		endm

; vim: set ft=asm68k sw=4 ts=4 noet:
