/*
 * radiusdb.h
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

#ifndef _radiusdb_h_
#define _radiusdb_h_

struct radiususer {
	unsigned int fieldlen;
	unsigned int usersize;
	char *user;
	unsigned int passwordsize;
	char *passwd;
	unsigned int downstream;
	unsigned int upstream;
	unsigned int expiration;
	unsigned int enabled;
	//more fields can be added in future
} __attribute__((packed));

struct radiusdb {
	unsigned int usercount;
	struct radiususer *users;
} __attribute__((packed));

struct radiusclient {
	unsigned int fieldlen;
	unsigned int clientsize;
	char *client;
	unsigned int passwordsize;
	char *passwd;
	//more fields can be added in future
} __attribute__((packed));

struct radiusclientdb {
	unsigned int usercount;
	struct radiusclient *users;
} __attribute__((packed));

struct radiusdb *loadradiusdb(void);
void writeradiusdb(struct radiusdb *db);
void freeradiusdb(struct radiusdb *db);

struct radiusclientdb *loadradiusclientdb(void);
void writeradiusclientdb(struct radiusclientdb *db);
void freeradiusclientdb(struct radiusclientdb *db);

#define TYPE_SERVER 0x01
#define TYPE_CA 0x2
#define TYPE_CLIENT 0x3

void gen_cert(char *name, int type, char *common, char *pass);

#endif
