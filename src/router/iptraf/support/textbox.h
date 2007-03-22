
/***

input.h - structure declarations and function prototypes for input.c
Written by Gerard Paul Java
Copyright (c) Gerard Paul Java 1997

***/

#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include "winops.h"

#define CTRL_X 24

struct FIELD {
    char *buf;
    unsigned int len;
    unsigned int tlen;
    unsigned int xpos;
    unsigned int ypos;
    struct FIELD *prevfield;
    struct FIELD *nextfield;
};

struct FIELDLIST {
    struct FIELD *list;
    WINDOW *fieldwin;
    PANEL *fieldpanel;
    int dlgtextattr;
    int fieldattr;
};

void tx_initfields(struct FIELDLIST *list, int leny, int lenx, int begy, int begx,
              int dlgtextattr, int dlgfieldattr);
void tx_addfield(struct FIELDLIST *list, unsigned int len, unsigned int y,
	      unsigned int x, char *initstr);
void tx_getinput(struct FIELDLIST *list, struct FIELD *field, int *exitkey);
void tx_fillfields(struct FIELDLIST *list, int *aborted);
void tx_destroyfields(struct FIELDLIST *list);
#include <curses.h>

void tx_printkeyhelp(char *keytext, char *desc, WINDOW * win,
                  int highattr, int textattr);
void tx_menukeyhelp(int textattr, int highattr);
void tx_listkeyhelp(int textattr, int highattr);
#include <curses.h>

#define MAX_TEXT_LENGTH 240

struct textlisttype {
    char text[MAX_TEXT_LENGTH];
    int cellwidth[10];          /* up to 10 cells per line */
    char *nodeptr;		/* generic pointer, cast to appropriate type */
    struct textlisttype *next_entry;
    struct textlisttype *prev_entry;
};

struct scroll_list {
    char *mainlist;             /* generic pointer, cast to appropriate type */
    char *mlistptr;		/* generic pointer, cast to appropriate type */
    struct textlisttype *textlist;         /* list of raw text entries */
    struct textlisttype *texttail;
    struct textlisttype *textptr;
    int height;
    int width;
    int mainattr;
    int selectattr;
    int keyattr;
    char *exitkeys;
        
    WINDOW *win;
    PANEL *panel;
    WINDOW *borderwin;
    PANEL *borderpanel;
    
};
void tx_init_listbox(struct scroll_list *list, int width, int height,
                      int startx, int starty,
                      int mainattr, int borderattr, int selectattr,
                      int keyattr);
void tx_set_listbox_title(struct scroll_list *list, char *text, int x);
void tx_add_list_entry(struct scroll_list *list, char *node, char *text);
void tx_show_listbox(struct scroll_list *list);
void tx_operate_listbox(struct scroll_list *list,
                        int *keystroke,
                        int *aborted);
void tx_hide_listbox(struct scroll_list *list);
void tx_unhide_listbox(struct scroll_list *list);
void tx_close_listbox(struct scroll_list *list);
void tx_destroy_list(struct scroll_list *list);
#define tx_destroy_listbox tx_destroy_list
/***
   menu.h - declaration file for my menu library
   Copyright (c) Gerard Paul R. Java 1997

***/

#define SELECTED 1
#define NOTSELECTED 0

#define SEPARATOR 0
#define REGULARITEM 1

#define OPTIONSTRLEN_MAX 50
#define DESCSTRLEN_MAX 81
#define SHORTCUTSTRLEN_MAX 25

struct ITEM {
    char option[OPTIONSTRLEN_MAX];
    char desc[DESCSTRLEN_MAX];
    unsigned int itemtype;
    struct ITEM *prev;
    struct ITEM *next;
};

struct MENU {
    struct ITEM *itemlist;
    struct ITEM *selecteditem;
    struct ITEM *lastitem;
    int itemcount;
    int postn;
    int x1, y1;
    int x2, y2;
    unsigned int menu_maxx;
    WINDOW *menuwin;
    PANEL *menupanel;
    WINDOW *descwin;
    PANEL *descpanel;
    int borderattr;
    int normalattr;
    int highattr;
    int barnormalattr;
    int barhighattr;
    int descriptionattr;
    char shortcuts[SHORTCUTSTRLEN_MAX];
};

extern void tx_initmenu(struct MENU *menu, int y1, int x1, int y2, int x2,
    int borderattr, int normalattr, int highattr,
    int barnormalattr, int barhighattr, int descattr);
extern void tx_additem(struct MENU *menu, char *item, char *desc);
extern void tx_showitem(struct MENU *menu, struct ITEM *itemptr, int selected);
extern void tx_showmenu(struct MENU *menu);
extern void tx_operatemenu(struct MENU *menu, int *row, int *aborted);
extern void tx_destroymenu(struct MENU *menu);
/***

stdwinset.h - prototype declaration for setting the standard window settings
for IPTraf

***/

#include <curses.h>

void tx_stdwinset(WINDOW * win);
void tx_refresh_screen(void);
void tx_colorwin(WINDOW *win);
void tx_coloreol(void);