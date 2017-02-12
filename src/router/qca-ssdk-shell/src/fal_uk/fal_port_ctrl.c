/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
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
 */



#include "sw.h"
#include "sw_ioctl.h"
#include "fal_port_ctrl.h"
#include "fal_uk_if.h"

sw_error_t
fal_port_duplex_set(a_uint32_t dev_id, fal_port_t port_id,
                    fal_port_duplex_t duplex)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_DUPLEX_SET, dev_id, port_id,
                    (a_uint32_t) duplex);
    return rv;
}

sw_error_t
fal_port_duplex_get(a_uint32_t dev_id, fal_port_t port_id,
                    fal_port_duplex_t * pduplex)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_DUPLEX_GET, dev_id, port_id,
                    (a_uint32_t) pduplex);
    return rv;
}

sw_error_t
fal_port_speed_set(a_uint32_t dev_id, fal_port_t port_id,
                   fal_port_speed_t speed)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_SPEED_SET, dev_id, port_id,
                    (a_uint32_t) speed);
    return rv;
}

sw_error_t
fal_port_speed_get(a_uint32_t dev_id, fal_port_t port_id,
                   fal_port_speed_t * pspeed)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_SPEED_GET, dev_id, port_id,
                    (a_uint32_t) pspeed);
    return rv;
}

sw_error_t
fal_port_autoneg_status_get(a_uint32_t dev_id, fal_port_t port_id,
                            a_bool_t * status)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_AN_GET, dev_id, port_id, (a_uint32_t) status);
    return rv;
}

sw_error_t
fal_port_autoneg_enable(a_uint32_t dev_id, fal_port_t port_id)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_AN_ENABLE, dev_id, port_id);
    return rv;
}

sw_error_t
fal_port_autoneg_restart(a_uint32_t dev_id, fal_port_t port_id)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_AN_RESTART, dev_id, port_id);
    return rv;
}

sw_error_t
fal_port_autoneg_adv_set(a_uint32_t dev_id, fal_port_t port_id,
                         a_uint32_t autoadv)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_AN_ADV_SET, dev_id, port_id, autoadv);
    return rv;
}

sw_error_t
fal_port_autoneg_adv_get(a_uint32_t dev_id, fal_port_t port_id,
                         a_uint32_t * autoadv)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_AN_ADV_GET, dev_id, port_id,
                    (a_uint32_t) autoadv);
    return rv;
}

sw_error_t
fal_port_hdr_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_HDR_SET, dev_id, port_id, (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_hdr_status_get(a_uint32_t dev_id, fal_port_t port_id,
                        a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_HDR_GET, dev_id, port_id, (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_flowctrl_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_FLOWCTRL_SET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_flowctrl_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_FLOWCTRL_GET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_flowctrl_forcemode_set(a_uint32_t dev_id, fal_port_t port_id,
                                a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_FLOWCTRL_MODE_SET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_flowctrl_forcemode_get(a_uint32_t dev_id, fal_port_t port_id,
                                a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_FLOWCTRL_MODE_GET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_powersave_set(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_POWERSAVE_SET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_powersave_get(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_POWERSAVE_GET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_hibernate_set(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_HIBERNATE_SET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_hibernate_get(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_HIBERNATE_GET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_cdt(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t mdi_pair,
             a_uint32_t *cable_status, a_uint32_t *cable_len)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_CDT, dev_id, port_id, mdi_pair,
                    (a_uint32_t) cable_status, (a_uint32_t)cable_len);
    return rv;
}

sw_error_t
fal_port_rxhdr_mode_set(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_header_mode_t mode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_RXHDR_SET, dev_id, port_id, (a_uint32_t)mode);
    return rv;
}

sw_error_t
fal_port_rxhdr_mode_get(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_header_mode_t * mode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_RXHDR_GET, dev_id, port_id, (a_uint32_t)mode);
    return rv;
}

sw_error_t
fal_port_txhdr_mode_set(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_header_mode_t mode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_TXHDR_SET, dev_id, port_id, (a_uint32_t)mode);
    return rv;
}

sw_error_t
fal_port_txhdr_mode_get(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_header_mode_t * mode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_TXHDR_GET, dev_id, port_id, (a_uint32_t)mode);
    return rv;
}

sw_error_t
fal_header_type_set(a_uint32_t dev_id, a_bool_t enable, a_uint32_t type)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_HEADER_TYPE_SET, dev_id, (a_uint32_t)enable, type);
    return rv;
}

sw_error_t
fal_header_type_get(a_uint32_t dev_id, a_bool_t * enable, a_uint32_t * type)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_HEADER_TYPE_GET, dev_id, (a_uint32_t)enable, (a_uint32_t)type);
    return rv;
}

