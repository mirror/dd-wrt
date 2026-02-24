/*
 * Copyright 2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <linux/timer.h>
#include <linux/bitfield.h>

#include "morse.h"
#include "mac.h"
#include "mesh.h"
#include "vendor.h"
#include "debug.h"

/**
 * Periodic Time to trigger probe req in Mesh mode for discovery of new neighbor peers
 */
#define MESH_DISCOVERY_PROBE_PERIOD_S (60)
/**
 * Initial delay to trigger probe req in Mesh mode for discovery of new neighbor peers
 */
#define MESH_INITIAL_DISCOVERY_PROBE_DELAY_S (5)

/**
 * If the RSSI of new peer is better than existing weak(lowest RSSI) peer by RSSI margin
 * then mesh STA will kick out the peer with lowest RSSI.
 */
#define DEFAULT_MESH_RSSI_MARGIN	5

/** Duration in seconds, a kicked out peer is blacklisted */
#define DEFAULT_MESH_BLACKLIST_TIMEOUT 30

static void morse_schedule_mesh_probe_timer(struct morse_mesh *mesh, int delay)
{
	unsigned long timeout;

	timeout = jiffies + msecs_to_jiffies(delay * 1000);

	mod_timer(&mesh->mesh_probe_timer, timeout);
}

int morse_cmd_process_mbca_conf(struct morse_vif *mors_vif,
				struct morse_cmd_req_set_mcba_conf *mbca)
{
	struct morse_mesh *mesh;

	if (!mors_vif || !mors_vif->mesh)
		return -EFAULT;

	mesh = mors_vif->mesh;
	mesh->mbca.config = mbca->mbca_config;
	mesh->mbca.beacon_timing_report_interval = mbca->beacon_timing_report_interval;
	mesh->mbca.min_beacon_gap_ms = mbca->min_beacon_gap_ms;
	mesh->mbca.tbtt_adj_interval_ms = le16_to_cpu(mbca->tbtt_adj_interval_ms);
	mesh->mbca.mbss_start_scan_duration_ms = le16_to_cpu(mbca->mbss_start_scan_duration_ms);

	return 0;
}

int morse_cmd_process_dynamic_peering_conf(struct morse_vif *mors_vif,
					   struct morse_cmd_req_dynamic_peering_config *req)
{
	struct morse_mesh *mesh;
	struct morse *mors;

	if (!mors_vif || !mors_vif->mesh)
		return -EFAULT;

	mors = morse_vif_to_morse(mors_vif);

	mesh = mors_vif->mesh;
	mesh->dynamic_peering = req->enabled;
	mesh->rssi_margin = req->rssi_margin;
	mesh->blacklist_timeout = le32_to_cpu(req->blacklist_timeout);

	MORSE_MESH_INFO(mors, "dynamic_peering=%u, rssi_margin=%u, timeout=%u\n",
			mesh->dynamic_peering, mesh->rssi_margin, mesh->blacklist_timeout);

	return 0;
}

int morse_cmd_cfg_mesh_bss(struct morse_vif *mors_vif, bool stop_mesh)
{
	struct morse_mesh *mesh = mors_vif->mesh;
	struct morse *mors = morse_vif_to_morse(mors_vif);
	int ret;

	/* Reset MBCA configuration parameter if beaconing is not enabled */
	if (mesh->mesh_beaconless_mode)
		mesh->mbca.config = 0;

	ret = morse_cmd_cfg_mesh(mors, mors_vif, stop_mesh, !mesh->mesh_beaconless_mode);
	if (!ret)
		MORSE_MESH_INFO(mors, "%s: beaconless:%u stop:%u mbca.config:0x%02x\n",
				__func__, mesh->mesh_beaconless_mode, stop_mesh, mesh->mbca.config);

	mesh->mbca.beacon_count = 0;

	return ret;
}

/**
 * morse_find_mesh_config - Find the mesh config for the VIF stored in the list
 *
 * @mors: Global morse sruct
 * @addr: VIF address to find the config
 *
 * @return: Pointer to stored config, else NULL
 */
