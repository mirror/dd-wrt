/*
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
 */

/**
 * @defgroup
 * @{
 */
#include "sw.h"
#include "adpt.h"
#include "mppe_athtag_reg.h"
#include "mppe_athtag.h"
#include "appe_portvlan_reg.h"
#include "appe_portvlan.h"
#include "hsl.h"
#include "hsl_dev.h"

sw_error_t
adpt_mppe_athtag_pri_mapping_set(a_uint32_t dev_id,
		fal_direction_t direction, fal_athtag_pri_mapping_t * pri_mapping)
{
	union prx_hdr_rcv_pri_mapping_u prx_pri_map = {0};
	union eg_hdr_xmit_pri_mapping_u ptx_pri_map = {0};

	ADPT_DEV_ID_CHECK(dev_id);

	if (direction == FAL_DIR_INGRESS || direction == FAL_DIR_BOTH)
	{
		/*ingress priority mapping*/
		SW_RTN_ON_ERROR(mppe_prx_hdr_rcv_pri_mapping_get(dev_id,
					pri_mapping->ath_pri, &prx_pri_map));
		prx_pri_map.bf.pri = pri_mapping->int_pri;
		SW_RTN_ON_ERROR(mppe_prx_hdr_rcv_pri_mapping_set(dev_id,
					pri_mapping->ath_pri, &prx_pri_map));
	}
	if (direction == FAL_DIR_EGRESS || direction == FAL_DIR_BOTH)
	{
		/*egress priority mapping*/
		SW_RTN_ON_ERROR(mppe_eg_hdr_xmit_pri_mapping_get(dev_id,
					pri_mapping->int_pri, &ptx_pri_map));
		ptx_pri_map.bf.pri = pri_mapping->ath_pri;
		SW_RTN_ON_ERROR(mppe_eg_hdr_xmit_pri_mapping_set(dev_id,
					pri_mapping->int_pri, &ptx_pri_map));
	}
	return SW_OK;
}

sw_error_t
adpt_mppe_athtag_pri_mapping_get(a_uint32_t dev_id,
		fal_direction_t direction, fal_athtag_pri_mapping_t * pri_mapping)
{
	union prx_hdr_rcv_pri_mapping_u prx_pri_map = {0};
	union eg_hdr_xmit_pri_mapping_u ptx_pri_map = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(pri_mapping);

	if (direction == FAL_DIR_INGRESS)
	{
		SW_RTN_ON_ERROR(mppe_prx_hdr_rcv_pri_mapping_get(dev_id,
					pri_mapping->ath_pri, &prx_pri_map));
		pri_mapping->int_pri = prx_pri_map.bf.pri;
	}
	else if (direction == FAL_DIR_EGRESS)
	{
		SW_RTN_ON_ERROR(mppe_eg_hdr_xmit_pri_mapping_get(dev_id,
					pri_mapping->int_pri, &ptx_pri_map));
		pri_mapping->ath_pri = ptx_pri_map.bf.pri;
	}
	else
	{
		SSDK_ERROR("not support direction %d\n", direction);
		return SW_NOT_SUPPORTED;
	}

	return SW_OK;
}

static a_uint32_t _adpt_mppe_athtag_bit_index(a_uint32_t bits)
{
	a_uint32_t i = 0;
	for (i=0; i<32; i++)
	{
		if((bits >> i) &0x1)
		{
			return i;
		}
	}
	return 0xff;
}

sw_error_t
adpt_mppe_port_athtag_rx_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_athtag_rx_cfg_t *cfg)
{
	union prx_port_to_vp_mapping_u prx_port_to_vp_map = {0};

	ADPT_DEV_ID_CHECK(dev_id);

	/*enable rx athtag for a specific athtag type*/
	SW_RTN_ON_ERROR(mppe_prx_port_to_vp_mapping_get(dev_id,
				port_id, &prx_port_to_vp_map));
	prx_port_to_vp_map.bf.prx_ath_hdr_en = cfg->athtag_en;
	prx_port_to_vp_map.bf.prx_ath_hdr_type = cfg->athtag_type;
	return mppe_prx_port_to_vp_mapping_set(dev_id,
				port_id, &prx_port_to_vp_map);
}

