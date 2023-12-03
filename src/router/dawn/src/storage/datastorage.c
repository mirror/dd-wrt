#include <stdbool.h>
#include <stdio.h>

#include "memory_utils.h"
#include "dawn_iwinfo.h"
#include "dawn_uci.h"
#include "mac_utils.h"

#include "datastorage.h"
#include "test_storage.h"
#include "msghandler.h"
#include "ubus.h"

struct probe_metric_s dawn_metric;
struct network_config_s network_config;
struct time_config_s timeout_config;
struct local_config_s local_config;

static client* client_array_get_client_for_bssid(struct dawn_mac bssid_mac, struct dawn_mac client_mac);

static int compare_station_count(ap* ap_entry_own, ap* ap_entry_to_compare, struct dawn_mac client_addr);


// ---------------- Global variables ----------------
// config section name
const char *band_config_name[__DAWN_BAND_MAX] = {
    "802_11g",
    "802_11a"
};

// starting frequency
// TODO: make this configurable
const int max_band_freq[__DAWN_BAND_MAX] = {
    2500,
    5925 // This may cause trouble because there's overlap between bands in different countries
};

// FIXME: Using ratio of 4 to exercise skip handling in small network.  Set to 32, 64 or 128 for real use on larger network.
struct probe_head_s probe_set = { 0, 0, 4, NULL, NULL };
pthread_mutex_t probe_array_mutex;

struct ap_s *ap_set = NULL;
int ap_entry_last = 0;
pthread_mutex_t ap_array_mutex;

struct client_s* client_set_bc = NULL; // Ordered by BSSID + client MAC
pthread_mutex_t client_array_mutex;

// TODO: How big does this get?
// TODO: No mutex on this - is it required?
struct mac_entry_s* mac_set = NULL;
int mac_set_last = 0;

/*
** The ..._find_first() functions perform an efficient search of the core storage linked lists.
** "Skipping" linear searches and binary searches are used depending on anticipated array size.
** TODO:  It may be more efficient to use skipping lists for all?  Telemetry required.
** The return is a pointer to the linked list field that references the element indicated by the
** target parameters. In this context "indicated by" means the first element in the list that matches
** the search parameters, or if the element is not in the list the position where it would be inserted.
** In other words, if A precedes B and B is sought then a pointer to the field in A that references B
** is returned.  If A links to C and B would be positioned between then the same pointer is returned.
** Hence the return should be checked to see if the element it references is the target or not.  If not
** then the target element does not exist, but can be inserted by using the returned reference.
*/

// Probe entries are ordered by client MAC and BSSID so that all APs for a client appear in a block
probe_entry* probe_array_find_first_entry(struct dawn_mac client_mac, struct dawn_mac bssid_mac, int do_bssid)
{
    dawnlog_debug_func("Entering...");

    dawn_mutex_require(&probe_array_mutex);
    struct probe_entry_s* curr_node = probe_set.first_probe;
    struct probe_entry_s* curr_skip = probe_set.first_probe_skip;

    int this_cmp = 0;
    while (curr_skip != NULL) {
        this_cmp = mac_compare_bb(curr_skip->client_addr, client_mac);

        if (this_cmp == 0 && do_bssid)
            this_cmp = mac_compare_bb(curr_skip->bssid_addr, bssid_mac);

        if (this_cmp >= 0) {
            break;
        }
        else {
            curr_node = curr_skip;
            curr_skip = curr_skip->next_probe_skip;
        }
    }

    while (curr_node != NULL) {
        this_cmp = mac_compare_bb(curr_node->client_addr, client_mac);

        if (this_cmp == 0 && do_bssid)
            this_cmp = mac_compare_bb(curr_node->bssid_addr, bssid_mac);

        if (this_cmp >= 0) {
            break;
        }
        else {
            curr_node = curr_node->next_probe;
        }
    }

    // Search including BSSID means that an exact match is required
    if (do_bssid && curr_node && this_cmp != 0)
        curr_node = NULL;

    return curr_node;
}

// Manage a list of client entries sorted by BSSID and client MAC
client** client_find_first_bc_entry(struct dawn_mac bssid_mac, struct dawn_mac client_mac, int do_client)
{
    dawnlog_debug_func("Entering...");

    dawn_mutex_require(&client_array_mutex);
    client** ret = &client_set_bc;

    while ((*ret != NULL))
    {
        int this_cmp = mac_compare_bb((*ret)->bssid_addr, bssid_mac);

        if (this_cmp == 0 && do_client)
            this_cmp = mac_compare_bb((*ret)->client_addr, client_mac);

        if (this_cmp >= 0)
            break;

        ret = &((*ret)->next_entry_bc);
    }

    return ret;
}

static client* client_array_get_client_for_bssid(struct dawn_mac bssid_mac, struct dawn_mac client_mac) {
    dawnlog_debug_func("Entering...");

    client* i = *client_find_first_bc_entry(bssid_mac, client_mac, true);

    if (i != NULL)
        if (!mac_is_equal_bb((i)->bssid_addr, bssid_mac) || !mac_is_equal_bb((i)->client_addr, client_mac))
            i = NULL;

    return i;
}

struct mac_entry_s* mac_find_entry(struct dawn_mac mac)
{
    dawnlog_debug_func("Entering...");

    struct mac_entry_s* ret = mac_set;

    while (ret && mac_compare_bb(ret->mac, mac) != 0)
    {
        ret = ret->next_mac;
    }

    return ret;
}

void send_beacon_requests(ap* target_ap, ap* host_ap, int sub_id) {
    dawnlog_debug_func("Entering...");

    dawn_mutex_require(&client_array_mutex);
    // Seach for BSSID
    client* i = *client_find_first_bc_entry(host_ap->bssid_addr, dawn_mac_null, false);

    // Go through clients to request BEACON
    while (i != NULL && mac_is_equal_bb(i->bssid_addr, host_ap->bssid_addr)) {
        if (dawnlog_showing(DAWNLOG_DEBUG))
            dawnlog_debug("Client " MACSTR ": rrm_enabled_capa=%02x: PASSIVE=%d, ACTIVE=%d, TABLE=%d\n",
                MAC2STR(i->client_addr.u8), i->rrm_enabled_capa,
                !!(i->rrm_enabled_capa & WLAN_RRM_CAPS_BEACON_REPORT_PASSIVE),
                !!(i->rrm_enabled_capa & WLAN_RRM_CAPS_BEACON_REPORT_ACTIVE),
                !!(i->rrm_enabled_capa & WLAN_RRM_CAPS_BEACON_REPORT_TABLE));

        // Bitwise AND to check for overlap in RRM capabilities
        if (i->rrm_enabled_capa & dawn_metric.rrm_mode_mask)
        {
            int ubus_status = ubus_send_beacon_request(i, target_ap, dawn_metric.duration, sub_id);
            if (ubus_status)
                dawnlog_warning("Client / BSSID = " MACSTR " / " MACSTR ": BEACON REQUEST failed",
                    MAC2STR(i->client_addr.u8), MAC2STR(target_ap->bssid_addr.u8));
        }

        i = i->next_entry_bc;
    }
}

