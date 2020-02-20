//Structure definitions for wireless packets

#define MAX_HOSTS 257

#ifdef DEFINE_TYPES
typedef unsigned short u_short;
typedef unsigned char u_char;
typedef unsigned int u_int;
#endif

typedef enum {
	mgt_assocRequest = 0x00,
	mgt_assocResponse = 0x10,
	mgt_reassocRequest = 0x20,
	mgt_reassocResponse = 0x30,
	mgt_probeRequest = 0x40,
	mgt_probeResponse = 0x50,
	mgt_beacon = 0x80,
	mgt_atim = 0x90,
	mgt_disassoc = 0xa0,
	mgt_auth = 0xb0,
	mgt_deauth = 0xc0
} wifi_frametype;

typedef struct ieee802_11_hdr {
	u_char frame_control;
	u_char flags;
#define IEEE80211_TO_DS 0x01
#define IEEE80211_FROM_DS 0x02
#define IEEE80211_MORE_FRAG 0x04
#define IEEE80211_RETRY 0x08
#define IEEE80211_PWR_MGT 0x10
#define IEEE80211_MORE_DATA 0x20
#define IEEE80211_WEP_FLAG 0x40
#define IEEE80211_ORDER_FLAG 0x80
	u_short duration;
	u_char addr1[6];
	u_char addr2[6];
	u_char addr3[6];
	u_short frag_and_seq;
} __attribute__((packed)) ieee802_11_hdr;

typedef struct {
	u_char timestamp[8];
	u_short bcn_interval;
	u_short caps;
#define MGT_CAPS_AP 0x1
#define MGT_CAPS_IBSS 0x2
#define MGT_CAPS_WEP 0x10
} __attribute__((packed)) ieee_802_11_mgt_frame;

typedef struct {
	u_char tag;
	u_char length;
} __attribute__((packed)) ieee_802_11_tag;

typedef enum {
	tagSSID = 0,
	tagRates = 1,
	tagChannel = 3,
	tagRSN = 48,
	tagVendorSpecific = 0xDD
} i81tag;

typedef struct prism_hdr {
	u_int msg_code;
	u_int msg_length;
	char cap_device[16];
	//char dids[0];
} __attribute__((packed)) prism_hdr;

#ifdef HAVE_MADWIFI
typedef struct prism_did {
	u_int did;
	u_short status1;
	u_short length;
	u_int data;
	//int value[0];
} __attribute__((packed)) prism_did;

/*	DIDmsg_lnxind_wlansniffrm		= 0x00000044,
	DIDmsg_lnxind_wlansniffrm_hosttime	= 0x00010044,
	DIDmsg_lnxind_wlansniffrm_mactime	= 0x00020044,
	DIDmsg_lnxind_wlansniffrm_channel	= 0x00030044,
	DIDmsg_lnxind_wlansniffrm_rssi		= 0x00040044,
	DIDmsg_lnxind_wlansniffrm_sq		= 0x00050044,
	DIDmsg_lnxind_wlansniffrm_signal	= 0x00060044,
	DIDmsg_lnxind_wlansniffrm_noise		= 0x00070044,
	DIDmsg_lnxind_wlansniffrm_rate		= 0x00080044,
	DIDmsg_lnxind_wlansniffrm_istx		= 0x00090044,
	DIDmsg_lnxind_wlansniffrm_frmlen	= 0x000A0044
*/
typedef enum prism_did_num {
	pdn_host_time = 0x00010044,
	pdn_mac_time = 0x00020044,
	pdn_rssi = 0x00040044,
	pdn_sq = 0x00050044,
	pdn_signal = 0x00060044,
	pdn_datarate = 0x00080044,
	pdn_framelen = 0x000a0044
} prism_did_num;
#else

typedef struct prism_did {
	u_short did;
	u_short status1;
	u_short status2;
	u_short length;
	//int value[0];
} __attribute__((packed)) prism_did;

typedef enum prism_did_num {
	pdn_host_time = 0x1041,
	pdn_mac_time = 0x2041,
	pdn_rssi = 0x4041,
	pdn_sq = 0x5041,
	pdn_datarate = 0x8041,
	pdn_framelen = 0xa041
} prism_did_num;
#endif

//Structure definitions for data collection

typedef enum {
	typeUnknown,
	typeAP,
	typeWDS,
	typeSta,
	typeAdhocHub
} host_type;

typedef enum {
	ssUnknown,
	ssUnassociated,
	ssAssociated
} sta_state;


typedef struct {
	u_char bssid[6];
	char ssid[33];
	u_char ssidlen;
	u_char channel;
	u_short flags;
	int encryption;
} ap_info;

typedef struct {
	sta_state state;
	u_char connectedBSSID[6];
	char lastssid[33];
	u_char lastssidlen;
} sta_info;

typedef struct {
	u_char occupied;
	u_char mac[6];
	host_type type;
	time_t lastSeen;
	int RSSI;
	ap_info *apInfo;
	sta_info *staInfo;
	u_char isSelf;
} wiviz_host;

//Primary config struct
typedef struct {
	wiviz_host hosts[MAX_HOSTS];
	int numHosts;
	int readFromWl;
	time_t lastKeepAlive;
	int channelHopping;
	int channelDwellTime;
	int channelHopSeq[14];
	int channelHopSeqLen;
	int curChannel;
	int channelHopperPID;
} wiviz_cfg;

void fprint_mac(FILE * outf, u_char * mac, char *extra);
void print_mac(u_char * mac, char *extra);
