/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :> 
*/


#ifndef _RDPA_COMMON_H_
#define _RDPA_COMMON_H_

#include "bdmf_dev.h"
#include "bdmf_errno.h"
#include "rdpa_port.h"

/** Direction + index, Underlying structure for rdpa_dir_index aggregate */
typedef struct
{
    rdpa_traffic_dir dir;       /** Traffic direction */
    bdmf_index index;           /** Index */
} rdpa_dir_index_t;

typedef struct
{
    int src;
    int dest;
} int2int_map_t;

static inline int int2int_map(int2int_map_t *map, int src, int last)
{
    for (; map->src != last && map->src != src; map++)
        ;
    return map->dest;
}

static inline int int2int_map_r(int2int_map_t *map, int src, int last)
{
    for (; map->src != last && map->dest != src; map++)
        ;
    return map->src;
}

bdmf_error_t rdpa_obj_get(struct bdmf_object **rdpa_objs, int max_rdpa_objs_num, int index, struct bdmf_object **mo);
int rdpa_dir_index_get_next(rdpa_dir_index_t *dir_index, bdmf_index max_index);

rdpa_if rdpa_port_map_from_hw_port(int hw_port, bdmf_boolean emac_only);

#ifdef BDMF_DRIVER
/*
 * Enum tables for framework CLI access
 */
extern const bdmf_attr_enum_table_t rdpa_if_enum_table;
extern const bdmf_attr_enum_table_t rdpa_lan_wan_if_enum_table;
extern const bdmf_attr_enum_table_t rdpa_lan_wan_wlan_if_enum_table;
extern const bdmf_attr_enum_table_t rdpa_lan_or_cpu_if_enum_table;
extern const bdmf_attr_enum_table_t rdpa_wlan_ssid_enum_table;
extern const bdmf_attr_enum_table_t rdpa_emac_enum_table;
extern const bdmf_attr_enum_table_t rdpa_wan_emac_enum_table;
extern const bdmf_attr_enum_table_t rdpa_wan_type_enum_table;
extern const bdmf_attr_enum_table_t rdpa_forward_action_enum_table;
extern const bdmf_attr_enum_table_t rdpa_filter_action_enum_table;
extern const bdmf_attr_enum_table_t rdpa_traffic_dir_enum_table;
extern const bdmf_attr_enum_table_t rdpa_port_frame_allow_enum_table;
extern const bdmf_attr_enum_table_t rdpa_qos_method_enum_table;
extern const bdmf_attr_enum_table_t rdpa_ip_version_enum_table;
extern const bdmf_attr_enum_table_t rdpa_forward_mode_enum_table;
extern const bdmf_attr_enum_table_t rdpa_classify_mode_enum_table;
extern const bdmf_attr_enum_table_t rdpa_disc_prty_enum_table;
extern const bdmf_attr_enum_table_t rdpa_flow_dest_enum_table;
extern const bdmf_attr_enum_table_t rdpa_ip_class_method_enum_table;
extern const bdmf_attr_enum_table_t rdpa_cpu_reason_enum_table;
extern const bdmf_attr_enum_table_t rdpa_epon_mode_enum_table;
extern const bdmf_attr_enum_table_t rdpa_ic_act_vect_enum_table;
extern const bdmf_attr_enum_table_t rdpa_ic_dei_command_enum_table;
extern const bdmf_attr_enum_table_t rdpa_bpm_buffer_size_enum_table;
extern const bdmf_attr_enum_table_t rdpa_wl_accel_enum_table;
#endif /* #ifdef BDMF_DRIVER */

#endif