sw_error_t
adpt_mppe_port_athtag_rx_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_athtag_rx_cfg_t *cfg)
{
	union prx_port_to_vp_mapping_u prx_port_to_vp_map = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);

	SW_RTN_ON_ERROR(mppe_prx_port_to_vp_mapping_get(dev_id,
				port_id, &prx_port_to_vp_map));
	cfg->athtag_en = prx_port_to_vp_map.bf.prx_ath_hdr_en;
	cfg->athtag_type = prx_port_to_vp_map.bf.prx_ath_hdr_type;

	return SW_OK;
}

sw_error_t
adpt_mppe_port_athtag_tx_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_athtag_tx_cfg_t *cfg)
{
	union eg_vp_tbl_u eg_vp_tbl = {0};
	union eg_gen_ctrl_u eg_gen_ctrl = {0};

	ADPT_DEV_ID_CHECK(dev_id);

	/*set tx athtag type*/
	SW_RTN_ON_ERROR(mppe_eg_gen_ctrl_get(dev_id, &eg_gen_ctrl));
	eg_gen_ctrl.bf.ath_hdr_type = cfg->athtag_type;
	SW_RTN_ON_ERROR(mppe_eg_gen_ctrl_set(dev_id, &eg_gen_ctrl));

	/*set tx athtag enable and other configurations*/
	SW_RTN_ON_ERROR(appe_egress_vp_tbl_get(dev_id,
				FAL_PORT_ID_VALUE(port_id), &eg_vp_tbl));
	eg_vp_tbl.bf.ath_hdr_insert = cfg->athtag_en;
	eg_vp_tbl.bf.ath_hdr_default_type = cfg->action;
	eg_vp_tbl.bf.ath_hdr_from_cpu = cfg->bypass_fwd_en;
	eg_vp_tbl.bf.ath_hdr_disable_bit = cfg->field_disable;
	if (cfg->version == FAL_ATHTAG_VER2)
	{
		eg_vp_tbl.bf.ath_hdr_ver = 2;
	}
	else if (cfg->version == FAL_ATHTAG_VER3)
	{
		eg_vp_tbl.bf.ath_hdr_ver = 3;
	}

	return appe_egress_vp_tbl_set(dev_id,
				FAL_PORT_ID_VALUE(port_id), &eg_vp_tbl);
}

sw_error_t
adpt_mppe_port_athtag_tx_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_athtag_tx_cfg_t *cfg)
{
	union eg_vp_tbl_u eg_vp_tbl = {0};
	union eg_gen_ctrl_u eg_gen_ctrl = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);

	/*get tx athtag type*/
	SW_RTN_ON_ERROR(mppe_eg_gen_ctrl_get(dev_id, &eg_gen_ctrl));
	cfg->athtag_type = eg_gen_ctrl.bf.ath_hdr_type;

	/*get tx athtag enable and other configurations*/
	SW_RTN_ON_ERROR(appe_egress_vp_tbl_get(dev_id,
				FAL_PORT_ID_VALUE(port_id), &eg_vp_tbl));
	cfg->athtag_en = eg_vp_tbl.bf.ath_hdr_insert;
	cfg->action = eg_vp_tbl.bf.ath_hdr_default_type;
	cfg->bypass_fwd_en = eg_vp_tbl.bf.ath_hdr_from_cpu;
	cfg->field_disable = eg_vp_tbl.bf.ath_hdr_disable_bit;
	if (eg_vp_tbl.bf.ath_hdr_ver == 2)
	{
		cfg->version = FAL_ATHTAG_VER2;
	}
	else if (eg_vp_tbl.bf.ath_hdr_ver == 3)
	{
		cfg->version = FAL_ATHTAG_VER3;
	}

	return SW_OK;
}

