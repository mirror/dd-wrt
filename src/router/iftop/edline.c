/*
 * edline.c:
 * Text input on a line of the screen.
 *
 */

static const char rcsid[] = "$Id: edline.c,v 1.2 2002/11/04 12:27:35 chris Exp $";

//#include <ctype.h>
#include <curses.h>
#include <string.h>

#include "iftop.h"

static int min(const int a, const int b) {
    return a < b ? a : b;
}

/* edline:
 * Display on line linenum of the screen the prompt and allow the user to input
 * a line of text, whose initial value is as supplied. */
char *edline(int linenum, const char *prompt, const char *initial) {
    int xstart, slen, off = 0, pos, i, c;
    char *str;
    
    xstart = strlen(prompt) + 2;

    if (initial) {
        str = xmalloc(slen = strlen(initial) * 2 + 1);
        strcpy(str, initial);
    } else {
        str = xmalloc(slen = 256);
        *str = 0;
    }

    pos = strlen(str);

    do {
        c = getch();
        switch (c) {
            case KEY_DL:
            case 21:    /* ^U */
                *str = 0;
                pos = 0;
                break;

            case KEY_LEFT:
                --pos;
                if (pos < 0) {
                    beep();
                    pos = 0;
                }
                break;

            case KEY_RIGHT:
                ++pos;
                if (pos > strlen(str)) {
                    beep();
                    pos = strlen(str);
                }
                break;

            case KEY_HOME:
            case 1:         /* ^A */
                pos = 0;
                break;

            case KEY_END:
            case 5:         /* ^E */
                pos = strlen(str);
                break;

            case KEY_DC:
                if (pos == strlen(str))
                    beep();
                else
                    memmove(str + pos, str + pos + 1, strlen(str + pos + 1) + 1);
                break;

            case KEY_BACKSPACE:
                if (pos == 0)
                    beep();
                else {
                    memmove(str + pos - 1, str + pos, strlen(str + pos) + 1);
                    --pos;
                }
                break;

            case 23:    /* ^W */
                for (i = pos; i > 0; --i)
                    if (!isspace((int)str[i])) break;
                for (; i > 0; --i)
                    if (isspace((int)str[i])) break;
                if (i != pos) {
                    memmove(str + i, str + pos, strlen(str + pos) + 1);
                    pos = i;
                }
                break;

            case ERR:
                break;

            default:
                if (isprint(c) && c != '\t') {
                    if (strlen(str) == slen - 1)
                        str = xrealloc(str, slen *= 2);
                    memmove(str + pos + 1, str + pos, strlen(str + pos) + 1);
                    str[pos++] = (char)c;
                } else
                    beep();
                break;
        }

        /* figure out the offset to use for the string */
        off = 0;
        if (pos > COLS - xstart - 1)
            off = pos - (COLS - xstart - 1);
        
        /* display the string */
        mvaddstr(linenum, 0, prompt);
        addstr("> ");
        addnstr(str + off, min(strlen(str + off), COLS - xstart - 1));
        clrtoeol();
        move(linenum, xstart + pos - off);
        refresh();
    } while (c != KEY_ENTER && c != '\r' && c != '\x1b' && c != 7 /* ^G */);

    if (c == KEY_ENTER || c == '\r')
        /* Success */
        return str;
    else {
        xfree(str);
        return NULL;
    }
}
