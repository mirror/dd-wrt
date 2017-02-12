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



/**
 * @defgroup fal_port_vlan FAL_PORT_VLAN
 * @{
 */
#ifndef _FAL_PORT_VLAN_H_
#define _FAL_PORT_VLAN_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "common/sw.h"
#include "fal/fal_type.h"

    /**
    @brief This enum defines 802.1q mode type.
    */
    typedef enum {
        FAL_1Q_DISABLE = 0, /**<  802.1q mode disbale, port based vlan */
        FAL_1Q_SECURE,      /**<   secure mode, packets which vid isn't in vlan table or source port isn't in vlan port member will be discarded.*/
        FAL_1Q_CHECK,       /**<   check mode, packets which vid isn't in vlan table will be discarded, packets which source port isn't in vlan port member will forward base on vlan port member*/
        FAL_1Q_FALLBACK,    /**<   fallback mode, packets which vid isn't in vlan table will forwarded base on port vlan, packet's which source port isn't in vlan port member will forward base on vlan port member.*/
        FAL_1Q_MODE_BUTT
    }
    fal_pt_1qmode_t;

    /**
    @brief This enum defines receive packets tagged mode.
    */
    typedef enum
    {
        FAL_INVLAN_ADMIT_ALL = 0,  /**<  receive all packets include tagged and untagged */
        FAL_INVLAN_ADMIT_TAGGED,   /**<  only receive tagged packets*/
        FAL_INVLAN_ADMIT_UNTAGGED, /**<  only receive untagged packets include priority tagged */
        FAL_INVLAN_MODE_BUTT
    } fal_pt_invlan_mode_t;

    /**
    @brief This enum defines vlan propagation mode.
    */
    typedef enum
    {
        FAL_VLAN_PROPAGATION_DISABLE = 0, /**<  vlan propagation disable */
        FAL_VLAN_PROPAGATION_CLONE,       /**<  vlan paopagation mode is clone */
        FAL_VLAN_PROPAGATION_REPLACE,     /**<  vlan paopagation mode is repalce */
        FAL_VLAN_PROPAGATION_MODE_BUTT
    } fal_vlan_propagation_mode_t;

    /**
      @details  Fields description:

     o_vid - original vlan id
     s_vid - service vid id
     c_vid - custom vid id
     bi_dir - entry search direction
     forward_dir - entry search direction only be forward
     reverse_dir - entry search direction only be reverse
     o_vid_is_cvid - o_vid in entry means c_vid not s_vid
     s_vid_enable  - s_vid in entry is valid
     c_vid_enable  - c_vid in entry is valid
     one_2_one_vlan- the entry used for 1:1 vlan
    @brief This structure defines the vlan translation entry.

    */
    typedef struct
    {
        a_uint32_t o_vid;
        a_uint32_t s_vid;
        a_uint32_t c_vid;
        a_bool_t   bi_dir;       /**< lookup can be forward and reverse*/
        a_bool_t   forward_dir;  /**< lookup direction only can be from o_vid to s_vid and/or c_vid*/
        a_bool_t   reverse_dir;  /**< lookup direction only can be from s_vid and/or c_vid to o_vid*/
        a_bool_t   o_vid_is_cvid;
        a_bool_t   s_vid_enable;
        a_bool_t   c_vid_enable;
        a_bool_t   one_2_one_vlan;
    } fal_vlan_trans_entry_t;

    /**
    @brief This enum defines qinq working mode.
    */
    typedef enum
    {
        FAL_QINQ_CTAG_MODE = 0,
        FAL_QINQ_STAG_MODE,
        FAL_QINQ_MODE_BUTT
    } fal_qinq_mode_t;

    /**
    @brief This enum defines port role in qinq mode.
    */
    typedef enum
    {
        FAL_QINQ_EDGE_PORT = 0,
        FAL_QINQ_CORE_PORT,
        FAL_QINQ_PORT_ROLE_BUTT
    } fal_qinq_port_role_t;


    sw_error_t
    fal_port_1qmode_set(a_uint32_t dev_id, fal_port_t port_id,
                        fal_pt_1qmode_t port_1qmode);



    sw_error_t
    fal_port_1qmode_get(a_uint32_t dev_id, fal_port_t port_id,
                        fal_pt_1qmode_t * pport_1qmode);



    sw_error_t
    fal_port_egvlanmode_set(a_uint32_t dev_id, fal_port_t port_id,
                            fal_pt_1q_egmode_t port_egvlanmode);



    sw_error_t
    fal_port_egvlanmode_get(a_uint32_t dev_id, fal_port_t port_id,
                            fal_pt_1q_egmode_t * pport_egvlanmode);



    sw_error_t
    fal_portvlan_member_add(a_uint32_t dev_id, fal_port_t port_id,
                            fal_port_t mem_port_id);



    sw_error_t
    fal_portvlan_member_del(a_uint32_t dev_id, fal_port_t port_id,
                            fal_port_t mem_port_id);



    sw_error_t
    fal_portvlan_member_update(a_uint32_t dev_id, fal_port_t port_id,
                               fal_pbmp_t mem_port_map);



    sw_error_t
    fal_portvlan_member_get(a_uint32_t dev_id, fal_port_t port_id,
                            fal_pbmp_t * mem_port_map);



    sw_error_t
    fal_port_default_vid_set(a_uint32_t dev_id, fal_port_t port_id,
                             a_uint32_t vid);



    sw_error_t
    fal_port_default_vid_get(a_uint32_t dev_id, fal_port_t port_id,
                             a_uint32_t * vid);



    sw_error_t
    fal_port_force_default_vid_set(a_uint32_t dev_id, fal_port_t port_id,
                                   a_bool_t enable);



    sw_error_t
    fal_port_force_default_vid_get(a_uint32_t dev_id, fal_port_t port_id,
                                   a_bool_t * enable);



    sw_error_t
    fal_port_force_portvlan_set(a_uint32_t dev_id, fal_port_t port_id,
                                a_bool_t enable);



    sw_error_t
    fal_port_force_portvlan_get(a_uint32_t dev_id, fal_port_t port_id,
                                a_bool_t * enable);



    sw_error_t
    fal_port_nestvlan_set(a_uint32_t dev_id, fal_port_t port_id,
                          a_bool_t enable);



    sw_error_t
    fal_port_nestvlan_get(a_uint32_t dev_id, fal_port_t port_id,
                          a_bool_t * enable);



    sw_error_t
    fal_nestvlan_tpid_set(a_uint32_t dev_id, a_uint32_t tpid);



    sw_error_t
    fal_nestvlan_tpid_get(a_uint32_t dev_id, a_uint32_t * tpid);


    sw_error_t
    fal_port_invlan_mode_set(a_uint32_t dev_id, fal_port_t port_id,
                             fal_pt_invlan_mode_t mode);


    sw_error_t
    fal_port_invlan_mode_get(a_uint32_t dev_id, fal_port_t port_id,
                             fal_pt_invlan_mode_t * mode);


    sw_error_t
    fal_port_tls_set(a_uint32_t dev_id, fal_port_t port_id,
                     a_bool_t enable);


    sw_error_t
    fal_port_tls_get(a_uint32_t dev_id, fal_port_t port_id,
                     a_bool_t * enable);


    sw_error_t
    fal_port_pri_propagation_set(a_uint32_t dev_id, fal_port_t port_id,
                                 a_bool_t enable);


    sw_error_t
    fal_port_pri_propagation_get(a_uint32_t dev_id, fal_port_t port_id,
                                 a_bool_t * enable);


    sw_error_t
    fal_port_default_svid_set(a_uint32_t dev_id, fal_port_t port_id,
                              a_uint32_t vid);


    sw_error_t
    fal_port_default_svid_get(a_uint32_t dev_id, fal_port_t port_id,
                              a_uint32_t * vid);


    sw_error_t
    fal_port_default_cvid_set(a_uint32_t dev_id, fal_port_t port_id,
                              a_uint32_t vid);


    sw_error_t
    fal_port_default_cvid_get(a_uint32_t dev_id, fal_port_t port_id,
                              a_uint32_t * vid);


    sw_error_t
    fal_port_vlan_propagation_set(a_uint32_t dev_id, fal_port_t port_id,
                                  fal_vlan_propagation_mode_t mode);


    sw_error_t
    fal_port_vlan_propagation_get(a_uint32_t dev_id, fal_port_t port_id,
                                  fal_vlan_propagation_mode_t * mode);


    sw_error_t
    fal_port_vlan_trans_add(a_uint32_t dev_id, fal_port_t port_id, fal_vlan_trans_entry_t *entry);


    sw_error_t
    fal_port_vlan_trans_del(a_uint32_t dev_id, fal_port_t port_id, fal_vlan_trans_entry_t *entry);


    sw_error_t
    fal_port_vlan_trans_get(a_uint32_t dev_id, fal_port_t port_id, fal_vlan_trans_entry_t *entry);


    sw_error_t
    fal_qinq_mode_set(a_uint32_t dev_id, fal_qinq_mode_t mode);


    sw_error_t
    fal_qinq_mode_get(a_uint32_t dev_id, fal_qinq_mode_t * mode);


    sw_error_t
    fal_port_qinq_role_set(a_uint32_t dev_id, fal_port_t port_id, fal_qinq_port_role_t role);


    sw_error_t
    fal_port_qinq_role_get(a_uint32_t dev_id, fal_port_t port_id, fal_qinq_port_role_t * role);


    sw_error_t
    fal_port_vlan_trans_iterate(a_uint32_t dev_id, fal_port_t port_id,
                                a_uint32_t * iterator, fal_vlan_trans_entry_t * entry);


    sw_error_t
    fal_port_mac_vlan_xlt_set(a_uint32_t dev_id, fal_port_t port_id,
                              a_bool_t enable);


    sw_error_t
    fal_port_mac_vlan_xlt_get(a_uint32_t dev_id, fal_port_t port_id,
                              a_bool_t * enable);

    sw_error_t
    fal_netisolate_set(a_uint32_t dev_id, a_uint32_t enable);

    sw_error_t
    fal_netisolate_get(a_uint32_t dev_id, a_uint32_t * enable);

    sw_error_t
    fal_eg_trans_filter_bypass_en_set(a_uint32_t dev_id, a_uint32_t enable);

    sw_error_t
    fal_eg_trans_filter_bypass_en_get(a_uint32_t dev_id, a_uint32_t * enable);

    sw_error_t
    fal_port_vrf_id_set(a_uint32_t dev_id, fal_port_t port_id,
                              a_uint32_t vrf_id);

    sw_error_t
    fal_port_vrf_id_get(a_uint32_t dev_id, fal_port_t port_id,
                              a_uint32_t * vrf_id);


#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _PORT_VLAN_H_ */
/**
 * @}
 */
