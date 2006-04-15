/*
 *  Copyright (c) 2004 Atheros Communications Inc.  All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <signal.h>

#include "eloop.h"
#include "hostapd.h"
#include "ieee802_1x.h"
#include "ieee802_11.h"
#include "accounting.h"
#include "eapol_sm.h"
#include "iapp.h"
#include "ap.h"
#include "sta_info.h"
#include "driver.h"
#include "radius_client.h"
#include "radius_server.h"
#include "wpa.h"
#include "ctrl_iface.h"
#include "tls.h"
#include "eap_sim_db.h"
#include "version.h"
#include "sha1.h"

#include "openssl/bn.h"
#include "openssl/dh.h"
#include "openssl/rand.h"
#include "eap_js.h"
#include "jswproto.h"
#include "jswAuth.h"

STATIC void jswP1ReqPending(struct jsw_session  *js, jsw_event_t event);
STATIC void jswP1JdsAckPending(struct jsw_session  *js, jsw_event_t event);
STATIC void jswP1PassPending(struct jsw_session  *js, jsw_event_t event);

STATIC void jswP2ReqPending(struct jsw_session  *js, jsw_event_t event);
STATIC void jswP2PmkAckPending(struct jsw_session  *js, jsw_event_t event);
STATIC void jswP2DonePending(struct jsw_session  *js, jsw_event_t event);

STATIC void jswP1Failed(struct jsw_session  *js, jsw_event_t event);
STATIC void jswP1Successful(struct jsw_session  *js);
STATIC void jswP2Failed(struct jsw_session  *js, jsw_event_t event);

STATIC int jswDeriveSecret(struct jsw_session  *js);
STATIC int jswP2HandleJdsReq(struct jsw_session  *js,
				 struct ieee802_1x_hdr *eapolHdr);
STATIC int jswP2HandlePMKAck(struct jsw_session  *js,
				 struct ieee802_1x_hdr *eapolHdr);
STATIC int jswHandleJdsAck(struct jsw_session *js,
				 struct ieee802_1x_hdr *eapolHdr);

STATIC int jswVerifyMic(struct jsw_session *js,
			struct ieee802_1x_hdr *hdr,
			u8 micKey[]);


struct jsw_led_pattern jsw_led_ready =
	{2,
		{
			{1, 2},
			{0, 2}
		}
	};

struct jsw_led_pattern jsw_led_success =
	{7,
		{
			{1, 6},
			{0, 6},
			{1, 6},
			{0, 6},
			{1, 6},
			{0, 6},
			{1, 1}
		}
	};

void
jswP1Start(struct jsw_session *js, jsw_event_t event)
{
	jswP1SendJdsOffer(js);
	jswTimerStart(js, JSW_DEFAULT_TIMEOUT);
	smSetState(js, jswP1ReqPending);
	return;
}

STATIC void
jswP1ReqPending(struct jsw_session *js, jsw_event_t event)
{
	struct ieee802_1x_hdr *eapolHdr;

	wpa_printf(MSG_DEBUG, "state jswP1ReqPending: event %d\n", event);
	switch (event) {
	case JSW_EVENT_JDS_REQ:
		eapolHdr = js->sm.event_arg;

		/* XXX Divy. Need to code this */
		/* Found a JS guy, so turn off legacy failover timer */

		if (jswGetTlvs(js, eapolHdr, JSW_OP_JDS_REQUEST) != 0) {
			goto failed;
		}
		if (jswDeriveSecret(js) != 0) {
			goto failed;
		}

		inc_byte_array(js->key_replay_counter, WPA_REPLAY_COUNTER_LEN);
		if ((jswSendJdsAck(js)) != 0) {
			goto failed;
		}
		jswTimerStart(js, JSW_DH_TIMEOUT);
		smSetState(js, jswP1JdsAckPending);
        	break;

	case JSW_EVENT_TIMEOUT:
		if (!jswDoRetry(js)) {
			goto failed;
		}

		inc_byte_array(js->key_replay_counter, WPA_REPLAY_COUNTER_LEN);
		jswTimerStart(js, JSW_DEFAULT_TIMEOUT);
		if ((jswP1SendJdsOffer(js)) != 0) {
			goto failed;
		}
		break;

	default:
		goto failed;
	}
	return;

failed:
	jswP1Failed(js, event);
}

