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

#include "telnet.h"

#ifdef ENABLE_TELNET

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>

#include "screen.h"

#include "ansi.h"
#include "misc.h"
#include "termcap.h"

static void TelReply(Window *, char *, size_t);
static void TelDocmd(Window *, int, int);
static void TelDosub(Window *);

// why TEL_DEFPORT has "
#define TEL_DEFPORT     "23"
#define TEL_CONNECTING	(-2)

#define TC_IAC          255
#define TC_DONT         254
#define TC_DO           253
#define TC_WONT         252
#define TC_WILL         251
#define TC_SB           250
#define TC_BREAK        243
#define TC_SE           240

#define TC_S "S  b      swWdDc"

#define TO_BINARY       0
#define TO_ECHO         1
#define TO_SGA          3
#define TO_TM           6
#define TO_TTYPE        24
#define TO_NAWS         31
#define TO_TSPEED       32
#define TO_LFLOW        33
#define TO_LINEMODE     34
#define TO_XDISPLOC     35
#define TO_NEWENV       39

#define TO_S "be c                    t      wsf xE  E"

static unsigned char tn_init[] = {
	TC_IAC, TC_DO, TO_SGA,
	TC_IAC, TC_WILL, TO_TTYPE,
	TC_IAC, TC_WILL, TO_NAWS,
	TC_IAC, TC_WILL, TO_LFLOW,
};

static void tel_connev_fn(Event *ev, void *data)
{
	Window *win = (Window *)data;

	(void)ev; /* unused */

	if (connect(win->w_ptyfd, (struct sockaddr *)&win->w_telsa, sizeof(struct sockaddr_in)) && errno != EISCONN) {
		char buf[1024];
		buf[0] = ' ';
		strncpy(buf + 1, strerror(errno), ARRAY_SIZE(buf) - 2);
		buf[ARRAY_SIZE(buf) - 1] = 0;
		WriteString(win, buf, strlen(buf));
		WindowDied(win, 0, 0);
		return;
	}
	WriteString(win, "connected.\r\n", 12);
	evdeq(&win->w_telconnev);
	win->w_telstate = 0;
}

int TelOpenAndConnect(Window *win)
{
	int fd, on = 1;
	char buf[256];

	struct addrinfo hints, *res0, *res;

	if (!(win->w_cmdargs[1])) {
		Msg(0, "Usage: screen //telnet host [port]");
		return -1;
	}

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = af;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (getaddrinfo(win->w_cmdargs[1], win->w_cmdargs[2] ? win->w_cmdargs[2] : TEL_DEFPORT, &hints, &res0)) {
		Msg(0, "unknown host: %s", win->w_cmdargs[1]);
		return -1;
	}

	for (res = res0; res; res = res->ai_next) {
		if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
			if (res->ai_next)
				continue;
			else {
				Msg(errno, "TelOpenAndConnect: socket");
				freeaddrinfo(res0);
				return -1;
			}
		}

		if (setsockopt(fd, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof(int)))
			Msg(errno, "TelOpenAndConnect: setsockopt SO_OOBINLINE");

		if (win->w_cmdargs[2] && strcmp(win->w_cmdargs[2], TEL_DEFPORT))
			snprintf(buf, 256, "Trying %s %s...", win->w_cmdargs[1], win->w_cmdargs[2]);
		else
			snprintf(buf, 256, "Trying %s...", win->w_cmdargs[1]);

		WriteString(win, buf, strlen(buf));
		if (connect(fd, res->ai_addr, res->ai_addrlen)) {
			if (errno == EINPROGRESS) {
				win->w_telstate = TEL_CONNECTING;
				win->w_telconnev.fd = fd;
				win->w_telconnev.handler = tel_connev_fn;
				win->w_telconnev.data = (void *)win;
				win->w_telconnev.type = EV_WRITE;
				win->w_telconnev.priority = 1;
				evenq(&win->w_telconnev);
			} else {
				close(fd);
				if (res->ai_next)
					continue;
				else {
					Msg(errno, "TelOpenAndConnect: connect");
					freeaddrinfo(res0);
					return -1;
				}
			}
		} else
			WriteString(win, "connected.\r\n", 12);

		if (!(win->w_cmdargs[2] && strcmp(win->w_cmdargs[2], TEL_DEFPORT)))
			TelReply(win, (char *)tn_init, ARRAY_SIZE(tn_init));

		win->w_ptyfd = fd;
		memmove(&win->w_telsa, &res->ai_addr, sizeof(res->ai_addr));
		freeaddrinfo(res0);
		return 0;
	}
	return -1;
}

int TelIsline(Window *win)
{
	(void)win; /* unused */
	return !fore->w_telropts[TO_SGA];
}