int get_band(int freq) {
    int band;

    dawnlog_debug_func("Entering...");

    for (band=0; band < __DAWN_BAND_MAX; band++)
        if (freq <= max_band_freq[band])
            return band;
    band--;
    dawnlog_warning("frequency %d is beyond the last known band. "
                    "Using '%s' band parameters.\n", freq, band_config_name[band]);
    return band;
}

// TODO: Can metric be cached once calculated? Add score_fresh indicator and reset when signal changes
// TODO: as rest of values look to be static fr any given entry.
int eval_probe_metric(struct probe_entry_s* probe_entry, ap* ap_entry) {
    dawnlog_debug_func("Entering...");
    int score = 0;

    if (probe_entry->signal != 0)
    {
        dawn_mutex_require(&ap_array_mutex);
        dawn_mutex_require(&probe_array_mutex);

        // TODO: Should RCPI be used here as well?
        int band = get_band(probe_entry->freq);
        score = dawn_metric.initial_score[band];

        score += probe_entry->signal >= dawn_metric.rssi_val[band] ? dawn_metric.rssi[band] : 0;
        score += probe_entry->signal <= dawn_metric.low_rssi_val[band] ? dawn_metric.low_rssi[band] : 0;
        score += (probe_entry->signal - dawn_metric.rssi_center[band]) * dawn_metric.rssi_weight[band];

        // check if ap entry is available
        if (ap_entry != NULL) {
            score += probe_entry->ht_capabilities && ap_entry->ht_support ? dawn_metric.ht_support[band] : 0;
            score += !probe_entry->ht_capabilities && !ap_entry->ht_support ? dawn_metric.no_ht_support[band] : 0;  // TODO: Is both devices not having a capability worthy of scoring?

            // performance anomaly?
            if (network_config.bandwidth >= 1000 || network_config.bandwidth == -1) {
                score += probe_entry->vht_capabilities && ap_entry->vht_support ? dawn_metric.vht_support[band] : 0;
            }

            score += !probe_entry->vht_capabilities && !ap_entry->vht_support ? dawn_metric.no_vht_support[band] : 0;  // TODO: Is both devices not having a capability worthy of scoring?
            score += ap_entry->channel_utilization <= dawn_metric.chan_util_val[band] ? dawn_metric.chan_util[band] : 0;
            score += ap_entry->channel_utilization > dawn_metric.max_chan_util_val[band] ? dawn_metric.max_chan_util[band] : 0;

            score += ap_entry->ap_weight;
        }
    }

    // TODO: This magic value never checked by caller.  What does it achieve?
    if (score < 0)
        score = -2; // -1 already used...

    return score;
}


static int compare_station_count(ap* ap_entry_own, ap* ap_entry_to_compare, struct dawn_mac client_addr) {

    dawnlog_debug_func("Entering...");

    dawnlog_info("Comparing own %d to %d\n", ap_entry_own->station_count, ap_entry_to_compare->station_count);

    dawn_mutex_require(&ap_array_mutex);
    int sta_count = ap_entry_own->station_count;
    int sta_count_to_compare = ap_entry_to_compare->station_count;

    dawn_mutex_require(&client_array_mutex);
    if (client_array_get_client_for_bssid(ap_entry_own->bssid_addr, client_addr)) {
        dawnlog_debug("Own is already connected! Decrease counter!\n");
        sta_count--;
    }

    dawn_mutex_require(&client_array_mutex);
    if (client_array_get_client_for_bssid(ap_entry_to_compare->bssid_addr, client_addr)) {
        dawnlog_debug("Comparing station is already connected! Decrease counter!\n");
        sta_count_to_compare--;
    }

    dawnlog_info("Comparing own station count %d to %d\n", sta_count, sta_count_to_compare);

    return sta_count - sta_count_to_compare;
}

static void remove_kicking_nr_list(struct kicking_nr *nr_list) {
    dawnlog_debug_func("Entering...");

    while(nr_list) {
        struct kicking_nr* next = nr_list->next;
        dawn_free(nr_list);
        nr_list = next;
    }
}

#if 0
static void prune_kicking_nr_list(struct kicking_nr **nr_list, int min_score) {
    dawnlog_debug_func("Entering...");

    while (*nr_list) {
        if ((*nr_list)->score <= min_score)
        {
            struct kicking_nr* next = (*nr_list)->next;
            dawn_free(*nr_list);
            *nr_list = next;
        }
        else
        {
            nr_list = &((*nr_list)->next);
        }
    }

    return;
}
#endif

static  void insert_kicking_nr_by_bssid(struct kicking_nr** nrlist, ap* nr_ap) {
    dawnlog_debug_func("Entering...");
    dawn_mutex_require(&ap_array_mutex);

    //  Look for the proposed BSSID in the current list
    while (*nrlist && mac_compare_bb((*nrlist)->nr_ap->bssid_addr, nr_ap->bssid_addr) < 0) {
        nrlist = &((*nrlist)->next);
    }

    // If it isn't there then add it
    if (!*nrlist || mac_compare_bb((*nrlist)->nr_ap->bssid_addr, nr_ap->bssid_addr) != 0)
    {
        // we are giving no error information here (not really critical)
        struct kicking_nr* new_entry = dawn_malloc(sizeof(struct kicking_nr));

        if (new_entry)
        {
            new_entry->nr_ap = nr_ap;
            new_entry->score = 0;

            new_entry->next = *nrlist;
            *nrlist = new_entry;
        }
    }

    return;
}

