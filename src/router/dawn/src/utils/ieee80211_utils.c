#include "ieee80211_utils.h"

double iee80211_calculate_bitrate(uint8_t supp_rate_val) {
    return ((double) supp_rate_val) / 2;
}

double iee80211_calculate_expected_throughput_mbit(int exp_thr) {
    return (((double) exp_thr) / 1000);
}

// FIXME: This calculation seems to be unreliable.  Is it device specific?
int rcpi_to_rssi(int rcpi)
{
    return rcpi / 2 - 110;
}

static int get_rrm_mode_val(char mode) {
    int ret = 0;
    switch (mode) {
    case 'A':
    case 'a':
        ret = WLAN_RRM_CAPS_BEACON_REPORT_ACTIVE;
        break;
    case 'P':
    case 'p':
        ret = WLAN_RRM_CAPS_BEACON_REPORT_PASSIVE;
        break;
    case 'B':
    case 'b':
    case 'T':
    case 't':
        ret = WLAN_RRM_CAPS_BEACON_REPORT_TABLE;
        break;
    }
    return ret;
}

int parse_rrm_mode(int* rrm_mode_order, const char* mode_string) {
    if (!mode_string)
        mode_string = DEFAULT_RRM_MODE_ORDER;

    int mask = 0;
    int order = 0;
    while ((*mode_string != 0) && (order < __RRM_BEACON_RQST_MODE_MAX)) {
        int mode_val = get_rrm_mode_val(*mode_string);
        if (mode_val && (mask & mode_val) == 0)
        {
            rrm_mode_order[order++] = mode_val;
            mask |= mode_val;
        }

        mode_string++;
    }

    while (order < __RRM_BEACON_RQST_MODE_MAX)
    {
        rrm_mode_order[order++] = 0;
    }

    return mask;
}
