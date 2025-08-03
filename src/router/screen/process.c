/* Copyright (c) 2013, 2015
 *      Mike Gerwitz (mtg@gnu.org)
 * Copyright (c) 2010
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Sadrul Habib Chowdhury (sadrul@users.sourceforge.net)
 * Copyright (c) 2008, 2009
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 *      Micah Cowan (micah@cowan.name)
 *      Sadrul Habib Chowdhury (sadrul@users.sourceforge.net)
 * Copyright (c) 1993-2002, 2003, 2005, 2006, 2007
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 * Copyright (c) 1987 Oliver Laumann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, see
 * https://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 */

#include "config.h"

#include "process.h"

#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <time.h>

#include "screen.h"

#include "display.h"
#include "encoding.h"
#include "fileio.h"
#include "help.h"
#include "input.h"
#include "kmapdef.h"
#include "layout.h"
#include "list_generic.h"
#include "logfile.h"
#include "mark.h"
#include "misc.h"
#include "resize.h"
#include "search.h"
#include "socket.h"
#include "telnet.h"
#include "termcap.h"
#include "tty.h"
#include "utmp.h"
#include "viewport.h"
#include "winmsg.h"


static int CheckArgNum(int, char **);
static void ClearAction(struct action *);
static void SaveAction(struct action *, int, char **, int *);
static Window *NextWindow(void);
static Window *PreviousWindow(void);
static Window *ParentWindow(void);
static int MoreWindows(void);
static void CollapseWindowlist(void);
static void LogToggle(bool);
static void ShowInfo(void);
static void ShowDInfo(void);
static Window *WindowByName(char *);
static int WindowByNumber(char *);
static int ParseSwitch(struct action *, bool *);
static int ParseOnOff(struct action *, bool *);
static int ParseWinNum(struct action *, int *);
static int ParseBase(struct action *, char *, int *, int, char *);
static int ParseSaveStr(struct action *, char **);
static int ParseNum(struct action *, int *);
static int ParseNum1000(struct action *, int *);
static char **SaveArgs(char **);
static bool IsNum(char *);
static void ColonFin(char *, size_t, void *);
static void InputSelect(void);
static void InputSetenv(char *);
static void InputAKA(void);
static int InputSu(struct acluser **, char *);
static void suFin(char *, size_t, void *);
static void AKAFin(char *, size_t, void *);
static void copy_reg_fn(char *, size_t, void *);
static void ins_reg_fn(char *, size_t, void *);
static void process_fn(char *, size_t, void *);
static void digraph_fn(char *, size_t, void *);
static int digraph_find(const char *buf);
static void confirm_fn(char *, size_t, void *);
static int IsOnDisplay(Window *);
static void ResizeRegions(char *, int);
static void ResizeFin(char *, size_t, void *);
static struct action *FindKtab(char *, int);
static void SelectFin(char *, size_t, void *);
static void SelectLayoutFin(char *, size_t, void *);
static void ShowWindowsX(char *);

char NullStr[] = "";

struct plop plop_tab[MAX_PLOP_DEFS];

#ifndef PTY_MODE
#define PTY_MODE 0620
#endif

int TtyMode = PTY_MODE;
bool hardcopy_append = false;
bool all_norefresh = 0;
int zmodem_mode = 0;
char *zmodem_sendcmd;
char *zmodem_recvcmd;
static char *zmodes[4] = { "off", "auto", "catch", "pass" };

int idletimo;
struct action idleaction;
char **blankerprg;

struct action ktab[256 + KMAP_KEYS];	/* command key translation table */
struct kclass {
	struct kclass *next;
	char *name;
	struct action ktab[256 + KMAP_KEYS];
};
struct kclass *kclasses;

struct action umtab[KMAP_KEYS + KMAP_AKEYS];
struct action dmtab[KMAP_KEYS + KMAP_AKEYS];
struct action mmtab[KMAP_KEYS + KMAP_AKEYS];
struct kmap_ext *kmap_exts;
int kmap_extn;
int maptimeout = 300;

#ifndef MAX_DIGRAPH
#define MAX_DIGRAPH 512
#endif

struct digraph {
	unsigned char d[2];
	int value;
};

/* digraph table taken from old vim and rfc1345 */
static struct digraph digraphs[MAX_DIGRAPH + 1] = {
	{{' ', ' '}, 160},	/*   */
	{{'N', 'S'}, 160},	/*   */
	{{'~', '!'}, 161},	/* ¡ */
	{{'!', '!'}, 161},	/* ¡ */
	{{'!', 'I'}, 161},	/* ¡ */
	{{'c', '|'}, 162},	/* ¢ */
	{{'c', 't'}, 162},	/* ¢ */
	{{'$', '$'}, 163},	/* £ */
	{{'P', 'd'}, 163},	/* £ */
	{{'o', 'x'}, 164},	/* ¤ */
	{{'C', 'u'}, 164},	/* ¤ */
	{{'C', 'u'}, 164},	/* ¤ */
	{{'E', 'u'}, 164},	/* ¤ */
	{{'Y', '-'}, 165},	/* ¥ */
	{{'Y', 'e'}, 165},	/* ¥ */
	{{'|', '|'}, 166},	/* ¦ */
	{{'B', 'B'}, 166},	/* ¦ */
	{{'p', 'a'}, 167},	/* § */
	{{'S', 'E'}, 167},	/* § */
	{{'"', '"'}, 168},	/* ¨ */
	{{'\'', ':'}, 168},	/* ¨ */
	{{'c', 'O'}, 169},	/* © */
	{{'C', 'o'}, 169},	/* © */
	{{'a', '-'}, 170},	/* ª */
	{{'<', '<'}, 171},	/* « */
	{{'-', ','}, 172},	/* ¬ */
	{{'N', 'O'}, 172},	/* ¬ */
	{{'-', '-'}, 173},	/* ­ */
	{{'r', 'O'}, 174},	/* ® */
	{{'R', 'g'}, 174},	/* ® */
	{{'-', '='}, 175},	/* ¯ */
	{{'\'', 'm'}, 175},	/* ¯ */
	{{'~', 'o'}, 176},	/* ° */
	{{'D', 'G'}, 176},	/* ° */
	{{'+', '-'}, 177},	/* ± */
	{{'2', '2'}, 178},	/* ² */
	{{'2', 'S'}, 178},	/* ² */
	{{'3', '3'}, 179},	/* ³ */
	{{'3', 'S'}, 179},	/* ³ */
	{{'\'', '\''}, 180},	/* ´ */
	{{'j', 'u'}, 181},	/* µ */
	{{'M', 'y'}, 181},	/* µ */
	{{'p', 'p'}, 182},	/* ¶ */
	{{'P', 'I'}, 182},	/* ¶ */
	{{'~', '.'}, 183},	/* · */
	{{'.', 'M'}, 183},	/* · */
	{{',', ','}, 184},	/* ¸ */
	{{'\'', ','}, 184},	/* ¸ */
	{{'1', '1'}, 185},	/* ¹ */
	{{'1', 'S'}, 185},	/* ¹ */
	{{'o', '-'}, 186},	/* º */
	{{'>', '>'}, 187},	/* » */
	{{'1', '4'}, 188},	/* ¼ */
	{{'1', '2'}, 189},	/* ½ */
	{{'3', '4'}, 190},	/* ¾ */
	{{'~', '?'}, 191},	/* ¿ */
	{{'?', '?'}, 191},	/* ¿ */
	{{'?', 'I'}, 191},	/* ¿ */
	{{'A', '`'}, 192},	/* À */
	{{'A', '!'}, 192},	/* À */
	{{'A', '\''}, 193},	/* Á */
	{{'A', '^'}, 194},	/* Â */
	{{'A', '>'}, 194},	/* Â */
	{{'A', '~'}, 195},	/* Ã */
	{{'A', '?'}, 195},	/* Ã */
	{{'A', '"'}, 196},	/* Ä */
	{{'A', ':'}, 196},	/* Ä */
	{{'A', '@'}, 197},	/* Å */
	{{'A', 'A'}, 197},	/* Å */
	{{'A', 'E'}, 198},	/* Æ */
	{{'C', ','}, 199},	/* Ç */
	{{'E', '`'}, 200},	/* È */
	{{'E', '!'}, 200},	/* È */
	{{'E', '\''}, 201},	/* É */
	{{'E', '^'}, 202},	/* Ê */
	{{'E', '>'}, 202},	/* Ê */
	{{'E', '"'}, 203},	/* Ë */
	{{'E', ':'}, 203},	/* Ë */
	{{'I', '`'}, 204},	/* Ì */
	{{'I', '!'}, 204},	/* Ì */
	{{'I', '\''}, 205},	/* Í */
	{{'I', '^'}, 206},	/* Î */
	{{'I', '>'}, 206},	/* Î */
	{{'I', '"'}, 207},	/* Ï */
	{{'I', ':'}, 207},	/* Ï */
	{{'D', '-'}, 208},	/* Ð */
	{{'N', '~'}, 209},	/* Ñ */
	{{'N', '?'}, 209},	/* Ñ */
	{{'O', '`'}, 210},	/* Ò */
	{{'O', '!'}, 210},	/* Ò */
	{{'O', '\''}, 211},	/* Ó */
	{{'O', '^'}, 212},	/* Ô */
	{{'O', '>'}, 212},	/* Ô */
	{{'O', '~'}, 213},	/* Õ */
	{{'O', '?'}, 213},	/* Õ */
	{{'O', '"'}, 214},	/* Ö */
	{{'O', ':'}, 214},	/* Ö */
	{{'/', '\\'}, 215},	/* × */
	{{'*', 'x'}, 215},	/* × */
	{{'O', '/'}, 216},	/* Ø */
	{{'U', '`'}, 217},	/* Ù */
	{{'U', '!'}, 217},	/* Ù */
	{{'U', '\''}, 218},	/* Ú */
	{{'U', '^'}, 219},	/* Û */
	{{'U', '>'}, 219},	/* Û */
	{{'U', '"'}, 220},	/* Ü */
	{{'U', ':'}, 220},	/* Ü */
	{{'Y', '\''}, 221},	/* Ý */
	{{'I', 'p'}, 222},	/* Þ */
	{{'T', 'H'}, 222},	/* Þ */
	{{'s', 's'}, 223},	/* ß */
	{{'s', '"'}, 223},	/* ß */
	{{'a', '`'}, 224},	/* à */
	{{'a', '!'}, 224},	/* à */
	{{'a', '\''}, 225},	/* á */
	{{'a', '^'}, 226},	/* â */
	{{'a', '>'}, 226},	/* â */
	{{'a', '~'}, 227},	/* ã */
	{{'a', '?'}, 227},	/* ã */
	{{'a', '"'}, 228},	/* ä */
	{{'a', ':'}, 228},	/* ä */
	{{'a', 'a'}, 229},	/* å */
	{{'a', 'e'}, 230},	/* æ */
	{{'c', ','}, 231},	/* ç */
	{{'e', '`'}, 232},	/* è */
	{{'e', '!'}, 232},	/* è */
	{{'e', '\''}, 233},	/* é */
	{{'e', '^'}, 234},	/* ê */
	{{'e', '>'}, 234},	/* ê */
	{{'e', '"'}, 235},	/* ë */
	{{'e', ':'}, 235},	/* ë */
	{{'i', '`'}, 236},	/* ì */
	{{'i', '!'}, 236},	/* ì */
	{{'i', '\''}, 237},	/* í */
	{{'i', '^'}, 238},	/* î */
	{{'i', '>'}, 238},	/* î */
	{{'i', '"'}, 239},	/* ï */
	{{'i', ':'}, 239},	/* ï */
	{{'d', '-'}, 240},	/* ð */
	{{'n', '~'}, 241},	/* ñ */
	{{'n', '?'}, 241},	/* ñ */
	{{'o', '`'}, 242},	/* ò */
	{{'o', '!'}, 242},	/* ò */
	{{'o', '\''}, 243},	/* ó */
	{{'o', '^'}, 244},	/* ô */
	{{'o', '>'}, 244},	/* ô */
	{{'o', '~'}, 245},	/* õ */
	{{'o', '?'}, 245},	/* õ */
	{{'o', '"'}, 246},	/* ö */
	{{'o', ':'}, 246},	/* ö */
	{{':', '-'}, 247},	/* ÷ */
	{{'o', '/'}, 248},	/* ø */
	{{'u', '`'}, 249},	/* ù */
	{{'u', '!'}, 249},	/* ù */
	{{'u', '\''}, 250},	/* ú */
	{{'u', '^'}, 251},	/* û */
	{{'u', '>'}, 251},	/* û */
	{{'u', '"'}, 252},	/* ü */
	{{'u', ':'}, 252},	/* ü */
	{{'y', '\''}, 253},	/* ý */
	{{'i', 'p'}, 254},	/* þ */
	{{'t', 'h'}, 254},	/* þ */
	{{'y', '"'}, 255},	/* ÿ */
	{{'y', ':'}, 255},	/* ÿ */
	{{'"', '['}, 196},	/* Ä */
	{{'"', '\\'}, 214},	/* Ö */
	{{'"', ']'}, 220},	/* Ü */
	{{'"', '{'}, 228},	/* ä */
	{{'"', '|'}, 246},	/* ö */
	{{'"', '}'}, 252},	/* ü */
	{{'"', '~'}, 223}	/* ß */
};

#define RESIZE_FLAG_H 1
#define RESIZE_FLAG_V 2
#define RESIZE_FLAG_L 4

static char *resizeprompts[] = {
	"resize # lines: ",
	"resize -h # columns: ",
	"resize -v # lines: ",
	"resize -b # columns: ",
	"resize -l # lines: ",
	"resize -l -h # columns: ",
	"resize -l -v # lines: ",
	"resize -l -b # columns: ",
};

static int parse_input_int(const char *buf, size_t len, int *val)
{
	int x = 0;
	size_t i;
	if (len >= 1 && ((*buf == 'U' && buf[1] == '+') || (*buf == '0' && (buf[1] == 'x' || buf[1] == 'X')))) {
		x = 0;
		for (i = 2; i < len; i++) {
			if (buf[i] >= '0' && buf[i] <= '9')
				x = x * 16 | (buf[i] - '0');
			else if (buf[i] >= 'a' && buf[i] <= 'f')
				x = x * 16 | (buf[i] - ('a' - 10));
			else if (buf[i] >= 'A' && buf[i] <= 'F')
				x = x * 16 | (buf[i] - ('A' - 10));
			else
				return 0;
		}
	} else if (buf[0] == '0') {
		x = 0;
		for (i = 1; i < len; i++) {
			if (buf[i] < '0' || buf[i] > '7')
				return 0;
			x = x * 8 | (buf[i] - '0');
		}
	} else
		return 0;
	*val = x;
	return 1;
}

char *noargs[1];

int enter_window_name_mode = 0;

void InitKeytab(void)
{
	unsigned int i;
	char *argarr[2];

	for (i = 0; i < ARRAY_SIZE(ktab); i++) {
		ktab[i].nr = RC_ILLEGAL;
		ktab[i].args = noargs;
		ktab[i].argl = NULL;
	}
	for (i = 0; i < KMAP_KEYS + KMAP_AKEYS; i++) {
		umtab[i].nr = RC_ILLEGAL;
		umtab[i].args = noargs;
		umtab[i].argl = NULL;
		dmtab[i].nr = RC_ILLEGAL;
		dmtab[i].args = noargs;
		dmtab[i].argl = NULL;
		mmtab[i].nr = RC_ILLEGAL;
		mmtab[i].args = noargs;
		mmtab[i].argl = NULL;
	}
	argarr[1] = NULL;
	for (i = 0; i < NKMAPDEF; i++) {
		if (i + KMAPDEFSTART < T_CAPS)
			continue;
		if (i + KMAPDEFSTART >= T_CAPS + KMAP_KEYS)
			continue;
		if (kmapdef[i] == NULL)
			continue;
		argarr[0] = kmapdef[i];
		SaveAction(dmtab + i + (KMAPDEFSTART - T_CAPS), RC_STUFF, argarr, NULL);
	}
	for (i = 0; i < NKMAPADEF; i++) {
		if (i + KMAPADEFSTART < T_CURSOR)
			continue;
		if (i + KMAPADEFSTART >= T_CURSOR + KMAP_AKEYS)
			continue;
		if (kmapadef[i] == NULL)
			continue;
		argarr[0] = kmapadef[i];
		SaveAction(dmtab + i + (KMAPADEFSTART - T_CURSOR + KMAP_KEYS), RC_STUFF, argarr, NULL);
	}
	for (i = 0; i < NKMAPMDEF; i++) {
		if (i + KMAPMDEFSTART < T_CAPS)
			continue;
		if (i + KMAPMDEFSTART >= T_CAPS + KMAP_KEYS)
			continue;
		if (kmapmdef[i] == NULL)
			continue;
		argarr[0] = kmapmdef[i];
		argarr[1] = NULL;
		SaveAction(mmtab + i + (KMAPMDEFSTART - T_CAPS), RC_STUFF, argarr, NULL);
	}

	ktab['h'].nr = RC_HARDCOPY;
	ktab['z'].nr = ktab[Ctrl('z')].nr = RC_SUSPEND;
	ktab['c'].nr = ktab[Ctrl('c')].nr = RC_SCREEN;
	ktab[' '].nr = ktab[Ctrl(' ')].nr = ktab['n'].nr = ktab[Ctrl('n')].nr = RC_NEXT;
	ktab['N'].nr = RC_NUMBER;
	ktab[Ctrl('h')].nr = ktab[0177].nr = ktab['p'].nr = ktab[Ctrl('p')].nr = RC_PREV;
	ktab['u'].nr = ktab[Ctrl('u')].nr = RC_PARENT;
	{
		char *args[2];
		args[0] = "--confirm";
		args[1] = NULL;
		SaveAction(ktab + 'k', RC_KILL, args, NULL);
		SaveAction(ktab + Ctrl('k'), RC_KILL, args, NULL);
	}
	ktab['l'].nr = ktab[Ctrl('l')].nr = RC_REDISPLAY;
	ktab['w'].nr = ktab[Ctrl('w')].nr = RC_WINDOWS;
	ktab['v'].nr = RC_VERSION;
	ktab[Ctrl('v')].nr = RC_DIGRAPH;
	ktab['q'].nr = ktab[Ctrl('q')].nr = RC_XON;
	ktab['s'].nr = ktab[Ctrl('s')].nr = RC_XOFF;
	ktab['i'].nr = ktab[Ctrl('i')].nr = RC_INFO;
	ktab['m'].nr = ktab[Ctrl('m')].nr = RC_LASTMSG;
	ktab['A'].nr = RC_TITLE;
#if defined(ENABLE_UTMP)
	ktab['L'].nr = RC_LOGIN;
#endif
	ktab[','].nr = RC_LICENSE;
	ktab['W'].nr = RC_WIDTH;
	ktab['.'].nr = RC_DUMPTERMCAP;
	{
		char *args[2];
		args[0] = "--confirm";
		args[1] = NULL;
		SaveAction(ktab + Ctrl('\\'), RC_QUIT, args, NULL);
	}
	ktab['d'].nr = ktab[Ctrl('d')].nr = RC_DETACH;
	{
		char *args[2];
		args[0] = "--confirm";
		args[1] = NULL;
		SaveAction(ktab + 'D', RC_POW_DETACH, args, NULL);
	}
	ktab['r'].nr = ktab[Ctrl('r')].nr = RC_WRAP;
	ktab['f'].nr = ktab[Ctrl('f')].nr = RC_FLOW;
	ktab['C'].nr = RC_CLEAR;
	ktab['Z'].nr = RC_RESET;
	ktab['H'].nr = RC_LOG;
	ktab['M'].nr = RC_MONITOR;
	ktab['?'].nr = RC_HELP;
	ktab['*'].nr = RC_DISPLAYS;
	{
		char *args[2];
		args[0] = "-";
		args[1] = NULL;
		SaveAction(ktab + '-', RC_SELECT, args, NULL);
	}
	for (i = 0; i < 10; i++) {
		char *args[2], arg1[10];
		args[0] = arg1;
		args[1] = NULL;
		sprintf(arg1, "%d", i);
		SaveAction(ktab + '0' + i, RC_SELECT, args, NULL);
	}
	ktab['\''].nr = RC_SELECT;	/* calling a window by name */
	{
		char *args[2];
		args[0] = "-b";
		args[1] = NULL;
		SaveAction(ktab + '"', RC_WINDOWLIST, args, NULL);
	}
	ktab[Ctrl('G')].nr = RC_VBELL;
	ktab[':'].nr = RC_COLON;
	ktab['['].nr = ktab[Ctrl('[')].nr = RC_COPY;
	{
		char *args[2];
		args[0] = ".";
		args[1] = NULL;
		SaveAction(ktab + ']', RC_PASTE, args, NULL);
		SaveAction(ktab + Ctrl(']'), RC_PASTE, args, NULL);
	}
	ktab['{'].nr = RC_HISTORY;
	ktab['}'].nr = RC_HISTORY;
	ktab['>'].nr = RC_WRITEBUF;
	ktab['<'].nr = RC_READBUF;
	ktab['='].nr = RC_REMOVEBUF;
	ktab['x'].nr = ktab[Ctrl('x')].nr = RC_LOCKSCREEN;
	ktab['b'].nr = ktab[Ctrl('b')].nr = RC_BREAK;
	ktab['B'].nr = RC_POW_BREAK;
	ktab['_'].nr = RC_SILENCE;
	ktab['S'].nr = RC_SPLIT;
	ktab['Q'].nr = RC_ONLY;
	ktab['X'].nr = RC_REMOVE;
	ktab['F'].nr = RC_FIT;
	ktab['\t'].nr = RC_FOCUS;
	{
		char *args[2];
		args[0] = "prev";
		args[1] = NULL;
		SaveAction(ktab + T_BACKTAB - T_CAPS + 256, RC_FOCUS, args, NULL);
	}
	{
		char *args[2];
		args[0] = "-v";
		args[1] = NULL;
		SaveAction(ktab + '|', RC_SPLIT, args, NULL);
	}
	/* These come last; they may want overwrite others: */
	if (DefaultEsc >= 0) {
		ClearAction(&ktab[DefaultEsc]);
		ktab[DefaultEsc].nr = RC_OTHER;
	}
	if (DefaultMetaEsc >= 0) {
		ClearAction(&ktab[DefaultMetaEsc]);
		ktab[DefaultMetaEsc].nr = RC_META;
	}

	idleaction.nr = RC_BLANKER;
	idleaction.args = noargs;
	idleaction.argl = NULL;
}

static struct action *FindKtab(char *class, int create)
{
	struct kclass *kp, **kpp;
	int i;

	if (class == NULL)
		return ktab;
	for (kpp = &kclasses; (kp = *kpp) != NULL; kpp = &kp->next)
		if (!strcmp(kp->name, class))
			break;
	if (kp == NULL) {
		if (!create)
			return NULL;
		if (strlen(class) > 80) {
			Msg(0, "Command class name too long.");
			return NULL;
		}
		kp = malloc(sizeof(struct kclass));
		if (kp == NULL) {
			Msg(0, "%s", strnomem);
			return NULL;
		}
		kp->name = SaveStr(class);
		for (i = 0; i < (int)(ARRAY_SIZE(kp->ktab)); i++) {
			kp->ktab[i].nr = RC_ILLEGAL;
			kp->ktab[i].args = noargs;
			kp->ktab[i].argl = NULL;
			kp->ktab[i].quiet = 0;
		}
		kp->next = NULL;
		*kpp = kp;
	}
	return kp->ktab;
}

static void ClearAction(struct action *act)
{
	char **p;

	if (act->nr == RC_ILLEGAL)
		return;
	act->nr = RC_ILLEGAL;
	if (act->args == noargs)
		return;
	for (p = act->args; *p; p++)
		free(*p);
	free((char *)act->args);
	act->args = noargs;
	act->argl = NULL;
}

/*
 * ProcessInput: process input from display and feed it into
 * the layer on canvas D_forecv.
 */

/*
 *  This ProcessInput just does the keybindings and passes
 *  everything else on to ProcessInput2.
 */

void ProcessInput(char *ibuf, size_t ilen)
{
	int ch;
	size_t slen;
	unsigned char *s, *q;
	int i, l;
	char *p;

	if (display == NULL || ilen == 0)
		return;
	if (D_seql)
		evdeq(&D_mapev);
	slen = ilen;
	s = (unsigned char *)ibuf;
	while (ilen-- > 0) {
		ch = *s++;
		if (D_dontmap || !D_nseqs) {
			D_dontmap = 0;
			continue;
		}
		for (;;) {
			if (*D_seqp != ch) {
				l = D_seqp[D_seqp[-D_seql - 1] + 1];
				if (l) {
					D_seqp += l * 2 + 4;
					continue;
				}
				D_mapdefault = 0;
				l = D_seql;
				p = (char *)D_seqp - l;
				D_seql = 0;
				D_seqp = D_kmaps + 3;
				if (l == 0)
					break;
				if ((q = D_seqh) != NULL) {
					D_seqh = NULL;
					i = q[0] << 8 | q[1];
					i &= ~KMAP_NOTIMEOUT;
					if (StuffKey(i))
						ProcessInput2((char *)q + 3, q[2]);
					if (display == NULL)
						return;
					l -= q[2];
					p += q[2];
				} else
					D_dontmap = 1;
				ProcessInput(p, l);
				if (display == NULL)
					return;
				evdeq(&D_mapev);
				continue;
			}
			if (D_seql++ == 0) {
				/* Finish old stuff */
				slen -= ilen + 1;
				if (slen)
					ProcessInput2(ibuf, slen);
				if (display == NULL)
					return;
				D_seqh = NULL;
			}
			ibuf = (char *)s;
			slen = ilen;
			D_seqp++;
			l = D_seql;
			if (l == D_seqp[-l - 1]) {
				if (D_seqp[l] != l) {
					q = D_seqp + 1 + l;
					if (D_kmaps + D_nseqs > q && q[2] > l && !memcmp(D_seqp - l, q + 3, l)) {
						D_seqh = D_seqp - 3 - l;
						D_seqp = q + 3 + l;
						break;
					}
				}
				i = D_seqp[-l - 3] << 8 | D_seqp[-l - 2];
				i &= ~KMAP_NOTIMEOUT;
				p = (char *)D_seqp - l;
				D_seql = 0;
				D_seqp = D_kmaps + 3;
				D_seqh = NULL;
				if (StuffKey(i))
					ProcessInput2(p, l);
				if (display == NULL)
					return;
			}
			break;
		}
	}
	if (D_seql) {
		l = D_seql;
		for (s = D_seqp;; s += i * 2 + 4) {
			if (s[-l - 3] & KMAP_NOTIMEOUT >> 8)
				break;
			if ((i = s[s[-l - 1] + 1]) == 0) {
				SetTimeout(&D_mapev, maptimeout);
				evenq(&D_mapev);
				break;
			}
		}
	}
	ProcessInput2(ibuf, slen);
}

/*
 *  Here only the screen escape commands are handled.
 */