static  void insert_kicking_nr_by_score(struct kicking_nr** nrlist, ap* nr_ap, int score) {
    dawnlog_debug_func("Entering...");
    dawn_mutex_require(&ap_array_mutex);

    // we are giving no error information here (not really critical)
    struct kicking_nr* new_entry = dawn_malloc(sizeof(struct kicking_nr));

    if (!new_entry)
        return;

    new_entry->nr_ap = nr_ap;
    new_entry->score = score;

    // Order entries high-low score so we can read first N highest values
    while (*nrlist && (*nrlist)->score > score) {
        nrlist = &((*nrlist)->next);
    }

    new_entry->next = *nrlist;
    *nrlist = new_entry;

    return;
}

int better_ap_available(ap *kicking_ap, probe_entry *own_probe, int own_score, struct kicking_nr **neighbor_report) {

    dawnlog_debug_func("Entering...");
    dawn_mutex_require(&ap_array_mutex);
    dawn_mutex_require(&probe_array_mutex);

    int tgt_score = own_score + dawn_metric.kicking_threshold;
    int better_ap_found = 0;
    int ap_count = 0;
    // Now go through all probe entries for this client looking for better score
    dawn_mutex_require(&probe_array_mutex);
    probe_entry* i = probe_array_find_first_entry(own_probe->client_addr, dawn_mac_null, false);

    while (i != NULL && mac_is_equal_bb(i->client_addr, own_probe->client_addr)) {
        if (i == own_probe) {
            dawnlog_trace("Own Score! Skipping!\n");
            i = i->next_probe;
            continue;
        }

        dawn_mutex_require(&ap_array_mutex);
        ap* candidate_ap = ap_array_get_ap(i->bssid_addr);

        if (candidate_ap == NULL) {
            dawnlog_trace("Candidate AP not in array\n");
            i = i->next_probe;
            continue;
        }

        // check if same ssid!
        if (strcmp((char*)kicking_ap->ssid, (char*)candidate_ap->ssid) != 0) {
            dawnlog_trace("Candidate AP has different SSID\n");
            i = i->next_probe;
            continue;
        }

        ap_count++;

        int score_to_compare = eval_probe_metric(i, candidate_ap);
        dawnlog_trace("Candidate score = %d from:\n", score_to_compare);
        print_probe_entry(DAWNLOG_TRACE, i);

        int ap_outcome = 0; // No kicking

        // Find better score...
        if (score_to_compare > tgt_score) {
            ap_outcome = 2; // Add and prune
            // TODO: Should we adjust this, or just stick with original "better than current AP" target?
            tgt_score = score_to_compare;
        }
        // Give a few marks for candidate AP having fewer clients than current
        // TODO: Is absolute number meaningful when AP have diffeent capacity?
        // TODO: This test doesn't really make sense if we have adjusted target score to best found so far
        else if (score_to_compare == tgt_score && dawn_metric.use_station_count > 0 ) {
            int compare = compare_station_count(kicking_ap, candidate_ap, own_probe->client_addr);
            if (compare > dawn_metric.max_station_diff) {
                ap_outcome = 2; // Add and prune
            }
            else if (compare == 0) {
                ap_outcome = 1; // Add but no prune
            }
        }
        else if (score_to_compare == tgt_score) {
            ap_outcome = 1; // Add but no prune
        }

        if (ap_outcome == 0)
        {
            dawnlog_trace("Not a better AP after full evaluation\n");
        }
        else
        {
            dawnlog_trace("Better AP after full evaluation\n");
            better_ap_found = 1;
        }

        // NR is NULL if we're only testing for a better AP without actually using it
        if (better_ap_found && neighbor_report == NULL)
        {
            // Short circuit loop if we're only finding a better AP without actually using it
            i = NULL;
        }
        else
        {
            // Always add the NR as we use it to adjust the "own AP" semi-static NR as well
            if (neighbor_report != NULL)
            {
                dawnlog_trace("Add to NR (%s pruning)\n", ap_outcome == 2 ? "with" : "without");
                // FIXME: Do we need to prune this list as we build it?  trying new approach to send N best entries to hostapd
                // if (ap_outcome == 2)
                //     prune_kicking_nr_list(neighbor_report, score_to_compare - dawn_metric.kicking_threshold);

                insert_kicking_nr_by_score(neighbor_report, candidate_ap, score_to_compare);
            }

            i = i->next_probe;
        }
    }

    if (neighbor_report != NULL)
        dawnlog_info("Client " MACSTR ": Compared %d alternate AP candidates\n", MAC2STR(own_probe->client_addr.u8), ap_count);

    return better_ap_found;
}

