/*
 * Wireless Multicast Forwarding (WMF)
 *
 * WMF is forwarding multicast packets as unicast packets to
 * multicast group members in a BSS
 *
 * Supported protocol families: IPV4
 *
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_wmf_linux.c 627731 2016-03-28 08:54:35Z $
 */
#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>

#include <epivers.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmdevs.h>

#include <proto/ethernet.h>
#include <proto/bcmevent.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_linux.h>
#include <dhd_bus.h>
#include <dhd_proto.h>
#include <dhd_dbg.h>

#include <dhd_wmf_linux.h>

extern bool
dhd_is_rxthread_enabled(dhd_pub_t *dhdp);

#if !defined(BCM_NO_WOFA)
extern int
dhd_wmf_fwder_sendup(void *wrapper, void *p);
#endif /* !BCM_NO_WOFA */

/*
 * Description: This function is called to instantiate emf
 *		and igs instance on enabling WMF
 *
 * Input:       dhdp - pointer to dhd_pub_t
 *		bssidx - BSS index
 */
int32
dhd_wmf_instance_add(dhd_pub_t *dhdp, uint32 bssidx)
{
	dhd_wmf_t *wmf = dhd_wmf_conf(dhdp, bssidx);
	dhd_wmf_instance_t *wmf_inst;
	emfc_wrapper_t wmf_emfc;
	igsc_wrapper_t wmf_igsc;
	dhd_wmf_wrapper_t *wmfh;
	char inst_id[10];

	wmf_inst = MALLOC(dhdp->osh, sizeof(dhd_wmf_instance_t));
	if (!wmf_inst) {
		DHD_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
			bssidx, __FUNCTION__, MALLOCED(dhdp->osh)));
		return BCME_ERROR;
	}

	wmfh = MALLOC(dhdp->osh, sizeof(dhd_wmf_wrapper_t));
	if (!wmfh) {
		DHD_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
			bssidx, __FUNCTION__, MALLOCED(dhdp->osh)));
		MFREE(dhdp->osh, wmf_inst, sizeof(dhd_wmf_instance_t));
		return BCME_ERROR;
	}

	wmfh->dhdp = dhdp;
	wmfh->bssidx = bssidx;

	/* Fill in the wmf efmc wrapper functions */
	wmf_emfc.forward_fn = dhd_wmf_forward;
	wmf_emfc.sendup_fn = dhd_wmf_sendup;
	wmf_emfc.hooks_register_fn = dhd_wmf_hooks_register;
	wmf_emfc.hooks_unregister_fn = dhd_wmf_hooks_unregister;

	/* Create Instance ID */
	sprintf(inst_id, "wmf%d", bssidx);

	/* Create EMFC instance for WMF */
	wmf_inst->emfci = emfc_init((int8 *)inst_id, wmfh, dhdp->osh, &wmf_emfc);
	if (wmf_inst->emfci == NULL) {
		DHD_ERROR(("wl%d: %s: WMF EMFC init failed\n", bssidx, __FUNCTION__));
		MFREE(dhdp->osh, wmf_inst, sizeof(dhd_wmf_instance_t));
		MFREE(dhdp->osh, wmfh, sizeof(dhd_wmf_instance_t));
		return BCME_ERROR;
	}

	/* Fill in the wmf igsc wrapper functions */
	wmf_igsc.igs_broadcast = dhd_wmf_igs_broadcast;

	/* Create IGSC instance */
	wmf_inst->igsci = igsc_init((int8 *)inst_id, wmfh, dhdp->osh, &wmf_igsc);
	if (wmf_inst->igsci == NULL) {
		DHD_ERROR(("wl%d: %s: WMF IGSC init failed\n", bssidx, __FUNCTION__));
		/* Free the earlier allocated resources */
		emfc_exit(wmf_inst->emfci);
		MFREE(dhdp->osh, wmf_inst, sizeof(dhd_wmf_instance_t));
		MFREE(dhdp->osh, wmfh, sizeof(dhd_wmf_instance_t));
		return BCME_ERROR;
	}

	/* Set the wmf instance pointer inside ifp */
	wmf->wmf_instance = wmf_inst;
	wmf->wmfh = wmfh;

	return BCME_OK;
}