void ProcessInput2(char *ibuf, size_t ilen)
{
	char *s;
	int ch;
	size_t slen;
	struct action *ktabp;

	while (ilen && display) {
		flayer = D_forecv->c_layer;
		fore = D_fore;
		slen = ilen;
		s = ibuf;
		if (!D_ESCseen) {
			while (ilen > 0) {
				if ((unsigned char)*s++ == D_user->u_Esc)
					break;
				ilen--;
			}
			slen -= ilen;
			if (slen)
				DoProcess(fore, &ibuf, &slen, NULL);
			if (ilen == 1) {
				D_ESCseen = ktab;
				WindowChanged(fore, WINESC_ESC_SEEN);
			}
			if (ilen > 0)
				ilen--;
		}
		if (ilen == 0)
			return;
		ktabp = D_ESCseen ? D_ESCseen : ktab;
		if (D_ESCseen) {
			D_ESCseen = NULL;
			WindowChanged(fore, WINESC_ESC_SEEN);
		}
		ch = (unsigned char)*s;

		/*
		 * As users have different esc characters, but a common ktab[],
		 * we fold back the users esc and meta-esc key to the Default keys
		 * that can be looked up in the ktab[]. grmbl. jw.
		 * XXX: make ktab[] a per user thing.
		 */
		if (ch == D_user->u_Esc)
			ch = DefaultEsc;
		else if (ch == D_user->u_MetaEsc)
			ch = DefaultMetaEsc;

		if (ch >= 0)
			DoAction(&ktabp[ch]);
		ibuf = (char *)(s + 1);
		ilen--;
	}
}

void DoProcess(Window *window, char **bufp, size_t *lenp, struct paster *pa)
{
	size_t oldlen;
	Display *d = display;

	/* XXX -> PasteStart */
	if (pa && *lenp > 1 && window && window->w_slowpaste) {
		/* schedule slowpaste event */
		SetTimeout(&window->w_paster.pa_slowev, window->w_slowpaste);
		evenq(&window->w_paster.pa_slowev);
		return;
	}
	while (flayer && *lenp) {
		if (!pa && window && window->w_paster.pa_pastelen && flayer == window->w_paster.pa_pastelayer) {
			WBell(window, visual_bell);
			*bufp += *lenp;
			*lenp = 0;
			display = d;
			return;
		}
		oldlen = *lenp;
		LayProcess(bufp, lenp);
		if (pa && !pa->pa_pastelayer)
			break;	/* flush rest of paste */
		if (*lenp == oldlen) {
			if (pa) {
				display = d;
				return;
			}
			/* We're full, let's beep */
			WBell(window, visual_bell);
			break;
		}
	}
	*bufp += *lenp;
	*lenp = 0;
	display = d;
	if (pa && pa->pa_pastelen == 0)
		FreePaster(pa);
}

int FindCommnr(const char *str)
{
	int x, m, l = 0, r = RC_LAST;
	while (l <= r) {
		m = (l + r) / 2;
		x = strcmp(str, comms[m].name);
		if (x > 0)
			l = m + 1;
		else if (x < 0)
			r = m - 1;
		else
			return m;
	}
	return RC_ILLEGAL;
}

static int CheckArgNum(int nr, char **args)
{
	int i, n;
	static char *argss[] = { "no", "one", "two", "three", "four", "OOPS" };
	static char *orformat[] = {
		"%s: %s: %s argument%s required",
		"%s: %s: %s or %s argument%s required",
		"%s: %s: %s, %s or %s argument%s required",
		"%s: %s: %s, %s, %s or %s argument%s required"
	};

	n = comms[nr].flags & ARGS_MASK;
	for (i = 0; args[i]; i++) ;
	if (comms[nr].flags & ARGS_ORMORE) {
		if (i < n) {
			Msg(0, "%s: %s: at least %s argument%s required",
			    rc_name, comms[nr].name, argss[n], n != 1 ? "s" : "");
			return -1;
		}
	} else if ((comms[nr].flags & ARGS_PLUS1) && (comms[nr].flags & ARGS_PLUS2) && (comms[nr].flags & ARGS_PLUS3)) {
		if (i != n && i != n + 1 && i != n + 2 && i != n + 3) {
			Msg(0, orformat[3], rc_name, comms[nr].name, argss[n],
			    argss[n + 1], argss[n + 2], argss[n + 3], "");
			return -1;
		}
	} else if ((comms[nr].flags & ARGS_PLUS1) && (comms[nr].flags & ARGS_PLUS2)) {
		if (i != n && i != n + 1 && i != n + 2) {
			Msg(0, orformat[2], rc_name, comms[nr].name, argss[n], argss[n + 1], argss[n + 2], "");
			return -1;
		}
	} else if ((comms[nr].flags & ARGS_PLUS1) && (comms[nr].flags & ARGS_PLUS3)) {
		if (i != n && i != n + 1 && i != n + 3) {
			Msg(0, orformat[2], rc_name, comms[nr].name, argss[n], argss[n + 1], argss[n + 3], "");
			return -1;
		}
	} else if ((comms[nr].flags & ARGS_PLUS2) && (comms[nr].flags & ARGS_PLUS3)) {
		if (i != n && i != n + 2 && i != n + 3) {
			Msg(0, orformat[2], rc_name, comms[nr].name, argss[n], argss[n + 2], argss[n + 3], "");
			return -1;
		}
	} else if (comms[nr].flags & ARGS_PLUS1) {
		if (i != n && i != n + 1) {
			Msg(0, orformat[1], rc_name, comms[nr].name, argss[n], argss[n + 1], n != 0 ? "s" : "");
			return -1;
		}
	} else if (comms[nr].flags & ARGS_PLUS2) {
		if (i != n && i != n + 2) {
			Msg(0, orformat[1], rc_name, comms[nr].name, argss[n], argss[n + 2], "s");
			return -1;
		}
	} else if (comms[nr].flags & ARGS_PLUS3) {
		if (i != n && i != n + 3) {
			Msg(0, orformat[1], rc_name, comms[nr].name, argss[n], argss[n + 3], "");
			return -1;
		}
	} else if (i != n) {
		Msg(0, orformat[0], rc_name, comms[nr].name, argss[n], n != 1 ? "s" : "");
		return -1;
	}
	return i;
}

static void StuffFin(char *buf, size_t len, void *data)
{
	(void)data; /* unused */

	if (!flayer)
		return;
	while (len)
		LayProcess(&buf, &len);
}

/* If the command is not 'quieted', then use Msg to output the message. If it's a remote
 * query, then Msg takes care of also outputting the message to the querying client.
 *
 * If we want the command to be quiet, and it's a remote query, then use QueryMsg so that
 * the response does go back to the querying client.
 *
 * If the command is quieted, and it's not a remote query, then just don't print the message.
 */
#define OutputMsg	(!act->quiet ? Msg : queryflag >= 0 ? QueryMsg : Dummy)

static void DoCommandSelect(struct action *act)
{
	char **args = act->args;
	int n;

	if (!*args)
		InputSelect();
	else if (args[0][0] == '-' && !args[0][1]) {
		SetForeWindow(NULL);
		Activate(0);
	} else if (args[0][0] == '.' && !args[0][1]) {
		if (!fore) {
			OutputMsg(0, "select . needs a window");
			queryflag = -1;
		} else {
			SetForeWindow(fore);
			Activate(0);
		}
	} else if (ParseWinNum(act, &n) == 0)
		SwitchWindow(GetWindowByNumber(n));
	else if (queryflag >= 0)
		queryflag = -1;	/* ParseWinNum already prints out an appropriate error message. */

}

static void DoCommandMultiinput(struct action *act)
{
	char **args = act->args;

	if (!*args) {
		if (!fore)
			OutputMsg(0, "multiinput needs a window");
		else
			fore->w_miflag = fore->w_miflag ? 0 : 1;
	} else {
		int n;
		if (ParseWinNum(act, &n) == 0) {
			Window *p = GetWindowByNumber(n);
			if (p)
				p->w_miflag = p->w_miflag ? 0 : 1;
			else
				ShowWindows(n);
		}
	}

}

static void DoCommandDefautonuke(struct action *act)
{
	int msgok = display && !*rc_name;

	if (ParseOnOff(act, &defautonuke) == 0 && msgok)
		OutputMsg(0, "Default autonuke turned %s", defautonuke ? "on" : "off");
	if (display && *rc_name)
		D_auto_nuke = defautonuke;
}


static void DoCommandAutonuke(struct action *act)
{
	int msgok = display && !*rc_name;

	if (ParseOnOff(act, &D_auto_nuke) == 0 && msgok)
		OutputMsg(0, "Autonuke turned %s", D_auto_nuke ? "on" : "off");
}

static void DoCommandDefobuflimit(struct action *act)
{
	int msgok = display && !*rc_name;

	if (ParseNum(act, &defobuflimit) == 0 && msgok)
		OutputMsg(0, "Default limit set to %d", defobuflimit);
	if (display && *rc_name) {
		D_obufmax = defobuflimit;
		D_obuflenmax = D_obuflen - D_obufmax;
	}
}

static void DoCommandObuflimit(struct action *act)
{
	int msgok = display && !*rc_name;
	char **args = act->args;

	if (*args == NULL)
		OutputMsg(0, "Limit is %d, current buffer size is %d", D_obufmax, D_obuflen);
	else if (ParseNum(act, &D_obufmax) == 0 && msgok)
		OutputMsg(0, "Limit set to %d", D_obufmax);
	D_obuflenmax = D_obuflen - D_obufmax;
}

static void DoCommandDumptermcap(struct action *act)
{
	(void)act; /* unused */

	struct acluser *user = display ? D_user : users;

	WriteFile(user, NULL, DUMP_TERMCAP);
}


static void DoCommandHardcopy(struct action *act)
{
	char **args = act->args;
	int mode = DUMP_HARDCOPY;
	char *file = NULL;
	struct acluser *user = display ? D_user : users;

	if (args[0]) {
		if (!strcmp(*args, "-h")) {
			mode = DUMP_SCROLLBACK;
			file = args[1];
		} else if (!strcmp(*args, "--") && args[1])
			file = args[1];
		else
			file = args[0];
	}

	if (args[0] && file == args[0] && args[1]) {
		OutputMsg(0, "%s: hardcopy: too many arguments", rc_name);
		return;
	}
	WriteFile(user, file, mode);
}

static void DoCommandDeflog(struct action *act)
{
	(void)ParseOnOff(act, &nwin_default.Lflag);
}

static void DoCommandLog(struct action *act)
{
	bool b = fore->w_log ? true : false;

	if (ParseSwitch(act, &b) == 0)
		LogToggle(b);
}


static void DoCommandSuspend(struct action *act)
{
	(void)act; /* unused */

	Detach(D_STOP);
}

static void DoCommandNext(struct action *act)
{
	(void)act; /* unused */

	if (MoreWindows())
		SwitchWindow(NextWindow());
}

static void DoCommandPrev(struct action *act)
{
	(void)act; /* unused */

	if (MoreWindows())
		SwitchWindow(PreviousWindow());
}

static void DoCommandParent(struct action *act)
{
	(void)act; /* unused */

	if (MoreWindows()) {
		Window *w = ParentWindow();
		if (w == NULL && fore != NULL)
			Msg(0, "Window has no parent.");
		else
			SwitchWindow(w);
	}
}

static void DoCommandKill(struct action *act)
{
	char **args = act->args;
	char *name;
	int n;

	if (*args) {
		if (!strcmp(*args, "--confirm")) {
			Input(fore->w_pwin ? "Really kill this filter [y/n]" : "Really kill this window [y/n]",
			      1, INP_RAW, confirm_fn, NULL, RC_KILL);
			return;
		} else {
			OutputMsg(0, "usage: kill [--confirm]");
			return;
		}
	}
	n = fore->w_number;
	if (fore->w_pwin) {
		FreePseudowin(fore);
		OutputMsg(0, "Filter removed.");
		return;
	}
	name = SaveStr(fore->w_title);
	KillWindow(fore);
	OutputMsg(0, "Window %d (%s) killed.", n, name);
	if (name)
		free(name);
}

static void DoCommandQuit(struct action *act)
{
	char **args = act->args;

	if (*args) {
		if (!strcmp(*args, "--confirm")) {
			Input("Really quit and kill all your windows [y/n]", 1, INP_RAW, confirm_fn, NULL, RC_QUIT);
			return;
		} else {
			OutputMsg(0, "usage: quit [--confirm]");
			return;
		}
	}
	Finit(0);
	/* does not return */
}

static void DoCommandDetach(struct action *act)
{
	char **args = act->args;

	if (*args && !strcmp(*args, "-h"))
		Hangup();
	else
		Detach(D_DETACH);
}

static void DoCommandPow_detach(struct action *act)
{
	char **args = act->args;

	if (*args) {
		if (!strcmp(*args, "--confirm")) {
			Input("Really detach and send HANGUP to parent process [y/n]", 1, INP_RAW, confirm_fn, NULL, RC_POW_DETACH);
			return;
		} else {
			OutputMsg(0, "usage: pow_detach [--confirm]");
			return;
		}
	}

	Detach(D_POWER);	/* detach and kill Attacher's parent */
}

static void DoCommandZmodem(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;

	if (*args && !strcmp(*args, "sendcmd")) {
		if (args[1]) {
			free(zmodem_sendcmd);
			zmodem_sendcmd = SaveStr(args[1]);
		}
		if (msgok)
			OutputMsg(0, "zmodem sendcmd: %s", zmodem_sendcmd);
		return;
	}
	if (*args && !strcmp(*args, "recvcmd")) {
		if (args[1]) {
			free(zmodem_recvcmd);
			zmodem_recvcmd = SaveStr(args[1]);
		}
		if (msgok)
			OutputMsg(0, "zmodem recvcmd: %s", zmodem_recvcmd);
		return;
	}
	if (*args) {
		int i;
		for (i = 0; i < 4; i++)
			if (!strcmp(zmodes[i], *args))
				break;
		if (i == 4 && !strcmp(*args, "on"))
			i = 1;
		if (i == 4) {
			OutputMsg(0, "usage: zmodem off|auto|catch|pass");
			return;
		}
		zmodem_mode = i;
	}
	if (msgok)
		OutputMsg(0, "zmodem mode is %s", zmodes[zmodem_mode]);
}

static void DoCommandUnbindall(struct action *act)
{
	(void)act; /* unused */

	for (size_t i = 0; i < ARRAY_SIZE(ktab); i++)
		ClearAction(&ktab[i]);
	OutputMsg(0, "Unbound all keys.");
}

static void DoCommandZombie(struct action *act)
{
	char **args = act->args;
	int *argl = act->argl;
	char *s = NULL;

	if (!(s = *args)) {
		ZombieKey_destroy = 0;
		return;
	}
	if (*argl == 0 || *argl > 2) {
		OutputMsg(0, "%s:zombie: one or two characters expected.", rc_name);
		return;
	}
	if (args[1]) {
		if (!strcmp(args[1], "onerror")) {
			ZombieKey_onerror = 1;
		} else {
			OutputMsg(0, "usage: zombie [keys [onerror]]");
			return;
		}
	} else
		ZombieKey_onerror = 0;
	ZombieKey_destroy = args[0][0];
	ZombieKey_resurrect = *argl == 2 ? args[0][1] : 0;
}

static void DoCommandWall(struct action *act)
{
	char **args = act->args;
	char *s = D_user->u_name;

	OutputMsg(0, "%s: %s", s, *args);
}

static void DoCommandAt(struct action *act)
{
	char **args = act->args;
	int *argl = act->argl;
	struct acluser *user = display ? D_user : users;
	char *s;
	size_t n;

	/* where this AT command comes from: */
	if (!user)
		return;
	s = SaveStr(user->u_name);
	/* DO NOT RETURN FROM HERE WITHOUT RESETTING THIS: */
	EffectiveAclUser = user;
	n = strlen(args[0]);
	if (n)
		n--;
	/*
	 * the windows/displays loops are quite dangerous here, take extra
	 * care not to trigger landmines. Things may appear/disappear while
	 * we are walking along.
	 */
	switch (args[0][n]) {
	case '*':	/* user */
		{
			Display *nd;
			struct acluser *u;

			if (!n)
				u = user;
			else {
				for (u = users; u; u = u->u_next) {
					if (!strncmp(*args, u->u_name, n))
						goto out;
				}
				if (!u) {
					args[0][n] = '\0';
					OutputMsg(0, "Did not find any user matching '%s'", args[0]);
					goto out;
				}
			}
			for (display = displays; display; display = nd) {
				nd = display->d_next;
				if (D_forecv == NULL)
					continue;
				flayer = D_forecv->c_layer;
				fore = D_fore;
				if (D_user != u)
					continue;
				DoCommand(args + 1, argl + 1);
				if (display)
					OutputMsg(0, "command from %s: %s %s",
						  s, args[1], args[2] ? args[2] : "");
				display = NULL;
				flayer = NULL;
				fore = NULL;
			}
		}
		break;
	case '%':	/* display */
		{
			Display *nd;

			for (display = displays; display; display = nd) {
				nd = display->d_next;
				if (D_forecv == NULL)
					continue;
				fore = D_fore;
				flayer = D_forecv->c_layer;
				if (strncmp(args[0], D_usertty, n) &&
				    (strncmp("/dev/", D_usertty, 5) ||
				     strncmp(args[0], D_usertty + 5, n)) &&
				    (strncmp("/dev/tty", D_usertty, 8) || strncmp(args[0], D_usertty + 8, n)))
					continue;
				DoCommand(args + 1, argl + 1);
				if (display)
					OutputMsg(0, "command from %s: %s %s",
						  s, args[1], args[2] ? args[2] : "");
				display = NULL;
				fore = NULL;
				flayer = NULL;
			}
		}
		break;
	case '#':	/* window */
		n--;
		/* FALLTHROUGH */
	default:
		{
			int ch;
			int i = 0;

			n++;
			ch = args[0][n];
			args[0][n] = '\0';
			if (!*args[0] || (i = WindowByNumber(args[0])) < 0) {
				args[0][n] = ch;	/* must restore string in case of bind */
				/* try looping over titles */
				for (fore = mru_window; fore; fore = fore->w_prev_mru) {
					if (strncmp(args[0], fore->w_title, n))
						continue;
					/*
					 * consider this a bug or a feature:
					 * while looping through windows, we have fore AND
					 * display context. This will confuse users who try to
					 * set up loops inside of loops, but often allows to do
					 * what you mean, even when you adress your context wrong.
					 */
					i = 0;
					/* XXX: other displays? */
					if (fore->w_layer.l_cvlist)
						display = fore->w_layer.l_cvlist->c_display;
					flayer = fore->w_savelayer ? fore->w_savelayer : &fore->w_layer;
					DoCommand(args + 1, argl + 1);	/* may destroy our display */
					if (fore && fore->w_layer.l_cvlist) {
						display = fore->w_layer.l_cvlist->c_display;
						OutputMsg(0, "command from %s: %s %s",
							  s, args[1], args[2] ? args[2] : "");
					}
				}
				display = NULL;
				fore = NULL;
				if (i < 0)
					OutputMsg(0, "%s: at '%s': no such window.\n", rc_name, args[0]);
				goto out;
			} else if ((fore = GetWindowByNumber(i))) {
				args[0][n] = ch;	/* must restore string in case of bind */
				if (fore->w_layer.l_cvlist)
					display = fore->w_layer.l_cvlist->c_display;
				flayer = fore->w_savelayer ? fore->w_savelayer : &fore->w_layer;
				DoCommand(args + 1, argl + 1);
				if (fore && fore->w_layer.l_cvlist) {
					display = fore->w_layer.l_cvlist->c_display;
					OutputMsg(0, "command from %s: %s %s",
						  s, args[1], args[2] ? args[2] : "");
				}
				display = NULL;
				fore = NULL;
			} else
				OutputMsg(0, "%s: at [identifier][%%|*|#] command [args]", rc_name);
		}
	}
out:
	free(s);
	EffectiveAclUser = NULL;
}

static void DoCommandReadreg(struct action *act)
{
	char **args = act->args;
	int *argl = act->argl;
	int i = fore ? fore->w_encoding : display ? display->d_encoding : 0;
	char ch;
	char *s;
	int n;

	if (args[0] && args[1] && !strcmp(args[0], "-e")) {
		i = FindEncoding(args[1]);
		if (i == -1) {
			OutputMsg(0, "%s: readreg: unknown encoding", rc_name);
			return;
		}
		args += 2;
	}
	/*
	 * Without arguments we prompt for a destination register.
	 * It will receive the copybuffer contents.
	 * This is not done by RC_PASTE, as we prompt for source
	 * (not dest) there.
	 */
	if (*args == NULL) {
		Input("Copy to register:", 1, INP_RAW, copy_reg_fn, NULL, 0);
		return;
	}
	if (*argl != 1) {
		OutputMsg(0, "%s: copyreg: character, ^x, or (octal) \\032 expected.", rc_name);
		return;
	}
	ch = args[0][0];
	/*
	 * With two arguments we *really* read register contents from file
	 */
	if (args[1]) {
		if (args[2]) {
			OutputMsg(0, "%s: readreg: too many arguments", rc_name);
			return;
		}
		if ((s = ReadFile(args[1], &n))) {
			struct plop *pp = plop_tab + (int)(unsigned char)ch;

			if (pp->buf)
				free(pp->buf);
			pp->buf = s;
			pp->len = n;
			pp->enc = i;
		}
	} else {
		/*
		 * with one argument we copy the copybuffer into a specified register
		 * This could be done with RC_PASTE too, but is here to be consistent
		 * with the zero argument call.
		 */
		copy_reg_fn(&ch, 0, NULL);
	}
}

static void DoCommandRegister(struct action *act)
{
	char **args = act->args;
	int *argl = act->argl;
	int i = fore ? fore->w_encoding : display ? display->d_encoding : 0;
	struct acluser *user = display ? D_user : users;
	int argc = CheckArgNum(act->nr, args);
	char ch;

	if (args[0] && args[1] && !strcmp(args[0], "-e")) {
		i = FindEncoding(args[1]);
		if (i == -1) {
			OutputMsg(0, "%s: register: unknown encoding", rc_name);
			return;
		}
		args += 2;
		argc -= 2;
	}
	if (argc != 2) {
		OutputMsg(0, "%s: register: illegal number of arguments.", rc_name);
		return;
	}
	if (*argl != 1) {
		OutputMsg(0, "%s: register: character, ^x, or (octal) \\032 expected.", rc_name);
		return;
	}
	ch = args[0][0];
	if (ch == '.') {
		if (user->u_plop.buf != NULL)
			UserFreeCopyBuffer(user);
		if (args[1] && args[1][0]) {
			user->u_plop.buf = SaveStrn(args[1], argl[1]);
			user->u_plop.len = argl[1];
			user->u_plop.enc = i;
		}
	} else {
		struct plop *plp = plop_tab + (int)(unsigned char)ch;

		if (plp->buf)
			free(plp->buf);
		plp->buf = SaveStrn(args[1], argl[1]);
		plp->len = argl[1];
		plp->enc = i;
	}
}

static void DoCommandProcess(struct action *act)
{
	char **args = act->args;
	int *argl = act->argl;
	char ch;

	if (*args == NULL) {
		Input("Process register:", 1, INP_RAW, process_fn, NULL, 0);
		return;
	}
	if (*argl != 1) {
		OutputMsg(0, "%s: process: character, ^x, or (octal) \\032 expected.", rc_name);
		return;
	}
	ch = args[0][0];
	process_fn(&ch, 0, NULL);

}

static void DoCommandStuff(struct action *act)
{
	char **args = act->args;
	int *argl = act->argl;
	char *s;
	size_t len;

	s = *args;
	if (!args[0]) {
		Input("Stuff:", 100, INP_COOKED, StuffFin, NULL, 0);
		return;
	}
	len = *argl;
	if (args[1]) {
		int i;
		if (strcmp(s, "-k")) {
			OutputMsg(0, "%s: stuff: invalid option %s", rc_name, s);
			return;
		}
		s = args[1];
		for (i = T_CAPS; i < T_OCAPS; i++)
			if (strcmp(term[i].tcname, s) == 0)
				return;
		if (i == T_OCAPS) {
			OutputMsg(0, "%s: stuff: unknown key '%s'", rc_name, s);
			return;
		}
		if (StuffKey(i - T_CAPS) == 0)
			return;
		s = display ? D_tcs[i].str : NULL;
		if (s == NULL)
			return;
		len = strlen(s);
	}
	while (len)
		LayProcess(&s, &len);
}

static void DoCommandRedisplay(struct action *act)
{
	(void)act; /* unused */

	Activate(-1);
}

static void DoCommandWindows(struct action *act)
{
	char **args = act->args;

	if (args[0])
		ShowWindowsX(args[0]);
	else
		ShowWindows(-1);
}

static void DoCommandVersion(struct action *act)
{
	(void)act; /* unused */

	OutputMsg(0, "screen %s", version);
}

static void DoCommandInfo(struct action *act)
{
	(void)act; /* unused */

	ShowInfo();
}

static void DoCommandDinfo(struct action *act)
{
	(void)act; /* unused */

	ShowDInfo();
}

static void DoCommandCommand(struct action *act)
{
	char **args = act->args;
	int argc = CheckArgNum(act->nr, args);
	struct action *ktabp = ktab;

	if (argc == 2 && !strcmp(*args, "-c")) {
		if ((ktabp = FindKtab(args[1], 0)) == NULL) {
			OutputMsg(0, "Unknown command class '%s'", args[1]);
			return;
		}
	}
	if (D_ESCseen != ktab || ktabp != ktab) {
		if (D_ESCseen != ktabp) {
			D_ESCseen = ktabp;
			WindowChanged(fore, WINESC_ESC_SEEN);
		}
		return;
	}
	if (D_ESCseen) {
		D_ESCseen = NULL;
		WindowChanged(fore, WINESC_ESC_SEEN);
	}
	if (MoreWindows())
		SwitchWindow(display && D_other ? D_other : NextWindow());
}

static void DoCommandOther(struct action *act)
{
	(void)act; /* unused */

	if (MoreWindows())
		SwitchWindow(display && D_other ? D_other : NextWindow());
}


static void DoCommandMeta(struct action *act)
{
	struct acluser *user = display ? D_user : users;
	char ch = user->u_Esc;
	char *s = &ch;
	size_t len = 1;

	(void)act; /* unused */

	if (ch != -1)
		LayProcess(&s, &len);
}

static void DoCommandXon(struct action *act)
{
	char ch = Ctrl('q');
	char *s = &ch;
	size_t len = 1;

	(void)act; /* unused */

	LayProcess(&s, &len);
}

static void DoCommandXoff(struct action *act)
{
	char ch = Ctrl('s');
	char *s = &ch;
	size_t len = 1;

	(void)act; /* unused */

	LayProcess(&s, &len);
}

static void DoCommandBreaktype(struct action *act)
{
	static char *types[] = { "TIOCSBRK", "TCSBRK", "tcsendbreak", NULL };
	char **args = act->args;
	char ch;
	int n;

	if (*args) {
		if (ParseNum(act, &n))
			for (n = 0; n < (int)(ARRAY_SIZE(types)); n++) {
				int i;
				for (i = 0; i < 4; i++) {
					ch = args[0][i];
					if (ch >= 'a' && ch <= 'z')
						ch -= 'a' - 'A';
					if (ch != types[n][i] && (ch + ('a' - 'A')) != types[n][i])
						break;
				}
				if (i == 4)
					break;
			}
		if (n < 0 || n >= (int)(ARRAY_SIZE(types)))
			OutputMsg(0, "%s invalid, chose one of %s, %s or %s", *args, types[0], types[1],
				  types[2]);
		else {
			breaktype = n;
			OutputMsg(0, "breaktype set to (%d) %s", n, types[n]);
		}
	}
}


