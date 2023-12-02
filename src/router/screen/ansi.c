/* Copyright (c) 2008, 2009
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

#include <sys/types.h>
#include <fcntl.h>
#ifndef sun	/* we want to know about TIOCPKT. */
# include <sys/ioctl.h>
#endif

#include "config.h"
#include "screen.h"
#include "braille.h"
#include "extern.h"
#include "logfile.h"

extern struct display *display, *displays;
extern struct win *fore;	/* for 83 escape */
extern struct layer *flayer;	/* for 83 escape */

extern struct NewWindow nwin_default;	/* for ResetWindow() */
extern int  nversion;		/* numerical version of screen */
extern int  log_flush, logtstamp_on, logtstamp_after;
extern char *logtstamp_string;
extern char *captionstring;
extern char *hstatusstring;
extern char *wliststr;
#ifdef COPY_PASTE
extern int compacthist;
#endif
#ifdef MULTIUSER
extern struct acluser *EffectiveAclUser;
#endif

/* widths for Z0/Z1 switching */
const int Z0width = 132;
const int Z1width = 80;

/* globals set in WriteString */
static struct win *curr;	/* window we are working on */
static int rows, cols;		/* window size of the curr window */

int visual_bell = 0;
int use_hardstatus = 1;		/* display status line in hs */
char *printcmd = 0;
int use_altscreen = 0;		/* enable alternate screen support? */

unsigned char *blank;		/* line filled with spaces */
unsigned char *null;		/* line filled with '\0' */

struct mline mline_old;
struct mline mline_blank;
struct mline mline_null;

struct mchar mchar_null;
struct mchar mchar_blank = {' ' /* , 0, 0, ... */};
struct mchar mchar_so    = {' ', A_SO /* , 0, 0, ... */};

int renditions[NUM_RENDS] = {65529 /* =ub */, 65531 /* =b */, 65533 /* =u */ };

/* keep string_t and string_t_string in sync! */
static char *string_t_string[] =
{
  "NONE",
  "DCS",			/* Device control string */
  "OSC",			/* Operating system command */
  "APC",			/* Application program command */
				/*  - used for status change */
  "PM",				/* Privacy message */
  "AKA",			/* title for current screen */
  "GM",				/* Global message to every display */
  "STATUS"			/* User hardstatus line */
};

/* keep state_t and state_t_string in sync! */
static char *state_t_string[] =
{
  "LIT",			/* Literal input */
  "ESC",			/* Start of escape sequence */
  "ASTR",			/* Start of control string */
  "STRESC",			/* ESC seen in control string */
  "CSI",			/* Reading arguments in "CSI Pn ;...*/
  "PRIN",			/* Printer mode */
  "PRINESC",			/* ESC seen in printer mode */
  "PRINCSI",			/* CSI seen in printer mode */
  "PRIN4"			/* CSI 4 seen in printer mode */
};

static int  Special __P((int));
static void DoESC __P((int, int));
static void DoCSI __P((int, int));
static void StringStart __P((enum string_t));
static void StringChar __P((int));
static int  StringEnd __P((void));
static void PrintStart __P((void));
static void PrintChar __P((int));
static void PrintFlush __P((void));
#ifdef FONT
static void DesignateCharset __P((int, int));
static void MapCharset __P((int));
static void MapCharsetR __P((int));
#endif
static void SaveCursor __P((struct cursor *));
static void RestoreCursor __P((struct cursor *));
static void BackSpace __P((void));
static void Return __P((void));
static void LineFeed __P((int));
static void ReverseLineFeed __P((void));
static void InsertChar __P((int));
static void DeleteChar __P((int));
static void DeleteLine __P((int));
static void InsertLine __P((int));
static void Scroll __P((char *, int, int, char *));
static void ForwardTab __P((void));
static void BackwardTab __P((void));
static void ClearScreen __P((void));
static void ClearFromBOS __P((void));
static void ClearToEOS __P((void));
static void ClearLineRegion __P((int, int));
static void CursorRight __P((int));
static void CursorUp __P((int));
static void CursorDown __P((int));
static void CursorLeft __P((int));
static void ASetMode __P((int));
static void SelectRendition __P((void));
static void RestorePosRendition __P((void));
static void FillWithEs __P((void));
static void FindAKA __P((void));
static void Report __P((char *, int, int));
static void ScrollRegion __P((int));
#ifdef COPY_PASTE
static void WAddLineToHist __P((struct win *, struct mline *));
#endif
static void WLogString __P((struct win *, char *, int));
static void WReverseVideo __P((struct win *, int));
static int  WindowChangedCheck __P((char *, int, int *));
static void MFixLine __P((struct win *, int, struct mchar *));
static void MScrollH __P((struct win *, int, int, int, int, int));
static void MScrollV __P((struct win *, int, int, int, int));
static void MClearArea __P((struct win *, int, int, int, int, int));
static void MInsChar __P((struct win *, struct mchar *, int, int));
static void MPutChar __P((struct win *, struct mchar *, int, int));
static void MPutStr __P((struct win *, char *, int, struct mchar *, int, int));
static void MWrapChar __P((struct win *, struct mchar *, int, int, int, int));
#ifdef COLOR
static void MBceLine __P((struct win *, int, int, int, int));
#endif

#ifdef COLOR
# define CURR_BCE (curr->w_bce ? rend_getbg(&curr->w_rend) : 0)
#else
# define CURR_BCE 0
#endif

void
ResetAnsiState(p)
struct win *p;
{
  p->w_state = LIT;
  p->w_StringType = NONE;
}

void
ResetWindow(p)
register struct win *p;
{
  register int i;

  p->w_wrap = nwin_default.wrap;
  p->w_origin = 0;
  p->w_insert = 0;
  p->w_revvid = 0;
  p->w_mouse = 0;
  p->w_curinv = 0;
  p->w_curvvis = 0;
  p->w_autolf = 0;
  p->w_keypad = 0;
  p->w_cursorkeys = 0;
  p->w_top = 0;
  p->w_bot = p->w_height - 1;
  p->w_saved.on = 0;
  p->w_x = p->w_y = 0;
  p->w_state = LIT;
  p->w_StringType = NONE;
  bzero(p->w_tabs, p->w_width);
  for (i = 8; i < p->w_width; i += 8)
    p->w_tabs[i] = 1;
  p->w_rend = mchar_null;
#ifdef FONT
  ResetCharsets(p);
#endif
#ifdef COLOR
  p->w_bce = nwin_default.bce;
#endif
}

/* adds max 22 bytes */
int
GetAnsiStatus(w, buf)
struct win *w;
char *buf;
{
  char *p = buf;

  if (w->w_state == LIT)
    return 0;

  strcpy(p, state_t_string[w->w_state]);
  p += strlen(p);
  if (w->w_intermediate)
    {
      *p++ = '-';
      if (w->w_intermediate > 0xff)
	p += AddXChar(p, w->w_intermediate >> 8);
      p += AddXChar(p, w->w_intermediate & 0xff);
      *p = 0;
    }
  if (w->w_state == ASTR || w->w_state == STRESC)
    sprintf(p, "-%s", string_t_string[w->w_StringType]);
  p += strlen(p);
  return p - buf;
}


#ifdef FONT

void
ResetCharsets(p)
struct win *p;
{
  p->w_gr = nwin_default.gr;
  p->w_c1 = nwin_default.c1;
  SetCharsets(p, "BBBB02");
  if (nwin_default.charset)
    SetCharsets(p, nwin_default.charset);
#ifdef ENCODINGS
  ResetEncoding(p);
#endif
}

void
SetCharsets(p, s)
struct win *p;
char *s;
{
  int i;

  for (i = 0; i < 4 && *s; i++, s++)
    if (*s != '.')
      p->w_charsets[i] = ((*s == 'B') ? ASCII : *s);
  if (*s && *s++ != '.')
    p->w_Charset = s[-1] - '0';
  if (*s && *s != '.')
    p->w_CharsetR = *s - '0';
  p->w_ss = 0;
  p->w_FontL = p->w_charsets[p->w_Charset];
  p->w_FontR = p->w_charsets[p->w_CharsetR];
}
#endif	/* FONT */

/*****************************************************************/


/*
 *  Here comes the vt100 emulator
 *  - writes logfiles,
 *  - sets timestamp and flags activity in window.
 *  - record program output in window scrollback
 *  - translate program output for the display and put it into the obuf.
 *
 */
