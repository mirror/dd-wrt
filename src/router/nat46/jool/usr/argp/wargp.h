#ifndef SRC_USR_ARGP_WARGP_H_
#define SRC_USR_ARGP_WARGP_H_

#include <argp.h>
#include <stdbool.h>
#include <stddef.h>

#include "common/types.h"

/*
 * I guess that argp is more versatile than I need it for. I use it for very
 * simple option parsing, which leads to lots of clumsy, redundant code.
 *
 * So I made this module. "wargp" stands for "wrapped argp". It's simply a mask
 * over argp that removes a lot of annoying unneeded stuff.
 */

typedef int (*wargp_parse_type)(void *input, int key, char *str);

struct wargp_type {
	/**
	 * This is the same as argp_option.arg.
	 * It's the name of the "argument" of the option.
	 * For example, if you have a `--foo` flag, and you declare its
	 * `argument` to be "<bar>", then argp will print this on `--help`:
	 *     `--foo=<bar>`
	 */
	char *argument;
	wargp_parse_type parse;
	/*
	 * A (single) string listing the autocompletion candidates for the
	 * values of this option, separated by spaces.
	 */
	char *candidates;
};

extern struct wargp_type wt_bool;
extern struct wargp_type wt_u32;
extern struct wargp_type wt_l4proto;
extern struct wargp_type wt_string;
extern struct wargp_type wt_addr;
extern struct wargp_type wt_prefix6;
extern struct wargp_type wt_prefix4;

struct wargp_option {
	const char *name;
	int key;
	const char *doc;
	size_t offset;
	struct wargp_type *type;
};

struct wargp_bool {
	bool value;
};

struct wargp_l4proto {
	bool set;
	l4_protocol proto;
};

struct wargp_string {
	char *value;
};

struct wargp_addr {
	__u8 proto;
	union {
		struct in6_addr v6;
		struct in_addr v4;
	} addr;
};

struct wargp_prefix6 {
	bool set;
	struct ipv6_prefix prefix;
};

struct wargp_prefix4 {
	bool set;
	struct ipv4_prefix prefix;
};

#define ARGP_TCP 't'
#define ARGP_UDP 'u'
#define ARGP_ICMP 'i'
#define ARGP_CSV 2000
#define ARGP_NO_HEADERS 2001
#define ARGP_NUMERIC 2002
#define ARGP_FORCE 'f'

#define WARGP_TCP(container, field, description) \
	{ \
		.name = "tcp", \
		.key = ARGP_TCP, \
		.doc = description, \
		.offset = offsetof(container, field), \
		.type = &wt_l4proto, \
	}
#define WARGP_UDP(container, field, description) \
	{ \
		.name = "udp", \
		.key = ARGP_UDP, \
		.doc = description, \
		.offset = offsetof(container, field), \
		.type = &wt_l4proto, \
	}
#define WARGP_ICMP(container, field, description) \
	{ \
		.name = "icmp", \
		.key = ARGP_ICMP, \
		.doc = description, \
		.offset = offsetof(container, field), \
		.type = &wt_l4proto, \
	}
#define WARGP_NO_HEADERS(container, field) \
	{ \
		.name = "no-headers", \
		.key = ARGP_NO_HEADERS, \
		.doc = "Do not print table headers", \
		.offset = offsetof(container, field), \
		.type = &wt_bool, \
	}
#define WARGP_CSV(container, field) { \
		.name = "csv", \
		.key = ARGP_CSV, \
		.doc = "Print in CSV format", \
		.offset = offsetof(container, field), \
		.type = &wt_bool, \
	}
#define WARGP_NUMERIC(container, field) { \
		.name = "numeric", \
		.key = ARGP_NUMERIC, \
		.doc = "Do not resolve names", \
		.offset = offsetof(container, field), \
		.type = &wt_bool, \
	}
#define WARGP_FORCE(container, field) { \
		.name = "force", \
		.key = ARGP_FORCE, \
		.doc = "Ignore warnings", \
		.offset = offsetof(container, field), \
		.type = &wt_bool, \
	}

int wargp_parse(struct wargp_option *wopts, int argc, char **argv, void *input);
void print_wargp_opts(struct wargp_option *opts);

#endif /* SRC_USR_ARGP_WARGP_H_ */
