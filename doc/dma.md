# DMA protection

Using the Z80 and DMA at the same time may cause issues on a real
Mega Drive system. Therefore, some precautions need to be taken
to ensure that your program will run properly when using the DMA.

## Background

A fairly well known issue in Mega Drive programming is the conflict
that occurs when the Z80 tries to access the 68000 bus just when the
VDP is starting a DMA transfer.

Normally, if a DMA is ongoing and the Z80 tries to access the 68000
bus, the bus arbiter will simply wait for the VDP to finish the DMA
before letting the Z80 know it is ready to read (or write) memory
from the bus.

However, what may happen when a DMA is just starting is that a bus
contention conflict occurs between the VDP and the bus arbiter, causing
glitches to appear on the address lines.

In the best case, the the data transferred by the DMA to VRAM will be
corrupted. In the worst case, the glitch occurs while the RAM (or
a flashcart) is refreshing its internal memory, the memory enters a
metastable state and you may get corruption on 68K RAM or flashcart
ROM.

The usual method to solve this is to halt the Z80 before starting a
DMA. However, since DMAs might last for quite a while, stopping the
Z80 for all this time will cause the PCM sample quality to be
degraded. The fix is to have the Z80 buffer samples during active
scan and make sure it only reads from the buffer while a DMA is
ongoing. MDSDRV contains a buffer, however the Z80 cannot by itself
determine if a DMA is (or will be) ongoing.

## Using DMA protection

After initializing MDSDRV using `mds_init`, the DMA protection is
disabled. This allows for low latency PCM and allows the Z80 to run
without needing an acknowledgement from the 68k. The 68k will simply
send a bus request whenever it does a DMA, as with other sound drivers.

### Enabling DMA protection

To enable the DMA protection, call `mds_command` with command number
$11 (`set_pcmmode`) in d0 and set d2 to the requested buffer size.
The byte at d1 must contain a valid PCM mixing mode or can be set to 0
to use the previous PCM mixing mode.

Example:

		lea		mds_work,a0
		moveq	#$11,d0			; set_pcmmode
		moveq	#$2,d1			; PCM mode 2
		move.w	#100,d2			; DMA protection on, buffer 100 bytes
		jsr		mds_command
		;		( or mdsdrv+$0c )

The requested buffer size must be larger than 40 bytes and smaller than
220 bytes.

A buffer of 100 bytes works well for the 2 channel mixing mode. If you
notice that the sound becomes crackly, increase the buffer size by
changing the value of `d2`. Notice that a larger buffer will introduce
latency.

The minimum buffer size threshold is 40 bytes. If the buffer falls
below this limit in the vertical interrupt routine, it will temporarily
stop playing back samples, which will affect the sound quality.

### Acknowledging interrupts

When DMA protection is enabled, **in each vertical interrupt** the 68k
must acknowledge to the Z80 that it has finished any DMAs and that the
Z80 may start filling the buffer from ROM again. This is done by writing
a non-zero value to `z_vbl_ack` (at `$A00E06`). Failure to acknowledge
while DMA protection is enabled will result in stopped PCM playback.

Example:

	; Run this code in the vertical interrupts after finishing all DMAs
		move.w	#$100,$a11100			; Send a bus request
	@wait
		btst	#0,$a11100
		bne.s	@wait
		st.b	$a00e06					; Set z_vbl_ack to $ff
		move.w	#$000,$a11100			; Clear bus request

### Disabling DMA protection

While DMA protection is active, if the 68k needs to disable interrupts for
a long time, therefore becoming unable to acknowledge the Z80, it is
possible to disable the DMA protection by issuing `set_pcmmode` with the
bytes in `d1` and `d2` being set to zero.

Example:

		lea		mds_work,a0
		moveq	#$11,d0			; set_pcmmode
		moveq	#0,d1			; Keep mixing mode
		moveq	#0,d2			; Disable DMA protection
		jsr		mds_command
		;		( or mdsdrv+$0c )

Note that when the DMA protection is re-enabled without resetting the
PCM mode, it will take effect after a short delay (just before the
next sample batch).

## The Z80 PCM driver

This section contains some information on the Z80 driver that can be
useful when working with the DMA protection.

The Z80 driver reads samples in "batches". Depending on the mixing mode,
the batch is either 32 or 30 bytes long. The size of the batch has been
selected to allow mixing, volume control and bank switching of multiple
channels without causing playback interruptions (which reduces sound
quality) or high latency.

The PCM buffer is organized as a ring buffer with a maximum width of
256 bytes.

### `z_load` (`$A00E01`)

This variable contains the current size of the sample buffer. It is
updated by the Z80 after a batch has been written to the buffer, or
when a sample has been read from the buffer while "idle" (that is, a
batch is not currently being written to the buffer).

If this is often being set to 40 (hex `$28`) in the vertical interrupt
routine, that is a sign that the buffer needs to be increased
(or simply that the Z80 was not able to buffer enough samples in time).

### `z_min_buffer` (`$A00E04`)

This variable specifies the minimum size of the buffer. If the current
size of the buffer is less than the value of this variable, the Z80
will start reading a new batch.

When DMA protection has been enabled, writing to this variable has the
same effect as adjusting the buffer size with `set_pcmmode`.

### `z_vbl_ack` (`$A00E06`)

The vertical interrupt acknowledge flag. At the beginning of the
vertical interrupt, the Z80 sets this to 0. It will then wait for the
68k to set this to a non-zero value, while depleting the sample buffer.

### Z80 vertical interrupt entry point (`$A00038`)

The DMA protection can be enabled or disabled by replacing the first
instruction of the interrupt routine.
For more information, read the source code of `mdssub.z80`.