static void DoCommandPow_break(struct action *act)
{
	char **args = act->args;
	int n = 0;

	if (*args && ParseNum(act, &n))
		return;
	SendBreak(fore, n, true);
}

static void DoCommandBreak(struct action *act)
{
	char **args = act->args;
	int n = 0;

	if (*args && ParseNum(act, &n))
		return;
	SendBreak(fore, n, false);
}

static void DoCommandLockscreen(struct action *act)
{
	(void)act; /* unused */

	Detach(D_LOCK);
}

static void DoCommandWidth(struct action *act)
{
	char **args = act->args;
	int w, h;
	int what = 0;

	if (*args && !strcmp(*args, "-w"))
		what = 1;
	else if (*args && !strcmp(*args, "-d"))
		what = 2;
	if (what)
		args++;
	if (what == 0 && flayer && !display)
		what = 1;
	if (what == 1) {
		if (!flayer) {
			OutputMsg(0, "%s: width: window required", rc_name);
			return;
		}
		w = flayer->l_width;
		h = flayer->l_height;
	} else {
		if (!display) {
			OutputMsg(0, "%s: width: display required", rc_name);
			return;
		}
		w = D_width;
		h = D_height;
	}
	if (*args && args[0][0] == '-') {
		OutputMsg(0, "%s: width: unknown option %s", rc_name, *args);
		return;
	}
	if (!*args) {
		if (w == Z0width)
			w = Z1width;
		else if (w == Z1width)
			w = Z0width;
		else if (w > (Z0width + Z1width) / 2)
			w = Z0width;
		else
			w = Z1width;
	} else {
		w = atoi(*args);
		if (args[1])
			h = atoi(args[1]);
	}
	if (*args && args[1] && args[2]) {
		OutputMsg(0, "%s: width: too many arguments", rc_name);
		return;
	}
	if (w <= 0) {
		OutputMsg(0, "Illegal width");
		return;
	}
	if (h <= 0) {
		OutputMsg(0, "Illegal height");
		return;
	}
	if (what == 1) {
		if (flayer->l_width == w && flayer->l_height == h)
			return;
		ResizeLayer(flayer, w, h, NULL);
		return;
	}
	if (D_width == w && D_height == h)
		return;
	if (what == 2) {
		ChangeScreenSize(w, h, 1);
	} else {
		if (ResizeDisplay(w, h) == 0) {
			Activate(D_fore ? D_fore->w_norefresh : 0);
			/* autofit */
			ResizeLayer(D_forecv->c_layer, D_forecv->c_xe - D_forecv->c_xs + 1,
				    D_forecv->c_ye - D_forecv->c_ys + 1, NULL);
			return;
		}
		if (h == D_height)
			OutputMsg(0,
				  "Your termcap does not specify how to change the terminal's width to %d.",
				  w);
		else if (w == D_width)
			OutputMsg(0,
				  "Your termcap does not specify how to change the terminal's height to %d.",
				  h);
		else
			OutputMsg(0,
				  "Your termcap does not specify how to change the terminal's resolution to %dx%d.",
				  w, h);
	}
}

static void DoCommandHeight(struct action *act)
{
	char **args = act->args;
	int w, h;
	int what = 0;

	if (*args && !strcmp(*args, "-w"))
		what = 1;
	else if (*args && !strcmp(*args, "-d"))
		what = 2;
	if (what)
		args++;
	if (what == 0 && flayer && !display)
		what = 1;
	if (what == 1) {
		if (!flayer) {
			OutputMsg(0, "%s: height: window required", rc_name);
			return;
		}
		w = flayer->l_width;
		h = flayer->l_height;
	} else {
		if (!display) {
			OutputMsg(0, "%s: height: display required", rc_name);
			return;
		}
		w = D_width;
		h = D_height;
	}
	if (*args && args[0][0] == '-') {
		OutputMsg(0, "%s: height: unknown option %s", rc_name, *args);
		return;
	}
	if (!*args) {
#define H0height 42
#define H1height 24
		if (h == H0height)
			h = H1height;
		else if (h == H1height)
			h = H0height;
		else if (h > (H0height + H1height) / 2)
			h = H0height;
		else
			h = H1height;
	} else {
		h = atoi(*args);
		if (args[1])
			w = atoi(args[1]);
	}
	if (*args && args[1] && args[2]) {
		OutputMsg(0, "%s: height: too many arguments", rc_name);
		return;
	}
	if (w <= 0) {
		OutputMsg(0, "Illegal width");
		return;
	}
	if (h <= 0) {
		OutputMsg(0, "Illegal height");
		return;
	}
	if (what == 1) {
		if (flayer->l_width == w && flayer->l_height == h)
			return;
		ResizeLayer(flayer, w, h, NULL);
		return;
	}
	if (D_width == w && D_height == h)
		return;
	if (what == 2) {
		ChangeScreenSize(w, h, 1);
	} else {
		if (ResizeDisplay(w, h) == 0) {
			Activate(D_fore ? D_fore->w_norefresh : 0);
			/* autofit */
			ResizeLayer(D_forecv->c_layer, D_forecv->c_xe - D_forecv->c_xs + 1,
				    D_forecv->c_ye - D_forecv->c_ys + 1, NULL);
			return;
		}
		if (h == D_height)
			OutputMsg(0,
				  "Your termcap does not specify how to change the terminal's width to %d.",
				  w);
		else if (w == D_width)
			OutputMsg(0,
				  "Your termcap does not specify how to change the terminal's height to %d.",
				  h);
		else
			OutputMsg(0,
				  "Your termcap does not specify how to change the terminal's resolution to %dx%d.",
				  w, h);
	}
}


static void DoCommandDefdynamictitle(struct action *act)
{
	(void)act; /* unused */

	(void)ParseOnOff(act, &nwin_default.dynamicaka);
}

static void DoCommandDynamictitle(struct action *act)
{
	(void)act; /* unused */

	(void)ParseOnOff(act, &fore->w_dynamicaka);
}

static void DoCommandTitle(struct action *act)
{
	char **args = act->args;

	if (queryflag >= 0) {
		if (fore)
			OutputMsg(0, "%s", fore->w_title);
		else
			queryflag = -1;
		return;
	}
	if (*args == NULL)
		InputAKA();
	else
		ChangeAKA(fore, *args, strlen(*args));
}

static void DoCommandColon(struct action *act)
{
	char **args = act->args;

	Input(":", MAXSTR, INP_EVERY, ColonFin, NULL, 0);
	if (*args && **args) {
		char *s = *args;
		size_t len = strlen(s);
		LayProcess(&s, &len);
	}
}

static void DoCommandLastmsg(struct action *act)
{
	(void)act; /* unused */

	if (D_status_lastmsg)
		OutputMsg(0, "%s", D_status_lastmsg);
}

static void DoCommandScreen(struct action *act)
{
	char **args = act->args;

	DoScreen("key", args);
}


static void DoCommandWrap(struct action *act)
{
	int msgok = display && !*rc_name;

	if (ParseSwitch(act, &fore->w_wrap) == 0 && msgok)
		OutputMsg(0, "%cwrap", fore->w_wrap ? '+' : '-');
}

static void DoCommandFlow(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;
	bool b;

	if (*args) {
		if (args[0][0] == 'a') {
			fore->w_flow =
			    (fore->w_flow & FLOW_AUTO) ? FLOW_AUTOFLAG | FLOW_AUTO | FLOW_ON : FLOW_AUTOFLAG;
		} else 	if (ParseOnOff(act, &b) == 0)
			fore->w_flow = (fore->w_flow & FLOW_AUTO) | b ? FLOW_ON : FLOW_OFF;
	} else {
		if (fore->w_flow & FLOW_AUTOFLAG)
			fore->w_flow = (fore->w_flow & FLOW_AUTO) | FLOW_ON;
		else if (fore->w_flow & FLOW_ON)
			fore->w_flow &= ~FLOW_ON;
		else
			fore->w_flow = fore->w_flow ? FLOW_AUTOFLAG | FLOW_AUTO | FLOW_ON : FLOW_AUTOFLAG;
	}
	SetFlow(fore->w_flow & FLOW_ON);
	if (msgok)
		OutputMsg(0, "%cflow%s", (fore->w_flow & FLOW_ON) ? '+' : '-',
			  (fore->w_flow & FLOW_AUTOFLAG) ? "(auto)" : "");
}

static void DoCommandDefwritelock(struct action *act)
{
	char **args = act->args;
	bool b;

	if (args[0][0] == 'a')
		nwin_default.wlock = WLOCK_AUTO;
	else if (ParseOnOff(act, &b) == 0)
		nwin_default.wlock = b ? WLOCK_ON : WLOCK_OFF;
}

static void DoCommandWritelock(struct action *act)
{
	char **args = act->args;
	bool b;

	if (*args) {
		if (args[0][0] == 'a') {
			fore->w_wlock = WLOCK_AUTO;
		} else {
			if (ParseOnOff(act, &b))
				return;
			fore->w_wlock = b ? WLOCK_ON : WLOCK_OFF;
		}
		/* 
		 * user may have permission to change the writelock setting, 
		 * but he may never aquire the lock himself without write permission
		 */
		if (!AclCheckPermWin(D_user, ACL_WRITE, fore))
			fore->w_wlockuser = D_user;
	}
	OutputMsg(0, "writelock %s", (fore->w_wlock == WLOCK_AUTO) ? "auto" :
		  ((fore->w_wlock == WLOCK_OFF) ? "off" : "on"));
}

static void DoCommandClear(struct action *act)
{
	(void)act; /* unused */

	ResetAnsiState(fore);
	WriteString(fore, "\033[H\033[J", 6);
}

static void DoCommandReset(struct action *act)
{
	(void)act; /* unused */

	ResetAnsiState(fore);
	if (fore->w_zdisplay)
		zmodem_abort(fore, fore->w_zdisplay);
	WriteString(fore, "\033c", 2);
}

static void DoCommandMonitor(struct action *act)
{
	bool b = fore->w_monitor != MON_OFF;
	int i;

	if (display)
		b = b && (ACLBYTE(fore->w_mon_notify, D_user->u_id) & ACLBIT(D_user->u_id));
	if (ParseSwitch(act, &b))
		return;
	if (b) {
		if (display)	/* we tell only this user */
			ACLBYTE(fore->w_mon_notify, D_user->u_id) |= ACLBIT(D_user->u_id);
		else
			for (i = 0; i < maxusercount; i++)
				ACLBYTE(fore->w_mon_notify, i) |= ACLBIT(i);
		if (fore->w_monitor == MON_OFF)
			fore->w_monitor = MON_ON;
		OutputMsg(0, "Window %d (%s) is now being monitored for all activity.", fore->w_number,
			  fore->w_title);
	} else {
		if (display)	/* we remove only this user */
			ACLBYTE(fore->w_mon_notify, D_user->u_id)
			    &= ~ACLBIT(D_user->u_id);
		else
			for (i = 0; i < maxusercount; i++)
				ACLBYTE(fore->w_mon_notify, i) &= ~ACLBIT(i);
		for (i = maxusercount - 1; i >= 0; i--)
			if (ACLBYTE(fore->w_mon_notify, i))
				return;
		if (i < 0)
			fore->w_monitor = MON_OFF;
		OutputMsg(0, "Window %d (%s) is no longer being monitored for activity.", fore->w_number,
			  fore->w_title);
	}
}

static void DoCommandDisplays(struct action *act)
{
	(void)act; /* unused */

	display_displays();
}

static void DoCommandWindowlist(struct action *act)
{
	char **args = act->args;
	int argc = CheckArgNum(act->nr, args);
	int msgok = display && !*rc_name;

	if (!*args)
		display_windows(0, WLIST_NUM, NULL);
	else if (!strcmp(*args, "string")) {
		if (args[1]) {
			if (wliststr)
				free(wliststr);
			wliststr = SaveStr(args[1]);
		}
		if (msgok)
			OutputMsg(0, "windowlist string is '%s'", wliststr);
	} else if (!strcmp(*args, "title")) {
		if (args[1]) {
			if (wlisttit)
				free(wlisttit);
			wlisttit = SaveStr(args[1]);
		}
		if (msgok)
			OutputMsg(0, "windowlist title is '%s'", wlisttit);
	} else {
		int flag = 0;
		int blank = 0;
		int i;
		for (i = 0; i < argc; i++)
			if (!args[i])
				continue;
			else if (!strcmp(args[i], "-m"))
				flag |= WLIST_MRU;
			else if (!strcmp(args[i], "-b"))
				blank = 1;
			else if (!strcmp(args[i], "-g"))
				flag |= WLIST_NESTED;
			else {
				OutputMsg(0,
					  "usage: windowlist [-b] [-g] [-m] [string [string] | title [title]]");
				return;
			}
		if (i == argc)
			display_windows(blank, flag, NULL);
	}
}

static void DoCommandHelp(struct action *act)
{
	char **args = act->args;
	int argc = CheckArgNum(act->nr, args);

	if (argc == 2 && !strcmp(*args, "-c")) {
		struct action *ktabp;
		if ((ktabp = FindKtab(args[1], 0)) == NULL) {
			OutputMsg(0, "Unknown command class '%s'", args[1]);
			return;
		}
		display_help(args[1], ktabp);
	} else
		display_help(NULL, ktab);
}

static void DoCommandLicense(struct action *act)
{
	(void)act; /* unused */

	display_license();
}


static void DoCommandCopy(struct action *act)
{
	(void)act; /* unused */

	if (flayer->l_layfn != &WinLf) {
		OutputMsg(0, "Must be on a window layer");
		return;
	}
	MarkRoutine();
	WindowChanged(fore, WINESC_COPY_MODE);
}

static void DoCommandHistory(struct action *act)
{
	static char *pasteargs[] = { ".", NULL };
	static int pasteargl[] = { 1 };
	struct acluser *user = display ? D_user : users;
	char **args;
	int *argl;
	int enc = -1;
	size_t l = 0;
	char *s, *ss, *dbuf;
	char ch, dch;

	if (flayer->l_layfn != &WinLf) {
		OutputMsg(0, "Must be on a window layer");
		return;
	}
	if (GetHistory() == 0)
		return;
	if (user->u_plop.buf == NULL)
		return;

	args = pasteargs;
	argl = pasteargl;

	/*
	 * without args we prompt for one(!) register to be pasted in the window
	 */
	if ((s = *args) == NULL) {
		Input("Paste from register:", 1, INP_RAW, ins_reg_fn, NULL, 0);
		return;
	}
	if (args[1] == NULL && !fore)	/* no window? */
		return;
	/*
	 * with two arguments we paste into a destination register
	 * (no window needed here).
	 */
	if (args[1] && argl[1] != 1) {
		OutputMsg(0, "%s: paste destination: character, ^x, or (octal) \\032 expected.",
			  rc_name);
		return;
	} else if (fore)
		enc = fore->w_encoding;

	/*
	 * measure length of needed buffer
	 */
	for (ss = s = *args; (ch = *ss); ss++) {
		if (ch == '.') {
			if (enc == -1)
				enc = user->u_plop.enc;
			if (enc != user->u_plop.enc)
				l += RecodeBuf((unsigned char *)user->u_plop.buf, user->u_plop.len,
					       user->u_plop.enc, enc, NULL);
			else
				l += user->u_plop.len;
		} else {
			if (enc == -1)
				enc = plop_tab[(int)(unsigned char)ch].enc;
			if (enc != plop_tab[(int)(unsigned char)ch].enc)
				l += RecodeBuf((unsigned char *)plop_tab[(int)(unsigned char)ch].buf,
					       plop_tab[(int)(unsigned char)ch].len,
					       plop_tab[(int)(unsigned char)ch].enc, enc,
					       NULL);
			else
				l += plop_tab[(int)(unsigned char)ch].len;
		}
	}
	if (l == 0) {
		OutputMsg(0, "empty buffer");
		return;
	}
	/*
	 * shortcut:
	 * if there is only one source and the destination is a window, then
	 * pass a pointer rather than duplicating the buffer.
	 */
	if (s[1] == 0 && args[1] == NULL)
		if (enc == (*s == '.' ? user->u_plop.enc : plop_tab[(int)(unsigned char)*s].enc)) {
			MakePaster(&fore->w_paster,
				   *s == '.' ? user->u_plop.buf : plop_tab[(int)(unsigned char)*s].buf,
				   l, 0);
			return;
		}
	/*
	 * if no shortcut, we construct a buffer
	 */
	if ((dbuf = malloc(l)) == NULL) {
		OutputMsg(0, "%s", strnomem);
		return;
	}
	l = 0;
	/*
	 * concatenate all sources into our own buffer, copy buffer is
	 * special and is skipped if no display exists.
	 */
	for (ss = s; (ch = *ss); ss++) {
		struct plop *pp = (ch == '.' ? &user->u_plop : &plop_tab[(int)(unsigned char)ch]);
		if (pp->enc != enc) {
			l += RecodeBuf((unsigned char *)pp->buf, pp->len, pp->enc, enc,
				       (unsigned char *)dbuf + l);
			continue;
		}
		memmove(dbuf + l, pp->buf, pp->len);
		l += pp->len;
	}
	/*
	 * when called with one argument we paste our buffer into the window
	 */
	if (args[1] == NULL) {
		MakePaster(&fore->w_paster, dbuf, l, 1);
	} else {
		/*
		 * we have two arguments, the second is already in dch.
		 * use this as destination rather than the window.
		 */
		dch = args[1][0];
		if (dch == '.') {
			if (user->u_plop.buf != NULL)
				UserFreeCopyBuffer(user);
			user->u_plop.buf = dbuf;
			user->u_plop.len = l;
			user->u_plop.enc = enc;
		} else {
			struct plop *pp = plop_tab + (int)(unsigned char)dch;
			if (pp->buf)
				free(pp->buf);
			pp->buf = dbuf;
			pp->len = l;
			pp->enc = enc;
		}
	}
}

static void DoCommandPaste(struct action *act)
{
	struct acluser *user = display ? D_user : users;
	char **args = act->args;
	int *argl = act->argl;
	int enc = -1;
	size_t l = 0;
	char *s, *ss, *dbuf;
	char ch, dch;

	/*
	 * without args we prompt for one(!) register to be pasted in the window
	 */
	if ((s = *args) == NULL) {
		Input("Paste from register:", 1, INP_RAW, ins_reg_fn, NULL, 0);
		return;
	}
	if (args[1] == NULL && !fore)	/* no window? */
		return;
	/*
	 * with two arguments we paste into a destination register
	 * (no window needed here).
	 */
	if (args[1] && argl[1] != 1) {
		OutputMsg(0, "%s: paste destination: character, ^x, or (octal) \\032 expected.",
			  rc_name);
		return;
	} else if (fore)
		enc = fore->w_encoding;

	/*
	 * measure length of needed buffer
	 */
	for (ss = s = *args; (ch = *ss); ss++) {
		if (ch == '.') {
			if (enc == -1)
				enc = user->u_plop.enc;
			if (enc != user->u_plop.enc)
				l += RecodeBuf((unsigned char *)user->u_plop.buf, user->u_plop.len,
					       user->u_plop.enc, enc, NULL);
			else
				l += user->u_plop.len;
		} else {
			if (enc == -1)
				enc = plop_tab[(int)(unsigned char)ch].enc;
			if (enc != plop_tab[(int)(unsigned char)ch].enc)
				l += RecodeBuf((unsigned char *)plop_tab[(int)(unsigned char)ch].buf,
					       plop_tab[(int)(unsigned char)ch].len,
					       plop_tab[(int)(unsigned char)ch].enc, enc,
					       NULL);
			else
				l += plop_tab[(int)(unsigned char)ch].len;
		}
	}
	if (l == 0) {
		OutputMsg(0, "empty buffer");
		return;
	}
	/*
	 * shortcut:
	 * if there is only one source and the destination is a window, then
	 * pass a pointer rather than duplicating the buffer.
	 */
	if (s[1] == 0 && args[1] == NULL)
		if (enc == (*s == '.' ? user->u_plop.enc : plop_tab[(int)(unsigned char)*s].enc)) {
			MakePaster(&fore->w_paster,
				   *s == '.' ? user->u_plop.buf : plop_tab[(int)(unsigned char)*s].buf,
				   l, 0);
			return;
		}
	/*
	 * if no shortcut, we construct a buffer
	 */
	if ((dbuf = malloc(l)) == NULL) {
		OutputMsg(0, "%s", strnomem);
		return;
	}
	l = 0;
	/*
	 * concatenate all sources into our own buffer, copy buffer is
	 * special and is skipped if no display exists.
	 */
	for (ss = s; (ch = *ss); ss++) {
		struct plop *pp = (ch == '.' ? &user->u_plop : &plop_tab[(int)(unsigned char)ch]);
		if (pp->enc != enc) {
			l += RecodeBuf((unsigned char *)pp->buf, pp->len, pp->enc, enc,
				       (unsigned char *)dbuf + l);
			continue;
		}
		memmove(dbuf + l, pp->buf, pp->len);
		l += pp->len;
	}
	/*
	 * when called with one argument we paste our buffer into the window
	 */
	if (args[1] == NULL) {
		MakePaster(&fore->w_paster, dbuf, l, 1);
	} else {
		/*
		 * we have two arguments, the second is already in dch.
		 * use this as destination rather than the window.
		 */
		dch = args[1][0];
		if (dch == '.') {
			if (user->u_plop.buf != NULL)
				UserFreeCopyBuffer(user);
			user->u_plop.buf = dbuf;
			user->u_plop.len = l;
			user->u_plop.enc = enc;
		} else {
			struct plop *pp = plop_tab + (int)(unsigned char)dch;
			if (pp->buf)
				free(pp->buf);
			pp->buf = dbuf;
			pp->len = l;
			pp->enc = enc;
		}
	}
}

static void DoCommandWritebuf(struct action *act)
{
	struct acluser *user = display ? D_user : users;
	char **args = act->args;
	int enc = -1;
	size_t l = 0;
	char *newbuf;
	struct plop oldplop;

	if (!user->u_plop.buf) {
		OutputMsg(0, "empty buffer");
		return;
	}

	oldplop = user->u_plop;
	if (args[0] && args[1] && !strcmp(args[0], "-e")) {
		enc = FindEncoding(args[1]);
		if (enc == -1) {
			OutputMsg(0, "%s: writebuf: unknown encoding", rc_name);
			return;
		}
		if (enc != oldplop.enc) {
			l = RecodeBuf((unsigned char *)oldplop.buf, oldplop.len, oldplop.enc, enc,
				      NULL);
			newbuf = malloc(l + 1);
			if (!newbuf) {
				OutputMsg(0, "%s", strnomem);
				return;
			}
			user->u_plop.len =
			    RecodeBuf((unsigned char *)oldplop.buf, oldplop.len, oldplop.enc, enc,
				      (unsigned char *)newbuf);
			user->u_plop.buf = newbuf;
			user->u_plop.enc = enc;
		}
		args += 2;
	}
	if (args[0] && args[1])
		OutputMsg(0, "%s: writebuf: too many arguments", rc_name);
	else
		WriteFile(user, args[0], DUMP_EXCHANGE);
	if (user->u_plop.buf != oldplop.buf)
		free(user->u_plop.buf);
	user->u_plop = oldplop;
}

static void DoCommandReadbuf(struct action *act)
{
	struct acluser *user = display ? D_user : users;
	char **args = act->args;
	char *s;
	int i;
	int l = 0;

	i = fore ? fore->w_encoding : display ? display->d_encoding : 0;
	if (args[0] && args[1] && !strcmp(args[0], "-e")) {
		i = FindEncoding(args[1]);
		if (i == -1) {
			OutputMsg(0, "%s: readbuf: unknown encoding", rc_name);
			return;
		}
		args += 2;
	}
	if (args[0] && args[1]) {
		OutputMsg(0, "%s: readbuf: too many arguments", rc_name);
		return;
	}
	if ((s = ReadFile(args[0] ? args[0] : BufferFile, &l))) {
		if (user->u_plop.buf)
			UserFreeCopyBuffer(user);
		user->u_plop.len = l;
		user->u_plop.buf = s;
		user->u_plop.enc = i;
		OutputMsg(0, "Read contents of %s into copybuffer",
                          args[0] ? args[0] : BufferFile);
	}
}

static void DoCommandRemovebuf(struct action *act)
{
	(void)act; /* unused */

	KillBuffers();
}

static void DoCommandIgnorecase(struct action *act)
{
	int msgok = display && !*rc_name;

	(void)ParseSwitch(act, &search_ic);
	if (msgok)
		OutputMsg(0, "Will %signore case in searches", search_ic ? "" : "not ");
}

static void DoCommandEscape(struct action *act)
{
	char **args = act->args;
	int *argl = act->argl;
	struct acluser *user = display ? D_user : users;

	if (*argl == 0)
		SetEscape(user, -1, -1);
	else if (*argl == 2)
		SetEscape(user, (int)(unsigned char)args[0][0], (int)(unsigned char)args[0][1]);
	else {
		OutputMsg(0, "%s: two characters required after escape.", rc_name);
		return;
	}
	/* Change defescape if master user. This is because we only
	 * have one ktab.
	 */
	if (display && user != users)
		return;
	if (*argl == 0)
		SetEscape(NULL, -1, -1);
	else if (*argl == 2)
		SetEscape(NULL, (int)(unsigned char)args[0][0], (int)(unsigned char)args[0][1]);
	CheckEscape();
}

static void DoCommandDefescape(struct action *act)
{
	char **args = act->args;
	int *argl = act->argl;

	if (*argl == 0)
		SetEscape(NULL, -1, -1);
	else if (*argl == 2)
		SetEscape(NULL, (int)(unsigned char)args[0][0], (int)(unsigned char)args[0][1]);
	else {
		OutputMsg(0, "%s: two characters required after defescape.", rc_name);
		return;
	}
	CheckEscape();
}

static void DoCommandChdir(struct action *act)
{
	char **args = act->args;
	char *s;

	s = *args ? *args : home;
	if (chdir(s) == -1)
		OutputMsg(errno, "%s", s);
}

static void DoCommandShell(struct action *act)
{
	if (ParseSaveStr(act, &ShellProg) == 0)
		ShellArgs[0] = ShellProg;
}

static void DoCommandHardcopydir(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;

	if (*args)
		(void)ParseSaveStr(act, &hardcopydir);
	if (msgok)
		OutputMsg(0, "hardcopydir is %s\n", hardcopydir && *hardcopydir ? hardcopydir : "<cwd>");
}

static void DoCommandLogfile(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;

	if (*args) {
		char buf[MAXPATHLEN];
		if (args[1] && !(strcmp(*args, "flush"))) {
			log_flush = atoi(args[1]);
			if (msgok)
				OutputMsg(0, "log flush timeout set to %ds\n", log_flush);
			return;
		}
		if (ParseSaveStr(act, &screenlogfile))
			return;
		if (fore && fore->w_log)
			if (DoStartLog(fore, buf, ARRAY_SIZE(buf)))
				OutputMsg(0, "Error opening logfile \"%s\"", buf);
		if (!msgok)
			return;
	}
	OutputMsg(0, "logfile is '%s'", screenlogfile);
}