static struct morse_mesh_config_list *morse_find_mesh_config(struct morse *mors, u8 addr[ETH_ALEN])
{
	struct morse_mesh_config_list *config;

	lockdep_assert_held(&mors->mesh_config.lock);
	if (!addr)
		return NULL;

	list_for_each_entry(config, &mors->mesh_config.list, list) {
		if (memcmp(config->addr, addr, ETH_ALEN) == 0)
			return config;
	}
	return NULL;
}

int morse_restore_mesh_config(struct morse_vif *mors_vif)
{
	struct morse_mesh_config_list *config;
	struct morse_mesh *mesh;
	struct morse *mors;
	struct ieee80211_vif *vif;

	if (!mors_vif)
		return -EFAULT;

	vif = morse_vif_to_ieee80211_vif(mors_vif);
	mesh = mors_vif->mesh;
	mors = morse_vif_to_morse(mors_vif);

	spin_lock_bh(&mors->mesh_config.lock);
	config = morse_find_mesh_config(mors, vif->addr);
	if (!config) {
		spin_unlock_bh(&mors->mesh_config.lock);
		return -ENOENT;
	}

	memcpy(&mesh->mbca, &config->mbca, sizeof(mesh->mbca));
	spin_unlock_bh(&mors->mesh_config.lock);

	if (morse_cmd_set_mesh_config(mors_vif, NULL, config))
		return -EINVAL;

	return 0;
}

static void morse_store_mesh_config(struct ieee80211_vif *vif, struct morse_mesh *mesh)
{
	struct morse_mesh_config_list *config;

	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);
	struct morse *mors = morse_vif_to_morse(mors_vif);

	spin_lock_bh(&mors->mesh_config.lock);
	config = morse_find_mesh_config(mors, vif->addr);

	if (!config) {
		config = kzalloc(sizeof(*config), GFP_ATOMIC);
		if (!config)
			goto exit;
		list_add(&config->list, &mors->mesh_config.list);
	}

	memcpy(&config->mbca, &mesh->mbca, sizeof(mesh->mbca));
	memcpy(config->addr, vif->addr, ETH_ALEN);
	memcpy(config->mesh_conf.mesh_id, mesh->mesh_id, mesh->mesh_id_len);
	config->mesh_conf.mesh_id_len = mesh->mesh_id_len;
	config->mesh_conf.mesh_beaconless_mode = mesh->mesh_beaconless_mode;
	config->mesh_conf.max_plinks = mesh->max_plinks;
	config->dynamic_peering = mesh->dynamic_peering;
	config->rssi_margin = mesh->rssi_margin;
	config->blacklist_timeout = mesh->blacklist_timeout;

exit:
	spin_unlock_bh(&mors->mesh_config.lock);
}

int morse_cmd_set_mesh_config(struct morse_vif *mors_vif,
			      struct morse_cmd_req_set_mesh_config *mesh_req,
			      struct morse_mesh_config_list *stored_config)
{
	struct ieee80211_vif *vif;
	struct morse_mesh *mesh;
	struct morse_mesh_config *mesh_config;

	if (!mors_vif)
		return -EFAULT;

	vif = morse_vif_to_ieee80211_vif(mors_vif);
	mesh = mors_vif->mesh;

	if (!ieee80211_vif_is_mesh(vif) || mesh->is_mesh_active)
		return -ENOENT;

	if (stored_config) {
		mesh_config = &stored_config->mesh_conf;
		if (mesh_config->mesh_id_len  > IEEE80211_MAX_SSID_LEN)
			return -EINVAL;
		memcpy(mesh->mesh_id, mesh_config->mesh_id, mesh_config->mesh_id_len);
		mesh->mesh_id_len = mesh_config->mesh_id_len;
		mesh->mesh_beaconless_mode = mesh_config->mesh_beaconless_mode;
		mesh->max_plinks = mesh_config->max_plinks;
	} else {
		if (mesh_req->mesh_id_len  > IEEE80211_MAX_SSID_LEN)
			return -EINVAL;
		memcpy(mesh->mesh_id, mesh_req->mesh_id, mesh_req->mesh_id_len);
		mesh->mesh_id_len = mesh_req->mesh_id_len;
		mesh->mesh_beaconless_mode = mesh_req->mesh_beaconless_mode;
		mesh->max_plinks = mesh_req->max_plinks;
	}

