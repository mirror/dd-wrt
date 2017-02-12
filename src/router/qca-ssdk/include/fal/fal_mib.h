/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
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
 * @defgroup fal_mib FAL_MIB
 * @{
 */
#ifndef _FAL_MIB_H
#define _FAL_MIB_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "common/sw.h"
#include "fal/fal_type.h"

    /**@brief This structure defines the mib infomation.
    */
    typedef struct
    {
        a_uint32_t RxBroad;
        a_uint32_t RxPause;
        a_uint32_t RxMulti;
        a_uint32_t RxFcsErr;
        a_uint32_t RxAllignErr;
        a_uint32_t RxRunt;
        a_uint32_t RxFragment;
        a_uint32_t Rx64Byte;
        a_uint32_t Rx128Byte;
        a_uint32_t Rx256Byte;
        a_uint32_t Rx512Byte;
        a_uint32_t Rx1024Byte;
        a_uint32_t Rx1518Byte;
        a_uint32_t RxMaxByte;
        a_uint32_t RxTooLong;
        a_uint32_t RxGoodByte_lo;       /**<  low 32 bits of RxGoodByte statistc item */
        a_uint32_t RxGoodByte_hi;       /**<   high 32 bits of RxGoodByte statistc item*/
        a_uint32_t RxBadByte_lo;        /**<   low 32 bits of RxBadByte statistc item */
        a_uint32_t RxBadByte_hi;        /**<   high 32 bits of RxBadByte statistc item */
        a_uint32_t RxOverFlow;
        a_uint32_t Filtered;
        a_uint32_t TxBroad;
        a_uint32_t TxPause;
        a_uint32_t TxMulti;
        a_uint32_t TxUnderRun;
        a_uint32_t Tx64Byte;
        a_uint32_t Tx128Byte;
        a_uint32_t Tx256Byte;
        a_uint32_t Tx512Byte;
        a_uint32_t Tx1024Byte;
        a_uint32_t Tx1518Byte;
        a_uint32_t TxMaxByte;
        a_uint32_t TxOverSize;
        a_uint32_t TxByte_lo;       /**<  low 32 bits of TxByte statistc item */
        a_uint32_t TxByte_hi;       /**<  high 32 bits of TxByte statistc item */
        a_uint32_t TxCollision;
        a_uint32_t TxAbortCol;
        a_uint32_t TxMultiCol;
        a_uint32_t TxSingalCol;
        a_uint32_t TxExcDefer;
        a_uint32_t TxDefer;
        a_uint32_t TxLateCol;
        a_uint32_t RxUniCast;
        a_uint32_t TxUniCast;
    } fal_mib_info_t;


    sw_error_t
    fal_get_mib_info(a_uint32_t dev_id, fal_port_t port_id,
                     fal_mib_info_t * mib_info );

    sw_error_t
    fal_get_rx_mib_info(a_uint32_t dev_id, fal_port_t port_id,
                     fal_mib_info_t * mib_info );

    sw_error_t
    fal_get_tx_mib_info(a_uint32_t dev_id, fal_port_t port_id,
                     fal_mib_info_t * mib_info );


    sw_error_t
    fal_mib_status_set(a_uint32_t dev_id, a_bool_t enable);



    sw_error_t
    fal_mib_status_get(a_uint32_t dev_id, a_bool_t * enable);


    sw_error_t
    fal_mib_port_flush_counters(a_uint32_t dev_id, fal_port_t port_id);


    sw_error_t
    fal_mib_cpukeep_set(a_uint32_t dev_id, a_bool_t  enable);

    sw_error_t
    fal_mib_cpukeep_get(a_uint32_t dev_id, a_bool_t * enable);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _FAL_MIB_H */
/**
 * @}
 */
