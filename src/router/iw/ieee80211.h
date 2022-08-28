#ifndef __IEEE80211
#define __IEEE80211

/* 802.11n HT capability AMPDU settings (for ampdu_params_info) */
#define IEEE80211_HT_AMPDU_PARM_FACTOR          0x03
#define IEEE80211_HT_AMPDU_PARM_DENSITY         0x1C

#define IEEE80211_HT_CAP_SUP_WIDTH_20_40        0x0002
#define IEEE80211_HT_CAP_SGI_40                 0x0040
#define IEEE80211_HT_CAP_MAX_AMSDU              0x0800

#define IEEE80211_HT_MCS_MASK_LEN               10

/**
 * struct ieee80211_mcs_info - MCS information
 * @rx_mask: RX mask
 * @rx_highest: highest supported RX rate. If set represents
 *      the highest supported RX data rate in units of 1 Mbps.
 *      If this field is 0 this value should not be used to
 *      consider the highest RX data rate supported.
 * @tx_params: TX parameters
 */
struct ieee80211_mcs_info {
	__u8 rx_mask[IEEE80211_HT_MCS_MASK_LEN];
	__u16 rx_highest;
	__u8 tx_params;
	__u8 reserved[3];
} __attribute__ ((packed));


/**
 * struct ieee80211_ht_cap - HT capabilities
 *
 * This structure is the "HT capabilities element" as
 * described in 802.11n D5.0 7.3.2.57
 */
struct ieee80211_ht_cap {
	__u16 cap_info;
	__u8 ampdu_params_info;

	/* 16 bytes MCS information */
	struct ieee80211_mcs_info mcs;

	__u16 extended_ht_cap_info;
	__u32 tx_BF_cap_info;
	__u8 antenna_selection_info;
} __attribute__ ((packed));

struct ieee80211_vht_mcs_info {
	__u16 rx_vht_mcs;
	__u16 rx_highest;
	__u16 tx_vht_mcs;
	__u16 tx_highest;
} __attribute__ ((packed));

struct ieee80211_vht_cap {
	__u32 cap_info;
	struct ieee80211_vht_mcs_info mcs;
} __attribute__ ((packed));

#define SUITE(oui, id)  (((oui) << 8) | (id))

/* cipher suite selectors */
#define WLAN_CIPHER_SUITE_USE_GROUP     SUITE(0x000FAC, 0)
#define WLAN_CIPHER_SUITE_WEP40         SUITE(0x000FAC, 1)
#define WLAN_CIPHER_SUITE_TKIP          SUITE(0x000FAC, 2)
/* reserved:                            SUITE(0x000FAC, 3) */
#define WLAN_CIPHER_SUITE_CCMP          SUITE(0x000FAC, 4)
#define WLAN_CIPHER_SUITE_WEP104        SUITE(0x000FAC, 5)
#define WLAN_CIPHER_SUITE_AES_CMAC      SUITE(0x000FAC, 6)
#define WLAN_CIPHER_SUITE_GCMP          SUITE(0x000FAC, 8)
#define WLAN_CIPHER_SUITE_GCMP_256      SUITE(0x000FAC, 9)
#define WLAN_CIPHER_SUITE_CCMP_256      SUITE(0x000FAC, 10)
#define WLAN_CIPHER_SUITE_BIP_GMAC_128  SUITE(0x000FAC, 11)
#define WLAN_CIPHER_SUITE_BIP_GMAC_256  SUITE(0x000FAC, 12)
#define WLAN_CIPHER_SUITE_BIP_CMAC_256  SUITE(0x000FAC, 13)

#endif /* __IEEE80211 */
