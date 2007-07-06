/*
 * in_proc.c		       /proc/net/dev Input (Linux)
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
#include <inttypes.h>

static char *c_path = "/proc/net/dev";

static void proc_read(void)
{
	FILE *  fd;
	char    buf[512], *p, *s;
	int     w;
	item_t *it;
	
	if (!(fd = fopen(c_path, "r")))
		quit("Unable to open file %s: %s\n", c_path, strerror(errno));
	
	fgets(buf, sizeof(buf), fd);
	fgets(buf, sizeof(buf), fd);
	
	for (; fgets(buf, sizeof(buf), fd);) {
		b_cnt_t rx_errors, rx_drop, rx_fifo, rx_frame, rx_compressed;
		b_cnt_t rx_multicast, tx_errors, tx_drop, tx_fifo, tx_frame;
		b_cnt_t tx_compressed, tx_multicast, rx_bytes, tx_bytes;
		b_cnt_t rx_packets, tx_packets;
		
		if (buf[0] == '\r' || buf[0] == '\n')
			continue;
		
		if (!(p = strchr(buf, ':')))
			continue;
		*p = '\0';
		s = (p + 1);
		
		for (p = &buf[0]; *p == ' '; p++);

		/*
		 * XXX: get_show_only_running
		 */
		
		if ((it = lookup_item(get_local_node(), p, 0, 0)) == NULL)
			continue;
		
		w = sscanf(s, "%" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " "
			      "%" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " "
			      "%" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " "
			      "%" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 "\n",
		    &rx_bytes, &rx_packets, &rx_errors, &rx_drop, &rx_fifo,
		    &rx_frame, &rx_compressed, &rx_multicast, &tx_bytes,
		    &tx_packets, &tx_errors, &tx_drop, &tx_fifo,
		    &tx_frame, &tx_compressed, &tx_multicast);
		
		if (w != 16)
			continue;
	
		it->i_major_attr = BYTES;
		it->i_minor_attr = PACKETS;
	
		update_attr(it, BYTES, rx_bytes, tx_bytes, RX_PROVIDED|TX_PROVIDED);
		update_attr(it, PACKETS, rx_packets, tx_packets, RX_PROVIDED|TX_PROVIDED);
		update_attr(it, ERRORS, rx_errors, tx_errors, RX_PROVIDED|TX_PROVIDED);
		update_attr(it, DROP, rx_drop, tx_drop, RX_PROVIDED|TX_PROVIDED);
		update_attr(it, FIFO, rx_fifo, tx_fifo, RX_PROVIDED|TX_PROVIDED);
		update_attr(it, FRAME, rx_frame, tx_frame, RX_PROVIDED|TX_PROVIDED);
		update_attr(it, COMPRESSED, rx_compressed, tx_compressed, RX_PROVIDED|TX_PROVIDED);
		update_attr(it, MULTICAST, rx_multicast, tx_multicast, RX_PROVIDED|TX_PROVIDED);

		notify_update(it, NULL);
		increase_lifetime(it, 1);
	}
	
	fclose(fd);
}

static void print_help(void)
{
	printf(
	"proc - procfs statistic collector for Linux" \
	"\n" \
	"  Reads statistics from procfs (/proc/net/dev)\n" \
	"  Author: Thomas Graf <tgraf@suug.ch>\n" \
	"\n" \
	"  Options:\n" \
	"    file=PATH	    Path to statistics file (default: /proc/net/dev)\n");
}

static void proc_set_opts(tv_t *attrs)
{
	while (attrs) {
		if (!strcasecmp(attrs->type, "file") && attrs->value)
			c_path = attrs->value;
		else if (!strcasecmp(attrs->type, "help")) {
			print_help();
			exit(0);
		}
		attrs = attrs->next;
	}
}

static int proc_probe(void)
{
	FILE *fd = fopen(c_path, "r");

	if (fd) {
		fclose(fd);
		return 1;
	}
	return 0;
}

static struct input_module proc_ops = {
	.im_name = "proc",
	.im_read = proc_read,
	.im_set_opts = proc_set_opts,
	.im_probe = proc_probe,
};

static void __init proc_init(void)
{
	register_input_module(&proc_ops);
}
