`mdsdrv`
========
A sound driver for Sega Mega Drive

## Features
- Runs on the 68000
- 16 monophonic tracks that can be used for BGM or sound effects.
	Each track can be assigned to a channel with 4 levels of priority.
- Supports 6 FM + 4 PSG
- Compact sequence format (similar to SMPS)
	- Sound data can be compiled from MML files using `ctrmml`
- Various effects supported:
	- PSG volume envelopes
	- Pitch envelopes
	- Portamento
- Equal temperament scale with 256 steps per semitone, allowing for
	easy detune or pitch slide effects.
- Unencumbered source code
- Fully position-independent code - supports any assembler or toolchain
	by including the binary blob.
- Uses approximately 1 kb (1024 bytes) of RAM.

### Upcoming features
- 2 channel PCM playback with volume control
- FM3 special mode

## How to use

### Compiling sound data
The `mdslink` tool of [ctrmml](https://github.com/superctr/ctrmml)
is capable of building the sound data files (`mdsseq.bin` and
`mdspcm.bin`).

	# creates 'mdsseq.bin' and 'mdspcm.bin'
	mdslink -o mdsseq.bin mdspcm.bin mymusic.mml ...

Sound data files can be in MML (Music Macro Language) or MDS formats.
For information on the MML dialect used by `mdslink`, see the `ctrmml`
documentation. For information on the MDS binary format, see
[mdsseq.md](doc/mdsseq.md).

### Building `mdsdrv`
All required tools for building are already in the repository, you just
need to run `build.bat`. These are Windows executables, so for other
operating systems, `wine` is required.

	# creates `main.bin` (test ROM) and `mdsdrv.bin` (sound driver blob)
	wine /c build.bat

### Using `mdsdrv`

#### File listing
- `main.bin` is the sound test ROM. It is built from  `main.68k`.
- `mdsdrv.bin` is the sound driver binary. It is built from `blob.68k`.
- `mdssub.bin` is the Z80 driver. It does not need to be included
	directly, it is done automatically by `mdsdrv.bin`.

#### Interfaces
- See [api.md](doc/api.md)

#### Sound data reference
- See [mdsseq.md](doc/mdsseq.md)

## Copyright
&copy; 2020 Ian Karlsson

## Special thanks
- Sik: for making suggestions and giving inspiration for the Z80 driver.
	Also for the SLZ algorithm used to compress the Z80 driver.
