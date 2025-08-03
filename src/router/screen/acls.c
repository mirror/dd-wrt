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

#include "acls.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/* XXX: WHY IS THIS HERE?? :XXX */

#ifdef CHECKLOGIN
#include <pwd.h>
#endif				/* CHECKLOGIN */

#include <syslog.h>

#include "screen.h"		/* includes acls.h */

#include "mark.h"
#include "misc.h"
#include "process.h"

/************************************************************************
 * user managing code, this does not really belong into the acl stuff   *
 ************************************************************************/

struct acluser *users;

int maxusercount = 0;		/* used in process.c: RC_MONITOR, RC_SILENCE */

/* record given user ids here */
static AclBits userbits;

/*
 * rights a new unknown user will have on windows and cmds. 
 * These are changed by a "umask ?-..." command: 
 */
static char default_w_bit[ACL_BITS_PER_WIN] = {
	1,			/* EXEC */
	1,			/* WRITE */
	1			/* READ */
};

static char default_c_bit[ACL_BITS_PER_CMD] = {
	0			/* EXEC */
};

/* rights of all users per newly created window */
/*
 * are now stored per user (umask)
 * static AclBits default_w_userbits[ACL_BITS_PER_WIN];
 * static AclBits default_c_userbits[ACL_BITS_PER_CMD];
 */

static int GrowBitfield(AclBits *, int, int, int);
static struct aclusergroup **FindGroupPtr(struct aclusergroup **, struct acluser *, int);
static int AclSetPermCmd(struct acluser *, char *, struct comm *);
static int AclSetPermWin(struct acluser *, struct acluser *, char *, Window *);
static int UserAcl(struct acluser *, struct acluser **, int, char **);
static int UserAclCopy(struct acluser **, struct acluser **);

static int GrowBitfield(AclBits * bfp, int len, int delta, int defaultbit)
{
	AclBits n, o = *bfp;

	if (!(n = (AclBits) calloc(1, (unsigned long)(&ACLBYTE((char *)NULL, len + delta) + 1))))
		return -1;
	for (int i = 0; i < (len + delta); i++) {
		if (((i < len) && (ACLBIT(i) & ACLBYTE(o, i))) || ((i >= len) && (defaultbit)))
			ACLBYTE(n, i) |= ACLBIT(i);
	}
	if (len)
		free((char *)o);
	*bfp = n;
	return 0;
}

/*
 * =====================================================================
 * FindUserPtr-
 *       Searches for user
 *
 * Returns:
 *       an nonzero Address. Its contents is either a User-ptr,
 *       or NULL which may be replaced by a User-ptr to create the entry.
 * =====================================================================
 */
struct acluser **FindUserPtr(char *name)
{
	struct acluser **u;

	for (u = &users; *u; u = &(*u)->u_next)
		if (!strcmp((*u)->u_name, name))
			break;
	return u;
}

int DefaultEsc = -1;		/* initialised by screen.c:main() */
int DefaultMetaEsc = -1;

/*
 * =====================================================================
 * UserAdd-
 *       Adds a new user. His password may be NULL or "" if none. His name must not
 *       be "none", as this represents the NULL-pointer when dealing with groups.
 *       He has default rights, determined by umask.
 * Returns:
 *       0 - on success
 *       1 - he is already there
 *      -1 - he still does not exist (didn't get memory)
 * =====================================================================
 */
