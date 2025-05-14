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

#include "fileio.h"

#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <pwd.h>

#include <signal.h>

#include "screen.h"

#include "misc.h"
#include "process.h"
#include "termcap.h"
#include "dumptermcap.h"
#include "encoding.h"

static char *CatExtra(char *, char *);
static char *findrcfile(char *);

char *rc_name = "";
int rc_recursion = 0;

static char *CatExtra(char *str1, char *str2)
{
	char *cp;
	size_t len1, len2;
	bool add_colon;

	len1 = strlen(str1);
	if (len1 == 0)
		return str2;
	add_colon = (str1[len1 - 1] != ':');
	if (str2) {
		len2 = strlen(str2);
		if ((cp = realloc(str2, len1 + len2 + add_colon + 1)) == NULL)
			Panic(0, "%s", strnomem);
		memmove(cp + len1 + add_colon, cp, len2 + 1);
	} else {
		if ((cp = malloc(len1 + add_colon + 1)) == NULL)
			Panic(0, "%s", strnomem);
		cp[len1 + add_colon] = '\0';
	}
	memmove(cp, str1, len1);
	if (add_colon)
		cp[len1] = ':';

	return cp;
}

static char *findrcfile(char *rcfile)
{
	char buf[256];
	char *p;

	/* Tilde prefix support courtesy <hesso@pool.math.tu-berlin.de>,
	 * taken from a Debian patch. */
	if (rcfile && *rcfile == '~') {
		static char rcfilename_tilde_exp[MAXPATHLEN + 1];
		char *slash_position = strchr(rcfile, '/');
		if (slash_position == rcfile + 1) {
			if (!home) {
				Msg(0, "%s: source: tilde expansion failed", rc_name);
				return NULL;
			}
			snprintf(rcfilename_tilde_exp, MAXPATHLEN, "%s/%s", home, rcfile + 2);
		} else if (slash_position) {
			struct passwd *p;
			*slash_position = 0;
			p = getpwnam(rcfile + 1);
			if (!p) {
				Msg(0, "%s: source: tilde expansion failed for user %s", rc_name, rcfile + 1);
				return NULL;
			}
			snprintf(rcfilename_tilde_exp, MAXPATHLEN, "%s/%s", p->pw_dir, slash_position + 1);
		} else {
			Msg(0, "%s: source: illegal tilde expression.", rc_name);
			return NULL;
		}
		rcfile = rcfilename_tilde_exp;
	}

	if (rcfile) {
		char *rcend = strrchr(rc_name, '/');
		if (*rcfile != '/' && rcend && (rcend - rc_name) + strlen(rcfile) + 2 < ARRAY_SIZE(buf)) {
			strncpy(buf, rc_name, rcend - rc_name + 1);
			strncpy(buf + (rcend - rc_name) + 1, rcfile, 256 - (rcend - rc_name));
			if (access(buf, R_OK) == 0)
				return SaveStr(buf);
		}
		return SaveStr(rcfile);
	}
	if ((p = getenv("SCREENRC")) != NULL && *p != '\0') {
		return SaveStr(p);
	} else {
		if (strlen(home) > ARRAY_SIZE(buf) - 12)
			Panic(0, "Rc: home too large");
		sprintf(buf, "%s/.screenrc", home);
		return SaveStr(buf);
	}
}

/*
 * this will be called twice:
 * 1) rcfilename = "/etc/screenrc"
 * 2) rcfilename = RcFileName
 */