void
WriteString(wp, buf, len)
struct win *wp;
register char *buf;
register int len;
{
  register int c;
#ifdef FONT
  register int font;
#endif
  struct canvas *cv;

  if (!len)
    return;
  if (wp->w_log)
    WLogString(wp, buf, len);

  /* set global variables (yuck!) */
  curr = wp;
  cols = curr->w_width;
  rows = curr->w_height;

  if (curr->w_silence)
    SetTimeout(&curr->w_silenceev, curr->w_silencewait * 1000);

  if (curr->w_monitor == MON_ON)
    {
      debug2("ACTIVITY %d %d\n", curr->w_monitor, curr->w_bell);
      curr->w_monitor = MON_FOUND;
    }

  if (cols > 0 && rows > 0)
    {
      do
	{
	  c = (unsigned char)*buf++;
#ifdef FONT
# ifdef DW_CHARS
	  if (!curr->w_mbcs)
# endif
	    curr->w_rend.font = curr->w_FontL;	/* Default: GL */
#endif

	  /* The next part is only for speedup */
	  if (curr->w_state == LIT &&
#ifdef UTF8
	      curr->w_encoding != UTF8 &&
#endif
#ifdef DW_CHARS
	      !is_dw_font(curr->w_rend.font) &&
# ifdef ENCODINGS
	      curr->w_rend.font != KANA && !curr->w_mbcs &&
# endif
#endif
#ifdef FONT
	      curr->w_rend.font != '<' &&
#endif
	      c >= ' ' && c != 0x7f &&
	      ((c & 0x80) == 0 || ((c >= 0xa0 || !curr->w_c1) && !curr->w_gr)) && !curr->w_ss &&
	      !curr->w_insert && curr->w_x < cols - 1)
	    {
	      register int currx = curr->w_x;
	      char *imp = buf - 1;

	      while (currx < cols - 1)
		{
		  currx++;
		  if (--len == 0)
		    break;
		  c = (unsigned char)*buf++;
		  if (c < ' ' || c == 0x7f || ((c & 0x80) && ((c < 0xa0 && curr->w_c1) || curr->w_gr)))
		    break;
		}
	      currx -= curr->w_x;
	      if (currx > 0)
		{
		  MPutStr(curr, imp, currx, &curr->w_rend, curr->w_x, curr->w_y);
		  LPutStr(&curr->w_layer, imp, currx, &curr->w_rend, curr->w_x, curr->w_y);
		  curr->w_x += currx;
		}
	      if (len == 0)
		break;
	    }
	  /* end of speedup code */

#ifdef UTF8
	  if (curr->w_encoding == UTF8)
	    {
	      c = FromUtf8(c, &curr->w_decodestate);
	      if (c == -1)
		continue;
	      if (c == -2)
		{
		  c = UCS_REPL;
		  /* try char again */
		  buf--;
		  len++;
		}
	      if (c > 0xff)
		debug1("read UNICODE %04x\n", c);
	    }
#endif

	tryagain:
	  switch (curr->w_state)
	    {
	    case PRIN:
	      switch (c)
		{
		case '\033':
		  curr->w_state = PRINESC;
		  break;
		default:
		  PrintChar(c);
		}
	      break;
	    case PRINESC:
	      switch (c)
		{
		case '[':
		  curr->w_state = PRINCSI;
		  break;
		default:
		  PrintChar('\033');
		  PrintChar(c);
		  curr->w_state = PRIN;
		}
	      break;
	    case PRINCSI:
	      switch (c)
		{
		case '4':
		  curr->w_state = PRIN4;
		  break;
		default:
		  PrintChar('\033');
		  PrintChar('[');
		  PrintChar(c);
		  curr->w_state = PRIN;
		}
	      break;
	    case PRIN4:
	      switch (c)
		{
		case 'i':
		  curr->w_state = LIT;
		  PrintFlush();
		  if (curr->w_pdisplay && curr->w_pdisplay->d_printfd >= 0)
		    {
		      close(curr->w_pdisplay->d_printfd);
		      curr->w_pdisplay->d_printfd = -1;
		    }
		  curr->w_pdisplay = 0;
		  break;
		default:
		  PrintChar('\033');
		  PrintChar('[');
		  PrintChar('4');
		  PrintChar(c);
		  curr->w_state = PRIN;
		}
	      break;
	    case ASTR:
	      if (c == 0)
		break;
	      if (c == '\033')
		{
		  curr->w_state = STRESC;
		  break;
		}
	      /* special xterm hack: accept SetStatus sequence. Yucc! */
	      /* allow ^E for title escapes */
	      if (!(curr->w_StringType == OSC && c < ' ' && c != '\005'))
		if (!curr->w_c1 || c != ('\\' ^ 0xc0))
		  {
		    StringChar(c);
		    break;
		  }
	      c = '\\';
	      /* FALLTHROUGH */
	    case STRESC:
	      switch (c)
		{
		case '\\':
		  if (StringEnd() == 0 || len <= 1)
		    break;
		  /* check if somewhere a status is displayed */
		  for (cv = curr->w_layer.l_cvlist; cv; cv = cv->c_lnext)
		    {
		      display = cv->c_display;
		      if (D_status == STATUS_ON_WIN)
			break;
		    }
		  if (cv)
		    {
		      if (len > IOSIZE + 1)
			len = IOSIZE + 1;
		      curr->w_outlen = len - 1;
		      bcopy(buf, curr->w_outbuf, len - 1);
		      return;	/* wait till status is gone */
		    }
		  break;
		case '\033':
		  StringChar('\033');
		  break;
		default:
		  curr->w_state = ASTR;
		  StringChar('\033');
		  StringChar(c);
		  break;
		}
	      break;
	    case ESC:
	      switch (c)
		{
		case '[':
		  curr->w_NumArgs = 0;
		  curr->w_intermediate = 0;
		  bzero((char *) curr->w_args, MAXARGS * sizeof(int));
		  curr->w_state = CSI;
		  break;
		case ']':
		  StringStart(OSC);
		  break;
		case '_':
		  StringStart(APC);
		  break;
		case 'P':
		  StringStart(DCS);
		  break;
		case '^':
		  StringStart(PM);
		  break;
		case '!':
		  StringStart(GM);
		  break;
		case '"':
		case 'k':
		  StringStart(AKA);
		  break;
		default:
		  if (Special(c))
		    {
		      curr->w_state = LIT;
		      break;
		    }
		  debug1("not special. c = %x\n", c);
		  if (c >= ' ' && c <= '/')
		    {
		      if (curr->w_intermediate)
			{
#ifdef DW_CHARS
			  if (curr->w_intermediate == '$')
			    c |= '$' << 8;
			  else
#endif
			  c = -1;
			}
		      curr->w_intermediate = c;
		    }
		  else if (c >= '0' && c <= '~')
		    {
		      DoESC(c, curr->w_intermediate);
		      curr->w_state = LIT;
		    }
		  else
		    {
		      curr->w_state = LIT;
		      goto tryagain;
		    }
		}
	      break;
	    case CSI:
	      switch (c)
		{
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		  if (curr->w_NumArgs >= 0 && curr->w_NumArgs < MAXARGS)
		    {
		      if (curr->w_args[curr->w_NumArgs] < 100000000)
			curr->w_args[curr->w_NumArgs] =
			  10 * curr->w_args[curr->w_NumArgs] + (c - '0');
		    }
		  break;
		case ';':
		case ':':
		  if (curr->w_NumArgs < MAXARGS)
		    curr->w_NumArgs++;
		  break;
		default:
		  if (Special(c))
		    break;
		  if (c >= '@' && c <= '~')
		    {
		      if (curr->w_NumArgs < MAXARGS)
			curr->w_NumArgs++;
		      DoCSI(c, curr->w_intermediate);
		      if (curr->w_state != PRIN)
			curr->w_state = LIT;
		    }
		  else if ((c >= ' ' && c <= '/') || (c >= '<' && c <= '?'))
		    curr->w_intermediate = curr->w_intermediate ? -1 : c;
		  else
		    {
		      curr->w_state = LIT;
		      goto tryagain;
		    }
		}
	      break;
	    case LIT:
	    default:
#ifdef DW_CHARS
	      if (curr->w_mbcs)
		if (c <= ' ' || c == 0x7f || (c >= 0x80 && c < 0xa0 && curr->w_c1))
		  curr->w_mbcs = 0;
#endif
	      if (c < ' ')
		{
		  if (c == '\033')
		    {
		      curr->w_intermediate = 0;
		      curr->w_state = ESC;
		      if (curr->w_autoaka < 0)
			curr->w_autoaka = 0;
		    }
		  else
		    Special(c);
		  break;
		}
	      if (c >= 0x80 && c < 0xa0 && curr->w_c1)
#ifdef FONT
		if ((curr->w_FontR & 0xf0) != 0x20
# ifdef UTF8
		       || curr->w_encoding == UTF8
# endif
		   )
#endif
		{
		  switch (c)
		    {
		    case 0xc0 ^ 'D':
		    case 0xc0 ^ 'E':
		    case 0xc0 ^ 'H':
		    case 0xc0 ^ 'M':
		    case 0xc0 ^ 'N':		/* SS2 */
		    case 0xc0 ^ 'O':		/* SS3 */
		      DoESC(c ^ 0xc0, 0);
		      break;
		    case 0xc0 ^ '[':
		      if (curr->w_autoaka < 0)
			curr->w_autoaka = 0;
		      curr->w_NumArgs = 0;
		      curr->w_intermediate = 0;
		      bzero((char *) curr->w_args, MAXARGS * sizeof(int));
		      curr->w_state = CSI;
		      break;
		    case 0xc0 ^ 'P':
		      StringStart(DCS);
		      break;
		    default:
		      break;
		    }
		  break;
		}

#ifdef FONT
# ifdef DW_CHARS
	      if (!curr->w_mbcs)
		{
# endif
		  if (c < 0x80 || curr->w_gr == 0)
		    curr->w_rend.font = curr->w_FontL;
# ifdef ENCODINGS
		  else if (curr->w_gr == 2 && !curr->w_ss)
		    curr->w_rend.font = curr->w_FontE;
# endif
		  else
		    curr->w_rend.font = curr->w_FontR;
# ifdef DW_CHARS
		}
# endif
# ifdef UTF8
	      if (curr->w_encoding == UTF8)
		{
		  if (curr->w_rend.font == '0')
		    {
		      struct mchar mc, *mcp;

		      debug1("SPECIAL %x\n", c);
		      mc.image = c;
		      mc.mbcs = 0;
		      mc.font = '0';
		      mc.fontx = 0;
		      mcp = recode_mchar(&mc, 0, UTF8);
		      debug2("%02x %02x\n", mcp->image, mcp->font);
		      c = mcp->image | mcp->font << 8;
		    }
		  curr->w_rend.font = 0;
		}
	      if (curr->w_encoding == UTF8 && c >= 0x0300 && utf8_iscomb(c))
		{
		  int ox, oy;
		  struct mchar omc;

		  ox = curr->w_x - 1;
		  oy = curr->w_y;
		  if (ox < 0)
		    {
		      ox = curr->w_width - 1;
		      oy--;
		    }
		  if (oy < 0)
		    oy = 0;
		  copy_mline2mchar(&omc, &curr->w_mlines[oy], ox);
		  if (omc.image == 0xff && omc.font == 0xff && omc.fontx == 0)
		    {
		      ox--;
		      if (ox >= 0)
			{
			  copy_mline2mchar(&omc, &curr->w_mlines[oy], ox);
			  omc.mbcs = 0xff;
			}
		    }
		  if (ox >= 0)
		    {
		      utf8_handle_comb(c, &omc);
		      MFixLine(curr, oy, &omc);
		      copy_mchar2mline(&omc, &curr->w_mlines[oy], ox);
		      LPutChar(&curr->w_layer, &omc, ox, oy);
		      LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
		    }
		  break;
		}
#  ifdef DW_CHARS
		if (curr->w_encoding == UTF8 && utf8_isdouble(c))
		  curr->w_mbcs = 0xff;
#  endif
	      font = curr->w_rend.font;
# endif
# ifdef DW_CHARS
#  ifdef ENCODINGS
	      if (font == KANA && curr->w_encoding == SJIS && curr->w_mbcs == 0)
		{
		  /* Lets see if it is the first byte of a kanji */
		  debug1("%x may be first of SJIS\n", c);
		  if ((0x81 <= c && c <= 0x9f) || (0xe0 <= c && c <= 0xef))
		    {
		      debug("YES!\n");
		      curr->w_mbcs = c;
		      break;
		    }
		}
#  endif
	      if (font == 031 && c == 0x80 && !curr->w_mbcs)
		font = curr->w_rend.font = 0;
	      if (is_dw_font(font) && c == ' ')
		font = curr->w_rend.font = 0;
	      if (is_dw_font(font) || curr->w_mbcs)
		{
		  int t = c;
		  if (curr->w_mbcs == 0)
		    {
		      curr->w_mbcs = c;
		      break;
		    }
		  if (curr->w_x == cols - 1)
		    {
		      curr->w_x += curr->w_wrap ? 1 : -1;
		      debug1("Patched w_x to %d\n", curr->w_x);
		    }
#  ifdef UTF8
		  if (curr->w_encoding != UTF8)
#  endif
		    {
		      c = curr->w_mbcs;
#  ifdef ENCODINGS
		      if (font == KANA && curr->w_encoding == SJIS)
			{
			  debug2("SJIS !! %x %x\n", c, t);
			  /*
			   * SJIS -> EUC mapping:
			   *   First byte:
			   *     81,82...9f -> 21,23...5d
			   *     e0,e1...ef -> 5f,61...7d
			   *   Second byte:
			   *     40-7e -> 21-5f
			   *     80-9e -> 60-7e
			   *     9f-fc -> 21-7e (increment first byte!)
			   */
			  if (0x40 <= t && t <= 0xfc && t != 0x7f)
			    {
			      if (c <= 0x9f) c = (c - 0x81) * 2 + 0x21;
			      else c = (c - 0xc1) * 2 + 0x21;
			      if (t <= 0x7e) t -= 0x1f;
			      else if (t <= 0x9e) t -= 0x20;
			      else t -= 0x7e, c++;
			      curr->w_rend.font = KANJI;
			    }
			  else
			    {
			      /* Incomplete shift-jis - skip first byte */
			      c = t;
			      t = 0;
			    }
			  debug2("SJIS after %x %x\n", c, t);
			}
#  endif
		      if (t && curr->w_gr && font != 030 && font != 031)
			{
			  t &= 0x7f;
			  if (t < ' ')
			    goto tryagain;
			}
		      if (t == '\177')
			break;
		      curr->w_mbcs = t;
		    }
		}
# endif	/* DW_CHARS */
	      if (font == '<' && c >= ' ')
		{
		  font = curr->w_rend.font = 0;
		  c |= 0x80;
		}
# ifdef UTF8
	      else if (curr->w_gr && curr->w_encoding != UTF8)
# else
	      else if (curr->w_gr)
# endif
		{
#ifdef ENCODINGS
		  if (c == 0x80 && font == 0 && curr->w_encoding == GBK)
		    c = 0xa4;
		  else
		    c &= 0x7f;
		  if (c < ' ' && font != 031)
		    goto tryagain;
#else
		  c &= 0x7f;
		  if (c < ' ')	/* this is ugly but kanji support */
		    goto tryagain;	/* prevents nicer programming */
#endif
		}
#endif /* FONT */
	      if (c == '\177')
		break;
	      curr->w_rend.image = c;
#ifdef UTF8
	      if (curr->w_encoding == UTF8)
		{
		  curr->w_rend.font = c >> 8;
		  curr->w_rend.fontx = c >> 16;
		}
#endif
#ifdef DW_CHARS
	      curr->w_rend.mbcs = curr->w_mbcs;
#endif
	      if (curr->w_x < cols - 1)
		{
		  if (curr->w_insert)
		    {
		      save_mline(&curr->w_mlines[curr->w_y], cols);
		      MInsChar(curr, &curr->w_rend, curr->w_x, curr->w_y);
		      LInsChar(&curr->w_layer, &curr->w_rend, curr->w_x, curr->w_y, &mline_old);
		      curr->w_x++;
		    }
		  else
		    {
		      MPutChar(curr, &curr->w_rend, curr->w_x, curr->w_y);
		      LPutChar(&curr->w_layer, &curr->w_rend, curr->w_x, curr->w_y);
		      curr->w_x++;
		    }
		}
	      else if (curr->w_x == cols - 1)
		{
		  MPutChar(curr, &curr->w_rend, curr->w_x, curr->w_y);
		  LPutChar(&curr->w_layer, &curr->w_rend, curr->w_x, curr->w_y);
		  if (curr->w_wrap)
		    curr->w_x++;
		}
	      else
		{
		  MWrapChar(curr, &curr->w_rend, curr->w_y, curr->w_top, curr->w_bot, curr->w_insert);
		  LWrapChar(&curr->w_layer, &curr->w_rend, curr->w_y, curr->w_top, curr->w_bot, curr->w_insert);
		  if (curr->w_y != curr->w_bot && curr->w_y != curr->w_height - 1)
		    curr->w_y++;
		  curr->w_x = 1;
		}
#ifdef FONT
# ifdef DW_CHARS
	      if (curr->w_mbcs)
		{
		  curr->w_rend.mbcs = curr->w_mbcs = 0;
		  curr->w_x++;
		}
# endif
	      if (curr->w_ss)
		{
		  curr->w_FontL = curr->w_charsets[curr->w_Charset];
		  curr->w_FontR = curr->w_charsets[curr->w_CharsetR];
		  curr->w_rend.font = curr->w_FontL;
		  LSetRendition(&curr->w_layer, &curr->w_rend);
		  curr->w_ss = 0;
		}
#endif /* FONT */
	      break;
	    }
	}
      while (--len);
    }
  if (!printcmd && curr->w_state == PRIN)
    PrintFlush();
}

