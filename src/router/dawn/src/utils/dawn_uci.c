#include <ctype.h>
#include <uci.h>
#include <stdlib.h>
#include <string.h>

#include "memory_utils.h"
#include "datastorage.h"
#include "dawn_iwinfo.h"
#include "dawn_uci.h"


static struct uci_context *uci_ctx = NULL;
static struct uci_package *uci_pkg = NULL;

static void set_if_present_int(int *ret, struct uci_section *s, const char* option) {
    const char *str;

    if (s && (str = uci_lookup_option_string(uci_ctx, s, option)))
        *ret = atoi(str);
}

#define DAWN_SET_CONFIG_INT(m, s, conf) \
    set_if_present_int(&m.conf, s, #conf)


void uci_get_hostname(char* hostname)
{
    dawnlog_debug_func("Entering...");

    char path[]= "system.@system[0].hostname";
    struct  uci_ptr ptr;
    struct  uci_context *c = uci_alloc_context();
    dawn_regmem(c);

    if(!c){
        return;
    }

    if ((uci_lookup_ptr(c, &ptr, path, true) != UCI_OK) || (ptr.o==NULL || ptr.o->v.string==NULL)){
        uci_free_context(c);
        dawn_unregmem(c);
        return;
    }

    if(ptr.flags & UCI_LOOKUP_COMPLETE)
    {
        char *dot = strchr(ptr.o->v.string, '.');
        size_t len = HOST_NAME_MAX - 1;

        if (dot && dot < ptr.o->v.string + len)
        {
            len = dot - ptr.o->v.string;
        }
        snprintf(hostname, HOST_NAME_MAX, "%.*s", (int)len, ptr.o->v.string);
    }

    uci_free_context(c);
    dawn_unregmem(c);
}


static void set_if_present_time_t(time_t *ret, struct uci_section *s, const char* option) {
    const char *str;

    if (s && (str = uci_lookup_option_string(uci_ctx, s, option)))
        *ret = atoi(str);
}

#define DAWN_SET_CONFIG_TIME(m, s, conf) \
    set_if_present_time_t(&m.conf, s, #conf)

struct time_config_s uci_get_time_config() {
    struct time_config_s ret = {
        .update_client = 10,
        .remove_client = 15,
        .remove_probe = 30,
        .update_hostapd = 10,
        .remove_ap = 460,
        .update_tcp_con = 10,
        .update_chan_util = 5,
        .update_beacon_reports = 20,
        .con_timeout = 60,
    };

    dawnlog_debug_func("Entering...");

    struct uci_element *e;
    uci_foreach_element(&uci_pkg->sections, e)
    {
        struct uci_section *s = uci_to_section(e);

        if (strcmp(s->type, "times") == 0) {
            //CONFIG-T: update_client|Timer to send refresh local connection information and revised NEIGHBOR REPORT to all clients|[10]
            DAWN_SET_CONFIG_TIME(ret, s, update_client);
            //CONFIG-T: remove_client|Timer to remove expired client entries from core data set|[15]
            DAWN_SET_CONFIG_TIME(ret, s, remove_client);
            //CONFIG-T: remove_probe|Timer to remove expired PROBE and BEACON entries from core data set|[30]
            DAWN_SET_CONFIG_TIME(ret, s, remove_probe);
            //CONFIG-T: update_hostapd|Timer to (re-)register for hostapd messages for each local BSSID|[10]
            DAWN_SET_CONFIG_TIME(ret, s, update_hostapd);
            //CONFIG-T: remove_ap|Timer to remove expired AP entries from core data set|[460]
            DAWN_SET_CONFIG_TIME(ret, s, remove_ap);
            //CONFIG-T: update_tcp_con|Timer to refresh / remove the TCP connections to other DAWN instances found via uMDNS|[10]
            DAWN_SET_CONFIG_TIME(ret, s, update_tcp_con);
            //CONFIG-T: update_chan_util|Timer to get recent channel utilisation figure for each local BSSID|[5]
            DAWN_SET_CONFIG_TIME(ret, s, update_chan_util);
            //CONFIG-T: update_beacon_reports|Timer to ask all connected clients for a new BEACON REPORT|[20]
            DAWN_SET_CONFIG_TIME(ret, s, update_beacon_reports);
            //CONFIG-T: con_timeout|Timespan to check if a connection timed out|[60]
            DAWN_SET_CONFIG_TIME(ret, s, con_timeout);
            return ret;
        }
    }

    return ret;
}

struct local_config_s uci_get_local_config() {
    struct local_config_s ret = {
        .loglevel = 0,
    };

    dawnlog_debug_func("Entering...");

    struct uci_element* e;
    uci_foreach_element(&uci_pkg->sections, e)
    {
        struct uci_section* s = uci_to_section(e);

        if (strcmp(s->type, "local") == 0) {
            // CONFIG-L: loglevel|Verbosity of messages in syslog|[0 = Important only - very few messages]; 1 = Show what DAWN is processing in a user friendly way; 2 = Trace certain operations - for debugging; 3 = Broad low level tracing - for debugging
            DAWN_SET_CONFIG_INT(ret, s, loglevel);
        }
    }

    switch (ret.loglevel)
    {
    case 3:
        dawnlog_minlevel(DAWNLOG_DEBUG);
        break;
    case 2:
        dawnlog_minlevel(DAWNLOG_TRACE);
        break;
    case 1:
        dawnlog_minlevel(DAWNLOG_INFO);
        break;
    case 0:
    default:
        dawnlog_minlevel(DAWNLOG_ALWAYS);
        break;
    }

    return ret;
}



static struct mac_entry_s *insert_neighbor_mac(struct mac_entry_s *head, const char* mac) {
    dawnlog_debug_func("Entering...");

    struct mac_entry_s *new = dawn_malloc(sizeof(struct mac_entry_s));

    if (new == NULL) {
        dawnlog_error("Failed to allocate neighbor entry for '%s'\n", mac);
        return head;
    }
    memset(new, 0, sizeof (struct mac_entry_s));
    if (hwaddr_aton(mac, new->mac.u8) != 0) {
        dawnlog_error("Failed to parse MAC from '%s'\n", mac);
        dawn_free(new);
        new = NULL;
        return head;
    }
    new->next_mac = head;
    return new;
}

static void free_neighbor_mac_list(struct mac_entry_s *list) {
    struct mac_entry_s *ptr = list;
    dawnlog_debug_func("Entering...");

    while (list) {
        ptr = list;
        list = list->next_mac;
        dawn_free(ptr);
        ptr = NULL;
    }
}

static struct mac_entry_s* uci_lookup_mac_list(struct uci_option *o) {
    struct uci_element *e = NULL;
    struct mac_entry_s *head = NULL;
    char* str = NULL;

    dawnlog_debug_func("Entering...");

    if (o == NULL)
        return NULL;

    // hostapd inserts the new neigbor reports at the top of the list.
    // Therefore, we must also do this backwares somewhere.  Let's do it
    // here instead of when sending the list through ubus.
    switch (o->type) {
    case UCI_TYPE_LIST:
        uci_foreach_element(&o->v.list, e)
            head = insert_neighbor_mac(head, e->name);
        break;
    case UCI_TYPE_STRING:
        if (!(str = strdup (o->v.string)))
            return NULL;
        for (char *mac = strtok(str, " "); mac; mac = strtok(NULL, " "))
            head = insert_neighbor_mac(head, mac);
        free(str);
    }
    return head;
}

static struct uci_section *uci_find_metric_section(const char *name) {
    struct uci_section *s;
    struct uci_element *e;

    dawnlog_debug_func("Entering...");

    uci_foreach_element(&uci_pkg->sections, e) {
        s = uci_to_section(e);
        if (strcmp(s->type, "metric") == 0 &&
            ((!name && s->anonymous) || strcmp(e->name, name) == 0)) {
            return s;
        }
    }
    return NULL;
}

#define DAWN_SET_BANDS_CONFIG_INT(m, global_s, band_s, conf) \
    do for (int band = 0; band < __DAWN_BAND_MAX; band++) { \
        if (global_s) \
            set_if_present_int(&m.conf[band], global_s, #conf); \
        if (band_s[band]) \
            set_if_present_int(&m.conf[band], band_s[band], #conf); \
    } while (0)

struct probe_metric_s uci_get_dawn_metric() {
    struct probe_metric_s ret = {
        // CONFIG-G: kicking|Method to select clients to move to better AP|0 = Disabled; 1 = RSSI Comparison; 2 = Absolute RSSI; [3 = Both] See note 1.
        .kicking = 3,
        //CONFIG-G: kicking_threshold|Minimum score difference to consider kicking to alternate AP|[20]
        .kicking_threshold = 20,
        // CONFIG-G: min_probe_count|Number of times a client should retry PROBE before acceptance| [3] See Note 1.
        .min_probe_count = 3,
        // CONFIG-G: use_station_count|Compare connected station counts when considering kicking|[0 = Disabled]; 1 = Enabled
        .use_station_count = 0,
        // CONFIG-G: eval_auth_req|Control whether AUTHENTICATION frames are evaluated for rejection|[0 = No evaluation]; 1 = Evaluated. See Note 1.
        .eval_auth_req = 0,
        // CONFIG-G: eval_assoc_req|Control whether ASSOCIATION frames are evaluated for rejection|[0 = No evaluation]; 1 = Evaluated. See Note 1.
        .eval_assoc_req = 0,
        // CONFIG-G: deny_auth_reason|802.11 code used when AUTHENTICATION is denied|[1] (802.11 UNSPECIFIED_FAILURE). See Note 1.
        .deny_auth_reason = 1,
        // CONFIG-G: deny_assoc_reason|802.11 code used when ASSOCIATION is denied|[17] (802.11 AP_UNABLE_TO_HANDLE_NEW_STA). See Note 1.
        .deny_assoc_reason = 17,
        // CONFIG-G: eval_probe_req|Control whether PROBE frames are evaluated for rejection|[0 = No evaluation]; 1 = Evaluated. See Note 1.
        .eval_probe_req = 0,
        // CONFIG-G: min_number_to_kick|Number of consecutive times a client should be evaluated as ready to kick before actually doing it|[3]
        .min_number_to_kick = 3,
        // CONFIG-G: set_hostapd_nr|Method used to set Neighbor Report on AP|[0 = Disabled]; 1 = "Static" based on all APs in network (plus set from configuration); 2 = "Dynamic" based on next nearest AP seen by current clients
        .set_hostapd_nr = 0,
        // CONFIG-G: disassoc_nr_length|Number of entries to include in a 802.11v DISASSOCIATE Neighbor Report|[6] (Documented for use by iOS)
        .disassoc_nr_length = 6,
        // CONFIG-G: max_station_diff|Number of connected stations to consider "better" for use_station_count|[1]
        .max_station_diff = 1,
        // CONFIG-G: bandwidth_threshold|Maximum reported AP-client bandwidth permitted when kicking. Set to zero to disable the check.|[6] (Mbits/s)
        .bandwidth_threshold = 6,
        // CONFIG-G: chan_util_avg_period|Number of sampling periods to average channel utilization values over|[3]
        .chan_util_avg_period = 3,
        // CONFIG-G: duration|802.11k BEACON request DURATION parameter|[0]
        .duration = 150,
        // CONFIG-G: rrm_mode|Preferred order for using Passive, Active or Table 802.11k BEACON information|[PAT] String of 'P', 'A' and / or 'T'
        .rrm_mode_mask = WLAN_RRM_CAPS_BEACON_REPORT_PASSIVE |
                         WLAN_RRM_CAPS_BEACON_REPORT_ACTIVE |
                         WLAN_RRM_CAPS_BEACON_REPORT_TABLE,
        .rrm_mode_order = { WLAN_RRM_CAPS_BEACON_REPORT_PASSIVE,
                            WLAN_RRM_CAPS_BEACON_REPORT_ACTIVE,
                            WLAN_RRM_CAPS_BEACON_REPORT_TABLE },
        // CONFIG-B: ap_weight|Per AP weighting|[0] (Deprecated)
        .ap_weight = { 0, 0 },
        // CONFIG-B: ht_support|Score increment if HT is supported|[5]
        .ht_support = { 5, 5 },
        // CONFIG-B: vht_support|Score increment if VHT is supported|[5]
        .vht_support = { 5, 5 },
        // CONFIG-B: no_ht_support|Score incrment if HT is not supported|[0] (Deprecated)
        .no_ht_support = { 0, 0 },
        // CONFIG-B: no_vht_support|Score incrment if VHT is not supported|[0] (Deprecated)
        .no_vht_support = { 0, 0 },
        // CONFIG-B: rssi|Score addition when signal exceeds threshold|[15] See note 2.
        .rssi = { 15, 15 },
        // CONFIG-B: rssi_val|Threshold for an good RSSI|[-60] See note 2.
        .rssi_val = { -60, -60 },
        // CONFIG-B: initial_score|Base score for AP based on operating band|[2.4GHz = 80; 5Ghz = 100]
        .initial_score = { 80, 100 },
        // CONFIG-B: chan_util|Score increment if channel utilization is below chan_util_val|[0]
        .chan_util = { 0, 0 },
        // CONFIG-B: max_chan_util_val|Lower threshold for bad channel utilization|[170]
        .max_chan_util_val = { 170, 170 },
        // CONFIG-B: chan_util_val|Upper threshold for good channel utilization|[140]
        .chan_util_val = { 140, 140 },
        // CONFIG-B: max_chan_util|Score increment if channel utilization is above max_chan_util_val|[-15]
        .max_chan_util = { -15, -15 },
        // CONFIG-B: low_rssi|Score addition when signal is below threshold|[-15] See note 2.
        .low_rssi = { -15, -15 },
        // CONFIG-B: low_rssi_val|Threshold for bad RSSI|[-80] See note 2.
        .low_rssi_val = { -80, -80 },
        // CONFIG-B: rssi_center|Midpoint for weighted RSSI evaluation|[-70] See note 2.
        .rssi_center = { -70, -70 },
        // CONFIG-B: rssi_weight|Per dB increment for weighted RSSI evaluation|[0] See note 2.
        .rssi_weight = { 0, 0 },
        .neighbors = {NULL, NULL},
    };
    struct uci_section *global_s, *band_s[__DAWN_BAND_MAX];
    struct uci_option *global_neighbors = NULL, *neighbors;

    dawnlog_debug_func("Entering...");

    global_s = uci_find_metric_section("global");

    if (!global_s && (global_s = uci_find_metric_section(NULL)))
        dawnlog_warning("config metric global section not found. "
            "Using first unnamed config metric.\n"
            "Consider naming a 'global' metric section to avoid ambiguity.\n");

    if (!global_s)
        dawnlog_warning("config metric global section not found! Using defaults.\n");

    if (global_s) {
        // True global configuration
        DAWN_SET_CONFIG_INT(ret, global_s, kicking);
        DAWN_SET_CONFIG_INT(ret, global_s, kicking_threshold);
        DAWN_SET_CONFIG_INT(ret, global_s, min_probe_count);
        DAWN_SET_CONFIG_INT(ret, global_s, use_station_count);
        DAWN_SET_CONFIG_INT(ret, global_s, eval_auth_req);
        DAWN_SET_CONFIG_INT(ret, global_s, eval_assoc_req);
        DAWN_SET_CONFIG_INT(ret, global_s, deny_auth_reason);
        DAWN_SET_CONFIG_INT(ret, global_s, deny_assoc_reason);
        DAWN_SET_CONFIG_INT(ret, global_s, eval_probe_req);
        DAWN_SET_CONFIG_INT(ret, global_s, min_number_to_kick);
        DAWN_SET_CONFIG_INT(ret, global_s, set_hostapd_nr);
        DAWN_SET_CONFIG_INT(ret, global_s, disassoc_nr_length);
        DAWN_SET_CONFIG_INT(ret, global_s, max_station_diff);
        DAWN_SET_CONFIG_INT(ret, global_s, bandwidth_threshold);
        DAWN_SET_CONFIG_INT(ret, global_s, chan_util_avg_period);
        DAWN_SET_CONFIG_INT(ret, global_s, duration);
        ret.rrm_mode_mask = parse_rrm_mode(ret.rrm_mode_order,
                                           uci_lookup_option_string(uci_ctx, global_s, "rrm_mode"));
        global_neighbors = uci_lookup_option(uci_ctx, global_s, "neighbors");
    }

    for (int band = 0; band < __DAWN_BAND_MAX; band++) {
        band_s[band] = uci_find_metric_section(band_config_name[band]);
        neighbors = band_s[band] ? uci_lookup_option(uci_ctx, band_s[band], "neighbors") : NULL;
        ret.neighbors[band] = uci_lookup_mac_list(neighbors ? : global_neighbors);
    }

    DAWN_SET_BANDS_CONFIG_INT(ret, global_s, band_s, initial_score);
    DAWN_SET_BANDS_CONFIG_INT(ret, global_s, band_s, ap_weight);
    DAWN_SET_BANDS_CONFIG_INT(ret, global_s, band_s, ht_support);
    DAWN_SET_BANDS_CONFIG_INT(ret, global_s, band_s, vht_support);
    DAWN_SET_BANDS_CONFIG_INT(ret, global_s, band_s, no_ht_support);
    DAWN_SET_BANDS_CONFIG_INT(ret, global_s, band_s, no_vht_support);
    DAWN_SET_BANDS_CONFIG_INT(ret, global_s, band_s, rssi);
    DAWN_SET_BANDS_CONFIG_INT(ret, global_s, band_s, rssi_val);
    DAWN_SET_BANDS_CONFIG_INT(ret, global_s, band_s, chan_util);
    DAWN_SET_BANDS_CONFIG_INT(ret, global_s, band_s, max_chan_util);
    DAWN_SET_BANDS_CONFIG_INT(ret, global_s, band_s, chan_util_val);
    DAWN_SET_BANDS_CONFIG_INT(ret, global_s, band_s, max_chan_util_val);
    DAWN_SET_BANDS_CONFIG_INT(ret, global_s, band_s, low_rssi);
    DAWN_SET_BANDS_CONFIG_INT(ret, global_s, band_s, low_rssi_val);
    DAWN_SET_BANDS_CONFIG_INT(ret, global_s, band_s, rssi_weight);
    DAWN_SET_BANDS_CONFIG_INT(ret, global_s, band_s, rssi_center);
    return ret;
}

struct network_config_s uci_get_dawn_network() {
    struct network_config_s ret = {
        .broadcast_ip = "",
        .broadcast_port = 1025,
        .server_ip = "",
        .tcp_port = 1026,
        .network_option = 2,
        .shared_key = "Niiiiiiiiiiiiiik",
        .iv = "Niiiiiiiiiiiiiik",
        .use_symm_enc = 0,
        .collision_domain = -1,
        .bandwidth = -1,
    };

    dawnlog_debug_func("Entering...");

    struct uci_element *e;
    uci_foreach_element(&uci_pkg->sections, e)
    {
        struct uci_section *s = uci_to_section(e);

        if (strcmp(s->type, "network") == 0) {
            // CONFIG-N: broadcast_ip|IP address for broadcast and multicast|No default
            const char* str_broadcast = uci_lookup_option_string(uci_ctx, s, "broadcast_ip");
            if (str_broadcast)
                strncpy(ret.broadcast_ip, str_broadcast, MAX_IP_LENGTH);

            // CONFIG-N: server_ip|IP address when not using UMDNS|No default
            const char* str_server_ip = uci_lookup_option_string(uci_ctx, s, "server_ip");
            if(str_server_ip)
                strncpy(ret.server_ip, str_server_ip, MAX_IP_LENGTH);

            // CONFIG-N: broadcast_port|IP port for broadcast and multicast|[1026]
            DAWN_SET_CONFIG_INT(ret, s, broadcast_port);

            // CONFIG-N: shared_key|Unused|N/A
            const char* str_shared_key = uci_lookup_option_string(uci_ctx, s, "shared_key");
            if (str_shared_key)
                strncpy(ret.shared_key, str_shared_key, MAX_KEY_LENGTH);

            // CONFIG-N: iv|Unused|N/A
            const char* str_iv = uci_lookup_option_string(uci_ctx, s, "iv");
            if (str_iv)
                strncpy(ret.iv, str_iv, MAX_KEY_LENGTH);

            // CONFIG-N: network_option|Method of networking between DAWN instances|0 = Broadcast; 2 = Multicast; [2 = TCP with UMDNS discovery]; 3 = TCP w/out UMDNS discovery
            DAWN_SET_CONFIG_INT(ret, s, network_option);
            // CONFIG-N: tcp_port|Port for TCP networking|[1025]
            DAWN_SET_CONFIG_INT(ret, s, tcp_port);
            // CONFIG-N: use_symm_enc|Enable encryption of network traffic|[0 = Disabled]; 1 = Enabled
            DAWN_SET_CONFIG_INT(ret, s, use_symm_enc);
            // CONFIG-N: collision_domain|Unused|N/A
            DAWN_SET_CONFIG_INT(ret, s, collision_domain);
            // CONFIG-N: bandwidth|Unused|N/A
            DAWN_SET_CONFIG_INT(ret, s, bandwidth);
            return ret;
        }
    }

    return ret;
}

bool uci_get_dawn_hostapd_dir() {
    dawnlog_debug_func("Entering...");

    struct uci_element *e;
    uci_foreach_element(&uci_pkg->sections, e)
    {
        struct uci_section *s = uci_to_section(e);

        if (strcmp(s->type, "hostapd") == 0) {
            // CONFIG-H: hostapd_dir|Path to hostapd runtime information|[/var/run/hostapd]
            const char* str = uci_lookup_option_string(uci_ctx, s, "hostapd_dir");
            if (str)
                strncpy(hostapd_dir_glob, str, HOSTAPD_DIR_LEN);

            return true;
        }
    }

    // If we get to here we haven't set a value yet
    strncpy(hostapd_dir_glob, "/var/run/hostapd", HOSTAPD_DIR_LEN);

    return false;
}

int uci_reset()
{
    dawnlog_debug_func("Entering...");

    struct uci_context *ctx = uci_ctx;

    if (!ctx) {
        ctx = uci_alloc_context();
        dawn_regmem(ctx);
        uci_ctx = ctx;
    }
    uci_pkg = uci_lookup_package(ctx, "dawn");
    uci_unload(uci_ctx, uci_pkg);
    dawn_unregmem(uci_pkg);
    uci_load(uci_ctx, "dawn", &uci_pkg);
    dawn_regmem(uci_pkg);

    return 0;
}

int uci_init() {
    dawnlog_debug_func("Entering...");

    struct uci_context *ctx = uci_ctx;

    if (!ctx) {
        ctx = uci_alloc_context();
        dawn_regmem(ctx);
        uci_ctx = ctx;

        ctx->flags &= ~UCI_FLAG_STRICT;
    } else {
        ctx->flags &= ~UCI_FLAG_STRICT;
        // shouldn't happen?
        uci_pkg = uci_lookup_package(ctx, "dawn");
        if (uci_pkg)
        {
            uci_unload(ctx, uci_pkg);
            dawn_unregmem(uci_pkg);
            uci_pkg = NULL;
        }
    }

    if (uci_load(ctx, "dawn", &uci_pkg))
        return -1;
    else
        dawn_regmem(uci_pkg);

    return 1;
}

int uci_clear() {
    dawnlog_debug_func("Entering...");

    // CONFIG-G: neighbors|Space seperated list of MACS to use in "static" AP Neighbor Report| None
    for (int band = 0; band < __DAWN_BAND_MAX; band++)
        free_neighbor_mac_list(dawn_metric.neighbors[band]);

    if (uci_pkg != NULL) {
        uci_unload(uci_ctx, uci_pkg);
        dawn_unregmem(uci_pkg);
        uci_pkg = NULL;
    }
    if (uci_ctx != NULL) {
        uci_free_context(uci_ctx);
        dawn_unregmem(uci_ctx);
    }
    return 1;
}

int uci_set_network(char* uci_cmd)
{
    dawnlog_debug_func("UCI command = \"%s\"...", uci_cmd);

    struct uci_context *ctx  = uci_ctx;

    if (!ctx) {
        ctx = uci_alloc_context();
        dawn_regmem(ctx);
        uci_ctx = ctx;
    }

    ctx->flags |= UCI_FLAG_STRICT;

    int ret = UCI_OK;
    struct uci_ptr ptr;

    if (ret == UCI_OK)
    {
        ret = uci_lookup_ptr(ctx, &ptr, uci_cmd, 1);
    }

    if (ret == UCI_OK)
    {
        // Magic code to add unnamed section like 'config times' exactly once - no idea if this is quite right
        if (ptr.target == UCI_TYPE_SECTION && ptr.value == NULL)
        {
            char new_cmd[1024]; // Magic number
            sprintf(new_cmd, "%s.@%s[0]", ptr.package, ptr.section);
            struct uci_ptr ptr2;
            ret = uci_lookup_ptr(ctx, &ptr2, new_cmd, 1);

            if (ret == UCI_OK && ptr2.s == NULL)
            {
                ret = uci_add_section(ctx, ptr.p, ptr.section, &ptr.s);
            }
        }
        else
        {
            ret = uci_set(ctx, &ptr);
        }
    }

    if (ret == UCI_OK)
    {
        ret = uci_lookup_ptr(ctx, &ptr, "dawn", 1);
    }

    if (ret == UCI_OK)
    {
        ret = uci_commit(ctx, &ptr.p, 0);
    }

    if (ret != UCI_OK) {
        dawnlog_error("Failed to commit UCI cmd: %s\n", uci_cmd);
    }

    return ret;
}