static void DoCommandLogtstamp(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;

	if (!*args || !strcmp(*args, "on") || !strcmp(*args, "off")) {
		if (ParseSwitch(act, &logtstamp_on) == 0 && msgok)
			OutputMsg(0, "timestamps turned %s", logtstamp_on ? "on" : "off");
	} else if (!strcmp(*args, "string")) {
		if (args[1]) {
			if (logtstamp_string)
				free(logtstamp_string);
			logtstamp_string = SaveStr(args[1]);
		}
		if (msgok)
			OutputMsg(0, "logfile timestamp is '%s'", logtstamp_string);
	} else if (!strcmp(*args, "after")) {
		if (args[1]) {
			logtstamp_after = atoi(args[1]);
			if (!msgok)
				return;
		}
		OutputMsg(0, "timestamp printed after %ds\n", logtstamp_after);
	} else
		OutputMsg(0, "usage: logtstamp [after [n]|string [str]|on|off]");
}

static void DoCommandShelltitle(struct action *act)
{
	(void)ParseSaveStr(act, &nwin_default.aka);
}

static void DoCommandTerminfo(struct action *act)
{
	(void)act; /* unused */

	if (!rc_name || !*rc_name)
		OutputMsg(0, "Sorry, too late now. Place that in your .screenrc file.");
}

static void DoCommandSleep(struct action *act)
{
	(void)act; /* unused */
}

static void DoCommandTerm(struct action *act)
{
	char *s = NULL;

	if (ParseSaveStr(act, &s))
		return;
	if (strlen(s) > MAXTERMLEN) {
		OutputMsg(0, "%s: term: argument too long ( < %d)", rc_name, MAXTERMLEN);
		free(s);
		return;
	}
	strncpy(screenterm, s, MAXTERMLEN);
	screenterm[MAXTERMLEN] = '\0';
	free(s);
	MakeTermcap((display == NULL));
}

static void DoCommandEcho(struct action *act)
{
	char **args = act->args;
	int argc = CheckArgNum(act->nr, args);
	int msgok = display && !*rc_name;
	char *s = NULL;

	if (!msgok && (!rc_name || strcmp(rc_name, "-X")))
		return;
	/*
	 * user typed ^A:echo... well, echo isn't FinishRc's job,
	 * but as he wanted to test us, we show good will
	 */
	if (argc > 1 && !strcmp(*args, "-n")) {
		args++;
		argc--;
	}
	s = *args;
	if (argc > 1 && !strcmp(*args, "-p")) {
		args++;
		argc--;
		s = *args;
		if (s)
			s = MakeWinMsg(s, fore, '%');
	}
	if (s)
		OutputMsg(0, "%s", s);
	else {
		OutputMsg(0, "%s: 'echo [-n] [-p] \"string\"' expected.", rc_name);
		queryflag = -1;
	}
}

static void DoCommandBell(struct action *act)
{
	char **args = act->args;

	if (*args == NULL) {
		char buf[256];
		AddXChars(buf, ARRAY_SIZE(buf), BellString);
		OutputMsg(0, "bell_msg is '%s'", buf);
		return;
	}
	(void)ParseSaveStr(act, &BellString);
}

static void DoCommandBufferfile(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;

	if (*args == NULL)
		BufferFile = SaveStr(DEFAULT_BUFFERFILE);
	else if (ParseSaveStr(act, &BufferFile))
		return;
	if (msgok)
		OutputMsg(0, "Bufferfile is now '%s'", BufferFile);
}

static void DoCommandActivity(struct action *act)
{
	(void)ParseSaveStr(act, &ActivityString);
}

static void DoCommandPow_detach_msg(struct action *act)
{
	char **args = act->args;

	if (*args == NULL) {
		char buf[256];
		AddXChars(buf, ARRAY_SIZE(buf), PowDetachString);
		OutputMsg(0, "pow_detach_msg is '%s'", buf);
		return;
	}
	(void)ParseSaveStr(act, &PowDetachString);
}

#if defined(ENABLE_UTMP) && defined(LOGOUTOK)
static void DoCommandLogin(struct action *act)
{
	char **args = act->args;
	bool b = fore->w_slot != (slot_t)(-1);

	if (*args && !strcmp(*args, "always")) {
		fore->w_lflag = 3;
		if (!displays && b)
			SlotToggle(b);
		return;
	}
	if (*args && !strcmp(*args, "attached")) {
		fore->w_lflag = 1;
		if (!displays && b)
			SlotToggle(0);
		return;
	}
	if (ParseSwitch(act, &b) == 0)
		SlotToggle(b);
}

static void DoCommandDeflogin(struct action *act)
{
	char **args = act->args;
	bool b;

	if (!strcmp(*args, "always"))
		nwin_default.lflag |= 2;
	else if (!strcmp(*args, "attached"))
		nwin_default.lflag &= ~2;
	else if (ParseOnOff(act, &b) == 0)
		nwin_default.lflag = b ? 1 : 0;
}

#endif

static void DoCommandDefflow(struct action *act)
{
	char **args = act->args;
	bool b;

	if (args[0] && args[1] && args[1][0] == 'i') {
		iflag = true;
		for (display = displays; display; display = display->d_next) {
			if (!D_flow)
				continue;
			D_NewMode.tio.c_cc[VINTR] = D_OldMode.tio.c_cc[VINTR];
			D_NewMode.tio.c_lflag |= ISIG;
			SetTTY(D_userfd, &D_NewMode);
		}
	}
	if (args[0] && args[0][0] == 'a')
		nwin_default.flowflag = FLOW_AUTOFLAG;
	else if (ParseOnOff(act, &b) == 0)
		nwin_default.flowflag = b ? FLOW_ON : FLOW_OFF;
}

static void DoCommandDefwrap(struct action *act)
{
	(void)ParseOnOff(act, &nwin_default.wrap);
}

static void DoCommandDefc1(struct action *act)
{
	(void)ParseOnOff(act, &nwin_default.c1);
}

static void DoCommandDefbce(struct action *act)
{
	bool b;

	if (ParseOnOff(act, &b) == 0)
		nwin_default.bce = b ? 1 : 0;
}

static void DoCommandDefgr(struct action *act)
{
	bool b;

	if (ParseOnOff(act, &b) == 0)
		nwin_default.gr = b ? 1 : 0;
}

static void DoCommandDefmonitor(struct action *act)
{
	bool b;

	if (ParseOnOff(act, &b) == 0)
		nwin_default.monitor = b ? MON_ON : MON_OFF;
}

static void DoCommandDefmousetrack(struct action *act)
{
	bool b;

	if (ParseOnOff(act, &b) == 0)
		defmousetrack = b ? 1000 : 0;
}

static void DoCommandMousetrack(struct action *act)
{
	char **args = act->args;
	bool b;

	if (!args[0]) {
		OutputMsg(0, "Mouse tracking for this display is turned %s", D_mousetrack ? "on" : "off");
	} else if (ParseOnOff(act, &b) == 0) {
		D_mousetrack = b ? 1000 : 0;
		if (D_fore)
			MouseMode(D_fore->w_mouse);
	}
}

static void DoCommandDefsilence(struct action *act)
{
	bool b;

	if (ParseOnOff(act, &b) == 0)
		nwin_default.silence = b ? SILENCE_ON : SILENCE_OFF;
}

static void DoCommandVerbose(struct action *act)
{
	char **args = act->args;

	if (!*args)
		OutputMsg(0, "W%s echo command when creating windows.", VerboseCreate ? "ill" : "on't");
	else
		(void)ParseOnOff(act, &VerboseCreate);
}

static void DoCommandHardstatus(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;
	char *s = NULL;

	if (display) {
		OutputMsg(0, "%s", "");	/* wait till mintime (keep gcc quiet) */
		RemoveStatus();
	}
	if (args[0] && strcmp(args[0], "on") && strcmp(args[0], "off")) {
		Display *olddisplay = display;
		int old_use, new_use = -1;

		s = args[0];
		if (!strncmp(s, "always", 6))
			s += 6;
		if (!strcmp(s, "firstline"))
			new_use = HSTATUS_FIRSTLINE;
		else if (!strcmp(s, "lastline"))
			new_use = HSTATUS_LASTLINE;
		else if (!strcmp(s, "ignore"))
			new_use = HSTATUS_IGNORE;
		else if (!strcmp(s, "message"))
			new_use = HSTATUS_MESSAGE;
		else if (!strcmp(args[0], "string")) {
			if (!args[1]) {
				char buf[256];
				AddXChars(buf, ARRAY_SIZE(buf), hstatusstring);
				OutputMsg(0, "hardstatus string is '%s'", buf);
				return;
			}
		} else {
			OutputMsg(0, "%s: usage: hardstatus [always]lastline|ignore|message|string [string]",
				  rc_name);
			return;
		}
		if (new_use != -1) {
			hardstatusemu = new_use | (s == args[0] ? 0 : HSTATUS_ALWAYS);
			for (display = displays; display; display = display->d_next) {
				RemoveStatus();
				new_use = hardstatusemu & ~HSTATUS_ALWAYS;
				if (D_HS && s == args[0])
					new_use = HSTATUS_HS;
				ShowHStatus(NULL);
				old_use = D_has_hstatus;
				D_has_hstatus = new_use;
				if ((new_use == HSTATUS_LASTLINE && old_use != HSTATUS_LASTLINE)
				    || (new_use != HSTATUS_LASTLINE && old_use == HSTATUS_LASTLINE))
					ChangeScreenSize(D_width, D_height, 1);
				if ((new_use == HSTATUS_FIRSTLINE && old_use != HSTATUS_FIRSTLINE)
				    || (new_use != HSTATUS_FIRSTLINE && old_use == HSTATUS_FIRSTLINE))
					ChangeScreenSize(D_width, D_height, 1);
				RefreshHStatus();
			}
		}
		if (args[1]) {
			if (hstatusstring)
				free(hstatusstring);
			hstatusstring = SaveStr(args[1]);
			for (display = displays; display; display = display->d_next)
				RefreshHStatus();
		}
		display = olddisplay;
		return;
	}
	(void)ParseSwitch(act, &use_hardstatus);
	if (msgok)
		OutputMsg(0, "messages displayed on %s", use_hardstatus ? "hardstatus line" : "window");
}

static void DoCommandStatus(struct action *act)
{
	char **args = act->args;

	if (display) {
		Msg(0, "%s", "");	/* wait till mintime (keep gcc quiet) */
		RemoveStatus();
	}
	{
		int i = 0;
		while ((i <= 1) && args[i]) {
			if ((strcmp(args[i], "top") == 0) || (strcmp(args[i], "up") == 0)) {
				statuspos.row = STATUS_TOP;
			} else if ((strcmp(args[i], "bottom") == 0) || (strcmp(args[i], "down") == 0)) {
				statuspos.row = STATUS_BOTTOM;
			} else if (strcmp(args[i], "left") == 0) {
				statuspos.col = STATUS_LEFT;
			} else if (strcmp(args[i], "right") == 0) {
				statuspos.col = STATUS_RIGHT;
			} else {
				Msg(0, "%s: usage: status [top|up|down|bottom] [left|right]", rc_name);
				break;
			}
			i++;
		}
	}
}

static void DoCommandCaption(struct action *act)
{
	char **args = act->args;

	if (!*args)
		return;
	if (strcmp(args[0], "top") == 0) {
		captiontop = 1;
		args++;
	} else if(strcmp(args[0], "bottom") == 0) {
		captiontop = 0;
		args++;
	}
	if (strcmp(args[0], "always") == 0 || strcmp(args[0], "splitonly") == 0) {
		Display *olddisplay = display;

		captionalways = args[0][0] == 'a';
		for (display = displays; display; display = display->d_next)
			ChangeScreenSize(D_width, D_height, 1);
		display = olddisplay;
	} else if (strcmp(args[0], "string") == 0) {
		if (!args[1]) {
			char buf[256];
			AddXChars(buf, ARRAY_SIZE(buf), captionstring);
			OutputMsg(0, "caption string is '%s'", buf);
			return;
		}
	} else {
		OutputMsg(0, "%s: usage: caption [ top | bottom ] always|splitonly|string <string>", rc_name);
		return;
	}
	if (!args[1])
		return;
	if (captionstring)
		free(captionstring);
	captionstring = SaveStr(args[1]);
	RedisplayDisplays(0);
}

static void DoCommandConsole(struct action *act)
{
	bool b = (console_window != NULL);

	if (ParseSwitch(act, &b))
		return;
	if (TtyGrabConsole(fore->w_ptyfd, b, rc_name))
		return;
	if (b == 0)
		OutputMsg(0, "%s: releasing console %s", rc_name, HostName);
	else if (console_window)
		OutputMsg(0, "%s: stealing console %s from window %d (%s)", rc_name,
			  HostName, console_window->w_number, console_window->w_title);
	else
		OutputMsg(0, "%s: grabbing console %s", rc_name, HostName);
	console_window = b ? fore : NULL;
}

static void DoCommandAllpartial(struct action *act)
{
	int msgok = display && !*rc_name;

	if (ParseOnOff(act, &all_norefresh))
		return;
	if (!all_norefresh && fore)
		Activate(-1);
	if (msgok)
		OutputMsg(0, all_norefresh ? "No refresh on window change!\n" : "Window specific refresh\n");
}

static void DoCommandPartial(struct action *act)
{
	bool b = fore->w_norefresh;

	(void)ParseSwitch(act, &b);
	fore->w_norefresh = b;
}


static void DoCommandVbell(struct action *act)
{
	int msgok = display && !*rc_name;

	if (ParseSwitch(act, &visual_bell) || !msgok)
		return;
	if (visual_bell == 0)
		OutputMsg(0, "switched to audible bell.");
	else
		OutputMsg(0, "switched to visual bell.");
}

static void DoCommandVbellwait(struct action *act)
{
	int msgok = display && !*rc_name;

	if (ParseNum1000(act, &VBellWait) == 0 && msgok)
		OutputMsg(0, "vbellwait set to %.10g seconds", VBellWait / 1000.);
}

static void DoCommandMsgwait(struct action *act)
{
	int msgok = display && !*rc_name;

	if (ParseNum1000(act, &MsgWait) == 0 && msgok)
		OutputMsg(0, "msgwait set to %.10g seconds", MsgWait / 1000.);
}

static void DoCommandMsgminwait(struct action *act)
{
	int msgok = display && !*rc_name;

	if (ParseNum1000(act, &MsgMinWait) == 0 && msgok)
		OutputMsg(0, "msgminwait set to %.10g seconds", MsgMinWait / 1000.);
}

static void DoCommandSilencewait(struct action *act)
{
	int msgok = display && !*rc_name;

	if (ParseNum(act, &SilenceWait))
		return;
	if (SilenceWait < 1)
		SilenceWait = 1;
	for (Window *win = mru_window; win; win = win->w_prev_mru)
		win->w_silencewait = SilenceWait;
	if (msgok)
		OutputMsg(0, "silencewait set to %d seconds", SilenceWait);
}

static void DoCommandBumpright(struct action *act)
{
	Window *win = NextWindow();

	(void)act; /* unused */

	if (fore->w_number < win->w_number)
		SwapWindows(fore->w_number, win->w_number);
}

static void DoCommandBumpleft(struct action *act)
{
	Window *win = PreviousWindow();

	(void)act; /* unused */

	if (fore->w_number > win->w_number)
		SwapWindows(fore->w_number, win->w_number);
}

static void DoCommandCollapse(struct action *act)
{
	(void)act; /* unused */

	CollapseWindowlist();
}

static void DoCommandNumber(struct action *act)
{
	char **args = act->args;

	if (*args == NULL)
		OutputMsg(0, queryflag >= 0 ? "%d (%s)" : "This is window %d (%s).", fore->w_number,
			  fore->w_title);
	else {
		int old = fore->w_number;
		int rel = 0, parse;
		int n = 0;
		if (args[0][0] == '+')
			rel = 1;
		else if (args[0][0] == '-')
			rel = -1;
		if (rel)
			++act->args[0];
		parse = ParseNum(act, &n);
		if (rel)
			--act->args[0];
		if (parse)
			return;
		if (rel > 0)
			n += old;
		else if (rel < 0)
			n = old - n;
		if (!SwapWindows(old, n)) {
			/* Window number could not be changed. */
			queryflag = -1;
			return;
		}
	}
}

static void DoCommandZombie_timeout(struct action *act)
{
	char **args = act->args;
	int argc = CheckArgNum(act->nr, args);

	if (argc != 1) {
		Msg(0, "Setting zombie polling needs a timeout arg\n");
		return;
	}

	nwin_default.poll_zombie_timeout = atoi(args[0]);
	if (fore)
		fore->w_poll_zombie_timeout = nwin_default.poll_zombie_timeout;
}

static void DoCommandSort(struct action *act)
{
	(void)act; /* unused */

	if (first_window == last_window) {
		Msg(0, "Less than two windows, sorting makes no sense.\n");
		return;
	}

	for (Window *w1 = first_window; w1; w1 = w1->w_next) {
		for (Window *w2 = w1; w2; w2 = w2->w_next) {
			if (strcmp(w2->w_title, w1->w_title) < 0) {
				Window *tmp;
				SwapWindows(w1->w_number, w2->w_number);
				/* swap pointers back, to continue looping correctly */
				tmp = w1;
				w1 = w2;
				w2 = tmp;
			}
		}
	}

	WindowChanged(NULL, 0);
}

static void DoCommandSilence(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;
	bool b = fore->w_silence != 0;
	int i = fore->w_silencewait;

	if (args[0] && (args[0][0] == '-' || (args[0][0] >= '0' && args[0][0] <= '9'))) {
		if (ParseNum(act, &i))
			return;
		b = i > 0;
	} else if (ParseSwitch(act, &b))
		return;
	if (b) {
		if (display)	/* we tell only this user */
			ACLBYTE(fore->w_lio_notify, D_user->u_id) |= ACLBIT(D_user->u_id);
		else
			for (int n = 0; n < maxusercount; n++)
				ACLBYTE(fore->w_lio_notify, n) |= ACLBIT(n);
		fore->w_silencewait = i;
		fore->w_silence = SILENCE_ON;
		SetTimeout(&fore->w_silenceev, fore->w_silencewait * 1000);
		evenq(&fore->w_silenceev);

		if (!msgok)
			return;
		OutputMsg(0, "The window is now being monitored for %d sec. silence.", fore->w_silencewait);
	} else {
		if (display)	/* we remove only this user */
			ACLBYTE(fore->w_lio_notify, D_user->u_id)
			    &= ~ACLBIT(D_user->u_id);
		else
			for (int n = 0; n < maxusercount; n++)
				ACLBYTE(fore->w_lio_notify, n) &= ~ACLBIT(n);
		for (i = maxusercount - 1; i >= 0; i--)
			if (ACLBYTE(fore->w_lio_notify, i))
				break;
		if (i < 0) {
			fore->w_silence = SILENCE_OFF;
			evdeq(&fore->w_silenceev);
		}
		if (!msgok)
			return;
		OutputMsg(0, "The window is no longer being monitored for silence.");
	}
}

static void DoCommandDefscrollback(struct action *act)
{
	(void)ParseNum(act, &nwin_default.histheight);
}

static void DoCommandScrollback(struct action *act)
{
	int msgok = display && !*rc_name;
	int n = 0;

	if (flayer->l_layfn == &MarkLf) {
		OutputMsg(0, "Cannot resize scrollback buffer in copy/scrollback mode.");
		return;
	}
	(void)ParseNum(act, &n);
	ChangeWindowSize(fore, fore->w_width, fore->w_height, n);
	if (msgok)
		OutputMsg(0, "scrollback set to %d", fore->w_histheight);
}

static void DoCommandSessionname(struct action *act)
{
	char **args = act->args;

	if (*args == NULL)
		OutputMsg(0, "This session is named '%s'\n", SocketName);
	else {
		char buf[MAXPATHLEN];
		char *s = NULL;

		if (ParseSaveStr(act, &s))
			return;
		if (!*s || strlen(s) + (SocketName - SocketPath) > MAXPATHLEN - 13 || strchr(s, '/')) {
			OutputMsg(0, "%s: bad session name '%s'\n", rc_name, s);
			free(s);
			return;
		}
		strncpy(buf, SocketPath, SocketName - SocketPath);
		sprintf(buf + (SocketName - SocketPath), "%d.%s", (int)getpid(), s);
		free(s);
		if ((access(buf, F_OK) == 0) || (errno != ENOENT)) {
			OutputMsg(0, "%s: inappropriate path: '%s'.", rc_name, buf);
			return;
		}
		if (rename(SocketPath, buf)) {
			OutputMsg(errno, "%s: failed to rename(%s, %s)", rc_name, SocketPath, buf);
			return;
		}
		strncpy(SocketPath, buf, ARRAY_SIZE(SocketPath));
		MakeNewEnv();
		WindowChanged(NULL, WINESC_SESS_NAME);
	}
}

static void DoCommandSetenv(struct action *act)
{
	char **args = act->args;

	if (!args[0] || !args[1]) {
		InputSetenv(args[0]);
	} else {
		setenv(args[0], args[1], 1);
		MakeNewEnv();
	}
}

static void DoCommandUnsetenv(struct action *act)
{
	char **args = act->args;

	if (*args)
		unsetenv(*args);
	MakeNewEnv();
}

static void DoCommandDefslowpaste(struct action *act)
{
	(void)ParseNum(act, &nwin_default.slow);
}

static void DoCommandSlowpaste(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;

	if (*args == NULL)
		OutputMsg(0, fore->w_slowpaste ?
			  "Slowpaste in window %d is %d milliseconds." :
			  "Slowpaste in window %d is unset.", fore->w_number, fore->w_slowpaste);
	else if (ParseNum(act, &fore->w_slowpaste) == 0 && msgok)
		OutputMsg(0, fore->w_slowpaste ?
			  "Slowpaste in window %d set to %d milliseconds." :
			  "Slowpaste in window %d now unset.", fore->w_number, fore->w_slowpaste);
}

static void DoCommandMarkkeys(struct action *act)
{
	char **args = act->args;
	int *argl = act->argl;

	if (CompileKeys(*args, *argl, mark_key_tab))
		OutputMsg(0, "%s: markkeys: syntax error.", rc_name);
}

static void DoCommandPastefont(struct action *act)
{
	int msgok = display && !*rc_name;

	if (ParseSwitch(act, &pastefont) == 0 && msgok)
		OutputMsg(0, "Will %spaste font settings", pastefont ? "" : "not ");
}

static void DoCommandCrlf(struct action *act)
{
	(void)ParseSwitch(act, &join_with_cr);
}

static void DoCommandCompacthist(struct action *act)
{
	int msgok = display && !*rc_name;

	if (ParseSwitch(act, &compacthist) == 0 && msgok)
		OutputMsg(0, "%scompacting history lines", compacthist ? "" : "not ");
}

static void DoCommandHardcopy_append(struct action *act)
{
	(void)ParseOnOff(act, &hardcopy_append);
}

static void DoCommandVbell_msg(struct action *act)
{
	char **args = act->args;

	if (*args == NULL) {
		char buf[256];
		AddXChars(buf, ARRAY_SIZE(buf), VisualBellString);
		OutputMsg(0, "vbell_msg is '%s'", buf);
		return;
	}
	(void)ParseSaveStr(act, &VisualBellString);
}

static void DoCommandDefmode(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;
	int n = 0;

	if (ParseBase(act, *args, &n, 8, "octal"))
		return;
	if (n < 0 || n > 0777) {
		OutputMsg(0, "%s: mode: Invalid tty mode %o", rc_name, n);
		return;
	}
	TtyMode = n;
	if (msgok)
		OutputMsg(0, "Ttymode set to %03o", TtyMode);
}

static void DoCommandAutodetach(struct action *act)
{
	(void)ParseOnOff(act, &auto_detach);
}

static void DoCommandStartup_message(struct action *act)
{
	(void)ParseOnOff(act, &default_startup);
}

static void DoCommandBind(struct action *act)
{
	char **args = act->args;
	int argc = CheckArgNum(act->nr, args);
	int *argl = act->argl;
	int n = 0;
	struct action *ktabp = ktab;
	int kflag = 0;

	for (;;) {
		if (argc > 2 && !strcmp(*args, "-c")) {
			ktabp = FindKtab(args[1], 1);
			if (ktabp == NULL)
				break;
			args += 2;
			argl += 2;
			argc -= 2;
		} else if (argc > 1 && !strcmp(*args, "-k")) {
			kflag = 1;
			args++;
			argl++;
			argc--;
		} else
			break;
	}
	if (kflag) {
		for (n = 0; n < KMAP_KEYS; n++)
			if (strcmp(term[n + T_CAPS].tcname, *args) == 0)
				break;
		if (n == KMAP_KEYS) {
			OutputMsg(0, "%s: bind: unknown key '%s'", rc_name, *args);
			return;
		}
		n += 256;
	} else if (*argl != 1) {
		OutputMsg(0, "%s: bind: character, ^x, or (octal) \\032 expected.", rc_name);
		return;
	} else
		n = (unsigned char)args[0][0];

	if (args[1]) {
		int i;
		if ((i = FindCommnr(args[1])) == RC_ILLEGAL) {
			OutputMsg(0, "%s: bind: unknown command '%s'", rc_name, args[1]);
			return;
		}
		if (CheckArgNum(i, args + 2) < 0)
			return;
		ClearAction(&ktabp[n]);
		SaveAction(ktabp + n, i, args + 2, argl + 2);
	} else
		ClearAction(&ktabp[n]);
}

static void DoCommandBindkey(struct action *act)
{
	char **args = act->args;
	int *argl = act->argl;
	struct action *newact;
	int newnr, fl = 0, kf = 0, af = 0, df = 0, mf = 0;
	Display *olddisplay = display;
	int used = 0;
	struct kmap_ext *kme = NULL;
	int i;

	for (; *args && **args == '-'; args++, argl++) {
		if (strcmp(*args, "-t") == 0)
			fl = KMAP_NOTIMEOUT;
		else if (strcmp(*args, "-k") == 0)
			kf = 1;
		else if (strcmp(*args, "-a") == 0)
			af = 1;
		else if (strcmp(*args, "-d") == 0)
			df = 1;
		else if (strcmp(*args, "-m") == 0)
			mf = 1;
		else if (strcmp(*args, "--") == 0) {
			args++;
			argl++;
			break;
		} else {
			OutputMsg(0, "%s: bindkey: invalid option %s", rc_name, *args);
			return;
		}
	}
	if (df && mf) {
		OutputMsg(0, "%s: bindkey: -d does not work with -m", rc_name);
		return;
	}
	if (*args == NULL) {
		if (mf)
			display_bindkey("Edit mode", mmtab);
		else if (df)
			display_bindkey("Default", dmtab);
		else
			display_bindkey("User", umtab);
		return;
	}
	if (kf == 0) {
		if (af) {
			OutputMsg(0, "%s: bindkey: -a only works with -k", rc_name);
			return;
		}
		if (*argl == 0) {
			OutputMsg(0, "%s: bindkey: empty string makes no sense", rc_name);
			return;
		}
		for (i = 0, kme = kmap_exts; i < kmap_extn; i++, kme++)
			if (kme->str == NULL) {
				if (args[1])
					break;
			} else
			    if (*argl == (kme->fl & ~KMAP_NOTIMEOUT)
				&& memcmp(kme->str, *args, *argl) == 0)
				break;
		if (i == kmap_extn) {
			if (!args[1]) {
				OutputMsg(0, "%s: bindkey: keybinding not found", rc_name);
				return;
			}
			kmap_extn += 8;
			kmap_exts = xrealloc((char *)kmap_exts, kmap_extn * sizeof(struct kmap_ext));
			kme = kmap_exts + i;
			memset((char *)kme, 0, 8 * sizeof(struct kmap_ext));
			for (; i < kmap_extn; i++, kme++) {
				kme->str = NULL;
				kme->dm.nr = kme->mm.nr = kme->um.nr = RC_ILLEGAL;
				kme->dm.args = kme->mm.args = kme->um.args = noargs;
				kme->dm.argl = kme->mm.argl = kme->um.argl = NULL;
			}
			i -= 8;
			kme -= 8;
		}
		if (df == 0 && kme->dm.nr != RC_ILLEGAL)
			used = 1;
		if (mf == 0 && kme->mm.nr != RC_ILLEGAL)
			used = 1;
		if ((df || mf) && kme->um.nr != RC_ILLEGAL)
			used = 1;
		i += KMAP_KEYS + KMAP_AKEYS;
		newact = df ? &kme->dm : mf ? &kme->mm : &kme->um;
	} else {
		for (i = T_CAPS; i < T_OCAPS; i++)
			if (strcmp(term[i].tcname, *args) == 0)
				break;
		if (i == T_OCAPS) {
			OutputMsg(0, "%s: bindkey: unknown key '%s'", rc_name, *args);
			return;
		}
		if (af && i >= T_CURSOR && i < T_OCAPS)
			i -= T_CURSOR - KMAP_KEYS;
		else
			i -= T_CAPS;
		newact = df ? &dmtab[i] : mf ? &mmtab[i] : &umtab[i];
	}
	if (args[1]) {
		if ((newnr = FindCommnr(args[1])) == RC_ILLEGAL) {
			OutputMsg(0, "%s: bindkey: unknown command '%s'", rc_name, args[1]);
			return;
		}
		if (CheckArgNum(newnr, args + 2) < 0)
			return;
		ClearAction(newact);
		SaveAction(newact, newnr, args + 2, argl + 2);
		if (kf == 0 && args[1]) {
			if (kme->str)
				free(kme->str);
			kme->str = SaveStrn(*args, *argl);
			kme->fl = fl | *argl;
		}
	} else
		ClearAction(newact);
	for (display = displays; display; display = display->d_next)
		remap(i, args[1] ? 1 : 0);
	if (kf == 0 && !args[1]) {
		if (!used && kme->str) {
			free(kme->str);
			kme->str = NULL;
			kme->fl = 0;
		}
	}
	display = olddisplay;
}

