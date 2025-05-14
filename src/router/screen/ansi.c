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

#include "config.h"

#include "ansi.h"

#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "screen.h"

#include "encoding.h"
#include "fileio.h"
#include "help.h"
#include "logfile.h"
#include "mark.h"
#include "misc.h"
#include "process.h"
#include "resize.h"
#include "winmsg.h"

/* widths for Z0/Z1 switching */
const int Z0width = 132;
const int Z1width = 80;

bool use_altscreen = false;	/* enable alternate screen support? */
bool use_hardstatus = true;	/* display status line in hs */
bool visual_bell = 0;

char *printcmd = NULL;

uint32_t *blank;		/* line filled with spaces */
uint32_t *null;			/* line filled with '\0' */

struct mline mline_old;
struct mline mline_blank;
struct mline mline_null;

struct mchar mchar_null;
struct mchar mchar_blank = { ' ', 0, 0, 0, 0, 0 };
struct mchar mchar_so = { ' ', A_RV, 0, 0, 0, 0};

uint64_t renditions[NUM_RENDS] = { 65529 /* =ub */ , 65531 /* =b */ , 65533 /* =u */  };

/* keep string_t and string_t_string in sync! */
static char *string_t_string[] = {
	"NONE",
	"DCS",			/* Device control string */
	"OSC",			/* Operating system command */
	"APC",			/* Application program command */
	/*  - used for status change */
	"PM",			/* Privacy message */
	"AKA",			/* title for current screen */
	"GM",			/* Global message to every display */
	"STATUS"		/* User hardstatus line */
};

/* keep state_t and state_t_string in sync! */
static char *state_t_string[] = {
	"LIT",			/* Literal input */
	"ESC",			/* Start of escape sequence */
	"ASTR",			/* Start of control string */
	"STRESC",		/* ESC seen in control string */
	"CSI",			/* Reading arguments in "CSI Pn ;... */
	"PRIN",			/* Printer mode */
	"PRINESC",		/* ESC seen in printer mode */
	"PRINCSI",		/* CSI seen in printer mode */
	"PRIN4"			/* CSI 4 seen in printer mode */
};

static int Special(Window *,int);
static void DoESC(Window *, int, int);
static void DoCSI(Window *, int, int);
static void StringStart(Window *, enum string_t);
static void StringChar(Window *, int);
static int StringEnd(Window *);
static void PrintStart(Window *);
static void PrintChar(Window *, int);
static void PrintFlush(Window *);
static void DesignateCharset(Window *, int, int);
static void MapCharset(Window *, int);
static void MapCharsetR(Window *, int);
static void SaveCursor(Window *, struct cursor *);
static void RestoreCursor(Window *, struct cursor *);
static void BackSpace(Window *);
static void Return(Window *);
static void LineFeed(Window *, int);
static void ReverseLineFeed(Window *);
static void InsertChar(Window *, int);
static void DeleteChar(Window *, int);
static void DeleteLine(Window *, int);
static void InsertLine(Window *, int);
static void Scroll(char *, int, int, char *);
static void ForwardTab(Window *win);
static void BackwardTab(Window *win);
static void ClearScreen(Window *win);
static void ClearFromBOS(Window *win);
static void ClearToEOS(Window *win);
static void ClearLineRegion(Window *, int, int);
static void CursorRight(Window *, int);
static void CursorUp(Window *, int);
static void CursorDown(Window *, int);
static void CursorLeft(Window *, int);
static void ASetMode(Window *win, bool);
static void SelectRendition(Window *win);
static void RestorePosRendition(Window *);
static void FillWithEs(Window *);
static void FindAKA(Window *);
static void Report(Window *, char *, int, int);
static void ScrollRegion(Window *win, int);
static void WAddLineToHist(Window *, struct mline *);
static void WLogString(Window *, char *, size_t);
static void WReverseVideo(Window *, bool);
static void MFixLine(Window *, int, struct mchar *);
static void MScrollH(Window *, int, int, int, int, int);
static void MScrollV(Window *, int, int, int, int);
static void MClearArea(Window *, int, int, int, int, int);
static void MInsChar(Window *, struct mchar *, int, int);
static void MPutChar(Window *, struct mchar *, int, int);
static void MWrapChar(Window *, struct mchar *, int, int, int, bool);
static void MBceLine(Window *, int, int, int, int);
static void WChangeSize(Window *, int, int);

void ResetAnsiState(Window *win)
{
	win->w_state = LIT;
	win->w_StringType = NONE;
}

/* adds max 22 bytes */
int GetAnsiStatus(Window *win, char *buf)
{
	char *p = buf;

	if (win->w_state == LIT)
		return 0;

	strcpy(p, state_t_string[win->w_state]);
	p += strlen(p);
	if (win->w_intermediate) {
		*p++ = '-';
		if (win->w_intermediate > 0xff)
			p += AddXChar(p, win->w_intermediate >> 8);
		p += AddXChar(p, win->w_intermediate & 0xff);
		*p = 0;
	}
	if (win->w_state == ASTR || win->w_state == STRESC)
		sprintf(p, "-%s", string_t_string[win->w_StringType]);
	p += strlen(p);
	return p - buf;
}

void ResetCharsets(Window *win)
{
	win->w_gr = nwin_default.gr;
	win->w_c1 = nwin_default.c1;
	SetCharsets(win, "BBBB02");
	if (nwin_default.charset)
		SetCharsets(win, nwin_default.charset);
	ResetEncoding(win);
}

void SetCharsets(Window *win, char *s)
{
	for (int i = 0; i < 4 && *s; i++, s++)
		if (*s != '.')
			win->w_charsets[i] = ((*s == 'B') ? ASCII : *s);
	if (*s && *s++ != '.')
		win->w_Charset = s[-1] - '0';
	if (*s && *s != '.')
		win->w_CharsetR = *s - '0';
	win->w_ss = 0;
	win->w_FontL = win->w_charsets[win->w_Charset];
	win->w_FontR = win->w_charsets[win->w_CharsetR];
}

/*****************************************************************/

/*
 *  Here comes the vt100 emulator
 *  - writes logfiles,
 *  - sets timestamp and flags activity in window.
 *  - record program output in window scrollback
 *  - translate program output for the display and put it into the obuf.
 *
 */