sw_error_t
_adpt_mppe_athtag_slave_switch_set (a_uint32_t dev_id, a_uint32_t ath_port)
{
#if defined(MHT)
	a_uint32_t port_id = 0;
	if (hsl_get_current_chip_type(dev_id) == CHIP_MHT)
	{
		fal_header_type_set(dev_id, A_TRUE, MHT_ATHTAG_TYPE);
		fal_port_rxhdr_mode_set(dev_id, SSDK_PHYSICAL_PORT0, FAL_ONLY_MANAGE_FRAME_EN);
		fal_port_txhdr_mode_set(dev_id, SSDK_PHYSICAL_PORT0, FAL_ALL_TYPE_FRAME_EN);
		if ((ath_port & (ath_port -1)) == 0)
		{
			/* single port mapping configurations */
			port_id = _adpt_mppe_athtag_bit_index(ath_port);
			fal_port_unk_uc_filter_set(dev_id, port_id, A_TRUE);
			fal_port_unk_mc_filter_set(dev_id, port_id, A_TRUE);
			fal_port_bc_filter_set(dev_id, port_id, A_TRUE);
		}
	}
#endif
	return SW_OK;
}

sw_error_t
adpt_mppe_athtag_port_mapping_set(a_uint32_t dev_id,
		fal_direction_t direction, fal_athtag_port_mapping_t * port_mapping)
{
	union prx_port_to_vp_mapping_u prx_port_to_vp_map = {0};
	union eg_vp_tbl_u eg_vp_tbl = {0};
	a_uint32_t port_id = 0;
	fal_athtag_rx_cfg_t rx_cfg = {0};
	fal_athtag_tx_cfg_t tx_cfg = {0};
	adpt_api_t *p_api = NULL;

	ADPT_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));
	SW_RTN_ON_NULL(p_api->adpt_port_bridge_txmac_set);

	if (direction == FAL_DIR_INGRESS || direction == FAL_DIR_BOTH)
	{
		/* enable rx athtag for mppe port1 */
		rx_cfg.athtag_en = A_TRUE;
#if defined(MHT)
		if (hsl_get_current_chip_type(dev_id + 1) == CHIP_MHT)
		{
			rx_cfg.athtag_type = MHT_ATHTAG_TYPE;
		}
#endif
		adpt_mppe_port_athtag_rx_set(dev_id, SSDK_PHYSICAL_PORT1, &rx_cfg);

		/* ingress port mapping */
		for (port_id = 0; port_id < PRX_PORT_TO_VP_MAPPING_MAX_ENTRY; port_id++)
		{
			if (!SW_IS_PBMP_MEMBER(port_mapping->ath_port, port_id))
				continue;
			SW_RTN_ON_ERROR(mppe_prx_port_to_vp_mapping_get(dev_id,
						port_id, &prx_port_to_vp_map));
			prx_port_to_vp_map.bf.prx_port_vp =
						FAL_PORT_ID_VALUE(port_mapping->int_port);
			SW_RTN_ON_ERROR(mppe_prx_port_to_vp_mapping_set(dev_id,
						port_id, &prx_port_to_vp_map));
		}
	}
	if (direction == FAL_DIR_EGRESS || direction == FAL_DIR_BOTH)
	{
		/* enable tx athtag for int_port */
		if (port_mapping->ath_port & (port_mapping->ath_port -1))
			tx_cfg.version = FAL_ATHTAG_VER2;
		else
			tx_cfg.version = FAL_ATHTAG_VER3;
		tx_cfg.athtag_en = A_TRUE;
#if defined(MHT)
		if (hsl_get_current_chip_type(dev_id + 1) == CHIP_MHT)
		{
			tx_cfg.athtag_type = MHT_ATHTAG_TYPE;
		}
#endif
		tx_cfg.action = FAL_ATHTAG_ACTION_DISABLE_LEARN;
		tx_cfg.bypass_fwd_en = A_TRUE;
		tx_cfg.field_disable = A_FALSE;
		adpt_mppe_port_athtag_tx_set(dev_id, port_mapping->int_port, &tx_cfg);

		/* egress port mapping */
		SW_RTN_ON_ERROR(appe_egress_vp_tbl_get(dev_id,
					FAL_PORT_ID_VALUE(port_mapping->int_port), &eg_vp_tbl));

		if (eg_vp_tbl.bf.ath_hdr_ver == 2)
		{
			/*for version2 the ath_port_bitmap in eg_vp_tbl is the port bitmap*/
			eg_vp_tbl.bf.ath_port_bitmap_0 = port_mapping->ath_port & 0x3f;
			eg_vp_tbl.bf.ath_port_bitmap_1 = (port_mapping->ath_port >>6) & 0x1;
		}
		else
		{
			/*get the first port id in the port bitmap*/
			port_id = _adpt_mppe_athtag_bit_index(port_mapping->ath_port);
			/*for version3 the ath_port_bitmap in eg_vp_tbl is the port id*/
			eg_vp_tbl.bf.ath_port_bitmap_0 = port_id & 0x3f;
			eg_vp_tbl.bf.ath_port_bitmap_1 = (port_id >>6) & 0x1;
		}
		SW_RTN_ON_ERROR(appe_egress_vp_tbl_set(dev_id,
					FAL_PORT_ID_VALUE(port_mapping->int_port), &eg_vp_tbl));

		/* enable txmac_en for int_port */
		p_api->adpt_port_bridge_txmac_set(dev_id, port_mapping->int_port, A_TRUE);
	}

	/* slave switch athtag configuration */
	_adpt_mppe_athtag_slave_switch_set (dev_id + 1, port_mapping->ath_port);

	return SW_OK;
}