static void DoCommandMaptimeout(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;
	int n = 0;

	if (*args) {
		if (ParseNum(act, &n))
			return;
		if (n < 0) {
			OutputMsg(0, "%s: maptimeout: illegal time %d", rc_name, n);
			return;
		}
		maptimeout = n;
	}
	if (*args == NULL || msgok)
		OutputMsg(0, "maptimeout is %dms", maptimeout);
}

static void DoCommandMapnotnext(struct action *act)
{
	(void)act; /* unused */

	D_dontmap = 1;
}

static void DoCommandMapdefault(struct action *act)
{
	(void)act; /* unused */

	D_mapdefault = 1;
}

static void DoCommandAclchg(struct action *act)
{
	char **args = act->args;
	int argc = CheckArgNum(act->nr, args);

	UsersAcl(NULL, argc, args);
}

static void DoCommandAcldel(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;

	if (UserDel(args[0], NULL))
		return;
	if (msgok)
		OutputMsg(0, "%s removed from acl database", args[0]);
}

static void DoCommandAclgrp(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;

	/*
	 * modify a user to gain or lose rights granted to a group.
	 * This group is actually a normal user whose rights were defined
	 * with chacl in the usual way.
	 */
	if (args[1]) {
		if (strcmp(args[1], "none")) {	/* link a user to another user */
			if (AclLinkUser(args[0], args[1]))
				return;
			if (msgok)
				OutputMsg(0, "User %s joined acl-group %s", args[0], args[1]);
		} else {	/* remove all groups from user */

			struct acluser *u;
			struct aclusergroup *g;

			if (!(u = *FindUserPtr(args[0])))
				return;
			while ((g = u->u_group)) {
				u->u_group = g->next;
				free((char *)g);
			}
		}
	} else {	/* show all groups of user */

		char buf[256], *p = buf;
		int ngroups = 0;
		struct acluser *u;
		struct aclusergroup *g;

		if (!(u = *FindUserPtr(args[0]))) {
			if (msgok)
				OutputMsg(0, "User %s does not exist.", args[0]);
			return;
		}
		g = u->u_group;
		while (g) {
			ngroups++;
			sprintf(p, "%s ", g->u->u_name);
			p += strlen(p);
			if (p > buf + 200)
				break;
			g = g->next;
		}
		if (ngroups)
			*(--p) = '\0';
		OutputMsg(0, "%s's group%s: %s.", args[0], (ngroups == 1) ? "" : "s",
			  (ngroups == 0) ? "none" : buf);
	}
}

static void DoCommandAclumask(struct action *act)
{
	char **args = act->args;
	char *s = NULL;

	while ((s = *args++)) {
		char *err = NULL;

		if (AclUmask(display ? D_user : users, s, &err))
			OutputMsg(0, "umask: %s\n", err);
	}
}

static void DoCommandMultiuser(struct action *act)
{
	int msgok = display && !*rc_name;
	bool b;

	if (ParseOnOff(act, &b))
		return;
	multi = b ? "" : NULL;
	chsock();
	if (msgok)
		OutputMsg(0, "Multiuser mode %s", multi ? "enabled" : "disabled");
}

static void DoCommandExec(struct action *act)
{
	char **args = act->args;

	winexec(args);
}

static void DoCommandNonblock(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;
	bool b = D_nonblock >= 0;
	int n = 0;

	if (*args && ((args[0][0] >= '0' && args[0][0] <= '9') || args[0][0] == '.')) {
		if (ParseNum1000(act, &n))
			return;
	} else if (!ParseSwitch(act, &b))
		n = b == 0 ? -1 : 1000;
	else
		return;
	if (msgok && n == -1)
		OutputMsg(0, "display set to blocking mode");
	else if (msgok && n == 0)
		OutputMsg(0, "display set to nonblocking mode, no timeout");
	else if (msgok)
		OutputMsg(0, "display set to nonblocking mode, %.10gs timeout", n / 1000.);
	D_nonblock = n;
	if (D_nonblock <= 0)
		evdeq(&D_blockedev);
}

static void DoCommandDefnonblock(struct action *act)
{
	char **args = act->args;
	bool b;

	if (*args && ((args[0][0] >= '0' && args[0][0] <= '9') || args[0][0] == '.')) {
		if (ParseNum1000(act, &defnonblock))
			return;
	} else if (!ParseOnOff(act, &b))
		defnonblock = b == 0 ? -1 : 1000;
	else
		return;
	if (display && *rc_name) {
		D_nonblock = defnonblock;
		if (D_nonblock <= 0)
			evdeq(&D_blockedev);
	}
}

static void DoCommandGr(struct action *act)
{
	int msgok = display && !*rc_name;
	bool b;

	if (fore->w_gr == 2)
		fore->w_gr = 0;
	b = fore->w_gr;
	if (ParseSwitch(act, &b) == 0 && msgok) {
		fore->w_gr = b ? 1 : 0;
		OutputMsg(0, "Will %suse GR", fore->w_gr ? "" : "not ");
	}
	if (fore->w_gr == 0 && fore->w_FontE)
		fore->w_gr = 2;
}

static void DoCommandC1(struct action *act)
{
	int msgok = display && !*rc_name;

	if (ParseSwitch(act, &fore->w_c1) == 0 && msgok)
		OutputMsg(0, "Will %suse C1", fore->w_c1 ? "" : "not ");
}

static void DoCommandEncoding(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;

	if (*args && !strcmp(args[0], "-d")) {
		if (!args[1])
			OutputMsg(0, "encodings directory is %s",
				  screenencodings ? screenencodings : "<unset>");
		else {
			free(screenencodings);
			screenencodings = SaveStr(args[1]);
		}
		return;
	}
	if (*args && !strcmp(args[0], "-l")) {
		if (!args[1])
			OutputMsg(0, "encoding: -l: argument required");
		else if (LoadFontTranslation(-1, args[1]))
			OutputMsg(0, "encoding: could not load utf8 encoding file");
		else if (msgok)
			OutputMsg(0, "encoding: utf8 encoding file loaded");
		return;
	}
	for (int i = 0; i < 2; i++) {
		int n;
		if (args[i] == NULL)
			break;
		if (!strcmp(args[i], "."))
			continue;
		n = FindEncoding(args[i]);
		if (n == -1) {
			OutputMsg(0, "encoding: unknown encoding '%s'", args[i]);
			break;
		}
		if (i == 0 && fore) {
			WinSwitchEncoding(fore, n);
			ResetCharsets(fore);
		} else if (i && display)
			D_encoding = n;
	}
}

static void DoCommandDefencoding(struct action *act)
{
	char **args = act->args;
	int n;

	n = FindEncoding(*args);
	if (n == -1) {
		OutputMsg(0, "defencoding: unknown encoding '%s'", *args);
		return;
	}
	nwin_default.encoding = n;
}

static void DoCommandDefutf8(struct action *act)
{
	int msgok = display && !*rc_name;
	bool b = nwin_default.encoding == UTF8;

	if (ParseSwitch(act, &b) == 0) {
		nwin_default.encoding = b ? UTF8 : 0;
		if (msgok)
			OutputMsg(0, "Will %suse UTF-8 encoding for new windows", b ? "" : "not ");
	}
}

static void DoCommandUtf8(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;

	for (int i = 0; i < 2; i++) {
		int n;
		if (i && args[i] == NULL)
			break;
		if (args[i] == NULL)
			n = fore->w_encoding != UTF8;
		else if (strcmp(args[i], "off") == 0)
			n = 0;
		else if (strcmp(args[i], "on") == 0)
			n = 1;
		else {
			OutputMsg(0, "utf8: illegal argument (%s)", args[i]);
			break;
		}
		if (i == 0) {
			WinSwitchEncoding(fore, n ? UTF8 : 0);
			if (msgok)
				OutputMsg(0, "Will %suse UTF-8 encoding", n ? "" : "not ");
		} else if (display)
			D_encoding = n ? UTF8 : 0;
		if (args[i] == NULL)
			break;
	}
}

static void DoCommandPrintcmd(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;

	if (*args) {
		if (printcmd)
			free(printcmd);
		printcmd = NULL;
		if (**args)
			printcmd = SaveStr(*args);
	}
	if (*args == NULL || msgok) {
		if (printcmd)
			OutputMsg(0, "using '%s' as print command", printcmd);
		else
			OutputMsg(0, "using termcap entries for printing");
		return;
	}
}

static void DoCommandDigraph(struct action *act)
{
	char **args = act->args;
	int *argl = act->argl;

	if (argl && argl[0] > 0 && args[1] && argl[1] > 0) {
		int i;
		if (argl[0] != 2) {
			OutputMsg(0, "Two characters expected to define a digraph");
			return;
		}
		i = digraph_find(args[0]);
		digraphs[i].d[0] = args[0][0];
		digraphs[i].d[1] = args[0][1];
		if (!parse_input_int(args[1], argl[1], &digraphs[i].value)) {
			if (!(digraphs[i].value = atoi(args[1]))) {
				if (!args[1][1])
					digraphs[i].value = (int)args[1][0];
				else {
					int t;
					unsigned char *s = (unsigned char *)args[1];
					digraphs[i].value = 0;
					while (*s) {
						t = FromUtf8(*s++, &digraphs[i].value);
						if (t == -1)
							continue;
						if (t == -2)
							digraphs[i].value = 0;
						else
							digraphs[i].value = t;
						break;
					}
				}
			}
		}
		return;
	}
	Input("Enter digraph: ", 10, INP_EVERY, digraph_fn, NULL, 0);
	if (*args && **args) {
		char *s = *args;
		size_t len = strlen(s);
		LayProcess(&s, &len);
	}
}

static void DoCommandDefhstatus(struct action *act)
{
	char **args = act->args;

	if (*args == NULL) {
		char buf[256] = { 0 };
		if (nwin_default.hstatus)
			AddXChars(buf, ARRAY_SIZE(buf), nwin_default.hstatus);
		OutputMsg(0, "default hstatus is '%s'", buf);
		return;
	}
	(void)ParseSaveStr(act, &nwin_default.hstatus);
	if (*nwin_default.hstatus == 0) {
		free(nwin_default.hstatus);
		nwin_default.hstatus = NULL;
	}
}

static void DoCommandHstatus(struct action *act)
{
	(void)ParseSaveStr(act, &fore->w_hstatus);
	if (*fore->w_hstatus == 0) {
		free(fore->w_hstatus);
		fore->w_hstatus = NULL;
	}
	WindowChanged(fore, WINESC_HSTATUS);
}

static void DoCommandDefcharset(struct action *act)
{
	char **args = act->args;
	size_t len;

	if (*args == NULL) {
		char buf[256] = { 0 };
		if (nwin_default.charset)
			AddXChars(buf, ARRAY_SIZE(buf), nwin_default.charset);
		OutputMsg(0, "default charset is '%s'", buf);
		return;
	}
	len = strlen(*args);
	if (len == 0 || len > 6) {
		OutputMsg(0, "%s: defcharset: string has illegal size.", rc_name);
		return;
	}
	if (len > 4 && (((args[0][4] < '0' || args[0][4] > '3') && args[0][4] != '.') ||
		        ((args[0][5] < '0' || args[0][5] > '3') && args[0][5] && args[0][5] != '.'))) {
		OutputMsg(0, "%s: defcharset: illegal mapping number.", rc_name);
		return;
	}
	if (nwin_default.charset)
		free(nwin_default.charset);
	nwin_default.charset = SaveStr(*args);
}

static void DoCommandCharset(struct action *act)
{
	char **args = act->args;
	size_t len;

	if (*args == NULL) {
		char buf[256] = { 0 };
		if (nwin_default.charset)
			AddXChars(buf, ARRAY_SIZE(buf), nwin_default.charset);
		OutputMsg(0, "default charset is '%s'", buf);
		return;
	}
	len = strlen(*args);
	if (len == 0 || len > 6) {
		OutputMsg(0, "%s: charset: string has illegal size.", rc_name);
		return;
	}
	if (len > 4 && (((args[0][4] < '0' || args[0][4] > '3') && args[0][4] != '.') ||
		        ((args[0][5] < '0' || args[0][5] > '3') && args[0][5] && args[0][5] != '.'))) {
		OutputMsg(0, "%s: charset: illegal mapping number.", rc_name);
		return;
	}
	SetCharsets(fore, *args);
}

static void DoCommandRendition(struct action *act)
{
	char **args = act->args;
	int *argl = act->argl;
	int msgok = display && !*rc_name;
	int i = -1;

	if (!*args)
		return;

	if (strcmp(args[0], "bell") == 0) {
		i = REND_BELL;
	} else if (strcmp(args[0], "monitor") == 0) {
		i = REND_MONITOR;
	} else if (strcmp(args[0], "silence") == 0) {
		i = REND_SILENCE;
	} else if (strcmp(args[0], "so") != 0) {
		OutputMsg(0, "Invalid option '%s' for rendition", args[0]);
		return;
	}

	++args;
	++argl;

	if (i != -1) {
		renditions[i] = ParseAttrColor(args[0], 1);
		WindowChanged(NULL, WINESC_WIN_NAMES);
		WindowChanged(NULL, WINESC_WIN_NAMES_NOCUR);
		WindowChanged(NULL, 0);
		return;
	}

	/* sorendition */
	if (args[0]) {
		uint64_t i = ParseAttrColor(args[0], 1);
		if (i == 0)
			return;
		ApplyAttrColor(i, &mchar_so);
		WindowChanged(NULL, 0);
	}
	if (msgok)
		OutputMsg(0, "Standout attributes 0x%02x  colorbg 0x%02x  colorfg 0x%02x", (unsigned char)mchar_so.attr,
			  (unsigned char)mchar_so.colorbg, (unsigned char)mchar_so.colorfg);

}

static void DoCommandSorendition(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;

	if (args[0]) {
		uint64_t i = ParseAttrColor(args[0], 1);
		if (i == 0)
			return;
		ApplyAttrColor(i, &mchar_so);
		WindowChanged(NULL, 0);
	}
	if (msgok)
		OutputMsg(0, "Standout attributes 0x%02x  colorbg 0x%02x  colorfg 0x%02x", (unsigned char)mchar_so.attr,
			  (unsigned char)mchar_so.colorbg, (unsigned char)mchar_so.colorfg);
}

static void DoCommandSource(struct action *act)
{
	char **args = act->args;

	do_source(*args);
}

static void DoCommandSu(struct action *act)
{
	char **args = act->args;
	char *s = NULL;

	if (!*args) {
		OutputMsg(0, "%s:%s screen login", HostName, SocketPath);
		InputSu(&D_user, NULL);
	} else if (!args[1])
		InputSu(&D_user, args[0]);
	else if (!args[2])
		s = DoSu(&D_user, args[0], args[1], "\377");
	else
		s = DoSu(&D_user, args[0], args[1], args[2]);
	if (s)
		OutputMsg(0, "%s", s);
}

static void DoCommandSplit(struct action *act)
{
	char **args = act->args;
	char *s = NULL;

	s = args[0];
	if (s && !strcmp(s, "-v"))
		AddCanvas(SLICE_HORI);
	else
		AddCanvas(SLICE_VERT);
	Activate(-1);
}

static void DoCommandRemove(struct action *act)
{
	(void)act; /* unused */

	RemCanvas();
	Activate(-1);
}

static void DoCommandOnly(struct action *act)
{
	(void)act; /* unused */

	OneCanvas();
	Activate(-1);
}

static void DoCommandFit(struct action *act)
{
	(void)act; /* unused */

	D_forecv->c_xoff = D_forecv->c_xs;
	D_forecv->c_yoff = D_forecv->c_ys;
	RethinkViewportOffsets(D_forecv);
	ResizeLayer(D_forecv->c_layer, D_forecv->c_xe - D_forecv->c_xs + 1, D_forecv->c_ye - D_forecv->c_ys + 1,
		    NULL);
	flayer = D_forecv->c_layer;
	LaySetCursor();
}

static void DoCommandFocus(struct action *act)
{
	char **args = act->args;
	Canvas *cv = NULL;

	if (!*args || !strcmp(*args, "next"))
		cv = D_forecv->c_next ? D_forecv->c_next : D_cvlist;
	else if (!strcmp(*args, "prev")) {
		for (cv = D_cvlist; cv->c_next && cv->c_next != D_forecv; cv = cv->c_next) ;
	} else if (!strcmp(*args, "top"))
		cv = D_cvlist;
	else if (!strcmp(*args, "bottom")) {
		for (cv = D_cvlist; cv->c_next; cv = cv->c_next) ;
	} else if (!strcmp(*args, "up"))
		cv = FindCanvas(D_forecv->c_xs, D_forecv->c_ys - 1);
	else if (!strcmp(*args, "down"))
		cv = FindCanvas(D_forecv->c_xs, D_forecv->c_ye + 2);
	else if (!strcmp(*args, "left"))
		cv = FindCanvas(D_forecv->c_xs - 1, D_forecv->c_ys);
	else if (!strcmp(*args, "right"))
		cv = FindCanvas(D_forecv->c_xe + 1, D_forecv->c_ys);
	else {
		OutputMsg(0, "%s: usage: focus [next|prev|up|down|left|right|top|bottom]", rc_name);
		return;
	}
	SetForeCanvas(display, cv);
}

static void DoCommandResize(struct action *act)
{
	char **args = act->args;
	int i = 0;

	if (D_forecv->c_slorient == SLICE_UNKN) {
		OutputMsg(0, "resize: need more than one region");
		return;
	}
	for (; *args; args++) {
		if (!strcmp(*args, "-h"))
			i |= RESIZE_FLAG_H;
		else if (!strcmp(*args, "-v"))
			i |= RESIZE_FLAG_V;
		else if (!strcmp(*args, "-b"))
			i |= RESIZE_FLAG_H | RESIZE_FLAG_V;
		else if (!strcmp(*args, "-p"))
			i |= D_forecv->c_slorient == SLICE_VERT ? RESIZE_FLAG_H : RESIZE_FLAG_V;
		else if (!strcmp(*args, "-l"))
			i |= RESIZE_FLAG_L;
		else
			break;
	}
	if (*args && args[1]) {
		OutputMsg(0, "%s: usage: resize [-h] [-v] [-l] [num]\n", rc_name);
		return;
	}
	if (*args)
		ResizeRegions(*args, i);
	else
		Input(resizeprompts[i], 20, INP_EVERY, ResizeFin, NULL, i);
}

static void DoCommandSetsid(struct action *act)
{
	(void)ParseSwitch(act, &separate_sids);
}

static void DoCommandEval(struct action *act)
{
	char **args = act->args;

	args = SaveArgs(args);
	for (int i = 0; args[i]; i++) {
		if (args[i][0])
			ColonFin(args[i], strlen(args[i]), NULL);
		free(args[i]);
	}
	free(args);
}

static void DoCommandAltscreen(struct action *act)
{
	int msgok = display && !*rc_name;


	(void)ParseSwitch(act, &use_altscreen);
	if (msgok)
		OutputMsg(0, "Will %sdo alternate screen switching", use_altscreen ? "" : "not ");
}


static void DoCommandAuth(struct action *act)
{
        int msgok = display && !*rc_name;

        (void)ParseSwitch(act, &do_auth);
        if (msgok)
                OutputMsg(0, "Authentication: %s" , do_auth ? "enabled" : "disabled");
}


static void DoCommandBacktick(struct action *act)
{
	char **args = act->args;
	int argc = CheckArgNum(act->nr, args);
	int n = 0;

	if (ParseBase(act, *args, &n, 10, "decimal"))
		return;
	if (!args[1])
		setbacktick(n, 0, 0, NULL);
	else {
		int lifespan, tick;
		if (argc < 4) {
			OutputMsg(0, "%s: usage: backtick num [lifespan tick cmd args...]", rc_name);
			return;
		}
		if (ParseBase(act, args[1], &lifespan, 10, "decimal"))
			return;
		if (ParseBase(act, args[2], &tick, 10, "decimal"))
			return;
		setbacktick(n, lifespan, tick, SaveArgs(args + 3));
	}
	WindowChanged(NULL, WINESC_BACKTICK);
}

static void DoCommandBlanker(struct action *act)
{
	(void)act; /* unused */

	if (blankerprg) {
		RunBlanker(blankerprg);
		return;
	}
	ClearAll();
	CursorVisibility(-1);
	D_blocked = 4;
}

static void DoCommandBlankerprg(struct action *act)
{
	char **args = act->args;

	if (!args[0]) {
		if (blankerprg) {
			char path[MAXPATHLEN];
			char *p = path, **pp;
			for (pp = blankerprg; *pp; pp++)
				p += snprintf(p, ARRAY_SIZE(path) - (p - path) - 1, "%s ", *pp);
			*(p - 1) = '\0';
			OutputMsg(0, "blankerprg: %s", path);
		} else
			OutputMsg(0, "No blankerprg set.");
		return;
	}
	if (blankerprg) {
		char **pp;
		for (pp = blankerprg; *pp; pp++)
			free(*pp);
		free(blankerprg);
		blankerprg = NULL;
	}
	if (args[0][0])
		blankerprg = SaveArgs(args);
}

static void DoCommandIdle(struct action *act)
{
	char **args = act->args;
	int *argl = act->argl;
	int argc = CheckArgNum(act->nr, args);
	int msgok = display && !*rc_name;

	if (*args) {
		Display *olddisplay = display;
		if (!strcmp(*args, "off"))
			idletimo = 0;
		else if (args[0][0])
			idletimo = atoi(*args) * 1000;
		if (argc > 1) {
			int i;
			if ((i = FindCommnr(args[1])) == RC_ILLEGAL) {
				OutputMsg(0, "%s: idle: unknown command '%s'", rc_name, args[1]);
				return;
			}
			if (CheckArgNum(i, args + 2) < 0)
				return;
			ClearAction(&idleaction);
			SaveAction(&idleaction, i, args + 2, argl + 2);
		}
		for (display = displays; display; display = display->d_next)
			ResetIdle();
		display = olddisplay;
	}
	if (msgok) {
		if (idletimo)
			OutputMsg(0, "idle timeout %ds, %s", idletimo / 1000, comms[idleaction.nr].name);
		else
			OutputMsg(0, "idle off");
	}
}

static void DoCommandFocusminsize(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;
	int n = 0;

	for (int i = 0; i < 2 && args[i]; i++) {
		if (!strcmp(args[i], "max") || !strcmp(args[i], "_"))
			n = -1;
		else
			n = atoi(args[i]);
		if (i == 0)
			focusminwidth = n;
		else
			focusminheight = n;
	}
	if (msgok) {
		char b[2][20];
		for (int i = 0; i < 2; i++) {
			n = i == 0 ? focusminwidth : focusminheight;
			if (n == -1)
				strncpy(b[i], "max", 20);
			else
				sprintf(b[i], "%d", n);
		}
		OutputMsg(0, "focus min size is %s %s\n", b[0], b[1]);
	}
}

static void DoCommandGroup(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;

	if (*args) {
		fore->w_group = NULL;
		if (args[0][0]) {
			fore->w_group = WindowByName(*args);
			if (fore->w_group == fore || (fore->w_group && fore->w_group->w_type != W_TYPE_GROUP))
				fore->w_group = NULL;
		}
		WindowChanged(NULL, WINESC_WIN_NAMES);
		WindowChanged(NULL, WINESC_WIN_NAMES_NOCUR);
		WindowChanged(NULL, 0);
	}
	if (msgok) {
		if (fore->w_group)
			OutputMsg(0, "window group is %d (%s)\n", fore->w_group->w_number,
				  fore->w_group->w_title);
		else
			OutputMsg(0, "window belongs to no group");
	}
}

