/*
 * watch - execute a program repeatedly, displaying output fullscreen
 *
 * Copyright © 2023 Roman Žilka <roman.zilka@gmail.com>
 * Copyright © 2010-2023 Jim Warner <james.warner@comcast.net>
 * Copyright © 2015-2024 Craig Small <csmall@dropbear.xyz>
 * Copyright © 2011-2012 Sami Kerola <kerolasa@iki.fi>
 * Copyright © 2002-2007 Albert Cahalan
 * Copyright © 1999      Mike Coleman <mkc@acm.org>.
 *
 * Based on the original 1991 'watch' by Tony Rems <rembo@unisoft.com>
 * (with mods and corrections by Francois Pinard).
 *
 * stderr handling, exec, and beep option added by Morty Abzug, 2008
 * Unicode Support added by Jarrod Lowe <procps@rrod.net> in 2009.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef WITH_WATCH8BIT
# define _XOPEN_SOURCE_EXTENDED 1
# include <wctype.h>
#else
# include <ctype.h>
#endif
#ifdef HAVE_NCURSESW_NCURSES_H
# include <ncursesw/ncurses.h>
#else
# include <ncurses.h>
#endif
#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <locale.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "c.h"
#include "config.h"
#include "fileutils.h"
#include "nls.h"
#include "signals.h"
#include "strutils.h"
#include "xalloc.h"

#ifdef FORCE_8BIT
# undef isprint
# define isprint(x) ( (x>=' '&&x<='~') || (x>=0xa0) )
#endif

#ifdef WITH_WATCH8BIT
#define XL(c) L ## c
#define XEOF WEOF
#define Xint wint_t
#define Xgetc(stream) getmb(stream)
#else
#define XL(c) c
#define XEOF EOF
#define Xint int
#define Xgetc(stream) getc(stream)
#endif

#define HEIGHT_FALLBACK 24
#define WIDTH_FALLBACK 80
#define TAB_WIDTH 8
#define MAX_ANSIBUF 100
#define HEADER_HEIGHT 2

/* Boolean command line options */
#define WATCH_DIFF     (1 << 0)
#define WATCH_CUMUL    (1 << 1)
#define WATCH_EXEC     (1 << 2)
#define WATCH_BEEP     (1 << 3)
#define WATCH_COLOR    (1 << 4)
#define WATCH_ERREXIT  (1 << 5)
#define WATCH_CHGEXIT  (1 << 6)
#define WATCH_EQUEXIT  (1 << 7)
#define WATCH_NORERUN  (1 << 8)
#define WATCH_PRECISE  (1 << 9)
#define WATCH_NOWRAP   (1 << 10)
#define WATCH_NOTITLE  (1 << 11)
#define WATCH_FOLLOW   (1 << 12)
// Do we care about screen contents changes at all?
#define WATCH_ALL_DIFF (WATCH_DIFF | WATCH_CHGEXIT | WATCH_EQUEXIT)

static uf16 flags;
static int height, width;
static bool first_screen = true, screen_size_changed, screen_changed;
static double interval_real = 2;
static char *command;
static size_t command_len;
static char *const *command_argv;
static const char *shotsdir = "";


WINDOW *mainwin;

// don't use EXIT_FAILURE, it can be anything and manpage makes guarantees about
// exitcodes
static void __attribute__ ((__noreturn__)) usage(FILE * out)
{
	fputs(USAGE_HEADER, out);
	fprintf(out, _(" %s [options] command\n"), program_invocation_short_name);
	fputs(USAGE_OPTIONS, out);
	// TODO: other tools in src/ use one leading blank
	fputs(_("  -b, --beep             beep if command has a non-zero exit\n"), out);
	fputs(_("  -c, --color            interpret ANSI color and style sequences\n"), out);
	fputs(_("  -C, --no-color         do not interpret ANSI color and style sequences\n"), out);
	fputs(_("  -d, --differences[=<permanent>]\n"
	        "                         highlight changes between updates\n"), out);
	fputs(_("  -e, --errexit          exit if command has a non-zero exit\n"), out);
        fputs(_("  -f, --follow           Follow the output and don't clear screen\n"), out);
	fputs(_("  -g, --chgexit          exit when output from command changes\n"), out);
	fputs(_("  -q, --equexit <cycles>\n"
	        "                         exit when output from command does not change\n"), out);
	fputs(_("  -n, --interval <secs>  seconds to wait between updates\n"), out);
	fputs(_("  -p, --precise          -n includes command running time\n"), out);  // TODO: gettext
	fputs(_("  -r, --no-rerun         do not rerun program on window resize\n"), out);
	fputs(_("  -s, --shotsdir         directory to store screenshots\n"), out);  // TODO: gettext
	fputs(_("  -t, --no-title         turn off header\n"), out);
	fputs(_("  -w, --no-wrap          turn off line wrapping\n"), out);
	fputs(_("  -x, --exec             pass command to exec instead of \"sh -c\"\n"), out);
	fputs(USAGE_SEPARATOR, out);
	fputs(USAGE_HELP, out);
	fputs(_(" -v, --version  output version information and exit\n"), out);
	fprintf(out, USAGE_MAN_TAIL("watch(1)"));

	exit(out == stderr ? 1 : EXIT_SUCCESS);
}

#define endwin_xerr(...) do { endwin(); err(__VA_ARGS__); } while (0)
#define endwin_error(...) do { endwin(); error(__VA_ARGS__); } while (0)
#define endwin_exit(status) do { endwin(); exit(status); } while (0)

static void die(int notused __attribute__ ((__unused__)))
{
	endwin_exit(EXIT_SUCCESS);
}

static void winch_handler(int notused __attribute__ ((__unused__)))
{
	screen_size_changed = true;
}



static int attributes;  // TODO: attr_t likely has more value digits than int
static int nr_of_colors, fg_col, bg_col;
static bool more_colors;

static void reset_ansi(void)
{
	attributes = A_NORMAL;
	fg_col = 0;
	bg_col = 0;
}



