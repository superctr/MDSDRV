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

That leads to the question, exactly how do you tell the Z80 to stop
reading from ROM? We will try to solve this by using several methods.

## The Z80 PCM buffer

The Z80 driver reads samples in "chunks". Depending on the mixing mode,
the chunk is either 32 or 30 bytes long. The size of the chunk has been
selected to allow mixing, volume control and bank switching of multiple
channels without causing playback interruptions (which reduces sound
quality) or high latency.

The PCM buffer is organized as a ring buffer with a maximum width of
256 bytes.

### `z_load`

This variable contains the current size of the sample buffer. It is
updated by the Z80 after a chunk has been written to the buffer, or
when a sample has been read from the buffer while "idle" (that is, a
chunk is not currently being written to the buffer).

### `z_min_buffer` (`$A00E04`)

This variable specifies the minimum size of the buffer. If the current
size of the buffer is less than the value of this variable, the Z80
will start reading a new chunk.

## The Vblank interrupt

At the end of active scan, the Z80 will execute the vertical blanking
interrupt (usually known as Vblank), assuming it is not already being
stopped by the 68000.

During the vblank interrupt, the Z80 sets the `z_min_buffer` variable
to the minimal allowed buffer size (40 samples).

From the 68000's perspective, DMA is usually most effective during this
period, so this is basically a "proactive" attempt to make sure that
the Z80 will not start reading a new chunk during the time between the
start of the vblank interrupt and when DMA is started.

## 68000 side protection

There are limitations of the current method. If the Z80 starts reading
a chunk just before the vblank interrupt triggers, it will be unable
to temporarily stop reading from the 68000 bus until the chunk has
completed, since there is not enough time to read the `z_min_buffer`.

Also, there may not always be a lot of time between the start of vblank
and when the first DMA transfer occurs. The more you wait in vblank
before starting the DMA, the bigger chance there is that the Z80 will
gracefully handle the DMA. However, what if there simply wasn't enough
time and the 68000 starts a DMA while the Z80 is in the middle of
reading a chunk.

Now we already know for sure that the Z80 will be stopped if it tries
to read from the bus while a DMA is ongoing. The sound quality will be
degraded during this DMA transfer. Now, in the worst case, the Z80 may
request the bus just as the the DMA is started, and the glitches that
I mentioned earlier will occur. So the question is, how do we prevent
them in this case?

The method that SGDK currently uses (and that which I would suggest for
now) is to from the 68000, assert a Z80 bus request and deassert it,
immediately before starting the DMA (writing the final word to the VDP
command port).

## Replenishing the buffer

After you have completed your DMA, you should now restore the value of
the `z_min_buffer` variable to let the Z80 replenish the buffer until
the next vblank. The amount of time needed for this of course vary
depending on how long the Z80 had to wait for the DMA.

## Future ideas

I am considering reading the VDP vcounter during the idle wait to allow
the Z80 to start depleting the buffer before the vblank occurs.
However, because the VDP is located on the 68000 bus, it would also be
problematic to constantly read this value.

Furthermore, on PAL systems, it is not possible to determine if we are
actually in vblank from just reading the vcounter, since it wraps
around to a value (0xCA) that indicates an active scanline (since it is
lower than 0xE0, which is when Vblank is normally started).

Because of this, it is possible that the DMA protection will trigger
more than once in the same vblank period, and if we already told the
Z80 the DMA is done, it would not buffer any samples during the next
frame!

