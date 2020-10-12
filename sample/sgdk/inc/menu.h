#ifndef MENU_H
#define MENU_H

#define MENU_MAX_ITEM 16

#define MENU_ACTION_A  1
#define MENU_ACTION_B  2
#define MENU_ACTION_C  3
#define MENU_ACTION_START 4

#define MENU_ACTION_UD 5
#define MENU_ACTION_LR 6

extern u16 menu_pad;
extern u16 menu_pad_last;
extern u16 menu_pad_press;

extern u16 menu_cursor;
extern u16 menu_cursor_max;

extern s16 menu_val[MENU_MAX_ITEM+1];
extern s16 menu_max[MENU_MAX_ITEM+1];
extern s16 menu_min[MENU_MAX_ITEM+1];
extern const char* menu_str[MENU_MAX_ITEM+1];

extern char menu_strbuf[20];

void menu_init(u16 max_cursor);
void menu_add_item(u16 item_id, const char* str, s16 val, s16 min, s16 max);
void menu_draw();
u16  menu_update();

void menu_update_value(u16 item_id, u16 value);

#endif
