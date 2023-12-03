#include <iwinfo.h>
#include <limits.h>

#include "mac_utils.h"
#include "datastorage.h"
#include "dawn_iwinfo.h"

char hostapd_dir_glob[HOSTAPD_DIR_LEN];

int call_iwinfo(char *client_addr);

int parse_rssi(char *iwinfo_string);

int get_rssi(const char *ifname, struct dawn_mac client_addr);

int get_bandwidth(const char *ifname, struct dawn_mac client_addr, float *rx_rate, float *tx_rate);

#define IWINFO_BUFSIZE    24 * 1024

#define IWINFO_ESSID_MAX_SIZE    32

int compare_essid_iwinfo(struct dawn_mac bssid_addr, struct dawn_mac bssid_addr_to_compare) {
    const struct iwinfo_ops *iw;

    DIR *dirp;
    struct dirent *entry;
    dirp = opendir(hostapd_dir_glob);  // error handling?
    if (!dirp) {
        dawnlog_error("[COMPARE ESSID] Failed to open %s\n", hostapd_dir_glob);
        return 0;
    }

    char *essid = NULL;
    char *essid_to_compare = NULL;

    char buf_essid[IWINFO_ESSID_MAX_SIZE + 1] = {0};
    char buf_essid_to_compare[IWINFO_ESSID_MAX_SIZE + 1] = {0};

    while ((entry = readdir(dirp)) != NULL && (essid == NULL || essid_to_compare == NULL)) {
        if (entry->d_type == DT_SOCK) {
            if (strcmp(entry->d_name, "global") == 0)
                continue;

            iw = iwinfo_backend(entry->d_name);

            // FIXME: Try to reduce string conversion and comparison here by using byte array compares
            // TODO: Magic number
            static char buf_bssid[18] = {0};
            if (iw->bssid(entry->d_name, buf_bssid))
                snprintf(buf_bssid, sizeof(buf_bssid), "00:00:00:00:00:00");

            char mac_buf[20];
            sprintf(mac_buf, MACSTR, MAC2STR(bssid_addr.u8));

            if (strcmp(mac_buf, buf_bssid) == 0) {

                if (iw->ssid(entry->d_name, buf_essid))
                    memset(buf_essid, 0, sizeof(buf_essid));
                essid = buf_essid;
            }

            char mac_buf_to_compare[20];
            sprintf(mac_buf_to_compare, MACSTR, MAC2STR(bssid_addr_to_compare.u8));

            if (strcmp(mac_buf_to_compare, buf_bssid) == 0) {
                if (iw->ssid(entry->d_name, buf_essid_to_compare))
                    memset(buf_essid_to_compare, 0, sizeof(buf_essid_to_compare));
                essid_to_compare = buf_essid_to_compare;
            }
            iwinfo_finish();
        }
    }
    closedir(dirp);

    dawnlog_debug("Comparing: %s with %s\n", essid, essid_to_compare);

    if (essid == NULL || essid_to_compare == NULL) {
        return -1;
    }

    if (strcmp(essid, essid_to_compare) == 0) {
        return 0;
    }

    return -1;
}

int get_bandwidth_iwinfo(struct dawn_mac client_addr, float *rx_rate, float *tx_rate) {

    DIR *dirp;
    struct dirent *entry;
    dirp = opendir(hostapd_dir_glob);  // error handling?
    if (!dirp) {
        dawnlog_error("[BANDWIDTH INFO] Failed to open %s\n", hostapd_dir_glob);
        return 0;
    }
    else
    {
        dawnlog_debug("[BANDWIDTH INFO] Opened %s\n", hostapd_dir_glob);
    }

    int sucess = 0;

    while (!sucess && ((entry = readdir(dirp)) != NULL)) {
        if (entry->d_type == DT_SOCK) {
            dawnlog_debug("[BANDWIDTH INFO] Trying %s\n", entry->d_name);
            sucess = get_bandwidth(entry->d_name, client_addr, rx_rate, tx_rate);
        }
    }
    closedir(dirp);
    return sucess;
}