int UserAdd(char *name, struct acluser **up)
{
	if (!up)
		up = FindUserPtr(name);
	if (*up) {
		return 1;
	}
	if (strcmp("none", name))	/* "none" is a reserved word */
		*up = calloc(1, sizeof(struct acluser));
	if (!*up)
		return -1;
	(*up)->u_plop.buf = NULL;
	(*up)->u_plop.len = 0;
	(*up)->u_plop.enc = 0;
	(*up)->u_Esc = DefaultEsc;
	(*up)->u_MetaEsc = DefaultMetaEsc;
	strncpy((*up)->u_name, name, MAXLOGINLEN);
	(*up)->u_detachwin = -1;
	(*up)->u_detachotherwin = -1;

	(*up)->u_group = NULL;
	/* now find an unused index */
	for ((*up)->u_id = 0; (*up)->u_id < maxusercount; (*up)->u_id++)
		if (!(ACLBIT((*up)->u_id) & ACLBYTE(userbits, (*up)->u_id)))
			break;
	if ((*up)->u_id == maxusercount) {
		/* the bitfields are full, grow a chunk */
		/* first, the used_uid_indicator: */
		if (GrowBitfield(&userbits, maxusercount, USER_CHUNK, 0)) {
			free((char *)*up);
			*up = NULL;
			return -1;
		}
		/* second, default command bits  */
		/* (only if we generate commands dynamically) */
/*
      for (int j = 0; j < ACL_BITS_PER_CMD; j++)
	if (GrowBitfield(&default_c_userbits[j], maxusercount, USER_CHUNK, 
	    default_c_bit[j]))
	  {
	    free((char *)*up); *up = NULL; return -1;
	  }
*/
		/* third, the bits for each commands */
		for (int j = 0; j <= RC_LAST; j++) {
			for (int i = 0; i < ACL_BITS_PER_CMD; i++)
				if (GrowBitfield(&comms[j].userbits[i], maxusercount, USER_CHUNK, default_c_bit[i])) {
					free((char *)*up);
					*up = NULL;
					return -1;
				}
		}
		/* fourth, default window creation bits per user */
		for (struct acluser *u = users; u != *up; u = u->u_next) {
			for (int j = 0; j < ACL_BITS_PER_WIN; j++) {
				if (GrowBitfield(&u->u_umask_w_bits[j], maxusercount, USER_CHUNK, default_w_bit[j])) {
					free((char *)*up);
					*up = NULL;
					return -1;
				}
			}
		}

		/* fifth, the bits for each window */
		/* keep these in sync with NewWindowAcl() */
		for (Window *w = mru_window; w; w = w->w_prev_mru) {
			/* five a: the access control list */
			for (int j = 0; j < ACL_BITS_PER_WIN; j++)
				if (GrowBitfield(&w->w_userbits[j], maxusercount, USER_CHUNK, default_w_bit[j])) {
					free((char *)*up);
					*up = NULL;
					return -1;
				}
			/* five b: the activity notify list */
			/* five c: the silence notify list */
			if (GrowBitfield(&w->w_mon_notify, maxusercount, USER_CHUNK, 0) ||
			    GrowBitfield(&w->w_lio_notify, maxusercount, USER_CHUNK, 0)) {
				free((char *)*up);
				*up = NULL;
				return -1;
			}
		}
		maxusercount += USER_CHUNK;
	}

	/* mark the user-entry as "in-use" */
	ACLBYTE(userbits, (*up)->u_id) |= ACLBIT((*up)->u_id);

	/* user id 0 is the session creator, he has all rights */
	if ((*up)->u_id == 0)
		AclSetPerm(NULL, *up, "+a", "#?");

	/* user nobody has a fixed set of rights: */
	if (!strcmp((*up)->u_name, "nobody")) {
		AclSetPerm(NULL, *up, "-rwx", "#?");
		AclSetPerm(NULL, *up, "+x", "su");
		AclSetPerm(NULL, *up, "+x", "detach");
		AclSetPerm(NULL, *up, "+x", "displays");
		AclSetPerm(NULL, *up, "+x", "version");
	}

	/* 
	 * Create his umask:
	 * Give default_w_bit's for all users, 
	 * but allow himself everything on "his" windows.
	 */
	for (int j = 0; j < ACL_BITS_PER_WIN; j++) {
		if (GrowBitfield(&(*up)->u_umask_w_bits[j], 0, maxusercount, default_w_bit[j])) {
			free((char *)*up);
			*up = NULL;
			return -1;
		}
		ACLBYTE((*up)->u_umask_w_bits[j], (*up)->u_id) |= ACLBIT((*up)->u_id);
	}
	return 0;
}

