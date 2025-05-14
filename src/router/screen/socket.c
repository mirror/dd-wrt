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

#include "socket.h"

#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#ifdef _OpenBSD_
#include <sys/uio.h>
#endif
#include <sys/un.h>
#include <utime.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#if ENABLE_PAM
  #include <security/pam_appl.h>
#else
  #ifdef _PWD_H_
    #include <pwd.h>
  #else
    #include <shadow.h>
  #endif /* PWD_H */
#endif

#include "screen.h"

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#else
#include <sys/dir.h>
#define dirent direct
#endif

#ifndef CMSG_LEN
#define CMSG_LEN(length) ((_CMSG_DATA_ALIGN(sizeof(struct cmsghdr))) + (length))
#endif
#ifndef CMSG_SPACE
#define CMSG_SPACE(length) ((_CMSG_DATA_ALIGN(sizeof(struct cmsghdr))) + (_CMSG_DATA_ALIGN(length)))
#endif

#include "encoding.h"
#include "fileio.h"
#include "list_generic.h"
#include "misc.h"
#include "process.h"
#include "resize.h"
#include "termcap.h"
#include "tty.h"
#include "utmp.h"

static int CheckPid(pid_t);
static void ExecCreate(Message *);
static void DoCommandMsg(Message *);
static void FinishAttach(Message *);
static void FinishDetach(Message *);
static void AskPassword(Message *);
static bool CheckPassword(const char *password);
static void PasswordProcessInput(char *, size_t);

#define SOCKMODE (S_IWRITE | S_IREAD | (displays ? S_IEXEC : 0) | (multi ? 1 : 0))

/*
 *  Socket directory manager
 *
 *  fdp: pointer to store the first good socket.
 *  nfoundp: pointer to store the number of sockets found matching.
 *  notherp: pointer to store the number of sockets not matching.
 *  match: string to match socket name.
 *
 *  The socket directory must be in SocketPath!
 *  The global variables LoginName, multi, rflag, xflag, dflag,
 *  quietflag, SocketPath are used.
 *
 *  The first good socket is stored in fdp and its name is
 *  appended to SocketPath.
 *  If none exists or fdp is NULL SocketPath is not changed.
 *
 *  Returns: number of good sockets.
 *
 */