sw_error_t
fal_port_txmac_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_TXMAC_STATUS_SET, dev_id, port_id, (a_uint32_t)enable);
    return rv;
}

sw_error_t
fal_port_txmac_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_TXMAC_STATUS_GET, dev_id, port_id, (a_uint32_t)enable);
    return rv;
}

sw_error_t
fal_port_rxmac_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_RXMAC_STATUS_SET, dev_id, port_id, (a_uint32_t)enable);
    return rv;
}

sw_error_t
fal_port_rxmac_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_RXMAC_STATUS_GET, dev_id, port_id, (a_uint32_t)enable);
    return rv;
}

sw_error_t
fal_port_txfc_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_TXFC_STATUS_SET, dev_id, port_id, (a_uint32_t)enable);
    return rv;
}

sw_error_t
fal_port_txfc_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_TXFC_STATUS_GET, dev_id, port_id, (a_uint32_t)enable);
    return rv;
}

sw_error_t
fal_port_rxfc_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_RXFC_STATUS_SET, dev_id, port_id, (a_uint32_t)enable);
    return rv;
}

sw_error_t
fal_port_rxfc_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_RXFC_STATUS_GET, dev_id, port_id, (a_uint32_t)enable);
    return rv;
}

sw_error_t
fal_port_bp_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_BP_STATUS_SET, dev_id, port_id, (a_uint32_t)enable);
    return rv;
}

sw_error_t
fal_port_bp_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_BP_STATUS_GET, dev_id, port_id, (a_uint32_t)enable);
    return rv;
}

sw_error_t
fal_port_link_forcemode_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_LINK_MODE_SET, dev_id, port_id, (a_uint32_t)enable);
    return rv;
}

sw_error_t
fal_port_link_forcemode_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_LINK_MODE_GET, dev_id, port_id, (a_uint32_t)enable);
    return rv;
}

sw_error_t
fal_port_link_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * status)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_LINK_STATUS_GET, dev_id, port_id, (a_uint32_t)status);
    return rv;
}

sw_error_t
fal_ports_link_status_get(a_uint32_t dev_id, a_uint32_t * status)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PTS_LINK_STATUS_GET, dev_id, (a_uint32_t)status);
    return rv;
}

sw_error_t
fal_port_mac_loopback_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_MAC_LOOPBACK_SET, dev_id, port_id, (a_uint32_t)enable);
    return rv;
}


sw_error_t
fal_port_mac_loopback_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_MAC_LOOPBACK_GET, dev_id, port_id, (a_uint32_t)enable);
    return rv;
}

sw_error_t
fal_port_congestion_drop_set(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t queue_id, a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_CONGESTION_DROP_SET, dev_id, port_id, queue_id, (a_uint32_t)enable);
    return rv;
}


sw_error_t
fal_port_congestion_drop_get(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t queue_id, a_bool_t *enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_CONGESTION_DROP_GET, dev_id, port_id, queue_id, (a_uint32_t)enable);
    return rv;
}

sw_error_t
fal_ring_flow_ctrl_thres_set(a_uint32_t dev_id, a_uint32_t ring_id, a_uint8_t on_thres, a_uint8_t off_thres)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_RING_FLOW_CTRL_THRES_SET, dev_id, ring_id, on_thres, off_thres);
    return rv;
}


sw_error_t
fal_ring_flow_ctrl_thres_get(a_uint32_t dev_id, a_uint32_t ring_id, a_uint8_t *on_thres, a_uint8_t *off_thres)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_RING_FLOW_CTRL_THRES_GET, dev_id, ring_id, (a_uint32_t)on_thres, (a_uint32_t)off_thres);
    return rv;
}

sw_error_t
fal_port_8023az_set(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_8023AZ_SET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_8023az_get(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_8023AZ_GET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_mdix_set(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_mdix_mode_t mode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_MDIX_SET, dev_id, port_id, (a_uint32_t)mode);
    return rv;
}

sw_error_t
fal_port_mdix_get(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_mdix_mode_t * mode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_MDIX_GET, dev_id, port_id, (a_uint32_t)mode);
    return rv;
}

sw_error_t
fal_port_mdix_status_get(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_mdix_status_t * mode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_MDIX_STATUS_GET, dev_id, port_id, (a_uint32_t)mode);
    return rv;
}

sw_error_t
fal_port_combo_prefer_medium_set(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_medium_t medium)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_COMBO_PREFER_MEDIUM_SET, dev_id, port_id, (a_uint32_t)medium);
    return rv;
}

