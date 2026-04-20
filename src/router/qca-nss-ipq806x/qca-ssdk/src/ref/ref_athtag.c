/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

static const char *athtag_primapping[] = {
	"direction",
	"ath_pri",
	"int_pri",
};

static const char *athtag_portmapping[] = {
	"direction",
	"ath_port",
	"int_port",
};

static const char *athtag_rx[] = {
	"port_id",
	"athtag_en",
	"athtag_type",
};

static const char *athtag_tx[] = {
	"port_id",
	"athtag_en",
	"athtag_type",
	"athtag_version",
	"athtag_action",
	"athtag_bypass_fwd_en",
	"athtag_field_disable",
};

int parse_athtag(const char *command_name, struct switch_val *val)
{
	int rv = -1;

	if (!strcmp(command_name, "Primapping")) {
		rv = parse_uci_option(val, athtag_primapping,
				sizeof(athtag_primapping)/sizeof(char *));
	} else if (!strcmp(command_name, "Portmapping")) {
		rv = parse_uci_option(val, athtag_portmapping,
				sizeof(athtag_portmapping)/sizeof(char *));
	} else if (!strcmp(command_name, "Rx")) {
		rv = parse_uci_option(val, athtag_rx,
				sizeof(athtag_rx)/sizeof(char *));
	} else if (!strcmp(command_name, "Tx")) {
		rv = parse_uci_option(val, athtag_tx,
				sizeof(athtag_tx)/sizeof(char *));
	}

	return rv;
}

/**
 * @}
 */
