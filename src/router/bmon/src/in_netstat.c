/*
 * in_netstat.c           netstat -i -a
 *
 * ... or probably-the-most-silly-attempt-to-gather-network-statistics
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
#include <bmon/item.h>
#include <bmon/node.h>
#include <bmon/conf.h>
#include <bmon/utils.h>
#include <inttypes.h>

static char *c_cmd = "netstat -i -a";

static void netstat_read(void)
{
	FILE *       fd;
	char         buf[512];
	
	if (!(fd = popen(c_cmd, "r")))
		quit("popen(c_cmd) failed: %s\n", strerror(errno));

    for (; fgets(buf, sizeof(buf), fd);) {
		char *p, *s;
		item_t *intf;

		if (buf[0] == '\n' || buf[0] == '\r')
			continue;
		
		if (!strncmp(buf, "Name", 4) ||
			!strncmp(buf, "Kernel", 6) ||
			!strncmp(buf, "Iface", 5))
			continue;
		
		
		if (!(p = strchr(buf, ' ')))
			continue;
		
		*p = '\0';
		s = p+1;
		
		for (p = &buf[0]; *p == ' ' || *p == '\t'; p++);


#if defined SYS_LINUX
		{
			int i;
			char flags[32];
			b_cnt_t rx_p, tx_p, rx_errors, tx_errors, rx_drop, tx_drop;

			i = sscanf(s, "%*s %*s %" SCNu64 " %" SCNu64 " %" SCNu64 " %*d "
				      "%" SCNu64 " %" SCNu64 " %" SCNu64 " %s",
					&rx_p, &rx_errors, &rx_drop, &tx_p, &tx_errors, &tx_drop, flags);
			
			if (i != 7)
				continue;

			if (get_show_only_running() && !strchr(flags, 'R'))
				continue;

			intf = lookup_item(get_local_node(), p, 0, 0);

			if (NULL == intf)
				continue;

			intf->i_major_attr = BYTES;
			intf->i_minor_attr = PACKETS;

			update_attr(intf, PACKETS, rx_p, tx_p, RX_PROVIDED | TX_PROVIDED);
			update_attr(intf, ERRORS, rx_errors, tx_errors,
				RX_PROVIDED | TX_PROVIDED);
			update_attr(intf, DROP, rx_drop, tx_drop,
				RX_PROVIDED | TX_PROVIDED);
		}
#else
		{
			int i;
			b_cnt_t rx_p, tx_p, rx_errors, tx_errors, tx_frame;
			
			i = sscanf(s, "%*s %*s %*s %llu %llu %llu %llu %llu",
				&rx_p, &rx_errors, &tx_p, &tx_errors, &tx_frame);
			
			if (i != 5)
				continue;

			/*
			 * XXX: get_show_only_running()
			 */

			intf = lookup_item(get_local_node(), p, 0, 0);

			if (NULL == intf)
				continue;

			intf->i_major_attr = BYTES;
			intf->i_minor_attr = PACKETS;

			update_attr(intf, PACKETS, rx_p, tx_p, RX_PROVIDED | TX_PROVIDED);
			update_attr(intf, ERRORS, rx_errors, tx_errors,
				RX_PROVIDED | TX_PROVIDED);
			update_attr(intf, FRAME, 0, tx_frame, TX_PROVIDED);
		}
#endif

		notify_update(intf, NULL);
		increase_lifetime(intf, 1);
	}
	
	pclose(fd);
}

static int netstat_probe(void)
{
	FILE *fd = popen(c_cmd, "r");

	if (fd) {
		pclose(fd);
		return 1;
	}

	return 0;
}

static void print_help(void)
{
	printf(
	"netstat - statistic collector using netstat -i\n" \
	"\n" \
	"  Very lame attempt to collect statistics by parsing the output\n" \
	"  of netstat -i -a. Only the packet counters and a few error counters\n" \
	"  are provided depending on the architecture. If all fails this could\n" \
	"  theoretically be used to see at least some of the statistics.\n" \
	"\n" \
	"  WARNING: The default behaviour is to start netstat -i -a without an absolute\n" \
	"  path which means that an attacker could create an executable `netstat'\n" \
	"  which would then be run under the privileges you started bmon. For that\n" \
	"  reason this input method is never used by default\n" \
	"  Author: Thomas Graf <tgraf@suug.ch>\n" \
	"\n" \
	"  Options:\n" \
	"    cmd=STR       Netstat command to use (default: netstat -i -a)\n" \
	"\n");
}

static void netstat_set_opts(tv_t *attrs)
{
	while (attrs) {
		if (!strcasecmp(attrs->type, "cmd") && attrs->value)
			c_cmd = attrs->value;
		else if (!strcasecmp(attrs->type, "help")) {
			print_help();
			exit(0);
		}
		attrs = attrs->next;
	}
}

static struct input_module netstat_ops = {
	.im_name = "netstat",
	.im_read = netstat_read,
	.im_set_opts = netstat_set_opts,
	.im_probe = netstat_probe,
	.im_no_default = 1,
};

static void __init netstat_init(void)
{
	register_input_module(&netstat_ops);
}