/*
 * =====================================================================
 * UserDel-
 *       Remove a user from the list.
 *       Destroy all his permissions and completely detach him from the session.
 * Returns
 *       0 - success
 *      -1 - he who does not exist cannot be removed
 * =====================================================================
 */
int UserDel(char *name, struct acluser **up)
{
	struct acluser *u;
	Display *old, *next;

	if (!up)
		up = FindUserPtr(name);
	if (!(u = *up))
		return -1;
	old = display;
	for (display = displays; display; display = next) {
		next = display->d_next;	/* read the next ptr now, Detach may zap it. */
		if (D_user != u)
			continue;
		if (display == old)
			old = NULL;
		Detach(D_REMOTE);
	}
	display = old;
	*up = u->u_next;

	for (up = &users; *up; up = &(*up)->u_next) {
		/* unlink all group references to this user */
		struct aclusergroup **g = &(*up)->u_group;

		while (*g) {
			if ((*g)->u == u) {
				struct aclusergroup *next = (*g)->next;

				free((char *)(*g));
				*g = next;
			} else
				g = &(*g)->next;
		}
	}
	ACLBYTE(userbits, u->u_id) &= ~ACLBIT(u->u_id);
	/* restore the bits in his slot to default: */
	AclSetPerm(NULL, u, default_w_bit[ACL_READ] ? "+r" : "-r", "#");
	AclSetPerm(NULL, u, default_w_bit[ACL_WRITE] ? "+w" : "-w", "#");
	AclSetPerm(NULL, u, default_w_bit[ACL_EXEC] ? "+x" : "-x", "#");
	AclSetPerm(NULL, u, default_c_bit[ACL_EXEC] ? "+x" : "-x", "?");
	for (int i = 0; i < ACL_BITS_PER_WIN; i++)
		free((char *)u->u_umask_w_bits[i]);
	UserFreeCopyBuffer(u);
	free((char *)u);
	if (!users) {
		Finit(0);	/* Destroying whole session. No one could ever attach again. */
	}
	return 0;
}

/*
 * =====================================================================
 * UserFreeCopyBuffer-
 *       frees user buffer
 *       Also removes any references into the user's copybuffer
 * Returns:
 *       0 - if the copy buffer was really deleted.
 *      -1 - cannot remove something that does not exist
 * =====================================================================
 */
int UserFreeCopyBuffer(struct acluser *u)
{
	if (!u->u_plop.buf)
		return -1;
	for (Window *w = mru_window; w; w = w->w_prev_mru) {
		struct paster *pa = &w->w_paster;
		if (pa->pa_pasteptr >= u->u_plop.buf && pa->pa_pasteptr - u->u_plop.buf < (ptrdiff_t)u->u_plop.len)
			FreePaster(pa);
	}
	free((char *)u->u_plop.buf);
	u->u_plop.len = 0;
	u->u_plop.buf = NULL;
	return 0;
}

/*
 * Traverses group nodes. It searches for a node that references user u. 
 * If recursive is true, nodes found in the users are also searched using 
 * depth first method.  If none of the nodes references u, the address of 
 * the last next pointer is returned. This address will contain NULL.
 */
static struct aclusergroup **FindGroupPtr(struct aclusergroup **gp, struct acluser *u, int recursive)
{
	struct aclusergroup **g;

	while (*gp) {
		if ((*gp)->u == u)
			return gp;	/* found him here. */
		if (recursive && *(g = FindGroupPtr(&(*gp)->u->u_group, u, recursive + 1)))
			return g;	/* found him there. */
		gp = &(*gp)->next;
	}
	return gp;		/* *gp is NULL */
}

/* 
 * Returns nonzero if failed or already linked.
 * Both users are created on demand. 
 * Cyclic links are prevented.
 */
