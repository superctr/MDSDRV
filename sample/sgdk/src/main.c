#include <genesis.h>
#include "menu.h"
#include "mdsdrv.h"

u16 cursor;

#define ITEM_BGM 0
#define ITEM_VOLUME 1
#define ITEM_TEMPO 2
#define ITEM_BGMVOL 3
#define ITEM_SEVOL 4
#define ITEM_GTEMPO 5
#define ITEM_FTARGET 6
#define ITEM_FSPEED 7
#define ITEM_SE1 8
#define ITEM_SE2 9
#define ITEM_SE3 10
#define ITEM_MAX 10

/* Temp text buffer */
static char buf[40];

/*
 *  MDSDRV must always run in vblank.
 */
void vbl_callback()
{
	MDS_update();
}

static const char* instruction_text[] = {
//   0123456789012345678901234567890123456789
	"     A: Play, B: Stop, C: Fade out      ",
	" A: Play SE1, B: Play SE2, C: Play SE3  ",
	"        A: Fade out, B: Fade in         "
};

void draw_instructions()
{
	const char* str = instruction_text[0];

	if(menu_cursor >= ITEM_SE1)
		str = instruction_text[1];
	else if(menu_cursor >= ITEM_FTARGET)
		str = instruction_text[2];

	VDP_setTextPalette(0);
	VDP_drawText(str, 0, 26);
}

void draw_status(u16 x, u16 y)
{
	sprintf(buf, "%04x %04x %04x %04x",
		MDS_command(MDS_CMD_GET_STATUS, 0),
		MDS_command(MDS_CMD_GET_STATUS, 1),
		MDS_command(MDS_CMD_GET_STATUS, 2),
		MDS_command(MDS_CMD_GET_STATUS, 3));
	VDP_setTextPalette(0);
	VDP_drawText(buf, x, y);
}

int main(u16 hard)
{
	JOY_init();
	VDP_drawText("Initializing MDSDRV ...", 2, 2);

	/*
	 *  Initialize MDSDRV (done once)
	 */
	MDS_init(mdsseqdat, mdspcmdat);

	/*
	 *  run MDSDRV in callback
	 */
	SYS_setVIntCallback(vbl_callback);

	/*
	 *  Draw menu
	 */
	VDP_clearPlane(0, FALSE);
	menu_add_item(ITEM_BGM, "BGM", 1, 0, MDS_command(MDS_CMD_GET_SOUND_CNT, 0));
	menu_add_item(ITEM_VOLUME, "BGM volume", 0, 0, 127);
	menu_add_item(ITEM_TEMPO, "BGM tempo", 0, 0, 511);
//	menu_add_item(ITEM_GVOLUME, "Global volume", 0, 127);
	menu_add_item(ITEM_BGMVOL, "Initial BGM vol", 0, 0, 127);
	menu_add_item(ITEM_SEVOL, "Initial SE vol", 0, 0, 127);
	menu_add_item(ITEM_GTEMPO, "Global tempo", 16, 16, 511); //lower calues cause higher CPU load
	menu_add_item(ITEM_FTARGET, "Fade target", 20, 0, 127);
	menu_add_item(ITEM_FSPEED, "Fade speed", 5, 0, 7);
	menu_add_item(ITEM_SE1, "SE1", SE_BEEP3, SE_MIN, SE_MAX);
	menu_add_item(ITEM_SE2, "SE2", SE_NOISE1, SE_MIN, SE_MAX);
	menu_add_item(ITEM_SE3, "SE3", SE_EXPLOSION2, SE_MIN, SE_MAX);

	menu_init(ITEM_MAX);
	draw_instructions();

	VDP_drawText("MDSDRV SGDK Test Program", 2, 2);
	VDP_drawText("Version", 2, 22);
	VDP_drawText(MDS_get_version_str(), 10, 22);
	VDP_drawText("Status", 2, 24);

	while(TRUE)
	{
		u16 action;
		menu_val[ITEM_VOLUME] = MDS_command(MDS_CMD_GET_VOLUME, MDS_BGM);
		menu_val[ITEM_TEMPO] = MDS_command(MDS_CMD_GET_TEMPO, MDS_BGM);
		menu_val[ITEM_GTEMPO] = MDS_command(MDS_CMD_GET_GTEMPO, 0);
#if 0
		// These values are currently not updated by the driver itself, so
		// there is normally no need to fetch the values in this application.
		action = MDS_command(MDS_CMD_GET_GVOLUME, 0);
		menu_val[ITEM_BGMVOL] = action >> 8;
		menu_val[ITEM_SEVOL] = action & 0xff;
#endif

		action = menu_update();

		switch(action)
		{
			default:
				break;
			case MENU_ACTION_UD: // Update cursor
				draw_instructions();
				break;
			case MENU_ACTION_LR: // Update value
				switch(menu_cursor)
				{
					case ITEM_VOLUME:
						MDS_command2(MDS_CMD_SET_VOLUME, MDS_BGM, menu_val[ITEM_VOLUME]);
						break;
					case ITEM_TEMPO:
						MDS_command2(MDS_CMD_SET_TEMPO, MDS_BGM, menu_val[ITEM_TEMPO]);
						break;
					case ITEM_GTEMPO:
						MDS_command(MDS_CMD_SET_GTEMPO, menu_val[ITEM_GTEMPO]);
						break;
					case ITEM_BGMVOL:
					case ITEM_SEVOL:
						MDS_command(MDS_CMD_SET_GVOLUME, (menu_val[ITEM_BGMVOL]<<8)|(menu_val[ITEM_SEVOL] & 0xff));
					default:
						break;
				}
				break;
			case MENU_ACTION_A: // Play music
				if(menu_cursor < ITEM_FTARGET)
					MDS_request(MDS_BGM, menu_val[ITEM_BGM]);
				else if(menu_cursor < ITEM_SE1)
					MDS_fade(menu_val[ITEM_FTARGET], menu_val[ITEM_FSPEED], FALSE);
				else
					MDS_request(MDS_SE1, menu_val[ITEM_SE1]);
				break;
			case MENU_ACTION_B: // Stop music
				if(menu_cursor < ITEM_FTARGET)
					MDS_request(MDS_BGM, 0);
				else if(menu_cursor < ITEM_SE1)
					MDS_fade(0x00, menu_val[ITEM_FSPEED], FALSE);
				else
					MDS_request(MDS_SE2, menu_val[ITEM_SE2]);
				break;
			case MENU_ACTION_C: // Fade out music
				if(menu_cursor < ITEM_FTARGET)
					MDS_fade(0x50, 3, TRUE);
				else if(menu_cursor >= ITEM_SE1)
					MDS_request(MDS_SE3, menu_val[ITEM_SE3]);
				break;
		}
		draw_status(10, 24);
		VDP_waitVSync();
	}
	return 0;
}
