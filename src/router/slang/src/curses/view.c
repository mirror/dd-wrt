/*
 * view.c -- a silly little viewer program
 *
 * written by Eric S. Raymond <esr@snark.thyrsus.com> December 1994
 * to test the scrolling code in ncurses.
 *
 * Takes a filename argument.  It's a simple file-viewer with various
 * scroll-up and scroll-down commands.
 *
 * n	-- scroll one line forward
 * p	-- scroll one line back
 *
 * Either command accepts a numeric prefix interpreted as a repeat count.
 * Thus, typing `5n' should scroll forward 5 lines in the file.
 *
 * The way you can tell this is working OK is that, in the trace file,
 * there should be one scroll operation plus a small number of line
 * updates, as opposed to a whole-page update.  This means the physical
 * scroll operation worked, and the refresh() code only had to do a
 * partial repaint.
 */
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#define MAXLINES        256        /* most lines we can handle */

static void finish(int sig);

static char        *lines[MAXLINES];

int main(int argc, char *argv[])
{
FILE        *fp;
char        buf[BUFSIZ];
int         i;
char        **lptr, **olptr;

#ifdef TRACE
    trace(TRACE_UPDATE);
#endif

    if (argc != 2) {
        fprintf(stderr, "usage: view file\n");
        exit(1);
    } else if ((fp = fopen(argv[1], "r")) == (FILE *)NULL) {
        perror(argv[1]);
        exit(1);
    }

    (void) signal(SIGINT, finish);      /* arrange interrupts to terminate */

    (void) initscr();      /* initialize the curses library */
    keypad(stdscr, TRUE);  /* enable keyboard mapping */
    (void) nonl();         /* tell curses not to do NL->CR/NL on output */
    (void) cbreak();       /* take input chars one at a time, no wait for \n */
    (void) noecho();       /* don't echo input */
    scrollok(stdscr, TRUE);

    /* slurp the file */
    for (lptr = &lines[0]; fgets(buf, BUFSIZ, fp) != (char *)NULL; lptr++) {
        if (lptr - lines >= MAXLINES) {
            endwin();
            (void) fprintf(stderr, "%s: %s is too large\n", argv[0], argv[1]);
            exit(1);
        }

        buf[strlen(buf) - 1] = '\0';
        *lptr = (char *)malloc((size_t)(COLS + 1));
        (void) strncpy(*lptr, buf, (size_t)COLS);
	(*lptr)[COLS] = '\0';
    }
    (void) fclose(fp);

    lptr = lines;
    for (;;) {
        int n, c;
	bool explicit;

        for (i = 0; i < LINES; i++) {
            move(i, 0);
            clrtoeol();
            if (lptr[i])
                addstr(lptr[i]);
        }

	explicit = FALSE;
	n = 0;
        for (;;) {
	    c = getch();
	    if (isdigit(c))
		n = 10 * n + (c - '0');
	    else
		break;
	}
	if (!explicit && n == 0)
	    n = 1;

        switch(c) {
        case KEY_DOWN:
	case 'n':
	    olptr = lptr;
	    for (i = 0; i < n; i++)
		if (lptr + LINES < lines + MAXLINES && lptr[LINES + 1])
		    lptr++;
	        else
		    break;
	    wscrl(stdscr, lptr - olptr);
            break;

        case KEY_UP:
	case 'p':
	    olptr = lptr;
	    for (i = 0; i < n; i++)
		if (lptr > lines)
		    lptr--;
	        else
		    break;
	    wscrl(stdscr, lptr - olptr);
            break;

	 default:
	   move (0,0);
	   clrtoeol ();
	   printw ("Invalid input: %c", c);
	   refresh ();
	   sleep (1);
	}
    }

    finish(0);               /* we're done */
}

static void finish(int sig)
{
    endwin();
    exit(sig != 0);
}

/* view.c ends here */