int AclLinkUser(char *from, char *to)
{
	struct acluser **u1, **u2;
	struct aclusergroup **g;

	if (!*(u1 = FindUserPtr(from)) && UserAdd(from, u1))
		return -1;
	if (!*(u2 = FindUserPtr(to)) && UserAdd(to, u2))
		return -1;	/* hmm, could not find both users. */

	if (*FindGroupPtr(&(*u2)->u_group, *u1, 1))
		return 1;	/* cyclic link detected! */
	if (*(g = FindGroupPtr(&(*u1)->u_group, *u2, 0)))
		return 2;	/* aha, we are already linked! */

	if (!(*g = (struct aclusergroup *)malloc(sizeof(struct aclusergroup))))
		return -1;	/* Could not alloc link. Poor screen */
	(*g)->u = (*u2);
	(*g)->next = NULL;
	return 0;
}

/*
 * The user pointer stored at *up will be substituted by a pointer
 * to the named user's structure, if passwords match.
 * returns NULL if successful, an static error string otherwise
 */
char *DoSu(struct acluser **up, char *name, char *pw1, char *pw2)
{
	(void) up;
	(void) name;
	(void) pw1;
	(void) pw2;

#if 0
	struct acluser *u;
	int sorry = 0;
	if (!(u = *FindUserPtr(name)))
		sorry++;
	else {
#ifdef CHECKLOGIN
		struct passwd *pp;
#ifdef SHADOWPW
		struct spwd *ss;
		int t, c;
#endif
		char *pass = "";

		if (!(pp = getpwnam(name))) {
			if (!(pw1 && *pw1 && *pw1 != '\377')) {
				sorry++;
			}
		} else
			pass = pp->pw_passwd;
#ifdef SHADOWPW
		for (t = 0; t < 13; t++) {
			c = pass[t];
			if (!(c == '.' || c == '/' ||
			      (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
				break;
		}
		if (t < 13) {
			if (!(ss = getspnam(name))) {
				sorry++;
			} else
				pass = ss->sp_pwdp;
		}
#endif				/* SHADOWPW */

		if (pw2 && *pw2 && *pw2 != '\377') {	/* provided a system password */
			if (!PasswordMatches(pw2, pass)) {
				sorry++;
			}
		} else /* no password provided */ if (*pass)	/* but need one */
			sorry++;
#endif				/* CHECKLOGIN */
		if (pw1 && *pw1 && *pw1 != '\377') {	/* provided a screen password */
			if (!PasswordMatches(pw1, u->u_password)) {
				sorry++;
			}
		} else /* no password provided */ if (*u->u_password)	/* but need one */
			sorry++;
	}

	openlog("screen", LOG_PID, LOG_AUTHPRIV);
	syslog(LOG_NOTICE, "%s: \"su %s\" %s for \"%s\"", SocketPath, name, sorry ? "failed" : "succeded", (*up)->u_name);
	closelog();

	if (sorry)
		return "Sorry.";
	else
		*up = u;	/* substitute user now */
#endif
	return NULL;
}

/************************************************************************
 *                     end of user managing code                        *
 ************************************************************************/

/* This gives the users default rights to the new window w created by u */
int NewWindowAcl(Window *w, struct acluser *u)
{
	/* keep these in sync with UserAdd part five. */
	if (GrowBitfield(&w->w_mon_notify, 0, maxusercount, 0) || GrowBitfield(&w->w_lio_notify, 0, maxusercount, 0))
		return -1;
	for (int j = 0; j < ACL_BITS_PER_WIN; j++) {
		/* we start with len 0 for the new bitfield size and add maxusercount */
		if (GrowBitfield(&w->w_userbits[j], 0, maxusercount, 0)) {
			while (--j >= 0)
				free((char *)w->w_userbits[j]);
			free((char *)w->w_mon_notify);
			free((char *)w->w_lio_notify);
			return -1;
		}
		for (int i = 0; i < maxusercount; i++)
			if (u ? (ACLBIT(i) & ACLBYTE(u->u_umask_w_bits[j], i)) : default_w_bit[j])
				ACLBYTE(w->w_userbits[j], i) |= ACLBIT(i);
	}
	return 0;
}

void FreeWindowAcl(Window *w)
{
	for (int i = 0; i < ACL_BITS_PER_WIN; i++)
		free((char *)w->w_userbits[i]);
	free((char *)w->w_mon_notify);
	free((char *)w->w_lio_notify);
}

/* if mode starts with '-' we remove the user's exec bit for cmd */
/*
 * NOTE: before you make this function look the same as 
 * AclSetPermWin, try to merge both functions. 
 */
static int AclSetPermCmd(struct acluser *u, char *mode, struct comm *cmd)
{
	int neg = 0;
	char *m = mode;

	while (*m) {
		switch (*m++) {
		case '-':
			neg = 1;
			continue;
		case '+':
			neg = 0;
			continue;
		case 'a':
		case 'e':
		case 'x':
			if (neg)
				ACLBYTE(cmd->userbits[ACL_EXEC], u->u_id) &= ~ACLBIT(u->u_id);
			else
				ACLBYTE(cmd->userbits[ACL_EXEC], u->u_id) |= ACLBIT(u->u_id);
			break;
		case 'r':
		case 'w':
			break;
		default:
			return -1;
		}
	}
	return 0;
}

/* mode strings of the form +rwx -w+rx r -wx are parsed and evaluated */
/*
 * aclchg nerd -w+w 2
 * releases a writelock on window 2 held by user nerd.
 * Letter n allows network access on a window.
 * uu should be NULL, except if you want to change his umask.
 */
static int AclSetPermWin(struct acluser *uu, struct acluser *u, char *mode, Window *win)
{
	int neg = 0;
	int bit, bits;
	AclBits *bitarray;
	char *m = mode;

	if (uu) {
		bitarray = uu->u_umask_w_bits;
	} else {
		bitarray = win->w_userbits;
	}

	while (*m) {
		switch (*m++) {
		case '-':
			neg = 1;
			continue;
		case '+':
			neg = 0;
			continue;
		case 'r':
			bits = (1 << ACL_READ);
			break;
		case 'w':
			bits = (1 << ACL_WRITE);
			break;
		case 'x':
			bits = (1 << ACL_EXEC);
			break;
		case 'a':
			bits = (1 << ACL_BITS_PER_WIN) - 1;
			break;
		default:
			return -1;
		}
		for (bit = 0; bit < ACL_BITS_PER_WIN; bit++) {
			if (!(bits & (1 << bit)))
				continue;
			if (neg)
				ACLBYTE(bitarray[bit], u->u_id) &= ~ACLBIT(u->u_id);
			else
				ACLBYTE(bitarray[bit], u->u_id) |= ACLBIT(u->u_id);
			if (!uu && (win->w_wlockuser == u) && neg && (bit == ACL_WRITE)) {
				win->w_wlockuser = NULL;
				if (win->w_wlock == WLOCK_ON)
					win->w_wlock = WLOCK_AUTO;
			}
		}
	}
	if (uu && u->u_name[0] == '?' && u->u_name[1] == '\0') {
		/* 
		 * It is Mr. '?', the unknown user. He deserves special treatment as
		 * he defines the defaults. Sorry, this is global, not per user.
		 */
		if (win) {
			for (bit = 0; bit < ACL_BITS_PER_WIN; bit++)
				default_w_bit[bit] = (ACLBYTE(bitarray[bit], u->u_id) & ACLBIT(u->u_id)) ? 1 : 0;
		} else {
			/*
			 * Hack. I do not want to duplicate all the above code for
			 * AclSetPermCmd. This assumes that there are not more bits
			 * per cmd than per win.
			 */
			for (bit = 0; bit < ACL_BITS_PER_CMD; bit++)
				default_c_bit[bit] = (ACLBYTE(bitarray[bit], u->u_id) & ACLBIT(u->u_id)) ? 1 : 0;
		}
		UserDel(u->u_name, NULL);
	}
	return 0;
}

/* 
 * String is broken down into command and window names, mode applies
 * A command name matches first, so do not use these as window names.
 * uu should be NULL, except if you want to change his umask.
 */
int AclSetPerm(struct acluser *uu, struct acluser *u, char *mode, char *s)
{
	int i;
	char *p, ch;

	while (*s) {
		switch (*s) {
		case '*':	/* all windows and all commands */
			return AclSetPerm(uu, u, mode, "#?");
		case '#':
			if (uu)	/* window umask or .. */
				AclSetPermWin(uu, u, mode, (Window *)1);
			else	/* .. or all windows */
				for (Window *w = mru_window; w; w = w->w_prev_mru)
					AclSetPermWin(NULL, u, mode, w);
			s++;
			break;
		case '?':
			if (uu)	/* command umask or .. */
				AclSetPermWin(uu, u, mode, NULL);
			else	/* .. or all commands */
				for (i = 0; i <= RC_LAST; i++)
					AclSetPermCmd(u, mode, &comms[i]);
			s++;
			break;
		default:
			for (p = s; *p && *p != ' ' && *p != '\t' && *p != ','; p++) ;
			if ((ch = *p))
				*p++ = '\0';
			if ((i = FindCommnr(s)) != RC_ILLEGAL)
				AclSetPermCmd(u, mode, &comms[i]);
			else if (((i = WindowByNoN(s)) >= 0) && GetWindowByNumber(i))
				AclSetPermWin(NULL, u, mode, GetWindowByNumber(i));
			else
				/* checking group name */
				return -1;
			if (ch)
				p[-1] = ch;
			s = p;
		}
	}
	return 0;
}

/* 
 * Generic ACL Manager:
 *
 * This handles acladd and aclchg identical.
 * With 3 parameters the last two parameters specify the permissions
 *   else user is added with full permissions.
 * With 1 parameter the users permissions are copied from user *argv.
 *   Unlike the other cases, u->u_name should not match *argv here.
 * uu should be NULL, except if you want to change his umask.
 */
static int UserAcl(struct acluser *uu, struct acluser **u, int argc, char **argv)
{
	if ((*u && !strcmp((*u)->u_name, "nobody")) || (argc > 1 && !strcmp(argv[0], "nobody")))
		return -1;	/* do not change nobody! */

	switch (argc) {
	case 1 + 2:
		return (UserAdd(argv[0], u) < 0) || AclSetPerm(uu, *u, argv[1], argv[2]);
	case 1:
		return (UserAdd(argv[0], u) < 0) || AclSetPerm(uu, *u, "+a", "#?");
	default:
		return -1;
	}
}

static int UserAclCopy(struct acluser **to_up, struct acluser **from_up)
{
	int to_id, from_id;

	if (!*to_up || !*from_up)
		return -1;
	if ((to_id = (*to_up)->u_id) == (from_id = (*from_up)->u_id))
		return -1;
	for (Window *w = mru_window; w; w = w->w_prev_mru) {
		for (int i = 0; i < ACL_BITS_PER_WIN; i++) {
			if (ACLBYTE(w->w_userbits[i], from_id) & ACLBIT(from_id))
				ACLBYTE(w->w_userbits[i], to_id) |= ACLBIT(to_id);
			else {
				ACLBYTE(w->w_userbits[i], to_id) &= ~ACLBIT(to_id);
				if ((w->w_wlockuser == *to_up) && (i == ACL_WRITE)) {
					w->w_wlockuser = NULL;
					if (w->w_wlock == WLOCK_ON)
						w->w_wlock = WLOCK_AUTO;
				}
			}
		}
	}
	for (int j = 0; j <= RC_LAST; j++) {
		for (int i = 0; i < ACL_BITS_PER_CMD; i++) {
			if (ACLBYTE(comms[j].userbits[i], from_id) & ACLBIT(from_id))
				ACLBYTE(comms[j].userbits[i], to_id) |= ACLBIT(to_id);
			else
				ACLBYTE(comms[j].userbits[i], to_id) &= ~ACLBIT(to_id);
		}
	}

	return 0;
}

/*
 * Syntax:
 * 	user [password] [+rwx #?]
 * 	* [password] [+rwx #?]
 *      user1,user2,user3 [password] [+rwx #?]
 *	user1,user2,user3=user
 * uu should be NULL, except if you want to change his umask.
 */
int UsersAcl(struct acluser *uu, int argc, char **argv)
{
	char *s;
	struct acluser **cf_u = NULL;

	if (argc == 1) {
		char *p = NULL;

		s = argv[0];
		while (*s)
			if (*s++ == '=')
				p = s;
		if (p) {
			p[-1] = '\0';
			cf_u = FindUserPtr(p);
		}
	}

	if (argv[0][0] == '*' && argv[0][1] == '\0') {
		for (struct acluser **u = &users; *u; u = &(*u)->u_next)
			if (strcmp("nobody", (*u)->u_name) &&
			    ((cf_u) ? ((UserAclCopy(u, cf_u)) < 0) : ((UserAcl(uu, u, argc, argv)) < 0)))
				return -1;
		return 0;
	}

	do {
		for (s = argv[0]; *s && *s != ' ' && *s != '\t' && *s != ',' && *s != '='; s++) ;
		*s ? (*s++ = '\0') : (*s = '\0');
		if ((cf_u) ?
		    ((UserAclCopy(FindUserPtr(argv[0]), cf_u)) < 0) :
		    ((UserAcl(uu, FindUserPtr(argv[0]), argc, argv)) < 0))
			return -1;
	} while (*(argv[0] = s));
	return 0;
}

/*
 * Preprocess arguments, so that umask can be set with UsersAcl
 * 
 * all current users		umask ±rwxn
 * one specific user		umask user1±rwxn
 * several users		umask user1,user2,...±rwxn
 * default_w_bits		umask ?±rwxn
 * default_c_bits		umask ??±rwxn
 */
int AclUmask(struct acluser *u, char *str, char **errp)
{
	char mode[16];
	char *av[3];
	char *p, c = '\0';

	/* split str into user and bits section. */
	for (p = str; *p; p++)
		if ((c = *p) == '+' || c == '-')
			break;
	if (!*p) {
		*errp = "Bad argument. Should be ``[user[,user...]{+|-}rwxn''.";
		return -1;
	}
	strncpy(mode, p, 15);
	mode[15] = '\0';
	*p = '\0';

	/* construct argument vector */
	if (!strcmp("??", str)) {
		str++;
		av[2] = "?";
	} else
		av[2] = "#";
	av[1] = mode;
	av[0] = *str ? str : "*";
	/* call UsersAcl */
	if (UsersAcl(u, 3, av)) {
		*errp = "UsersAcl failed. Hmmm.";
		*p = c;
		return -1;
	}
	*p = c;
	return 0;
}

struct acluser *EffectiveAclUser = NULL;	/* hook for AT command permission */

int AclCheckPermWin(struct acluser *u, int mode, Window *w)
{
	int ok;

	if (mode < 0 || mode >= ACL_BITS_PER_WIN)
		return -1;
	if (EffectiveAclUser) {
		u = EffectiveAclUser;
	}
	ok = ACLBYTE(w->w_userbits[mode], u->u_id) & ACLBIT(u->u_id);

	if (!ok) {
		struct aclusergroup **g = &u->u_group;
		struct acluser *saved_eff = EffectiveAclUser;

		EffectiveAclUser = NULL;
		while (*g) {
			if (!AclCheckPermWin((*g)->u, mode, w))
				break;
			g = &(*g)->next;
		}
		EffectiveAclUser = saved_eff;
		if (*g)
			ok = 1;
	}
	return !ok;
}

int AclCheckPermCmd(struct acluser *u, int mode, struct comm *c)
{
	int ok;

	if (mode < 0 || mode >= ACL_BITS_PER_CMD)
		return -1;
	if (EffectiveAclUser) {
		u = EffectiveAclUser;
	}
	ok = ACLBYTE(c->userbits[mode], u->u_id) & ACLBIT(u->u_id);
	if (!ok) {
		struct aclusergroup **g = &u->u_group;
		struct acluser *saved_eff = EffectiveAclUser;

		EffectiveAclUser = NULL;
		while (*g) {
			if (!AclCheckPermCmd((*g)->u, mode, c))
				break;
			g = &(*g)->next;
		}
		EffectiveAclUser = saved_eff;
		if (*g)
			ok = 1;
	}
	return !ok;
}
