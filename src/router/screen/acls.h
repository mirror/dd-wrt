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

#ifndef SCREEN_ACLS_H
#define SCREEN_ACLS_H

#include "os.h"

/* three known bits: */
#define ACL_EXEC 0		
#define ACL_WRITE 1
#define ACL_READ 2

#define ACL_BITS_PER_CMD 1	/* for comm.h */
#define ACL_BITS_PER_WIN 3	/* for window.h */

#define USER_CHUNK 1

#define ACLBYTE(data, w)   ((data)[(w) >> 3])
#define ACLBIT(w)   (0x80 >> ((w) & 7))

typedef struct Window Window;

typedef unsigned char * AclBits;

/*
 * How a user joins a group.
 * Here is the node to construct one list per user.
 */
struct aclusergroup
{
  struct acluser *u;	/* the user who borrows us his rights */
  struct aclusergroup *next;
};

/***************
 *  ==> user.h
 */

struct plop {
	char *buf;
	size_t len;
	int enc;
};

/*
 * A User has a list of groups, and points to other users.
 * users is the User entry of the session owner (creator)
 * and anchors all other users. Add/Delete users there.
 */
typedef struct acluser
{
  struct acluser *u_next;		/* continue the main user list */
  char u_name[MAXLOGINLEN + 1];		/* login name how he showed up */
  int  u_detachwin;		/* the window where he last detached */
  int  u_detachotherwin;	/* window that was "other" when he detached */
  int  u_Esc, u_MetaEsc;	/* the users screen escape character */
  struct plop u_plop;
  int u_id;			/* a uniq index in the bitfields. */
  AclBits u_umask_w_bits[ACL_BITS_PER_WIN];	/* his window create umask */
  struct aclusergroup *u_group;	/* linked list of pointers to other users */
} User;


/* forward declaration */
struct comm;

int AclSetPerm (struct acluser *, struct acluser *, char *, char *);
int AclUmask (struct acluser *, char *, char **);
int UsersAcl (struct acluser *, int, char **);
void AclWinSwap (int, int);
char *DoSu (struct acluser **, char *, char *, char *);
int AclLinkUser (char *, char *);
int UserFreeCopyBuffer (struct acluser *);
struct acluser **FindUserPtr (char *);
int UserAdd (char *, struct acluser **);
int UserDel (char *, struct acluser **);
int AclCheckPermWin(struct acluser *, int, Window *);
int NewWindowAcl(Window *, struct acluser *);
void FreeWindowAcl(Window *);
int AclCheckPermCmd(struct acluser *, int, struct comm *);

/* global variables */

extern int DefaultEsc, DefaultMetaEsc;
extern int maxusercount;

extern struct acluser *users, *EffectiveAclUser;

#endif /* SCREEN_ACLS_H */