int FindSocket(int *fdp, int *nfoundp, int *notherp, char *match)
{
	DIR *dirp;
	struct dirent *dp;
	struct stat st;
	int mode;
	int sdirlen;
	int matchlen = 0;
	char *name, *n;
	int firsts = -1, sockfd;
	char *firstn = NULL;
	int nfound = 0, ngood = 0, ndead = 0, nwipe = 0, npriv = 0;
	int nperfect = 0;
	struct sent {
		struct sent *next;
		int mode;
		char *name;
	} *slist, **slisttail, *sent, *nsent;

	if (match) {
		matchlen = strlen(match);
		if (matchlen > FILENAME_MAX)
			matchlen = FILENAME_MAX;
	}

	/*
	 * SocketPath contains the socket directory.
	 * At the end of FindSocket the socket name will be appended to it.
	 * Thus FindSocket() can only be called once!
	 */
	sdirlen = strlen(SocketPath);

	xseteuid(real_uid);
	xsetegid(real_gid);

	if ((dirp = opendir(SocketPath)) == NULL)
		Panic(errno, "Cannot opendir %s", SocketPath);

	slist = NULL;
	slisttail = &slist;
	while ((dp = readdir(dirp))) {
		int cmatch = 0;
		name = dp->d_name;
		if (*name == 0 || *name == '.' || strlen(name) > 2 * MAXSTR)
			continue;
		if (matchlen) {
			n = name;
			/* if we don't want to match digits. Skip them */
			if ((*match <= '0' || *match > '9') && (*n > '0' && *n <= '9')) {
				while (*n >= '0' && *n <= '9')
					n++;
				if (*n == '.')
					n++;
			}
			/* the tty prefix is optional */
			if (strncmp(match, "tty", 3) && strncmp(n, "tty", 3) == 0)
				n += 3;
			if (strncmp(match, n, matchlen)) {
				if (n == name && *match > '0' && *match <= '9') {
					while (*n >= '0' && *n <= '9')
						n++;
					if (*n == '.')
						n++;
					if (strncmp(match, n, matchlen))
						continue;
				} else
					continue;
			} else
				cmatch = (*(n + matchlen) == 0);
		}
		sprintf(SocketPath + sdirlen, "/%s", name);

		errno = 0;
		if (stat(SocketPath, &st)) {
			continue;
		}

#ifdef SOCKET_DIR	/* if SOCKET_DIR is not defined, the socket is in $HOME.
			   in that case it does not make sense to compare uids. */
		if (st.st_uid != real_uid)
			continue;
#endif
		mode = (int)st.st_mode & 0777;
		if (multi && ((mode & 0677) != 0601)) {
			if (strcmp(multi, LoginName)) {
				mode = -4;
			} else {
			}
		}
		if ((sent = malloc(sizeof(struct sent))) == NULL)
			continue;
		sent->next = NULL;
		sent->name = SaveStr(name);
		sent->mode = mode;
		*slisttail = sent;
		slisttail = &sent->next;
		nfound++;
		sockfd = MakeClientSocket(0);
		/* MakeClientSocket sets ids back to eff */
		xseteuid(real_uid);
		xsetegid(real_gid);
		if (sockfd == -1) {
			sent->mode = -3;
#ifndef SOCKDIR_IS_LOCAL_TO_HOST
			/* Unreachable - it is dead if we detect that it's local
			 * or we specified a match
			 */
			n = name + strlen(name) - 1;
			while (n != name && *n != '.')
				n--;
			if (matchlen == 0 && !(*n == '.' && n[1] && strncmp(HostName, n + 1, strlen(n + 1)) == 0)) {
				npriv++;	/* a good socket that was not for us */
				continue;
			}
#endif
			ndead++;
			sent->mode = -1;
			if (wipeflag) {
				if (unlink(SocketPath) == 0) {
					sent->mode = -2;
					nwipe++;
				}
			}
			continue;
		}

		mode &= 0776;
		/* Shall we connect ? */

		/*
		 * mode 600: socket is detached.
		 * mode 700: socket is attached.
		 * xflag implies rflag here.
		 *
		 * fail, when socket mode mode is not 600 or 700
		 * fail, when we want to detach w/o reattach, but it already is detached.
		 * fail, when we only want to attach, but mode 700 and not xflag.
		 * fail, if none of dflag, rflag, xflag is set.
		 */
		if ((mode != 0700 && mode != 0600) ||
		    (dflag && !rflag && !xflag && mode == 0600) ||
		    (!dflag && rflag && mode == 0700 && !xflag) || (!dflag && !rflag && !xflag)) {
			close(sockfd);
			npriv++;	/* a good socket that was not for us */
			continue;
		}
		ngood++;
		if (cmatch)
			nperfect++;
		if (fdp && (firsts == -1 || (cmatch && nperfect == 1))) {
			if (firsts != -1)
				close(firsts);
			firsts = sockfd;
			firstn = sent->name;
		} else {
			close(sockfd);
		}
	}
	(void)closedir(dirp);
	if (!lsflag && nperfect == 1)
		ngood = nperfect;
	if (nfound && (lsflag || ngood != 1) && !quietflag) {
		switch (ngood) {
		case 0:
			Msg(0, nfound > 1 ? "There are screens on:" : "There is a screen on:");
			break;
		case 1:
			Msg(0, nfound > 1 ? "There are several screens on:" : "There is a suitable screen on:");
			break;
		default:
			Msg(0, "There are several suitable screens on:");
			break;
		}
		for (sent = slist; sent; sent = sent->next) {
			switch (sent->mode) {
			case 0700:
				printf("\t%s\t(Attached)\n", sent->name);
				break;
			case 0600:
				printf("\t%s\t(Detached)\n", sent->name);
				break;
			case 0701:
				printf("\t%s\t(Multi, attached)\n", sent->name);
				break;
			case 0601:
				printf("\t%s\t(Multi, detached)\n", sent->name);
				break;
			case -1:
				/* No trigraphs here! */
				printf("\t%s\t(Dead ?%c?)\n", sent->name, '?');
				break;
			case -2:
				printf("\t%s\t(Removed)\n", sent->name);
				break;
			case -3:
				printf("\t%s\t(Remote or dead)\n", sent->name);
				break;
			case -4:
				printf("\t%s\t(Private)\n", sent->name);
				break;
			}
		}
	}
	if (ndead && !quietflag) {
		char *m = "Remove dead screens with 'screen -wipe'.";
		if (wipeflag)
			Msg(0, "%d socket%s wiped out.", nwipe, nwipe > 1 ? "s" : "");
		else
			Msg(0, m, ndead > 1 ? "s" : "", ndead > 1 ? "" : "es");
	}
	if (firsts != -1) {
		sprintf(SocketPath + sdirlen, "/%s", firstn);
		*fdp = firsts;
	} else
		SocketPath[sdirlen] = 0;
	for (sent = slist; sent; sent = nsent) {
		nsent = sent->next;
		free(sent->name);
		free((char *)sent);
	}
	xseteuid(eff_uid);
	xsetegid(eff_gid);
	if (notherp)
		*notherp = npriv;
	if (nfoundp)
		*nfoundp = nfound - nwipe;
	return ngood;
}

/*
**
**        Socket/pipe create routines
**
*/

int MakeServerSocket(void)
{
	int s;
	struct sockaddr_un a;
	struct stat st;

	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
		Panic(errno, "socket");
	a.sun_family = AF_UNIX;
	strncpy(a.sun_path, SocketPath, ARRAY_SIZE(a.sun_path));
	a.sun_path[ARRAY_SIZE(a.sun_path) - 1] = 0;
	xseteuid(real_uid);
	xsetegid(real_gid);
	if (connect(s, (struct sockaddr *)&a, strlen(SocketPath) + 2) != -1) {
		if (quietflag) {
			Kill(D_userpid, SIG_BYE);
			/*
			 * oh, well. nobody receives that return code. papa
			 * dies by signal.
			 */
			eexit(11);
		}
		Msg(0, "There is already a screen running on %s.", Filename(SocketPath));
		if (stat(SocketPath, &st) == -1)
			Panic(errno, "stat");
#ifdef SOCKET_DIR	/* if SOCKET_DIR is not defined, the socket is in $HOME.
			   in that case it does not make sense to compare uids. */
		if (st.st_uid != real_uid)
			Panic(0, "Unfortunately you are not its owner.");
#endif
		if ((st.st_mode & 0700) == 0600)
			Panic(0, "To resume it, use \"screen -r\"");
		else
			Panic(0, "It is not detached.");
		/* NOTREACHED */
	}
	(void)unlink(SocketPath);
	if (bind(s, (struct sockaddr *)&a, strlen(SocketPath) + 2) == -1)
		Panic(errno, "bind (%s)", SocketPath);
	chmod(SocketPath, SOCKMODE);
	if (chown(SocketPath, real_uid, real_gid))
		Panic(errno, "chown");
	if (listen(s, 5) == -1)
		Panic(errno, "listen");
#ifdef F_SETOWN
	fcntl(s, F_SETOWN, getpid());
#endif				/* F_SETOWN */
	xseteuid(eff_uid);
	xsetegid(eff_gid);
	return s;
}

