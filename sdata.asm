MML	equ 1
	include "mdsseq.inc"

sdcnt:
	dc.w	SOUND_MAX
sdtop:
	rsreset
SOUND_00	rs.b	1
	dc.l	bgm00-sdtop			;bgm 01
SOUND_01	rs.b	1
	dc.l	bgm01-sdtop			;bgm 02
SOUND_02	rs.b	1
	dc.l	bgm02-sdtop			;bgm 03
SOUND_03	rs.b	1
	dc.l	bgm03-sdtop			;bgm 04
SOUND_04	rs.b	1
	dc.l	bgm04-sdtop			;bgm 05
SOUND_05	rs.b	1
	dc.l	bgm05-sdtop			;bgm 05
SOUND_MAX	rs.b	0

SD_FM_00
	dc.b	$00,$42,$26,$01		;dt/mul
	dc.b	$1f,$1f,$1f,$1f		;ks/ar
	dc.b	$11,$0f,$0f,$11		;am/dr
	dc.b	$00,$00,$00,$00		;sr
	dc.b	$60,$25,$ff,$1b		;sl/rr
	dc.b	$00,$00,$00,$00		;ssg-eg
	dc.b	$0f,$32,$0b,$00		;tl
	dc.b	$3a,$30				;fb/alg,transpose

SD_PSG_00
	dc.b	$14,$12,$50,$61,$62,$43,$01,$10,$11,$22,$13,$14,$15,$26,$17,$18
	dc.b	$29,$1a,$1b,$1c,$2d,$1e,$1f,$00
	even
SD_PEG_00
	dc.b	$00,$00,$00,$13,$00,$00,$2a,$02,$00,$80,$d6,$05,$ff,$80,$2a,$02
	dc.b	$7f,$01
	even

;====================================================================
;  test bgm 01
;====================================================================
bgm00:
@TTAB
	dc.w	@BASE-@TTAB			;offset to song base
	dc.w	$c000				;track enable mask
	dc.w	$0000,@T0-@BASE		;channel select(hi) channel flags(lo), track position
	dc.w	$0400,@T1-@BASE		;channel select(hi) channel flags(lo), track position

@BASE:
	rsreset
@FM_0		rs.b	1
	dc.w	SD_FM_00-sdtop
@PSG_0		rs.b	1
	dc.w	SD_PSG_00-sdtop
@PEG_0		rs.b	1
	dc.w	SD_PEG_00-sdtop

@T0
	dc.b	ins,0,vol,$8f,pan,$40
	dc.b	en3,l8, en3,l16,en3,gn3,l8, en3,l16,an3,l8, en3,l16,an3,slr,as3,slr,an3,gn3,en3,l8, l8, en3, l8
	dc.b	finish
	even
@T1
	dc.b	ins,0,vol,$8d,pan,$80,dtn,10, l32
	dc.b	en3,l8, en3,l16,en3,gn3,l8, en3,l16,an3,l8, en3,l16,an3,slr,as3,slr,an3,gn3,en3,l8, l8, en3, l8
	dc.b	finish
	even

;====================================================================
;  test bgm 02 - pitch eg test
;====================================================================
bgm01:
@TTAB
	dc.w	@BASE-@TTAB			;offset to song base
	dc.w	$8000				;track enable mask
	dc.w	$0000,@T0-@BASE		;channel select(hi) channel flags(lo), track position

@BASE:
	rsreset
@FM_0		rs.b	1
	dc.w	SD_FM_00-sdtop
@PSG_0		rs.b	1
	dc.w	SD_PSG_00-sdtop
@PEG_0		rs.b	1
	dc.w	SD_PEG_00-sdtop
@PEG_1		rs.b	1
	dc.w	@PEG_1D-sdtop

@PEG_1D
	dc.b	$00,$00,$70,$ff

@T0
	dc.b	tempo,60,ins,0,vol,$8f
	dc.b	pta,50, en4,l4, en5,l4, en4,l4, pta,0, peg,2, en4,l2, peg,3, en4,l1, l2
	dc.b	finish
	even

;====================================================================
;  test bgm 03 - stack test
;====================================================================
bgm02:
@TTAB
	dc.w	@BASE-@TTAB			;offset to song base
	dc.w	$8000				;track enable mask
	dc.w	$0000,@T0-@BASE		;channel select(hi) channel flags(lo), track position

@BASE:
	rsreset
@FM_0		rs.b	1
	dc.w	SD_FM_00-sdtop
@PSG_0		rs.b	1
	dc.w	SD_PSG_00-sdtop
@PEG_0		rs.b	1
	dc.w	SD_PEG_00-sdtop
