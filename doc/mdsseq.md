`mdsdrv` Sound Data Reference
=============================

This document describes the public data structures of `mdsdrv`, as well
as the `.MDS` format used for data exchange.

This document may contain some minor errors until all the bugs have
been fixed. I take no responsibility for causing the end of the
universe as a result of these errors.

##### Formatting note
This document may not display properly in all markdown viewers or
editors. However it should look fine in GitHub, as well as in a text
editor (As with the rest of the code in this repository, please use
a tab size of 4 spaces.)

Table of contents
-----------------
1. [Definitions](#definitions)
2. [.MDS binary file format](#mds-binary-file-format)
	1. [.MDS file header (version 0)](#mds-file-header-version-0)
	2. [Sequence data chunk](#sequence-data-chunk)
	3. [Data block list chunk](#data-block-list-chunk)
		1. [Global data block subchunk](#global-data-block-subchunk)
		2. [PCM sample header subchunk](#pcm-sample-header-subchunk)
	4. [PCM sample data chunk](#pcm-sample-data-chunk)
3. [Sound data header format](#sound-data-header-format)
4. [Sequence header format](#sequence-header-format)
	1. [Channel ID table](#channel-id-table)
5. [Sequence data format](#sequence-data-format)
	1. [Using Drum mode](#using-drum-mode)
	2. [Using FM3 special mode](#using-fm3-special-mode)
	3. [Using PSG noise mode](#using-psg-noise-mode)
6. [FM voice format](#fm-voice-format)
7. [PSG envelope format](#psg-envelope-format)
8. [Pitch envelope format](#pitch-envelope-format)

Definitions
-----------

Struct member definitions will be given like this:

-	`+offset (type) [count]` - `name`

	- `offset` refers to the beginning of the struct.
	- `type` is the type. Refer to the following list. Unless otherwise
		mentioned, assume words are big endian (Motorola format)
		- `ub` - unsigned byte (8-bits)
		- `uw` - unsigned word (16-bits)
		- `ul` - unsigned long (32-bits)
		- `sb` - signed byte (8-bits)
		- `sw` - signed word (16-bits)
		- `sl` - signed long (32-bits)
	- `count` The number of elements in an array.
	- `name` The name of the byte.

.MDS binary file format
-----------------------

The `.MDS` format is based on RIFF (Resource interchangable file
format); various data types are stored in named chunks.

If the size of the chunk is non-even, an extra padding byte is
inserted. This does not count into the chunk size. Though if a padding
byte is inserted between two subchunks, they will count into the size
of the parent chunk.

### .MDS file header (version 0)
This is a standard RIFF header.

- `+0 (ub) [0..3]` - `file_id`
	`"RIFF"` (standard RIFF format)

- `+4 (ul)` - `file_size`
	Size of the file - 8. (little endian)

- `+8 (ub) [0..3]` - `file_format`
	`"MDS0"` (MDS file version 0)

- `+12 (ub) [0..file_size-4]` - `file_data`
	Contains the chunks listed below

### Sequence data chunk
This contains MDSDRV sequence data. This data is copied almost straight
into the ROM. The song data table entries may be stored as zeroes here.
In the ROM, they should have been changed to point to the locations of
the respective data blocks.

- `+0 (ub) [0..3]` - `chunk_id`
	`"seq "`

- `+4 (ul)` - `chunk_size`
	Size of the chunk content (little endian)

- `+8 (ub) [0..chunk_size]` - `seq_data`
	Chunk content

### Data block list chunk
This is a RIFF LIST chunk that contains the data block list. List items
(subchunks) can be global data blocks ("glob") or PCM data blocks
("wave").

- `+0 (ub) [0..3]` - `chunk_id`
	`"LIST"` (standard RIFF list)

- `+4 (ul)` - `chunk_size`
	Size of the chunk data (including all subchunks) (little endian)

- `+8 (ub) [0..3]` - `list_type`
	`"dblk"`

- `+12 (ub) [0..chunk_size-4]` - `subchunk_data`
	Contains any number of subchunks listed below

#### Global data block subchunk
This defines an instrument or envelope that is to be inserted into
the global data bank during .MDS to ROM compilation.

- `+0 (ub) [0..3]` - `subchunk_id`
	`"glob"`

- `+4 (ul)` - `subchunk_size`
	Size of chunk - 8 (little endian)

- `+8 (ul)` - `data_id`
	Data ID (little endian)

- `+12 (ub) [0..chunk_size-4]` - `subchunk_data`
	Data content

##### Compilation note
When the address of each data block in ROM is finalized, a `(uw)`
pointer in the sequence data at `tbase + data_id*2` should be replaced
with the address of the data block relative from `sdtop`.

#### PCM sample header subchunk
This defines a PCM sample with associated metadata.

- `+0 (ub) [0..3]` - `subchunk_id`
	`"pcmh"`

- `+4 (ul)` - `subchunk_size`
	Size of chunk - 8 (little endian). For PCM samples, this is always
	36 for now.

- `+8 (ul)` - `data_id`
	Data ID (little endian)

- `+12 (ul)` - `position`
	Position of the PCM sample in the PCM data chunk. Also see the
	`start` parameter.

- `+16 (ul)` - `start`
	Sample start offset.

- `+20 (ul)` - `size`
	Sample size in bytes

- `+24 (ul)` - `loop_start`
	This value is not used by MDSDRV.

- `+28 (ul)` - `loop_end`
	This value is not used by MDSDRV,

- `+32 (ul)` - `rate`
	Sample playback frequency.

- `+36 (ul)` - `transpose`
	This value is not used by MDSDRV.

- `+40 (ul)` - `flags`
	This value is not used by MDSDRV.

##### Compilation note
When the address of each data block in ROM is finalized, a `(uw)`
word in the sequence data at `tbase + data_id*2` should be replaced
with the index of the PCM sample in the PCM sample table.

The sample data should be read from the PCM data chunk, at position
`pcm_data+position+start`.

### PCM sample data chunk
Contains the PCM sample data used by the song.

- `+0 (ub) [0..3]` - `subchunk_id`
	`"pcmd"`

- `+4 (ul)` - `chunk_size`
	Size of chunk - 8 (little endian).

- `+8 (ub) [0..chunk_size]` - `pcm_data`
	PCM sample data.

Sound data header format
------------------------

- `+0 (uw)` - `sdcnt`
	This is the total count of sequences.

- `+2 (ul) [0..sdcnt]` - `sdtop`
	Sequence pointers.
	Each sequence pointer is offset from the beginning of this
	table.

Sequence header format
----------------------

- `+0 (sw)` - `tbase`
	Offset to song data table. Also used as base offset when
	calculating track pointers.

- `+2 (uw)` - `tmask`
	This is a bitmask which is read from the MSB. If the bit is
	set, the corresponding track is enabled. Bit 15 corresponds
	to track 0, bit 14 to track 1 and so on.

- `+4 (4)`
	Track table, read as follows:
	- `+0 (ub)` - `t_channel_id`
		[Channel ID](#channel-id-table)
	- `+1 (ub)` - `t_channel_flag`
		Must be `0x00`
	- `+2 (sw)` - `t_position`
		Sequence data offset from song data table (tbase)

- `tbase+0 (sw) [0..n]`
	Song data entries. Each entry is a `uw` offset, but the base
	offset depends on the type of entry.
	| Data type            | Base offset |
	| -------------------- | ----------- |
	| FM voice             | `sdtop`     |
	| PSG envelope         | `sdtop`     |
	| Pitch envelope       | `sdtop`     |
	| `pat` subroutine     | `tbase`     |
	| Drum mode subroutine | `tbase`     |

### Channel ID table
| ID       | Channel        |
| -------- | -------------- |
| `00..05` | FM1 - FM6      |
| `06..08` | PSG1 - PSG3    |
| `09`     | PSG noise      |
| `0a..0f` | Dummy/reserved |
| `10..ff` | Do not use     |

Sequence data format
--------------------
Please note that the byte values are in hexadecimal here. The mnemonics
correspond to the defines in `mdsseq.inc`, which is generated by
`gendef.c` and should match with the assembly code.

- `00..7f`
	Rest. The length is the command byte.

- `80 [00..7f]` - `rst`
	Rest. If the argument is omitted, assume the same length as
	the previous rest.

- `81 [00..7f]` - `tie`
	Tie.  Argument is the length. If omitted, assume the
	same length as the previous note or tie.

- `82..df [00..7f]`
	Note. The note number (starting from C1) is the command
	byte - 0x82. The argument is the length. If omitted, assume
	the same length as the previous note or tie.

	See "Note values" for a list of note values here.

- `e0` - `slr`
	Slur/legato - key-off before the next note is omitted.

- `e1 dd(ub)` - `ins`
	Change instrument. `dd` is an index into the song
	data table. It must be the appropriate type (FM voice or
	PSG envelope) for the current channel!

- `e2 dd(ub)` - `vol`
	Set volume.
	- If `dd` is between `80..8f`, use a typical MML volume
		volume scale, where `80` is the lowest value,
		corresponding to -31.5 dB, and `8f` is the highest value,
		corresponding to -1.5 dB, effectively 2 dB per step.
	- If `dd` is between `00..7f`, use an FM volume scale,
		where `00` is the lowest value, corresponding to -0 dB,
		and `7f` is the highest value, corresponding to -95.25
		dB, effectively -0.75 dB per step.
	- Values `90..ff` are reserved for future use.

- `e3 dd(sb)` - `volm`
	Modify volume. No overflow checking is done.

- `e4 dd(sb)` - `trs`
	Set transpose.

- `e5 dd(sb)` - `trsm`
	Modify transpose. No overflow checking is done.

- `e6 dd(sb)` - `dtn`
	Set detune.

- `e7 dd(ub)` - `pta`
	Set portamento. This causes the pitch to slide between
	notes with the argument controlling the speed (lower=faster)

- `e8 dd(ub)` - `peg`
	Set pitch envelope. The argument is an index into the song
	table (plus one). If zero, the pitch enveloep is disabled.
	See the pitch envelope format specified in this file.

- `e9 dd(ub)` - `pan`
	Set panning for this channel. Only applies for FM channels.
	| Value | Description      |
	| ----- | ---------------- |
	| `80`  | Left             |
	| `c0`  | Center (default) |
	| `40`  | Right            |
	| `00`  | Disables output  |

- `ea dd(ub)` - `lfo` (FM only)
	Set LFO parameters for this channel. Bits 0-2 sets the phase
	modulation sensitivity (vibrato), and bits 5-6 sets the
	amplitude modulation sensitivity (tremolo).

- `ea dd(ub)` - `lfo` (PSG only)
	Set the PSG noise mode. Valid values are one of the following:
	| Value | Description                              |
	| ----- | ---------------------------------------- |
	| `00`  | Disable noise mode, use keycode directly |
	| `e3`  | Periodic noise, use PSG3 frequency       |
	| `e7`  | White noise, use PSG3 frequency          |
	See [PSG noise mode](#using-psg-noise-mode) for more details.

- `eb dd(ub)` - `lfod`
	Set LFO delay for this channel. Causes the LFO sensitivity to
	be 0 until `dd` ticks have passed since key-on.

- `ec dd(ub)` - `flg`
	Set flags for this channel. Values other than those in this
	table should not be used.
	| Enable | Disable | Description                                 |
	| ------ | ------- | ------------------------------------------- |
	| `08`   | `00`    | [Drum mode](#using-drum-mode)               |
	| `8x`   | `80`    | [FM3 special mode](#using-fm3-special-mode) |

- `ed rr(ub) ww(ub)` - `fmcreg`
	FM channel register write. Write data `dd` to register `rr`
	adding a fixed offset for the channel.

- `ee oo(ub) dd(ub)` - `fmtl`
	Set the instrument base TL for operator `oo` to `dd`.

- `ef oo(ub) dd(ub)` - `fmtlm`
	Modify the instrument base TL for operator `oo` with `dd`.

- `f0 dd(ub)` - `pcm`
	Set waveform number and enable PCM mode for this channel.

- `f1..f5`
	Reserved for future use.

- `f6 rr(ub) ww(ub)` - `fmreg`
	FM register write. Write data `dd` to register `rr`. Please
	note that the register is always written even if the channel
	is suppressed due to sound effects.

- `f7 dd(ub)` - `dmfinish`
	Plays a note with pitch dd, Stops processing of Drum Mode,
	and reads duration from the original `81..df` command.

- `f8 dd(ub)` - `comm`
	Communication byte write. Used to sync events outside the
	sound driver with the music.

- `f9 dd(ub)` - `tempo`
	Set tempo. Tempo is given as `dd`*300/256 beats per minute.

- `fa` - `lp`
	Loop start. Each loop requires 4 bytes of stack space.

- `fb dd(ub)` - `lpf`
	Loop finish. `dd` is the loop count, a value of `00` causes
	an infinite loop.

- `fc dd(ub)` - `lpb`
	Loop break. On the last loop count, skip `dd` bytes from the
	beginning of this command.

- `fd ww(uw)` - `lpbl`
	Loop break (long). On the last loop count, skip `ww` bytes from the
	beginning of this command.

- `fe dd(ub)` - `pat`
	Pattern (subroutine). `dd` is an index into the song table.
	Each subroutine requires 2 bytes of stack space.

- `ff` - `finish`
	Finish.

### Using Drum mode
Drum mode is a special mode in MDSDRV that allows note events
(`82..df`) to call a subroutine, where channel parameters can be
adjusted, before returning and reading the duration.

The note number (command minus `0x82`) is used as index into the
song table, in order to read the address of the subroutine.

### Using FM3 special mode
FM3 special mode allows the individual operators to be split into
two or more tracks, allowing for increased polyphony. The bitmask
specified by the lower 4 bits of the `flg` command argument sets which
operators of FM3 should be controlled by this tracks.

### Using PSG noise mode
The PSG noise channel can normally only generate 3 noise frequencies.
However, it can also use the frequency generator of PSG3. The PSG
noise mode allows the PSG noise track to set the PSG3 frequency.
By doing so however the noise channel must be given priority over PSG3.

Therefore, please take the following precautions if you use both music
and sound effects in your program:

- If neither music or sound effects uses PSG3 normally, there is no
issue. Only the PSG noise channel needs to be allocated.

- If music uses PSG3 normally and a sound effect that uses the PSG3
noise mode is played, the sound effect should allocate one track each
to PSG3 and PSG noise. The PSG3 track must be filled with rests so it
is at least as long as the PSG noise track.

- If music uses the PSG noise mode and a sound effect that uses PSG3
normally is played, again, the sound effect should allocate one track
each to PSG3 and PSG noise. The PSG noise track must be filled with
rests so it is at least as long as the PSG3 track.

FM voice format
---------------

- `+0 (ub) [0..3]` - DT/MUL
- `+4 (ub) [0..3]` - KS/AR
- `+8 (ub) [0..3]` - AM Enable/DR
- `+12 (ub) [0..3]` - SR
- `+16 (ub) [0..3]` - SL/RR
- `+20 (ub) [0..3]` - SSG-EG
- `+24 (ub) [0..3]` - TL
- `+28` - FB/ALG
- `+29` - Transpose

The instrument transpose setting works by adjusting the F-num of the
notes in the scale. Using a lower value makes it possible to use
lower F-nums (and thus increased detune and reduced key scaling), but
the pitch accuracy will be worse.

The possible values are 0..31, to be multiplied by 2. A suggested
default value is 24, corresponding to `0x30`.

PSG envelope format
-------------------

A PSG envelope consists of a sequence of commands. The envelope is
always restarted on a key-on.

- `00` - Silence channel and end envelope
- `01` - Sustain current volume until key off
- `02 dd` - Jump to position dd (counting from beginning of envelope).
	If key off has been issued, treat this command as `00`.
- `xy` - Set volume to `y` and wait for `x-1` frames.
	`x` must be a value between `2` and `f`.

Pitch envelope format
---------------------

A pitch envelope consists of a sequence of 4-byte nodes.
Each node is 4 bytes long and consists of the following values:

- `+0 (sw)` - Initial modulation
	Initial modulation. Each `0x100` step is a semitone.
	Range is from (`8000` to `7eff`)

- `+2 (sb)` - Delta
	This signed value is added to the modulation counter on each
	tick.

- `+3 (ub)` - Length
	The amount of ticks before the the envelope continues to the
	next node.

A jump command consists of a single word `7fxx` where `xx` is the
position of the next node, in logical elements (not bytes).

[comment]: # ( vim: set sw=4 ts=4 noet: )
