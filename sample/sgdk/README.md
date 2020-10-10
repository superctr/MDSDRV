# Example SGDK project for MDSDRV
This project contains the MDSDRV C language wrapper and an example of
how to use MDSDRV in a C program using the [Sega Genesis Development
Kit (SGDK)](https://github.com/Stephane-D/SGDK).

## How to use MDSDRV in your own SGDK programs
1. Copy `inc/mdsdrv.h`, `src/mdsdrv.c`, and `res/mdsdat.res` into your
   project.
2. Copy `mdsdrv.bin`, `mdspcm.bin`, `mdsseq.bin`from the MDSDRV `out`
   directory.

## Important notes
To initialize MDSDRV, call `MDS_init()`.

The sound driver runs on the 68000 and thus it needs to be updated from
user code every frame. To do this, call `MDS_update()`. Make sure that
the sound driver has been initialized (using `MDS_init()`) before
calling the update function.

To play a song, use `MDS_request()`. This function takes two
parameters, the first defines the request slot. The second parameter is
the sound knumber.

There are 4 request slots available, the lowest priority one (3) is
intended for use with background music (BGM). As such it has the alias
`MDS_BGM`. The other slots are called `MDS_SE1`, `MDS_SE2` and
`MDS_SE3`. Only the BGM track can be faded out using the `MDS_fade()`
function.

This should be a given, but you cannot have SGDK's included Z80
sound drivers running running at the same time as MDSDRV. So, if you
have already initialized a Z80 driver in SGDK, call
`Z80_unloadDriver()` before `MDS_init()`. And when you want to use the
SGDK drivers again, make sure you stop calling `MDS_update()` first.

Most of the API functions are defined in `mdsdrv.c` and used in
`main.c`, so please look in those files for more information.
