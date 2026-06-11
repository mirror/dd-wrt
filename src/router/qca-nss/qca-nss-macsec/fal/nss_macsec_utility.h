/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
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

#ifndef _NSS_MACSEC_UTILITY_H_
#define _NSS_MACSEC_UTILITY_H_

#include "nss_macsec_types.h"

#define FAL_SECY_ID_NUM                 (fal_macsec_secy_num_get())
#define FAL_SECY_SC_SA_MODE(secy_id)    (fal_secy_sc_sa_mapping_mode_get(secy_id))
#define FAL_SECY_SA_NUM_PER_SC(secy_id) (fal_secy_get_sa_num_per_sc(secy_id))
#define FAL_SECY_CHANNEL_NUM(secy_id)   (fal_secy_channel_num(secy_id))
#define FAL_SECY_ID_TO_PORT(secy_id)    (fal_macsec_port_get_by_device_id(secy_id))


struct macsec_port *fal_macsec_port_get_by_device_id(u32 dev_id);

u32 fal_macsec_secy_num_get(void);

u32 fal_secy_sc_sa_mapping_mode_get(u32 secy_id);

u32 fal_secy_get_sa_num_per_sc(u32 secy_id);

u32 fal_secy_channel_num(u32 secy_id);

u32 fal_tx_channel_2_sc_id(u32 secy_id, u32 channel);

u32 fal_tx_sc_id_2_channel(u32 secy_id, u32 sc_id);

u32 fal_rx_channel_2_sc_id(u32 secy_id, u32 channel);

u32 fal_rx_sc_id_2_channel(u32 secy_id, u32 sc_id);

#define FAL_API_LOCK nss_macsec_mutex_lock();
#define FAL_API_UNLOCK nss_macsec_mutex_unlock();

u32 nss_macsec_mutex_init(void);
u32 nss_macsec_mutex_destroy(void);

u32 nss_macsec_mutex_lock(void);
u32 nss_macsec_mutex_unlock(void);

#endif /* _NSS_MACSEC_UTILITY_H_ */