static void DoCommandLayout(struct action *act)
{
	char **args = act->args;
	int msgok = display && !*rc_name;

	if (!*args)
		return;
	// A number of the subcommands for "layout" are ignored, or not processed correctly when there
	// is no attached display.

	if (!strcmp(args[0], "title")) {
		if (!display) {
			if (!args[1])	// There is no display, and there is no new title. Ignore.
				return;
			if (!layout_attach || layout_attach == &layout_last_marker)
				layout_attach = CreateLayout(args[1], 0);
			else
				RenameLayout(layout_attach, args[1]);
			return;
		}

		if (!D_layout) {
			OutputMsg(0, "not on a layout");
			return;
		}
		if (!args[1]) {
			OutputMsg(0, "current layout is %d (%s)", D_layout->lay_number, D_layout->lay_title);
			return;
		}
		RenameLayout(D_layout, args[1]);
	} else if (!strcmp(args[0], "number")) {
		if (!display) {
			if (args[1] && layout_attach && layout_attach != &layout_last_marker)
				RenumberLayout(layout_attach, atoi(args[1]));
			return;
		}

		if (!D_layout) {
			OutputMsg(0, "not on a layout");
			return;
		}
		if (!args[1]) {
			OutputMsg(0, "This is layout %d (%s).\n", D_layout->lay_number, D_layout->lay_title);
			return;
		}
		RenumberLayout(D_layout, atoi(args[1]));
		return;
	} else if (!strcmp(args[0], "autosave")) {
		if (!display) {
			if (args[1] && layout_attach && layout_attach != &layout_last_marker) {
				if (!strcmp(args[1], "on"))
					layout_attach->lay_autosave = 1;
				else if (!strcmp(args[1], "off"))
					layout_attach->lay_autosave = 0;
			}
			return;
		}

		if (!D_layout) {
			OutputMsg(0, "not on a layout");
			return;
		}
		if (args[1]) {
			if (!strcmp(args[1], "on"))
				D_layout->lay_autosave = 1;
			else if (!strcmp(args[1], "off"))
				D_layout->lay_autosave = 0;
			else {
				OutputMsg(0, "invalid argument. Give 'on' or 'off");
				return;
			}
		}
		if (msgok)
			OutputMsg(0, "autosave is %s", D_layout->lay_autosave ? "on" : "off");
	} else if (!strcmp(args[0], "new")) {
		char *t = args[1];
		int n = 0;
		if (t) {
			while (*t >= '0' && *t <= '9')
				t++;
			if (t != args[1] && (!*t || *t == ':')) {
				n = atoi(args[1]);
				if (*t)
					t++;
			} else
				t = args[1];
		}
		if (!t || !*t)
			t = "layout";
		NewLayout(t, n);
		Activate(-1);
	} else if (!strcmp(args[0], "save")) {
		if (!args[1]) {
			OutputMsg(0, "usage: layout save <name>");
			return;
		}
		if (display)
			SaveLayout(args[1], &D_canvas);
	} else if (!strcmp(args[0], "select")) {
		if (!display) {
			if (args[1])
				layout_attach = FindLayout(args[1]);
			return;
		}
		if (!args[1]) {
			Input("Switch to layout: ", 20, INP_COOKED, SelectLayoutFin, NULL, 0);
			return;
		}
		SelectLayoutFin(args[1], strlen(args[1]), NULL);
	} else if (!strcmp(args[0], "next")) {
		if (!display) {
			if (layout_attach && layout_attach != &layout_last_marker)
				layout_attach = layout_attach->lay_next ? layout_attach->lay_next : layouts;;
			return;
		}
		Layout *lay = D_layout;
		if (lay)
			lay = lay->lay_next ? lay->lay_next : layouts;
		else
			lay = layouts;
		if (!lay) {
			OutputMsg(0, "no layout defined");
			return;
		}
		if (lay == D_layout)
			return;
		LoadLayout(lay);
		Activate(-1);
	} else if (!strcmp(args[0], "prev")) {
		Layout *lay = display ? D_layout : layout_attach;
		Layout *target = lay;
		if (lay) {
			for (lay = layouts; lay->lay_next && lay->lay_next != target; lay = lay->lay_next) ;
		} else
			lay = layouts;

		if (!display) {
			layout_attach = lay;
			return;
		}

		if (!lay) {
			OutputMsg(0, "no layout defined");
			return;
		}
		if (lay == D_layout)
			return;
		LoadLayout(lay);
		Activate(-1);
	} else if (!strcmp(args[0], "attach")) {
		if (!args[1]) {
			if (!layout_attach)
				OutputMsg(0, "no attach layout set");
			else if (layout_attach == &layout_last_marker)
				OutputMsg(0, "will attach to last layout");
			else
				OutputMsg(0, "will attach to layout %d (%s)", layout_attach->lay_number,
					  layout_attach->lay_title);
			return;
		}
		if (!strcmp(args[1], ":last"))
			layout_attach = &layout_last_marker;
		else if (!args[1][0])
			layout_attach = NULL;
		else {
			Layout *lay;
			lay = FindLayout(args[1]);
			if (!lay) {
				OutputMsg(0, "unknown layout '%s'", args[1]);
				return;
			}
			layout_attach = lay;
		}
	} else if (!strcmp(args[0], "show")) {
		ShowLayouts(-1);
	} else if (!strcmp(args[0], "remove")) {
		Layout *lay = display ? D_layout : layouts;
		if (args[1]) {
			lay = layouts ? FindLayout(args[1]) : NULL;
			if (!lay) {
				OutputMsg(0, "unknown layout '%s'", args[1]);
				return;
			}
		}
		if (lay)
			RemoveLayout(lay);
	} else if (!strcmp(args[0], "dump")) {
		if (!display)
			OutputMsg(0, "Must have a display for 'layout dump'.");
		else if (!LayoutDumpCanvas(&D_canvas, args[1] ? args[1] : "layout-dump"))
			OutputMsg(errno, "Error dumping layout.");
		else
			OutputMsg(0, "Layout dumped to \"%s\"", args[1] ? args[1] : "layout-dump");
	} else
		OutputMsg(0, "unknown layout subcommand");
}

static void DoCommandCjkwidth(struct action *act)
{
	int msgok = display && !*rc_name;

	if (ParseSwitch(act, &cjkwidth) == 0) {
		if (msgok)
			OutputMsg(0, "Treat ambiguous width characters as %s width",
				  cjkwidth ? "full" : "half");
	}
}

static void DoCommandTruecolor(struct action *act)
{
	if (ParseOnOff(act, &hastruecolor) == 0)
		Activate(-1);
}

void DoAction(struct action *act)
{
	int nr = act->nr;
	char **args = act->args;
	int argc, n;
	Display *odisplay = display;

	if (nr == RC_ILLEGAL) {
		return;
	}
	n = comms[nr].flags;
	/* Commands will have a CAN_QUERY flag, depending on whether they have
	 * something to return on a query. For example, 'windows' can return a result,
	 * but 'other' cannot.
	 * If some command causes an error, then it should reset queryflag to -1, so that
	 * the process requesting the query can be notified that an error happened.
	 */
	if (!(n & CAN_QUERY) && queryflag >= 0) {
		/* Query flag is set, but this command cannot be queried. */
		OutputMsg(0, "%s command cannot be queried.", comms[nr].name);
		queryflag = -1;
		return;
	}
	if ((n & NEED_DISPLAY) && display == NULL) {
		OutputMsg(0, "%s: %s: display required", rc_name, comms[nr].name);
		queryflag = -1;
		return;
	}
	if ((n & NEED_FORE) && fore == NULL) {
		OutputMsg(0, "%s: %s: window required", rc_name, comms[nr].name);
		queryflag = -1;
		return;
	}
	if ((n & NEED_LAYER) && flayer == NULL) {
		OutputMsg(0, "%s: %s: display or window required", rc_name, comms[nr].name);
		queryflag = -1;
		return;
	}
	if ((argc = CheckArgNum(nr, args)) < 0)
		return;
	if (display) {
		if (AclCheckPermCmd(D_user, ACL_EXEC, &comms[nr])) {
			OutputMsg(0, "%s: %s: permission denied (user %s)",
				  rc_name, comms[nr].name, (EffectiveAclUser ? EffectiveAclUser : D_user)->u_name);
			queryflag = -1;
			return;
		}
	}

	switch (nr) {
	case RC_SELECT:
		DoCommandSelect(act);
		break;
	case RC_MULTIINPUT:
		DoCommandMultiinput(act);
		break;
	case RC_DEFAUTONUKE:
		DoCommandDefautonuke(act);
		break;
	case RC_AUTONUKE:
		DoCommandAutonuke(act);
		break;
	case RC_DEFOBUFLIMIT:
		DoCommandDefobuflimit(act);
		break;
	case RC_OBUFLIMIT:
		DoCommandObuflimit(act);
		break;
	case RC_DUMPTERMCAP:
		DoCommandDumptermcap(act);
		break;
	case RC_HARDCOPY:
		DoCommandHardcopy(act);
		break;
	case RC_DEFLOG:
		DoCommandDeflog(act);
		break;
	case RC_LOG:
		DoCommandLog(act);
		break;
	case RC_SUSPEND:
		DoCommandSuspend(act);
		break;
	case RC_NEXT:
		DoCommandNext(act);
		break;
	case RC_PREV:
		DoCommandPrev(act);
		break;
	case RC_PARENT:
		DoCommandParent(act);
		break;
	case RC_KILL:
		DoCommandKill(act);
		break;
	case RC_QUIT:
		DoCommandQuit(act);
		break;
	case RC_DETACH:
		DoCommandDetach(act);
		break;
	case RC_POW_DETACH:
		DoCommandPow_detach(act);
		break;
	case RC_ZMODEM:
		DoCommandZmodem(act);
		break;
	case RC_UNBINDALL:
		DoCommandUnbindall(act);
		break;
	case RC_ZOMBIE:
		DoCommandZombie(act);
		break;
	case RC_WALL:
		DoCommandWall(act);
		break;
	case RC_AT:
		DoCommandAt(act);
		break;
	case RC_READREG:
		DoCommandReadreg(act);
		break;
	case RC_REGISTER:
		DoCommandRegister(act);
		break;
	case RC_PROCESS:
		DoCommandProcess(act);
		break;
	case RC_STUFF:
		DoCommandStuff(act);
		break;
	case RC_REDISPLAY:
		DoCommandRedisplay(act);
		break;
	case RC_WINDOWS:
		DoCommandWindows(act);
		break;
	case RC_VERSION:
		DoCommandVersion(act);
		break;
	case RC_INFO:
		DoCommandInfo(act);
		break;
	case RC_DINFO:
		DoCommandDinfo(act);
		break;
	case RC_COMMAND:
		DoCommandCommand(act);
		break;
	case RC_OTHER:
		DoCommandOther(act);
		break;
	case RC_META:
		DoCommandMeta(act);
		break;
	case RC_XON:
		DoCommandXon(act);
		break;
	case RC_XOFF:
		DoCommandXoff(act);
		break;
	case RC_DEFBREAKTYPE:
	case RC_BREAKTYPE:
		DoCommandBreaktype(act);
		break;
	case RC_POW_BREAK:
		DoCommandPow_break(act);
		break;
	case RC_BREAK:
		DoCommandBreak(act);
		break;
	case RC_LOCKSCREEN:
		DoCommandLockscreen(act);
		break;
	case RC_WIDTH:
		DoCommandWidth(act);
		break;
	case RC_HEIGHT:
		DoCommandHeight(act);
		break;
	case RC_DEFDYNAMICTITLE:
		DoCommandDefdynamictitle(act);
		break;
	case RC_DYNAMICTITLE:
		DoCommandDynamictitle(act);
		break;
	case RC_TITLE:
		DoCommandTitle(act);
		break;
	case RC_COLON:
		DoCommandColon(act);
		break;
	case RC_LASTMSG:
		DoCommandLastmsg(act);
		break;
	case RC_SCREEN:
		DoCommandScreen(act);
		break;
	case RC_WRAP:
		DoCommandWrap(act);
		break;
	case RC_FLOW:
		DoCommandFlow(act);
		break;
	case RC_DEFWRITELOCK:
		DoCommandDefwritelock(act);
		break;
	case RC_WRITELOCK:
		DoCommandWritelock(act);
		break;
	case RC_CLEAR:
		DoCommandClear(act);
		break;
	case RC_RESET:
		DoCommandReset(act);
		break;
	case RC_MONITOR:
		DoCommandMonitor(act);
		break;
	case RC_DISPLAYS:
		DoCommandDisplays(act);
		break;
	case RC_WINDOWLIST:
		DoCommandWindowlist(act);
		break;
	case RC_HELP:
		DoCommandHelp(act);
		break;
	case RC_LICENSE:
		DoCommandLicense(act);
		break;
	case RC_COPY:
		DoCommandCopy(act);
		break;
	case RC_HISTORY:
		DoCommandHistory(act);
		break;
	case RC_PASTE:
		DoCommandPaste(act);
		break;
	case RC_WRITEBUF:
		DoCommandWritebuf(act);
		break;
	case RC_READBUF:
		DoCommandReadbuf(act);
		break;
	case RC_REMOVEBUF:
		DoCommandRemovebuf(act);
		break;
	case RC_IGNORECASE:
		DoCommandIgnorecase(act);
		break;
	case RC_ESCAPE:
		DoCommandEscape(act);
		break;
	case RC_DEFESCAPE:
		DoCommandDefescape(act);
		break;
	case RC_CHDIR:
		DoCommandChdir(act);
		break;
	case RC_SHELL:
	case RC_DEFSHELL:
		DoCommandShell(act);
		break;
	case RC_HARDCOPYDIR:
		DoCommandHardcopydir(act);
		break;
	case RC_LOGFILE:
		DoCommandLogfile(act);
		break;
	case RC_LOGTSTAMP:
		DoCommandLogtstamp(act);
		break;
	case RC_SHELLTITLE:
		DoCommandShelltitle(act);
		break;
	case RC_TERMCAP:
	case RC_TERMCAPINFO:
	case RC_TERMINFO:
		DoCommandTerminfo(act);
		break;
	case RC_SLEEP:
		DoCommandSleep(act);
		break;
	case RC_TERM:
		DoCommandTerm(act);
		break;
	case RC_ECHO:
		DoCommandEcho(act);
		break;
	case RC_BELL:
	case RC_BELL_MSG:
		DoCommandBell(act);
		break;
	case RC_BUFFERFILE:
		DoCommandBufferfile(act);
		break;
	case RC_ACTIVITY:
		DoCommandActivity(act);
		break;
	case RC_POW_DETACH_MSG:
		DoCommandPow_detach_msg(act);
		break;
#if defined(ENABLE_UTMP) && defined(LOGOUTOK)
	case RC_LOGIN:
		DoCommandLogin(act);
		break;
	case RC_DEFLOGIN:
		DoCommandDeflogin(act);
		break;
#endif
	case RC_DEFFLOW:
		DoCommandDefflow(act);
		break;
	case RC_DEFWRAP:
		DoCommandDefwrap(act);
		break;
	case RC_DEFC1:
		DoCommandDefc1(act);
		break;
	case RC_DEFBCE:
		DoCommandDefbce(act);
		break;
	case RC_DEFGR:
		DoCommandDefgr(act);
		break;
	case RC_DEFMONITOR:
		DoCommandDefmonitor(act);
		break;
	case RC_DEFMOUSETRACK:
		DoCommandDefmousetrack(act);
		break;
	case RC_MOUSETRACK:
		DoCommandMousetrack(act);
		break;
	case RC_DEFSILENCE:
		DoCommandDefsilence(act);
		break;
	case RC_VERBOSE:
		DoCommandVerbose(act);
		break;
	case RC_HARDSTATUS:
		DoCommandHardstatus(act);
		break;
	case RC_STATUS:
		DoCommandStatus(act);
		break;
	case RC_CAPTION:
		DoCommandCaption(act);
		break;
	case RC_CONSOLE:
		DoCommandConsole(act);
		break;
	case RC_ALLPARTIAL:
		DoCommandAllpartial(act);
		break;
	case RC_PARTIAL:
		DoCommandPartial(act);
		break;
	case RC_VBELL:
		DoCommandVbell(act);
		break;
	case RC_VBELLWAIT:
		DoCommandVbellwait(act);
		break;
	case RC_MSGWAIT:
		DoCommandMsgwait(act);
		break;
	case RC_MSGMINWAIT:
		DoCommandMsgminwait(act);
		break;
	case RC_SILENCEWAIT:
		DoCommandSilencewait(act);
		break;
	case RC_BUMPRIGHT:
		DoCommandBumpright(act);
		break;
	case RC_BUMPLEFT:
		DoCommandBumpleft(act);
		break;
	case RC_COLLAPSE:
		DoCommandCollapse(act);
		break;
	case RC_NUMBER:
		DoCommandNumber(act);
		break;
	case RC_ZOMBIE_TIMEOUT:
		DoCommandZombie_timeout(act);
		break;
	case RC_SORT:
		DoCommandSort(act);
		break;
	case RC_SILENCE:
		DoCommandSilence(act);
		break;
	case RC_DEFSCROLLBACK:
		DoCommandDefscrollback(act);
		break;
	case RC_SCROLLBACK:
		DoCommandScrollback(act);
		break;
	case RC_SESSIONNAME:
		DoCommandSessionname(act);
		break;
	case RC_SETENV:
		DoCommandSetenv(act);
		break;
	case RC_UNSETENV:
		DoCommandUnsetenv(act);
		break;
	case RC_DEFSLOWPASTE:
		DoCommandDefslowpaste(act);
		break;
	case RC_SLOWPASTE:
		DoCommandSlowpaste(act);
		break;
	case RC_MARKKEYS:
		DoCommandMarkkeys(act);
		break;
	case RC_PASTEFONT:
		DoCommandPastefont(act);
		break;
	case RC_CRLF:
		DoCommandCrlf(act);
		break;
	case RC_COMPACTHIST:
		DoCommandCompacthist(act);
		break;
	case RC_HARDCOPY_APPEND:
		DoCommandHardcopy_append(act);
		break;
	case RC_VBELL_MSG:
		DoCommandVbell_msg(act);
		break;
	case RC_DEFMODE:
		DoCommandDefmode(act);
		break;
	case RC_AUTODETACH:
		DoCommandAutodetach(act);
		break;
	case RC_STARTUP_MESSAGE:
		DoCommandStartup_message(act);
		break;
	case RC_BIND:
		DoCommandBind(act);
		break;
	case RC_BINDKEY:
		DoCommandBindkey(act);
		break;
	case RC_MAPTIMEOUT:
		DoCommandMaptimeout(act);
		break;
	case RC_MAPNOTNEXT:
		DoCommandMapnotnext(act);
		break;
	case RC_MAPDEFAULT:
		DoCommandMapdefault(act);
		break;
	case RC_ACLCHG:
	case RC_ACLADD:
	case RC_ADDACL:
	case RC_CHACL:
		DoCommandAclchg(act);
		break;
	case RC_ACLDEL:
		DoCommandAcldel(act);
		break;
	case RC_ACLGRP:
		DoCommandAclgrp(act);
		break;
	case RC_ACLUMASK:
	case RC_UMASK:
		DoCommandAclumask(act);
		break;
	case RC_MULTIUSER:
		DoCommandMultiuser(act);
		break;
	case RC_EXEC:
		DoCommandExec(act);
		break;
	case RC_NONBLOCK:
		DoCommandNonblock(act);
		break;
	case RC_DEFNONBLOCK:
		DoCommandDefnonblock(act);
		break;
	case RC_GR:
		DoCommandGr(act);
		break;
	case RC_C1:
		DoCommandC1(act);
		break;
	case RC_KANJI:
	case RC_ENCODING:
		DoCommandEncoding(act);
		break;
	case RC_DEFKANJI:
	case RC_DEFENCODING:
		DoCommandDefencoding(act);
		break;
	case RC_DEFUTF8:
		DoCommandDefutf8(act);
		break;
	case RC_UTF8:
		DoCommandUtf8(act);
		break;
	case RC_PRINTCMD:
		DoCommandPrintcmd(act);
		break;
	case RC_DIGRAPH:
		DoCommandDigraph(act);
		break;
	case RC_DEFHSTATUS:
		DoCommandDefhstatus(act);
		break;
	case RC_HSTATUS:
		DoCommandHstatus(act);
		break;
	case RC_DEFCHARSET:
		DoCommandDefcharset(act);
		break;
	case RC_CHARSET:
		DoCommandCharset(act);
		break;
	case RC_RENDITION:
		DoCommandRendition(act);
		break;
	case RC_SORENDITION:
		DoCommandSorendition(act);
		break;
	case RC_SOURCE:
		DoCommandSource(act);
		break;
	case RC_SU:
		DoCommandSu(act);
		break;
	case RC_SPLIT:
		DoCommandSplit(act);
		break;
	case RC_REMOVE:
		DoCommandRemove(act);
		break;
	case RC_ONLY:
		DoCommandOnly(act);
		break;
	case RC_FIT:
		DoCommandFit(act);
		break;
	case RC_FOCUS:
		DoCommandFocus(act);
		break;
	case RC_RESIZE:
		DoCommandResize(act);
		break;
	case RC_SETSID:
		DoCommandSetsid(act);
		break;
	case RC_EVAL:
		DoCommandEval(act);
		break;
	case RC_ALTSCREEN:
		DoCommandAltscreen(act);
		break;
	case RC_AUTH:
		DoCommandAuth(act);
		break;
	case RC_BACKTICK:
		DoCommandBacktick(act);
		break;
	case RC_BLANKER:
		DoCommandBlanker(act);
		break;
	case RC_BLANKERPRG:
		DoCommandBlankerprg(act);
		break;
	case RC_IDLE:
		DoCommandIdle(act);
		break;
	case RC_FOCUSMINSIZE:
		DoCommandFocusminsize(act);
		break;
	case RC_GROUP:
		DoCommandGroup(act);
		break;
	case RC_LAYOUT:
		DoCommandLayout(act);
		break;
	case RC_CJKWIDTH:
		DoCommandCjkwidth(act);
		break;
	case RC_TRUECOLOR:
		DoCommandTruecolor(act);
		break;
	default:
		break;
	}
	if (display != odisplay) {
		for (display = displays; display; display = display->d_next)
			if (display == odisplay)
				break;
	}
}

#undef OutputMsg

void CollapseWindowlist(void)
/* renumber windows from 0, leaving no gaps */
{
	int n = 0;

	for (Window *w = first_window; w; w = w->w_next)
		w->w_number = n++;
}

void DoCommand(char **argv, int *argl)
{
	struct action act;
	const char *cmd = *argv;

	act.quiet = 0;
	/* For now, we actually treat both 'supress error' and 'suppress normal message' as the
	 * same, and ignore all messages on either flag. If we wanted to do otherwise, we would
	 * need to change the definition of 'OutputMsg' slightly. */
	if (*cmd == '@') {	/* Suppress error */
		act.quiet |= 0x01;
		cmd++;
	}
	if (*cmd == '-') {	/* Suppress normal message */
		act.quiet |= 0x02;
		cmd++;
	}

	if ((act.nr = FindCommnr(cmd)) == RC_ILLEGAL) {
		Msg(0, "%s: unknown command '%s'", rc_name, cmd);
		return;
	}
	act.args = argv + 1;
	act.argl = argl + 1;
	DoAction(&act);
}

static void SaveAction(struct action *act, int nr, char **args, int *argl)
{
	int argc = 0;
	char **pp;
	int *lp;

	if (args)
		while (args[argc])
			argc++;
	if (argc == 0) {
		act->nr = nr;
		act->args = noargs;
		act->argl = NULL;
		return;
	}
	if ((pp = malloc((unsigned)(argc + 1) * sizeof(char *))) == NULL)
		Panic(0, "%s", strnomem);
	if ((lp = malloc((unsigned)(argc) * sizeof(int))) == NULL)
		Panic(0, "%s", strnomem);
	act->nr = nr;
	act->args = pp;
	act->argl = lp;
	while (argc--) {
		*lp = argl ? *argl++ : (int)strlen(*args);
		*pp++ = SaveStrn(*args++, *lp++);
	}
	*pp = NULL;
}

static char **SaveArgs(char **args)
{
	char **ap, **pp;
	int argc = 0;

	while (args[argc])
		argc++;
	if ((pp = ap = malloc((unsigned)(argc + 1) * sizeof(char *))) == NULL)
		Panic(0, "%s", strnomem);
	while (argc--)
		*pp++ = SaveStr(*args++);
	*pp = NULL;
	return ap;
}

/*
 * buf is split into argument vector args.
 * leading whitespace is removed.
 * @!| abbreviations are expanded.
 * the end of buffer is recognized by '\0' or an un-escaped '#'.
 * " and ' are interpreted.
 *
 * argc is returned.
 */
int Parse(char *buf, int bufl, char **args, int *argl)
{
	char *p = buf, **ap = args, *pp;
	int delim, argc;
	int *lp = argl;

	argc = 0;
	pp = buf;
	delim = 0;
	for (;;) {
		*lp = 0;
		while (*p && (*p == ' ' || *p == '\t'))
			++p;
		if (argc == 0 && *p == '!') {
			*ap++ = "exec";
			*lp++ = 4;
			p++;
			argc++;
			continue;
		}
		if (*p == '\0' || *p == '#' || *p == '\n') {
			*p = '\0';
			args[argc] = NULL;
			return argc;
		}
		if (++argc >= MAXARGS) {
			Msg(0, "%s: too many tokens.", rc_name);
			return 0;
		}
		*ap++ = pp;

		while (*p) {
			if (*p == delim)
				delim = 0;
			else if (delim != '\'' && *p == '\\'
				 && (p[1] == 'n' || p[1] == 'r' || p[1] == 't' || p[1] == '\'' || p[1] == '"'
				     || p[1] == '\\' || p[1] == '$' || p[1] == '#' || p[1] == '^' || (p[1] >= '0'
												      && p[1] <=
												      '7'))) {
				p++;
				if (*p >= '0' && *p <= '7') {
					*pp = *p - '0';
					if (p[1] >= '0' && p[1] <= '7') {
						p++;
						*pp = (*pp << 3) | (*p - '0');
						if (p[1] >= '0' && p[1] <= '7') {
							p++;
							*pp = (*pp << 3) | (*p - '0');
						}
					}
					pp++;
				} else {
					switch (*p) {
					case 'n':
						*pp = '\n';
						break;
					case 'r':
						*pp = '\r';
						break;
					case 't':
						*pp = '\t';
						break;
					default:
						*pp = *p;
						break;
					}
					pp++;
				}
			} else if (delim != '\'' && *p == '$'
				   && (p[1] == '{' || p[1] == ':' || (p[1] >= 'a' && p[1] <= 'z')
				       || (p[1] >= 'A' && p[1] <= 'Z') || (p[1] >= '0' && p[1] <= '9') || p[1] == '_'))
			{
				char *ps, *pe, op, *v, xbuf[11], path[MAXPATHLEN];
				int vl;

				ps = ++p;
				p++;
				while (*p) {
					if (*ps == '{' && *p == '}')
						break;
					if (*ps == ':' && *p == ':')
						break;
					if (*ps != '{' && *ps != ':' && (*p < 'a' || *p > 'z') && (*p < 'A' || *p > 'Z')
					    && (*p < '0' || *p > '9') && *p != '_')
						break;
					p++;
				}
				pe = p;
				if (*ps == '{' || *ps == ':') {
					if (!*p) {
						Msg(0, "%s: bad variable name.", rc_name);
						return 0;
					}
					p++;
				}
				op = *pe;
				*pe = 0;
				if (*ps == ':')
					v = gettermcapstring(ps + 1);
				else {
					if (*ps == '{')
						ps++;
					v = xbuf;
					if (!strcmp(ps, "TERM"))
						v = display ? D_termname : "unknown";
					else if (!strcmp(ps, "COLUMNS"))
						sprintf(xbuf, "%d", display ? D_width : -1);
					else if (!strcmp(ps, "LINES"))
						sprintf(xbuf, "%d", display ? D_height : -1);
					else if (!strcmp(ps, "PID"))
						sprintf(xbuf, "%d", getpid());
					else if (!strcmp(ps, "PWD")) {
						if (getcwd(path, ARRAY_SIZE(path) - 1) == NULL)
							v = "?";
						else
							v = path;
					} else if (!strcmp(ps, "STY")) {
						if ((v = strchr(SocketName, '.')))	/* Skip the PID */
							v++;
						else
							v = SocketName;
					} else
						v = getenv(ps);
				}
				*pe = op;
				vl = v ? strlen(v) : 0;
				if (vl) {
					if (p - pp < vl) {
						int right = buf + bufl - (p + strlen(p) + 1);
						if (right > 0) {
							memmove(p + right, p, strlen(p) + 1);
							p += right;
						}
					}
					if (p - pp < vl) {
						Msg(0, "%s: no space left for variable expansion.", rc_name);
						return 0;
					}
					memmove(pp, v, vl);
					pp += vl;
				}
				continue;
			} else if (delim != '\'' && *p == '^' && p[1]) {
				p++;
				*pp++ = *p == '?' ? '\177' : *p & 0x1f;
			} else if (delim == 0 && (*p == '\'' || *p == '"'))
				delim = *p;
			else if (delim == 0 && (*p == ' ' || *p == '\t' || *p == '\n'))
				break;
			else
				*pp++ = *p;
			p++;
		}
		if (delim) {
			Msg(0, "%s: Missing %c quote.", rc_name, delim);
			return 0;
		}
		if (*p)
			p++;
		*pp = 0;
		*lp++ = pp - ap[-1];
		pp++;
	}
}

