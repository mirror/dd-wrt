/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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


/**
 * @defgroup
 * @{
 */
#include <linux/switch.h>
#include "sw.h"
#include "ref_uci.h"

static const char *vport_phyport[] = {
	"port_id",
	"physical_port",
};

static const char *vport_statecheck[] = {
	"check_en",
	"vport_type",
	"vport_active",
	"tunnel_active",
};

int parse_vport(const char *command_name, struct switch_val *val)
{
	int rv = -1;

	if (!strcmp(command_name, "Phyport")) {
		rv = parse_uci_option(val, vport_phyport,
				sizeof(vport_phyport)/sizeof(char *));
	} else if (!strcmp(command_name, "Statecheck")) {
		rv = parse_uci_option(val, vport_statecheck,
				sizeof(vport_statecheck)/sizeof(char *));
	}

	return rv;
}

/**
 * @}
 */