void TelProcessLine(char **bufpp, size_t *lenp)
{
	int echo = !fore->w_telropts[TO_ECHO];
	unsigned char c;
	char *tb;
	size_t tl;

	char *buf = *bufpp;
	size_t l = *lenp;
	while (l--) {
		c = *(unsigned char *)buf++;
		if (fore->w_telbufl + 2 >= IOSIZE) {
			WBell(fore, visual_bell);
			continue;
		}
		if (c == '\r') {
			if (echo)
				WriteString(fore, "\r\n", 2);
			fore->w_telbuf[fore->w_telbufl++] = '\r';
			fore->w_telbuf[fore->w_telbufl++] = '\n';
			tb = fore->w_telbuf;
			tl = fore->w_telbufl;
			LayProcess(&tb, &tl);
			fore->w_telbufl = 0;
			continue;
		}
		if (c == '\b' && fore->w_telbufl > 0) {
			if (echo) {
				WriteString(fore, (char *)&c, 1);
				WriteString(fore, " ", 1);
				WriteString(fore, (char *)&c, 1);
			}
			fore->w_telbufl--;
		}
		if ((c >= 0x20 && c <= 0x7e) || c >= 0xa0) {
			if (echo)
				WriteString(fore, (char *)&c, 1);
			fore->w_telbuf[fore->w_telbufl++] = c;
		}
	}
	*lenp = 0;
}

int DoTelnet(char *buf, size_t *lenp, int f)
{
	int echo = !fore->w_telropts[TO_ECHO];
	int cmode = fore->w_telropts[TO_SGA];
	int bin = fore->w_telropts[TO_BINARY];
	char *p = buf, *sbuf;
	int trunc = 0;
	int c;
	ssize_t l = *lenp;

	sbuf = p;
	while (l-- > 0) {
		c = *(unsigned char *)p++;
		if (c == TC_IAC || (c == '\r' && (l == 0 || *p != '\n') && cmode && !bin)) {
			if (cmode && echo) {
				WriteString(fore, sbuf, p - sbuf);
				sbuf = p;
			}
			if (f-- <= 0) {
				trunc++;
				l--;
			}
			if (l < 0) {
				p--;	/* drop char */
				break;
			}
			if (l)
				memmove(p + 1, p, l);
			if (c == TC_IAC)
				*p++ = c;
			else if (c == '\r')
				*p++ = 0;
			else if (c == '\n') {
				p[-1] = '\r';
				*p++ = '\n';
			}
		}
	}
	*lenp = p - buf;
	return trunc;
}

/* modifies data in-place, returns new length */
int TelIn(Window *win, char *buf, size_t len, int free)
{
	char *rp, *wp;
	int c;

	rp = wp = buf;
	while (len-- > 0) {
		c = *(unsigned char *)rp++;

		if (win->w_telstate >= TC_WILL && win->w_telstate <= TC_DONT) {
			TelDocmd(win, win->w_telstate, c);
			win->w_telstate = 0;
			continue;
		}
		if (win->w_telstate == TC_SB || win->w_telstate == TC_SE) {
			if (win->w_telstate == TC_SE && c == TC_IAC)
				win->w_telsubidx--;
			if (win->w_telstate == TC_SE && c == TC_SE) {
				win->w_telsubidx--;
				TelDosub(win);
				win->w_telstate = 0;
				continue;
			}
			if (win->w_telstate == TC_SB && c == TC_IAC)
				win->w_telstate = TC_SE;
			else
				win->w_telstate = TC_SB;
			win->w_telsubbuf[win->w_telsubidx] = c;
			if (win->w_telsubidx < (int)ARRAY_SIZE(win->w_telsubbuf) - 1)
				win->w_telsubidx++;
			continue;
		}
		if (win->w_telstate == TC_IAC) {
			if ((c >= TC_WILL && c <= TC_DONT) || c == TC_SB) {
				win->w_telsubidx = 0;
				win->w_telstate = c;
				continue;
			}
			win->w_telstate = 0;
			if (c != TC_IAC)
				continue;
		} else if (c == TC_IAC) {
			win->w_telstate = c;
			continue;
		}
		if (win->w_telstate == '\r') {
			win->w_telstate = 0;
			if (c == 0)
				continue;	/* suppress trailing \0 */
		} else if (c == '\n' && !win->w_telropts[TO_SGA]) {
			/* oops... simulate terminal line mode: insert \r */
			if (wp + 1 == rp) {
				if (free-- > 0) {
					if (len)
						memmove(rp + 1, rp, len);
					rp++;
					*wp++ = '\r';
				}
			} else
				*wp++ = '\r';
		}
		if (c == '\r')
			win->w_telstate = c;
		*wp++ = c;
	}
	return wp - buf;
}

