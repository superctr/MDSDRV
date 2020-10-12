#include <genesis.h>
#include "menu.h"

u16 menu_pad;
u16 menu_pad_last;
u16 menu_pad_press;
u16 menu_pad_hold;

u16 menu_cursor;
u16 menu_cursor_max;

s16 menu_val[MENU_MAX_ITEM+1];
s16 menu_max[MENU_MAX_ITEM+1];
s16 menu_min[MENU_MAX_ITEM+1];
const char* menu_str[MENU_MAX_ITEM+1];

char menu_strbuf[20];

void menu_init(u16 max_cursor)
{
	menu_pad = 0;
	menu_pad_last = 0;
	menu_pad_press = 0;

	menu_cursor = 0;
	menu_cursor_max = max_cursor;

	menu_draw();
}

void menu_add_item(u16 item_id, const char* str, s16 val, s16 min, s16 max)
{
	menu_val[item_id] = val;
	menu_min[item_id] = min;
	menu_max[item_id] = max;
	menu_str[item_id] = str;
}

void menu_draw()
{
	u16 i = menu_cursor_max+1;

	DMA_waitCompletion();

	while(i--)
	{
		VDP_setTextPalette((i == menu_cursor) ? 1 : 0);
		sprintf(menu_strbuf, "%04x [%04x,%04x]", menu_val[i], menu_min[i], menu_max[i]);
		VDP_drawText(menu_str[i], 4, 4+i);
		VDP_drawText(menu_strbuf, 20, 4+i);
	}
}

u16 menu_update()
{
	u16 action = 0;

	menu_pad_last = menu_pad;
	menu_pad = JOY_readJoypad(JOY_1) & 0x00FF;
	menu_pad_press = (menu_pad ^ menu_pad_last) & menu_pad;

	menu_pad_hold = (menu_pad == menu_pad_last) ? menu_pad_hold + 1 : 0;
	if(menu_pad_hold > 15)
	{
		menu_pad_press |= menu_pad;
		menu_pad_hold -= 2;
	}

	if(menu_pad_press & BUTTON_UP)
	{
		if(menu_cursor > 0)
			menu_cursor--;
		else
			menu_cursor = menu_cursor_max;
		action = MENU_ACTION_UD;
	}
	else if(menu_pad_press & BUTTON_DOWN)
	{
		if(menu_cursor < menu_cursor_max)
			menu_cursor++;
		else
			menu_cursor = 0;
		action = MENU_ACTION_UD;
	}
	else if((menu_pad_press & BUTTON_LEFT) && menu_val[menu_cursor] > menu_min[menu_cursor])
	{
		menu_val[menu_cursor]--;
		action = MENU_ACTION_LR;
	}
	else if((menu_pad_press & BUTTON_RIGHT) && menu_val[menu_cursor] < menu_max[menu_cursor])
	{
		menu_val[menu_cursor]++;
		action = MENU_ACTION_LR;
	}
	else if(menu_pad_press & BUTTON_A)
	{
		action = MENU_ACTION_A;
	}
	else if(menu_pad_press & BUTTON_B)
	{
		action = MENU_ACTION_B;
	}
	else if(menu_pad_press & BUTTON_C)
	{
		action = MENU_ACTION_C;
	}
	else if(menu_pad_press & BUTTON_START)
	{
		action = MENU_ACTION_START;
	}

	if(action >= MENU_ACTION_UD)
		menu_draw();
	return action;
}

void menu_update_value(u16 item_id, u16 value)
{
	menu_val[item_id] = value;
	VDP_setTextPalette((item_id == menu_cursor) ? 1 : 0);
	sprintf(menu_strbuf, "%04x", value);
	VDP_drawText(menu_strbuf, 20, 4+item_id);
}
