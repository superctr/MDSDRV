//======================================================================
// MDSDRV API wrapper for SGDK
//======================================================================
// Copyright (c) 2020 Ian Karlsson
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must
//    not claim that you wrote the original software. If you use this
//    software in a product, an acknowledgment in the product
//    documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must
//    not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source
//    distribution.
//======================================================================

#include <genesis.h>
#include "mdsdrv.h"

//! Sound driver work area.
u16 MDS_work [MDS_WORK_SIZE];

//! Initialize the sound driver
/*!
 *  This initializes the work area, unpacks and starts up the PCM
 *  driver. MDSDRV should then be updated every vblank using
 *  MDS_update().
 *
 *  \param seqdata Pointer to sequence data file. Must be word aligned.
 *  \param pcmdata Pointer to PCM data file. Must be 32K aligned.
 */
u16 MDS_init(const u8* seqdata, const u8* pcmdata)
{
	register u16* a0 asm ("a0") = MDS_work;
	register const u8* a1 asm ("a1") = seqdata;
	register const u8* a2 asm ("a2") = pcmdata;
	register u16 d0 asm ("d0") = 0;
	asm volatile (
		"jsr mdsdrvdat+0"
		: "+a" (a0), "+a" (a1), "=r" (d0)
		: "a" (a2)
		: "d1", "cc");
	return d0;
}

//! Sound request
/*!
 *  \param slot Request priority (range 0-3). I suggest using the
 *              predefined values MDS_BGM, MDS_SE1, etc.
 *  \param id   Request sound number. Set 0 to stop the currently
 *              playing sound.
 */
void MDS_request(u16 slot, u16 id)
{
	register u16* a0 asm ("a0") = MDS_work;
	register u16 d0 asm ("d0") = id;
	register u16 d1 asm ("d1") = slot;
	asm volatile (
		"jsr mdsdrvdat+8"
		:
		: "r" (d0), "r" (d1), "a" (a0)
		: "cc" );
}

//! Command request for low-level access.
/*!
 *  Not all commands return a meaningful value, see MDSDRV
 *  documentation for details.
 *
 *  \param id    Command number
 *  \param param Command parameters
 */
u32 MDS_command(u16 id, u16 param)
{
	register u16* a0 asm ("a0") = MDS_work;
	register u32 d0 asm ("d0") = id;
	register u16 d1 asm ("d1") = param;
	asm volatile (
		"jsr mdsdrvdat+12"
		: "+r" (d0), "+r" (d1)
		: "a" (a0)
		: "a1", "d2", "cc");
	return d0;
}


//! Command request for low-level access (command number >= 0x09)
/*!
 *  Not all commands return a meaningful value, see MDSDRV
 *  documentation for details.
 *
 *  \param id     Command number
 *  \param param1 Command parameters
 *  \param param2 Command parameters
 */
u32 MDS_command2(u16 id, u16 param1, u16 param2)
{
	register u16* a0 asm ("a0") = MDS_work;
	register u32 d0 asm ("d0") = id;
	register u16 d1 asm ("d1") = param1;
	register u16 d2 asm ("d2") = param2;
	asm volatile (
		"jsr mdsdrvdat+12"
		: "+r" (d0), "+r" (d1), "+r" (d2)
		: "a" (a0)
		: "a1", "cc");
	return d0;
}

//! MDSDRV update function
/*!
 *  This must be called every vblank.
 */
void MDS_update()
{
	register u16* a0 asm ("a0") = MDS_work;
	register void* a6 asm ("a6"); /* Hack since gcc didn't like clobbering the frame pointer */
	asm volatile (
		"jsr mdsdrvdat+4"
		: "=a" (a6)
		: "a" (a0)
		: "a1", "a2", "a3", "a4", "a5", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc");
}

//! Get sound driver version
/*!
 *  Returns a pointer to a zero-terminated string containing the
 *  sound driver version and date.
 */
char* MDS_get_version_str()
{
	register u16* a0 asm ("a0") = MDS_work;
	register u32 d0 asm ("d0") = MDS_CMD_GET_VERSION;
	asm volatile (
		"jsr mdsdrvdat+12"
		: "+r" (d0), "+a" (a0)
		:
		: "a1", "d1", "d2", "cc");
	return (char*)a0;
}

//! Set pause on/off
/*!
 *  This pauses all tracks playing with the specified priority.
 *
 *  \param slot Request priority (range 0-3)
 *  \param state Non-zero to pause, zero to unpause
 */
void MDS_pause(u16 slot, bool state)
{
	MDS_command2(MDS_CMD_SET_PAUSE, slot, state);
}

//! Set BGM fade in/out
/*!
 *  This fades the volume of the BGM track (MDS_BGM) to the target
 *  level. The exact time it takes to fade out is as follows:
 *  (target-current) / (1+speed)
 *
 *  \param target Target volume. Range 0 to 127, -0.75 dB per step.
 *  \param speed Fading speed, 0 is fastest, 7 is slowest.
 *  \param stop_when_done BGM track
 */
void MDS_fade(u8 target, u8 speed, bool stop_when_done)
{
	MDS_command(MDS_CMD_FADE_BGM, (speed << 8) | ((stop_when_done & 1) << 7) | (target & 0x7f));
}
