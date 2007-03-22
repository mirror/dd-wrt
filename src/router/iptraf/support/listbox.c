/*
 * listbox.c - scrollable listbox management module
 *
 * Written by Gerard Paul Java
 * Copyright (c) Gerard Paul Java 2001
 */

#include <curses.h>
#include <panel.h>
#include <string.h>
#include <stdlib.h>
#include "winops.h"
#include "labels.h"
#include "listbox.h"
#include "msgboxes.h"

void tx_init_listbox(struct scroll_list *list, int width, int height,
                      int startx, int starty,
                      int mainattr, int borderattr, int selectattr,
                      int keyattr)
{
    bzero(list, sizeof(struct scroll_list));
    list->borderwin = newwin(height, width, starty, startx);
    list->borderpanel = new_panel(list->borderwin);
    wattrset(list->borderwin, borderattr);
    tx_box(list->borderwin, ACS_VLINE, ACS_HLINE);
    
    list->win = newwin(height - 2, width - 2, starty + 1, startx + 1);
    list->panel = new_panel(list->win);
    wattrset(list->win, mainattr);
    tx_colorwin(list->win);
    
    list->mainattr = mainattr;
    list->selectattr = selectattr;
    list->height = height;
    list->width = width;
    list->keyattr = keyattr;
    
    tx_stdwinset(list->win);
    scrollok(list->win, 0);
}

void tx_set_listbox_title(struct scroll_list *list, char *text, int x)
{
    mvwprintw(list->borderwin, 0, x, " %s ", text);
}

void tx_add_list_entry(struct scroll_list *list, char *node, char *text)
{
    struct textlisttype *ptmp;
    
    ptmp = malloc(sizeof(struct textlisttype));
    bzero(ptmp, sizeof(struct textlisttype));
        
    strncpy(ptmp->text, text, MAX_TEXT_LENGTH);
    ptmp->nodeptr = node;
    
    if (list->textlist == NULL) {
        list->textlist = ptmp;
        ptmp->prev_entry = NULL;
    } else {
        list->texttail->next_entry = ptmp;
        ptmp->prev_entry = list->texttail;
    }
    
    list->texttail = ptmp;
    ptmp->next_entry = NULL;
}

void tx_show_listbox(struct scroll_list *list)
{
    int i = 0;
    struct textlisttype *tptr = list->textlist;
    
    while ((i <= list->height - 3) && (tptr != NULL)) {
        mvwprintw(list->win, i, 1, tptr->text);
        tptr = tptr->next_entry;
        i++;
    }
    
    update_panels();
    doupdate();
}

void tx_operate_listbox(struct scroll_list *list,
                        int *keystroke,
                        int *aborted)
{
    int ch;
    int exitloop = 0;
    int row = 0;
    char padding[MAX_TEXT_LENGTH];
    char sp_buf[10];
    
    if (list->textlist == NULL) {
        tx_errbox("No list entries", ANYKEY_MSG, &ch);
        *aborted = 1;
        return;
    }
    
    list->textptr = list->textlist;
     
    tx_listkeyhelp(list->mainattr, list->keyattr);
    update_panels();
    doupdate();
    
    while (!exitloop) {
        snprintf(sp_buf, 9, "%%%dc", list->width - strlen(list->textptr->text) - 3);
        snprintf(padding, MAX_TEXT_LENGTH - 1, sp_buf, ' ');
        wattrset(list->win, list->selectattr);
        mvwprintw(list->win, row, 0, " %s%s", list->textptr->text, padding);
            
        ch = wgetch(list->win);

        wattrset(list->win, list->mainattr);
        mvwprintw(list->win, row, 0, " %s%s", list->textptr->text, padding);
        
        switch(ch) {
        case KEY_UP:
            if (list->textptr == NULL)
                continue;
                
            if (list->textptr->prev_entry != NULL) {
                if (row == 0) {
                    scrollok(list->win, 1);
                    wscrl(list->win, -1);
                    scrollok(list->win, 0);
                } else
                    row--;
                    
                list->textptr = list->textptr->prev_entry;
            }
            break;
        case KEY_DOWN:
            if (list->textptr == NULL)
                continue;
                
            if (list->textptr->next_entry != NULL) {
                if (row == list->height - 3) {
                    scrollok(list->win, 1);
                    wscrl(list->win, 1);
                    scrollok(list->win, 0);
                } else
                    row++;
                
                list->textptr = list->textptr->next_entry;
            }
            break;
        case 13:
            *aborted = 0;
            exitloop = 1;
            break;
        case 27:
        case 'x':
        case 'X':
        case 24:
            *aborted = 1;
            exitloop = 1;
        case 12:
        case 'l':
        case 'L':
            tx_refresh_screen();
            break;
        }
    }
    *keystroke = ch;
}

void tx_hide_listbox(struct scroll_list *list)
{
    hide_panel(list->panel);
    hide_panel(list->borderpanel);
    update_panels();
    doupdate();
}

void tx_unhide_listbox(struct scroll_list *list)
{
    show_panel(list->panel);
    show_panel(list->panel);
    update_panels();
    doupdate();
}

void tx_close_listbox(struct scroll_list *list)
{
    del_panel(list->panel);
    del_panel(list->borderpanel);
    delwin(list->win);
    delwin(list->borderwin);
    
    update_panels();
    doupdate();
}

void tx_destroy_list(struct scroll_list *list)
{
    struct textlisttype *ttmp = list->textlist;
    struct textlisttype *ctmp;
    
    if (ttmp != NULL) {
       ctmp = ttmp->next_entry;
       
       while (ttmp != NULL) {
           free(ttmp);
           ttmp = ctmp;
           
           if (ctmp != NULL)
               ctmp = ctmp->next_entry;
       }
    }
}
