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

#include "utmp.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>

#include "screen.h"

#ifdef HAVE_UTEMPTER
#include <utempter.h>
#endif

#include "misc.h"
#include "tty.h"
#include "winmsg.h"

/*
 *  we have a suid-root helper app that changes the utmp for us
 *  (won't work for login-slots)
 */
#if defined(HAVE_UTEMPTER)
#define UTMP_HELPER
#endif

#ifdef ENABLE_UTMP

static slot_t TtyNameSlot(char *);
static void makeuser(struct utmpx *, char *, char *, pid_t);
static void makedead(struct utmpx *);
static int pututslot(slot_t, struct utmpx *, char *, Window *);
static struct utmpx *getutslot(slot_t);

static int utmpok;
static char UtmpName[] = UTMPXFILE;
#ifndef UTMP_HELPER
static int utmpfd = -1;
#endif

#undef  D_loginhost
#define D_loginhost D_utmp_logintty.ut_host
#if !defined(HAVE_UT_HOST)
#undef  D_loginhost
#define D_loginhost (NULL)
#endif

#endif				/* ENABLE_UTMP */

/*
 * SlotToggle - modify the utmp slot of the fore window.
 *
 * how == true  try to set a utmp slot.
 * how == false try to withdraw a utmp slot.
 *
 * w_slot = -1  window not logged in.
 * w_slot = 0   window not logged in, but should be logged in.
 *              (unable to write utmp, or detached).
 */

#ifndef ENABLE_UTMP
void SlotToggle(bool how)
{
	(void)how; /* unused */
#ifdef UTMPXFILE
	Msg(0, "Unable to modify %s.\n", UTMPXFILE);
#else
	Msg(0, "Unable to modify utmp-database.\n");
#endif
}
#endif

#ifdef ENABLE_UTMP

void SlotToggle(bool how)
{
	if (fore->w_type != W_TYPE_PTY) {
		Msg(0, "Can only work with normal windows.\n");
		return;
	}
	if (how) {
		if ((fore->w_slot == (slot_t) - 1) || (fore->w_slot == (slot_t) 0)) {
			if (SetUtmp(fore) == 0)
				Msg(0, "This window is now logged in.");
			else
				Msg(0, "This window should now be logged in.");
			WindowChanged(fore, WINESC_WFLAGS);
		} else
			Msg(0, "This window is already logged in.");
	} else {
		if (fore->w_slot == (slot_t) - 1)
			Msg(0, "This window is already logged out\n");
		else if (fore->w_slot == (slot_t) 0) {
			Msg(0, "This window is not logged in.");
			fore->w_slot = (slot_t) - 1;
		} else {
			RemoveUtmp(fore);
			if (fore->w_slot != (slot_t) - 1)
				Msg(0, "What? Cannot remove Utmp slot?");
			else
				Msg(0, "This window is no longer logged in.");
#ifdef CAREFULUTMP
			CarefulUtmp();
#endif
			WindowChanged(fore, WINESC_WFLAGS);
		}
	}
}

#ifdef CAREFULUTMP

/* CAREFULUTMP: goodie for paranoid sysadmins: always leave one
 * window logged in
 */
void CarefulUtmp()
{
	Window *p;

	if (!mru_window)		/* hopeless */
		return;
	for (p = mru_window; p; p = p->w_prev_mru)
		if (p->w_ptyfd >= 0 && p->w_slot != (slot_t)-1)
			return;	/* found one, nothing to do */

	for (p = mru_window; p; p = p->w_prev_mru)
		if (p->w_ptyfd >= 0)	/* no zombies please */
			break;
	if (!p)
		return;		/* really hopeless */
	SetUtmp(p);
	Msg(0, "Window %d is now logged in.\n", p->w_number);
}
#endif				/* CAREFULUTMP */

void InitUtmp(void)
{
#ifndef UTMP_HELPER
	if ((utmpfd = open(UtmpName, O_RDWR)) == -1) {
		if (errno != EACCES)
			Msg(errno, "%s", UtmpName);
		utmpok = 0;
		return;
	}
	close(utmpfd);		/* it was just a test */
	utmpfd = -1;
#endif				/* UTMP_HELPER */
	utmpok = 1;
}

