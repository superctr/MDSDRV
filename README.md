MDSDRV
======
A sound driver for Sega Mega Drive/Genesis.

## Features
- Runs on the 68000 with the Z80 controlling PCM playback.
- 16 monophonic tracks that can be used for BGM or sound effects.
	Each track can be assigned to a channel with 4 levels of priority.
- Can use any combination of FM, PSG and PCM sound sources.
- FM3 special mode:
	- Can assign each or a pair of FM3 operators to one or more
		separate tracks with volume control.
- PCM playback modes:
	- 2 channel PCM playback at ~17.5 kHz with volume control
- Compact sequence format
	- Sound data can be compiled from MML files using `ctrmml`
- Various effects supported:
	- PSG volume envelopes
	- Pitch envelopes
	- Portamento
	- Fade in/out
	- Song volume and tempo control
- Equal temperament scale with 256 steps per semitone, allowing for
	easy detune or pitch slide effects.
- Unencumbered source code
- Fully position-independent code - supports any assembler or toolchain
	by including the binary blob.
- Uses approximately 1 kb (1024 bytes) of RAM.
- Integration with SGDK

### Upcoming features
- LFO control
- Macro tables
	- Allows for channel parameters and registers to be modified
		automatically as a note is playing.
	- I haven't decided on how this should be implemented.
- 3 channel PCM playback at ~13 kHz with volume control
	- This is supported by the PCM driver but not yet used by the
	  68k code.

## How to use

### Composing sound data
[mmlgui](https://github.com/superctr/mmlgui) provides an easy way
to compose music and sound effects using Music Macro Language (MML).

For information on the MML dialect, see
[here](https://github.com/superctr/ctrmml/blob/master/mml_ref.md).

### Compiling sound data
The `mdslink` tool of [ctrmml](https://github.com/superctr/ctrmml)
is capable of building the sound data files (`mdsseq.bin` and
`mdspcm.bin`).

	# creates 'mdsseq.bin' and 'mdspcm.bin'
	mdslink -o mdsseq.bin mdspcm.bin mymusic.mml ...

Sound data files can be in MML (Music Macro Language) or MDS formats.
For information on the MDS binary format, see
[mdsseq.md](doc/mdsseq.md).

### Building MDSDRV
All required tools for building are already in the repository, you just
need to run `build.bat`. These are Windows executables, so for other
operating systems, `wine` is required.

	# creates `main.bin` (test ROM) and `mdsdrv.bin` (sound driver blob)
	wine /c build.bat

### Using MDSDRV
Once compiled, using MDSDRV should be as easy as including the binary
files (`mdsdrv.bin`, `mdsseq.bin`, `mdspcm.bin`) in your project, and
calling the correct addresses inside the `mdsdrv.bin` blob.

#### File listing
- `main.bin` is the sound test ROM. It is built from  `main.68k`.
- `out/mdsdrv.bin` is the sound driver binary. It is built from
	`blob.68k`.
- `out/mdsseq.bin` is the compiled sequence data. It is built from
	music and sound effect files using `mdslink`.
- `out/mdspcm.bin` contains PCM sample data. It is also built using
	`mdslink`.

#### SGDK test project
- See [sample/sgdk](sample/sgdk/README.md).

If the SGDK environment variables are set, calling `build.bat` should
automatically build the SGDK test program.

#### Interfaces
- See [api.md](doc/api.md)

#### Sound data reference
- See [mdsseq.md](doc/mdsseq.md)

#### Precautions when using DMA
- See [dma.md](doc/dma.md)

## Copyright
&copy; 2019-2020 Ian Karlsson.

MDSDRV is licensed under the terms of the [zlib license](COPYING).

### Other programs included in this repository
`mdslink` is part of [ctrmml](https://github.com/superctr/ctrmml) and
is licensed under the terms of the GPLv2. But all code is written by me
and I give myself permission to use it here :).

`asm68k` is owned by SN Systems and is non-free. However it was last
released 25 years ago. I hope to move away from using it soon. The code
does however currently still use a lot of specific directives, and I
prefer its listing outputs for debugging... Please bear with me as I
try to find a good replacement.

`sjasmplus` is licensed under the zlib license. The version included in
this repository has been downloaded from the
[Github repository](https://github.com/sjasmplus/sjasmplus).

`slz` is licensed under the zlib license. The version included in this
repository has been compiled from the source code in the
[Github repository](https://github.com/sikthehedgehog/mdtools).

## Special thanks
- Sik: for making suggestions and giving inspiration for the Z80 driver.
	Also for the SLZ algorithm used to compress the Z80 driver.
- Yuzo Koshiro: For creating the MUCOM88 driver which was an
	inspiration to this project.
- All beta testers.
