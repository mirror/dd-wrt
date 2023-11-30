/*
 * Copyright (C) 2011 Felix Fietkau <nbd@openwrt.org>
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

#ifndef __UBUSMSG_H
#define __UBUSMSG_H

#include <stdint.h>
#include <libubox/blob.h>

#define __packetdata __attribute__((packed)) __attribute__((__aligned__(4)))

#define UBUS_MSG_CHUNK_SIZE	65536

#define UBUS_SYSTEM_OBJECT_EVENT	1
#define UBUS_SYSTEM_OBJECT_ACL		2
#define UBUS_SYSTEM_OBJECT_MONITOR	3
#define UBUS_SYSTEM_OBJECT_MAX		1024

struct ubus_msghdr {
	uint8_t version;
	uint8_t type;
	uint16_t seq;
	uint32_t peer;
} __packetdata;

enum ubus_msg_type {
	/* initial server message */
	UBUS_MSG_HELLO,

	/* generic command response */
	UBUS_MSG_STATUS,

	/* data message response */
	UBUS_MSG_DATA,

	/* ping request */
	UBUS_MSG_PING,

	/* look up one or more objects */
	UBUS_MSG_LOOKUP,

	/* invoke a method on a single object */
	UBUS_MSG_INVOKE,

	UBUS_MSG_ADD_OBJECT,
	UBUS_MSG_REMOVE_OBJECT,

	/*
	 * subscribe/unsubscribe to object notifications
	 * The unsubscribe message is sent from ubusd when
	 * the object disappears
	 */
	UBUS_MSG_SUBSCRIBE,
	UBUS_MSG_UNSUBSCRIBE,

	/*
	 * send a notification to all subscribers of an object.
	 * when sent from the server, it indicates a subscription
	 * status change
	 */
	UBUS_MSG_NOTIFY,

	UBUS_MSG_MONITOR,

	/* must be last */
	__UBUS_MSG_LAST,
};

enum ubus_msg_attr {
	UBUS_ATTR_UNSPEC,

	UBUS_ATTR_STATUS,

	UBUS_ATTR_OBJPATH,
	UBUS_ATTR_OBJID,
	UBUS_ATTR_METHOD,

	UBUS_ATTR_OBJTYPE,
	UBUS_ATTR_SIGNATURE,

	UBUS_ATTR_DATA,
	UBUS_ATTR_TARGET,

	UBUS_ATTR_ACTIVE,
	UBUS_ATTR_NO_REPLY,

	UBUS_ATTR_SUBSCRIBERS,

	UBUS_ATTR_USER,
	UBUS_ATTR_GROUP,

	/* must be last */
	UBUS_ATTR_MAX,
};

enum ubus_monitor_attr {
	UBUS_MONITOR_CLIENT,
	UBUS_MONITOR_PEER,
	UBUS_MONITOR_SEND,
	UBUS_MONITOR_SEQ,
	UBUS_MONITOR_TYPE,
	UBUS_MONITOR_DATA,

	/* must be last */
	UBUS_MONITOR_MAX,
};

enum ubus_msg_status {
	UBUS_STATUS_OK,
	UBUS_STATUS_INVALID_COMMAND,
	UBUS_STATUS_INVALID_ARGUMENT,
	UBUS_STATUS_METHOD_NOT_FOUND,
	UBUS_STATUS_NOT_FOUND,
	UBUS_STATUS_NO_DATA,
	UBUS_STATUS_PERMISSION_DENIED,
	UBUS_STATUS_TIMEOUT,
	UBUS_STATUS_NOT_SUPPORTED,
	UBUS_STATUS_UNKNOWN_ERROR,
	UBUS_STATUS_CONNECTION_FAILED,
	UBUS_STATUS_NO_MEMORY,
	UBUS_STATUS_PARSE_ERROR,
	UBUS_STATUS_SYSTEM_ERROR,
	__UBUS_STATUS_LAST
};

#endif