STATIC void
jswP1JdsAckPending(struct jsw_session *js, jsw_event_t event)
{
	struct ieee802_1x_hdr *eapolHdr;

	wpa_printf(MSG_DEBUG, "state jswP1JdsAckPending: event %d\n", event);
	switch (event) {
	case JSW_EVENT_JDS_ACK:
		eapolHdr = js->sm.event_arg;
		if ((jswHandleJdsAck(js, eapolHdr)) != 0) {
			goto failed;
		}
		smSetState(js, jswP1PassPending);
		jsw_led_stop(js->hapd);
		jsw_set_led_pattern(js->hapd, &jsw_led_success);
		break;

	case JSW_EVENT_TIMEOUT:
		if (!jswDoRetry(js)) {
			goto failed;
		}

		inc_byte_array(js->key_replay_counter, WPA_REPLAY_COUNTER_LEN);
		if ((jswSendJdsAck(js)) != 0) {
			goto failed;
		}
		jswTimerStart(js, JSW_DH_TIMEOUT);
		break;

	default:
		goto failed;
	}
	return;

failed:
	jswP1Failed(js, event);
}

STATIC void
jswP1PassPending(struct jsw_session *js, jsw_event_t event)
{
	struct ieee802_1x_hdr *eapolHdr;
	int i;

	wpa_printf(MSG_DEBUG, "state jswP1PassPending: event %d\n", event);
	switch (event) {
	case JSW_EVENT_JSS_PW_RESP:
		eapolHdr = js->sm.event_arg;

		if (jswGetTlvs(js, eapolHdr, JSW_OP_JSS_PASSWORD_RESP) != 0) {
			goto failed;
		}

		inc_byte_array(js->key_replay_counter, WPA_REPLAY_COUNTER_LEN);
		/*
		 * Just send a burst and hope that at least one
		 * makes it to the station.
		 */
		if ((jswSendJdsPassAck(js)) != 0) {
			goto failed;
		}
		for( i = 0; i < JSW_LAST_FRAME_RETRY; i++) {
			jswSendJdsPassAck(js);
		}
		jswP1Successful(js);
		break;

	case JSW_EVENT_JDS_ACK:
		eapolHdr = js->sm.event_arg;

		if (!jswDoRetry(js)) {
			goto failed;
        	}
		if ((jswHandleJdsAck(js, eapolHdr)) != 0) {
			goto failed;
		}
		break;

		default:
			goto failed;
	}
	return;

failed:
	jswP1Failed(js, event);
}

/*
 * Protocol 2 sm
 */
void
jswP2Start(struct jsw_session *js, jsw_event_t event)
{
	struct ieee802_1x_hdr *eapolHdr = js->sm.event_arg;
	struct wpa_eapol_key *p;
	wpa_printf(MSG_DEBUG, "starting P2\n");

	p = jswEapol2Key(eapolHdr);

	/* XXX Divy. Destroy started wpa state machine */

	jswGetReplayCtr(js, eapolHdr);
	if ((jswP2SendJdsOffer(js)) != 0) {
		jswP2Failed(js, event);
	}

	jswTimerStart(js, JSW_DEFAULT_TIMEOUT);
	smSetState(js, jswP2ReqPending);
}

STATIC void
jswP2ReqPending(struct jsw_session *js, jsw_event_t event)
{
	struct ieee802_1x_hdr *eapolHdr;

	wpa_printf(MSG_DEBUG, "state jswP2ReqPending: event %d\n", event);
	switch (event) {
	case JSW_EVENT_JDS_REQ:
		eapolHdr = js->sm.event_arg;

		if ((jswP2HandleJdsReq(js, eapolHdr)) != 0) {
			goto failed;
		}
		smSetState(js, jswP2PmkAckPending);
		break;

	case JSW_EVENT_JDS_DISCOVER:
		eapolHdr = js->sm.event_arg;

		if (!jswDoRetry(js)) {
			goto failed;
		}

		jswGetReplayCtr(js, eapolHdr);
		if ((jswP2SendJdsOffer(js)) != 0) {
			goto failed;
		}
		jswTimerStart(js, JSW_DH_TIMEOUT);
		break;

	case JSW_EVENT_TIMEOUT:
		if (!jswDoRetry(js)) {
			goto failed;
		}
		if ((jswP2SendJdsOffer(js)) != 0) {
			goto failed;
		}
		jswTimerStart(js, JSW_DH_TIMEOUT);
		break;

	default:
		goto failed;
    }
	return;
failed:
	jswP2Failed(js, event);
}

