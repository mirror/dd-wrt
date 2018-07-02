/*
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
       All Rights Reserved
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/

#include "rdd.h"

BL_LILAC_RDD_ERROR_DTE rdd_vlan_command_config ( rdpa_traffic_dir                        xi_direction,
                                                 rdd_vlan_command_params                 *xi_vlan_command_params )
{
    return ( BL_LILAC_RDD_OK );
}

#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_us_vlan_aggregation_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE       xi_bridge_port,
                                                        BL_LILAC_RDD_AGGREGATION_MODE_DTE  xi_vlan_aggregation_mode )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_vlan_switching_config ( BL_LILAC_RDD_VLAN_SWITCHING_CONFIG_DTE  xi_vlan_switching_mode,
                                                   BL_LILAC_RDD_VLAN_BINDING_CONFIG_DTE    xi_vlan_binding_mode )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_lan_vid_entry_add ( RDD_LAN_VID_PARAMS  *xi_lan_vid_params_ptr,
                                               uint32_t            *xo_entry_index )
{
    return ( BL_LILAC_RDD_ERROR_CAM_LOOKUP_TABLE_FULL );
}

BL_LILAC_RDD_ERROR_DTE rdd_lan_vid_entry_delete ( uint32_t  xi_entry_index )
{
    return ( BL_LILAC_RDD_OK );
}
#endif

BL_LILAC_RDD_ERROR_DTE rdd_lan_vid_entry_modify ( uint32_t            xi_entry_index,
                                                  RDD_LAN_VID_PARAMS  *xi_lan_vid_params_ptr )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_wan_vid_config ( uint8_t   xi_wan_vid_index,
                                            uint16_t  xi_wan_vid )
{
    return ( BL_LILAC_RDD_OK );
}

#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_dscp_to_pbits_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                  uint32_t                      xi_dscp,
                                                  uint32_t                      xi_pbits )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_pbits_to_pbits_config ( uint32_t  xi_table_number,
                                                   uint32_t  xi_input_pbits,
                                                   uint32_t  xi_output_pbits )
{ 
    return ( BL_LILAC_RDD_OK );
}
#else
void rdd_dscp_to_pbits_cfg(rdpa_traffic_dir direction, rdd_vport_id_t vport_id, uint32_t dscp, uint32_t pbits)
{
    return;
}

int rdd_pbits_to_pbits_config ( uint32_t  xi_table_number,
                                uint32_t  xi_input_pbits,
                                uint32_t  xi_output_pbits )
{ 
    return BDMF_ERR_OK;
}
#endif

BL_LILAC_RDD_ERROR_DTE rdd_tpid_detect_filter_value_config ( rdpa_traffic_dir  xi_direction,
                                                             uint16_t          tpid_detect_filter_value )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_tpid_detect_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                       BL_LILAC_RDD_SUBNET_ID_DTE    xi_subnet_id,
                                                       BL_LILAC_RDD_FILTER_MODE_DTE  xi_tpid_filter_mode )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_dhcp_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                BL_LILAC_RDD_SUBNET_ID_DTE    xi_subnet_id,
                                                BL_LILAC_RDD_FILTER_MODE_DTE  xi_dhcp_filter_mode )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_mld_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                               BL_LILAC_RDD_SUBNET_ID_DTE    xi_subnet_id,
                                               BL_LILAC_RDD_FILTER_MODE_DTE  xi_mld_filter_mode )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_igmp_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE    xi_bridge_port,
                                                BL_LILAC_RDD_SUBNET_ID_DTE      xi_subnet_id,
                                                BL_LILAC_RDD_FILTER_MODE_DTE    xi_igmp_filter_mode,
                                                BL_LILAC_RDD_FILTER_ACTION_DTE  xi_filter_action )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_icmpv6_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                  BL_LILAC_RDD_SUBNET_ID_DTE    xi_subnet_id,
                                                  BL_LILAC_RDD_FILTER_MODE_DTE  xi_icmpv6_filter_mode )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_ether_type_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE               xi_bridge_port,
                                                      BL_LILAC_RDD_SUBNET_ID_DTE                 xi_subnet_id,
                                                      BL_LILAC_RDD_FILTER_MODE_DTE               xi_ether_type_filter_mode,
                                                      BL_LILAC_RDD_ETHER_TYPE_FILTER_NUMBER_DTE  xi_ether_type_filter_number,
                                                      BL_LILAC_RDD_FILTER_ACTION_DTE             xi_ether_type_action )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_broadcast_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE    xi_bridge_port,
                                                     BL_LILAC_RDD_SUBNET_ID_DTE      xi_subnet_id,
                                                     BL_LILAC_RDD_FILTER_MODE_DTE    xi_broadcast_filter_mode,
                                                     BL_LILAC_RDD_FILTER_ACTION_DTE  xi_filter_action )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_multicast_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE    xi_bridge_port,
                                                     BL_LILAC_RDD_SUBNET_ID_DTE      xi_subnet_id,
                                                     BL_LILAC_RDD_FILTER_MODE_DTE    xi_multicast_filter_mode,
                                                     BL_LILAC_RDD_FILTER_ACTION_DTE  xi_filter_action )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_local_switching_filters_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                            BL_LILAC_RDD_FILTER_MODE_DTE  xi_local_switching_filters_mode )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_ip_fragments_ingress_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE    xi_bridge_port,
                                                                BL_LILAC_RDD_SUBNET_ID_DTE      xi_subnet_id,
                                                                BL_LILAC_RDD_FILTER_MODE_DTE    xi_ip_fragments_filter_mode,
                                                                BL_LILAC_RDD_FILTER_ACTION_DTE  xi_filter_action )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_header_error_ingress_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE    xi_bridge_port,
                                                                BL_LILAC_RDD_SUBNET_ID_DTE      xi_subnet_id,
                                                                BL_LILAC_RDD_FILTER_MODE_DTE    xi_header_error_filter_mode,
                                                                BL_LILAC_RDD_FILTER_ACTION_DTE  xi_filter_action )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_bridge_flooding_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_ports_vector,
                                                    uint16_t                      xi_wifi_ssid_vector )
{
    return ( BL_LILAC_RDD_OK );
}

#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_add ( RDD_MAC_PARAMS  *xi_mac_params_ptr,
                                           uint32_t        *xo_mac_entry_index )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_delete ( bdmf_mac_t                       *xi_mac_addr,
                                              BL_LILAC_RDD_MAC_ENTRY_TYPE_DTE  xi_entry_type )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_modify ( RDD_MAC_PARAMS  *xi_mac_params_ptr )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_search ( RDD_MAC_PARAMS  *xi_mac_params_ptr,
                                              uint32_t        *xo_entry_index )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_get ( uint32_t        xi_entry_index,
                                           RDD_MAC_PARAMS  *xo_mac_params_ptr,
                                           uint32_t        *xo_valid_bit,
                                           uint32_t        *xo_skip_bit,
                                           uint32_t        *xo_aging_bit )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_aging_set ( uint32_t  xi_entry_index,
                                                 uint32_t  *xo_activity_status )
{
    return ( BL_LILAC_RDD_OK );
}
#endif

BL_LILAC_RDD_ERROR_DTE rdd_clear_mac_table ( void )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_forwarding_matrix_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE     xi_src_bridge_port,
                                                      BL_LILAC_RDD_BRIDGE_PORT_DTE     xi_dst_bridge_port,
                                                      BL_LILAC_RDD_FORWARD_MATRIX_DTE  xi_enable )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_wifi_ssid_forwarding_matrix_config ( uint16_t                      xi_wifi_ssid_vector,
                                                                BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_dst_bridge_port )
{
    return ( BL_LILAC_RDD_OK );
}

#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_egress_ethertype_config ( uint16_t  xi_1st_ether_type,
                                                     uint16_t  xi_2nd_ether_type )
{
    return ( BL_LILAC_RDD_OK );
}
#else
void rdd_egress_ethertype_config ( uint16_t  xi_1st_ether_type,
                                   uint16_t  xi_2nd_ether_type )
{
}
#endif

BL_LILAC_RDD_ERROR_DTE rdd_src_mac_anti_spoofing_lookup_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                                 BL_LILAC_RDD_FILTER_MODE_DTE  xi_filter_mode )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_src_mac_anti_spoofing_entry_add ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                             uint32_t                      xi_src_mac_prefix )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_src_mac_anti_spoofing_entry_delete  ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                                 uint32_t                      xi_src_mac_prefix )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_tpid_overwrite_table_config ( uint16_t          *tpid_overwrite_array,
                                                         rdpa_traffic_dir  xi_direction )
{
    return ( BL_LILAC_RDD_OK );
}

#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_vlan_command_always_egress_ether_type_config ( uint16_t  xi_3rd_ether_type )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_vlan_switching_isolation_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                             rdpa_traffic_dir              xi_direction,
                                                             BL_LILAC_RDD_FILTER_MODE_DTE  xi_vlan_switching_isolation_mode )
{
    return ( BL_LILAC_RDD_OK );
}
#else
void rdd_vlan_command_always_egress_ether_type_config ( uint16_t  xi_3rd_ether_type )
{
}
#endif

BL_LILAC_RDD_ERROR_DTE rdd_lan_get_stats ( uint32_t   xi_lan_port,
                                           uint32_t   *rx_packet,
                                           uint32_t   *tx_packet,
                                           uint16_t   *tx_discard )
{
    BL_LILAC_RDD_ERROR_DTE rdd_error = BL_LILAC_RDD_OK;

    uint32_t counter_offset = xi_lan_port + 1;  /* skip the first one which is used for WAN */
    
    rdd_error = rdd_2_bytes_counter_get ( BRIDGE_DOWNSTREAM_TX_CONGESTION_GROUP, counter_offset, tx_discard );
    if (!rdd_error)
        rdd_error = rdd_4_bytes_counter_get ( LAN_TX_PACKETS_GROUP, counter_offset + 8, tx_packet );
    if (!rdd_error)
        rdd_error = rdd_4_bytes_counter_get ( LAN_RX_PACKETS_GROUP, counter_offset + 8, rx_packet );

    return ( rdd_error );
}


