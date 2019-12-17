MDSDRV Sound data format
========================

Definitions
-----------

Struct member definitions will be given like this:

	+offset (type) [count] - name

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

The .MDS format is based on RIFF (Resource interchangable file format);
various data types are stored in named chunks.

If the size of the chunk is non-even, an extra padding byte is
inserted. This does not count into the chunk size. Though if a padding
byte is inserted between two subchunks, they will count into the size
of the parent chunk.

### .MDS File header (version 0)
This is a standard RIFF header.

	+0 (ub) [0..3] - file_id
		"RIFF" (standard RIFF format)
	+4 (ul) - file_size
		Size of the file - 8. (little endian)
	+8 (ub) [0..3] - file_format
		"MDS0" (MDS file version 0)
	+12 (ub) [0..file_size-4] - file_data
		Contains the chunks listed below

### Sequence data chunk
This contains MDSDRV sequence data. This data is copied almost straight
into the ROM. The song data table entries may be stored as zeroes here.
In the ROM, they should have been changed to point to the locations of
the respective data blocks.

	+0 (ub) [0..3] - chunk_id
		"seq "
	+4 (ul) - chunk_size
		Size of the chunk_data (little endian)
	+8 (ub) [0..chunk_size] - chunk_data
		Sequence header and sequence data

### Data block list chunk
This is a RIFF LIST chunk that contains the data block list. List items
(subchunks) can be global data blocks ("glob") or PCM data blocks
("wave").

	+0 (ub) [0..3] - chunk_id
		"LIST" (standard RIFF list)
	+4 (ul) - chunk_size
		Size of the chunk data (including all subchunks) (little endian)
	+8 (ub) [0..3] - list_type
		"dblk"
	+12 (ub) [0..chunk_size-4] - subchunk_data
		Contains any number of subchunks listed below

#### Global data block subchunk definition
This defines an instrument or envelope that is to be inserted into
the global data bank during .MDS to ROM compilation.

	+0 (ub) [0..3] - subchunk_id
		"glob"
	+4 (ul) - subchunk_size
		Size of chunk - 8 (little endian)
	+8 (uw) - data_type
		Data type (little endian)
	+10 (uw) - data_id
		Data ID (little endian)
	+12 (ub) [0..chunk_size-4] - subchunk_data
		Data

##### Compilation process
When the address of each data block in ROM is finalized, a (uw) pointer
in the sequence data at `tbase + data_id*2` should be replaced with the
address of the data block relative from `sdtop`.

#### PCM data block subchunk definition
This defines a PCM sample with associated metadata.

	TODO: define

Sound data header format
------------------------

	+0 (uw) - sdcnt
		This is the total count of sequences.
	+2 (ul) [0..sdcnt] - sdtop
		Sequence pointers.
		Each sequence pointer is offset from the beginning of this
		table.

Sequence header format
----------------------

	+0 (sw) - tbase
		Offset to song data table. Also used as base offset when
		calculating track pointers.
	+2 (uw) - tmask
		This is a bitmask which is read from the MSB. If the bit is
		set, the corresponding track is enabled. Bit 15 corresponds
		to track 0, bit 14 to track 1 and so on.
	+4 (4)
		Track table, read as follows:
			+0 (ub) - t_channel_id
				Channel ID
			+1 (ub) - t_channel_flag
				Must be 0x00
			+2 (sw) - t_position
				Sequence data offset from song data table (tbase)

	tbase+0 (sw) [0..n]
		Song data entries. Note that these are offset from sdtop
		(the sequence pointer table)

Sequence data format (WIP)
--------------------------

	00..7f
		Rest. The length is the command byte.
	80 [00..7f] - rst
		Rest. If the argument is omitted, assume the same length as
		the previous rest.
	81 [00..7f] - tie
		Tie.  Argument is the length. If omitted, assume the
		same length as the previous note or tie.
	81..df [00..7f]
		Note. The note number (starting from C1) is the command
		byte - 0x81. Argument is the length. If omitted, assume the
		same length as the previous note or tie.
	e0 - slr
		Slur
	e1 dd(ub) - ins
		Change instrument. The argument is an index into the song
		data table.
	e2 dd(ub) - vol
		Set volume. If argument is >0x80, use the volume table.
	e3 dd(sb) - volm
		Modify volume. If the previous `vol` command uses the volume
		table, do so here as well. No overflow checking is done.
	e4 dd(sb) - trs
		Set transpose.
	e5 dd(sb) - trsm
		Modify transpose. No overflow checking is done.
	e6 dd(sb) - dtn
		Set detune.
	e7 dd(ub) - pta
		Set portamento. This causes the pitch to slide between
		notes with the argument controlling the speed (lower=faster)
	e8 dd(ub) - peg
		Set pitch envelope. The argument is an index into the song
		table. See the pitch envelope format specified in this file.
	e9 dd(ub) - pan
		Set panning for this channel.
		TODO: explain further
	ea dd(ub) - lfo
		Set LFO parameters for this channel.
		TODO: explain further
	eb dd(ub) - lfod
		Set LFO delay for this channel.
		TODO: explain further.
	ec dd(ub) - flg
		Set flags for this channel.
		TODO: explain further.
	ed rr(ub) ww(ub) - fmcreg
		FM channel register write. Write data dd to register rr
		adding a fixed offset for the channel.
	ee oo(ub) dd(ub) - fmtl
		FM TL write.
		TODO: explain further
	ef oo(ub) dd(ub) - fmtlm
		FM TL modify.
		TODO: explain further
	f0 dd(ub) - pcm
		Set instrument and enable PCM mode for this channel.
		TODO: explain further
	f8 rr(ub) ww(ub) - fmreg
		FM register write. Write data dd to register rr.
	f9 dd(ub) - tempo
		Set tempo. Tempo is given as dd*256/300
	fa - lp
		Loop start.
	fb - lpf
		Loop finish.
	fc ww(sw) - lpb
		Loop break.
	fd ww(sl) - lpbl
		Loop break (long).
	fe ww(sw) - pat
		Pattern (subroutine)
	ff - finish
		Finish.

Voice data format
-----------------

	+0 (ub) [0..3] - DT/MUL
	+4 (ub) [0..3] - KS/AR
	+8 (ub) [0..3] - AM Enable/DR
	+12 (ub) [0..3] - SR
	+16 (ub) [0..3] - SL/RR
	+20 (ub) [0..3] - SSG-EG
	+24 (ub) [0..3] - TL
	+28 - FB/ALG

PSG envelope format
-------------------
TODO

Pitch envelope format
---------------------
TODO

