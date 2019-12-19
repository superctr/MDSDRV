	list -
;======================================================================
; Hardware defines
;======================================================================
vdp_data				equ $c00000
vdp_control				equ $c00004
vdp_counter				equ $c00008
vdp_debug				equ $c0001c

z80_ram					equ $A00000	; start of Z80 RAM
z80_ram_end				equ $A02000	; end of non-reserved Z80 RAM
z80_bus_request			equ $A11100
z80_reset				equ $A11200

io_version				equ $A10001
io_port_1_data			equ $A10002
io_port_1_control		equ $A10008
io_port_2_control		equ $A1000A
io_expansion_control	equ $A1000C
io_tmss					equ $A14000

; ym2612 (ym3438) wait requirement:
;  after address write:   17 cycles
;  after data write:      83 cycles (reg 21-9e)
;                         47 cycles (reg a0-b6)
sound_fm_a0				equ $A04000
sound_fm_d0				equ $A04001
sound_fm_a1				equ $A04002
sound_fm_d1				equ $A04003
sound_fm_status			equ $A04000

sound_psg				equ $C00011

;======================================================================
; joypad control
;======================================================================
joy1_push				equ $fffffff0
joy1_press				equ $fffffff2
joy2_push				equ $fffffff4
joy2_press				equ $fffffff6

joy_up					equ %0000000000000001
joy_down				equ %0000000000000010
joy_left				equ %0000000000000100
joy_right				equ %0000000000001000
joy_b					equ %0000000000010000
joy_c					equ %0000000000100000
joy_a					equ %0000000001000000
joy_start				equ %0000000010000000
joy_z					equ %0000000100000000
joy_y					equ %0000001000000000
joy_x					equ %0000010000000000
joy_mode				equ %0000100000000000
joy_is_6btn				equ %0001000000000000

joy_bit_up				equ 0
joy_bit_down			equ 1
joy_bit_left			equ 2
joy_bit_right			equ 3
joy_bit_b				equ 4
joy_bit_c				equ 5
joy_bit_a				equ 6
joy_bit_start			equ 7
joy_bit_z				equ 8
joy_bit_y				equ 9
joy_bit_x				equ 10
joy_bit_mode			equ 11

last_vbl				equ $fffffff8

;======================================================================
; VDP control
;======================================================================
vram_read_flag			equ $00000000
vram_write_flag			equ $40000000
cram_read_flag			equ $00000020
cram_write_flag			equ $c0000000
vsram_read_flag			equ $00000010
vsram_write_flag		equ $40000010

vram_dma_flag			equ $40000080
cram_dma_flag			equ $c0000080
vsram_dma_flag			equ $40000090

vram_plane_a			equ $a000
vram_plane_b			equ $c000

;======================================================================
; VDP access macro
;======================================================================

vdp_quick macro
	move.l #((\1<<16)&0x3fff0000)|((\1>>14)&0x3)|\2,vdp_control
	endm

scroll_a_quick macro
	vdp_quick	(vram_plane_a+(\1<<6)+(\2<<1)),vram_write_flag
	endm

vdp_address	macro
	swap	\1
	clr.w	\1
	rol.l	#2,\1
	swap	\1
	lsr.w	#2,\1
	swap	\1
	endm

setup_vram_read		macro
	vdp_address \1
	ori.w #vram_read_flag,\1
	move.l \1, vdp_control
	endm

setup_vram_write	macro
	vdp_address \1
	ori.l #vram_write_flag,\1
	move.l \1, vdp_control
	endm

setup_cram_read		macro
	swap \1
	move.w #cram_read_flag,\1
	move.l \1, vdp_control
	endm

setup_cram_write	macro
	swap \1
	clr.w \1
	ori.l #cram_write_flag,\1
	move.l \1, vdp_control
	endm

setup_vsram_read	macro
	swap \1
	move.w #vsram_read_flag,\1
	move.l \1, vdp_control
	endm

setup_vsram_write	macro
	swap \1
	clr.w \1
	ori.l #vsram_write_flag,\1
	move.l \1, vdp_control
	endm

;======================================================================
; Print command list
;======================================================================

; vdp_printa <row>, <col>, <ptr to string>, <flags>, <count>
p_end		macro
	dc.b	$00
	endm
p_pos		macro
	dc.b	$01
	dc.w	vram_plane_a+((\1)<<7)+((\2)<<1)
	endm
p_shift		macro
	dc.b	$02
	dc.w	((\1)<<7)+((\2)<<1)
	endm
p_palette	macro
	dc.b	$03
	dc.b	((\1)<<5)
	endm
p_text_n	macro
	dc.b	$04
	dc.b	\1
	dc.b	\2,0
	endm
p_text		macro
	dc.b	$05
	dc.b	\1,0
	endm
p_offset	macro
	dc.l	$06000000|(\1&$ffffff)
	endm
p_shift_offset	macro
	dc.l	$07000000|(\1&$ffffff)
	endm
p_hex_b		macro
	dc.l	$08000000|(\1&$ffffff)
	endm
p_hex_w		macro
	dc.l	$09000000|(\1&$ffffff)
	endm
p_hex_l		macro
	dc.l	$0a000000|(\1&$ffffff)
	endm

;======================================================================
; Z80 access macro
;======================================================================
; stop Z80 and wait for bus grant
stop_z80	macro
	move.w	#$100, (z80_bus_request)
stop_z80\@:
	btst.b	#0, (z80_bus_request)
	bne.s	stop_z80\@
	endm

; stop Z80 without waiting for bus grant.
block_z80	macro
	move.w	#$100, (z80_bus_request)
	endm

; reset Z80 and YM2612. Note that d0 is trashed
reset_z80	macro
	move.w	#$000,z80_reset
	move.w	#20,d0
@reset_wait
	dbra	d0,@reset_wait
	move.w	#$100,z80_reset
	endm

; start Z80
start_z80	macro
	move.w	#0,(z80_bus_request).l
	endm

	list +
; vim: set ft=asm68k sw=4 ts=4 noet:
