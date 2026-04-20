/*
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

/*
 * nss_ppe_vxlanmgr.h
 */

#ifndef __NSS_PPE_VXLANMGR_H
#define __NSS_PPE_VXLANMGR_H

/*
 * Define the nack limit for the VXLAN VP.
 * The vp_status is updated in the hash table for every-remote. Whenever the ECM-queries, the vp_status is returned.
 * There is some small time needed, to create VP and update the hash table with the VP status. During this time, the VP status is returned as IN-PROGRESS.
 * But there may be a chance when the hash memory allocation may itself fail. Therefore after NSS_PPE_VXLANMGR_VP_STATUS_DEFAULT_MAX_NACK for each remote, the VP status will be sent as FAILURE.
 * This is to allow ECM to go-ahead with different accel engine if not PPE.
 * This is a very rare-case to happen. Hence pretty high value is chosen so that sufficient time is given for the clients to join the remote and create the VP.
 */
#define NSS_PPE_VXLANMGR_VP_STATUS_DEFAULT_MAX_NACK 250

/*
 * enum nss_ppe_vxlanmgr_vp_creation
 *	VxLAN VP creation status.
 */
enum nss_ppe_vxlanmgr_vp_creation {
	NSS_PPE_VXLANMGR_VP_CREATION_SUCCESS,	/* VP has been created successfully for the child/dummy nss-netdevice. */
	NSS_PPE_VXLANMGR_VP_CREATION_IN_PROGRESS,	/* VP creation is in progress for the child/dummy nss-netdevice. */
	NSS_PPE_VXLANMGR_VP_CREATION_FAILED,	/* VP creation failed for the child/ dummy nss-netdevice. */
	NSS_PPE_VXLANMGR_VP_CREATION_INVALID	/* Invalid state. */
};

/*
 * VXLAN tunnel functionality APIs exported
 */
enum nss_ppe_vxlanmgr_vp_creation nss_ppe_vxlanmgr_get_ifindex_and_vp_status(struct net_device *dev, uint32_t *remote_ip, uint8_t ip_type, int *ifindex);
#endif
