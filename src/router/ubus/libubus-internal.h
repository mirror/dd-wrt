/*
 * Copyright (C) 2011-2014 Felix Fietkau <nbd@openwrt.org>
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

#ifndef __LIBUBUS_IO_H
#define __LIBUBUS_IO_H

extern struct blob_buf b;
extern const struct ubus_method watch_method;

struct blob_attr **ubus_parse_msg(struct blob_attr *msg, size_t len);
bool ubus_validate_hdr(struct ubus_msghdr *hdr);
void ubus_handle_data(struct uloop_fd *u, unsigned int events);
int ubus_send_msg(struct ubus_context *ctx, uint32_t seq,
		  struct blob_attr *msg, int cmd, uint32_t peer, int fd);
void ubus_process_msg(struct ubus_context *ctx, struct ubus_msghdr_buf *buf, int fd);
int __hidden ubus_start_request(struct ubus_context *ctx, struct ubus_request *req,
				struct blob_attr *msg, int cmd, uint32_t peer);
int __hidden __ubus_start_request(struct ubus_context *ctx, struct ubus_request *req,
				struct blob_attr *msg, int cmd, uint32_t peer);
void ubus_process_obj_msg(struct ubus_context *ctx, struct ubus_msghdr_buf *buf, int fd);
void ubus_process_req_msg(struct ubus_context *ctx, struct ubus_msghdr_buf *buf, int fd);
void __hidden ubus_poll_data(struct ubus_context *ctx, int timeout);


#endif