/*
 * Description: This function is called to start wmf operation
 *              when enabled
 *
 * Input:       pub - pointer to dhd_pub_t
 *		bssidx - BSS index
 */
int
dhd_wmf_start(dhd_pub_t *pub, uint32 bssidx)
{
	emf_cfg_request_t *req;
	char inst_id[10];
	dhd_wmf_t *wmf = dhd_wmf_conf(pub, bssidx);
	dhd_wmf_instance_t *wmf_inst = wmf->wmf_instance;

	if (!wmf_inst) {
		return BCME_ERROR;
	}

	if (!(req = (emf_cfg_request_t *)MALLOC(pub->osh, sizeof(emf_cfg_request_t)))) {
		DHD_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			bssidx, __FUNCTION__, MALLOCED(pub->osh)));
		return BCME_ERROR;
	}
	bzero((char *)req, sizeof(emf_cfg_request_t));

	sprintf(inst_id, "wmf%d", bssidx);
	strcpy((char *)req->inst_id, inst_id);
	req->command_id = EMFCFG_CMD_EMF_ENABLE;
	req->size = sizeof(bool);
	req->oper_type = EMFCFG_OPER_TYPE_SET;
	*(bool *)req->arg = TRUE;

	emfc_cfg_request_process(wmf_inst->emfci, req);

	if (req->status != EMFCFG_STATUS_SUCCESS) {
		DHD_ERROR(("wl%d: %s: failed\n", bssidx, __FUNCTION__));
		MFREE(pub->osh, req, sizeof(emf_cfg_request_t));
		return BCME_ERROR;
	}

	MFREE(pub->osh, req, sizeof(emf_cfg_request_t));

	return BCME_OK;
}

/*
 * Description: This function is called to stop wmf
 *		operation it is disabled or interface is down
 *
 * Input:   pub - pointer to dhd_pub_t
 *	    bssidx - BSS index
 */
void
dhd_wmf_stop(dhd_pub_t *pub, uint32 bssidx)
{
	emf_cfg_request_t *req;
	char inst_id[10];
	dhd_wmf_t *wmf = dhd_wmf_conf(pub, bssidx);
	dhd_wmf_instance_t *wmf_inst = wmf->wmf_instance;

	if (!wmf_inst) {
		return;
	}

	if (!(req = (emf_cfg_request_t *)MALLOC(pub->osh, sizeof(emf_cfg_request_t)))) {
		DHD_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			bssidx, __FUNCTION__, MALLOCED(pub->osh)));
		return;
	}
	bzero((char *)req, sizeof(emf_cfg_request_t));

	sprintf(inst_id, "wmf%d", bssidx);
	strcpy((char *)req->inst_id, inst_id);
	req->command_id = EMFCFG_CMD_EMF_ENABLE;
	req->size = sizeof(bool);
	req->oper_type = EMFCFG_OPER_TYPE_SET;
	*(bool *)req->arg = FALSE;

	emfc_cfg_request_process(wmf_inst->emfci, req);
	if (req->status != EMFCFG_STATUS_SUCCESS) {
		DHD_ERROR(("wl%d: %s: failed\n", bssidx, __FUNCTION__));
		MFREE(pub->osh, req, sizeof(emf_cfg_request_t));
		return;
	}
	MFREE(pub->osh, req, sizeof(emf_cfg_request_t));
}

/*
 * Description: Stop and delete WMF instance
 *
 * Input:   ptr - pointer to ifp
 *	    bssidx - BSS index
 */
void
dhd_wmf_cleanup(dhd_pub_t *pub, uint32 bssidx)
{
	dhd_wmf_t *wmf = dhd_wmf_conf(pub, bssidx);

	if (wmf->wmf_enable) {
		/* Stop WMF if it is enabled for this BSS */
		dhd_wmf_stop(pub, bssidx);
		/* Delete WMF instance for this bsscfg */
		dhd_wmf_instance_del(pub, bssidx);
	}

	return;
}

