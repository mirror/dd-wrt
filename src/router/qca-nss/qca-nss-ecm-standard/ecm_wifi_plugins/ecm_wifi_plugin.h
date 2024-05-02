/*
 ***************************************************************************
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <linux/of.h>
#include <linux/module.h>
#include <linux/skbuff.h>

#ifdef ECM_WIFI_PLUGIN_OPEN_PROFILE_ENABLE
#include <ath_sawf.h>
#include <ath_fse.h>
#else
#include <qca_mscs_if.h>
#include <qca_mesh_latency_if.h>
#include <qca_sawf_if.h>
#include <qca_fse_if.h>
#ifdef ECM_CLASSIFIER_MSCS_SCS_ENABLE
#include <qca_scs_if.h>
#endif
#endif

#include <ecm_classifier_mscs_public.h>
#include <ecm_classifier_emesh_public.h>
#include <ecm_front_end_common_public.h>

#if defined(CONFIG_DYNAMIC_DEBUG)

/*
 * Compile messages for dynamic enable/disable
 */
#define ecm_wifi_plugin_warning(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define ecm_wifi_plugin_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define ecm_wifi_plugin_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels
 */
#if (ECM_WIFI_PLUGIN_DEBUG_LEVEL < 2)
#define ecm_wifi_plugin_warning(s, ...)
#else
#define ecm_wifi_plugin_warning(s, ...) pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (ECM_WIFI_PLUGIN_DEBUG_LEVEL < 3)
#define ecm_wifi_plugin_info(s, ...)
#else
#define ecm_wifi_plugin_info(s, ...)   pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (ECM_WIFI_PLUGIN_DEBUG_LEVEL < 4)
#define ecm_wifi_plugin_trace(s, ...)
#else
#define ecm_wifi_plugin_trace(s, ...)  pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif

#ifdef ECM_WIFI_PLUGIN_OPEN_PROFILE_ENABLE
#define ECM_WIFI_PLUGIN_SAWF_TAG 0xAA
#define ECM_WIFI_PLUGIN_SAWF_TAG_SHIFT 8
#define ECM_WIFI_PLUGIN_SAWF_SERVICE_CLASS_MASK 0xFF
#define ECM_WIFI_PLUGIN_SAWF_SERVICE_CLASS_SHIFT 16
#define ECM_WIFI_PLUGIN_SAWF_MSDUQ_MASK 0xFFFF
#endif

/*
 * ecm_wifi_plugin_emesh_register()
 *	API to register emesh callbacks.
 */
int ecm_wifi_plugin_emesh_register(void);

/*
 * ecm_wifi_plugin_emesh_unregister()
 *	API to unregister the emesh callbacks.
 */
void ecm_wifi_plugin_emesh_unregister(void);

/*
 * ecm_wifi_plugin_fse_cb_register()
 *	API to register FSE (Flow Search Engine) programming callbacks.
 */
int ecm_wifi_plugin_fse_cb_register(void);

/*
 * ecm_wifi_plugin_fse_cb_unregister()
 *	API to unregister FSE (Flow Search Engine) programming callbacks.
 */
void ecm_wifi_plugin_fse_cb_unregister(void);

#ifndef ECM_WIFI_PLUGIN_OPEN_PROFILE_ENABLE
/*
 * ecm_wifi_plugin_sdwf_deprio
 * 	API to deprioritize a flow
 */
void ecm_wifi_plugin_sdwf_deprio(struct qca_sawf_flow_deprioritize_params *param);

/*
 * ecm_wifi_plugin_adm_ctrl_cb_register()
 *	API to register admission control callbacks.
 */
int ecm_wifi_plugin_adm_ctrl_cb_register(void);

/*
 * ecm_wifi_plugin_adm_ctrl_cb_unregister()
 *	API to unregister the admission control callbacks.
 */
void ecm_wifi_plugin_adm_ctrl_cb_unregister(void);

/*
 * ecm_wifi_plugin_mscs_register()
 *	API to register mscs callbacks.
 */
int ecm_wifi_plugin_mscs_register(void);

/*
 * ecm_wifi_plugin_mscs_unregister()
 *	API to unregister the mscs callbacks.
 */
void ecm_wifi_plugin_mscs_unregister(void);
#endif
