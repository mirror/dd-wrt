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


struct mt_credentials mt_users[MT_CRED_MAXNUM];

void read_userfile() {
	FILE *file = fopen(USERSFILE, "r");
	char line [BUFSIZ];
	int i = 0;

	if (file == NULL) {
		perror(USERSFILE);
		exit(1);
	}

	while ( fgets(line, sizeof line, file) ) {
		char *user;
		char *password;

		user = strtok(line, ":");
		password = strtok(NULL, "\n");

		if (user == NULL || password == NULL) {
			continue;
		}

		if (user[0] == '#')
			continue;

		memcpy(mt_users[i].username, user, strlen(user) < MT_CRED_LEN - 1? strlen(user) : MT_CRED_LEN);
		memcpy(mt_users[i++].password, password, strlen(password)  < MT_CRED_LEN - 1? strlen(password)  : MT_CRED_LEN);

		if (i == MT_CRED_MAXNUM)
			break;

		mt_users[i].username[0] = '\0';
	}
	fclose(file);
}

struct mt_credentials* find_user(char *username) {
	int i = 0;

	while (i < MT_CRED_MAXNUM && mt_users[i].username[0] != 0) {
		if (strcmp(username, mt_users[i].username) == 0) {
			return &(mt_users[i]);
		}
		i++;
	}
	return NULL;
}