@PAT_0		rs.b	1
	dc.w	@P0-@BASE

@P0
	dc.b	lp, cn4,l8, dn4
	dc.b	lpb, @P0a-*, en4, fn4, lpf, 2
@P0a
	dc.b	finish

@T0
	dc.b	tempo,60,ins,0,vol,$8f
	dc.b	pat, @PAT_0, cn5, rest, l4, finish
	even

;====================================================================
;  test bgm 05 - drum mode test
;====================================================================
bgm04:
@TTAB
	dc.w	@BASE-@TTAB			;offset to song base
	dc.w	$8000				;track enable mask
	dc.w	$0000,@T0-@BASE		;channel select(hi) channel flags(lo), track position

@BASE:
	rsreset
@FM_0		rs.b	1
	dc.w	SD_FM_00-sdtop
@PSG_0		rs.b	1
	dc.w	SD_PSG_00-sdtop
@PEG_0		rs.b	1
	dc.w	SD_PEG_00-sdtop
@D_0		rs.b	1
	dc.w	@D0-@BASE
@D_1		rs.b	1
	dc.w	@D1-@BASE

@D0
	dc.b	pan,$40, dmfinish,cn4-cn1

@D1
	dc.b	pan,$80, dmfinish,cn4-cn1

@T0
	dc.b	tempo,60,ins,0,vol,$8f,flg,8+cf_drum_mode
	dc.b	cn1+@D_0,l8,  cn1+@D_1,l4,   cn1+@D_0,l2,  cn1+@D_0,  cn1+@D_1,l4,  cn1+@D_1, finish
	even

;====================================================================
;  test bgm 06 - psg test
;====================================================================
bgm05:
@TTAB
	dc.w	@BASE-@TTAB			;offset to song base
	dc.w	$8000				;track enable mask
	dc.w	$0600,@T0-@BASE		;channel select(hi) channel flags(lo), track position

@BASE:
	rsreset

@PSG_0		rs.b	1
	dc.w	@PSG_0D-sdtop

@PSG_0D
	dc.b	$10,$21,$32,$43,$01,$98,$a9,$ba,$cb,$00

	even
@T0
	dc.b	tempo,60,ins,0,vol,$8f
	dc.b	cn4,l8,l8,dn4,l4,en4,l2,fn4,l2,l2
	dc.b	finish

	even