int StartRc(char *rcfilename, int nopanic)
{
	int argc, len;
	char *p, *cp;
	char buf[2048];
	char *args[MAXARGS];
	int argl[MAXARGS];
	FILE *fp;
	char *oldrc_name = rc_name;

	/* always fix termcap/info capabilities */
	extra_incap = CatExtra("TF", extra_incap);

	/* Special settings for vt100 and others */
	if (display && (!strncmp(D_termname, "vt", 2) || !strncmp(D_termname, "xterm", 5)))
		extra_incap =
		    CatExtra
		    ("xn:f0=\033Op:f1=\033Oq:f2=\033Or:f3=\033Os:f4=\033Ot:f5=\033Ou:f6=\033Ov:f7=\033Ow:f8=\033Ox:f9=\033Oy:f.=\033On:f,=\033Ol:fe=\033OM:f+=\033Ok:f-=\033Om:f*=\033Oj:f/=\033Oo:fq=\033OX",
		     extra_incap);

	rc_name = findrcfile(rcfilename);

	if (rc_name == NULL || (fp = secfopen(rc_name, "r")) == NULL) {
		const char *rc_nonnull = rc_name ? rc_name : rcfilename;
		if (!rc_recursion && RcFileName && !strcmp(RcFileName, rc_nonnull)) {
			/*
			 * User explicitly gave us that name,
			 * this is the only case, where we get angry, if we can't read
			 * the file.
			 */
			if (!nopanic)
				Panic(0, "Unable to open \"%s\".", rc_nonnull);
			/* possibly NOTREACHED */
		}
		if (rc_name)
			Free(rc_name);
		rc_name = oldrc_name;
		return 1;
	}
	while (fgets(buf, ARRAY_SIZE(buf), fp) != NULL) {
		if ((p = strrchr(buf, '\n')) != NULL)
			*p = '\0';
		if ((argc = Parse(buf, ARRAY_SIZE(buf), args, argl)) == 0)
			continue;
		if (strcmp(args[0], "echo") == 0) {
			if (!display)
				continue;
			if (argc < 2 || (argc == 3 && strcmp(args[1], "-n")) || argc > 3) {
				Msg(0, "%s: 'echo [-n] \"string\"' expected.", rc_name);
				continue;
			}
			AddStr(args[argc - 1]);
			if (argc != 3) {
				AddStr("\r\n");
				Flush(0);
			}
		} else if (strcmp(args[0], "sleep") == 0) {
			if (!display)
				continue;
			if (argc != 2) {
				Msg(0, "%s: sleep: one numeric argument expected.", rc_name);
				continue;
			}
			DisplaySleep1000(1000 * atoi(args[1]), 1);
		}
		else if (!strcmp(args[0], "termcapinfo") || !strcmp(args[0], "terminfo"))
		{
			if (!display)
				continue;
			if (argc < 3 || argc > 4) {
				Msg(0, "%s: %s: incorrect number of arguments.", rc_name, args[0]);
				continue;
			}
			for (p = args[1]; p && *p; p = cp) {
				if ((cp = strchr(p, '|')) != NULL)
					*cp++ = '\0';
				len = strlen(p);
				if (p[len - 1] == '*') {
					if (!(len - 1) || !strncmp(p, D_termname, len - 1))
						break;
				} else if (!strcmp(p, D_termname))
					break;
			}
			if (!(p && *p))
				continue;
			extra_incap = CatExtra(args[2], extra_incap);
			if (argc == 4)
				extra_outcap = CatExtra(args[3], extra_outcap);
		} else if (!strcmp(args[0], "source")) {
			if (rc_recursion <= 10) {
				rc_recursion++;
				(void)StartRc(args[1], 0);
				rc_recursion--;
			}
		}
	}
	fclose(fp);
	Free(rc_name);
	rc_name = oldrc_name;
	return 0;
}

void FinishRc(char *rcfilename)
{
	char buf[2048];
	FILE *fp;
	char *oldrc_name = rc_name;

	rc_name = findrcfile(rcfilename);

	if (rc_name == NULL || (fp = secfopen(rc_name, "r")) == NULL) {
		const char *rc_nonnull = rc_name ? rc_name : rcfilename;
		if (rc_recursion)
			Msg(errno, "%s: source %s", oldrc_name, rc_nonnull);
		else if (RcFileName && !strcmp(RcFileName, rc_nonnull)) {
			/*
			 * User explicitly gave us that name,
			 * this is the only case, where we get angry, if we can't read
			 * the file.
			 */
			Panic(0, "Unable to open \"%s\".", rc_nonnull);
			/* NOTREACHED */
		}
		if (rc_name)
			Free(rc_name);
		rc_name = oldrc_name;
		return;
	}

	while (fgets(buf, ARRAY_SIZE(buf), fp) != NULL)
		RcLine(buf, ARRAY_SIZE(buf));
	(void)fclose(fp);
	Free(rc_name);
	rc_name = oldrc_name;
}

void do_source(char *rcfilename)
{
	if (rc_recursion > 10) {
		Msg(0, "%s: source: recursion limit reached", rc_name);
		return;
	}
	rc_recursion++;
	FinishRc(rcfilename);
	rc_recursion--;
}

/*
 * Running a Command Line in the environment determined by the display.
 * The fore window is taken from the display as well as the user.
 * This is bad when we run detached.
 */
void RcLine(char *ubuf, int ubufl)
{
	char *args[MAXARGS];
	int argl[MAXARGS];

	if (display) {
		fore = D_fore;
		flayer = D_forecv->c_layer;
	} else
		flayer = fore ? fore->w_savelayer : NULL;
	if (Parse(ubuf, ubufl, args, argl) <= 0)
		return;
	if (!display) {
		/* the session owner does it, when there is no display here */
		EffectiveAclUser = users;
	}
	DoCommand(args, argl);
	EffectiveAclUser = NULL;
}


