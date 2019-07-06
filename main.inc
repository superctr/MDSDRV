
	rsset	$ffff0000

ram_begin
r_break			rs.w	1			; quick and dirty debug
r_intcnt		rs.w	1			; Vint counter
r_pcount0		rs.w	1			; VDP counter at beginning of sound driver
r_pcount1		rs.w	1			; VDP counter at end of sound driver
				rs.w	4

r_driver		rs.w	WSIZE		; sound driver

r_request		rs.w	1