int get_bandwidth(const char *ifname, struct dawn_mac client_addr, float *rx_rate, float *tx_rate) {
    int ret = 0;

    if (strcmp(ifname, "global") != 0)
    {
        const struct iwinfo_ops* iw = iwinfo_backend(ifname);

        char buf[IWINFO_BUFSIZE];
        int len;
        if (iw->assoclist(ifname, buf, &len) == 0 && len > 0)
        {
            struct iwinfo_assoclist_entry* e = (struct iwinfo_assoclist_entry*)buf;
            for (int i = 0; ret == 0 && i < len; i += sizeof(struct iwinfo_assoclist_entry)) {

                if (mac_is_equal(client_addr.u8, e->mac)) {
                    *rx_rate = e->rx_rate.rate / 1000.0;
                    *tx_rate = e->tx_rate.rate / 1000.0;

                    ret = 1;
                }

                e++;
            }
        }

        iwinfo_finish();
    }

    return ret;
}

int get_rssi_iwinfo(struct dawn_mac client_addr) {

    DIR *dirp;
    struct dirent *entry;
    dirp = opendir(hostapd_dir_glob);  // error handling?
    if (!dirp) {
        dawnlog_error("[RSSI INFO] No hostapd sockets!\n");
        return INT_MIN;
    }

    int rssi = INT_MIN;

    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_SOCK) {
            rssi = get_rssi(entry->d_name, client_addr);
            if (rssi != INT_MIN)
                break;
        }
    }
    closedir(dirp);
    return rssi;
}

int get_rssi(const char *ifname, struct dawn_mac client_addr) {

    int i, len;
    char buf[IWINFO_BUFSIZE];
    struct iwinfo_assoclist_entry *e;
    const struct iwinfo_ops *iw;
    if (strcmp(ifname, "global") == 0)
        return INT_MIN;
    iw = iwinfo_backend(ifname);

    if (iw->assoclist(ifname, buf, &len)) {
        dawnlog_warning("No information available\n");
        iwinfo_finish();
        return INT_MIN;
    } else if (len <= 0) {
        dawnlog_trace("No station connected to %s\n", ifname);
        iwinfo_finish();
        return INT_MIN;
    }

    for (i = 0; i < len; i += sizeof(struct iwinfo_assoclist_entry)) {
        e = (struct iwinfo_assoclist_entry *) &buf[i];

        if (mac_is_equal(client_addr.u8, e->mac)) {
            iwinfo_finish();
            return e->signal;
        }
    }

    iwinfo_finish();
    return INT_MIN;
}

int get_expected_throughput_iwinfo(struct dawn_mac client_addr) {

    DIR *dirp;
    struct dirent *entry;
    dirp = opendir(hostapd_dir_glob);  // error handling?
    if (!dirp) {
        dawnlog_error("[RSSI INFO] Failed to open dir:%s\n", hostapd_dir_glob);
        return INT_MIN;
    }

    int exp_thr = INT_MIN;

    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_SOCK) {
            exp_thr = get_expected_throughput(entry->d_name, client_addr);
            if (exp_thr != INT_MIN)
                break;
        }
    }
    closedir(dirp);
    return exp_thr;
}

int get_expected_throughput(const char *ifname, struct dawn_mac client_addr) {

    int i, len;
    char buf[IWINFO_BUFSIZE];
    struct iwinfo_assoclist_entry *e;
    const struct iwinfo_ops *iw;
    if (strcmp(ifname, "global") == 0)
        return INT_MIN;
    iw = iwinfo_backend(ifname);

    if (iw->assoclist(ifname, buf, &len)) {
        dawnlog_warning("No information available\n");
        iwinfo_finish();
        return INT_MIN;
    } else if (len <= 0) {
        dawnlog_trace("No station connected to %s\n", ifname);
        iwinfo_finish();
        return INT_MIN;
    }

    for (i = 0; i < len; i += sizeof(struct iwinfo_assoclist_entry)) {
        e = (struct iwinfo_assoclist_entry *) &buf[i];

        if (mac_is_equal(client_addr.u8, e->mac)) {
            iwinfo_finish();
            return e->thr;
        }
    }
    iwinfo_finish();

    return INT_MIN;
}

