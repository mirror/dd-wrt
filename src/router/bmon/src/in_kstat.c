/*
 * in_kstat.c                  libkstat (SunOS)
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
#include <bmon/utils.h>

#if defined HAVE_KSTAT && defined SYS_SUNOS
#include <kstat.h>

static void kstat_do_read(void)
{
	kstat_ctl_t *   kc;
	kstat_t *       kst;
	kstat_named_t * kn, *kn2;
	item_t *        i;
	
	if (!(kc = kstat_open()))
		quit("kstat_open() failed");
	
	if ((kst = kstat_lookup(kc, NULL, -1, NULL))) {
		for (; kst; kst = kst->ks_next) {
			if (strcmp(kst->ks_class, "net"))
				continue;
			
			if (kstat_read(kc, kst, NULL) < 0)
				continue;

			if (!strcmp(kst->ks_name, "zero_copy"))
				continue;

			i = lookup_item(get_local_node(), kst->ks_name, 0, 0);

			if (NULL == i)
				continue;

			i->i_major_attr = BYTES;
			i->i_minor_attr = PACKETS;

#define KSTAT_GET(S) (kstat_named_t *) kstat_data_lookup(kst, #S)

			if ((kn = KSTAT_GET(rbytes64)) && (kn2 = KSTAT_GET(obytes64))) {
				update_attr(i, BYTES, kn->value.ui64, kn2->value.ui64,
				    RX_PROVIDED | TX_PROVIDED | IS64BIT);
			} else if ((kn = KSTAT_GET(rbytes)) && (kn2 = KSTAT_GET(obytes)))
				update_attr(i, BYTES, kn->value.ui32, kn2->value.ui32,
				    RX_PROVIDED | TX_PROVIDED);

			if ((kn = KSTAT_GET(ipackets64)) && (kn2 = KSTAT_GET(opackets64))) {
				update_attr(i, PACKETS, kn->value.ui64, kn2->value.ui64,
				    RX_PROVIDED | TX_PROVIDED | IS64BIT);
			} else if ((kn = KSTAT_GET(ipackets)) && (kn2 = KSTAT_GET(opackets)))
				update_attr(i, PACKETS, kn->value.ui32, kn2->value.ui32,
				    RX_PROVIDED | TX_PROVIDED);

			if ((kn = KSTAT_GET(ierror)) && (kn2 = KSTAT_GET(oerrors)))
				update_attr(i, ERRORS, kn->value.ui32, kn2->value.ui32,
					RX_PROVIDED | TX_PROVIDED);

			if ((kn = KSTAT_GET(multircv64)) && (kn2 = KSTAT_GET(multixmt64)))
				update_attr(i, MULTICAST, kn->value.ui64, kn2->value.ui64,
					RX_PROVIDED | TX_PROVIDED);
			else if ((kn = KSTAT_GET(multircv)) && (kn2 = KSTAT_GET(multixmt)))
				update_attr(i, MULTICAST, kn->value.ui32, kn2->value.ui32,
					RX_PROVIDED | TX_PROVIDED);

			if ((kn = KSTAT_GET(brdcstrcv)) && (kn2 = KSTAT_GET(brdcstxmt)))
				update_attr(i, BROADCAST, kn->value.ui32, kn2->value.ui32,
					RX_PROVIDED | TX_PROVIDED);

#undef KSTAT_GET

			notify_update(i, NULL);
			increase_lifetime(i, 1);
		}
	}
	
	kstat_close(kc);
}

static void print_help(void)
{
	printf(
	"kstat - kstat statistic collector for SunOS\n" \
	"\n" \
	"  SunOS statistic collector using libkstat, kindly provides both,\n" \
	"  32bit and 64bit counters.\n" \
	"  Author: Thomas Graf <tgraf@suug.ch>\n" \
	"\n");
}

static void kstat_set_opts(tv_t *attrs)
{
	while (attrs) {
		if (!strcasecmp(attrs->type, "help")) {
			print_help();
			exit(0);
		}
		attrs = attrs->next;
	}
}

static int kstat_probe(void)
{
	kstat_ctl_t *kc = kstat_open();

	if (kc) {
		kstat_t * kst = kstat_lookup(kc, NULL, -1, NULL);
		kstat_close(kc);

		if (kst)
			return 1;
	}

	return 0;
}

static struct input_module kstat_ops = {
	.im_name = "kstat",
	.im_read = kstat_do_read,
	.im_set_opts = kstat_set_opts,
	.im_probe = kstat_probe,
};

static void __init kstat_init(void)
{
	register_input_module(&kstat_ops);
}

#endif