int kick_clients(struct dawn_mac bssid_mac, uint32_t id) {
    dawnlog_debug_func("Entering...");

    dawn_mutex_lock(&client_array_mutex);
    dawn_mutex_lock(&probe_array_mutex);
    dawn_mutex_lock(&ap_array_mutex);

    dawn_mutex_require(&ap_array_mutex);
    ap* kicking_ap = ap_array_get_ap(bssid_mac);

    int kicked_clients = 0;

    // Keep a list of nearby APs to update local AP Neighbor Report from
    struct kicking_nr* ap_nr_list = NULL;

    dawnlog_info("AP BSSID " MACSTR ": Looking for candidates to kick\n", MAC2STR(kicking_ap->bssid_addr.u8));

    // Seach for BSSID
    client *j = *client_find_first_bc_entry(kicking_ap->bssid_addr, dawn_mac_null, false);

    // Go through clients: only kick one each time (not sure why, was in original algorithm...)
    while (kicked_clients == 0 && j  != NULL && mac_is_equal_bb(j->bssid_addr, kicking_ap->bssid_addr)) {
        struct kicking_nr *kick_nr_list = NULL;

        int kick_type = 0;
        int own_score = 0;

        if (mac_find_entry(j->client_addr)) {
            dawnlog_info("Client " MACSTR ": Suppressing check due to MAC list entry\n", MAC2STR(j->client_addr.u8));
        }
        else {
            dawn_mutex_require(&probe_array_mutex);
            probe_entry* own_probe = probe_array_get_entry(j->client_addr, kicking_ap->bssid_addr);

            if (own_probe == NULL) {
                // no entry for own ap - may happen if DAWN is started after client has connected, and then "sleeps" so sends no BEACON / PROBE
                dawnlog_info("Current AP " MACSTR " for client " MACSTR " not found in probe array!\n", MAC2STR(kicking_ap->bssid_addr.u8), MAC2STR(j->client_addr.u8));
                print_probe_array();

                // TODO: Work out a way to handle clients that are reluctant to share a probe / beacon that doesn't become DoS for them
                // do_kick = -1;
            }
           
            if ((kick_type == 0) && own_probe && (dawn_metric.kicking & 1) == 1) {
                own_score = eval_probe_metric(own_probe, kicking_ap);
                dawnlog_trace("Current AP score = %d for:\n", own_score);
                print_probe_entry(DAWNLOG_TRACE, own_probe);

                kick_type = better_ap_available(kicking_ap, own_probe, own_score, &kick_nr_list);

                // If we found any candidates by evaluating PROBEs (even if too low to kick to) add the highest scoring one to local AP NR set
                if (kick_type != 0 && dawn_metric.set_hostapd_nr == 2 && kick_nr_list)
                {
                    dawnlog_trace("Adding " MACSTR "as local NR entry candidate\n", MAC2STR(kick_nr_list->nr_ap->bssid_addr.u8));
                    insert_kicking_nr_by_bssid(&ap_nr_list, kick_nr_list->nr_ap);
                }
            }
           
            if ((kick_type == 0) && own_probe && (dawn_metric.kicking & 2) == 2) {
                int band = get_band(own_probe->freq);

                if (own_probe->signal < dawn_metric.rssi_center[band])
                {
                    dawnlog_info("Client " MACSTR ": Low asolute RSSI - proposing other APs\n", MAC2STR(j->client_addr.u8));
                    dawn_mutex_require(&ap_array_mutex);

                    // FIXME: Using all AP is OK for a small network, but should ideally use the local-to-current-AP set
                    for (ap* candidate_ap = ap_set; candidate_ap != NULL; candidate_ap = candidate_ap->next_ap) {
                        // Check is same SSID< but is not the current AP
                        if (candidate_ap != kicking_ap && strcmp((char*)kicking_ap->ssid, (char*)candidate_ap->ssid) == 0) {
                            insert_kicking_nr_by_bssid(&kick_nr_list, candidate_ap);
                        }
                    }

                    kick_type = 2;
                }
            }
        }

        // better ap available
        if (kick_type > 0) {

            // kick after algorithm decided to kick several times
            // + rssi is changing a lot
            // + chan util is changing a lot
            // + ping pong behavior of clients will be reduced
            j->kick_count++;
            if (j->kick_count < dawn_metric.min_number_to_kick) {
                dawnlog_info("Client " MACSTR ": kickcount %d below threshold of %d!\n", MAC2STR(j->client_addr.u8), j->kick_count,
                    dawn_metric.min_number_to_kick);
            }
            else {
                float rx_rate, tx_rate;
                bool have_bandwidth_iwinfo = get_bandwidth_iwinfo(j->client_addr, &rx_rate, &tx_rate);
                if (!have_bandwidth_iwinfo && dawn_metric.bandwidth_threshold > 0) {
                    dawnlog_info("Client " MACSTR ": No active transmission data for client. Don't kick!\n", MAC2STR(j->client_addr.u8));
                }
                else
                {
                    // only use rx_rate for indicating if transmission is going on
                    // <= 6MBits <- probably no transmission
                    // tx_rate has always some weird value so don't use ist
                    if (have_bandwidth_iwinfo && dawn_metric.bandwidth_threshold != 0 && rx_rate > dawn_metric.bandwidth_threshold) {
                        dawnlog_info("Client " MACSTR ": Don't kick due to active data transfer: RX rate %f exceeds %d limit\n", MAC2STR(j->client_addr.u8), rx_rate, dawn_metric.bandwidth_threshold);
                    }
                    else
                    {
                        if (have_bandwidth_iwinfo && dawn_metric.bandwidth_threshold != 0)
                            dawnlog_always("Client " MACSTR ": Kicking due to low active data transfer: RX rate %f below %d limit\n", MAC2STR(j->client_addr.u8), rx_rate, dawn_metric.bandwidth_threshold);
                        else
                            dawnlog_always("Client " MACSTR ": Kicking as no active transmission data for client, and / or limit of %d is OK.\n",
                                MAC2STR(j->client_addr.u8), dawn_metric.bandwidth_threshold);

                        print_client_entry(DAWNLOG_TRACE, j);

                        if (dawnlog_showing(DAWNLOG_INFO))
                        {
                            for (struct kicking_nr* n = kick_nr_list; n; n = n->next)
                                dawnlog_info("Kicking NR entry: " NR_MACSTR ", score=%d\n", NR_MAC2STR(n->nr_ap->neighbor_report), n->score);
                        }

                        // here we should send a messsage to set the probe.count for all aps to the min that there is no delay between switching
                        // the hearing map is full...
                        send_set_probe(j->client_addr);

                        // don't deauth station? <- deauth is better!
                        // maybe we can use handovers...
                        //del_client_interface(id, client_array[j].client_addr, NO_MORE_STAS, 1, 1000);
                        if (kick_type == 1)
                        {
                            int sync_kick = wnm_disassoc_imminent(id, j->client_addr, kick_nr_list, own_score + dawn_metric.kicking_threshold, dawn_metric.duration);

                            if (!sync_kick)
                            {
                                client_array_delete(j, false);

                                // don't delete clients in a row. use update function again...
                                // -> chan_util update, ...
                                //FIXME: Why / 4?
                                add_client_update_timer(timeout_config.update_client * 1000 / 4);
                            }
                        }
                        else if (kick_type == 2)
                        {
                            bss_transition_request(id, j->client_addr, kick_nr_list, dawn_metric.duration);
                        }

                        kicked_clients++;
                    }
                }
            }
        }
        else if (kick_type == -1) {
            // FIXME: Causes clients to be kicked until first probe is received, which is a bit brutal for pre-802.11k clients.
            dawnlog_info("Client " MACSTR ": No Information about client. Force reconnect:\n", MAC2STR(j->client_addr.u8));
            print_client_entry(DAWNLOG_TRACE, j);
            del_client_interface(id, j->client_addr, 0, 1, 0);
        }
        else {
            dawnlog_info("Client " MACSTR ": Current AP is best. Client will stay:\n", MAC2STR(j->client_addr.u8));
            print_client_entry(DAWNLOG_TRACE, j);
            // set kick counter to 0 again
            j->kick_count = 0;
        }

        remove_kicking_nr_list(kick_nr_list);
        kick_nr_list = NULL;

        j = j->next_entry_bc;
    }

    if (dawn_metric.set_hostapd_nr == 2)
        ubus_set_nr_from_clients(ap_nr_list);

    // FIXME: Consider retaining this so it can be used for kick type 2 - low absolute RSSI
    remove_kicking_nr_list(ap_nr_list);
    ap_nr_list = NULL;

    dawnlog_trace("KICKING: --------- AP Finished ---------\n");

    dawn_mutex_unlock(&ap_array_mutex);
    dawn_mutex_unlock(&probe_array_mutex);
    dawn_mutex_unlock(&client_array_mutex);

    return kicked_clients;
}