	if (morse_cmd_cfg_mesh_bss(mors_vif, false))
		return -EPERM;

	if (mesh->mesh_beaconless_mode)
		morse_schedule_mesh_probe_timer(mesh, 0);
	mesh->is_mesh_active = true;

	if (!stored_config)
		morse_store_mesh_config(vif, mesh);

	return 0;
}

#if KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE
static void morse_mesh_probe_timer_cb(unsigned long addr)
#else
static void morse_mesh_probe_timer_cb(struct timer_list *t)
#endif
{
#if KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE
	struct morse_mesh *mesh = (struct morse_mesh *)addr;
#else
	struct morse_mesh *mesh = from_timer(mesh, t, mesh_probe_timer);
#endif
	struct morse_vif *mors_vif = mesh->mors_vif;
	struct ieee80211_vif *vif;
	u8 bcast_addr[ETH_ALEN];
	u32 next_probe_delay;

	if (!mors_vif) {
		pr_info("Mesh probe timer: ERROR! mors_vif NULL\n");
		MORSE_WARN_ON(FEATURE_ID_MESH, 1);
		return;
	}
	vif = morse_vif_to_ieee80211_vif(mors_vif);

	if (!ieee80211_vif_is_mesh(vif))
		return;

	eth_broadcast_addr(bcast_addr);
	morse_mac_tx_mesh_probe_req(mors_vif, bcast_addr);

	if (mors_vif->ap->num_stas)
		next_probe_delay = MESH_DISCOVERY_PROBE_PERIOD_S;
	else
		next_probe_delay = MESH_INITIAL_DISCOVERY_PROBE_DELAY_S;

	morse_schedule_mesh_probe_timer(mesh, next_probe_delay);
}

int morse_mac_tx_mesh_probe_req(struct morse_vif *mors_vif, const u8 *dest_addr)
{
	struct ieee80211_vif *vif;
	struct morse_mesh *mesh;
	struct morse *mors;
	struct sk_buff *skb = NULL;
	struct ieee80211_hdr *hdr;

	if (!mors_vif)
		return -EFAULT;

	vif = morse_vif_to_ieee80211_vif(mors_vif);
	mors = morse_vif_to_morse(mors_vif);
	mesh = mors_vif->mesh;

	if (!ieee80211_vif_is_mesh(vif) || !mesh || !mesh->mesh_id_len) {
		MORSE_MESH_ERR(mors, "Failed to send mesh probe req\n");
		return -ENOENT;
	}

	skb = ieee80211_probereq_get(mors->hw, vif->addr, mesh->mesh_id, mesh->mesh_id_len, 0);
	if (!skb) {
		MORSE_MESH_ERR(mors, "Failed to allocate mesh probe req\n");
		return -ENOMEM;
	}
	hdr = (struct ieee80211_hdr *)skb->data;
	memcpy(hdr->addr1, dest_addr, ETH_ALEN);

	if (morse_mac_tx_mgmt_frame(vif, skb)) {
		MORSE_MESH_ERR(mors, "Failed to send mesh probe req\n");
		dev_kfree_skb_any(skb);
		return -EPERM;
	}

	return 0;
}

int morse_mac_process_rx_mesh_probe_req(struct morse_vif *mors_vif,
					struct dot11ah_ies_mask *ies_mask,
					const struct ieee80211_rx_status *rx_status,
					const u8 *src_addr)
{
	struct ieee80211_vif *vif;
	struct ie_element *mesh_id_ie = &ies_mask->ies[WLAN_EID_MESH_ID];
	struct morse_mesh *mesh;
	struct morse *mors;
	struct ieee80211_sta *sta;

	if (!mors_vif)
		return -EFAULT;

	vif = morse_vif_to_ieee80211_vif(mors_vif);
	mors = morse_vif_to_morse(mors_vif);
	mesh = mors_vif->mesh;

	if (!ieee80211_vif_is_mesh(vif))
		return -ENOENT;

	rcu_read_lock();
	sta = ieee80211_find_sta_by_ifaddr(mors->hw, src_addr, vif->addr);
	if (sta) {
		rcu_read_unlock();
		return -EACCES;
	}
	rcu_read_unlock();

	if (morse_dot11ah_is_mesh_peer_known(src_addr))
		return 0;

	if (mesh_id_ie->len != 0 && (mesh_id_ie->len == mesh->mesh_id_len &&
				     !memcmp(mesh_id_ie->ptr, mesh->mesh_id, mesh->mesh_id_len))) {
		memcpy(&mesh->probe_rx_status, rx_status, sizeof(mesh->probe_rx_status));
	}

	return 0;
}

