MML	equ 1
	include "mdsseq.inc"

sdcnt:
	dc.w	SOUND_MAX
sdtop:
	rsreset
SOUND_00	rs.b	1
	dc.l	bgm00-sdtop			;bgm 00
SOUND_01	rs.b	1
	dc.l	bgm00-sdtop			;bgm 00
SOUND_02	rs.b	1
	dc.l	bgm00-sdtop			;bgm 00
SOUND_MAX	rs.b	0

bgm00:
@TTAB
	dc.w	@BASE-@TTAB			;offset to song base
	dc.w	$8000				;track mask
	dc.w	$0000,@T0-@BASE		;trk0 - ch0

@BASE:
	rsreset
@FM_0		rs.b	1
	dc.w	@FM_0D-@BASE
@PSG_0		rs.b	1
	dc.w	@PSG_0D-@BASE
@PEG_0		rs.b	1
	dc.w	@PEG_0D-@BASE

@FM_0D
	dc.b	$00,$42,$26,$01		;dt/mul
	dc.b	$1f,$1f,$1f,$1f		;ks/ar
	dc.b	$11,$0f,$0f,$11		;am/dr
	dc.b	$00,$00,$00,$00		;sr
	dc.b	$60,$25,$ff,$1b		;sl/rr
	dc.b	$00,$00,$00,$00		;ssg-eg
	dc.b	$0f,$32,$0b,$00		;tl
	dc.b	$3a					;fb/alg

@PSG_0D
	dc.b	$14,$12,$50,$61,$62,$43,$01,$10,$11,$22,$13,$14,$15,$26,$17,$18
	dc.b	$29,$1a,$1b,$1c,$2d,$1e,$1f,$00

	even
@PEG_0D;	0	-	-	-	1	-	-	-	2	-	-	-	3	-	-	-
	dc.b	$00,$00,$00,$13,$00,$00,$2a,$02,$00,$80,$d6,$05,$ff,$80,$2a,$02
	dc.b	$7f,$01

@T0
	dc.b	ins,0,vol,$8f
	dc.b	en3,l8, en3,l16,en3,gn3,l8, en3,l16,an3,l8, en3,l16,an3,as3,an3,gn3,en3,l8, l8, en3, l8
	dc.b	finish
; vim: set ft=asm68k sw=4 ts=4 noet: