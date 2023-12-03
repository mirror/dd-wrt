#ifndef DAWN_IEEE80211_UTILS_H
#define DAWN_IEEE80211_UTILS_H

#include <stdint.h>

#ifndef BIT
#define BIT(x) (1U << (x))
#endif
#define WLAN_RRM_CAPS_BEACON_REPORT_PASSIVE BIT(4)
#define WLAN_RRM_CAPS_BEACON_REPORT_ACTIVE BIT(5)
#define WLAN_RRM_CAPS_BEACON_REPORT_TABLE BIT(6)

#define DEFAULT_RRM_MODE_ORDER "pat"
#define RRM_MODE_COUNT 3

enum rrm_beacon_rqst_mode {
    RRM_BEACON_RQST_MODE_PASSIVE,
    RRM_BEACON_RQST_MODE_ACTIVE,
    RRM_BEACON_RQST_MODE_BEACON_TABLE,
    __RRM_BEACON_RQST_MODE_MAX
};

/**
 * Calculate bitrate using the supported rates values.
 * @param supp_rate_val
 * @return the bitrate.
 */
double iee80211_calculate_bitrate(uint8_t supp_rate_val);

/**
 * Calculate expected throughput in Mbit/sec.
 * @param exp_thr
 * @return
 */
double iee80211_calculate_expected_throughput_mbit(int exp_thr);

/**
 * Convert 802.11k RCPI value to RSSI dB
 * @param rcpi
 * @return
 */
int rcpi_to_rssi(int rcpi);

/**
 * Convert mode string to array of RRM masks 
 * @param rrm_mode_order
 * @param mode_string
 * @return
 */
int parse_rrm_mode(int* rrm_mode_order, const char* mode_string);

#endif //DAWN_IEEE80211_UTILS_H