static void
WLogString(p, buf, len)
struct win *p;
char *buf;
int len;
{
  if (!p->w_log)
    return;
  if (logtstamp_on && p->w_logsilence >= logtstamp_after * 2)
    {
      char *t = MakeWinMsg(logtstamp_string, p, '%');
      logfwrite(p->w_log, t, strlen(t));	/* long time no write */
    }
  p->w_logsilence = 0;
  if (logfwrite(p->w_log, buf, len) < 1)
    {
      WMsg(p, errno, "Error writing logfile");
      logfclose(p->w_log);
      p->w_log = 0;
    }
  if (!log_flush)
    logfflush(p->w_log);
}

static int
Special(c)
register int c;
{
  switch (c)
    {
    case '\b':
      BackSpace();
      return 1;
    case '\r':
      Return();
      return 1;
    case '\n':
      if (curr->w_autoaka)
	FindAKA();
    case '\013':	/* Vertical tab is the same as Line Feed */
      LineFeed(0);
      return 1;
    case '\007':
      WBell(curr, visual_bell);
      return 1;
    case '\t':
      ForwardTab();
      return 1;
#ifdef FONT
    case '\017':		/* SI */
      MapCharset(G0);
      return 1;
    case '\016':		/* SO */
      MapCharset(G1);
      return 1;
#endif
    }
  return 0;
}

static void
DoESC(c, intermediate)
int c, intermediate;
{
  debug2("DoESC: %x - inter = %x\n", c, intermediate);
  switch (intermediate)
    {
    case 0:
      switch (c)
	{
	case 'E':
	  LineFeed(1);
	  break;
	case 'D':
	  LineFeed(0);
	  break;
	case 'M':
	  ReverseLineFeed();
	  break;
	case 'H':
	  curr->w_tabs[curr->w_x] = 1;
	  break;
	case 'Z':		/* jph: Identify as VT100 */
	  Report("\033[?%d;%dc", 1, 2);
	  break;
	case '7':
	  SaveCursor(&curr->w_saved);
	  break;
	case '8':
	  RestoreCursor(&curr->w_saved);
	  break;
	case 'c':
	  ClearScreen();
	  ResetWindow(curr);
	  LKeypadMode(&curr->w_layer, 0);
	  LCursorkeysMode(&curr->w_layer, 0);
#ifndef TIOCPKT
	  WNewAutoFlow(curr, 1);
#endif
	  /* XXX
          SetRendition(&mchar_null);
	  InsertMode(0);
	  ChangeScrollRegion(0, rows - 1);
	  */
	  LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
	  break;
	case '=':
	  LKeypadMode(&curr->w_layer, curr->w_keypad = 1);
#ifndef TIOCPKT
	  WNewAutoFlow(curr, 0);
#endif /* !TIOCPKT */
	  break;
	case '>':
	  LKeypadMode(&curr->w_layer, curr->w_keypad = 0);
#ifndef TIOCPKT
	  WNewAutoFlow(curr, 1);
#endif /* !TIOCPKT */
	  break;
#ifdef FONT
	case 'n':		/* LS2 */
	  MapCharset(G2);
	  break;
	case 'o':		/* LS3 */
	  MapCharset(G3);
	  break;
	case '~':
	  MapCharsetR(G1);	/* LS1R */
	  break;
	/* { */
	case '}':
	  MapCharsetR(G2);	/* LS2R */
	  break;
	case '|':
	  MapCharsetR(G3);	/* LS3R */
	  break;
	case 'N':		/* SS2 */
	  if (curr->w_charsets[curr->w_Charset] != curr->w_charsets[G2]
	      || curr->w_charsets[curr->w_CharsetR] != curr->w_charsets[G2])
	    curr->w_FontR = curr->w_FontL = curr->w_charsets[curr->w_ss = G2];
	  else
	    curr->w_ss = 0;
	  break;
	case 'O':		/* SS3 */
	  if (curr->w_charsets[curr->w_Charset] != curr->w_charsets[G3]
	      || curr->w_charsets[curr->w_CharsetR] != curr->w_charsets[G3])
	    curr->w_FontR = curr->w_FontL = curr->w_charsets[curr->w_ss = G3];
	  else
	    curr->w_ss = 0;
	  break;
#endif /* FONT */
        case 'g':		/* VBELL, private screen sequence */
	  WBell(curr, 1);
          break;
	}
      break;
    case '#':
      switch (c)
	{
	case '8':
	  FillWithEs();
	  break;
	}
      break;
#ifdef FONT
    case '(':
      DesignateCharset(c, G0);
      break;
    case ')':
      DesignateCharset(c, G1);
      break;
    case '*':
      DesignateCharset(c, G2);
      break;
    case '+':
      DesignateCharset(c, G3);
      break;
# ifdef DW_CHARS
/*
 * ESC $ ( Fn: invoke multi-byte charset, Fn, to G0
 * ESC $ Fn: same as above.  (old sequence)
 * ESC $ ) Fn: invoke multi-byte charset, Fn, to G1
 * ESC $ * Fn: invoke multi-byte charset, Fn, to G2
 * ESC $ + Fn: invoke multi-byte charset, Fn, to G3
 */
    case '$':
    case '$'<<8 | '(':
      DesignateCharset(c & 037, G0);
      break;
    case '$'<<8 | ')':
      DesignateCharset(c & 037, G1);
      break;
    case '$'<<8 | '*':
      DesignateCharset(c & 037, G2);
      break;
    case '$'<<8 | '+':
      DesignateCharset(c & 037, G3);
      break;
# endif
#endif /* FONT */
    }
}

static void
DoCSI(c, intermediate)
int c, intermediate;
{
  register int i, a1 = curr->w_args[0], a2 = curr->w_args[1];

  if (curr->w_NumArgs > MAXARGS)
    curr->w_NumArgs = MAXARGS;
  switch (intermediate)
    {
    case 0:
      switch (c)
	{
	case 'H':
	case 'f':
	  if (a1 < 1)
	    a1 = 1;
	  if (curr->w_origin)
	    a1 += curr->w_top;
	  if (a1 > rows)
	    a1 = rows;
	  if (a2 < 1)
	    a2 = 1;
	  if (a2 > cols)
	    a2 = cols;
	  LGotoPos(&curr->w_layer, --a2, --a1);
	  curr->w_x = a2;
	  curr->w_y = a1;
	  if (curr->w_autoaka)
	    curr->w_autoaka = a1 + 1;
	  break;
	case 'J':
	  if (a1 < 0 || a1 > 2)
	    a1 = 0;
	  switch (a1)
	    {
	    case 0:
	      ClearToEOS();
	      break;
	    case 1:
	      ClearFromBOS();
	      break;
	    case 2:
	      ClearScreen();
	      LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
	      break;
	    }
	  break;
	case 'K':
	  if (a1 < 0 || a1 > 2)
	    a1 %= 3;
	  switch (a1)
	    {
	    case 0:
	      ClearLineRegion(curr->w_x, cols - 1);
	      break;
	    case 1:
	      ClearLineRegion(0, curr->w_x);
	      break;
	    case 2:
	      ClearLineRegion(0, cols - 1);
	      break;
	    }
	  break;
	case 'X':
	  a1 = curr->w_x + (a1 ? a1 - 1 : 0);
	  ClearLineRegion(curr->w_x, a1 < cols ? a1 : cols - 1);
	  break;
	case 'A':
	  CursorUp(a1 ? a1 : 1);
	  break;
	case 'B':
	  CursorDown(a1 ? a1 : 1);
	  break;
	case 'C':
	  CursorRight(a1 ? a1 : 1);
	  break;
	case 'D':
	  CursorLeft(a1 ? a1 : 1);
	  break;
	case 'E':
	  curr->w_x = 0;
	  CursorDown(a1 ? a1 : 1);	/* positions cursor */
	  break;
	case 'F':
	  curr->w_x = 0;
	  CursorUp(a1 ? a1 : 1);	/* positions cursor */
	  break;
	case 'G':
	case '`':			/* HPA */
	  curr->w_x = a1 ? a1 - 1 : 0;
	  if (curr->w_x >= cols)
	    curr->w_x = cols - 1;
	  LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
	  break;
	case 'd':			/* VPA */
	  curr->w_y = a1 ? a1 - 1 : 0;
	  if (curr->w_y >= rows)
	    curr->w_y = rows - 1;
	  LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
	  break;
	case 'm':
	  SelectRendition();
	  break;
	case 'g':
	  if (a1 == 0)
	    curr->w_tabs[curr->w_x] = 0;
	  else if (a1 == 3)
	    bzero(curr->w_tabs, cols);
	  break;
	case 'r':
	  if (!a1)
	    a1 = 1;
	  if (!a2)
	    a2 = rows;
	  if (a1 < 1 || a2 > rows || a1 >= a2)
	    break;
	  curr->w_top = a1 - 1;
	  curr->w_bot = a2 - 1;
	  /* ChangeScrollRegion(curr->w_top, curr->w_bot); */
	  if (curr->w_origin)
	    {
	      curr->w_y = curr->w_top;
	      curr->w_x = 0;
	    }
	  else
	    curr->w_y = curr->w_x = 0;
	  LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
	  break;
	case 's':
	  SaveCursor(&curr->w_saved);
	  break;
	case 't':
	  switch(a1)
	    {
	    case 11:
	      if (curr->w_layer.l_cvlist)
		Report("\033[1t", 0, 0);
	      else
		Report("\033[2t", 0, 0);
	      break;
	    case 7:
	      LRefreshAll(&curr->w_layer, 0);
	      break;
	    case 21:
	      a1 = strlen(curr->w_title);
	      if ((unsigned)(curr->w_inlen + 5 + a1) <= sizeof(curr->w_inbuf))
		{
		  bcopy("\033]l", curr->w_inbuf + curr->w_inlen, 3);
		  bcopy(curr->w_title, curr->w_inbuf + curr->w_inlen + 3, a1);
		  bcopy("\033\\", curr->w_inbuf + curr->w_inlen + 3 + a1, 2);
		  curr->w_inlen += 5 + a1;
		}
	      break;
	    case 8:
	      a1 = curr->w_args[2];
	      if (a1 < 1)
		a1 = curr->w_width;
	      if (a2 < 1)
		a2 = curr->w_height;
	      if (a1 > 10000 || a2 > 10000)
		break;
	      WChangeSize(curr, a1, a2);
	      cols = curr->w_width;
	      rows = curr->w_height;
	      break;
	    default:
	      break;
	    }
	  break;
	case 'u':
	  RestoreCursor(&curr->w_saved);
	  break;
	case 'I':
	  if (!a1)
	    a1 = 1;
	  while (a1--)
	    ForwardTab();
	  break;
	case 'Z':
	  if (!a1)
	    a1 = 1;
	  while (a1--)
	    BackwardTab();
	  break;
	case 'L':
	  InsertLine(a1 ? a1 : 1);
	  break;
	case 'M':
	  DeleteLine(a1 ? a1 : 1);
	  break;
	case 'P':
	  DeleteChar(a1 ? a1 : 1);
	  break;
	case '@':
	  InsertChar(a1 ? a1 : 1);
	  break;
	case 'h':
	  ASetMode(1);
	  break;
	case 'l':
	  ASetMode(0);
	  break;
	case 'i':		/* MC Media Control */
	  if (a1 == 5)
	    PrintStart();
	  break;
	case 'n':
	  if (a1 == 5)		/* Report terminal status */
	    Report("\033[0n", 0, 0);
	  else if (a1 == 6)		/* Report cursor position */
	    Report("\033[%d;%dR", curr->w_y + 1, curr->w_x + 1);
	  break;
	case 'c':		/* Identify as VT100 */
	  if (a1 == 0)
	    Report("\033[?%d;%dc", 1, 2);
	  break;
	case 'x':		/* decreqtparm */
	  if (a1 == 0 || a1 == 1)
	    Report("\033[%d;1;1;112;112;1;0x", a1 + 2, 0);
	  break;
	case 'p':		/* obscure code from a 97801 term */
	  if (a1 == 6 || a1 == 7)
	    {
	      curr->w_curinv = 7 - a1;
	      LCursorVisibility(&curr->w_layer, curr->w_curinv ? -1 : curr->w_curvvis);
	    }
	  break;
	case 'S':		/* code from a 97801 term / DEC vt400 */
	  ScrollRegion(a1 ? a1 : 1);
	  break;
	case 'T':		/* code from a 97801 term / DEC vt400 */
	case '^':		/* SD as per ISO 6429 */
	  ScrollRegion(a1 ? -a1 : -1);
	  break;
	}
      break;
    case '?':
      for (a2 = 0; a2 < curr->w_NumArgs; a2++)
	{
	  a1 = curr->w_args[a2];
	  debug2("\\E[?%d%c\n",a1,c);
	  if (c != 'h' && c != 'l')
	    break;
	  i = (c == 'h');
	  switch (a1)
	    {
	    case 1:	/* CKM:  cursor key mode */
	      LCursorkeysMode(&curr->w_layer, curr->w_cursorkeys = i);
#ifndef TIOCPKT
	      WNewAutoFlow(curr, !i);
#endif /* !TIOCPKT */
	      break;
	    case 2:	/* ANM:  ansi/vt52 mode */
	      if (i)
		{
#ifdef FONT
# ifdef ENCODINGS
		  if (curr->w_encoding)
		    break;
# endif
		  curr->w_charsets[0] = curr->w_charsets[1] =
		    curr->w_charsets[2] = curr->w_charsets[3] =
		    curr->w_FontL = curr->w_FontR = ASCII;
		  curr->w_Charset = 0;
		  curr->w_CharsetR = 2;
		  curr->w_ss = 0;
#endif
		}
	      break;
	    case 3:	/* COLM: column mode */
	      i = (i ? Z0width : Z1width);
	      ClearScreen();
	      curr->w_x = 0;
	      curr->w_y = 0;
	      WChangeSize(curr, i, curr->w_height);
	      cols = curr->w_width;
	      rows = curr->w_height;
	      break;
	 /* case 4:	   SCLM: scrolling mode */
	    case 5:	/* SCNM: screen mode */
	      if (i != curr->w_revvid)
	        WReverseVideo(curr, i);
	      curr->w_revvid = i;
	      break;
	    case 6:	/* OM:   origin mode */
	      if ((curr->w_origin = i) != 0)
		{
		  curr->w_y = curr->w_top;
		  curr->w_x = 0;
		}
	      else
		curr->w_y = curr->w_x = 0;
	      LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
	      break;
	    case 7:	/* AWM:  auto wrap mode */
	      curr->w_wrap = i;
	      break;
	 /* case 8:	   ARM:  auto repeat mode */
	 /* case 9:	   INLM: interlace mode */
	    case 9:	/* X10 mouse tracking */
	      curr->w_mouse = i ? 9 : 0;
	      LMouseMode(&curr->w_layer, curr->w_mouse);
	      break;
	 /* case 10:	   EDM:  edit mode */
	 /* case 11:	   LTM:  line transmit mode */
	 /* case 13:	   SCFDM: space compression / field delimiting */
	 /* case 14:	   TEM:  transmit execution mode */
	 /* case 16:	   EKEM: edit key execution mode */
	 /* case 18:	   PFF:  Printer term form feed */
	 /* case 19:	   PEX:  Printer extend screen / scroll. reg */
	    case 25:	/* TCEM: text cursor enable mode */
	      curr->w_curinv = !i;
	      LCursorVisibility(&curr->w_layer, curr->w_curinv ? -1 : curr->w_curvvis);
	      break;
	 /* case 34:	   RLM:  Right to left mode */
	 /* case 35:	   HEBM: hebrew keyboard map */
	 /* case 36:	   HEM:  hebrew encoding */
	 /* case 38:	         TeK Mode */
	 /* case 40:	         132 col enable */
	 /* case 42:	   NRCM: 7bit NRC character mode */
	 /* case 44:	         margin bell enable */
	    case 47:    /*       xterm-like alternate screen */
	    case 1047:  /*       xterm-like alternate screen */
	    case 1049:  /*       xterm-like alternate screen */
	      if (use_altscreen)
		{
		  if (i)
		    {
		      if (!curr->w_alt.on) {
			SaveCursor(&curr->w_alt.cursor);
			EnterAltScreen(curr);
		      }
		    }
		  else
		    {
		      if (curr->w_alt.on) {
		        RestoreCursor(&curr->w_alt.cursor);
		        LeaveAltScreen(curr);
		      }
		    }
		  if (a1 == 47 && !i)
		    curr->w_saved.on = 0;
		  LRefreshAll(&curr->w_layer, 0);
		  LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
		}
	      break;
	    case 1048:
	      if (i)
		SaveCursor(&curr->w_saved);
	      else
		RestoreCursor(&curr->w_saved);
	      break;
	 /* case 66:	   NKM:  Numeric keypad appl mode */
	 /* case 68:	   KBUM: Keyboard usage mode (data process) */
	    case 1000:	/* VT200 mouse tracking */
	    case 1001:	/* VT200 highlight mouse */
	    case 1002:	/* button event mouse*/
	    case 1003:	/* any event mouse*/
	      curr->w_mouse = i ? a1 : 0;
	      LMouseMode(&curr->w_layer, curr->w_mouse);
	      break;
         /* case 1005:     UTF-8 mouse mode rejected */
	    case 1006:  /* SGR mouse mode */
		curr->w_extmouse = i ? a1 : 0;
		LExtMouseMode(&curr->w_layer, curr->w_extmouse);
                break;
	 /* case 1015:     UXRVT mouse mode rejected */
	    }
	}
      break;
    case '>':
      switch (c)
	{
	case 'c':	/* secondary DA */
	  if (a1 == 0)
	    Report("\033[>%d;%d;0c", 83, nversion);	/* 83 == 'S' */
	  break;
	}
      break;
    }
}


static void
StringStart(type)
enum string_t type;
{
  curr->w_StringType = type;
  curr->w_stringp = curr->w_string;
  curr->w_state = ASTR;
}

static void
StringChar(c)
int c;
{
  if (curr->w_stringp >= curr->w_string + MAXSTR - 1)
    curr->w_state = LIT;
  else
    *(curr->w_stringp)++ = c;
}

/*
 * Do string processing. Returns -1 if output should be suspended
 * until status is gone.
 */
static int
StringEnd()
{
  struct canvas *cv;
  char *p;
  int typ;
  char *t;

  /* There's two ways to terminate an OSC. If we've seen an ESC
   * then it's been ST otherwise it's BEL. */
  t = curr->w_state == STRESC ? "\033\\" : "\a";

  curr->w_state = LIT;
  *curr->w_stringp = '\0';
  switch (curr->w_StringType)
    {
    case OSC:	/* special xterm compatibility hack */
      if (curr->w_string[0] == ';' || (p = index(curr->w_string, ';')) == 0)
	break;
      typ = atoi(curr->w_string);
      p++;
#ifdef MULTIUSER
      if (typ == 83)	/* 83 = 'S' */
	{
	  /* special execute commands sequence */
	  char *args[MAXARGS];
	  int argl[MAXARGS];
	  struct acluser *windowuser;

	  windowuser = *FindUserPtr(":window:");
	  if (windowuser && Parse(p, sizeof(curr->w_string) - (p - curr->w_string), args, argl))
	    {
	      for (display = displays; display; display = display->d_next)
		if (D_forecv->c_layer->l_bottom == &curr->w_layer)
		  break;	/* found it */
	      if (display == 0 && curr->w_layer.l_cvlist)
		display = curr->w_layer.l_cvlist->c_display;
	      if (display == 0)
		display = displays;
	      EffectiveAclUser = windowuser;
	      fore = curr;
	      flayer = fore->w_savelayer ? fore->w_savelayer : &fore->w_layer;
	      DoCommand(args, argl);
	      EffectiveAclUser = 0;
	      fore = 0;
	      flayer = 0;
	    }
	  break;
	}
#endif
#ifdef RXVT_OSC
      if (typ == 0 || typ == 1 || typ == 2 || typ == 11 || typ == 20 || typ == 39 || typ == 49)
	{
	  int typ2;
	  typ2 = typ / 10;
	  if (strcmp(curr->w_xtermosc[typ2], p))
	    {
	      if (typ != 11 || strcmp("?", p))
		{
		  strncpy(curr->w_xtermosc[typ2], p, sizeof(curr->w_xtermosc[typ2]) - 1);
		  curr->w_xtermosc[typ2][sizeof(curr->w_xtermosc[typ2]) - 1] = 0;
		}

	      for (display = displays; display; display = display->d_next)
		{
		  if (!D_CXT)
		    continue;
		  if (D_forecv->c_layer->l_bottom == &curr->w_layer)
		    SetXtermOSC(typ2, p, t);
		  if ((typ2 == 3 || typ2 == 4) && D_xtermosc[typ2])
		    Redisplay(0);
		  if (typ == 11 && !strcmp("?", p))
		    break;
		}
	    }
	}
      if (typ != 0 && typ != 2)
	break;
#else
      if (typ < 0 || typ > 2)
	break;
#endif
      
      curr->w_stringp -= p - curr->w_string;
      if (curr->w_stringp > curr->w_string)
	bcopy(p, curr->w_string, curr->w_stringp - curr->w_string);
      *curr->w_stringp = '\0';
      /* FALLTHROUGH */
    case APC:
      if (curr->w_hstatus)
	{
	  if (strcmp(curr->w_hstatus, curr->w_string) == 0)
	    break;	/* not changed */
	  free(curr->w_hstatus);
	  curr->w_hstatus = 0;
	}
      if (curr->w_string != curr->w_stringp)
	curr->w_hstatus = SaveStr(curr->w_string);
      WindowChanged(curr, 'h');
      break;
    case PM:
    case GM:
      for (display = displays; display; display = display->d_next)
	{
	  for (cv = D_cvlist; cv; cv = cv->c_next)
	    if (cv->c_layer->l_bottom == &curr->w_layer)
	      break;
	  if (cv || curr->w_StringType == GM)
	    MakeStatus(curr->w_string);
	}
      return -1;
    case DCS:
      LAY_DISPLAYS(&curr->w_layer, AddStr(curr->w_string));
      break;
    case AKA:
      if (curr->w_title == curr->w_akabuf && !*curr->w_string)
	break;
      if (curr->w_dynamicaka)
	ChangeAKA(curr, curr->w_string, strlen(curr->w_string));
      if (!*curr->w_string)
	curr->w_autoaka = curr->w_y + 1;
      break;
    default:
      break;
    }
  return 0;
}

static void
PrintStart()
{
  curr->w_pdisplay = 0;

  /* find us a nice display to print on, fore preferred */
  display = curr->w_lastdisp;
  if (!(display && curr == D_fore && (printcmd || D_PO)))
    for (display = displays; display; display = display->d_next)
      if (curr == D_fore && (printcmd || D_PO))
        break;
  if (!display)
    {
      struct canvas *cv;
      for (cv = curr->w_layer.l_cvlist; cv; cv = cv->c_lnext)
	{
	  display = cv->c_display;
	  if (printcmd || D_PO)
	    break;
	}
      if (!cv)
	{
	  display = displays;
	  if (!display || display->d_next || !(printcmd || D_PO))
	    return;
	}
    }
  curr->w_pdisplay = display;
  curr->w_stringp = curr->w_string;
  curr->w_state = PRIN;
  if (printcmd && curr->w_pdisplay->d_printfd < 0)
    curr->w_pdisplay->d_printfd = printpipe(curr, printcmd);
}

static void
PrintChar(c)
int c;
{
  if (curr->w_stringp >= curr->w_string + MAXSTR - 1)
    PrintFlush();
  *(curr->w_stringp)++ = c;
}

static void
PrintFlush()
{
  display = curr->w_pdisplay;
  if (display && printcmd)
    {
      char *bp = curr->w_string;
      int len = curr->w_stringp - curr->w_string;
      int r;
      while (len && display->d_printfd >= 0)
	{
	  r = write(display->d_printfd, bp, len);
	  if (r <= 0)
	    {
	      WMsg(curr, errno, "printing aborted");
	      close(display->d_printfd);
	      display->d_printfd = -1;
	      break;
	    }
	  bp += r;
	  len -= r;
	}
    }
  else if (display && curr->w_stringp > curr->w_string)
    {
      AddCStr(D_PO);
      AddStrn(curr->w_string, curr->w_stringp - curr->w_string);
      AddCStr(D_PF);
      Flush(3);
    }
  curr->w_stringp = curr->w_string;
}


void
WNewAutoFlow(win, on)
struct win *win;
int on;
{
  debug1("WNewAutoFlow: %d\n", on);
  if (win->w_flow & FLOW_AUTOFLAG)
    win->w_flow = FLOW_AUTOFLAG | (FLOW_AUTO|FLOW_NOW) * on;
  else
    win->w_flow = (win->w_flow & ~FLOW_AUTO) | FLOW_AUTO * on;
  LSetFlow(&win->w_layer, win->w_flow & FLOW_NOW);
}


#ifdef FONT

static void
DesignateCharset(c, n)
int c, n;
{
  curr->w_ss = 0;
# ifdef ENCODINGS
  if (c == ('@' & 037))		/* map JIS 6226 to 0208 */
    c = KANJI;
# endif
  if (c == 'B')
    c = ASCII;
  if (curr->w_charsets[n] != c)
    {
      curr->w_charsets[n] = c;
      if (curr->w_Charset == n)
	{
	  curr->w_FontL = c;
	  curr->w_rend.font = curr->w_FontL;
	  LSetRendition(&curr->w_layer, &curr->w_rend);
	}
      if (curr->w_CharsetR == n)
        curr->w_FontR = c;
    }
}

static void
MapCharset(n)
int n;
{
  curr->w_ss = 0;
  if (curr->w_Charset != n)
    {
      curr->w_Charset = n;
      curr->w_FontL = curr->w_charsets[n];
      curr->w_rend.font = curr->w_FontL;
      LSetRendition(&curr->w_layer, &curr->w_rend);
    }
}

static void
MapCharsetR(n)
int n;
{
  curr->w_ss = 0;
  if (curr->w_CharsetR != n)
    {
      curr->w_CharsetR = n;
      curr->w_FontR = curr->w_charsets[n];
    }
  curr->w_gr = 1;
}

#endif /* FONT */

static void
SaveCursor(cursor)
struct cursor *cursor;
{
  cursor->on = 1;
  cursor->x = curr->w_x;
  cursor->y = curr->w_y;
  cursor->Rend = curr->w_rend;
#ifdef FONT
  cursor->Charset = curr->w_Charset;
  cursor->CharsetR = curr->w_CharsetR;
  bcopy((char *) curr->w_charsets, (char *) cursor->Charsets,
	4 * sizeof(int));
#endif
}

static void
RestoreCursor(cursor)
struct cursor *cursor;
{
  if (!cursor->on)
    return;
  LGotoPos(&curr->w_layer, cursor->x, cursor->y);
  curr->w_x = cursor->x;
  curr->w_y = cursor->y;
  curr->w_rend = cursor->Rend;
#ifdef FONT
  bcopy((char *) cursor->Charsets, (char *) curr->w_charsets,
	4 * sizeof(int));
  curr->w_Charset = cursor->Charset;
  curr->w_CharsetR = cursor->CharsetR;
  curr->w_ss = 0;
  curr->w_FontL = curr->w_charsets[curr->w_Charset];
  curr->w_FontR = curr->w_charsets[curr->w_CharsetR];
#endif
  LSetRendition(&curr->w_layer, &curr->w_rend);
}

static void
BackSpace()
{
  if (curr->w_x > 0)
    {
      curr->w_x--;
    }
  else if (curr->w_wrap && curr->w_y > 0)
    {
      curr->w_x = cols - 1;
      curr->w_y--;
    }
  LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
}

static void
Return()
{
  if (curr->w_x == 0)
    return;
  curr->w_x = 0;
  LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
}

static void
LineFeed(out_mode)
int out_mode;
{
  /* out_mode: 0=lf, 1=cr+lf */
  if (out_mode)
    curr->w_x = 0;
  if (curr->w_y != curr->w_bot)		/* Don't scroll */
    {
      if (curr->w_y < rows-1)
	curr->w_y++;
      LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
      return;
    }
  if (curr->w_autoaka > 1)
    curr->w_autoaka--;
  MScrollV(curr, 1, curr->w_top, curr->w_bot, CURR_BCE);
  LScrollV(&curr->w_layer, 1, curr->w_top, curr->w_bot, CURR_BCE);
  LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
}

static void
ReverseLineFeed()
{
  if (curr->w_y == curr->w_top)
    {
      MScrollV(curr, -1, curr->w_top, curr->w_bot, CURR_BCE);
      LScrollV(&curr->w_layer, -1, curr->w_top, curr->w_bot, CURR_BCE);
      LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
    }
  else if (curr->w_y > 0)
    CursorUp(1);
}

static void
InsertChar(n)
int n;
{
  register int y = curr->w_y, x = curr->w_x;

  if (n <= 0)
    return;
  if (x == cols)
    x--;
  save_mline(&curr->w_mlines[y], cols);
  MScrollH(curr, -n, y, x, curr->w_width - 1, CURR_BCE);
  LScrollH(&curr->w_layer, -n, y, x, curr->w_width - 1, CURR_BCE, &mline_old);
  LGotoPos(&curr->w_layer, x, y);
}

static void
DeleteChar(n)
int n;
{
  register int y = curr->w_y, x = curr->w_x;

  if (x == cols)
    x--;
  save_mline(&curr->w_mlines[y], cols);
  MScrollH(curr, n, y, x, curr->w_width - 1, CURR_BCE);
  LScrollH(&curr->w_layer, n, y, x, curr->w_width - 1, CURR_BCE, &mline_old);
  LGotoPos(&curr->w_layer, x, y);
}

static void
DeleteLine(n)
int n;
{
  if (curr->w_y < curr->w_top || curr->w_y > curr->w_bot)
    return;
  if (n > curr->w_bot - curr->w_y + 1)
    n = curr->w_bot - curr->w_y + 1;
  MScrollV(curr, n, curr->w_y, curr->w_bot, CURR_BCE);
  LScrollV(&curr->w_layer, n, curr->w_y, curr->w_bot, CURR_BCE);
  LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
}

static void
InsertLine(n)
int n;
{
  if (curr->w_y < curr->w_top || curr->w_y > curr->w_bot)
    return;
  if (n > curr->w_bot - curr->w_y + 1)
    n = curr->w_bot - curr->w_y + 1;
  MScrollV(curr, -n, curr->w_y, curr->w_bot, CURR_BCE);
  LScrollV(&curr->w_layer, -n, curr->w_y, curr->w_bot, CURR_BCE);
  LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
}