void WriteString(Window *win, char *buf, size_t len)
{
	int c;
	int font;
	Canvas *cv;

	if (len == 0)
		return;
	if (win->w_log)
		WLogString(win, buf, len);

	if (win->w_silence)
		SetTimeout(&win->w_silenceev, win->w_silencewait * 1000);

	if (win->w_monitor == MON_ON) {
		win->w_monitor = MON_FOUND;
	}

	if (win->w_width > 0 && win->w_height > 0) {
		do {
			c = (unsigned char)*buf++;
			if (!win->w_mbcs)
				win->w_rend.font = win->w_FontL;	/* Default: GL */

			if (win->w_encoding == UTF8) {
				c = FromUtf8(c, &win->w_decodestate);
				if (c == -1)
					continue;
				if (c == -2) {
					c = UCS_REPL;
					/* try char again */
					buf--;
					len++;
				}
			}

 tryagain:
			switch (win->w_state) {
			case PRIN:
				switch (c) {
				case '\033':
					win->w_state = PRINESC;
					break;
				default:
					PrintChar(win, c);
				}
				break;
			case PRINESC:
				switch (c) {
				case '[':
					win->w_state = PRINCSI;
					break;
				default:
					PrintChar(win, '\033');
					PrintChar(win, c);
					win->w_state = PRIN;
				}
				break;
			case PRINCSI:
				switch (c) {
				case '4':
					win->w_state = PRIN4;
					break;
				default:
					PrintChar(win, '\033');
					PrintChar(win, '[');
					PrintChar(win, c);
					win->w_state = PRIN;
				}
				break;
			case PRIN4:
				switch (c) {
				case 'i':
					win->w_state = LIT;
					PrintFlush(win);
					if (win->w_pdisplay && win->w_pdisplay->d_printfd >= 0) {
						close(win->w_pdisplay->d_printfd);
						win->w_pdisplay->d_printfd = -1;
					}
					win->w_pdisplay = NULL;
					break;
				default:
					PrintChar(win, '\033');
					PrintChar(win, '[');
					PrintChar(win, '4');
					PrintChar(win, c);
					win->w_state = PRIN;
				}
				break;
			case ASTR:
				if (c == 0)
					break;
				if (c == '\033') {
					win->w_state = STRESC;
					break;
				}
				/* special xterm hack: accept SetStatus sequence. Yucc! */
				/* allow ^E for title escapes */
				if (!(win->w_StringType == OSC && c < ' ' && c != '\005'))
					if (!win->w_c1 || c != ('\\' ^ 0xc0)) {
						StringChar(win, c);
						break;
					}
				c = '\\';
				/* FALLTHROUGH */
			case STRESC:
				switch (c) {
				case '\\':
					if (StringEnd(win) == 0 || len <= 1)
						break;
					/* check if somewhere a status is displayed */
					for (cv = win->w_layer.l_cvlist; cv; cv = cv->c_lnext) {
						display = cv->c_display;
						if (D_status == STATUS_ON_WIN)
							break;
					}
					if (cv) {
						if (len > IOSIZE + 1)
							len = IOSIZE + 1;
						win->w_outlen = len - 1;
						memmove(win->w_outbuf, buf, len - 1);
						return;	/* wait till status is gone */
					}
					break;
				case '\033':
					StringChar(win, '\033');
					break;
				default:
					win->w_state = ASTR;
					StringChar(win, '\033');
					StringChar(win, c);
					break;
				}
				break;
			case ESC:
				switch (c) {
				case '[':
					win->w_NumArgs = 0;
					win->w_intermediate = 0;
					memset((char *)win->w_args, 0, MAXARGS * sizeof(int));
					win->w_state = CSI;
					break;
				case ']':
					StringStart(win, OSC);
					break;
				case '_':
					StringStart(win, APC);
					break;
				case 'P':
					StringStart(win, DCS);
					break;
				case '^':
					StringStart(win, PM);
					break;
				case '!':
					StringStart(win, GM);
					break;
				case '"':
				case 'k':
					StringStart(win, AKA);
					break;
				default:
					if (Special(win, c)) {
						win->w_state = LIT;
						break;
					}
					if (c >= ' ' && c <= '/') {
						if (win->w_intermediate) {
							if (win->w_intermediate == '$')
								c |= '$' << 8;
							else
								c = -1;
						}
						win->w_intermediate = c;
					} else if (c >= '0' && c <= '~') {
						DoESC(win, c, win->w_intermediate);
						win->w_state = LIT;
					} else {
						win->w_state = LIT;
						goto tryagain;
					}
				}
				break;
			case CSI:
				switch (c) {
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
					if (win->w_NumArgs >= 0 && win->w_NumArgs < MAXARGS) {
						if (win->w_args[win->w_NumArgs] < 100000000)
							win->w_args[win->w_NumArgs] =
							    10 * win->w_args[win->w_NumArgs] + (c - '0');
					}
					break;
				case ';':
				case ':':
					if (win->w_NumArgs < MAXARGS)
						win->w_NumArgs++;
					break;
				default:
					if (Special(win, c))
						break;
					if (c >= '@' && c <= '~') {
						if (win->w_NumArgs < MAXARGS)
							win->w_NumArgs++;
						DoCSI(win, c, win->w_intermediate);
						if (win->w_state != PRIN)
							win->w_state = LIT;
					} else if ((c >= ' ' && c <= '/') || (c >= '<' && c <= '?'))
						win->w_intermediate = win->w_intermediate ? -1 : c;
					else {
						win->w_state = LIT;
						goto tryagain;
					}
				}
				break;
			case LIT:
			default:
				if (win->w_mbcs)
					if (c <= ' ' || c == 0x7f || (c >= 0x80 && c < 0xa0 && win->w_c1))
						win->w_mbcs = 0;
				if (c < ' ') {
					if (c == '\033') {
						win->w_intermediate = 0;
						win->w_state = ESC;
						if (win->w_autoaka < 0)
							win->w_autoaka = 0;
					} else
						Special(win, c);
					break;
				}
				if (c >= 0x80 && c < 0xa0 && win->w_c1)
					if ((win->w_FontR & 0xf0) != 0x20 || win->w_encoding == UTF8) {
						switch (c) {
						case 0xc0 ^ 'D':
						case 0xc0 ^ 'E':
						case 0xc0 ^ 'H':
						case 0xc0 ^ 'M':
						case 0xc0 ^ 'N':	/* SS2 */
						case 0xc0 ^ 'O':	/* SS3 */
							DoESC(win, c ^ 0xc0, 0);
							break;
						case 0xc0 ^ '[':
							if (win->w_autoaka < 0)
								win->w_autoaka = 0;
							win->w_NumArgs = 0;
							win->w_intermediate = 0;
							memset((char *)win->w_args, 0, MAXARGS * sizeof(int));
							win->w_state = CSI;
							break;
						case 0xc0 ^ 'P':
							StringStart(win, DCS);
							break;
						default:
							break;
						}
						break;
					}

				if (!win->w_mbcs) {
					if (c < 0x80 || win->w_gr == 0)
						win->w_rend.font = win->w_FontL;
					else if (win->w_gr == 2 && !win->w_ss)
						win->w_rend.font = win->w_FontE;
					else
						win->w_rend.font = win->w_FontR;
				}
				if (win->w_encoding == UTF8) {
					if (win->w_rend.font == '0') {
						struct mchar mc, *mcp;

						mc.image = c;
						mc.mbcs = 0;
						mc.font = '0';
						mcp = recode_mchar(&mc, 0, UTF8);
						c = mcp->image | mcp->font << 8;
					}
					win->w_rend.font = 0;
				}
				if (win->w_encoding == UTF8 && utf8_isdouble(c))
					win->w_mbcs = 0xff;
				if (win->w_encoding == UTF8 && c >= 0x0300 && utf8_iscomb(c)) {
					int ox, oy;
					struct mchar omc;

					ox = win->w_x - 1;
					oy = win->w_y;
					if (ox < 0) {
						ox = win->w_width - 1;
						oy--;
					}
					if (oy < 0)
						oy = 0;
					copy_mline2mchar(&omc, &win->w_mlines[oy], ox);
					if (omc.image == 0xff && omc.font == 0xff) {
						ox--;
						if (ox >= 0) {
							copy_mline2mchar(&omc, &win->w_mlines[oy], ox);
							omc.mbcs = 0xff;
						}
					}
					if (ox >= 0) {
						utf8_handle_comb(c, &omc);
						MFixLine(win, oy, &omc);
						copy_mchar2mline(&omc, &win->w_mlines[oy], ox);
						LPutChar(&win->w_layer, &omc, ox, oy);
						LGotoPos(&win->w_layer, win->w_x, win->w_y);
					}
					break;
				}
				font = win->w_rend.font;
				if (font == KANA && win->w_encoding == SJIS && win->w_mbcs == 0) {
					/* Lets see if it is the first byte of a kanji */
					if ((0x81 <= c && c <= 0x9f) || (0xe0 <= c && c <= 0xef)) {
						win->w_mbcs = c;
						break;
					}
				}
				if (font == 031 && c == 0x80 && !win->w_mbcs)
					font = win->w_rend.font = 0;
				if (is_dw_font(font) && c == ' ')
					font = win->w_rend.font = 0;
				if (is_dw_font(font) || win->w_mbcs) {
					int t = c;
					if (win->w_mbcs == 0) {
						win->w_mbcs = c;
						break;
					}
					if (win->w_x == win->w_width - 1) {
						win->w_x += win->w_wrap ? true : false;
					}
					if (win->w_encoding != UTF8) {
						c = win->w_mbcs;
						if (font == KANA && win->w_encoding == SJIS) {
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
							if (0x40 <= t && t <= 0xfc && t != 0x7f) {
								if (c <= 0x9f)
									c = (c - 0x81) * 2 + 0x21;
								else
									c = (c - 0xc1) * 2 + 0x21;
								if (t <= 0x7e)
									t -= 0x1f;
								else if (t <= 0x9e)
									t -= 0x20;
								else
									t -= 0x7e, c++;
								win->w_rend.font = KANJI;
							} else {
								/* Incomplete shift-jis - skip first byte */
								c = t;
								t = 0;
							}
						}
						if (t && win->w_gr && font != 030 && font != 031) {
							t &= 0x7f;
							if (t < ' ')
								goto tryagain;
						}
						if (t == '\177')
							break;
						win->w_mbcs = t;
					}
				}
				if (font == '<' && c >= ' ') {
					win->w_rend.font = 0;
					c |= 0x80;
				} else if (win->w_gr && win->w_encoding != UTF8) {
					if (c == 0x80 && font == 0 && win->w_encoding == GBK)
						c = 0xa4;
					else
						c &= 0x7f;
					if (c < ' ' && font != 031)
						goto tryagain;
				}
				if (c == '\177')
					break;
				win->w_rend.image = c;
				if (win->w_encoding == UTF8) {
					win->w_rend.font = c >> 8;
				}
				win->w_rend.mbcs = win->w_mbcs;
				if (win->w_x < win->w_width - 1) {
					if (win->w_insert) {
						save_mline(&win->w_mlines[win->w_y], win->w_width);
						MInsChar(win, &win->w_rend, win->w_x, win->w_y);
						LInsChar(&win->w_layer, &win->w_rend, win->w_x, win->w_y,
							 &mline_old);
						win->w_x++;
					} else {
						MPutChar(win, &win->w_rend, win->w_x, win->w_y);
						LPutChar(&win->w_layer, &win->w_rend, win->w_x, win->w_y);
						win->w_x++;
					}
				} else if (win->w_x == win->w_width - 1) {
					MPutChar(win, &win->w_rend, win->w_x, win->w_y);
					LPutChar(&win->w_layer, &win->w_rend, win->w_x, win->w_y);
					if (win->w_wrap)
						win->w_x++;
				} else {
					MWrapChar(win, &win->w_rend, win->w_y, win->w_top, win->w_bot,
						  win->w_insert);
					LWrapChar(&win->w_layer, &win->w_rend, win->w_y, win->w_top, win->w_bot,
						  win->w_insert);
					if (win->w_y != win->w_bot && win->w_y != win->w_height - 1)
						win->w_y++;
					win->w_x = 1;
				}
				if (win->w_mbcs) {
					win->w_rend.mbcs = win->w_mbcs = 0;
					win->w_x++;
				}
				if (win->w_ss) {
					win->w_FontL = win->w_charsets[win->w_Charset];
					win->w_FontR = win->w_charsets[win->w_CharsetR];
					win->w_rend.font = win->w_FontL;
					LSetRendition(&win->w_layer, &win->w_rend);
					win->w_ss = 0;
				}
				break;
			}
		}
		while (--len);
	}
	if (!printcmd && win->w_state == PRIN)
		PrintFlush(win);
}

