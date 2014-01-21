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
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "autologin.h"
#include "config.h"

#define _(String) String

struct autologin_profile login_profiles[AUTOLOGIN_MAXPROFILES];

struct autologin_profile *autologin_find_profile(char *identifier) {
	int i;
	struct autologin_profile *default_profile = NULL;

	if (strlen(identifier) == 0) return NULL;

	for (i = 0; i < AUTOLOGIN_MAXPROFILES; ++i) {
		if (login_profiles[i].inuse && strcasecmp(identifier, login_profiles[i].identifier) == 0) {
			return &login_profiles[i];
		}
		if (login_profiles[i].inuse && strcasecmp("default", login_profiles[i].identifier) == 0) {
			default_profile = &login_profiles[i];
		}
	}
	return default_profile;
}

static char *tilde_to_path(char *path) {
	char *homepath;
	if (*path == '~' && (homepath = getenv("HOME"))) {
		static char newpath[256];
		memset(newpath, 0, sizeof(newpath));
		strncpy(newpath, homepath, 255);
		strncat(newpath, path+1, 255);
		return newpath;
	}
	return path;
}

int autologin_readfile(char *configfile) {
	FILE *fp;
	char c;
	int i = -1;
	char *p = NULL;
	char *file_to_read;
	char key[AUTOLOGIN_MAXSTR];
	char value[AUTOLOGIN_MAXSTR];
	int line_counter=1;
	enum autologin_state state = ALS_NONE;

	memset(login_profiles, 0, sizeof(login_profiles));

	/* Convert ~/path to /home/username/path */
	file_to_read = tilde_to_path(configfile);

	fp = fopen(file_to_read, "r");
	if (fp <= 0) {
		if (strcmp(configfile, AUTOLOGIN_PATH) == 0) {
			/* Silent ignore? */
		} else {
			fprintf(stderr, _("Error opening autologin file %s: %s\n"), file_to_read, strerror(errno));
		}
		return 0;
	}
	while ((c = fgetc(fp)) && !feof(fp)) {
		if (c == '#') {
			while ((c = fgetc(fp)) != '\n' && !feof(fp));
		}

		switch (state) {
			case ALS_PREIDENTIFIER:
				i++;
				if (i == AUTOLOGIN_MAXPROFILES) {
					goto done;
				}
				p = login_profiles[i].identifier;
				state++;
				break;

			case ALS_PREKEY:
				memset(key, 0, AUTOLOGIN_MAXSTR);
				memset(value, 0, AUTOLOGIN_MAXSTR);
				p = key;
				login_profiles[i].inuse = 1;
				state++;
				break;

			case ALS_PREVALUE:
				memset(value, 0, AUTOLOGIN_MAXSTR);
				p = value;
				state++;
				break;
			default:
				break;
		}

		switch (state) {
			case ALS_NONE:
				if (c == '[') {
					state = ALS_PREIDENTIFIER;
				}
				break;

			case ALS_IDENTIFIER:
				if (c == ']') {
					state = ALS_PREKEY;
					break;
				}
				if (c == '\n') {
					fprintf(stderr, _("Error on line %d in %s: New line in middle of identifier\n"), line_counter, configfile);
					state = ALS_NONE;
					break;
				}
				*p++ = c;
				if (p - login_profiles[i].identifier == AUTOLOGIN_MAXSTR-1) {
					*p = 0;
					fprintf(stderr, _("Error on line %d in %s: Identifier string too long.\n"), line_counter, configfile);
					while ((c = fgetc(fp)) != '\n' && c != ']' && !feof(fp));
					state = ALS_PREKEY;
					break;
				}
				break;

			case ALS_KEY:
				if (p == key && c == '\n') break;
				if (c == '=') {
					state = ALS_PREVALUE;
					break;
				}
				if (c == '[') {
					state = ALS_PREIDENTIFIER;
					break;
				}
				if (c == ' ') { /* ignore whitespace */
					break;
				}
				if (c == '\n') {
					fprintf(stderr, _("Error on line %d in %s: Newline before '=' character\n"), line_counter, configfile);
					state = ALS_PREKEY;
					break;
				}
				*p++ = c;
				if (p - key == AUTOLOGIN_MAXSTR-1) {
					*p = 0;
					fprintf(stderr, _("Error on line %d in %s: Key string too long.\n"), line_counter, configfile);
					while ((c = fgetc(fp)) != '\n' && c != '=' && !feof(fp));
					if (c == '\n') {
						state = ALS_PREKEY;
					} else {
						state = ALS_PREVALUE;
					}
				}
				break;

			case ALS_VALUE:
				if (p == value && c == '\n') break;
				if (c == '\n') {
					if (strncasecmp(key, "user", AUTOLOGIN_MAXSTR) == 0) {
						strncpy(login_profiles[i].username, value, AUTOLOGIN_MAXSTR);
						login_profiles[i].hasUsername = 1;
					} else if (strncasecmp(key, "password", AUTOLOGIN_MAXSTR) == 0) {
						strncpy(login_profiles[i].password, value, AUTOLOGIN_MAXSTR);
						login_profiles[i].hasPassword = 1;
					} else {
						fprintf(stderr, _("Warning on line %d of %s: Unknown parameter %s, ignoring.\n"), line_counter, configfile, key);
					}
					state = ALS_PREKEY;
					break;
				}
				if (c == ' ') { /* ignore whitespace */
					break;
				}
				*p++ = c;
				if (p - value == AUTOLOGIN_MAXSTR-1) {
					*p = 0;
					fprintf(stderr, _("Error on line %d in %s: Value string too long.\n"), line_counter, configfile);
					while ((c = fgetc(fp)) != '\n' && !feof(fp));
					if (c == '\n') {
						state = ALS_PREKEY;
					}
				}
				break;

			default:
				break;
		}
		if (c == '\n') {
			line_counter++;
		}
		if (feof(fp)) {
			break;
		}
	}

	done:
	fclose(fp);

	return 1;
}