int MakeClientSocket(int err)
{
	int s;
	struct sockaddr_un a;

	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
		Panic(errno, "socket");
	a.sun_family = AF_UNIX;
	strncpy(a.sun_path, SocketPath, ARRAY_SIZE(a.sun_path));
	a.sun_path[ARRAY_SIZE(a.sun_path) - 1] = 0;
	xseteuid(real_uid);
	xsetegid(real_gid);
	if (connect(s, (struct sockaddr *)&a, strlen(SocketPath) + 2) == -1) {
		if (err)
			Msg(errno, "%s: connect", SocketPath);
		close(s);
		s = -1;
	}
	xseteuid(eff_uid);
	xsetegid(eff_gid);
	return s;
}

/*
**
**       Message send and receive routines
**
*/

void SendCreateMsg(char *sty, struct NewWindow *nwin)
{
	int s;
	Message m;
	char *p;
	size_t len, n;
	char **av;

	if (strlen(sty) > FILENAME_MAX)
		sty[FILENAME_MAX] = 0;
	if (strlen(sty) > 2 * MAXSTR - 1)
		sty[2 * MAXSTR - 1] = 0;
	sprintf(SocketPath + strlen(SocketPath), "/%s", sty);
	if ((s = MakeClientSocket(1)) == -1)
		exit(1);
	memset((char *)&m, 0, sizeof(Message));
	m.type = MSG_CREATE;
	strncpy(m.m_tty, attach_tty, ARRAY_SIZE(m.m_tty) - 1);
	m.m_tty[ARRAY_SIZE(m.m_tty) - 1] = 0;
	p = m.m.create.line;
	n = 0;
	if (nwin->args != nwin_undef.args)
		for (av = nwin->args; *av && n < MAXARGS - 1; ++av, ++n) {
			len = strlen(*av) + 1;
			if (p + len >= m.m.create.line + ARRAY_SIZE(m.m.create.line) - 1)
				break;
			strcpy(p, *av);
			p += len;
		}
	if (nwin->aka != nwin_undef.aka && p + strlen(nwin->aka) + 1 < m.m.create.line + ARRAY_SIZE(m.m.create.line))
		strcpy(p, nwin->aka);
	else
		*p = '\0';
	m.m.create.nargs = n;
	m.m.create.aflag = nwin->aflag;
	m.m.create.flowflag = nwin->flowflag;
	m.m.create.lflag = nwin->lflag;
	m.m.create.Lflag = nwin->Lflag;
	m.m.create.hheight = nwin->histheight;
	if (getcwd(m.m.create.dir, ARRAY_SIZE(m.m.create.dir)) == NULL) {
		Msg(errno, "getcwd");
		goto end;
	}
	if (nwin->term != nwin_undef.term)
		strncpy(m.m.create.screenterm, nwin->term, MAXTERMLEN);
	m.m.create.screenterm[MAXTERMLEN] = '\0';
	m.protocol_revision = MSG_REVISION;
	if (write(s, (char *)&m, sizeof(Message)) != sizeof(Message))
		Msg(errno, "write");
end:
	close(s);
}

int SendErrorMsg(char *tty, char *buf)
{
	int s;
	int ret = 0;
	Message m;

	strncpy(m.m.message, buf, ARRAY_SIZE(m.m.message) - 1);
	m.m.message[ARRAY_SIZE(m.m.message) - 1] = 0;
	s = MakeClientSocket(0);
	if (s < 0)
		return -1;
	m.type = MSG_ERROR;
	strncpy(m.m_tty, tty, ARRAY_SIZE(m.m_tty) - 1);
	m.m_tty[ARRAY_SIZE(m.m_tty) - 1] = 0;
	m.protocol_revision = MSG_REVISION;
	if (write(s, (char *)&m, sizeof(Message)))
		ret = -2;
	close(s);
	return ret;
}

static void ExecCreate(Message *mp)
{
	struct NewWindow nwin;
	char *args[MAXARGS];
	int n;
	char **pp = args, *p = mp->m.create.line;
	char buf[20];

	nwin = nwin_undef;
	n = mp->m.create.nargs;
	if (n > MAXARGS - 1)
		n = MAXARGS - 1;
	/* ugly hack alert... should be done by the frontend! */
	if (n) {
		int l, num;

		l = strlen(p);
		if (IsNumColon(p, buf, ARRAY_SIZE(buf))) {
			if (*buf)
				nwin.aka = buf;
			num = atoi(p);
			if (num < 0 || num > last_window->w_number)
				num = 0;
			nwin.StartAt = num;
			p += l + 1;
			n--;
		}
	}
	for (; n > 0; n--) {
		*pp++ = p;
		p += strlen(p) + 1;
	}
	*pp = NULL;
	if (*p)
		nwin.aka = p;
	if (*args)
		nwin.args = args;
	nwin.aflag = mp->m.create.aflag;
	nwin.flowflag = mp->m.create.flowflag;
	if (*mp->m.create.dir)
		nwin.dir = mp->m.create.dir;
	nwin.lflag = mp->m.create.lflag;
	nwin.Lflag = mp->m.create.Lflag;
	nwin.histheight = mp->m.create.hheight;
	if (*mp->m.create.screenterm)
		nwin.term = mp->m.create.screenterm;
	MakeWindow(&nwin);
}

static int CheckPid(pid_t pid)
{
	if (pid < 2)
		return -1;
	if (eff_uid == real_uid)
		return kill(pid, 0);
	if (UserContext() > 0)
		UserReturn(kill(pid, 0));
	return UserStatus();
}