STATIC void
jswP2PmkAckPending(struct jsw_session *js, jsw_event_t event)
{
	struct ieee802_1x_hdr *eapolHdr;

	wpa_printf(MSG_DEBUG, "state jswP2PmkAckPending: event %d\n", event);
	switch (event) {
	case JSW_EVENT_JSS_PMK_ACK:
		eapolHdr = js->sm.event_arg;

		if ((jswP2HandlePMKAck(js, eapolHdr)) != 0) {
			goto failed;
		}

		jswTimerStart(js, JSW_LINGER_TIMEOUT);
		smSetState(js, jswP2DonePending);
		break;

		case JSW_EVENT_JDS_REQ:
		eapolHdr = js->sm.event_arg;

		if (!jswDoRetry(js)) {
			goto failed;
		}
		if ((jswP2HandleJdsReq(js, eapolHdr)) != 0) {
			goto failed;
		}
		break;

	case JSW_EVENT_TIMEOUT:
		if (!jswDoRetry(js)) {
			goto failed;
		}
		if ((jswSendPMK(js)) != 0) {
			goto failed;
		}
		jswTimerStart(js, JSW_DEFAULT_TIMEOUT);
		break;

		default:
		goto failed;
	}

	return;
failed:
	jswP2Failed(js, event);
}

STATIC void
jswP2DonePending(struct jsw_session *js, jsw_event_t event)
{
	struct ieee802_1x_hdr *eapolHdr;
	struct sta_info *sta = js->sta;

	wpa_printf(MSG_DEBUG, "state jswP2DonePending: event %d\n", event);
	switch(event) {
	case JSW_EVENT_JSS_PMK_ACK:
		eapolHdr = js->sm.event_arg;

		if (!jswDoRetry(js)) {
			goto failed;
		}

		if ((jswP2HandlePMKAck(js, eapolHdr)) != 0) {
			goto failed;
		}
		break;

	case JSW_EVENT_TIMEOUT:
		hostapd_logger(js->hapd, js->sta->addr, HOSTAPD_MODULE_JS,
		       		HOSTAPD_LEVEL_WARNING, "JS P2 succeded");
		js_free_station(sta);
		break;
	default:
		goto failed;
	}
	return;

failed:
	jswP2Failed(js, event);
}

STATIC void
jswP1Failed(struct jsw_session *js, jsw_event_t event)
{
	struct hostapd_data *hapd = js->hapd;
	struct sta_info *sta = js->sta;

	hostapd_logger(js->hapd, js->sta->addr, HOSTAPD_MODULE_JS,
		       HOSTAPD_LEVEL_WARNING, "JS P1 failed");
	hapd->jsw_profile->p1_in_progress = 0;
	jsw_led_stop(js->hapd);
	jsw_set_led_pattern(hapd, &jsw_led_ready);
	js_free_station(sta);
}

STATIC void
jswP2Failed(struct jsw_session *js, jsw_event_t event)
{
	struct hostapd_data *hapd = js->hapd;
	struct sta_info *sta = js->sta;

	hostapd_logger(js->hapd, js->sta->addr, HOSTAPD_MODULE_JS,
		       HOSTAPD_LEVEL_WARNING, "JS P2 failed");
	hostapd_sta_deauth(hapd, sta->addr,
			     WLAN_REASON_UNSPECIFIED);
	js_free_station(sta);
}

STATIC void
jswP1Successful(struct jsw_session *js)
{
	struct hostapd_data *hapd;
	struct sta_info *sta;
	struct jsw_profile *jp;
	pid_t pid;

	hostapd_logger(js->hapd, js->sta->addr, HOSTAPD_MODULE_JS,
		       HOSTAPD_LEVEL_WARNING, "JS P1 succeeded");

	hapd = js->hapd;
	sta = js->sta;
	jp = hapd->jsw_profile;

	 /* This session won - this is the final secret. Store it. */
	memcpy((u8 *)&jp->secret, (u8 *)&js->secret,
		sizeof(js->secret));
	jsw_led_stop(hapd);

	jsw_create_psk_conf(hapd, js);
	hostapd_logger(js->hapd, js->sta->addr, HOSTAPD_MODULE_JS,
		       HOSTAPD_LEVEL_WARNING, "hostapd is about to reload");
	js_free_station(sta);

	pid = getpid();
	kill(pid, SIGHUP);
}

