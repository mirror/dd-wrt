/*
 **************************************************************************
 * Copyright (c) 2021, The Linux Foundation.  All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
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
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/ctype.h>
#include <linux/etherdevice.h>

#include "ecm_ae_classifier_public.h"

#if defined(ECM_FRONT_END_PPE_ENABLE)

/*
 * Sysctl table
 */
static struct ctl_table_header *ecm_ae_select_fallback_tbl_hdr;
static int ecm_ae_select_fallback_enable = true;

static int ecm_ae_test_select = ECM_AE_CLASSIFIER_RESULT_PPE_VP;

/*
 * ecm_ae_select_select()
 *	Select the acceleration engine amongst PPE-VP/PPE-DS/SFE
 */
ecm_ae_classifier_result_t ecm_ae_select(struct ecm_ae_classifier_info *info)
{
	return ecm_ae_test_select;

}
static struct ecm_ae_classifier_ops ae_ops = {
	.ae_get = ecm_ae_select,
	.ae_flags = (ECM_AE_CLASSIFIER_FLAG_EXTERNAL_AE_REGISTERED | ECM_AE_CLASSIFIER_FLAG_FALLBACK_ENABLE)
};

/*
 * ecm_ae_select_fallback_enable_handler()
 *	Fallback sysctl handler
 */
int ecm_ae_select_fallback_enable_handler(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int ret;

	/*
	 * Write the variable with user input
	 */
	ret = proc_dointvec(ctl, write, buffer, lenp, ppos);
	if (ret || (!write)) {
		return ret;
	}

	if ((ecm_ae_select_fallback_enable != 0) && (ecm_ae_select_fallback_enable != 1)) {
		pr_info("Invalid input. Valid values 0/1\n");
		return -EINVAL;
	}

	if (ecm_ae_select_fallback_enable) {
		ae_ops.ae_flags |= ECM_AE_CLASSIFIER_FLAG_FALLBACK_ENABLE;
	} else {
		ae_ops.ae_flags &= ~ECM_AE_CLASSIFIER_FLAG_FALLBACK_ENABLE;
	}

	return ret;
}

/*
 * ecm_ae_select_fallback_enable_handler()
 *	Fallback sysctl handler
 */
int ecm_ae_test_select_handler(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int ret;

	/*
	 * Write the variable with user input
	 */
	ret = proc_dointvec(ctl, write, buffer, lenp, ppos);
	if (ret || (!write)) {
		return ret;
	}

	if ((ecm_ae_test_select < 1) || (ecm_ae_test_select > 7)) {
		pr_info("Invalid input. Valid values 1 to 6\n");
		return -EINVAL;
	}

	return ret;
}

/*
 * With addition of hybrid classifier, we want to add a new sysctl to
 * control the fallback options that we have at our disposal.
 */
static struct ctl_table ecm_ae_select_fallback_tbl[] = {
	{
		.procname	= "ae_select_fallback",
		.data		= &ecm_ae_select_fallback_enable,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &ecm_ae_select_fallback_enable_handler,
	},

	/*
	 * Selecting options:
	 * 1: ECM_AE_CLASSIFIER_RESULT_PPE_VP
	 * 2: ECM_AE_CLASSIFIER_RESULT_PPE_DS
	 * 3: ECM_AE_CLASSIFIER_RESULT_SFE
	 * 4: ECM_AE_CLASSIFIER_RESULT_NONE
	 * 5: ECM_AE_CLASSIFIER_RESULT_NOT_YET
	 * 6: ECM_AE_CLASSIFIER_RESULT_DONT_CARE
	 */
	{
		.procname	= "test_select",
		.data		= &ecm_ae_test_select,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &ecm_ae_test_select_handler,
	},
	{}
};

/*
 * ecm_ae_select_sysctl_init()
 * 	Register sysctl for SFE
 */
int ecm_ae_select_sysctl_init(void)
{
	ecm_ae_select_fallback_tbl_hdr = register_sysctl("/net/ecm", ecm_ae_select_fallback_tbl);
	if (!ecm_ae_select_fallback_tbl_hdr) {
		pr_info("Unable to register ecm_ae_select_fallback_tbl");
		return -EINVAL;
	}

	pr_info("ECM AE sysctl INIT\n");

	return 0;
}

#else

/*
 * This is the module which selects the underlying acceleration engine
 * based on the 5-tuple and type of the flow. The flow can be multicast, routed,
 * bridged, ported/non-ported.
 */

/*
 * ecm_ae_select()
 *	Selects the acceleration engine based on the given flow information.
 */
ecm_ae_classifier_result_t ecm_ae_select(struct ecm_ae_classifier_info *info)
{
	pr_debug("%px: Acceleration engine selection\n", info);

	/*
	 * Multicast flows can be accelerated by NSS and SFE for now.
	 */
	if (info->flag & ECM_AE_CLASSIFIER_FLOW_MULTICAST) {
#ifdef ECM_FRONT_END_NSS_ENABLE
		return ECM_AE_CLASSIFIER_RESULT_NSS;
#else
		return ECM_AE_CLASSIFIER_RESULT_SFE;
#endif
	}

	/*
	 * Non-ported flows can be accelerated by NSS only.
	 */
	if (info->protocol != IPPROTO_TCP && info->protocol != IPPROTO_UDP) {
		return ECM_AE_CLASSIFIER_RESULT_NSS;
	}

	/*
	 * Let's accelerate all other routed flows by SFE.
	 */
	if (info->flag & ECM_AE_CLASSIFIER_FLOW_ROUTED) {
		return ECM_AE_CLASSIFIER_RESULT_SFE;
	}

	/*
	 * Let's accelerate all bridge flows by NSS.
	 */
	return ECM_AE_CLASSIFIER_RESULT_NSS;
}

static struct ecm_ae_classifier_ops ae_ops = {
	.ae_get = ecm_ae_select,
	.ae_flags = (ECM_AE_CLASSIFIER_FLAG_EXTERNAL_AE_REGISTERED)
};
#endif

/*
 * ecm_ae_select_init()
 */
static int __init ecm_ae_select_init(void)
{
	pr_info("ECM AE Select INIT\n");

	/*
	 * Register the callbacks.
	 */
	ecm_ae_classifier_ops_register(&ae_ops);

#if defined(ECM_FRONT_END_PPE_ENABLE)
	ecm_ae_select_sysctl_init();
#endif
	return 0;
}

/*
 * ecm_ae_select_exit()
 */
static void __exit ecm_ae_select_exit(void)
{
	pr_info("ECM AE Select EXIT\n");

	/*
	 * Unregister the callbacks.
	 */
	ecm_ae_classifier_ops_unregister();
}

module_init(ecm_ae_select_init)
module_exit(ecm_ae_select_exit)

MODULE_DESCRIPTION("ECM Acceleration Engine Selection Module");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif
