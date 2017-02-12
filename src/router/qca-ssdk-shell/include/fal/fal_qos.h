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
 * @defgroup fal_qos FAL_QOS
 * @{
 */
#ifndef _FAL_QOS_H_
#define _FAL_QOS_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "common/sw.h"
#include "fal/fal_type.h"

    /**
    @brief This enum defines traffic scheduling mode.
    */
    typedef enum {
        FAL_SCH_SP_MODE = 0,    /**<  strict priority scheduling mode */
        FAL_SCH_WRR_MODE,       /**<   weight round robin scheduling mode*/
        FAL_SCH_MIX_MODE,       /**<  sp and wrr mixed scheduling mode */
        FAL_SCH_MIX_PLUS_MODE,  /**<  sp and wrr mixed plus scheduling mode */
        FAL_SCH_MODE_BUTT
    }
    fal_sch_mode_t;

    /**
    @brief This enum defines qos assignment mode.
    */
    typedef enum
    {
        FAL_QOS_DA_MODE = 0,    /**<   qos assignment based on destination mac address*/
        FAL_QOS_UP_MODE,        /**<   qos assignment based on 802.1p field in vlan tag*/
        FAL_QOS_DSCP_MODE,      /**<  qos assignment based on dscp field in ip header */
        FAL_QOS_PORT_MODE,      /**<  qos assignment based on port */
        FAL_QOS_FLOW_MODE,      /**<  qos assignment based on flow */
        FAL_QOS_MODE_BUTT
    } fal_qos_mode_t;

#define FAL_DOT1P_MIN    0
#define FAL_DOT1P_MAX    7

#define FAL_DSCP_MIN     0
#define FAL_DSCP_MAX     63


    sw_error_t
    fal_qos_sch_mode_set(a_uint32_t dev_id,
                         fal_sch_mode_t mode, const a_uint32_t weight[]);



    sw_error_t
    fal_qos_sch_mode_get(a_uint32_t dev_id,
                         fal_sch_mode_t * mode, a_uint32_t weight[]);



    sw_error_t
    fal_qos_queue_tx_buf_status_set(a_uint32_t dev_id, fal_port_t port_id,
                                    a_bool_t enable);


    sw_error_t
    fal_qos_queue_tx_buf_status_get(a_uint32_t dev_id, fal_port_t port_id,
                                    a_bool_t * enable);



    sw_error_t
    fal_qos_queue_tx_buf_nr_set(a_uint32_t dev_id, fal_port_t port_id,
                                fal_queue_t queue_id, a_uint32_t * number);



    sw_error_t
    fal_qos_queue_tx_buf_nr_get(a_uint32_t dev_id, fal_port_t port_id,
                                fal_queue_t queue_id, a_uint32_t * number);



    sw_error_t
    fal_qos_port_tx_buf_status_set(a_uint32_t dev_id, fal_port_t port_id,
                                   a_bool_t enable);



    sw_error_t
    fal_qos_port_tx_buf_status_get(a_uint32_t dev_id, fal_port_t port_id,
                                   a_bool_t * enable);

    sw_error_t
    fal_qos_port_red_en_set(a_uint32_t dev_id, fal_port_t port_id,
                            a_bool_t enable);


    sw_error_t
    fal_qos_port_red_en_get(a_uint32_t dev_id, fal_port_t port_id,
                            a_bool_t * enable);


    sw_error_t
    fal_qos_port_tx_buf_nr_set(a_uint32_t dev_id, fal_port_t port_id,
                               a_uint32_t * number);



    sw_error_t
    fal_qos_port_tx_buf_nr_get(a_uint32_t dev_id, fal_port_t port_id,
                               a_uint32_t * number);

    sw_error_t
    fal_qos_port_rx_buf_nr_set(a_uint32_t dev_id, fal_port_t port_id,
                               a_uint32_t * number);


    sw_error_t
    fal_qos_port_rx_buf_nr_get(a_uint32_t dev_id, fal_port_t port_id,
                               a_uint32_t * number);


    sw_error_t
    fal_cosmap_up_queue_set(a_uint32_t dev_id, a_uint32_t up,
                            fal_queue_t queue);



    sw_error_t
    fal_cosmap_up_queue_get(a_uint32_t dev_id, a_uint32_t up,
                            fal_queue_t * queue);



    sw_error_t
    fal_cosmap_dscp_queue_set(a_uint32_t dev_id, a_uint32_t dscp,
                              fal_queue_t queue);



    sw_error_t
    fal_cosmap_dscp_queue_get(a_uint32_t dev_id, a_uint32_t dscp,
                              fal_queue_t * queue);



    sw_error_t
    fal_qos_port_mode_set(a_uint32_t dev_id, fal_port_t port_id,
                          fal_qos_mode_t mode, a_bool_t enable);



    sw_error_t
    fal_qos_port_mode_get(a_uint32_t dev_id, fal_port_t port_id,
                          fal_qos_mode_t mode, a_bool_t * enable);



    sw_error_t
    fal_qos_port_mode_pri_set(a_uint32_t dev_id, fal_port_t port_id,
                              fal_qos_mode_t mode, a_uint32_t pri);



    sw_error_t
    fal_qos_port_mode_pri_get(a_uint32_t dev_id, fal_port_t port_id,
                              fal_qos_mode_t mode, a_uint32_t * pri);



    sw_error_t
    fal_qos_port_default_up_set(a_uint32_t dev_id, fal_port_t port_id,
                                a_uint32_t up);



    sw_error_t
    fal_qos_port_default_up_get(a_uint32_t dev_id, fal_port_t port_id,
                                a_uint32_t * up);


    sw_error_t
    fal_qos_port_sch_mode_set(a_uint32_t dev_id, a_uint32_t port_id,
                              fal_sch_mode_t mode, const a_uint32_t weight[]);


    sw_error_t
    fal_qos_port_sch_mode_get(a_uint32_t dev_id, a_uint32_t port_id,
                              fal_sch_mode_t * mode, a_uint32_t weight[]);


    sw_error_t
    fal_qos_port_default_spri_set(a_uint32_t dev_id, fal_port_t port_id,
                                  a_uint32_t spri);


    sw_error_t
    fal_qos_port_default_spri_get(a_uint32_t dev_id, fal_port_t port_id,
                                  a_uint32_t * spri);


    sw_error_t
    fal_qos_port_default_cpri_set(a_uint32_t dev_id, fal_port_t port_id,
                                  a_uint32_t cpri);


    sw_error_t
    fal_qos_port_default_cpri_get(a_uint32_t dev_id, fal_port_t port_id,
                                  a_uint32_t * cpri);

    sw_error_t
    fal_qos_port_force_spri_status_set(a_uint32_t dev_id, fal_port_t port_id,
                                       a_bool_t enable);

    sw_error_t
    fal_qos_port_force_spri_status_get(a_uint32_t dev_id, fal_port_t port_id,
                                       a_bool_t* enable);

    sw_error_t
    fal_qos_port_force_cpri_status_set(a_uint32_t dev_id, fal_port_t port_id,
                                       a_bool_t enable);

    sw_error_t
    fal_qos_port_force_cpri_status_get(a_uint32_t dev_id, fal_port_t port_id,
                                       a_bool_t* enable);

    sw_error_t
    fal_qos_queue_remark_table_set(a_uint32_t dev_id, fal_port_t port_id,
                                   fal_queue_t queue_id, a_uint32_t tbl_id, a_bool_t enable);


    sw_error_t
    fal_qos_queue_remark_table_get(a_uint32_t dev_id, fal_port_t port_id,
                                   fal_queue_t queue_id, a_uint32_t * tbl_id, a_bool_t * enable);


#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _PORT_QOS_H_ */
/**
 * @}
 */