/* Enable/Disable sending multicast packets to host for WMF instance */
int
dhd_wmf_mcast_data_sendup(dhd_pub_t *pub, uint32 bssidx, bool set, bool enable)
{
	emf_cfg_request_t *req;
	dhd_wmf_t *wmf = dhd_wmf_conf(pub, bssidx);
	dhd_wmf_instance_t *wmf_inst = wmf->wmf_instance;
	int arg;

	if (!wmf_inst) {
		DHD_ERROR(("%s failed: WMF not enabled\n", __FUNCTION__));
		return BCME_ERROR;
	}

	if (!(req = (emf_cfg_request_t *)MALLOC(pub->osh, sizeof(emf_cfg_request_t)))) {
		DHD_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			bssidx, __FUNCTION__, MALLOCED(pub->osh)));
		return BCME_ERROR;
	}
	bzero((char *)req, sizeof(emf_cfg_request_t));

	snprintf((char *)req->inst_id, sizeof(req->inst_id), "wmf%d", bssidx);
	req->command_id = EMFCFG_CMD_MC_DATA_IND;
	req->size = sizeof(bool);
	req->oper_type = EMFCFG_OPER_TYPE_GET;
	if (set) {
		req->oper_type = EMFCFG_OPER_TYPE_SET;
		*(bool *)req->arg = enable;
	}

	emfc_cfg_request_process(wmf_inst->emfci, req);
	if (req->status != EMFCFG_STATUS_SUCCESS) {
		DHD_ERROR(("%s failed\n", __FUNCTION__));
		MFREE(pub->osh, req, sizeof(emf_cfg_request_t));
		return BCME_ERROR;
	}
	if (set) {
		MFREE(pub->osh, req, sizeof(emf_cfg_request_t));
		return BCME_OK;
	}

	arg = (int)(*((bool *)req->arg));
	MFREE(pub->osh, req, sizeof(emf_cfg_request_t));
	return arg;
}

/*
 * Description: This function is called to destroy emf
 *		and igs instances on disabling WMF
 *
 * Input:       dhdp - pointer to dhd_pub_t
 *		bssidx - BSS index
 */
void
dhd_wmf_instance_del(dhd_pub_t *dhdp, uint32 bssidx)
{
	dhd_wmf_t *wmf = dhd_wmf_conf(dhdp, bssidx);
	dhd_wmf_instance_t *wmf_inst = wmf->wmf_instance;

	if (!wmf_inst)
		return;

	/* Free the EMFC instance */
	emfc_exit(wmf_inst->emfci);

	/* Free the IGSC instance */
	igsc_exit(wmf_inst->igsci);

	/* Free the WMF instance */
	MFREE(dhdp->osh, wmf_inst, sizeof(dhd_wmf_instance_t));

	/* Free the WMF wrapper for EMFC */
	MFREE(dhdp->osh, wmf->wmfh, sizeof(dhd_wmf_wrapper_t));

	/* Make the pointer NULL */
	wmf_inst = NULL;
	wmf->wmfh = NULL;

	return;
}

/*
 * Description: WMF Packet forwarding routine
 *		Convert the packet to unicast and forward
 *
 * Input:   wrapper - pointer to dhd_put_t
 *	    p - pointer to the packet
 *	    mgrp_ip - Group IP address
 *	    txif - STA Info
 *	    rt_port - packet from BSS or DS
 */
