#ifndef DAWN_RSSI_H
#define DAWN_RSSI_H

#include <stddef.h>
#include <stdint.h>

#include "mac_utils.h"

// ---------------- Global variables ----------------
#define HOSTAPD_DIR_LEN 200
extern char hostapd_dir_glob[];

/**
 * Get RSSI using the mac adress of the client.
 * Function uses libiwinfo and searches through all interfaces that are existing.
 * @param client_addr - mac adress of the client
 * @return The RSSI of the client if successful. INT_MIN if client was not found.
 */
int get_rssi_iwinfo(struct dawn_mac client_addr);

/**
 * Get expected throughut using the mac adress of the client.
 * Function uses libiwinfo and searches through all interfaces that are existing.
 * @param client_addr - mac adress of the client
 * @return
 * + The expected throughput of the client if successful.
 * + INT_MIN if client was not found.
 * + 0 if the client is not supporting this feature.
 */
int get_expected_throughput_iwinfo(struct dawn_mac client_addr);

/**
 * Get rx and tx bandwidth using the mac of the client.
 * Function uses libiwinfo and searches through all interfaces that are existing.
 * @param client_addr - mac adress of the client
 * @param rx_rate - float pointer for returning the rx rate
 * @param tx_rate - float pointer for returning the tx rate
 * @return 0 if successful 1 otherwise.
 */
int get_bandwidth_iwinfo(struct dawn_mac client_addr, float *rx_rate, float *tx_rate);

/**
 * Function checks if two bssid adresses have the same essid.
 * Function uses libiwinfo and searches through all interfaces that are existing.
 * @param bssid_addr
 * @param bssid_addr_to_compares
 * @return 1 if the bssid adresses have the same essid.
 */
int compare_essid_iwinfo(struct dawn_mac bssid_addr, struct dawn_mac bssid_addr_to_compare);

/**
 * Function returns the expected throughput using the interface and the client address.
 * @param ifname
 * @param client_addr
 * @return
 * + The expected throughput of the client if successful.
 * + INT_MIN if client was not found.
 * + 0 if the client is not supporting this feature.
 */
int get_expected_throughput(const char *ifname, struct dawn_mac client_addr);

int get_bssid(const char *ifname, uint8_t bssid_addr[]);

int get_ssid(const char *ifname, char *ssid, size_t ssidmax);

int get_channel_utilization(const char *ifname, uint64_t *last_channel_time, uint64_t *last_channel_time_busy);

int support_ht(const char *ifname);

int support_vht(const char *ifname);

#endif //DAWN_RSSI_H
