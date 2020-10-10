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

#ifndef MDSDRV_H
#define MDSDRV_H

#include "mdsdat.h"
#include "mdsseq.h"

/* Work area size */
#define MDS_WORK_SIZE 512

/* Sound effect / music priority slots */
#define MDS_BGM 3
#define MDS_SE1 2
#define MDS_SE2 1
#define MDS_SE3 0

/* Command numbers */
#define MDS_CMD_GET_CMD_CNT    0
#define MDS_CMD_GET_SOUND_CNT  1
#define MDS_CMD_GET_STATUS     2
#define MDS_CMD_GET_VERSION    3
#define MDS_CMD_GET_GTEMPO     4
#define MDS_CMD_SET_GTEMPO     5
#define MDS_CMD_GET_GVOLUME    6
#define MDS_CMD_SET_GVOLUME    7
#define MDS_CMD_WRITE_FM_PORT0 8
#define MDS_CMD_WRITE_FM_PORT1 9
#define MDS_CMD_FADE_BGM       10
#define MDS_CMD_SET_PAUSE      11
#define MDS_CMD_GET_VOLUME     12
#define MDS_CMD_SET_VOLUME     13
#define MDS_CMD_GET_TEMPO      14
#define MDS_CMD_SET_TEMPO      15

/* Work area, resides in ram */
extern u16 MDS_work [MDS_WORK_SIZE];

/* API functions - see mdsdrv.c */
u16 MDS_init(const u8* seqdata, const u8* pcmdata);
void MDS_request(u16 slot, u16 id);
u32 MDS_command(u16 id, u16 param);
u32 MDS_command2(u16 id, u16 param1, u16 param2);

/* Manually update sound driver */
void MDS_update();

/* Wrapper functions */
char* MDS_get_version_str();
void MDS_pause(u16 slot, bool state);
void MDS_fade(u8 target, u8 speed, bool stop_when_done);

#endif