int32
dhd_wmf_forward(void *wrapper, void *p, uint32 mgrp_ip, void *txif, bool rt_port)
{
	dhd_wmf_wrapper_t *wmfh = (dhd_wmf_wrapper_t*)wrapper;
	dhd_pub_t *dhdp = wmfh->dhdp;
	uchar reorder_info_buf[WLHOST_REORDERDATA_TOTLEN];
	uint reorder_info_len;
	struct ether_header *eh;
	int ifidx = 0, ret = 0;
	dhd_sta_t *sta = (dhd_sta_t *)txif;
	dhd_wmf_t *wmf = dhd_wmf_conf(dhdp, wmfh->bssidx);
	dhd_wmf_instance_t *wmf_inst = wmf->wmf_instance;

	if (!wmf_inst) {
		return BCME_TXFAIL;
	}
	ASSERT(txif);

	eh = (struct ether_header *)PKTDATA(dhdp->osh, p);

	/* If the protocol uses a data header, check and remove it */
	if (dhd_prot_hdrpull(dhdp, &ifidx, p, reorder_info_buf,
		&reorder_info_len) != 0) {
		DHD_ERROR(("%s error dhd_prot_hdrpull\n", __FUNCTION__));
		PKTFREE(dhdp->osh, p, FALSE);
		dhdp->rx_errors++;
		return BCME_TXFAIL;
	}

	memcpy(eh->ether_dhost, sta->ea.octet, ETHER_ADDR_LEN);

	ret = dhd_sendpkt(dhdp, wmfh->bssidx, p);

	if (ret) {
		DHD_INFO(("%s: WMF: send failure\n", __FUNCTION__));
	}

	return ret;
}

/*
 * Description: WMF Packet sendup routine - Sendup to host
 *
 * Input:   wrapper - pointer to dhd_put_t
 *	    p - pointer to the packet
 */
void
dhd_wmf_sendup(void *wrapper, void *p)
{
	struct sk_buff *skb;
	dhd_wmf_wrapper_t *wmfh = (dhd_wmf_wrapper_t*)wrapper;
	dhd_pub_t *dhdp = wmfh->dhdp;
	uchar reorder_info_buf[WLHOST_REORDERDATA_TOTLEN];
	uint reorder_info_len;
	int ifidx = 0;
	void *skbhead = NULL;
	void *skbprev = NULL;
	struct ether_header *eh;
	dhd_wmf_t *wmf = dhd_wmf_conf(dhdp, wmfh->bssidx);
	dhd_wmf_instance_t *wmf_inst = wmf->wmf_instance;
#if defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3) && defined(DHD_WMF) && \
	!defined(BCM_NO_WOFA)
	int ret = 0;
#endif /* BCM_ROUTER_DHD && BCM_GMAC3 && DHD_WMF && !BCM_NO_WOFA */

	if (!wmf_inst) {
		return;
	}

	eh = (struct ether_header *)PKTDATA(dhdp->osh, p);

	/* If the protocol uses a data header, check and remove it */
	if (dhd_prot_hdrpull(dhdp, &ifidx, p, reorder_info_buf,
		&reorder_info_len) != 0) {
		DHD_ERROR(("%s: rx protocol error\n", __FUNCTION__));
		PKTFREE(dhdp->osh, p, FALSE);
		dhdp->rx_errors++;
		return;
	}

	skb = PKTTONATIVE(dhdp->osh, p);

#if defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3) && defined(DHD_WMF) && \
	!defined(BCM_NO_WOFA)
	ret = dhd_wmf_fwder_sendup(wrapper, p);
	switch (ret) {
		case WMF_TAKEN:
			return;
		default:
			break;
	}
#endif /* BCM_ROUTER_DHD && BCM_GMAC3 && DHD_WMF && !BCM_NO_WOFA */

	skb->dev = dhd_idx2net((void *)dhdp, wmfh->bssidx);
	skb->protocol = eth_type_trans(skb, skb->dev);

	if (in_interrupt()) {
		netif_rx(skb);
	} else {
		if (dhd_is_rxthread_enabled(dhdp)) {
			if (!skbhead) {
				skbhead = skb;
			} else {
				PKTSETNEXT(dhdp->osh, skbprev, skb);
			}
			skbprev = skb;
		} else {

			/* If the receive is not processed inside an ISR,
			 * the softirqd must be woken explicitly to service
			 * the NET_RX_SOFTIRQ.	In 2.6 kernels, this is handled
			 * by netif_rx_ni(), but in earlier kernels, we need
			 * to do it manually.
			 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
			netif_rx_ni(skb);
#else
			ulong flags;
			netif_rx(skb);
			local_irq_save(flags);
			RAISE_RX_SOFTIRQ();
			local_irq_restore(flags);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0) */
		}
	}

	return;
}

/*
 * Description: This function is called to register hooks
 *		into wl for packet reception
 *
 * Input:       wrapper  - pointer to the bsscfg
 */
