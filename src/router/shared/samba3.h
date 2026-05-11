/*
 * samba3.h
 *
 * Copyright (C) 2026 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#ifndef _samba3_h_
#define _samba3_h_
#include <jansson.h>

struct samba3_shareuser {
	char username[64];
	struct samba3_shareuser *next;
};

struct samba3_share {
	char mp[32];
	char sd[64];
	char label[64];
	char access_perms[4];
	int public;
	struct samba3_shareuser *users;
	struct samba3_share *next;
};

#define SHARETYPE_SAMBA 0x1
#define SHARETYPE_FTP 0x2
struct samba3_user {
	char username[64];
	char password[64];
	int sharetype;
	struct samba3_user *next;
};

struct samba3_user *getsamba3users(void);
struct samba3_share *getsamba3shares(void);
struct samba3_share *getsamba3share(char *mp, char *sd, char *label, char *access_perms, int public,
				    struct samba3_shareuser *users);
struct samba3_shareuser *getsamba3shareuser(const char *username);
struct samba3_shareuser *getsamba3shareusers(json_t *users);
struct samba3_user *getsamba3user(char *username, char *password, int type);

#endif
