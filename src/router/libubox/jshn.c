/*
 * Copyright (C) 2011-2013 Felix Fietkau <nbd@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifdef JSONC
        #include <json.h>
#else
        #include <json/json.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include "list.h"

#include "avl.h"
#include "blob.h"
#include "blobmsg_json.h"

#define MAX_VARLEN	256

static struct avl_tree env_vars;
static struct blob_buf b = { 0 };

static const char *var_prefix = "";
static int var_prefix_len = 0;

static int add_json_element(const char *key, json_object *obj);

struct env_var {
	struct avl_node avl;
	char *val;
};

static int add_json_object(json_object *obj)
{
	int ret = 0;

	json_object_object_foreach(obj, key, val) {
		ret = add_json_element(key, val);
		if (ret)
			break;
	}
	return ret;
}

static int add_json_array(struct array_list *a)
{
	char seq[12];
	int i, len;
	int ret;

	for (i = 0, len = array_list_length(a); i < len; i++) {
		snprintf(seq, sizeof(seq), "%d", i);
		ret = add_json_element(seq, array_list_get_idx(a, i));
		if (ret)
			return ret;
	}

	return 0;
}

static void add_json_string(const char *str)
{
	char *ptr = (char *) str;
	int len;
	char *c;

	while ((c = strchr(ptr, '\'')) != NULL) {
		len = c - ptr;
		if (len > 0)
			fwrite(ptr, len, 1, stdout);
		ptr = c + 1;
		c = "'\\''";
		fwrite(c, strlen(c), 1, stdout);
	}
	len = strlen(ptr);
	if (len > 0)
		fwrite(ptr, len, 1, stdout);
}

static void write_key_string(const char *key)
{
	while (*key) {
		putc(isalnum(*key) ? *key : '_', stdout);
		key++;
	}
}

static int add_json_element(const char *key, json_object *obj)
{
	char *type;

	switch (json_object_get_type(obj)) {
	case json_type_object:
		type = "object";
		break;
	case json_type_array:
		type = "array";
		break;
	case json_type_string:
		type = "string";
		break;
	case json_type_boolean:
		type = "boolean";
		break;
	case json_type_int:
		type = "int";
		break;
	case json_type_double:
		type = "double";
		break;
	case json_type_null:
		type = "null";
		break;
	default:
		return -1;
	}

	fprintf(stdout, "json_add_%s '", type);
	write_key_string(key);

	switch (json_object_get_type(obj)) {
	case json_type_object:
		fprintf(stdout, "';\n");
		add_json_object(obj);
		fprintf(stdout, "json_close_object;\n");
		break;
	case json_type_array:
		fprintf(stdout, "';\n");
		add_json_array(json_object_get_array(obj));
		fprintf(stdout, "json_close_array;\n");
		break;
	case json_type_string:
		fprintf(stdout, "' '");
		add_json_string(json_object_get_string(obj));
		fprintf(stdout, "';\n");
		break;
	case json_type_boolean:
		fprintf(stdout, "' %d;\n", json_object_get_boolean(obj));
		break;
	case json_type_int:
		fprintf(stdout, "' %"PRId64";\n", json_object_get_int64(obj));
		break;
	case json_type_double:
		fprintf(stdout, "' %lf;\n", json_object_get_double(obj));
		break;
	case json_type_null:
		fprintf(stdout, "';\n");
		break;
	default:
		return -1;
	}

	return 0;
}

static int jshn_parse(const char *str)
{
	json_object *obj;

	obj = json_tokener_parse(str);
	if (!obj || json_object_get_type(obj) != json_type_object) {
		if (obj)
			json_object_put(obj);
		fprintf(stderr, "Failed to parse message data\n");
		return 1;
	}
	fprintf(stdout, "json_init;\n");
	add_json_object(obj);
	fflush(stdout);
	json_object_put(obj);

	return 0;
}

static char *getenv_avl(const char *key)
{
	struct env_var *var = avl_find_element(&env_vars, key, var, avl);
	return var ? var->val : NULL;
}

static char *get_keys(const char *prefix)
{
	char *keys;
	size_t len = var_prefix_len + strlen(prefix) + sizeof("K_") + 1;

	keys = alloca(len);
	snprintf(keys, len, "%sK_%s", var_prefix, prefix);
	return getenv_avl(keys);
}

static void get_var(const char *prefix, const char **name, char **var, char **type)
{
	char *tmpname, *varname;
	size_t len = var_prefix_len + strlen(prefix) + 1 + strlen(*name) + 1 + sizeof("T_");

	tmpname = alloca(len);

	snprintf(tmpname, len, "%s%s_%s", var_prefix, prefix, *name);
	*var = getenv_avl(tmpname);

	snprintf(tmpname, len, "%sT_%s_%s", var_prefix, prefix, *name);
	*type = getenv_avl(tmpname);

	snprintf(tmpname, len, "%sN_%s_%s", var_prefix, prefix, *name);
	varname = getenv_avl(tmpname);
	if (varname)
		*name = varname;
}

static json_object *jshn_add_objects(json_object *obj, const char *prefix, bool array);

static void jshn_add_object_var(json_object *obj, bool array, const char *prefix, const char *name)
{
	json_object *new;
	char *var, *type;

	get_var(prefix, &name, &var, &type);
	if (!var || !type)
		return;

	if (!strcmp(type, "array")) {
		new = json_object_new_array();
		jshn_add_objects(new, var, true);
	} else if (!strcmp(type, "object")) {
		new = json_object_new_object();
		jshn_add_objects(new, var, false);
	} else if (!strcmp(type, "string")) {
		new = json_object_new_string(var);
	} else if (!strcmp(type, "int")) {
		new = json_object_new_int64(atoll(var));
	} else if (!strcmp(type, "double")) {
		new = json_object_new_double(strtod(var, NULL));
	} else if (!strcmp(type, "boolean")) {
		new = json_object_new_boolean(!!atoi(var));
	} else if (!strcmp(type, "null")) {
		new = NULL;
	} else {
		return;
	}

	if (array)
		json_object_array_add(obj, new);
	else
		json_object_object_add(obj, name, new);
}

static json_object *jshn_add_objects(json_object *obj, const char *prefix, bool array)
{
	char *keys, *key, *brk;

	keys = get_keys(prefix);
	if (!keys || !obj)
		goto out;

	for (key = strtok_r(keys, " ", &brk); key;
	     key = strtok_r(NULL, " ", &brk)) {
		jshn_add_object_var(obj, array, prefix, key);
	}

out:
	return obj;
}

static int jshn_format(bool no_newline, bool indent, FILE *stream)
{
	json_object *obj;
	const char *output;
	char *blobmsg_output = NULL;
	int ret = -1;

	if (!(obj = json_object_new_object()))
		return -1;

	jshn_add_objects(obj, "J_V", false);
	if (!(output = json_object_to_json_string(obj)))
		goto out;

	if (indent) {
		blob_buf_init(&b, 0);
		if (!blobmsg_add_json_from_string(&b, output))
			goto out;
		if (!(blobmsg_output = blobmsg_format_json_indent(b.head, 1, 0)))
			goto out;
		output = blobmsg_output;
	}
	fprintf(stream, "%s%s", output, no_newline ? "" : "\n");
	free(blobmsg_output);
	ret = 0;

out:
	json_object_put(obj);
	return ret;
}

static int usage(const char *progname)
{
	fprintf(stderr, "Usage: %s [-n] [-i] -r <message>|-R <file>|-o <file>|-p <prefix>|-w\n", progname);
	return 2;
}

static int avl_strcmp_var(const void *k1, const void *k2, void *ptr)
{
	const char *s1 = k1;
	const char *s2 = k2;
	char c1, c2;

	while (*s1 && *s1 == *s2) {
		s1++;
		s2++;
	}

	c1 = *s1;
	c2 = *s2;
	if (c1 == '=')
		c1 = 0;
	if (c2 == '=')
		c2 = 0;

	return c1 - c2;
}

static int jshn_parse_file(const char *path)
{
	struct stat sb;
	int ret = 0;
	char *fbuf;
	int fd;

	if ((fd = open(path, O_RDONLY)) == -1) {
		fprintf(stderr, "Error opening %s\n", path);
		return 3;
	}

	if (fstat(fd, &sb) == -1) {
		fprintf(stderr, "Error getting size of %s\n", path);
		close(fd);
		return 3;
	}

	if (!(fbuf = calloc(1, sb.st_size+1))) {
		fprintf(stderr, "Error allocating memory for %s\n", path);
		close(fd);
		return 3;
	}

	if (read(fd, fbuf, sb.st_size) != sb.st_size) {
		fprintf(stderr, "Error reading %s\n", path);
		free(fbuf);
		close(fd);
		return 3;
	}

	ret = jshn_parse(fbuf);
	free(fbuf);
	close(fd);

	return ret;
}

static int jshn_format_file(const char *path, bool no_newline, bool indent)
{
	FILE *fp = NULL;
	int ret = 0;

	fp = fopen(path, "w");
	if (!fp) {
		fprintf(stderr, "Error opening %s\n", path);
		return 3;
	}

	ret = jshn_format(no_newline, indent, fp);
	fclose(fp);

	return ret;
}

int main(int argc, char **argv)
{
	extern char **environ;
	bool no_newline = false;
	bool indent = false;
	struct env_var *vars;
	int i;
	int ret = 0;
	int ch;

	avl_init(&env_vars, avl_strcmp_var, false, NULL);
	for (i = 0; environ[i]; i++);

	vars = calloc(i, sizeof(*vars));
	if (!vars) {
		fprintf(stderr, "%m\n");
		return -1;
	}
	for (i = 0; environ[i]; i++) {
		char *c;

		vars[i].avl.key = environ[i];
		c = strchr(environ[i], '=');
		if (!c)
			continue;

		vars[i].val = c + 1;
		avl_insert(&env_vars, &vars[i].avl);
	}

	while ((ch = getopt(argc, argv, "p:nir:R:o:w")) != -1) {
		switch(ch) {
		case 'p':
			var_prefix = optarg;
			var_prefix_len = strlen(var_prefix);
			break;
		case 'r':
			ret = jshn_parse(optarg);
			goto exit;
		case 'R':
			ret = jshn_parse_file(optarg);
			goto exit;
		case 'w':
			ret = jshn_format(no_newline, indent, stdout);
			goto exit;
		case 'o':
			ret = jshn_format_file(optarg, no_newline, indent);
			goto exit;
		case 'n':
			no_newline = true;
			break;
		case 'i':
			indent = true;
			break;
		default:
			free(vars);
			return usage(argv[0]);
		}
	}

	free(vars);
	return usage(argv[0]);

exit:
	free(vars);
	return ret;
}