;====================================================================
;  test bgm 04 - music (Please don't sue me, SNK)
;====================================================================
bgm03:
@TTAB
	dc.w	@BASE-@TTAB			;offset to song base
	dc.w	$f000				;track enable mask
	dc.w	$0000,@T0-@BASE		;channel select(hi) channel flags(lo), track position
	dc.w	$0100,@T1-@BASE		;channel select(hi) channel flags(lo), track position
	dc.w	$0200,@T2-@BASE		;channel select(hi) channel flags(lo), track position
	dc.w	$0300,@T3-@BASE		;channel select(hi) channel flags(lo), track position
@BASE:
	rsreset
@FM_0		rs.b	1
	dc.w	@FM_0D-sdtop
@FM_1		rs.b	1
	dc.w	@FM_1D-sdtop
@PAT_0		rs.b	1
	dc.w	@P0-@BASE
@PAT_1		rs.b	1
	dc.w	@P1-@BASE

@FM_0D
	dc.b	$33,$74,$78,$30
	dc.b	$0f,$0f,$0f,$0f
	dc.b	$0c,$0c,$0c,$05
	dc.b	$06,$06,$05,$01
	dc.b	$07,$97,$17,$27
	dc.b	$00,$00,$00,$00
	dc.b	$15,$40,$00,$00
	dc.b	$04,$30
@FM_1D
	dc.b	$00,$06,$00,$01
	dc.b	$19,$d9,$59,$59
	dc.b	$02,$02,$06,$0c
	dc.b	$0a,$09,$0a,$0f
	dc.b	$14,$02,$16,$a6
	dc.b	$00,$00,$00,$00
	dc.b	$14,$1a,$00,$00
	dc.b	$0c,$30

@P0
	dc.b	gn6,l8d,volm,8,tie,volm,-8
	dc.b	en6,    volm,8,tie,volm,-8
	dc.b	bn6,l8, volm,8,tie,volm,-8
	dc.b	an6,    volm,2,tie,volm,2,tie,volm,2,tie,volm,2,tie,volm,2,tie,volm,2,tie,volm,2,tie,volm,-14
	dc.b	cn7,l8d,volm,8,tie,volm,-8
	dc.b	bn6,    volm,8,tie,volm,-8
	dc.b	gn6,l8, volm,8,tie,volm,-8
	dc.b	an6,    volm,3,tie,volm,3,tie,volm,3,tie,volm,3,tie,volm,3,tie,volm,3,tie,volm,3,tie
	dc.b	rest,l2,finish
@P1
	dc.b	gn5,l32
	dc.b		volm,8, tie,volm,-8,cn6,volm,8, tie,volm,-8,pan,$40
	dc.b	dn6,volm,8, tie,volm,-8,fn6,volm,8, tie,volm,-8,pan,$c0
	dc.b	gn6,volm,8, tie,volm,-8,fn6,volm,8, tie,volm,-8,pan,$80
	dc.b	dn6,volm,8, tie,volm,-8,cn6,volm,8, tie,volm,-8,pan,$c0
	dc.b	dn6,volm,8, tie,volm,-8,gn5,volm,8, tie,volm,-8,pan,$40
	dc.b	cn6,volm,8, tie,volm,-8,dn6,volm,8, tie,volm,-8,pan,$c0
	dc.b	dn6,volm,8, tie,volm,-8,gn5,volm,8, tie,volm,-8,pan,$80
	dc.b	cn6,volm,8, tie,volm,-8,dn6,volm,8, tie,volm,-8,pan,$c0
	dc.b	an5,volm,8, tie,volm,-8,dn6,volm,8, tie,volm,-8,pan,$40
	dc.b	en6,volm,8, tie,volm,-8,gn6,volm,8, tie,volm,-8,pan,$c0
	dc.b	an6,volm,8, tie,volm,-8,gn6,volm,8, tie,volm,-8,pan,$80
	dc.b	en6,volm,8, tie,volm,-8,dn6,volm,8, tie,volm,-8,pan,$c0
	dc.b	en6,volm,8, tie,volm,-8,an5,volm,8, tie,volm,-8,pan,$40
	dc.b	dn6,volm,8, tie,volm,-8,en6,volm,8, tie,volm,-8,pan,$c0
	dc.b	en6,volm,8, tie,volm,-8,an5,volm,8, tie,volm,-8,pan,$80
	dc.b	dn6,volm,8, tie,volm,-8,en6,volm,8, tie,volm,-8,pan,$c0
	dc.b	cn6,volm,8, tie,volm,-8,en6,volm,8, tie,volm,-8,pan,$40
	dc.b	gn6,volm,8, tie,volm,-8,bf6,volm,8, tie,volm,-8,pan,$c0
	dc.b	cn7,volm,8, tie,volm,-8,bf6,volm,8, tie,volm,-8,pan,$80
	dc.b	gn6,volm,8, tie,volm,-8,en6,volm,8, tie,volm,-8,pan,$c0
	dc.b	bf6,volm,8, tie,volm,-8,cn6,volm,8, tie,volm,-8,pan,$40
	dc.b	en6,volm,8, tie,volm,-8,gn6,volm,8, tie,volm,-8,pan,$c0
	dc.b	bf6,volm,8, tie,volm,-8,cn6,volm,8, tie,volm,-8,pan,$80
	dc.b	en6,volm,8, tie,volm,-8,gn6,volm,8, tie,volm,-8,pan,$c0
	dc.b	an5,volm,8, tie,volm,-8,dn6,volm,10,tie,volm,-8,pan,$40
	dc.b	en6,volm,10,tie,volm,-8,an6,volm,10,tie,volm,-8,pan,$c0
	dc.b	an5,volm,10,tie,volm,-8,dn6,volm,10,tie,volm,-8,pan,$80
	dc.b	en6,volm,10,tie,volm,-8,an6,volm,10,tie,volm,-8,pan,$c0
	dc.b	an5,volm,10,tie,volm,-8,dn6,volm,10,tie,volm,-8,pan,$40
	dc.b	en6,volm,10,tie,volm,-8,an6,volm,10,tie,volm,-8,pan,$c0
	dc.b	an5,volm,10,tie,volm,-8,dn6,volm,10,tie,volm,-8,pan,$80
	dc.b	en6,volm,10,tie,volm,-8,an6,volm,10,tie,volm,-8,pan,$c0
	dc.b	l2,finish

@T0
	dc.b	tempo,$75
	dc.b	ins,@FM_0,vol,20,pat,@PAT_0,finish
@T1
	dc.b	ins,@FM_0,vol,22,l32,dtn,35,pat,@PAT_0,finish
@T2
	dc.b	ins,@FM_1,vol,26,pat,@PAT_1,finish
@T3
	dc.b	ins,@FM_1,vol,28,l32-1,dtn,35,pat,@PAT_1,finish
; vim: set ft=asm68k sw=4 ts=4 noet: