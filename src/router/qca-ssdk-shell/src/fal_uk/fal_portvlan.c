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
#include "fal_portvlan.h"
#include "fal_uk_if.h"

sw_error_t
fal_port_1qmode_set(a_uint32_t dev_id, fal_port_t port_id,
                    fal_pt_1qmode_t port_1qmode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_ING_MODE_SET, dev_id, port_id,
                    (a_uint32_t) port_1qmode);
    return rv;
}

sw_error_t
fal_port_1qmode_get(a_uint32_t dev_id, fal_port_t port_id,
                    fal_pt_1qmode_t * pport_1qmode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_ING_MODE_GET, dev_id, port_id,
                    (a_uint32_t) pport_1qmode);
    return rv;
}

sw_error_t
fal_port_egvlanmode_set(a_uint32_t dev_id, fal_port_t port_id,
                        fal_pt_1q_egmode_t port_egvlanmode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_EG_MODE_SET, dev_id, port_id,
                    (a_uint32_t) port_egvlanmode);
    return rv;
}

sw_error_t
fal_port_egvlanmode_get(a_uint32_t dev_id, fal_port_t port_id,
                        fal_pt_1q_egmode_t * pport_egvlanmode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_EG_MODE_GET, dev_id, port_id,
                    (a_uint32_t) pport_egvlanmode);
    return rv;
}

sw_error_t
fal_portvlan_member_add(a_uint32_t dev_id, fal_port_t port_id,
                        a_uint32_t mem_port_id)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_VLAN_MEM_ADD, dev_id, port_id,
                    (a_uint32_t) mem_port_id);
    return rv;
}

sw_error_t
fal_portvlan_member_del(a_uint32_t dev_id, fal_port_t port_id,
                        a_uint32_t mem_port_id)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_VLAN_MEM_DEL, dev_id, port_id,
                    (a_uint32_t) mem_port_id);
    return rv;
}

sw_error_t
fal_portvlan_member_update(a_uint32_t dev_id, fal_port_t port_id,
                           fal_pbmp_t mem_port_map)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_VLAN_MEM_UPDATE, dev_id, port_id,
                    (a_uint32_t) mem_port_map);
    return rv;
}

sw_error_t
fal_portvlan_member_get(a_uint32_t dev_id, fal_port_t port_id,
                        fal_pbmp_t * mem_port_map)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_VLAN_MEM_GET, dev_id, port_id,
                    (a_uint32_t) mem_port_map);
    return rv;
}

sw_error_t
fal_port_default_vid_set(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t vid)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_DEF_VID_SET, dev_id, port_id,
                    vid);
    return rv;
}

sw_error_t
fal_port_default_vid_get(a_uint32_t dev_id, fal_port_t port_id,
                         a_uint32_t * vid)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_DEF_VID_GET, dev_id, port_id,
                    (a_uint32_t) vid);
    return rv;
}

sw_error_t
fal_port_force_default_vid_set(a_uint32_t dev_id, fal_port_t port_id,
                               a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_FORCE_DEF_VID_SET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_force_default_vid_get(a_uint32_t dev_id, fal_port_t port_id,
                               a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_FORCE_DEF_VID_GET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_force_portvlan_set(a_uint32_t dev_id, fal_port_t port_id,
                            a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_FORCE_PORTVLAN_SET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_force_portvlan_get(a_uint32_t dev_id, fal_port_t port_id,
                            a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_FORCE_PORTVLAN_GET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_nestvlan_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_NESTVLAN_SET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_nestvlan_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_NESTVLAN_GET, dev_id, port_id,
                    (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_nestvlan_tpid_set(a_uint32_t dev_id, a_uint32_t tpid)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_NESTVLAN_TPID_SET, dev_id, tpid);
    return rv;
}

sw_error_t
fal_nestvlan_tpid_get(a_uint32_t dev_id, a_uint32_t * tpid)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_NESTVLAN_TPID_GET, dev_id, (a_uint32_t) tpid);
    return rv;
}

sw_error_t
fal_port_invlan_mode_set(a_uint32_t dev_id, fal_port_t port_id,
                         fal_pt_invlan_mode_t mode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_IN_VLAN_MODE_SET, dev_id, port_id, (a_uint32_t) mode);
    return rv;
}

sw_error_t
fal_port_invlan_mode_get(a_uint32_t dev_id, fal_port_t port_id,
                         fal_pt_invlan_mode_t * mode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_IN_VLAN_MODE_GET, dev_id, port_id, (a_uint32_t) mode);
    return rv;
}

sw_error_t
fal_port_tls_set(a_uint32_t dev_id, fal_port_t port_id,
                 a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_TLS_SET, dev_id, port_id, (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_tls_get(a_uint32_t dev_id, fal_port_t port_id,
                 a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_TLS_GET, dev_id, port_id, (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_pri_propagation_set(a_uint32_t dev_id, fal_port_t port_id,
                             a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_PRI_PROPAGATION_SET, dev_id, port_id, (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_pri_propagation_get(a_uint32_t dev_id, fal_port_t port_id,
                             a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_PRI_PROPAGATION_GET, dev_id, port_id, (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_default_svid_set(a_uint32_t dev_id, fal_port_t port_id,
                          a_uint32_t vid)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_DEF_SVID_SET, dev_id, port_id, vid);
    return rv;
}

sw_error_t
fal_port_default_svid_get(a_uint32_t dev_id, fal_port_t port_id,
                          a_uint32_t * vid)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_DEF_SVID_GET, dev_id, port_id, (a_uint32_t)vid);
    return rv;
}

sw_error_t
fal_port_default_cvid_set(a_uint32_t dev_id, fal_port_t port_id,
                          a_uint32_t vid)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_DEF_CVID_SET, dev_id, port_id, vid);
    return rv;
}

sw_error_t
fal_port_default_cvid_get(a_uint32_t dev_id, fal_port_t port_id,
                          a_uint32_t * vid)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_DEF_CVID_GET, dev_id, port_id, (a_uint32_t)vid);
    return rv;
}

sw_error_t
fal_port_vlan_propagation_set(a_uint32_t dev_id, fal_port_t port_id,
                              fal_vlan_propagation_mode_t mode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_VLAN_PROPAGATION_SET, dev_id, port_id, (a_uint32_t)mode);
    return rv;
}

sw_error_t
fal_port_vlan_propagation_get(a_uint32_t dev_id, fal_port_t port_id,
                              fal_vlan_propagation_mode_t * mode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_VLAN_PROPAGATION_GET, dev_id, port_id, (a_uint32_t)mode);
    return rv;
}

sw_error_t
fal_port_vlan_trans_add(a_uint32_t dev_id, fal_port_t port_id, fal_vlan_trans_entry_t *entry)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_VLAN_TRANS_ADD, dev_id, port_id, entry);
    return rv;
}

sw_error_t
fal_port_vlan_trans_del(a_uint32_t dev_id, fal_port_t port_id, fal_vlan_trans_entry_t *entry)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_VLAN_TRANS_DEL, dev_id, port_id, entry);
    return rv;
}

sw_error_t
fal_port_vlan_trans_get(a_uint32_t dev_id, fal_port_t port_id, fal_vlan_trans_entry_t *entry)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_VLAN_TRANS_GET, dev_id, port_id, entry);
    return rv;
}

sw_error_t
fal_qinq_mode_set(a_uint32_t dev_id, fal_qinq_mode_t mode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_QINQ_MODE_SET, dev_id, (a_uint32_t)mode);
    return rv;
}

sw_error_t
fal_qinq_mode_get(a_uint32_t dev_id, fal_qinq_mode_t * mode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_QINQ_MODE_GET, dev_id, (a_uint32_t)mode);
    return rv;
}

sw_error_t
fal_port_qinq_role_set(a_uint32_t dev_id, fal_port_t port_id, fal_qinq_port_role_t role)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_QINQ_ROLE_SET, dev_id, port_id, (a_uint32_t)role);
    return rv;
}

sw_error_t
fal_port_qinq_role_get(a_uint32_t dev_id, fal_port_t port_id, fal_qinq_port_role_t * role)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_QINQ_ROLE_GET, dev_id, port_id, (a_uint32_t)role);
    return rv;
}

sw_error_t
fal_port_vlan_trans_iterate(a_uint32_t dev_id, fal_port_t port_id,
                            a_uint32_t * iterator, fal_vlan_trans_entry_t * entry)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_VLAN_TRANS_ITERATE, dev_id, port_id,
                    (a_uint32_t)iterator,(a_uint32_t)entry);
    return rv;
}

sw_error_t
fal_port_mac_vlan_xlt_set(a_uint32_t dev_id, fal_port_t port_id,
                          a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_MAC_VLAN_XLT_SET, dev_id, port_id, (a_uint32_t)enable);
    return rv;
}

sw_error_t
fal_port_mac_vlan_xlt_get(a_uint32_t dev_id, fal_port_t port_id,
                          a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_MAC_VLAN_XLT_GET, dev_id, port_id, (a_uint32_t)enable);
    return rv;
}

sw_error_t
fal_netisolate_set(a_uint32_t dev_id, a_uint32_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_NETISOLATE_SET, dev_id, enable);
    return rv;
}

sw_error_t
fal_netisolate_get(a_uint32_t dev_id, a_uint32_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_NETISOLATE_GET, dev_id, (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_eg_trans_filter_bypass_en_set(a_uint32_t dev_id, a_uint32_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_EG_FLTR_BYPASS_EN_SET, dev_id, enable);
    return rv;
}

sw_error_t
fal_eg_trans_filter_bypass_en_get(a_uint32_t dev_id, a_uint32_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_EG_FLTR_BYPASS_EN_GET, dev_id, (a_uint32_t) enable);
    return rv;
}

sw_error_t
fal_port_vrf_id_set(a_uint32_t dev_id, fal_port_t port_id,
                              a_uint32_t vrf_id)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_VRF_ID_SET, dev_id, port_id, vrf_id);
    return rv;
}

sw_error_t
fal_port_vrf_id_get(a_uint32_t dev_id, fal_port_t port_id,
                              a_uint32_t * vrf_id)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_VRF_ID_GET, dev_id, port_id, (a_uint32_t) vrf_id);
    return rv;
}