static void putc_encoded(FILE *f, uint32_t c, uint32_t font, int encoding) {
	/* Filler character which is used for double-width characters. */
	if (c == 0xff && font == 0xff)
		return;

	char buf[10];
	int length = EncodeChar(buf, c, encoding, NULL);
	if (length < 0) {
		return;
	}
	buf[length] = 0;
	fputs(buf, f);
}

/*
 * needs display for copybuffer access and termcap dumping
 */
void WriteFile(struct acluser *user, char *fn, int dump)
{
	/* dump==0:   create .termcap,
	 * dump==1:   hardcopy,
	 * dump==2:   BUFFERFILE
	 * dump==1:   scrollback,
	 */
	int i, j, k;
	char *c;
	FILE *f;
	char fnbuf[FILENAME_MAX];
	char *mode = "w";
	int public = 0;
	struct stat stb, stb2;
	int fd, exists = 0;

	switch (dump) {
	case DUMP_TERMCAP:
		if (fn == NULL) {
			i = SocketName - SocketPath;
			if (i > (int)ARRAY_SIZE(fnbuf) - 9)
				i = 0;
			strncpy(fnbuf, SocketPath, i);
			strncpy(fnbuf + i, ".termcap", 9);
			fn = fnbuf;
		}
		break;
	case DUMP_HARDCOPY:
	case DUMP_SCROLLBACK:
		if (fn == NULL) {
			if (fore == NULL)
				return;
			if (hardcopydir && *hardcopydir && strlen(hardcopydir) < ARRAY_SIZE(fnbuf) - 21)
				sprintf(fnbuf, "%s/hardcopy.%d", hardcopydir, fore->w_number);
			else
				sprintf(fnbuf, "hardcopy.%d", fore->w_number);
			fn = fnbuf;
		}
		if (hardcopy_append && !access(fn, W_OK))
			mode = "a";
		break;
	case DUMP_EXCHANGE:
		if (fn == NULL) {
			strncpy(fnbuf, BufferFile, ARRAY_SIZE(fnbuf) - 1);
			fnbuf[ARRAY_SIZE(fnbuf) - 1] = 0;
			fn = fnbuf;
		}
		public = !strcmp(fn, DEFAULT_BUFFERFILE);
		exists = !lstat(fn, &stb);
		if (public && exists && (S_ISLNK(stb.st_mode) || stb.st_nlink > 1)) {
			Msg(0, "No write to links, please.");
			return;
		}
		break;
	}

	if (UserContext() > 0) {
		if (dump == DUMP_EXCHANGE && public) {
			if (exists) {
				if ((fd = open(fn, O_WRONLY, 0666)) >= 0) {
					if (fstat(fd, &stb2) == 0
					    && stb.st_dev == stb2.st_dev
					    && stb.st_ino == stb2.st_ino) {
						if (ftruncate(fd, 0) != 0) {
							close(fd);
							fd = -1;
						}
					} else {
						close(fd);
						fd = -1;
					}
				}
			} else
				fd = open(fn, O_WRONLY | O_CREAT | O_EXCL, 0666);
			f = fd >= 0 ? fdopen(fd, mode) : NULL;
		} else
			f = fopen(fn, mode);
		if (f == NULL) {
			UserReturn(0);
		} else {
			uint32_t *p, *pf;
			switch (dump) {
			case DUMP_HARDCOPY:
			case DUMP_SCROLLBACK:
				if (!fore)
					break;
				if (*mode == 'a') {
					putc('>', f);
					for (j = fore->w_width - 2; j > 0; j--)
						putc('=', f);
					fputs("<\n", f);
				}
				if (dump == DUMP_SCROLLBACK) {
					for (i = fore->w_histheight - fore->w_scrollback_height; i < fore->w_histheight; i++) {
						p = (WIN(i)->image);
						pf = WIN(i)->font;
						for (k = fore->w_width - 1; k >= 0 && p[k] == ' '; k--) ;
						for (j = 0; j <= k; j++)
							putc_encoded(f, p[j], pf[j], fore->w_encoding);
						putc('\n', f);
					}
				}
				for (i = 0; i < fore->w_height; i++) {
					p = fore->w_mlines[i].image;
					pf = fore->w_mlines[i].font;
					for (k = fore->w_width - 1; k >= 0 && p[k] == ' '; k--) ;
					for (j = 0; j <= k; j++)
						putc_encoded(f, p[j], pf[j], fore->w_encoding);
					putc('\n', f);
				}
				break;
			case DUMP_TERMCAP:
				DumpTermcap(fore->w_aflag, f);
				break;
			case DUMP_EXCHANGE:
				c = user->u_plop.buf;
				for (i = user->u_plop.len; i-- > 0; c++)
					if (*c == '\r' && (i == 0 || c[1] != '\n'))
						putc('\n', f);
					else
						putc(*c, f);
				break;
			}
			(void)fclose(f);
			UserReturn(1);
		}
	}
	if (UserStatus() <= 0)
		Msg(0, "Cannot open \"%s\"", fn);
	else if (display && !*rc_name) {
		switch (dump) {
		case DUMP_TERMCAP:
			Msg(0, "Termcap entry written to \"%s\".", fn);
			break;
		case DUMP_HARDCOPY:
		case DUMP_SCROLLBACK:
			Msg(0, "Screen image %s to \"%s\".", (*mode == 'a') ? "appended" : "written", fn);
			break;
		case DUMP_EXCHANGE:
			Msg(0, "Copybuffer written to \"%s\".", fn);
		}
	}
}