void update_iw_info(struct dawn_mac bssid_mac) {
    dawnlog_debug_func("Entering...");

    dawn_mutex_lock(&client_array_mutex);
    dawn_mutex_lock(&probe_array_mutex);
    dawn_mutex_lock(&ap_array_mutex);

    dawn_mutex_require(&ap_array_mutex);
    ap* this_ap = ap_array_get_ap(bssid_mac);

    dawnlog_trace("IW info update for AP " MACSTR "\n", MAC2STR(bssid_mac.u8));

    // Go through clients for BSSID
    for (client* j = *client_find_first_bc_entry(bssid_mac, dawn_mac_null, false);
            j != NULL && mac_is_equal_bb(j->bssid_addr, bssid_mac); j = j->next_entry_bc) {
        // update rssi
        int rssi = get_rssi_iwinfo(j->client_addr);
        dawnlog_trace("Expected throughput %f Mbit/sec\n",
                iee80211_calculate_expected_throughput_mbit(get_expected_throughput_iwinfo(j->client_addr)));

        if (rssi != INT_MIN) {
            if (probe_array_update_rssi(j->client_addr, this_ap, rssi, true) == NULL) {
                dawnlog_info("Failed to update rssi!\n");
            }
            else {
                dawnlog_trace("Updated rssi: %d\n", rssi);
            }
        }
    }

    dawnlog_trace("---------------------------\n");

    dawn_mutex_unlock(&ap_array_mutex);
    dawn_mutex_unlock(&probe_array_mutex);
    dawn_mutex_unlock(&client_array_mutex);
}

client *client_array_get_client(const struct dawn_mac client_addr)
{
    dawnlog_debug_func("Entering...");

    dawn_mutex_require(&client_array_mutex);
    client* ret = client_set_bc;
    while (ret != NULL && !mac_is_equal_bb(client_addr, ret->client_addr))
    {
        ret = ret->next_entry_bc;
    }

    return ret;
}

static client* client_array_unlink_entry(client** ref_bc, int unlink_only)
{
    dawnlog_debug_func("Entering...");
    client* entry = *ref_bc;

    *ref_bc = entry->next_entry_bc;

    if (unlink_only)
    {
        entry->next_entry_bc = NULL;
    }
    else
    {
        dawn_free(entry);
        entry = NULL;
    }

    return entry;
}

client* client_array_delete_bc(struct dawn_mac bssid_mac, struct dawn_mac client_mac) {
    dawnlog_debug_func("Entering...");

    client* ret = NULL;
    client** client_entry = client_find_first_bc_entry(bssid_mac, client_mac, true);

    dawn_mutex_require(&client_array_mutex);
    if (*client_entry && mac_is_equal_bb(bssid_mac, (*client_entry)->bssid_addr) && mac_is_equal_bb(client_mac, (*client_entry)->client_addr))
        ret = client_array_unlink_entry(client_entry, false);

    return ret;
}

client* client_array_delete(client* entry, int unlink_only) {
    dawnlog_debug_func("Entering...");

    client* ret = NULL;
    client** ref_bc = NULL;

    // Bodyless for-loop: test done in control logic
    dawn_mutex_require(&client_array_mutex);
    for (ref_bc = &client_set_bc; (*ref_bc != NULL) && (*ref_bc != entry); ref_bc = &((*ref_bc)->next_entry_bc));

    // Should never fail, but better to be safe...
    if (*ref_bc == entry)
        ret = client_array_unlink_entry(ref_bc, unlink_only);

    return ret;
}

int probe_array_delete(struct dawn_mac client_mac, struct dawn_mac bssid_mac) {
    int found_in_array = false;
    dawn_mutex_require(&probe_array_mutex);
    struct probe_entry_s** node_ref = &probe_set.first_probe;
    struct probe_entry_s** skip_ref = &probe_set.first_probe_skip;

    int cmp = 0;
    while ((*skip_ref) != NULL) {
        cmp = mac_compare_bb((*skip_ref)->client_addr, client_mac);

        if (cmp == 0)
            cmp = mac_compare_bb((*skip_ref)->bssid_addr, bssid_mac);

        if (cmp >= 0) {
            break;
        }
        else {
            node_ref = &((*skip_ref)->next_probe);
            skip_ref = &((*skip_ref)->next_probe_skip);
        }
    }

    while ((*node_ref) != NULL) {
        cmp = mac_compare_bb((*node_ref)->client_addr, client_mac);

        if (cmp == 0)
            cmp = mac_compare_bb((*node_ref)->bssid_addr, bssid_mac);

        if (cmp >= 0) {
            break;
        }
        else {
            node_ref = &((*node_ref)->next_probe);
        }
    }

    if (*node_ref && cmp == 0) {
        struct probe_entry_s* victim = *node_ref;

        *node_ref = victim->next_probe;
        probe_set.node_count--;

        if (*skip_ref == victim)
        {
            *skip_ref = victim->next_probe_skip;
            probe_set.skip_count--;
        }

        dawn_free(victim);
        victim = NULL;
        found_in_array = true;
    }

    return found_in_array;
}

int probe_array_set_all_probe_count(struct dawn_mac client_addr, uint32_t probe_count) {

    int updated = 0;

    dawnlog_debug_func("Entering...");

    // MUSTDO: Has some code been lost here?  updated never set... Certain to hit not found...
    dawn_mutex_lock(&probe_array_mutex);
    dawn_mutex_require(&probe_array_mutex);
    for (probe_entry *i = probe_array_find_first_entry(client_addr, dawn_mac_null, false);
            i != NULL && mac_is_equal_bb(client_addr, i->client_addr); 
            i = i->next_probe) {
        dawnlog_debug("Setting probecount for given mac!\n");
        i->counter = probe_count;
    }
    dawn_mutex_unlock(&probe_array_mutex);

    return updated;
}

