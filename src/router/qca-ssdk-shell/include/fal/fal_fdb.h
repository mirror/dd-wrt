/*
 * Copyright (c) 2014, 2015, The Linux Foundation. All rights reserved.
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
 * @defgroup fal_fdb FAL_FDB
 * @{
 */
#ifndef _FAL_FDB_H_
#define _FAL_FDB_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "common/sw.h"
#include "fal/fal_type.h"

    /**
      @details  Fields description:

        portmap_en - If value of portmap_en is A_TRUE then port.map is valid
        otherwise port.id is valid.


        leaky_en - If value of leaky_en is A_TRUE then packets which
        destination address equals addr in this entry would be leaky.


        mirror_en - If value of mirror_en is A_TRUE then packets which
        destination address equals addr in this entry would be mirrored.


        clone_en - If value of clone_en is A_TRUE which means this address is
        a mac clone address.
    @brief This structure defines the Fdb entry.

    */
    typedef struct
    {
        fal_mac_addr_t addr;
        a_uint16_t    fid;
        fal_fwd_cmd_t dacmd;
        fal_fwd_cmd_t sacmd;
        union
        {
            a_uint32_t id;
            fal_pbmp_t map;
        } port;
        a_bool_t portmap_en;
        a_bool_t is_multicast;
        a_bool_t static_en;
        a_bool_t leaky_en;
        a_bool_t mirror_en;
        a_bool_t clone_en;
        a_bool_t cross_pt_state;
        a_bool_t da_pri_en;
        a_uint8_t da_queue;
        a_bool_t white_list_en;
        a_bool_t load_balance_en;
        a_uint8_t load_balance;
    } fal_fdb_entry_t;

#define FAL_FDB_DEL_STATIC   0x1

    typedef struct
    {
        a_bool_t port_en;
        a_bool_t fid_en;
        a_bool_t multicast_en;
    } fal_fdb_op_t;

    typedef enum
    {
        INVALID_VLAN_SVL=0,
        INVALID_VLAN_IVL
    } fal_fdb_smode;

	typedef struct
    {
        fal_mac_addr_t addr;
        a_uint16_t    fid;
        a_uint8_t load_balance;
    } fal_fdb_rfs_t;

	sw_error_t
    fal_fdb_rfs_set(a_uint32_t dev_id, fal_fdb_rfs_t * entry);

	sw_error_t
    fal_fdb_rfs_del(a_uint32_t dev_id, fal_fdb_rfs_t * entry);

    sw_error_t
    fal_fdb_add(a_uint32_t dev_id, const fal_fdb_entry_t * entry);



    sw_error_t
    fal_fdb_del_all(a_uint32_t dev_id, a_uint32_t flag);



    sw_error_t
    fal_fdb_del_by_port(a_uint32_t dev_id, a_uint32_t port_id, a_uint32_t flag);



    sw_error_t
    fal_fdb_del_by_mac(a_uint32_t dev_id, const fal_fdb_entry_t *entry);



    sw_error_t
    fal_fdb_first(a_uint32_t dev_id, fal_fdb_entry_t * entry);



    sw_error_t
    fal_fdb_next(a_uint32_t dev_id, fal_fdb_entry_t * entry);



    sw_error_t
    fal_fdb_find(a_uint32_t dev_id, fal_fdb_entry_t * entry);



    sw_error_t
    fal_fdb_port_learn_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);



    sw_error_t
    fal_fdb_port_learn_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable);



    sw_error_t
    fal_fdb_age_ctrl_set(a_uint32_t dev_id, a_bool_t enable);



    sw_error_t
    fal_fdb_age_ctrl_get(a_uint32_t dev_id, a_bool_t * enable);


    sw_error_t
    fal_fdb_vlan_ivl_svl_set(a_uint32_t dev_id, fal_fdb_smode smode);


    sw_error_t
    fal_fdb_vlan_ivl_svl_get(a_uint32_t dev_id, fal_fdb_smode * smode);


    sw_error_t
    fal_fdb_age_time_set(a_uint32_t dev_id, a_uint32_t * time);



    sw_error_t
    fal_fdb_age_time_get(a_uint32_t dev_id, a_uint32_t * time);


    sw_error_t
    fal_fdb_iterate(a_uint32_t dev_id, a_uint32_t * iterator, fal_fdb_entry_t * entry);


    sw_error_t
    fal_fdb_extend_next(a_uint32_t dev_id, fal_fdb_op_t * option,
                        fal_fdb_entry_t * entry);


    sw_error_t
    fal_fdb_extend_first(a_uint32_t dev_id, fal_fdb_op_t * option,
                         fal_fdb_entry_t * entry);


    sw_error_t
    fal_fdb_transfer(a_uint32_t dev_id, fal_port_t old_port, fal_port_t new_port,
                     a_uint32_t fid, fal_fdb_op_t * option);


    sw_error_t
    fal_port_fdb_learn_limit_set(a_uint32_t dev_id, fal_port_t port_id,
                                 a_bool_t enable, a_uint32_t cnt);


    sw_error_t
    fal_port_fdb_learn_limit_get(a_uint32_t dev_id, fal_port_t port_id,
                                 a_bool_t * enable, a_uint32_t * cnt);


    sw_error_t
    fal_port_fdb_learn_exceed_cmd_set(a_uint32_t dev_id, fal_port_t port_id,
                                      fal_fwd_cmd_t cmd);


    sw_error_t
    fal_port_fdb_learn_exceed_cmd_get(a_uint32_t dev_id, fal_port_t port_id,
                                      fal_fwd_cmd_t * cmd);


    sw_error_t
    fal_fdb_learn_limit_set(a_uint32_t dev_id, a_bool_t enable, a_uint32_t cnt);


    sw_error_t
    fal_fdb_learn_limit_get(a_uint32_t dev_id, a_bool_t * enable, a_uint32_t * cnt);


    sw_error_t
    fal_fdb_learn_exceed_cmd_set(a_uint32_t dev_id, fal_fwd_cmd_t cmd);


    sw_error_t
    fal_fdb_learn_exceed_cmd_get(a_uint32_t dev_id, fal_fwd_cmd_t * cmd);


    sw_error_t
    fal_fdb_resv_add(a_uint32_t dev_id, fal_fdb_entry_t * entry);

    sw_error_t
    fal_fdb_resv_del(a_uint32_t dev_id, fal_fdb_entry_t * entry);


    sw_error_t
    fal_fdb_resv_find(a_uint32_t dev_id, fal_fdb_entry_t * entry);


    sw_error_t
    fal_fdb_resv_iterate(a_uint32_t dev_id, a_uint32_t * iterator, fal_fdb_entry_t * entry);


    sw_error_t
    fal_fdb_port_learn_static_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);


    sw_error_t
    fal_fdb_port_learn_static_get(a_uint32_t dev_id, fal_port_t port_id,
                                  a_bool_t * enable);

    sw_error_t
    fal_fdb_port_add(a_uint32_t dev_id, a_uint32_t fid, fal_mac_addr_t * addr, fal_port_t port_id);

    sw_error_t
    fal_fdb_port_del(a_uint32_t dev_id, a_uint32_t fid, fal_mac_addr_t * addr, fal_port_t port_id);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _FAL_FDB_H_ */
/**
 * @}
 */
