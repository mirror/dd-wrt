#include <dlna.h>
#include <bcmnvram.h>
#include <string.h>

#ifndef HAVE_SAMBA3
void refjson(void)
{
	json_dumps(NULL, JSON_COMPACT);
}
#endif

struct dlna_share *getdlnashare(char *mp, int types)
{

	struct dlna_share *share = calloc(1, sizeof(struct dlna_share));

	strncpy(share->mp, mp, sizeof(share->mp) - 1);
	share->types = types;
	return share;
}

struct dlna_share *getdlnashares(void)
{

	struct dlna_share *list, *current;
	int count, entry_count;
	json_t *json;
	json_error_t error;
	const char *key;
	json_t *iterator, *entry, *value;
	char mp[64], types;

	// first create dummy entry
	list = getdlnashare("", 0);
	current = list;

//      json = json_loads( "[{\"mp\":\"/jffs\",\"label\":\"testshare\",\"perms\":\"rw\",\"public\":0},{\"mp\":\"/mnt\",\"label\":\"othertest\",\"perms\":\"ro\",\"public\":1},{\"label\":\"blah\"}]", &error );
	json = json_loads(nvram_default_get("dlna_shares", "[]"), 0, &error);
	if (!json) {
		fprintf(stderr, "[JASON] ERROR\n");
	} else {
		entry_count = json_array_size(json);
		for (count = 0; count < entry_count; count++) {
			entry = json_array_get(json, count);
			iterator = json_object_iter(entry);

			// reset
			mp[0] = 0;
			types = 0;

			while (iterator) {
				key = json_object_iter_key(iterator);
				value = json_object_iter_value(iterator);
				/* use key and value ... */
				if (!strcmp(key, "mp")) {
					strncpy(mp, json_string_value(value), sizeof(mp) - 1);
				} else if (!strcmp(key, "types")) {
					types = json_integer_value(value);
				}
				iterator = json_object_iter_next(entry, iterator);
			}
			if (mp[0] != 0) {
				current->next = getdlnashare(mp, types);
				current = current->next;
			}
		}
		json_array_clear(json);
	}
	return list;
}
