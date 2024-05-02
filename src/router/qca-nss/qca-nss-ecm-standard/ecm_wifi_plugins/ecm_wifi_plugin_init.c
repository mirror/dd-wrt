/*
 **************************************************************************
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
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
 **************************************************************************
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/skbuff.h>

#include "ecm_wifi_plugin.h"

/*
 * ecm_wifi_plugin_init_module()
 *	ECM_WIFI_PLUGIN module init function
 */
int __init ecm_wifi_plugin_init_module(void)
{
	int ret = 0;

	ret = ecm_wifi_plugin_emesh_register();
	if (ret) {
		ecm_wifi_plugin_warning("EMESH callback registration failed\n");
		return ret;
	}

	ret = ecm_wifi_plugin_fse_cb_register();
	if (ret) {
		ecm_wifi_plugin_emesh_unregister();
		ecm_wifi_plugin_warning("FSE callback registration failed\n");
		return ret;
	}

#ifndef ECM_WIFI_PLUGIN_OPEN_PROFILE_ENABLE
	ret = ecm_wifi_plugin_mscs_register();
	if (ret) {
		ecm_wifi_plugin_emesh_unregister();
		ecm_wifi_plugin_fse_cb_unregister();
		ecm_wifi_plugin_warning("MSCS callback registration failed\n");
		return ret;
	}

	ret = ecm_wifi_plugin_adm_ctrl_cb_register();
	if (ret) {
		ecm_wifi_plugin_emesh_unregister();
		ecm_wifi_plugin_fse_cb_unregister();
		ecm_wifi_plugin_mscs_unregister();
		ecm_wifi_plugin_warning("Admission control callback registration failed\n");
		return ret;
	}
#endif

	ecm_wifi_plugin_info("ECM_WIFI_PLUGIN module loaded");
	return 0;
}

/*
 * ecm_wifi_plugin_exit_module()
 *	ECM_WIFI_PLUGIN module exit function
 */
static void __exit ecm_wifi_plugin_exit_module(void)
{
	ecm_wifi_plugin_emesh_unregister();
	ecm_wifi_plugin_fse_cb_unregister();
#ifndef ECM_WIFI_PLUGIN_OPEN_PROFILE_ENABLE
	ecm_wifi_plugin_mscs_unregister();
	ecm_wifi_plugin_adm_ctrl_cb_unregister();
#endif
	ecm_wifi_plugin_info("ECM_WIFI_PLUGIN unloaded\n");
}

module_init(ecm_wifi_plugin_init_module);
module_exit(ecm_wifi_plugin_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("ECM_WIFI_PLUGIN module");