void SetEscape(struct acluser *u, int e, int me)
{
	if (u) {
		u->u_Esc = e;
		u->u_MetaEsc = me;
	} else {
		if (users) {
			if (DefaultEsc >= 0)
				ClearAction(&ktab[DefaultEsc]);
			if (DefaultMetaEsc >= 0)
				ClearAction(&ktab[DefaultMetaEsc]);
		}
		DefaultEsc = e;
		DefaultMetaEsc = me;
		if (users) {
			if (DefaultEsc >= 0) {
				ClearAction(&ktab[DefaultEsc]);
				ktab[DefaultEsc].nr = RC_OTHER;
			}
			if (DefaultMetaEsc >= 0) {
				ClearAction(&ktab[DefaultMetaEsc]);
				ktab[DefaultMetaEsc].nr = RC_META;
			}
		}
	}
}

static int ParseSwitch(struct action *act, bool *var)
{
	if (*act->args == NULL) {
		*var ^= true;
		return 0;
	}
	return ParseOnOff(act, var);
}

static int ParseOnOff(struct action *act, bool *var)
{
	int num = -1;
	char **args = act->args;

	if (*args && args[1] == NULL) {
		if (strcmp(args[0], "on") == 0)
			num = true;
		else if (strcmp(args[0], "off") == 0)
			num = false;
	}
	if (num < 0) {
		Msg(0, "%s: %s: invalid argument. Give 'on' or 'off'", rc_name, comms[act->nr].name);
		return -1;
	}
	*var = num;
	return 0;
}

static int ParseSaveStr(struct action *act, char **var)
{
	char **args = act->args;
	if (*args == NULL || args[1]) {
		Msg(0, "%s: %s: one argument required.", rc_name, comms[act->nr].name);
		return -1;
	}
	if (*var)
		free(*var);
	*var = SaveStr(*args);
	return 0;
}

static int ParseNum(struct action *act, int *var)
{
	int i;
	char *p, **args = act->args;

	p = *args;
	if (p == NULL || *p == 0 || args[1]) {
		Msg(0, "%s: %s: invalid argument. Give one argument.", rc_name, comms[act->nr].name);
		return -1;
	}
	i = 0;
	while (*p) {
		if (*p >= '0' && *p <= '9')
			i = 10 * i + (*p - '0');
		else {
			Msg(0, "%s: %s: invalid argument. Give numeric argument.", rc_name, comms[act->nr].name);
			return -1;
		}
		p++;
	}
	*var = i;
	return 0;
}

static int ParseNum1000(struct action *act, int *var)
{
	int i;
	char *p, **args = act->args;
	int dig = 0;

	p = *args;
	if (p == NULL || *p == 0 || args[1]) {
		Msg(0, "%s: %s: invalid argument. Give one argument.", rc_name, comms[act->nr].name);
		return -1;
	}
	i = 0;
	while (*p) {
		if (*p >= '0' && *p <= '9') {
			if (dig < 4)
				i = 10 * i + (*p - '0');
			else if (dig == 4 && *p >= '5')
				i++;
			if (dig)
				dig++;
		} else if (*p == '.' && !dig)
			dig++;
		else {
			Msg(0, "%s: %s: invalid argument. Give floating point argument.", rc_name, comms[act->nr].name);
			return -1;
		}
		p++;
	}
	if (dig == 0)
		i *= 1000;
	else
		while (dig++ < 4)
			i *= 10;
	if (i < 0)
		i = (int)((unsigned int)~0 >> 1);
	*var = i;
	return 0;
}

static Window *WindowByName(char *s)
{
	Window *window;

	for (window = mru_window; window; window = window->w_prev_mru)
		if (!strcmp(window->w_title, s))
			return window;
	for (window = mru_window; window; window = window->w_prev_mru)
		if (!strncmp(window->w_title, s, strlen(s)))
			return window;
	return NULL;
}

static int WindowByNumber(char *string)
{
	int i;
	char *s;

	for (i = 0, s = string; *s; s++) {
		if (*s < '0' || *s > '9')
			break;
		i = i * 10 + (*s - '0');
	}
	return *s ? -1 : i;
}

/*
 * Get window number from Name or Number string.
 * Numbers are tried first, then names, a prefix match suffices.
 * Be careful when assigning numeric strings as WindowTitles.
 */
int WindowByNoN(char *string)
{
	int i;
	Window *window;

	if ((i = WindowByNumber(string)) < 0 || i > last_window->w_number) {
		if ((window = WindowByName(string)))
			return window->w_number;
		return -1;
	}
	return i;
}

static int ParseWinNum(struct action *act, int *var)
{
	char **args = act->args;
	int i = 0;

	if (*args == NULL || args[1]) {
		Msg(0, "%s: %s: one argument required.", rc_name, comms[act->nr].name);
		return -1;
	}

	i = WindowByNoN(*args);
	if (i < 0) {
		Msg(0, "%s: %s: invalid argument. Give window number or name.", rc_name, comms[act->nr].name);
		return -1;
	}
	*var = i;
	return 0;
}

static int ParseBase(struct action *act, char *p, int *var, int base, char *bname)
{
	int i = 0;
	int c;

	if (!p || *p == 0) {
		Msg(0, "%s: %s: empty argument.", rc_name, comms[act->nr].name);
		return -1;
	}
	while ((c = *p++)) {
		if (c >= 'a' && c <= 'z')
			c -= 'a' - 'A';
		if (c >= 'A' && c <= 'Z')
			c -= 'A' - ('0' + 10);
		c -= '0';
		if (c < 0 || c >= base) {
			Msg(0, "%s: %s: argument is not %s.", rc_name, comms[act->nr].name, bname);
			return -1;
		}
		i = base * i + c;
	}
	*var = i;
	return 0;
}

static bool IsNum(char *s)
{
	for (; *s; ++s)
		if (*s < '0' || *s > '9')
			return false;
	return true;
}

int IsNumColon(char *s, char *p, int psize)
{
	char *q;
	if ((q = strrchr(s, ':')) != NULL) {
		strncpy(p, q + 1, psize - 1);
		p[psize - 1] = '\0';
		*q = '\0';
	} else
		*p = '\0';
	return IsNum(s);
}

void SwitchWindow(Window *window)
{
	if (window == NULL) {
		ShowWindows(-1);
		return;
	}
	if (display == NULL) {
		fore = window;
		return;
	}
	if (window == D_fore) {
		Msg(0, "This IS window %d (%s).", window->w_number, window->w_title);
		return;
	}
	if (AclCheckPermWin(D_user, ACL_READ, window)) {
		Msg(0, "Access to window %d denied.", window->w_number);
		return;
	}
	SetForeWindow(window);
	Activate(fore->w_norefresh);
}

/*
 * SetForeWindow changes the window in the input focus of the display.
 * Puts window wi in canvas display->d_forecv.
 */
void SetForeWindow(Window *window)
{
	Window *oldfore;

	if (display == NULL) {
		fore = window;
		return;
	}
	oldfore = Layer2Window(D_forecv->c_layer);
	SetCanvasWindow(D_forecv, window);
	if (oldfore)
		WindowChanged(oldfore, 'u');
	if (window)
		WindowChanged(window, 'u');
	flayer = D_forecv->c_layer;
	/* Activate called afterwards, so no RefreshHStatus needed */
}

/*****************************************************************/

/*
 *  Activate - make fore window active
 *  norefresh = -1 forces a refresh, disregard all_norefresh then.
 */
void Activate(int norefresh)
{
	if (display == NULL)
		return;
	if (D_status) {
		Msg(0, "%s", "");	/* wait till mintime (keep gcc quiet) */
		RemoveStatus();
	}

	if (MayResizeLayer(D_forecv->c_layer))
		ResizeLayer(D_forecv->c_layer, D_forecv->c_xe - D_forecv->c_xs + 1, D_forecv->c_ye - D_forecv->c_ys + 1,
			    display);

	fore = D_fore;
	if (fore) {
		/* XXX ? */
		if (fore->w_monitor != MON_OFF)
			fore->w_monitor = MON_ON;
		fore->w_bell = BELL_ON;
		WindowChanged(fore, WINESC_WFLAGS);
	}
	Redisplay(norefresh + all_norefresh);
}

static Window *NextWindow(void)
{
	Window *w;
	Window *group = fore ? fore->w_group : NULL;

	for (w = fore ? fore->w_next : first_window; w != fore; w = w->w_next) {
		if (w == NULL)
			w = first_window;
		if (!fore || group == w->w_group)
			break;
	}
	return w;
}

static Window *PreviousWindow(void)
{
	Window *w;
	Window *group = fore ? fore->w_group : NULL;

	for (w = fore ? fore->w_prev : last_window; w != fore; w = w->w_prev) {
		if (w == NULL)
			w = last_window;
		if (!fore || group == w->w_group)
			break;
	}
	return w;
}

static Window *ParentWindow(void)
{
	Window *w = fore ? fore->w_group : NULL;
	return w;
}

static int MoreWindows(void)
{
	char *m = "No other window.";
	if (mru_window && (fore == NULL || mru_window->w_prev_mru))
		return 1;
	if (fore == NULL) {
		Msg(0, "No window available");
		return 0;
	}
	Msg(0, m, fore->w_number);
	return 0;
}

void KillWindow(Window *window)
{
	Window **pp, *p;
	Canvas *cv;
	int gotone;
	Layout *lay;

	/*
	 * Remove window from linked list.
	 */
	for (pp = &mru_window; (p = *pp); pp = &p->w_prev_mru)
		if (p == window)
			break;
	*pp = p->w_prev_mru;

	if (first_window == last_window) {
		/* we remove last element, so we can do nothing here */
	} else if (window == first_window) {
		window->w_next->w_prev = NULL;
		first_window = window->w_next;
	} else if (window == last_window) {
		window->w_prev->w_next = NULL;
		last_window = window->w_prev;
	} else {
		window->w_next->w_prev = window->w_prev;
		window->w_prev->w_next = window->w_next;
	}

	window->w_inlen = 0;

	if (mru_window == NULL) {
		FreeWindow(window);
		Finit(0);
	}

	/*
	 * switch to different window on all canvases
	 */
	for (display = displays; display; display = display->d_next) {
		gotone = 0;
		for (cv = D_cvlist; cv; cv = cv->c_next) {
			if (Layer2Window(cv->c_layer) != window)
				continue;
			/* switch to other window */
			SetCanvasWindow(cv, FindNiceWindow(D_other, NULL));
			gotone = 1;
		}
		if (gotone) {
			if (window->w_zdisplay == display) {
				D_blocked = 0;
				D_readev.condpos = D_readev.condneg = NULL;
			}
			Activate(-1);
		}
	}

	/* do the same for the layouts */
	for (lay = layouts; lay; lay = lay->lay_next)
		UpdateLayoutCanvas(&lay->lay_canvas, window);

	FreeWindow(window);
	WindowChanged(NULL, WINESC_WIN_NAMES);
	WindowChanged(NULL, WINESC_WIN_NAMES_NOCUR);
	WindowChanged(NULL, 0);
}

static void LogToggle(bool on)
{
	char buf[1024];

	if ((fore->w_log != NULL) == on) {
		if (display && !*rc_name)
			Msg(0, "You are %s logging.", on ? "already" : "not");
		return;
	}
	if (fore->w_log != NULL) {
		Msg(0, "Logfile \"%s\" closed.", fore->w_log->name);
		logfclose(fore->w_log);
		fore->w_log = NULL;
		WindowChanged(fore, WINESC_WFLAGS);
		return;
	}
	if (DoStartLog(fore, buf, ARRAY_SIZE(buf))) {
		Msg(errno, "Error opening logfile \"%s\"", buf);
		return;
	}
	if (ftell(fore->w_log->fp) == 0)
		Msg(0, "Creating logfile \"%s\".", fore->w_log->name);
	else
		Msg(0, "Appending to logfile \"%s\".", fore->w_log->name);
	WindowChanged(fore, WINESC_WFLAGS);
}

/* TODO: wmb encapsulation; flags enum; update all callers */
/* also maybe give window pointer instead of number in 'where'? */
char *AddWindows(WinMsgBufContext *wmbc, int len, int flags, int where)
{
	char *s, *ss;
	Window *win;
	char *cmd;
	char *buf = wmbc->p;
	int l;
	Window *cur;

	s = ss = buf;
	if (where < 0) {
		*s = 0;
		return ss;
	}

	cur = GetWindowByNumber(where);

	for (win = (flags & 4) ? cur->w_next : first_window; win; win = win->w_next) {
		int rend = -1;
		if (win == cur && ss == buf)
			ss = s;
		if (win == NULL)
			continue;
		if ((flags & 1) && display && win == D_fore)
			continue;
		if (display && D_fore && D_fore->w_group != win->w_group)
			continue;

		cmd = win->w_title;
		l = strlen(cmd);
		if (l > 20)
			l = 20;
		if (s - buf + l > len - 24)
			break;
		if (s > buf || (flags & 4)) {
			*s++ = ' ';
			*s++ = ' ';
		}
		if (win == cur) {
			ss = s;
			if (flags & 8)
				break;
		}
		if (win->w_monitor == MON_DONE && renditions[REND_MONITOR] != 0)
			rend = renditions[REND_MONITOR];
		else if ((win->w_bell == BELL_DONE || win->w_bell == BELL_FOUND) && renditions[REND_BELL] != 0)
			rend = renditions[REND_BELL];
		else if ((win->w_silence == SILENCE_FOUND || win->w_silence == SILENCE_DONE)
			 && renditions[REND_SILENCE] != 0)
			rend = renditions[REND_SILENCE];
		if (rend != -1)
			AddWinMsgRend(wmbc->buf, s, rend);
		sprintf(s, "%d", win->w_number);
		s += strlen(s);
		if (!(flags & 2)) {
			s = AddWindowFlags(s, len, win);
		}
		*s++ = ' ';
		strncpy(s, cmd, l);
		s += l;
		if (rend != -1)
			AddWinMsgRend(wmbc->buf, s, 0);
	}
	*s = 0;
	return ss;
}

char *AddWindowFlags(char *buf, int len, Window *p)
{
	char *s = buf;
	if (p == NULL || len < 12) {
		*s = 0;
		return s;
	}
	if (display && p == D_fore)
		*s++ = '*';
	if (display && p == D_other)
		*s++ = '-';
	if (p->w_layer.l_cvlist && p->w_layer.l_cvlist->c_lnext)
		*s++ = '&';
	if (display && p->w_monitor == MON_DONE && (ACLBYTE(p->w_mon_notify, D_user->u_id) & ACLBIT(D_user->u_id))
	    )
		*s++ = '@';
	if (p->w_bell == BELL_DONE)
		*s++ = '!';
#ifdef ENABLE_UTMP
	if (p->w_slot != (slot_t) 0 && p->w_slot != (slot_t) - 1)
		*s++ = '$';
#endif
	if (p->w_log != NULL) {
		strcpy(s, "(L)");
		s += 3;
	}
	if (p->w_ptyfd < 0 && p->w_type != W_TYPE_GROUP)
		*s++ = 'Z';
	if (p->w_miflag)
		*s++ = '>';
	*s = 0;
	return s;
}

char *AddOtherUsers(char *buf, int len, Window *p)
{
	Display *d, *olddisplay = display;
	Canvas *cv;
	char *s;
	int l;

	s = buf;
	for (display = displays; display; display = display->d_next) {
		if (olddisplay && D_user == olddisplay->d_user)
			continue;
		for (cv = D_cvlist; cv; cv = cv->c_next)
			if (Layer2Window(cv->c_layer) == p)
				break;
		if (!cv)
			continue;
		for (d = displays; d && d != display; d = d->d_next)
			if (D_user == d->d_user)
				break;
		if (d && d != display)
			continue;
		if (len > 1 && s != buf) {
			*s++ = ',';
			len--;
		}
		l = strlen(D_user->u_name);
		if (l + 1 > len)
			break;
		strcpy(s, D_user->u_name);
		s += l;
		len -= l;
	}
	*s = 0;
	display = olddisplay;
	return s;
}


/* Display window list as a message.  WHERE denotes the active window
 * number; if -1, then the active window will be determined using the
 * current foreground window, if available. */
void ShowWindows(int where)
{
	const char *buf, *s, *ss;

	WinMsgBuf *wmb = wmb_create();
	WinMsgBufContext *wmbc = wmbc_create(wmb);
	size_t max = wmbc_bytesleft(wmbc);

	if (display && where == -1 && D_fore)
		where = D_fore->w_number;

	/* TODO: this is a confusing mix of old and new; modernize */
	ss = AddWindows(wmbc, max, 0, where);
	wmbc_fastfw0(wmbc);
	buf = wmbc_finish(wmbc);
	s = buf + strlen(buf);

	if (display && ss - buf > D_width / 2) {
		ss -= D_width / 2;
		if (s - ss < D_width) {
			ss = s - D_width;
			if (ss < buf)
				ss = buf;
		}
	} else
		ss = buf;
	Msg(0, "%s", ss);

	wmbc_free(wmbc);
	wmb_free(wmb);
}

/*
* String Escape based windows listing
* mls: currently does a Msg() call for each(!) window, dunno why
*/
static void ShowWindowsX(char *str)
{
	for (Window *w = first_window; w; w = w->w_next)
		Msg(0, "%s", MakeWinMsg(str, w, '%'));
}

static void ShowInfo(void)
{
	char buf[512], *p;
	Window *wp = fore;
	int i;

	if (wp == NULL) {
		Msg(0, "(%d,%d)/(%d,%d) no window", D_x + 1, D_y + 1, D_width, D_height);
		return;
	}
	p = buf;
	if (buf < (p += GetAnsiStatus(wp, p)))
		*p++ = ' ';
	sprintf(p, "(%d,%d)/(%d,%d)", wp->w_x + 1, wp->w_y + 1, wp->w_width, wp->w_height);
	sprintf(p += strlen(p), "+%d", wp->w_histheight);
	sprintf(p += strlen(p), " %c%sflow",
		(wp->w_flow & FLOW_ON) ? '+' : '-',
		(wp->w_flow & FLOW_AUTOFLAG) ? "" : ((wp->w_flow & FLOW_AUTO) ? "(+)" : "(-)"));
	if (!wp->w_wrap)
		sprintf(p += strlen(p), " -wrap");
	if (wp->w_insert)
		sprintf(p += strlen(p), " ins");
	if (wp->w_origin)
		sprintf(p += strlen(p), " org");
	if (wp->w_keypad)
		sprintf(p += strlen(p), " app");
	if (wp->w_log)
		sprintf(p += strlen(p), " log");
	if (wp->w_monitor != MON_OFF && (ACLBYTE(wp->w_mon_notify, D_user->u_id) & ACLBIT(D_user->u_id))
	    )
		sprintf(p += strlen(p), " mon");
	if (wp->w_mouse)
		sprintf(p += strlen(p), " mouse");
	if (!wp->w_c1)
		sprintf(p += strlen(p), " -c1");
	if (wp->w_norefresh)
		sprintf(p += strlen(p), " nored");

	p += strlen(p);
	if (wp->w_encoding && (display == NULL || D_encoding != wp->w_encoding || EncodingDefFont(wp->w_encoding) <= 0)) {
		*p++ = ' ';
		strcpy(p, EncodingName(wp->w_encoding));
		p += strlen(p);
	}
	if (wp->w_encoding != UTF8)
		if (display && (D_CC0 || (D_CS0 && *D_CS0))) {
			if (wp->w_gr == 2) {
				sprintf(p, " G%c", wp->w_Charset + '0');
				if (wp->w_FontE >= ' ')
					p[3] = wp->w_FontE;
				else {
					p[3] = '^';
					p[4] = wp->w_FontE ^ 0x40;
					p++;
				}
				p[4] = '[';
				p++;
			} else if (wp->w_gr)
				sprintf(p++, " G%c%c[", wp->w_Charset + '0', wp->w_CharsetR + '0');
			else
				sprintf(p, " G%c[", wp->w_Charset + '0');
			p += 4;
			for (i = 0; i < 4; i++) {
				if (wp->w_charsets[i] == ASCII)
					*p++ = 'B';
				else if (wp->w_charsets[i] >= ' ')
					*p++ = wp->w_charsets[i];
				else {
					*p++ = '^';
					*p++ = wp->w_charsets[i] ^ 0x40;
				}
			}
			*p++ = ']';
			*p = 0;
		}

	if (wp->w_type == W_TYPE_PLAIN) {
		/* add info about modem control lines */
		*p++ = ' ';
		TtyGetModemStatus(wp->w_ptyfd, p);
	}
#ifdef ENABLE_TELNET
	else if (wp->w_type == W_TYPE_TELNET) {
		*p++ = ' ';
		TelStatus(wp, p, ARRAY_SIZE(buf) - 1 - (p - buf));
	}
#endif
	Msg(0, "%s %d(%s)", buf, wp->w_number, wp->w_title);
}

static void ShowDInfo(void)
{
	char buf[512], *p;
	int l, w;
	if (display == NULL)
		return;
	p = buf;
	l = 512;
	sprintf(p, "(%d,%d)", D_width, D_height);
	w = strlen(p);
	l -= w;
	p += w;
	if (D_encoding) {
		*p++ = ' ';
		strncpy(p, EncodingName(D_encoding), l);
		w = strlen(p);
		l -= w;
		p += w;
	}
	if (D_CXT) {
		strncpy(p, " xterm", l);
		w = strlen(p);
		l -= w;
		p += w;
	}
	if (D_hascolor) {
		strncpy(p, " color", l);
		w = strlen(p);
		l -= w;
		p += w;
	}
	if (D_CG0)
		strncpy(p, " iso2022", l);
	else if (D_CS0 && *D_CS0)
		strncpy(p, " altchar", l);
	Msg(0, "%s", buf);
}

static void AKAFin(char *buf, size_t len, void *data)
{
	(void)data; /* unused */

	if (len && fore)
		ChangeAKA(fore, buf, strlen(buf));

	enter_window_name_mode = 0;
}

static void InputAKA(void)
{
	char *s, *ss;
	size_t len;

	if (enter_window_name_mode == 1)
		return;

	enter_window_name_mode = 1;

	Input("Set window's title to: ", ARRAY_SIZE(fore->w_akabuf) - 1, INP_COOKED, AKAFin, NULL, 0);
	s = fore->w_title;
	if (!s)
		return;
	for (; *s; s++) {
		if ((*(unsigned char *)s & 0x7f) < 0x20 || *s == 0x7f)
			continue;
		ss = s;
		len = 1;
		LayProcess(&ss, &len);
	}
}

static void ColonFin(char *buf, size_t len, void *data)
{
	char mbuf[256];

	(void)data; /* unused */

	RemoveStatus();
	if (buf[len] == '\t') {
		int m, x;
		int l = 0, r = RC_LAST;
		int showmessage = 0;
		char *s = buf;

		while (*s && (uintptr_t)(s - buf) < len)
			if (*s++ == ' ')
				return;

		/* Showing a message when there's no hardstatus or caption cancels the input */
		if (display &&
		    (captionalways || D_has_hstatus == HSTATUS_LASTLINE
		     || (D_canvas.c_slperp && D_canvas.c_slperp->c_slnext)))
			showmessage = 1;

		while (l <= r) {
			m = (l + r) / 2;
			x = strncmp(buf, comms[m].name, len);
			if (x > 0)
				l = m + 1;
			else if (x < 0)
				r = m - 1;
			else {
				s = mbuf;
				for (l = m - 1; l >= 0 && strncmp(buf, comms[l].name, len) == 0; l--) ;
				for (m = ++l;
				     m <= r && strncmp(buf, comms[m].name, len) == 0 && (uintptr_t)(s - mbuf) < ARRAY_SIZE(mbuf); m++)
					s += snprintf(s, ARRAY_SIZE(mbuf) - (s - mbuf), " %s", comms[m].name);
				if (l < m - 1) {
					if (showmessage)
						Msg(0, "Possible commands:%s", mbuf);
				} else {
					s = mbuf;
					len = snprintf(mbuf, ARRAY_SIZE(mbuf), "%s \t", comms[l].name + len);
					if (len > 0 && len < ARRAY_SIZE(mbuf))
						LayProcess(&s, &len);
				}
				break;
			}
		}
		if (l > r && showmessage)
			Msg(0, "No commands matching '%*s'", (int)len, buf);
		return;
	}

	if (!len || buf[len])
		return;

	len = strlen(buf) + 1;
	if (len > (int)ARRAY_SIZE(mbuf))
		RcLine(buf, len);
	else {
		memmove(mbuf, buf, len);
		RcLine(mbuf, ARRAY_SIZE(mbuf));
	}
}

static void SelectFin(char *buf, size_t len, void *data)
{
	int n;

	(void)data; /* unused */

	if (!len || !display)
		return;
	if (len == 1 && *buf == '-') {
		SetForeWindow(NULL);
		Activate(0);
		return;
	}
	if ((n = WindowByNoN(buf)) < 0)
		return;
	SwitchWindow(GetWindowByNumber(n));
}

static void SelectLayoutFin(char *buf, size_t len, void *data)
{
	Layout *lay;

	(void)data; /* unused */

	if (!len || !display)
		return;
	if (len == 1 && *buf == '-') {
		LoadLayout(NULL);
		Activate(0);
		return;
	}
	lay = FindLayout(buf);
	if (!lay)
		Msg(0, "No such layout\n");
	else if (lay == D_layout)
		Msg(0, "This IS layout %d (%s).\n", lay->lay_number, lay->lay_title);
	else {
		LoadLayout(lay);
		Activate(0);
	}
}

static void InputSelect(void)
{
	Input("Switch to window: ", 20, INP_COOKED, SelectFin, NULL, 0);
}

static char setenv_var[31];

static void SetenvFin1(char *buf, size_t len, void *data)
{
	(void)data; /* unused */

	if (!len || !display)
		return;
	InputSetenv(buf);
}

static void SetenvFin2(char *buf, size_t len, void *data)
{
	(void)data; /* unused */

	if (!len || !display)
		return;
	setenv(setenv_var, buf, 1);
	MakeNewEnv();
}

static void InputSetenv(char *arg)
{
	static char setenv_buf[50 + ARRAY_SIZE(setenv_var)];	/* need to be static here, cannot be freed */

	if (arg) {
		strncpy(setenv_var, arg, ARRAY_SIZE(setenv_var) - 1);
		sprintf(setenv_buf, "Enter value for %s: ", setenv_var);
		Input(setenv_buf, 30, INP_COOKED, SetenvFin2, NULL, 0);
	} else
		Input("Setenv: Enter variable name: ", 30, INP_COOKED, SetenvFin1, NULL, 0);
}

/*
 * the following options are understood by this parser:
 * -f, -f0, -f1, -fy, -fa
 * -t title, -T terminal-type, -h height-of-scrollback,
 * -ln, -l0, -ly, -l1, -l
 * -a, -M, -L
 */
