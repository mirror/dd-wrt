// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Felix Fietkau <nbd@nbd.name>
 */

#include <stdarg.h>
#include "bridger.h"

static int debug_level = 0;

#ifdef UBUS_SUPPORT
static struct udebug ud;
static struct udebug_buf udb_log;
struct udebug_buf udb_nl;
static const struct udebug_buf_meta meta_log = {
	.name = "bridger_log",
	.format = UDEBUG_FORMAT_STRING,
};
static const struct udebug_buf_meta meta_nl = {
	.name = "bridger_nl",
	.format = UDEBUG_FORMAT_PACKET,
	.sub_format = UDEBUG_DLT_NETLINK,
};
static struct udebug_ubus_ring rings[] = {
	{
		.buf = &udb_log,
		.meta = &meta_log,
		.default_entries = 1024,
		.default_size = 64 * 1024,
	},
	{
		.buf = &udb_nl,
		.meta = &meta_nl,
		.default_entries = 1024,
		.default_size = 64 * 1024,
	},
};

void bridger_udebug_config(struct udebug_ubus *ctx, struct blob_attr *data,
			   bool enabled)
{
    udebug_ubus_apply_config(&ud, rings, ARRAY_SIZE(rings), data, enabled);
}
#endif


void bridger_dprintf(const char *format, ...)
{
	va_list ap;

#ifdef UBUS_SUPPORT
	if (udebug_buf_valid(&udb_log)) {
		va_start(ap, format);
		udebug_entry_init(&udb_log);
		udebug_entry_vprintf(&udb_log, format, ap);
		udebug_entry_add(&udb_log);
		va_end(ap);
	}
#endif

	if (!debug_level)
		return;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

const char *format_macaddr(const uint8_t *mac)
{
	static char str[sizeof("ff:ff:ff:ff:ff:ff ")];

	snprintf(str, sizeof(str), "%02x:%02x:%02x:%02x:%02x:%02x",
	mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	return str;
}

int main(int argc, char **argv)
{
	int ch;

	while ((ch = getopt(argc, argv, "d")) != -1) {
		switch (ch) {
		case 'd':
			debug_level++;
			break;
		default:
			break;
		}
	}

	uloop_init();

#ifdef UBUS_SUPPORT
	udebug_init(&ud);
	udebug_auto_connect(&ud, NULL);
	for (size_t i = 0; i < ARRAY_SIZE(rings); i++)
		udebug_ubus_ring_init(&ud, &rings[i]);
#endif

	if (bridger_bpf_init()) {
		perror("bridger_bpf_init");
		return 1;
	}

	if (bridger_nl_init()) {
		perror("bridger_nl_init");
		return 1;
	}

	bridger_device_init();
	bridger_flow_init();
	bridger_ubus_init();

	uloop_run();

	bridger_ubus_stop();
	bridger_device_stop();

	uloop_done();

	return 0;
}