probe_entry* probe_array_update_rssi(struct dawn_mac client_addr, ap* connected_ap, uint32_t rssi, int send_network)
{
    dawnlog_debug_func("Entering...");

    dawn_mutex_require(&probe_array_mutex);
    probe_entry* probe_req_new = dawn_malloc(sizeof(probe_entry));
    probe_entry* probe_req_updated = NULL;

    if (probe_req_new) {
        // Fields we will update
        probe_req_new->client_addr = client_addr;
        probe_req_new->bssid_addr = connected_ap->bssid_addr;
        probe_req_new->signal = rssi;
        probe_req_new->freq = connected_ap->freq;

        // Other fields in case entry is new
        probe_req_new->ht_capabilities = false;
        probe_req_new->vht_capabilities = false;
        probe_req_new->rcpi = -1;
        probe_req_new->rsni = -1;

        probe_req_updated = insert_to_probe_array(probe_req_new, false, false, time(0));
        if (probe_req_new != probe_req_updated)
        {
            dawnlog_info("RSSI PROBE used to update client / BSSID = " MACSTR " / " MACSTR " \n", MAC2STR(probe_req_updated->client_addr.u8), MAC2STR(probe_req_updated->bssid_addr.u8));

            dawn_free(probe_req_new);
            probe_req_new = NULL;
        }
        else
        {
            dawnlog_info("RSSI PROBE is new for client / BSSID = " MACSTR " / " MACSTR " \n", MAC2STR(probe_req_updated->client_addr.u8), MAC2STR(probe_req_updated->bssid_addr.u8));
        }

        if (send_network)
            ubus_send_probe_via_network(probe_req_updated, false);
    }

    return probe_req_updated;
}

probe_entry *probe_array_get_entry(struct dawn_mac client_mac, struct dawn_mac bssid_mac) {
    dawnlog_debug_func("Entering...");

    dawn_mutex_require(&probe_array_mutex);
    probe_entry* ret = probe_array_find_first_entry(client_mac, bssid_mac, true);

    return ret;
}

void print_probe_array() {
    if (dawnlog_showing(DAWNLOG_DEBUG))
    {
        dawnlog_debug("------------------\n");
        // dawnlog_debug("Probe Entry Last: %d\n", probe_entry_last);
        dawn_mutex_require(&probe_array_mutex);
        for (probe_entry* i = probe_set.first_probe; i != NULL; i = i->next_probe) {
            print_probe_entry(DAWNLOG_DEBUG, i);
        }
        dawnlog_debug("------------------\n");
    }
}

probe_entry* insert_to_probe_array(probe_entry* entry, int inc_probe_count, int is_beacon, time_t expiry) {
    dawnlog_debug_func("Entering...");

    dawn_mutex_require(&probe_array_mutex);
    struct probe_entry_s** node_ref = &probe_set.first_probe;
    struct probe_entry_s** skip_ref = &probe_set.first_probe_skip;

    int cmp = 0;
    while ((*skip_ref) != NULL) {
        cmp = mac_compare_bb((*skip_ref)->client_addr, entry->client_addr);

        if (cmp == 0)
            cmp = mac_compare_bb((*skip_ref)->bssid_addr, entry->bssid_addr);

        if (cmp >= 0) {
            break;
        }
        else {
            node_ref = &((*skip_ref)->next_probe);
            skip_ref = &((*skip_ref)->next_probe_skip);
        }
    }

    while ((*node_ref) != NULL) {
        cmp = mac_compare_bb((*node_ref)->client_addr, entry->client_addr);

        if (cmp == 0)
            cmp = mac_compare_bb((*node_ref)->bssid_addr, entry->bssid_addr);

        if (cmp >= 0) {
            break;
        }
        else {
            node_ref = &((*node_ref)->next_probe);
        }
    }

    if (*node_ref && cmp == 0) {
        dawnlog_debug("Updating...\n");

        if (entry->freq)
            (*node_ref)->freq = entry->freq;

        if (!is_beacon)
        {
            if (inc_probe_count)
                (*node_ref)->counter++;

            entry->rssi_timestamp = expiry;

            // BEACON reports don't have these fields, so only update them from PROBE
            (*node_ref)->signal = entry->signal;

            // Some "synthetic" PROBE entries have FALSE for these which would overwrite genuine values
            if (entry->ht_capabilities)
                (*node_ref)->ht_capabilities = entry->ht_capabilities;

            if (entry->vht_capabilities)
                (*node_ref)->vht_capabilities = entry->vht_capabilities;
        }
        else
        {
            // FIXME: Equivalent to inserting BEACON based entry
            (*node_ref)->counter = dawn_metric.min_probe_count;

            entry->rcpi_timestamp = expiry;

            // PROBE reports don't have these fields, so only update them from BEACOM
            if (entry->rcpi != -1)
                (*node_ref)->rcpi = entry->rcpi;

            if (entry->rsni != -1)
                (*node_ref)->rsni = entry->rsni;
        }

        entry = *node_ref;
    }
    else
    {
        dawnlog_debug("Adding...\n");

        if (inc_probe_count && !is_beacon)
            entry->counter = 1;
        else
            entry->counter = 0;

        entry->rssi_timestamp = expiry;
        entry->rcpi_timestamp = expiry;
        
        entry->next_probe = *node_ref;
        *node_ref = entry;

        probe_set.node_count++;

        if (probe_set.node_count > (probe_set.skip_count * probe_set.skip_ratio))
        {
            probe_set.skip_count++;

            entry->next_probe_skip = *skip_ref;
            *skip_ref = entry;
        }
        else
        {
            entry->next_probe_skip = NULL;
        }
    }

    return entry;  // return pointer to what we used, which may not be what was passed in
}

static __inline__ ap* ap_array_unlink_entry(ap** i)
{
    ap* entry = *i;
    *i = entry->next_ap;
    ap_entry_last--;

    return entry;
}

static ap* ap_array_update_entry(ap* entry) {
    dawnlog_debug_func("Entering...");

    ap* old_entry = NULL;
    dawn_mutex_require(&ap_array_mutex);
    ap** i = &ap_set;
    while (*i != NULL && mac_compare_bb(entry->bssid_addr, (*i)->bssid_addr) != 0) {
        i = &((*i)->next_ap);
    }

    // Check that the SSID has not changed
    if (*i && strcmp((char*)(*i)->ssid, (char*)entry->ssid) == 0)
    {
        // Swap entries if same SSID...
        old_entry = *i;
        entry->next_ap = old_entry->next_ap;
        old_entry->next_ap = NULL;
        *i = entry;
    }
    else
    {
        // ... otherwise find new position
        if (*i)
            old_entry = ap_array_unlink_entry(i);

        ap_array_insert(entry);
    }

    return old_entry;
}