sw_error_t
adpt_mppe_athtag_port_mapping_get(a_uint32_t dev_id,
		fal_direction_t direction, fal_athtag_port_mapping_t * port_mapping)
{
	union prx_port_to_vp_mapping_u prx_port_to_vp_map = {0};
	union eg_vp_tbl_u eg_vp_tbl = {0};
	a_uint32_t port_id = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(port_mapping);

	if (direction == FAL_DIR_INGRESS)
	{
		/*get the first port id in the port bitmap*/
		port_id = _adpt_mppe_athtag_bit_index(port_mapping->ath_port);
		SW_RTN_ON_ERROR(mppe_prx_port_to_vp_mapping_get(dev_id,
					port_id, &prx_port_to_vp_map));
		port_mapping->int_port = prx_port_to_vp_map.bf.prx_port_vp;
	}
	else if (direction == FAL_DIR_EGRESS)
	{
		SW_RTN_ON_ERROR(appe_egress_vp_tbl_get(dev_id,
					FAL_PORT_ID_VALUE(port_mapping->int_port), &eg_vp_tbl));

		if (eg_vp_tbl.bf.ath_hdr_ver == 2)
		{
			/*for version2 the ath_port_bitmap in eg_vp_tbl is the port bitmap*/
			port_mapping->ath_port =
			(eg_vp_tbl.bf.ath_port_bitmap_1 << 6) | eg_vp_tbl.bf.ath_port_bitmap_0;
		}
		else
		{
			/*for version3 the ath_port_bitmap in eg_vp_tbl is the port id*/
			port_mapping->ath_port =
			BIT((eg_vp_tbl.bf.ath_port_bitmap_1 << 6) | eg_vp_tbl.bf.ath_port_bitmap_0);
		}
	}
	else
	{
		SSDK_ERROR("not support direction %d\n", direction);
		return SW_NOT_SUPPORTED;
	}

	return SW_OK;
}

sw_error_t adpt_mppe_athtag_init(a_uint32_t dev_id)
{
	adpt_api_t *p_adpt_api = NULL;

	p_adpt_api = adpt_api_ptr_get(dev_id);

	ADPT_NULL_POINT_CHECK(p_adpt_api);

	p_adpt_api->adpt_athtag_pri_mapping_set = adpt_mppe_athtag_pri_mapping_set;
	p_adpt_api->adpt_athtag_pri_mapping_get = adpt_mppe_athtag_pri_mapping_get;
	p_adpt_api->adpt_athtag_port_mapping_set = adpt_mppe_athtag_port_mapping_set;
	p_adpt_api->adpt_athtag_port_mapping_get = adpt_mppe_athtag_port_mapping_get;
	p_adpt_api->adpt_port_athtag_rx_set = adpt_mppe_port_athtag_rx_set;
	p_adpt_api->adpt_port_athtag_rx_get = adpt_mppe_port_athtag_rx_get;
	p_adpt_api->adpt_port_athtag_tx_set = adpt_mppe_port_athtag_tx_set;
	p_adpt_api->adpt_port_athtag_tx_get = adpt_mppe_port_athtag_tx_get;

	return SW_OK;
}

/**
 * @}
 */