static int CreateTempDisplay(Message *m, int recvfd, Window *win)
{
	pid_t pid;
	int attach;
	char *user;
	int i;
	struct mode Mode;
	Display *olddisplays = displays;

	switch (m->type) {
	case MSG_CONT:
	case MSG_ATTACH:
		pid = m->m.attach.apid;
		user = m->m.attach.auser;
		attach = 1;
		break;
	case MSG_DETACH:
	case MSG_POW_DETACH:
		pid = m->m.detach.dpid;
		user = m->m.detach.duser;
		attach = 0;
		break;
	default:
		return -1;
	}

	if (CheckPid(pid)) {
		Msg(0, "Attach attempt with bad pid(%d)!", pid);
		return -1;
	}
	if (recvfd != -1) {
		char ttyname_in_ns[MAXPATHLEN] = {0};
		char *myttyname;
		i = recvfd;
		errno = 0;
		myttyname = GetPtsPathOrSymlink(i);
		if (myttyname && errno == ENODEV) {
			ssize_t ret = readlink(myttyname, ttyname_in_ns,
				       ARRAY_SIZE(ttyname_in_ns));
			if (ret < 0 || (size_t)ret >= ARRAY_SIZE(ttyname_in_ns)) {
				Msg(errno, "Could not perform necessary sanity "
					   "checks on pts device.");
				close(i);
				Kill(pid, SIG_BYE);
				return -1;
			}
			if (strcmp(ttyname_in_ns, m->m_tty)) {
				Msg(errno, "Attach: passed fd does not match "
					   "tty: %s - %s!",
				    ttyname_in_ns,
				    m->m_tty[0] != '\0' ? m->m_tty : "(null)");
				close(i);
				Kill(pid, SIG_BYE);
				return -1;
			}
			/* m->m_tty so far contains the actual name of the pts
			 * device in its namespace (e.g. /dev/pts/0). This name
			 * however is not valid in the current namespace. So
			 * after we verified that the symlink returned by
			 * GetPtsPathOrSymlink() refers to the same pts device
			 * in this namespace we need to update m->m_tty to use
			 * that symlink for all future operations.
			 */
			strncpy(m->m_tty, myttyname, ARRAY_SIZE(m->m_tty) - 1);
			m->m_tty[ARRAY_SIZE(m->m_tty) - 1] = 0;
		} else if (myttyname == NULL || strcmp(myttyname, m->m_tty)) {
			Msg(errno,
			    "Attach: passed fd does not match tty: %s - %s!",
			    m->m_tty, myttyname ? myttyname : "NULL");
			close(i);
			Kill(pid, SIG_BYE);
			return -1;
		}
	} else if ((i = secopen(m->m_tty, O_RDWR | O_NONBLOCK, 0)) < 0) {
		Msg(errno, "Attach: Could not open %s!", m->m_tty);
		Kill(pid, SIG_BYE);
		return -1;
	}

	if (attach)
		Kill(pid, SIGCONT);

	if (attach) {
		if (display || win) {
			int unused_result = write(i, "Attaching from inside of screen?\n", 33);
			(void)unused_result; /* unused */
			close(i);
			Kill(pid, SIG_BYE);
			Msg(0, "Attach msg ignored: coming from inside.");
			return -1;
		}

		if (strcmp(user, LoginName))
			if (*FindUserPtr(user) == NULL) {
				int unused_result = write(i, "Access to session denied.\n", 26);
				(void)unused_result; /* unused */
				close(i);
				Kill(pid, SIG_BYE);
				Msg(0, "Attach: access denied for user %s.", user);
				return -1;
			}
	}

	/* create new display */
	GetTTY(i, &Mode);
	if (MakeDisplay(user, m->m_tty, attach ? m->m.attach.envterm : "", i, pid, &Mode) == NULL) {
		int unused_result = write(i, "Could not make display.\n", 24);
		(void)unused_result; /* unused */
		close(i);
		Msg(0, "Attach: could not make display for user %s", user);
		Kill(pid, SIG_BYE);
		return -1;
	}
	if (attach) {
		D_encoding = m->m.attach.encoding == 1 ? UTF8 : m->m.attach.encoding ? m->m.attach.encoding - 1 : 0;
		if (D_encoding < 0 || !EncodingName(D_encoding))
			D_encoding = 0;
	}

	if (iflag && olddisplays) {
		iflag = false;
		olddisplays->d_NewMode.tio.c_cc[VINTR] = VDISABLE;
		olddisplays->d_NewMode.tio.c_lflag &= ~ISIG;
		SetTTY(olddisplays->d_userfd, &olddisplays->d_NewMode);
	}
	SetMode(&D_OldMode, &D_NewMode, D_flow, iflag);
	SetTTY(D_userfd, &D_NewMode);
	if (fcntl(D_userfd, F_SETFL, FNBLOCK))
		Msg(errno, "Warning: NBLOCK fcntl failed");
	return 0;
}