static void WLogString(Window *win, char *buf, size_t len)
{
	if (!win->w_log)
		return;
	if (logtstamp_on && win->w_logsilence >= logtstamp_after * 2) {
		char *t = MakeWinMsg(logtstamp_string, win, '%');
		logfwrite(win->w_log, t, strlen(t));	/* long time no write */
	}
	win->w_logsilence = 0;
	if (logfwrite(win->w_log, buf, len) < 1) {
		WMsg(win, errno, "Error writing logfile");
		logfclose(win->w_log);
		win->w_log = NULL;
	}
	if (!log_flush)
		logfflush(win->w_log);
}

static int Special(Window *win, int c)
{
	switch (c) {
	case '\b':
		BackSpace(win);
		return 1;
	case '\r':
		Return(win);
		return 1;
	case '\n':
		if (win->w_autoaka)
			FindAKA(win);
		/* fall through */
	case '\013':		/* Vertical tab is the same as Line Feed */
		LineFeed(win, 0);
		return 1;
	case '\007':
		WBell(win, visual_bell);
		return 1;
	case '\t':
		ForwardTab(win);
		return 1;
	case '\017':		/* SI */
		MapCharset(win, G0);
		return 1;
	case '\016':		/* SO */
		MapCharset(win, G1);
		return 1;
	}
	return 0;
}

static void DoESC(Window *win, int c, int intermediate)
{
	switch (intermediate) {
	case 0:
		switch (c) {
		case 'E':
			LineFeed(win, 1);
			break;
		case 'D':
			LineFeed(win, 0);
			break;
		case 'M':
			ReverseLineFeed(win);
			break;
		case 'H':
			win->w_tabs[win->w_x] = 1;
			break;
		case 'Z':	/* jph: Identify as VT100 */
			Report(win, "\033[?%d;%dc", 1, 2);
			break;
		case '7':
			SaveCursor(win, &win->w_saved);
			break;
		case '8':
			RestoreCursor(win, &win->w_saved);
			break;
		case 'c':
			ClearScreen(win);
			ResetWindow(win);
			LKeypadMode(&win->w_layer, 0);
			LCursorkeysMode(&win->w_layer, 0);
#ifndef TIOCPKT
			WNewAutoFlow(win, 1);
#endif
			/* XXX
			   SetRendition(&mchar_null);
			   InsertMode(false);
			   ChangeScrollRegion(0, win->w_height - 1);
			 */
			LGotoPos(&win->w_layer, win->w_x, win->w_y);
			break;
		case '=':
			LKeypadMode(&win->w_layer, win->w_keypad = 1);
#ifndef TIOCPKT
			WNewAutoFlow(win, 0);
#endif				/* !TIOCPKT */
			break;
		case '>':
			LKeypadMode(&win->w_layer, win->w_keypad = 0);
#ifndef TIOCPKT
			WNewAutoFlow(win, 1);
#endif				/* !TIOCPKT */
			break;
		case 'n':	/* LS2 */
			MapCharset(win, G2);
			break;
		case 'o':	/* LS3 */
			MapCharset(win, G3);
			break;
		case '~':
			MapCharsetR(win, G1);	/* LS1R */
			break;
			/* { */
		case '}':
			MapCharsetR(win, G2);	/* LS2R */
			break;
		case '|':
			MapCharsetR(win, G3);	/* LS3R */
			break;
		case 'N':	/* SS2 */
			if (win->w_charsets[win->w_Charset] != win->w_charsets[G2]
			    || win->w_charsets[win->w_CharsetR] != win->w_charsets[G2])
				win->w_FontR = win->w_FontL = win->w_charsets[win->w_ss = G2];
			else
				win->w_ss = 0;
			break;
		case 'O':	/* SS3 */
			if (win->w_charsets[win->w_Charset] != win->w_charsets[G3]
			    || win->w_charsets[win->w_CharsetR] != win->w_charsets[G3])
				win->w_FontR = win->w_FontL = win->w_charsets[win->w_ss = G3];
			else
				win->w_ss = 0;
			break;
		case 'g':	/* VBELL, private screen sequence */
			WBell(win, true);
			break;
		}
		break;
	case '#':
		switch (c) {
		case '8':
			FillWithEs(win);
			break;
		}
		break;
	case '(':
		DesignateCharset(win, c, G0);
		break;
	case ')':
		DesignateCharset(win, c, G1);
		break;
	case '*':
		DesignateCharset(win, c, G2);
		break;
	case '+':
		DesignateCharset(win, c, G3);
		break;
/*
 * ESC $ ( Fn: invoke multi-byte charset, Fn, to G0
 * ESC $ Fn: same as above.  (old sequence)
 * ESC $ ) Fn: invoke multi-byte charset, Fn, to G1
 * ESC $ * Fn: invoke multi-byte charset, Fn, to G2
 * ESC $ + Fn: invoke multi-byte charset, Fn, to G3
 */
	case '$':
	case '$' << 8 | '(':
		DesignateCharset(win, c & 037, G0);
		break;
	case '$' << 8 | ')':
		DesignateCharset(win, c & 037, G1);
		break;
	case '$' << 8 | '*':
		DesignateCharset(win, c & 037, G2);
		break;
	case '$' << 8 | '+':
		DesignateCharset(win, c & 037, G3);
		break;
	}
}

