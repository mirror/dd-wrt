/* Copyright (c) 2010
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

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#if !defined(sun) && !defined(B43) && !defined(ISC) && !defined(pyr) && !defined(_CX_UX)
# include <time.h>
#endif
#include <sys/time.h>
#ifndef sun
#include <sys/ioctl.h>
#endif


/* for solaris 2.1, Unixware (SVR4.2) and possibly others: */
#ifdef HAVE_STROPTS_H
# include <sys/stropts.h>
#endif

#include "screen.h"
#include "extern.h"
#include "logfile.h"
#include "layout.h"
#include "viewport.h"
#include "list_generic.h"

extern struct comm comms[];
extern char *rc_name;
extern char *RcFileName, *home;
extern char *BellString, *ActivityString, *ShellProg, *ShellArgs[];
extern char *hstatusstring, *captionstring, *timestring;
extern char *wliststr, *wlisttit;
extern int captionalways;
extern int queryflag;
extern char *hardcopydir, *screenlogfile, *logtstamp_string;
extern int log_flush, logtstamp_on, logtstamp_after;
extern char *VisualBellString;
extern int VBellWait, MsgWait, MsgMinWait, SilenceWait;
extern char SockPath[], *SockName;
extern int TtyMode, auto_detach, use_altscreen;
extern int iflag, maxwin;
extern int focusminwidth, focusminheight;
extern int use_hardstatus, visual_bell;
#ifdef COLOR
extern int attr2color[][4];
extern int nattr2color;
#endif
extern int hardstatusemu;
extern char *printcmd;
extern int default_startup;
extern int defobuflimit;
extern int defnonblock;
extern int defmousetrack;
extern int ZombieKey_destroy;
extern int ZombieKey_resurrect;
extern int ZombieKey_onerror;
#ifdef AUTO_NUKE
extern int defautonuke;
#endif
extern int separate_sids;
extern struct NewWindow nwin_default, nwin_undef;
#ifdef COPY_PASTE
extern int join_with_cr;
extern int compacthist;
extern int search_ic;
# ifdef FONT
extern int pastefont;
# endif
extern unsigned char mark_key_tab[];
extern char *BufferFile;
#endif
#ifdef POW_DETACH
extern char *BufferFile, *PowDetachString;
#endif
#ifdef MULTIUSER
extern struct acluser *EffectiveAclUser;	/* acl.c */
#endif
extern struct term term[];      /* terminal capabilities */
#ifdef MAPKEYS
extern char *kmapdef[];
extern char *kmapadef[];
extern char *kmapmdef[];
#endif
extern struct mchar mchar_so, mchar_null;
extern int renditions[];
extern int VerboseCreate;
#ifdef UTF8
extern char *screenencodings;
#endif
#ifdef DW_CHARS
extern int cjkwidth;
#endif

static int  CheckArgNum __P((int, char **));
static void ClearAction __P((struct action *));
static void SaveAction __P((struct action *, int, char **, int *));
static int  NextWindow __P((void));
static int  PreviousWindow __P((void));
static int  MoreWindows __P((void));
static void CollapseWindowlist __P((void));
static void LogToggle __P((int));
static void ShowInfo __P((void));
static void ShowDInfo __P((void));
static struct win *WindowByName __P((char *));
static int  WindowByNumber __P((char *));
static int  ParseOnOff __P((struct action *, int *));
static int  ParseWinNum __P((struct action *, int *));
static int  ParseBase __P((struct action *, char *, int *, int, char *));
static int  ParseNum1000 __P((struct action *, int *));
static char **SaveArgs __P((char **));
static int  IsNum __P((char *, int));
static void Colonfin __P((char *, int, char *));
static void InputSelect __P((void));
static void InputSetenv __P((char *));
static void InputAKA __P((void));
#ifdef MULTIUSER
static int  InputSu __P((struct win *, struct acluser **, char *));
static void su_fin __P((char *, int, char *));
#endif
static void AKAfin __P((char *, int, char *));
#ifdef COPY_PASTE
static void copy_reg_fn __P((char *, int, char *));
static void ins_reg_fn __P((char *, int, char *));
#endif
static void process_fn __P((char *, int, char *));
#ifdef PASSWORD
static void pass1 __P((char *, int, char *));
static void pass2 __P((char *, int, char *));
#endif
#ifdef POW_DETACH
static void pow_detach_fn __P((char *, int, char *));
#endif
static void digraph_fn __P((char *, int, char *));
static int  digraph_find __P((const char *buf));
static void confirm_fn __P((char *, int, char *));
static int  IsOnDisplay __P((struct win *));
static void ResizeRegions __P((char *, int));
static void ResizeFin __P((char *, int, char *));
static struct action *FindKtab __P((char *, int));
static void SelectFin __P((char *, int, char *));
static void SelectLayoutFin __P((char *, int, char *));
static void ShowWindowsX __P((char *));


extern struct layer *flayer;
extern struct display *display, *displays;
extern struct win *fore, *console_window, *windows;
extern struct acluser *users;
extern struct layout *layouts, *layout_attach, layout_last_marker;
extern struct layout *laytab[];

extern char screenterm[], HostName[], version[];
extern struct NewWindow nwin_undef, nwin_default;
extern struct LayFuncs WinLf, MarkLf;

extern const int Z0width, Z1width;
extern int real_uid, real_gid;

#ifdef NETHACK
extern int nethackflag;
#endif


extern struct win **wtab;

#ifdef MULTIUSER
extern char *multi;
extern int maxusercount;
#endif
char NullStr[] = "";

struct plop plop_tab[MAX_PLOP_DEFS];

#ifndef PTYMODE
# define PTYMODE 0622
#endif

int TtyMode = PTYMODE;
int hardcopy_append = 0;
int all_norefresh = 0;
#ifdef ZMODEM
int zmodem_mode = 0;
char *zmodem_sendcmd;
char *zmodem_recvcmd;
static char *zmodes[4] = {"off", "auto", "catch", "pass"};
#endif

int idletimo;
struct action idleaction;
#ifdef BLANKER_PRG
char **blankerprg;
#endif

struct action ktab[256 + KMAP_KEYS];	/* command key translation table */
struct kclass {
  struct kclass *next;
  char *name;
  struct action ktab[256 + KMAP_KEYS];
};
struct kclass *kclasses;

#ifdef MAPKEYS
struct action umtab[KMAP_KEYS+KMAP_AKEYS];
struct action dmtab[KMAP_KEYS+KMAP_AKEYS];
struct action mmtab[KMAP_KEYS+KMAP_AKEYS];
struct kmap_ext *kmap_exts;
int kmap_extn;
static int maptimeout = 300;
#endif

#ifndef MAX_DIGRAPH
#define MAX_DIGRAPH 512
#endif

struct digraph
{
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
  "resize -h # lines: ",
  "resize -v # lines: ",
  "resize -b # lines: ",
  "resize -l # lines: ",
  "resize -l -h # lines: ",
  "resize -l -v # lines: ",
  "resize -l -b # lines: ",
};


static int
parse_input_int(buf, len, val)
const char *buf;
int len;
int *val;
{
  int x = 0, i;
  if (len >= 1 && ((*buf == 'U' && buf[1] == '+') || (*buf == '0' && (buf[1] == 'x' || buf[1] == 'X'))))
    {
      x = 0;
      for (i = 2; i < len; i++)
	{
	  if (buf[i] >= '0' && buf[i] <= '9')
	    x = x * 16 | (buf[i] - '0');
	  else if (buf[i] >= 'a' && buf[i] <= 'f')
	    x = x * 16 | (buf[i] - ('a' - 10));
	  else if (buf[i] >= 'A' && buf[i] <= 'F')
	    x = x * 16 | (buf[i] - ('A' - 10));
	  else
	    return 0;
	}
    }
  else if (buf[0] == '0')
    {
      x = 0;
      for (i = 1; i < len; i++)
	{
	  if (buf[i] < '0' || buf[i] > '7')
	    return 0;
	  x = x * 8 | (buf[i] - '0');
	}
    }
  else
    return 0;
  *val = x;
  return 1;
}

char *noargs[1];

int enter_window_name_mode = 0;

void
InitKeytab()
{
  register unsigned int i;
#ifdef MAPKEYS
  char *argarr[2];
#endif

  for (i = 0; i < sizeof(ktab)/sizeof(*ktab); i++)
    {
      ktab[i].nr = RC_ILLEGAL;
      ktab[i].args = noargs;
      ktab[i].argl = 0;
    }
#ifdef MAPKEYS
  for (i = 0; i < KMAP_KEYS+KMAP_AKEYS; i++)
    {
      umtab[i].nr = RC_ILLEGAL;
      umtab[i].args = noargs;
      umtab[i].argl = 0;
      dmtab[i].nr = RC_ILLEGAL;
      dmtab[i].args = noargs;
      dmtab[i].argl = 0;
      mmtab[i].nr = RC_ILLEGAL;
      mmtab[i].args = noargs;
      mmtab[i].argl = 0;
    }
  argarr[1] = 0;
  for (i = 0; i < NKMAPDEF; i++)
    {
      if (i + KMAPDEFSTART < T_CAPS)
	continue;
      if (i + KMAPDEFSTART >= T_CAPS + KMAP_KEYS)
	continue;
      if (kmapdef[i] == 0)
	continue;
      argarr[0] = kmapdef[i];
      SaveAction(dmtab + i + (KMAPDEFSTART - T_CAPS), RC_STUFF, argarr, 0);
    }
  for (i = 0; i < NKMAPADEF; i++)
    {
      if (i + KMAPADEFSTART < T_CURSOR)
	continue;
      if (i + KMAPADEFSTART >= T_CURSOR + KMAP_AKEYS)
	continue;
      if (kmapadef[i] == 0)
	continue;
      argarr[0] = kmapadef[i];
      SaveAction(dmtab + i + (KMAPADEFSTART - T_CURSOR + KMAP_KEYS), RC_STUFF, argarr, 0);
    }
  for (i = 0; i < NKMAPMDEF; i++)
    {
      if (i + KMAPMDEFSTART < T_CAPS)
	continue;
      if (i + KMAPMDEFSTART >= T_CAPS + KMAP_KEYS)
	continue;
      if (kmapmdef[i] == 0)
	continue;
      argarr[0] = kmapmdef[i];
      argarr[1] = 0;
      SaveAction(mmtab + i + (KMAPMDEFSTART - T_CAPS), RC_STUFF, argarr, 0);
    }
#endif

  ktab['h'].nr = RC_HARDCOPY;
#ifdef BSDJOBS
  ktab['z'].nr = ktab[Ctrl('z')].nr = RC_SUSPEND;
#endif
  ktab['c'].nr = ktab[Ctrl('c')].nr = RC_SCREEN;
  ktab[' '].nr = ktab[Ctrl(' ')].nr =
    ktab['n'].nr = ktab[Ctrl('n')].nr = RC_NEXT;
  ktab['N'].nr = RC_NUMBER;
  ktab[Ctrl('h')].nr = ktab[0177].nr = ktab['p'].nr = ktab[Ctrl('p')].nr = RC_PREV;
  ktab['k'].nr = ktab[Ctrl('k')].nr = RC_KILL;
  ktab['l'].nr = ktab[Ctrl('l')].nr = RC_REDISPLAY;
  ktab['w'].nr = ktab[Ctrl('w')].nr = RC_WINDOWS;
  ktab['v'].nr = RC_VERSION;
  ktab[Ctrl('v')].nr = RC_DIGRAPH;
  ktab['q'].nr = ktab[Ctrl('q')].nr = RC_XON;
  ktab['s'].nr = ktab[Ctrl('s')].nr = RC_XOFF;
  ktab['t'].nr = ktab[Ctrl('t')].nr = RC_TIME;
  ktab['i'].nr = ktab[Ctrl('i')].nr = RC_INFO;
  ktab['m'].nr = ktab[Ctrl('m')].nr = RC_LASTMSG;
  ktab['A'].nr = RC_TITLE;
#if defined(UTMPOK) && defined(LOGOUTOK)
  ktab['L'].nr = RC_LOGIN;
#endif
  ktab[','].nr = RC_LICENSE;
  ktab['W'].nr = RC_WIDTH;
  ktab['.'].nr = RC_DUMPTERMCAP;
  ktab[Ctrl('\\')].nr = RC_QUIT;
#ifdef DETACH
  ktab['d'].nr = ktab[Ctrl('d')].nr = RC_DETACH;
# ifdef POW_DETACH
  ktab['D'].nr = RC_POW_DETACH;
# endif
#endif
  ktab['r'].nr = ktab[Ctrl('r')].nr = RC_WRAP;
  ktab['f'].nr = ktab[Ctrl('f')].nr = RC_FLOW;
  ktab['C'].nr = RC_CLEAR;
  ktab['Z'].nr = RC_RESET;
  ktab['H'].nr = RC_LOG;
  ktab['M'].nr = RC_MONITOR;
  ktab['?'].nr = RC_HELP;
#ifdef MULTI
  ktab['*'].nr = RC_DISPLAYS;
#endif
  {
    char *args[2];
    args[0] = "-";
    args[1] = NULL;
    SaveAction(ktab + '-', RC_SELECT, args, 0);
  }
  for (i = 0; i < ((maxwin && maxwin < 10) ? maxwin : 10); i++)
    {
      char *args[2], arg1[10];
      args[0] = arg1;
      args[1] = 0;
      sprintf(arg1, "%d", i);
      SaveAction(ktab + '0' + i, RC_SELECT, args, 0);
    }
  ktab['\''].nr = RC_SELECT; /* calling a window by name */
  {
    char *args[2];
    args[0] = "-b";
    args[1] = 0;
    SaveAction(ktab + '"', RC_WINDOWLIST, args, 0);
  }
  ktab[Ctrl('G')].nr = RC_VBELL;
  ktab[':'].nr = RC_COLON;
#ifdef COPY_PASTE
  ktab['['].nr = ktab[Ctrl('[')].nr = RC_COPY;
  {
    char *args[2];
    args[0] = ".";
    args[1] = 0;
    SaveAction(ktab + ']', RC_PASTE, args, 0);
    SaveAction(ktab + Ctrl(']'), RC_PASTE, args, 0);
  }
  ktab['{'].nr = RC_HISTORY;
  ktab['}'].nr = RC_HISTORY;
  ktab['>'].nr = RC_WRITEBUF;
  ktab['<'].nr = RC_READBUF;
  ktab['='].nr = RC_REMOVEBUF;
#endif
#ifdef POW_DETACH
  ktab['D'].nr = RC_POW_DETACH;
#endif
#ifdef LOCK
  ktab['x'].nr = ktab[Ctrl('x')].nr = RC_LOCKSCREEN;
#endif
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
    args[1] = 0;
    SaveAction(ktab + T_BACKTAB - T_CAPS + 256, RC_FOCUS, args, 0);
  }
  {
    char *args[2];
    args[0] = "-v";
    args[1] = 0;
    SaveAction(ktab + '|', RC_SPLIT, args, 0);
  }
  /* These come last; they may want overwrite others: */
  if (DefaultEsc >= 0)
    {
      ClearAction(&ktab[DefaultEsc]);
      ktab[DefaultEsc].nr = RC_OTHER;
    }
  if (DefaultMetaEsc >= 0)
    {
      ClearAction(&ktab[DefaultMetaEsc]);
      ktab[DefaultMetaEsc].nr = RC_META;
    }

  idleaction.nr = RC_BLANKER;
  idleaction.args = noargs;
  idleaction.argl = 0;
}

static struct action *
FindKtab(class, create)
char *class;
int create;
{
  struct kclass *kp, **kpp;
  int i;

  if (class == 0)
    return ktab;
  for (kpp = &kclasses; (kp = *kpp) != 0; kpp = &kp->next)
    if (!strcmp(kp->name, class))
      break;
  if (kp == 0)
    {
      if (!create)
	return 0;
      if (strlen(class) > 80)
	{
	  Msg(0, "Command class name too long.");
	  return 0;
	}
      kp = malloc(sizeof(*kp));
      if (kp == 0)
	{
	  Msg(0, "%s", strnomem);
	  return 0;
	}
      kp->name = SaveStr(class);
      for (i = 0; i < (int)(sizeof(kp->ktab)/sizeof(*kp->ktab)); i++)
	{
	  kp->ktab[i].nr = RC_ILLEGAL;
	  kp->ktab[i].args = noargs;
	  kp->ktab[i].argl = 0;
	  kp->ktab[i].quiet = 0;
	}
      kp->next = 0;
      *kpp = kp;
    }
  return kp->ktab;
}

static void
ClearAction(act)
struct action *act;
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
  act->argl = 0;
}

/*
 * ProcessInput: process input from display and feed it into
 * the layer on canvas D_forecv.
 */

#ifdef MAPKEYS

/*
 *  This ProcessInput just does the keybindings and passes
 *  everything else on to ProcessInput2.
 */

void
ProcessInput(ibuf, ilen)
char *ibuf;
int ilen;
{
  int ch, slen;
  unsigned char *s, *q;
  int i, l;
  char *p;

  debug1("ProcessInput: %d bytes\n", ilen);
  if (display == 0 || ilen == 0)
    return;
  if (D_seql)
    evdeq(&D_mapev);
  slen = ilen;
  s = (unsigned char *)ibuf;
  while (ilen-- > 0)
    {
      ch = *s++;
      if (D_dontmap || !D_nseqs)
	{
          D_dontmap = 0;
	  continue;
	}
      for (;;)
	{
	  debug3("cmp %c %c[%ld]\n", ch, *D_seqp, (long)(D_seqp - D_kmaps));
	  if (*D_seqp != ch)
	    {
	      l = D_seqp[D_seqp[-D_seql-1] + 1];
	      if (l)
		{
		  D_seqp += l * 2 + 4;
		  debug1("miss %ld\n", (long)(D_seqp - D_kmaps));
		  continue;
		}
	      debug("complete miss\n");
	      D_mapdefault = 0;
	      l = D_seql;
	      p = (char *)D_seqp - l;
	      D_seql = 0;
	      D_seqp = D_kmaps + 3;
	      if (l == 0)
		break;
	      if ((q = D_seqh) != 0)
		{
		  D_seqh = 0;
		  i = q[0] << 8 | q[1];
		  i &= ~KMAP_NOTIMEOUT;
		  debug1("Mapping former hit #%d - ", i);
		  debug2("%d(%s) - ", q[2], q + 3);
		  if (StuffKey(i))
		    ProcessInput2((char *)q + 3, q[2]);
		  if (display == 0)
		    return;
		  l -= q[2];
		  p += q[2];
		}
	      else
	        D_dontmap = 1;
	      debug1("flush old %d\n", l);
	      ProcessInput(p, l);
	      if (display == 0)
		return;
	      evdeq(&D_mapev);
	      continue;
	    }
	  if (D_seql++ == 0)
	    {
	      /* Finish old stuff */
	      slen -= ilen + 1;
	      debug1("finish old %d\n", slen);
	      if (slen)
	        ProcessInput2(ibuf, slen);
	      if (display == 0)
		return;
	      D_seqh = 0;
	    }
	  ibuf = (char *)s;
	  slen = ilen;
	  D_seqp++;
	  l = D_seql;
	  debug2("length am %d, want %d\n", l, D_seqp[-l - 1]);
	  if (l == D_seqp[-l - 1])
	    {
	      if (D_seqp[l] != l)
		{
		  q = D_seqp + 1 + l;
		  if (D_kmaps + D_nseqs > q && q[2] > l && !bcmp(D_seqp - l, q + 3, l))
		    {
		      debug1("have another mapping (%s), delay execution\n", q + 3);
		      D_seqh = D_seqp - 3 - l;
		      D_seqp = q + 3 + l;
		      break;
		    }
		}
	      i = D_seqp[-l - 3] << 8 | D_seqp[-l - 2];
	      i &= ~KMAP_NOTIMEOUT;
	      debug1("Mapping #%d - ", i);
	      p = (char *)D_seqp - l;
	      debug2("%d(%s) - ", l, p);
	      D_seql = 0;
	      D_seqp = D_kmaps + 3;
	      D_seqh = 0;
	      if (StuffKey(i))
		ProcessInput2(p, l);
	      if (display == 0)
		return;
	    }
	  break;
	}
    }
  if (D_seql)
    {
      debug("am in sequence -> check for timeout\n");
      l = D_seql;
      for (s = D_seqp; ; s += i * 2 + 4)
	{
	  if (s[-l-3] & KMAP_NOTIMEOUT >> 8)
	    break;
	  if ((i = s[s[-l-1] + 1]) == 0)
	    {
	      SetTimeout(&D_mapev, maptimeout);
	      evenq(&D_mapev);
	      break;
	    }
	}
    }
  ProcessInput2(ibuf, slen);
}

#else
# define ProcessInput2 ProcessInput
#endif


/*
 *  Here only the screen escape commands are handled.
 */