void ReceiveMsg(void)
{
	int left, len;
	static Message m;
	char *p;
	int ns = ServerSocket;
	Window *win = NULL;
	int recvfd = -1;

	struct sockaddr_un a;
	struct msghdr msg;
	struct iovec iov;
	char control[1024];

	len = sizeof(a);
	if ((ns = accept(ns, (struct sockaddr *)&a, (socklen_t *) &len)) < 0) {
		Msg(errno, "accept");
		return;
	}

	p = (char *)&m;
	left = sizeof(Message);
	memset(&msg, 0, sizeof(struct msghdr));
	iov.iov_base = &m;
	iov.iov_len = left;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_controllen = ARRAY_SIZE(control);
	msg.msg_control = &control;
	while (left > 0) {
		len = recvmsg(ns, &msg, 0);
		if (len < 0 && errno == EINTR)
			continue;
		if (len < 0) {
			close(ns);
			Msg(errno, "read");
			return;
		}
		if (msg.msg_controllen) {
			struct cmsghdr *cmsg;
			for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
				size_t cl;
				char *cp;
				if (cmsg->cmsg_level != SOL_SOCKET || cmsg->cmsg_type != SCM_RIGHTS)
					continue;
				cp = (char *)CMSG_DATA(cmsg);
				cl = cmsg->cmsg_len;
				while (cl >= CMSG_LEN(sizeof(int))) {
					int passedfd;
					memmove(&passedfd, cp, sizeof(int));
					if (recvfd >= 0 && passedfd != recvfd)
						close(recvfd);
					recvfd = passedfd;
					cl -= CMSG_LEN(sizeof(int));
				}
			}
		}
		p += len;
		left -= len;
		break;
	}

	while (left > 0) {
		len = read(ns, p, left);
		if (len < 0 && errno == EINTR)
			continue;
		if (len <= 0)
			break;
		p += len;
		left -= len;
	}

	close(ns);

	if (len < 0) {
		Msg(errno, "read");
		if (recvfd != -1)
			close(recvfd);
		return;
	}
	if (left > 0) {
		if (left != sizeof(Message))
			Msg(0, "Message %d of %d bytes too small", left, (int)sizeof(Message));
		return;
	}
	if (m.protocol_revision != MSG_REVISION) {
		if (recvfd != -1)
			close(recvfd);
		Msg(0, "Invalid message (magic 0x%08x).", m.protocol_revision);
		return;
	}

	if (m.type != MSG_ATTACH && recvfd != -1) {
		close(recvfd);
		recvfd = -1;
	}

	for (display = displays; display; display = display->d_next)
		if (strcmp(D_usertty, m.m_tty) == 0)
			break;
	if (!display) {
		for (win = mru_window; win; win = win->w_prev_mru)
			if (!strcmp(m.m_tty, win->w_tty)) {
				/* XXX: hmmm, rework this? */
				display = win->w_layer.l_cvlist ? win->w_layer.l_cvlist->c_display : NULL;
				break;
			}
	}

	/* Remove the status to prevent garbage on the screen */
	if (display && D_status)
		RemoveStatus();

	if (display && !D_tcinited && m.type != MSG_HANGUP) {
		if (recvfd != -1)
			close(recvfd);
		return;		/* ignore messages for bad displays */
	}

	switch (m.type) {
	case MSG_WINCH:
		if (display)
			CheckScreenSize(1);	/* Change fore */
		break;
	case MSG_CREATE:
		/*
		 * the window that issued the create message need not be an active
		 * window. Then we create the window without having a display.
		 * Resulting in another inactive window.
		 */
		ExecCreate(&m);
		break;
	case MSG_CONT:
		if (display && D_userpid != 0 && kill(D_userpid, 0) == 0)
			break;	/* Intruder Alert */
		/* FALLTHROUGH */

	case MSG_ATTACH:
		if (CreateTempDisplay(&m, recvfd, win))
			break;
		if (do_auth)
			AskPassword(&m);
		else
			FinishAttach(&m);
		break;
	case MSG_ERROR:
		{
			int blocked = D_blocked;
			if (D_blocked == 4)	/* allow error messages while in blanker mode */
				D_blocked = 0;	/* likely they're from failed blanker */
			Msg(0, "%s", m.m.message);
			D_blocked = blocked;
		}
		break;
	case MSG_HANGUP:
		if (!win)	/* ignore hangups from inside */
			Hangup();
		break;
	case MSG_DETACH:
	case MSG_POW_DETACH:
		if (CreateTempDisplay(&m, recvfd, NULL))
			break;
		if (do_auth)
			AskPassword(&m);
		else
			FinishAttach(&m);
		break;
	case MSG_QUERY:
		{
			char *oldSocketPath = SaveStr(SocketPath);
			strncpy(SocketPath, m.m.command.writeback, ARRAY_SIZE(SocketPath));
			int s = MakeClientSocket(0);
			strncpy(SocketPath, oldSocketPath, ARRAY_SIZE(SocketPath));
			Free(oldSocketPath);
			if (s >= 0) {
				queryflag = s;
				DoCommandMsg(&m);
				close(s);
			} else
				queryflag = -1;
			if (CheckPid(m.m.command.apid)) {
				Msg(0, "Query attempt with bad pid(%d)!", m.m.command.apid);
			}
			else {
				Kill(m.m.command.apid, (queryflag >= 0) ? SIGCONT : SIG_BYE);	/* Send SIG_BYE if an error happened */
				queryflag = -1;
			}
		}
		break;
	case MSG_COMMAND:
		DoCommandMsg(&m);
		break;
	default:
		Msg(0, "Invalid message (type %d).", m.type);
	}
}

void ReceiveRaw(int s)
{
	char rd[256];
	ssize_t len = 0;
	struct sockaddr_un a;
	len = sizeof(a);
	if ((s = accept(s, (struct sockaddr *)&a, (socklen_t *)&len)) < 0) {
		Msg(errno, "accept");
		return;
	}
	while ((len = read(s, rd, 255)) > 0) {
		rd[len] = 0;
		printf("%s", rd);
	}
	close(s);
}

/*
 * Set the mode bits of the socket to the current status
 */