static void DoCSI(Window *win, int c, int intermediate)
{
	int i, a1 = win->w_args[0], a2 = win->w_args[1];

	if (win->w_NumArgs > MAXARGS)
		win->w_NumArgs = MAXARGS;
	switch (intermediate) {
	case 0:
		switch (c) {
		case 'H':
		case 'f':
			if (a1 < 1)
				a1 = 1;
			if (win->w_origin)
				a1 += win->w_top;
			if (a1 > win->w_height)
				a1 = win->w_height;
			if (a2 < 1)
				a2 = 1;
			if (a2 > win->w_width)
				a2 = win->w_width;
			LGotoPos(&win->w_layer, --a2, --a1);
			win->w_x = a2;
			win->w_y = a1;
			if (win->w_autoaka)
				win->w_autoaka = a1 + 1;
			break;
		case 'J':
			if (a1 < 0 || a1 > 2)
				a1 = 0;
			switch (a1) {
			case 0:
				ClearToEOS(win);
				break;
			case 1:
				ClearFromBOS(win);
				break;
			case 2:
				ClearScreen(win);
				LGotoPos(&win->w_layer, win->w_x, win->w_y);
				break;
			}
			break;
		case 'K':
			if (a1 < 0 || a1 > 2)
				a1 %= 3;
			switch (a1) {
			case 0:
				ClearLineRegion(win, win->w_x, win->w_width - 1);
				break;
			case 1:
				ClearLineRegion(win, 0, win->w_x);
				break;
			case 2:
				ClearLineRegion(win, 0, win->w_width - 1);
				break;
			}
			break;
		case 'X':
			a1 = win->w_x + (a1 ? a1 - 1 : 0);
			ClearLineRegion(win, win->w_x, a1 < win->w_width ? a1 : win->w_width - 1);
			break;
		case 'A':
			CursorUp(win, a1 ? a1 : 1);
			break;
		case 'B':
			CursorDown(win, a1 ? a1 : 1);
			break;
		case 'C':
			CursorRight(win, a1 ? a1 : 1);
			break;
		case 'D':
			CursorLeft(win, a1 ? a1 : 1);
			break;
		case 'E':
			win->w_x = 0;
			CursorDown(win, a1 ? a1 : 1);	/* positions cursor */
			break;
		case 'F':
			win->w_x = 0;
			CursorUp(win, a1 ? a1 : 1);	/* positions cursor */
			break;
		case 'G':
		case '`':	/* HPA */
			win->w_x = a1 ? a1 - 1 : 0;
			if (win->w_x >= win->w_width)
				win->w_x = win->w_width - 1;
			LGotoPos(&win->w_layer, win->w_x, win->w_y);
			break;
		case 'd':	/* VPA */
			win->w_y = a1 ? a1 - 1 : 0;
			if (win->w_y >= win->w_height)
				win->w_y = win->w_height - 1;
			LGotoPos(&win->w_layer, win->w_x, win->w_y);
			break;
		case 'm':
			SelectRendition(win);
			break;
		case 'g':
			if (a1 == 0)
				win->w_tabs[win->w_x] = 0;
			else if (a1 == 3)
				memset(win->w_tabs, 0, win->w_width);
			break;
		case 'r':
			if (!a1)
				a1 = 1;
			if (!a2)
				a2 = win->w_height;
			if (a1 < 1 || a2 > win->w_height || a1 >= a2)
				break;
			win->w_top = a1 - 1;
			win->w_bot = a2 - 1;
			/* ChangeScrollRegion(win->w_top, win->w_bot); */
			if (win->w_origin) {
				win->w_y = win->w_top;
				win->w_x = 0;
			} else
				win->w_y = win->w_x = 0;
			LGotoPos(&win->w_layer, win->w_x, win->w_y);
			break;
		case 's':
			SaveCursor(win, &win->w_saved);
			break;
		case 't':
			switch (a1) {
			case 7:
				LRefreshAll(&win->w_layer, 0);
				break;
			case 8:
				a1 = win->w_args[2];
				if (a1 < 1)
					a1 = win->w_width;
				if (a2 < 1)
					a2 = win->w_height;
				if (a1 > 10000 || a2 > 10000)
					break;
				WChangeSize(win, a1, a2);
				break;
			case 11:
				if (win->w_layer.l_cvlist)
					Report(win, "\033[1t", 0, 0);
				else
					Report(win, "\033[2t", 0, 0);
				break;
			case 18:
				Report(win, "\033[8;%d;%dt", win->w_height, win->w_width);
				break;
			case 21:
				a1 = strlen(win->w_title);
				if ((unsigned)(win->w_inlen + 5 + a1) <= ARRAY_SIZE(win->w_inbuf)) {
					memmove(win->w_inbuf + win->w_inlen, "\033]l", 3);
					memmove(win->w_inbuf + win->w_inlen + 3, win->w_title, a1);
					memmove(win->w_inbuf + win->w_inlen + 3 + a1, "\033\\", 2);
					win->w_inlen += 5 + a1;
				}
				break;
			default:
				break;
			}
			break;
		case 'u':
			RestoreCursor(win, &win->w_saved);
			break;
		case 'I':
			if (!a1)
				a1 = 1;
			while (a1--)
				ForwardTab(win);
			break;
		case 'Z':
			if (!a1)
				a1 = 1;
			while (a1--)
				BackwardTab(win);
			break;
		case 'L':
			InsertLine(win, a1 ? a1 : 1);
			break;
		case 'M':
			DeleteLine(win, a1 ? a1 : 1);
			break;
		case 'P':
			DeleteChar(win, a1 ? a1 : 1);
			break;
		case '@':
			InsertChar(win, a1 ? a1 : 1);
			break;
		case 'h':
			ASetMode(win, true);
			break;
		case 'l':
			ASetMode(win, false);
			break;
		case 'i':	/* MC Media Control */
			if (a1 == 5)
				PrintStart(win);
			break;
		case 'n':
			if (a1 == 5)	/* Report terminal status */
				Report(win, "\033[0n", 0, 0);
			else if (a1 == 6)	/* Report cursor position */
				Report(win, "\033[%d;%dR", win->w_y + 1, win->w_x + 1);
			break;
		case 'c':	/* Identify as VT100 */
			if (a1 == 0)
				Report(win, "\033[?%d;%dc", 1, 2);
			break;
		case 'x':	/* decreqtparm */
			if (a1 == 0 || a1 == 1)
				Report(win, "\033[%d;1;1;112;112;1;0x", a1 + 2, 0);
			break;
		case 'p':	/* obscure code from a 97801 term */
			if (a1 == 6 || a1 == 7) {
				win->w_curinv = 7 - a1;
				LCursorVisibility(&win->w_layer, win->w_curinv ? -1 : win->w_curvvis);
			}
			break;
		case 'S':	/* code from a 97801 term / DEC vt400 */
			ScrollRegion(win, a1 ? a1 : 1);
			break;
		case 'T':	/* code from a 97801 term / DEC vt400 */
		case '^':	/* SD as per ISO 6429 */
			ScrollRegion(win, a1 ? -a1 : -1);
			break;
		}
		break;
	case ' ':
		if (c == 'q') {
			win->w_cursorstyle = a1;
			LCursorStyle(&win->w_layer, win->w_cursorstyle);
		}
		break;
	case '?':
		for (a2 = 0; a2 < win->w_NumArgs; a2++) {
			a1 = win->w_args[a2];
			if (c != 'h' && c != 'l')
				break;
			i = (c == 'h');
			switch (a1) {
			case 1:	/* CKM:  cursor key mode */
				LCursorkeysMode(&win->w_layer, win->w_cursorkeys = i);
#ifndef TIOCPKT
				WNewAutoFlow(win, !i);
#endif				/* !TIOCPKT */
				break;
			case 2:	/* ANM:  ansi/vt52 mode */
				if (i) {
					if (win->w_encoding)
						break;
					win->w_charsets[0] = win->w_charsets[1] =
					    win->w_charsets[2] = win->w_charsets[3] =
					    win->w_FontL = win->w_FontR = ASCII;
					win->w_Charset = 0;
					win->w_CharsetR = 2;
					win->w_ss = 0;
				}
				break;
			case 3:	/* COLM: column mode */
				i = (i ? Z0width : Z1width);
				ClearScreen(win);
				win->w_x = 0;
				win->w_y = 0;
				WChangeSize(win, i, win->w_height);
				break;
				/* case 4:        SCLM: scrolling mode */
			case 5:	/* SCNM: screen mode */
				if (i != win->w_revvid)
					WReverseVideo(win, i);
				win->w_revvid = i;
				break;
			case 6:	/* OM:   origin mode */
				if ((win->w_origin = i) != 0) {
					win->w_y = win->w_top;
					win->w_x = 0;
				} else
					win->w_y = win->w_x = 0;
				LGotoPos(&win->w_layer, win->w_x, win->w_y);
				break;
			case 7:	/* AWM:  auto wrap mode */
				win->w_wrap = i;
				break;
				/* case 8:        ARM:  auto repeat mode */
				/* case 9:        INLM: interlace mode */
			case 9:	/* X10 mouse tracking */
				win->w_mouse = i ? 9 : 0;
				LMouseMode(&win->w_layer, win->w_mouse);
				break;
				/* case 10:       EDM:  edit mode */
				/* case 11:       LTM:  line transmit mode */
				/* case 13:       SCFDM: space compression / field delimiting */
				/* case 14:       TEM:  transmit execution mode */
				/* case 16:       EKEM: edit key execution mode */
				/* case 18:       PFF:  Printer term form feed */
				/* case 19:       PEX:  Printer extend screen / scroll. reg */
			case 25:	/* TCEM: text cursor enable mode */
				win->w_curinv = !i;
				LCursorVisibility(&win->w_layer, win->w_curinv ? -1 : win->w_curvvis);
				break;
				/* case 34:       RLM:  Right to left mode */
				/* case 35:       HEBM: hebrew keyboard map */
				/* case 36:       HEM:  hebrew encoding */
				/* case 38:             TeK Mode */
				/* case 40:             132 col enable */
				/* case 42:       NRCM: 7bit NRC character mode */
				/* case 44:             margin bell enable */
			case 47:	/*       xterm-like alternate screen */
			case 1047:	/*       xterm-like alternate screen */
			case 1049:	/*       xterm-like alternate screen */
				if (use_altscreen) {
					if (i) {
						if (!win->w_alt.on) {
							SaveCursor(win, &win->w_alt.cursor);
							EnterAltScreen(win);
						}
					} else {
						if (win->w_alt.on) {
							RestoreCursor(win, &win->w_alt.cursor);
							LeaveAltScreen(win);
						}
					}
					if (a1 == 47 && !i)
						win->w_saved.on = 0;
					LRefreshAll(&win->w_layer, 0);
					LGotoPos(&win->w_layer, win->w_x, win->w_y);
				}
				break;
			case 1048:
				if (i)
					SaveCursor(win, &win->w_saved);
				else
					RestoreCursor(win, &win->w_saved);
				break;
				/* case 66:       NKM:  Numeric keypad appl mode */
				/* case 68:       KBUM: Keyboard usage mode (data process) */
			case 1000:	/* VT200 mouse tracking */
			case 1001:	/* VT200 highlight mouse */
			case 1002:	/* button event mouse */
			case 1003:	/* any event mouse */
				win->w_mouse = i ? a1 : 0;
				LMouseMode(&win->w_layer, win->w_mouse);
				break;
			/* case 1005:     UTF-8 mouse mode rejected */
			case 1006:	/* SGR mouse mode */
				win->w_extmouse = i ? a1 : 0;
				LExtMouseMode(&win->w_layer, win->w_extmouse);
				break;
			/* case 1015:     UXRVT mouse mode rejected */
			case 2004:	/* bracketed paste mode */
				win->w_bracketed = i ? true : false;
				LBracketedPasteMode(&win->w_layer, win->w_bracketed);
				break;
			}
		}
		break;
	case '>':
		switch (c) {
		case 'c':	/* secondary DA */
			if (a1 == 0)
				Report(win, "\033[>%d;%d;0c", 83, nversion);	/* 83 == 'S' */
			break;
		}
		break;
	}
}

static void StringStart(Window *win, enum string_t type)
{
	win->w_StringType = type;
	win->w_stringp = win->w_string;
	win->w_state = ASTR;
}

static void StringChar(Window *win, int c)
{
	if (win->w_stringp >= win->w_string + MAXSTR - 1)
		win->w_state = LIT;
	else
		*(win->w_stringp)++ = c;
}

/*
 * Do string processing. Returns -1 if output should be suspended
 * until status is gone.
 */