/*
 * the utmp entry for tty is located and removed.
 * it is stored in D_utmp_logintty.
 */
void RemoveLoginSlot(void)
{
	struct utmpx u, *uu;

	D_loginslot = TtyNameSlot(D_usertty);
	if (D_loginslot == (slot_t) 0 || D_loginslot == (slot_t) - 1)
		return;
#ifdef UTMP_HELPER
	if (eff_uid)		/* helpers can't do login slots. sigh. */
#else
	if (!utmpok)
#endif
	{
		D_loginslot = NULL;
	} else {
		if ((uu = getutslot(D_loginslot)) == NULL) {
			D_loginslot = NULL;
		} else {
			D_utmp_logintty = *uu;
			u = *uu;
			makedead(&u);
			if (pututslot(D_loginslot, &u, NULL, NULL) == 0)
				D_loginslot = NULL;
		}
	}
	if (D_loginslot == (slot_t) 0) {
		/* couldn't remove slot, do a 'mesg n' at least. */
		struct stat stb;
		char *tty;
		D_loginttymode = 0;
		if ((tty = GetPtsPathOrSymlink(D_userfd)) && stat(tty, &stb) == 0 && stb.st_uid == real_uid && !CheckTtyname(tty)
		    && ((int)stb.st_mode & 0777) != 0666) {
			D_loginttymode = (int)stb.st_mode & 0777;
			chmod(D_usertty, stb.st_mode & 0600);
		}
	}
}

/*
 * D_utmp_logintty is reinserted into utmp
 */
void RestoreLoginSlot(void)
{
	char *tty;

	if (utmpok && D_loginslot != (slot_t) 0 && D_loginslot != (slot_t) - 1) {
		if (pututslot(D_loginslot, &D_utmp_logintty, D_loginhost, NULL) == 0)
			Msg(errno, "Could not write %s", UtmpName);
	}
	D_loginslot = (slot_t) 0;
	if (D_loginttymode && (tty = GetPtsPathOrSymlink(D_userfd)) && !CheckTtyname(tty))
		fchmod(D_userfd, D_loginttymode);
}

/*
 * Construct a utmp entry for window wi.
 * the hostname field reflects what we know about the user (display)
 * location. If d_loginhost is not set, then he is local and we write
 * down the name of his terminal line; else he is remote and we keep
 * the hostname here. The letter S and the window id will be appended.
 * A saved utmp entry in wi->w_savut serves as a template, usually.
 */

int SetUtmp(Window *win)
{
	slot_t slot;
	struct utmpx u = { 0 };
	int saved_ut;
#if defined(HAVE_UT_HOST)
	char host[ARRAY_SIZE(u.ut_host)];
#else
	char *host = NULL;
#endif

	win->w_slot = (slot_t) 0;
	if (!utmpok || win->w_type != W_TYPE_PTY)
		return -1;
	if ((slot = TtyNameSlot(win->w_tty)) == (slot_t) 0) {
		return -1;
	}

	if ((saved_ut = memcmp((char *)&win->w_savut, (char *)&u, sizeof(struct utmpx))))
		/* restore original, of which we will adopt all fields but ut_host */
		memmove((char *)&u, (char *)&win->w_savut, sizeof(struct utmpx));

	if (!saved_ut)
		makeuser(&u, stripdev(win->w_tty), LoginName, win->w_pid);

#if defined(HAVE_UT_HOST)
	if (display) {
		snprintf(host, ARRAY_SIZE(host), "%s", D_loginhost);
		if (D_loginslot == (slot_t)0 || D_loginslot == (slot_t)-1 || host[0] == '\0')
			snprintf(host, ARRAY_SIZE(host), ":%s", stripdev(D_usertty));
	} else
		snprintf(host, ARRAY_SIZE(host), "local");

	snprintf(host + strlen(host), ARRAY_SIZE(host) - strlen(host), ":S.%d", win->w_number);

	memcpy(u.ut_host, host, ARRAY_SIZE(u.ut_host));
#endif

	if (pututslot(slot, &u, host, win) == 0) {
		Msg(errno, "Could not write %s", UtmpName);
		return -1;
	}
	win->w_slot = slot;
	memmove((char *)&win->w_savut, (char *)&u, sizeof(struct utmpx));
	return 0;
}