int morse_mac_add_meshid_ie(struct morse_vif *mors_vif, struct sk_buff *skb,
			    struct dot11ah_ies_mask *ies_mask)
{
	u8 mesh_id[IEEE80211_MAX_SSID_LEN];
	struct ie_element *ssid_ie = &ies_mask->ies[WLAN_EID_SSID];
	struct ie_element *mesh_id_ie = &ies_mask->ies[WLAN_EID_MESH_ID];
	struct morse_mesh *mesh;

	if (!mors_vif || !ies_mask)
		return -EFAULT;

	mesh = mors_vif->mesh;

	if (!mesh || !mesh->mesh_id_len)
		return -1;

	memcpy(mesh_id, mesh->mesh_id, mesh->mesh_id_len);

	mesh_id_ie->ptr = NULL;

	morse_dot11ah_insert_element(ies_mask, WLAN_EID_MESH_ID, (u8 *)mesh_id, mesh->mesh_id_len);

	ssid_ie->len = 0;

	return 0;
}

int morse_mac_process_mesh_tx_mgmt(struct morse_vif *mors_vif,
				   struct sk_buff *skb, struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_vif *vif;
	struct ieee80211_hdr *hdr;
	struct morse_mesh *mesh;
	struct morse *mors;
	struct sk_buff *skb_probe_resp;
	struct ieee80211_rx_status *rx_status;

	if (!mors_vif || !skb || !ies_mask)
		return -EFAULT;

	hdr = (struct ieee80211_hdr *)skb->data;
	vif = morse_vif_to_ieee80211_vif(mors_vif);
	mors = morse_vif_to_morse(mors_vif);
	mesh = mors_vif->mesh;

	if (ieee80211_is_probe_resp(hdr->frame_control)) {
		/* Probe resp processing */
		if (mesh->mesh_beaconless_mode) {
			struct ieee80211_mgmt *mgt_probe_resp;

			rx_status = &mesh->probe_rx_status;
			skb_probe_resp = skb_copy(skb, GFP_ATOMIC);

			if (!skb_probe_resp) {
				MORSE_MESH_ERR(mors, "%s: SKB for probe resp failed\n", __func__);
				return -ENOMEM;
			}
			mgt_probe_resp = (struct ieee80211_mgmt *)skb_probe_resp->data;
			memcpy(IEEE80211_SKB_RXCB(skb_probe_resp), rx_status, sizeof(*rx_status));
			memcpy(mgt_probe_resp->sa, mgt_probe_resp->da, ETH_ALEN);
			memcpy(mgt_probe_resp->bssid, mgt_probe_resp->da, ETH_ALEN);
			memcpy(mgt_probe_resp->da, vif->addr, ETH_ALEN);

			if (skb_probe_resp->len > 0 && rx_status->band == NL80211_BAND_5GHZ) {
				MORSE_MESH_DBG(mors, "%s: Indicating SKB for probe resp\n",
					       __func__);
				ieee80211_rx_irqsafe(mors->hw, skb_probe_resp);
			}
			/* Add this mesh peer into cssid list */
			morse_dot11ah_add_mesh_peer(ies_mask,
					le16_to_cpu(mgt_probe_resp->u.probe_resp.capab_info),
					hdr->addr1);

		} else if (mesh->mbca.config != 0) {
			u8 *ptr = ies_mask->ies[WLAN_EID_MESH_CONFIG].ptr;

			/* Enable MBCA Capability */
			if (ptr)
				morse_enable_mbca_capability(ptr);
		}
	} else if (ieee80211_is_probe_req(hdr->frame_control)) {
		/* Add Mesh ID to probe req in beaconless mode */
		if (mesh->mesh_beaconless_mode)
			morse_mac_add_meshid_ie(mors_vif, skb, ies_mask);
	}

	return 0;
}