/*
 * returns an allocated buffer which holds a copy of the file named filename.
 * lenp (if nonzero) points to a location, where the buffer size should be
 * stored.
 */
char *ReadFile(char *filename, int *lenp)
{
	FILE *file;
	int l, size;
	char *buf = NULL;

	if ((file = secfopen(filename, "r")) == NULL) {
		Msg(errno, "no %s -- no slurp", filename);
		return NULL;
	}

	if (fseek(file, 0L, SEEK_END) < 0) {
		fclose(file);
		Msg(errno, "fseek %s", filename);
		return NULL;
	}

	size = ftell(file);
	if (size < 0) {
		fclose(file);
		Msg(errno, "ftell %s", filename);
		return NULL;
	}
	if ((buf = malloc(size)) == NULL) {
		fclose(file);
		Msg(0, "%s", strnomem);
		return NULL;
	}
	if (fseek(file, 0L, SEEK_SET) < 0) {
		free(buf);
		fclose(file);
		Msg(errno, "fseek %s", filename);
		return NULL;
	}
	errno = 0;

	if ((l = fread(buf, sizeof(char), size, file)) != size) {
		Msg(errno, "Got only %d bytes from %s", l, filename);
	}
	fclose(file);
	*lenp = l;
	return buf;
}

void KillBuffers(void)
{
	if (UserContext() > 0)
		UserReturn(unlink(BufferFile) ? errno : 0);
	errno = UserStatus();
	Msg(errno, "%s %sremoved", BufferFile, errno ? "not " : "");
}

/*
 * (Almost) secure open and fopen...
 */

FILE *secfopen(char *name, char *mode)
{
	FILE *fi;

	xseteuid(real_uid);
	xsetegid(real_gid);
	fi = fopen(name, mode);
	xseteuid(eff_uid);
	xsetegid(eff_gid);
	return fi;
}

int secopen(char *name, int flags, int mode)
{
	int fd;

	xseteuid(real_uid);
	xsetegid(real_gid);
	fd = open(name, flags, mode);
	xseteuid(eff_uid);
	xsetegid(eff_gid);
	return fd;
}

int printpipe(Window *p, char *cmd)
{
	int pi[2];
	if (pipe(pi)) {
		WMsg(p, errno, "printing pipe");
		return -1;
	}
	switch (fork()) {
	case -1:
		WMsg(p, errno, "printing fork");
		return -1;
	case 0:
		display = p->w_pdisplay;
		displays = NULL;
		ServerSocket = -1;
		close(0);
		if (dup(pi[0]) < 0)
			Panic(errno, "printpipe dup");
		closeallfiles(0);
		if (setgid(real_gid) || setuid(real_uid))
			Panic(errno, "printpipe setuid");
		eff_uid = real_uid;
		eff_gid = real_gid;
#ifdef SIGPIPE
		xsignal(SIGPIPE, SIG_DFL);
#endif
		execl("/bin/sh", "sh", "-c", cmd, NULL);
		Panic(errno, "/bin/sh");
	default:
		break;
	}
	close(pi[0]);
	return pi[1];
}

int readpipe(char **cmdv)
{
	int pi[2];

	if (pipe(pi)) {
		Msg(errno, "pipe");
		return -1;
	}
	switch (fork()) {
	case -1:
		Msg(errno, "fork");
		return -1;
	case 0:
		displays = NULL;
		ServerSocket = -1;
		close(1);
		if (dup(pi[1]) != 1) {
			close(pi[1]);
			Panic(0, "dup");
		}
		closeallfiles(1);
		if (setgid(real_gid) || setuid(real_uid)) {
			close(1);
			Panic(errno, "setuid/setgid");
		}
		eff_uid = real_uid;
		eff_gid = real_gid;
		execvp(*cmdv, cmdv);
		close(1);
		Panic(errno, "%s", *cmdv);
	default:
		break;
	}
	close(pi[1]);
	return pi[0];
}
