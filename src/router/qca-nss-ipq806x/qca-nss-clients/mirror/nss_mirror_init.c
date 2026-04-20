/*
 ***************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 ***************************************************************************
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>

#include "nss_mirror_ctl.h"
#include "nss_mirror.h"

/*
 * nss_mirror_module_init()
 *	Initialize mirror module.
 */
static int __init nss_mirror_module_init(void)
{
	int ret = 0;

	ret = nss_mirror_ctl_register();
	if (ret) {
		nss_mirror_warn("Mirror ctl registration failed\n");
		return ret;
	}

	nss_mirror_info("Mirror module initialized\n");
	return 0;
}

/*
 * nss_mirror_module_exit()
 *	De-initialize mirror module.
 */
static void __exit nss_mirror_module_exit(void)
{
	if (nss_mirror_destroy_all() < 0) {
		nss_mirror_warn("Error in destroying all the configured mirror devices\n");
	}
	nss_mirror_ctl_unregister();
	nss_mirror_info("Mirror module unloaded.\n");
}

module_init(nss_mirror_module_init);
module_exit(nss_mirror_module_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS MIRROR Client");