struct lowest_peer_rssi_iter {
	const struct ieee80211_vif *on_vif;
	bool is_set;
	s16 rssi;
	u8 peer[ETH_ALEN];
};

static void peer_with_lowest_rssi(void *data, struct ieee80211_sta *sta)
{
	struct lowest_peer_rssi_iter *iter_data = data;
	struct morse_vif *mors_vif =
		ieee80211_vif_to_morse_vif((struct ieee80211_vif *)iter_data->on_vif);
	const struct morse *mors = morse_vif_to_morse(mors_vif);
	struct morse_sta *msta = (struct morse_sta *)sta->drv_priv;

	if (!msta) {
		MORSE_WARN_ON(FEATURE_ID_MESH, 1);
		return;
	}

	if (msta->vif != iter_data->on_vif)
		return;

	MORSE_MESH_DBG(mors, "msta %pM with rssi %d and peerings=%u\n",
		       msta->addr, msta->avg_rssi, msta->mesh_no_of_peerings);

	/* Ignore if number of peerings is 1 */
	if (msta->mesh_no_of_peerings == 1)
		return;

	if (!iter_data->is_set || iter_data->rssi > msta->avg_rssi) {
		iter_data->is_set = true;
		iter_data->rssi = msta->avg_rssi;
		memcpy(iter_data->peer, msta->addr, sizeof(iter_data->peer));
	}
}

/**
 * morse_mac_check_for_dynamic_peering() - Checks if a link can be established with the
 * new peer by kicking out one of existing peer (with low signal strength).
 *
 * @mors_vif: pointer to morse interface
 * @sa: address of the new peer
 * @rssi: RSSI of the new peer
 * @ies_mask: pointer to information elements.
 */
static void morse_mac_check_for_dynamic_peering(struct morse_vif *mors_vif, u8 *sa,
						s16 rssi, struct dot11ah_ies_mask *ies_mask)
{
	struct morse_mesh *mesh = mors_vif->mesh;
	struct morse *mors = morse_vif_to_morse(mors_vif);
	struct ie_element *mesh_id_ie = &ies_mask->ies[WLAN_EID_MESH_ID];
	struct ie_element *mesh_conf_ie = &ies_mask->ies[WLAN_EID_MESH_CONFIG];
	struct ieee80211_vif *vif = morse_vif_to_ieee80211_vif(mors_vif);
	struct lowest_peer_rssi_iter data = {
		.is_set = false,
		.on_vif = vif
	};
	bool accept_additional_peer;

	/* Check if number of peers reached the limit */
	if (mors_vif->ap->num_stas < mesh->max_plinks)
		return;

	/* process frames from same mesh BSS */
	if (!mesh_id_ie || mesh_id_ie->len != mesh->mesh_id_len ||
	    memcmp(mesh_id_ie->ptr, mesh->mesh_id, mesh_id_ie->len)) {
		return;
	}

	/* Ignore the frame if mesh config element is not present */
	if (!mesh_conf_ie)
		return;

	accept_additional_peer =
	    !!(mesh_conf_ie->ptr[MESH_CONF_IE_CAPABILITY_FLAG_BYTE_OFFSET] &
	       MESH_CAP_ACCEPT_ADDITIONAL_PEER);

	/* Consider new peer only if it accepts additional peering */
	if (!accept_additional_peer)
		return;

	ieee80211_iterate_stations_atomic(mors->hw, peer_with_lowest_rssi, &data);

	/* Check if the new peer has better signal than existing peer */
	if (data.is_set && (data.rssi + mesh->rssi_margin) < rssi) {
		struct morse_mesh_peer_addr_vendor_evt event;
		int ret;

		memcpy(event.addr, data.peer, ETH_ALEN);

		/* New peer has better rssi - indicate peer to supplicant to kick out */
		ret = morse_vendor_send_peer_addr_event(vif, &event);
		if (!ret) {
			memcpy(mesh->kickout_peer_addr, data.peer, ETH_ALEN);
			mesh->kickout_ts = jiffies;
		}
		MORSE_MESH_INFO(mors, "Kickout Peer %pM rssi %d, new peer %pM rssi %d, ret=%d\n",
				mesh->kickout_peer_addr, data.rssi, sa, rssi, ret);
	}
}