void DoScreen(char *fn, char **av)
{
	struct NewWindow nwin;
	int num;
	char buf[20];

	nwin = nwin_undef;
	while (av && *av && av[0][0] == '-') {
		if (av[0][1] == '-') {
			av++;
			break;
		}
		switch (av[0][1]) {
		case 'f':
			switch (av[0][2]) {
			case 'n':
			case '0':
				nwin.flowflag = FLOW_OFF;
				break;
			case 'y':
			case '1':
			case '\0':
				nwin.flowflag = FLOW_ON;
				break;
			case 'a':
				nwin.flowflag = FLOW_AUTOFLAG;
				break;
			default:
				break;
			}
			break;
		case 't':	/* no more -k */
			if (av[0][2])
				nwin.aka = &av[0][2];
			else if (*++av)
				nwin.aka = *av;
			else
				--av;
			break;
		case 'T':
			if (av[0][2])
				nwin.term = &av[0][2];
			else if (*++av)
				nwin.term = *av;
			else
				--av;
			break;
		case 'h':
			if (av[0][2])
				nwin.histheight = atoi(av[0] + 2);
			else if (*++av)
				nwin.histheight = atoi(*av);
			else
				--av;
			break;
#if defined(ENABLE_UTMP)
		case 'l':
			switch (av[0][2]) {
			case 'n':
			case '0':
				nwin.lflag = 0;
				break;
			case 'y':
			case '1':
			case '\0':
				nwin.lflag = 1;
				break;
			case 'a':
				nwin.lflag = 3;
				break;
			default:
				break;
			}
			break;
#endif
		case 'a':
			nwin.aflag = true;
			break;
		case 'M':
			nwin.monitor = MON_ON;
			break;
		case 'L':
			nwin.Lflag = true;
			break;
		default:
			Msg(0, "%s: screen: invalid option -%c.", fn, av[0][1]);
			break;
		}
		++av;
	}
	if (av && *av && IsNumColon(*av, buf, ARRAY_SIZE(buf))) {
		if (*buf != '\0')
			nwin.aka = buf;
		num = atoi(*av);
		if (num < 0) {
			Msg(0, "%s: illegal screen number %d.", fn, num);
			num = 0;
		}
		nwin.StartAt = num;
		++av;
	}
	if (av && *av) {
		nwin.args = av;
		if (!nwin.aka)
			nwin.aka = Filename(*av);
	}
	MakeWindow(&nwin);
}

/*
 * CompileKeys must be called before Markroutine is first used.
 * to initialise the keys with defaults, call CompileKeys(NULL, mark_key_tab);
 *
 * s is an ascii string in a termcap-like syntax. It looks like
 *   "j=u:k=d:l=r:h=l: =.:" and so on...
 * this example rebinds the cursormovement to the keys u (up), d (down),
 * l (left), r (right). placing a mark will now be done with ".".
 */
int CompileKeys(char *s, int sl, unsigned char *array)
{
	int i;
	unsigned char key, value;

	if (sl == 0) {
		for (i = 0; i < 256; i++)
			array[i] = i;
		return 0;
	}
	while (sl) {
		key = *(unsigned char *)s++;
		if (*s != '=' || sl < 3)
			return -1;
		sl--;
		do {
			s++;
			sl -= 2;
			value = *(unsigned char *)s++;
			array[value] = key;
		}
		while (*s == '=' && sl >= 2);
		if (sl == 0)
			break;
		if (*s++ != ':')
			return -1;
		sl--;
	}
	return 0;
}

/*
 *  Asynchronous input functions
 */

static void copy_reg_fn(char *buf, size_t len, void *data)
{
	(void)data; /* unused */

	struct plop *pp = plop_tab + (int)(unsigned char)*buf;

	if (len) {
		memset(buf, 0, len);
		return;
	}
	if (pp->buf)
		free(pp->buf);
	pp->buf = NULL;
	pp->len = 0;
	if (D_user->u_plop.len) {
		if ((pp->buf = malloc(D_user->u_plop.len)) == NULL) {
			Msg(0, "%s", strnomem);
			return;
		}
		memmove(pp->buf, D_user->u_plop.buf, D_user->u_plop.len);
	}
	pp->len = D_user->u_plop.len;
	pp->enc = D_user->u_plop.enc;
	Msg(0, "Copied %zu characters into register %c", D_user->u_plop.len, *buf);
}

static void ins_reg_fn(char *buf, size_t len, void *data)
{
	(void)data; /* unused */

	struct plop *pp = plop_tab + (int)(unsigned char)*buf;

	if (len) {
		memset(buf, 0, len);
		return;
	}
	if (!fore)
		return;		/* Input() should not call us w/o fore, but you never know... */
	if (*buf == '.')
		Msg(0, "ins_reg_fn: Warning: pasting real register '.'!");
	if (pp->buf) {
		MakePaster(&fore->w_paster, pp->buf, pp->len, 0);
		return;
	}
	Msg(0, "Empty register.");
}

static void process_fn(char *buf, size_t len, void *data)
{
	struct plop *pp = plop_tab + (int)(unsigned char)*buf;

	(void)data; /* unused */

	if (len) {
		memset(buf, 0, len);
		return;
	}
	if (pp->buf) {
		ProcessInput(pp->buf, pp->len);
		return;
	}
	Msg(0, "Empty register.");
}

static void confirm_fn(char *buf, size_t len, void *data)
{
	struct action act;

	if (len || (*buf != 'y' && *buf != 'Y')) {
		memset(buf, 0, len);
		return;
	}
	act.nr = *(int *)data;
	act.args = noargs;
	act.argl = NULL;
	act.quiet = 0;
	DoAction(&act);
}

struct inputsu {
	struct acluser **up;
	char name[24];
	char pw1[130];		/* FreeBSD crypts to 128 bytes */
	char pw2[130];
};

static void suFin(char *buf, size_t len, void *data)
{
	struct inputsu *i = (struct inputsu *)data;
	char *p;
	size_t l;

	if (!*i->name) {
		p = i->name;
		l = ARRAY_SIZE(i->name) - 1;
	} else if (!*i->pw1) {
		strcpy(p = i->pw1, "\377");
		l = ARRAY_SIZE(i->pw1) - 1;
	} else {
		strcpy(p = i->pw2, "\377");
		l = ARRAY_SIZE(i->pw2) - 1;
	}
	if (buf && len)
		strncpy(p, buf, 1 + ((l < len) ? l : len));
	if (!*i->name)
		Input("Screen User: ", ARRAY_SIZE(i->name) - 1, INP_COOKED, suFin, (char *)i, 0);
	else if (!*i->pw1)
		Input("User's UNIX Password: ", ARRAY_SIZE(i->pw1) - 1, INP_COOKED | INP_NOECHO, suFin, (char *)i, 0);
	else if (!*i->pw2)
		Input("User's Screen Password: ", ARRAY_SIZE(i->pw2) - 1, INP_COOKED | INP_NOECHO, suFin, (char *)i, 0);
	else {
		if ((p = DoSu(i->up, i->name, i->pw2, i->pw1)))
			Msg(0, "%s", p);
		free((char *)i);
	}
}

static int InputSu(struct acluser **up, char *name)
{
	struct inputsu *i;

	if (!(i = (struct inputsu *)calloc(1, sizeof(struct inputsu))))
		return -1;

	i->up = up;
	if (name && *name)
		suFin(name, (int)strlen(name), (char *)i);	/* can also initialise stuff */
	else
		suFin(NULL, 0, (char *)i);
	return 0;
}

static int digraph_find(const char *buf)
{
	uint32_t i;
	for (i = 0; i < ARRAY_SIZE(digraphs) && digraphs[i].d[0]; i++)
		if ((digraphs[i].d[0] == (unsigned char)buf[0] && digraphs[i].d[1] == (unsigned char)buf[1]))
			break;
	return i;
}

static void digraph_fn(char *buf, size_t len, void *data)
{
	int ch, i, x;
	size_t l;

	(void)data; /* unused */

	ch = buf[len];
	if (ch) {
		buf[len + 1] = ch;	/* so we can restore it later */
		if (ch < ' ' || ch == '\177')
			return;
		if (len >= 1 && ((*buf == 'U' && buf[1] == '+') || (*buf == '0' && (buf[1] == 'x' || buf[1] == 'X')))) {
			if (len == 1)
				return;
			if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f') && (ch < 'A' || ch > 'F')) {
				buf[len] = '\034';	/* ^] is ignored by Input() */
				return;
			}
			if (len == (*buf == 'U' ? 5 : 3))
				buf[len] = '\n';
			return;
		}
		if (len && *buf == '0') {
			if (ch < '0' || ch > '7') {
				buf[len] = '\034';	/* ^] is ignored by Input() */
				return;
			}
			if (len == 3)
				buf[len] = '\n';
			return;
		}
		if (len == 1)
			buf[len] = '\n';
		return;
	}
	if (len < 1)
		return;
	if (buf[len + 1]) {
		buf[len] = buf[len + 1];	/* stored above */
		len++;
	}
	if (len < 2)
		return;
	if (!parse_input_int(buf, len, &x)) {
		i = digraph_find(buf);
		if ((x = digraphs[i].value) <= 0) {
			Msg(0, "Unknown digraph");
			return;
		}
	}
	l = 1;
	*buf = x;
	if (flayer->l_encoding == UTF8)
		l = ToUtf8(buf, x);	/* buf is big enough for all UTF-8 codes */
	while (l)
		LayProcess(&buf, &l);
}

int StuffKey(int i)
{
	struct action *act;
	int discard = 0;
	int keyno = i;

	if (i < KMAP_KEYS && D_ESCseen) {
		struct action *act = &D_ESCseen[i + 256];
		if (act->nr != RC_ILLEGAL) {
			D_ESCseen = NULL;
			WindowChanged(fore, WINESC_ESC_SEEN);
			DoAction(act);
			return 0;
		}
		discard = 1;
	}

	if (i >= T_CURSOR - T_CAPS && i < T_KEYPAD - T_CAPS && D_cursorkeys)
		i += T_OCAPS - T_CURSOR;
	else if (i >= T_KEYPAD - T_CAPS && i < T_OCAPS - T_CAPS && D_keypad)
		i += T_OCAPS - T_CURSOR;
	flayer = D_forecv->c_layer;
	fore = D_fore;
	act = NULL;
	if (flayer && flayer->l_mode == 1)
		act = i < KMAP_KEYS + KMAP_AKEYS ? &mmtab[i] : &kmap_exts[i - (KMAP_KEYS + KMAP_AKEYS)].mm;
	if ((!act || act->nr == RC_ILLEGAL) && !D_mapdefault)
		act = i < KMAP_KEYS + KMAP_AKEYS ? &umtab[i] : &kmap_exts[i - (KMAP_KEYS + KMAP_AKEYS)].um;
	if (!act || act->nr == RC_ILLEGAL)
		act = i < KMAP_KEYS + KMAP_AKEYS ? &dmtab[i] : &kmap_exts[i - (KMAP_KEYS + KMAP_AKEYS)].dm;

	if (discard && (!act || act->nr != RC_COMMAND)) {
		/* if the input was just a single byte we let it through */
		if (D_tcs[keyno + T_CAPS].str && strlen(D_tcs[keyno + T_CAPS].str) == 1)
			return -1;
		if (D_ESCseen) {
			D_ESCseen = NULL;
			WindowChanged(fore, WINESC_ESC_SEEN);
		}
		return 0;
	}
	D_mapdefault = 0;

	if (act == NULL || act->nr == RC_ILLEGAL)
		return -1;
	DoAction(act);
	return 0;
}

static int IsOnDisplay(Window *win)
{
	Canvas *cv;
	for (cv = D_cvlist; cv; cv = cv->c_next)
		if (Layer2Window(cv->c_layer) == win)
			return 1;
	return 0;
}

Window *FindNiceWindow(Window *win, char *presel)
{
	int i;

	if (presel) {
		i = WindowByNoN(presel);
		if (i >= 0)
			win = GetWindowByNumber(i);
	}
	if (!display)
		return win;
	if (win && AclCheckPermWin(D_user, ACL_READ, win))
		win = NULL;
	if (!win || (IsOnDisplay(win) && !presel)) {
		/* try to get another window */
		win = NULL;
		for (win = mru_window; win; win = win->w_prev_mru)
			if (!win->w_layer.l_cvlist && !AclCheckPermWin(D_user, ACL_WRITE, win))
				break;
		if (!win)
			for (win = mru_window; win; win = win->w_prev_mru)
				if (win->w_layer.l_cvlist && !IsOnDisplay(win)
				    && !AclCheckPermWin(D_user, ACL_WRITE, win))
					break;
		if (!win)
			for (win = mru_window; win; win = win->w_prev_mru)
				if (!win->w_layer.l_cvlist && !AclCheckPermWin(D_user, ACL_READ, win))
					break;
		if (!win)
			for (win = mru_window; win; win = win->w_prev_mru)
				if (win->w_layer.l_cvlist && !IsOnDisplay(win)
				    && !AclCheckPermWin(D_user, ACL_READ, win))
					break;
		if (!win)
			for (win = mru_window; win; win = win->w_prev_mru)
				if (!win->w_layer.l_cvlist)
					break;
		if (!win)
			for (win = mru_window; win; win = win->w_prev_mru)
				if (win->w_layer.l_cvlist && !IsOnDisplay(win))
					break;
	}
	if (win && AclCheckPermWin(D_user, ACL_READ, win))
		win = NULL;
	return win;
}

static int CalcSlicePercent(Canvas *cv, int percent)
{
	int w, wsum, up;
	if (!cv || !cv->c_slback)
		return percent;
	up = CalcSlicePercent(cv->c_slback->c_slback, percent);
	w = cv->c_slweight;
	for (cv = cv->c_slback->c_slperp, wsum = 0; cv; cv = cv->c_slnext)
		wsum += cv->c_slweight;
	if (wsum == 0)
		return 0;
	return (up * w) / wsum;
}

static int ChangeCanvasSize(Canvas *fcv, int abs, int diff, bool gflag, int percent)
/* Canvas *fcv;	 make this canvas bigger
   int abs;		 mode: 0:rel 1:abs 2:max
   int diff;		 change this much
   bool gflag;		 go up if neccessary
   int percent; */
{
	Canvas *cv;
	int done, have, m, dir;

	if (abs == 0 && diff == 0)
		return 0;
	if (abs == 2) {
		if (diff == 0)
			fcv->c_slweight = 0;
		else {
			for (cv = fcv->c_slback->c_slperp; cv; cv = cv->c_slnext)
				cv->c_slweight = 0;
			fcv->c_slweight = 1;
			cv = fcv->c_slback->c_slback;
			if (gflag && cv && cv->c_slback)
				ChangeCanvasSize(cv, abs, diff, gflag, percent);
		}
		return diff;
	}
	if (abs) {
		if (diff < 0)
			diff = 0;
		if (percent && diff > percent)
			diff = percent;
	}
	if (percent) {
		int wsum, up;
		for (cv = fcv->c_slback->c_slperp, wsum = 0; cv; cv = cv->c_slnext)
			wsum += cv->c_slweight;
		if (wsum) {
			up = gflag ? CalcSlicePercent(fcv->c_slback->c_slback, percent) : percent;
			if (wsum < 1000) {
				int scale = wsum < 10 ? 1000 : 100;
				for (cv = fcv->c_slback->c_slperp; cv; cv = cv->c_slnext)
					cv->c_slweight *= scale;
				wsum *= scale;
			}
			for (cv = fcv->c_slback->c_slperp; cv; cv = cv->c_slnext) {
				if (cv->c_slweight) {
					cv->c_slweight = (cv->c_slweight * up) / percent;
					if (cv->c_slweight == 0)
						cv->c_slweight = 1;
				}
			}
			diff = (diff * wsum) / percent;
			percent = wsum;
		}
	} else {
		if (abs
		    && diff == (fcv->c_slorient == SLICE_VERT ? fcv->c_ye - fcv->c_ys + 2 : fcv->c_xe - fcv->c_xs + 2))
			return 0;
		/* fix weights to real size (can't be helped, sorry) */
		for (cv = fcv->c_slback->c_slperp; cv; cv = cv->c_slnext) {
			cv->c_slweight =
			    cv->c_slorient == SLICE_VERT ? cv->c_ye - cv->c_ys + 2 : cv->c_xe - cv->c_xs + 2;
		}
	}
	if (abs)
		diff = diff - fcv->c_slweight;
	if (diff == 0)
		return 0;
	if (diff < 0) {
		cv = fcv->c_slnext ? fcv->c_slnext : fcv->c_slprev;
		fcv->c_slweight += diff;
		cv->c_slweight -= diff;
		return diff;
	}
	done = 0;
	dir = 1;
	for (cv = fcv->c_slnext; diff > 0; cv = dir > 0 ? cv->c_slnext : cv->c_slprev) {
		if (!cv) {
			if (dir == -1)
				break;
			dir = -1;
			cv = fcv;
			continue;
		}
		if (percent)
			m = 1;
		else
			m = cv->c_slperp ? CountCanvasPerp(cv) * 2 : 2;
		if (cv->c_slweight > m) {
			have = cv->c_slweight - m;
			if (have > diff)
				have = diff;
			cv->c_slweight -= have;
			done += have;
			diff -= have;
		}
	}
	if (diff && gflag) {
		/* need more room! */
		cv = fcv->c_slback->c_slback;
		if (cv && cv->c_slback)
			done += ChangeCanvasSize(fcv->c_slback->c_slback, 0, diff, gflag, percent);
	}
	fcv->c_slweight += done;
	return done;
}

static void ResizeRegions(char *arg, int flags)
{
	Canvas *cv;
	int diff, l;
	bool gflag = 0;
	int abs = 0, percent = 0;
	int orient = 0;

	if (!*arg)
		return;
	if (D_forecv->c_slorient == SLICE_UNKN) {
		Msg(0, "resize: need more than one region");
		return;
	}
	gflag = flags & RESIZE_FLAG_L ? 0 : 1;
	orient |= flags & RESIZE_FLAG_H ? SLICE_HORI : 0;
	orient |= flags & RESIZE_FLAG_V ? SLICE_VERT : 0;
	if (orient == 0)
		orient = D_forecv->c_slorient;
	l = strlen(arg);
	if (*arg == '=') {
		/* make all regions the same height */
		Canvas *cv = gflag ? &D_canvas : D_forecv->c_slback;
		if (cv->c_slperp->c_slorient & orient)
			EqualizeCanvas(cv->c_slperp, gflag);
		/* can't use cv->c_slorient directly as it can be D_canvas */
		if ((cv->c_slperp->c_slorient ^ (SLICE_HORI ^ SLICE_VERT)) & orient) {
			if (cv->c_slback) {
				cv = cv->c_slback;
				EqualizeCanvas(cv->c_slperp, gflag);
			} else
				EqualizeCanvas(cv, gflag);
		}
		ResizeCanvas(cv);
		RecreateCanvasChain();
		RethinkDisplayViewports();
		ResizeLayersToCanvases();
		return;
	}
	if (!strcmp(arg, "min") || !strcmp(arg, "0")) {
		abs = 2;
		diff = 0;
	} else if (!strcmp(arg, "max") || !strcmp(arg, "_")) {
		abs = 2;
		diff = 1;
	} else {
		if (l > 0 && arg[l - 1] == '%')
			percent = 1000;
		if (*arg == '+')
			diff = atoi(arg + 1);
		else if (*arg == '-')
			diff = -atoi(arg + 1);
		else {
			diff = atoi(arg);	/* +1 because of caption line */
			if (diff < 0)
				diff = 0;
			abs = diff == 0 ? 2 : 1;
		}
	}
	if (!abs && !diff)
		return;
	if (percent)
		diff = diff * percent / 100;
	cv = D_forecv;
	if (cv->c_slorient & orient)
		ChangeCanvasSize(cv, abs, diff, gflag, percent);
	if (cv->c_slback->c_slorient & orient)
		ChangeCanvasSize(cv->c_slback, abs, diff, gflag, percent);

	ResizeCanvas(&D_canvas);
	RecreateCanvasChain();
	RethinkDisplayViewports();
	ResizeLayersToCanvases();
	return;
}

static void ResizeFin(char *buf, size_t len, void *data)
{
	int ch;
	int flags = *(int *)data;
	ch = ((unsigned char *)buf)[len];
	if (ch == 0) {
		ResizeRegions(buf, flags);
		return;
	}
	if (ch == 'h')
		flags ^= RESIZE_FLAG_H;
	else if (ch == 'v')
		flags ^= RESIZE_FLAG_V;
	else if (ch == 'b')
		flags |= RESIZE_FLAG_H | RESIZE_FLAG_V;
	else if (ch == 'p')
		flags ^= D_forecv->c_slorient == SLICE_VERT ? RESIZE_FLAG_H : RESIZE_FLAG_V;
	else if (ch == 'l')
		flags ^= RESIZE_FLAG_L;
	else
		return;
	inp_setprompt(resizeprompts[flags], NULL);
	*(int *)data = flags;
	buf[len] = '\034';
}

void SetForeCanvas(Display *d, Canvas *cv)
{
	Display *odisplay = display;
	if (d->d_forecv == cv)
		return;

	display = d;
	D_forecv = cv;
	if ((focusminwidth && (focusminwidth < 0 || D_forecv->c_xe - D_forecv->c_xs + 1 < focusminwidth)) ||
	    (focusminheight && (focusminheight < 0 || D_forecv->c_ye - D_forecv->c_ys + 1 < focusminheight))) {
		ResizeCanvas(&D_canvas);
		RecreateCanvasChain();
		RethinkDisplayViewports();
		ResizeLayersToCanvases();	/* redisplays */
	}
	fore = D_fore = Layer2Window(D_forecv->c_layer);
	if (D_other == fore)
		D_other = NULL;
	flayer = D_forecv->c_layer;
	if (D_xtermosc[2] || D_xtermosc[3]) {
		Activate(-1);
	} else {
		RefreshHStatus();
		RefreshXtermOSC();
		flayer = D_forecv->c_layer;
		CV_CALL(D_forecv, LayRestore();
			LaySetCursor());
		WindowChanged(NULL, WINESC_FOCUS);
	}

	display = odisplay;
}

void RefreshXtermOSC(void)
{
	Window *p = Layer2Window(D_forecv->c_layer);

	for (int i = 4; i >= 0; i--)
		SetXtermOSC(i, p ? p->w_xtermosc[i] : NULL, "\a");
}

/*
 *  ParseAttrColor - parses attributes and color
 *  	str - string containing attributes and/or colors
 *  	        d - dim
 *  	        u - underscore
 *  	        b - bold
 *  	        r - reverse
 *  	        s - standout
 *  	        l - blinking
 *  	        0-255;0-255 - foreground;background
 *  	        xABCDEF;xABCDEF - truecolor foreground;background
 *  	        #ABCDEF;#ABCDEF - truecolor foreground;background
 *  	        xABC;xABC - truecolor foreground;background
 *  	        #ABC;#ABC - truecolor foreground;background
 *  	msgok - can we be verbose if something is wrong
 *
 *  returns value representing encoded value
 */
uint64_t ParseAttrColor(char *str, int msgok)
{
	uint64_t r;

	uint32_t attr = 0;
	uint32_t bg = 0, fg = 0;
	uint8_t bm = 0, fm = 0;

	uint32_t *cl;
	uint8_t *cm;
	cl = &fg;
	cm = &fm;

	while (*str) {
		if (*cm < 4) {
			switch (*str) {
			case 'd':
				attr |= A_DI;
				break;
			case 'u':
				attr |= A_US;
				break;
			case 'b':
				attr |= A_BD;
				break;
			case 'r':
				attr |= A_RV;
				break;
			case 'i':
				attr |= A_IT;
				break;
			case 'l':
				attr |= A_BL;
				break;
			case '-':
				*cm = 0;
				break;
			case 'x':
			case '#':
				*cm = 4;
				str++;
				continue;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (!*cm) *cm = 1;
				*cl = *cl * 10 + (*str - '0');
				break;
			case ';':
				cl = &bg;
				cm = &bm;
				break;
			case ' ':
				break;
			default:
				if (msgok)
					Msg(0, "junk after description: '%c'\n", *str);
				break;
			}
		}
		if (*cm == 4) {
			switch (*str) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				*cl = ((*cl & 0x0F000000)+0x01000000) + ((*cl << 4) | (*str - '0'));
				break;
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
				*cl = ((*cl & 0x0F000000)+0x01000000) + ((*cl << 4) | (*str - ('a' - 10)));
				break;
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
				*cl = ((*cl & 0x0F000000)+0x01000000) + ((*cl << 4) | (*str - ('A' - 10)));
				break;
			case ';':
				cl = &bg;
				cm = &bm;
				break;
			default:
				if (msgok)
					Msg(0, "junk after description: '%c'\n", *str);
				break;
			}
		}
		str++;
	}

	/* fm/bm auto switches from 1 to 2 when color >= 16
	 * fm/bm mode 4 uses 4th byte as a digit counter. If it's 3
	 *       digits, translate 0x00000ABC to 0x00AABBCC
	 * fm/bm mode 4 gets & 0x0FFFFFF at the end, so we don't care
	 *       about upper boundaries.
	 */
	if (fm == 4) {
		if (fg <= 0x03000000)
		{
			fg &= 0x0FFF;
			fg = ((fg & 0x0F00) << 8) | ((fg & 0x0F0) << 4) | (fg & 0x0F);
			fg = fg | (fg << 4);
		}
	} else {
		if (fg > 7) {
			fm = 2;
		}
		if (fg > 255) {
			fg = fm = 0;
		}
	}
	if (bm == 4) {
		if (bg <= 0x03000000)
		{
			bg &= 0x0FFF;
			bg = ((bg & 0x0F00) << 8) | ((bg & 0x0F0) << 4) | (bg & 0x0F);
			bg = bg | (bg << 4);
		}
	} else {
		if (bg > 7) {
			bm = 2;
		}
		if (bg > 255) {
			bg = bm = 0;
		}
	}

	r = (((uint64_t)attr & 0x0FF) << 56);

	r |= (((uint64_t)bg & 0x0FFFFFF) << 24);
	r |= ((uint64_t)fg & 0x0FFFFFF);
	r |= ((uint64_t)fm << 48);
	r |= ((uint64_t)bm << 52);

	return r;
}

/*
 *   ApplyAttrColor - decodes color attributes and sets them in structure
 *	i - encoded attributes and color
 *	00 00 00 00 00 00 00 00
 *	xx 00 00 00 00 00 00 00 - attr
 *	00 x0 00 00 00 00 00 00 - what kind of background
 *	00 0x 00 00 00 00 00 00 - what kind of foreground
 *	                          0 - default, 1 - base 16; 2 - 256, 4 - truecolor
 *	00 00 xx xx xx 00 00 00 - background
 *	00 00 00 00 00 xx xx xx - foreground
 *	mc -structure to modify
 */
void ApplyAttrColor(uint64_t i, struct mchar *mc)
{
	uint32_t a, b, f;
	unsigned char h;
	a = (0xFF00000000000000 & i) >> 56;
	b = (0x0000FFFFFF000000 & i) >> 24;
	f = (0x0000000000FFFFFF & i);

	h = (0x00FF000000000000 & i) >> 48;

	if (h & 0x40) b |= 0x04000000;
	if (h & 0x20) b |= 0x02000000;
	if (h & 0x10) b |= 0x01000000;
	if (h & 0x04) f |= 0x04000000;
	if (h & 0x02) f |= 0x02000000;
	if (h & 0x01) f |= 0x01000000;
	
	mc->attr	= a;
	mc->colorbg	= b;
	mc->colorfg	= f;
}
