// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Felix Fietkau <nbd@nbd.name>
 */
#ifndef __BRIDGER_H
#define __BRIDGER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <libubox/uloop.h>
#include <libubox/list.h>
#include <libubox/avl.h>
#include <libubox/utils.h>

#include <bpf/bpf.h>
#include <bpf/libbpf.h>

#include "bridger-bpf.h"
#include "device.h"
#include "flow.h"
#include "fdb.h"
#include "bpf.h"
#include "nl.h"
#include "ubus.h"

#define D(format, ...) \
	bridger_dprintf("%s(%d) " format, __func__, __LINE__, ##__VA_ARGS__)

extern void bridger_dprintf(const char *format, ...);

#ifdef UBUS_SUPPORT
#include <udebug.h>
extern struct udebug_buf udb_nl;
void bridger_udebug_config(struct udebug_ubus *ctx, struct blob_attr *data,
			   bool enabled);
#endif

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define BRIDGER_PROG_PATH	"/lib/bpf/bridger-bpf.o"
#define BRIDGER_PIN_PATH	"/sys/fs/bpf/bridger"
#define BRIDGER_DATA_PATH	"/sys/fs/bpf/bridger_data"

#define BRIDGER_EWMA_SHIFT	8

#define BRIDGER_PRIO_BPF	0xbb00

#define BRIDGER_PRIO_OFFLOAD_8021Q	0xbb01
#define BRIDGER_PRIO_OFFLOAD_8021AD	0xbb02
#define BRIDGER_PRIO_OFFLOAD_UNTAG	0xbb03

#define BRIDGER_PRIO_OFFLOAD_START	BRIDGER_PRIO_OFFLOAD_8021Q
#define BRIDGER_PRIO_OFFLOAD_END	BRIDGER_PRIO_OFFLOAD_UNTAG


extern bool bridge_local_rx;

static inline void bridger_ewma(uint64_t *avg, uint32_t val)
{
	if (*avg)
		*avg = (*avg * 3) / 4 + ((uint64_t)val << BRIDGER_EWMA_SHIFT) / 4;
	else
		*avg = (uint64_t)val << BRIDGER_EWMA_SHIFT;
}

const char *format_macaddr(const uint8_t *mac);

#endif
