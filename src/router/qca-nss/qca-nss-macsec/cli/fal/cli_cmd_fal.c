/*
 * Copyright (c) 2014, 2016, 2018-2019, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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


#include "vty.h"
#include "cli_lib.h"
#include "cli.h"
#include "cli_cmd.h"
#include "nss_macsec_mib.h"
#include "nss_macsec_secy.h"
#include "nss_macsec_secy_rx.h"
#include "nss_macsec_secy_tx.h"

fal_tx_sak_t g_fal_tx_sak_t;
fal_rx_ctl_filt_t g_fal_rx_ctl_filt_t;
fal_rx_prc_lut_t g_fal_rx_prc_lut_t;
fal_tx_ctl_filt_t g_fal_tx_ctl_filt_t;
fal_tx_mib_t g_fal_tx_mib_t;
fal_rx_mib_t g_fal_rx_mib_t;
fal_rx_sa_mib_t g_fal_rx_sa_mib_t;
fal_rx_sak_t g_fal_rx_sak_t;
fal_tx_class_lut_t g_fal_tx_class_lut_t;
fal_tx_sa_mib_t g_fal_tx_sa_mib_t;
fal_tx_sc_mib_t g_fal_tx_sc_mib_t;
struct fal_tx_sa_ki_t g_fal_tx_sa_ki_t;
struct fal_rx_sa_ki_t g_fal_rx_sa_ki_t;
struct fal_tx_udf_filt_cfg_t g_fal_tx_udf_ufilt_cfg_t;
struct fal_tx_udf_filt_cfg_t g_fal_tx_udf_cfilt_cfg_t;
struct fal_tx_udf_filt_t g_fal_tx_udf_filt_t;
struct fal_rx_udf_filt_cfg_t g_fal_rx_udf_ufilt_cfg_t;
struct fal_rx_udf_filt_t g_fal_rx_udf_filt_t;

DEFCMD(g_fal_tx_sa_ki_t_init_func,
	g_fal_tx_sa_ki_t_init_cmd,
	"g_fal_tx_sa_ki_t init", "g_fal_tx_sa_ki_t\n init\n")
{
	memset(&g_fal_tx_sa_ki_t, 0, sizeof(g_fal_tx_sa_ki_t));

	return CLI_OK;
}

void _dump_g_fal_tx_sa_ki_t(VTY_T *pVty, struct fal_tx_sa_ki_t *value)
{
	char str[64] = { 0 };

	cli_sak_2_str(value->ki, str, 63);
	vty_print("ki : %s\n", str);
	vty_print("\n");
}

DEFCMD(fal_tx_sa_ki_t_ki_add_func,
	fal_tx_sa_ki_t_ki_add_cmd,
	"fal_tx_sa_ki_t ki add WORD", "fal_tx_sa_ki_t\n ki\n add\n ki\n")
{
	CLI_EXEC_API(cli_str_2_sak(argv[3], g_fal_tx_sa_ki_t.ki));
	return CLI_OK;
}

DEFCMD(g_fal_rx_sa_ki_t_init_func,
	g_fal_rx_sa_ki_t_init_cmd,
	"g_fal_rx_sa_ki_t init", "g_fal_rx_sa_ki_t\n init\n")
{
	memset(&g_fal_rx_sa_ki_t, 0, sizeof(g_fal_rx_sa_ki_t));

	return CLI_OK;
}

void _dump_g_fal_rx_sa_ki_t(VTY_T *pVty, struct fal_rx_sa_ki_t *value)
{
	char str[64] = { 0 };

	cli_sak_2_str(value->ki, str, 63);
	vty_print("ki : %s\n", str);
	vty_print("\n");
}
DEFCMD(fal_rx_sa_ki_t_ki_add_func,
	fal_rx_sa_ki_t_ki_add_cmd,
	"fal_rx_sa_ki_t ki add WORD", "fal_rx_sa_ki_t\n ki\n add\n ki\n")
{
	CLI_EXEC_API(cli_str_2_sak(argv[3], g_fal_rx_sa_ki_t.ki));
	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sa_ki_get_func,
	nss_macsec_secy_tx_sa_ki_get_cmd,
	"nss_macsec_secy_tx_sa_ki get HEX HEX HEX",
	"nss_macsec_secy_tx_sa_ki\n get\n secy_id\n channel\n an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_tx_sa_ki_get
		     (secy_id, channel, an, &g_fal_tx_sa_ki_t));

	_dump_g_fal_tx_sa_ki_t(pVty, &g_fal_tx_sa_ki_t);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sa_ki_set_func,
	nss_macsec_secy_tx_sa_ki_set_cmd,
	"nss_macsec_secy_tx_sa_ki set HEX HEX HEX",
	"nss_macsec_secy_tx_sa_ki\n set\n secy_id\n channel\n an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_tx_sa_ki_set
		     (secy_id, channel, an, &g_fal_tx_sa_ki_t));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sa_ki_get_func,
	nss_macsec_secy_rx_sa_ki_get_cmd,
	"nss_macsec_secy_rx_sa_ki get HEX HEX HEX",
	"nss_macsec_secy_rx_sa_ki\n get\n secy_id\n channel\n an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_rx_sa_ki_get
		     (secy_id, channel, an, &g_fal_rx_sa_ki_t));

	_dump_g_fal_rx_sa_ki_t(pVty, &g_fal_rx_sa_ki_t);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sa_ki_set_func,
	nss_macsec_secy_rx_sa_ki_set_cmd,
	"nss_macsec_secy_rx_sa_ki set HEX HEX HEX",
	"nss_macsec_secy_rx_sa_ki\n set\n secy_id\n channel\n an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_rx_sa_ki_set
		     (secy_id, channel, an, &g_fal_rx_sa_ki_t));

	return CLI_OK;
}

DEFCMD(g_fal_tx_sak_t_init_func,
       g_fal_tx_sak_t_init_cmd,
       "g_fal_tx_sak_t init", "g_fal_tx_sak_t\n" "init\n")
{
	memset(&g_fal_tx_sak_t, 0, sizeof(g_fal_tx_sak_t));

	return CLI_OK;
}

void _dump_g_fal_tx_sak_t(VTY_T *pVty, fal_tx_sak_t *value)
{
	char str[64] = { 0 };

	cli_sak_2_str(value->sak, str, 63);
	vty_print("sak : %s\n", str);
	vty_print("\n");
}

void _dump_g_fal_tx_sak1_t(VTY_T *pVty, fal_tx_sak_t *value)
{
	char str[64] = { 0 };

	cli_sak_2_str(value->sak1, str, 63);
	vty_print("sak1 : %s\n", str);
	vty_print("\n");
}

DEFCMD(fal_tx_sak_t_key_len_add_func,
	fal_tx_sak_t_key_len_add_cmd,
	"fal_tx_sak_t sak_len add HEX",
	"fal_tx_sak_t\n sak_len\n add\n sak_len\n")
{
	u32 len = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &len));
	g_fal_tx_sak_t.sak_len = len;

	return CLI_OK;
}

DEFCMD(fal_tx_sak_t_sak_add_func,
	fal_tx_sak_t_sak_add_cmd,
	"fal_tx_sak_t sak add WORD", "fal_tx_sak_t\n sak\n add\n sak\n")
{
	CLI_EXEC_API(cli_str_2_sak(argv[3], g_fal_tx_sak_t.sak));
	return CLI_OK;
}

DEFCMD(fal_tx_sak_t_sak1_add_func,
	fal_tx_sak_t_sak1_add_cmd,
	"fal_tx_sak_t sak1 add WORD", "fal_tx_sak_t\n sak1\n add\n sak\n")
{
	CLI_EXEC_API(cli_str_2_sak(argv[3], g_fal_tx_sak_t.sak1));
	return CLI_OK;
}

DEFCMD(g_fal_rx_ctl_filt_t_init_func,
       g_fal_rx_ctl_filt_t_init_cmd,
       "g_fal_rx_ctl_filt_t init", "g_fal_rx_ctl_filt_t\n" "init\n")
{
	memset(&g_fal_rx_ctl_filt_t, 0, sizeof(g_fal_rx_ctl_filt_t));

	return CLI_OK;
}

void _dump_g_fal_rx_ctl_filt_t(VTY_T *pVty, fal_rx_ctl_filt_t *value)
{
	char mac_str[32] = { 0 };

	vty_print("match_type : 0x%x\n", value->match_type);
	vty_print("match_mask : 0x%x\n", value->match_mask);
	vty_print("bypass : 0x%x\n", value->bypass);
	cli_mac_2_str(value->sa_da_addr, mac_str, sizeof(mac_str));
	vty_print("sa_da_addr : %s\n", mac_str);
	vty_print("ether_type_da_range : 0x%x\n", value->ether_type_da_range);
	vty_print("\n");
}

DEFCMD(fal_rx_ctl_filt_t_match_type_add_func,
       fal_rx_ctl_filt_t_match_type_add_cmd,
       "fal_rx_ctl_filt_t match_type add HEX",
       "fal_rx_ctl_filt_t\n" "match_type\n" "add\n" "match_type\n")
{
	u32 match_type = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &match_type));
	g_fal_rx_ctl_filt_t.match_type = (fal_ig_ctl_match_type_e) match_type;

	return CLI_OK;
}

DEFCMD(fal_rx_ctl_filt_t_match_mask_add_func,
       fal_rx_ctl_filt_t_match_mask_add_cmd,
       "fal_rx_ctl_filt_t match_mask add HEX",
       "fal_rx_ctl_filt_t\n" "match_mask\n" "add\n" "match_mask\n")
{
	u32 match_mask = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &match_mask));
	g_fal_rx_ctl_filt_t.match_mask = (u16) match_mask;

	return CLI_OK;
}

DEFCMD(fal_rx_ctl_filt_t_bypass_add_func,
       fal_rx_ctl_filt_t_bypass_add_cmd,
       "fal_rx_ctl_filt_t bypass add HEX",
       "fal_rx_ctl_filt_t\n" "bypass\n" "add\n" "bypass\n")
{
	u32 bypass = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &bypass));
	g_fal_rx_ctl_filt_t.bypass = (bool) bypass;

	return CLI_OK;
}

DEFCMD(fal_rx_ctl_filt_t_sa_da_addr_add_func,
       fal_rx_ctl_filt_t_sa_da_addr_add_cmd,
       "fal_rx_ctl_filt_t sa_da_addr add H.H.H",
       "fal_rx_ctl_filt_t\n" "sa_da_addr\n" "add\n" "mac\n")
{
	CLI_EXEC_API(cli_str_2_mac(argv[3], g_fal_rx_ctl_filt_t.sa_da_addr));
	return CLI_OK;
}

DEFCMD(fal_rx_ctl_filt_t_ether_type_da_range_add_func,
       fal_rx_ctl_filt_t_ether_type_da_range_add_cmd,
       "fal_rx_ctl_filt_t ether_type_da_range add HEX",
       "fal_rx_ctl_filt_t\n"
       "ether_type_da_range\n" "add\n" "ether_type_da_range\n")
{
	u32 ether_type_da_range = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &ether_type_da_range));
	g_fal_rx_ctl_filt_t.ether_type_da_range = (u16) ether_type_da_range;

	return CLI_OK;
}

DEFCMD(g_fal_rx_prc_lut_t_init_func,
       g_fal_rx_prc_lut_t_init_cmd,
       "g_fal_rx_prc_lut_t init", "g_fal_rx_prc_lut_t\n" "init\n")
{
	memset(&g_fal_rx_prc_lut_t, 0, sizeof(g_fal_rx_prc_lut_t));

	return CLI_OK;
}

void _dump_g_fal_rx_prc_lut_t(VTY_T *pVty, fal_rx_prc_lut_t *value)
{
	char mac_str[32] = { 0 };

	vty_print("ether_type_mask : 0x%x\n", value->ether_type_mask);
	vty_print("sa_mask : 0x%x\n", value->sa_mask);
	vty_print("da_mask : 0x%x\n", value->da_mask);
	vty_print("sci_mask : 0x%x\n", value->sci_mask);
	vty_print("ether_type : 0x%x\n", value->ether_type);
	cli_mac_2_str(value->da, mac_str, sizeof(mac_str));
	vty_print("da : %s\n", mac_str);
	vty_print("channel : 0x%x\n", value->channel);
	vty_print("uncontrolled_port : 0x%x\n", value->uncontrolled_port);
	vty_print("valid : 0x%x\n", value->valid);
	vty_print("tci_mask : 0x%x\n", value->tci_mask);
	cli_sci_2_str(value->sci, mac_str, sizeof(mac_str));
	vty_print("sci : %s\n", mac_str);
	vty_print("tci : 0x%x\n", value->tci);
	cli_mac_2_str(value->sa, mac_str, sizeof(mac_str));
	vty_print("sa : %s\n", mac_str);
	vty_print("action : 0x%x\n", value->action);
	vty_print("offset : 0x%x\n", value->offset);
	vty_print("outer_vlanid : 0x%x\n", value->outer_vlanid);
	vty_print("inner_vlanid : 0x%x\n", value->inner_vlanid);
	vty_print("bc_flag : 0x%x\n", value->bc_flag);
	vty_print("rule_mask : 0x%x\n", value->rule_mask);

	vty_print("\n");
}

DEFCMD(fal_rx_prc_lut_t_outer_vlanid_add_func,
	fal_rx_prc_lut_t_outer_vlanid_add_cmd,
	"fal_rx_prc_lut_t outer_vlanid add HEX",
	"fal_rx_prc_lut_t\n outer_vlanid\n add\n outer_vlanid\n")
{
	u32 outer_vlanid = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &outer_vlanid));
	g_fal_rx_prc_lut_t.outer_vlanid = (u16) outer_vlanid;

	return CLI_OK;
}
DEFCMD(fal_rx_prc_lut_t_inner_vlanid_add_func,
	fal_rx_prc_lut_t_inner_vlanid_add_cmd,
	"fal_rx_prc_lut_t inner_vlanid add HEX",
	"fal_rx_prc_lut_t\n inner_vlanid\n add\n inner_vlanid\n")
{
	u32 inner_vlanid = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &inner_vlanid));
	g_fal_rx_prc_lut_t.inner_vlanid = (u16) inner_vlanid;

	return CLI_OK;
}
DEFCMD(fal_rx_prc_lut_t_bc_flag_add_func,
	fal_rx_prc_lut_t_bc_flag_add_cmd,
	"fal_rx_prc_lut_t bc_flag add HEX",
	"fal_rx_prc_lut_t\n bc_flag\n add\n bc_flag\n")
{
	u32 bc_flag = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &bc_flag));
	g_fal_rx_prc_lut_t.bc_flag = (bool) bc_flag;

	return CLI_OK;
}
DEFCMD(fal_rx_prc_lut_t_rule_mask_add_func,
	fal_rx_prc_lut_t_rule_mask_add_cmd,
	"fal_rx_prc_lut_t rule_mask add HEX",
	"fal_rx_prc_lut_t\n rule_mask\n add\n rule_mask\n")
{
	u32 rule_mask = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &rule_mask));
	g_fal_rx_prc_lut_t.rule_mask = rule_mask;

	return CLI_OK;
}

DEFCMD(fal_rx_prc_lut_t_ether_type_mask_add_func,
       fal_rx_prc_lut_t_ether_type_mask_add_cmd,
       "fal_rx_prc_lut_t ether_type_mask add HEX",
       "fal_rx_prc_lut_t\n" "ether_type_mask\n" "add\n" "ether_type_mask\n")
{
	u32 ether_type_mask = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &ether_type_mask));
	g_fal_rx_prc_lut_t.ether_type_mask = (u8) ether_type_mask;

	return CLI_OK;
}

DEFCMD(fal_rx_prc_lut_t_sa_mask_add_func,
       fal_rx_prc_lut_t_sa_mask_add_cmd,
       "fal_rx_prc_lut_t sa_mask add HEX",
       "fal_rx_prc_lut_t\n" "sa_mask\n" "add\n" "sa_mask\n")
{
	u32 sa_mask = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &sa_mask));
	g_fal_rx_prc_lut_t.sa_mask = (u8) sa_mask;

	return CLI_OK;
}

DEFCMD(fal_rx_prc_lut_t_da_mask_add_func,
       fal_rx_prc_lut_t_da_mask_add_cmd,
       "fal_rx_prc_lut_t da_mask add HEX",
       "fal_rx_prc_lut_t\n" "da_mask\n" "add\n" "da_mask\n")
{
	u32 da_mask = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &da_mask));
	g_fal_rx_prc_lut_t.da_mask = (u8) da_mask;

	return CLI_OK;
}

DEFCMD(fal_rx_prc_lut_t_sci_mask_add_func,
       fal_rx_prc_lut_t_sci_mask_add_cmd,
       "fal_rx_prc_lut_t sci_mask add HEX",
       "fal_rx_prc_lut_t\n" "sci_mask\n" "add\n" "sci_mask\n")
{
	u32 sci_mask = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &sci_mask));
	g_fal_rx_prc_lut_t.sci_mask = (u8) sci_mask;

	return CLI_OK;
}

DEFCMD(fal_rx_prc_lut_t_ether_type_add_func,
       fal_rx_prc_lut_t_ether_type_add_cmd,
       "fal_rx_prc_lut_t ether_type add HEX",
       "fal_rx_prc_lut_t\n" "ether_type\n" "add\n" "ether_type\n")
{
	u32 ether_type = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &ether_type));
	g_fal_rx_prc_lut_t.ether_type = (u16) ether_type;

	return CLI_OK;
}

DEFCMD(fal_rx_prc_lut_t_da_add_func,
       fal_rx_prc_lut_t_da_add_cmd,
       "fal_rx_prc_lut_t da add H.H.H",
       "fal_rx_prc_lut_t\n" "da\n" "add\n" "mac\n")
{
	CLI_EXEC_API(cli_str_2_mac(argv[3], g_fal_rx_prc_lut_t.da));
	return CLI_OK;
}

DEFCMD(fal_rx_prc_lut_t_channel_add_func,
       fal_rx_prc_lut_t_channel_add_cmd,
       "fal_rx_prc_lut_t channel add HEX",
       "fal_rx_prc_lut_t\n" "channel\n" "add\n" "channel\n")
{
	u32 channel = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &channel));
	g_fal_rx_prc_lut_t.channel = (u32) channel;

	return CLI_OK;
}

DEFCMD(fal_rx_prc_lut_t_uncontrolled_port_add_func,
       fal_rx_prc_lut_t_uncontrolled_port_add_cmd,
       "fal_rx_prc_lut_t uncontrolled_port add HEX",
       "fal_rx_prc_lut_t\n" "uncontrolled_port\n" "add\n" "uncontrolled_port\n")
{
	u32 uncontrolled_port = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &uncontrolled_port));
	g_fal_rx_prc_lut_t.uncontrolled_port = (bool) uncontrolled_port;

	return CLI_OK;
}

DEFCMD(fal_rx_prc_lut_t_valid_add_func,
       fal_rx_prc_lut_t_valid_add_cmd,
       "fal_rx_prc_lut_t valid add HEX",
       "fal_rx_prc_lut_t\n" "valid\n" "add\n" "valid\n")
{
	u32 valid = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &valid));
	g_fal_rx_prc_lut_t.valid = (bool) valid;

	return CLI_OK;
}

DEFCMD(fal_rx_prc_lut_t_tci_mask_add_func,
       fal_rx_prc_lut_t_tci_mask_add_cmd,
       "fal_rx_prc_lut_t tci_mask add HEX",
       "fal_rx_prc_lut_t\n" "tci_mask\n" "add\n" "tci_mask\n")
{
	u32 tci_mask = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &tci_mask));
	g_fal_rx_prc_lut_t.tci_mask = (u8) tci_mask;

	return CLI_OK;
}

DEFCMD(fal_rx_prc_lut_t_sci_add_func,
       fal_rx_prc_lut_t_sci_add_cmd,
       "fal_rx_prc_lut_t sci add WORD",
       "fal_rx_prc_lut_t\n" "sci\n" "add\n" "mac\n")
{
	CLI_EXEC_API(cli_str_2_sci(argv[3], g_fal_rx_prc_lut_t.sci));
	return CLI_OK;
}

DEFCMD(fal_rx_prc_lut_t_tci_add_func,
       fal_rx_prc_lut_t_tci_add_cmd,
       "fal_rx_prc_lut_t tci add HEX",
       "fal_rx_prc_lut_t\n" "tci\n" "add\n" "tci\n")
{
	u32 tci = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &tci));
	g_fal_rx_prc_lut_t.tci = (u8) tci;

	return CLI_OK;
}

DEFCMD(fal_rx_prc_lut_t_sa_add_func,
       fal_rx_prc_lut_t_sa_add_cmd,
       "fal_rx_prc_lut_t sa add H.H.H",
       "fal_rx_prc_lut_t\n" "sa\n" "add\n" "mac\n")
{
	CLI_EXEC_API(cli_str_2_mac(argv[3], g_fal_rx_prc_lut_t.sa));
	return CLI_OK;
}

DEFCMD(fal_rx_prc_lut_t_action_add_func,
       fal_rx_prc_lut_t_action_add_cmd,
       "fal_rx_prc_lut_t action add HEX",
       "fal_rx_prc_lut_t\n" "action\n" "add\n" "action\n")
{
	u32 action = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &action));
	g_fal_rx_prc_lut_t.action = (fal_rx_prc_lut_action_e) action;

	return CLI_OK;
}

DEFCMD(fal_rx_prc_lut_t_offset_add_func,
       fal_rx_prc_lut_t_offset_add_cmd,
       "fal_rx_prc_lut_t offset add HEX",
       "fal_rx_prc_lut_t\n" "offset\n" "add\n" "offset\n")
{
	u32 offset = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &offset));
	g_fal_rx_prc_lut_t.offset = offset;

	return CLI_OK;
}


DEFCMD(g_fal_tx_ctl_filt_t_init_func,
       g_fal_tx_ctl_filt_t_init_cmd,
       "g_fal_tx_ctl_filt_t init", "g_fal_tx_ctl_filt_t\n" "init\n")
{
	memset(&g_fal_tx_ctl_filt_t, 0, sizeof(g_fal_tx_ctl_filt_t));

	return CLI_OK;
}

void _dump_g_fal_tx_ctl_filt_t(VTY_T *pVty, fal_tx_ctl_filt_t *value)
{
	char mac_str[32] = { 0 };

	vty_print("match_type : 0x%x\n", value->match_type);
	vty_print("match_mask : 0x%x\n", value->match_mask);
	vty_print("bypass : 0x%x\n", value->bypass);
	cli_mac_2_str(value->sa_da_addr, mac_str, sizeof(mac_str));
	vty_print("sa_da_addr : %s\n", mac_str);
	vty_print("ether_type_da_range : 0x%x\n", value->ether_type_da_range);
	vty_print("\n");
}

DEFCMD(fal_tx_ctl_filt_t_match_type_add_func,
       fal_tx_ctl_filt_t_match_type_add_cmd,
       "fal_tx_ctl_filt_t match_type add HEX",
       "fal_tx_ctl_filt_t\n" "match_type\n" "add\n" "match_type\n")
{
	u32 match_type = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &match_type));
	g_fal_tx_ctl_filt_t.match_type = (fal_eg_ctl_match_type_e) match_type;

	return CLI_OK;
}

DEFCMD(fal_tx_ctl_filt_t_match_mask_add_func,
       fal_tx_ctl_filt_t_match_mask_add_cmd,
       "fal_tx_ctl_filt_t match_mask add HEX",
       "fal_tx_ctl_filt_t\n" "match_mask\n" "add\n" "match_mask\n")
{
	u32 match_mask = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &match_mask));
	g_fal_tx_ctl_filt_t.match_mask = (u16) match_mask;

	return CLI_OK;
}

DEFCMD(fal_tx_ctl_filt_t_bypass_add_func,
       fal_tx_ctl_filt_t_bypass_add_cmd,
       "fal_tx_ctl_filt_t bypass add HEX",
       "fal_tx_ctl_filt_t\n" "bypass\n" "add\n" "bypass\n")
{
	u32 bypass = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &bypass));
	g_fal_tx_ctl_filt_t.bypass = (bool) bypass;

	return CLI_OK;
}

DEFCMD(fal_tx_ctl_filt_t_sa_da_addr_add_func,
       fal_tx_ctl_filt_t_sa_da_addr_add_cmd,
       "fal_tx_ctl_filt_t sa_da_addr add H.H.H",
       "fal_tx_ctl_filt_t\n" "sa_da_addr\n" "add\n" "mac\n")
{
	CLI_EXEC_API(cli_str_2_mac(argv[3], g_fal_tx_ctl_filt_t.sa_da_addr));
	return CLI_OK;
}

DEFCMD(fal_tx_ctl_filt_t_ether_type_da_range_add_func,
       fal_tx_ctl_filt_t_ether_type_da_range_add_cmd,
       "fal_tx_ctl_filt_t ether_type_da_range add HEX",
       "fal_tx_ctl_filt_t\n"
       "ether_type_da_range\n" "add\n" "ether_type_da_range\n")
{
	u32 ether_type_da_range = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &ether_type_da_range));
	g_fal_tx_ctl_filt_t.ether_type_da_range = (u16) ether_type_da_range;

	return CLI_OK;
}

DEFCMD(g_fal_tx_mib_t_init_func,
       g_fal_tx_mib_t_init_cmd,
       "g_fal_tx_mib_t init", "g_fal_tx_mib_t\n" "init\n")
{
	memset(&g_fal_tx_mib_t, 0, sizeof(g_fal_tx_mib_t));

	return CLI_OK;
}

void _dump_g_fal_tx_mib_t(VTY_T *pVty, fal_tx_mib_t *value)
{

	vty_print("untagged_pkts : 0x%llx\n", value->untagged_pkts);
	vty_print("unknown_sa_pkts : 0x%llx\n", value->unknown_sa_pkts);
	vty_print("ecc_error_pkts : 0x%llx\n", value->ecc_error_pkts);
	vty_print("too_long : 0x%llx\n", value->too_long);
	vty_print("ctl_pkts : 0x%llx\n", value->ctl_pkts);
	vty_print("unctrl_hit_drop_redir_pkts : 0x%llx\n",
				value->unctrl_hit_drop_redir_pkts);
	vty_print("\n");
}

DEFCMD(fal_tx_mib_t_untagged_pkts_add_func,
       fal_tx_mib_t_untagged_pkts_add_cmd,
       "fal_tx_mib_t untagged_pkts add HEX",
       "fal_tx_mib_t\n" "untagged_pkts\n" "add\n" "untagged_pkts\n")
{
	u64 untagged_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &untagged_pkts));
	g_fal_tx_mib_t.untagged_pkts = (u64) untagged_pkts;

	return CLI_OK;
}

DEFCMD(fal_tx_mib_t_unknown_sa_pkts_add_func,
       fal_tx_mib_t_unknown_sa_pkts_add_cmd,
       "fal_tx_mib_t unknown_sa_pkts add HEX",
       "fal_tx_mib_t\n" "unknown_sa_pkts\n" "add\n" "unknown_sa_pkts\n")
{
	u64 unknown_sa_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &unknown_sa_pkts));
	g_fal_tx_mib_t.unknown_sa_pkts = (u64) unknown_sa_pkts;

	return CLI_OK;
}

DEFCMD(fal_tx_mib_t_ecc_error_pkts_add_func,
       fal_tx_mib_t_ecc_error_pkts_add_cmd,
       "fal_tx_mib_t ecc_error_pkts add HEX",
       "fal_tx_mib_t\n" "ecc_error_pkts\n" "add\n" "ecc_error_pkts\n")
{
	u64 ecc_error_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &ecc_error_pkts));
	g_fal_tx_mib_t.ecc_error_pkts = (u64) ecc_error_pkts;

	return CLI_OK;
}

DEFCMD(fal_tx_mib_t_too_long_add_func,
       fal_tx_mib_t_too_long_add_cmd,
       "fal_tx_mib_t too_long add HEX",
       "fal_tx_mib_t\n" "too_long\n" "add\n" "too_long\n")
{
	u64 too_long = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &too_long));
	g_fal_tx_mib_t.too_long = (u64) too_long;

	return CLI_OK;
}

DEFCMD(fal_tx_mib_t_ctl_pkts_add_func,
       fal_tx_mib_t_ctl_pkts_add_cmd,
       "fal_tx_mib_t ctl_pkts add HEX",
       "fal_tx_mib_t\n" "ctl_pkts\n" "add\n" "ctl_pkts\n")
{
	u64 ctl_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &ctl_pkts));
	g_fal_tx_mib_t.ctl_pkts = (u64) ctl_pkts;

	return CLI_OK;
}

DEFCMD(g_fal_rx_mib_t_init_func,
       g_fal_rx_mib_t_init_cmd,
       "g_fal_rx_mib_t init", "g_fal_rx_mib_t\n" "init\n")
{
	memset(&g_fal_rx_mib_t, 0, sizeof(g_fal_rx_mib_t));

	return CLI_OK;
}

void _dump_g_fal_rx_mib_t(VTY_T *pVty, fal_rx_mib_t *value)
{

	vty_print("ctrl_prt_fail_pkts : 0x%llx\n", value->ctrl_prt_fail_pkts);
	vty_print("too_long_packets : 0x%llx\n", value->too_long_packets);
	vty_print("untagged_pkts : 0x%llx\n", value->untagged_pkts);
	vty_print("no_sci_pkts : 0x%llx\n", value->no_sci_pkts);
	vty_print("unctrl_prt_pass_pkts : 0x%llx\n",
		  value->unctrl_prt_pass_pkts);
	vty_print("untagged_hit_pkts : 0x%llx\n", value->untagged_hit_pkts);
	vty_print("ctrl_prt_pass_pkts : 0x%llx\n", value->ctrl_prt_pass_pkts);
	vty_print("unctrl_prt_fail_pkts : 0x%llx\n",
		  value->unctrl_prt_fail_pkts);
	vty_print("igpoc_ctl_pkts : 0x%llx\n", value->igpoc_ctl_pkts);
	vty_print("unknown_sci_pkts : 0x%llx\n", value->unknown_sci_pkts);
	vty_print("ecc_error_pkts : 0x%llx\n", value->ecc_error_pkts);
	vty_print("notag_pkts : 0x%llx\n", value->notag_pkts);
	vty_print("bad_tag_pkts : 0x%llx\n", value->bad_tag_pkts);
	vty_print("ctl_pkts : 0x%llx\n", value->ctl_pkts);
	vty_print("tagged_miss_pkts : 0x%llx\n", value->tagged_miss_pkts);
	vty_print("unctrl_prt_tx_octets : 0x%llx\n", value->unctrl_prt_tx_octets);

	vty_print("\n");
}

DEFCMD(fal_rx_mib_t_ctrl_prt_fail_pkts_add_func,
       fal_rx_mib_t_ctrl_prt_fail_pkts_add_cmd,
       "fal_rx_mib_t ctrl_prt_fail_pkts add HEX",
       "fal_rx_mib_t\n" "ctrl_prt_fail_pkts\n" "add\n" "ctrl_prt_fail_pkts\n")
{
	u64 ctrl_prt_fail_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &ctrl_prt_fail_pkts));
	g_fal_rx_mib_t.ctrl_prt_fail_pkts = (u64) ctrl_prt_fail_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_mib_t_too_long_packets_add_func,
       fal_rx_mib_t_too_long_packets_add_cmd,
       "fal_rx_mib_t too_long_packets add HEX",
       "fal_rx_mib_t\n" "too_long_packets\n" "add\n" "too_long_packets\n")
{
	u64 too_long_packets = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &too_long_packets));
	g_fal_rx_mib_t.too_long_packets = (u64) too_long_packets;

	return CLI_OK;
}

DEFCMD(fal_rx_mib_t_untagged_pkts_add_func,
       fal_rx_mib_t_untagged_pkts_add_cmd,
       "fal_rx_mib_t untagged_pkts add HEX",
       "fal_rx_mib_t\n" "untagged_pkts\n" "add\n" "untagged_pkts\n")
{
	u64 untagged_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &untagged_pkts));
	g_fal_rx_mib_t.untagged_pkts = (u64) untagged_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_mib_t_no_sci_pkts_add_func,
       fal_rx_mib_t_no_sci_pkts_add_cmd,
       "fal_rx_mib_t no_sci_pkts add HEX",
       "fal_rx_mib_t\n" "no_sci_pkts\n" "add\n" "no_sci_pkts\n")
{
	u64 no_sci_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &no_sci_pkts));
	g_fal_rx_mib_t.no_sci_pkts = (u64) no_sci_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_mib_t_unctrl_prt_pass_pkts_add_func,
       fal_rx_mib_t_unctrl_prt_pass_pkts_add_cmd,
       "fal_rx_mib_t unctrl_prt_pass_pkts add HEX",
       "fal_rx_mib_t\n"
       "unctrl_prt_pass_pkts\n" "add\n" "unctrl_prt_pass_pkts\n")
{
	u64 unctrl_prt_pass_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &unctrl_prt_pass_pkts));
	g_fal_rx_mib_t.unctrl_prt_pass_pkts = (u64) unctrl_prt_pass_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_mib_t_untagged_hit_pkts_add_func,
       fal_rx_mib_t_untagged_hit_pkts_add_cmd,
       "fal_rx_mib_t untagged_hit_pkts add HEX",
       "fal_rx_mib_t\n" "untagged_hit_pkts\n" "add\n" "untagged_hit_pkts\n")
{
	u64 untagged_hit_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &untagged_hit_pkts));
	g_fal_rx_mib_t.untagged_hit_pkts = (u64) untagged_hit_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_mib_t_ctrl_prt_pass_pkts_add_func,
       fal_rx_mib_t_ctrl_prt_pass_pkts_add_cmd,
       "fal_rx_mib_t ctrl_prt_pass_pkts add HEX",
       "fal_rx_mib_t\n" "ctrl_prt_pass_pkts\n" "add\n" "ctrl_prt_pass_pkts\n")
{
	u64 ctrl_prt_pass_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &ctrl_prt_pass_pkts));
	g_fal_rx_mib_t.ctrl_prt_pass_pkts = (u64) ctrl_prt_pass_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_mib_t_unctrl_prt_fail_pkts_add_func,
       fal_rx_mib_t_unctrl_prt_fail_pkts_add_cmd,
       "fal_rx_mib_t unctrl_prt_fail_pkts add HEX",
       "fal_rx_mib_t\n"
       "unctrl_prt_fail_pkts\n" "add\n" "unctrl_prt_fail_pkts\n")
{
	u64 unctrl_prt_fail_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &unctrl_prt_fail_pkts));
	g_fal_rx_mib_t.unctrl_prt_fail_pkts = (u64) unctrl_prt_fail_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_mib_t_igpoc_ctl_pkts_add_func,
       fal_rx_mib_t_igpoc_ctl_pkts_add_cmd,
       "fal_rx_mib_t igpoc_ctl_pkts add HEX",
       "fal_rx_mib_t\n" "igpoc_ctl_pkts\n" "add\n" "igpoc_ctl_pkts\n")
{
	u64 igpoc_ctl_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &igpoc_ctl_pkts));
	g_fal_rx_mib_t.igpoc_ctl_pkts = (u64) igpoc_ctl_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_mib_t_unknown_sci_pkts_add_func,
       fal_rx_mib_t_unknown_sci_pkts_add_cmd,
       "fal_rx_mib_t unknown_sci_pkts add HEX",
       "fal_rx_mib_t\n" "unknown_sci_pkts\n" "add\n" "unknown_sci_pkts\n")
{
	u64 unknown_sci_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &unknown_sci_pkts));
	g_fal_rx_mib_t.unknown_sci_pkts = (u64) unknown_sci_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_mib_t_ecc_error_pkts_add_func,
       fal_rx_mib_t_ecc_error_pkts_add_cmd,
       "fal_rx_mib_t ecc_error_pkts add HEX",
       "fal_rx_mib_t\n" "ecc_error_pkts\n" "add\n" "ecc_error_pkts\n")
{
	u64 ecc_error_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &ecc_error_pkts));
	g_fal_rx_mib_t.ecc_error_pkts = (u64) ecc_error_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_mib_t_notag_pkts_add_func,
       fal_rx_mib_t_notag_pkts_add_cmd,
       "fal_rx_mib_t notag_pkts add HEX",
       "fal_rx_mib_t\n" "notag_pkts\n" "add\n" "notag_pkts\n")
{
	u64 notag_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &notag_pkts));
	g_fal_rx_mib_t.notag_pkts = (u64) notag_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_mib_t_bad_tag_pkts_add_func,
       fal_rx_mib_t_bad_tag_pkts_add_cmd,
       "fal_rx_mib_t bad_tag_pkts add HEX",
       "fal_rx_mib_t\n" "bad_tag_pkts\n" "add\n" "bad_tag_pkts\n")
{
	u64 bad_tag_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &bad_tag_pkts));
	g_fal_rx_mib_t.bad_tag_pkts = (u64) bad_tag_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_mib_t_ctl_pkts_add_func,
       fal_rx_mib_t_ctl_pkts_add_cmd,
       "fal_rx_mib_t ctl_pkts add HEX",
       "fal_rx_mib_t\n" "ctl_pkts\n" "add\n" "ctl_pkts\n")
{
	u64 ctl_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &ctl_pkts));
	g_fal_rx_mib_t.ctl_pkts = (u64) ctl_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_mib_t_tagged_miss_pkts_add_func,
       fal_rx_mib_t_tagged_miss_pkts_add_cmd,
       "fal_rx_mib_t tagged_miss_pkts add HEX",
       "fal_rx_mib_t\n" "tagged_miss_pkts\n" "add\n" "tagged_miss_pkts\n")
{
	u64 tagged_miss_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &tagged_miss_pkts));
	g_fal_rx_mib_t.tagged_miss_pkts = (u64) tagged_miss_pkts;

	return CLI_OK;
}

DEFCMD(g_fal_rx_sa_mib_t_init_func,
       g_fal_rx_sa_mib_t_init_cmd,
       "g_fal_rx_sa_mib_t init", "g_fal_rx_sa_mib_t\n" "init\n")
{
	memset(&g_fal_rx_sa_mib_t, 0, sizeof(g_fal_rx_sa_mib_t));

	return CLI_OK;
}

void _dump_g_fal_rx_sa_mib_t(VTY_T *pVty, fal_rx_sa_mib_t *value)
{

	vty_print("ok_pkts : 0x%llx\n", value->ok_pkts);
	vty_print("unused_sa : 0x%llx\n", value->unused_sa);
	vty_print("late_pkts : 0x%llx\n", value->late_pkts);
	vty_print("decrypted_octets : 0x%llx\n", value->decrypted_octets);
	vty_print("delayed_pkts : 0x%llx\n", value->delayed_pkts);
	vty_print("invalid_pkts : 0x%llx\n", value->invalid_pkts);
	vty_print("validated_octets : 0x%llx\n", value->validated_octets);
	vty_print("untagged_hit_pkts : 0x%llx\n", value->untagged_hit_pkts);
	vty_print("hit_drop_redir_pkts : 0x%llx\n", value->hit_drop_redir_pkts);
	vty_print("not_using_sa : 0x%llx\n", value->not_using_sa);
	vty_print("unchecked_pkts : 0x%llx\n", value->unchecked_pkts);
	vty_print("not_valid_pkts : 0x%llx\n", value->not_valid_pkts);
	vty_print("\n");
}

DEFCMD(fal_rx_sa_mib_t_ok_pkts_add_func,
       fal_rx_sa_mib_t_ok_pkts_add_cmd,
       "fal_rx_sa_mib_t ok_pkts add HEX",
       "fal_rx_sa_mib_t\n" "ok_pkts\n" "add\n" "ok_pkts\n")
{
	u64 ok_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &ok_pkts));
	g_fal_rx_sa_mib_t.ok_pkts = (u64) ok_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_sa_mib_t_unused_sa_add_func,
       fal_rx_sa_mib_t_unused_sa_add_cmd,
       "fal_rx_sa_mib_t unused_sa add HEX",
       "fal_rx_sa_mib_t\n" "unused_sa\n" "add\n" "unused_sa\n")
{
	u64 unused_sa = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &unused_sa));
	g_fal_rx_sa_mib_t.unused_sa = (u64) unused_sa;

	return CLI_OK;
}

DEFCMD(fal_rx_sa_mib_t_late_pkts_add_func,
       fal_rx_sa_mib_t_late_pkts_add_cmd,
       "fal_rx_sa_mib_t late_pkts add HEX",
       "fal_rx_sa_mib_t\n" "late_pkts\n" "add\n" "late_pkts\n")
{
	u64 late_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &late_pkts));
	g_fal_rx_sa_mib_t.late_pkts = (u64) late_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_sa_mib_t_decrypted_octets_add_func,
       fal_rx_sa_mib_t_decrypted_octets_add_cmd,
       "fal_rx_sa_mib_t decrypted_octets add HEX",
       "fal_rx_sa_mib_t\n" "decrypted_octets\n" "add\n" "decrypted_octets\n")
{
	u64 decrypted_octets = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &decrypted_octets));
	g_fal_rx_sa_mib_t.decrypted_octets = (u64) decrypted_octets;

	return CLI_OK;
}

DEFCMD(fal_rx_sa_mib_t_delayed_pkts_add_func,
       fal_rx_sa_mib_t_delayed_pkts_add_cmd,
       "fal_rx_sa_mib_t delayed_pkts add HEX",
       "fal_rx_sa_mib_t\n" "delayed_pkts\n" "add\n" "delayed_pkts\n")
{
	u64 delayed_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &delayed_pkts));
	g_fal_rx_sa_mib_t.delayed_pkts = (u64) delayed_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_sa_mib_t_invalid_pkts_add_func,
       fal_rx_sa_mib_t_invalid_pkts_add_cmd,
       "fal_rx_sa_mib_t invalid_pkts add HEX",
       "fal_rx_sa_mib_t\n" "invalid_pkts\n" "add\n" "invalid_pkts\n")
{
	u64 invalid_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &invalid_pkts));
	g_fal_rx_sa_mib_t.invalid_pkts = (u64) invalid_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_sa_mib_t_validated_octets_add_func,
       fal_rx_sa_mib_t_validated_octets_add_cmd,
       "fal_rx_sa_mib_t validated_octets add HEX",
       "fal_rx_sa_mib_t\n" "validated_octets\n" "add\n" "validated_octets\n")
{
	u64 validated_octets = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &validated_octets));
	g_fal_rx_sa_mib_t.validated_octets = (u64) validated_octets;

	return CLI_OK;
}

DEFCMD(fal_rx_sa_mib_t_untagged_hit_pkts_add_func,
       fal_rx_sa_mib_t_untagged_hit_pkts_add_cmd,
       "fal_rx_sa_mib_t untagged_hit_pkts add HEX",
       "fal_rx_sa_mib_t\n" "untagged_hit_pkts\n" "add\n" "untagged_hit_pkts\n")
{
	u64 untagged_hit_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &untagged_hit_pkts));
	g_fal_rx_sa_mib_t.untagged_hit_pkts = (u64) untagged_hit_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_sa_mib_t_hit_drop_redir_pkts_add_func,
       fal_rx_sa_mib_t_hit_drop_redir_pkts_add_cmd,
       "fal_rx_sa_mib_t hit_drop_redir_pkts add HEX",
       "fal_rx_sa_mib_t\n"
       "hit_drop_redir_pkts\n" "add\n" "hit_drop_redir_pkts\n")
{
	u64 hit_drop_redir_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &hit_drop_redir_pkts));
	g_fal_rx_sa_mib_t.hit_drop_redir_pkts = (u64) hit_drop_redir_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_sa_mib_t_not_using_sa_add_func,
       fal_rx_sa_mib_t_not_using_sa_add_cmd,
       "fal_rx_sa_mib_t not_using_sa add HEX",
       "fal_rx_sa_mib_t\n" "not_using_sa\n" "add\n" "not_using_sa\n")
{
	u64 not_using_sa = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &not_using_sa));
	g_fal_rx_sa_mib_t.not_using_sa = (u64) not_using_sa;

	return CLI_OK;
}

DEFCMD(fal_rx_sa_mib_t_unchecked_pkts_add_func,
       fal_rx_sa_mib_t_unchecked_pkts_add_cmd,
       "fal_rx_sa_mib_t unchecked_pkts add HEX",
       "fal_rx_sa_mib_t\n" "unchecked_pkts\n" "add\n" "unchecked_pkts\n")
{
	u64 unchecked_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &unchecked_pkts));
	g_fal_rx_sa_mib_t.unchecked_pkts = (u64) unchecked_pkts;

	return CLI_OK;
}

DEFCMD(fal_rx_sa_mib_t_not_valid_pkts_add_func,
       fal_rx_sa_mib_t_not_valid_pkts_add_cmd,
       "fal_rx_sa_mib_t not_valid_pkts add HEX",
       "fal_rx_sa_mib_t\n" "not_valid_pkts\n" "add\n" "not_valid_pkts\n")
{
	u64 not_valid_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &not_valid_pkts));
	g_fal_rx_sa_mib_t.not_valid_pkts = (u64) not_valid_pkts;

	return CLI_OK;
}

DEFCMD(g_fal_rx_sak_t_init_func,
       g_fal_rx_sak_t_init_cmd,
       "g_fal_rx_sak_t init", "g_fal_rx_sak_t\n" "init\n")
{
	memset(&g_fal_rx_sak_t, 0, sizeof(g_fal_rx_sak_t));

	return CLI_OK;
}

DEFCMD(fal_rx_sak_t_key_len_add_func,
	fal_rx_sak_t_key_len_add_cmd,
	"fal_rx_sak_t sak_len add HEX",
	"fal_rx_sak_t\n sak_len\n add\n sak_len\n")
{
	u32 len = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &len));
	g_fal_rx_sak_t.sak_len = len;

	return CLI_OK;
}
void _dump_g_fal_rx_sak_t(VTY_T *pVty, fal_rx_sak_t *value)
{
	char str[64] = { 0 };

	cli_sak_2_str(value->sak, str, 63);
	vty_print("sak : %s\n", str);
	vty_print("\n");
}

void _dump_g_fal_rx_sak1_t(VTY_T *pVty, fal_rx_sak_t *value)
{
	char str[64] = { 0 };

	cli_sak_2_str(value->sak1, str, 63);
	vty_print("sak1 : %s\n", str);
	vty_print("\n");
}

DEFCMD(fal_rx_sak_t_sak_add_func,
       fal_rx_sak_t_sak_add_cmd,
       "fal_rx_sak_t sak add WORD", "fal_rx_sak_t\n" "sak\n" "add\n" "sak\n")
{
	CLI_EXEC_API(cli_str_2_sak(argv[3], g_fal_rx_sak_t.sak));
	return CLI_OK;
}
DEFCMD(fal_rx_sak_t_sak1_add_func,
	fal_rx_sak_t_sak1_add_cmd,
	"fal_rx_sak_t sak1 add WORD", "fal_rx_sak_t\n sak1\n add\n sak\n")
{
	CLI_EXEC_API(cli_str_2_sak(argv[3], g_fal_rx_sak_t.sak1));
	return CLI_OK;
}

DEFCMD(g_fal_tx_class_lut_t_init_func,
       g_fal_tx_class_lut_t_init_cmd,
       "g_fal_tx_class_lut_t init", "g_fal_tx_class_lut_t\n" "init\n")
{
	memset(&g_fal_tx_class_lut_t, 0, sizeof(g_fal_tx_class_lut_t));

	return CLI_OK;
}

void _dump_g_fal_tx_class_lut_t(VTY_T *pVty, fal_tx_class_lut_t *value)
{
	char mac_str[32] = { 0 };

	vty_print("sa_mask : 0x%x\n", value->sa_mask);
	vty_print("vlan_up_mask : 0x%x\n", value->vlan_up_mask);
	vty_print("vlan_id_mask : 0x%x\n", value->vlan_id_mask);
	vty_print("udf1_valid : 0x%x\n", value->udf1_valid);
	vty_print("ether_type : 0x%x\n", value->ether_type);
	cli_mac_2_str(value->da, mac_str, sizeof(mac_str));
	vty_print("da : %s\n", mac_str);
	vty_print("udf0_valid : 0x%x\n", value->udf0_valid);
	vty_print("channel : 0x%x\n", value->channel);
	vty_print("udf3_byte : 0x%x\n", value->udf3_byte);
	vty_print("valid : 0x%x\n", value->valid);
	vty_print("vlan_valid : 0x%x\n", value->vlan_valid);
	vty_print("udf1_location : 0x%x\n", value->udf1_location);
	cli_mac_2_str(value->sa, mac_str, sizeof(mac_str));
	vty_print("sa : %s\n", mac_str);
	vty_print("vlan_valid_mask : 0x%x\n", value->vlan_valid_mask);
	vty_print("udf2_location : 0x%x\n", value->udf2_location);
	vty_print("da_mask : 0x%x\n", value->da_mask);
	vty_print("ether_type_mask : 0x%x\n", value->ether_type_mask);
	vty_print("udf0_byte : 0x%x\n", value->udf0_byte);
	vty_print("udf0_location : 0x%x\n", value->udf0_location);
	vty_print("udf2_valid : 0x%x\n", value->udf2_valid);
	vty_print("udf3_valid : 0x%x\n", value->udf3_valid);
	vty_print("vlan_id : 0x%x\n", value->vlan_id);
	vty_print("udf2_byte : 0x%x\n", value->udf2_byte);
	vty_print("action : 0x%x\n", value->action);
	vty_print("udf3_location : 0x%x\n", value->udf3_location);
	vty_print("vlan_up : 0x%x\n", value->vlan_up);
	vty_print("udf1_byte : 0x%x\n", value->udf1_byte);

	cli_sci_2_str(value->sci, mac_str, sizeof(mac_str));
	vty_print("sci : %s\n", mac_str);
	vty_print("tci : 0x%x\n", value->tci);
	vty_print("offset : 0x%x\n", value->offset);
	vty_print("outervlan_id : 0x%x\n", value->outer_vlanid);
	vty_print("bc_flag : 0x%x\n", value->bc_flag);
	vty_print("rule_mask : 0x%x\n", value->rule_mask);

	vty_print("\n");
}

DEFCMD(fal_tx_class_lut_t_sa_mask_add_func,
       fal_tx_class_lut_t_sa_mask_add_cmd,
       "fal_tx_class_lut_t sa_mask add HEX",
       "fal_tx_class_lut_t\n" "sa_mask\n" "add\n" "sa_mask\n")
{
	u32 sa_mask = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &sa_mask));
	g_fal_tx_class_lut_t.sa_mask = (u8) sa_mask;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_vlan_up_mask_add_func,
       fal_tx_class_lut_t_vlan_up_mask_add_cmd,
       "fal_tx_class_lut_t vlan_up_mask add HEX",
       "fal_tx_class_lut_t\n" "vlan_up_mask\n" "add\n" "vlan_up_mask\n")
{
	u32 vlan_up_mask = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &vlan_up_mask));
	g_fal_tx_class_lut_t.vlan_up_mask = (u8) vlan_up_mask;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_vlan_id_mask_add_func,
       fal_tx_class_lut_t_vlan_id_mask_add_cmd,
       "fal_tx_class_lut_t vlan_id_mask add HEX",
       "fal_tx_class_lut_t\n" "vlan_id_mask\n" "add\n" "vlan_id_mask\n")
{
	u32 vlan_id_mask = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &vlan_id_mask));
	g_fal_tx_class_lut_t.vlan_id_mask = (u8) vlan_id_mask;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_rule_mask_add_func,
	fal_tx_class_lut_t_rule_mask_add_cmd,
	"fal_tx_class_lut_t rule_mask add HEX",
	"fal_tx_class_lut_t\n rule_mask\n add\n rule_mask\n")
{
	u32 rule_mask = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &rule_mask));
	g_fal_tx_class_lut_t.rule_mask = rule_mask;

	return CLI_OK;
}
DEFCMD(fal_tx_class_lut_t_outervlan_id_add_func,
	fal_tx_class_lut_t_outervlan_id_add_cmd,
	"fal_tx_class_lut_t outervlan_id add HEX",
	"fal_tx_class_lut_t\n outervlan_id\n add\n outervlan_id\n")
{
	u32 outervlan_id = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &outervlan_id));
	g_fal_tx_class_lut_t.outer_vlanid = (u16) outervlan_id;

	return CLI_OK;
}
DEFCMD(fal_tx_class_lut_t_offset_add_func,
	fal_tx_class_lut_t_offset_add_cmd,
	"fal_tx_class_lut_t offset add HEX",
	"fal_tx_class_lut_t\n offset\n add\n offset\n")
{
	u32 offset = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &offset));
	g_fal_tx_class_lut_t.offset = (u8) offset;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_tcl_add_func,
	fal_tx_class_lut_t_tci_add_cmd,
	"fal_tx_class_lut_t tci add HEX",
	"fal_tx_class_lut_t\n tci\n add\n tci\n")
{
	u32 tci = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &tci));
	g_fal_tx_class_lut_t.tci = (u8) tci;

	return CLI_OK;
}
DEFCMD(fal_tx_class_lut_t_bc_flag_add_func,
	fal_tx_class_lut_t_bc_flag_add_cmd,
	"fal_tx_class_lut_t bc_flag add HEX",
	"fal_tx_class_lut_t\n bc_flag\n add\n bc_flag\n")
{
	u32 bc_flag = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &bc_flag));
	g_fal_tx_class_lut_t.bc_flag = (bool) bc_flag;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_sci_add_func,
	fal_tx_class_lut_t_sci_add_cmd,
	"fal_tx_class_lut_t sci add WORD",
	"fal_tx_class_lut_t\n sci\n add\n sci\n")
{
	CLI_EXEC_API(cli_str_2_sci(argv[3], g_fal_tx_class_lut_t.sci));
	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_udf1_valid_add_func,
       fal_tx_class_lut_t_udf1_valid_add_cmd,
       "fal_tx_class_lut_t udf1_valid add HEX",
       "fal_tx_class_lut_t\n" "udf1_valid\n" "add\n" "udf1_valid\n")
{
	u32 udf1_valid = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &udf1_valid));
	g_fal_tx_class_lut_t.udf1_valid = (bool) udf1_valid;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_ether_type_add_func,
       fal_tx_class_lut_t_ether_type_add_cmd,
       "fal_tx_class_lut_t ether_type add HEX",
       "fal_tx_class_lut_t\n" "ether_type\n" "add\n" "ether_type\n")
{
	u32 ether_type = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &ether_type));
	g_fal_tx_class_lut_t.ether_type = (u16) ether_type;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_da_add_func,
       fal_tx_class_lut_t_da_add_cmd,
       "fal_tx_class_lut_t da add H.H.H",
       "fal_tx_class_lut_t\n" "da\n" "add\n" "mac\n")
{
	CLI_EXEC_API(cli_str_2_mac(argv[3], g_fal_tx_class_lut_t.da));
	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_udf0_valid_add_func,
       fal_tx_class_lut_t_udf0_valid_add_cmd,
       "fal_tx_class_lut_t udf0_valid add HEX",
       "fal_tx_class_lut_t\n" "udf0_valid\n" "add\n" "udf0_valid\n")
{
	u32 udf0_valid = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &udf0_valid));
	g_fal_tx_class_lut_t.udf0_valid = (bool) udf0_valid;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_channel_add_func,
       fal_tx_class_lut_t_channel_add_cmd,
       "fal_tx_class_lut_t channel add HEX",
       "fal_tx_class_lut_t\n" "channel\n" "add\n" "channel\n")
{
	u32 channel = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &channel));
	g_fal_tx_class_lut_t.channel = (u32) channel;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_udf3_byte_add_func,
       fal_tx_class_lut_t_udf3_byte_add_cmd,
       "fal_tx_class_lut_t udf3_byte add HEX",
       "fal_tx_class_lut_t\n" "udf3_byte\n" "add\n" "udf3_byte\n")
{
	u32 udf3_byte = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &udf3_byte));
	g_fal_tx_class_lut_t.udf3_byte = (u8) udf3_byte;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_valid_add_func,
       fal_tx_class_lut_t_valid_add_cmd,
       "fal_tx_class_lut_t valid add HEX",
       "fal_tx_class_lut_t\n" "valid\n" "add\n" "valid\n")
{
	u32 valid = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &valid));
	g_fal_tx_class_lut_t.valid = (bool) valid;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_vlan_valid_add_func,
       fal_tx_class_lut_t_vlan_valid_add_cmd,
       "fal_tx_class_lut_t vlan_valid add HEX",
       "fal_tx_class_lut_t\n" "vlan_valid\n" "add\n" "vlan_valid\n")
{
	u32 vlan_valid = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &vlan_valid));
	g_fal_tx_class_lut_t.vlan_valid = (bool) vlan_valid;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_udf1_location_add_func,
       fal_tx_class_lut_t_udf1_location_add_cmd,
       "fal_tx_class_lut_t udf1_location add HEX",
       "fal_tx_class_lut_t\n" "udf1_location\n" "add\n" "udf1_location\n")
{
	u32 udf1_location = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &udf1_location));
	g_fal_tx_class_lut_t.udf1_location = (u8) udf1_location;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_sa_add_func,
       fal_tx_class_lut_t_sa_add_cmd,
       "fal_tx_class_lut_t sa add H.H.H",
       "fal_tx_class_lut_t\n" "sa\n" "add\n" "mac\n")
{
	CLI_EXEC_API(cli_str_2_mac(argv[3], g_fal_tx_class_lut_t.sa));
	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_vlan_valid_mask_add_func,
       fal_tx_class_lut_t_vlan_valid_mask_add_cmd,
       "fal_tx_class_lut_t vlan_valid_mask add HEX",
       "fal_tx_class_lut_t\n" "vlan_valid_mask\n" "add\n" "vlan_valid_mask\n")
{
	u32 vlan_valid_mask = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &vlan_valid_mask));
	g_fal_tx_class_lut_t.vlan_valid_mask = (u8) vlan_valid_mask;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_udf2_location_add_func,
       fal_tx_class_lut_t_udf2_location_add_cmd,
       "fal_tx_class_lut_t udf2_location add HEX",
       "fal_tx_class_lut_t\n" "udf2_location\n" "add\n" "udf2_location\n")
{
	u32 udf2_location = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &udf2_location));
	g_fal_tx_class_lut_t.udf2_location = (u8) udf2_location;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_da_mask_add_func,
       fal_tx_class_lut_t_da_mask_add_cmd,
       "fal_tx_class_lut_t da_mask add HEX",
       "fal_tx_class_lut_t\n" "da_mask\n" "add\n" "da_mask\n")
{
	u32 da_mask = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &da_mask));
	g_fal_tx_class_lut_t.da_mask = (u8) da_mask;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_ether_type_mask_add_func,
       fal_tx_class_lut_t_ether_type_mask_add_cmd,
       "fal_tx_class_lut_t ether_type_mask add HEX",
       "fal_tx_class_lut_t\n" "ether_type_mask\n" "add\n" "ether_type_mask\n")
{
	u32 ether_type_mask = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &ether_type_mask));
	g_fal_tx_class_lut_t.ether_type_mask = (u8) ether_type_mask;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_udf0_byte_add_func,
       fal_tx_class_lut_t_udf0_byte_add_cmd,
       "fal_tx_class_lut_t udf0_byte add HEX",
       "fal_tx_class_lut_t\n" "udf0_byte\n" "add\n" "udf0_byte\n")
{
	u32 udf0_byte = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &udf0_byte));
	g_fal_tx_class_lut_t.udf0_byte = (u8) udf0_byte;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_udf0_location_add_func,
       fal_tx_class_lut_t_udf0_location_add_cmd,
       "fal_tx_class_lut_t udf0_location add HEX",
       "fal_tx_class_lut_t\n" "udf0_location\n" "add\n" "udf0_location\n")
{
	u32 udf0_location = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &udf0_location));
	g_fal_tx_class_lut_t.udf0_location = (u8) udf0_location;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_udf2_valid_add_func,
       fal_tx_class_lut_t_udf2_valid_add_cmd,
       "fal_tx_class_lut_t udf2_valid add HEX",
       "fal_tx_class_lut_t\n" "udf2_valid\n" "add\n" "udf2_valid\n")
{
	u32 udf2_valid = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &udf2_valid));
	g_fal_tx_class_lut_t.udf2_valid = (bool) udf2_valid;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_udf3_valid_add_func,
       fal_tx_class_lut_t_udf3_valid_add_cmd,
       "fal_tx_class_lut_t udf3_valid add HEX",
       "fal_tx_class_lut_t\n" "udf3_valid\n" "add\n" "udf3_valid\n")
{
	u32 udf3_valid = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &udf3_valid));
	g_fal_tx_class_lut_t.udf3_valid = (bool) udf3_valid;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_vlan_id_add_func,
       fal_tx_class_lut_t_vlan_id_add_cmd,
       "fal_tx_class_lut_t vlan_id add HEX",
       "fal_tx_class_lut_t\n" "vlan_id\n" "add\n" "vlan_id\n")
{
	u32 vlan_id = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &vlan_id));
	g_fal_tx_class_lut_t.vlan_id = (u16) vlan_id;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_udf2_byte_add_func,
       fal_tx_class_lut_t_udf2_byte_add_cmd,
       "fal_tx_class_lut_t udf2_byte add HEX",
       "fal_tx_class_lut_t\n" "udf2_byte\n" "add\n" "udf2_byte\n")
{
	u32 udf2_byte = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &udf2_byte));
	g_fal_tx_class_lut_t.udf2_byte = (u8) udf2_byte;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_action_add_func,
       fal_tx_class_lut_t_action_add_cmd,
       "fal_tx_class_lut_t action add HEX",
       "fal_tx_class_lut_t\n" "action\n" "add\n" "action\n")
{
	u32 action = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &action));
	g_fal_tx_class_lut_t.action = (fal_tx_class_lut_action_e) action;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_udf3_location_add_func,
       fal_tx_class_lut_t_udf3_location_add_cmd,
       "fal_tx_class_lut_t udf3_location add HEX",
       "fal_tx_class_lut_t\n" "udf3_location\n" "add\n" "udf3_location\n")
{
	u32 udf3_location = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &udf3_location));
	g_fal_tx_class_lut_t.udf3_location = (u8) udf3_location;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_vlan_up_add_func,
       fal_tx_class_lut_t_vlan_up_add_cmd,
       "fal_tx_class_lut_t vlan_up add HEX",
       "fal_tx_class_lut_t\n" "vlan_up\n" "add\n" "vlan_up\n")
{
	u32 vlan_up = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &vlan_up));
	g_fal_tx_class_lut_t.vlan_up = (u8) vlan_up;

	return CLI_OK;
}

DEFCMD(fal_tx_class_lut_t_udf1_byte_add_func,
       fal_tx_class_lut_t_udf1_byte_add_cmd,
       "fal_tx_class_lut_t udf1_byte add HEX",
       "fal_tx_class_lut_t\n" "udf1_byte\n" "add\n" "udf1_byte\n")
{
	u32 udf1_byte = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &udf1_byte));
	g_fal_tx_class_lut_t.udf1_byte = (u8) udf1_byte;

	return CLI_OK;
}

DEFCMD(g_fal_tx_sa_mib_t_init_func,
       g_fal_tx_sa_mib_t_init_cmd,
       "g_fal_tx_sa_mib_t init", "g_fal_tx_sa_mib_t\n" "init\n")
{
	memset(&g_fal_tx_sa_mib_t, 0, sizeof(g_fal_tx_sa_mib_t));

	return CLI_OK;
}

void _dump_g_fal_tx_sa_mib_t(VTY_T *pVty, fal_tx_sa_mib_t *value)
{

	vty_print("encrypted_pkts : 0x%llx\n", value->encrypted_pkts);
	vty_print("protected_pkts : 0x%llx\n", value->protected_pkts);
	vty_print("protected2_pkts : 0x%llx\n", value->protected2_pkts);
	vty_print("hit_drop_redirect : 0x%llx\n", value->hit_drop_redirect);
	vty_print("\n");
}

DEFCMD(fal_tx_sa_mib_t_encrypted_pkts_add_func,
       fal_tx_sa_mib_t_encrypted_pkts_add_cmd,
       "fal_tx_sa_mib_t encrypted_pkts add HEX",
       "fal_tx_sa_mib_t\n" "encrypted_pkts\n" "add\n" "encrypted_pkts\n")
{
	u32 encrypted_pkts = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &encrypted_pkts));
	g_fal_tx_sa_mib_t.encrypted_pkts = (u32) encrypted_pkts;

	return CLI_OK;
}

DEFCMD(fal_tx_sa_mib_t_protected_pkts_add_func,
       fal_tx_sa_mib_t_protected_pkts_add_cmd,
       "fal_tx_sa_mib_t protected_pkts add HEX",
       "fal_tx_sa_mib_t\n" "protected_pkts\n" "add\n" "protected_pkts\n")
{
	u32 protected_pkts = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &protected_pkts));
	g_fal_tx_sa_mib_t.protected_pkts = (u32) protected_pkts;

	return CLI_OK;
}

DEFCMD(fal_tx_sa_mib_t_protected2_pkts_add_func,
       fal_tx_sa_mib_t_protected2_pkts_add_cmd,
       "fal_tx_sa_mib_t protected2_pkts add HEX",
       "fal_tx_sa_mib_t\n" "protected2_pkts\n" "add\n" "protected2_pkts\n")
{
	u32 protected2_pkts = 0;
	CLI_EXEC_API(cli_str_2_hex(argv[3], &protected2_pkts));
	g_fal_tx_sa_mib_t.protected2_pkts = (u32) protected2_pkts;

	return CLI_OK;
}

DEFCMD(fal_tx_sa_mib_t_hit_drop_redirect_add_func,
       fal_tx_sa_mib_t_hit_drop_redirect_add_cmd,
       "fal_tx_sa_mib_t hit_drop_redirect add HEX",
       "fal_tx_sa_mib_t\n" "hit_drop_redirect\n" "add\n" "hit_drop_redirect\n")
{
	u64 hit_drop_redirect = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &hit_drop_redirect));
	g_fal_tx_sa_mib_t.hit_drop_redirect = (u64) hit_drop_redirect;

	return CLI_OK;
}

DEFCMD(g_fal_tx_sc_mib_t_init_func,
       g_fal_tx_sc_mib_t_init_cmd,
       "g_fal_tx_sc_mib_t init", "g_fal_tx_sc_mib_t\n" "init\n")
{
	memset(&g_fal_tx_sc_mib_t, 0, sizeof(g_fal_tx_sc_mib_t));

	return CLI_OK;
}

void _dump_g_fal_tx_sc_mib_t(VTY_T *pVty, fal_tx_sc_mib_t *value)
{

	vty_print("protected_octets : 0x%llx\n", value->protected_octets);
	vty_print("encrypted_pkts : 0x%llx\n", value->encrypted_pkts);
	vty_print("protected_pkts : 0x%llx\n", value->protected_pkts);
	vty_print("encrypted_octets : 0x%llx\n", value->encrypted_octets);
	vty_print("\n");
}

DEFCMD(fal_tx_sc_mib_t_protected_octets_add_func,
       fal_tx_sc_mib_t_protected_octets_add_cmd,
       "fal_tx_sc_mib_t protected_octets add HEX",
       "fal_tx_sc_mib_t\n" "protected_octets\n" "add\n" "protected_octets\n")
{
	u64 protected_octets = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &protected_octets));
	g_fal_tx_sc_mib_t.protected_octets = (u64) protected_octets;

	return CLI_OK;
}

DEFCMD(fal_tx_sc_mib_t_encrypted_pkts_add_func,
       fal_tx_sc_mib_t_encrypted_pkts_add_cmd,
       "fal_tx_sc_mib_t encrypted_pkts add HEX",
       "fal_tx_sc_mib_t\n" "encrypted_pkts\n" "add\n" "encrypted_pkts\n")
{
	u64 encrypted_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &encrypted_pkts));
	g_fal_tx_sc_mib_t.encrypted_pkts = (u64) encrypted_pkts;

	return CLI_OK;
}

DEFCMD(fal_tx_sc_mib_t_protected_pkts_add_func,
       fal_tx_sc_mib_t_protected_pkts_add_cmd,
       "fal_tx_sc_mib_t protected_pkts add HEX",
       "fal_tx_sc_mib_t\n" "protected_pkts\n" "add\n" "protected_pkts\n")
{
	u64 protected_pkts = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &protected_pkts));
	g_fal_tx_sc_mib_t.protected_pkts = (u64) protected_pkts;

	return CLI_OK;
}

DEFCMD(fal_tx_sc_mib_t_encrypted_octets_add_func,
       fal_tx_sc_mib_t_encrypted_octets_add_cmd,
       "fal_tx_sc_mib_t encrypted_octets add HEX",
       "fal_tx_sc_mib_t\n" "encrypted_octets\n" "add\n" "encrypted_octets\n")
{
	u64 encrypted_octets = 0;
	CLI_EXEC_API(cli_str_2_long_hex(argv[3], &encrypted_octets));
	g_fal_tx_sc_mib_t.encrypted_octets = (u64) encrypted_octets;

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_ext_reg_get_func,
       nss_macsec_secy_ext_reg_get_cmd,
       "nss_macsec_secy_extend_reg get HEX HEX",
       "nss_macsec_secy_extend_reg\n" "get\n" "secy_id\n" "addr\n")
{
	u32 secy_id = 0;
	u32 addr = 0;
	u32 pvalue = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&addr));

	CLI_EXEC_API(nss_macsec_secy_ext_reg_get(secy_id, addr, &pvalue));
	vty_print("pvalue : 0x%x\n", pvalue);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_ext_reg_set_func,
       nss_macsec_secy_ext_reg_set_cmd,
       "nss_macsec_secy_extend_reg set HEX HEX HEX",
       "nss_macsec_secy_extend_reg\n" "set\n" "secy_id\n" "addr\n" "value\n")
{
	u32 secy_id = 0;
	u32 addr = 0;
	u32 value = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&addr));
	cli_str_2_hex(argv[4], (u32 *) (&value));

	CLI_EXEC_API(nss_macsec_secy_ext_reg_set(secy_id, addr, value));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_ctl_filt_get_func,
       nss_macsec_secy_tx_ctl_filt_get_cmd,
       "nss_macsec_secy_tx_ctl_filt get HEX HEX",
       "nss_macsec_secy_tx_ctl_filt\n" "get\n" "secy_id\n" "filt_id\n")
{
	u32 secy_id = 0;
	u32 filt_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&filt_id));

	CLI_EXEC_API(nss_macsec_secy_tx_ctl_filt_get
		     (secy_id, filt_id, &g_fal_tx_ctl_filt_t));
	_dump_g_fal_tx_ctl_filt_t(pVty, &g_fal_tx_ctl_filt_t);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_ctl_filt_set_func,
       nss_macsec_secy_tx_ctl_filt_set_cmd,
       "nss_macsec_secy_tx_ctl_filt set HEX HEX",
       "nss_macsec_secy_tx_ctl_filt\n" "set\n" "secy_id\n" "filt_id\n")
{
	u32 secy_id = 0;
	u32 filt_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&filt_id));

	CLI_EXEC_API(nss_macsec_secy_tx_ctl_filt_set
		     (secy_id, filt_id, &g_fal_tx_ctl_filt_t));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_ctl_filt_clear_func,
       nss_macsec_secy_tx_ctl_filt_clear_cmd,
       "nss_macsec_secy_tx_ctl_filt clear HEX HEX",
       "nss_macsec_secy_tx_ctl_filt\n" "clear\n" "secy_id\n" "filt_id\n")
{
	u32 secy_id = 0;
	u32 filt_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&filt_id));

	CLI_EXEC_API(nss_macsec_secy_tx_ctl_filt_clear(secy_id, filt_id));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_ctl_filt_clear_all_func,
       nss_macsec_secy_tx_ctl_filt_clear_all_cmd,
       "nss_macsec_secy_tx_ctl_filt_clear all HEX",
       "nss_macsec_secy_tx_ctl_filt_clear\n" "all\n" "secy_id\n")
{
	u32 secy_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_tx_ctl_filt_clear_all(secy_id));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_class_lut_get_func,
       nss_macsec_secy_tx_class_lut_get_cmd,
       "nss_macsec_secy_tx_class_lut get HEX HEX",
       "nss_macsec_secy_tx_class_lut\n" "get\n" "secy_id\n" "index\n")
{
	u32 secy_id = 0;
	u32 index = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&index));

	CLI_EXEC_API(nss_macsec_secy_tx_class_lut_get
		     (secy_id, index, &g_fal_tx_class_lut_t));
	_dump_g_fal_tx_class_lut_t(pVty, &g_fal_tx_class_lut_t);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_class_lut_set_func,
       nss_macsec_secy_tx_class_lut_set_cmd,
       "nss_macsec_secy_tx_class_lut set HEX HEX",
       "nss_macsec_secy_tx_class_lut\n" "set\n" "secy_id\n" "index\n")
{
	u32 secy_id = 0;
	u32 index = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&index));

	CLI_EXEC_API(nss_macsec_secy_tx_class_lut_set
		     (secy_id, index, &g_fal_tx_class_lut_t));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_class_lut_clear_func,
       nss_macsec_secy_tx_class_lut_clear_cmd,
       "nss_macsec_secy_tx_class_lut clear HEX HEX",
       "nss_macsec_secy_tx_class_lut\n" "clear\n" "secy_id\n" "index\n")
{
	u32 secy_id = 0;
	u32 index = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&index));

	CLI_EXEC_API(nss_macsec_secy_tx_class_lut_clear(secy_id, index));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_class_lut_clear_all_func,
       nss_macsec_secy_tx_class_lut_clear_all_cmd,
       "nss_macsec_secy_tx_class_lut_clear all HEX",
       "nss_macsec_secy_tx_class_lut_clear\n" "all\n" "secy_id\n")
{
	u32 secy_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_tx_class_lut_clear_all(secy_id));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sc_create_func,
       nss_macsec_secy_tx_sc_create_cmd,
       "nss_macsec_secy_tx_sc create HEX HEX WORD HEX",
       "nss_macsec_secy_tx_sc\n"
       "create\n" "secy_id\n" "channel\n" "psci\n" "sci_len\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u8 sci[8];
	u32 sci_len = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_sci(argv[4], sci);
	cli_str_2_hex(argv[5], (u32 *) (&sci_len));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_create
		     (secy_id, channel, sci, sci_len));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sc_en_get_func,
       nss_macsec_secy_tx_sc_en_get_cmd,
       "nss_macsec_secy_tx_sc_en get HEX HEX",
       "nss_macsec_secy_tx_sc_en\n" "get\n" "secy_id\n" "channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	bool penable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_en_get(secy_id, channel, &penable));
	vty_print("penable : 0x%x\n", penable);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sc_en_set_func,
       nss_macsec_secy_tx_sc_en_set_cmd,
       "nss_macsec_secy_tx_sc_en set HEX HEX HEX",
       "nss_macsec_secy_tx_sc_en\n" "set\n" "secy_id\n" "channel\n" "enable\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 enable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&enable));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_en_set(secy_id, channel, enable));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sc_del_func,
       nss_macsec_secy_tx_sc_del_cmd,
       "nss_macsec_secy_tx_sc del HEX HEX",
       "nss_macsec_secy_tx_sc\n" "del\n" "secy_id\n" "channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_del(secy_id, channel));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sc_del_all_func,
       nss_macsec_secy_tx_sc_del_all_cmd,
       "nss_macsec_secy_tx_sc_del all HEX",
       "nss_macsec_secy_tx_sc_del\n" "all\n" "secy_id\n")
{
	u32 secy_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_del_all(secy_id));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sc_an_get_func,
       nss_macsec_secy_tx_sc_an_get_cmd,
       "nss_macsec_secy_tx_sc_an get HEX HEX",
       "nss_macsec_secy_tx_sc_an\n" "get\n" "secy_id\n" "channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 pan = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_an_get(secy_id, channel, &pan));
	vty_print("pan : 0x%x\n", pan);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sc_an_set_func,
       nss_macsec_secy_tx_sc_an_set_cmd,
       "nss_macsec_secy_tx_sc_an set HEX HEX HEX",
       "nss_macsec_secy_tx_sc_an\n" "set\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_an_set(secy_id, channel, an));

	return CLI_OK;
}


DEFCMD(nss_macsec_secy_tx_sc_in_used_get_func,
       nss_macsec_secy_tx_sc_in_used_get_cmd,
       "nss_macsec_secy_tx_sc_in_used get HEX HEX",
       "nss_macsec_secy_tx_sc_in_used\n" "get\n" "secy_id\n" "channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	bool p_in_used = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_in_used_get
		     (secy_id, channel, &p_in_used));
	vty_print("p_in_used : 0x%x\n", p_in_used);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sc_tci_7_2_get_func,
       nss_macsec_secy_tx_sc_tci_7_2_get_cmd,
       "nss_macsec_secy_tx_sc_tci_7_2 get HEX HEX",
       "nss_macsec_secy_tx_sc_tci_7_2\n" "get\n" "secy_id\n" "channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u8 ptci = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_tci_7_2_get
		     (secy_id, channel, &ptci));
	vty_print("ptci : 0x%x\n", ptci);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sc_tci_7_2_set_func,
       nss_macsec_secy_tx_sc_tci_7_2_set_cmd,
       "nss_macsec_secy_tx_sc_tci_7_2 set HEX HEX HEX",
       "nss_macsec_secy_tx_sc_tci_7_2\n"
       "set\n" "secy_id\n" "channel\n" "tci\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 tci = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&tci));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_tci_7_2_set(secy_id, channel, tci));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sc_confidentiality_offset_get_func,
       nss_macsec_secy_tx_sc_confidentiality_offset_get_cmd,
       "nss_macsec_secy_tx_sc_confidentiality_offset get HEX HEX",
       "nss_macsec_secy_tx_sc_confidentiality_offset\n"
       "get\n" "secy_id\n" "channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 poffset = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_confidentiality_offset_get
		     (secy_id, channel, &poffset));
	vty_print("poffset : 0x%x\n", poffset);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sc_confidentiality_offset_set_func,
       nss_macsec_secy_tx_sc_confidentiality_offset_set_cmd,
       "nss_macsec_secy_tx_sc_confidentiality_offset set HEX HEX HEX",
       "nss_macsec_secy_tx_sc_confidentiality_offset\n"
       "set\n" "secy_id\n" "channel\n" "offset\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 offset = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&offset));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_confidentiality_offset_set
		     (secy_id, channel, offset));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sc_protect_get_func,
       nss_macsec_secy_tx_sc_protect_get_cmd,
       "nss_macsec_secy_tx_sc_protect get HEX HEX",
       "nss_macsec_secy_tx_sc_protect\n" "get\n" "secy_id\n" "channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	bool penable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_protect_get
		     (secy_id, channel, &penable));
	vty_print("penable : 0x%x\n", penable);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sc_protect_set_func,
       nss_macsec_secy_tx_sc_protect_set_cmd,
       "nss_macsec_secy_tx_sc_protect set HEX HEX HEX",
       "nss_macsec_secy_tx_sc_protect\n"
       "set\n" "secy_id\n" "channel\n" "enable\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 enable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&enable));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_protect_set
		     (secy_id, channel, enable));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sc_sci_get_func,
       nss_macsec_secy_tx_sc_sci_get_cmd,
       "nss_macsec_secy_tx_sc_sci get HEX HEX HEX",
       "nss_macsec_secy_tx_sc_sci\n"
       "get\n" "secy_id\n" "channel\n" "sci_len\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u8 sci[8];
	u32 sci_len = 0;
	char str[64];

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&sci_len));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_sci_get
		     (secy_id, channel, sci, sci_len));
	cli_sci_2_str(sci, str, sizeof(str));
	vty_print("sci : %s\n", str);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sa_create_func,
       nss_macsec_secy_tx_sa_create_cmd,
       "nss_macsec_secy_tx_sa create HEX HEX HEX",
       "nss_macsec_secy_tx_sa\n" "create\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_tx_sa_create(secy_id, channel, an));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sa_en_get_func,
       nss_macsec_secy_tx_sa_en_get_cmd,
       "nss_macsec_secy_tx_sa_en get HEX HEX HEX",
       "nss_macsec_secy_tx_sa_en\n" "get\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;
	bool penable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_tx_sa_en_get
		     (secy_id, channel, an, &penable));
	vty_print("penable : 0x%x\n", penable);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sa_en_set_func,
       nss_macsec_secy_tx_sa_en_set_cmd,
       "nss_macsec_secy_tx_sa_en set HEX HEX HEX HEX",
       "nss_macsec_secy_tx_sa_en\n"
       "set\n" "secy_id\n" "channel\n" "an\n" "enable\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;
	u32 enable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));
	cli_str_2_hex(argv[5], (u32 *) (&enable));

	CLI_EXEC_API(nss_macsec_secy_tx_sa_en_set
		     (secy_id, channel, an, enable));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sa_del_func,
       nss_macsec_secy_tx_sa_del_cmd,
       "nss_macsec_secy_tx_sa del HEX HEX HEX",
       "nss_macsec_secy_tx_sa\n" "del\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_tx_sa_del(secy_id, channel, an));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sa_del_all_func,
       nss_macsec_secy_tx_sa_del_all_cmd,
       "nss_macsec_secy_tx_sa_del all HEX",
       "nss_macsec_secy_tx_sa_del\n" "all\n" "secy_id\n")
{
	u32 secy_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_tx_sa_del_all(secy_id));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sa_next_pn_get_func,
       nss_macsec_secy_tx_sa_next_pn_get_cmd,
       "nss_macsec_secy_tx_sa_next_pn get HEX HEX HEX",
       "nss_macsec_secy_tx_sa_next_pn\n" "get\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;
	u32 p_next_pn = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_tx_sa_next_pn_get
		     (secy_id, channel, an, &p_next_pn));
	vty_print("p_next_pn : 0x%x\n", p_next_pn);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sa_next_pn_set_func,
       nss_macsec_secy_tx_sa_next_pn_set_cmd,
       "nss_macsec_secy_tx_sa_next_pn set HEX HEX HEX HEX",
       "nss_macsec_secy_tx_sa_next_pn\n"
       "set\n" "secy_id\n" "channel\n" "an\n" "next_pn\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;
	u32 next_pn = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));
	cli_str_2_hex(argv[5], (u32 *) (&next_pn));

	CLI_EXEC_API(nss_macsec_secy_tx_sa_next_pn_set
		     (secy_id, channel, an, next_pn));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sa_in_used_get_func,
       nss_macsec_secy_tx_sa_in_used_get_cmd,
       "nss_macsec_secy_tx_sa_in_used get HEX HEX HEX",
       "nss_macsec_secy_tx_sa_in_used\n" "get\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;
	bool p_in_used = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_tx_sa_in_used_get
		     (secy_id, channel, an, &p_in_used));
	vty_print("p_in_used : 0x%x\n", p_in_used);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sak_get_func,
       nss_macsec_secy_tx_sak_get_cmd,
       "nss_macsec_secy_tx_sak get HEX HEX HEX",
       "nss_macsec_secy_tx_sak\n" "get\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_tx_sak_get
		     (secy_id, channel, an, &g_fal_tx_sak_t));

	vty_print("sak_len : %d\n", g_fal_tx_sak_t.sak_len);
	if (g_fal_tx_sak_t.sak_len == 32) {
		_dump_g_fal_tx_sak_t(pVty, &g_fal_tx_sak_t);
		_dump_g_fal_tx_sak1_t(pVty, &g_fal_tx_sak_t);
	} else {
		_dump_g_fal_tx_sak_t(pVty, &g_fal_tx_sak_t);
	}

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sak_set_func,
       nss_macsec_secy_tx_sak_set_cmd,
       "nss_macsec_secy_tx_sak set HEX HEX HEX",
       "nss_macsec_secy_tx_sak\n" "set\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_tx_sak_set
		     (secy_id, channel, an, &g_fal_tx_sak_t));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_genl_reg_get_func,
       nss_macsec_secy_genl_reg_get_cmd,
       "nss_macsec_secy_genl_reg get HEX HEX",
       "nss_macsec_secy_genl_reg\n" "get\n" "secy_id\n" "addr\n")
{
	u32 secy_id = 0;
	u32 addr = 0;
	u32 pvalue = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&addr));

	CLI_EXEC_API(nss_macsec_secy_genl_reg_get(secy_id, addr, &pvalue));
	vty_print("pvalue : 0x%x\n", pvalue);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_genl_reg_set_func,
       nss_macsec_secy_genl_reg_set_cmd,
       "nss_macsec_secy_genl_reg set HEX HEX HEX",
       "nss_macsec_secy_genl_reg\n" "set\n" "secy_id\n" "addr\n" "value\n")
{
	u32 secy_id = 0;
	u32 addr = 0;
	u32 value = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&addr));
	cli_str_2_hex(argv[4], (u32 *) (&value));

	CLI_EXEC_API(nss_macsec_secy_genl_reg_set(secy_id, addr, value));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_ctl_filt_get_func,
       nss_macsec_secy_rx_ctl_filt_get_cmd,
       "nss_macsec_secy_rx_ctl_filt get HEX HEX",
       "nss_macsec_secy_rx_ctl_filt\n" "get\n" "secy_id\n" "filt_id\n")
{
	u32 secy_id = 0;
	u32 filt_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&filt_id));

	CLI_EXEC_API(nss_macsec_secy_rx_ctl_filt_get
		     (secy_id, filt_id, &g_fal_rx_ctl_filt_t));
	_dump_g_fal_rx_ctl_filt_t(pVty, &g_fal_rx_ctl_filt_t);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_ctl_filt_set_func,
       nss_macsec_secy_rx_ctl_filt_set_cmd,
       "nss_macsec_secy_rx_ctl_filt set HEX HEX",
       "nss_macsec_secy_rx_ctl_filt\n" "set\n" "secy_id\n" "filt_id\n")
{
	u32 secy_id = 0;
	u32 filt_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&filt_id));

	CLI_EXEC_API(nss_macsec_secy_rx_ctl_filt_set
		     (secy_id, filt_id, &g_fal_rx_ctl_filt_t));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_ctl_filt_clear_func,
       nss_macsec_secy_rx_ctl_filt_clear_cmd,
       "nss_macsec_secy_rx_ctl_filt clear HEX HEX",
       "nss_macsec_secy_rx_ctl_filt\n" "clear\n" "secy_id\n" "filt_id\n")
{
	u32 secy_id = 0;
	u32 filt_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&filt_id));

	CLI_EXEC_API(nss_macsec_secy_rx_ctl_filt_clear(secy_id, filt_id));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_ctl_filt_clear_all_func,
       nss_macsec_secy_rx_ctl_filt_clear_all_cmd,
       "nss_macsec_secy_rx_ctl_filt_clear all HEX",
       "nss_macsec_secy_rx_ctl_filt_clear\n" "all\n" "secy_id\n")
{
	u32 secy_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_rx_ctl_filt_clear_all(secy_id));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_prc_lut_get_func,
       nss_macsec_secy_rx_prc_lut_get_cmd,
       "nss_macsec_secy_rx_prc_lut get HEX HEX",
       "nss_macsec_secy_rx_prc_lut\n" "get\n" "secy_id\n" "index\n")
{
	u32 secy_id = 0;
	u32 index = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&index));

	CLI_EXEC_API(nss_macsec_secy_rx_prc_lut_get
		     (secy_id, index, &g_fal_rx_prc_lut_t));
	_dump_g_fal_rx_prc_lut_t(pVty, &g_fal_rx_prc_lut_t);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_prc_lut_set_func,
       nss_macsec_secy_rx_prc_lut_set_cmd,
       "nss_macsec_secy_rx_prc_lut set HEX HEX",
       "nss_macsec_secy_rx_prc_lut\n" "set\n" "secy_id\n" "index\n")
{
	u32 secy_id = 0;
	u32 index = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&index));

	CLI_EXEC_API(nss_macsec_secy_rx_prc_lut_set
		     (secy_id, index, &g_fal_rx_prc_lut_t));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_prc_lut_clear_func,
       nss_macsec_secy_rx_prc_lut_clear_cmd,
       "nss_macsec_secy_rx_prc_lut clear HEX HEX",
       "nss_macsec_secy_rx_prc_lut\n" "clear\n" "secy_id\n" "index\n")
{
	u32 secy_id = 0;
	u32 index = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&index));

	CLI_EXEC_API(nss_macsec_secy_rx_prc_lut_clear(secy_id, index));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_prc_lut_clear_all_func,
       nss_macsec_secy_rx_prc_lut_clear_all_cmd,
       "nss_macsec_secy_rx_prc_lut_clear all HEX",
       "nss_macsec_secy_rx_prc_lut_clear\n" "all\n" "secy_id\n")
{
	u32 secy_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_rx_prc_lut_clear_all(secy_id));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sc_create_func,
       nss_macsec_secy_rx_sc_create_cmd,
       "nss_macsec_secy_rx_sc create HEX HEX",
       "nss_macsec_secy_rx_sc\n"
       "create\n"
       "secy_id\n"
       "channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_rx_sc_create(secy_id, channel));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sc_en_get_func,
       nss_macsec_secy_rx_sc_en_get_cmd,
       "nss_macsec_secy_rx_sc_en get HEX HEX",
       "nss_macsec_secy_rx_sc_en\n" "get\n" "secy_id\n" "channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	bool penable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_rx_sc_en_get(secy_id, channel, &penable));
	vty_print("penable : 0x%x\n", penable);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sc_en_set_func,
       nss_macsec_secy_rx_sc_en_set_cmd,
       "nss_macsec_secy_rx_sc_en set HEX HEX HEX",
       "nss_macsec_secy_rx_sc_en\n" "set\n" "secy_id\n" "channel\n" "enable\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 enable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&enable));

	CLI_EXEC_API(nss_macsec_secy_rx_sc_en_set(secy_id, channel, enable));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sc_del_func,
       nss_macsec_secy_rx_sc_del_cmd,
       "nss_macsec_secy_rx_sc del HEX HEX",
       "nss_macsec_secy_rx_sc\n" "del\n" "secy_id\n" "channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_rx_sc_del(secy_id, channel));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sc_del_all_func,
       nss_macsec_secy_rx_sc_del_all_cmd,
       "nss_macsec_secy_rx_sc_del all HEX",
       "nss_macsec_secy_rx_sc_del\n" "all\n" "secy_id\n")
{
	u32 secy_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_rx_sc_del_all(secy_id));

	return CLI_OK;
}
#if 0
DEFCMD(nss_macsec_secy_rx_sc_sci_get_func,
       nss_macsec_secy_rx_sc_sci_get_cmd,
       "nss_macsec_secy_rx_sc_sci get HEX HEX HEX",
       "nss_macsec_secy_rx_sc_sci\n"
       "get\n" "secy_id\n" "channel\n" "sci_len\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u8 sci[8];
	u32 sci_len = 0;
	char str[64];

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&sci_len));

	CLI_EXEC_API(nss_macsec_secy_rx_sc_sci_get
		     (secy_id, channel, sci, sci_len));
	cli_sci_2_str(sci, str);
	vty_print("sci : %s\n", str);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sc_confidentiality_offset_get_func,
       nss_macsec_secy_rx_sc_confidentiality_offset_get_cmd,
       "nss_macsec_secy_rx_sc_confidentiality_offset get HEX HEX",
       "nss_macsec_secy_rx_sc_confidentiality_offset\n"
       "get\n" "secy_id\n" "channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 poffset = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_rx_sc_confidentiality_offset_get
		     (secy_id, channel, &poffset));
	vty_print("poffset : 0x%x\n", poffset);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sc_confidentiality_offset_set_func,
       nss_macsec_secy_rx_sc_confidentiality_offset_set_cmd,
       "nss_macsec_secy_rx_sc_confidentiality_offset set HEX HEX HEX",
       "nss_macsec_secy_rx_sc_confidentiality_offset\n"
       "set\n" "secy_id\n" "channel\n" "offset\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 offset = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&offset));

	CLI_EXEC_API(nss_macsec_secy_rx_sc_confidentiality_offset_set
		     (secy_id, channel, offset));

	return CLI_OK;
}
#endif
DEFCMD(nss_macsec_secy_rx_sc_validate_frame_get_func,
       nss_macsec_secy_rx_sc_validate_frame_get_cmd,
       "nss_macsec_secy_rx_sc_validate_frame get HEX HEX",
       "nss_macsec_secy_rx_sc_validate_frame\n" "get\n" "secy_id\n" "channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	fal_rx_sc_validate_frame_e pmode = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_rx_sc_validate_frame_get
		     (secy_id, channel, &pmode));
	vty_print("pmode : 0x%x\n", pmode);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sc_validate_frame_set_func,
       nss_macsec_secy_rx_sc_validate_frame_set_cmd,
       "nss_macsec_secy_rx_sc_validate_frame set HEX HEX HEX",
       "nss_macsec_secy_rx_sc_validate_frame\n"
       "set\n" "secy_id\n" "channel\n" "mode\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 mode = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&mode));

	CLI_EXEC_API(nss_macsec_secy_rx_sc_validate_frame_set
		     (secy_id, channel, mode));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sc_replay_protect_get_func,
       nss_macsec_secy_rx_sc_replay_protect_get_cmd,
       "nss_macsec_secy_rx_sc_replay_protect get HEX HEX",
       "nss_macsec_secy_rx_sc_replay_protect\n" "get\n" "secy_id\n" "channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	bool penable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_rx_sc_replay_protect_get
		     (secy_id, channel, &penable));
	vty_print("penable : 0x%x\n", penable);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sc_replay_protect_set_func,
       nss_macsec_secy_rx_sc_replay_protect_set_cmd,
       "nss_macsec_secy_rx_sc_replay_protect set HEX HEX HEX",
       "nss_macsec_secy_rx_sc_replay_protect\n"
       "set\n" "secy_id\n" "channel\n" "enable\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 enable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&enable));

	CLI_EXEC_API(nss_macsec_secy_rx_sc_replay_protect_set
		     (secy_id, channel, enable));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sc_anti_replay_window_get_func,
       nss_macsec_secy_rx_sc_anti_replay_window_get_cmd,
       "nss_macsec_secy_rx_sc_anti_replay_window get HEX HEX",
       "nss_macsec_secy_rx_sc_anti_replay_window\n"
       "get\n" "secy_id\n" "channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 pwindow = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_rx_sc_anti_replay_window_get
		     (secy_id, channel, &pwindow));
	vty_print("pwindow : 0x%x\n", pwindow);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sc_anti_replay_window_set_func,
       nss_macsec_secy_rx_sc_anti_replay_window_set_cmd,
       "nss_macsec_secy_rx_sc_anti_replay_window set HEX HEX HEX",
       "nss_macsec_secy_rx_sc_anti_replay_window\n"
       "set\n" "secy_id\n" "channel\n" "window\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 window = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&window));

	CLI_EXEC_API(nss_macsec_secy_rx_sc_anti_replay_window_set
		     (secy_id, channel, window));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sc_in_used_get_func,
       nss_macsec_secy_rx_sc_in_used_get_cmd,
       "nss_macsec_secy_rx_sc_in_used get HEX HEX",
       "nss_macsec_secy_rx_sc_in_used\n" "get\n" "secy_id\n" "channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	bool p_in_used = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_rx_sc_in_used_get
		     (secy_id, channel, &p_in_used));
	vty_print("p_in_used : 0x%x\n", p_in_used);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sa_create_func,
       nss_macsec_secy_rx_sa_create_cmd,
       "nss_macsec_secy_rx_sa create HEX HEX HEX",
       "nss_macsec_secy_rx_sa\n" "create\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_rx_sa_create(secy_id, channel, an));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sa_en_get_func,
       nss_macsec_secy_rx_sa_en_get_cmd,
       "nss_macsec_secy_rx_sa_en get HEX HEX HEX",
       "nss_macsec_secy_rx_sa_en\n" "get\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;
	bool penable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_rx_sa_en_get
		     (secy_id, channel, an, &penable));
	vty_print("penable : 0x%x\n", penable);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sa_en_set_func,
       nss_macsec_secy_rx_sa_en_set_cmd,
       "nss_macsec_secy_rx_sa_en set HEX HEX HEX HEX",
       "nss_macsec_secy_rx_sa_en\n"
       "set\n" "secy_id\n" "channel\n" "an\n" "enable\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;
	u32 enable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));
	cli_str_2_hex(argv[5], (u32 *) (&enable));

	CLI_EXEC_API(nss_macsec_secy_rx_sa_en_set
		     (secy_id, channel, an, enable));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sa_next_pn_get_func,
       nss_macsec_secy_rx_sa_next_pn_get_cmd,
       "nss_macsec_secy_rx_sa_next_pn get HEX HEX HEX",
       "nss_macsec_secy_rx_sa_next_pn\n" "get\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;
	u32 pnpn = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));
	cli_str_2_hex(argv[5], (u32 *) (&pnpn));

	CLI_EXEC_API(nss_macsec_secy_rx_sa_next_pn_get
		     (secy_id, channel, an, &pnpn));
	vty_print("pnpn : 0x%x\n", pnpn);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sa_del_func,
       nss_macsec_secy_rx_sa_del_cmd,
       "nss_macsec_secy_rx_sa del HEX HEX HEX",
       "nss_macsec_secy_rx_sa\n" "del\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_rx_sa_del(secy_id, channel, an));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sa_del_all_func,
       nss_macsec_secy_rx_sa_del_all_cmd,
       "nss_macsec_secy_rx_sa_del all HEX",
       "nss_macsec_secy_rx_sa_del\n" "all\n" "secy_id\n")
{
	u32 secy_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_rx_sa_del_all(secy_id));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sak_get_func,
       nss_macsec_secy_rx_sak_get_cmd,
       "nss_macsec_secy_rx_sak get HEX HEX HEX",
       "nss_macsec_secy_rx_sak\n" "get\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_rx_sak_get
		     (secy_id, channel, an, &g_fal_rx_sak_t));

	vty_print("sak_len : %d\n", g_fal_rx_sak_t.sak_len);
	if (g_fal_rx_sak_t.sak_len == 32) {
		_dump_g_fal_rx_sak_t(pVty, &g_fal_rx_sak_t);
		_dump_g_fal_rx_sak1_t(pVty, &g_fal_rx_sak_t);
	} else {
		_dump_g_fal_rx_sak_t(pVty, &g_fal_rx_sak_t);
	}

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sak_set_func,
       nss_macsec_secy_rx_sak_set_cmd,
       "nss_macsec_secy_rx_sak set HEX HEX HEX",
       "nss_macsec_secy_rx_sak\n" "set\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_rx_sak_set
		     (secy_id, channel, an, &g_fal_rx_sak_t));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sa_in_used_get_func,
       nss_macsec_secy_rx_sa_in_used_get_cmd,
       "nss_macsec_secy_rx_sa_in_used get HEX HEX HEX",
       "nss_macsec_secy_rx_sa_in_used\n" "get\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;
	bool p_in_used = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_rx_sa_in_used_get
		     (secy_id, channel, an, &p_in_used));
	vty_print("p_in_used : 0x%x\n", p_in_used);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_sc_sa_mapping_mode_get_func,
       nss_macsec_secy_sc_sa_mapping_mode_get_cmd,
       "nss_macsec_secy_sc_sa_mapping_mode get HEX",
       "nss_macsec_secy_sc_sa_mapping_mode\n" "get\n" "secy_id\n")
{
	u32 secy_id = 0;
	fal_sc_sa_mapping_mode_e pmode = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_sc_sa_mapping_mode_get(secy_id, &pmode));
	vty_print("pmode : 0x%x\n", pmode);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_sc_sa_mapping_mode_set_func,
       nss_macsec_secy_sc_sa_mapping_mode_set_cmd,
       "nss_macsec_secy_sc_sa_mapping_mode set HEX HEX",
       "nss_macsec_secy_sc_sa_mapping_mode\n" "set\n" "secy_id\n" "mode\n")
{
	u32 secy_id = 0;
	u32 mode = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&mode));

	CLI_EXEC_API(nss_macsec_secy_sc_sa_mapping_mode_set(secy_id, mode));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_controlled_port_en_get_func,
       nss_macsec_secy_controlled_port_en_get_cmd,
       "nss_macsec_secy_controlled_port_en get HEX",
       "nss_macsec_secy_controlled_port_en\n" "get\n" "secy_id\n")
{
	u32 secy_id = 0;
	bool penable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_controlled_port_en_get(secy_id, &penable));
	vty_print("penable : 0x%x\n", penable);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_controlled_port_en_set_func,
       nss_macsec_secy_controlled_port_en_set_cmd,
       "nss_macsec_secy_controlled_port_en set HEX HEX",
       "nss_macsec_secy_controlled_port_en\n" "set\n" "secy_id\n" "enable\n")
{
	u32 secy_id = 0;
	u32 enable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&enable));

	CLI_EXEC_API(nss_macsec_secy_controlled_port_en_set(secy_id, enable));

	return CLI_OK;
}


DEFCMD(nss_macsec_secy_cipher_suite_get_func,
       nss_macsec_secy_cipher_suite_get_cmd,
       "nss_macsec_secy_cipher_suite get HEX",
       "nss_macsec_secy_cipher_suite\n" "get\n" "secy_id\n")
{
	u32 secy_id = 0;
	fal_cipher_suite_e p_cipher_suite = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_cipher_suite_get
		     (secy_id, &p_cipher_suite));
	vty_print("p_cipher_suite : 0x%x\n", p_cipher_suite);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_cipher_suite_set_func,
       nss_macsec_secy_cipher_suite_set_cmd,
       "nss_macsec_secy_cipher_suite set HEX HEX",
       "nss_macsec_secy_cipher_suite\n" "set\n" "secy_id\n" "cipher_suite\n")
{
	u32 secy_id = 0;
	u32 cipher_suite = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&cipher_suite));

	CLI_EXEC_API(nss_macsec_secy_cipher_suite_set(secy_id, cipher_suite));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_mtu_get_func,
       nss_macsec_secy_mtu_get_cmd,
       "nss_macsec_secy_mtu get HEX",
       "nss_macsec_secy_mtu\n" "get\n" "secy_id\n")
{
	u32 secy_id = 0;
	u32 pmtu = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_mtu_get(secy_id, &pmtu));
	vty_print("pmtu : 0x%x\n", pmtu);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_mtu_set_func,
       nss_macsec_secy_mtu_set_cmd,
       "nss_macsec_secy_mtu set HEX HEX",
       "nss_macsec_secy_mtu\n" "set\n" "secy_id\n" "mtu\n")
{
	u32 secy_id = 0;
	u32 mtu = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&mtu));

	CLI_EXEC_API(nss_macsec_secy_mtu_set(secy_id, mtu));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_id_get_func,
	nss_macsec_secy_id_get_cmd,
	"nss_macsec_secy_id get WORD",
	"nss_macsec_secy_id\n get\n dev_name\n")
{
	u32 secy_id = 0;

	CLI_EXEC_API(nss_macsec_secy_id_get((u8 *)argv[2], &secy_id));
	vty_print("secy_id : 0x%x\n", secy_id);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_en_get_func,
       nss_macsec_secy_en_get_cmd,
       "nss_macsec_secy_en get HEX", "nss_macsec_secy_en\n" "get\n" "secy_id\n")
{
	u32 secy_id = 0;
	bool penable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_en_get(secy_id, &penable));
	vty_print("penable : 0x%x\n", penable);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_en_set_func,
       nss_macsec_secy_en_set_cmd,
       "nss_macsec_secy_en set HEX HEX",
       "nss_macsec_secy_en\n" "set\n" "secy_id\n" "enable\n")
{
	u32 secy_id = 0;
	u32 enable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&enable));

	CLI_EXEC_API(nss_macsec_secy_en_set(secy_id, enable));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sc_mib_get_func,
       nss_macsec_secy_tx_sc_mib_get_cmd,
       "nss_macsec_secy_tx_sc_mib get HEX HEX",
       "nss_macsec_secy_tx_sc_mib\n" "get\n" "secy_id\n" "channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_mib_get
		     (secy_id, channel, &g_fal_tx_sc_mib_t));
	_dump_g_fal_tx_sc_mib_t(pVty, &g_fal_tx_sc_mib_t);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sa_mib_get_func,
       nss_macsec_secy_tx_sa_mib_get_cmd,
       "nss_macsec_secy_tx_sa_mib get HEX HEX HEX",
       "nss_macsec_secy_tx_sa_mib\n" "get\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_tx_sa_mib_get
		     (secy_id, channel, an, &g_fal_tx_sa_mib_t));
	_dump_g_fal_tx_sa_mib_t(pVty, &g_fal_tx_sa_mib_t);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_mib_get_func,
       nss_macsec_secy_tx_mib_get_cmd,
       "nss_macsec_secy_tx_mib get HEX",
       "nss_macsec_secy_tx_mib\n" "get\n" "secy_id\n")
{
	u32 secy_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_tx_mib_get(secy_id, &g_fal_tx_mib_t));
	_dump_g_fal_tx_mib_t(pVty, &g_fal_tx_mib_t);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sa_mib_get_func,
       nss_macsec_secy_rx_sa_mib_get_cmd,
       "nss_macsec_secy_rx_sa_mib get HEX HEX HEX",
       "nss_macsec_secy_rx_sa_mib\n" "get\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_rx_sa_mib_get
		     (secy_id, channel, an, &g_fal_rx_sa_mib_t));
	_dump_g_fal_rx_sa_mib_t(pVty, &g_fal_rx_sa_mib_t);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_mib_get_func,
       nss_macsec_secy_rx_mib_get_cmd,
       "nss_macsec_secy_rx_mib get HEX",
       "nss_macsec_secy_rx_mib\n" "get\n" "secy_id\n")
{
	u32 secy_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_rx_mib_get(secy_id, &g_fal_rx_mib_t));
	_dump_g_fal_rx_mib_t(pVty, &g_fal_rx_mib_t);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_mib_clear_func,
       nss_macsec_secy_tx_mib_clear_cmd,
       "nss_macsec_secy_tx_mib clear HEX",
       "nss_macsec_secy_tx_mib\n" "clear\n" "secy_id\n")
{
	u32 secy_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_tx_mib_clear(secy_id));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sc_mib_clear_func,
       nss_macsec_secy_tx_sc_mib_clear_cmd,
       "nss_macsec_secy_tx_sc_mib clear HEX HEX",
       "nss_macsec_secy_tx_sc_mib\n" "clear\n" "secy_id\n" "channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_mib_clear(secy_id, channel));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sa_mib_clear_func,
       nss_macsec_secy_tx_sa_mib_clear_cmd,
       "nss_macsec_secy_tx_sa_mib clear HEX HEX HEX",
       "nss_macsec_secy_tx_sa_mib\n" "clear\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_tx_sa_mib_clear(secy_id, channel, an));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_mib_clear_func,
       nss_macsec_secy_rx_mib_clear_cmd,
       "nss_macsec_secy_rx_mib clear HEX",
       "nss_macsec_secy_rx_mib\n" "clear\n" "secy_id\n")
{
	u32 secy_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_rx_mib_clear(secy_id));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sa_mib_clear_func,
       nss_macsec_secy_rx_sa_mib_clear_cmd,
       "nss_macsec_secy_rx_sa_mib clear HEX HEX HEX",
       "nss_macsec_secy_rx_sa_mib\n" "clear\n" "secy_id\n" "channel\n" "an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));

	CLI_EXEC_API(nss_macsec_secy_rx_sa_mib_clear(secy_id, channel, an));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_replay_protect_set_func,
       nss_macsec_secy_rx_replay_protect_set_cmd,
       "nss_macsec_secy_rx_replay_protect set HEX HEX",
       "nss_macsec_secy_rx_replay_protect\n" "set\n" "secy_id\n" "enable\n")
{
	u32 secy_id = 0;
	u32 enable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&enable));

	CLI_EXEC_API(nss_macsec_secy_rx_replay_protect_set(secy_id, enable));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_replay_protect_get_func,
       nss_macsec_secy_rx_replay_protect_get_cmd,
       "nss_macsec_secy_rx_replay_protect get HEX",
       "nss_macsec_secy_rx_replay_protect\n" "get\n" "secy_id\n")
{
	u32 secy_id = 0;
	u32 enable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_rx_replay_protect_get(secy_id, &enable));
	vty_print("penable : 0x%x\n", enable);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_validate_frame_set_func,
       nss_macsec_secy_rx_validate_frame_set_cmd,
       "nss_macsec_secy_rx_validate_frame set HEX HEX",
       "nss_macsec_secy_rx_validate_frame\n" "set\n" "secy_id\n" "mode\n")
{
	u32 secy_id = 0;
	u32 mode = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&mode));

	CLI_EXEC_API(nss_macsec_secy_rx_validate_frame_set(secy_id, mode));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_validate_frame_get_func,
       nss_macsec_secy_rx_validate_frame_get_cmd,
       "nss_macsec_secy_rx_validate_frame get HEX",
       "nss_macsec_secy_rx_validate_frame\n" "get\n" "secy_id\n")
{
	u32 secy_id = 0;
	u32 mode = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_rx_validate_frame_get(secy_id, &mode));
	vty_print("mode : 0x%x\n", mode);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_xpn_en_get_func,
	nss_macsec_secy_xpn_en_get_cmd,
	"nss_macsec_secy_xpn_en get HEX",
	"nss_macsec_secy_xpn_en\n get\n secy_id\n")
{
	u32 secy_id = 0;
	bool penable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_xpn_en_get(secy_id, &penable));
	vty_print("penable : 0x%x\n", penable);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_xpn_en_set_func,
	nss_macsec_secy_xpn_en_set_cmd,
	"nss_macsec_secy_xpn_en set HEX HEX",
	"nss_macsec_secy_xpn_en\n set\n secy_id\n enable\n")
{
	u32 secy_id = 0;
	u32 enable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&enable));

	CLI_EXEC_API(nss_macsec_secy_xpn_en_set(secy_id, enable));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sa_next_xpn_get_func,
	nss_macsec_secy_rx_sa_next_xpn_get_cmd,
	"nss_macsec_secy_rx_sa_next_xpn get HEX HEX HEX",
	"nss_macsec_secy_rx_sa_next_xpn\n get\n secy_id\n channel\n an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;
	u32 pnpn = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));
	cli_str_2_hex(argv[5], (u32 *) (&pnpn));

	CLI_EXEC_API(nss_macsec_secy_rx_sa_next_xpn_get
		     (secy_id, channel, an, &pnpn));
	vty_print("xpn : 0x%x\n", pnpn);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sa_next_xpn_set_func,
	nss_macsec_secy_rx_sa_next_xpn_set_cmd,
	"nss_macsec_secy_rx_sa_next_xpn set HEX HEX HEX HEX",
	"nss_macsec_secy_rx_sa_next_xpn\n"
	"set\n secy_id\n channel\n an\n next_xpn\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;
	u32 next_pn = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));
	cli_str_2_hex(argv[5], (u32 *) (&next_pn));

	CLI_EXEC_API(nss_macsec_secy_rx_sa_next_xpn_set
		     (secy_id, channel, an, next_pn));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sa_next_xpn_get_func,
	nss_macsec_secy_tx_sa_next_xpn_get_cmd,
	"nss_macsec_secy_tx_sa_next_xpn get HEX HEX HEX",
	"nss_macsec_secy_tx_sa_next_xpn\n get\n secy_id\n channel\n an\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;
	u32 pnpn = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));
	cli_str_2_hex(argv[5], (u32 *) (&pnpn));

	CLI_EXEC_API(nss_macsec_secy_tx_sa_next_xpn_get
		     (secy_id, channel, an, &pnpn));
	vty_print("xpn : 0x%x\n", pnpn);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sa_next_xpn_set_func,
	nss_macsec_secy_tx_sa_next_xpn_set_cmd,
	"nss_macsec_secy_tx_sa_next_xpn set HEX HEX HEX HEX",
	"nss_macsec_secy_tx_sa_next_xpn\n"
	"set\n secy_id\n channel\n an\n next_xpn\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;
	u32 next_pn = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));
	cli_str_2_hex(argv[5], (u32 *) (&next_pn));

	CLI_EXEC_API(nss_macsec_secy_tx_sa_next_xpn_set
		     (secy_id, channel, an, next_pn));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sc_ssci_get_func,
	nss_macsec_secy_rx_sc_ssci_get_cmd,
	"nss_macsec_secy_rx_sc_ssci get HEX HEX",
	"nss_macsec_secy_rx_sc_ssci\n get\n secy_id\n channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 ssci = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_rx_sc_ssci_get(secy_id, channel, &ssci));
	vty_print("ssci : 0x%x\n", ssci);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sc_ssci_set_func,
	nss_macsec_secy_rx_sc_ssci_set_cmd,
	"nss_macsec_secy_rx_sc_ssci set HEX HEX HEX",
	"nss_macsec_secy_rx_sc_ssci\n set\n secy_id\n channel\n ssci\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 ssci = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&ssci));

	CLI_EXEC_API(nss_macsec_secy_rx_sc_ssci_set(secy_id, channel, ssci));

	return CLI_OK;
}
DEFCMD(nss_macsec_secy_tx_sc_ssci_get_func,
	nss_macsec_secy_tx_sc_ssci_get_cmd,
	"nss_macsec_secy_tx_sc_ssci get HEX HEX",
	"nss_macsec_secy_tx_sc_ssci\n get\n secy_id\n channel\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 ssci = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_ssci_get(secy_id, channel, &ssci));
	vty_print("ssci : 0x%x\n", ssci);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_tx_sc_ssci_set_func,
	nss_macsec_secy_tx_sc_ssci_set_cmd,
	"nss_macsec_secy_tx_sc_ssci set HEX HEX HEX",
	"nss_macsec_secy_tx_sc_ssci\n set\n secy_id\n channel\n ssci\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 ssci = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&ssci));

	CLI_EXEC_API(nss_macsec_secy_tx_sc_ssci_set(secy_id, channel, ssci));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_flow_control_en_get_func,
	nss_macsec_secy_flow_control_en_get_cmd,
	"nss_macsec_secy_flow_control_en get HEX",
	"nss_macsec_secy_flow_control_en\n get\n secy_id\n")
{
	u32 secy_id = 0;
	bool penable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_flow_control_en_get(secy_id, &penable));
	vty_print("penable : 0x%x\n", penable);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_flow_control_en_set_func,
	nss_macsec_secy_flow_control_en_set_cmd,
	"nss_macsec_secy_flow_control_en set HEX HEX",
	"nss_macsec_secy_flow_control_en\n set\n secy_id\n enable\n")
{
	u32 secy_id = 0;
	u32 enable = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&enable));

	CLI_EXEC_API(nss_macsec_secy_flow_control_en_set(secy_id, enable));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_special_pkt_ctrl_get_func,
	nss_macsec_secy_special_pkt_ctrl_get_cmd,
	"nss_macsec_secy_special_pkt_ctrl get HEX HEX",
	"nss_macsec_secy_special_pkt_ctrl\n get\n secy_id\n packet_type\n")
{
	u32 secy_id = 0;
	enum fal_packet_type_t packet_type;
	enum fal_packet_action_t packet_action = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&packet_type));

	CLI_EXEC_API(nss_macsec_secy_special_pkt_ctrl_get(secy_id,
		packet_type, &packet_action));
	vty_print("packet_action : 0x%x\n", packet_action);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_special_pkt_ctrl_set_func,
	nss_macsec_secy_special_pkt_ctrl_set_cmd,
	"nss_macsec_secy_special_pkt_ctrl set HEX HEX HEX",
	"nss_macsec_secy_special_pkt_ctrl\n set\n secy_id\n type\n action\n")
{
	u32 secy_id = 0;
	u32 packet_type = 0, packet_action = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&packet_type));
	cli_str_2_hex(argv[4], (u32 *) (&packet_action));

	CLI_EXEC_API(nss_macsec_secy_special_pkt_ctrl_set(secy_id, packet_type,
		packet_action));

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_udf_ethtype_get_func,
	nss_macsec_secy_udf_ethtype_get_cmd,
	"nss_macsec_secy_udf_ethtype get HEX",
	"nss_macsec_secy_udf_ethtype\n get\n secy_id\n")
{
	u32 secy_id = 0;
	bool penable = 0;
	u16 ethtype = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_udf_ethtype_get(secy_id, &penable,
		&ethtype));
	vty_print("enable : 0x%x\n", penable);
	vty_print("ethtype : 0x%x\n", ethtype);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_udf_ethtype_set_func,
	nss_macsec_secy_udf_ethtype_set_cmd,
	"nss_macsec_secy_udf_ethtype set HEX HEX HEX",
	"nss_macsec_secy_udf_ethtype\n set\n secy_id\n enable\n ethtype\n")
{
	u32 secy_id = 0;
	u32 enable = 0;
	u32 ethtype = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&enable));
	cli_str_2_hex(argv[4], (u32 *) (&ethtype));

	CLI_EXEC_API(nss_macsec_secy_udf_ethtype_set(secy_id, enable,
		ethtype));

	return CLI_OK;
}
DEFCMD(g_fal_tx_udf_filt_t_init_func,
	g_fal_tx_udf_filt_t_init_cmd,
	"g_fal_tx_udf_filt_t init", "g_fal_tx_udf_filt_t\n init\n")

{
	memset(&g_fal_tx_udf_filt_t, 0, sizeof(g_fal_tx_udf_filt_t));

	return CLI_OK;
}

DEFCMD(fal_tx_udf_filt_t_field0_add_func,
	fal_tx_udf_filt_t_field0_add_cmd,
	"fal_tx_udf_filt_t field0 add HEX",
	"fal_tx_udf_filt_t\n field0\n add\n field0\n")
{
	u32 field0 = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &field0));
	g_fal_tx_udf_filt_t.udf_field0 = (u16) field0;

	return CLI_OK;
}
DEFCMD(fal_tx_udf_filt_t_field1_add_func,
	fal_tx_udf_filt_t_field1_add_cmd,
	"fal_tx_udf_filt_t field1 add HEX",
	"fal_tx_udf_filt_t\n field1\n add\n field1\n")
{
	u32 field1 = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &field1));
	g_fal_tx_udf_filt_t.udf_field1 = (u16) field1;

	return CLI_OK;
}
DEFCMD(fal_tx_udf_filt_t_field2_add_func,
	fal_tx_udf_filt_t_field2_add_cmd,
	"fal_tx_udf_filt_t field2 add HEX",
	"fal_tx_udf_filt_t\n field2\n add\n field2\n")
{
	u32 field2 = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &field2));
	g_fal_tx_udf_filt_t.udf_field2 = (u16) field2;

	return CLI_OK;
}
DEFCMD(fal_tx_udf_filt_t_mask_add_func,
	fal_tx_udf_filt_t_mask_add_cmd,
	"fal_tx_udf_filt_t mask add HEX",
	"fal_tx_udf_filt_t\n mask\n add\n mask\n")
{
	u32 mask = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &mask));
	g_fal_tx_udf_filt_t.mask = (u16) mask;

	return CLI_OK;
}
DEFCMD(fal_tx_udf_filt_t_type_add_func,
	fal_tx_udf_filt_t_type_add_cmd,
	"fal_tx_udf_filt_t type add HEX",
	"fal_tx_udf_filt_t\n type\n add\n type\n")
{
	u32 type = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &type));
	g_fal_tx_udf_filt_t.type = (enum fal_tx_udf_filt_type_e) type;

	return CLI_OK;
}
DEFCMD(fal_tx_udf_filt_t_offset_add_func,
	fal_tx_udf_filt_t_offset_add_cmd,
	"fal_tx_udf_filt_t offset add HEX",
	"fal_tx_udf_filt_t\n offset\n add\n offset\n")
{
	u32 offset = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &offset));
	g_fal_tx_udf_filt_t.offset = (u16) offset;

	return CLI_OK;
}
DEFCMD(fal_tx_udf_filt_t_operator_add_func,
	fal_tx_udf_filt_t_operator_add_cmd,
	"fal_tx_udf_filt_t operator add HEX",
	"fal_tx_udf_filt_t\n operator\n add\n operator\n")
{
	u32 operator = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &operator));
	g_fal_tx_udf_filt_t.operator = (enum fal_tx_udf_filt_op_e) operator;

	return CLI_OK;
}
DEFCMD(nss_macsec_secy_tx_udf_filt_set_func,
	nss_macsec_secy_tx_udf_filt_set_cmd,
	"nss_macsec_secy_tx_udf_filt set HEX HEX",
	"nss_macsec_secy_tx_udf_filt\n set\n secy_id\n filt_id\n")
{
	u32 secy_id = 0;
	u32 filt_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&filt_id));

	CLI_EXEC_API(nss_macsec_secy_tx_udf_filt_set
		     (secy_id, filt_id, &g_fal_tx_udf_filt_t));

	return CLI_OK;
}
void _dump_g_fal_tx_udf_filt_t(VTY_T *pVty, struct fal_tx_udf_filt_t *value)
{
	vty_print("udf_field0 : 0x%x\n", value->udf_field0);
	vty_print("udf_field1 : 0x%x\n", value->udf_field1);
	vty_print("udf_field2 : 0x%x\n", value->udf_field2);
	vty_print("mask : 0x%x\n", value->mask);
	vty_print("type : 0x%x\n", value->type);
	vty_print("offset : 0x%x\n", value->offset);
	vty_print("operator : 0x%x\n", value->operator);
	vty_print("\n");
}

DEFCMD(nss_macsec_secy_tx_udf_filt_get_func,
	nss_macsec_secy_tx_udf_filt_get_cmd,
	"nss_macsec_secy_tx_udf_filt get HEX HEX",
	"nss_macsec_secy_tx_udf_filt\n get\n secy_id\n filt_id\n")
{
	u32 secy_id = 0;
	u32 filt_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&filt_id));

	CLI_EXEC_API(nss_macsec_secy_tx_udf_filt_get
		     (secy_id, filt_id, &g_fal_tx_udf_filt_t));
	_dump_g_fal_tx_udf_filt_t(pVty, &g_fal_tx_udf_filt_t);

	return CLI_OK;
}

DEFCMD(g_fal_rx_udf_filt_t_init_func,
	g_fal_rx_udf_filt_t_init_cmd,
	"g_fal_rx_udf_filt_t init", "g_fal_rx_udf_filt_t\n init\n")
{
	memset(&g_fal_rx_udf_filt_t, 0, sizeof(g_fal_rx_udf_filt_t));

	return CLI_OK;
}
DEFCMD(fal_rx_udf_filt_t_field0_add_func,
	fal_rx_udf_filt_t_field0_add_cmd,
	"fal_rx_udf_filt_t field0 add HEX",
	"fal_rx_udf_filt_t\n field0\n add\n field0\n")
{
	u32 field0 = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &field0));
	g_fal_rx_udf_filt_t.udf_field0 = (u16) field0;

	return CLI_OK;
}
DEFCMD(fal_rx_udf_filt_t_field1_add_func,
	fal_rx_udf_filt_t_field1_add_cmd,
	"fal_rx_udf_filt_t field1 add HEX",
	"fal_rx_udf_filt_t\n field1\n add\n field1\n")
{
	u32 field1 = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &field1));
	g_fal_rx_udf_filt_t.udf_field1 = (u16) field1;

	return CLI_OK;
}
DEFCMD(fal_rx_udf_filt_t_field2_add_func,
	fal_rx_udf_filt_t_field2_add_cmd,
	"fal_rx_udf_filt_t field2 add HEX",
	"fal_rx_udf_filt_t\n field2\n add\n field2\n")
{
	u32 field2 = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &field2));
	g_fal_rx_udf_filt_t.udf_field2 = (u16) field2;

	return CLI_OK;
}
DEFCMD(fal_rx_udf_filt_t_mask_add_func,
	fal_rx_udf_filt_t_mask_add_cmd,
	"fal_rx_udf_filt_t mask add HEX",
	"fal_rx_udf_filt_t\n mask\n add\n mask\n")
{
	u32 mask = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &mask));
	g_fal_rx_udf_filt_t.mask = (u16) mask;

	return CLI_OK;
}
DEFCMD(fal_rx_udf_filt_t_type_add_func,
	fal_rx_udf_filt_t_type_add_cmd,
	"fal_rx_udf_filt_t type add HEX",
	"fal_rx_udf_filt_t\n type\n add\n type\n")
{
	u32 type = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &type));
	g_fal_rx_udf_filt_t.type = (enum fal_rx_udf_filt_type_e) type;

	return CLI_OK;
}
DEFCMD(fal_rx_udf_filt_t_offset_add_func,
	fal_rx_udf_filt_t_offset_add_cmd,
	"fal_rx_udf_filt_t offset add HEX",
	"fal_rx_udf_filt_t\n offset\n add\n offset\n")
{
	u32 offset = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &offset));
	g_fal_rx_udf_filt_t.offset = (u16) offset;

	return CLI_OK;
}
DEFCMD(fal_rx_udf_filt_t_operator_add_func,
	fal_rx_udf_filt_t_operator_add_cmd,
	"fal_rx_udf_filt_t operator add HEX",
	"fal_rx_udf_filt_t\n operator\n add\n operator\n")
{
	u32 operator = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &operator));
	g_fal_rx_udf_filt_t.operator = (enum fal_rx_udf_filt_op_e) operator;

	return CLI_OK;
}
DEFCMD(nss_macsec_secy_rx_udf_filt_set_func,
	nss_macsec_secy_rx_udf_filt_set_cmd,
	"nss_macsec_secy_rx_udf_filt set HEX HEX",
	"nss_macsec_secy_rx_udf_filt\n set\n secy_id\n filt_id\n")
{
	u32 secy_id = 0;
	u32 filt_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&filt_id));

	CLI_EXEC_API(nss_macsec_secy_rx_udf_filt_set
		     (secy_id, filt_id, &g_fal_rx_udf_filt_t));

	return CLI_OK;
}
void _dump_g_fal_rx_udf_filt_t(VTY_T *pVty, struct fal_rx_udf_filt_t *value)
{
	vty_print("udf_field0 : 0x%x\n", value->udf_field0);
	vty_print("udf_field1 : 0x%x\n", value->udf_field1);
	vty_print("udf_field2 : 0x%x\n", value->udf_field2);
	vty_print("mask : 0x%x\n", value->mask);
	vty_print("type : 0x%x\n", value->type);
	vty_print("offset : 0x%x\n", value->offset);
	vty_print("operator : 0x%x\n", value->operator);
	vty_print("\n");
}

DEFCMD(nss_macsec_secy_rx_udf_filt_get_func,
	nss_macsec_secy_rx_udf_filt_get_cmd,
	"nss_macsec_secy_rx_udf_filt get HEX HEX",
	"nss_macsec_secy_rx_udf_filt\n get\n secy_id\n filt_id\n")
{
	u32 secy_id = 0;
	u32 filt_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&filt_id));

	CLI_EXEC_API(nss_macsec_secy_rx_udf_filt_get
		     (secy_id, filt_id, &g_fal_rx_udf_filt_t));
	_dump_g_fal_rx_udf_filt_t(pVty, &g_fal_rx_udf_filt_t);

	return CLI_OK;
}

DEFCMD(g_fal_tx_udf_ufilt_cfg_t_init_func,
	g_fal_tx_udf_ufilt_cfg_t_init_cmd,
	"g_fal_tx_udf_ufilt_cfg_t init", "g_fal_tx_udf_ufilt_cfg_t\n init\n")
{
	memset(&g_fal_tx_udf_ufilt_cfg_t, 0, sizeof(g_fal_tx_udf_ufilt_cfg_t));

	return CLI_OK;
}
DEFCMD(fal_tx_udf_ufilt_cfg_t_enable_add_func,
	fal_tx_udf_ufilt_cfg_t_enable_add_cmd,
	"fal_tx_udf_ufilt_cfg_t enable add HEX",
	"fal_tx_udf_ufilt_cfg_t\n enable\n add\n enable\n")
{
	u32 enable = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &enable));
	g_fal_tx_udf_ufilt_cfg_t.enable = (bool) enable;

	return CLI_OK;
}
DEFCMD(fal_tx_udf_ufilt_cfg_t_priority_add_func,
	fal_tx_udf_ufilt_cfg_t_priority_add_cmd,
	"fal_tx_udf_ufilt_cfg_t priority add HEX",
	"fal_tx_udf_ufilt_cfg_t\n priority\n add\n priority\n")
{
	u32 priority = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &priority));
	g_fal_tx_udf_ufilt_cfg_t.priority = (u16) priority;

	return CLI_OK;
}
DEFCMD(fal_tx_udf_ufilt_cfg_t_inverse_add_func,
	fal_tx_udf_ufilt_cfg_t_inverse_add_cmd,
	"fal_tx_udf_ufilt_cfg_t inverse add HEX",
	"fal_tx_udf_ufilt_cfg_t\n inverse\n add\n inverse\n")
{
	u32 inverse = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &inverse));
	g_fal_tx_udf_ufilt_cfg_t.inverse = (u16) inverse;

	return CLI_OK;
}

DEFCMD(fal_tx_udf_ufilt_cfg_t_pattern0_add_func,
	fal_tx_udf_ufilt_cfg_t_pattern0_add_cmd,
	"fal_tx_udf_ufilt_cfg_t pattern0 add HEX",
	"fal_tx_udf_ufilt_cfg_t\n pattern0\n add\n pattern0\n")
{
	u32 pattern0 = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &pattern0));
	g_fal_tx_udf_ufilt_cfg_t.pattern0 =
		(enum fal_tx_udf_filt_cfg_pattern_e) pattern0;

	return CLI_OK;
}
DEFCMD(fal_tx_udf_ufilt_cfg_t_pattern1_add_func,
	fal_tx_udf_ufilt_cfg_t_pattern1_add_cmd,
	"fal_tx_udf_ufilt_cfg_t pattern1 add HEX",
	"fal_tx_udf_ufilt_cfg_t\n pattern1\n add\n pattern1\n")
{
	u32 pattern1 = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &pattern1));
	g_fal_tx_udf_ufilt_cfg_t.pattern1 =
		(enum fal_tx_udf_filt_cfg_pattern_e) pattern1;

	return CLI_OK;
}
DEFCMD(fal_tx_udf_ufilt_cfg_t_pattern2_add_func,
	fal_tx_udf_ufilt_cfg_t_pattern2_add_cmd,
	"fal_tx_udf_ufilt_cfg_t pattern2 add HEX",
	"fal_tx_udf_ufilt_cfg_t\n pattern2\n add\n pattern2\n")
{
	u32 pattern2 = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &pattern2));
	g_fal_tx_udf_ufilt_cfg_t.pattern2 =
		(enum fal_tx_udf_filt_cfg_pattern_e) pattern2;

	return CLI_OK;
}
DEFCMD(nss_macsec_secy_tx_udf_ufilt_cfg_set_func,
	nss_macsec_secy_tx_udf_ufilt_cfg_set_cmd,
	"nss_macsec_secy_tx_udf_ufilt_cfg set HEX",
	"nss_macsec_secy_tx_udf_ufilt_cfg\n set\n secy_id\n")
{
	u32 secy_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_tx_udf_ufilt_cfg_set
		     (secy_id, &g_fal_tx_udf_ufilt_cfg_t));

	return CLI_OK;
}
void _dump_g_fal_tx_udf_ufilt_cfg_t(VTY_T *pVty,
	struct fal_tx_udf_filt_cfg_t *value)
{
	vty_print("enable : 0x%x\n", value->enable);
	vty_print("priority : 0x%x\n", value->priority);
	vty_print("inverse : 0x%x\n", value->inverse);
	vty_print("pattern0 : 0x%x\n", value->pattern0);
	vty_print("pattern1 : 0x%x\n", value->pattern1);
	vty_print("pattern2 : 0x%x\n", value->pattern2);
	vty_print("\n");
}

DEFCMD(nss_macsec_secy_tx_udf_ufilt_cfg_get_func,
	nss_macsec_secy_tx_udf_ufilt_cfg_get_cmd,
	"nss_macsec_secy_tx_udf_ufilt_cfg get HEX",
	"nss_macsec_secy_tx_udf_ufilt_cfg\n get\n secy_id\n")
{
	u32 secy_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_tx_udf_ufilt_cfg_get
		     (secy_id, &g_fal_tx_udf_ufilt_cfg_t));
	_dump_g_fal_tx_udf_ufilt_cfg_t(pVty, &g_fal_tx_udf_ufilt_cfg_t);

	return CLI_OK;
}

DEFCMD(g_fal_tx_udf_cfilt_cfg_t_init_func,
	g_fal_tx_udf_cfilt_cfg_t_init_cmd,
	"g_fal_tx_udf_cfilt_cfg_t init", "g_fal_tx_udf_cfilt_cfg_t\n init\n")
{
	memset(&g_fal_tx_udf_cfilt_cfg_t, 0, sizeof(g_fal_tx_udf_cfilt_cfg_t));

	return CLI_OK;
}
DEFCMD(fal_tx_udf_cfilt_cfg_t_enable_add_func,
	fal_tx_udf_cfilt_cfg_t_enable_add_cmd,
	"fal_tx_udf_cfilt_cfg_t enable add HEX",
	"fal_tx_udf_cfilt_cfg_t\n enable\n add\n enable\n")
{
	u32 enable = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &enable));
	g_fal_tx_udf_cfilt_cfg_t.enable = (bool) enable;

	return CLI_OK;
}
DEFCMD(fal_tx_udf_cfilt_cfg_t_priority_add_func,
	fal_tx_udf_cfilt_cfg_t_priority_add_cmd,
	"fal_tx_udf_cfilt_cfg_t priority add HEX",
	"fal_tx_udf_cfilt_cfg_t\n priority\n add\n priority\n")
{
	u32 priority = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &priority));
	g_fal_tx_udf_cfilt_cfg_t.priority = (u16) priority;

	return CLI_OK;
}
DEFCMD(fal_tx_udf_cfilt_cfg_t_inverse_add_func,
	fal_tx_udf_cfilt_cfg_t_inverse_add_cmd,
	"fal_tx_udf_cfilt_cfg_t inverse add HEX",
	"fal_tx_udf_cfilt_cfg_t\n inverse\n add\n inverse\n")
{
	u32 inverse = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &inverse));
	g_fal_tx_udf_cfilt_cfg_t.inverse = (u16) inverse;

	return CLI_OK;
}

DEFCMD(fal_tx_udf_cfilt_cfg_t_pattern0_add_func,
	fal_tx_udf_cfilt_cfg_t_pattern0_add_cmd,
	"fal_tx_udf_cfilt_cfg_t pattern0 add HEX",
	"fal_tx_udf_cfilt_cfg_t\n pattern0\n add\n pattern0\n")
{
	u32 pattern0 = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &pattern0));
	g_fal_tx_udf_cfilt_cfg_t.pattern0 =
		(enum fal_tx_udf_filt_cfg_pattern_e) pattern0;

	return CLI_OK;
}
DEFCMD(fal_tx_udf_cfilt_cfg_t_pattern1_add_func,
	fal_tx_udf_cfilt_cfg_t_pattern1_add_cmd,
	"fal_tx_udf_cfilt_cfg_t pattern1 add HEX",
	"fal_tx_udf_cfilt_cfg_t\n pattern1\n add\n pattern1\n")
{
	u32 pattern1 = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &pattern1));
	g_fal_tx_udf_cfilt_cfg_t.pattern1 =
		(enum fal_tx_udf_filt_cfg_pattern_e) pattern1;

	return CLI_OK;
}
DEFCMD(fal_tx_udf_cfilt_cfg_t_pattern2_add_func,
	fal_tx_udf_cfilt_cfg_t_pattern2_add_cmd,
	"fal_tx_udf_cfilt_cfg_t pattern2 add HEX",
	"fal_tx_udf_cfilt_cfg_t\n pattern2\n add\n pattern2\n")
{
	u32 pattern2 = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &pattern2));
	g_fal_tx_udf_cfilt_cfg_t.pattern2 =
		(enum fal_tx_udf_filt_cfg_pattern_e) pattern2;

	return CLI_OK;
}
DEFCMD(nss_macsec_secy_tx_udf_cfilt_cfg_set_func,
	nss_macsec_secy_tx_udf_cfilt_cfg_set_cmd,
	"nss_macsec_secy_tx_udf_cfilt_cfg set HEX",
	"nss_macsec_secy_tx_udf_cfilt_cfg\n set\n secy_id\n")
{
	u32 secy_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_tx_udf_cfilt_cfg_set
		     (secy_id, &g_fal_tx_udf_cfilt_cfg_t));

	return CLI_OK;
}
void _dump_g_fal_tx_udf_cfilt_cfg_t(VTY_T *pVty,
	struct fal_tx_udf_filt_cfg_t *value)
{
	vty_print("enable : 0x%x\n", value->enable);
	vty_print("priority : 0x%x\n", value->priority);
	vty_print("inverse : 0x%x\n", value->inverse);
	vty_print("pattern0 : 0x%x\n", value->pattern0);
	vty_print("pattern1 : 0x%x\n", value->pattern1);
	vty_print("pattern2 : 0x%x\n", value->pattern2);
	vty_print("\n");
}

DEFCMD(nss_macsec_secy_tx_udf_cfilt_cfg_get_func,
	nss_macsec_secy_tx_udf_cfilt_cfg_get_cmd,
	"nss_macsec_secy_tx_udf_cfilt_cfg get HEX",
	"nss_macsec_secy_tx_udf_cfilt_cfg\n get\n secy_id\n")
{
	u32 secy_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_tx_udf_cfilt_cfg_get
		     (secy_id, &g_fal_tx_udf_cfilt_cfg_t));
	_dump_g_fal_tx_udf_cfilt_cfg_t(pVty, &g_fal_tx_udf_cfilt_cfg_t);

	return CLI_OK;
}

DEFCMD(g_fal_rx_udf_ufilt_cfg_t_init_func,
	g_fal_rx_udf_ufilt_cfg_t_init_cmd,
	"g_fal_rx_udf_ufilt_cfg_t init", "g_fal_rx_udf_ufilt_cfg_t\n init\n")
{
	memset(&g_fal_rx_udf_ufilt_cfg_t, 0, sizeof(g_fal_rx_udf_ufilt_cfg_t));

	return CLI_OK;
}
DEFCMD(fal_rx_udf_ufilt_cfg_t_enable_add_func,
	fal_rx_udf_ufilt_cfg_t_enable_add_cmd,
	"fal_rx_udf_ufilt_cfg_t enable add HEX",
	"fal_rx_udf_ufilt_cfg_t\n enable\n add\n enable\n")
{
	u32 enable = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &enable));
	g_fal_rx_udf_ufilt_cfg_t.enable = (bool) enable;

	return CLI_OK;
}
DEFCMD(fal_rx_udf_ufilt_cfg_t_priority_add_func,
	fal_rx_udf_ufilt_cfg_t_priority_add_cmd,
	"fal_rx_udf_ufilt_cfg_t priority add HEX",
	"fal_rx_udf_ufilt_cfg_t\n priority\n add\n priority\n")
{
	u32 priority = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &priority));
	g_fal_rx_udf_ufilt_cfg_t.priority = (u16) priority;

	return CLI_OK;
}
DEFCMD(fal_rx_udf_ufilt_cfg_t_inverse_add_func,
	fal_rx_udf_ufilt_cfg_t_inverse_add_cmd,
	"fal_rx_udf_ufilt_cfg_t inverse add HEX",
	"fal_rx_udf_ufilt_cfg_t\n inverse\n add\n inverse\n")
{
	u32 inverse = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &inverse));
	g_fal_rx_udf_ufilt_cfg_t.inverse = (u16) inverse;

	return CLI_OK;
}

DEFCMD(fal_rx_udf_ufilt_cfg_t_pattern0_add_func,
	fal_rx_udf_ufilt_cfg_t_pattern0_add_cmd,
	"fal_rx_udf_ufilt_cfg_t pattern0 add HEX",
	"fal_rx_udf_ufilt_cfg_t\n pattern0\n add\n pattern0\n")
{
	u32 pattern0 = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &pattern0));
	g_fal_rx_udf_ufilt_cfg_t.pattern0 =
		(enum fal_rx_udf_filt_cfg_pattern_e) pattern0;

	return CLI_OK;
}
DEFCMD(fal_rx_udf_ufilt_cfg_t_pattern1_add_func,
	fal_rx_udf_ufilt_cfg_t_pattern1_add_cmd,
	"fal_rx_udf_ufilt_cfg_t pattern1 add HEX",
	"fal_rx_udf_ufilt_cfg_t\n pattern1\n add\n pattern1\n")
{
	u32 pattern1 = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &pattern1));
	g_fal_rx_udf_ufilt_cfg_t.pattern1 =
		(enum fal_rx_udf_filt_cfg_pattern_e) pattern1;

	return CLI_OK;
}
DEFCMD(fal_rx_udf_ufilt_cfg_t_pattern2_add_func,
	fal_rx_udf_ufilt_cfg_t_pattern2_add_cmd,
	"fal_rx_udf_ufilt_cfg_t pattern2 add HEX",
	"fal_rx_udf_ufilt_cfg_t\n pattern2\n add\n pattern2\n")
{
	u32 pattern2 = 0;

	CLI_EXEC_API(cli_str_2_hex(argv[3], &pattern2));
	g_fal_rx_udf_ufilt_cfg_t.pattern2 =
		(enum fal_rx_udf_filt_cfg_pattern_e) pattern2;

	return CLI_OK;
}
DEFCMD(nss_macsec_secy_rx_udf_ufilt_cfg_set_func,
	nss_macsec_secy_rx_udf_ufilt_cfg_set_cmd,
	"nss_macsec_secy_rx_udf_ufilt_cfg set HEX",
	"nss_macsec_secy_rx_udf_ufilt_cfg\n set\n secy_id\n")
{
	u32 secy_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_rx_udf_ufilt_cfg_set
		     (secy_id, &g_fal_rx_udf_ufilt_cfg_t));

	return CLI_OK;
}
void _dump_g_fal_rx_udf_ufilt_cfg_t(VTY_T *pVty,
	struct fal_rx_udf_filt_cfg_t *value)
{
	vty_print("enable : 0x%x\n", value->enable);
	vty_print("priority : 0x%x\n", value->priority);
	vty_print("inverse : 0x%x\n", value->inverse);
	vty_print("pattern0 : 0x%x\n", value->pattern0);
	vty_print("pattern1 : 0x%x\n", value->pattern1);
	vty_print("pattern2 : 0x%x\n", value->pattern2);
	vty_print("\n");
}

DEFCMD(nss_macsec_secy_rx_udf_ufilt_cfg_get_func,
	nss_macsec_secy_rx_udf_ufilt_cfg_get_cmd,
	"nss_macsec_secy_rx_udf_ufilt_cfg get HEX",
	"nss_macsec_secy_rx_udf_ufilt_cfg\n get\n secy_id\n")
{
	u32 secy_id = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));

	CLI_EXEC_API(nss_macsec_secy_rx_udf_ufilt_cfg_get
		     (secy_id, &g_fal_rx_udf_ufilt_cfg_t));
	_dump_g_fal_rx_udf_ufilt_cfg_t(pVty, &g_fal_rx_udf_ufilt_cfg_t);

	return CLI_OK;
}

DEFCMD(nss_macsec_secy_rx_sa_next_pn_set_func,
	nss_macsec_secy_rx_sa_next_pn_set_cmd,
	"nss_macsec_secy_rx_sa_next_pn set HEX HEX HEX HEX",
	"nss_macsec_secy_rx_sa_next_pn\n"
	"set\n secy_id\n channel\n an\n next_pn\n")
{
	u32 secy_id = 0;
	u32 channel = 0;
	u32 an = 0;
	u32 next_pn = 0;

	cli_str_2_hex(argv[2], (u32 *) (&secy_id));
	cli_str_2_hex(argv[3], (u32 *) (&channel));
	cli_str_2_hex(argv[4], (u32 *) (&an));
	cli_str_2_hex(argv[5], (u32 *) (&next_pn));

	CLI_EXEC_API(nss_macsec_secy_rx_sa_next_pn_set
		     (secy_id, channel, an, next_pn));

	return CLI_OK;
}

int cli_cmd_fal_init(void)
{
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_ext_reg_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_ext_reg_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_ctl_filt_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_ctl_filt_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_ctl_filt_clear_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_tx_ctl_filt_clear_all_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_class_lut_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_class_lut_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_class_lut_clear_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_tx_class_lut_clear_all_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sc_create_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sc_en_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sc_en_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sc_del_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sc_del_all_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sc_an_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sc_an_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sc_in_used_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sc_tci_7_2_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sc_tci_7_2_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_tx_sc_confidentiality_offset_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_tx_sc_confidentiality_offset_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sc_protect_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sc_protect_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sc_sci_get_cmd);

	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sa_create_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sa_en_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sa_en_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sa_del_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sa_del_all_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sa_next_pn_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sa_next_pn_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sa_in_used_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sak_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sak_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_genl_reg_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_genl_reg_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_ctl_filt_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_ctl_filt_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_ctl_filt_clear_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_rx_ctl_filt_clear_all_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_prc_lut_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_prc_lut_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_prc_lut_clear_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_rx_prc_lut_clear_all_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sc_create_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sc_en_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sc_en_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sc_del_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sc_del_all_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_rx_sc_validate_frame_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_rx_sc_validate_frame_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_rx_sc_replay_protect_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_rx_sc_replay_protect_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_rx_sc_anti_replay_window_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_rx_sc_anti_replay_window_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sc_in_used_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sa_create_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sa_en_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sa_en_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sa_next_pn_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sa_del_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sa_del_all_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sak_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sak_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sa_in_used_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_sc_sa_mapping_mode_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_sc_sa_mapping_mode_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_controlled_port_en_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_controlled_port_en_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_cipher_suite_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_cipher_suite_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_mtu_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_mtu_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_id_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_en_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_en_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sc_mib_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sa_mib_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_mib_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sa_mib_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_mib_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_mib_clear_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sc_mib_clear_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sa_mib_clear_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_mib_clear_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sa_mib_clear_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &g_fal_tx_sak_t_init_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_sak_t_sak_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &g_fal_rx_ctl_filt_t_init_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_ctl_filt_t_match_type_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_ctl_filt_t_match_mask_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_ctl_filt_t_bypass_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_ctl_filt_t_sa_da_addr_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_rx_ctl_filt_t_ether_type_da_range_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &g_fal_rx_prc_lut_t_init_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_rx_prc_lut_t_ether_type_mask_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_prc_lut_t_sa_mask_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_prc_lut_t_da_mask_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_prc_lut_t_sci_mask_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_prc_lut_t_ether_type_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_prc_lut_t_da_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_prc_lut_t_channel_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_rx_prc_lut_t_uncontrolled_port_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_prc_lut_t_valid_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_prc_lut_t_tci_mask_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_prc_lut_t_sci_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_prc_lut_t_tci_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_prc_lut_t_sa_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_prc_lut_t_action_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_prc_lut_t_offset_add_cmd);

	cli_install_cmd(CLI_MODE_ENABLE,
		&fal_rx_prc_lut_t_outer_vlanid_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
		&fal_rx_prc_lut_t_inner_vlanid_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_prc_lut_t_bc_flag_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_prc_lut_t_rule_mask_add_cmd);

	cli_install_cmd(CLI_MODE_ENABLE, &g_fal_tx_ctl_filt_t_init_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_ctl_filt_t_match_type_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_ctl_filt_t_match_mask_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_ctl_filt_t_bypass_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_ctl_filt_t_sa_da_addr_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_ctl_filt_t_ether_type_da_range_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &g_fal_tx_mib_t_init_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_mib_t_untagged_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_mib_t_unknown_sa_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_mib_t_ecc_error_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_mib_t_too_long_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_mib_t_ctl_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &g_fal_rx_mib_t_init_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_mib_t_ctrl_prt_fail_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_mib_t_too_long_packets_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_mib_t_untagged_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_mib_t_no_sci_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_rx_mib_t_unctrl_prt_pass_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_mib_t_untagged_hit_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_mib_t_ctrl_prt_pass_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_rx_mib_t_unctrl_prt_fail_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_mib_t_igpoc_ctl_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_mib_t_unknown_sci_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_mib_t_ecc_error_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_mib_t_notag_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_mib_t_bad_tag_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_mib_t_ctl_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_mib_t_tagged_miss_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &g_fal_rx_sa_mib_t_init_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_sa_mib_t_ok_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_sa_mib_t_unused_sa_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_sa_mib_t_late_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_rx_sa_mib_t_decrypted_octets_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_sa_mib_t_delayed_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_sa_mib_t_invalid_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_rx_sa_mib_t_validated_octets_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_rx_sa_mib_t_untagged_hit_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_rx_sa_mib_t_hit_drop_redir_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_sa_mib_t_not_using_sa_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_sa_mib_t_unchecked_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_sa_mib_t_not_valid_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &g_fal_rx_sak_t_init_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_sak_t_sak_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &g_fal_tx_class_lut_t_init_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_sa_mask_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_vlan_up_mask_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_vlan_id_mask_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_udf1_valid_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_ether_type_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_da_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_udf0_valid_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_channel_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_udf3_byte_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_valid_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_vlan_valid_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_class_lut_t_udf1_location_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_sa_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_class_lut_t_vlan_valid_mask_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_class_lut_t_udf2_location_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_da_mask_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_class_lut_t_ether_type_mask_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_udf0_byte_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_class_lut_t_udf0_location_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_udf2_valid_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_udf3_valid_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_vlan_id_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_udf2_byte_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_action_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_class_lut_t_udf3_location_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_vlan_up_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_udf1_byte_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_sci_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_bc_flag_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_tci_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_offset_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
		&fal_tx_class_lut_t_outervlan_id_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_class_lut_t_rule_mask_add_cmd);

	cli_install_cmd(CLI_MODE_ENABLE, &g_fal_tx_sa_mib_t_init_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_sa_mib_t_encrypted_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_sa_mib_t_protected_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_sa_mib_t_protected2_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_sa_mib_t_hit_drop_redirect_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &g_fal_tx_sc_mib_t_init_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_sc_mib_t_protected_octets_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_sc_mib_t_encrypted_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_sc_mib_t_protected_pkts_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_sc_mib_t_encrypted_octets_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_rx_replay_protect_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_rx_replay_protect_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_rx_validate_frame_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_rx_validate_frame_get_cmd);

	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_sak_t_key_len_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_sak_t_sak1_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_sak_t_key_len_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_sak_t_sak1_add_cmd);

	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_xpn_en_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_xpn_en_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
		&nss_macsec_secy_rx_sa_next_xpn_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
		&nss_macsec_secy_rx_sa_next_xpn_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
		&nss_macsec_secy_tx_sa_next_xpn_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
		&nss_macsec_secy_tx_sa_next_xpn_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sc_ssci_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sc_ssci_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sc_ssci_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sc_ssci_set_cmd);

	cli_install_cmd(CLI_MODE_ENABLE, &g_fal_tx_sa_ki_t_init_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &g_fal_rx_sa_ki_t_init_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_sa_ki_t_ki_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_sa_ki_t_ki_add_cmd);

	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sa_ki_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_sa_ki_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sa_ki_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_sa_ki_set_cmd);

	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_udf_ethtype_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_udf_ethtype_set_cmd);

	cli_install_cmd(CLI_MODE_ENABLE,
		&nss_macsec_secy_special_pkt_ctrl_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
		&nss_macsec_secy_special_pkt_ctrl_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
		&nss_macsec_secy_flow_control_en_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
		&nss_macsec_secy_flow_control_en_set_cmd);

	cli_install_cmd(CLI_MODE_ENABLE, &g_fal_tx_udf_filt_t_init_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_udf_filt_t_field0_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_udf_filt_t_field1_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_udf_filt_t_field2_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_udf_filt_t_mask_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_udf_filt_t_type_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_udf_filt_t_offset_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_tx_udf_filt_t_operator_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_udf_filt_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_tx_udf_filt_get_cmd);

	cli_install_cmd(CLI_MODE_ENABLE, &g_fal_rx_udf_filt_t_init_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_udf_filt_t_field0_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_udf_filt_t_field1_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_udf_filt_t_field2_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_udf_filt_t_mask_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_udf_filt_t_type_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_udf_filt_t_offset_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &fal_rx_udf_filt_t_operator_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_udf_filt_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE, &nss_macsec_secy_rx_udf_filt_get_cmd);

	cli_install_cmd(CLI_MODE_ENABLE, &g_fal_tx_udf_ufilt_cfg_t_init_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_udf_ufilt_cfg_t_enable_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_udf_ufilt_cfg_t_priority_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_udf_ufilt_cfg_t_inverse_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_udf_ufilt_cfg_t_pattern0_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_udf_ufilt_cfg_t_pattern1_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_udf_ufilt_cfg_t_pattern2_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_tx_udf_ufilt_cfg_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_tx_udf_ufilt_cfg_get_cmd);

	cli_install_cmd(CLI_MODE_ENABLE, &g_fal_tx_udf_cfilt_cfg_t_init_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_udf_cfilt_cfg_t_enable_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_udf_cfilt_cfg_t_priority_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_udf_cfilt_cfg_t_inverse_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_udf_cfilt_cfg_t_pattern0_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_udf_cfilt_cfg_t_pattern1_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_tx_udf_cfilt_cfg_t_pattern2_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_tx_udf_cfilt_cfg_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_tx_udf_cfilt_cfg_get_cmd);

	cli_install_cmd(CLI_MODE_ENABLE, &g_fal_rx_udf_ufilt_cfg_t_init_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_rx_udf_ufilt_cfg_t_enable_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_rx_udf_ufilt_cfg_t_priority_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_rx_udf_ufilt_cfg_t_inverse_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_rx_udf_ufilt_cfg_t_pattern0_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_rx_udf_ufilt_cfg_t_pattern1_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&fal_rx_udf_ufilt_cfg_t_pattern2_add_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_rx_udf_ufilt_cfg_set_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_rx_udf_ufilt_cfg_get_cmd);
	cli_install_cmd(CLI_MODE_ENABLE,
			&nss_macsec_secy_rx_sa_next_pn_set_cmd);

	return CLI_OK;
}