int get_bssid(const char *ifname, uint8_t bssid_addr[]) {
    const struct iwinfo_ops *iw;
    if (strcmp(ifname, "global") == 0)
        return 0;
    iw = iwinfo_backend(ifname);

    static char buf[18] = { 0 };

    if (iw->bssid(ifname, buf))
        snprintf(buf, sizeof(buf), "00:00:00:00:00:00");

    hwaddr_aton(buf,bssid_addr);
    iwinfo_finish();

    return 0;
}

int get_ssid(const char *ifname, char* ssid, size_t ssidmax) {
    const struct iwinfo_ops *iw;
    char buf[IWINFO_ESSID_MAX_SIZE+1] = { 0 };
    if (strcmp(ifname, "global") == 0)
        return 0;
    iw = iwinfo_backend(ifname);
    if (iw->ssid(ifname, buf))
        memset(buf, 0, sizeof(buf));

    strncpy(ssid, buf, ssidmax);
    iwinfo_finish();
    return 0;
}

int get_channel_utilization(const char *ifname, uint64_t *last_channel_time, uint64_t *last_channel_time_busy) {

    int len;
    const struct iwinfo_ops *iw;
    char buf[IWINFO_BUFSIZE];
    struct iwinfo_survey_entry *e;
    int ret = 0;
    if (strcmp(ifname, "global") == 0)
        return 0;
    iw = iwinfo_backend(ifname);

    int freq;
    if (iw->frequency(ifname, &freq))
    {
        iwinfo_finish();
        return 0;
    }

    if (iw->survey(ifname, buf, &len))
    {
        dawnlog_warning("Survey not possible!\n");
        iwinfo_finish();
        return 0;
    }
    else if (len <= 0)
    {
        dawnlog_warning("No survey results\n");
        iwinfo_finish();
        return 0;
    }

    for (int i = 0, x = 1; i < len; i += sizeof(struct iwinfo_survey_entry), x++)
    {
        e = (struct iwinfo_survey_entry *) &buf[i];

        if(e->mhz == freq)
        {
            uint64_t dividend = e->busy_time - *last_channel_time_busy;
            uint64_t divisor =  e->active_time - *last_channel_time;
            *last_channel_time = e->active_time;
            *last_channel_time_busy = e->busy_time;

            if(divisor)
                ret = (int)(dividend * 255 / divisor);

            break;
        }
    }

    iwinfo_finish();
    return ret;
}

int support_ht(const char *ifname) {
    const struct iwinfo_ops *iw;
    if (strcmp(ifname, "global") == 0)
        return 0;
    iw = iwinfo_backend(ifname);
    int htmodes = 0;

    if (iw->htmodelist(ifname, &htmodes))
    {
        dawnlog_debug("No HT mode information available\n");
        iwinfo_finish();
        return 0;
    }

    uint32_t ht_support_bitmask = (1 << 0) | (1 << 2);
    int ret = htmodes & ht_support_bitmask ? 1 : 0;
    iwinfo_finish();
    return ret;
}

int support_vht(const char *ifname) {
    const struct iwinfo_ops *iw;
    if (strcmp(ifname, "global") == 0)
        return 0;
    iw = iwinfo_backend(ifname);
    int htmodes = 0;

    if (iw->htmodelist(ifname, &htmodes))
    {
        dawnlog_error("No VHT mode information available\n");
        iwinfo_finish();
        return 0;
    }

    // TODO: 1 << 2 duplicated here, and also appears as HT mode mask
    uint32_t vht_support_bitmask = (1 << 2) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
    int ret = htmodes & vht_support_bitmask ? 1 : 0;
    iwinfo_finish();
    return ret;
}