static void TelReply(Window *win, char *str, size_t len)
{
	if (len == 0)
		return;
	if (win->w_inlen + len > IOSIZE) {
		Msg(0, "Warning: telnet protocol overrun!");
		return;
	}
	memmove(win->w_inbuf + win->w_inlen, str, len);
	win->w_inlen += len;
}

static void TelDocmd(Window *win, int cmd, int opt)
{
	unsigned char b[3];
	int repl = 0;

	switch (cmd) {
	case TC_WILL:
		if (win->w_telropts[opt] || opt == TO_TM)
			return;
		repl = TC_DONT;
		if (opt == TO_ECHO || opt == TO_SGA || opt == TO_BINARY) {
			win->w_telropts[opt] = 1;
			/* setcon(); */
			repl = TC_DO;
		}
		break;
	case TC_WONT:
		if (!win->w_telropts[opt] || opt == TO_TM)
			return;
		repl = TC_DONT;
		win->w_telropts[opt] = 0;
		break;
	case TC_DO:
		if (win->w_telmopts[opt])
			return;
		repl = TC_WONT;
		if (opt == TO_TTYPE || opt == TO_SGA || opt == TO_BINARY || opt == TO_NAWS || opt == TO_TM
		    || opt == TO_LFLOW) {
			repl = TC_WILL;
			win->w_telmopts[opt] = 1;
		}
		win->w_telmopts[TO_TM] = 0;
		break;
	case TC_DONT:
		if (!win->w_telmopts[opt])
			return;
		repl = TC_WONT;
		win->w_telmopts[opt] = 0;
		break;
	}
	b[0] = TC_IAC;
	b[1] = repl;
	b[2] = opt;

	TelReply(win, (char *)b, 3);
	if (cmd == TC_DO && opt == TO_NAWS)
		TelWindowSize(win);
}

static void TelDosub(Window *win)
{
	char trepl[MAXTERMLEN + 6 + 1];
	size_t len;

	switch (win->w_telsubbuf[0]) {
	case TO_TTYPE:
		if (win->w_telsubidx != 2 || win->w_telsubbuf[1] != 1)
			return;
		len = strlen(screenterm);
		if (len >= MAXTERMLEN)
			break;
		sprintf(trepl, "%c%c%c%c%s%c%c", TC_IAC, TC_SB, TO_TTYPE, 0, screenterm, TC_IAC, TC_SE);
		TelReply(win, trepl, len + 6);
		break;
	case TO_LFLOW:
		if (win->w_telsubidx != 2)
			return;
		break;
	default:
		break;
	}
}

void TelBreak(Window *win)
{
	static unsigned char tel_break[] = { TC_IAC, TC_BREAK };
	TelReply(win, (char *)tel_break, 2);
}

void TelWindowSize(Window *win)
{
	char s[20], trepl[20], *t;

	if (win->w_width == 0 || win->w_height == 0 || !win->w_telmopts[TO_NAWS])
		return;
	sprintf(s, "%c%c%c%c%c%c%c%c%c", TC_SB, TC_SB, TO_NAWS, win->w_width / 256, win->w_width & 255, win->w_height / 256,
		win->w_height & 255, TC_SE, TC_SE);
	t = trepl;
	for (size_t i = 0; i < 9; i++)
		if ((unsigned char)(*t++ = s[i]) == TC_IAC)
			*t++ = (char)TC_IAC;
	trepl[0] = (char)TC_IAC;
	t[-2] = (char)TC_IAC;
	TelReply(win, trepl, t - trepl);
}

static char tc_s[] = TC_S;
static char to_s[] = TO_S;

void TelStatus(Window *win, char *buf, size_t len)
{
	(void)len; /* unused */

	*buf++ = '[';
	for (size_t i = 0; to_s[i]; i++) {
		if (to_s[i] == ' ' || win->w_telmopts[i] == 0)
			continue;
		*buf++ = to_s[i];
	}
	*buf++ = ':';
	for (size_t i = 0; to_s[i]; i++) {
		if (to_s[i] == ' ' || win->w_telropts[i] == 0)
			continue;
		*buf++ = to_s[i];
	}
	if (win->w_telstate == TEL_CONNECTING)
		buf[-1] = 'C';
	else if (win->w_telstate && win->w_telstate != '\r') {
		*buf++ = ':';
		*buf++ = tc_s[win->w_telstate - TC_SE];
	}
	*buf++ = ']';
	*buf = 0;
	return;
}

#endif				/* ENABLE_TELNET */
