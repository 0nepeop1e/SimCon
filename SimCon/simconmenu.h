#ifndef SimConMenu
#define SimConMenu

#ifdef SIMCON_EXPORTS
#define SIMCON __declspec(dllexport) 
#else
#define SIMCON __declspec(dllimport) 
#endif

#define MENU_MAX_ITEM 20

/*
Pointer to a callback function which will be called when
item was selected. This function have a parameter which
is the index of the selected item and return a boolean
which mean the should stay on the same menu or not.
*/
typedef _Bool (*SCI_ITEM_CALLBACK)(unsigned int);

/*
This function will be called after the menu title draw
and before the items draw.
*/
typedef void(*SCI_MENU_PRE)();

typedef struct{
	char name[31];
	SCI_ITEM_CALLBACK callback;
} SCI_ITEM;

typedef struct{
	char title[31];
	SCI_ITEM items[MENU_MAX_ITEM];
	SCI_MENU_PRE pre;
	unsigned int size;
} SCI_MENU;

/*
Start the menu interface. Wont have any effect after menu started.
menu: pointer to the menu.
*/
SIMCON void menu_start(SCI_MENU *menu);

/*
Switch from one menu to another.
menu: pointer to the menu, the menu will end when NULL.
*/
SIMCON void menu_switch(SCI_MENU *menu);

/*
Create an empty menu.
title: title of the menu, maximum 30 character.
*/
SIMCON SCI_MENU menu_create(const char title[]);

/*
Add an item to a menu.
menu: pointer to the menu.
item: the item which to be added.
*/
SIMCON _Bool menu_add_item(SCI_MENU *menu, SCI_ITEM item);

#endif