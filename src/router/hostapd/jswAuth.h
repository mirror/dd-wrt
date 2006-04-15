/*
 *  Copyright (c) 2004 Atheros Communications Inc., All rights reserved
 */
#ifndef _JSWAUTH_H
#define _JSWAUTH_H

/* XXX Divy. GDB Debug artifact */
#define STATIC static

struct jsw_secret {
	u8 PMK[32];
	u8 micKey[16];
	u8 KEK[16];
	u8 WEP40[5];
	u8 WEP104[13];
	u8 reserved[46];
};

#define JSW_SSID_SUFF_LEN   3

#define JSW_P1_DEF_SSID_PREF      "Jumpstart-P1-"
#define JSW_DEF_CHAN              2437
#define JSW_DEF_FREQ              MODE_SELECT_11G
#define JSW_P2_DEF_SSID_PREF      "JS-P2-"

/* XXX Divy. Make it configurable */
#define APCFG_JSW_DEF_LEGACY_TMO        (60*60*8) /* 8 hrs */

struct jsw_profile {
	u32			version;
	u8			passHash[JSW_SHA1_LEN];
	u8			salt[JSW_NONCE_SIZE];
	struct jsw_secret	secret;
	u8			p1_in_progress;
};

#define JSW_PRIME_BITS			1024
#define JSW_WR_BUF_SIZE			1500
#define JSW_RD_BUF_SIZE			JSW_WR_BUF_SIZE
#define JSW_MAX_KEYDATA_LEN		JSW_RD_BUF_SIZE
#define JSW_MIC_SIZE			16
#define JSW_MICKEY_SIZE			16
#define JSW_KEK_SIZE			16
#define JSW_DIGEST_SIZE			32
#define JSW_DEFAULT_TIMEOUT		2	/* 2 seconds */
#define JSW_DH_TIMEOUT			3
#define JSW_LINGER_TIMEOUT		3
#define JSW_PASSWORD_RESP_TIMEOUT	(5*60)	/* 5 minutes */
#define JSW_LAST_FRAME_RETRY		5

#define OFFSETOF(TYPE, MEMBER) ((size_t)&(((TYPE *)0)->MEMBER))

#define jswDoRetry(_jp)	((++(_jp)->sm.retries < JSW_MAX_RETRIES) ? 1 : 0)

#define jswEapol2Key(_eapol)	(struct wpa_eapol_key *)((u8 *)(_eapol)  + \
				 sizeof(struct ieee802_1x_hdr)		 + \
				 OFFSETOF(struct eap_vp_header, data)	 + \
				 sizeof(struct ath_eap_header)		 + \
				 sizeof(struct jsw_op_hdr))

#define jswMinLen()		(jswEapolLen(0) 			+ \
				 sizeof(struct ieee802_1x_hdr))


#define jswEapolLen(_keyDataLen) (jswVpLen(_keyDataLen)			+ \
				  OFFSETOF(struct eap_vp_header, type))


#define jswVpLen(_keyDataLen)	(sizeof(struct eap_vp_header) - 1 -	  \
				 OFFSETOF(struct eap_vp_header, type)	+ \
				 sizeof(struct ath_eap_header)		+ \
				 sizeof(struct jsw_op_hdr)		+ \
				 jswOpLen(_keyDataLen))


#define jswOpLen(_keyDataLen)	(sizeof(struct wpa_eapol_key) 		+ \
				 (_keyDataLen))

#define jswTlvIsWpa(_tlv)	(((_tlv)->oui[0] == 0x00)		&& \
 				 ((_tlv)->oui[1] == 0x50)		&& \
				 ((_tlv)->oui[2] == 0xF2))

#define jswTlvIsAth(_tlv)						   \
	(!memcmp(ouiAtheros, (_tlv)->oui, sizeof(ouiAtheros)))

#define jswGetReplayCtr(_js, _eap)					   \
	(memcpy((_js)->key_replay_counter, 				   \
		(jswEapol2Key(_eap))->replay_counter,			   \
		sizeof((_js)->key_replay_counter)))

/*
 * state machine
 */

typedef void (*SM_CBFN)(void *arg, int event);
struct js_core_sm {
	SM_CBFN   state;
	int       retries;
	void      *event_arg;
};

#define smSendEvent(_js, _event) do {					\
	(_js)->sm.state((_js), (_event));		\
} while (0)

