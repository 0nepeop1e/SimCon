#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <Windows.h>
#include <wincon.h>

#include "simconio.h"
#include "simconmenu.h"

#define BORDER_LEFT (COMMON_LVB_GRID_LVERTICAL | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)
#define BORDER_TOP (COMMON_LVB_GRID_HORIZONTAL | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)
#define BORDER_RIGHT (COMMON_LVB_GRID_RVERTICAL | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)
#define BORDER_BOTTOM (COMMON_LVB_UNDERSCORE | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)
#define BORDER_TOPLEFT (COMMON_LVB_GRID_LVERTICAL | COMMON_LVB_GRID_HORIZONTAL | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)
#define BORDER_TOPRIGHT (COMMON_LVB_GRID_RVERTICAL | COMMON_LVB_GRID_HORIZONTAL | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)
#define BORDER_BOTTOMLEFT (COMMON_LVB_GRID_LVERTICAL | COMMON_LVB_UNDERSCORE | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)
#define BORDER_BOTTOMRIGHT (COMMON_LVB_GRID_RVERTICAL | COMMON_LVB_UNDERSCORE | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)
#define CONOUT GetStdHandle(STD_OUTPUT_HANDLE)

void menu_draw(SCI_MENU*, unsigned int selected);
void menu_draw_item(SCI_MENU*, unsigned int selected);
void menu_handle(SCI_MENU*);

COORD GetConsoleCursorPosition();

SCI_MENU *scene;
bool started = false;

SIMCON SCI_MENU menu_create(const char title[]){
	SCI_MENU menu;
	strcpy_s(menu.title, 31, title);
	menu.pre = NULL;
	menu.size = 0;
	return menu;
}

SIMCON void menu_start(SCI_MENU *menu){
	CONSOLE_CURSOR_INFO origci, newci = { .bVisible = false, .dwSize = 25 };
	GetConsoleCursorInfo(CONOUT, &origci);
	SetConsoleCursorInfo(CONOUT, &newci);
	if (started) return;
	started = true;
	scene = menu;
	SCI_MENU *cache = menu;
	while (scene != NULL){
		menu_draw(scene, 0);
		cache = scene;
		if (scene->pre != NULL){
			newci.bVisible = true;
			SetConsoleCursorInfo(CONOUT, &newci);
			(*scene->pre)();
			newci.bVisible = false;
			SetConsoleCursorInfo(CONOUT, &newci);
		}
		if (scene == cache && scene->size > 0) menu_handle(scene);
	}
	started = false;
	SetConsoleCursorInfo(CONOUT, &origci);
}

SIMCON void menu_switch(SCI_MENU *menu){
	scene = menu;
}

void menu_draw(SCI_MENU *menu, unsigned int selected){
	SetConsoleTextAttribute(CONOUT, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
	system("cls");
	COORD cur = { 1, 1 };
	SetConsoleCursorPosition(CONOUT, cur);
	SetConsoleTextAttribute(CONOUT, BORDER_TOPLEFT);
	_putch(' ');
	SetConsoleTextAttribute(CONOUT, BORDER_TOP);
	printf("%-30s", "");
	SetConsoleTextAttribute(CONOUT, BORDER_TOPRIGHT);
	_putch(' ');
	cur.Y += 1;
	SetConsoleCursorPosition(CONOUT, cur);
	SetConsoleTextAttribute(CONOUT, BORDER_LEFT);
	_putch(' ');
	SetConsoleTextAttribute(CONOUT, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
	printf("%s", menu->title);
	cur.X = 32;
	SetConsoleCursorPosition(CONOUT, cur);
	SetConsoleTextAttribute(CONOUT, BORDER_RIGHT);
	_putch(' ');
	cur.X = 1; cur.Y += 1;
	SetConsoleCursorPosition(CONOUT, cur);
	if (menu->size > 0)menu_draw_item(menu, selected);
	cur.Y += (menu->size == 0) ? 0 : menu->size + 1;
	SetConsoleCursorPosition(CONOUT, cur);
	SetConsoleTextAttribute(CONOUT, BORDER_BOTTOMLEFT);
	_putch(' ');
	SetConsoleTextAttribute(CONOUT, BORDER_BOTTOM);
	printf("%-30s", "");
	SetConsoleTextAttribute(CONOUT, BORDER_BOTTOMRIGHT);
	_putch(' ');
	cur.Y += 2;
	cur.X = 0;
	SetConsoleTextAttribute(CONOUT, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
	SetConsoleCursorPosition(CONOUT, cur);
}

void menu_draw_item(SCI_MENU *menu, unsigned int selected){
	COORD cur = { 1, 3 };
	SetConsoleCursorPosition(CONOUT, cur);
	SetConsoleTextAttribute(CONOUT, BORDER_LEFT);
	_putch(' ');
	SetConsoleTextAttribute(CONOUT, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
	printf("%30s", "");
	cur.X = 32;
	SetConsoleCursorPosition(CONOUT, cur);
	SetConsoleTextAttribute(CONOUT, BORDER_RIGHT);
	_putch(' ');
	cur.X = 1;
	cur.Y += 1;
	for (unsigned int i = 0; i < menu->size; i++){
		SetConsoleCursorPosition(CONOUT, cur);
		SetConsoleTextAttribute(CONOUT, BORDER_LEFT);
		_putch(' ');
		if (i == selected) SetConsoleTextAttribute(CONOUT, BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED);
		else SetConsoleTextAttribute(CONOUT, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
		printf("%-30s", menu->items[i].name);
		SetConsoleTextAttribute(CONOUT, BORDER_RIGHT);
		_putch(' ');
		cur.Y++;
	}
	SetConsoleCursorPosition(CONOUT, cur);
}

void menu_handle(SCI_MENU *menu){
	menu_draw(scene, 0);
	CONSOLE_CURSOR_INFO ci = { .bVisible = true, .dwSize = 25 };
	SetConsoleCursorInfo(CONOUT, &ci);
	int index = 0;
	CHS ch;
	while (true){
		ch = getch_s();
		if (ch.f == 0xE0 || ch.f == 0){
			if (ch.s == 0x48 || ch.s == 0x50){
				ci.bVisible = false;
				SetConsoleCursorInfo(CONOUT, &ci);
				CONSOLE_SCREEN_BUFFER_INFO binfo;
				GetConsoleScreenBufferInfo(CONOUT, &binfo);
				index += menu->size + (ch.s == 0x50 ? 1 : -1);
				index %= menu->size;
				menu_draw_item(menu, index);
				SetConsoleCursorPosition(CONOUT, binfo.dwCursorPosition);
				ci.bVisible = true;
				SetConsoleTextAttribute(CONOUT, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
				SetConsoleCursorInfo(CONOUT, &ci);
			}
		}
		else if (ch.f == 0x0D || ch.f == 0x20){
			if (!(*menu->items[index].callback)(index)) return;
			menu_draw(menu, index);
		}
	}
}

SIMCON bool menu_add_item(SCI_MENU *menu, SCI_ITEM item){
	if (menu->size == MENU_MAX_ITEM) return false;
	memmove(&menu->items[menu->size], &item, sizeof(item));
	menu->size += 1;
	return true;
}