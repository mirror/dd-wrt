/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2020 embedd.ch 
 *   Copyright (C) 2020 Felix Fietkau <nbd@nbd.name> 
 *   Copyright (C) 2020 John Crispin <john@phrozen.org> 
 */

#include <libubox/blobmsg.h>
#include <libubus.h>
#include <stdio.h>
#include <getopt.h>
#include "utils.h"
#include "timeout.h"

static struct blob_buf b;
static LIST_HEAD(stations);
static struct usteer_timeout_queue tq;
static FILE *r_fd;
static struct ubus_object bss_obj;
static struct ubus_context *ubus_ctx;
static int freq = 2412;
static int verbose;

struct var {
	int cur;
	int min;
	int max;
};

struct sta_data {
	struct list_head list;
	struct usteer_timeout probe_t;
	struct var probe;
	struct var signal;
	uint8_t addr[6];
};

static void gen_val(struct var *val)
{
	int delta = val->max - val->min;
	uint8_t v;

	val->cur = val->min;
	if (!delta)
		return;

	if (fread(&v, sizeof(v), 1, r_fd) != sizeof(v))
		fprintf(stderr, "short read\n");
	val->cur += (((unsigned int) v) * delta) / 0xff;
}

static void
blobmsg_add_macaddr(struct blob_buf *buf, const char *name, const uint8_t *addr)
{
	char *s = blobmsg_alloc_string_buffer(buf, name, 20);
	sprintf(s, MAC_ADDR_FMT, MAC_ADDR_DATA(addr));
	blobmsg_add_string_buffer(buf);
}

static void sta_send_probe(struct sta_data *sta)
{
	const char *type = "probe";
	int ret;
	int sig = -95 + sta->signal.cur;

	blob_buf_init(&b, 0);
	blobmsg_add_macaddr(&b, "address", sta->addr);
	blobmsg_add_u32(&b, "freq", freq);
	blobmsg_add_u32(&b, "signal", sig);
	ret = ubus_notify(ubus_ctx, &bss_obj, type, b.head, 100);
	if (verbose)
		fprintf(stderr, "STA "MAC_ADDR_FMT" probe: %d (%d ms, signal: %d)\n",
			MAC_ADDR_DATA(sta->addr), ret, sta->probe.cur, sig);
}

static void sta_schedule_probe(struct sta_data *sta)
{
	gen_val(&sta->probe);
	gen_val(&sta->signal);
	usteer_timeout_set(&tq, &sta->probe_t, sta->probe.cur);
}

static void sta_probe(struct usteer_timeout_queue *q, struct usteer_timeout *t)
{
	struct sta_data *sta = container_of(t, struct sta_data, probe_t);

	sta_send_probe(sta);
	sta_schedule_probe(sta);
}

static void init_station(struct sta_data *sta)
{
	list_add_tail(&sta->list, &stations);
	if (fread(&sta->addr, sizeof(sta->addr), 1, r_fd) != sizeof(sta->addr))
		fprintf(stderr, "short read\n");
	sta->addr[0] &= ~1;

	sta_schedule_probe(sta);
}

static void create_stations(struct sta_data *ref, int n)
{
	struct sta_data *sta;
	int i;

	tq.cb = sta_probe;
	sta = calloc(n, sizeof(*sta));
	for (i = 0; i < n; i++) {
		memcpy(sta, ref, sizeof(*sta));
		init_station(sta);
		sta++;
	}
}

static int usage(const char *prog)
{
	fprintf(stderr, "Usage: %s <options>\n"
		"Options:\n"
		"	-p <msec>[-<msec>]:             probing interval (fixed or min-max)\n"
		"	-s <rssi>[-<rssi>]:             rssi (signal strength) (fixed or min-max)\n"
		"	-n <n>:                         create <n> stations\n"
		"	-f <freq>:			set operating frequency\n"
		"	                                uses parameters set before this option\n"
		"	-v:				verbose\n"
		"\n", prog);
	return 1;
}

static bool parse_var(struct var *var, const char *str)
{
	char *err;

	var->min = strtoul(str, &err, 0);
	var->max = var->min;
	if (!*err)
		return true;

	if (*err != ':')
		return false;

	var->max = strtoul(err + 1, &err, 0);
	if (!*err)
		return true;

	return false;
}

static int
hostapd_bss_get_clients(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	blob_buf_init(&b, 0);
	ubus_send_reply(ctx, req, b.head);
	return 0;
}

static const struct ubus_method bss_methods[] = {
	UBUS_METHOD_NOARG("get_clients", hostapd_bss_get_clients),
};

static struct ubus_object_type bss_object_type =
	UBUS_OBJECT_TYPE("hostapd_bss", bss_methods);

static struct ubus_object bss_obj = {
	.name = "hostapd.wlan0",
	.type = &bss_object_type,
	.methods = bss_methods,
	.n_methods = ARRAY_SIZE(bss_methods),
};

int main(int argc, char **argv)
{
	struct sta_data sdata = {
		.signal = { 0, -30, -30 },
		.probe = { 0, 1000, 30000 },
	};
	int ch;

	uloop_init();

	r_fd = fopen("/dev/urandom", "r");
	if (!r_fd) {
		perror("fopen");
		return 1;
	}

	usteer_timeout_init(&tq);

	while ((ch = getopt(argc, argv, "p:s:f:n:v")) != -1) {
		switch(ch) {
		case 'p':
			if (!parse_var(&sdata.probe, optarg))
				goto usage;
			break;
		case 's':
			if (!parse_var(&sdata.signal, optarg))
				goto usage;
			break;
		case 'f':
			freq = atoi(optarg);
			break;
		case 'n':
			create_stations(&sdata, atoi(optarg));
			break;
		case 'v':
			verbose++;
			break;
		default:
			goto usage;
		}
	}

	ubus_ctx = ubus_connect(NULL);
	if (!ubus_ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
		return 1;
	}

	ubus_add_uloop(ubus_ctx);

	if (ubus_add_object(ubus_ctx, &bss_obj)) {
		fprintf(stderr, "Failed to register AP ubus object\n");
		return 1;
	}
	uloop_run();

	uloop_done();
	return 0;
usage:
	return usage(argv[0]);
}
