/*
	mds2asm - .MDS file disassembler

	Tries to generate somewhat readable assembly code, including MML durations and
	note definitions.

	Compile with `gcc -o mds2asm mds2asm.c`, run with `mds2asm input.mds > output.asm`

	Note: currently, PCM data blocks are not being read from the MDS file.
	Some rarely used commands (lpbl for example) might not be read properly.

	/ctr 2021-01-03
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "seqdef.h"

#define FAIL(msg,...) do {fprintf(stderr,"[FAIL] : " msg, ##__VA_ARGS__); exit(-1);} while(0)
#define WARN(msg,...) do {fprintf(stderr,"[WARN] : " msg, ##__VA_ARGS__);} while(0)

#if 0
#define DEBUG(msg,...) do {fprintf(stderr,"[DEBUG] : " msg, ##__VA_ARGS__);} while (0)
#else
#define DEBUG(msg,...) do {} while (0)
#endif

enum data_type
{
	DATA_UNKNOWN = 0,
	//----- global data offset from sdtop
	DATA_FM = 1,
	DATA_PSG = 2,
	DATA_PCM = 3,
	DATA_PEG = 4,
	//----- local data offset from tbase
	DATA_LOCAL = 8,
	DATA_SUB = 8,
	DATA_DRUM = 9,
	DATA_TYPE_MASK = 15,
	DATA_PARSED = 16,
	DATA_IN_DRUM_MODE = 32
};

struct header_t
{
	uint16_t type;
	uint8_t channel_id;
	uint8_t channel_flag;
	uint16_t length;
	uint16_t loop;
	uint8_t* data;
	uint32_t hash;
	char alias[32];
	char label[32];
};

struct song_t
{
	char* filename;
	uint16_t tbase;
	uint8_t tvol;
	uint8_t tcount;
	uint8_t max_data;
	struct header_t tracks[16];
	struct header_t data[256];
};

struct song_t song;
char cmd_tab[256][16];

uint8_t* mds_data = NULL;
uint32_t mds_size;

uint8_t* seq_chunk = NULL;
uint32_t seq_size = 0;

uint32_t fnv1a32(uint8_t* data, uint32_t size);
void gen_tab();
void dump_tab();

int load_file(const char* path, uint8_t** data, uint32_t* size);
int read_mds_chunk(uint8_t* data, uint32_t size);
int extract_song();
int emit_song();

//=====================================================================

int main(int argc, char* argv[])
{
	gen_tab();
	if(argc < 2)
	{
		fprintf(stderr, "usage : %s <path to MDS file>\n", argv[0]);
		dump_tab();
	}
	else
	{
		if(load_file(argv[1], &mds_data, &mds_size))
		{
			FAIL("Couldn't open file '%s'\n", argv[1]);
		}
		else
		{
			song.filename = argv[1];
			int status = 0;
			if(!status) status = read_mds_chunk(mds_data, mds_size);
			if(!status) status = extract_song();
			if(!status) status = emit_song();
			free(mds_data);
			return status;
		}
	}
	return -1;
}

//=====================================================================

uint32_t fnv1a32(uint8_t* data, uint32_t size)
{
	uint32_t i, d = 2166136261;
	for(i = 0; i < size; i++)
	{
		d = d ^ data[i];
		d = d * 16777619;
	}
	return d;
}

//=====================================================================

void add_length(char* str, int length, char* last_str, int* last_length)
{
	int prev_length = (*last_length)--;
	while(*last_length > length)
	{
		if(strlen(last_str) && (prev_length - *last_length) < (*last_length - length))
		{
			static char swap_sign_buf[16];
			char* a = swap_sign_buf;
			strcpy(swap_sign_buf, cmd_tab[prev_length - *last_length - 1]);
			while(a = strpbrk(a, "+-"))
			{
				if(*a == '+')
					*a = '-';
				else
					*a = '+';
				a++;
			}
			sprintf(cmd_tab[*last_length - 1], "%s-%s", last_str, swap_sign_buf);
		}
		else
		{
			sprintf(cmd_tab[*last_length - 1], "%s+%s", str, cmd_tab[*last_length - length - 1]);
		}
		(*last_length)--;
	}
	strcpy(cmd_tab[length - 1], str);
	strcpy(last_str, str);
}

//=====================================================================

void add_lengths(char prefix)
{
	char str[16], last_str[16];
	int i, interval = 96, last_interval = 129;
	strcpy(last_str, "");
	for(i = 1; i < 64; i <<= 1)
	{
		if(interval < 96 && (~interval & 1))
		{
			sprintf(str, "%c%dd", prefix, i);
			add_length(str, interval + (interval >> 1), last_str, &last_interval);
		}
		sprintf(str, "%c%d", prefix, i);
		add_length(str, interval, last_str, &last_interval);
		interval >>= 1;
	}
}

//=====================================================================

void gen_tab()
{
	int i;
	memset(cmd_tab, 0, sizeof(cmd_tab) * sizeof(char));

	// note duration parameters (00-7f)
	for(i = 0; i < 128; i++)
		sprintf(cmd_tab[i], "%d", i + 1);
	for(i = 4; i; --i)
		add_lengths(i == 1 ? 'l' : 't');
	for(i = 0; i < 2; i++)
		sprintf(cmd_tab[i], "%d", i);

	// note commands (80-df)
	strcpy(cmd_tab[rest_cmd], "rest");
	strcpy(cmd_tab[tie_cmd], "tie");

	for(i = 0; i < 9*17; i++)
	{
		int note, oct, nn;
		oct = i / 17;
		note = i % 17;
		nn = note_start + (oct * 12) + note_value[note];
		if(nn <= note_end && !strlen(cmd_tab[nn]))
			sprintf(cmd_tab[nn], "%s%d", note_name[note],oct+1);
	}

	// channel commands
	for(i = 0; i < chn_cmd_cnt; i++)
	{
		strcpy(cmd_tab[chn_cmd_base(i)], chn_cmd[i].name);
	}

	// system commands
	for(i = 0; i < sys_cmd_cnt; i++)
	{
		strcpy(cmd_tab[sys_cmd_base(i)], sys_cmd[i].name);
	}
}

//=====================================================================

void dump_tab()
{
	int i;
	for(i = 0; i < 256; i++)
	{
		if(strlen(cmd_tab[i]))
			DEBUG("%d => '%s'\n", i, cmd_tab[i]);
	}
}

//=====================================================================

int load_file(const char* path, uint8_t** data, uint32_t* size)
{
	FILE* f = fopen(path, "rb");
	if(!f)
		return -1;
	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	rewind(f);
	*data = (uint8_t*) malloc((*size) + 128);

	int res = fread(*data, 1, *size, f) != *size;
	fclose(f);

	if(res)
		free(*data);

	return res;
}

//=====================================================================

int read_list_chunk(uint8_t* data)
{
	int pos = 4;
	uint32_t riff_size, chunk, chunk_size;

	riff_size = *(uint32_t*)(data + pos);

	if(riff_size < 8)
		FAIL("File size unexpectedly short\n");

	pos+=4;

	riff_size += pos;

	if(memcmp(&data[pos], "dblk", 4))
		return 0;

	pos+=4;

	while(pos < riff_size)
	{
		uint32_t data_id;

		// chunks must be word aligned
		if(pos & 1)
			pos++;

		chunk = *(uint32_t*)(data+pos);
		chunk_size = *(uint32_t*)(data+pos+4);

		DEBUG("Subchunk id: %c%c%c%c (%08x), size %d bytes\n",
			data[pos],data[pos+1],data[pos+2],data[pos+3],chunk,chunk_size);

		switch(chunk)
		{
		case 0x626f6c67: // 'glob'
			if(chunk_size < 4)
				FAIL("glob chunk is too small\n");
			data_id = *(uint32_t*)(data + pos + 8);
			if(data_id > 255)
				FAIL("data id is too high\n");
			song.data[data_id].data = data + pos + 12;
			song.data[data_id].length = chunk_size - 4;
			break;
		default: // ignore all other chunks
			break;
		}
		pos += chunk_size+8;
	}
	return 0;
}

//=====================================================================

int read_mds_chunk(uint8_t* data, uint32_t size)
{
	int pos = 4;
	uint32_t riff_size, chunk, chunk_size;

	if(size < 13)
		FAIL("File size unexpectedly short\n");

	if(memcmp(&data[0],"RIFF",4))
		FAIL("Riff header not found\n");

	riff_size = *(uint32_t*)(data + pos);
	pos+=4;

	riff_size += pos;
	if(size != riff_size)
	{
		WARN("warning: reported file size and actual file size do not match\n"
			"reported = %d\nactual = %d\n", riff_size, size
		);
	}

	if(memcmp(&data[pos], "MDS0", 4))
		FAIL("Not an MDS0 format file\n");

	pos+=4;

	while(pos < riff_size)
	{
		// chunks must be word aligned
		if(pos & 1)
			pos++;

		chunk = *(uint32_t*)(data+pos);
		chunk_size = *(uint32_t*)(data+pos+4);

		DEBUG("Chunk id: %c%c%c%c (%08x), size %d bytes\n",
			data[pos],data[pos+1],data[pos+2],data[pos+3],chunk,chunk_size);

		switch(chunk)
		{
		case 0x20716573: // 'seq '
			seq_chunk = data + pos + 8;
			seq_size = chunk_size;
			break;
		case 0x5453494c: // 'LIST'
			read_list_chunk(data + pos);
			break;
		default: // ignore all other chunks
			break;
		}
		pos += chunk_size+8;
	}
	return 0;
}

//=====================================================================

uint16_t read_be16(uint8_t* a)
{
	return (a[0] << 8) | a[1];
}

//=====================================================================

void analyze_track(struct header_t* header, uint8_t* data, uint32_t max_size, uint16_t track_id)
{
	DEBUG("analyzing track %03x, pos = %04x, max length = %04x\n", track_id, data - mds_data, max_size);
	uint32_t pos = 0;
	header->data = data;
	while(pos < max_size)
	{
		uint8_t cmd = data[pos++];
		if(cmd <= note_end)
		{
			if(cmd >= note_start && track_id & 0x200)
			{
				uint8_t id = cmd - note_start;
				uint32_t addr = song.tbase + read_be16(seq_chunk + song.tbase + id * 2);
				if(song.max_data <= id)
					song.max_data = id + 1;
				if(song.data[id].type != (DATA_DRUM|DATA_PARSED))
				{
					song.data[id].type = (DATA_DRUM|DATA_PARSED);
					song.data[id].channel_id = header->channel_id;
					song.data[id].channel_flag = header->channel_flag;
					analyze_track(&song.data[id], seq_chunk + addr, seq_size - addr, id + 0x400);
				}
			}
			continue;
		}
		else if(cmd >= chn_cmd_base(0) && (cmd - chn_cmd_base(0) < chn_cmd_cnt))
		{
			const struct cmd_def* def = &chn_cmd[cmd - chn_cmd_base(0)];
			if(def->flag & FLG_INS_ID)
			{
				uint8_t id = data[pos];
				if(song.max_data <= id)
					song.max_data = id + 1;
				song.data[id].type = (header->channel_id < 6) ? DATA_FM : DATA_PSG;
			}
			else if(def->flag & FLG_PEG_ID)
			{
				uint8_t id = data[pos];
				if(id--)
				{
					if(song.max_data <= id)
						song.max_data = id + 1;
					song.data[id].type = DATA_PEG;
				}
			}
			else if(def->flag & FLG_PCM_ID)
			{
				uint8_t id = data[pos];
				if(song.max_data <= id)
					song.max_data = id + 1;
				song.data[id].type = DATA_PCM;
			}
			else if(def->flag & FLG_FLG_ID)
			{
				if(data[pos] == 0x08) // drum mode on
					track_id |= 0x200;
				else if(data[pos] == 0x00) // drum mode off
					track_id &= ~0x200;
			}
			if(def->length < 0)
				pos -= def->length;
			else
				pos += def->length;
		}
		else if(cmd >= sys_cmd_base(0) && (cmd - sys_cmd_base(0) < sys_cmd_cnt))
		{
			const struct cmd_def* def = &sys_cmd[cmd - sys_cmd_base(0)];
			if(def->length == -2 && def->flag & FLG_FINISH)
			{
				int16_t offset = read_be16(data + pos);
				header->loop = pos + offset + 3;
			}
			else if(def->flag & FLG_PAT_ID)
			{
				uint8_t id = data[pos];
				uint32_t addr = song.tbase + read_be16(seq_chunk + song.tbase + id * 2);
				if(song.max_data <= id)
					song.max_data = id + 1;
				uint16_t type = DATA_SUB | DATA_PARSED | (track_id & 0x200 ? DATA_IN_DRUM_MODE : 0);
				if(song.data[id].type != type)
				{
					song.data[id].type = type;
					song.data[id].channel_id = header->channel_id;
					song.data[id].channel_flag = header->channel_flag;
					analyze_track(&song.data[id], seq_chunk + addr, seq_size - addr, id + 0x100);
				}
			}
			if(def->length < 0)
				pos -= def->length;
			else
				pos += def->length;
			if(def->flag & FLG_FINISH)
			{
				header->length = pos;
				break;
			}
		}
		else
		{
			FAIL("unknown command %02x at 0x%08x", cmd, (data - mds_data) + pos);
		}
	}
	DEBUG("analyzed track  %03x, length = %04x, loop = %04x\n", track_id, header->length, header->loop);
}

//=====================================================================

int extract_song()
{
	uint32_t i = 0;
	uint32_t pos = 0;

	if(!seq_chunk)
		FAIL("no seq chunk\n");
	if(seq_size < 4)
		FAIL("seq chunk too small");

	song.tbase = read_be16(seq_chunk + 0);
	song.tvol = seq_chunk[2];
	song.tcount = seq_chunk[3];
	song.max_data = 0;
	pos += 4;

	i = 0;
	while(pos < seq_size)
	{
		uint32_t addr;
		if(i == song.tcount)
			break;

		song.tracks[i].channel_id = seq_chunk[pos + 0];
		song.tracks[i].channel_flag = seq_chunk[pos + 1];
		addr = song.tbase + read_be16(seq_chunk + pos + 2);
		analyze_track(&song.tracks[i], seq_chunk + addr, seq_size - addr, i);

		pos += 4;
		i++;
	}

	for(i = 0; i < song.max_data; i++)
	{
		if(!(song.data[i].type & DATA_LOCAL) && song.data[i].length)
		{
			song.data[i].hash = fnv1a32(song.data[i].data, song.data[i].length);
			song.data[i].type |= DATA_PARSED;
		}
	}

	return 0;
}

//=====================================================================

#define EMIT(msg,...) do {fprintf(stdout, msg, ##__VA_ARGS__); } while(0)
void emit_track(struct header_t* header, uint16_t track_id)
{
	const uint32_t max_events_per_line = 8;
	uint32_t event_count = 0;

	uint32_t pos = 0, max_size = header->length;
	uint8_t* data = header->data;

	uint8_t last_cmd = 0x00;

	static uint32_t lpb_pos[5];
	static uint32_t lpb_id[5];
	uint32_t lpb_stack = 0;
	uint32_t lpb_count = 0;

	while(pos < max_size)
	{
		uint8_t cmd = data[pos++];

		if(lpb_stack && pos == lpb_pos[lpb_stack-1])
		{
			event_count = 0;
			EMIT("\n%sB%d",header->label, lpb_id[lpb_stack-1]);
			lpb_stack--;
		}
		if(pos == header->loop)
		{
			event_count = 0;
			EMIT("\n%sL",header->label);
		}

		if(event_count++ == 0 || cmd >= sys_cmd_base(0))
			EMIT("\n\tdc.b\t");
		else if(last_cmd <= note_end)
			EMIT(", ");
		else
			EMIT(",");
		last_cmd = cmd;
		event_count %= max_events_per_line;

		if(cmd <= note_end)
		{
			if(cmd >= note_start && track_id & 0x200)
			{
				uint8_t id = cmd - note_start;
				EMIT("%s",song.data[id].alias);
			}
			else
			{
				EMIT("%s",cmd_tab[cmd]);
			}
			// extra length parameter
			if(cmd >= rest_cmd && data[pos] < 0x80)
				EMIT(",%s",cmd_tab[data[pos++]]);
		}
		else
		{
			EMIT("%s",cmd_tab[cmd]);
			if(cmd >= chn_cmd_base(0) && (cmd - chn_cmd_base(0) < chn_cmd_cnt))
			{
				const struct cmd_def* def = &chn_cmd[cmd - chn_cmd_base(0)];
				if(def->flag & FLG_PEG_ID)
				{
					uint8_t id = data[pos++];
					if(id--)
						EMIT(",%s",song.data[id].alias);
					else
						EMIT(",0");
				}
				else if(def->flag & FLG_INS_ID || def->flag & FLG_PCM_ID)
				{
					uint8_t id = data[pos++];
					EMIT(",%s",song.data[id].alias);
				}
				else if(def->flag & FLG_FLG_ID)
				{
					if(data[pos] == 0x08) // drum mode on
						track_id |= 0x200;
					else if(data[pos] == 0x00) // drum mode off
						track_id &= ~0x200;
					EMIT(",$%02x",data[pos++]);
				}
				else if(def->length > 0)
				{
					int i;
					for(i = 0; i < def->length; i++)
					{
						EMIT(",$%02x",data[pos++]);
					}
				}
				else if(def->length < 0)
				{
					FAIL("unhandled channel parameter type");
				}
			}
			else if(cmd >= sys_cmd_base(0) && (cmd - sys_cmd_base(0) < sys_cmd_cnt))
			{
				const struct cmd_def* def = &sys_cmd[cmd - sys_cmd_base(0)];
				if(def->flag & FLG_FINISH && def->length == -2)
				{
					EMIT("\n\tdc.w\t%sL-*-2", header->label);
					break;
				}
				else if(def->flag & FLG_PAT_ID)
				{
					uint8_t id = data[pos++];
					EMIT(",%s",song.data[id].alias);
				}
				else if(def->flag & FLG_LPB && def->length == -1)
				{
					lpb_pos[lpb_stack] = (pos + 1) + data[pos++];
					lpb_id[lpb_stack] = lpb_count++;
					EMIT(",%sB%d-*-2", header->label, lpb_id[lpb_stack++]);
				}
				else if(def->flag & FLG_LPB && def->length == -2)
				{
					lpb_pos[lpb_stack] = (pos + 2) + ((data[pos] << 8) | data[pos+1]);
					lpb_id[lpb_stack] = lpb_count++;
					EMIT("\n\tdc.w\t%sB%d-*-2", header->label, lpb_id[lpb_stack++]); //untested
					pos += 2;
				}
				else if(def->length > 0)
				{
					int i;
					for(i = 0; i < def->length; i++)
					{
						EMIT(",$%02x",data[pos++]);
					}
				}
				else if(def->length < 0)
				{
					FAIL("unhandled system parameter type");
				}

				if(def->flag & FLG_FINISH)
				{
					break;
				}

				event_count = 0;
			}
			else
			{
				FAIL("unknown command %02x at 0x%08x", cmd, (data - mds_data) + pos);
			}
		}
	}
}

void emit_fm(struct header_t* header)
{
	const char* const comments[8] = {
		" ;dt/mul",
		" ;ks/ar",
		" ;am/dr",
		" ;sr",
		" ;sl/rr",
		" ;ssg-eg",
		" ;tl",
		"         ;fb/alg, transpose",
	};
	uint32_t comment_id = 0;

	const uint32_t max_events_per_line = 4;
	uint32_t event_count = 0;

	uint32_t pos = 0, max_size = header->length;
	uint8_t* data = header->data;

	while(pos < max_size)
	{
		uint8_t cmd = data[pos++];

		if(event_count++ == 0)
			EMIT("\n\tdc.b\t");
		else
			EMIT(",");

		EMIT("$%02x", cmd);

		if(event_count == max_events_per_line)
			EMIT("%s", comments[comment_id++]);

		event_count %= max_events_per_line;
	}
	EMIT("%s", comments[(comment_id++) % 8]);
}

void emit_psg(struct header_t* header)
{
	const uint32_t max_events_per_line = 8;
	uint32_t event_count = 0;

	uint32_t pos = 0, max_size = header->length;
	uint8_t* data = header->data;

	uint32_t loop_pos = 0xffffffff;
	if(max_size > 2 && data[max_size - 2] == 0x02)
	{
		loop_pos = data[max_size - 1] + 1;
		EMIT("\n@start");
	}

	while(pos < max_size)
	{
		uint8_t cmd = data[pos++];

		if(pos == loop_pos)
		{
			event_count = 0;
			EMIT("\n@loop");
		}

		if(event_count++ == 0 || cmd < 0x10)
			EMIT("\n\tdc.b\t");
		else
			EMIT(",");

		EMIT("$%02x", cmd);

		if(cmd == 0x00)
		{
			EMIT(" ;stop envelope");
			break;
		}
		else if(cmd == 0x01)
		{
			event_count = 0;
			EMIT(" ;sustain");
		}
		else if(cmd == 0x02)
		{
			EMIT(",@loop-@start ;loop");
			break;
		}

		event_count %= max_events_per_line;
	}
}

void emit_peg(struct header_t* header)
{
	const uint32_t max_events_per_line = 2;
	uint32_t event_count = 0;

	uint32_t pos = 0, max_size = header->length;
	uint8_t* data = header->data;

	uint32_t loop_pos = 0xffffffff;
	if(max_size > 2 && data[max_size - 2] == 0x7f)
	{
		loop_pos = (data[max_size - 1] * 4) + 2;
		EMIT("\n@start");
	}

	while(pos < max_size)
	{
		uint16_t cmd = (data[pos] << 8) | (data[pos + 1]);
		pos += 2;

		if(pos == loop_pos)
		{
			event_count = 0;
			EMIT("\n@loop");
		}

		if(event_count++ == 0)
			EMIT("\n\tdc.w\t");
		else
			EMIT(",");

		if(event_count == 1 && ((cmd & 0xff00) == 0x7f00))
		{
			EMIT("$7f00+((@loop-@start)/4) ; loop");
			break;
		}
		else
		{
			EMIT("$%04x", cmd);
			if(event_count == 2)
			{
				int16_t initial = (data[pos-4] << 8) | data[pos-3];
				int8_t delta = data[pos-2];
				uint8_t wait = data[pos-1];
				EMIT(" ; from %6d add %3d", initial, delta);
				if(wait == 0xff)
					EMIT(" forever");
				else
					EMIT(" for %3d frames", wait + 1);
			}
		}

		event_count %= max_events_per_line;
	}
}
//=====================================================================

int emit_song()
{
	int i;
	static int data_type_count[DATA_TYPE_MASK + 1];
	for(i = 0; i < DATA_TYPE_MASK + 1; i++)
	{
		data_type_count[i] = 0;
	}

	EMIT(";========================================================================\n");
	EMIT("; MDSDRV sequence data file: '%s'\n", song.filename);
	EMIT("; This file is autogenerated by 'mds2asm.c'\n");
	EMIT(";========================================================================\n");
	EMIT("@TTAB\n");
	EMIT("\tdc.w\t@BASE-@TTAB\t\t;offset to song base\n");
	EMIT("\tdc.w\t$%02x%02x\t\t;track volume(hi) track count(lo)\n", song.tvol, song.tcount);
	for(i = 0; i < song.tcount; i++)
	{
		sprintf(song.tracks[i].label, "@T%c", song.tracks[i].channel_id + 'A');
		EMIT("\tdc.w\t$%02x%02x,%s-@BASE\t;channel select(hi) flags(lo), position\n",
				song.tracks[i].channel_id, song.tracks[i].channel_flag, song.tracks[i].label);
	}
	EMIT("\n");
	EMIT(";========================================================================\n");
	EMIT("@BASE\n");
	EMIT("\trsreset\n");
	for(i = 0; i < song.max_data; i++)
	{
		uint32_t type_index = data_type_count[song.data[i].type & DATA_TYPE_MASK]++;
		uint32_t rsoffset = 0;

		switch(song.data[i].type & DATA_TYPE_MASK)
		{
			case DATA_SUB:
				sprintf(song.data[i].alias, "@PAT_%d", type_index);
				sprintf(song.data[i].label, "@P%d", type_index);
				break;
			case DATA_DRUM:
				rsoffset = note_start;
				sprintf(song.data[i].alias, "@DRM_%d", type_index);
				sprintf(song.data[i].label, "@D%d", type_index);
				break;
			case DATA_FM:
				sprintf(song.data[i].alias, "@FM_%d", type_index);
				sprintf(song.data[i].label, "sd_fm_%08x", song.data[i].hash);
				break;
			case DATA_PSG:
				sprintf(song.data[i].alias, "@PSG_%d", type_index);
				sprintf(song.data[i].label, "sd_psg_%08x", song.data[i].hash);
				break;
			case DATA_PCM:
				sprintf(song.data[i].alias, "@PCM_%d", type_index);
				sprintf(song.data[i].label, "sd_pcm_%08x", song.data[i].hash);
				break;
			case DATA_PEG:
				rsoffset = 1;
				sprintf(song.data[i].alias, "@PEG_%d", type_index);
				sprintf(song.data[i].label, "sd_peg_%08x", song.data[i].hash);
				break;
			default:
				sprintf(song.data[i].alias, "");
				sprintf(song.data[i].label, "");
				break;
		}

		if(strlen(song.data[i].alias))
			EMIT("%s = __RS+%d\n", song.data[i].alias, rsoffset);
		if(song.data[i].type & DATA_PARSED)
		{
			if(song.data[i].type & DATA_LOCAL)
				EMIT("\tdc.w\t%s-@BASE\n", song.data[i].label);
			else
				EMIT("\tdc.w\t%s-sdtop\n", song.data[i].label);
		}
		else
		{
			EMIT("\tdc.w\t$0000 ; unparsed data type %04x\n", song.data[i].type);
		}
		EMIT("\trs.b\t1\n", song.data[i].alias);
	}
	for(i = 0; i < song.tcount; i++)
	{
		EMIT(";========================================================================\n");
		EMIT("\n%s", song.tracks[i].label);
		emit_track(&song.tracks[i], (song.tracks[i].type & DATA_IN_DRUM_MODE) ? 0x200 : 0);
		EMIT("\n\n");
	}
	for(i = 0; i < song.max_data; i++)
	{
		if(song.data[i].type & DATA_LOCAL)
		{
			EMIT(";========================================================================\n");
			EMIT("\n%s", song.data[i].label);
			emit_track(&song.data[i], (song.data[i].type & DATA_IN_DRUM_MODE) ? 0x200 : 0);
			EMIT("\n\n");
		}
	}
	EMIT(";========================================================================\n");
	for(i = 0; i < song.max_data; i++)
	{
		if(!(song.data[i].type & DATA_LOCAL))
		{
			EMIT("\n\tif ~def(%s)", song.data[i].label);
			EMIT("\n%s", song.data[i].label);
			switch(song.data[i].type & DATA_TYPE_MASK)
			{
				case DATA_FM:
					emit_fm(&song.data[i]);
					break;
				case DATA_PSG:
					emit_psg(&song.data[i]);
					break;
				case DATA_PCM:
					break;
				case DATA_PEG:
					emit_peg(&song.data[i]);
					break;
				default:
					break;
			}
			EMIT("\n\teven\n\tendif\n");
		}
	}
	return 0;
}