static void
ScrollRegion(n)
int n;
{
  MScrollV(curr, n, curr->w_top, curr->w_bot, CURR_BCE);
  LScrollV(&curr->w_layer, n, curr->w_top, curr->w_bot, CURR_BCE);
  LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
}


static void
ForwardTab()
{
  register int x = curr->w_x;

  if (x == cols)
    {
      LineFeed(1);
      x = 0;
    }
  if (curr->w_tabs[x] && x < cols - 1)
    x++;
  while (x < cols - 1 && !curr->w_tabs[x])
    x++;
  curr->w_x = x;
  LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
}

static void
BackwardTab()
{
  register int x = curr->w_x;

  if (curr->w_tabs[x] && x > 0)
    x--;
  while (x > 0 && !curr->w_tabs[x])
    x--;
  curr->w_x = x;
  LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
}

static void
ClearScreen()
{
  LClearArea(&curr->w_layer, 0, 0, curr->w_width - 1, curr->w_height - 1, CURR_BCE, 1);
#ifdef COPY_PASTE
  MScrollV(curr, curr->w_height, 0, curr->w_height - 1, CURR_BCE);
#else
  MClearArea(curr, 0, 0, curr->w_width - 1, curr->w_height - 1, CURR_BCE);
#endif
}

static void
ClearFromBOS()
{
  register int y = curr->w_y, x = curr->w_x;

  LClearArea(&curr->w_layer, 0, 0, x, y, CURR_BCE, 1);
  MClearArea(curr, 0, 0, x, y, CURR_BCE);
  RestorePosRendition();
}

static void
ClearToEOS()
{
  register int y = curr->w_y, x = curr->w_x;

  if (x == 0 && y == 0)
    {
      ClearScreen();
      RestorePosRendition();
      return;
    }
  LClearArea(&curr->w_layer, x, y, cols - 1, rows - 1, CURR_BCE, 1);
  MClearArea(curr, x, y, cols - 1, rows - 1, CURR_BCE);
  RestorePosRendition();
}

static void
ClearLineRegion(from, to)
int from, to;
{
  register int y = curr->w_y;
  LClearArea(&curr->w_layer, from, y, to, y, CURR_BCE, 1);
  MClearArea(curr, from, y, to, y, CURR_BCE);
  RestorePosRendition();
}

static void
CursorRight(n)
register int n;
{
  register int x = curr->w_x;

  if (x == cols)
    {
      LineFeed(1);
      x = 0;
    }
  if ((curr->w_x += n) >= cols)
    curr->w_x = cols - 1;
  LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
}

static void
CursorUp(n)
register int n;
{
  if (curr->w_y < curr->w_top)		/* if above scrolling rgn, */
    {
      if ((curr->w_y -= n) < 0)		/* ignore its limits      */
         curr->w_y = 0;
    }
  else
    if ((curr->w_y -= n) < curr->w_top)
      curr->w_y = curr->w_top;
  LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
}

static void
CursorDown(n)
register int n;
{
  if (curr->w_y > curr->w_bot)		/* if below scrolling rgn, */
    {
      if ((curr->w_y += n) > rows - 1)	/* ignore its limits      */
        curr->w_y = rows - 1;
    }
  else
    if ((curr->w_y += n) > curr->w_bot)
      curr->w_y = curr->w_bot;
  LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
}

static void
CursorLeft(n)
register int n;
{
  if ((curr->w_x -= n) < 0)
    curr->w_x = 0;
  LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
}

static void
ASetMode(on)
int on;
{
  register int i;

  for (i = 0; i < curr->w_NumArgs; ++i)
    {
      switch (curr->w_args[i])
	{
     /* case 2:		   KAM: Lock keyboard */
	case 4:		/* IRM: Insert mode */
	  curr->w_insert = on;
	  LAY_DISPLAYS(&curr->w_layer, InsertMode(on));
	  break;
     /* case 12:	   SRM: Echo mode on */
	case 20:	/* LNM: Linefeed mode */
	  curr->w_autolf = on;
	  break;
	case 34:
	  curr->w_curvvis = !on;
	  LCursorVisibility(&curr->w_layer, curr->w_curinv ? -1 : curr->w_curvvis);
	  break;
	default:
	  break;
	}
    }
}

static char rendlist[] =
{
  ~((1 << NATTR) - 1), A_BD, A_DI, A_SO, A_US, A_BL, 0, A_RV, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, ~(A_BD|A_SO|A_DI), ~A_SO, ~A_US, ~A_BL, 0, ~A_RV
};

static void
SelectRendition()
{
#ifdef COLOR
  register int j, i = 0, a = curr->w_rend.attr, c = curr->w_rend.color;
# ifdef COLORS256
  int cx = curr->w_rend.colorx;
# endif
#else
  register int j, i = 0, a = curr->w_rend.attr;
#endif

  do
    {
      j = curr->w_args[i];
#ifdef COLOR
      if ((j == 38 || j == 48) && i + 2 < curr->w_NumArgs && curr->w_args[i + 1] == 5)
	{
	  int jj;

	  i += 2;
	  jj = curr->w_args[i];
	  if (jj < 0 || jj > 255)
	    continue;
# ifdef COLORS256
	  if (j == 38)
	    {
	      c = (c & 0xf0) | ((jj & 0x0f) ^ 9);
	      a |= A_BFG;
	      if (jj >= 8 && jj < 16)
		c |= 0x08;
	      else
		a ^= A_BFG;
	      a = (a & 0xbf) | (jj & 8 ? 0x40 : 0);
	      cx = (cx & 0xf0) | (jj >> 4 & 0x0f);
	    }
	  else
	    {
	      c = (c & 0x0f) | ((jj & 0x0f) ^ 9) << 4;
	      a |= A_BBG;
	      if (jj >= 8 && jj < 16)
		c |= 0x80;
	      else
		a ^= A_BBG;
	      cx = (cx & 0x0f) | (jj & 0xf0);
	    }
	  continue;
# else
	  jj = color256to16(jj) + 30;
	  if (jj >= 38)
	    jj += 60 - 8;
	  j = j == 38 ? jj : jj + 10;
# endif
	}
# ifdef COLORS16
      if (j == 0 || (j >= 30 && j <= 39 && j != 38))
	a &= 0xbf;
      if (j == 0 || (j >= 40 && j <= 49 && j != 48))
	a &= 0x7f;
      if (j >= 90 && j <= 97)
	a |= 0x40;
      if (j >= 100 && j <= 107)
	a |= 0x80;
# endif
      if (j >= 90 && j <= 97)
	j -= 60;
      if (j >= 100 && j <= 107)
	j -= 60;
      if (j >= 30 && j <= 39 && j != 38)
	c = (c & 0xf0) | ((j - 30) ^ 9);
      else if (j >= 40 && j <= 49 && j != 48)
	c = (c & 0x0f) | (((j - 40) ^ 9) << 4);
      if (j == 0)
	c = 0;
# ifdef COLORS256
      if (j == 0 || (j >= 30 && j <= 39 && j != 38))
	cx &= 0xf0;
      if (j == 0 || (j >= 40 && j <= 49 && j != 48))
	cx &= 0x0f;
# endif
#endif
      if (j < 0 || j >= (int)(sizeof(rendlist)/sizeof(*rendlist)))
	continue;
      j = rendlist[j];
      if (j & (1 << NATTR))
        a &= j;
      else
        a |= j;
    }
  while (++i < curr->w_NumArgs);
  curr->w_rend.attr = a;
#ifdef COLOR
  curr->w_rend.color = c;
# ifdef COLORS256
  curr->w_rend.colorx = cx;
# endif
#endif
  LSetRendition(&curr->w_layer, &curr->w_rend);
}

static void
FillWithEs()
{
  register int i;
  register unsigned char *p, *ep;

  LClearAll(&curr->w_layer, 1);
  curr->w_y = curr->w_x = 0;
  for (i = 0; i < rows; ++i)
    {
      clear_mline(&curr->w_mlines[i], 0, cols + 1);
      p = curr->w_mlines[i].image;
      ep = p + cols;
      while (p < ep)
	*p++ = 'E';
    }
  LRefreshAll(&curr->w_layer, 1);
}


/*
 *  Ugly autoaka hack support:
 *    ChangeAKA() sets a new aka
 *    FindAKA() searches for an autoaka match
 */

void
ChangeAKA(p, s, l)
struct win *p;
char *s;
int l;
{
  int i, c;

  for (i = 0; l > 0; l--)
    {
      if (p->w_akachange + i == p->w_akabuf + sizeof(p->w_akabuf) - 1)
	break;
      c = (unsigned char)*s++;
      if (c == 0)
	break;
      if (c < 32 || c == 127 || (c >= 128 && c < 160 && p->w_c1))
	continue;
      p->w_akachange[i++] = c;
    }
  p->w_akachange[i] = 0;
  p->w_title = p->w_akachange;
  if (p->w_akachange != p->w_akabuf)
    if (p->w_akachange[0] == 0 || p->w_akachange[-1] == ':')
      p->w_title = p->w_akabuf + strlen(p->w_akabuf) + 1;
  WindowChanged(p, 't');
  WindowChanged((struct win *)0, 'w');
  WindowChanged((struct win *)0, 'W');
}

static void
FindAKA()
{
  register unsigned char *cp, *line;
  register struct win *wp = curr;
  register int len = strlen(wp->w_akabuf);
  int y;

  y = (wp->w_autoaka > 0 && wp->w_autoaka <= wp->w_height) ? wp->w_autoaka - 1 : wp->w_y;
  cols = wp->w_width;
 try_line:
  cp = line = wp->w_mlines[y].image;
  if (wp->w_autoaka > 0 &&  *wp->w_akabuf != '\0')
    {
      for (;;)
	{
	  if (cp - line >= cols - len)
	    {
	      if (++y == wp->w_autoaka && y < rows)
		goto try_line;
	      return;
	    }
	  if (strncmp((char *)cp, wp->w_akabuf, len) == 0)
	    break;
	  cp++;
	}
      cp += len;
    }
  for (len = cols - (cp - line); len && *cp == ' '; len--, cp++)
    ;
  if (len)
    {
      if (wp->w_autoaka > 0 && (*cp == '!' || *cp == '%' || *cp == '^'))
	wp->w_autoaka = -1;
      else
	wp->w_autoaka = 0;
      line = cp;
      while (len && *cp != ' ')
	{
	  if (*cp++ == '/')
	    line = cp;
	  len--;
	}
      ChangeAKA(wp, (char *)line, cp - line);
    }
  else
    wp->w_autoaka = 0;
}

static void
RestorePosRendition()
{
  LGotoPos(&curr->w_layer, curr->w_x, curr->w_y);
  LSetRendition(&curr->w_layer, &curr->w_rend);
}

/* Send a terminal report as if it were typed. */ 
static void
Report(fmt, n1, n2)
char *fmt;
int n1, n2;
{
  register int len;
  char rbuf[40];	/* enough room for all replies */

  sprintf(rbuf, fmt, n1, n2);
  len = strlen(rbuf);

#ifdef PSEUDOS
  if (W_UWP(curr))
    {
      if ((unsigned)(curr->w_pwin->p_inlen + len) <= sizeof(curr->w_pwin->p_inbuf))
	{
	  bcopy(rbuf, curr->w_pwin->p_inbuf + curr->w_pwin->p_inlen, len);
	  curr->w_pwin->p_inlen += len;
	}
    }
  else
#endif
    {
      if ((unsigned)(curr->w_inlen + len) <= sizeof(curr->w_inbuf))
	{
	  bcopy(rbuf, curr->w_inbuf + curr->w_inlen, len);
	  curr->w_inlen += len;
	}
    }
}