void
ProcessInput2(ibuf, ilen)
char *ibuf;
int ilen;
{
  char *s;
  int ch, slen;
  struct action *ktabp;

  debug1("ProcessInput2: %d bytes\n", ilen);
  while (ilen && display)
    {
      debug1(" - ilen now %d bytes\n", ilen);
      flayer = D_forecv->c_layer;
      fore = D_fore;
      slen = ilen;
      s = ibuf;
      if (!D_ESCseen)
	{
	  while (ilen > 0)
	    {
	      if ((unsigned char)*s++ == D_user->u_Esc)
		break;
	      ilen--;
	    }
	  slen -= ilen;
	  if (slen)
	    DoProcess(fore, &ibuf, &slen, 0);
	  if (--ilen == 0)
	    {
	      D_ESCseen = ktab;
	      WindowChanged(fore, 'E');
	    }
	}
      if (ilen <= 0)
        return;
      ktabp = D_ESCseen ? D_ESCseen : ktab;
      if (D_ESCseen)
        {
          D_ESCseen = 0;
          WindowChanged(fore, 'E');
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
        DoAction(&ktabp[ch], ch);
      ibuf = (char *)(s + 1);
      ilen--;
    }
}

void
DoProcess(p, bufp, lenp, pa)
struct win *p;
char **bufp;
int *lenp;
struct paster *pa;
{
  int oldlen;
  struct display *d = display;

#ifdef COPY_PASTE
  /* XXX -> PasteStart */
  if (pa && *lenp > 1 && p && p->w_slowpaste)
    {
      /* schedule slowpaste event */
      SetTimeout(&p->w_paster.pa_slowev, p->w_slowpaste);
      evenq(&p->w_paster.pa_slowev);
      return;
    }
#endif
  while (flayer && *lenp)
    {
#ifdef COPY_PASTE
      if (!pa && p && p->w_paster.pa_pastelen && flayer == p->w_paster.pa_pastelayer)
	{
	  debug("layer is busy - beep!\n");
	  WBell(p, visual_bell);
	  *bufp += *lenp;
	  *lenp = 0;
	  display = d;
	  return;
	}
#endif
      oldlen = *lenp;
      LayProcess(bufp, lenp);
#ifdef COPY_PASTE
      if (pa && !pa->pa_pastelayer)
	break;		/* flush rest of paste */
#endif
      if (*lenp == oldlen)
	{
	  if (pa)
	    {
	      display = d;
	      return;
	    }
	  /* We're full, let's beep */
	  debug("layer is full - beep!\n");
	  WBell(p, visual_bell);
	  break;
	}
    }
  *bufp += *lenp;
  *lenp = 0;
  display = d;
#ifdef COPY_PASTE
  if (pa && pa->pa_pastelen == 0)
    FreePaster(pa);
#endif
}

int
FindCommnr(str)
const char *str;
{
  int x, m, l = 0, r = RC_LAST;
  while (l <= r)
    {
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

static int
CheckArgNum(nr, args)
int nr;
char **args;
{
  int i, n;
  static char *argss[] = {"no", "one", "two", "three", "four", "OOPS"};
  static char *orformat[] = 
    {
      "%s: %s: %s argument%s required",
      "%s: %s: %s or %s argument%s required",
      "%s: %s: %s, %s or %s argument%s required",
      "%s: %s: %s, %s, %s or %s argument%s required"
    };

  n = comms[nr].flags & ARGS_MASK;
  for (i = 0; args[i]; i++)
    ;
  if (comms[nr].flags & ARGS_ORMORE)
    {
      if (i < n)
	{
	  Msg(0, "%s: %s: at least %s argument%s required", 
	      rc_name, comms[nr].name, argss[n], n != 1 ? "s" : "");
	  return -1;
	}
    }
  else if ((comms[nr].flags & ARGS_PLUS1) && 
           (comms[nr].flags & ARGS_PLUS2) &&
	   (comms[nr].flags & ARGS_PLUS3))
    {
      if (i != n && i != n + 1 && i != n + 2 && i != n + 3)
        {
	  Msg(0, orformat[3], rc_name, comms[nr].name, argss[n], 
	      argss[n + 1], argss[n + 2], argss[n + 3], "");
	  return -1;
	}
    }
  else if ((comms[nr].flags & ARGS_PLUS1) &&
           (comms[nr].flags & ARGS_PLUS2))
    {
      if (i != n && i != n + 1 && i != n + 2)
	{
	  Msg(0, orformat[2], rc_name, comms[nr].name, argss[n], 
	      argss[n + 1], argss[n + 2], "");
          return -1;
	}
    }
  else if ((comms[nr].flags & ARGS_PLUS1) &&
           (comms[nr].flags & ARGS_PLUS3))
    {
      if (i != n && i != n + 1 && i != n + 3)
        {
	  Msg(0, orformat[2], rc_name, comms[nr].name, argss[n], 
	      argss[n + 1], argss[n + 3], "");
	  return -1;
	}
    }
  else if ((comms[nr].flags & ARGS_PLUS2) &&
           (comms[nr].flags & ARGS_PLUS3))
    {
      if (i != n && i != n + 2 && i != n + 3)
        {
	  Msg(0, orformat[2], rc_name, comms[nr].name, argss[n], 
	      argss[n + 2], argss[n + 3], "");
	  return -1;
	}
    }
  else if (comms[nr].flags & ARGS_PLUS1)
    {
      if (i != n && i != n + 1)
        {
	  Msg(0, orformat[1], rc_name, comms[nr].name, argss[n], 
	      argss[n + 1], n != 0 ? "s" : "");
	  return -1;
	}
    }
  else if (comms[nr].flags & ARGS_PLUS2)
    {
      if (i != n && i != n + 2)
        {
	  Msg(0, orformat[1], rc_name, comms[nr].name, argss[n], 
	      argss[n + 2], "s");
	  return -1;
	}
    }
  else if (comms[nr].flags & ARGS_PLUS3)
    {
      if (i != n && i != n + 3)
        {
	  Msg(0, orformat[1], rc_name, comms[nr].name, argss[n], 
	      argss[n + 3], "");
	  return -1;
	}
    }
  else if (i != n)
    {
      Msg(0, orformat[0], rc_name, comms[nr].name, argss[n], n != 1 ? "s" : "");
      return -1;
    }
  return i;
}

static void
StuffFin(buf, len, data)
char *buf;
int len;
char *data;
{
  if (!flayer)
    return;
  while(len)
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

/*ARGSUSED*/
void
DoAction(act, key)
struct action *act;
int key;
{
  int nr = act->nr;
  char **args = act->args;
  int *argl = act->argl;
  struct win *p;
  int argc, i, n, msgok;
  char *s;
  char ch;
  struct display *odisplay = display;
  struct acluser *user;

  user = display ? D_user : users;
  if (nr == RC_ILLEGAL)
    {
      debug1("key '%c': No action\n", key);
      return;
    }
  n = comms[nr].flags;
  /* Commands will have a CAN_QUERY flag, depending on whether they have
   * something to return on a query. For example, 'windows' can return a result,
   * but 'other' cannot.
   * If some command causes an error, then it should reset queryflag to -1, so that
   * the process requesting the query can be notified that an error happened.
   */
  if (!(n & CAN_QUERY) && queryflag >= 0)
    {
      /* Query flag is set, but this command cannot be queried. */
      OutputMsg(0, "%s command cannot be queried.", comms[nr].name);
      queryflag = -1;
      return;
    }
  if ((n & NEED_DISPLAY) && display == 0)
    {
      OutputMsg(0, "%s: %s: display required", rc_name, comms[nr].name);
      queryflag = -1;
      return;
    }
  if ((n & NEED_FORE) && fore == 0)
    {
      OutputMsg(0, "%s: %s: window required", rc_name, comms[nr].name);
      queryflag = -1;
      return;
    }
  if ((n & NEED_LAYER) && flayer == 0)
    {
      OutputMsg(0, "%s: %s: display or window required", rc_name, comms[nr].name);
      queryflag = -1;
      return;
    }
  if ((argc = CheckArgNum(nr, args)) < 0)
    return;
#ifdef MULTIUSER
  if (display)
    {
      if (AclCheckPermCmd(D_user, ACL_EXEC, &comms[nr]))
        {
	  OutputMsg(0, "%s: %s: permission denied (user %s)", 
	      rc_name, comms[nr].name, (EffectiveAclUser ? EffectiveAclUser : D_user)->u_name);
	  queryflag = -1;
	  return;
	}
    }
#endif /* MULTIUSER */

  msgok = display && !*rc_name;
  switch(nr)
    {
    case RC_SELECT:
      if (!*args)
        InputSelect();
      else if (args[0][0] == '-' && !args[0][1])
	{
	  SetForeWindow((struct win *)0);
	  Activate(0);
	}
      else if (args[0][0] == '.' && !args[0][1])
	{
	  if (!fore)
	    {
	      OutputMsg(0, "select . needs a window");
	      queryflag = -1;
	    }
	  else
	    {
	      SetForeWindow(fore);
	      Activate(0);
	    }
	}
      else if (ParseWinNum(act, &n) == 0)
        SwitchWindow(n);
      else if (queryflag >= 0)
	queryflag = -1;	/* ParseWinNum already prints out an appropriate error message. */
      break;
#ifdef AUTO_NUKE
    case RC_DEFAUTONUKE:
      if (ParseOnOff(act, &defautonuke) == 0 && msgok)
	OutputMsg(0, "Default autonuke turned %s", defautonuke ? "on" : "off");
      if (display && *rc_name)
	D_auto_nuke = defautonuke;
      break;
    case RC_AUTONUKE:
      if (ParseOnOff(act, &D_auto_nuke) == 0 && msgok)
	OutputMsg(0, "Autonuke turned %s", D_auto_nuke ? "on" : "off");
      break;
#endif
    case RC_DEFOBUFLIMIT:
      if (ParseNum(act, &defobuflimit) == 0 && msgok)
	OutputMsg(0, "Default limit set to %d", defobuflimit);
      if (display && *rc_name)
	{
	  D_obufmax = defobuflimit;
	  D_obuflenmax = D_obuflen - D_obufmax;
	}
      break;
    case RC_OBUFLIMIT:
      if (*args == 0)
	OutputMsg(0, "Limit is %d, current buffer size is %d", D_obufmax, D_obuflen);
      else if (ParseNum(act, &D_obufmax) == 0 && msgok)
	OutputMsg(0, "Limit set to %d", D_obufmax);
      D_obuflenmax = D_obuflen - D_obufmax;
      break;
    case RC_DUMPTERMCAP:
      WriteFile(user, (char *)0, DUMP_TERMCAP);
      break;
    case RC_HARDCOPY:
      {
	int mode = DUMP_HARDCOPY;
	char *file = NULL;

	if (args[0])
	  {
	    if (!strcmp(*args, "-h"))
	      {
		mode = DUMP_SCROLLBACK;
		file = args[1];
	      }
	    else if (!strcmp(*args, "--") && args[1])
	      file = args[1];
	    else
	      file = args[0];
	  }

	if (args[0] && file == args[0] && args[1])
	  {
	    OutputMsg(0, "%s: hardcopy: too many arguments", rc_name);
	    break;
	  }
	WriteFile(user, file, mode);
      }
      break;
    case RC_DEFLOG:
      (void)ParseOnOff(act, &nwin_default.Lflag);
      break;
    case RC_LOG:
      n = fore->w_log ? 1 : 0;
      ParseSwitch(act, &n);
      LogToggle(n);
      break;
#ifdef BSDJOBS
    case RC_SUSPEND:
      Detach(D_STOP);
      break;
#endif
    case RC_NEXT:
      if (MoreWindows())
	SwitchWindow(NextWindow());
      break;
    case RC_PREV:
      if (MoreWindows())
	SwitchWindow(PreviousWindow());
      break;
    case RC_KILL:
      {
	char *name;

	if (key >= 0)
	  {
#ifdef PSEUDOS
	    Input(fore->w_pwin ? "Really kill this filter [y/n]" : "Really kill this window [y/n]", 1, INP_RAW, confirm_fn, NULL, RC_KILL);
#else
	    Input("Really kill this window [y/n]", 1, INP_RAW, confirm_fn, NULL, RC_KILL);
#endif
	    break;
	  }
	n = fore->w_number;
#ifdef PSEUDOS
	if (fore->w_pwin)
	  {
	    FreePseudowin(fore);
	    OutputMsg(0, "Filter removed.");
	    break;
	  }
#endif
	name = SaveStr(fore->w_title);
	KillWindow(fore);
	OutputMsg(0, "Window %d (%s) killed.", n, name);
	if (name)
	  free(name);
	break;
      }
    case RC_QUIT:
      if (key >= 0)
	{
	  Input("Really quit and kill all your windows [y/n]", 1, INP_RAW, confirm_fn, NULL, RC_QUIT);
	  break;
	}
      Finit(0);
      /* NOTREACHED */
#ifdef DETACH
    case RC_DETACH:
      if (*args && !strcmp(*args, "-h"))
        Hangup();
      else
        Detach(D_DETACH);
      break;
# ifdef POW_DETACH
    case RC_POW_DETACH:
      if (key >= 0)
	{
	  static char buf[2];

	  buf[0] = key;
	  Input(buf, 1, INP_RAW, pow_detach_fn, NULL, 0);
	}
      else
        Detach(D_POWER); /* detach and kill Attacher's parent */
      break;
# endif
#endif
    case RC_DEBUG:
#ifdef DEBUG
      if (!*args)
        {
	  if (dfp)
	    OutputMsg(0, "debugging info is written to %s/", DEBUGDIR);
	  else
	    OutputMsg(0, "debugging is currently off. Use 'debug on' to enable.");
	  break;
	}
      if (dfp)
        {
	  debug("debug: closing debug file.\n");
	  fflush(dfp);
	  fclose(dfp);
	  dfp = NULL;
	}
      if (strcmp("off", *args))
        opendebug(0, 1);
# ifdef SIG_NODEBUG
      else if (display)
        kill(D_userpid, SIG_NODEBUG);	/* a one shot item, but hey... */
# endif /* SIG_NODEBUG */
#else
      if (*args == 0 || strcmp("off", *args))
        OutputMsg(0, "Sorry, screen was compiled without -DDEBUG option.");
#endif
      break;
#ifdef ZMODEM
    case RC_ZMODEM:
      if (*args && !strcmp(*args, "sendcmd"))
	{
	  if (args[1])
	    {
	      free(zmodem_sendcmd);
	      zmodem_sendcmd = SaveStr(args[1]);
	    }
	  if (msgok)
	    OutputMsg(0, "zmodem sendcmd: %s", zmodem_sendcmd);
	  break;
	}
      if (*args && !strcmp(*args, "recvcmd"))
	{
	  if (args[1])
	    {
	      free(zmodem_recvcmd);
	      zmodem_recvcmd = SaveStr(args[1]);
	    }
	  if (msgok)
	    OutputMsg(0, "zmodem recvcmd: %s", zmodem_recvcmd);
	  break;
	}
      if (*args)
	{
	  for (i = 0; i < 4; i++)
	    if (!strcmp(zmodes[i], *args))
	      break;
	  if (i == 4 && !strcmp(*args, "on"))
	    i = 1;
	  if (i == 4)
	    {
	      OutputMsg(0, "usage: zmodem off|auto|catch|pass");
	      break;
	    }
	  zmodem_mode = i;
	}
      if (msgok)
	OutputMsg(0, "zmodem mode is %s", zmodes[zmodem_mode]);
      break;
#endif
    case RC_UNBINDALL:
      {
        register unsigned int i;

        for (i = 0; i < sizeof(ktab)/sizeof(*ktab); i++)
	  ClearAction(&ktab[i]);
        OutputMsg(0, "Unbound all keys." );
        break;
      }
    case RC_ZOMBIE:
      {
        if (!(s = *args))
          {
            ZombieKey_destroy = 0;
            break;
          }
	if (*argl == 0 || *argl > 2)
	  {
	    OutputMsg(0, "%s:zombie: one or two characters expected.", rc_name);
	    break;
	  }
	if (args[1])
	  {
	    if (!strcmp(args[1], "onerror"))
	      {
		ZombieKey_onerror = 1;
	      } else {
		OutputMsg(0, "usage: zombie [keys [onerror]]");
	    	break;
	      }
	  } else
	    ZombieKey_onerror = 0;
        ZombieKey_destroy = args[0][0];
        ZombieKey_resurrect = *argl == 2 ? args[0][1] : 0;
      }
      break;
    case RC_WALL:
#ifdef MULTIUSER
      s = D_user->u_name;
#else
      s = D_usertty;
#endif
        {
	  struct display *olddisplay = display;
          display = 0;		/* no display will cause a broadcast */
          OutputMsg(0, "%s: %s", s, *args);
	  display = olddisplay;
        }
      break;
    case RC_AT:
      /* where this AT command comes from: */
      if (!user)
	break;
#ifdef MULTIUSER
      s = SaveStr(user->u_name);
      /* DO NOT RETURN FROM HERE WITHOUT RESETTING THIS: */
      EffectiveAclUser = user;
#else
      s = SaveStr(display ? D_usertty : user->u_name);
#endif
      n = strlen(args[0]);
      if (n) n--;
      /*
       * the windows/displays loops are quite dangerous here, take extra
       * care not to trigger landmines. Things may appear/disappear while
       * we are walking along.
       */
      switch (args[0][n])
        {
	case '*':		/* user */
	  {
	    struct display *nd;
	    struct acluser *u;

	    if (!n)
	      u = user;
	    else
	      {
		for (u = users; u; u = u->u_next)
		  {
		    debug3("strncmp('%s', '%s', %d)\n", *args, u->u_name, n);
		    if (!strncmp(*args, u->u_name, n))
		      break;
		  }
		if (!u)
		  {
		    args[0][n] = '\0';
		    OutputMsg(0, "Did not find any user matching '%s'", args[0]);
		    break;
		  }
	      }
	    debug1("at all displays of user %s\n", u->u_name);
	    for (display = displays; display; display = nd)
	      {
		nd = display->d_next;
		if (D_forecv == 0)
		  continue;
		flayer = D_forecv->c_layer;
		fore = D_fore;
	        if (D_user != u)
		  continue;
		debug1("AT display %s\n", D_usertty);
		DoCommand(args + 1, argl + 1);
		if (display)
		  OutputMsg(0, "command from %s: %s %s", 
		      s, args[1], args[2] ? args[2] : "");
		display = NULL;
		flayer = 0;
		fore = NULL;
	      }
	    break;
	  }
	case '%':		/* display */
	  {
	    struct display *nd;

	    debug1("at display matching '%s'\n", args[0]);
	    for (display = displays; display; display = nd)
	      {
	        nd = display->d_next;
		if (D_forecv == 0)
		  continue;
		fore = D_fore;
		flayer = D_forecv->c_layer;
	        if (strncmp(args[0], D_usertty, n) && 
		    (strncmp("/dev/", D_usertty, 5) || 
		     strncmp(args[0], D_usertty + 5, n)) &&
		    (strncmp("/dev/tty", D_usertty, 8) ||
		     strncmp(args[0], D_usertty + 8, n)))
		  continue;
		debug1("AT display %s\n", D_usertty);
		DoCommand(args + 1, argl + 1);
		if (display)
		  OutputMsg(0, "command from %s: %s %s", 
		      s, args[1], args[2] ? args[2] : "");
		display = NULL;
		fore = NULL;
		flayer = 0;
	      }
	    break;
	  }
	case '#':		/* window */
	  n--;
	  /* FALLTHROUGH */
	default:
	  {
	    struct win *nw;
	    int ch;

	    n++;
	    ch = args[0][n];
	    args[0][n] = '\0';
	    if (!*args[0] || (i = WindowByNumber(args[0])) < 0)
	      {
	        args[0][n] = ch;      /* must restore string in case of bind */
	        /* try looping over titles */
		for (fore = windows; fore; fore = nw)
		  {
		    nw = fore->w_next;
		    if (strncmp(args[0], fore->w_title, n))
		      continue;
		    debug2("AT window %d(%s)\n", fore->w_number, fore->w_title);
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
		    if (fore && fore->w_layer.l_cvlist)
		      {
		        display = fore->w_layer.l_cvlist->c_display;
		        OutputMsg(0, "command from %s: %s %s", 
			    s, args[1], args[2] ? args[2] : "");
		      }
		  }
		display = NULL;
		fore = NULL;
		if (i < 0)
		  OutputMsg(0, "%s: at '%s': no such window.\n", rc_name, args[0]);
		break;
	      }
	    else if (i < maxwin && (fore = wtab[i]))
	      {
	        args[0][n] = ch;      /* must restore string in case of bind */
	        debug2("AT window %d (%s)\n", fore->w_number, fore->w_title);
		if (fore->w_layer.l_cvlist)
		  display = fore->w_layer.l_cvlist->c_display;
		flayer = fore->w_savelayer ? fore->w_savelayer : &fore->w_layer;
		DoCommand(args + 1, argl + 1);
		if (fore && fore->w_layer.l_cvlist)
		  {
		    display = fore->w_layer.l_cvlist->c_display;
		    OutputMsg(0, "command from %s: %s %s", 
		        s, args[1], args[2] ? args[2] : "");
		  }
		display = NULL;
		fore = NULL;
	      }
	    else
	      OutputMsg(0, "%s: at [identifier][%%|*|#] command [args]", rc_name);
	    break;
	  }
	}
      free(s);
#ifdef MULTIUSER
      EffectiveAclUser = NULL;
#endif
      break;

#ifdef COPY_PASTE
    case RC_READREG:
#ifdef ENCODINGS
      i = fore ? fore->w_encoding : display ? display->d_encoding : 0;
      if (args[0] && args[1] && !strcmp(args[0], "-e"))
	{
	  i = FindEncoding(args[1]);
	  if (i == -1)
	    {
	      OutputMsg(0, "%s: readreg: unknown encoding", rc_name);
	      break;
	    }
	  args += 2;
	}
#endif
      /* 
       * Without arguments we prompt for a destination register.
       * It will receive the copybuffer contents.
       * This is not done by RC_PASTE, as we prompt for source
       * (not dest) there.
       */
      if ((s = *args) == NULL)
	{
	  Input("Copy to register:", 1, INP_RAW, copy_reg_fn, NULL, 0);
	  break;
	}
      if (*argl != 1)
	{
	  OutputMsg(0, "%s: copyreg: character, ^x, or (octal) \\032 expected.", rc_name);
	  break;
	}
      ch = args[0][0];
      /* 
       * With two arguments we *really* read register contents from file
       */
      if (args[1])
        {
	  if (args[2])
	    {
	      OutputMsg(0, "%s: readreg: too many arguments", rc_name);
	      break;
	    }
	  if ((s = ReadFile(args[1], &n)))
	    {
	      struct plop *pp = plop_tab + (int)(unsigned char)ch;

	      if (pp->buf)
		free(pp->buf);
	      pp->buf = s;
	      pp->len = n;
#ifdef ENCODINGS
	      pp->enc = i;
#endif
	    }
	}
      else
        /*
	 * with one argument we copy the copybuffer into a specified register
	 * This could be done with RC_PASTE too, but is here to be consistent
	 * with the zero argument call.
	 */
        copy_reg_fn(&ch, 0, NULL);
      break;
#endif
    case RC_REGISTER:
#ifdef ENCODINGS
      i = fore ? fore->w_encoding : display ? display->d_encoding : 0;
      if (args[0] && args[1] && !strcmp(args[0], "-e"))
	{
	  i = FindEncoding(args[1]);
	  if (i == -1)
	    {
	      OutputMsg(0, "%s: register: unknown encoding", rc_name);
	      break;
	    }
	  args += 2;
	  argc -= 2;
	}
#endif
      if (argc != 2)
	{
	  OutputMsg(0, "%s: register: illegal number of arguments.", rc_name);
	  break;
	}
      if (*argl != 1)
	{
	  OutputMsg(0, "%s: register: character, ^x, or (octal) \\032 expected.", rc_name);
	  break;
	}
      ch = args[0][0];
#ifdef COPY_PASTE
      if (ch == '.')
	{
	  if (user->u_plop.buf != NULL)
	    UserFreeCopyBuffer(user);
	  if (args[1] && args[1][0])
	    {
	      user->u_plop.buf = SaveStrn(args[1], argl[1]);
	      user->u_plop.len = argl[1];
#ifdef ENCODINGS
	      user->u_plop.enc = i;
#endif
	    }
	}
      else
#endif
	{
	  struct plop *plp = plop_tab + (int)(unsigned char)ch;

	  if (plp->buf)
	    free(plp->buf);
	  plp->buf = SaveStrn(args[1], argl[1]);
	  plp->len = argl[1];
#ifdef ENCODINGS
	  plp->enc = i;
#endif
	}
      break;
    case RC_PROCESS:
      if ((s = *args) == NULL)
	{
	  Input("Process register:", 1, INP_RAW, process_fn, NULL, 0);
	  break;
	}
      if (*argl != 1)
	{
	  OutputMsg(0, "%s: process: character, ^x, or (octal) \\032 expected.", rc_name);
	  break;
	}
      ch = args[0][0];
      process_fn(&ch, 0, NULL);
      break;
    case RC_STUFF:
      s = *args;
      if (!args[0])
	{
	  Input("Stuff:", 100, INP_COOKED, StuffFin, NULL, 0);
	  break;
	}
      n = *argl;
      if (args[1])
	{
	  if (strcmp(s, "-k"))
	    {
	      OutputMsg(0, "%s: stuff: invalid option %s", rc_name, s);
	      break;
	    }
	  s = args[1];
	  for (i = T_CAPS; i < T_OCAPS; i++)
	    if (strcmp(term[i].tcname, s) == 0)
	      break;
	  if (i == T_OCAPS)
	    {
	      OutputMsg(0, "%s: stuff: unknown key '%s'", rc_name, s);
	      break;
	    }
#ifdef MAPKEYS
	  if (StuffKey(i - T_CAPS) == 0)
	    break;
#endif
	  s = display ? D_tcs[i].str : 0;
	  if (s == 0)
	    break;
	  n = strlen(s);
	}
      while(n)
        LayProcess(&s, &n);
      break;
    case RC_REDISPLAY:
      Activate(-1);
      break;
    case RC_WINDOWS:
			if (args[0]) {
				ShowWindowsX(args[0]);
				break;
			}
      ShowWindows(-1);
      break;
    case RC_VERSION:
      OutputMsg(0, "screen %s", version);
      break;
    case RC_TIME:
      if (*args)
	{
	  timestring = SaveStr(*args);
	  break;
	}
      OutputMsg(0, "%s", MakeWinMsg(timestring, fore, '%'));
      break;
    case RC_INFO:
      ShowInfo();
      break;
    case RC_DINFO:
      ShowDInfo();
      break;
    case RC_COMMAND:
	{
	  struct action *ktabp = ktab;
	  if (argc == 2 && !strcmp(*args, "-c"))
	    {
	      if ((ktabp = FindKtab(args[1], 0)) == 0)
		{
		  OutputMsg(0, "Unknown command class '%s'", args[1]);
		  break;
		}
	    }
	  if (D_ESCseen != ktab || ktabp != ktab)
	    {
	      if (D_ESCseen != ktabp)
	        {
	          D_ESCseen = ktabp;
	          WindowChanged(fore, 'E');
	        }
	      break;
	    }
	  if (D_ESCseen)
	    {
	      D_ESCseen = 0;
	      WindowChanged(fore, 'E');
	    }
	}
      /* FALLTHROUGH */
    case RC_OTHER:
      if (MoreWindows())
	SwitchWindow(display && D_other ? D_other->w_number : NextWindow());
      break;
    case RC_META:
      if (user->u_Esc == -1)
        break;
      ch = user->u_Esc;
      s = &ch;
      n = 1;
      LayProcess(&s, &n);
      break;
    case RC_XON:
      ch = Ctrl('q');
      s = &ch;
      n = 1;
      LayProcess(&s, &n);
      break;
    case RC_XOFF:
      ch = Ctrl('s');
      s = &ch;
      n = 1;
      LayProcess(&s, &n);
      break;
    case RC_DEFBREAKTYPE:
    case RC_BREAKTYPE:
	{
	  static char *types[] = { "TIOCSBRK", "TCSBRK", "tcsendbreak", NULL };
	  extern int breaktype;

	  if (*args)
	    {
	      if (ParseNum(act, &n))
		for (n = 0; n < (int)(sizeof(types)/sizeof(*types)); n++)
		  {
		    for (i = 0; i < 4; i++)
		      {
			ch = args[0][i];
			if (ch >= 'a' && ch <= 'z')
			  ch -= 'a' - 'A';
			if (ch != types[n][i] && (ch + ('a' - 'A')) != types[n][i])
			  break;
		      }
		    if (i == 4)
		      break;
		  }
	      if (n < 0 || n >= (int)(sizeof(types)/sizeof(*types)))
	        OutputMsg(0, "%s invalid, chose one of %s, %s or %s", *args, types[0], types[1], types[2]);
	      else
	        {
		  breaktype = n;
	          OutputMsg(0, "breaktype set to (%d) %s", n, types[n]);
		}
	    }
	  else
	    OutputMsg(0, "breaktype is (%d) %s", breaktype, types[breaktype]);
	}
      break;
    case RC_POW_BREAK:
    case RC_BREAK:
      n = 0;
      if (*args && ParseNum(act, &n))
	break;
      SendBreak(fore, n, nr == RC_POW_BREAK);
      break;
#ifdef LOCK
    case RC_LOCKSCREEN:
      Detach(D_LOCK);
      break;
#endif
    case RC_WIDTH:
    case RC_HEIGHT:
      {
	int w, h;
	int what = 0;
	
        i = 1;
	if (*args && !strcmp(*args, "-w"))
	  what = 1;
	else if (*args && !strcmp(*args, "-d"))
	  what = 2;
	if (what)
	  args++;
	if (what == 0 && flayer && !display)
	  what = 1;
	if (what == 1)
	  {
	    if (!flayer)
	      {
		OutputMsg(0, "%s: %s: window required", rc_name, comms[nr].name);
		break;
	      }
	    w = flayer->l_width;
	    h = flayer->l_height;
	  }
	else
	  {
	    if (!display)
	      {
		OutputMsg(0, "%s: %s: display required", rc_name, comms[nr].name);
		break;
	      }
	    w = D_width;
	    h = D_height;
	  }
        if (*args && args[0][0] == '-')
	  {
	    OutputMsg(0, "%s: %s: unknown option %s", rc_name, comms[nr].name, *args);
	    break;
	  }
	if (nr == RC_HEIGHT)
	  {
	    if (!*args)
	      {
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
	      }
	    else
	      {
		h = atoi(*args);
		if (args[1])
		  w = atoi(args[1]);
	      }
	  }
	else
	  {
	    if (!*args)
	      {
		if (w == Z0width)
		  w = Z1width;
		else if (w == Z1width)
		  w = Z0width;
		else if (w > (Z0width + Z1width) / 2)
		  w = Z0width;
		else
		  w = Z1width;
	      }
	    else
	      {
		w = atoi(*args);
		if (args[1])
		  h = atoi(args[1]);
	      }
	  }
        if (*args && args[1] && args[2])
	  {
	    OutputMsg(0, "%s: %s: too many arguments", rc_name, comms[nr].name);
	    break;
	  }
	if (w <= 0)
	  {
	    OutputMsg(0, "Illegal width");
	    break;
	  }
	if (h <= 0)
	  {
	    OutputMsg(0, "Illegal height");
	    break;
	  }
	if (what == 1)
	  {
	    if (flayer->l_width == w && flayer->l_height == h)
	      break;
	    ResizeLayer(flayer, w, h, (struct display *)0);
	    break;
	  }
	if (D_width == w && D_height == h)
	  break;
	if (what == 2)
	  {
	    ChangeScreenSize(w, h, 1);
	  }
	else
	  {
	    if (ResizeDisplay(w, h) == 0)
	      {
		Activate(D_fore ? D_fore->w_norefresh : 0);
		/* autofit */
		ResizeLayer(D_forecv->c_layer, D_forecv->c_xe - D_forecv->c_xs + 1, D_forecv->c_ye - D_forecv->c_ys + 1, 0);
		break;
	      }
	    if (h == D_height)
	      OutputMsg(0, "Your termcap does not specify how to change the terminal's width to %d.", w);
	    else if (w == D_width)
	      OutputMsg(0, "Your termcap does not specify how to change the terminal's height to %d.", h);
	    else
	      OutputMsg(0, "Your termcap does not specify how to change the terminal's resolution to %dx%d.", w, h);
	  }
      }
      break;
    case RC_DEFDYNAMICTITLE:
      (void)ParseOnOff(act, &nwin_default.dynamicaka);
      break;
    case RC_DYNAMICTITLE:
      (void)ParseOnOff(act, &fore->w_dynamicaka);
      break;
    case RC_TITLE:
      if (queryflag >= 0)
	{
	  if (fore)
	    OutputMsg(0, "%s", fore->w_title);
	  else
	    queryflag = -1;
	  break;
	}
      if (*args == 0)
	InputAKA();
      else
	ChangeAKA(fore, *args, strlen(*args));
      break;
    case RC_COLON:
      Input(":", MAXSTR, INP_EVERY, Colonfin, NULL, 0);
      if (*args && **args)
	{
	  s = *args;
	  n = strlen(s);
	  LayProcess(&s, &n);
	}
      break;
    case RC_LASTMSG:
      if (D_status_lastmsg)
	OutputMsg(0, "%s", D_status_lastmsg);
      break;
    case RC_SCREEN:
      DoScreen("key", args);
      break;
    case RC_WRAP:
      if (ParseSwitch(act, &fore->w_wrap) == 0 && msgok)
        OutputMsg(0, "%cwrap", fore->w_wrap ? '+' : '-');
      break;
    case RC_FLOW:
      if (*args)
	{
	  if (args[0][0] == 'a')
	    {
	      fore->w_flow = (fore->w_flow & FLOW_AUTO) ? FLOW_AUTOFLAG |FLOW_AUTO|FLOW_NOW : FLOW_AUTOFLAG;
	    }
	  else
	    {
	      if (ParseOnOff(act, &n))
		break;
	      fore->w_flow = (fore->w_flow & FLOW_AUTO) | n;
	    }
	}
      else
	{
	  if (fore->w_flow & FLOW_AUTOFLAG)
	    fore->w_flow = (fore->w_flow & FLOW_AUTO) | FLOW_NOW;
	  else if (fore->w_flow & FLOW_NOW)
	    fore->w_flow &= ~FLOW_NOW;
	  else
	    fore->w_flow = fore->w_flow ? FLOW_AUTOFLAG|FLOW_AUTO|FLOW_NOW : FLOW_AUTOFLAG;
	}
      SetFlow(fore->w_flow & FLOW_NOW);
      if (msgok)
	OutputMsg(0, "%cflow%s", (fore->w_flow & FLOW_NOW) ? '+' : '-',
	    (fore->w_flow & FLOW_AUTOFLAG) ? "(auto)" : "");
      break;
#ifdef MULTIUSER
    case RC_DEFWRITELOCK:
      if (args[0][0] == 'a')
	nwin_default.wlock = WLOCK_AUTO;
      else
	{
	  if (ParseOnOff(act, &n))
	    break;
	  nwin_default.wlock = n ? WLOCK_ON : WLOCK_OFF;
	}
      break;
    case RC_WRITELOCK:
      if (*args)
	{
	  if (args[0][0] == 'a')
	    {
	      fore->w_wlock = WLOCK_AUTO;
	    }
	  else
	    {
	      if (ParseOnOff(act, &n))
		break;
	      fore->w_wlock = n ? WLOCK_ON : WLOCK_OFF;
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
      break;
#endif
    case RC_CLEAR:
      ResetAnsiState(fore);
      WriteString(fore, "\033[H\033[J", 6);
      break;
    case RC_RESET:
      ResetAnsiState(fore);
#ifdef ZMODEM
      if (fore->w_zdisplay)
        zmodem_abort(fore, fore->w_zdisplay);
#endif
      WriteString(fore, "\033c", 2);
      break;
    case RC_MONITOR:
      n = fore->w_monitor != MON_OFF;
#ifdef MULTIUSER
      if (display)
	n = n && (ACLBYTE(fore->w_mon_notify, D_user->u_id) & ACLBIT(D_user->u_id));
#endif
      if (ParseSwitch(act, &n))
	break;
      if (n)
	{
#ifdef MULTIUSER
	  if (display)	/* we tell only this user */
	    ACLBYTE(fore->w_mon_notify, D_user->u_id) |= ACLBIT(D_user->u_id);
	  else
	    for (i = 0; i < maxusercount; i++)
	      ACLBYTE(fore->w_mon_notify, i) |= ACLBIT(i);
#endif
	  if (fore->w_monitor == MON_OFF)
	    fore->w_monitor = MON_ON;
	  OutputMsg(0, "Window %d (%s) is now being monitored for all activity.", fore->w_number, fore->w_title);
	}
      else
	{
#ifdef MULTIUSER
	  if (display) /* we remove only this user */
	    ACLBYTE(fore->w_mon_notify, D_user->u_id) 
	      &= ~ACLBIT(D_user->u_id);
	  else
	    for (i = 0; i < maxusercount; i++)
	      ACLBYTE(fore->w_mon_notify, i) &= ~ACLBIT(i);
	  for (i = maxusercount - 1; i >= 0; i--)
	    if (ACLBYTE(fore->w_mon_notify, i))
	      break;
	  if (i < 0)
#endif
	    fore->w_monitor = MON_OFF;
	  OutputMsg(0, "Window %d (%s) is no longer being monitored for activity.", fore->w_number, fore->w_title);
	}
      break;
#ifdef MULTI
    case RC_DISPLAYS:
      display_displays();
      break;
#endif
    case RC_WINDOWLIST:
      if (!*args)
        display_windows(0, WLIST_NUM, (struct win *)0);
      else if (!strcmp(*args, "string"))
	{
	  if (args[1])
	    {
	      if (wliststr)
		free(wliststr);
	      wliststr = SaveStr(args[1]);
	    }
	  if (msgok)
	    OutputMsg(0, "windowlist string is '%s'", wliststr);
	}
      else if (!strcmp(*args, "title"))
	{
	  if (args[1])
	    {
	      if (wlisttit)
		free(wlisttit);
	      wlisttit = SaveStr(args[1]);
	    }
	  if (msgok)
	    OutputMsg(0, "windowlist title is '%s'", wlisttit);
	}
      else
	{
	  int flag = 0;
	  int blank = 0;
	  for (i = 0; i < argc; i++)
	    if (!args[i])
	      continue;
	    else if (!strcmp(args[i], "-m"))
	      flag |= WLIST_MRU;
	    else if (!strcmp(args[i], "-b"))
	      blank = 1;
	    else if (!strcmp(args[i], "-g"))
	      flag |= WLIST_NESTED;
	    else
	      {
		OutputMsg(0, "usage: windowlist [-b] [-g] [-m] [string [string] | title [title]]");
		break;
	      }
	  if (i == argc)
	    display_windows(blank, flag, (struct win *)0);
	}
      break;
    case RC_HELP:
      if (argc == 2 && !strcmp(*args, "-c"))
	{
	  struct action *ktabp;
	  if ((ktabp = FindKtab(args[1], 0)) == 0)
	    {
	      OutputMsg(0, "Unknown command class '%s'", args[1]);
	      break;
	    }
          display_help(args[1], ktabp);
	}
      else
        display_help((char *)0, ktab);
      break;
    case RC_LICENSE:
      display_copyright();
      break;
#ifdef COPY_PASTE
    case RC_COPY:
      if (flayer->l_layfn != &WinLf)
	{
	  OutputMsg(0, "Must be on a window layer");
	  break;
	}
      MarkRoutine();
      WindowChanged(fore, 'P');
      break;
    case RC_HISTORY:
      {
        static char *pasteargs[] = {".", 0};
	static int pasteargl[] = {1};

	if (flayer->l_layfn != &WinLf)
	  {
	    OutputMsg(0, "Must be on a window layer");
	    break;
	  }
	if (GetHistory() == 0)
	  break;
	if (user->u_plop.buf == NULL)
	  break;
	args = pasteargs;
	argl = pasteargl;
      }
      /*FALLTHROUGH*/
    case RC_PASTE:
      {
        char *ss, *dbuf, dch;
        int l = 0;
# ifdef ENCODINGS
	int enc = -1;
# endif

	/*
	 * without args we prompt for one(!) register to be pasted in the window
	 */
	if ((s = *args) == NULL)
	  {
	    Input("Paste from register:", 1, INP_RAW, ins_reg_fn, NULL, 0);
	    break;
	  }
	if (args[1] == 0 && !fore)	/* no window? */
	  break;
	/*	
	 * with two arguments we paste into a destination register
	 * (no window needed here).
	 */
	if (args[1] && argl[1] != 1)
	  {
	    OutputMsg(0, "%s: paste destination: character, ^x, or (octal) \\032 expected.",
		rc_name);
	    break;
	  }
# ifdef ENCODINGS
        else if (fore)
	  enc = fore->w_encoding;
# endif

	/*
	 * measure length of needed buffer 
	 */
        for (ss = s = *args; (ch = *ss); ss++)
          {
	    if (ch == '.')
	      {
# ifdef ENCODINGS
		if (enc == -1)
		  enc = user->u_plop.enc;
		if (enc != user->u_plop.enc)
		  l += RecodeBuf((unsigned char *)user->u_plop.buf, user->u_plop.len, user->u_plop.enc, enc, (unsigned char *)0);
		else
# endif
		  l += user->u_plop.len;
	      }
	    else
	      {
# ifdef ENCODINGS
		if (enc == -1)
		  enc = plop_tab[(int)(unsigned char)ch].enc;
		if (enc != plop_tab[(int)(unsigned char)ch].enc)
		  l += RecodeBuf((unsigned char *)plop_tab[(int)(unsigned char)ch].buf, plop_tab[(int)(unsigned char)ch].len, plop_tab[(int)(unsigned char)ch].enc, enc, (unsigned char *)0);
		else
# endif
                  l += plop_tab[(int)(unsigned char)ch].len;
	      }
          }
        if (l == 0)
	  {
	    OutputMsg(0, "empty buffer");
	    break;
	  }
	/*
	 * shortcut: 
	 * if there is only one source and the destination is a window, then
	 * pass a pointer rather than duplicating the buffer.
	 */
        if (s[1] == 0 && args[1] == 0)
# ifdef ENCODINGS
	  if (enc == (*s == '.' ? user->u_plop.enc : plop_tab[(int)(unsigned char)*s].enc))
# endif
            {
	      MakePaster(&fore->w_paster, *s == '.' ? user->u_plop.buf : plop_tab[(int)(unsigned char)*s].buf, l, 0);
	      break;
            }
	/*
	 * if no shortcut, we construct a buffer
	 */
        if ((dbuf = (char *)malloc(l)) == 0)
          {
	    OutputMsg(0, "%s", strnomem);
	    break;
          }
        l = 0;
	/*
	 * concatenate all sources into our own buffer, copy buffer is
	 * special and is skipped if no display exists.
	 */
        for (ss = s; (ch = *ss); ss++)
          {
	    struct plop *pp = (ch == '.' ? &user->u_plop : &plop_tab[(int)(unsigned char)ch]);
#ifdef ENCODINGS
	    if (pp->enc != enc)
	      {
		l += RecodeBuf((unsigned char *)pp->buf, pp->len, pp->enc, enc, (unsigned char *)dbuf + l);
		continue;
	      }
#endif
	    bcopy(pp->buf, dbuf + l, pp->len);
	    l += pp->len;
          }
	/*
	 * when called with one argument we paste our buffer into the window 
	 */
	if (args[1] == 0)
	  {
	    MakePaster(&fore->w_paster, dbuf, l, 1);
	  }
	else
	  {
	    /*
	     * we have two arguments, the second is already in dch.
	     * use this as destination rather than the window.
	     */
	    dch = args[1][0];
	    if (dch == '.')
	      {
	        if (user->u_plop.buf != NULL)
	          UserFreeCopyBuffer(user);
		user->u_plop.buf = dbuf;
		user->u_plop.len = l;
#ifdef ENCODINGS
		user->u_plop.enc = enc;
#endif
	      }
	    else
	      {
		struct plop *pp = plop_tab + (int)(unsigned char)dch;
		if (pp->buf)
		  free(pp->buf);
		pp->buf = dbuf;
		pp->len = l;
#ifdef ENCODINGS
		pp->enc = enc;
#endif
	      }
	  }
        break;
      }
    case RC_WRITEBUF:
      if (!user->u_plop.buf)
	{
	  OutputMsg(0, "empty buffer");
	  break;
	}
#ifdef ENCODINGS
	{
	  struct plop oldplop;

	  oldplop = user->u_plop;
	  if (args[0] && args[1] && !strcmp(args[0], "-e"))
	    {
	      int enc, l;
	      char *newbuf;

	      enc = FindEncoding(args[1]);
	      if (enc == -1)
		{
		  OutputMsg(0, "%s: writebuf: unknown encoding", rc_name);
		  break;
		}
	      if (enc != oldplop.enc)
		{
		  l = RecodeBuf((unsigned char *)oldplop.buf, oldplop.len, oldplop.enc, enc, (unsigned char *)0);
		  newbuf = malloc(l + 1);
		  if (!newbuf)
		    {
		      OutputMsg(0, "%s", strnomem);
		      break;
		    }
		  user->u_plop.len = RecodeBuf((unsigned char *)oldplop.buf, oldplop.len, oldplop.enc, enc, (unsigned char *)newbuf);
		  user->u_plop.buf = newbuf;
		  user->u_plop.enc = enc;
		}
	      args += 2;
	    }
#endif
	  if (args[0] && args[1])
	    OutputMsg(0, "%s: writebuf: too many arguments", rc_name);
	  else
	    WriteFile(user, args[0], DUMP_EXCHANGE);
#ifdef ENCODINGS
	  if (user->u_plop.buf != oldplop.buf)
	    free(user->u_plop.buf);
	  user->u_plop = oldplop;
	}
#endif
      break;
    case RC_READBUF:
#ifdef ENCODINGS
      i = fore ? fore->w_encoding : display ? display->d_encoding : 0;
      if (args[0] && args[1] && !strcmp(args[0], "-e"))
	{
	  i = FindEncoding(args[1]);
	  if (i == -1)
	    {
	      OutputMsg(0, "%s: readbuf: unknown encoding", rc_name);
	      break;
	    }
	  args += 2;
	}
#endif
      if (args[0] && args[1])
	{
	  OutputMsg(0, "%s: readbuf: too many arguments", rc_name);
	  break;
	}
      if ((s = ReadFile(args[0] ? args[0] : BufferFile, &n)))
	{
	  if (user->u_plop.buf)
	    UserFreeCopyBuffer(user);
	  user->u_plop.len = n;
	  user->u_plop.buf = s;
#ifdef ENCODINGS
	  user->u_plop.enc = i;
#endif
	}
      break;
    case RC_REMOVEBUF:
      KillBuffers();
      break;
    case RC_IGNORECASE:
      (void)ParseSwitch(act, &search_ic);
      if (msgok)
        OutputMsg(0, "Will %signore case in searches", search_ic ? "" : "not ");
      break;
#endif				/* COPY_PASTE */
    case RC_ESCAPE:
      if (*argl == 0)
	SetEscape(user, -1, -1);
      else if (*argl == 2)
	SetEscape(user, (int)(unsigned char)args[0][0], (int)(unsigned char)args[0][1]);
      else
	{
	  OutputMsg(0, "%s: two characters required after escape.", rc_name);
	  break;
	}
      /* Change defescape if master user. This is because we only
       * have one ktab.
       */
      if (display && user != users)
	break;
      /* FALLTHROUGH */
    case RC_DEFESCAPE:
      if (*argl == 0)
	SetEscape(NULL, -1, -1);
      else if (*argl == 2)
	SetEscape(NULL, (int)(unsigned char)args[0][0], (int)(unsigned char)args[0][1]);
      else
	{
	  OutputMsg(0, "%s: two characters required after defescape.", rc_name);
	  break;
	}
#ifdef MAPKEYS
      CheckEscape();
#endif
      break;
    case RC_CHDIR:
      s = *args ? *args : home;
      if (chdir(s) == -1)
	OutputMsg(errno, "%s", s);
      break;
    case RC_SHELL:
    case RC_DEFSHELL:
      if (ParseSaveStr(act, &ShellProg) == 0)
        ShellArgs[0] = ShellProg;
      break;
    case RC_HARDCOPYDIR:
      if (*args)
        (void)ParseSaveStr(act, &hardcopydir);
      if (msgok)
	OutputMsg(0, "hardcopydir is %s\n", hardcopydir && *hardcopydir ? hardcopydir : "<cwd>");
      break;
    case RC_LOGFILE:
      if (*args)
	{
	  char buf[1024];
	  if (args[1] && !(strcmp(*args, "flush")))
	    {
	      log_flush = atoi(args[1]);
	      if (msgok)
		OutputMsg(0, "log flush timeout set to %ds\n", log_flush);
	      break;
	    }
	  if (ParseSaveStr(act, &screenlogfile))
	    break;
	  if (fore && fore->w_log)
	    if (DoStartLog(fore, buf, sizeof(buf)))
	      OutputMsg(0, "Error opening logfile \"%s\"", buf);
	  if (!msgok)
	    break;
	}
      OutputMsg(0, "logfile is '%s'", screenlogfile);
      break;
    case RC_LOGTSTAMP:
      if (!*args || !strcmp(*args, "on") || !strcmp(*args, "off"))
        {
	  if (ParseSwitch(act, &logtstamp_on) == 0 && msgok)
            OutputMsg(0, "timestamps turned %s", logtstamp_on ? "on" : "off");
        }
      else if (!strcmp(*args, "string"))
	{
	  if (args[1])
	    {
	      if (logtstamp_string)
		free(logtstamp_string);
	      logtstamp_string = SaveStr(args[1]);
	    }
	  if (msgok)
	    OutputMsg(0, "logfile timestamp is '%s'", logtstamp_string);
	}
      else if (!strcmp(*args, "after"))
	{
	  if (args[1])
	    {
	      logtstamp_after = atoi(args[1]);
	      if (!msgok)
		break;
	    }
	  OutputMsg(0, "timestamp printed after %ds\n", logtstamp_after);
	}
      else
        OutputMsg(0, "usage: logtstamp [after [n]|string [str]|on|off]");
      break;
    case RC_SHELLTITLE:
      (void)ParseSaveStr(act, &nwin_default.aka);
      break;
    case RC_TERMCAP:
    case RC_TERMCAPINFO:
    case RC_TERMINFO:
      if (!rc_name || !*rc_name)
        OutputMsg(0, "Sorry, too late now. Place that in your .screenrc file.");
      break;
    case RC_SLEEP:
      break;			/* Already handled */
    case RC_TERM:
      s = NULL;
      if (ParseSaveStr(act, &s))
	break;
      if (strlen(s) > MAXTERMLEN)
	{
	  OutputMsg(0, "%s: term: argument too long ( < %d)", rc_name, MAXTERMLEN);
	  free(s);
	  break;
	}
      strncpy(screenterm, s, MAXTERMLEN);
      screenterm[MAXTERMLEN] = '\0';
      free(s);
      debug1("screenterm set to %s\n", screenterm);
      MakeTermcap((display == 0));
      debug("new termcap made\n");
      break;
    case RC_ECHO:
      if (!msgok && (!rc_name || strcmp(rc_name, "-X")))
	break;
      /*
       * user typed ^A:echo... well, echo isn't FinishRc's job,
       * but as he wanted to test us, we show good will
       */
      if (argc > 1 && !strcmp(*args, "-n"))
	{
	  args++;
	  argc--;
	}
      s = *args;
      if (argc > 1 && !strcmp(*args, "-p"))
	{
	  args++;
	  argc--;
	  s = *args;
	  if (s)
	    s = MakeWinMsg(s, fore, '%');
	}
      if (s)
	OutputMsg(0, "%s", s);
      else
	{
	  OutputMsg(0, "%s: 'echo [-n] [-p] \"string\"' expected.", rc_name);
	  queryflag = -1;
	}
      break;
    case RC_BELL:
    case RC_BELL_MSG:
      if (*args == 0)
	{
	  char buf[256];
	  AddXChars(buf, sizeof(buf), BellString);
	  OutputMsg(0, "bell_msg is '%s'", buf);
	  break;
	}
      (void)ParseSaveStr(act, &BellString);
      break;
#ifdef COPY_PASTE
    case RC_BUFFERFILE:
      if (*args == 0)
	BufferFile = SaveStr(DEFAULT_BUFFERFILE);
      else if (ParseSaveStr(act, &BufferFile))
        break;
      if (msgok)
        OutputMsg(0, "Bufferfile is now '%s'", BufferFile);
      break;
#endif
    case RC_ACTIVITY:
      (void)ParseSaveStr(act, &ActivityString);
      break;
#if defined(DETACH) && defined(POW_DETACH)
    case RC_POW_DETACH_MSG:
      if (*args == 0)
        {
	  char buf[256];
          AddXChars(buf, sizeof(buf), PowDetachString);
	  OutputMsg(0, "pow_detach_msg is '%s'", buf);
	  break;
	}
      (void)ParseSaveStr(act, &PowDetachString);
      break;
#endif
#if defined(UTMPOK) && defined(LOGOUTOK)
    case RC_LOGIN:
      n = fore->w_slot != (slot_t)-1;
      if (*args && !strcmp(*args, "always"))
	{
	  fore->w_lflag = 3;
	  if (!displays && n)
	    SlotToggle(n);
	  break;
	}
      if (*args && !strcmp(*args, "attached"))
	{
	  fore->w_lflag = 1;
	  if (!displays && n)
	    SlotToggle(0);
	  break;
	}
      if (ParseSwitch(act, &n) == 0)
        SlotToggle(n);
      break;
    case RC_DEFLOGIN:
      if (!strcmp(*args, "always"))
	nwin_default.lflag |= 2;
      else if (!strcmp(*args, "attached"))
	nwin_default.lflag &= ~2;
      else
        (void)ParseOnOff(act, &nwin_default.lflag);
      break;
#endif
    case RC_DEFFLOW:
      if (args[0] && args[1] && args[1][0] == 'i')
	{
	  iflag = 1;
	  for (display = displays; display; display = display->d_next)
	    {
	      if (!D_flow)
		continue;
#if defined(TERMIO) || defined(POSIX)
	      D_NewMode.tio.c_cc[VINTR] = D_OldMode.tio.c_cc[VINTR];
	      D_NewMode.tio.c_lflag |= ISIG;
#else /* TERMIO || POSIX */
	      D_NewMode.m_tchars.t_intrc = D_OldMode.m_tchars.t_intrc;
#endif /* TERMIO || POSIX */
	      SetTTY(D_userfd, &D_NewMode);
	    }
	}
      if (args[0] && args[0][0] == 'a')
	nwin_default.flowflag = FLOW_AUTOFLAG;
      else
	(void)ParseOnOff(act, &nwin_default.flowflag);
      break;
    case RC_DEFWRAP:
      (void)ParseOnOff(act, &nwin_default.wrap);
      break;
    case RC_DEFC1:
      (void)ParseOnOff(act, &nwin_default.c1);
      break;
#ifdef COLOR
    case RC_DEFBCE:
      (void)ParseOnOff(act, &nwin_default.bce);
      break;
#endif
    case RC_DEFGR:
      (void)ParseOnOff(act, &nwin_default.gr);
      break;
    case RC_DEFMONITOR:
      if (ParseOnOff(act, &n) == 0)
        nwin_default.monitor = (n == 0) ? MON_OFF : MON_ON;
      break;
    case RC_DEFMOUSETRACK:
      if (ParseOnOff(act, &n) == 0)
	defmousetrack = (n == 0) ? 0 : 1000;
      break;
    case RC_MOUSETRACK:
      if (!args[0])
	{
	  OutputMsg(0, "Mouse tracking for this display is turned %s", D_mousetrack ? "on" : "off");
	}
      else if (ParseOnOff(act, &n) == 0)
	{
	  D_mousetrack = n == 0 ? 0 : 1000;
	  if (D_fore)
	    MouseMode(D_fore->w_mouse);
	}
      break;
    case RC_DEFSILENCE:
      if (ParseOnOff(act, &n) == 0)
        nwin_default.silence = (n == 0) ? SILENCE_OFF : SILENCE_ON;
      break;
    case RC_VERBOSE:
      if (!*args)
	OutputMsg(0, "W%s echo command when creating windows.", 
	  VerboseCreate ? "ill" : "on't");
      else if (ParseOnOff(act, &n) == 0)
        VerboseCreate = n;
      break;
    case RC_HARDSTATUS:
      if (display)
	{
	  OutputMsg(0, "%s", "");	/* wait till mintime (keep gcc quiet) */
          RemoveStatus();
	}
      if (args[0] && strcmp(args[0], "on") && strcmp(args[0], "off"))
	{
          struct display *olddisplay = display;
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
	  else if (!strcmp(args[0], "string"))
	    {
	      if (!args[1])
		{
		  char buf[256];
		  AddXChars(buf, sizeof(buf), hstatusstring);
		  OutputMsg(0, "hardstatus string is '%s'", buf);
		  break;
		}
	    }
	  else
	    {
	      OutputMsg(0, "%s: usage: hardstatus [always]lastline|ignore|message|string [string]", rc_name);
	      break;
	    }
	  if (new_use != -1)
	    {
	      hardstatusemu = new_use | (s == args[0] ? 0 : HSTATUS_ALWAYS);
	      for (display = displays; display; display = display->d_next)
		{
		  RemoveStatus();
		  new_use = hardstatusemu & ~HSTATUS_ALWAYS;
		  if (D_HS && s == args[0])
		    new_use = HSTATUS_HS;
		  ShowHStatus((char *)0);
		  old_use = D_has_hstatus;
		  D_has_hstatus = new_use;
		  if ((new_use == HSTATUS_LASTLINE && old_use != HSTATUS_LASTLINE) || (new_use != HSTATUS_LASTLINE && old_use == HSTATUS_LASTLINE))
		    ChangeScreenSize(D_width, D_height, 1);
		  if ((new_use == HSTATUS_FIRSTLINE && old_use != HSTATUS_FIRSTLINE) || (new_use != HSTATUS_FIRSTLINE && old_use == HSTATUS_FIRSTLINE))
		    ChangeScreenSize(D_width, D_height, 1);
		  RefreshHStatus();
		}
	    }
	  if (args[1])
	    {
	      if (hstatusstring)
		free(hstatusstring);
	      hstatusstring = SaveStr(args[1]);
	      for (display = displays; display; display = display->d_next)
	        RefreshHStatus();
	    }
	  display = olddisplay;
	  break;
	}
      (void)ParseSwitch(act, &use_hardstatus);
      if (msgok)
        OutputMsg(0, "messages displayed on %s", use_hardstatus ? "hardstatus line" : "window");
      break;
    case RC_CAPTION:
      if (strcmp(args[0], "always") == 0 || strcmp(args[0], "splitonly") == 0)
	{
	  struct display *olddisplay = display;

	  captionalways = args[0][0] == 'a';
	  for (display = displays; display; display = display->d_next)
	    ChangeScreenSize(D_width, D_height, 1);
	  display = olddisplay;
	}
      else if (strcmp(args[0], "string") == 0)
	{
	  if (!args[1])
	    {
	      char buf[256];
	      AddXChars(buf, sizeof(buf), captionstring);
	      OutputMsg(0, "caption string is '%s'", buf);
	      break;
	    }
	}
      else
	{
	  OutputMsg(0, "%s: usage: caption always|splitonly|string <string>", rc_name);
	  break;
	}
      if (!args[1])
	break;
      if (captionstring)
	free(captionstring);
      captionstring = SaveStr(args[1]);
      RedisplayDisplays(0);
      break;
    case RC_CONSOLE:
      n = (console_window != 0);
      if (ParseSwitch(act, &n))
        break;
      if (TtyGrabConsole(fore->w_ptyfd, n, rc_name))
	break;
      if (n == 0)
	  OutputMsg(0, "%s: releasing console %s", rc_name, HostName);
      else if (console_window)
	  OutputMsg(0, "%s: stealing console %s from window %d (%s)", rc_name, 
	      HostName, console_window->w_number, console_window->w_title);
      else
	  OutputMsg(0, "%s: grabbing console %s", rc_name, HostName);
      console_window = n ? fore : 0;
      break;
    case RC_ALLPARTIAL:
      if (ParseOnOff(act, &all_norefresh))
	break;
      if (!all_norefresh && fore)
	Activate(-1);
      if (msgok)
        OutputMsg(0, all_norefresh ? "No refresh on window change!\n" :
			       "Window specific refresh\n");
      break;
    case RC_PARTIAL:
      (void)ParseSwitch(act, &n);
      fore->w_norefresh = n;
      break;
    case RC_VBELL:
      if (ParseSwitch(act, &visual_bell) || !msgok)
        break;
      if (visual_bell == 0)
        OutputMsg(0, "switched to audible bell.");
      else
        OutputMsg(0, "switched to visual bell.");
      break;
    case RC_VBELLWAIT:
      if (ParseNum1000(act, &VBellWait) == 0 && msgok)
        OutputMsg(0, "vbellwait set to %.10g seconds", VBellWait/1000.);
      break;
    case RC_MSGWAIT:
      if (ParseNum1000(act, &MsgWait) == 0 && msgok)
        OutputMsg(0, "msgwait set to %.10g seconds", MsgWait/1000.);
      break;
    case RC_MSGMINWAIT:
      if (ParseNum1000(act, &MsgMinWait) == 0 && msgok)
        OutputMsg(0, "msgminwait set to %.10g seconds", MsgMinWait/1000.);
      break;
    case RC_SILENCEWAIT:
      if (ParseNum(act, &SilenceWait))
	break;
      if (SilenceWait < 1)
	SilenceWait = 1;
      for (p = windows; p; p = p->w_next)
	p->w_silencewait = SilenceWait;
      if (msgok)
	OutputMsg(0, "silencewait set to %d seconds", SilenceWait);
      break;
    case RC_BUMPRIGHT:
      if (fore->w_number < NextWindow())
        WindowChangeNumber(fore->w_number, NextWindow());
      break;
    case RC_BUMPLEFT:
      if (fore->w_number > PreviousWindow())
        WindowChangeNumber(fore->w_number, PreviousWindow());
      break;
    case RC_COLLAPSE:
      CollapseWindowlist();
      break;
    case RC_NUMBER:
      if (*args == 0)
        OutputMsg(0, queryflag >= 0 ? "%d (%s)" : "This is window %d (%s).", fore->w_number, fore->w_title);
      else
        {
	  int old = fore->w_number;
	  int rel = 0, parse;
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
	    break;
	  if (rel > 0)
	    n += old;
	  else if (rel < 0)
	    n = old - n;
	  if (!WindowChangeNumber(old, n))
	    {
	      /* Window number could not be changed. */
	      queryflag = -1;
	      return;
	    }
	}
      break;

		case RC_ZOMBIE_TIMEOUT:
			if (argc != 1) {
				Msg(0, "Setting zombie polling needs a timeout arg\n");
				break;
			}

			nwin_default.poll_zombie_timeout = atoi(args[0]);
			if (fore)
				fore->w_poll_zombie_timeout = nwin_default.poll_zombie_timeout;
			debug1("Setting zombie polling to %d\n", nwin_default.poll_zombie_timeout);
			break;

		case RC_SORT:
			if (fore) {
			/* Better do not allow this. Not sure what the utmp stuff in number
			* command above is for (you get four entries in e.g. /var/log/wtmp
			* per number switch). But I don't know enough about this.
			*/
				Msg(0, "Sorting inside a window is not allowed. Push CTRL-a \" "
					"and try again\n");
				break;
			}
			/*
			* Simple sort algorithm: Look out for the smallest, put it
			* to the first place, look out for the 2nd smallest, ...
			*/
			for (i = 0; i < maxwin ; i++) {
				if (wtab[i] == NULL)
					continue;
				n = i;

				for (nr = i + 1; nr < maxwin; nr++) {
					if (wtab[nr] == NULL)
						continue;
					debug2("Testing window %d and %d.\n", nr, n);
					if (strcmp(wtab[nr]->w_title,wtab[n]->w_title) < 0)
						n = nr;
				}

				if (n != i) {
					debug2("Exchange window %d and %d.\n", i, n);
					p = wtab[n];
					wtab[n] = wtab[i];
					wtab[i] = p;
					wtab[n]->w_number = n;
					wtab[i]->w_number = i;
#ifdef MULTIUSER
					/* exchange the acls for these windows. */
					AclWinSwap(i, n);
#endif
				}
			}
			WindowChanged((struct win *)0, 0);
			break;

    case RC_SILENCE:
      n = fore->w_silence != 0;
      i = fore->w_silencewait;
      if (args[0] && (args[0][0] == '-' || (args[0][0] >= '0' && args[0][0] <= '9')))
        {
	  if (ParseNum(act, &i))
	    break;
	  n = i > 0;
	}
      else if (ParseSwitch(act, &n))
        break;
      if (n)
        {
#ifdef MULTIUSER
	  if (display)	/* we tell only this user */
	    ACLBYTE(fore->w_lio_notify, D_user->u_id) |= ACLBIT(D_user->u_id);
	  else
	    for (n = 0; n < maxusercount; n++)
	      ACLBYTE(fore->w_lio_notify, n) |= ACLBIT(n);
#endif
	  fore->w_silencewait = i;
	  fore->w_silence = SILENCE_ON;
	  SetTimeout(&fore->w_silenceev, fore->w_silencewait * 1000);
	  evenq(&fore->w_silenceev);

	  if (!msgok)
	    break;
	  OutputMsg(0, "The window is now being monitored for %d sec. silence.", fore->w_silencewait);
	}
      else
        {
#ifdef MULTIUSER
	  if (display) /* we remove only this user */
	    ACLBYTE(fore->w_lio_notify, D_user->u_id) 
	      &= ~ACLBIT(D_user->u_id);
	  else
	    for (n = 0; n < maxusercount; n++)
	      ACLBYTE(fore->w_lio_notify, n) &= ~ACLBIT(n);
	  for (i = maxusercount - 1; i >= 0; i--)
	    if (ACLBYTE(fore->w_lio_notify, i))
	      break;
	  if (i < 0)
#endif
	    {
	      fore->w_silence = SILENCE_OFF;
	      evdeq(&fore->w_silenceev);
	    }
	  if (!msgok)
	    break;
	  OutputMsg(0, "The window is no longer being monitored for silence.");
	}
      break;
#ifdef COPY_PASTE
    case RC_DEFSCROLLBACK:
      (void)ParseNum(act, &nwin_default.histheight);
      break;
    case RC_SCROLLBACK:
      if (flayer->l_layfn == &MarkLf)
	{
	  OutputMsg(0, "Cannot resize scrollback buffer in copy/scrollback mode.");
	  break;
	}
      (void)ParseNum(act, &n);
      ChangeWindowSize(fore, fore->w_width, fore->w_height, n);
      if (msgok)
	OutputMsg(0, "scrollback set to %d", fore->w_histheight);
      break;
#endif
    case RC_SESSIONNAME:
      if (*args == 0)
	OutputMsg(0, "This session is named '%s'\n", SockName);
      else
	{
	  char buf[MAXPATHLEN];

	  s = 0;
	  if (ParseSaveStr(act, &s))
	    break;
	  if (!*s || strlen(s) + (SockName - SockPath) > MAXPATHLEN - 13 || index(s, '/'))
	    {
	      OutputMsg(0, "%s: bad session name '%s'\n", rc_name, s);
	      free(s);
	      break;
	    }
	  strncpy(buf, SockPath, SockName - SockPath);
	  sprintf(buf + (SockName - SockPath), "%d.%s", (int)getpid(), s); 
	  free(s);
	  if ((access(buf, F_OK) == 0) || (errno != ENOENT))
	    {
	      OutputMsg(0, "%s: inappropriate path: '%s'.", rc_name, buf);
	      break;
	    }
	  if (rename(SockPath, buf))
	    {
	      OutputMsg(errno, "%s: failed to rename(%s, %s)", rc_name, SockPath, buf);
	      break;
	    }
	  debug2("rename(%s, %s) done\n", SockPath, buf);
	  strcpy(SockPath, buf);
	  MakeNewEnv();
	  WindowChanged((struct win *)0, 'S');
	}
      break;
    case RC_SETENV:
      if (!args[0] || !args[1])
        {
	  debug1("RC_SETENV arguments missing: %s\n", args[0] ? args[0] : "");
          InputSetenv(args[0]);
	}
      else
        {
          xsetenv(args[0], args[1]);
          MakeNewEnv();
	}
      break;
    case RC_UNSETENV:
      unsetenv(*args);
      MakeNewEnv();
      break;
#ifdef COPY_PASTE
    case RC_DEFSLOWPASTE:
      (void)ParseNum(act, &nwin_default.slow);
      break;
    case RC_SLOWPASTE:
      if (*args == 0)
	OutputMsg(0, fore->w_slowpaste ? 
               "Slowpaste in window %d is %d milliseconds." :
               "Slowpaste in window %d is unset.", 
	    fore->w_number, fore->w_slowpaste);
      else if (ParseNum(act, &fore->w_slowpaste) == 0 && msgok)
	OutputMsg(0, fore->w_slowpaste ?
               "Slowpaste in window %d set to %d milliseconds." :
               "Slowpaste in window %d now unset.", 
	    fore->w_number, fore->w_slowpaste);
      break;
    case RC_MARKKEYS:
      if (CompileKeys(*args, *argl, mark_key_tab))
	{
	  OutputMsg(0, "%s: markkeys: syntax error.", rc_name);
	  break;
	}
      debug1("markkeys %s\n", *args);
      break;
# ifdef FONT
    case RC_PASTEFONT:
      if (ParseSwitch(act, &pastefont) == 0 && msgok)
        OutputMsg(0, "Will %spaste font settings", pastefont ? "" : "not ");
      break;
# endif
    case RC_CRLF:
      (void)ParseSwitch(act, &join_with_cr);
      break;
    case RC_COMPACTHIST:
      if (ParseSwitch(act, &compacthist) == 0 && msgok)
	OutputMsg(0, "%scompacting history lines", compacthist ? "" : "not ");
      break;
#endif
#ifdef NETHACK
    case RC_NETHACK:
      (void)ParseOnOff(act, &nethackflag);
      break;
#else
    case RC_NETHACK:
      Msg(0, "nethack disabled at build time");
      break;
#endif
    case RC_HARDCOPY_APPEND:
      (void)ParseOnOff(act, &hardcopy_append);
      break;
    case RC_VBELL_MSG:
      if (*args == 0) 
        { 
	  char buf[256];
          AddXChars(buf, sizeof(buf), VisualBellString);
	  OutputMsg(0, "vbell_msg is '%s'", buf);
	  break; 
	}
      (void)ParseSaveStr(act, &VisualBellString);
      debug1(" new vbellstr '%s'\n", VisualBellString);
      break;
    case RC_DEFMODE:
      if (ParseBase(act, *args, &n, 8, "octal"))
        break;
      if (n < 0 || n > 0777)
	{
	  OutputMsg(0, "%s: mode: Invalid tty mode %o", rc_name, n);
          break;
	}
      TtyMode = n;
      if (msgok)
	OutputMsg(0, "Ttymode set to %03o", TtyMode);
      break;
    case RC_AUTODETACH:
      (void)ParseOnOff(act, &auto_detach);
      break;
    case RC_STARTUP_MESSAGE:
      (void)ParseOnOff(act, &default_startup);
      break;
#ifdef PASSWORD
    case RC_PASSWORD:
      if (*args)
	{
	  n = (*user->u_password) ? 1 : 0;
	  if (user->u_password != NullStr) free((char *)user->u_password);
	  user->u_password = SaveStr(*args);
	  if (!strcmp(user->u_password, "none"))
	    {
	      if (n)
	        OutputMsg(0, "Password checking disabled");
	      free(user->u_password);
	      user->u_password = NullStr;
	    }
	}
      else
	{
	  if (!fore)
	    {
	      OutputMsg(0, "%s: password: window required", rc_name);
	      break;
	    }
	  Input("New screen password:", 100, INP_NOECHO, pass1, display ? (char *)D_user : (char *)users, 0);
	}
      break;
#endif				/* PASSWORD */
    case RC_BIND:
	{
	  struct action *ktabp = ktab;
	  int kflag = 0;

	  for (;;)
	    {
	      if (argc > 2 && !strcmp(*args, "-c"))
		{
		  ktabp = FindKtab(args[1], 1);
		  if (ktabp == 0)
		    break;
		  args += 2;
		  argl += 2;
		  argc -= 2;
		}
	      else if (argc > 1 && !strcmp(*args, "-k"))
	        {
		  kflag = 1;
		  args++;
		  argl++;
		  argc--;
		}
	      else
	        break;
	    }
#ifdef MAPKEYS
          if (kflag)
	    {
	      for (n = 0; n < KMAP_KEYS; n++)
		if (strcmp(term[n + T_CAPS].tcname, *args) == 0)
		  break;
	      if (n == KMAP_KEYS)
		{
		  OutputMsg(0, "%s: bind: unknown key '%s'", rc_name, *args);
		  break;
		}
	      n += 256;
	    }
	  else
#endif
	  if (*argl != 1)
	    {
	      OutputMsg(0, "%s: bind: character, ^x, or (octal) \\032 expected.", rc_name);
	      break;
	    }
	  else
	    n = (unsigned char)args[0][0];

	  if (args[1])
	    {
	      if ((i = FindCommnr(args[1])) == RC_ILLEGAL)
		{
		  OutputMsg(0, "%s: bind: unknown command '%s'", rc_name, args[1]);
		  break;
		}
	      if (CheckArgNum(i, args + 2) < 0)
		break;
	      ClearAction(&ktabp[n]);
	      SaveAction(ktabp + n, i, args + 2, argl + 2);
	    }
	  else
	    ClearAction(&ktabp[n]);
	}
      break;
#ifdef MAPKEYS
    case RC_BINDKEY:
	{
	  struct action *newact;
          int newnr, fl = 0, kf = 0, af = 0, df = 0, mf = 0;
	  struct display *odisp = display;
	  int used = 0;
          struct kmap_ext *kme = NULL;

	  for (; *args && **args == '-'; args++, argl++)
	    {
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
	      else if (strcmp(*args, "--") == 0)
		{
		  args++;
		  argl++;
		  break;
		}
	      else
		{
	          OutputMsg(0, "%s: bindkey: invalid option %s", rc_name, *args);
		  return;
		}
	    }
	  if (df && mf)
	    {
	      OutputMsg(0, "%s: bindkey: -d does not work with -m", rc_name);
	      break;
	    }
	  if (*args == 0)
	    {
	      if (mf)
		display_bindkey("Edit mode", mmtab);
	      else if (df)
		display_bindkey("Default", dmtab);
	      else
		display_bindkey("User", umtab);
	      break;
	    }
	  if (kf == 0)
	    {
	      if (af)
		{
		  OutputMsg(0, "%s: bindkey: -a only works with -k", rc_name);
		  break;
		}
	      if (*argl == 0)
		{
		  OutputMsg(0, "%s: bindkey: empty string makes no sense", rc_name);
		  break;
		}
	      for (i = 0, kme = kmap_exts; i < kmap_extn; i++, kme++)
		if (kme->str == 0)
		  {
		    if (args[1])
		      break;
		  }
		else
		  if (*argl == (kme->fl & ~KMAP_NOTIMEOUT) && bcmp(kme->str, *args, *argl) == 0)
		      break;
	      if (i == kmap_extn)
		{
		  if (!args[1])
		    {
		      OutputMsg(0, "%s: bindkey: keybinding not found", rc_name);
		      break;
		    }
		  kmap_extn += 8;
		  kmap_exts = (struct kmap_ext *)xrealloc((char *)kmap_exts, kmap_extn * sizeof(*kmap_exts));
		  kme = kmap_exts + i;
		  bzero((char *)kme, 8 * sizeof(*kmap_exts));
		  for (; i < kmap_extn; i++, kme++)
		    {
		      kme->str = 0;
		      kme->dm.nr = kme->mm.nr = kme->um.nr = RC_ILLEGAL;
		      kme->dm.args = kme->mm.args = kme->um.args = noargs;
		      kme->dm.argl = kme->mm.argl = kme->um.argl = 0;
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
	    }
	  else
	    {
	      for (i = T_CAPS; i < T_OCAPS; i++)
		if (strcmp(term[i].tcname, *args) == 0)
		  break;
	      if (i == T_OCAPS)
		{
		  OutputMsg(0, "%s: bindkey: unknown key '%s'", rc_name, *args);
		  break;
		}
	      if (af && i >= T_CURSOR && i < T_OCAPS)
	        i -=  T_CURSOR - KMAP_KEYS;
	      else
	        i -=  T_CAPS;
	      newact = df ? &dmtab[i] : mf ? &mmtab[i] : &umtab[i];
	    }
	  if (args[1])
	    {
	      if ((newnr = FindCommnr(args[1])) == RC_ILLEGAL)
		{
		  OutputMsg(0, "%s: bindkey: unknown command '%s'", rc_name, args[1]);
		  break;
		}
	      if (CheckArgNum(newnr, args + 2) < 0)
		break;
	      ClearAction(newact);
	      SaveAction(newact, newnr, args + 2, argl + 2);
	      if (kf == 0 && args[1])
		{
		  if (kme->str)
		    free(kme->str);
		  kme->str = SaveStrn(*args, *argl);
		  kme->fl = fl | *argl;
		}
	    }
	  else
	    ClearAction(newact);
	  for (display = displays; display; display = display->d_next)
	    remap(i, args[1] ? 1 : 0);
	  if (kf == 0 && !args[1])
	    {
	      if (!used && kme->str)
		{
		  free(kme->str);
		  kme->str = 0;
		  kme->fl = 0;
		}
	    }
	  display = odisp;
	}
      break;
    case RC_MAPTIMEOUT:
      if (*args)
	{
          if (ParseNum(act, &n))
	    break;
	  if (n < 0)
	    {
	      OutputMsg(0, "%s: maptimeout: illegal time %d", rc_name, n);
	      break;
	    }
	  maptimeout = n;
	}
      if (*args == 0 || msgok)
        OutputMsg(0, "maptimeout is %dms", maptimeout);
      break;
    case RC_MAPNOTNEXT:
      D_dontmap = 1;
      break;
    case RC_MAPDEFAULT:
      D_mapdefault = 1;
      break;
#endif
#ifdef MULTIUSER
    case RC_ACLCHG:
    case RC_ACLADD:
    case RC_ADDACL:
    case RC_CHACL:
      UsersAcl(NULL, argc, args);
      break;
    case RC_ACLDEL:
      if (UserDel(args[0], NULL))
	break;
      if (msgok)
	OutputMsg(0, "%s removed from acl database", args[0]);
      break;
    case RC_ACLGRP:
      /*
       * modify a user to gain or lose rights granted to a group.
       * This group is actually a normal user whose rights were defined
       * with chacl in the usual way.
       */
      if (args[1])
        {
	  if (strcmp(args[1], "none"))	/* link a user to another user */
	    {
	      if (AclLinkUser(args[0], args[1]))
		break;
	      if (msgok)
		OutputMsg(0, "User %s joined acl-group %s", args[0], args[1]);
	    }
	  else				/* remove all groups from user */
	    {
	      struct acluser *u;
	      struct aclusergroup *g;

	      if (!(u = *FindUserPtr(args[0])))
	        break;
	      while ((g = u->u_group))
	        {
		  u->u_group = g->next;
	  	  free((char *)g);
	        }
	    }
	}
      else				/* show all groups of user */
	{
	  char buf[256], *p = buf;
	  int ngroups = 0;
	  struct acluser *u;
	  struct aclusergroup *g;

	  if (!(u = *FindUserPtr(args[0])))
	    {
	      if (msgok)
		OutputMsg(0, "User %s does not exist.", args[0]);
	      break;
	    }
	  g = u->u_group;
	  while (g)
	    {
	      ngroups++;
	      sprintf(p, "%s ", g->u->u_name);
	      p += strlen(p);
	      if (p > buf+200)
		break;
	      g = g->next;
	    }
	  if (ngroups)
	    *(--p) = '\0';
	  OutputMsg(0, "%s's group%s: %s.", args[0], (ngroups == 1) ? "" : "s",
	      (ngroups == 0) ? "none" : buf);
	}
      break;
    case RC_ACLUMASK:
    case RC_UMASK:
      while ((s = *args++))
        {
	  char *err = 0;

	  if (AclUmask(display ? D_user : users, s, &err))
	    OutputMsg(0, "umask: %s\n", err);
	}
      break;
    case RC_MULTIUSER:
      if (ParseOnOff(act, &n))
	break;
      multi = n ? "" : 0;
      chsock();
      if (msgok)
	OutputMsg(0, "Multiuser mode %s", multi ? "enabled" : "disabled");
      break;
#endif /* MULTIUSER */
#ifdef PSEUDOS
    case RC_EXEC:
      winexec(args);
      break;
#endif
#ifdef MULTI
    case RC_NONBLOCK:
      i = D_nonblock >= 0;
      if (*args && ((args[0][0] >= '0' && args[0][0] <= '9') || args[0][0] == '.'))
	{
          if (ParseNum1000(act, &i))
	    break;
	}
      else if (!ParseSwitch(act, &i))
	i = i == 0 ? -1 : 1000;
      else
	break;
      if (msgok && i == -1)
        OutputMsg(0, "display set to blocking mode");
      else if (msgok && i == 0)
        OutputMsg(0, "display set to nonblocking mode, no timeout");
      else if (msgok)
        OutputMsg(0, "display set to nonblocking mode, %.10gs timeout", i/1000.);
      D_nonblock = i;
      if (D_nonblock <= 0)
	evdeq(&D_blockedev);
      break;
    case RC_DEFNONBLOCK:
      if (*args && ((args[0][0] >= '0' && args[0][0] <= '9') || args[0][0] == '.'))
	{
          if (ParseNum1000(act, &defnonblock))
	    break;
	}
      else if (!ParseOnOff(act, &defnonblock))
        defnonblock = defnonblock == 0 ? -1 : 1000;
      else
	break;
      if (display && *rc_name)
	{
	  D_nonblock = defnonblock;
          if (D_nonblock <= 0)
	    evdeq(&D_blockedev);
	}
      break;
#endif
    case RC_GR:
#ifdef ENCODINGS
      if (fore->w_gr == 2)
	fore->w_gr = 0;
#endif
      if (ParseSwitch(act, &fore->w_gr) == 0 && msgok)
        OutputMsg(0, "Will %suse GR", fore->w_gr ? "" : "not ");
#ifdef ENCODINGS
      if (fore->w_gr == 0 && fore->w_FontE)
	fore->w_gr = 2;
#endif
      break;
    case RC_C1:
      if (ParseSwitch(act, &fore->w_c1) == 0 && msgok)
        OutputMsg(0, "Will %suse C1", fore->w_c1 ? "" : "not ");
      break;
#ifdef COLOR
    case RC_BCE:
      if (ParseSwitch(act, &fore->w_bce) == 0 && msgok)
        OutputMsg(0, "Will %serase with background color", fore->w_bce ? "" : "not ");
      break;
#endif
#ifdef ENCODINGS
    case RC_KANJI:
    case RC_ENCODING:
#ifdef UTF8
      if (*args && !strcmp(args[0], "-d"))
	{
	  if (!args[1])
	    OutputMsg(0, "encodings directory is %s", screenencodings ? screenencodings : "<unset>");
	  else
	    {
	      free(screenencodings);
	      screenencodings = SaveStr(args[1]);
	    }
	  break;
	}
      if (*args && !strcmp(args[0], "-l"))
	{
	  if (!args[1])
	    OutputMsg(0, "encoding: -l: argument required");
	  else if (LoadFontTranslation(-1, args[1]))
	    OutputMsg(0, "encoding: could not load utf8 encoding file");
	  else if (msgok)
	    OutputMsg(0, "encoding: utf8 encoding file loaded");
	  break;
	}
#else
      if (*args && (!strcmp(args[0], "-l") || !strcmp(args[0], "-d")))
	{
	  if (msgok)
	    OutputMsg(0, "encoding: screen is not compiled for UTF-8.");
	  break;
	}
#endif
      for (i = 0; i < 2; i++)
	{
	  if (args[i] == 0)
	    break;
	  if (!strcmp(args[i], "."))
	    continue;
	  n = FindEncoding(args[i]);
	  if (n == -1)
	    {
	      OutputMsg(0, "encoding: unknown encoding '%s'", args[i]);
	      break;
	    }
	  if (i == 0 && fore)
	    {
	      WinSwitchEncoding(fore, n);
	      ResetCharsets(fore);
	    }
	  else if (i && display)
	    D_encoding  = n;
	}
      break;
    case RC_DEFKANJI:
    case RC_DEFENCODING:
      n = FindEncoding(*args);
      if (n == -1)
	{
	  OutputMsg(0, "defencoding: unknown encoding '%s'", *args);
	  break;
	}
      nwin_default.encoding = n;
      break;
#endif

#ifdef UTF8
    case RC_DEFUTF8:
      n = nwin_default.encoding == UTF8;
      if (ParseSwitch(act, &n) == 0)
	{
	  nwin_default.encoding = n ? UTF8 : 0;
	  if (msgok)
            OutputMsg(0, "Will %suse UTF-8 encoding for new windows", n ? "" : "not ");
	}
      break;
    case RC_UTF8:
      for (i = 0; i < 2; i++)
	{
	  if (i && args[i] == 0)
	    break;
	  if (args[i] == 0)
	    n = fore->w_encoding != UTF8;
	  else if (strcmp(args[i], "off") == 0)
	    n = 0;
	  else if (strcmp(args[i], "on") == 0)
	    n = 1;
	  else
	    {
	      OutputMsg(0, "utf8: illegal argument (%s)", args[i]);
	      break;
	    }
	  if (i == 0)
	    {
	      WinSwitchEncoding(fore, n ? UTF8 : 0);
	      if (msgok)
		OutputMsg(0, "Will %suse UTF-8 encoding", n ? "" : "not ");
	    }
	  else if (display)
	    D_encoding = n ? UTF8 : 0;
	  if (args[i] == 0)
	    break;
	}
      break;
#endif

    case RC_PRINTCMD:
      if (*args)
	{
	  if (printcmd)
	    free(printcmd);
	  printcmd = 0;
	  if (**args)
	    printcmd = SaveStr(*args);
	}
      if (*args == 0 || msgok)
	{
	  if (printcmd)
	    OutputMsg(0, "using '%s' as print command", printcmd);
	  else
	    OutputMsg(0, "using termcap entries for printing");
	    break;
	}
      break;

    case RC_DIGRAPH:
      if (argl && argl[0] > 0 && args[1] && argl[1] > 0)
	{
	  if (argl[0] != 2)
	    {
	      OutputMsg(0, "Two characters expected to define a digraph");
	      break;
	    }
	  i = digraph_find(args[0]);
	  digraphs[i].d[0] = args[0][0];
	  digraphs[i].d[1] = args[0][1];
	  if (!parse_input_int(args[1], argl[1], &digraphs[i].value))
	    {
	      if (!(digraphs[i].value = atoi(args[1])))
		{
		  if (!args[1][1])
		    digraphs[i].value = (int)args[1][0];
#ifdef UTF8
		  else
		    {
		      int t;
		      unsigned char *s = (unsigned char *)args[1];
		      digraphs[i].value = 0;
		      while (*s)
			{
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
#endif
		}
	    }
	  break;
	}
      Input("Enter digraph: ", 10, INP_EVERY, digraph_fn, NULL, 0);
      if (*args && **args)
	{
	  s = *args;
	  n = strlen(s);
	  LayProcess(&s, &n);
	}
      break;

    case RC_DEFHSTATUS:
      if (*args == 0)
	{
	  char buf[256];
          *buf = 0;
	  if (nwin_default.hstatus)
            AddXChars(buf, sizeof(buf), nwin_default.hstatus);
	  OutputMsg(0, "default hstatus is '%s'", buf);
	  break;
        }
      (void)ParseSaveStr(act, &nwin_default.hstatus);
      if (*nwin_default.hstatus == 0)
	{
	  free(nwin_default.hstatus);
	  nwin_default.hstatus = 0;
	}
      break;
    case RC_HSTATUS:
      (void)ParseSaveStr(act, &fore->w_hstatus);
      if (*fore->w_hstatus == 0)
	{
	  free(fore->w_hstatus);
	  fore->w_hstatus = 0;
	}
      WindowChanged(fore, 'h');
      break;

#ifdef FONT
    case RC_DEFCHARSET:
    case RC_CHARSET:
      if (*args == 0)
        {
	  char buf[256];
          *buf = 0;
	  if (nwin_default.charset)
            AddXChars(buf, sizeof(buf), nwin_default.charset);
	  OutputMsg(0, "default charset is '%s'", buf);
	  break;
        }
      n = strlen(*args);
      if (n == 0 || n > 6)
	{
	  OutputMsg(0, "%s: %s: string has illegal size.", rc_name, comms[nr].name);
	  break;
	}
      if (n > 4 && (
        ((args[0][4] < '0' || args[0][4] > '3') && args[0][4] != '.') ||
        ((args[0][5] < '0' || args[0][5] > '3') && args[0][5] && args[0][5] != '.')))
	{
	  OutputMsg(0, "%s: %s: illegal mapping number.", rc_name, comms[nr].name);
	  break;
	}
      if (nr == RC_CHARSET)
	{
	  SetCharsets(fore, *args);
	  break;
	}
      if (nwin_default.charset)
	free(nwin_default.charset);
      nwin_default.charset = SaveStr(*args);
      break;
#endif
#ifdef COLOR
    case RC_ATTRCOLOR:
      s = args[0];
      if (*s >= '0' && *s <= '9')
        i = *s - '0';
      else
	for (i = 0; i < 8; i++)
	  if (*s == "dubrsBiI"[i])
	    break;
      s++;
      nr = 0;
      if (*s && s[1] && !s[2])
	{
	  if (*s == 'd' && s[1] == 'd')
	    nr = 3;
	  else if (*s == '.' && s[1] == 'd')
	    nr = 2;
	  else if (*s == 'd' && s[1] == '.')
	    nr = 1;
	  else if (*s != '.' || s[1] != '.')
	    s--;
	  s += 2;
	}
      if (*s || i < 0 || i >= 8)
	{
	  OutputMsg(0, "%s: attrcolor: unknown attribute '%s'.", rc_name, args[0]);
	  break;
	}
      n = 0;
      if (args[1])
        n = ParseAttrColor(args[1], args[2], 1);
      if (n == -1)
	break;
      attr2color[i][nr] = n;
      n = 0;
      for (i = 0; i < 8; i++)
	if (attr2color[i][0] || attr2color[i][1] || attr2color[i][2] || attr2color[i][3])
	  n |= 1 << i;
      nattr2color = n;
      break;
#endif
    case RC_RENDITION:
      i = -1;
      if (strcmp(args[0], "bell") == 0)
	{
	  i = REND_BELL;
	}
      else if (strcmp(args[0], "monitor") == 0)
	{
	  i = REND_MONITOR;
	}
      else if (strcmp(args[0], "silence") == 0)
	{
	  i = REND_SILENCE;
	}
      else if (strcmp(args[0], "so") != 0)
	{
	  OutputMsg(0, "Invalid option '%s' for rendition", args[0]);
	  break;
	}

      ++args;
      ++argl;

      if (i != -1)
	{
	  renditions[i] = ParseAttrColor(args[0], args[1], 1);
	  WindowChanged((struct win *)0, 'w');
	  WindowChanged((struct win *)0, 'W');
	  WindowChanged((struct win *)0, 0);
	  break;
	}

      /* We are here, means we want to set the sorendition. */
      /* FALLTHROUGH*/
    case RC_SORENDITION:
      i = 0;
      if (*args)
	{
          i = ParseAttrColor(*args, args[1], 1);
	  if (i == -1)
	    break;
	  ApplyAttrColor(i, &mchar_so);
	  WindowChanged((struct win *)0, 0);
	  debug2("--> %x %x\n", mchar_so.attr, mchar_so.color);
	}
      if (msgok)
#ifdef COLOR
        OutputMsg(0, "Standout attributes 0x%02x  color 0x%02x", (unsigned char)mchar_so.attr, 0x99 ^ (unsigned char)mchar_so.color);
#else
        OutputMsg(0, "Standout attributes 0x%02x ", (unsigned char)mchar_so.attr);
#endif
      break;

      case RC_SOURCE:
	do_source(*args);
	break;

#ifdef MULTIUSER
    case RC_SU:
      s = NULL;
      if (!*args)
        {
	  OutputMsg(0, "%s:%s screen login", HostName, SockPath);
          InputSu(D_fore, &D_user, NULL);
	}
      else if (!args[1])
        InputSu(D_fore, &D_user, args[0]);
      else if (!args[2])
        s = DoSu(&D_user, args[0], args[1], "\377");
      else
        s = DoSu(&D_user, args[0], args[1], args[2]);
      if (s)
        OutputMsg(0, "%s", s);
      break;
#endif /* MULTIUSER */
    case RC_SPLIT:
      s = args[0];
      if (s && !strcmp(s, "-v"))
        AddCanvas(SLICE_HORI);
      else
        AddCanvas(SLICE_VERT);
      Activate(-1);
      break;
    case RC_REMOVE:
      RemCanvas();
      Activate(-1);
      break;
    case RC_ONLY:
      OneCanvas();
      Activate(-1);
      break;
    case RC_FIT:
      D_forecv->c_xoff = D_forecv->c_xs;
      D_forecv->c_yoff = D_forecv->c_ys;
      RethinkViewportOffsets(D_forecv);
      ResizeLayer(D_forecv->c_layer, D_forecv->c_xe - D_forecv->c_xs + 1, D_forecv->c_ye - D_forecv->c_ys + 1, 0);
      flayer = D_forecv->c_layer;
      LaySetCursor();
      break;
    case RC_FOCUS:
      {
	struct canvas *cv = 0;
	if (!*args || !strcmp(*args, "next"))
	  cv = D_forecv->c_next ? D_forecv->c_next : D_cvlist;
	else if (!strcmp(*args, "prev"))
	  {
	    for (cv = D_cvlist; cv->c_next && cv->c_next != D_forecv; cv = cv->c_next)
	      ;
	  }
	else if (!strcmp(*args, "top"))
	  cv = D_cvlist;
	else if (!strcmp(*args, "bottom"))
	  {
	    for (cv = D_cvlist; cv->c_next; cv = cv->c_next)
	      ;
	  }
	else if (!strcmp(*args, "up"))
	  cv = FindCanvas(D_forecv->c_xs, D_forecv->c_ys - 1);
	else if (!strcmp(*args, "down"))
	  cv = FindCanvas(D_forecv->c_xs, D_forecv->c_ye + 2);
	else if (!strcmp(*args, "left"))
	  cv = FindCanvas(D_forecv->c_xs - 1, D_forecv->c_ys);
	else if (!strcmp(*args, "right"))
	  cv = FindCanvas(D_forecv->c_xe + 1, D_forecv->c_ys);
	else
	  {
	    OutputMsg(0, "%s: usage: focus [next|prev|up|down|left|right|top|bottom]", rc_name);
	    break;
	  }
	SetForeCanvas(display, cv);
      }
      break;
    case RC_RESIZE:
      i = 0;
      if (D_forecv->c_slorient == SLICE_UNKN)
	{
	  OutputMsg(0, "resize: need more than one region");
	  break;
	}
      for (; *args; args++)
	{
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
      if (*args && args[1])
	{
	  OutputMsg(0, "%s: usage: resize [-h] [-v] [-l] [num]\n", rc_name);
	  break;
	}
      if (*args)
	ResizeRegions(*args, i);
      else
	Input(resizeprompts[i], 20, INP_EVERY, ResizeFin, (char*)0, i);
      break;
    case RC_SETSID:
      (void)ParseSwitch(act, &separate_sids);
      break;
    case RC_EVAL:
      args = SaveArgs(args);
      for (i = 0; args[i]; i++)
	{
	  if (args[i][0])
	    Colonfin(args[i], strlen(args[i]), (char *)0);
	  free(args[i]);
	}
      free(args);
      break;
    case RC_ALTSCREEN:
      (void)ParseSwitch(act, &use_altscreen);
      if (msgok)
        OutputMsg(0, "Will %sdo alternate screen switching", use_altscreen ? "" : "not ");
      break;
    case RC_MAXWIN:
      if (!args[0])
	{
	  OutputMsg(0, "maximum windows allowed: %d", maxwin);
	  break;
	}
      if (ParseNum(act, &n))
	break;
      if (n < 1)
        OutputMsg(0, "illegal maxwin number specified");
      else if (n > 2048)
	OutputMsg(0, "maximum 2048 windows allowed");
      else if (n > maxwin && windows)
	OutputMsg(0, "may increase maxwin only when there's no window");
      else
	{
	  if (!windows)
            {
	      wtab = realloc(wtab, n * sizeof(struct win *));
              bzero(wtab, n * sizeof(struct win *));
            }
	  maxwin = n;
	}
      break;
    case RC_BACKTICK:
      if (ParseBase(act, *args, &n, 10, "decimal"))
	break;
      if (!args[1])
        setbacktick(n, 0, 0, (char **)0);
      else
	{
	  int lifespan, tick;
	  if (argc < 4)
	    {
	      OutputMsg(0, "%s: usage: backtick num [lifespan tick cmd args...]", rc_name);
	      break;
	    }
	  if (ParseBase(act, args[1], &lifespan, 10, "decimal"))
	    break;
	  if (ParseBase(act, args[2], &tick, 10, "decimal"))
	    break;
	  setbacktick(n, lifespan, tick, SaveArgs(args + 3));
	}
      WindowChanged(0, '`');
      break;
    case RC_BLANKER:
#ifdef BLANKER_PRG
      if (blankerprg)
	{
          RunBlanker(blankerprg);
	  break;
	}
#endif
      ClearAll();
      CursorVisibility(-1);
      D_blocked = 4;
      break;
#ifdef BLANKER_PRG
    case RC_BLANKERPRG:
      if (!args[0])
	{
	  if (blankerprg)
	    {
	      char path[MAXPATHLEN];
	      char *p = path, **pp;
	      for (pp = blankerprg; *pp; pp++)
		p += snprintf(p, sizeof(path) - (p - path) - 1, "%s ", *pp);
	      *(p - 1) = '\0';
	      OutputMsg(0, "blankerprg: %s", path);
	    }
	  else
	    OutputMsg(0, "No blankerprg set.");
	  break;
	}
      if (blankerprg)
	{
	  char **pp;
	  for (pp = blankerprg; *pp; pp++)
	    free(*pp);
	  free(blankerprg);
	  blankerprg = 0;
	}
      if (args[0][0])
	blankerprg = SaveArgs(args);
      break;
#endif
    case RC_IDLE:
      if (*args)
	{
	  struct display *olddisplay = display;
	  if (!strcmp(*args, "off"))
	    idletimo = 0;
	  else if (args[0][0])
	    idletimo = atoi(*args) * 1000;
	  if (argc > 1)
	    {
	      if ((i = FindCommnr(args[1])) == RC_ILLEGAL)
		{
		  OutputMsg(0, "%s: idle: unknown command '%s'", rc_name, args[1]);
		  break;
		}
	      if (CheckArgNum(i, args + 2) < 0)
		break;
	      ClearAction(&idleaction);
	      SaveAction(&idleaction, i, args + 2, argl + 2);
	    }
	  for (display = displays; display; display = display->d_next)
	    ResetIdle();
	  display = olddisplay;
	}
      if (msgok)
	{
	  if (idletimo)
	    OutputMsg(0, "idle timeout %ds, %s", idletimo / 1000, comms[idleaction.nr].name);
	  else
	    OutputMsg(0, "idle off");
	}
      break;
    case RC_FOCUSMINSIZE:
      for (i = 0; i < 2 && args[i]; i++)
	{
	  if (!strcmp(args[i], "max") || !strcmp(args[i], "_"))
	    n = -1;
	  else
	    n = atoi(args[i]);
	  if (i == 0)
	    focusminwidth = n;
	  else
            focusminheight = n;
	}
      if (msgok)
	{
	  char b[2][20];
	  for (i = 0; i < 2; i++)
	    {
	      n = i == 0 ? focusminwidth : focusminheight;
	      if (n == -1)
		strcpy(b[i], "max");
	      else
		sprintf(b[i], "%d", n);
	    }
          OutputMsg(0, "focus min size is %s %s\n", b[0], b[1]);
	}
      break;
    case RC_GROUP:
      if (*args)
	{
	  fore->w_group = 0;
	  if (args[0][0])
	    {
	      fore->w_group = WindowByName(*args);
	      if (fore->w_group == fore || (fore->w_group && fore->w_group->w_type != W_TYPE_GROUP))
		fore->w_group = 0;
	    }
	  WindowChanged((struct win *)0, 'w');
	  WindowChanged((struct win *)0, 'W');
	  WindowChanged((struct win *)0, 0);
	}
      if (msgok)
	{
	  if (fore->w_group)
	    OutputMsg(0, "window group is %d (%s)\n", fore->w_group->w_number, fore->w_group->w_title);
	  else
	    OutputMsg(0, "window belongs to no group");
	}
      break;
    case RC_LAYOUT:
      // A number of the subcommands for "layout" are ignored, or not processed correctly when there
      // is no attached display.

      if (!strcmp(args[0], "title"))
	{
          if (!display)
            {
	      if (!args[1])  // There is no display, and there is no new title. Ignore.
		break;
	      if (!layout_attach || layout_attach == &layout_last_marker)
		layout_attach = CreateLayout(args[1], 0);
	      else
		RenameLayout(layout_attach, args[1]);
	      break;
	    }

	  if (!D_layout)
	    {
	      OutputMsg(0, "not on a layout");
	      break;
	    }
	  if (!args[1])
	    {
	      OutputMsg(0, "current layout is %d (%s)", D_layout->lay_number, D_layout->lay_title);
	      break;
	    }
	  RenameLayout(D_layout, args[1]);
	}
      else if (!strcmp(args[0], "number"))
	{
	  if (!display)
	    {
	      if (args[1] && layout_attach && layout_attach != &layout_last_marker)
		RenumberLayout(layout_attach, atoi(args[1]));
	      break;
	    }

	  if (!D_layout)
	    {
	      OutputMsg(0, "not on a layout");
	      break;
	    }
	  if (!args[1])
	    {
	      OutputMsg(0, "This is layout %d (%s).\n", D_layout->lay_number, D_layout->lay_title);
	      break;
	    }
	   RenumberLayout(D_layout, atoi(args[1]));
	   break;
	}
      else if (!strcmp(args[0], "autosave"))
	{
	  if (!display)
	    {
	      if (args[1] && layout_attach && layout_attach != &layout_last_marker)
		{
		  if (!strcmp(args[1], "on"))
		    layout_attach->lay_autosave = 1;
		  else if (!strcmp(args[1], "off"))
		    layout_attach->lay_autosave = 0;
		}
	      break;
	    }

	  if (!D_layout)
	    {
	      OutputMsg(0, "not on a layout");
	      break;
	    }
	  if (args[1])
	    {
	      if (!strcmp(args[1], "on"))
		D_layout->lay_autosave = 1;
	      else if (!strcmp(args[1], "off"))
		D_layout->lay_autosave = 0;
	      else
		{
		  OutputMsg(0, "invalid argument. Give 'on' or 'off");
		  break;
		}
	    }
	  if (msgok)
	    OutputMsg(0, "autosave is %s", D_layout->lay_autosave ? "on" : "off");
	}
      else if (!strcmp(args[0], "new"))
	{
	  char *t = args[1];
	  n = 0;
	  if (t)
	    {
	      while (*t >= '0' && *t <= '9')
		t++;
	      if (t != args[1] && (!*t || *t == ':'))
		{
		  n = atoi(args[1]);
		  if (*t)
		    t++;
		}
	      else
		t = args[1];
	    }
	  if (!t || !*t)
	    t = "layout";
          NewLayout(t, n);
	  Activate(-1);
	}
      else if (!strcmp(args[0], "save"))
	{
	  if (!args[1])
	    {
	      OutputMsg(0, "usage: layout save <name>");
	      break;
	    }
	  if (display)
	    SaveLayout(args[1], &D_canvas);
	}
      else if (!strcmp(args[0], "select"))
	{
	  if (!display)
	    {
	      if (args[1])
		layout_attach = FindLayout(args[1]);
	      break;
	    }
          if (!args[1])
	    {
	      Input("Switch to layout: ", 20, INP_COOKED, SelectLayoutFin, NULL, 0);
	      break;
	    }
	  SelectLayoutFin(args[1], strlen(args[1]), (char *)0);
	}
      else if (!strcmp(args[0], "next"))
	{
	  if (!display)
	    {
	      if (layout_attach && layout_attach != &layout_last_marker)
		layout_attach = layout_attach->lay_next ? layout_attach->lay_next : layouts;;
	      break;
	    }
	  struct layout *lay = D_layout;
	  if (lay)
	    lay = lay->lay_next ? lay->lay_next : layouts;
	  else
	    lay = layouts;
	  if (!lay)
	    {
	      OutputMsg(0, "no layout defined");
	      break;
	    }
	  if (lay == D_layout)
	    break;
	  LoadLayout(lay, &D_canvas);
	  Activate(-1);
	}
      else if (!strcmp(args[0], "prev"))
	{
	  struct layout *lay = display ? D_layout : layout_attach;
	  struct layout *target = lay;
	  if (lay)
	    {
	      for (lay = layouts; lay->lay_next && lay->lay_next != target; lay = lay->lay_next)
		;
	    }
	  else
	    lay = layouts;

	  if (!display)
	    {
	      layout_attach = lay;
	      break;
	    }

	  if (!lay)
	    {
	      OutputMsg(0, "no layout defined");
	      break;
	    }
	  if (lay == D_layout)
	    break;
	  LoadLayout(lay, &D_canvas);
	  Activate(-1);
	}
      else if (!strcmp(args[0], "attach"))
	{
	  if (!args[1])
	    {
	      if (!layout_attach)
	        OutputMsg(0, "no attach layout set");
	      else if (layout_attach == &layout_last_marker)
	        OutputMsg(0, "will attach to last layout");
	      else
	        OutputMsg(0, "will attach to layout %d (%s)", layout_attach->lay_number, layout_attach->lay_title);
	      break;
	    }
	  if (!strcmp(args[1], ":last"))
	    layout_attach = &layout_last_marker;
	  else if (!args[1][0])
	    layout_attach = 0;
	  else
	    {
	      struct layout *lay;
	      lay = FindLayout(args[1]);
	      if (!lay)
		{
		  OutputMsg(0, "unknown layout '%s'", args[1]);
		  break;
		}
	      layout_attach = lay;
	    }
	}
      else if (!strcmp(args[0], "show"))
	{
	  ShowLayouts(-1);
	}
      else if (!strcmp(args[0], "remove"))
	{
	  struct layout *lay = display ? D_layout : layouts;
	  if (args[1])
	    {
	      lay = layouts ? FindLayout(args[1]) : (struct layout *)0;
	      if (!lay)
		{
		  OutputMsg(0, "unknown layout '%s'", args[1]);
		  break;
		}
	    }
	  if (lay)
	    RemoveLayout(lay);
	}
      else if (!strcmp(args[0], "dump"))
	{
	  if (!display)
	    OutputMsg(0, "Must have a display for 'layout dump'.");
	  else if (!LayoutDumpCanvas(&D_canvas, args[1] ? args[1] : "layout-dump"))
	    OutputMsg(errno, "Error dumping layout.");
	  else
	    OutputMsg(0, "Layout dumped to \"%s\"", args[1] ? args[1] : "layout-dump");
	}
      else
	OutputMsg(0, "unknown layout subcommand");
      break;
#ifdef DW_CHARS
    case RC_CJKWIDTH:
      if(ParseSwitch(act, &cjkwidth) == 0)
      {
        if(msgok)
          OutputMsg(0, "Treat ambiguous width characters as %s width", cjkwidth ? "full" : "half");
      }
      break;
#endif
    default:
#ifdef HAVE_BRAILLE
      /* key == -2: input from braille keybord, msgok always 0 */
      DoBrailleAction(act, key == -2 ? 0 : msgok);
#endif
      break;
    }
  if (display != odisplay)
    {
      for (display = displays; display; display = display->d_next)
        if (display == odisplay)
	  break;
    }
}
#undef OutputMsg

void
CollapseWindowlist()
/* renumber windows from 0, leaving no gaps */
{
  int pos, moveto=0;

  for (pos = 1; pos < MAXWIN; pos++)
    if (wtab[pos])
      for (; moveto < pos; moveto++)
        if (!wtab[moveto])
          {
          WindowChangeNumber(pos, moveto);
          break;
          }
}

void
DoCommand(argv, argl) 
char **argv;
int *argl;
{
  struct action act;
  const char *cmd = *argv;

  act.quiet = 0;
  /* For now, we actually treat both 'supress error' and 'suppress normal message' as the
   * same, and ignore all messages on either flag. If we wanted to do otherwise, we would
   * need to change the definition of 'OutputMsg' slightly. */
  if (*cmd == '@')	/* Suppress error */
    {
      act.quiet |= 0x01;
      cmd++;
    }
  if (*cmd == '-')	/* Suppress normal message */
    {
      act.quiet |= 0x02;
      cmd++;
    }

  if ((act.nr = FindCommnr(cmd)) == RC_ILLEGAL)
    {
      Msg(0, "%s: unknown command '%s'", rc_name, cmd);
      return;
    }
  act.args = argv + 1;
  act.argl = argl + 1;
  DoAction(&act, -1);
}

static void
SaveAction(act, nr, args, argl)
struct action *act;
int nr;
char **args;
int *argl;
{
  register int argc = 0;
  char **pp;
  int *lp;

  if (args)
    while (args[argc])
      argc++;
  if (argc == 0)
    {
      act->nr = nr;
      act->args = noargs;
      act->argl = 0;
      return;
    }
  if ((pp = (char **)malloc((unsigned)(argc + 1) * sizeof(char *))) == 0)
    Panic(0, "%s", strnomem);
  if ((lp = (int *)malloc((unsigned)(argc) * sizeof(int))) == 0)
    Panic(0, "%s", strnomem);
  act->nr = nr;
  act->args = pp;
  act->argl = lp;
  while (argc--)
    {
      *lp = argl ? *argl++ : (int)strlen(*args);
      *pp++ = SaveStrn(*args++, *lp++);
    }
  *pp = 0;
}

static char **
SaveArgs(args)
char **args;
{
  register char **ap, **pp;
  register int argc = 0;

  while (args[argc])
    argc++;
  if ((pp = ap = (char **)malloc((unsigned)(argc + 1) * sizeof(char **))) == 0)
    Panic(0, "%s", strnomem);
  while (argc--)
    *pp++ = SaveStr(*args++);
  *pp = 0;
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
int 
Parse(buf, bufl, args, argl)
char *buf, **args;
int bufl, *argl;
{
  register char *p = buf, **ap = args, *pp;
  register int delim, argc;
  int *lp = argl;

  debug2("Parse %d %s\n", bufl, buf);
  argc = 0;
  pp = buf;
  delim = 0;
  for (;;)
    {
      *lp = 0;
      while (*p && (*p == ' ' || *p == '\t'))
	++p;
#ifdef PSEUDOS
      if (argc == 0 && *p == '!')
	{
	  *ap++ = "exec";
	  *lp++ = 4;
	  p++;
	  argc++;
	  continue;
        }
#endif
      if (*p == '\0' || *p == '#' || *p == '\n')
	{
	  *p = '\0';
	  for (delim = 0; delim < argc; delim++)
	    debug1("-- %s\n", args[delim]);
	  args[argc] = 0;
	  return argc;
	}
      if (++argc >= MAXARGS)
	{
	  Msg(0, "%s: too many tokens.", rc_name);
	  return 0;
	}
      *ap++ = pp;

      debug1("- new arg %s\n", p);
      while (*p)
	{
	  if (*p == delim)
	    delim = 0;
	  else if (delim != '\'' && *p == '\\' && (p[1] == 'n' || p[1] == 'r' || p[1] == 't' || p[1] == '\'' || p[1] == '"' || p[1] == '\\' || p[1] == '$' || p[1] == '#' || p[1] == '^' || (p[1] >= '0' && p[1] <= '7')))
	    {
	      p++;
	      if (*p >= '0' && *p <= '7')
		{
		  *pp = *p - '0';
		  if (p[1] >= '0' && p[1] <= '7')
		    {
		      p++;
		      *pp = (*pp << 3) | (*p - '0');
		      if (p[1] >= '0' && p[1] <= '7')
			{
			  p++;
			  *pp = (*pp << 3) | (*p - '0');
			}
		    }
		  pp++;
		}
	      else
		{
		  switch (*p)
		    {
		      case 'n': *pp = '\n'; break;
		      case 'r': *pp = '\r'; break;
		      case 't': *pp = '\t'; break;
		      default: *pp = *p; break;
		    }
		  pp++;
		}
	    }
	  else if (delim != '\'' && *p == '$' && (p[1] == '{' || p[1] == ':' || (p[1] >= 'a' && p[1] <= 'z') || (p[1] >= 'A' && p[1] <= 'Z') || (p[1] >= '0' && p[1] <= '9') || p[1] == '_'))

	    {
	      char *ps, *pe, op, *v, xbuf[11], path[MAXPATHLEN];
	      int vl;

	      ps = ++p;
	      debug1("- var %s\n", ps);
	      p++;
	      while (*p)
		{
		  if (*ps == '{' && *p == '}')
		    break;
		  if (*ps == ':' && *p == ':')
		    break;
		  if (*ps != '{' && *ps != ':' && (*p < 'a' || *p > 'z') && (*p < 'A' || *p > 'Z') && (*p < '0' || *p > '9') && *p != '_')
		    break;
		  p++;
		}
	      pe = p;
	      if (*ps == '{' || *ps == ':')
		{
		  if (!*p)
		    {
		      Msg(0, "%s: bad variable name.", rc_name);
		      return 0;
		    }
		  p++;
		}
	      op = *pe;
	      *pe = 0;
	      debug1("- var is '%s'\n", ps);
	      if (*ps == ':')
		v = gettermcapstring(ps + 1);
	      else
		{
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
		  else if (!strcmp(ps, "PWD"))
		    {
		      if (getcwd(path, sizeof(path) - 1) == 0)
			v = "?";
		      else
			v = path;
		    }
		  else if (!strcmp(ps, "STY"))
		    {
		      if ((v = strchr(SockName, '.')))	/* Skip the PID */
			v++;
		      else
			v = SockName;
		    }
		  else
		    v = getenv(ps);
		}
	      *pe = op;
	      vl = v ? strlen(v) : 0;
	      if (vl)
		{
		  debug1("- sub is '%s'\n", v);
		  if (p - pp < vl)
		    {
		      int right = buf + bufl - (p + strlen(p) + 1);
		      if (right > 0)
			{
			  bcopy(p, p + right, strlen(p) + 1);
			  p += right;
			}
		    }
		  if (p - pp < vl)
		    {
		      Msg(0, "%s: no space left for variable expansion.", rc_name);
		      return 0;
		    }
		  bcopy(v, pp, vl);
		  pp += vl;
		}
	      continue;
	    }
	  else if (delim != '\'' && *p == '^' && p[1])
	    {
	      p++;
	      *pp++ = *p == '?' ? '\177' : *p & 0x1f;
	    }
	  else if (delim == 0 && (*p == '\'' || *p == '"'))
	    delim = *p;
	  else if (delim == 0 && (*p == ' ' || *p == '\t' || *p == '\n'))
	    break;
	  else
	    *pp++ = *p;
	  p++;
	}
      if (delim)
	{
	  Msg(0, "%s: Missing %c quote.", rc_name, delim);
	  return 0;
	}
      if (*p)
	p++;
      *pp = 0;
      debug2("- arg done, '%s' rest %s\n", ap[-1], p);
      *lp++ = pp - ap[-1];
      pp++;
    }
}

void
SetEscape(u, e, me)
struct acluser *u;
int e, me;
{
  if (u)
    {
      u->u_Esc = e;
      u->u_MetaEsc = me;
    }
  else
    {
      if (users)
	{
	  if (DefaultEsc >= 0)
	    ClearAction(&ktab[DefaultEsc]);
	  if (DefaultMetaEsc >= 0)
	    ClearAction(&ktab[DefaultMetaEsc]);
	}
      DefaultEsc = e;
      DefaultMetaEsc = me;
      if (users)
	{
	  if (DefaultEsc >= 0)
	    {
	      ClearAction(&ktab[DefaultEsc]);
	      ktab[DefaultEsc].nr = RC_OTHER;
	    }
	  if (DefaultMetaEsc >= 0)
	    {
	      ClearAction(&ktab[DefaultMetaEsc]);
	      ktab[DefaultMetaEsc].nr = RC_META;
	    }
	}
    }
}

int
ParseSwitch(act, var)
struct action *act;
int *var;
{
  if (*act->args == 0)
    {
      *var ^= 1;
      return 0;
    }
  return ParseOnOff(act, var);
}

static int
ParseOnOff(act, var)
struct action *act;
int *var;
{
  register int num = -1;
  char **args = act->args;

  if (args[1] == 0)
    {
      if (strcmp(args[0], "on") == 0)
	num = 1;
      else if (strcmp(args[0], "off") == 0)
	num = 0;
    }
  if (num < 0)
    {
      Msg(0, "%s: %s: invalid argument. Give 'on' or 'off'", rc_name, comms[act->nr].name);
      return -1;
    }
  *var = num;
  return 0;
}

int
ParseSaveStr(act, var)
struct action *act;
char **var;
{
  char **args = act->args;
  if (*args == 0 || args[1])
    {
      Msg(0, "%s: %s: one argument required.", rc_name, comms[act->nr].name);
      return -1;
    }
  if (*var)
    free(*var);
  *var = SaveStr(*args);
  return 0;
}

int
ParseNum(act, var)
struct action *act;
int *var;
{
  int i;
  char *p, **args = act->args;

  p = *args;
  if (p == 0 || *p == 0 || args[1])
    {
      Msg(0, "%s: %s: invalid argument. Give one argument.",
          rc_name, comms[act->nr].name);
      return -1;
    }
  i = 0; 
  while (*p)
    {
      if (*p >= '0' && *p <= '9')
	i = 10 * i + (*p - '0');
      else
	{
	  Msg(0, "%s: %s: invalid argument. Give numeric argument.",
	      rc_name, comms[act->nr].name);
	  return -1;
	}    
      p++;
    }
  debug1("ParseNum got %d\n", i);
  *var = i;
  return 0;
}

static int
ParseNum1000(act, var)
struct action *act;
int *var;
{
  int i;
  char *p, **args = act->args;
  int dig = 0;

  p = *args;
  if (p == 0 || *p == 0 || args[1])
    {
      Msg(0, "%s: %s: invalid argument. Give one argument.",
          rc_name, comms[act->nr].name);
      return -1;
    }
  i = 0; 
  while (*p)
    {
      if (*p >= '0' && *p <= '9')
	{
	  if (dig < 4)
	    i = 10 * i + (*p - '0');
          else if (dig == 4 && *p >= '5')
	    i++;
	  if (dig)
	    dig++;
	}
      else if (*p == '.' && !dig)
        dig++;
      else
	{
	  Msg(0, "%s: %s: invalid argument. Give floating point argument.",
	      rc_name, comms[act->nr].name);
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
  debug1("ParseNum1000 got %d\n", i);
  *var = i;
  return 0;
}

static struct win *
WindowByName(s)
char *s;
{
  struct win *p;

  for (p = windows; p; p = p->w_next)
    if (!strcmp(p->w_title, s))
      return p;
  for (p = windows; p; p = p->w_next)
    if (!strncmp(p->w_title, s, strlen(s)))
      return p;
  return 0;
}

static int
WindowByNumber(str)
char *str;
{
  int i;
  char *s;

  for (i = 0, s = str; *s; s++)
    {
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
int
WindowByNoN(str)
char *str;
{
  int i;
  struct win *p;
  
  if ((i = WindowByNumber(str)) < 0 || i >= maxwin)
    {
      if ((p = WindowByName(str)))
	return p->w_number;
      return -1;
    }
  return i;
}

static int
ParseWinNum(act, var)
struct action *act;
int *var;
{
  char **args = act->args;
  int i = 0;

  if (*args == 0 || args[1])
    {
      Msg(0, "%s: %s: one argument required.", rc_name, comms[act->nr].name);
      return -1;
    }
  
  i = WindowByNoN(*args);
  if (i < 0)
    {
      Msg(0, "%s: %s: invalid argument. Give window number or name.",
          rc_name, comms[act->nr].name);
      return -1;
    }
  debug1("ParseWinNum got %d\n", i);
  *var = i;
  return 0;
}

static int
ParseBase(act, p, var, base, bname)
struct action *act;
char *p;
int *var;
int base;
char *bname;
{
  int i = 0;
  int c;

  if (*p == 0)
    {
      Msg(0, "%s: %s: empty argument.", rc_name, comms[act->nr].name);
      return -1;
    }
  while ((c = *p++))
    {
      if (c >= 'a' && c <= 'z')
	c -= 'a' - 'A';
      if (c >= 'A' && c <= 'Z')
	c -= 'A' - ('0' + 10);
      c -= '0';
      if (c < 0 || c >= base)
	{
	  Msg(0, "%s: %s: argument is not %s.", rc_name, comms[act->nr].name, bname);
	  return -1;
	}    
      i = base * i + c;
    }
  debug1("ParseBase got %d\n", i);
  *var = i;
  return 0;
}

static int
IsNum(s, base)
register char *s;
register int base;
{
  for (base += '0'; *s; ++s)
    if (*s < '0' || *s > base)
      return 0;
  return 1;
}

int
IsNumColon(s, base, p, psize)
int base, psize;
char *s, *p;
{
  char *q;
  if ((q = rindex(s, ':')) != 0)
    {
      strncpy(p, q + 1, psize - 1);
      p[psize - 1] = '\0';
      *q = '\0';
    }
  else
    *p = '\0';
  return IsNum(s, base);
}

void
SwitchWindow(n)
int n;
{
  struct win *p;

  debug1("SwitchWindow %d\n", n);
  if (n < 0 || n >= maxwin)
    {
      ShowWindows(-1);
      return;
    }
  if ((p = wtab[n]) == 0)
    {
      ShowWindows(n);
      return;
    }
  if (display == 0)
    {
      fore = p;
      return;
    }
  if (p == D_fore)
    {
      Msg(0, "This IS window %d (%s).", n, p->w_title);
      return;
    }
#ifdef MULTIUSER
  if (AclCheckPermWin(D_user, ACL_READ, p))
    {
      Msg(0, "Access to window %d denied.", p->w_number);
      return;
    }
#endif
  SetForeWindow(p);
  Activate(fore->w_norefresh);  
}

/*
 * SetForeWindow changes the window in the input focus of the display.
 * Puts window wi in canvas display->d_forecv.
 */
void
SetForeWindow(wi)
struct win *wi;
{
  struct win *p;
  if (display == 0)
    {
      fore = wi;
      return;
    }
  p = Layer2Window(D_forecv->c_layer);
  SetCanvasWindow(D_forecv, wi);
  if (p)
    WindowChanged(p, 'u');
  if (wi)
    WindowChanged(wi, 'u');
  flayer = D_forecv->c_layer;
  /* Activate called afterwards, so no RefreshHStatus needed */
}


/*****************************************************************/

/* 
 *  Activate - make fore window active
 *  norefresh = -1 forces a refresh, disregard all_norefresh then.
 */
void
Activate(norefresh)
int norefresh;
{
  debug1("Activate(%d)\n", norefresh);
  if (display == 0)
    return;
  if (D_status)
    {
      Msg(0, "%s", "");	/* wait till mintime (keep gcc quiet) */
      RemoveStatus();
    }

  if (MayResizeLayer(D_forecv->c_layer))
    ResizeLayer(D_forecv->c_layer, D_forecv->c_xe - D_forecv->c_xs + 1, D_forecv->c_ye - D_forecv->c_ys + 1, display);

  fore = D_fore;
  if (fore)
    {
      /* XXX ? */
      if (fore->w_monitor != MON_OFF)
	fore->w_monitor = MON_ON;
      fore->w_bell = BELL_ON;
      WindowChanged(fore, 'f');

#if 0
      if (ResizeDisplay(fore->w_width, fore->w_height))
	{
	  debug2("Cannot resize from (%d,%d)", D_width, D_height);
	  debug2(" to (%d,%d) -> resize window\n", fore->w_width, fore->w_height);
	  DoResize(D_width, D_height);
	}
#endif
    }
  Redisplay(norefresh + all_norefresh);
}


static int
NextWindow()
{
  register struct win **pp;
  int n = fore ? fore->w_number : maxwin;
  struct win *group = fore ? fore->w_group : 0;

  for (pp = fore ? wtab + n + 1 : wtab; pp != wtab + n; pp++)
    {
      if (pp == wtab + maxwin)
	pp = wtab;
      if (*pp)
	{
	  if (!fore || group == (*pp)->w_group)
	    break;
	}
    }
  if (pp == wtab + n)
    return -1;
  return pp - wtab;
}

static int
PreviousWindow()
{
  register struct win **pp;
  int n = fore ? fore->w_number : -1;
  struct win *group = fore ? fore->w_group : 0;

  for (pp = wtab + n - 1; pp != wtab + n; pp--)
    {
      if (pp == wtab - 1)
	pp = wtab + maxwin - 1;
      if (*pp)
	{
	  if (!fore || group == (*pp)->w_group)
	    break;
	}
    }
  if (pp == wtab + n)
    return -1;
  return pp - wtab;
}

static int
MoreWindows()
{
  char *m = "No other window.";
  if (windows && (fore == 0 || windows->w_next))
    return 1;
  if (fore == 0)
    {
      Msg(0, "No window available");
      return 0;
    }
  Msg(0, m, fore->w_number);	/* other arg for nethack */
  return 0;
}

void
KillWindow(wi)
struct win *wi;
{
  struct win **pp, *p;
  struct canvas *cv;
  int gotone;
  struct layout *lay;

  /*
   * Remove window from linked list.
   */
  for (pp = &windows; (p = *pp); pp = &p->w_next)
    if (p == wi)
      break;
  ASSERT(p);
  *pp = p->w_next;
  wi->w_inlen = 0;
  wtab[wi->w_number] = 0;

  if (windows == 0)
    {
      FreeWindow(wi);
      Finit(0);
    }

  /*
   * switch to different window on all canvases
   */
  for (display = displays; display; display = display->d_next)
    {
      gotone = 0;
      for (cv = D_cvlist; cv; cv = cv->c_next)
	{
	  if (Layer2Window(cv->c_layer) != wi)
	    continue;
	  /* switch to other window */
	  SetCanvasWindow(cv, FindNiceWindow(D_other, 0));
	  gotone = 1;
	}
      if (gotone)
	{
#ifdef ZMODEM
	  if (wi->w_zdisplay == display)
	    {
	      D_blocked = 0;
	      D_readev.condpos = D_readev.condneg = 0;
	    }
#endif
	  Activate(-1);
	}
    }

  /* do the same for the layouts */
  for (lay = layouts; lay; lay = lay->lay_next)
    UpdateLayoutCanvas(&lay->lay_canvas, wi);

  FreeWindow(wi);
  WindowChanged((struct win *)0, 'w');
  WindowChanged((struct win *)0, 'W');
  WindowChanged((struct win *)0, 0);
}

static void
LogToggle(on)
int on;
{
  char buf[1024];

  if ((fore->w_log != 0) == on)
    {
      if (display && !*rc_name)
	Msg(0, "You are %s logging.", on ? "already" : "not");
      return;
    }
  if (fore->w_log != 0)
    {
      Msg(0, "Logfile \"%s\" closed.", fore->w_log->name);
      logfclose(fore->w_log);
      fore->w_log = 0;
      WindowChanged(fore, 'f');
      return;
    }
  if (DoStartLog(fore, buf, sizeof(buf)))
    {
      Msg(errno, "Error opening logfile \"%s\"", buf);
      return;
    }
  if (ftell(fore->w_log->fp) == 0)
    Msg(0, "Creating logfile \"%s\".", fore->w_log->name);
  else
    Msg(0, "Appending to logfile \"%s\".", fore->w_log->name);
  WindowChanged(fore, 'f');
}

char *
AddWindows(buf, len, flags, where)
char *buf;
int len;
int flags;
int where;
{
  register char *s, *ss;
  register struct win **pp, *p;
  register char *cmd;
  int l;

  s = ss = buf;
  if ((flags & 8) && where < 0)
    {
      *s = 0;
      return ss;
    }
  for (pp = ((flags & 4) && where >= 0) ? wtab + where + 1: wtab; pp < wtab + maxwin; pp++)
    {
      int rend = -1;
      if (pp - wtab == where && ss == buf)
        ss = s;
      if ((p = *pp) == 0)
	continue;
      if ((flags & 1) && display && p == D_fore)
	continue;
      if (display && D_fore && D_fore->w_group != p->w_group)
	continue;

      cmd = p->w_title;
      l = strlen(cmd);
      if (l > 20)
        l = 20;
      if (s - buf + l > len - 24)
	break;
      if (s > buf || (flags & 4))
	{
	  *s++ = ' ';
	  *s++ = ' ';
	}
      if (p->w_number == where)
        {
          ss = s;
          if (flags & 8)
            break;
        }
      if (!(flags & 4) || where < 0 || ((flags & 4) && where < p->w_number))
	{
	  if (p->w_monitor == MON_DONE && renditions[REND_MONITOR] != -1)
	    rend = renditions[REND_MONITOR];
	  else if ((p->w_bell == BELL_DONE || p->w_bell == BELL_FOUND) && renditions[REND_BELL] != -1)
	    rend = renditions[REND_BELL];
	  else if ((p->w_silence == SILENCE_FOUND || p->w_silence == SILENCE_DONE) && renditions[REND_SILENCE] != -1)
	    rend = renditions[REND_SILENCE];
	}
      if (rend != -1)
	AddWinMsgRend(s, rend);
      sprintf(s, "%d", p->w_number);
      s += strlen(s);
      if (display && p == D_fore)
	*s++ = '*';
      if (!(flags & 2))
	{
          if (display && p == D_other)
	    *s++ = '-';
          s = AddWindowFlags(s, len, p);
	}
      *s++ = ' ';
      strncpy(s, cmd, l);
      s += l;
      if (rend != -1)
	AddWinMsgRend(s, -1);
    }
  *s = 0;
  return ss;
}

char *
AddWindowFlags(buf, len, p)
char *buf;
int len;
struct win *p;
{
  char *s = buf;
  if (p == 0 || len < 12)
    {
      *s = 0;
      return s;
    }
#if 0
  if (display && p == D_fore)
    *s++ = '*';
  if (display && p == D_other)
    *s++ = '-';
#endif
  if (p->w_layer.l_cvlist && p->w_layer.l_cvlist->c_lnext)
    *s++ = '&';
  if (p->w_monitor == MON_DONE
#ifdef MULTIUSER
      && display && (ACLBYTE(p->w_mon_notify, D_user->u_id) & ACLBIT(D_user->u_id))
#endif
     )
    *s++ = '@';
  if (p->w_bell == BELL_DONE)
    *s++ = '!';
#ifdef UTMPOK
  if (p->w_slot != (slot_t) 0 && p->w_slot != (slot_t) -1)
    *s++ = '$';
#endif
  if (p->w_log != 0)
    {
      strcpy(s, "(L)");
      s += 3;
    }
  if (p->w_ptyfd < 0 && p->w_type != W_TYPE_GROUP)
    *s++ = 'Z';
  *s = 0;
  return s;
}

char *
AddOtherUsers(buf, len, p)
char *buf;
int len;
struct win *p;
{
  struct display *d, *olddisplay = display;
  struct canvas *cv;
  char *s;
  int l;

  s = buf;
  for (display = displays; display; display = display->d_next)
    {
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
      if (len > 1 && s != buf)
	{
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

void
ShowWindows(where)
int where;
{
  char buf[1024];
  char *s, *ss;

  if (display && where == -1 && D_fore)
    where = D_fore->w_number;
  ss = AddWindows(buf, sizeof(buf), 0, where);
  s = buf + strlen(buf);
  if (display && ss - buf > D_width / 2)
    {
      ss -= D_width / 2;
      if (s - ss < D_width)
	{
	  ss = s - D_width;
	  if (ss < buf)
	    ss = buf;
	}
    }
  else
    ss = buf;
  Msg(0, "%s", ss);
}

/*
* String Escape based windows listing
* mls: currently does a Msg() call for each(!) window, dunno why
*/
static void
ShowWindowsX(str)
char *str;
{
	int i;
	debug1("ShowWindowsX: string [%s]", str);
	for (i = 0; i < maxwin ; i++) {
		if (!wtab[i])
			continue;
		Msg(0, "%s", MakeWinMsg(str, wtab[i], '%'));
	}
}


static void
ShowInfo()
{
  char buf[512], *p;
  register struct win *wp = fore;
  register int i;

  if (wp == 0)
    {
      Msg(0, "(%d,%d)/(%d,%d) no window", D_x + 1, D_y + 1, D_width, D_height);
      return;
    }
  p = buf;
  if (buf < (p += GetAnsiStatus(wp, p)))
    *p++ = ' ';
  sprintf(p, "(%d,%d)/(%d,%d)",
    wp->w_x + 1, wp->w_y + 1, wp->w_width, wp->w_height);
#ifdef COPY_PASTE
  sprintf(p += strlen(p), "+%d", wp->w_histheight);
#endif
  sprintf(p += strlen(p), " %c%sflow",
  	  (wp->w_flow & FLOW_NOW) ? '+' : '-',
	  (wp->w_flow & FLOW_AUTOFLAG) ? "" : 
	   ((wp->w_flow & FLOW_AUTO) ? "(+)" : "(-)"));
  if (!wp->w_wrap) sprintf(p += strlen(p), " -wrap");
  if (wp->w_insert) sprintf(p += strlen(p), " ins");
  if (wp->w_origin) sprintf(p += strlen(p), " org");
  if (wp->w_keypad) sprintf(p += strlen(p), " app");
  if (wp->w_log)    sprintf(p += strlen(p), " log");
  if (wp->w_monitor != MON_OFF
#ifdef MULTIUSER
      && (ACLBYTE(wp->w_mon_notify, D_user->u_id) & ACLBIT(D_user->u_id))
#endif
     )
    sprintf(p += strlen(p), " mon");
  if (wp->w_mouse) sprintf(p += strlen(p), " mouse");
#ifdef COLOR
  if (wp->w_bce) sprintf(p += strlen(p), " bce");
#endif
  if (!wp->w_c1) sprintf(p += strlen(p), " -c1");
  if (wp->w_norefresh) sprintf(p += strlen(p), " nored");

  p += strlen(p);
#ifdef FONT
# ifdef ENCODINGS
  if (wp->w_encoding && (display == 0 || D_encoding != wp->w_encoding || EncodingDefFont(wp->w_encoding) <= 0))
    {
      *p++ = ' ';
      strcpy(p, EncodingName(wp->w_encoding));
      p += strlen(p);
    }
#  ifdef UTF8
  if (wp->w_encoding != UTF8)
#  endif
# endif
    if (display && (D_CC0 || (D_CS0 && *D_CS0)))
      {
	if (wp->w_gr == 2)
	  {
	    sprintf(p, " G%c", wp->w_Charset + '0');
	    if (wp->w_FontE >= ' ')
	      p[3] = wp->w_FontE;
	    else
	      {
	        p[3] = '^';
	        p[4] = wp->w_FontE ^ 0x40;
		p++;
	      }
	    p[4] = '[';
	    p++;
	  }
	else if (wp->w_gr)
	  sprintf(p++, " G%c%c[", wp->w_Charset + '0', wp->w_CharsetR + '0');
	else
	  sprintf(p, " G%c[", wp->w_Charset + '0');
	p += 4;
	for (i = 0; i < 4; i++)
	  {
	    if (wp->w_charsets[i] == ASCII)
	      *p++ = 'B';
	    else if (wp->w_charsets[i] >= ' ')
	      *p++ = wp->w_charsets[i];
	    else
	      {
		*p++ = '^';
		*p++ = wp->w_charsets[i] ^ 0x40;
	      }
	  }
	*p++ = ']';
	*p = 0;
      }
#endif

  if (wp->w_type == W_TYPE_PLAIN)
    {
      /* add info about modem control lines */
      *p++ = ' ';
      TtyGetModemStatus(wp->w_ptyfd, p);
    }
#ifdef BUILTIN_TELNET
  else if (wp->w_type == W_TYPE_TELNET)
    {
      *p++ = ' ';
      TelStatus(wp, p, sizeof(buf) - 1 - (p - buf));
    }
#endif
  Msg(0, "%s %d(%s)", buf, wp->w_number, wp->w_title);
}

static void
ShowDInfo()
{
  char buf[512], *p;
  if (display == 0)
    return;
  p = buf;
  sprintf(p, "(%d,%d)", D_width, D_height),
  p += strlen(p);
#ifdef ENCODINGS
  if (D_encoding)
    {
      *p++ = ' ';
      strcpy(p, EncodingName(D_encoding));
      p += strlen(p);
    }
#endif
  if (D_CXT)
    {
      strcpy(p, " xterm");
      p += strlen(p);
    }
#ifdef COLOR
  if (D_hascolor)
    {
      strcpy(p, " color");
      p += strlen(p);
    }
#endif
#ifdef FONT
  if (D_CG0)
    {
      strcpy(p, " iso2022");
      p += strlen(p);
    }
  else if (D_CS0 && *D_CS0)
    {
      strcpy(p, " altchar");
      p += strlen(p);
    }
#endif
  Msg(0, "%s", buf);
}

static void
AKAfin(buf, len, data)
char *buf;
int len;
char *data;	/* dummy */
{
  ASSERT(display);
  if (len && fore)
    ChangeAKA(fore, buf, strlen(buf));

  enter_window_name_mode = 0;
}

static void
InputAKA()
{
  char *s, *ss;
  int n;

  if (enter_window_name_mode == 1) return;

  enter_window_name_mode = 1;

  Input("Set window's title to: ", sizeof(fore->w_akabuf) - 1, INP_COOKED, AKAfin, NULL, 0);
  s = fore->w_title;
  if (!s)
    return;
  for (; *s; s++)
    {
      if ((*(unsigned char *)s & 0x7f) < 0x20 || *s == 0x7f)
	continue;
      ss = s;
      n = 1;
      LayProcess(&ss, &n);
    }
}

static void
Colonfin(buf, len, data)
char *buf;
int len;
char *data;	/* dummy */
{
  char mbuf[256];

  RemoveStatus();
  if (buf[len] == '\t')
    {
      int m, x;
      int l = 0, r = RC_LAST;
      int showmessage = 0;
      char *s = buf;

      while (*s && s - buf < len)
	if (*s++ == ' ')
	  return;

      /* Showing a message when there's no hardstatus or caption cancels the input */
      if (display &&
	  (captionalways || D_has_hstatus == HSTATUS_LASTLINE || (D_canvas.c_slperp && D_canvas.c_slperp->c_slnext)))
	showmessage = 1;

      while (l <= r)
	{
	  m = (l + r) / 2;
	  x = strncmp(buf, comms[m].name, len);
	  if (x > 0)
	    l = m + 1;
	  else if (x < 0)
	    r = m - 1;
	  else
	    {
	      s = mbuf;
	      for (l = m - 1; l >= 0 && strncmp(buf, comms[l].name, len) == 0; l--)
		;
	      for (m = ++l; m <= r && strncmp(buf, comms[m].name, len) == 0 && s - mbuf < sizeof(mbuf); m++)
		s += snprintf(s, sizeof(mbuf) - (s - mbuf), " %s", comms[m].name);
	      if (l < m - 1)
		{
		  if (showmessage)
		    Msg(0, "Possible commands:%s", mbuf);
		}
	      else
		{
		  s = mbuf;
		  len = snprintf(mbuf, sizeof(mbuf), "%s \t", comms[l].name + len);
		  if (len > 0 && len < sizeof(mbuf))
		    LayProcess(&s, &len);
		}
	      break;
	    }
	}
      if (l > r && showmessage)
	Msg(0, "No commands matching '%*s'", len, buf);
      return;
    }

  if (!len || buf[len])
    return;

  len = strlen(buf) + 1;
  if (len > (int)sizeof(mbuf))
    RcLine(buf, len);
  else
    {
      bcopy(buf, mbuf, len);
      RcLine(mbuf, sizeof mbuf);
    }
}

static void
SelectFin(buf, len, data)
char *buf;
int len;
char *data;	/* dummy */
{
  int n;

  if (!len || !display)
    return;
  if (len == 1 && *buf == '-')
    {
      SetForeWindow((struct win *)0);
      Activate(0);
      return;
    }
  if ((n = WindowByNoN(buf)) < 0)
    return;
  SwitchWindow(n);
}

static void
SelectLayoutFin(buf, len, data)
char *buf;
int len;
char *data;	/* dummy */
{
  struct layout *lay;

  if (!len || !display)
    return;
  if (len == 1 && *buf == '-')
    {
      LoadLayout((struct layout *)0, (struct canvas *)0);
      Activate(0);
      return;
    }
  lay = FindLayout(buf);
  if (!lay)
    Msg(0, "No such layout\n");
  else if (lay == D_layout)
    Msg(0, "This IS layout %d (%s).\n", lay->lay_number, lay->lay_title);
  else
    {
      LoadLayout(lay, &D_canvas);
      Activate(0);
    }
}

    
static void
InputSelect()
{
  Input("Switch to window: ", 20, INP_COOKED, SelectFin, NULL, 0);
}

static char setenv_var[31];


static void
SetenvFin1(buf, len, data)
char *buf;
int len;
char *data;	/* dummy */
{
  if (!len || !display)
    return;
  InputSetenv(buf);
}
  
static void
SetenvFin2(buf, len, data)
char *buf;
int len;
char *data;	/* dummy */
{
  if (!len || !display)
    return;
  debug2("SetenvFin2: setenv '%s' '%s'\n", setenv_var, buf);
  xsetenv(setenv_var, buf);
  MakeNewEnv();
}

static void
InputSetenv(arg)
char *arg;
{
  static char setenv_buf[50 + sizeof(setenv_var)];	/* need to be static here, cannot be freed */

  if (arg)
    {
      strncpy(setenv_var, arg, sizeof(setenv_var) - 1);
      sprintf(setenv_buf, "Enter value for %s: ", setenv_var);
      Input(setenv_buf, 30, INP_COOKED, SetenvFin2, NULL, 0);
    }
  else
    Input("Setenv: Enter variable name: ", 30, INP_COOKED, SetenvFin1, NULL, 0);
}

/*
 * the following options are understood by this parser:
 * -f, -f0, -f1, -fy, -fa
 * -t title, -T terminal-type, -h height-of-scrollback, 
 * -ln, -l0, -ly, -l1, -l
 * -a, -M, -L
 */
void
DoScreen(fn, av)
char *fn, **av;
{
  struct NewWindow nwin;
  register int num;
  char buf[20];

  nwin = nwin_undef;
  while (av && *av && av[0][0] == '-')
    {
      if (av[0][1] == '-')
	{
	  av++;
	  break;
	}
      switch (av[0][1])
	{
	case 'f':
	  switch (av[0][2])
	    {
	    case 'n':
	    case '0':
	      nwin.flowflag = FLOW_NOW * 0;
	      break;
	    case 'y':
	    case '1':
	    case '\0':
	      nwin.flowflag = FLOW_NOW * 1;
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
#ifdef LOGOUTOK
	case 'l':
	  switch (av[0][2])
	    {
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
	  nwin.aflag = 1;
	  break;
	case 'M':
	  nwin.monitor = MON_ON;
	  break;
	case 'L':
	  nwin.Lflag = 1;
	  break;
	default:
	  Msg(0, "%s: screen: invalid option -%c.", fn, av[0][1]);
	  break;
	}
      ++av;
    }
  if (av && *av && IsNumColon(*av, 10, buf, sizeof(buf)))
    {
      if (*buf != '\0')
	nwin.aka = buf;
      num = atoi(*av);
      if (num < 0 || (maxwin && num > maxwin - 1) || (!maxwin && num > MAXWIN - 1))
	{
	  Msg(0, "%s: illegal screen number %d.", fn, num);
	  num = 0;
	}
      nwin.StartAt = num;
      ++av;
    }
  if (av && *av)
    {
      nwin.args = av;
      if (!nwin.aka)
        nwin.aka = Filename(*av);
    }
  MakeWindow(&nwin);
}

#ifdef COPY_PASTE
/*
 * CompileKeys must be called before Markroutine is first used.
 * to initialise the keys with defaults, call CompileKeys(NULL, mark_key_tab);
 *
 * s is an ascii string in a termcap-like syntax. It looks like
 *   "j=u:k=d:l=r:h=l: =.:" and so on...
 * this example rebinds the cursormovement to the keys u (up), d (down),
 * l (left), r (right). placing a mark will now be done with ".".
 */
int
CompileKeys(s, sl, array)
char *s;
int sl;
unsigned char *array;
{
  int i;
  unsigned char key, value;

  if (sl == 0)
    {
      for (i = 0; i < 256; i++)
        array[i] = i;
      return 0;
    }
  debug1("CompileKeys: '%s'\n", s);
  while (sl)
    {
      key = *(unsigned char *)s++;
      if (*s != '=' || sl < 3)
	return -1;
      sl--;
      do 
	{
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
#endif /* COPY_PASTE */

/*
 *  Asynchronous input functions
 */

#if defined(DETACH) && defined(POW_DETACH)
static void
pow_detach_fn(buf, len, data)
char *buf;
int len;
char *data;	/* dummy */
{
  debug("pow_detach_fn called\n");
  if (len)
    {
      memset(buf, 0, len);
      return;
    }
  if (ktab[(int)(unsigned char)*buf].nr != RC_POW_DETACH)
    {
      if (display)
        write(D_userfd, "\007", 1);
      Msg(0, "Detach aborted.");
    }
  else
    Detach(D_POWER);
}
#endif /* POW_DETACH */

#ifdef COPY_PASTE
static void
copy_reg_fn(buf, len, data)
char *buf;
int len;
char *data;	/* dummy */
{
  struct plop *pp = plop_tab + (int)(unsigned char)*buf;

  if (len)
    {
      memset(buf, 0, len);
      return;
    }
  if (pp->buf)
    free(pp->buf);
  pp->buf = 0;
  pp->len = 0;
  if (D_user->u_plop.len)
    {
      if ((pp->buf = (char *)malloc(D_user->u_plop.len)) == NULL)
	{
	  Msg(0, "%s", strnomem);
	  return;
	}
      bcopy(D_user->u_plop.buf, pp->buf, D_user->u_plop.len);
    }
  pp->len = D_user->u_plop.len;
#ifdef ENCODINGS
  pp->enc = D_user->u_plop.enc;
#endif
  Msg(0, "Copied %d characters into register %c", D_user->u_plop.len, *buf);
}

static void
ins_reg_fn(buf, len, data)
char *buf;
int len;
char *data;	/* dummy */
{
  struct plop *pp = plop_tab + (int)(unsigned char)*buf;

  if (len)
    {
      memset(buf, 0, len);
      return;
    }
  if (!fore)
    return;	/* Input() should not call us w/o fore, but you never know... */
  if (*buf == '.')
    Msg(0, "ins_reg_fn: Warning: pasting real register '.'!");
  if (pp->buf)
    {
      MakePaster(&fore->w_paster, pp->buf, pp->len, 0);
      return;
    }
  Msg(0, "Empty register.");
}
#endif /* COPY_PASTE */

static void
process_fn(buf, len, data)
char *buf;
int len;
char *data;	/* dummy */
{
  struct plop *pp = plop_tab + (int)(unsigned char)*buf;

  if (len)
    {
      memset(buf, 0, len);
      return;
    }
  if (pp->buf)
    {
      ProcessInput(pp->buf, pp->len);
      return;
    }
  Msg(0, "Empty register.");
}

static void
confirm_fn(buf, len, data)
char *buf;
int len;
char *data;
{
  struct action act;

  if (len || (*buf != 'y' && *buf != 'Y'))
    {
      memset(buf, 0, len);
      return;
    }
  act.nr = *(int *)data;
  act.args = noargs;
  act.argl = 0;
  act.quiet = 0;
  DoAction(&act, -1);
}

#ifdef MULTIUSER
struct inputsu
{
  struct acluser **up;
  char name[24];
  char pw1[130];	/* FreeBSD crypts to 128 bytes */
  char pw2[130];
};

static void
su_fin(buf, len, data)
char *buf;
int len;
char *data;
{
  struct inputsu *i = (struct inputsu *)data;
  char *p;
  int l;

  if (!*i->name)
    { p = i->name; l = sizeof(i->name) - 1; }
  else if (!*i->pw1)
    { strcpy(p = i->pw1, "\377"); l = sizeof(i->pw1) - 1; }
  else
    { strcpy(p = i->pw2, "\377"); l = sizeof(i->pw2) - 1; }
  if (buf && len)
    strncpy(p, buf, 1 + ((l < len) ? l : len));
  if (!*i->name)
    Input("Screen User: ", sizeof(i->name) - 1, INP_COOKED, su_fin, (char *)i, 0);
  else if (!*i->pw1)
    Input("User's UNIX Password: ", sizeof(i->pw1)-1, INP_COOKED|INP_NOECHO, su_fin, (char *)i, 0);
  else if (!*i->pw2)
    Input("User's Screen Password: ", sizeof(i->pw2)-1, INP_COOKED|INP_NOECHO, su_fin, (char *)i, 0);
  else
    {
      if ((p = DoSu(i->up, i->name, i->pw2, i->pw1)))
        Msg(0, "%s", p);
      free((char *)i);
    }
}
 
static int
InputSu(w, up, name)
struct win *w;
struct acluser **up;
char *name;
{
  struct inputsu *i;

  if (!(i = (struct inputsu *)calloc(1, sizeof(struct inputsu))))
    return -1;

  i->up = up;
  if (name && *name)
    su_fin(name, (int)strlen(name), (char *)i); /* can also initialise stuff */
  else
    su_fin((char *)0, 0, (char *)i);
  return 0;
}
#endif	/* MULTIUSER */

#ifdef PASSWORD

static void
pass1(buf, len, data)
char *buf;
int len;
char *data;
{
  struct acluser *u = (struct acluser *)data;

  if (!*buf)
    return;
  ASSERT(u);
  if (u->u_password != NullStr)
    free((char *)u->u_password);
  u->u_password = SaveStr(buf);
  bzero(buf, strlen(buf));
  Input("Retype new password:", 100, INP_NOECHO, pass2, data, 0);
}

static void
pass2(buf, len, data)
char *buf;
int len;
char *data;
{
  int st;
  char salt[3];
  struct acluser *u = (struct acluser *)data;

  ASSERT(u);
  if (!buf || strcmp(u->u_password, buf))
    {
      Msg(0, "[ Passwords don't match - checking turned off ]");
      if (u->u_password != NullStr)
        {
          bzero(u->u_password, strlen(u->u_password));
          free((char *)u->u_password);
	}
      u->u_password = NullStr;
    }
  else if (u->u_password[0] == '\0')
    {
      Msg(0, "[ No password - no secure ]");
      if (buf)
        bzero(buf, strlen(buf));
    }
  
  if (u->u_password != NullStr)
    {
      for (st = 0; st < 2; st++)
	salt[st] = 'A' + (int)((time(0) >> 6 * st) % 26);
      salt[2] = 0;
      buf = crypt(u->u_password, salt);
      bzero(u->u_password, strlen(u->u_password));
      free((char *)u->u_password);
      if (!buf)
	{
	  Msg(0, "[ crypt() error - no secure ]");
	  u->u_password = NullStr;
	  return;
	}
      u->u_password = SaveStr(buf);
      bzero(buf, strlen(buf));
#ifdef COPY_PASTE
      if (u->u_plop.buf)
	UserFreeCopyBuffer(u);
      u->u_plop.len = strlen(u->u_password);
# ifdef ENCODINGS
      u->u_plop.enc = 0;
#endif
      if (!(u->u_plop.buf = SaveStr(u->u_password)))
	{
	  Msg(0, "%s", strnomem);
          D_user->u_plop.len = 0;
	}
      else
	Msg(0, "[ Password moved into copybuffer ]");
#else				/* COPY_PASTE */
      Msg(0, "[ Crypted password is \"%s\" ]", u->u_password);
#endif				/* COPY_PASTE */
    }
}
#endif /* PASSWORD */

static int
digraph_find(buf)
const char *buf;
{
  int i;
  for (i = 0; i < MAX_DIGRAPH && digraphs[i].d[0]; i++)
    if ((digraphs[i].d[0] == (unsigned char)buf[0] && digraphs[i].d[1] == (unsigned char)buf[1]) ||
	(digraphs[i].d[0] == (unsigned char)buf[1] && digraphs[i].d[1] == (unsigned char)buf[0]))
      break;
  return i;
}

static void
digraph_fn(buf, len, data)
char *buf;
int len;
char *data;	/* dummy */
{
  int ch, i, x;

  ch = buf[len];
  if (ch)
    {
      buf[len + 1] = ch;		/* so we can restore it later */
      if (ch < ' ' || ch == '\177')
	return;
      if (len >= 1 && ((*buf == 'U' && buf[1] == '+') || (*buf == '0' && (buf[1] == 'x' || buf[1] == 'X'))))
	{
	  if (len == 1)
	    return;
	  if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f') && (ch < 'A' || ch > 'F'))
	    {
	      buf[len] = '\034';	/* ^] is ignored by Input() */
	      return;
	    }
	  if (len == (*buf == 'U' ? 5 : 3))
	    buf[len] = '\n';
	  return;
	}
      if (len && *buf == '0')
	{
	  if (ch < '0' || ch > '7')
	    {
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
  if (buf[len + 1])
    {
      buf[len] = buf[len + 1];	/* stored above */
      len++;
    }
  if (len < 2)
    return;
  if (!parse_input_int(buf, len, &x))
    {
      i = digraph_find(buf);
      if ((x = digraphs[i].value) <= 0)
	{
	  Msg(0, "Unknown digraph");
	  return;
	}
    }
  i = 1;
  *buf = x;
#ifdef UTF8
  if (flayer->l_encoding == UTF8)
    i = ToUtf8(buf, x);	/* buf is big enough for all UTF-8 codes */
#endif
  while(i)
    LayProcess(&buf, &i);
}

#ifdef MAPKEYS
int
StuffKey(i)
int i;
{
  struct action *act;
  int discard = 0;
  int keyno = i;

  debug1("StuffKey #%d", i);
#ifdef DEBUG
  if (i < KMAP_KEYS)
    debug1(" - %s", term[i + T_CAPS].tcname);
#endif

  if (i < KMAP_KEYS && D_ESCseen)
    {
      struct action *act = &D_ESCseen[i + 256];
      if (act->nr != RC_ILLEGAL)
	{
	  D_ESCseen = 0;
	  WindowChanged(fore, 'E');
          DoAction(act, i + 256);
	  return 0;
	}
      discard = 1;
    }

  if (i >= T_CURSOR - T_CAPS && i < T_KEYPAD - T_CAPS && D_cursorkeys)
    i += T_OCAPS - T_CURSOR;
  else if (i >= T_KEYPAD - T_CAPS && i < T_OCAPS - T_CAPS && D_keypad)
    i += T_OCAPS - T_CURSOR;
  debug1(" - action %d\n", i);
  flayer = D_forecv->c_layer;
  fore = D_fore;
  act = 0;
#ifdef COPY_PASTE
  if (flayer && flayer->l_mode == 1)
    act = i < KMAP_KEYS+KMAP_AKEYS ? &mmtab[i] : &kmap_exts[i - (KMAP_KEYS+KMAP_AKEYS)].mm;
#endif
  if ((!act || act->nr == RC_ILLEGAL) && !D_mapdefault)
    act = i < KMAP_KEYS+KMAP_AKEYS ? &umtab[i] : &kmap_exts[i - (KMAP_KEYS+KMAP_AKEYS)].um;
  if (!act || act->nr == RC_ILLEGAL)
    act = i < KMAP_KEYS+KMAP_AKEYS ? &dmtab[i] : &kmap_exts[i - (KMAP_KEYS+KMAP_AKEYS)].dm;

  if (discard && (!act || act->nr != RC_COMMAND))
    {
      /* if the input was just a single byte we let it through */
      if (D_tcs[keyno + T_CAPS].str && strlen(D_tcs[keyno + T_CAPS].str) == 1)
	return -1;
      if (D_ESCseen)
        {
          D_ESCseen = 0;
          WindowChanged(fore, 'E');
        }
      return 0;
    }
  D_mapdefault = 0;

  if (act == 0 || act->nr == RC_ILLEGAL)
    return -1;
  DoAction(act, 0);
  return 0;
}
#endif


static int
IsOnDisplay(wi)
struct win *wi;
{
  struct canvas *cv;
  ASSERT(display);
  for (cv = D_cvlist; cv; cv = cv->c_next)
    if (Layer2Window(cv->c_layer) == wi)
      return 1;
  return 0;
}

struct win *
FindNiceWindow(wi, presel)
struct win *wi;
char *presel;
{
  int i;

  debug2("FindNiceWindow %d %s\n", wi ? wi->w_number : -1 , presel ? presel : "NULL");
  if (presel)
    {
      i = WindowByNoN(presel);
      if (i >= 0)
	wi = wtab[i];
    }
  if (!display)
    return wi;
#ifdef MULTIUSER
  if (wi && AclCheckPermWin(D_user, ACL_READ, wi))
    wi = 0;
#endif
  if (!wi || (IsOnDisplay(wi) && !presel))
    {
      /* try to get another window */
      wi = 0;
#ifdef MULTIUSER
      for (wi = windows; wi; wi = wi->w_next)
	if (!wi->w_layer.l_cvlist && !AclCheckPermWin(D_user, ACL_WRITE, wi))
	  break;
      if (!wi)
        for (wi = windows; wi; wi = wi->w_next)
	  if (wi->w_layer.l_cvlist && !IsOnDisplay(wi) && !AclCheckPermWin(D_user, ACL_WRITE, wi))
	    break;
      if (!wi)
	for (wi = windows; wi; wi = wi->w_next)
	  if (!wi->w_layer.l_cvlist && !AclCheckPermWin(D_user, ACL_READ, wi))
	    break;
      if (!wi)
	for (wi = windows; wi; wi = wi->w_next)
	  if (wi->w_layer.l_cvlist && !IsOnDisplay(wi) && !AclCheckPermWin(D_user, ACL_READ, wi))
	    break;
#endif
      if (!wi)
	for (wi = windows; wi; wi = wi->w_next)
	  if (!wi->w_layer.l_cvlist)
	    break;
      if (!wi)
	for (wi = windows; wi; wi = wi->w_next)
	  if (wi->w_layer.l_cvlist && !IsOnDisplay(wi))
	    break;
    }
#ifdef MULTIUSER
  if (wi && AclCheckPermWin(D_user, ACL_READ, wi))
    wi = 0;
#endif
  return wi;
}

#if 0

/* sorted list of all commands */
static struct comm **commtab;
static int ncommtab;

void
AddComms(cos, hand)
struct comm *cos;
void (*hand) __P((struct comm *, char **, int));
{
  int n, i, j, r;
  for (n = 0; cos[n].name; n++)
    ;
  if (n == 0)
    return;
  if (commtab)
    commtab = (struct commt *)realloc(commtab, sizeof(*commtab) * (ncommtab + n));
  else
    commtab = (struct commt *)malloc(sizeof(*commtab) * (ncommtab + n));
  if (!commtab)
    Panic(0, strnomem);
  for (i = 0; i < n; i++)
    {
      for (j = 0; j < ncommtab; j++)
	{
	  r = strcmp(cos[i].name, commtab[j]->name);
	  if (r == 0)
	    Panic(0, "Duplicate command: %s\n", cos[i].name);
	  if (r < 0)
	    break;
	}
      for (r = ncommtab; r > j; r--)
	commtab[r] = commtab[r - 1];
      commtab[j] = cos + i;
      cos[i].handler = hand;
      bzero(cos[i].userbits, sizeof(cos[i].userbits));
      ncommtab++;
    }
}

struct comm *
FindComm(str)
char *str;
{
  int x, m, l = 0, r = ncommtab - 1;
  while (l <= r)
    {
      m = (l + r) / 2;
      x = strcmp(str, commtab[m]->name);
      if (x > 0)
	l = m + 1;
      else if (x < 0)
	r = m - 1;
      else
	return commtab[m];
    }
  return 0;
}

#endif

static int
CalcSlicePercent(cv, percent)
struct canvas *cv;
int percent;
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

static int
ChangeCanvasSize(fcv, abs, diff, gflag, percent)
struct canvas *fcv;	/* make this canvas bigger */
int abs;		/* mode: 0:rel 1:abs 2:max */
int diff;		/* change this much */
int gflag;		/* go up if neccessary */
int percent;
{
  struct canvas *cv;
  int done, have, m, dir;

  debug3("ChangeCanvasSize abs %d diff %d percent=%d\n", abs, diff, percent);
  if (abs == 0 && diff == 0)
    return 0;
  if (abs == 2)
    {
      if (diff == 0)
	  fcv->c_slweight = 0;
      else
	{
          for (cv = fcv->c_slback->c_slperp; cv; cv = cv->c_slnext)
	    cv->c_slweight = 0;
	  fcv->c_slweight = 1;
	  cv = fcv->c_slback->c_slback;
	  if (gflag && cv && cv->c_slback)
	    ChangeCanvasSize(cv, abs, diff, gflag, percent);
	}
      return diff;
    }
  if (abs)
    {
      if (diff < 0)
	diff = 0;
      if (percent && diff > percent)
	diff = percent;
    }
  if (percent)
    {
      int wsum, up;
      for (cv = fcv->c_slback->c_slperp, wsum = 0; cv; cv = cv->c_slnext)
	wsum += cv->c_slweight;
      if (wsum)
	{
	  up = gflag ? CalcSlicePercent(fcv->c_slback->c_slback, percent) : percent;
          debug3("up=%d, wsum=%d percent=%d\n", up, wsum, percent);
	  if (wsum < 1000)
	    {
	      int scale = wsum < 10 ? 1000 : 100;
	      for (cv = fcv->c_slback->c_slperp; cv; cv = cv->c_slnext)
		cv->c_slweight *= scale;
	      wsum *= scale;
	      debug1("scaled wsum to %d\n", wsum);
	    }
	  for (cv = fcv->c_slback->c_slperp; cv; cv = cv->c_slnext)
	    {
	      if (cv->c_slweight)
		{
	          cv->c_slweight = (cv->c_slweight * up) / percent;
		  if (cv->c_slweight == 0)
		    cv->c_slweight = 1;
		}
	      debug1("  - weight %d\n", cv->c_slweight);
	    }
	  diff = (diff * wsum) / percent;
	  percent = wsum;
	}
    }
  else
    {
      if (abs && diff == (fcv->c_slorient == SLICE_VERT ? fcv->c_ye - fcv->c_ys + 2 : fcv->c_xe - fcv->c_xs + 2))
	return 0;
      /* fix weights to real size (can't be helped, sorry) */
      for (cv = fcv->c_slback->c_slperp; cv; cv = cv->c_slnext)
	{
	  cv->c_slweight = cv->c_slorient == SLICE_VERT ? cv->c_ye - cv->c_ys + 2 : cv->c_xe - cv->c_xs + 2;
	  debug1("  - weight %d\n", cv->c_slweight);
	}
    }
  if (abs)
    diff = diff - fcv->c_slweight;
  debug1("diff = %d\n", diff);
  if (diff == 0)
    return 0;
  if (diff < 0)
    {
      cv = fcv->c_slnext ? fcv->c_slnext : fcv->c_slprev;
      fcv->c_slweight += diff;
      cv->c_slweight -= diff;
      return diff;
    }
  done = 0;
  dir = 1;
  for (cv = fcv->c_slnext; diff > 0; cv = dir > 0 ? cv->c_slnext : cv->c_slprev)
    {
      if (!cv)
	{
	  debug1("reached end, dir is %d\n", dir);
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
      debug2("min is %d, have %d\n", m, cv->c_slweight);
      if (cv->c_slweight > m)
	{
	  have = cv->c_slweight - m;
	  if (have > diff)
	    have = diff;
	  debug1("subtract %d\n", have);
	  cv->c_slweight -= have;
	  done += have;
	  diff -= have;
	}
    }
  if (diff && gflag)
    {
      /* need more room! */
      cv = fcv->c_slback->c_slback;
      if (cv && cv->c_slback)
        done += ChangeCanvasSize(fcv->c_slback->c_slback, 0, diff, gflag, percent);
    }
  fcv->c_slweight += done;
  debug1("ChangeCanvasSize returns %d\n", done);
  return done;
}

static void
ResizeRegions(arg, flags)
char *arg;
int flags;
{
  struct canvas *cv;
  int diff, l;
  int gflag = 0, abs = 0, percent = 0;
  int orient = 0;

  ASSERT(display);
  if (!*arg)
    return;
  if (D_forecv->c_slorient == SLICE_UNKN)
    {
      Msg(0, "resize: need more than one region");
      return;
    }
  gflag = flags & RESIZE_FLAG_L ? 0 : 1;
  orient |= flags & RESIZE_FLAG_H ? SLICE_HORI : 0;
  orient |= flags & RESIZE_FLAG_V ? SLICE_VERT : 0;
  if (orient == 0)
    orient = D_forecv->c_slorient;
  l = strlen(arg);
  if (*arg == '=')
    {
      /* make all regions the same height */
      struct canvas *cv = gflag ? &D_canvas : D_forecv->c_slback;
      if (cv->c_slperp->c_slorient & orient)
	EqualizeCanvas(cv->c_slperp, gflag);
      /* can't use cv->c_slorient directly as it can be D_canvas */
      if ((cv->c_slperp->c_slorient ^ (SLICE_HORI ^ SLICE_VERT)) & orient)
        {
	  if (cv->c_slback)
	    {
	      cv = cv->c_slback;
	      EqualizeCanvas(cv->c_slperp, gflag);
	    }
	  else
	   EqualizeCanvas(cv, gflag);
        }
      ResizeCanvas(cv);
      RecreateCanvasChain();
      RethinkDisplayViewports();
      ResizeLayersToCanvases();
      return;
    }
  if (!strcmp(arg, "min") || !strcmp(arg, "0"))
    {
      abs = 2;
      diff = 0;
    }
  else if (!strcmp(arg, "max") || !strcmp(arg, "_"))
    {
      abs = 2;
      diff = 1;
    }
  else
    {
      if (l > 0 && arg[l - 1] == '%')
	percent = 1000;
      if (*arg == '+')
	diff = atoi(arg + 1);
      else if (*arg == '-')
	diff = -atoi(arg + 1);
      else
	{
	  diff = atoi(arg);		/* +1 because of caption line */
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

#if 0

  if (siz + diff < 1)
    diff = 1 - siz;
  if (siz + diff > dsize - (nreg - 1) * 2 - 1)
    diff = dsize - (nreg - 1) * 2 - 1 - siz;
  if (diff == 0 || siz + diff < 1)
    return;

  if (diff < 0)
    {
      if (D_forecv->c_next)
	{
	  D_forecv->c_ye += diff;
	  D_forecv->c_next->c_ys += diff;
	  D_forecv->c_next->c_yoff += diff;
	}
      else
	{
	  for (cv = D_cvlist; cv; cv = cv->c_next)
	    if (cv->c_next == D_forecv)
	      break;
	  ASSERT(cv);
	  cv->c_ye -= diff;
	  D_forecv->c_ys -= diff;
	  D_forecv->c_yoff -= diff;
	}
    }
  else
    {
      int s, i = 0, found = 0, di = diff, d2;
      s = dsize - (nreg - 1) * 2 - 1 - siz;
      for (cv = D_cvlist; cv; i = cv->c_ye + 2, cv = cv->c_next)
	{
	  if (cv == D_forecv)
	    {
	      cv->c_ye = i + (cv->c_ye - cv->c_ys) + diff;
	      cv->c_yoff -= cv->c_ys - i;
	      cv->c_ys = i;
	      found = 1;
	      continue;
	    }
	  s -= cv->c_ye - cv->c_ys;
	  if (!found)
	    {
	      if (s >= di)
		continue;
	      d2 = di - s;
	    }
	  else
	    d2 = di > cv->c_ye - cv->c_ys ? cv->c_ye - cv->c_ys : di;
	  di -= d2;
	  cv->c_ye = i + (cv->c_ye - cv->c_ys) - d2;
	  cv->c_yoff -= cv->c_ys - i;
	  cv->c_ys = i;
        }
    }
  RethinkDisplayViewports();
  ResizeLayersToCanvases();
#endif
}

static void
ResizeFin(buf, len, data)
char *buf;
int len;
char *data;
{
  int ch;
  int flags = *(int *)data;
  ch = ((unsigned char *)buf)[len];
  if (ch == 0)
    {
      ResizeRegions(buf, flags);
      return;
    }
  if (ch == 'h')
    flags ^= RESIZE_FLAG_H;
  else if (ch == 'v')
    flags ^= RESIZE_FLAG_V;
  else if (ch == 'b')
    flags |= RESIZE_FLAG_H|RESIZE_FLAG_V;
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

void
SetForeCanvas(d, cv)
struct display *d;
struct canvas *cv;
{
  struct display *odisplay = display;
  if (d->d_forecv == cv)
    return;

  display = d;
  D_forecv = cv;
  if ((focusminwidth && (focusminwidth < 0 || D_forecv->c_xe - D_forecv->c_xs + 1 < focusminwidth)) ||
      (focusminheight && (focusminheight < 0 || D_forecv->c_ye - D_forecv->c_ys + 1 < focusminheight)))
    {
      ResizeCanvas(&D_canvas);
      RecreateCanvasChain();
      RethinkDisplayViewports();
      ResizeLayersToCanvases();	/* redisplays */
    }
  fore = D_fore = Layer2Window(D_forecv->c_layer);
  if (D_other == fore)
    D_other = 0;
  flayer = D_forecv->c_layer;
#ifdef RXVT_OSC
  if (D_xtermosc[2] || D_xtermosc[3])
    {
      Activate(-1);
    }
  else
#endif
    {
      RefreshHStatus();
#ifdef RXVT_OSC
      RefreshXtermOSC();
#endif
      flayer = D_forecv->c_layer;
      CV_CALL(D_forecv, LayRestore();LaySetCursor());
      WindowChanged(0, 'F');
    }

  display = odisplay;
}

#ifdef RXVT_OSC
void
RefreshXtermOSC()
{
  int i;
  struct win *p;

  p = Layer2Window(D_forecv->c_layer);
  for (i = 4; i >=0; i--)
    SetXtermOSC(i, p ? p->w_xtermosc[i] : 0, "\a");
}
#endif

int
ParseAttrColor(s1, s2, msgok)
char *s1, *s2;
int msgok;
{
  int i, n;
  char *s, *ss;
  int r = 0;

  s = s1;
  while (*s == ' ')
    s++;
  ss = s;
  while (*ss && *ss != ' ')
    ss++;
  while (*ss == ' ')
    ss++;
  if (*s && (s2 || *ss || !((*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z') || *s == '.')))
    {
      int mode = 0, n = 0;
      if (*s == '+')
	{
	  mode = 1;
	  s++;
	}
      else if (*s == '-')
	{
	  mode = -1;
	  s++;
	}
      else if (*s == '!')
	{
	  mode = 2;
	  s++;
	}
      else if (*s == '=')
	s++;
      if (*s >= '0' && *s <= '9')
	{
	  n = *s++ - '0';
	  if (*s >= '0' && *s <= '9')
	    n = n * 16 + (*s++ - '0');
	  else if (*s >= 'a' && *s <= 'f')
	    n = n * 16 + (*s++ - ('a' - 10));
	  else if (*s >= 'A' && *s <= 'F')
	    n = n * 16 + (*s++ - ('A' - 10));
	  else if (*s && *s != ' ')
	    {
	      if (msgok)
		Msg(0, "Illegal attribute hexchar '%c'", *s);
	      return -1;
	    }
	}
      else
	{
	  while (*s && *s != ' ')
	    {
	      if (*s == 'd')
		n |= A_DI;
	      else if (*s == 'u')
		n |= A_US;
	      else if (*s == 'b')
		n |= A_BD;
	      else if (*s == 'r')
		n |= A_RV;
	      else if (*s == 's')
		n |= A_SO;
	      else if (*s == 'B')
		n |= A_BL;
	      else
		{
		  if (msgok)
		    Msg(0, "Illegal attribute specifier '%c'", *s);
		  return -1;
		}
	      s++;
	    }
	}
      if (*s && *s != ' ')
	{
	  if (msgok)
	    Msg(0, "junk after attribute description: '%c'", *s);
	  return -1;
	}
      if (mode == -1)
	r = n << 8 | n;
      else if (mode == 1)
	r = n << 8;
      else if (mode == 2)
	r = n;
      else if (mode == 0)
	r = 0xffff ^ n;
    }
  while (*s && *s == ' ')
    s++;

  if (s2)
    {
      if (*s)
	{
	  if (msgok)
	    Msg(0, "junk after description: '%c'", *s);
	  return -1;
	}
      s = s2;
      while (*s && *s == ' ')
	s++;
    }

#ifdef COLOR
  if (*s)
    {
      static char costr[] = "krgybmcw d    i.01234567 9     f               FKRGYBMCW      I ";
      int numco = 0, j;

      n = 0;
      if (*s == '.')
	{
	  numco++;
	  n = 0x0f;
	  s++;
	}
      for (j = 0; j < 2 && *s && *s != ' '; j++)
	{
	  for (i = 0; costr[i]; i++)
	    if (*s == costr[i])
	      break;
	  if (!costr[i])
	    {
	      if (msgok)
		Msg(0, "illegal color descriptor: '%c'", *s);
	      return -1;
	    }
	  numco++;
	  n = n << 4 | (i & 15);
#ifdef COLORS16
	  if (i >= 48)
	    n = (n & 0x20ff) | 0x200;
#endif
	  s++;
	}
      if ((n & 0xf00) == 0xf00)
        n ^= 0xf00;	/* clear superflous bits */
#ifdef COLORS16
      if (n & 0x2000)
	n ^= 0x2400;	/* shift bit into right position */
#endif
      if (numco == 1)
	n |= 0xf0;	/* don't change bg color */
      if (numco != 2 && n != 0xff)
	n |= 0x100;	/* special invert mode */
      if (*s && *s != ' ')
	{
	  if (msgok)
	    Msg(0, "junk after color description: '%c'", *s);
	  return -1;
	}
      n ^= 0xff;
      r |= n << 16;
    }
#endif

  while (*s && *s == ' ')
    s++;
  if (*s)
    {
      if (msgok)
	Msg(0, "junk after description: '%c'", *s);
      return -1;
    }
  debug1("ParseAttrColor %06x\n", r);
  return r;
}

/*
 *  Color coding:
 *    0-7 normal colors
 *    9   default color
 *    e   just set intensity
 *    f   don't change anything
 *  Intensity is encoded into bits 17(fg) and 18(bg).
 */
void
ApplyAttrColor(i, mc)
int i;
struct mchar *mc;
{
  debug1("ApplyAttrColor %06x\n", i);
  mc->attr |= i >> 8 & 255;
  mc->attr ^= i & 255;
#ifdef COLOR
  i = (i >> 16) ^ 0xff;
  if ((i & 0x100) != 0)
    {
      i &= 0xeff;
      if (mc->attr & (A_SO|A_RV))
# ifdef COLORS16
        i = ((i & 0x0f) << 4) | ((i & 0xf0) >> 4) | ((i & 0x200) << 1) | ((i & 0x400) >> 1);
# else
        i = ((i & 0x0f) << 4) | ((i & 0xf0) >> 4);
# endif
    }
# ifdef COLORS16
  if ((i & 0x0f) != 0x0f)
    mc->attr = (mc->attr & 0xbf) | ((i >> 3) & 0x40);
  if ((i & 0xf0) != 0xf0)
    mc->attr = (mc->attr & 0x7f) | ((i >> 3) & 0x80);
# endif
  mc->color = 0x99 ^ mc->color;
  if ((i & 0x0e) == 0x0e)
    i = (i & 0xf0) | (mc->color & 0x0f);
  if ((i & 0xe0) == 0xe0)
    i = (i & 0x0f) | (mc->color & 0xf0);
  mc->color = 0x99 ^ i;
  debug2("ApplyAttrColor - %02x %02x\n", mc->attr, i);
#endif
}