#if 0 // Delete pending if no longer required
// TODO: What is collision domain used for?
int ap_get_collision_count(int col_domain) {

    int ret_sta_count = 0;

    dawnlog_debug_func("Entering...");;

    pthread_mutex_lock(&ap_array_mutex);

    for (ap* i = ap_set; i != NULL; i = i->next_ap) {
        if (i->collision_domain == col_domain)
            ret_sta_count += i->station_count;
    }
    pthread_mutex_unlock(&ap_array_mutex);

    return ret_sta_count;
}
#endif

ap *insert_to_ap_array(ap* entry, time_t expiry) {
    dawnlog_debug_func("Entering...");

    entry->time = expiry;

    dawn_mutex_lock(&ap_array_mutex);

    dawn_mutex_require(&ap_array_mutex);
    ap* old_entry = ap_array_update_entry(entry);
    if (old_entry)
        dawn_free(old_entry);

    print_ap_array();

    dawn_mutex_unlock(&ap_array_mutex);

    return entry;
}


// AP entries are sorted by SSID and BSSID to simplify display of network tree
 void ap_array_insert(ap* entry) {
    dawnlog_debug_func("Entering...");;

    dawn_mutex_require(&ap_array_mutex);
    ap** insert_pos = &ap_set;

    while (*insert_pos != NULL)
    {
        int this_cmp = strcmp((char*)entry->ssid, (char*)(*insert_pos)->ssid);

        if (this_cmp == 0)
        {
            this_cmp = mac_compare_bb(entry->bssid_addr, (*insert_pos)->bssid_addr);
        }

        if (this_cmp <= 0)
            break;

        insert_pos = &((*insert_pos)->next_ap);
    }

    entry->next_ap = *insert_pos;
    *insert_pos = entry;
    ap_entry_last++;
}

ap* ap_array_get_ap(struct dawn_mac bssid_mac) {

    dawnlog_debug_func("Entering...");;

    dawn_mutex_require(&ap_array_mutex);
    ap* ret = ap_set;

    while (ret && mac_compare_bb(bssid_mac, ret->bssid_addr) != 0) {
        ret = ret->next_ap;
    }

    return ret;
}

int ap_array_delete(ap* entry) {
    int not_found = 1;

    dawnlog_debug_func("Entering...");;

    dawn_mutex_require(&ap_array_mutex);
    ap** i = &ap_set;
    while (*i != NULL) {
        if (*i == entry) {
            dawn_free(ap_array_unlink_entry(i));
            not_found = 0;
            break;
        }

        i = &((*i)->next_ap);
    }

    return not_found;
}

void remove_old_client_entries(time_t current_time, long long int threshold) {
    dawnlog_debug_func("Entering...");

    dawn_mutex_require(&client_array_mutex);
    client **i = &client_set_bc;
    while (*i  != NULL) {
        if ((*i)->time < current_time - threshold) {
            client_array_unlink_entry(i, false);
        }
        else {
            i = &((*i)->next_entry_bc);
        }
    }
}

void remove_old_probe_entries(time_t current_time, long long int threshold) {
    dawnlog_debug_func("Entering...");

    dawn_mutex_require(&probe_array_mutex);
    probe_entry** i = &(probe_set.first_probe);
    probe_entry** s = &(probe_set.first_probe_skip);

    dawn_mutex_lock(&client_array_mutex);

    while (*i != NULL ) {
        int rssi_expired = 0;
        int rcpi_expired = 0;

        // FIXME: Why do we not delete the old probe just because it is a client?  Maybe because legacy devices are slow to send another?
        if (((*i)->rssi_timestamp < current_time - threshold) && !client_array_get_client_for_bssid((*i)->bssid_addr, (*i)->client_addr))
        {
            (*i)->signal = 0;
            rssi_expired = 1;
        }

        if (((*i)->rcpi_timestamp < current_time - threshold))
        {
            (*i)->rcpi = -1;
            (*i)->rsni = -1;
            rcpi_expired = 1;
        }

        if (rssi_expired && rcpi_expired) {
            probe_entry* victim = *i;

            *i = victim->next_probe;

            if (*s == victim)
            {
                *s = victim->next_probe_skip;
            }

            dawn_free(victim);
            victim = NULL;
        }
        else {
            if ((*i)->next_probe_skip != NULL)
            {
                s = &((*i)->next_probe_skip);
            }

            i = &((*i)->next_probe);
        }
    }

    dawn_mutex_unlock(&client_array_mutex);
}

void remove_old_ap_entries(time_t current_time, long long int threshold) {
    dawn_mutex_require(&ap_array_mutex);
    ap **i = &ap_set;
    while (*i != NULL) {
        if (((*i)->time) < (current_time - threshold)) {
            dawn_free(ap_array_unlink_entry(i));
        }
        else {
            i = &((*i)->next_ap);
        }
    }
}

client* client_array_update_entry(client* entry, time_t expiry) {
    dawnlog_debug_func("Entering...");
    dawn_mutex_require(&client_array_mutex);

    client* old_entry = NULL;
    client** entry_pos = client_find_first_bc_entry(entry->bssid_addr, entry->client_addr, true);

    entry->time = expiry;

	    if (*entry_pos != NULL &&
	            mac_compare_bb(entry->bssid_addr, (*entry_pos)->bssid_addr) == 0 && 
	            mac_compare_bb(entry->client_addr, (*entry_pos)->client_addr) == 0)
    {
        dawnlog_debug_func("Replacing entry...");
        // Swap entries if same BSSID + MAC...
        old_entry = *entry_pos;
        entry->kick_count = old_entry->kick_count;  // TODO: Not sure we need to keep this...

        entry->next_entry_bc = old_entry->next_entry_bc;
        old_entry->next_entry_bc = NULL;
        *entry_pos = entry;
    }
    else
    {
        dawnlog_debug_func("Adding entry...");
        // ... or add if new
        entry->kick_count = 0;

        entry->next_entry_bc = *entry_pos;
        *entry_pos = entry;
    }

    return old_entry;
}

client *insert_client_to_array(client *entry, time_t expiry) {
    dawnlog_debug_func("Entering...");
    dawn_mutex_require(&client_array_mutex);

    client * ret = NULL;

    client **client_tmp = client_find_first_bc_entry(entry->bssid_addr, entry->client_addr, true);

    if (*client_tmp == NULL || !mac_is_equal_bb(entry->bssid_addr, (*client_tmp)->bssid_addr) || !mac_is_equal_bb(entry->client_addr, (*client_tmp)->client_addr)) {
        entry->kick_count = 0;
        entry->time = expiry;

        entry->next_entry_bc = *client_tmp;
        *client_tmp = entry;

        ret = entry;
    }
    else {
        (*client_tmp)->time = expiry;
    }

    return ret;
}

