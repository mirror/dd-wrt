/*
 * in_dummy.c                Dummy Input Method
 *
 * Copyright (c) 2001-2004 Thomas Graf <tgraf@suug.ch>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <bmon/bmon.h>
#include <bmon/input.h>
#include <bmon/node.h>
#include <bmon/item.h>
#include <bmon/utils.h>

static b_cnt_t c_rx_b_inc = 1000000000;
static b_cnt_t c_tx_b_inc = 80000000;
static b_cnt_t c_rx_p_inc = 1000;
static b_cnt_t c_tx_p_inc = 800;
static int c_numdev = 5;
static int c_randomize = 0;
static int c_mtu = 1540;
static int c_maxpps = 100000;

static void dummy_read(void)
{
	int n;

	for (n = 0; n < c_numdev; n++) {
		char ifname[IFNAMSIZ];
		item_t *it;

		snprintf(ifname, sizeof(ifname), "dummy%d", n);
		
		it = lookup_item(get_local_node(), ifname, 0, 0);
		if (it == NULL)
			return;

		it->i_major_attr = BYTES;
		it->i_minor_attr = PACKETS;

		if (c_randomize) {
			b_cnt_t rx = rand() % c_maxpps;
			b_cnt_t tx = rand() % c_maxpps;
			
			update_attr(it, PACKETS, rx, tx, RX_PROVIDED | TX_PROVIDED);
			update_attr(it, BYTES, rx * (rand() % c_mtu),
				    tx * (rand() % c_mtu), RX_PROVIDED | TX_PROVIDED);
		} else {
			update_attr(it, PACKETS, c_rx_p_inc, c_tx_p_inc,
				    RX_PROVIDED | TX_PROVIDED);
			update_attr(it, BYTES, c_rx_b_inc, c_tx_b_inc,
				    RX_PROVIDED | TX_PROVIDED);
		}

		notify_update(it, NULL);
		increase_lifetime(it, 1);
	}
}

static void print_help(void)
{
	printf(
	"dummy - Statistic generator module (dummy)\n" \
	"\n" \
	"  Basic statistic generator for testing purposes. Can produce a\n" \
	"  constant or random statistic flow with configurable parameters.\n" \
	"  Author: Thomas Graf <tgraf@suug.ch>\n" \
	"\n" \
	"  Options:\n" \
	"    rxb=NUM        RX bytes increment amount (default: 10^9)\n" \
	"    txb=NUM        TX bytes increment amount (default: 8*10^7)\n" \
	"    rxp=NUM        RX packets increment amount (default: 1K)\n" \
	"    txp=NUM        TX packets increment amount (default: 800)\n" \
	"    num=NUM        Number of devices (default: 5)\n" \
	"    randomize      Randomize counters (default: off)\n" \
	"    seed=NUM       Seed for randomizer (default: time(0))\n" \
	"    mtu=NUM        Maximal Transmission Unit (default: 1540)\n" \
	"    maxpps=NUM     Upper limit for packets per second (default: 100K)\n" \
	"\n" \
	"  Randomizer:\n" \
	"    RX-packets := Rand() %% maxpps\n" \
	"    TX-packets := Rand() %% maxpps\n" \
	"    RX-bytes   := RX-packets * (Rand() %% mtu)\n" \
	"    TX-bytes   := TX-packets * (Rand() %% mtu)\n");
}

static void dummy_set_opts(tv_t *attrs)
{
	while (attrs) {
		if (!strcasecmp(attrs->type, "rxb") && attrs->value)
			c_rx_b_inc = strtol(attrs->value, NULL, 0);
		else if (!strcasecmp(attrs->type, "txb") && attrs->value)
			c_tx_b_inc = strtol(attrs->value, NULL, 0);
		else if (!strcasecmp(attrs->type, "rxp") && attrs->value)
			c_rx_p_inc = strtol(attrs->value, NULL, 0);
		else if (!strcasecmp(attrs->type, "txp") && attrs->value)
			c_tx_p_inc = strtol(attrs->value, NULL, 0);
		else if (!strcasecmp(attrs->type, "num") && attrs->value)
			c_numdev = strtol(attrs->value, NULL, 0);
		else if (!strcasecmp(attrs->type, "randomize")) {
			c_randomize = 1;
			srand(time(0));
		} else if (!strcasecmp(attrs->type, "seed") && attrs->value)
			srand(strtol(attrs->value, NULL, 0));
		else if (!strcasecmp(attrs->type, "mtu") && attrs->value)
			c_mtu = strtol(attrs->value, NULL, 0);
		else if (!strcasecmp(attrs->type, "maxpps") && attrs->value)
			c_maxpps = strtol(attrs->value, NULL, 0);
		else if (!strcasecmp(attrs->type, "help")) {
			print_help();
			exit(0);
		}
		
		attrs = attrs->next;
	}
}

static int dummy_probe(void)
{
	return 1;
}

static struct input_module dummy_ops = {
	.im_name = "dummy",
	.im_read = dummy_read,
	.im_set_opts = dummy_set_opts,
	.im_probe = dummy_probe,
	.im_no_default = 1,
};

static void __init dummy_init(void)
{
	register_input_module(&dummy_ops);
}
