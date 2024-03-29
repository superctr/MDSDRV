`mdsdrv` Interface Reference
============================

This document describes the public interfaces of `mdsdrv`.
These functions are used by the main 68000 program to interface with
the sound driver.

Functions
---------

### `mds_init`
- **Description**: Initializes the sound driver.
- **Address**: `mdsdrv+0`
- **Input**:
	- `a0` - Pointer to work area in RAM.
	- `a1` - Pointer to sound data (`mdsseq.bin`). Must be an even
		address.
- **Trashes**:
	- `a0-a1`, `d0-d1`.

### `mds_update`
- **Description**: Updates all tracks and channels. Should be called
	on every VINT.
- **Address**: `mdsdrv+4`
- **Input**:
	- `a0` - Pointer to work area in RAM.
- **Trashes**:
	- `a0-a6`, `d0-d7`.

### `mds_request`
- **Description**: Requests a sound (BGM or effect)
- **Address**: `mdsdrv+8`
- **Input**:
	- `a0` - Pointer to work area in RAM.
	- `d0` - Sound number.
	- `d1` - Priority level (request slot).
- **Trashes**:
	- `d0-d1`.
- **Notes**:
	- Calling this function will stop all already playing tracks with
		that priority level.
	- Request sound number `0` to stop all tracks on the specified
		priority level.
	- Add `$2000 + (fade_speed << 10)` to the sound number to fade in
		the sound. Only works for BGM tracks. Bits 10-12 control the
		fading speed. For example, adding `$2000` will fade in slowly
		and `$3c00` will fade in quickly.
	- Requesting an invalid sound number will _not_ crash the sound
		driver.

### `mds_command`
- **Description**: Send a command to the sound driver.
- **Address**: `mdsdrv+12`
- **Input**:
	- `a0` - Pointer to work area in RAM.
	- `d0` - Command number.
	- `d1` - Command parameter (if applicable).
	- `d2` - Command parameter (if command number > `0x09`)
- **Output**:
	- `a0` - Return value (if applicable)
	- `d0` - Return value (if applicable)
- **Trashes**:
	- `a0-a1`, `d0-d1`.
	- Command numbers >= `0x09` will also trash `d2`.

#### Command numbers:
These are currently all the possible command numbers for `mds_command`.

##### `get_cmd_count`
- **Command** `0x00`
- **Description**: Returns the total number of defined commands in
	`d0`.

##### `get_sound_count`
- **Command** `0x01`
- **Description**: Returns the total number of defined sounds in `d0`.

##### `get_status`
- **Command** `0x02`
- **Description**: Returns a bitmask in `d0` where each bit represents
	a track. If the track is active with the specified priority level
	in `d1`, the bit is set.

##### `get_version`
- **Command** `0x03`
- **Description**: Returns a pointer to the version string in `a0`. The
	string is zero-terminated. A hexadecimal representation of the
	version is returned in `d0`. The length of the version string (in
	bytes) is given in `d1`.

##### `get_gtempo`
- **Command** `0x04`
- **Description**: Returns the global tempo in `d0`. The default value
	is `128` for NTSC systems and `107` for PAL systems.

##### `set_gtempo`
- **Command** `0x05`
- **Description**: Sets the global tempo to the value in `d1`. Automatic
	PAL/NTSC adjustment will not be done.

##### `get_gvolume`
- **Command** `0x06`
- **Description**: Returns a 16-bit value in d0 with the initial BGM
	(request slot 3) in the upper 8 bits and the initial sound effect
	(request slots 0..2) volume in the lower 8 bits.

##### `set_gvolume`
- **Command** `0x07`
- **Description**: Sets the initial volume to the 16-bit value in d0.
	See `get_gvolume` for more details.

##### `write_fm_port0`
- **Command** `0x08`
- **Description**: Writes the data defined in the lower 8 bits of `d1`
	to the FM port 0 register defined in the upper 8 bits of `d1`.
	Port 0 contains timer, DAC, key-on and FM1-FM3 operator registers.

##### `write_fm_port1`
- **Command** `0x09`
- **Description**: Writes the data defined in the lower 8 bits of `d1`
	to the FM port 1 register defined in the upper 8 bits of `d1`.
	Port 1 contains FM4-FM6 operator registers.

##### `fade_bgm`
- **Command** `0x0A`
- **Description**: Begins fade out or fade in of the background music
	(priority level 3).

	The upper 8 bits of `d1` define the fade speed. The valid range
	is 0 to 7, where 0 causes the volume to be updated every 8 frames,
	and 7 causes the volume to be updated every frame.

	If bit 7 of `d1` is set, the music will be stopped after the fade
	is complete.

	Bits 0-6 of `d1` sets the target volume level. 0 is the maximum
	volume, and 127 is the lowest volume. The attenuation is -0.75 dB
	per step.

##### `set_pause`
- **Command** `0x0B`
- **Description**: Pauses or resumes the tracks with the priority
	level specified in `d1`. Set `d2` to non-zero to pause, and zero
	to resume.

##### `get_volume`
- **Command** `0x0C`
- **Description**: Returns in `d0` the current song volume for the
	priority level specified in `d1`.

##### `set_volume`
- **Command** `0x0D`
- **Description**: Sets the song volume of the tracks with the priority
	level specified in `d1`. Set the new volume in `d2`. 0 is the
	maximum volume, and 127 is the lowest volume. The attenuation is
	-0.75 dB per step.

##### `get_tempo`
- **Command** `0x0E`
- **Description**: Returns in `d0` the current song tempo for the
	priority level specified in `d1`.

##### `set_tempo`
- **Command** `0x0F`
- **Description**: Sets the song tempo for the priority level specified
	in `d1` to the value in `d2`.

##### `get_comm`
- **Command** `0x10`
- **Description**: Returns in `d0` the communication byte value, set in
	the song data.

##### `set_pcmmode`
- **Command** `0x11`
- **Description**: Set the PCM mixing mode and buffering level.

	The byte in `d1` define the mixing mode. Valid values are
    2 or 3, for mixing 2 or 3 channels respectively. If the value is 0,
	the previous PCM mixing mode is kept.
	See [mdsseq.md](the sequence format) for more information.

	The byte in `d2` define the maximum buffering threshold.
	When this is 0, the DMA buffer is disabled, allowing for low latency
	PCM samples, however the Z80 needs to be stopped manually whenever
	a DMA tranfer is done. If this is non-zero, DMA buffering is enabled,
	and the value needs to be in the range 40-220. Other values may
	result in buffer underruns or overruns causing audio glitches.

	For more information see [dma.md](DMA protection).

##### `get_pcmmode`
- **Command** `0x12`
- **Description**: Get the PCM mixing mode

	Return the current PCM mode in `d0`. If the Z80 is still
	initializing, this will return 0.