void insert_macs_from_file() {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    dawnlog_debug_func("Entering...");

    fp = fopen("/tmp/dawn_mac_list", "r");
    if (fp == NULL)
    {
        dawnlog_error("Failed opening MAC list file - quitting!\n");
        exit(EXIT_FAILURE);
    }

    dawn_regmem(fp);

    read = getline(&line, &len, fp);
#ifdef DAWN_MEMORY_AUDITING
    if (line)
        dawn_regmem(line);
#endif

    while (read != -1) {

        dawnlog_debug("Retrieved line of length %zu :\n", read);
        dawnlog_debug("%s", line);

        insert_to_maclist(str2mac(line));

#ifdef DAWN_MEMORY_AUDITING
        char* old_line = line;
#endif
        read = getline(&line, &len, fp);
#ifdef DAWN_MEMORY_AUDITING
        if (old_line != line)
        {
            dawn_unregmem(old_line);
            dawn_regmem(line);
        }
#endif
    }

    if (dawnlog_showing(DAWNLOG_DEBUG))
    {
        dawnlog_debug("Printing MAC list:\n");
        for (struct mac_entry_s* i = mac_set; i != NULL; i = i->next_mac) {
            dawnlog_debug(MACSTR "\n", MAC2STR(i->mac.u8));
        }
    }

    fclose(fp);
    dawn_unregmem(fp);
    if (line)
    {
        free(line);
        dawn_unregmem(line);
    }

    //exit(EXIT_SUCCESS);
}


// TODO: This list only ever seems to get longer.  Why do we need it?
struct mac_entry_s *insert_to_maclist(struct dawn_mac mac) {
    dawnlog_debug_func("Entering...");

    struct mac_entry_s* new_mac = NULL;

    if (mac_find_entry(mac) == NULL)
    {
        struct mac_entry_s* new_mac = dawn_malloc(sizeof(struct mac_entry_s));
        if (new_mac == NULL)
        {
            dawnlog_error("malloc of MAC struct failed!\n");
        }
        else
        {
            new_mac->mac = mac;

            new_mac->next_mac = mac_set;
            mac_set = new_mac;
            mac_set_last++;
        }
    }

    return new_mac;
}

void print_probe_entry(int level, probe_entry *entry) {
    dawn_mutex_require(&probe_array_mutex);
    if (dawnlog_showing(level))
    {
        dawnlog(level,
            "bssid_addr: " MACSTR ", client_addr: " MACSTR ", signal : % d, freq : "
            "%d, counter: %d, vht: %d, min_rate: %d, max_rate: %d\n",
            MAC2STR(entry->bssid_addr.u8), MAC2STR(entry->client_addr.u8),
            entry->signal, entry->freq, entry->counter, entry->vht_capabilities,
            entry->min_supp_datarate, entry->max_supp_datarate);
    }
}

void print_client_req_entry(int level, client_req_entry *entry) {
    if (dawnlog_showing(DAWNLOG_INFO))
    {
        dawnlog_info(
            "bssid_addr: " MACSTR ", client_addr: " MACSTR ", signal : % d, freq : %d\n",
            MAC2STR(entry->bssid_addr.u8), MAC2STR(entry->client_addr.u8), entry->signal, entry->freq);
    }
}

void print_client_entry(int level, client *entry) {
    dawn_mutex_require(&client_array_mutex);
    if (dawnlog_showing(level))
    {
        dawnlog(level, "bssid_addr: " MACSTR ", client_addr: " MACSTR ", freq: %d, ht_supported: %d, vht_supported: %d, ht: %d, vht: %d, kick: %d\n",
            MAC2STR(entry->bssid_addr.u8), MAC2STR(entry->client_addr.u8), entry->freq, entry->ht_supported, entry->vht_supported, entry->ht, entry->vht,
            entry->kick_count);
    }
}

void print_client_array() {
    if (dawnlog_showing(DAWNLOG_DEBUG))
    {
        dawnlog_debug("--------Clients------\n");
        //dawnlog_debug("Client Entry Last: %d\n", client_entry_last);
        dawn_mutex_require(&client_array_mutex);
        for (client* i = client_set_bc; i != NULL; i = i->next_entry_bc) {
            print_client_entry(DAWNLOG_DEBUG, i);
        }
        dawnlog_debug("------------------\n");
    }
}

static void print_ap_entry(int level, ap *entry) {
    dawn_mutex_require(&ap_array_mutex);
    if (dawnlog_showing(level))
    {
        dawnlog(level,"ssid: %s, bssid_addr: " MACSTR ", freq: %d, ht: %d, vht: %d, chan_utilz: %d, neighbor_report: %s\n",
            entry->ssid, MAC2STR(entry->bssid_addr.u8), entry->freq, entry->ht_support, entry->vht_support, entry->channel_utilization,
	        //entry->collision_domain, ap_get_collision_count(entry->collision_domain), // TODO: Fix format string if readding
	        entry->neighbor_report
            );
    }
}

void print_ap_array() {
    if (dawnlog_showing(DAWNLOG_TRACE))
    {
        dawnlog_trace("--------APs------\n");
        dawn_mutex_require(&ap_array_mutex);
        for (ap* i = ap_set; i != NULL; i = i->next_ap) {
            print_ap_entry(DAWNLOG_TRACE, i);
        }
        dawnlog_debug("------------------\n");
    }
}

void destroy_mutex() {

    // free resources
    dawnlog_info("Freeing mutex resources\n");
    pthread_mutex_destroy(&probe_array_mutex);
    pthread_mutex_destroy(&client_array_mutex);
    pthread_mutex_destroy(&ap_array_mutex);

    return;
}

int init_mutex() {

    if (pthread_mutex_init(&probe_array_mutex, NULL) != 0) {
        dawnlog_error("Mutex init failed!\n");
        return 1;
    }

    if (pthread_mutex_init(&client_array_mutex, NULL) != 0) {
        dawnlog_error("Mutex init failed!\n");
        return 1;
    }

    if (pthread_mutex_init(&ap_array_mutex, NULL) != 0) {
        dawnlog_error("Mutex init failed!\n");
        return 1;
    }

    return 0;
}
