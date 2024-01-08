/*
 * rsync.c
 *
 * Copyright (C) 2017 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <rsync.h>
#include <bcmnvram.h>
#include <string.h>

#ifndef HAVE_SAMBA3
void refjson(void)
{
	json_dumps(NULL, JSON_COMPACT);
}
#endif

struct rsync_share *getrsyncshare(char *mp, char *sd, char *label)
{
	struct rsync_share *share = calloc(1, sizeof(struct rsync_share));

	strncpy(share->mp, mp, sizeof(share->mp) - 1);
	strncpy(share->sd, sd, sizeof(share->sd) - 1);
	strncpy(share->label, label, sizeof(share->label) - 1);
	return share;
}

struct rsync_share *getrsyncshares(void)
{
	struct rsync_share *list, *current;
	int count, entry_count;
	json_t *json;
	json_error_t error;
	const char *key;
	json_t *iterator, *entry, *value;
	char mp[64], sd[64], label[64];

	// first create dummy entry
	list = getrsyncshare("", "", "");
	current = list;

	//      json = json_loads( "[{\"mp\":\"/jffs\",\"label\":\"testshare\",\"perms\":\"rw\",\"public\":0},{\"mp\":\"/mnt\",\"label\":\"othertest\",\"perms\":\"ro\",\"public\":1},{\"label\":\"blah\"}]", &error );
	json = json_loads(nvram_default_get("rsync_shares", "[]"), 0, &error);
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

			while (iterator) {
				key = json_object_iter_key(iterator);
				value = json_object_iter_value(iterator);
				/* use key and value ... */
				if (!strcmp(key, "mp")) {
					strncpy(mp, json_string_value(value),
						sizeof(mp) - 1);
				} else if (!strcmp(key, "sd")) {
					strncpy(sd, json_string_value(value),
						sizeof(sd) - 1);
				} else if (!strcmp(key, "label")) {
					strncpy(label, json_string_value(value),
						sizeof(label) - 1);
				}
				iterator =
					json_object_iter_next(entry, iterator);
			}
			if (*mp && *label) {
				current->next = getrsyncshare(mp, sd, label);
				current = current->next;
			}
		}
		json_array_clear(json);
	}
	return list;
}