static int StringEnd(Window *win)
{
	Canvas *cv;
	char *p;
	int typ;
	char *t;

	/* There's two ways to terminate an OSC. If we've seen an ESC
	 * then it's been ST otherwise it's BEL. */
	t = win->w_state == STRESC ? "\033\\" : "\a";

	win->w_state = LIT;
	*win->w_stringp = '\0';
	switch (win->w_StringType) {
	case OSC:		/* special xterm compatibility hack */
		if (win->w_string[0] == ';' || (p = strchr(win->w_string, ';')) == NULL)
			break;
		typ = atoi(win->w_string);
		p++;
		if (typ == 83) {	/* 83 = 'S' */
			/* special execute commands sequence */
			char *args[MAXARGS];
			int argl[MAXARGS];
			struct acluser *windowuser;

			windowuser = *FindUserPtr(":window:");
			if (windowuser && Parse(p, ARRAY_SIZE(win->w_string) - (p - win->w_string), args, argl)) {
				for (display = displays; display; display = display->d_next)
					if (D_forecv->c_layer->l_bottom == &win->w_layer)
						break;	/* found it */
				if (display == NULL && win->w_layer.l_cvlist)
					display = win->w_layer.l_cvlist->c_display;
				if (display == NULL)
					display = displays;
				EffectiveAclUser = windowuser;
				fore = win;
				flayer = fore->w_savelayer ? fore->w_savelayer : &fore->w_layer;
				DoCommand(args, argl);
				EffectiveAclUser = NULL;
				fore = NULL;
				flayer = NULL;
			}
			break;
		}
		if (typ == 0 || typ == 1 || typ == 2 || typ == 11 || typ == 20 || typ == 39 || typ == 49) {
			int typ2;
			typ2 = typ / 10;
			if (strcmp(win->w_xtermosc[typ2], p)) {
				if (typ != 11 || strcmp("?", p)) {
					strncpy(win->w_xtermosc[typ2], p, ARRAY_SIZE(win->w_xtermosc[typ2]) - 1);
					win->w_xtermosc[typ2][ARRAY_SIZE(win->w_xtermosc[typ2]) - 1] = 0;
				}

				for (display = displays; display; display = display->d_next) {
					if (!D_CXT)
						continue;
					if (D_forecv->c_layer->l_bottom == &win->w_layer)
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

		win->w_stringp -= p - win->w_string;
		if (win->w_stringp > win->w_string)
			memmove(win->w_string, p, win->w_stringp - win->w_string);
		*win->w_stringp = '\0';
		/* FALLTHROUGH */
	case APC:
		if (win->w_hstatus) {
			if (strcmp(win->w_hstatus, win->w_string) == 0)
				break;	/* not changed */
			free(win->w_hstatus);
			win->w_hstatus = NULL;
		}
		if (win->w_string != win->w_stringp)
			win->w_hstatus = SaveStr(win->w_string);
		WindowChanged(win, WINESC_HSTATUS);
		break;
	case PM:
	case GM:
		for (display = displays; display; display = display->d_next) {
			for (cv = D_cvlist; cv; cv = cv->c_next)
				if (cv->c_layer->l_bottom == &win->w_layer)
					break;
			if (cv || win->w_StringType == GM)
				MakeStatus(win->w_string);
		}
		return -1;
	case DCS:
		LAY_DISPLAYS(&win->w_layer, AddStr(win->w_string));
		break;
	case AKA:
		if (win->w_title == win->w_akabuf && !*win->w_string)
			break;
		if (win->w_dynamicaka)
			ChangeAKA(win, win->w_string, strlen(win->w_string));
		if (!*win->w_string)
			win->w_autoaka = win->w_y + 1;
		break;
	default:
		break;
	}
	return 0;
}

static void PrintStart(Window *win)
{
	win->w_pdisplay = NULL;

	/* find us a nice display to print on, fore prefered */
	display = win->w_lastdisp;
	if (!(display && win== D_fore && (printcmd || D_PO)))
		for (display = displays; display; display = display->d_next)
			if (win== D_fore && (printcmd || D_PO))
				break;
	if (!display) {
		Canvas *cv;
		for (cv = win->w_layer.l_cvlist; cv; cv = cv->c_lnext) {
			display = cv->c_display;
			if (printcmd || D_PO)
				break;
		}
		if (!cv) {
			display = displays;
			if (!display || display->d_next || !(printcmd || D_PO))
				return;
		}
	}
	win->w_pdisplay = display;
	win->w_stringp = win->w_string;
	win->w_state = PRIN;
	if (printcmd && win->w_pdisplay->d_printfd < 0)
		win->w_pdisplay->d_printfd = printpipe(win, printcmd);
}

static void PrintChar(Window *win, int c)
{
	if (win->w_stringp >= win->w_string + MAXSTR - 1)
		PrintFlush(win);
	*(win->w_stringp)++ = c;
}

static void PrintFlush(Window *win)
{
	display = win->w_pdisplay;
	if (display && printcmd) {
		char *bp = win->w_string;
		int len = win->w_stringp - win->w_string;
		int r;
		while (len && display->d_printfd >= 0) {
			r = write(display->d_printfd, bp, len);
			if (r <= 0) {
				WMsg(win, errno, "printing aborted");
				close(display->d_printfd);
				display->d_printfd = -1;
				break;
			}
			bp += r;
			len -= r;
		}
	} else if (display && win->w_stringp > win->w_string) {
		AddCStr(D_PO);
		AddStrn(win->w_string, win->w_stringp - win->w_string);
		AddCStr(D_PF);
		Flush(3);
	}
	win->w_stringp = win->w_string;
}

void WNewAutoFlow(Window *win, int on)
{
	if (win->w_flow & FLOW_AUTOFLAG)
		win->w_flow = FLOW_AUTOFLAG | (FLOW_AUTO | FLOW_ON) * on;
	else
		win->w_flow = (win->w_flow & ~FLOW_AUTO) | FLOW_AUTO * on;
	LSetFlow(&win->w_layer, win->w_flow & FLOW_ON);
}

static void DesignateCharset(Window *win, int c, int n)
{
	win->w_ss = 0;
	if (c == ('@' & 037))	/* map JIS 6226 to 0208 */
		c = KANJI;
	if (c == 'B')
		c = ASCII;
	if (win->w_charsets[n] != c) {
		win->w_charsets[n] = c;
		if (win->w_Charset == n) {
			win->w_FontL = c;
			win->w_rend.font = win->w_FontL;
			LSetRendition(&win->w_layer, &win->w_rend);
		}
		if (win->w_CharsetR == n)
			win->w_FontR = c;
	}
}

static void MapCharset(Window *win, int n)
{
	win->w_ss = 0;
	if (win->w_Charset != n) {
		win->w_Charset = n;
		win->w_FontL = win->w_charsets[n];
		win->w_rend.font = win->w_FontL;
		LSetRendition(&win->w_layer, &win->w_rend);
	}
}

static void MapCharsetR(Window *win, int n)
{
	win->w_ss = 0;
	if (win->w_CharsetR != n) {
		win->w_CharsetR = n;
		win->w_FontR = win->w_charsets[n];
	}
	win->w_gr = 1;
}

static void SaveCursor(Window *win, struct cursor *cursor)
{
	cursor->on = 1;
	cursor->x = win->w_x;
	cursor->y = win->w_y;
	cursor->Rend = win->w_rend;
	cursor->Charset = win->w_Charset;
	cursor->CharsetR = win->w_CharsetR;
	memmove((char *)cursor->Charsets, (char *)win->w_charsets, 4 * sizeof(int));
}

static void RestoreCursor(Window *win, struct cursor *cursor)
{
	if (!cursor->on)
		return;
	LGotoPos(&win->w_layer, cursor->x, cursor->y);
	win->w_x = cursor->x;
	win->w_y = cursor->y;
	win->w_rend = cursor->Rend;
	memmove((char *)win->w_charsets, (char *)cursor->Charsets, 4 * sizeof(int));
	win->w_Charset = cursor->Charset;
	win->w_CharsetR = cursor->CharsetR;
	win->w_ss = 0;
	win->w_FontL = win->w_charsets[win->w_Charset];
	win->w_FontR = win->w_charsets[win->w_CharsetR];
	LSetRendition(&win->w_layer, &win->w_rend);
}

static void BackSpace(Window *win)
{
	if (win->w_x > 0) {
		win->w_x--;
	} else if (win->w_wrap && win->w_y > 0) {
		win->w_x = win->w_width - 1;
		win->w_y--;
	}
	LGotoPos(&win->w_layer, win->w_x, win->w_y);
}

static void Return(Window *win)
{
	if (win->w_x == 0)
		return;
	win->w_x = 0;
	LGotoPos(&win->w_layer, win->w_x, win->w_y);
}

static void LineFeed(Window *win, int out_mode)
{
	/* out_mode: 0=lf, 1=cr+lf */
	if (out_mode)
		win->w_x = 0;
	if (win->w_y != win->w_bot) {	/* Don't scroll */
		if (win->w_y < win->w_height - 1)
			win->w_y++;
		LGotoPos(&win->w_layer, win->w_x, win->w_y);
		return;
	}
	if (win->w_autoaka > 1)
		win->w_autoaka--;
	MScrollV(win, 1, win->w_top, win->w_bot, win->w_rend.colorbg);
	LScrollV(&win->w_layer, 1, win->w_top, win->w_bot, win->w_rend.colorbg);
	LGotoPos(&win->w_layer, win->w_x, win->w_y);
}

static void ReverseLineFeed(Window *win)
{
	if (win->w_y == win->w_top) {
		MScrollV(win, -1, win->w_top, win->w_bot, win->w_rend.colorbg);
		LScrollV(&win->w_layer, -1, win->w_top, win->w_bot, win->w_rend.colorbg);
		LGotoPos(&win->w_layer, win->w_x, win->w_y);
	} else if (win->w_y > 0)
		CursorUp(win, 1);
}

static void InsertChar(Window *win, int n)
{
	int y = win->w_y, x = win->w_x;

	if (n <= 0)
		return;
	if (x == win->w_width)
		x--;
	save_mline(&win->w_mlines[y], win->w_width);
	MScrollH(win, -n, y, x, win->w_width - 1, win->w_rend.colorbg);
	LScrollH(&win->w_layer, -n, y, x, win->w_width - 1, win->w_rend.colorbg, &mline_old);
	LGotoPos(&win->w_layer, x, y);
}

static void DeleteChar(Window *win, int n)
{
	int y = win->w_y, x = win->w_x;

	if (x == win->w_width)
		x--;
	save_mline(&win->w_mlines[y], win->w_width);
	MScrollH(win, n, y, x, win->w_width - 1, win->w_rend.colorbg);
	LScrollH(&win->w_layer, n, y, x, win->w_width - 1, win->w_rend.colorbg, &mline_old);
	LGotoPos(&win->w_layer, x, y);
}

static void DeleteLine(Window *win, int n)
{
	if (win->w_y < win->w_top || win->w_y > win->w_bot)
		return;
	if (n > win->w_bot - win->w_y + 1)
		n = win->w_bot - win->w_y + 1;
	MScrollV(win, n, win->w_y, win->w_bot, win->w_rend.colorbg);
	LScrollV(&win->w_layer, n, win->w_y, win->w_bot, win->w_rend.colorbg);
	LGotoPos(&win->w_layer, win->w_x, win->w_y);
}

static void InsertLine(Window *win, int n)
{
	if (win->w_y < win->w_top || win->w_y > win->w_bot)
		return;
	if (n > win->w_bot - win->w_y + 1)
		n = win->w_bot - win->w_y + 1;
	MScrollV(win, -n, win->w_y, win->w_bot, win->w_rend.colorbg);
	LScrollV(&win->w_layer, -n, win->w_y, win->w_bot, win->w_rend.colorbg);
	LGotoPos(&win->w_layer, win->w_x, win->w_y);
}

static void ScrollRegion(Window *win, int n)
{
	MScrollV(win, n, win->w_top, win->w_bot, win->w_rend.colorbg);
	LScrollV(&win->w_layer, n, win->w_top, win->w_bot, win->w_rend.colorbg);
	LGotoPos(&win->w_layer, win->w_x, win->w_y);
}

static void ForwardTab(Window *win)
{
	int x = win->w_x;

	if (x == win->w_width) {
		LineFeed(win, 1);
		x = 0;
	}
	if (win->w_tabs[x] && x < win->w_width - 1)
		x++;
	while (x < win->w_width - 1 && !win->w_tabs[x])
		x++;
	win->w_x = x;
	LGotoPos(&win->w_layer, win->w_x, win->w_y);
}

static void BackwardTab(Window *win)
{
	int x = win->w_x;

	if (win->w_tabs[x] && x > 0)
		x--;
	while (x > 0 && !win->w_tabs[x])
		x--;
	win->w_x = x;
	LGotoPos(&win->w_layer, win->w_x, win->w_y);
}

static void ClearScreen(Window *win)
{
	LClearArea(&win->w_layer, 0, 0, win->w_width - 1, win->w_height - 1, win->w_rend.colorbg, 1);
	MScrollV(win, win->w_height, 0, win->w_height - 1, win->w_rend.colorbg);
}

static void ClearFromBOS(Window *win)
{
	int y = win->w_y, x = win->w_x;

	LClearArea(&win->w_layer, 0, 0, x, y, win->w_rend.colorbg, 1);
	MClearArea(win, 0, 0, x, y, win->w_rend.colorbg);
	RestorePosRendition(win);
}

static void ClearToEOS(Window *win)
{
	int y = win->w_y, x = win->w_x;

	if (x == 0 && y == 0) {
		ClearScreen(win);
		RestorePosRendition(win);
		return;
	}
	LClearArea(&win->w_layer, x, y, win->w_width - 1, win->w_height - 1, win->w_rend.colorbg, 1);
	MClearArea(win, x, y, win->w_width - 1, win->w_height - 1, win->w_rend.colorbg);
	RestorePosRendition(win);
}

static void ClearLineRegion(Window *win, int from, int to)
{
	int y = win->w_y;
	LClearArea(&win->w_layer, from, y, to, y, win->w_rend.colorbg, 1);
	MClearArea(win, from, y, to, y, win->w_rend.colorbg);
	RestorePosRendition(win);
}

static void CursorRight(Window *win, int n)
{
	int x = win->w_x;

	if (x == win->w_width)
		LineFeed(win, 1);
	if ((win->w_x += n) >= win->w_width)
		win->w_x = win->w_width - 1;
	LGotoPos(&win->w_layer, win->w_x, win->w_y);
}

static void CursorUp(Window *win, int n)
{
	if (win->w_y < win->w_top) {	/* if above scrolling rgn, */
		if ((win->w_y -= n) < 0)	/* ignore its limits      */
			win->w_y = 0;
	} else if ((win->w_y -= n) < win->w_top)
		win->w_y = win->w_top;
	LGotoPos(&win->w_layer, win->w_x, win->w_y);
}

static void CursorDown(Window *win, int n)
{
	if (win->w_y > win->w_bot) {	/* if below scrolling rgn, */
		if ((win->w_y += n) > win->w_height - 1)	/* ignore its limits      */
			win->w_y = win->w_height - 1;
	} else if ((win->w_y += n) > win->w_bot)
		win->w_y = win->w_bot;
	LGotoPos(&win->w_layer, win->w_x, win->w_y);
}

static void CursorLeft(Window *win, int n)
{
	if ((win->w_x -= n) < 0)
		win->w_x = 0;
	LGotoPos(&win->w_layer, win->w_x, win->w_y);
}

static void ASetMode(Window *win, bool on)
{
	for (int i = 0; i < win->w_NumArgs; ++i) {
		switch (win->w_args[i]) {
			/* case 2:            KAM: Lock keyboard */
		case 4:	/* IRM: Insert mode */
			win->w_insert = on;
			LAY_DISPLAYS(&win->w_layer, InsertMode(on));
			break;
			/* case 12:           SRM: Echo mode on */
		case 20:	/* LNM: Linefeed mode */
			win->w_autolf = on;
			break;
		case 34:
			win->w_curvvis = !on;
			LCursorVisibility(&win->w_layer, win->w_curinv ? -1 : win->w_curvvis);
			break;
		default:
			break;
		}
	}
}

static char rendlist[] = {
	~((1 << NATTR) - 1), A_BD, A_DI, A_IT, A_US, A_BL, 0, A_RV, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, ~(A_BD | A_DI), ~A_IT, ~A_US, ~A_BL, 0, ~A_RV
};

static void SelectRendition(Window *win)
{
	int j, i = 0;
	int attr = win->w_rend.attr;
	int colorbg = win->w_rend.colorbg;
	int colorfg = win->w_rend.colorfg;

	do {
		j = win->w_args[i];
		/* indexed colour space aka 256 colours; example escape \e[48;2;12m */
		if ((j == 38 || j == 48) && i + 2 < win->w_NumArgs && win->w_args[i + 1] == 5) {
			int jj;

			i += 2;
			jj = win->w_args[i];
			if (jj < 0 || jj > 255)
				continue;
			if (j == 38) {
				colorfg = jj | 0x02000000;
			} else {
				colorbg = jj | 0x02000000;
			}
			continue;
		}
		/* truecolor (24bit) colour space; example escape \e[48;5;12;13;14m 
		 * where 12;13;14 are rgb values */
		if ((j == 38 || j == 48) && i + 4 < win->w_NumArgs && win->w_args[i + 1] == 2) {
			uint8_t r, g, b;

			r = win->w_args[i + 2];
			g = win->w_args[i + 3];
			b = win->w_args[i + 4];

			if (j == 38) {
				colorfg = 0x04000000 | (r << 16) | (g << 8) | b;
			} else {
				colorbg = 0x04000000 | (r << 16) | (g << 8) | b;
			}
			i += 4;
			continue;
		}
		if (j >= 90 && j <= 97)
			colorfg = (j - 90 + 8) | 0x01000000;
		if (j >= 100 && j <= 107)
			colorbg = (j - 100 + 8) | 0x01000000;
		if (j >= 30 && j < 38)
			colorfg = (j - 30) | 0x01000000;
		if (j >= 40 && j < 48)
			colorbg = (j - 40) | 0x01000000;
		if (j == 39)
			colorfg = 0;
		if (j == 49)
			colorbg = 0;
		if (j == 0) {
			attr = 0;
			/* will be xored to 0 */
			colorbg = 0;
			colorfg = 0;
		}

		if (j < 0 || j >= (int)(ARRAY_SIZE(rendlist)))
			continue;
		j = rendlist[j];
		if (j & (1 << NATTR))
			attr &= j;
		else
			attr |= j;
	} while (++i < win->w_NumArgs);
	
	win->w_rend.attr = attr;
	
	win->w_rend.colorbg = colorbg;
	win->w_rend.colorfg = colorfg;
	LSetRendition(&win->w_layer, &win->w_rend);
}

static void FillWithEs(Window *win)
{
	uint32_t *p, *ep;

	LClearAll(&win->w_layer, 1);
	win->w_y = win->w_x = 0;
	for (int i = 0; i < win->w_height; ++i) {
		clear_mline(&win->w_mlines[i], 0, win->w_width + 1);
		p = win->w_mlines[i].image;
		ep = p + win->w_width;
		while (p < ep)
			*p++ = 'E';
	}
	LRefreshAll(&win->w_layer, 1);
}

/*
 *  Ugly autoaka hack support:
 *    ChangeAKA() sets a new aka
 *    FindAKA() searches for an autoaka match
 */

void ChangeAKA(Window *win, char *s, size_t len)
{
	int i, c;

	for (i = 0; len > 0; len--) {
		if (win->w_akachange + i == win->w_akabuf + ARRAY_SIZE(win->w_akabuf) - 1)
			break;
		c = (unsigned char)*s++;
		if (c == 0)
			break;
		if (c < 32 || c == 127 || (c >= 128 && c < 160 && win->w_c1))
			continue;
		win->w_akachange[i++] = c;
	}
	win->w_akachange[i] = 0;
	win->w_title = win->w_akachange;
	if (win->w_akachange != win->w_akabuf)
		if (win->w_akachange[0] == 0 || win->w_akachange[-1] == ':')
			win->w_title = win->w_akabuf + strlen(win->w_akabuf) + 1;
	WindowChanged(win, WINESC_WIN_TITLE);
	WindowChanged(NULL, WINESC_WIN_NAMES);
	WindowChanged(NULL, WINESC_WIN_NAMES_NOCUR);
}

static void FindAKA(Window *win)
{
	uint32_t *cp, *line;
	int len = strlen(win->w_akabuf);
	int y;

	y = (win->w_autoaka > 0 && win->w_autoaka <= win->w_height) ? win->w_autoaka - 1 : win->w_y;
 try_line:
	cp = line = win->w_mlines[y].image;
	if (win->w_autoaka > 0 && *win->w_akabuf != '\0') {
		for (;;) {
			if (cp - line >= win->w_width - len) {
				if (++y == win->w_autoaka && y < win->w_height)
					goto try_line;
				return;
			}
			if (strncmp((char *)cp, win->w_akabuf, len) == 0)
				break;
			cp++;
		}
		cp += len;
	}
	for (len = win->w_width - (cp - line); len && *cp == ' '; len--, cp++) ;
	if (len) {
		if (win->w_autoaka > 0 && (*cp == '!' || *cp == '%' || *cp == '^'))
			win->w_autoaka = -1;
		else
			win->w_autoaka = 0;
		line = cp;
		while (len && *cp != ' ') {
			if (*cp++ == '/')
				line = cp;
			len--;
		}
		ChangeAKA(win, (char *)line, cp - line);
	} else
		win->w_autoaka = 0;
}

static void RestorePosRendition(Window *win)
{
	LGotoPos(&win->w_layer, win->w_x, win->w_y);
	LSetRendition(&win->w_layer, &win->w_rend);
}

/* Send a terminal report as if it were typed. */
static void Report(Window *win, char *fmt, int n1, int n2)
{
	int len;
	char rbuf[40];		/* enough room for all replys */

	sprintf(rbuf, fmt, n1, n2);
	len = strlen(rbuf);

	if (W_UWP(win)) {
		if ((unsigned)(win->w_pwin->p_inlen + len) <= ARRAY_SIZE(win->w_pwin->p_inbuf)) {
			memmove(win->w_pwin->p_inbuf + win->w_pwin->p_inlen, rbuf, len);
			win->w_pwin->p_inlen += len;
		}
	} else {
		if ((unsigned)(win->w_inlen + len) <= ARRAY_SIZE(win->w_inbuf)) {
			memmove(win->w_inbuf + win->w_inlen, rbuf, len);
			win->w_inlen += len;
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

static void MFixLine(Window *win, int y, struct mchar *mc)
{
	struct mline *ml = &win->w_mlines[y];
	if (mc->attr && ml->attr == null) {
		if ((ml->attr = calloc(win->w_width + 1, 4)) == NULL) {
			ml->attr = null;
			mc->attr = win->w_rend.attr = 0;
			WMsg(win, 0, "Warning: no space for attr - turned off");
		}
	}
	if (mc->font && ml->font == null) {
		if ((ml->font = calloc(win->w_width + 1, 4)) == NULL) {
			ml->font = null;
			win->w_FontL = win->w_charsets[win->w_ss ? win->w_ss : win->w_Charset] = 0;
			win->w_FontR = win->w_charsets[win->w_ss ? win->w_ss : win->w_CharsetR] = 0;
			mc->font = win->w_rend.font = 0;
			WMsg(win, 0, "Warning: no space for font - turned off");
		}
	}
	if (mc->colorbg && ml->colorbg == null) {
		if ((ml->colorbg = calloc(win->w_width + 1, 4)) == NULL) {
			ml->colorbg = null;
			mc->colorbg = win->w_rend.colorbg = 0;
			WMsg(win, 0, "Warning: no space for color background - turned off");
		}
	}
	if (mc->colorfg && ml->colorfg == null) {
		if ((ml->colorfg = calloc(win->w_width + 1, 4)) == NULL) {
			ml->colorfg = null;
			mc->colorfg = win->w_rend.colorfg = 0;
			WMsg(win, 0, "Warning: no space for color foreground - turned off");
		}
	}
}

/*****************************************************************/

#define MKillDwRight(p, ml, x)					\
  if (dw_right(ml, x, p->w_encoding))				\
    {								\
      if (x > 0)						\
	copy_mchar2mline(&mchar_blank, ml, x - 1);		\
      copy_mchar2mline(&mchar_blank, ml, x);			\
    }

#define MKillDwLeft(p, ml, x)					\
  if (dw_left(ml, x, p->w_encoding))				\
    {								\
      copy_mchar2mline(&mchar_blank, ml, x);			\
      copy_mchar2mline(&mchar_blank, ml, x + 1);		\
    }

static void MScrollH(Window *win, int n, int y, int xs, int xe, int bce)
{
	struct mline *ml;

	if (n == 0)
		return;
	ml = &win->w_mlines[y];
	MKillDwRight(win, ml, xs);
	MKillDwLeft(win, ml, xe);
	if (n > 0) {
		if (xe - xs + 1 > n) {
			MKillDwRight(win, ml, xs + n);
			copy_mline(ml, xs + n, xs, xe + 1 - xs - n);
		} else
			n = xe - xs + 1;
		clear_mline(ml, xe + 1 - n, n);
		if (bce)
			MBceLine(win, y, xe + 1 - n, n, bce);
	} else {
		n = -n;
		if (xe - xs + 1 > n) {
			MKillDwLeft(win, ml, xe - n);
			copy_mline(ml, xs, xs + n, xe + 1 - xs - n);
		} else
			n = xe - xs + 1;
		clear_mline(ml, xs, n);
		if (bce)
			MBceLine(win, y, xs, n, bce);
	}
}

static void MScrollV(Window *win, int n, int ys, int ye, int bce)
{
	int cnt1, cnt2;
	struct mline tmp[256];
	struct mline *ml;

	if (n == 0)
		return;
	if (n > 0) {
		if (ye - ys + 1 < n)
			n = ye - ys + 1;
		if (n > 256) {
			MScrollV(win, n - 256, ys, ye, bce);
			n = 256;
		}
		if (compacthist) {
			ye = MFindUsedLine(win, ye, ys);
			if (ye - ys + 1 < n)
				n = ye - ys + 1;
			if (n <= 0)
				return;
		}
		/* Clear lines */
		ml = win->w_mlines + ys;
		for (int i = ys; i < ys + n; i++, ml++) {
			if (ys == win->w_top)
				WAddLineToHist(win, ml);
			if (ml->attr != null)
				free(ml->attr);
			ml->attr = null;
			if (ml->font != null)
				free(ml->font);
			ml->font = null;
			if (ml->colorbg != null)
				free(ml->colorbg);
			ml->colorbg = null;
			if (ml->colorfg != null)
				free(ml->colorfg);
			ml->colorfg = null;
			memmove(ml->image, blank, (win->w_width + 1) * 4);
			if (bce)
				MBceLine(win, i, 0, win->w_width, bce);
		}
		/* switch 'em over */
		cnt1 = n * sizeof(struct mline);
		cnt2 = (ye - ys + 1 - n) * sizeof(struct mline);
		if (cnt1 && cnt2)
			Scroll((char *)(win->w_mlines + ys), cnt1, cnt2, (char *)tmp);
	} else {
		n = -n;
		if (ye - ys + 1 < n)
			n = ye - ys + 1;
		if (n > 256) {
			MScrollV(win, - (n - 256), ys, ye, bce);
			n = 256;
		}

		ml = win->w_mlines + ye;
		/* Clear lines */
		for (int i = ye; i > ye - n; i--, ml--) {
			if (ml->attr != null)
				free(ml->attr);
			ml->attr = null;
			if (ml->font != null)
				free(ml->font);
			ml->font = null;
			if (ml->colorbg != null)
				free(ml->colorbg);
			ml->colorbg = null;
			if (ml->colorfg != null)
				free(ml->colorfg);
			ml->colorfg = null;
			memmove(ml->image, blank, (win->w_width + 1) * 4);
			if (bce)
				MBceLine(win, i, 0, win->w_width, bce);
		}
		cnt1 = n * sizeof(struct mline);
		cnt2 = (ye - ys + 1 - n) * sizeof(struct mline);
		if (cnt1 && cnt2)
			Scroll((char *)(win->w_mlines + ys), cnt2, cnt1, (char *)tmp);
	}
}

static void Scroll(char *cp, int cnt1, int cnt2, char *tmp)
{
	if (!cnt1 || !cnt2)
		return;
	if (cnt1 <= cnt2) {
		memmove(tmp, cp, cnt1);
		memmove(cp, cp + cnt1, cnt2);
		memmove(cp + cnt2, tmp, cnt1);
	} else {
		memmove(tmp, cp + cnt1, cnt2);
		memmove(cp + cnt2, cp, cnt1);
		memmove(cp, tmp, cnt2);
	}
}

static void MClearArea(Window *win, int xs, int ys, int xe, int ye, int bce)
{
	int n;
	int xxe;
	struct mline *ml;

	/* Check for zero-height window */
	if (ys < 0 || ye < ys)
		return;

	/* check for magic margin condition */
	if (xs >= win->w_width)
		xs = win->w_width - 1;
	if (xe >= win->w_width)
		xe = win->w_width - 1;

	MKillDwRight(win, win->w_mlines + ys, xs);
	MKillDwLeft(win, win->w_mlines + ye, xe);

	ml = win->w_mlines + ys;
	for (int y = ys; y <= ye; y++, ml++) {
		xxe = (y == ye) ? xe : win->w_width - 1;
		n = xxe - xs + 1;
		if (n > 0)
			clear_mline(ml, xs, n);
		if (n > 0 && bce)
			MBceLine(win, y, xs, xs + n - 1, bce);
		xs = 0;
	}
}

static void MInsChar(Window *win, struct mchar *c, int x, int y)
{
	int n;
	struct mline *ml;

	MFixLine(win, y, c);
	ml = win->w_mlines + y;
	n = win->w_width - x - 1;
	MKillDwRight(win, ml, x);
	if (n > 0) {
		MKillDwRight(win, ml, win->w_width - 1);
		copy_mline(ml, x, x + 1, n);
	}
	copy_mchar2mline(c, ml, x);
	if (c->mbcs) {
		if (--n > 0) {
			MKillDwRight(win, ml, win->w_width - 1);
			copy_mline(ml, x + 1, x + 2, n);
		}
		copy_mchar2mline(c, ml, x + 1);
		ml->image[x + 1] = c->mbcs;
		if (win->w_encoding != UTF8)
			ml->font[x + 1] |= 0x80;
		else if (win->w_encoding == UTF8 && c->mbcs) {
			ml->font[x + 1] = c->mbcs;
		}
	}
}

static void MPutChar(Window *win, struct mchar *c, int x, int y)
{
	struct mline *ml;

	MFixLine(win, y, c);
	ml = &win->w_mlines[y];
	MKillDwRight(win, ml, x);
	MKillDwLeft(win, ml, x);
	copy_mchar2mline(c, ml, x);
	if (c->mbcs) {
		MKillDwLeft(win, ml, x + 1);
		copy_mchar2mline(c, ml, x + 1);
		ml->image[x + 1] = c->mbcs;
		if (win->w_encoding != UTF8)
			ml->font[x + 1] |= 0x80;
		else if (win->w_encoding == UTF8 && c->mbcs) {
			ml->font[x + 1] = c->mbcs;
		}
	}
}

static void MWrapChar(Window *win, struct mchar *c, int y, int top, int bot, bool ins)
{
	struct mline *ml;
	int bce;

	bce = c->colorbg;
	MFixLine(win, y, c);
	ml = &win->w_mlines[y];
	copy_mchar2mline(&mchar_null, ml, win->w_width);
	if (y == bot)
		MScrollV(win, 1, top, bot, bce);
	else if (y < win->w_height - 1)
		y++;
	if (ins)
		MInsChar(win, c, 0, y);
	else
		MPutChar(win, c, 0, y);
}

static void MBceLine(Window *win, int y, int xs, int xe, int bce)
{
	struct mchar mc;
	struct mline *ml;

	mc = mchar_null;
	mc.colorbg = bce;
	MFixLine(win, y, &mc);
	ml = win->w_mlines + y;
	if (mc.attr)
		for (int x = xs; x <= xe; x++)
			ml->attr[x] = mc.attr;
	if (mc.colorbg)
		for (int x = xs; x <= xe; x++)
			ml->colorbg[x] = mc.colorbg;
	if (mc.colorfg)
		for (int x = xs; x <= xe; x++)
			ml->colorfg[x] = mc.colorfg;
}

static void WAddLineToHist(Window *win, struct mline *ml)
{
	uint32_t *q, *o;
	struct mline *hml;

	if (win->w_histheight == 0)
		return;
	hml = &win->w_hlines[win->w_histidx];
	q = ml->image;
	ml->image = hml->image;
	hml->image = q;

	q = ml->attr;
	o = hml->attr;
	hml->attr = q;
	ml->attr = null;
	if (o != null)
		free(o);
	q = ml->font;
	o = hml->font;
	hml->font = q;
	ml->font = null;
	if (o != null)
		free(o);
	q = ml->colorbg;
	o = hml->colorbg;
	hml->colorbg = q;
	ml->colorbg = null;
	if (o != null)
		free(o);
	q = ml->colorfg;
	o = hml->colorfg;
	hml->colorfg = q;
	ml->colorfg = null;
	if (o != null)
		free(o);

	if (++win->w_histidx >= win->w_histheight)
		win->w_histidx = 0;
	if (win->w_scrollback_height < win->w_histheight)
		++win->w_scrollback_height;
}

int MFindUsedLine(Window *win, int ye, int ys)
{
	int y;
	struct mline *ml = win->w_mlines + ye;

	for (y = ye; y >= ys; y--, ml--) {
		if (memcmp(ml->image, blank, win->w_width * 4))
			break;
		if (ml->attr != null && memcmp(ml->attr, null, win->w_width * 4))
			break;
		if (ml->colorbg != null && memcmp(ml->colorbg, null, win->w_width * 4))
			break;
		if (ml->colorfg != null && memcmp(ml->colorfg, null, win->w_width * 4))
			break;
		if (win->w_encoding == UTF8) {
			if (ml->font != null && memcmp(ml->font, null, win->w_width))
				break;
		}
	}
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
void WBell(Window *win, bool visual)
{
	Canvas *cv;
	if (displays == NULL)
		win->w_bell = BELL_DONE;
	for (display = displays; display; display = display->d_next) {
		for (cv = D_cvlist; cv; cv = cv->c_next)
			if (cv->c_layer->l_bottom == &win->w_layer)
				break;
		if (cv && !visual)
			AddCStr(D_BL);
		else if (cv && D_VB)
			AddCStr(D_VB);
		else
			win->w_bell = visual ? BELL_VISUAL : BELL_FOUND;
	}
}

/*
 * This should be reverse video.
 * Only change video if window is fore.
 * Because it is used in some termcaps to emulate
 * a visual bell we do this hack here.
 * (screen uses \Eg as special vbell sequence)
 */
static void WReverseVideo(Window *win, bool on)
{
	for (Canvas *cv = win->w_layer.l_cvlist; cv; cv = cv->c_lnext) {
		display = cv->c_display;
		if (cv != D_forecv)
			continue;
		ReverseVideo(on);
		if (!on && win->w_revvid && !D_CVR) {
			if (D_VB)
				AddCStr(D_VB);
			else
				win->w_bell = BELL_VISUAL;
		}
	}
}

void WMsg(Window *win, int err, char *str)
{
	Layer *oldflayer = flayer;
	flayer = &win->w_layer;
	LMsg(err, "%s", str);
	flayer = oldflayer;
}

static void WChangeSize(Window *win, int w, int h)
{
	int wok = 0;
	Canvas *cv;

	if (win->w_layer.l_cvlist == NULL) {
		/* window not displayed -> works always */
		ChangeWindowSize(win, w, h, win->w_histheight);
		return;
	}
	for (cv = win->w_layer.l_cvlist; cv; cv = cv->c_lnext) {
		display = cv->c_display;
		if (win != D_fore)
			continue;	/* change only fore */
		if (D_CWS)
			break;
		if (D_CZ0 && (w == Z0width || w == Z1width))
			wok = 1;
	}
	if (cv == NULL && wok == 0)	/* can't change any display */
		return;
	if (!D_CWS)
		h = win->w_height;
	ChangeWindowSize(win, w, h, win->w_histheight);
	for (display = displays; display; display = display->d_next) {
		if (win == D_fore) {
			if (D_cvlist && D_cvlist->c_next == NULL)
				ResizeDisplay(w, h);
			else
				ResizeDisplay(w, D_height);
			ResizeLayersToCanvases();	/* XXX Hmm ? */
			continue;
		}
		for (cv = D_cvlist; cv; cv = cv->c_next)
			if (cv->c_layer->l_bottom == &win->w_layer)
				break;
		if (cv)
			Redisplay(0);
	}
}

