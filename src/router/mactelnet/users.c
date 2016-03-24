/*
    Mac-Telnet - Connect to RouterOS or mactelnetd devices via MAC address
    Copyright (C) 2010, Håkon Nessjøen <haakon.nessjoen@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "users.h"
#include "config.h"
#include "utlist.h"

#define _(String) (String)

struct mt_credentials *mt_users = NULL;

void read_userfile() {
	struct mt_credentials *cred, *tmp;
	FILE *file = fopen(USERSFILE, "r");
	char line [BUFSIZ];

	if (file == NULL) {
		perror(USERSFILE);
		exit(1);
	}

	DL_FOREACH_SAFE(mt_users, cred, tmp) {
		DL_DELETE(mt_users, cred);
		free(cred);
	}

	while ( fgets(line, sizeof line, file) ) {
		char *user;
		char *password;

		user = strtok(line, ":");
		password = strtok(NULL, "\n");

		if (user == NULL || password == NULL || user[0] == '#') {
			continue;
		}

		cred = (struct mt_credentials *)calloc(1, sizeof(struct mt_credentials));
		if (cred == NULL) {
			fprintf(stderr, _("Error allocating memory for user information\n"));
			exit(1);
		}

		memcpy(cred->username, user, strlen(user) < MT_CRED_LEN - 1? strlen(user) : MT_CRED_LEN);
		memcpy(cred->password, password, strlen(password)  < MT_CRED_LEN - 1? strlen(password)  : MT_CRED_LEN);
		DL_APPEND(mt_users, cred);
	}
	fclose(file);
}

struct mt_credentials* find_user(char *username) {
	struct mt_credentials *cred;

	DL_FOREACH(mt_users, cred) {
		if (strcmp(username, cred->username) == 0) {
			return cred;
		}
	}
	return NULL;
}