static void init_ansi_colors(void)
{
	const short ncurses_colors[] = {
		-1, COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
		COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE
	};
	nr_of_colors = sizeof(ncurses_colors) / sizeof(*ncurses_colors);

	more_colors = (COLORS >= 16) && (COLOR_PAIRS >= 16 * 16);

	// Initialize ncurses colors. -1 is terminal default
	// 0-7 are auto created standard colors initialized by ncurses
	if (more_colors) {
		// Initialize using ANSI SGR 8-bit specified colors
		// 8-15 are bright colors
		init_color(8, 333, 333, 333);  // Bright black
		init_color(9, 1000, 333, 333);  // Bright red
		init_color(10, 333, 1000, 333);  // Bright green
		init_color(11, 1000, 1000, 333);  // Bright yellow
		init_color(12, 333, 333, 1000);  // Bright blue
		init_color(13, 1000, 333, 1000);  // Bright magenta
		init_color(14, 333, 1000, 1000);  // Bright cyan
		// Often ncurses is built with only 256 colors, so lets
		// stop here - so we can support the -1 terminal default
		//init_color(15, 1000, 1000, 1000);  // Bright white
		nr_of_colors += 7;
	}
#ifdef WITH_WATCH8BIT
        if (COLORS >= 256 && COLOR_PAIRS >= 65536)
        {
            int red,green,blue;
            int code, r,g,b;
            // 16 to 231 are a 6x6x6 color cube
            for (red=0; red<6; red++)
                for(green=0; green<6; green++)
                    for(blue=0; blue<6; blue++) {
                        code = 16 + (red * 36) + (green * 6) + blue;
                        r = g = b = 0;
                        if (red > 0)
                            r = (red * 40 + 55) * 1000 / 256;
                        if (green > 0)
                            g = (green * 40 + 55) * 1000 / 256;
                        if (blue > 0)
                            b = (blue * 40 + 55) * 1000 / 256;
                        init_extended_color(code, r, g, b);
                        nr_of_colors++;
                    }
            for (red=0; red<24; red++) {
                code = 232 + red;
                r = (red * 10 + 8) * 1000 / 256;
                init_extended_color(code, r, r, r);
                nr_of_colors++;
            }
        }
#endif /*8bit*/
	// Initialize all color pairs with ncurses
	for (bg_col = 0; bg_col < nr_of_colors; bg_col++)
		for (fg_col = 0; fg_col < nr_of_colors; fg_col++)
#ifdef WITH_WATCH8BIT
			init_extended_pair(bg_col * nr_of_colors + fg_col + 1, fg_col - 1, bg_col - 1);
#else
			init_pair(bg_col * nr_of_colors + fg_col + 1, fg_col - 1, bg_col - 1);
#endif

	reset_ansi();
}



static uf8 process_ansi_color_escape_sequence(char **const escape_sequence) {
	// process SGR ANSI color escape sequence
	// Eg 8-bit
	// 38;5;⟨n⟩  (set fg color to n)
	// 48;5;⟨n⟩  (set bg color to n)
	//
	// Eg 24-bit (not yet implemented)
	// ESC[ 38;2;⟨r⟩;⟨g⟩;⟨b⟩ m Select RGB foreground color
	// ESC[ 48;2;⟨r⟩;⟨g⟩;⟨b⟩ m Select RGB background color

	if (!escape_sequence || !*escape_sequence)
		return 0; /* avoid NULLPTR dereference, return "not understood" */

	if ((*escape_sequence)[0] != ';')
		return 0; /* not understood */

	if ((*escape_sequence)[1] == '5') {
		// 8 bit! ANSI specifies a predefined set of 256 colors here.
		if ((*escape_sequence)[2] != ';')
			return 0; /* not understood */
		long num = strtol((*escape_sequence) + 3, escape_sequence, 10);
		if (num >= 0 && num <= 7) {
			// 0-7 are standard colors  same as SGR 30-37
			return num + 1;
		}
		if (num >= 8 && num <= 15) {
			// 8-15 are standard colors  same as SGR 90-97
			 return more_colors ? num + 1 : num - 8 + 1;
		}
		// 16-231:  6 × 6 × 6 cube (216 colors): 16 + 36 × r + 6 × g + b
		//                                       (0 ≤ r, g, b ≤ 5)
		// 232-255:  grayscale from black to white in 24 steps
#ifdef WITH_WATCH8BIT
                if (num > 15 && num < 256)
                    return more_colors ? num + 1 : 0;
#endif
	}

	return 0; /* not understood */
}



static bool set_ansi_attribute(const int attrib, char** escape_sequence)
{
	if (!(flags & WATCH_COLOR))
		return true;

	switch (attrib) {
	case -1:	/* restore last settings */
		break;
	case 0:		/* restore default settings */
		reset_ansi();
		break;
	case 1:		/* set bold / increased intensity */
		attributes |= A_BOLD;
		break;
	case 2:		/* set decreased intensity (if supported) */
		attributes |= A_DIM;
		break;
#ifdef A_ITALIC
	case 3:		/* set italic (if supported) */
		attributes |= A_ITALIC;
		break;
#endif
	case 4:		/* set underline */
		attributes |= A_UNDERLINE;
		break;
	case 5:		/* set blinking */
		attributes |= A_BLINK;
		break;
	case 7:		/* set inversed */
		attributes |= A_REVERSE;
		break;
	case 21:	/* unset bold / increased intensity */
		attributes &= ~A_BOLD;
		break;
	case 22:	/* unset bold / any intensity modifier */
		attributes &= ~(A_BOLD | A_DIM);
		break;
#ifdef A_ITALIC
	case 23:	/* unset italic */
		attributes &= ~A_ITALIC;
		break;
#endif
	case 24:	/* unset underline */
		attributes &= ~A_UNDERLINE;
		break;
	case 25:	/* unset blinking */
		attributes &= ~A_BLINK;
		break;
	case 27:	/* unset inversed */
		attributes &= ~A_REVERSE;
		break;
    case 38:
        fg_col = process_ansi_color_escape_sequence(escape_sequence);
        if (fg_col == 0) {
            return false; /* not understood */
        }
        break;
	case 39:
		fg_col = 0;
		break;
    case 48:
        bg_col = process_ansi_color_escape_sequence(escape_sequence);
        if (bg_col == 0) {
            return false; /* not understood */
        }
        break;
    case 49:
        bg_col = 0;
        break;
	default:
		if (attrib >= 30 && attrib <= 37) {	/* set foreground color */
			fg_col = attrib - 30 + 1;
		} else if (attrib >= 40 && attrib <= 47) { /* set background color */
			bg_col = attrib - 40 + 1;
		} else if (attrib >= 90 && attrib <= 97) { /* set bright fg color */
			fg_col = more_colors ? attrib - 90 + 9 : attrib - 90 + 1;
		} else if (attrib >= 100 && attrib <= 107) { /* set bright bg color */
			bg_col = more_colors ? attrib - 100 + 9 : attrib - 100 + 1;
		} else {
			return false; /* Not understood */
		}
	}
    int c = bg_col * nr_of_colors + fg_col + 1;
    wattr_set(mainwin, attributes, 0, &c);
    return true;
}



