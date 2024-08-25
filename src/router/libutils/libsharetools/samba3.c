/*
 * samba.c
 *
 * Copyright (C) 2017 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
#include <samba3.h>
#include <bcmnvram.h>
#include <string.h>

struct samba3_shareuser *getsamba3shareuser(const char *username)
{
	struct samba3_shareuser *user = calloc(1, sizeof(struct samba3_shareuser));

	strncpy(user->username, username, sizeof(user->username) - 1);

	return user;
};

struct samba3_shareuser *getsamba3shareusers(json_t *users)
{
	int count, entry_count;
	const char *value;
	struct samba3_shareuser *shareusers = NULL, *current;

	entry_count = json_array_size(users);
	if (entry_count == 0) {
		return NULL;
	}
	for (count = 0; count < entry_count; count++) {
		value = json_string_value(json_array_get(users, count));
		if (count == 0) {
			current = getsamba3shareuser(value);
			shareusers = current;
		} else {
			current->next = getsamba3shareuser(value);
			current = current->next;
		}
	}

	return shareusers;
}

struct samba3_user *getsamba3user(char *username, char *password, int type)
{
	struct samba3_user *user = calloc(1, sizeof(struct samba3_user));
	if (user) {
		strncpy(user->username, username, sizeof(user->username) - 1);
		strncpy(user->password, password, sizeof(user->password) - 1);
		user->sharetype = type;
	}
	return user;
}

struct samba3_user *getsamba3users(void)
{
	struct samba3_user *list, *current;
	int count, entry_count;
	json_t *json;
	json_error_t error;
	const char *key;
	json_t *iterator, *entry, *value;
	char username[64], password[64];
	int type;

	// first create dummy entry
	list = getsamba3user("", "", 0);
	current = list;

	//json = json_loads( "[{\"user\":\"peter\",\"pass\":\"test\"},{\"user\":\"chris\",\"pass\":\"test\"}]", &error );
	json = json_loads(nvram_default_get("samba3_users", "[]"), 0, &error);
	if (!json) {
		fprintf(stderr, "[JASON] ERROR\n");
	} else {
		entry_count = json_array_size(json);
		for (count = 0; count < entry_count; count++) {
			entry = json_array_get(json, count);
			iterator = json_object_iter(entry);

			// reset
			username[0] = 0;
			password[0] = 0;
			type = -1;
			while (iterator) {
				key = json_object_iter_key(iterator);
				value = json_object_iter_value(iterator);
				/* use key and value ... */
				if (!strcmp(key, "user")) {
					strncpy(username, json_string_value(value), sizeof(username) - 1);
				} else if (!strcmp(key, "pass")) {
					strncpy(password, json_string_value(value), sizeof(password) - 1);
				} else if (!strcmp(key, "type")) {
					type = json_integer_value(value);
				}
				iterator = json_object_iter_next(entry, iterator);
			}
			if (username[0] != 0 && type != -1) {
				current->next = getsamba3user(username, password, type);
				current = current->next;
			}
		}
		json_array_clear(json);
		free(json);
	}

	return list;
}

struct samba3_share *getsamba3share(char *mp, char *sd, char *label, char *access_perms, int public, struct samba3_shareuser *users)
{
	struct samba3_share *share = calloc(1, sizeof(struct samba3_share));
	if (share) {
		strncpy(share->mp, mp, sizeof(share->mp) - 1);
		strncpy(share->sd, sd, sizeof(share->sd) - 1);
		strncpy(share->label, label, sizeof(share->label) - 1);
		strncpy(share->access_perms, access_perms, sizeof(share->access_perms) - 1);
		share->public = public;
		if (users != NULL) {
			//fprintf(stderr, "[SHAREUSERS] add\n");
			share->users = users;
		}
	}
	return share;
}

void refjson(void)
{
	json_dumps(NULL, JSON_COMPACT);
}

struct samba3_share *getsamba3shares(void)
{
	struct samba3_share *list, *current;
	int public, count, entry_count;
	struct samba3_shareuser *shareusers;
	json_t *json;
	json_error_t error;
	const char *key;
	json_t *iterator, *entry, *value;
	char mp[32], sd[64], label[64], access_perms[4];

	// first create dummy entry
	list = getsamba3share("", "", "", "", 0, NULL);
	current = list;

	//      json = json_loads( "[{\"mp\":\"/jffs\",\"sd\":\"subdirectory\",\"label\":\"testshare\",\"perms\":\"rw\",\"public\":0},{\"mp\":\"/mnt\",\"label\":\"othertest\",\"perms\":\"ro\",\"public\":1},{\"label\":\"blah\"}]", &error );
	json = json_loads(nvram_default_get("samba3_shares", "[]"), 0, &error);
	if (!json) {
		fprintf(stderr, "[JASON] ERROR\n");
	} else {
		entry_count = json_array_size(json);
		for (count = 0; count < entry_count; count++) {
			entry = json_array_get(json, count);
			iterator = json_object_iter(entry);

			// reset
			mp[0] = 0;
			sd[0] = 0;
			label[0] = 0;
			access_perms[0] = 0;
			public = 0;
			shareusers = NULL;

			while (iterator) {
				key = json_object_iter_key(iterator);
				value = json_object_iter_value(iterator);
				/* use key and value ... */
				if (!strcmp(key, "mp")) {
					strncpy(mp, json_string_value(value), sizeof(mp) - 1);
				} else if (!strcmp(key, "sd")) {
					strncpy(sd, json_string_value(value), sizeof(sd) - 1);
				} else if (!strcmp(key, "label")) {
					strncpy(label, json_string_value(value), sizeof(label) - 1);
				} else if (!strcmp(key, "perms")) {
					strncpy(access_perms, json_string_value(value), sizeof(access_perms) - 1);
				} else if (!strcmp(key, "public")) {
					public = json_integer_value(value);
				} else if (!strcmp(key, "users")) {
					//fprintf( stderr, "[SAMABA SHARES] users\n" );
					shareusers = getsamba3shareusers(value);
				}
				iterator = json_object_iter_next(entry, iterator);
			}
			if (*mp && *label && *access_perms) {
				current->next = getsamba3share(mp, sd, label, access_perms, public, shareusers);
				current = current->next;
			}
		}
		json_array_clear(json);
		free(json);
	}
	return list;
}