sw_error_t
fal_port_combo_prefer_medium_get(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_medium_t * medium)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_COMBO_PREFER_MEDIUM_GET, dev_id, port_id, (a_uint32_t)medium);
    return rv;
}

sw_error_t
fal_port_combo_medium_status_get(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_medium_t * medium)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_COMBO_MEDIUM_STATUS_GET, dev_id, port_id, (a_uint32_t)medium);
    return rv;
}

sw_error_t
fal_port_combo_fiber_mode_set(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_fiber_mode_t  mode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_COMBO_FIBER_MODE_SET, dev_id, port_id, (a_uint32_t)mode);
    return rv;
}

sw_error_t
fal_port_combo_fiber_mode_get(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_fiber_mode_t * mode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_COMBO_FIBER_MODE_GET, dev_id, port_id, (a_uint32_t)mode);
    return rv;
}

sw_error_t
fal_port_local_loopback_set(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_LOCAL_LOOPBACK_SET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_local_loopback_get(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_LOCAL_LOOPBACK_GET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_remote_loopback_set(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_REMOTE_LOOPBACK_SET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_remote_loopback_get(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_REMOTE_LOOPBACK_GET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_reset(a_uint32_t dev_id, fal_port_t port_id)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_RESET, dev_id, port_id);
    return rv;
}

sw_error_t
fal_port_power_off(a_uint32_t dev_id, fal_port_t port_id)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_POWER_OFF, dev_id, port_id);
    return rv;
}

sw_error_t
fal_port_power_on(a_uint32_t dev_id, fal_port_t port_id)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_POWER_ON, dev_id, port_id);
    return rv;
}

    sw_error_t
    fal_port_magic_frame_mac_set (a_uint32_t dev_id, fal_port_t port_id,
				   fal_mac_addr_t * mac)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_MAGIC_FRAME_MAC_SET, dev_id, port_id,(a_uint32_t) mac);
    return rv;

}

   sw_error_t
   fal_port_magic_frame_mac_get (a_uint32_t dev_id, fal_port_t port_id,
				   fal_mac_addr_t * mac)
{

    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_MAGIC_FRAME_MAC_GET, dev_id, port_id,(a_uint32_t) mac);
    return rv;


}
 sw_error_t
    fal_port_phy_id_get (a_uint32_t dev_id, fal_port_t port_id,
		      a_uint16_t * org_id, a_uint16_t * rev_id)
 {
             sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_PHY_ID_GET, dev_id, port_id, (a_uint32_t) org_id, (a_uint32_t) rev_id);
    return rv;
 }
 sw_error_t
    fal_port_wol_status_set (a_uint32_t dev_id, fal_port_t port_id,
			      a_bool_t enable)
{
        sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_WOL_STATUS_SET, dev_id, port_id,(a_uint32_t) enable);
    return rv;

 }
 sw_error_t
    fal_port_wol_status_get (a_uint32_t dev_id, fal_port_t port_id,
			      a_bool_t * enable)

 {
           sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_WOL_STATUS_GET, dev_id, port_id,(a_uint32_t) enable);
    return rv;
 }

sw_error_t
fal_port_interface_mode_set (a_uint32_t dev_id, fal_port_t port_id, fal_port_interface_mode_t  mode)
{
  sw_error_t rv;

  rv = sw_uk_exec(SW_API_PT_INTERFACE_MODE_SET, dev_id, port_id,(a_uint32_t) mode);
  return rv;
}

sw_error_t
fal_port_interface_mode_get (a_uint32_t dev_id, fal_port_t port_id, fal_port_interface_mode_t * mode)
{
  sw_error_t rv;

  rv = sw_uk_exec(SW_API_PT_INTERFACE_MODE_GET, dev_id, port_id,(a_uint32_t) mode);
  return rv;
}

sw_error_t
fal_port_interface_mode_status_get (a_uint32_t dev_id, fal_port_t port_id, fal_port_interface_mode_t * mode)
{
  sw_error_t rv;

  rv = sw_uk_exec(SW_API_PT_INTERFACE_MODE_STATUS_GET, dev_id, port_id,(a_uint32_t) mode);
  return rv;
}

sw_error_t
fal_port_counter_set(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_COUNTER_SET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_counter_get(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_COUNTER_GET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_counter_show(a_uint32_t dev_id, fal_port_t port_id,
                       fal_port_counter_info_t * counter_info)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_COUNTER_SHOW, dev_id, port_id,
                    (a_uint32_t) counter_info);
    return rv;
}