static void process_ansi(FILE * fp)
{
	if (!(flags & WATCH_COLOR))
		return;

	int i, c;
	char buf[MAX_ANSIBUF];
	char *numstart, *endptr;
	int ansi_attribute;

	c = getc(fp);

	if (c == '(') {
		c = getc(fp);
		c = getc(fp);
	}
	if (c != '[') {
		ungetc(c, fp);
		return;
	}
	for (i = 0; i < MAX_ANSIBUF; i++) {
		c = getc(fp);
		/* COLOUR SEQUENCE ENDS in 'm' */
		if (c == 'm') {
			buf[i] = '\0';
			break;
		}
		if ((c < '0' || c > '9') && c != ';') {
			return;
		}
		assert(c >= 0 && c <= SCHAR_MAX);
		buf[i] = c;
	}
	/*
	 * buf now contains a semicolon-separated list of decimal integers,
	 * each indicating an attribute to apply.
	 * For example, buf might contain "0;1;31", derived from the color
	 * escape sequence "<ESC>[0;1;31m". There can be 1 or more
	 * attributes to apply, but typically there are between 1 and 3.
	 */

    /* Special case of <ESC>[m */
    if (buf[0] == '\0')
        set_ansi_attribute(0, NULL);

    for (endptr = numstart = buf; *endptr != '\0'; numstart = endptr + 1) {
        ansi_attribute = strtol(numstart, &endptr, 10);
        if (!set_ansi_attribute(ansi_attribute, &endptr))
            break;
        if (numstart == endptr)
            set_ansi_attribute(0, NULL); /* [m treated as [0m */
    }
}



typedef uf64 watch_usec_t;
#define USECS_PER_SEC ((watch_usec_t)1000000)  // same type

static inline watch_usec_t get_time_usec(void)
{
	struct timeval now;
#if defined(HAVE_CLOCK_GETTIME) && defined(_POSIX_TIMERS)
	struct timespec ts;
	if (0 > clock_gettime(CLOCK_MONOTONIC, &ts))
		endwin_xerr(1, "clock_gettime(CLOCK_MONOTONIC)");
	TIMESPEC_TO_TIMEVAL(&now, &ts);
#else
	gettimeofday(&now, NULL);
#endif /* HAVE_CLOCK_GETTIME */
	return USECS_PER_SEC * now.tv_sec + now.tv_usec;
}



