/*
 * blobmsg - library for generating/parsing structured blob messages
 *
 * Copyright (C) 2010 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __BLOBMSG_JSON_H
#define __BLOBMSG_JSON_H

#include <json/json.h>
#include <stdbool.h>
#include "blobmsg.h"

bool blobmsg_add_json_element(struct blob_buf *b, const char *name, json_object *obj);
bool blobmsg_add_json_from_string(struct blob_buf *b, const char *str);

typedef const char *(*blobmsg_json_format_t)(void *priv, struct blob_attr *attr);

char *blobmsg_format_json_with_cb(struct blob_attr *attr, bool list,
				  blobmsg_json_format_t cb, void *priv,
				  int indent);

static inline char *blobmsg_format_json(struct blob_attr *attr, bool list)
{
	return blobmsg_format_json_with_cb(attr, list, NULL, NULL, -1);
}

static inline char *blobmsg_format_json_indent(struct blob_attr *attr, bool list, int indent)
{
	return blobmsg_format_json_with_cb(attr, list, NULL, NULL, indent);
}

#endif