/*
 *====================================================================*
 *====================================================================*
 */

/**********************************************************************
 *
 * Memory subsystem.
 *
 */

static void
MFixLine(p, y, mc)
struct win *p;
int y;
struct mchar *mc;
{
  struct mline *ml = &p->w_mlines[y];
  if (mc->attr && ml->attr == null)
    {
      if ((ml->attr = (unsigned char *)calloc(p->w_width + 1, 1)) == 0)
	{
	  ml->attr = null;
	  mc->attr = p->w_rend.attr = 0;
	  WMsg(p, 0, "Warning: no space for attr - turned off");
	}
    }
#ifdef FONT
  if (mc->font && ml->font == null)
    {
      if ((ml->font = (unsigned char *)calloc(p->w_width + 1, 1)) == 0)
	{
	  ml->font = null;
	  p->w_FontL = p->w_charsets[p->w_ss ? p->w_ss : p->w_Charset] = 0;
	  p->w_FontR = p->w_charsets[p->w_ss ? p->w_ss : p->w_CharsetR] = 0;
	  mc->font = mc->fontx = p->w_rend.font  = 0;
	  WMsg(p, 0, "Warning: no space for font - turned off");
	}
    }
  if (mc->fontx && ml->fontx == null)
    {
      if ((ml->fontx = (unsigned char *)calloc(p->w_width + 1, 1)) == 0)
	{
	  ml->fontx = null;
	  mc->fontx = 0;
	}
    }
#endif
#ifdef COLOR
  if (mc->color && ml->color == null)
    {
      if ((ml->color = (unsigned char *)calloc(p->w_width + 1, 1)) == 0)
	{
	  ml->color = null;
	  mc->color = p->w_rend.color = 0;
	  WMsg(p, 0, "Warning: no space for color - turned off");
	}
    }
# ifdef COLORS256
  if (mc->colorx && ml->colorx == null)
    {
      if ((ml->colorx = (unsigned char *)calloc(p->w_width + 1, 1)) == 0)
	{
	  ml->colorx = null;
	  mc->colorx = p->w_rend.colorx = 0;
	  WMsg(p, 0, "Warning: no space for extended colors - turned off");
	}
    }
# endif
#endif
}

/*****************************************************************/

#ifdef DW_CHARS
# define MKillDwRight(p, ml, x)					\
  if (dw_right(ml, x, p->w_encoding))				\
    {								\
      if (x > 0)						\
	copy_mchar2mline(&mchar_blank, ml, x - 1);		\
      copy_mchar2mline(&mchar_blank, ml, x);			\
    }

# define MKillDwLeft(p, ml, x)					\
  if (dw_left(ml, x, p->w_encoding))				\
    {								\
      copy_mchar2mline(&mchar_blank, ml, x);			\
      copy_mchar2mline(&mchar_blank, ml, x + 1);		\
    }
#else
# define MKillDwRight(p, ml, x) ;
# define MKillDwLeft(p, ml, x) ;
#endif

static void
MScrollH(p, n, y, xs, xe, bce)
struct win *p;
int n, y, xs, xe, bce;
{
  struct mline *ml;

  if (n == 0)
    return;
  ml = &p->w_mlines[y];
  MKillDwRight(p, ml, xs);
  MKillDwLeft(p, ml, xe);
  if (n > 0)
    {
      if (xe - xs + 1 > n)
	{
	  MKillDwRight(p, ml, xs + n);
	  bcopy_mline(ml, xs + n, xs, xe + 1 - xs - n);
	}
      else
	n = xe - xs + 1;
      clear_mline(ml, xe + 1 - n, n);
#ifdef COLOR
      if (bce)
        MBceLine(p, y, xe + 1 - n, n, bce);
#endif
    }
  else
    {
      n = -n;
      if (xe - xs + 1 > n)
	{
	  MKillDwLeft(p, ml, xe - n);
	  bcopy_mline(ml, xs, xs + n, xe + 1 - xs - n);
	}
      else
	n = xe - xs + 1;
      clear_mline(ml, xs, n);
#ifdef COLOR
      if (bce)
        MBceLine(p, y, xs, n, bce);
#endif
    }
}

static void
MScrollV(p, n, ys, ye, bce)
struct win *p;
int n, ys, ye, bce;
{
  int i, cnt1, cnt2;
  struct mline tmp[256];
  struct mline *ml;

  if (n == 0)
    return;
  if (n > 0)
    {
      if (ye - ys + 1 < n)
	n = ye - ys + 1;
      if (n > 256)
	{
	  MScrollV(p, n - 256, ys, ye, bce);
	  n = 256;
	}
#ifdef COPY_PASTE
      if (compacthist)
	{
	  ye = MFindUsedLine(p, ye, ys);
	  if (ye - ys + 1 < n)
	    n = ye - ys + 1;
	  if (n <= 0)
	    return;
	}
#endif
      /* Clear lines */
      ml = p->w_mlines + ys;
      for (i = ys; i < ys + n; i++, ml++)
	{
#ifdef COPY_PASTE
	  if (ys == p->w_top)
	    WAddLineToHist(p, ml);
#endif
	  if (ml->attr != null)
	    free(ml->attr);
	  ml->attr = null;
#ifdef FONT
	  if (ml->font != null)
	    free(ml->font);
	  ml->font = null;
	  if (ml->fontx != null)
	    free(ml->fontx);
	  ml->fontx = null;
#endif
#ifdef COLOR
	  if (ml->color != null)
	    free(ml->color);
	  ml->color = null;
# ifdef COLORS256
	  if (ml->colorx != null)
	    free(ml->colorx);
	  ml->colorx = null;
# endif
#endif
	  bclear((char *)ml->image, p->w_width + 1);
#ifdef COLOR
	  if (bce)
	    MBceLine(p, i, 0, p->w_width, bce);
#endif
	}
      /* switch 'em over */
      cnt1 = n * sizeof(struct mline);
      cnt2 = (ye - ys + 1 - n) * sizeof(struct mline);
      if (cnt1 && cnt2)
	Scroll((char *)(p->w_mlines + ys), cnt1, cnt2, (char *)tmp);
    }
  else
    {
      n = -n;
      if (ye - ys + 1 < n)
	n = ye - ys + 1;
      if (n > 256)
	{
	  MScrollV(p, - (n - 256), ys, ye, bce);
	  n = 256;
	}

      ml = p->w_mlines + ye;
      /* Clear lines */
      for (i = ye; i > ye - n; i--, ml--)
	{
	  if (ml->attr != null)
	    free(ml->attr);
	  ml->attr = null;
#ifdef FONT
	  if (ml->font != null)
	    free(ml->font);
	  ml->font = null;
	  if (ml->fontx != null)
	    free(ml->fontx);
	  ml->fontx = null;
#endif
#ifdef COLOR
	  if (ml->color != null)
	    free(ml->color);
	  ml->color = null;
# ifdef COLORS256
	  if (ml->colorx != null)
	    free(ml->colorx);
	  ml->colorx = null;
# endif
#endif
	  bclear((char *)ml->image, p->w_width + 1);
#ifdef COLOR
	  if (bce)
	    MBceLine(p, i, 0, p->w_width, bce);
#endif
	}
      cnt1 = n * sizeof(struct mline);
      cnt2 = (ye - ys + 1 - n) * sizeof(struct mline);
      if (cnt1 && cnt2)
	Scroll((char *)(p->w_mlines + ys), cnt2, cnt1, (char *)tmp);
    }
}

static void
Scroll(cp, cnt1, cnt2, tmp)
char *cp, *tmp;
int cnt1, cnt2;
{
  if (!cnt1 || !cnt2)
    return;
  if (cnt1 <= cnt2)
    {
      bcopy(cp, tmp, cnt1);
      bcopy(cp + cnt1, cp, cnt2);
      bcopy(tmp, cp + cnt2, cnt1);
    }
  else
    {
      bcopy(cp + cnt1, tmp, cnt2);
      bcopy(cp, cp + cnt2, cnt1);
      bcopy(tmp, cp, cnt2);
    }
}

static void
MClearArea(p, xs, ys, xe, ye, bce)
struct win *p;
int xs, ys, xe, ye, bce;
{
  int n, y;
  int xxe;
  struct mline *ml;

  /* Check for zero-height window */
  if (ys < 0 || ye < ys)
    return;

  /* check for magic margin condition */
  if (xs >= p->w_width)
    xs = p->w_width - 1;
  if (xe >= p->w_width)
    xe = p->w_width - 1;

  MKillDwRight(p, p->w_mlines + ys, xs);
  MKillDwLeft(p, p->w_mlines + ye, xe);

  ml = p->w_mlines + ys;
  for (y = ys; y <= ye; y++, ml++)
    {
      xxe = (y == ye) ? xe : p->w_width - 1;
      n = xxe - xs + 1;
      if (n > 0)
	clear_mline(ml, xs, n);
#ifdef COLOR
      if (n > 0 && bce)
	MBceLine(p, y, xs, xs + n - 1, bce);
#endif
      xs = 0;
    }
}

static void
MInsChar(p, c, x, y)
struct win *p;
struct mchar *c;
int x, y;
{
  int n;
  struct mline *ml;

  ASSERT(x >= 0 && x < p->w_width);
  MFixLine(p, y, c);
  ml = p->w_mlines + y;
  n = p->w_width - x - 1;
  MKillDwRight(p, ml, x);
  if (n > 0)
    {
      MKillDwRight(p, ml, p->w_width - 1);
      bcopy_mline(ml, x, x + 1, n);
    }
  copy_mchar2mline(c, ml, x);
#ifdef DW_CHARS
  if (c->mbcs)
    {
      if (--n > 0)
        {
          MKillDwRight(p, ml, p->w_width - 1);
	  bcopy_mline(ml, x + 1, x + 2, n);
	}
      copy_mchar2mline(c, ml, x + 1);
      ml->image[x + 1] = c->mbcs;
# ifdef UTF8
      if (p->w_encoding != UTF8)
	ml->font[x + 1] |= 0x80;
      else if (p->w_encoding == UTF8 && c->mbcs)
	{
	  ml->font[x + 1] = c->mbcs;
	  ml->fontx[x + 1] = 0;
	}
# else
      ml->font[x + 1] |= 0x80;
# endif
    }
#endif
}

static void
MPutChar(p, c, x, y)
struct win *p;
struct mchar *c;
int x, y;
{
  struct mline *ml;

  MFixLine(p, y, c);
  ml = &p->w_mlines[y];
  MKillDwRight(p, ml, x);
  MKillDwLeft(p, ml, x);
  copy_mchar2mline(c, ml, x);
#ifdef DW_CHARS
  if (c->mbcs)
    {
      MKillDwLeft(p, ml, x + 1);
      copy_mchar2mline(c, ml, x + 1);
      ml->image[x + 1] = c->mbcs;
# ifdef UTF8
      if (p->w_encoding != UTF8)
	ml->font[x + 1] |= 0x80;
      else if (p->w_encoding == UTF8 && c->mbcs)
	{
	  ml->font[x + 1] = c->mbcs;
	  ml->fontx[x + 1] = 0;
	}
# else
      ml->font[x + 1] |= 0x80;
# endif
    }
#endif
}


