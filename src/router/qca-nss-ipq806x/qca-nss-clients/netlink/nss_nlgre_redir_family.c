/*
 **************************************************************************
 * Copyright (c) 2015-2016,2018-2020, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/*
 * nss_nlgre_redir.c
 * 	NSS Netlink gre_redir Handler
 */
#include <linux/version.h>
#include <net/genetlink.h>
#include <nss_api_if.h>
#include <nss_nlcmn_if.h>
#include <nss_nl_if.h>
#include <nss_nlgre_redir_if.h>
#include "nss_nl.h"
#include "nss_nlgre_redir_cmd.h"

/*
 * nss_nlgre_redir_family_init()
 * 	handler init
 */
bool nss_nlgre_redir_family_init(void)
{
	int err;
	nss_nl_info_always("Init NSS netlink gre_redir handler\n");

	/*
	 * register NETLINK ops with the family
	 */
	err = genl_register_family(&nss_nlgre_redir_cmd_family);
	if (err) {
		nss_nl_info_always("Error: %d unable to register gre_redir family\n", err);
		return false;
	}

	return true;
}

/*
 * nss_nlgre_redir_family_exit()
 *	handler exit
 */
bool nss_nlgre_redir_family_exit(void)
{
	int err;
	nss_nl_info_always("Exit NSS netlink gre_redir handler\n");

	/*
	 * unregister the ops family
	 */
	err = genl_unregister_family(&nss_nlgre_redir_cmd_family);
	if (err) {
		nss_nl_info_always("Error: %d unable to unregister gre_redir NETLINK family\n", err);
		return false;
	}

	return true;
}