static void screenshot(void) {
	static time_t last;
	static uf8 last_nr;
	static char *dumpfile;
	static size_t dumpfile_mark;
	static uf8 dumpfile_avail = 128;

	if (! dumpfile) {
		dumpfile_mark = strlen(shotsdir);  // can be empty
		if (SIZE_MAX - dumpfile_mark < dumpfile_avail)
			endwin_error(1, ENAMETOOLONG, "%s", shotsdir);
		dumpfile = xmalloc(dumpfile_mark + dumpfile_avail);  // never freed
		if (dumpfile_mark) {
			memcpy(dumpfile, shotsdir, dumpfile_mark);
			if (dumpfile[dumpfile_mark-1] != '/') {
				dumpfile[dumpfile_mark++] = '/';
				--dumpfile_avail;
			}
		}
		memcpy(dumpfile+dumpfile_mark, "watch_", 6);
		dumpfile_mark += 6;
		dumpfile_avail -= 6;
	}

	const time_t now = time(NULL);
	if (! strftime(dumpfile+dumpfile_mark, dumpfile_avail, "%Y%m%d-%H%M%S", localtime(&now))) {
		assert(false);
		dumpfile[dumpfile_mark] = '\0';
	}
	if (now == last) {
		const uf8 l = strlen(dumpfile+dumpfile_mark);
		assert(dumpfile_avail - l >= 5);
		snprintf(dumpfile+dumpfile_mark+l, dumpfile_avail-l, "-%03" PRIuFAST8, last_nr);
		assert(last_nr < UINT_FAST8_MAX);
		if (last_nr < UINT_FAST8_MAX)
			++last_nr;
	}
	else {
		last = now;
		last_nr = 0;
	}

	const int f = open(dumpfile, O_WRONLY|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (f == -1)
		endwin_xerr(1, "open(%s)", dumpfile);

	int bufsize = 0;  // int because of mvinnstr()
#ifdef WITH_WATCH8BIT
	if ( INT_MAX / width >= CCHARW_MAX &&
	     MB_CUR_MAX <= INT_MAX &&
	     INT_MAX / (width*CCHARW_MAX) >= (int)MB_CUR_MAX &&
	     width * CCHARW_MAX * MB_CUR_MAX < INT_MAX
	   ) {
		bufsize = width*CCHARW_MAX*MB_CUR_MAX + 1;
	}
#else
	if (width < INT_MAX)
		bufsize = width + 1;
#endif
	if (! bufsize || (uintmax_t)bufsize > SIZE_MAX)
		endwin_error(1, EOVERFLOW, "%s(%s)", __func__, dumpfile);
	char *const buf = xmalloc(bufsize);

	int yin, xout, tmpy, tmpx;
        getyx(mainwin, tmpy, tmpx);
	for (int y=0; y<height; ++y) {
		yin = mvwinnstr(mainwin, y, 0, buf, bufsize-1);
		if (yin == ERR)  // screen resized
			yin = 0;
		buf[yin] = '\n';
		for (int x=0; x<yin+1; x+=xout) {
			xout = write(f, buf+x, (yin+1)-x);
			if (xout == -1)
				endwin_xerr(1, "write(%s)", dumpfile);
		}
	}
        wmove(mainwin, tmpy, tmpx);
	free(buf);

	if (close(f) == -1)
		endwin_xerr(1, "close(%s)", dumpfile);

	if (screen_size_changed)
		endwin_error(1, ECANCELED, "%s(%s)", __func__, dumpfile);
}



static void output_header(
        WINDOW *hdrwin // Header window
        )
{
	if (flags & WATCH_NOTITLE)
		return;

	static char *lheader;
	static int lheader_len;
#ifdef WITH_WATCH8BIT
	static wchar_t *wlheader;
	static int wlheader_wid;
#endif

	static char rheader[256+128];  // hostname and timestamp
	static int rheader_lenmid;  // just before timestamp
	int rheader_len;
#ifdef WITH_WATCH8BIT
	wchar_t *wrheader;
	int wrheader_wid;

	static wchar_t *wcommand;
	static size_t wcommand_len;
	static int wcommand_wid;

	static sf8 ellipsis_wid;
#endif

	if (! lheader_len) {
		// my glibc says HOST_NAME_MAX is 64, but as soon as it updates it to be
		// at least POSIX.1-2001-compliant, it will be one of: 255, very large,
		// unspecified
		if (gethostname(rheader, 256))
			rheader[0] = '\0';
		rheader[255] = '\0';
		rheader_lenmid = strlen(rheader);
		rheader[rheader_lenmid++] = ':';
		rheader[rheader_lenmid++] = ' ';

		// never freed for !WATCH8BIT
		lheader_len = asprintf(&lheader, _("Every %.1fs: "), interval_real);
		if (lheader_len == -1)
			endwin_xerr(1, "%s()", __func__);
#ifdef WITH_WATCH8BIT
		// never freed
		wlheader_wid = mbswidth(lheader, &wlheader);
		if (wlheader_wid == -1) {
			wlheader = L"";
			wlheader_wid = 0;
		}
		free(lheader);

		// never freed
		wcommand_wid = mbswidth(command, &wcommand);
		if (wcommand_wid == -1) {
			wcommand = L"";
			wcommand_wid = 0;
		}
		wcommand_len = wcslen(wcommand);

		ellipsis_wid = wcwidth(L'\u2026');
#endif
	}

	// TODO: a gettext string for rheader no longer used
	const time_t t = time(NULL);
	rheader_len = rheader_lenmid;
	rheader_len += strftime(rheader+rheader_lenmid, sizeof(rheader)-rheader_lenmid, "%c", localtime(&t));
	if (rheader_len == rheader_lenmid)
		rheader[rheader_len] = '\0';
#ifdef WITH_WATCH8BIT
	wrheader_wid = mbswidth(rheader, &wrheader);
	if (wrheader_wid == -1) {
		wrheader = xmalloc(sizeof(*wrheader));
		wrheader[0] = L'\0';
		wrheader_wid = 0;
	}
#endif

	if (first_screen) {
		wmove(hdrwin, 0, 0);
		wclrtoeol(hdrwin);
	}

	/* left justify interval and command, right justify hostname and time,
	 * clipping all to fit window width
	 *
	 * the rules:
	 *   width < rhlen : print nothing
	 *   width < rhlen + hlen + 1: print hostname, ts
	 *   width = rhlen + hlen + 1: print header, hostname, ts
	 *   width < rhlen + hlen + 4: print header, ..., hostname, ts
	 *   width < rhlen + hlen + wcommand_columns: print header,
	 *                           truncated wcommand, ..., hostname, ts
	 *   width > "": print header, wcomand, hostname, ts
	 * this is slightly different from how it used to be */

// (w)command_* can be large, *header_* are relatively small
#ifdef WITH_WATCH8BIT
	if (width >= wrheader_wid) {
		mvwaddwstr(hdrwin, 0, width - wrheader_wid, wrheader);
		const int avail4cmd = width - wlheader_wid - wrheader_wid;
		if (avail4cmd >= 0) {
			mvwaddwstr(hdrwin,0, 0, wlheader);
			// All of cmd fits, +1 for delimiting space
			if (avail4cmd > wcommand_wid)
				waddwstr(hdrwin, wcommand);
			// Else print truncated cmd (to 0 chars, possibly) + ellipsis. If
			// there's too little space even for the ellipsis, print nothing.
			else if (avail4cmd > ellipsis_wid) {
				assert(wcommand_len > 0);
				int newwcmdwid;
				size_t newwcmdlen = wcommand_len;
				// from the back
				do newwcmdwid = wcswidth(wcommand, --newwcmdlen);
				while (newwcmdwid > avail4cmd-ellipsis_wid-1);
				waddnwstr(hdrwin, wcommand, newwcmdlen&INT_MAX);
				waddwstr(hdrwin, L"\u2026");
			}
		}
	}
	free(wrheader);
#else
	if (width >= rheader_len) {
		mvwaddstr(hdrwin, 0, width - rheader_len, rheader);
		const int avail4cmd = width - lheader_len - rheader_len;
		if (avail4cmd >= 0) {
			mvwaddstr(hdrwin, 0, 0, lheader);
			if ((uintmax_t)avail4cmd > command_len)
				waddstr(hdrwin, command);
			else if (avail4cmd > 3) {
				waddnstr(hdrwin, command, avail4cmd - 3 - 1);
				waddstr(hdrwin, "...");
			}
		}
	}
#endif
}



static void output_lowheader_pre(
        WINDOW *hdrwin
        )
{
	if (flags & WATCH_NOTITLE)
		return;

	wmove(hdrwin, 1, 0);
	wclrtoeol(hdrwin);
}

static void output_lowheader(
        WINDOW *hdrwin,
        watch_usec_t span,
        uint8_t exitcode)
{
	if (flags & WATCH_NOTITLE)
		return;

	char s[64];
	int skip;
	// TODO: gettext everywhere
	if (span > USECS_PER_SEC * 24 * 60 * 60)
		snprintf(s, sizeof(s), "%s >1 %s (%" PRIu8 ")", "in", "day", exitcode);
	// for the localized decimal point
	else if (span < 1000)
		snprintf(s, sizeof(s), "%s <%.3f%s (%" PRIu8 ")", "in", 0.001, "s", exitcode);
	else snprintf(s, sizeof(s), "%s %.3Lf%s (%" PRIu8 ")", "in", (long double)span/USECS_PER_SEC, "s", exitcode);

	wmove(hdrwin, 1, 0);
	wclrtoeol(hdrwin);

#ifdef WITH_WATCH8BIT
	wchar_t *ws;
	skip = mbswidth(s, &ws);
	if (skip == -1)
		return;
	skip = width - skip;
	if (skip >= 0)
		mvwaddwstr(hdrwin, 1, skip, ws);
	free(ws);
#else
	skip = width - (int)strlen(s);
	if (skip >= 0)
		mvwaddstr(hdrwin, 1, skip, s);
#endif
}



// When first_screen, returns false. Otherwise, when WATCH_ALL_DIFF is false,
// return value is unspecified. Otherwise, returns true <==> the character at
// (y, x) changed. After return, cursor position is indeterminate.
//
// The change detection algorithm assumes that all characters (spacing and
// non-spacing) belonging to a set of coords are display_char()d one after
// another. That occurs naturally when writing out text from beginning to end.
//
// The function emulates the behavior ncurses claims to have according to
// curs_add_wch(3x) in that a non-spacing c is added to the spacing character
// already present at (y, x). In reality, ncurses (as of 6.4-20230401) adds it
// to the character at (y, x-1). This affects add_wch() as well as addwstr() et
// al.
static bool display_char(int y, int x, Xint c, int cwid) {
	assert(c != XEOF && c != XL('\0'));  // among other things
	assert(cwid >= 0);
	assert(width-x >= cwid);  // fits
	bool changed = false;
	bool old_standout = false;

#ifdef WITH_WATCH8BIT
// there's an array on stack the size of a function of this
#if (CCHARW_MAX < 3 || CCHARW_MAX > 15)
#error "ncurses' CCHARW_MAX has an unexpected value!"
#endif
	if (! first_screen && flags&WATCH_ALL_DIFF) {
		assert(cwid <= 15);
		static wchar_t oldcc[15][CCHARW_MAX];
		static uf8 oldcclen[15];  // each in [1, CCHARW_MAX)
		static uf8 oldccwid;
		static bool oldstnd;
		static int curx = -1, cury = -1;
		uf8 i, j;
		// This wouldm't work properly if cmd output had a single character and
		// we weren't manually printing ' 's to empty the rest of screen. But
		// when flags&WATCH_ALL_DIFF we are printing the ' 's.
		if (y != cury || x != curx) {
			cchar_t cc;
			short dummy;
			attr_t attr;
			cury = y; curx = x;
			oldstnd = false;
			// If cwid=0, do anything. It shouldn't happen in a proper string.
			oldccwid = cwid;
			// Check every column the new c will occupy. Takes care of, e.g.,
			// 日a -> a日a (日 highlighted because of change in its 2nd column).
			for (i=0; i<cwid; ++i) {
				// terrible interface, so much copying
				mvwin_wch(mainwin, y, x+i, &cc);  // c fits => ok
				getcchar(&cc, oldcc[i], &attr, &dummy, NULL);
				oldstnd |= attr & A_STANDOUT;
				oldcclen[i] = wcslen(oldcc[i]);
				// if nothing else, there is the ' ' there
				assert(oldcclen[i] > 0);
			}
		}

		// If there's no change, then c must be a component of each of the
		// characters. A component not found yet. Find it and mark as found
		// (L'\0').
		for (i=0; i<oldccwid; ++i) {
			for (j=0; j<oldcclen[i]; ++j) {
				if (oldcc[i][j] == (wchar_t)c) {
					oldcc[i][j] = L'\0';
					break;
				}
			}
			if (j == oldcclen[i]) {
				oldccwid = 0;  // mark as changed for good
				break;
			}
		}
		if (! oldccwid)
			changed = true;
		else {
			changed = false;
			for (i=0; i<oldccwid; ++i) {
				for (j=0; j<oldcclen[i]; ++j) {
					if (oldcc[i][j]) {
						changed = true;
						break;
					}
				}
				if (j < oldcclen[i])
					break;
			}
		}

		old_standout = oldstnd;
	}

        if (!(flags & WATCH_FOLLOW))
	        wmove(mainwin, y, x);
	if (cwid > 0) {
		wchar_t c2 = c;
		waddnwstr(mainwin, &c2, 1);
	}
	else {
		cchar_t cc;
		wchar_t wcs[CCHARW_MAX];
		short dummy;
		attr_t dummy2;
		win_wch(mainwin,&cc);
		getcchar(&cc, wcs, &dummy2, &dummy, NULL);
		uf8 len = wcslen(wcs);
		if (len < CCHARW_MAX - 1) {
			wcs[len] = c;
			wcs[len+1] = L'\0';
		}
		setcchar(&cc, wcs, dummy2, dummy, NULL);
		wadd_wch(mainwin, &cc);
	}
#else
	if (! first_screen && flags&WATCH_ALL_DIFF) {
		chtype oldc = mvwinch(mainwin, y, x);
		changed = (unsigned char)c != (oldc & A_CHARTEXT);
		old_standout = oldc & A_STANDOUT;
	}

	wmove(mainwin,y, x);
	waddch(mainwin, c);
#endif

	if (flags & WATCH_DIFF) {
		attr_t newattr;
		short newcolor;
		wattr_get(mainwin, &newattr, &newcolor, NULL);
		// standout can flip on/off as the components of a compound char arrive

		if (changed || (flags&WATCH_CUMUL && old_standout))
			mvwchgat(mainwin, y, x, 1, newattr | A_STANDOUT, newcolor, NULL);
		else
			mvwchgat(mainwin, y, x, 1, newattr & ~(attr_t)A_STANDOUT, newcolor, NULL);
	}

	return changed;
}



static void skiptoeol(FILE *f)
{
	Xint c;
	do c = Xgetc(f);
	while (c != XEOF && c != XL('\n'));
}

static void skiptoeof(FILE *f) {
	unsigned char dummy[4096];
	while (! feof(f) && ! ferror(f))
		(void)!fread(dummy, sizeof(dummy), 1, f);
}

static bool my_clrtoeol(int y, int x)
{
	if (flags & WATCH_ALL_DIFF) {
		bool changed = false;
		while (x < width)
			changed = display_char(y, x++, XL(' '), 1) || changed;
		return changed;
	}

	// make sure color is preserved
	wmove(mainwin, y, x);
	wclrtoeol(mainwin);  // faster, presumably
	return false;
}

static bool my_clrtobot(int y, int x)
{
	if (flags & WATCH_ALL_DIFF) {
		bool changed = false;
		while (y < height) {
			while (x < width)
				changed = display_char(y, x++, XL(' '), 1) || changed;
			x = 0;
			++y;
		}
		return changed;
	}
	// make sure color is preserved
        if (!(flags & WATCH_FOLLOW)) {
            wmove(mainwin, y, x);
            wclrtobot(mainwin);  // faster, presumably
        }
	return false;
}



// Sets screen_changed: when first_screen, screen_changed=false. Otherwise, when
// ! WATCH_ALL_DIFF, screen_changed will be unspecified. Otherwise,
// screen_changed=true <==> the screen changed.
//
// Make sure not to leak system resources (incl. fds, processes). Suggesting
// -D_XOPEN_SOURCE=600 and an EINTR loop around every fclose() as well.
static uint8_t run_command(void)
{
	int pipefd[2], status;  // [0] = output, [1] = input
	if (pipe(pipefd) < 0)
		endwin_xerr(2, _("unable to create IPC pipes"));
	// child will share buffered data, will print it at fclose()
	fflush(stdout);
	fflush(stderr);

	pid_t child = fork();
	if (child < 0)
		endwin_xerr(2, _("unable to fork process"));
	else if (child == 0) {  /* in child */
		// stdout/err can't be used here. Avoid xerr(), close_stdout(), ...
		// fclose() so as not to confuse _Exit().
		fclose(stdout);
		fclose(stderr);
		// connect out and err up with pipe input
		while (close(pipefd[0]) == -1 && errno == EINTR) ;
		while (dup2(pipefd[1], STDOUT_FILENO) == -1 && errno == EINTR) ;
		while (close(pipefd[1]) == -1 && errno == EINTR) ;
		while (dup2(STDOUT_FILENO, STDERR_FILENO) == -1 && errno == EINTR) ;
		// TODO: 0 left open. Is that intentional? I suppose the application
		// might conclude it's run interactively (see ps). And hang if it should
		// wait for input (watch 'read A; echo $A').

		if (flags & WATCH_EXEC) {
			execvp(command_argv[0], command_argv);
			const char *const errmsg = strerror(errno);
			(void)!write(STDERR_FILENO, command_argv[0], strlen(command_argv[0]));
			// TODO: gettext?
			(void)!write(STDERR_FILENO, ": ", 2);
			(void)!write(STDERR_FILENO, errmsg, strlen(errmsg));
			_Exit(0x7f);  // sort of like sh
		}
		status = system(command);
		// errno from system() not guaranteed
		// -1 = error from system() (exec(), wait(), ...), not command
		if (status == -1) {
			(void)!write(STDERR_FILENO, command, command_len);
			// TODO: gettext
			(void)!write(STDERR_FILENO, ": unable to run", 15);
			_Exit(0x7f);
		}
		/* propagate command exit status as child exit status */
		// error msg on stderr provided by sh
		if (WIFEXITED(status))
			_Exit(WEXITSTATUS(status));
		// Else terminated by signal. system() ignores the stopping of children.
		assert(WIFSIGNALED(status));
		_Exit(0x80 + (WTERMSIG(status) & 0x7f));
	}
	/* otherwise, we're in parent */

	while (close(pipefd[1]) == -1 && errno == EINTR) ;
	FILE *p = fdopen(pipefd[0], "r");
	if (! p)
		endwin_xerr(2, _("fdopen"));
	setvbuf(p, NULL, _IOFBF, BUFSIZ);  // We'll getc() from it. A lot.

	Xint c, carry = XEOF;
	int cwid, y, x;  // cwid = character width in terminal columns
	screen_changed = false;

	for (y = 0; y < height || (flags & WATCH_FOLLOW); ++y) {
		x = 0;
		while (true) {
			// x is where the next char will be put. When x==width only
			// characters with wcwidth()==0 are output. Used, e.g., for
			// codepoints which modify the preceding character and swallowing a
			// newline / a color sequence / ... after a printable character in
			// the rightmost column.
			assert(x <= width);
			assert(x == 0 || carry == XEOF);

			if (carry != XEOF) {
				c = carry;
				carry = XEOF;
			}
			else c = Xgetc(p);
			assert(carry == XEOF);

			if (c == XEOF) {
                                if (!(flags & WATCH_FOLLOW)) {
				    screen_changed = my_clrtobot(y, x) || screen_changed;
				    y = height - 1;
                                }
				break;
			}
			if (c == XL('\n')) {
                                if (flags & WATCH_FOLLOW)
                                    waddch(mainwin, c);
                                else
				    screen_changed = my_clrtoeol(y, x) || screen_changed;
				break;
			}
			if (c == XL('\033')) {
				process_ansi(p);
				continue;
			}
			if (c == XL('\a')) {
				beep();
				continue;
			}
			if (c == XL('\t'))  // not is(w)print()
				// one space is enough to consider a \t printed, if there're no
				// more columns
				cwid = 1;
			else {
#ifdef WITH_WATCH8BIT
				// There used to be (! iswprint(c) && c < 128) because of Debian
				// #240989. Presumably because glibc of the time didn't
				// recognize ä, ö, ü, Π, ά, λ, ς, ... as printable. Today,
				// iswprint() in glibc works as expected and the "c<128" is
				// letting all non-printables >=128 get through.
				if (! iswprint(c))
					continue;
				cwid = wcwidth(c);
				assert(cwid >= 0 && cwid <= 2);
#else
				if (! isprint(c))
					continue;
				cwid = 1;
#endif
			}

			// now c is something printable
			// if it doesn't fit
			if (cwid > width-x) {
				assert(cwid > 0 && cwid <= 2);
				assert(width-x <= 1);  // !!
				if (! (flags & WATCH_NOWRAP))
					carry = c;
				else {
					skiptoeol(p);
					reset_ansi();
					set_ansi_attribute(-1, NULL);
				}
				screen_changed = my_clrtoeol(y, x) || screen_changed;
				break;
			}

			// it fits, print it
			if (c == XL('\t')) {
				do screen_changed = display_char(y, x++, XL(' '), 1) || screen_changed;
				while (x % TAB_WIDTH && x < width);
			}
			else {
				// cwid=0 => non-spacing char modifying the preceding spacing
				// char
				screen_changed = display_char(y, x-!cwid, c, cwid) || screen_changed;
				x += cwid;
			}
		}
                if (c == XEOF) {
                    break;
                }
	}

	skiptoeof(p);  // avoid SIGPIPE in child
	fclose(p);

	/* harvest child process and get status, propagated from command */
	// TODO: gettext string no longer used
	while (waitpid(child, &status, 0) == -1) {
		if (errno != EINTR)
			return 0x7f;
	}
	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	assert(WIFSIGNALED(status));
	return 0x80 + (WTERMSIG(status) & 0x7f);
}



static void get_terminal_size(void)
{
	static bool height_fixed, width_fixed;

	if (height_fixed && width_fixed)
		return;

	if (! width) {
		width = WIDTH_FALLBACK;
		height = HEIGHT_FALLBACK;
		const char *env;
		char *endptr;
		long t;

		env = getenv("LINES");
		if (env && env[0] >= '0' && env[0] <= '9') {
			errno = 0;
			t = strtol(env, &endptr, 0);
			if (! *endptr && ! errno && t > 0 && t <= INT_MAX) {
				height_fixed = true;
				height = t;
			}
		}
		env = getenv("COLUMNS");
		if (env && env[0] >= '0' && env[0] <= '9') {
			errno = 0;
			t = strtol(env, &endptr, 0);
			if (! *endptr && ! errno && t > 0 && t <= INT_MAX) {
				width_fixed = true;
				width = t;
			}
		}
	}

	struct winsize w;
	if (ioctl(STDERR_FILENO, TIOCGWINSZ, &w) == 0) {
		if (! height_fixed && w.ws_row > 0) {
			static char env_row_buf[24] = "LINES=";
			height = w.ws_row & INT_MAX;
			snprintf(env_row_buf+6, sizeof(env_row_buf)-6, "%d", height);
			putenv(env_row_buf);
		}
		if (! width_fixed && w.ws_col > 0) {
			static char env_col_buf[24] = "COLUMNS=";
			width = w.ws_col & INT_MAX;
			snprintf(env_col_buf+8, sizeof(env_col_buf)-8, "%d", width);
			putenv(env_col_buf);
		}
	}
        height -= 2;

	assert(width > 0 && height > 0);
}



int main(int argc, char *argv[])
{
	int i;
	watch_usec_t interval, last_tick = 0, t;
	long max_cycles = 1, cycle_count = 1;
	fd_set select_stdin;
	uint8_t cmdexit;
	struct timeval tosleep;
	bool sleep_dontsleep, sleep_scrdumped, sleep_exit;
        int height, width;
        WINDOW *hdrwin = NULL;
	const struct option longopts[] = {
		{"color", no_argument, 0, 'c'},
		{"no-color", no_argument, 0, 'C'},
		{"differences", optional_argument, 0, 'd'},
		{"help", no_argument, 0, 'h'},
		{"interval", required_argument, 0, 'n'},
		{"beep", no_argument, 0, 'b'},
		{"errexit", no_argument, 0, 'e'},
		{"follow", no_argument, 0, 'f'},
		{"chgexit", no_argument, 0, 'g'},
		{"equexit", required_argument, 0, 'q'},
		{"exec", no_argument, 0, 'x'},
		{"precise", no_argument, 0, 'p'},
		{"no-rerun", no_argument, 0, 'r'},
		{"shotsdir", required_argument, 0, 's'},
		{"no-title", no_argument, 0, 't'},
		{"no-wrap", no_argument, 0, 'w'},
		{"version", no_argument, 0, 'v'},
		{0}
	};

	atexit(close_stdout);
	setbuf(stdin, NULL);  // for select()
#ifdef HAVE_PROGRAM_INVOCATION_NAME
	program_invocation_name = program_invocation_short_name;
#endif
	// TODO: when !WATCH8BIT, setlocale() should be omitted or initd as "C",
	// shouldn't it? Also, everywhere we rely on the fact that with !8BIT
	// strlen(s) is the col width of s, for instance.
	// Also, the build system doesn't honor WATCH8BIT when linking. On my system
	// it links against libncursesw even when !WATCH8BIT. That results in half
	// of the program working in wchars, half in chars. On the other hand,
	// people with libncursesw.so probably configure with WATCH8BIT.
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

#ifdef WITH_COLORWATCH
	flags |= WATCH_COLOR;
#endif /* WITH_COLORWATCH */

	const char *const interval_string = getenv("WATCH_INTERVAL");
	if (interval_string != NULL)
		interval_real = strtod_nol_or_err(interval_string, _("Could not parse interval from WATCH_INTERVAL"));

	while ((i = getopt_long(argc,argv,"+bCcefd::ghq:n:prs:twvx",longopts,NULL)) != EOF) {
		switch (i) {
		case 'b':
			flags |= WATCH_BEEP;
			break;
		case 'c':
			flags |= WATCH_COLOR;
			break;
		case 'C':
			flags &= ~WATCH_COLOR;
			break;
		case 'd':
			flags |= WATCH_DIFF;
			if (optarg)
				flags |= WATCH_CUMUL;
			break;
		case 'e':
			flags |= WATCH_ERREXIT;
			break;
                case 'f':
                        flags |= WATCH_FOLLOW;
                        break;
		case 'g':
			flags |= WATCH_CHGEXIT;
			break;
		case 'q':
			max_cycles = strtol_or_err(optarg, _("failed to parse argument"));
			if (max_cycles < 1)
				max_cycles = 1;
			flags |= WATCH_EQUEXIT;
			break;
		case 'r':
			flags |= WATCH_NORERUN;
			break;
		case 's':
			shotsdir = optarg;
			break;
		case 't':
			flags |= WATCH_NOTITLE;
			break;
		case 'w':
			flags |= WATCH_NOWRAP;
			break;
		case 'x':
			flags |= WATCH_EXEC;
			break;
		case 'n':
			interval_real = strtod_nol_or_err(optarg, _("failed to parse argument"));
			break;
		case 'p':
			flags |= WATCH_PRECISE;
			break;
		case 'h':
			usage(stdout);
			break;
		case 'v':
			printf(PROCPS_NG_VERSION);
			return EXIT_SUCCESS;
		default:
			usage(stderr);
			break;
		}
	}

	if (optind >= argc)
		usage(stderr);

        if ((flags & WATCH_FOLLOW) && (flags & WATCH_ALL_DIFF)) {
            fprintf(stderr, _("Follow -f option conflicts with change options -d,-e or -q"));
            usage(stderr);
        }
	command_argv = argv + optind;  // for exec*()
	command_len = strlen(argv[optind]);
	command = xmalloc(command_len+1);  // never freed
	memcpy(command, argv[optind++], command_len+1);
	for (; optind < argc; optind++) {
		size_t s = strlen(argv[optind]);
		/* space and \0 */
		command = xrealloc(command, command_len + s + 2);
		command[command_len] = ' ';
		memcpy(command+command_len+1, argv[optind], s);
		/* space then string length */
		command_len += 1 + s;
		command[command_len] = '\0';
	}

	// interval_real must
	// * be >= 0.1 (program design)
	// * fit in time_t (in struct timeval), which may be 32b signed
	// * be <=31 days (limitation of select(), as per POSIX 2001)
	// * fit in watch_usec_t, even when multiplied by USECS_PER_SEC
	if (interval_real < 0.1)
		interval_real = 0.1;
	if (interval_real > 60L * 60 * 24 * 31)
		interval_real = 60L * 60 * 24 * 31;
	interval = (long double)interval_real * USECS_PER_SEC;
	tzset();

	FD_ZERO(&select_stdin);

	// Catch keyboard interrupts so we can put tty back in a sane state.
	signal(SIGINT, die);
	signal(SIGTERM, die);
	signal(SIGHUP, die);
	signal(SIGWINCH, winch_handler);
	/* Set up tty for curses use.  */
	get_terminal_size();
	initscr();  // succeeds or exit()s, may install sig handlers
        getmaxyx(stdscr, height, width);
        if (flags & WATCH_NOTITLE) {
            mainwin = newwin(height, width, 0,0);
        } else {
            mainwin = newwin(height-HEADER_HEIGHT, width, HEADER_HEIGHT,0);
            hdrwin = newwin(HEADER_HEIGHT, width, 0, 0);
        }
        if (flags & WATCH_FOLLOW)
            scrollok(mainwin, TRUE);
	if (flags & WATCH_COLOR) {
		if (has_colors()) {
			start_color();
			use_default_colors();
			init_ansi_colors();
		}
		else flags &= ~WATCH_COLOR;
	}
	nonl();
	noecho();
	cbreak();
	curs_set(0);

	while (1) {
		reset_ansi();
		set_ansi_attribute(-1, NULL);
		if (screen_size_changed) {
			screen_size_changed = false;  // "atomic" test-and-set
			get_terminal_size();
			resizeterm(height, width);
			first_screen = true;
		}

		output_lowheader_pre(hdrwin);
		output_header(hdrwin);
		t = get_time_usec();
		if (flags & WATCH_PRECISE)
			last_tick = t;
		cmdexit = run_command();
		if (flags & WATCH_PRECISE)
			output_lowheader(hdrwin, get_time_usec() - t, cmdexit);
		else {
			last_tick = get_time_usec();
			output_lowheader(hdrwin, last_tick - t, cmdexit);
		}
		wrefresh(hdrwin);

		if (cmdexit) {
			if (flags & WATCH_BEEP)
				beep();  // doesn't require refresh()
			if (flags & WATCH_ERREXIT) {
				// TODO: Hard to see when there's cmd output around it. Add
				// spaces or move to lowheader.
				mvwaddstr(mainwin, height-1, 0, _("command exit with a non-zero status, press a key to exit"));
				i = fcntl(STDIN_FILENO, F_GETFL);
				if (i >= 0 && fcntl(STDIN_FILENO, F_SETFL, i|O_NONBLOCK) >= 0) {
					while (getchar() != EOF) ;
					fcntl(STDIN_FILENO, F_SETFL, i);
				}
				refresh();
				getchar();
				endwin_exit(cmdexit);
			}
		}

		// [BUG] When screen resizes, its contents change, but not
		// necessarily because cmd output's changed. It may have, but that
		// event is lost. Prevents cycle_count from soaring while resizing.
		if (! first_screen) {
			if (flags & WATCH_CHGEXIT && screen_changed)
				break;
			if (flags & WATCH_EQUEXIT) {
				if (screen_changed)
					cycle_count = 1;
				else {
					if (cycle_count == max_cycles)
						break;
					++cycle_count;
				}
			}
		}

		wrefresh(mainwin);  // takes some time
		first_screen = false;

		// first process all available input, then respond to
		// screen_size_changed, then sleep
		sleep_dontsleep = sleep_scrdumped = sleep_exit = false;
		do {
			assert(FD_SETSIZE > STDIN_FILENO);
			FD_SET(STDIN_FILENO, &select_stdin);
			sleep_dontsleep |= screen_size_changed && ! (flags & WATCH_NORERUN);
			if (! sleep_dontsleep && (t=get_time_usec()-last_tick) < interval) {
				tosleep.tv_sec = (interval-t) / USECS_PER_SEC;
				tosleep.tv_usec = (interval-t) % USECS_PER_SEC;
			}
			else memset(&tosleep, 0, sizeof(tosleep));
			i = select(STDIN_FILENO+1, &select_stdin, NULL, NULL, &tosleep);
			assert(i != -1 || errno == EINTR);
			if (i > 0) {
				// all keys idempotent
				switch (getchar()) {
				case EOF:
					if (errno != EINTR)
						endwin_xerr(1, "getchar()");
					break;
				case 'q':
					sleep_dontsleep = sleep_exit = true;
					break;
				case ' ':
					sleep_dontsleep = true;
					break;
				case 's':
					if (! sleep_scrdumped) {
						screenshot();
						sleep_scrdumped = true;
					}
					break;
				}
			}
		} while (i);
		if (sleep_exit)
			break;
	}

	endwin_exit(EXIT_SUCCESS);
}