int chsock(void)
{
	int ret;
	uid_t euid = geteuid();
	if (euid != real_uid) {
		if (UserContext() <= 0)
			return UserStatus();
	}
	ret = chmod(SocketPath, SOCKMODE);
	/*
	 * Sockets usually reside in the /tmp/ area, where sysadmin scripts
	 * may be happy to remove old files. We manually prevent the socket
	 * from becoming old. (chmod does not touch mtime).
	 */
	(void)utimes(SocketPath, NULL);

	if (euid != real_uid)
		UserReturn(ret);
	return ret;
}

/*
 * Try to recreate the socket/pipe
 */
int RecoverSocket(void)
{
	close(ServerSocket);
	if (geteuid() != real_uid) {
		if (UserContext() > 0)
			UserReturn(unlink(SocketPath));
		(void)UserStatus();
	} else
		(void)unlink(SocketPath);

	if ((ServerSocket = MakeServerSocket()) < 0)
		return 0;
	evdeq(&serv_read);
	serv_read.fd = ServerSocket;
	evenq(&serv_read);
	return 1;
}

static void FinishAttach(Message *m)
{
	pid_t pid;
	int noshowwin;

	pid = D_userpid;

	if (m->m.attach.detachfirst == MSG_DETACH || m->m.attach.detachfirst == MSG_POW_DETACH)
		FinishDetach(m);

	/*
	 * We reboot our Terminal Emulator. Forget all we knew about
	 * the old terminal, reread the termcap entries in .screenrc
	 * (and nothing more from .screenrc is read. Mainly because
	 * I did not check, whether a full reinit is safe. jw)
	 * and /etc/screenrc, and initialise anew.
	 */
	if (extra_outcap)
		free(extra_outcap);
	if (extra_incap)
		free(extra_incap);
	extra_incap = extra_outcap = NULL;
	StartRc(SYSTEM_SCREENRC, 1);
	StartRc(RcFileName, 1);
	if (InitTermcap(m->m.attach.columns, m->m.attach.lines)) {
		FreeDisplay();
		Kill(pid, SIG_BYE);
		return;
	}
	MakeDefaultCanvas();
	InitTerm(m->m.attach.adaptflag);	/* write init string on fd */
	if (displays->d_next == NULL)
		(void)chsock();
	xsignal(SIGHUP, SigHup);
	if (m->m.attach.esc != -1 && m->m.attach.meta_esc != -1) {
		D_user->u_Esc = m->m.attach.esc;
		D_user->u_MetaEsc = m->m.attach.meta_esc;
	}
#ifdef ENABLE_UTMP
	/*
	 * we set the Utmp slots again, if we were detached normally
	 * and if we were detached by ^Z.
	 * don't log zomies back in!
	 */
	RemoveLoginSlot();
	if (displays->d_next == NULL)
		for (Window *win = mru_window; win; win = win->w_prev_mru)
			if (win->w_ptyfd >= 0 && win->w_slot != (slot_t) - 1)
				SetUtmp(win);
#endif

	D_fore = NULL;
	if (layout_attach) {
		Layout *lay = layout_attach;
		if (lay == &layout_last_marker)
			lay = layout_last;
		if (lay) {
			LoadLayout(lay);
			SetCanvasWindow(D_forecv, NULL);
		}
	}
	/*
	 * there may be a window that we remember from last detach:
	 */
	if (D_user->u_detachwin >= 0)
		fore = GetWindowByNumber(D_user->u_detachwin);
	else
		fore = NULL;

	/* Wayne wants us to restore the other window too. */
	if (D_user->u_detachotherwin >= 0)
		D_other = GetWindowByNumber(D_user->u_detachotherwin);

	noshowwin = 0;
	if (*m->m.attach.preselect) {
		if (!strcmp(m->m.attach.preselect, "="))
			fore = NULL;
		else if (!strcmp(m->m.attach.preselect, "-")) {
			fore = NULL;
			noshowwin = 1;
		} else if (!strcmp(m->m.attach.preselect, "+")) {
			struct action newscreen;
			char *na = NULL;
			newscreen.nr = RC_SCREEN;
			newscreen.args = &na;
			newscreen.quiet = 0;
			DoAction(&newscreen);
		} else
			fore = FindNiceWindow(fore, m->m.attach.preselect);
	} else
		fore = FindNiceWindow(fore, NULL);
	if (fore)
		SetForeWindow(fore);
	else if (!noshowwin) {
		if (!AclCheckPermCmd(D_user, ACL_EXEC, &comms[RC_WINDOWLIST])) {
			Display *olddisplay = display;
			flayer = D_forecv->c_layer;
			display_windows(1, WLIST_NUM, NULL);
			noshowwin = 1;
			display = olddisplay;	/* display_windows can change display */
		}
	}
	Activate(0);
	ResetIdle();
	if (!D_fore && !noshowwin)
		ShowWindows(-1);
	if (displays->d_next == NULL && console_window) {
		if (TtyGrabConsole(console_window->w_ptyfd, true, "reattach") == 0)
			Msg(0, "console %s is on window %d", HostName, console_window->w_number);
	}
}

static void FinishDetach(Message *m)
{
	Display *next, **d, *det;
	pid_t pid;

	if (m->type == MSG_ATTACH)
		pid = D_userpid;
	else
		pid = m->m.detach.dpid;

	/* Remove the temporary display prompting for the password from the list */
	for (d = &displays; (det = *d); d = &det->d_next) {
		if (det->d_userpid == pid)
			break;
	}
	if (det) {
		*d = det->d_next;
		det->d_next = NULL;
	}

	for (display = displays; display; display = next) {
		next = display->d_next;
		if (m->type == MSG_POW_DETACH)
			Detach(D_REMOTE_POWER);
		else if (m->type == MSG_DETACH)
			Detach(D_REMOTE);
		else if (m->type == MSG_ATTACH) {
			if (m->m.attach.detachfirst == MSG_POW_DETACH)
				Detach(D_REMOTE_POWER);
			else if (m->m.attach.detachfirst == MSG_DETACH)
				Detach(D_REMOTE);
		}
	}
	display = displays = det;
	if (m->type != MSG_ATTACH) {
		if (display)
			FreeDisplay();
		Kill(pid, SIGCONT);
	}
}

struct pwdata {
	size_t len;
	char buf[MAXLOGINLEN + 1];
	Message m;
};

static void AskPassword(Message *m)
{
	struct pwdata *pwdata;
	char prompt[MAXSTR];
	char *gecos_comma;
	char *realname = NULL;

	pwdata = calloc(1, sizeof(struct pwdata));
	if (!pwdata)
		Panic(0, "%s", strnomem);

	pwdata->len = 0;
	pwdata->m = *m;

	D_processinputdata = pwdata;
	D_processinput = PasswordProcessInput;

	/* if GECOS data is CSV, we only want the text before the first comma */
	if ((gecos_comma = strchr(ppp->pw_gecos, ',')))
		if (!(realname = strndup(ppp->pw_gecos, gecos_comma -  ppp->pw_gecos)))
			gecos_comma = NULL; /* well, it was worth a shot. */

	snprintf(prompt, sizeof(prompt), "\ascreen used by %s%s<%s> on %s.\r\nPassword: ",
		 gecos_comma ? realname : ppp->pw_gecos,
		 ppp->pw_gecos[0] ? " " : "", ppp->pw_name, HostName);

	free(realname);
	AddStr(prompt);
}

#if ENABLE_PAM
static int screen_conv(int num_msg, const struct pam_message **msg,
		struct pam_response **resp, void *data)
{
	(void)num_msg;	/* unused */
	(void)msg;	/* unused */

	*resp = (struct pam_response *)data;
	return PAM_SUCCESS;
}

static bool CheckPassword(const char *password) {
	bool ret = false;

	struct pam_response *reply;

	pam_handle_t *pamh = NULL;
	struct pam_conv pamc;
	int pam_ret;
	char *tty_name;

	reply = (struct pam_response *)malloc(sizeof(struct pam_response));  

	reply[0].resp = strdup(password);  
	reply[0].resp_retcode = 0;  

	pamc.conv = &screen_conv; 
	pamc.appdata_ptr = (void *)reply;
	pam_ret= pam_start("screen", ppp->pw_name, &pamc, &pamh);
	if (pam_ret!= PAM_SUCCESS) {
		return false;
	}

	if (strncmp(attach_tty, "/dev/", 5) == 0) {
		tty_name = attach_tty + 5;
	} else {
		tty_name = attach_tty;
	}
	pam_ret = pam_set_item(pamh, PAM_TTY, tty_name);
	if (pam_ret != PAM_SUCCESS) {
		return false;
	}

	pam_ret = pam_authenticate(pamh, 0);
	pam_end(pamh, pam_ret);

	if (pam_ret == PAM_MAXTRIES) {
		AddStr("\r\nmaximum number of tries exceeded\r\n");
		return false;
	} else if (pam_ret == PAM_ABORT) {
		AddStr("\r\nabort requested by PAM\r\n");
		return false;
	} else if (pam_ret == PAM_SUCCESS) {
		ret = true;
	}

	return ret;
}

#else /* ENABLE_PAM */

static bool CheckPassword(const char *password) {
	bool ret = false;
	char *passwd = 0;

#ifndef _PWD_H
	struct passwd *p;
#else
	struct spwd *p;
#endif

	gid_t gid = getegid();
	uid_t uid = geteuid();

	if (seteuid(0) || setegid(0))
		Panic(0, "\r\ncan't get root uid/gid\r\n");

#ifndef _PWD_H
	p = getpwnam_shadow(ppp->pw_name);
#else
	p = getspnam(ppp->pw_name);
#endif
	if (p == NULL)
		return false;

	if (seteuid(uid) || setegid(gid))
		Panic(0, "\r\ncan't restore uid/gid\r\n");

	if (p == NULL) {
		AddStr("\r\ncan't open passwd file\r\n");
		return false;
	}
#ifndef _PWD_H
	passwd = crypt(password, p->pw_passwd);
	ret    = (strcmp(passwd, p->pw_passwd) == 0);
#else
	passwd = crypt(password, p->sp_pwdp);
	ret = (strcmp(passwd, p->sp_pwdp) == 0);
#endif
	return ret;
}
#endif /* ENABLE_PAM */

static void PasswordProcessInput(char *ibuf, size_t ilen)
{
	struct pwdata *pwdata;
	int c;
	size_t len;
	int pid = D_userpid;

	pwdata = D_processinputdata;
	len = pwdata->len;
	while (ilen-- > 0) {
		c = *(unsigned char *)ibuf++;
		if (c == '\r' || c == '\n') {
			pwdata->buf[len] = 0;

			if (!CheckPassword(pwdata->buf)) {
				/* uh oh, user failed */
				memset(pwdata->buf, 0, sizeof(pwdata->buf));
				AddStr("\r\nPassword incorrect.\r\n");
				D_processinputdata = NULL; /* otherwise freed by FreeDis */
				FreeDisplay();
				Msg(0, "Illegal reattach attempt from terminal %s.", pwdata->m.m_tty);
				free(pwdata);
				Kill(pid, SIG_BYE);
				return;
			}

			/* great, pw matched, all is fine */
			memset(pwdata->buf, 0, sizeof(pwdata->buf));
			AddStr("\r\n");

			D_processinputdata = NULL;
			D_processinput = ProcessInput;
			if (pwdata->m.type == MSG_DETACH || pwdata->m.type == MSG_POW_DETACH)
				FinishDetach(&pwdata->m);
			else
				FinishAttach(&pwdata->m);
			free(pwdata);
			return;
		}
		if (c == Ctrl('c')) {
			memset(pwdata->buf, 0, sizeof(pwdata->buf));
			AddStr("\r\n");
			FreeDisplay();
			Kill(pid, SIG_BYE);
			return;
		}
		if (c == '\b' || c == 0177) {
			if (len > 0) {
				pwdata->buf[len] = 0;
				len--;
			}
			continue;
		}
		if (c == Ctrl('u')) {
			memset(pwdata->buf, 0, sizeof(pwdata->buf));
			len = 0;
			continue;
		}
		if (len < sizeof(pwdata->buf) - 1)
			pwdata->buf[len++] = c;
	}
	pwdata->len = len;
}