STATIC int
jswDeriveSecret(struct jsw_session *js)
{
	u8 *buf;
	int blen, bout;

	blen = DH_size(js->dh);
	buf = (unsigned char *)OPENSSL_malloc(blen);
	bout = DH_compute_key(buf, &js->staPubKey, js->dh);

	if (bout < sizeof(js->secret)) {
		OPENSSL_free(buf);
        	wpa_printf(MSG_DEBUG, "secret length %d...erroring out\n",
			   bout);
        	return -1;
	}

	memcpy((u8 *)&js->secret, buf, sizeof(js->secret));

	OPENSSL_free(buf);
	return 0;
}

STATIC int
jswP2HandleJdsReq(struct jsw_session *js, struct ieee802_1x_hdr *eapolHdr)
{
	u8 micKey[JSW_SHA1_LEN];

	jswP2GetMicKey(js, micKey);
	if ((jswVerifyMic(js, eapolHdr, micKey)) != 0) {
		return -1;
	}
	if (jswGetTlvs(js, eapolHdr, JSW_OP_JDS_REQUEST) != 0) {
		return -1;
	}
	if ((jswDeriveSecret(js)) != 0) {
		return -1;
	}
	jswGetReplayCtr(js, eapolHdr);
	jswTimerStart(js, JSW_DEFAULT_TIMEOUT);

	return(jswSendPMK(js));
}

STATIC int
jswP2HandlePMKAck(struct jsw_session *js, struct ieee802_1x_hdr *eapolHdr)
{
	u8 micKey[JSW_SHA1_LEN];

	jswP2GetMic2Key(js, micKey);
	if ((jswVerifyMic(js, eapolHdr, micKey)) != 0) {
		return 0;
	}
	if (jswGetTlvs(js, eapolHdr, JSW_OP_JSS_PMK_ACK) != 0) {
	return -1;
	}

	jswGetReplayCtr(js, eapolHdr);
	jswTimerStart(js, JSW_DEFAULT_TIMEOUT);

	return(jswSendPMKAck(js));
}

STATIC int
jswVerifyMic(struct jsw_session *js, struct ieee802_1x_hdr *eapolHdr,
	     u8 *micKey)
{
	struct wpa_eapol_key *hdr = jswEapol2Key(eapolHdr);
	u8 digest[JSW_DIGEST_SIZE], tmpMic[JSW_MIC_SIZE];
	u16 to_mic_len;

	memcpy(&tmpMic, hdr->key_mic, sizeof(tmpMic));
	memset(hdr->key_mic, 0, sizeof(hdr->key_mic));
	to_mic_len = ntohs(eapolHdr->length);

	hmac_sha1(micKey, JSW_MICKEY_SIZE,
		  (u8 *)eapolHdr, to_mic_len,
		  digest);

	if (memcmp(digest, tmpMic, sizeof(tmpMic))) {
		wpa_printf(MSG_DEBUG, "Bad Mic\nExpected: ");
		jswDumpBuffer(digest, JSW_MIC_SIZE);
		wpa_printf(MSG_DEBUG, "Gotten: ");
		jswDumpBuffer(tmpMic, JSW_MIC_SIZE);
		return -1;
	}

	return 0;
}

STATIC int
jswHandleJdsAck(struct jsw_session *js, struct ieee802_1x_hdr *eapolHdr)
{
	if ((jswVerifyMic(js, eapolHdr, js->secret.micKey)) != 0) {
		return -1;
	}
	if (jswGetTlvs(js, eapolHdr, JSW_OP_JDS_ACK) != 0) {
		return -1;
	}

	inc_byte_array(js->key_replay_counter, WPA_REPLAY_COUNTER_LEN);
	jswTimerStart(js, JSW_PASSWORD_RESP_TIMEOUT);

	return(jswSendPassReq(js));
}

int
jswIsPassPending(struct jsw_session *js)
{
	return((smGetState(js) == (SM_CBFN)jswP1PassPending));
}