int32
dhd_wmf_hooks_register(void *wrapper)
{
	/*
	 * We dont need to do anything here. WMF enable status will be checked
	 * in the wl before handing off packets to WMF
	 */

	return BCME_OK;
}

/*
 * Description: This function is called to unregister hooks
 *		into wl for packet reception
 *
 * Input:       wrapper  - pointer to the bsscfg
 */
int32
dhd_wmf_hooks_unregister(void *wrapper)
{
	/*
	 * We dont need to do anything here. WMF enable status will be checked
	 * in the wl before handing off packets to WMF
	 */

	return BCME_OK;
}

int32
dhd_wmf_igs_broadcast(void *wrapper, uint8 *ip, uint32 length, uint32 mgrp_ip)
{
	dhd_wmf_wrapper_t *wmfh = (dhd_wmf_wrapper_t*)wrapper;
	dhd_pub_t *dhdp = wmfh->dhdp;
	uchar reorder_info_buf[WLHOST_REORDERDATA_TOTLEN];
	uint reorder_info_len;
	void *pkt;
	struct ether_header *eh;
	int ifidx = 0, ret = 0;

	/* Allocate the packet, copy the ip part */
	pkt = PKTGET(dhdp->osh, length + ETHER_HDR_LEN, TRUE);
	if (pkt == NULL) {
		DHD_ERROR(("%s: Out of memory allocating IGMP Query packet\n", __FUNCTION__));
		return BCME_TXFAIL;
	}
	/* Add the ethernet header */
	eh = (struct ether_header *)PKTDATA(dhdp->osh, pkt);
	eh->ether_type = hton16(ETHER_TYPE_IP);
	ETHER_FILL_MCAST_ADDR_FROM_IP(eh->ether_dhost, mgrp_ip);
	/* Add my own address as the source ether address */
	memcpy(eh->ether_shost, &dhdp->mac.octet, ETHER_ADDR_LEN);

	/* Copy the IP part */
	memcpy((uint8 *)eh + ETHER_HDR_LEN, ip, length);

	/* If the protocol uses a data header, check and remove it */
	if (dhd_prot_hdrpull(dhdp, &ifidx, pkt, reorder_info_buf,
		&reorder_info_len) != 0) {
		DHD_ERROR(("%s error\n", __FUNCTION__));
		PKTFREE(dhdp->osh, pkt, FALSE);
		dhdp->rx_errors++;
		return BCME_TXFAIL;
	}

	/* Send the frame */
	ret = dhd_sendpkt(dhdp, wmfh->bssidx, pkt);

	if (ret)
		DHD_ERROR(("%s: WMF: send failure\n", __FUNCTION__));

	return ret;
}

/*
 * Description: WMF Packet processing routine
 *
 * Input:   pub	- pointer to dhd_pub_t
 *	    p - pointer to the packet
 *	    sta	- sta specific info
 *	    ifidx - if index
 *	    frombss - packet from BSS or DS
 */
int
dhd_wmf_packets_handle(void *pub, void *p, void *sta_wmf, int ifidx, bool frombss)
{
	uint8 *iph;
	struct ether_header *eh;
	dhd_pub_t *dhdp = (dhd_pub_t*)pub;
	dhd_wmf_t *wmf = dhd_wmf_conf(dhdp, ifidx);
	dhd_wmf_instance_t *wmf_inst = wmf->wmf_instance;
	dhd_sta_t *sta = (dhd_sta_t*)sta_wmf;

	eh = (struct ether_header *)PKTDATA(dhdp->osh, p);
	iph = (uint8 *)eh + ETHER_HDR_LEN;

	/* Only IP packets are handled */
	if (ntoh16(eh->ether_type) != ETHER_TYPE_IP)
		return WMF_NOP;

	/* Change interface to primary interface of proxySTA */
	if (frombss && sta && sta->psta_prim && !dhd_get_wmf_psta_disable(dhdp, ifidx)) {
		sta = sta->psta_prim;
	}
	/* Hand it over to EMFC */
	return (emfc_input(wmf_inst->emfci, p, sta, iph, !frombss));
}
