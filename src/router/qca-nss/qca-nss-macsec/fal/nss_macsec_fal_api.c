/*
 * Copyright (c) 2014, 2018, The Linux Foundation. All rights reserved.
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

#include "nss_macsec_sdk_api.h"
#include "nss_macsec_interrupt.h"
#include "nss_macsec_mib.h"
#include "nss_macsec_secy.h"
#include "nss_macsec_secy_rx.h"
#include "nss_macsec_secy_tx.h"
#include "nss_macsec_fal_api.h"

unsigned int nss_macsec_cmd_len[] = {
	sizeof(struct nss_macsec_secy_interrupt_en_get_cmd),
	sizeof(struct nss_macsec_secy_interrupt_en_set_cmd),
	sizeof(struct nss_macsec_secy_interrupt_get_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_mib_get_cmd),
	sizeof(struct nss_macsec_secy_tx_sa_mib_get_cmd),
	sizeof(struct nss_macsec_secy_tx_mib_get_cmd),
	sizeof(struct nss_macsec_secy_rx_sa_mib_get_cmd),
	sizeof(struct nss_macsec_secy_rx_mib_get_cmd),
	sizeof(struct nss_macsec_secy_tx_mib_clear_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_mib_clear_cmd),
	sizeof(struct nss_macsec_secy_tx_sa_mib_clear_cmd),
	sizeof(struct nss_macsec_secy_rx_mib_clear_cmd),
	sizeof(struct nss_macsec_secy_rx_sa_mib_clear_cmd),
	sizeof(struct nss_macsec_secy_genl_reg_get_cmd),
	sizeof(struct nss_macsec_secy_genl_reg_set_cmd),
	sizeof(struct nss_macsec_secy_rx_ctl_filt_get_cmd),
	sizeof(struct nss_macsec_secy_rx_ctl_filt_set_cmd),
	sizeof(struct nss_macsec_secy_rx_ctl_filt_clear_cmd),
	sizeof(struct nss_macsec_secy_rx_ctl_filt_clear_all_cmd),
	sizeof(struct nss_macsec_secy_rx_prc_lut_get_cmd),
	sizeof(struct nss_macsec_secy_rx_prc_lut_set_cmd),
	sizeof(struct nss_macsec_secy_rx_prc_lut_clear_cmd),
	sizeof(struct nss_macsec_secy_rx_prc_lut_clear_all_cmd),
	sizeof(struct nss_macsec_secy_rx_sc_create_cmd),
	sizeof(struct nss_macsec_secy_rx_sc_en_get_cmd),
	sizeof(struct nss_macsec_secy_rx_sc_en_set_cmd),
	sizeof(struct nss_macsec_secy_rx_sc_del_cmd),
	sizeof(struct nss_macsec_secy_rx_sc_del_all_cmd),
	sizeof(struct nss_macsec_secy_rx_sc_validate_frame_get_cmd),
	sizeof(struct nss_macsec_secy_rx_sc_validate_frame_set_cmd),
	sizeof(struct nss_macsec_secy_rx_sc_replay_protect_get_cmd),
	sizeof(struct nss_macsec_secy_rx_sc_replay_protect_set_cmd),
	sizeof(struct nss_macsec_secy_rx_sc_anti_replay_window_get_cmd),
	sizeof(struct nss_macsec_secy_rx_sc_anti_replay_window_set_cmd),
	sizeof(struct nss_macsec_secy_rx_sc_in_used_get_cmd),
	sizeof(struct nss_macsec_secy_rx_sa_create_cmd),
	sizeof(struct nss_macsec_secy_rx_sa_en_get_cmd),
	sizeof(struct nss_macsec_secy_rx_sa_en_set_cmd),
	sizeof(struct nss_macsec_secy_rx_sa_next_pn_get_cmd),
	sizeof(struct nss_macsec_secy_rx_sa_del_cmd),
	sizeof(struct nss_macsec_secy_rx_sa_del_all_cmd),
	sizeof(struct nss_macsec_secy_rx_sak_get_cmd),
	sizeof(struct nss_macsec_secy_rx_sak_set_cmd),
	sizeof(struct nss_macsec_secy_rx_sa_in_used_get_cmd),
	sizeof(struct nss_macsec_secy_rx_replay_protect_get_cmd),
	sizeof(struct nss_macsec_secy_rx_replay_protect_set_cmd),
	sizeof(struct nss_macsec_secy_rx_validate_frame_get_cmd),
	sizeof(struct nss_macsec_secy_rx_validate_frame_set_cmd),
	sizeof(struct nss_macsec_secy_ext_reg_get_cmd),
	sizeof(struct nss_macsec_secy_ext_reg_set_cmd),
	sizeof(struct nss_macsec_secy_tx_ctl_filt_get_cmd),
	sizeof(struct nss_macsec_secy_tx_ctl_filt_set_cmd),
	sizeof(struct nss_macsec_secy_tx_ctl_filt_clear_cmd),
	sizeof(struct nss_macsec_secy_tx_ctl_filt_clear_all_cmd),
	sizeof(struct nss_macsec_secy_tx_class_lut_get_cmd),
	sizeof(struct nss_macsec_secy_tx_class_lut_set_cmd),
	sizeof(struct nss_macsec_secy_tx_class_lut_clear_cmd),
	sizeof(struct nss_macsec_secy_tx_class_lut_clear_all_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_create_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_en_get_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_en_set_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_del_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_del_all_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_an_get_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_an_set_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_in_used_get_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_tci_7_2_get_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_tci_7_2_set_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_confidentiality_offset_get_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_confidentiality_offset_set_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_protect_get_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_protect_set_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_sci_get_cmd),
	sizeof(struct nss_macsec_secy_tx_sa_create_cmd),
	sizeof(struct nss_macsec_secy_tx_sa_en_get_cmd),
	sizeof(struct nss_macsec_secy_tx_sa_en_set_cmd),
	sizeof(struct nss_macsec_secy_tx_sa_del_cmd),
	sizeof(struct nss_macsec_secy_tx_sa_del_all_cmd),
	sizeof(struct nss_macsec_secy_tx_sa_next_pn_get_cmd),
	sizeof(struct nss_macsec_secy_tx_sa_next_pn_set_cmd),
	sizeof(struct nss_macsec_secy_tx_sa_in_used_get_cmd),
	sizeof(struct nss_macsec_secy_tx_sak_get_cmd),
	sizeof(struct nss_macsec_secy_tx_sak_set_cmd),
	sizeof(struct nss_macsec_secy_sc_sa_mapping_mode_get_cmd),
	sizeof(struct nss_macsec_secy_sc_sa_mapping_mode_set_cmd),
	sizeof(struct nss_macsec_secy_controlled_port_en_get_cmd),
	sizeof(struct nss_macsec_secy_controlled_port_en_set_cmd),
	sizeof(struct nss_macsec_secy_cipher_suite_get_cmd),
	sizeof(struct nss_macsec_secy_cipher_suite_set_cmd),
	sizeof(struct nss_macsec_secy_mtu_get_cmd),
	sizeof(struct nss_macsec_secy_mtu_set_cmd),
	sizeof(struct nss_macsec_secy_id_get_cmd),
	sizeof(struct nss_macsec_secy_en_get_cmd),
	sizeof(struct nss_macsec_secy_en_set_cmd),
	sizeof(struct nss_macsec_secy_xpn_en_get_cmd),
	sizeof(struct nss_macsec_secy_xpn_en_set_cmd),
	sizeof(struct nss_macsec_secy_rx_sa_next_xpn_get_cmd),
	sizeof(struct nss_macsec_secy_rx_sa_next_xpn_set_cmd),
	sizeof(struct nss_macsec_secy_tx_sa_next_xpn_get_cmd),
	sizeof(struct nss_macsec_secy_tx_sa_next_xpn_set_cmd),
	sizeof(struct nss_macsec_secy_rx_sc_ssci_get_cmd),
	sizeof(struct nss_macsec_secy_rx_sc_ssci_set_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_ssci_get_cmd),
	sizeof(struct nss_macsec_secy_tx_sc_ssci_get_cmd),
	sizeof(struct nss_macsec_secy_rx_sa_ki_get_cmd),
	sizeof(struct nss_macsec_secy_rx_sa_ki_set_cmd),
	sizeof(struct nss_macsec_secy_tx_sa_ki_get_cmd),
	sizeof(struct nss_macsec_secy_tx_sa_ki_set_cmd),
	sizeof(struct nss_macsec_secy_flow_control_en_get_cmd),
	sizeof(struct nss_macsec_secy_flow_control_en_set_cmd),
	sizeof(struct nss_macsec_secy_special_pkt_ctrl_get_cmd),
	sizeof(struct nss_macsec_secy_special_pkt_ctrl_set_cmd),
	sizeof(struct nss_macsec_secy_udf_ethtype_get_cmd),
	sizeof(struct nss_macsec_secy_udf_ethtype_set_cmd),
	sizeof(struct nss_macsec_secy_tx_udf_filt_set_cmd),
	sizeof(struct nss_macsec_secy_tx_udf_filt_get_cmd),
	sizeof(struct nss_macsec_secy_rx_udf_filt_set_cmd),
	sizeof(struct nss_macsec_secy_rx_udf_filt_get_cmd),
	sizeof(struct nss_macsec_secy_tx_udf_ufilt_cfg_set_cmd),
	sizeof(struct nss_macsec_secy_tx_udf_ufilt_cfg_get_cmd),
	sizeof(struct nss_macsec_secy_tx_udf_cfilt_cfg_set_cmd),
	sizeof(struct nss_macsec_secy_tx_udf_cfilt_cfg_get_cmd),
	sizeof(struct nss_macsec_secy_rx_udf_ufilt_cfg_set_cmd),
	sizeof(struct nss_macsec_secy_rx_udf_ufilt_cfg_get_cmd),
	sizeof(struct nss_macsec_secy_rx_sa_next_pn_set_cmd),
};

unsigned int nss_macsec_fal_msg_handle(struct sdk_msg_header *header)
{
	unsigned int ret = SDK_RET_NOT_SUPPORT;

	if(header->sub_type >= sizeof(nss_macsec_cmd_len)/sizeof(nss_macsec_cmd_len[0])) {
		printk("sub_type:0x%x is unvalid\n", header->sub_type);
		return SDK_RET_PARAM_ERR;
	}

	if(header->buf_len != nss_macsec_cmd_len[header->sub_type]) {
		printk("buf_len:0x%x is unvalid for command sub_type:0x%x\n", header->buf_len, header->sub_type);
		return SDK_RET_PARAM_ERR;
	}

	switch (header->sub_type) {
	case NSS_MACSEC_SECY_INTERRUPT_EN_GET_CMD:{
			struct nss_macsec_secy_interrupt_en_get_cmd *param =
			    (struct nss_macsec_secy_interrupt_en_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_interrupt_en_get(param->secy_id,
							     &param->p_int_en);
		}
		break;
	case NSS_MACSEC_SECY_INTERRUPT_EN_SET_CMD:{
			struct nss_macsec_secy_interrupt_en_set_cmd *param =
			    (struct nss_macsec_secy_interrupt_en_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_interrupt_en_set(param->secy_id,
							     &param->p_int_en);
		}
		break;
	case NSS_MACSEC_SECY_INTERRUPT_GET_CMD:{
			struct nss_macsec_secy_interrupt_get_cmd *param =
			    (struct nss_macsec_secy_interrupt_get_cmd *)(header
									 + 1);
			ret =
			    nss_macsec_secy_interrupt_get(param->secy_id,
							  &param->pint);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_MIB_GET_CMD:{
			struct nss_macsec_secy_tx_sc_mib_get_cmd *param =
			    (struct nss_macsec_secy_tx_sc_mib_get_cmd *)(header
									 + 1);
			ret =
			    nss_macsec_secy_tx_sc_mib_get(param->secy_id,
							  param->channel,
							  &param->pmib);
		}
		break;
	case NSS_MACSEC_SECY_TX_SA_MIB_GET_CMD:{
			struct nss_macsec_secy_tx_sa_mib_get_cmd *param =
			    (struct nss_macsec_secy_tx_sa_mib_get_cmd *)(header
									 + 1);
			ret =
			    nss_macsec_secy_tx_sa_mib_get(param->secy_id,
							  param->channel,
							  param->an,
							  &param->pmib);
		}
		break;
	case NSS_MACSEC_SECY_TX_MIB_GET_CMD:{
			struct nss_macsec_secy_tx_mib_get_cmd *param =
			    (struct nss_macsec_secy_tx_mib_get_cmd *)(header +
								      1);
			ret =
			    nss_macsec_secy_tx_mib_get(param->secy_id,
						       &param->pmib);
		}
		break;
	case NSS_MACSEC_SECY_RX_SA_MIB_GET_CMD:{
			struct nss_macsec_secy_rx_sa_mib_get_cmd *param =
			    (struct nss_macsec_secy_rx_sa_mib_get_cmd *)(header
									 + 1);
			ret =
			    nss_macsec_secy_rx_sa_mib_get(param->secy_id,
							  param->channel,
							  param->an,
							  &param->pmib);
		}
		break;
	case NSS_MACSEC_SECY_RX_MIB_GET_CMD:{
			struct nss_macsec_secy_rx_mib_get_cmd *param =
			    (struct nss_macsec_secy_rx_mib_get_cmd *)(header +
								      1);
			ret =
			    nss_macsec_secy_rx_mib_get(param->secy_id,
						       &param->pmib);
		}
		break;
	case NSS_MACSEC_SECY_TX_MIB_CLEAR_CMD:{
			struct nss_macsec_secy_tx_mib_clear_cmd *param =
			    (struct nss_macsec_secy_tx_mib_clear_cmd *)(header +
									1);
			ret = nss_macsec_secy_tx_mib_clear(param->secy_id);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_MIB_CLEAR_CMD:{
			struct nss_macsec_secy_tx_sc_mib_clear_cmd *param =
			    (struct nss_macsec_secy_tx_sc_mib_clear_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_sc_mib_clear(param->secy_id,
							    param->channel);
		}
		break;
	case NSS_MACSEC_SECY_TX_SA_MIB_CLEAR_CMD:{
			struct nss_macsec_secy_tx_sa_mib_clear_cmd *param =
			    (struct nss_macsec_secy_tx_sa_mib_clear_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_sa_mib_clear(param->secy_id,
							    param->channel,
							    param->an);
		}
		break;
	case NSS_MACSEC_SECY_RX_MIB_CLEAR_CMD:{
			struct nss_macsec_secy_rx_mib_clear_cmd *param =
			    (struct nss_macsec_secy_rx_mib_clear_cmd *)(header +
									1);
			ret = nss_macsec_secy_rx_mib_clear(param->secy_id);
		}
		break;
	case NSS_MACSEC_SECY_RX_SA_MIB_CLEAR_CMD:{
			struct nss_macsec_secy_rx_sa_mib_clear_cmd *param =
			    (struct nss_macsec_secy_rx_sa_mib_clear_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_sa_mib_clear(param->secy_id,
							    param->channel,
							    param->an);
		}
		break;
	case NSS_MACSEC_SECY_GENL_REG_GET_CMD:{
			struct nss_macsec_secy_genl_reg_get_cmd *param =
			    (struct nss_macsec_secy_genl_reg_get_cmd *)(header +
								      1);
			ret =
			    nss_macsec_secy_genl_reg_get(param->secy_id,
						       param->addr,
						       &param->pvalue);
		}
		break;
	case NSS_MACSEC_SECY_GENL_REG_SET_CMD:{
			struct nss_macsec_secy_genl_reg_set_cmd *param =
			    (struct nss_macsec_secy_genl_reg_set_cmd *)(header +
								      1);
			ret =
			    nss_macsec_secy_genl_reg_set(param->secy_id,
						       param->addr,
						       param->value);
		}
		break;
	case NSS_MACSEC_SECY_RX_CTL_FILT_GET_CMD:{
			struct nss_macsec_secy_rx_ctl_filt_get_cmd *param =
			    (struct nss_macsec_secy_rx_ctl_filt_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_ctl_filt_get(param->secy_id,
							    param->filt_id,
							    &param->pfilt);
		}
		break;
	case NSS_MACSEC_SECY_RX_CTL_FILT_SET_CMD:{
			struct nss_macsec_secy_rx_ctl_filt_set_cmd *param =
			    (struct nss_macsec_secy_rx_ctl_filt_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_ctl_filt_set(param->secy_id,
							    param->filt_id,
							    &param->pfilt);
		}
		break;
	case NSS_MACSEC_SECY_RX_CTL_FILT_CLEAR_CMD:{
			struct nss_macsec_secy_rx_ctl_filt_clear_cmd *param =
			    (struct nss_macsec_secy_rx_ctl_filt_clear_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_ctl_filt_clear(param->secy_id,
							      param->filt_id);
		}
		break;
	case NSS_MACSEC_SECY_RX_CTL_FILT_CLEAR_ALL_CMD:{
			struct nss_macsec_secy_rx_ctl_filt_clear_all_cmd *param
			    =
			    (struct nss_macsec_secy_rx_ctl_filt_clear_all_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_ctl_filt_clear_all(param->
								  secy_id);
		}
		break;
	case NSS_MACSEC_SECY_RX_PRC_LUT_GET_CMD:{
			struct nss_macsec_secy_rx_prc_lut_get_cmd *param =
			    (struct nss_macsec_secy_rx_prc_lut_get_cmd *)(header
									  + 1);
			ret =
			    nss_macsec_secy_rx_prc_lut_get(param->secy_id,
							   param->index,
							   &param->pentry);
		}
		break;
	case NSS_MACSEC_SECY_RX_PRC_LUT_SET_CMD:{
			struct nss_macsec_secy_rx_prc_lut_set_cmd *param =
			    (struct nss_macsec_secy_rx_prc_lut_set_cmd *)(header
									  + 1);
			ret =
			    nss_macsec_secy_rx_prc_lut_set(param->secy_id,
							   param->index,
							   &param->pentry);
		}
		break;
	case NSS_MACSEC_SECY_RX_PRC_LUT_CLEAR_CMD:{
			struct nss_macsec_secy_rx_prc_lut_clear_cmd *param =
			    (struct nss_macsec_secy_rx_prc_lut_clear_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_prc_lut_clear(param->secy_id,
							     param->index);
		}
		break;
	case NSS_MACSEC_SECY_RX_PRC_LUT_CLEAR_ALL_CMD:{
			struct nss_macsec_secy_rx_prc_lut_clear_all_cmd *param =
			    (struct nss_macsec_secy_rx_prc_lut_clear_all_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_prc_lut_clear_all(param->
								 secy_id);
		}
		break;
	case NSS_MACSEC_SECY_RX_SC_CREATE_CMD:{
			struct nss_macsec_secy_rx_sc_create_cmd *param =
			    (struct nss_macsec_secy_rx_sc_create_cmd *)(header +
									1);
			ret =
			    nss_macsec_secy_rx_sc_create(param->secy_id,
							 param->channel);
		}
		break;
	case NSS_MACSEC_SECY_RX_SC_EN_GET_CMD:{
			struct nss_macsec_secy_rx_sc_en_get_cmd *param =
			    (struct nss_macsec_secy_rx_sc_en_get_cmd *)(header +
									1);
			ret =
			    nss_macsec_secy_rx_sc_en_get(param->secy_id,
							 param->channel,
							 &param->penable);
		}
		break;
	case NSS_MACSEC_SECY_RX_SC_EN_SET_CMD:{
			struct nss_macsec_secy_rx_sc_en_set_cmd *param =
			    (struct nss_macsec_secy_rx_sc_en_set_cmd *)(header +
									1);
			ret =
			    nss_macsec_secy_rx_sc_en_set(param->secy_id,
							 param->channel,
							 param->enable);
		}
		break;
	case NSS_MACSEC_SECY_RX_SC_DEL_CMD:{
			struct nss_macsec_secy_rx_sc_del_cmd *param =
			    (struct nss_macsec_secy_rx_sc_del_cmd *)(header +
								     1);
			ret =
			    nss_macsec_secy_rx_sc_del(param->secy_id,
						      param->channel);
		}
		break;
	case NSS_MACSEC_SECY_RX_SC_DEL_ALL_CMD:{
			struct nss_macsec_secy_rx_sc_del_all_cmd *param =
			    (struct nss_macsec_secy_rx_sc_del_all_cmd *)(header
									 + 1);
			ret = nss_macsec_secy_rx_sc_del_all(param->secy_id);
		}
		break;
	case NSS_MACSEC_SECY_RX_SC_VALIDATE_FRAME_GET_CMD:{
			struct nss_macsec_secy_rx_sc_validate_frame_get_cmd
			    *param =
			    (struct nss_macsec_secy_rx_sc_validate_frame_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_sc_validate_frame_get(param->
								     secy_id,
								     param->
								     channel,
								     &param->
								     pmode);
		}
		break;
	case NSS_MACSEC_SECY_RX_SC_VALIDATE_FRAME_SET_CMD:{
			struct nss_macsec_secy_rx_sc_validate_frame_set_cmd
			    *param =
			    (struct nss_macsec_secy_rx_sc_validate_frame_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_sc_validate_frame_set(param->
								     secy_id,
								     param->
								     channel,
								     param->
								     mode);
		}
		break;
	case NSS_MACSEC_SECY_RX_SC_REPLAY_PROTECT_GET_CMD:{
			struct nss_macsec_secy_rx_sc_replay_protect_get_cmd
			    *param =
			    (struct nss_macsec_secy_rx_sc_replay_protect_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_sc_replay_protect_get(param->
								     secy_id,
								     param->
								     channel,
								     &param->
								     penable);
		}
		break;
	case NSS_MACSEC_SECY_RX_SC_REPLAY_PROTECT_SET_CMD:{
			struct nss_macsec_secy_rx_sc_replay_protect_set_cmd
			    *param =
			    (struct nss_macsec_secy_rx_sc_replay_protect_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_sc_replay_protect_set(param->
								     secy_id,
								     param->
								     channel,
								     param->
								     enable);
		}
		break;
	case NSS_MACSEC_SECY_RX_SC_ANTI_REPLAY_WINDOW_GET_CMD:{
			struct nss_macsec_secy_rx_sc_anti_replay_window_get_cmd
			    *param =
			    (struct
			     nss_macsec_secy_rx_sc_anti_replay_window_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_sc_anti_replay_window_get(param->
									 secy_id,
									 param->
									 channel,
									 &param->
									 pwindow);
		}
		break;
	case NSS_MACSEC_SECY_RX_SC_ANTI_REPLAY_WINDOW_SET_CMD:{
			struct nss_macsec_secy_rx_sc_anti_replay_window_set_cmd
			    *param =
			    (struct
			     nss_macsec_secy_rx_sc_anti_replay_window_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_sc_anti_replay_window_set(param->
									 secy_id,
									 param->
									 channel,
									 param->
									 window);
		}
		break;
	case NSS_MACSEC_SECY_RX_SC_IN_USED_GET_CMD:{
			struct nss_macsec_secy_rx_sc_in_used_get_cmd *param =
			    (struct nss_macsec_secy_rx_sc_in_used_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_sc_in_used_get(param->secy_id,
							      param->channel,
							      &param->
							      p_in_used);
		}
		break;
	case NSS_MACSEC_SECY_RX_SA_CREATE_CMD:{
			struct nss_macsec_secy_rx_sa_create_cmd *param =
			    (struct nss_macsec_secy_rx_sa_create_cmd *)(header +
									1);
			ret =
			    nss_macsec_secy_rx_sa_create(param->secy_id,
							 param->channel,
							 param->an);
		}
		break;
	case NSS_MACSEC_SECY_RX_SA_EN_GET_CMD:{
			struct nss_macsec_secy_rx_sa_en_get_cmd *param =
			    (struct nss_macsec_secy_rx_sa_en_get_cmd *)(header +
									1);
			ret =
			    nss_macsec_secy_rx_sa_en_get(param->secy_id,
							 param->channel,
							 param->an,
							 &param->penable);
		}
		break;
	case NSS_MACSEC_SECY_RX_SA_EN_SET_CMD:{
			struct nss_macsec_secy_rx_sa_en_set_cmd *param =
			    (struct nss_macsec_secy_rx_sa_en_set_cmd *)(header +
									1);
			ret =
			    nss_macsec_secy_rx_sa_en_set(param->secy_id,
							 param->channel,
							 param->an,
							 param->enable);
		}
		break;
	case NSS_MACSEC_SECY_RX_SA_NEXT_PN_GET_CMD:{
			struct nss_macsec_secy_rx_sa_next_pn_get_cmd *param =
			    (struct nss_macsec_secy_rx_sa_next_pn_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_sa_next_pn_get(param->secy_id,
							      param->channel,
							      param->an,
							      &param->pnpn);
		}
		break;
	case NSS_MACSEC_SECY_RX_SA_DEL_CMD:{
			struct nss_macsec_secy_rx_sa_del_cmd *param =
			    (struct nss_macsec_secy_rx_sa_del_cmd *)(header +
								     1);
			ret =
			    nss_macsec_secy_rx_sa_del(param->secy_id,
						      param->channel,
						      param->an);
		}
		break;
	case NSS_MACSEC_SECY_RX_SA_DEL_ALL_CMD:{
			struct nss_macsec_secy_rx_sa_del_all_cmd *param =
			    (struct nss_macsec_secy_rx_sa_del_all_cmd *)(header
									 + 1);
			ret = nss_macsec_secy_rx_sa_del_all(param->secy_id);
		}
		break;
	case NSS_MACSEC_SECY_RX_SAK_GET_CMD:{
			struct nss_macsec_secy_rx_sak_get_cmd *param =
			    (struct nss_macsec_secy_rx_sak_get_cmd *)(header +
								      1);
			ret =
			    nss_macsec_secy_rx_sak_get(param->secy_id,
						       param->channel,
						       param->an, &param->pkey);
		}
		break;
	case NSS_MACSEC_SECY_RX_SAK_SET_CMD:{
			struct nss_macsec_secy_rx_sak_set_cmd *param =
			    (struct nss_macsec_secy_rx_sak_set_cmd *)(header +
								      1);
			ret =
			    nss_macsec_secy_rx_sak_set(param->secy_id,
						       param->channel,
						       param->an, &param->pkey);
		}
		break;
	case NSS_MACSEC_SECY_RX_SA_IN_USED_GET_CMD:{
			struct nss_macsec_secy_rx_sa_in_used_get_cmd *param =
			    (struct nss_macsec_secy_rx_sa_in_used_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_sa_in_used_get(param->secy_id,
							      param->channel,
							      param->an,
							      &param->
							      p_in_used);
		}
		break;
	case NSS_MACSEC_SECY_RX_REPLAY_PROTECT_GET_CMD:{
			struct nss_macsec_secy_rx_replay_protect_get_cmd *param
			    =
			    (struct nss_macsec_secy_rx_replay_protect_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_replay_protect_get(param->
								  secy_id,
								  &param->
								  enable);
		}
		break;
	case NSS_MACSEC_SECY_RX_REPLAY_PROTECT_SET_CMD:{
			struct nss_macsec_secy_rx_replay_protect_set_cmd *param
			    =
			    (struct nss_macsec_secy_rx_replay_protect_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_replay_protect_set(param->
								  secy_id,
								  param->
								  enable);
		}
		break;
	case NSS_MACSEC_SECY_RX_VALIDATE_FRAME_GET_CMD:{
			struct nss_macsec_secy_rx_validate_frame_get_cmd *param
			    =
			    (struct nss_macsec_secy_rx_validate_frame_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_validate_frame_get(param->
								  secy_id,
								  &param->mode);
		}
		break;
	case NSS_MACSEC_SECY_RX_VALIDATE_FRAME_SET_CMD:{
			struct nss_macsec_secy_rx_validate_frame_set_cmd *param
			    =
			    (struct nss_macsec_secy_rx_validate_frame_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_validate_frame_set(param->
								  secy_id,
								  param->mode);
		}
		break;
	case NSS_MACSEC_SECY_EXT_REG_GET_CMD:{
			struct nss_macsec_secy_ext_reg_get_cmd *param =
			    (struct nss_macsec_secy_ext_reg_get_cmd *)(header +
								      1);
			ret =
			    nss_macsec_secy_ext_reg_get(param->secy_id,
						       param->addr,
						       &param->pvalue);
		}
		break;
	case NSS_MACSEC_SECY_EXT_REG_SET_CMD:{
			struct nss_macsec_secy_ext_reg_set_cmd *param =
			    (struct nss_macsec_secy_ext_reg_set_cmd *)(header +
								      1);
			ret =
			    nss_macsec_secy_ext_reg_set(param->secy_id,
						       param->addr,
						       param->value);
		}
		break;
	case NSS_MACSEC_SECY_TX_CTL_FILT_GET_CMD:{
			struct nss_macsec_secy_tx_ctl_filt_get_cmd *param =
			    (struct nss_macsec_secy_tx_ctl_filt_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_ctl_filt_get(param->secy_id,
							    param->filt_id,
							    &param->pfilt);
		}
		break;
	case NSS_MACSEC_SECY_TX_CTL_FILT_SET_CMD:{
			struct nss_macsec_secy_tx_ctl_filt_set_cmd *param =
			    (struct nss_macsec_secy_tx_ctl_filt_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_ctl_filt_set(param->secy_id,
							    param->filt_id,
							    &param->pfilt);
		}
		break;
	case NSS_MACSEC_SECY_TX_CTL_FILT_CLEAR_CMD:{
			struct nss_macsec_secy_tx_ctl_filt_clear_cmd *param =
			    (struct nss_macsec_secy_tx_ctl_filt_clear_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_ctl_filt_clear(param->secy_id,
							      param->filt_id);
		}
		break;
	case NSS_MACSEC_SECY_TX_CTL_FILT_CLEAR_ALL_CMD:{
			struct nss_macsec_secy_tx_ctl_filt_clear_all_cmd *param
			    =
			    (struct nss_macsec_secy_tx_ctl_filt_clear_all_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_ctl_filt_clear_all(param->
								  secy_id);
		}
		break;
	case NSS_MACSEC_SECY_TX_CLASS_LUT_GET_CMD:{
			struct nss_macsec_secy_tx_class_lut_get_cmd *param =
			    (struct nss_macsec_secy_tx_class_lut_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_class_lut_get(param->secy_id,
							     param->index,
							     &param->pentry);
		}
		break;
	case NSS_MACSEC_SECY_TX_CLASS_LUT_SET_CMD:{
			struct nss_macsec_secy_tx_class_lut_set_cmd *param =
			    (struct nss_macsec_secy_tx_class_lut_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_class_lut_set(param->secy_id,
							     param->index,
							     &param->pentry);
		}
		break;
	case NSS_MACSEC_SECY_TX_CLASS_LUT_CLEAR_CMD:{
			struct nss_macsec_secy_tx_class_lut_clear_cmd *param =
			    (struct nss_macsec_secy_tx_class_lut_clear_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_class_lut_clear(param->secy_id,
							       param->index);
		}
		break;
	case NSS_MACSEC_SECY_TX_CLASS_LUT_CLEAR_ALL_CMD:{
			struct nss_macsec_secy_tx_class_lut_clear_all_cmd *param
			    =
			    (struct nss_macsec_secy_tx_class_lut_clear_all_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_class_lut_clear_all(param->
								   secy_id);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_CREATE_CMD:{
			struct nss_macsec_secy_tx_sc_create_cmd *param =
			    (struct nss_macsec_secy_tx_sc_create_cmd *)(header +
									1);
			ret =
			    nss_macsec_secy_tx_sc_create(param->secy_id,
							 param->channel,
							 param->psci,
							 param->sci_len);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_EN_GET_CMD:{
			struct nss_macsec_secy_tx_sc_en_get_cmd *param =
			    (struct nss_macsec_secy_tx_sc_en_get_cmd *)(header +
									1);
			ret =
			    nss_macsec_secy_tx_sc_en_get(param->secy_id,
							 param->channel,
							 &param->penable);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_EN_SET_CMD:{
			struct nss_macsec_secy_tx_sc_en_set_cmd *param =
			    (struct nss_macsec_secy_tx_sc_en_set_cmd *)(header +
									1);
			ret =
			    nss_macsec_secy_tx_sc_en_set(param->secy_id,
							 param->channel,
							 param->enable);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_DEL_CMD:{
			struct nss_macsec_secy_tx_sc_del_cmd *param =
			    (struct nss_macsec_secy_tx_sc_del_cmd *)(header +
								     1);
			ret =
			    nss_macsec_secy_tx_sc_del(param->secy_id,
						      param->channel);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_DEL_ALL_CMD:{
			struct nss_macsec_secy_tx_sc_del_all_cmd *param =
			    (struct nss_macsec_secy_tx_sc_del_all_cmd *)(header
									 + 1);
			ret = nss_macsec_secy_tx_sc_del_all(param->secy_id);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_AN_GET_CMD:{
			struct nss_macsec_secy_tx_sc_an_get_cmd *param =
			    (struct nss_macsec_secy_tx_sc_an_get_cmd *)(header +
									1);
			ret =
			    nss_macsec_secy_tx_sc_an_get(param->secy_id,
							 param->channel,
							 &param->pan);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_AN_SET_CMD:{
			struct nss_macsec_secy_tx_sc_an_set_cmd *param =
			    (struct nss_macsec_secy_tx_sc_an_set_cmd *)(header +
									1);
			ret =
			    nss_macsec_secy_tx_sc_an_set(param->secy_id,
							 param->channel,
							 param->an);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_IN_USED_GET_CMD:{
			struct nss_macsec_secy_tx_sc_in_used_get_cmd *param =
			    (struct nss_macsec_secy_tx_sc_in_used_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_sc_in_used_get(param->secy_id,
							      param->channel,
							      &param->
							      p_in_used);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_TCI_7_2_GET_CMD:{
			struct nss_macsec_secy_tx_sc_tci_7_2_get_cmd *param =
			    (struct nss_macsec_secy_tx_sc_tci_7_2_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_sc_tci_7_2_get(param->secy_id,
							      param->channel,
							      &param->ptci);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_TCI_7_2_SET_CMD:{
			struct nss_macsec_secy_tx_sc_tci_7_2_set_cmd *param =
			    (struct nss_macsec_secy_tx_sc_tci_7_2_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_sc_tci_7_2_set(param->secy_id,
							      param->channel,
							      param->tci);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_CONFIDENTIALITY_OFFSET_GET_CMD:{
			struct
			    nss_macsec_secy_tx_sc_confidentiality_offset_get_cmd
			    *param =
			    (struct
			     nss_macsec_secy_tx_sc_confidentiality_offset_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_sc_confidentiality_offset_get
			    (param->secy_id, param->channel, &param->poffset);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_CONFIDENTIALITY_OFFSET_SET_CMD:{
			struct
			    nss_macsec_secy_tx_sc_confidentiality_offset_set_cmd
			    *param =
			    (struct
			     nss_macsec_secy_tx_sc_confidentiality_offset_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_sc_confidentiality_offset_set
			    (param->secy_id, param->channel, param->offset);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_PROTECT_GET_CMD:{
			struct nss_macsec_secy_tx_sc_protect_get_cmd *param =
			    (struct nss_macsec_secy_tx_sc_protect_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_sc_protect_get(param->secy_id,
							      param->channel,
							      &param->penable);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_PROTECT_SET_CMD:{
			struct nss_macsec_secy_tx_sc_protect_set_cmd *param =
			    (struct nss_macsec_secy_tx_sc_protect_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_sc_protect_set(param->secy_id,
							      param->channel,
							      param->enable);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_SCI_GET_CMD:{
			struct nss_macsec_secy_tx_sc_sci_get_cmd *param =
			    (struct nss_macsec_secy_tx_sc_sci_get_cmd *)(header
									 + 1);
			ret =
			    nss_macsec_secy_tx_sc_sci_get(param->secy_id,
							  param->channel,
							  param->psci,
							  param->sci_len);
		}
		break;
	case NSS_MACSEC_SECY_TX_SA_CREATE_CMD:{
			struct nss_macsec_secy_tx_sa_create_cmd *param =
			    (struct nss_macsec_secy_tx_sa_create_cmd *)(header +
									1);
			ret =
			    nss_macsec_secy_tx_sa_create(param->secy_id,
							 param->channel,
							 param->an);
		}
		break;
	case NSS_MACSEC_SECY_TX_SA_EN_GET_CMD:{
			struct nss_macsec_secy_tx_sa_en_get_cmd *param =
			    (struct nss_macsec_secy_tx_sa_en_get_cmd *)(header +
									1);
			ret =
			    nss_macsec_secy_tx_sa_en_get(param->secy_id,
							 param->channel,
							 param->an,
							 &param->penable);
		}
		break;
	case NSS_MACSEC_SECY_TX_SA_EN_SET_CMD:{
			struct nss_macsec_secy_tx_sa_en_set_cmd *param =
			    (struct nss_macsec_secy_tx_sa_en_set_cmd *)(header +
									1);
			ret =
			    nss_macsec_secy_tx_sa_en_set(param->secy_id,
							 param->channel,
							 param->an,
							 param->enable);
		}
		break;
	case NSS_MACSEC_SECY_TX_SA_DEL_CMD:{
			struct nss_macsec_secy_tx_sa_del_cmd *param =
			    (struct nss_macsec_secy_tx_sa_del_cmd *)(header +
								     1);
			ret =
			    nss_macsec_secy_tx_sa_del(param->secy_id,
						      param->channel,
						      param->an);
		}
		break;
	case NSS_MACSEC_SECY_TX_SA_DEL_ALL_CMD:{
			struct nss_macsec_secy_tx_sa_del_all_cmd *param =
			    (struct nss_macsec_secy_tx_sa_del_all_cmd *)(header
									 + 1);
			ret = nss_macsec_secy_tx_sa_del_all(param->secy_id);
		}
		break;
	case NSS_MACSEC_SECY_TX_SA_NEXT_PN_GET_CMD:{
			struct nss_macsec_secy_tx_sa_next_pn_get_cmd *param =
			    (struct nss_macsec_secy_tx_sa_next_pn_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_sa_next_pn_get(param->secy_id,
							      param->channel,
							      param->an,
							      &param->
							      p_next_pn);
		}
		break;
	case NSS_MACSEC_SECY_TX_SA_NEXT_PN_SET_CMD:{
			struct nss_macsec_secy_tx_sa_next_pn_set_cmd *param =
			    (struct nss_macsec_secy_tx_sa_next_pn_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_sa_next_pn_set(param->secy_id,
							      param->channel,
							      param->an,
							      param->next_pn);
		}
		break;
	case NSS_MACSEC_SECY_TX_SA_IN_USED_GET_CMD:{
			struct nss_macsec_secy_tx_sa_in_used_get_cmd *param =
			    (struct nss_macsec_secy_tx_sa_in_used_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_sa_in_used_get(param->secy_id,
							      param->channel,
							      param->an,
							      &param->
							      p_in_used);
		}
		break;
	case NSS_MACSEC_SECY_TX_SAK_GET_CMD:{
			struct nss_macsec_secy_tx_sak_get_cmd *param =
			    (struct nss_macsec_secy_tx_sak_get_cmd *)(header +
								      1);
			ret =
			    nss_macsec_secy_tx_sak_get(param->secy_id,
						       param->channel,
						       param->an,
						       &param->pentry);
		}
		break;
	case NSS_MACSEC_SECY_TX_SAK_SET_CMD:{
			struct nss_macsec_secy_tx_sak_set_cmd *param =
			    (struct nss_macsec_secy_tx_sak_set_cmd *)(header +
								      1);
			ret =
			    nss_macsec_secy_tx_sak_set(param->secy_id,
						       param->channel,
						       param->an,
						       &param->pentry);
		}
		break;
	case NSS_MACSEC_SECY_SC_SA_MAPPING_MODE_GET_CMD:{
			struct nss_macsec_secy_sc_sa_mapping_mode_get_cmd *param
			    =
			    (struct nss_macsec_secy_sc_sa_mapping_mode_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_sc_sa_mapping_mode_get(param->
								   secy_id,
								   &param->
								   pmode);
		}
		break;
	case NSS_MACSEC_SECY_SC_SA_MAPPING_MODE_SET_CMD:{
			struct nss_macsec_secy_sc_sa_mapping_mode_set_cmd *param
			    =
			    (struct nss_macsec_secy_sc_sa_mapping_mode_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_sc_sa_mapping_mode_set(param->
								   secy_id,
								   param->mode);
		}
		break;
	case NSS_MACSEC_SECY_CONTROLLED_PORT_EN_GET_CMD:{
			struct nss_macsec_secy_controlled_port_en_get_cmd *param
			    =
			    (struct nss_macsec_secy_controlled_port_en_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_controlled_port_en_get(param->
								   secy_id,
								   &param->
								   penable);
		}
		break;
	case NSS_MACSEC_SECY_CONTROLLED_PORT_EN_SET_CMD:{
			struct nss_macsec_secy_controlled_port_en_set_cmd *param
			    =
			    (struct nss_macsec_secy_controlled_port_en_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_controlled_port_en_set(param->
								   secy_id,
								   param->
								   enable);
		}
		break;
	case NSS_MACSEC_SECY_CIPHER_SUITE_GET_CMD:{
			struct nss_macsec_secy_cipher_suite_get_cmd *param =
			    (struct nss_macsec_secy_cipher_suite_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_cipher_suite_get(param->secy_id,
							     &param->
							     p_cipher_suite);
		}
		break;
	case NSS_MACSEC_SECY_CIPHER_SUITE_SET_CMD:{
			struct nss_macsec_secy_cipher_suite_set_cmd *param =
			    (struct nss_macsec_secy_cipher_suite_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_cipher_suite_set(param->secy_id,
							     param->
							     cipher_suite);
		}
		break;
	case NSS_MACSEC_SECY_MTU_GET_CMD:{
			struct nss_macsec_secy_mtu_get_cmd *param =
			    (struct nss_macsec_secy_mtu_get_cmd *)(header + 1);
			ret =
			    nss_macsec_secy_mtu_get(param->secy_id,
						    &param->pmtu);
		}
		break;
	case NSS_MACSEC_SECY_MTU_SET_CMD:{
			struct nss_macsec_secy_mtu_set_cmd *param =
			    (struct nss_macsec_secy_mtu_set_cmd *)(header + 1);
			ret =
			    nss_macsec_secy_mtu_set(param->secy_id, param->mtu);
		}
		break;
	case NSS_MACSEC_SECY_ID_GET_CMD:{
			struct nss_macsec_secy_id_get_cmd *param =
			    (struct nss_macsec_secy_id_get_cmd *)(header + 1);
			ret =
			    nss_macsec_secy_id_get(param->dev_name,
						   &param->secy_id);
		}
		break;
	case NSS_MACSEC_SECY_EN_GET_CMD:{
			struct nss_macsec_secy_en_get_cmd *param =
			    (struct nss_macsec_secy_en_get_cmd *)(header + 1);
			ret =
			    nss_macsec_secy_en_get(param->secy_id,
						   &param->penable);
		}
		break;
	case NSS_MACSEC_SECY_EN_SET_CMD:{
			struct nss_macsec_secy_en_set_cmd *param =
			    (struct nss_macsec_secy_en_set_cmd *)(header + 1);
			ret =
			    nss_macsec_secy_en_set(param->secy_id,
						   param->enable);
		}
		break;
	case NSS_MACSEC_SECY_XPN_EN_GET_CMD:{
			struct nss_macsec_secy_xpn_en_get_cmd *param =
			(struct nss_macsec_secy_xpn_en_get_cmd *)(header + 1);
			ret =
			    nss_macsec_secy_xpn_en_get(param->secy_id,
						   &param->penable);
		}
		break;
	case NSS_MACSEC_SECY_XPN_EN_SET_CMD:{
			struct nss_macsec_secy_xpn_en_set_cmd *param =
			(struct nss_macsec_secy_xpn_en_set_cmd *)(header + 1);
			ret =
			    nss_macsec_secy_xpn_en_set(param->secy_id,
						   param->enable);
		}
		break;
	case NSS_MACSEC_SECY_RX_SA_NEXT_XPN_GET_CMD:{
			struct nss_macsec_secy_rx_sa_next_xpn_get_cmd *param =
			    (struct nss_macsec_secy_rx_sa_next_xpn_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_sa_next_xpn_get(param->secy_id,
							      param->channel,
							      param->an,
							      &param->
							      p_next_xpn);
		}
		break;
	case NSS_MACSEC_SECY_RX_SA_NEXT_XPN_SET_CMD:{
			struct nss_macsec_secy_rx_sa_next_xpn_set_cmd *param =
			    (struct nss_macsec_secy_rx_sa_next_xpn_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_sa_next_xpn_set(param->secy_id,
							      param->channel,
							      param->an,
							      param->next_xpn);
		}
		break;
	case NSS_MACSEC_SECY_TX_SA_NEXT_XPN_GET_CMD:{
			struct nss_macsec_secy_tx_sa_next_xpn_get_cmd *param =
			    (struct nss_macsec_secy_tx_sa_next_xpn_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_sa_next_xpn_get(param->secy_id,
							      param->channel,
							      param->an,
							      &param->
							      p_next_xpn);
		}
		break;
	case NSS_MACSEC_SECY_TX_SA_NEXT_XPN_SET_CMD:{
			struct nss_macsec_secy_tx_sa_next_xpn_set_cmd *param =
			    (struct nss_macsec_secy_tx_sa_next_xpn_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_sa_next_xpn_set(param->secy_id,
							      param->channel,
							      param->an,
							      param->next_xpn);
		}
		break;
	case NSS_MACSEC_SECY_RX_SC_SSCI_GET_CMD:{
			struct nss_macsec_secy_rx_sc_ssci_get_cmd *param =
			    (struct nss_macsec_secy_rx_sc_ssci_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_sc_ssci_get(param->secy_id,
							      param->channel,
							      &param->ssci);
		}
		break;
	case NSS_MACSEC_SECY_RX_SC_SSCI_SET_CMD:{
			struct nss_macsec_secy_rx_sc_ssci_set_cmd *param =
			    (struct nss_macsec_secy_rx_sc_ssci_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_sc_ssci_set(param->secy_id,
							      param->channel,
							      param->ssci);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_SSCI_GET_CMD:{
			struct nss_macsec_secy_tx_sc_ssci_get_cmd *param =
			    (struct nss_macsec_secy_tx_sc_ssci_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_sc_ssci_get(param->secy_id,
							      param->channel,
							      &param->ssci);
		}
		break;
	case NSS_MACSEC_SECY_TX_SC_SSCI_SET_CMD:{
			struct nss_macsec_secy_tx_sc_ssci_set_cmd *param =
			    (struct nss_macsec_secy_tx_sc_ssci_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_sc_ssci_set(param->secy_id,
							      param->channel,
							      param->ssci);
		}
		break;
	case NSS_MACSEC_SECY_TX_SA_KI_GET_CMD:{
			struct nss_macsec_secy_tx_sa_ki_get_cmd *param =
			    (struct nss_macsec_secy_tx_sa_ki_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_sa_ki_get(param->secy_id,
							      param->channel,
							      param->an,
							      &param->pki);
		}
		break;
	case NSS_MACSEC_SECY_TX_SA_KI_SET_CMD:{
			struct nss_macsec_secy_tx_sa_ki_set_cmd *param =
			    (struct nss_macsec_secy_tx_sa_ki_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_sa_ki_set(param->secy_id,
							      param->channel,
							      param->an,
							      &param->pki);
		}
		break;
	case NSS_MACSEC_SECY_RX_SA_KI_GET_CMD:{
			struct nss_macsec_secy_rx_sa_ki_get_cmd *param =
			    (struct nss_macsec_secy_rx_sa_ki_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_sa_ki_get(param->secy_id,
							      param->channel,
							      param->an,
							      &param->pki);
		}
		break;
	case NSS_MACSEC_SECY_RX_SA_KI_SET_CMD:{
			struct nss_macsec_secy_rx_sa_ki_set_cmd *param =
			    (struct nss_macsec_secy_rx_sa_ki_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_sa_ki_set(param->secy_id,
							      param->channel,
							      param->an,
							      &param->pki);
		}
		break;
	case NSS_MACSEC_SECY_FLOW_CONTROL_EN_GET_CMD:{
			struct nss_macsec_secy_flow_control_en_get_cmd *param =
			    (struct nss_macsec_secy_flow_control_en_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_flow_control_en_get(param->secy_id,
							      &param->penable
							      );
		}
		break;
	case NSS_MACSEC_SECY_FLOW_CONTROL_EN_SET_CMD:{
			struct nss_macsec_secy_flow_control_en_set_cmd *param =
			    (struct nss_macsec_secy_flow_control_en_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_flow_control_en_set(param->secy_id,
							      param->enable
							      );
		}
		break;
	case NSS_MACSEC_SECY_SPECIAL_PKT_CTRL_GET_CMD:{
			struct nss_macsec_secy_special_pkt_ctrl_get_cmd *param =
			    (struct nss_macsec_secy_special_pkt_ctrl_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_special_pkt_ctrl_get(param->secy_id,
							      param->type,
							      &param->action
							      );
		}
		break;
	case NSS_MACSEC_SECY_SPECIAL_PKT_CTRL_SET_CMD:{
			struct nss_macsec_secy_special_pkt_ctrl_set_cmd *param =
			    (struct nss_macsec_secy_special_pkt_ctrl_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_special_pkt_ctrl_set(param->secy_id,
							      param->type,
							      param->action
							      );
		}
		break;
	case NSS_MACSEC_SECY_UDF_ETHTYPE_GET_CMD:{
			struct nss_macsec_secy_udf_ethtype_get_cmd *param =
			    (struct nss_macsec_secy_udf_ethtype_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_udf_ethtype_get(param->secy_id,
							      &param->enable,
							      &param->type
							      );
		}
		break;
	case NSS_MACSEC_SECY_UDF_ETHTYPE_SET_CMD:{
			struct nss_macsec_secy_udf_ethtype_set_cmd *param =
			    (struct nss_macsec_secy_udf_ethtype_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_udf_ethtype_set(param->secy_id,
							      param->enable,
							      param->type
							      );
		}
		break;
	case NSS_MACSEC_SECY_TX_UDF_FILT_GET_CMD:{
			struct nss_macsec_secy_tx_udf_filt_get_cmd *param =
			    (struct nss_macsec_secy_tx_udf_filt_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_udf_filt_get(param->secy_id,
							    param->filt_id,
							    &param->pfilt);
		}
		break;
	case NSS_MACSEC_SECY_TX_UDF_FILT_SET_CMD:{
			struct nss_macsec_secy_tx_udf_filt_set_cmd *param =
			    (struct nss_macsec_secy_tx_udf_filt_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_udf_filt_set(param->secy_id,
							    param->filt_id,
							    &param->pfilt);
		}
		break;
	case NSS_MACSEC_SECY_RX_UDF_FILT_GET_CMD:{
			struct nss_macsec_secy_rx_udf_filt_get_cmd *param =
			    (struct nss_macsec_secy_rx_udf_filt_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_udf_filt_get(param->secy_id,
							    param->filt_id,
							    &param->pfilt);
		}
		break;
	case NSS_MACSEC_SECY_RX_UDF_FILT_SET_CMD:{
			struct nss_macsec_secy_rx_udf_filt_set_cmd *param =
			    (struct nss_macsec_secy_rx_udf_filt_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_udf_filt_set(param->secy_id,
							    param->filt_id,
							    &param->pfilt);
		}
		break;
	case NSS_MACSEC_SECY_TX_UDF_UFILT_CFG_GET_CMD:{
			struct nss_macsec_secy_tx_udf_ufilt_cfg_get_cmd *param =
			    (struct nss_macsec_secy_tx_udf_ufilt_cfg_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_udf_ufilt_cfg_get(param->secy_id,
							    &param->pcfg);
		}
		break;
	case NSS_MACSEC_SECY_TX_UDF_UFILT_CFG_SET_CMD:{
			struct nss_macsec_secy_tx_udf_ufilt_cfg_set_cmd *param =
			    (struct nss_macsec_secy_tx_udf_ufilt_cfg_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_udf_ufilt_cfg_set(param->secy_id,
							    &param->pcfg);
		}
		break;
	case NSS_MACSEC_SECY_TX_UDF_CFILT_CFG_GET_CMD:{
			struct nss_macsec_secy_tx_udf_cfilt_cfg_get_cmd *param =
			    (struct nss_macsec_secy_tx_udf_cfilt_cfg_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_udf_cfilt_cfg_get(param->secy_id,
							    &param->pcfg);
		}
		break;
	case NSS_MACSEC_SECY_TX_UDF_CFILT_CFG_SET_CMD:{
			struct nss_macsec_secy_tx_udf_cfilt_cfg_set_cmd *param =
			    (struct nss_macsec_secy_tx_udf_cfilt_cfg_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_tx_udf_cfilt_cfg_set(param->secy_id,
							    &param->pcfg);
		}
		break;
	case NSS_MACSEC_SECY_RX_UDF_UFILT_CFG_GET_CMD:{
			struct nss_macsec_secy_rx_udf_ufilt_cfg_get_cmd *param =
			    (struct nss_macsec_secy_rx_udf_ufilt_cfg_get_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_udf_ufilt_cfg_get(param->secy_id,
							    &param->pcfg);
		}
		break;
	case NSS_MACSEC_SECY_RX_UDF_UFILT_CFG_SET_CMD:{
			struct nss_macsec_secy_rx_udf_ufilt_cfg_set_cmd *param =
			    (struct nss_macsec_secy_rx_udf_ufilt_cfg_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_udf_ufilt_cfg_set(param->secy_id,
							    &param->pcfg);
		}
		break;
	case NSS_MACSEC_SECY_RX_SA_NEXT_PN_SET_CMD:{
			struct nss_macsec_secy_rx_sa_next_pn_set_cmd *param =
			    (struct nss_macsec_secy_rx_sa_next_pn_set_cmd
			     *)(header + 1);
			ret =
			    nss_macsec_secy_rx_sa_next_pn_set(param->secy_id,
							      param->channel,
							      param->an,
							      param->next_pn);
		}
		break;
	default:
		break;
	}

	return ret;
}