/*
 * if slot could be removed or was 0,  win->w_slot = -1;
 * else not changed.
 */

int RemoveUtmp(Window *win)
{
	struct utmpx u = { 0 };
	struct utmpx *uu;
	slot_t slot;

	slot = win->w_slot;
	if (!utmpok)
		return -1;
	if (slot == (slot_t)0 || slot == (slot_t)-1) {
		win->w_slot = (slot_t)-1;
		return 0;
	}
	if ((uu = getutslot(slot)) == NULL) {
		Msg(0, "Utmp slot not found -> not removed");
		return -1;
	}
	memmove((char *)&win->w_savut, (char *)uu, sizeof(struct utmpx));
	u = *uu;
	makedead(&u);
	if (pututslot(slot, &u, NULL, win) == 0) {
		Msg(errno, "Could not write %s", UtmpName);
		return -1;
	}
	win->w_slot = (slot_t)-1;
	return 0;
}

/*********************************************************************
 *
 *  routines using the getut* api
 */

#define SLOT_USED(u) (u->ut_type == USER_PROCESS)

static struct utmpx *getutslot(slot_t slot)
{
	struct utmpx u = { 0 };
	memcpy(u.ut_line, (char *)slot, ARRAY_SIZE(u.ut_line));
	setutxent();
	return getutxline(&u);
}

static int pututslot(slot_t slot, struct utmpx *u, char *host, Window *win)
{
	(void)slot; /* unused */
#ifdef HAVE_UTEMPTER
	if (eff_uid && win && win->w_ptyfd != -1) {
		/* sigh, linux hackers made the helper functions void */
		if (SLOT_USED(u))
			utempter_add_record(win->w_ptyfd, host);
		else
			utempter_remove_record(win->w_ptyfd);
		/*
		 * As documented in libutempter: "During execution of
		 * the privileged process spawned by these functions,
		 * SIGCHLD signal handler will be temporarily set to
		 * the default action." Thus in case a SIGCHLD has
		 * been lost, we send a SIGCHLD to oneself in order to
		 * avoid zombies: https://savannah.gnu.org/bugs/?25089
		 */
		kill(getpid(), SIGCHLD);

		return 1;	/* pray for success */
	}
#else
	(void)host; /* unused */
	(void)win; /* unused */
#endif

	setutxent();
	return pututxline(u) != NULL;
}

static void makedead(struct utmpx *u)
{
	u->ut_type = DEAD_PROCESS;
#if defined(HAVE_UT_EXIT)
	u->ut_exit.e_termination = 0;
	u->ut_exit.e_exit = 0;
#endif
	u->ut_user[0] = 0;	/* for Digital UNIX, kilbi@rad.rwth-aachen.de */
}

static void makeuser(struct utmpx *u, char *line, char *user, pid_t pid)
{
	time_t now;
	u->ut_type = USER_PROCESS;
	memcpy(u->ut_user, user, ARRAY_SIZE(u->ut_user));
	/* Now the tricky part... guess ut_id */
	memcpy(u->ut_id, line + 3, ARRAY_SIZE(u->ut_id));
	memcpy(u->ut_line, line, ARRAY_SIZE(u->ut_line));
	u->ut_pid = pid;
	/* must use temp variable because of NetBSD/sparc64, where
	 * ut_xtime is long(64) but time_t is int(32) */
	(void)time(&now);
	u->ut_tv.tv_sec = now;
}

static slot_t TtyNameSlot(char *name)
{
	return stripdev(name);
}

#endif				/* ENABLE_UTMP */