/**
 * morse_dynamic_peering_is_frame_allowed() - Checks if a given frame is allowed to
 * indicate to upper stack or not.
 *
 * @mesh: pointer to mesh interface
 * @mgmt: pointer to management frame
 *
 * Returns: True if the packet can be indicated to upper stack.
 */
static bool morse_dynamic_peering_is_frame_allowed(struct morse_mesh *mesh,
						   struct ieee80211_mgmt *mgmt)
{
	u16 stype = le16_to_cpu(mgmt->frame_control) & IEEE80211_FCTL_STYPE;
	bool allowed = true;

	switch (stype) {
	case IEEE80211_STYPE_S1G_BEACON:
	case IEEE80211_STYPE_PROBE_RESP:
		/* Drop beacon and probe response frames as it will trigger a new peering */
		allowed = false;
		break;
	case IEEE80211_STYPE_ACTION:
		if (mgmt->u.action.category == WLAN_ACTION_SELF_PROTECTED &&
		    (mgmt->u.action.u.self_prot.action_code == PLINK_OPEN ||
		     mgmt->u.action.u.self_prot.action_code == PLINK_CONFIRM ||
		     mgmt->u.action.u.self_prot.action_code == PLINK_CLOSE)) {
			/* Drop peering open, confirm and close frames */
			allowed = false;
		}
		break;
	default:
		break;
	}
	return allowed;
}

int morse_mac_process_mesh_rx_mgmt(struct morse_vif *mors_vif, struct sk_buff *skb,
				   struct dot11ah_ies_mask *ies_mask,
				   const struct ieee80211_rx_status *rx_status)
{
	struct ieee80211_vif *vif = morse_vif_to_ieee80211_vif(mors_vif);
	struct morse_mesh *mesh = mors_vif->mesh;
	struct morse *mors = morse_vif_to_morse(mors_vif);
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)skb->data;
	struct ieee80211_sta *sta;
	u8 *src_addr;
	u16 stype;

	if (!mesh->is_mesh_active)
		return -ENOENT;

	if (ieee80211_is_s1g_beacon(mgmt->frame_control))
		src_addr = mgmt->da;
	else
		src_addr = mgmt->sa;
	stype = le16_to_cpu(mgmt->frame_control) & IEEE80211_FCTL_STYPE;

	if (mesh->mesh_beaconless_mode && stype == IEEE80211_STYPE_PROBE_REQ)
		return morse_mac_process_rx_mesh_probe_req(mors_vif, ies_mask, rx_status, mgmt->sa);

	if (mesh->dynamic_peering && (ieee80211_is_s1g_beacon(mgmt->frame_control) ||
				      stype == IEEE80211_STYPE_ACTION ||
				      stype == IEEE80211_STYPE_PROBE_RESP)) {
		struct ie_element *mesh_conf_ie = &ies_mask->ies[WLAN_EID_MESH_CONFIG];
		u8 no_of_peerings = 0;

		/* check if timeout has expired */
		if (mesh->kickout_ts && (jiffies_to_msecs(jiffies - mesh->kickout_ts) >=
					 MORSE_SECS_TO_MSECS(mesh->blacklist_timeout))) {
			MORSE_MESH_DBG(mors, "Reset blacklisted peer=%pM\n",
				       mesh->kickout_peer_addr);
			memset(mesh->kickout_peer_addr, 0, ETH_ALEN);
			mesh->kickout_ts = 0;
		}

		/* check if the frame is from kicked out peer */
		if (!memcmp(mesh->kickout_peer_addr, src_addr, ETH_ALEN) &&
		    !morse_dynamic_peering_is_frame_allowed(mesh, mgmt)) {
			return -EACCES;
		}

		if (mesh_conf_ie->ptr)
			no_of_peerings =
			    MESH_PARSE_NO_OF_PEERINGS(mesh_conf_ie->ptr
						      [MESH_CONF_IE_FORMATION_INFO_BYTE_OFFSET]);

		rcu_read_lock();
		sta = ieee80211_find_sta_by_ifaddr(mors->hw, src_addr, vif->addr);
		if (sta && mesh_conf_ie->ptr) {
			struct morse_sta *msta = (struct morse_sta *)sta->drv_priv;

			msta->mesh_no_of_peerings = no_of_peerings;
		}
		rcu_read_unlock();

		/* check for dynamic peering if the frame is from a new peer and
		 * not part of the network already. Also check if kick out period timed out.
		 */
		if (!sta && !mesh->kickout_ts && !no_of_peerings)
			morse_mac_check_for_dynamic_peering(mors_vif, src_addr, rx_status->signal,
							    ies_mask);
	}
	return 0;
}

