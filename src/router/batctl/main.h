/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Andreas Langer <an.langer@gmx.de>, Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#ifndef _BATCTL_MAIN_H
#define _BATCTL_MAIN_H

#include <stdint.h>

#include <net/if.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <netlink/netlink.h>

#ifndef SOURCE_VERSION
#define SOURCE_VERSION "2023.3"
#endif

#define EXIT_NOSUCCESS 2

#if BYTE_ORDER == BIG_ENDIAN
#define __BIG_ENDIAN_BITFIELD
#elif BYTE_ORDER == LITTLE_ENDIAN
#define __LITTLE_ENDIAN_BITFIELD
#else
#error "unknown endianess"
#endif

#define __maybe_unused __attribute__((unused))
#define BIT(nr)                 (1UL << (nr)) /* linux kernel compat */

extern char module_ver_path[];

#ifndef VLAN_VID_MASK
#define VLAN_VID_MASK   0xfff
#endif

#define BATADV_PRINT_VID(vid) (vid & BATADV_VLAN_HAS_TAG ? \
			       (int)(vid & VLAN_VID_MASK) : -1)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))

#ifndef container_of
#define container_of(ptr, type, member) __extension__ ({ \
	const __typeof__(((type *)0)->member) *__pmember = (ptr); \
	(type *)((char *)__pmember - offsetof(type, member)); })
#endif

enum command_flags {
	COMMAND_FLAG_MESH_IFACE = BIT(0),
	COMMAND_FLAG_NETLINK = BIT(1),
	COMMAND_FLAG_INVERSE = BIT(2),
};

enum selector_prefix {
	SP_NONE_OR_MESHIF,
	SP_MESHIF,
	SP_VLAN,
	SP_HARDIF,
};

enum command_type {
	SUBCOMMAND,
	SUBCOMMAND_MIF,
	SUBCOMMAND_VID,
	SUBCOMMAND_HIF,
	DEBUGTABLE,
	JSON_MIF,
	JSON_VID,
	JSON_HIF,
};

struct state {
	char *arg_iface;
	enum selector_prefix selector;
	char mesh_iface[IF_NAMESIZE];
	unsigned int mesh_ifindex;
	char hard_iface[IF_NAMESIZE];
	union {
		unsigned int hif;
		int vid;
	};
	const struct command *cmd;

	struct nl_sock *sock;
	struct nl_cb *cb;
	int batadv_family;
};

struct command {
	enum command_type type;
	const char *name;
	const char *abbr;
	int (*handler)(struct state *state, int argc, char **argv);
	uint32_t flags;
	void *arg;
	const char *usage;
};

#define COMMAND_NAMED(_type, _name, _abbr, _handler, _flags, _arg, _usage) \
	static const struct command command_ ## _name ## _ ## _type = { \
		.type = (_type), \
		.name = (#_name), \
		.abbr = _abbr, \
		.handler = (_handler), \
		.flags = (_flags), \
		.arg = (_arg), \
		.usage = (_usage), \
	}; \
	static const struct command *__command_ ## _name ## _ ## _type \
	__attribute__((__used__,__section__ ("__command"))) = &command_ ## _name ## _ ## _type

#define COMMAND(_type, _handler, _abbr, _flags, _arg, _usage) \
	COMMAND_NAMED(_type, _handler, _abbr, _handler, _flags, _arg, _usage)

#endif
