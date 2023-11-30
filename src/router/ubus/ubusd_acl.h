/*
 * Copyright (C) 2015 John Crispin <blogic@openwrt.org>
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

#ifndef __UBUSD_ACL_H
#define __UBUSD_ACL_H

enum ubusd_acl_type {
	UBUS_ACL_PUBLISH,
	UBUS_ACL_SUBSCRIBE,
	UBUS_ACL_ACCESS,
	UBUS_ACL_LISTEN,
	UBUS_ACL_SEND,
};

int ubusd_acl_check(struct ubus_client *cl, const char *obj, const char *method, enum ubusd_acl_type type);
int ubusd_acl_init_client(struct ubus_client *cl, int fd);
void ubusd_acl_free_client(struct ubus_client *cl);
void ubusd_acl_load(void);

#endif
