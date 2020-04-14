`mdsdrv` Interface Reference
============================

This document describes the public interfaces of `mdsdrv`.
These functions are used by the main 68000 program to interface with the sound driver.

Functions
---------

### `mds_init`
- **Description**: Initializes the sound driver.
- **Address**: `mdsdrv+0`
- **Input**:
	- `a0` - Pointer to work area in RAM.
	- `a1` - Pointer to sequence data (`mdsseq.bin`)
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
	- Request sound number `#0` to stop all tracks on the specified
		priority level.
	- Requesting an invalid sound number will _not_ crash the sound
		driver.

### `mds_command`
- **Description**: Send a command to the sound driver.
- **Address**: `mdsdrv+12`
- **Input**:
	- `a0` - Pointer to work area in RAM.
	- `d0` - Command number.
	- `d1` - Command parameter (if applicable).
- **Output**:
	- `a0` - Return value (if applicable)
	- `d0` - Return value (if applicable)
- **Trashes**:
	- `a0-a1`, `d0-d1`.

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
	size of the version string (in bytes) is given in `d0`.

##### `get_gtempo`
- **Command** `0x04`
- **Description**: Returns the global tempo in `d0`. The default value
	is `128` for NTSC systems and `107` for PAL systems.

##### `set_gtempo`
- **Command** `0x05`
- **Description**: Sets the global tempo to the value in `d1`. Automatic
	PAL/NTSC adjustment will not be done.