/* 'end' is exclusive, i.e. you should *not* write in *end */
static char *strncpy_escape_quote(char *dst, const char *src, const char *end)
{
	while (*src && dst < end) {
		if (*src == '"') {
			if (dst + 2 < end)	/* \\ \" \0 */
				*dst++ = '\\';
			else
				return NULL;
		}
		*dst++ = *src++;
	}
	if (dst >= end)
		return NULL;

	*dst = '\0';
	return dst;
}

static void DoCommandMsg(Message *mp)
{
	char *args[MAXARGS];
	int argl[MAXARGS];
	char fullcmd[MAXSTR];
	char *fc;
	int n;
	char *p = mp->m.command.cmd;
	struct acluser *user;

	n = mp->m.command.nargs;
	if (n > MAXARGS - 1)
		n = MAXARGS - 1;
	for (fc = fullcmd; n > 0; n--) {
		size_t len = strlen(p);
		*fc++ = '"';
		if (!(fc = strncpy_escape_quote(fc, p, fullcmd + ARRAY_SIZE(fullcmd) - 2))) {	/* '"' ' ' */
			Msg(0, "Remote command too long.");
			queryflag = -1;
			return;
		}
		p += len + 1;
		*fc++ = '"';
		*fc++ = ' ';
	}
	if (fc != fullcmd)
		*--fc = 0;
	if (Parse(fullcmd, ARRAY_SIZE(fullcmd), args, argl) <= 0) {
		queryflag = -1;
		return;
	}
	user = *FindUserPtr(mp->m.attach.auser);
	if (user == NULL) {
		Msg(0, "Unknown user %s tried to send a command!", mp->m.attach.auser);
		queryflag = -1;
		return;
	}
	/*if (user->u_password && *user->u_password) {
		Msg(0, "User %s has a password, cannot use remote commands (using -Q or -X option).",
		    mp->m.attach.auser);
		queryflag = -1;
		return;
	}*/
	if (!display)
		for (display = displays; display; display = display->d_next)
			if (D_user == user)
				break;
	for (fore = mru_window; fore; fore = fore->w_prev_mru)
		if (!strcmp(mp->m_tty, fore->w_tty)) {
			if (!display)
				display = fore->w_layer.l_cvlist ? fore->w_layer.l_cvlist->c_display : NULL;

			/* If the window is not visibile in any display, then do not use the originating window as
			 * the foreground window for the command. This way, if there is an existing display, then
			 * the command will execute from the foreground window of that display. This is necessary so
			 * that commands that are relative to the window (e.g. 'next' etc.) do the right thing. */
			if (!fore->w_layer.l_cvlist || !fore->w_layer.l_cvlist->c_display)
				fore = NULL;
			break;
		}
	if (!display)
		display = displays;	/* sigh */
	if (*mp->m.command.preselect) {
		int i = -1;
		if (strcmp(mp->m.command.preselect, "-")) {
			i = WindowByNoN(mp->m.command.preselect);
			if (i < 0 || !GetWindowByNumber(i)) {
				Msg(0, "Could not find pre-select window.");
				queryflag = -1;
				return;
			}
		}
		fore = i >= 0 ? GetWindowByNumber(i) : NULL;
	} else if (!fore) {
		if (display && D_user == user)
			fore = Layer2Window(display->d_forecv->c_layer);
		if (!fore) {
			fore = user->u_detachwin >= 0 ? GetWindowByNumber(user->u_detachwin) : NULL;
			fore = FindNiceWindow(fore, NULL);
		}
	}
	if (!fore)
		fore = mru_window;	/* sigh */
	EffectiveAclUser = user;
	if (*args) {
		char *oldrcname = rc_name;
		rc_name = "-X";
		flayer = fore ? &fore->w_layer : NULL;
		if (fore && fore->w_savelayer && (fore->w_blocked || fore->w_savelayer->l_cvlist == NULL))
			flayer = fore->w_savelayer;
		DoCommand(args, argl);
		rc_name = oldrcname;
	}
	EffectiveAclUser = NULL;
}

int SendAttachMsg(int s, Message *m, int fd)
{
	struct msghdr msg;
	struct iovec iov;
	char buf[CMSG_SPACE(sizeof(int))];
	struct cmsghdr *cmsg;

	iov.iov_base = (char *)m;
	iov.iov_len = sizeof(Message);
	memset(&msg, 0, sizeof(struct msghdr));
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = buf;
	msg.msg_controllen = ARRAY_SIZE(buf);
	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));
	memmove(CMSG_DATA(cmsg), &fd, sizeof(int));
	msg.msg_controllen = cmsg->cmsg_len;
	while (1) {
		int ret = sendmsg(s, &msg, 0);
		if (ret == -1 && errno == EINTR)
			continue;
		if (ret == -1)
			return -1;
		return 0;
	}
}
