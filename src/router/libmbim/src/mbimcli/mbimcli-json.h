#ifndef MBIMCLI_JSON_H
#define MBIMCLI_JSON_H

#include <json-c/json.h>
#include <stdio.h>

extern json_object *j_root;

static inline void
j_add_str(json_object *j_obj, const char *key, const char *val)
{
	json_object_object_add(j_obj, key, json_object_new_string(val));
}

static inline void
j_add_int(json_object *j_obj, const char *key, const int val)
{
	json_object_object_add(j_obj, key, json_object_new_int(val));
}

static inline void
j_add_i64(json_object *j_obj, const char *key, const int64_t val)
{
	json_object_object_add(j_obj, key, json_object_new_int64(val));
}

static inline void j_finish(const char *name, json_object *j_obj)
{
	if (!j_root)
		j_root = json_object_new_object();

	json_object_object_add(j_root, name, j_obj);
}

static inline void j_dump(void)
{
	const char *json_str;

	if (!j_root)
		return;

	json_str = json_object_to_json_string_ext(j_root,
		JSON_C_TO_STRING_PRETTY);
	puts(json_str);
	json_object_put(j_root);
	j_root = NULL;
}

#endif