static void
MWrapChar(p, c, y, top, bot, ins)
struct win *p;
struct mchar *c;
int y, top, bot;
int ins;
{
  struct mline *ml;
  int bce;

#ifdef COLOR
  bce = rend_getbg(c);
#else
  bce = 0;
#endif
  MFixLine(p, y, c);
  ml = &p->w_mlines[y];
  copy_mchar2mline(&mchar_null, ml, p->w_width);
  if (y == bot)
    MScrollV(p, 1, top, bot, bce);
  else if (y < p->w_height - 1)
    y++;
  if (ins)
    MInsChar(p, c, 0, y);
  else
    MPutChar(p, c, 0, y);
}

static void
MPutStr(p, s, n, r, x, y)
struct win *p;
char *s;
int n;
struct mchar *r;
int x, y;
{
  struct mline *ml;
  int i;
  unsigned char *b;

  if (n <= 0)
    return;
  MFixLine(p, y, r);
  ml = &p->w_mlines[y];
  MKillDwRight(p, ml, x);
  MKillDwLeft(p, ml, x + n - 1);
  bcopy(s, (char *)ml->image + x, n);
  if (ml->attr != null)
    {
      b = ml->attr + x;
      for (i = n; i-- > 0;)
	*b++ = r->attr;
    }
#ifdef FONT
  if (ml->font != null)
    {
      b = ml->font + x;
      for (i = n; i-- > 0;)
	*b++ = r->font;
    }
  if (ml->fontx != null)
    {
      b = ml->fontx + x;
      for (i = n; i-- > 0;)
	*b++ = r->fontx;
    }
#endif
#ifdef COLOR
  if (ml->color != null)
    {
      b = ml->color + x;
      for (i = n; i-- > 0;)
	*b++ = r->color;
    }
# ifdef COLORS256
  if (ml->colorx != null)
    {
      b = ml->colorx + x;
      for (i = n; i-- > 0;)
	*b++ = r->colorx;
    }
# endif
#endif
}

#ifdef COLOR
static void
MBceLine(p, y, xs, xe, bce)
struct win *p;
int y, xs, xe, bce;
{
  struct mchar mc;
  struct mline *ml;
  int x;

  mc = mchar_null;
  rend_setbg(&mc, bce);
  MFixLine(p, y, &mc);
  ml = p->w_mlines + y;
# ifdef COLORS16
  if (mc.attr)
    for (x = xs; x <= xe; x++)
      ml->attr[x] = mc.attr;
# endif
  if (mc.color)
    for (x = xs; x <= xe; x++)
      ml->color[x] = mc.color;
# ifdef COLORS256
  if (mc.colorx)
    for (x = xs; x <= xe; x++)
      ml->colorx[x] = mc.colorx;
# endif
}
#endif


#ifdef COPY_PASTE
static void
WAddLineToHist(wp, ml)
struct win *wp;
struct mline *ml;
{
  register unsigned char *q, *o;
  struct mline *hml;

  if (wp->w_histheight == 0)
    return;
  hml = &wp->w_hlines[wp->w_histidx];
  q = ml->image; ml->image = hml->image; hml->image = q;

  q = ml->attr; o = hml->attr; hml->attr = q; ml->attr = null;
  if (o != null)
    free(o);
 
#ifdef FONT
  q = ml->font; o = hml->font; hml->font = q; ml->font = null;
  if (o != null)
    free(o);
  q = ml->fontx; o = hml->fontx; hml->fontx = q; ml->fontx = null;
  if (o != null)
    free(o);
#endif

#ifdef COLOR
  q = ml->color; o = hml->color; hml->color = q; ml->color = null;
  if (o != null)
    free(o);
# ifdef COLORS256
  q = ml->colorx; o = hml->colorx; hml->colorx = q; ml->colorx = null;
  if (o != null)
    free(o);
# endif
#endif

  if (++wp->w_histidx >= wp->w_histheight)
    wp->w_histidx = 0;
  if (wp->w_scrollback_height < wp->w_histheight)
    ++wp->w_scrollback_height;
}
#endif

int
MFindUsedLine(p, ye, ys)
struct win *p;
int ys, ye;
{
  int y;
  struct mline *ml = p->w_mlines + ye;

  debug2("MFindUsedLine: %d %d\n", ye, ys);
  for (y = ye; y >= ys; y--, ml--)
    {
      if (bcmp((char*)ml->image, blank, p->w_width))
	break;
      if (ml->attr != null && bcmp((char*)ml->attr, null, p->w_width))
	break;
#ifdef COLOR
      if (ml->color != null && bcmp((char*)ml->color, null, p->w_width))
	break;
# ifdef COLORS256
      if (ml->colorx != null && bcmp((char*)ml->colorx, null, p->w_width))
	break;
# endif
#endif
#ifdef UTF8
      if (p->w_encoding == UTF8)
	{
	  if (ml->font != null && bcmp((char*)ml->font, null, p->w_width))
	    break;
	  if (ml->fontx != null && bcmp((char*)ml->fontx, null, p->w_width))
	    break;
	}
#endif
    }
  debug1("MFindUsedLine returning  %d\n", y);
  return y;
}


/*
 *====================================================================*
 *====================================================================*
 */

/*
 * Tricky: send only one bell even if the window is displayed
 * more than once.
 */
void
WBell(p, visual)
struct win *p;
int visual;
{
  struct canvas *cv;
  if (displays == NULL)
    p->w_bell = BELL_DONE;
  for (display = displays; display; display = display->d_next)
    {
      for (cv = D_cvlist; cv; cv = cv->c_next)
	if (cv->c_layer->l_bottom == &p->w_layer)
	  break;
      if (cv && !visual)
	AddCStr(D_BL);
      else if (cv && D_VB)
	AddCStr(D_VB);
      else
        p->w_bell = visual ? BELL_VISUAL : BELL_FOUND;
    }
}

/*
 * This should be reverse video.
 * Only change video if window is fore.
 * Because it is used in some termcaps to emulate
 * a visual bell we do this hack here.
 * (screen uses \Eg as special vbell sequence)
 */
static void
WReverseVideo(p, on)
struct win *p;
int on;
{
  struct canvas *cv;
  for (cv = p->w_layer.l_cvlist; cv; cv = cv->c_lnext)
    {
      display = cv->c_display;
      if (cv != D_forecv)
	continue;
      ReverseVideo(on);
      if (!on && p->w_revvid && !D_CVR)
	{
	  if (D_VB)
	    AddCStr(D_VB);
	  else
	    p->w_bell = BELL_VISUAL;
	}
    }
}

void
WMsg(p, err, str)
struct win *p;
int err;
char *str;
{
  extern struct layer *flayer;
  struct layer *oldflayer = flayer;
  flayer = &p->w_layer;
  LMsg(err, "%s", str);
  flayer = oldflayer;
}

void
WChangeSize(p, w, h)
struct win *p;
int w, h;
{
  int wok = 0;
  struct canvas *cv;

  if (p->w_layer.l_cvlist == 0)
    {
      /* window not displayed -> works always */
      ChangeWindowSize(p, w, h, p->w_histheight);
      return;
    }
  for (cv = p->w_layer.l_cvlist; cv; cv = cv->c_lnext)
    {
      display = cv->c_display;
      if (p != D_fore)
	continue;		/* change only fore */
      if (D_CWS)
	break;
      if (D_CZ0 && (w == Z0width || w == Z1width))
	wok = 1;
    }
  if (cv == 0 && wok == 0)	/* can't change any display */
    return;
  if (!D_CWS)
    h = p->w_height;
  ChangeWindowSize(p, w, h, p->w_histheight);
  for (display = displays; display; display = display->d_next)
    {
      if (p == D_fore)
	{
	  if (D_cvlist && D_cvlist->c_next == 0)
	    ResizeDisplay(w, h);
	  else
	    ResizeDisplay(w, D_height);
	  ResizeLayersToCanvases();	/* XXX Hmm ? */
	  continue;
	}
      for (cv = D_cvlist; cv; cv = cv->c_next)
	if (cv->c_layer->l_bottom == &p->w_layer)
	  break;
      if (cv)
	Redisplay(0);
    }
}

static int
WindowChangedCheck(s, what, hp)
char *s;
int what;
int *hp;
{
  int h = 0;
  int l;
  while(*s)
    {
      if (*s++ != (hp ? '%' : '\005'))
	continue;
      l = 0;
      s += (*s == '+');
      s += (*s == '-');
      while (*s >= '0' && *s <= '9')
	s++;
      if (*s == 'L')
	{
	  s++;
	  l = 0x100;
	}
      if (*s == 'h')
	h = 1;
      if (*s == what || ((*s | l) == what) || what == 'd')
	break;
      if (*s)
	s++;
    }
  if (hp)
    *hp = h;
  return *s ? 1 : 0;
}

void
WindowChanged(p, what)
struct win *p;
int what;
{
  int inwstr, inhstr, inlstr;
  int inwstrh = 0, inhstrh = 0, inlstrh = 0;
  int got, ox, oy;
  struct display *olddisplay = display;
  struct canvas *cv;

  inwstr = inhstr = 0;

  if (what == 'f')
    {
      WindowChanged((struct win *)0, 'w'|0x100);
      WindowChanged((struct win *)0, 'W'|0x100);
    }

  if (what)
    {
      inwstr = WindowChangedCheck(captionstring, what, &inwstrh);
      inhstr = WindowChangedCheck(hstatusstring, what, &inhstrh);
      inlstr = WindowChangedCheck(wliststr, what, &inlstrh);
    }
  else
    {
      inwstr = inhstr = 0;
      inlstr = 1;
    }

  if (p == 0)
    {
      for (display = displays; display; display = display->d_next)
	{
	  ox = D_x;
	  oy = D_y;
	  for (cv = D_cvlist; cv; cv = cv->c_next)
	    {
	      if (inlstr || (inlstrh && p && p->w_hstatus && *p->w_hstatus && WindowChangedCheck(p->w_hstatus, what, (int *)0)))
		WListUpdatecv(cv, (struct win *)0);
	      p = Layer2Window(cv->c_layer);
	      if (inwstr || (inwstrh && p && p->w_hstatus && *p->w_hstatus && WindowChangedCheck(p->w_hstatus, what, (int *)0)))
	        if (cv->c_ye + 1 < D_height)
		  RefreshLine(cv->c_ye + 1, 0, D_width - 1, 0);
	    }
	  p = D_fore;
	  if (inhstr || (inhstrh && p && p->w_hstatus && *p->w_hstatus && WindowChangedCheck(p->w_hstatus, what, (int *)0)))
	    RefreshHStatus();
	  if (ox != -1 && oy != -1)
	    GotoPos(ox, oy);
	}
      display = olddisplay;
      return;
    }

  if (p->w_hstatus && *p->w_hstatus && (inwstrh || inhstrh || inlstrh) && WindowChangedCheck(p->w_hstatus, what, (int *)0))
    {
      inwstr |= inwstrh;
      inhstr |= inhstrh;
      inlstr |= inlstrh;
    }
  if (!inwstr && !inhstr && !inlstr)
    return;
  for (display = displays; display; display = display->d_next)
    {
      got = 0;
      ox = D_x;
      oy = D_y;
      for (cv = D_cvlist; cv; cv = cv->c_next)
	{
	  if (inlstr)
	    WListUpdatecv(cv, p);
	  if (Layer2Window(cv->c_layer) != p)
	    continue;
	  got = 1;
	  if (inwstr && cv->c_ye + 1 < D_height)
	    RefreshLine(cv->c_ye + 1, 0, D_width - 1, 0);
	}
      if (got && inhstr && p == D_fore)
	RefreshHStatus();
      if (ox != -1 && oy != -1)
	GotoPos(ox, oy);
    }
  display = olddisplay;
}

