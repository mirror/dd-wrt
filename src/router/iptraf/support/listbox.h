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