void morse_mac_clear_mesh_list(struct morse *mors)
{
	struct morse_mesh_config_list *config, *tmp;

	spin_lock_bh(&mors->mesh_config.lock);
	/* Free allocated list */
	list_for_each_entry_safe(config, tmp, &mors->mesh_config.list, list) {
		list_del(&config->list);
		kfree(config);
	}
	spin_unlock_bh(&mors->mesh_config.lock);
}

int morse_mesh_deinit(struct morse_vif *mors_vif)
{
	struct morse_mesh *mesh = mors_vif->mesh;

	del_timer_sync(&mesh->mesh_probe_timer);
	kfree(mors_vif->mesh);

	return 0;
}

void morse_mesh_config_list_init(struct morse *mors)
{
	struct morse_mesh_config_list *mesh_config = &mors->mesh_config;

	INIT_LIST_HEAD(&mesh_config->list);
	spin_lock_init(&mesh_config->lock);
}

int morse_mesh_init(struct morse_vif *mors_vif)
{
	struct morse_mesh *mesh;

	mors_vif->mesh = kzalloc(sizeof(*mors_vif->mesh), GFP_KERNEL);
	if (!mors_vif->mesh)
		return -ENOMEM;

	mesh = mors_vif->mesh;
	mesh->mors_vif = mors_vif;
	mesh->mesh_id_len = 0;
#if KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE
	init_timer(&mesh->mesh_probe_timer);
	mesh->mesh_probe_timer.data = (unsigned long)mesh;
	mesh->mesh_probe_timer.function = morse_mesh_probe_timer_cb;
	add_timer(&mesh->mesh_probe_timer);
#else
	timer_setup(&mesh->mesh_probe_timer, morse_mesh_probe_timer_cb, 0);
#endif

	/* Assign default values for Mesh Beacon Collision Avoidance configuration
	 * parameters. It will be overwritten by supplicant configuration before interface
	 * start.
	 */
	mesh->mbca.config = MESH_MBCA_CFG_TBTT_SEL_ENABLE;
	mesh->mbca.beacon_timing_report_interval = DEFAULT_MESH_BCN_TIMING_REPORT_INT;
	mesh->mbca.mbss_start_scan_duration_ms = DEFAULT_MBSS_START_SCAN_DURATION_MS;
	mesh->mbca.min_beacon_gap_ms = DEFAULT_MBCA_MIN_BEACON_GAP_MS;
	mesh->mbca.tbtt_adj_interval_ms = DEFAULT_TBTT_ADJ_INTERVAL_MSEC;
	mesh->dynamic_peering = DEFAULT_DYNAMIC_MESH_PEERING;
	mesh->rssi_margin = DEFAULT_MESH_RSSI_MARGIN;
	mesh->blacklist_timeout = DEFAULT_MESH_BLACKLIST_TIMEOUT;

	morse_set_max_skb_txq_len(MORSE_MESH_MAX_TXQ_LENGTH);

	return 0;
}