#define smSetState(_js, _state) do {					\
	(_js)->sm.state    = (SM_CBFN)(_state);				\
	(_js)->sm.retries  = 0; 					\
} while (0)

#define smGetState(_js) ((_js)->sm.state)

typedef enum {
	JSW_EVENT_ASSOC 	= 1,
	JSW_EVENT_JDS_REQ,
	JSW_EVENT_JDS_ACK,
	JSW_EVENT_JSS_PW_RESP,
	JSW_EVENT_JSS_PW_ACK,
	JSW_EVENT_TIMEOUT,
	JSW_EVENT_DISASSOC,
	JSW_EVENT_JDS_DISCOVER,
	JSW_EVENT_JSS_PMK_ACK,
	JSW_EVENT_PORT_ENABLED,
	JSW_EVENT_PORT_DISABLED,
	JSW_EVENT_DISABLED
} jsw_event_t;

#define JSW_MAX_LED_STATES 10

struct jsw_led_state {
	u8 val;
	u8 duration_factor;
};

struct jsw_led_pattern {
	u8 			pattern_len;
	struct jsw_led_state 	state[JSW_MAX_LED_STATES];
};

struct jsw_led {
	int 			led_pattern_index;
	struct jsw_led_pattern 	*led_pattern;
};

struct jsw_session {
	struct hostapd_data 	*hapd;
	struct sta_info 	*sta;
	struct js_core_sm	sm;
	int			timeout;
	u8			key_replay_counter[WPA_REPLAY_COUNTER_LEN];
	u8 			*staCaps;
	BIGNUM			staPubKey;
	u8			nonce[JSW_NONCE_SIZE];
	DH			*dh;
	struct jsw_secret	secret;
	u8			buf[JSW_WR_BUF_SIZE];
};

struct jsw_ieset {
	struct key_data	*pubKey;
	struct key_data	*passHash;
	struct key_data	*salt;
	u8 *wpaIe;
	u8 *wpa2Ie;
};

#define JSW_MAX_TLV_VALS 2 /* max value an opcode can expect - hacky? */
struct jsw_tlv {
	JSW_FRAME_OPS	op;
	int		cnt;
	int		val[JSW_MAX_TLV_VALS];
};

/*
 * jswAuth.c
 */
int jsw_init(struct hostapd_data *hapd);
void jsw_deinit(struct hostapd_data *hapd);
void js_free_station(struct sta_info *sta);
void js_p1_new_station(struct hostapd_data *hapd, struct sta_info *sta);
void js_p2_new_station(struct hostapd_data *hapd, struct sta_info *sta);
void jswP2GetMicKey(struct jsw_session *js, u8 *micKey);
void jswP2GetMic2Key(struct jsw_session *js, u8 *micKey);
void js_create_ssid(struct hostapd_data *hapd, const char *pref);
void jsw_create_psk_conf(struct hostapd_data *hapd, struct jsw_session *js);
void jsw_led_start(struct hostapd_data *hapd);
void jsw_led_stop(struct hostapd_data *hapd);
void jsw_set_led_pattern(struct hostapd_data *hapd,
			 struct jsw_led_pattern *led);
/*
 * jswAuthSm.c
 */
void jswP2Start(struct jsw_session *js, jsw_event_t event);
void jswP1Start(struct jsw_session *js, jsw_event_t event);
int jswIsPassPending(struct jsw_session *js);
void jswTimerStart(struct jsw_session *js, int timer);
void jswTimerStop(struct jsw_session *js);
/*
 * jswMsg.c
 */
int jswSendPMKAck(struct jsw_session *js);
int jswSendPMK(struct jsw_session *js);
int jswP2SendJdsOffer(struct jsw_session *js);
int jswSendJdsPassAck(struct jsw_session *js);
int jswSendJdsAck(struct jsw_session *js);
int jswP1SendJdsOffer(struct jsw_session *js);
int jswGetTlvs(struct jsw_session *js, struct ieee802_1x_hdr *eapolHdr,
	       JSW_FRAME_OPS op);
int jswSendPassReq(struct jsw_session *js);
void jswDumpFrame(u8 *buf);
void jswDumpBuffer(u8 *buf, int len);
int js_claim_frame(struct hostapd_data *hapd, struct sta_info *sta,
		   u8 *data, size_t data_len);

#endif /* _JSW_AUTH_H */
