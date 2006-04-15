/* 
 *  Copyright 2005, Atheros Communications Inc.  All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h> 

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
#include "aes_wrap.h"
#include "sha1.h"

#include "openssl/bn.h"
#include "openssl/dh.h"
#include "openssl/sha.h"
#include "openssl/rand.h"
#include "eap_js.h"
#include "jswproto.h"
#include "jswAuth.h"

STATIC int jswMsgOk(int len, struct ieee802_1x_hdr *eapolHdr, 
	 	    struct eap_vp_header *vpHdr,
	 	    struct ath_eap_header *jswProtoHdr, 
	 	    struct wpa_eapol_key *eapolKeyHdr);
STATIC int jswBuildCommon(u8 *buf, int bufLen, JSW_FRAME_OPS op, u8 *replayCtr,
			  const u8 nonce[], int ack, int mic, u8 micKey[],
		  	  int encrypt, u8 KEK[], int keyDataLen, u8 *keyData);
STATIC int jswSend(struct jsw_session *js, u8 *buf, int len);
STATIC int jswBuildDhTlv(u8 *buf, BIGNUM *bn, u8 type);
STATIC struct jsw_tlv *jswGetTlvProfile(JSW_FRAME_OPS);
STATIC int jswBuildTlv(u8 *to, u8 *from, u8 dataLen, u8 type);
STATIC int jswIsReplayCtrOk(struct jsw_session *js, 
			    struct ieee802_1x_hdr *eapolHdr,
			    JSW_FRAME_OPS op);

STATIC int jswParseTlvs(struct jsw_session *js, struct jsw_ieset *ieSet,
			JSW_FRAME_OPS op);
STATIC void jswDecodePacket(struct hostapd_data *hapd, struct sta_info *sta,
			    u8 *data, size_t data_len);

STATIC int jswBuildCapsChoiceTlv(struct jsw_session *js, u8 *to);
STATIC int jswBuildCapsMenuTlv  (struct jsw_session *js, u8 *to);

const u8 ouiAtheros[3]   = { 0x00, 0x03, 0x7f };
const u8 eapSmiOID[3]    = { 0x00, 0x24, 0xE2 };

/*
 * For TLV verification
 */
#define JSW_TLV_WPA		0x00000001
#define JSW_TLV_WPA2		0x00010000
#define JSW_TLV_PUB_KEY		0x00000010
#define JSW_TLV_SHA1_PASSWORD	0x00000100
#define JSW_TLV_SALT		0x00001000

#define MAX_ETHER_BODY_SIZE     1500

STATIC struct jsw_tlv gTlv[] =
{
	{JSW_OP_JDS_REQUEST, 2, {JSW_TLV_WPA | JSW_TLV_PUB_KEY,
				 JSW_TLV_WPA2 | JSW_TLV_PUB_KEY}},
	{JSW_OP_JDS_ACK, 1, {JSW_TLV_WPA | JSW_TLV_WPA2}},
	{JSW_OP_JSS_PASSWORD_RESP, 1, {JSW_TLV_SHA1_PASSWORD | JSW_TLV_SALT}},
	{JSW_OP_JSS_PMK_ACK, 1, {JSW_TLV_WPA | JSW_TLV_WPA2}},
};

/*
 * Frame building/sending functions
 */
STATIC int
jswBuildCommon(u8 *buf, int bufLen, JSW_FRAME_OPS op, u8 *replayCtr,
	       const u8 nonce[], int ack, int mic, u8 micKey[], int encrypt,
	       u8 KEK[], int keyDataLen, u8 *keyData)
{

	struct ieee802_1x_hdr *eapolHdr;
	struct eap_vp_header *vpHdr;
	struct ath_eap_header *jswProtoHdr;
	struct jsw_op_hdr *jswOpHdr;
	struct wpa_eapol_key *eapolKeyHdr;
	u16 key_info = 0;
	u8 digest[JSW_DIGEST_SIZE];
	int eapolLen, pad;
	u8 *pData;
	
	eapolHdr = (struct ieee802_1x_hdr *)buf;
	vpHdr = (struct eap_vp_header *)(eapolHdr + 1);
	jswProtoHdr = (struct ath_eap_header *)(vpHdr->data);
	jswOpHdr = (struct jsw_op_hdr *)(jswProtoHdr + 1);
	eapolKeyHdr = (struct wpa_eapol_key *)(jswOpHdr + 1);
	pData = (u8 *)(eapolKeyHdr + 1);
	/*
	 * eapol Key
	 */
	memset(eapolKeyHdr, 0, sizeof(struct wpa_eapol_key));
	eapolKeyHdr->type = JSW_KEY_DESC;
	memcpy(eapolKeyHdr->replay_counter, replayCtr, 
	       WPA_REPLAY_COUNTER_LEN);
	       
	key_info |= JSW_KEY_INFO_TYPE;      
	if (encrypt)
		key_info |= WPA_KEY_INFO_SECURE;
	if (mic)
		key_info |= WPA_KEY_INFO_MIC;
	if (ack)
		key_info |= WPA_KEY_INFO_ACK;
	eapolKeyHdr->key_info = htons(key_info);

	if (nonce) {
		memcpy(eapolKeyHdr->key_nonce, nonce,
		       sizeof(eapolKeyHdr->key_nonce));
	}
	
	if (encrypt) {
		pad = keyDataLen % 8;
		if (pad)
			pad = 8 - pad;
		keyDataLen += pad + 8;
		if (bufLen < keyDataLen) {
			wpa_printf(MSG_DEBUG, "Not enough space for roundup\n");
			return -1;
		}
		aes_wrap(KEK, (keyDataLen - 8) / 8, keyData,
				 pData);
	} else {
		memcpy(pData, keyData, keyDataLen);
	}
	eapolKeyHdr->key_data_length = htons(keyDataLen);
	
	/*
	 * JSW hdrs
	 */
	jswOpHdr->op = htons(op);
	jswOpHdr->length = htons(jswOpLen(keyDataLen));

	jswProtoHdr->proto     = ATHL2P_JUMPSTART_PROTO;
	jswProtoHdr->version   = JSW_VERSION;
        *(u32 *)jswProtoHdr    = htonl(*(u32 *)jswProtoHdr);

	/*
	 * Vendor Private Hdr
	 */
	vpHdr->code = 0;
	vpHdr->identifier = 0;
	vpHdr->length = htons(jswVpLen(keyDataLen));
	vpHdr->type = JSW_EAP_VP_TYPE;
	memcpy(vpHdr->smiOID, eapSmiOID, sizeof(vpHdr->smiOID));

	/*
	 * EAPOL hdr
	 */
	eapolLen = jswEapolLen(keyDataLen);
	eapolHdr->version = EAPOL_VERSION;
	eapolHdr->type = IEEE802_1X_TYPE_EAP_PACKET;
	eapolHdr->length = htons(eapolLen);

	if (mic) {
		memset(digest, 0, sizeof(digest));
		hmac_sha1(micKey, JSW_MICKEY_SIZE,
			  (u8 *)eapolHdr, eapolLen, 
			  digest);
		memcpy(eapolKeyHdr->key_mic, digest, 
			sizeof(eapolKeyHdr->key_mic));
	}

	return (eapolLen + sizeof(struct ieee802_1x_hdr));
}

STATIC int
jswSend(struct jsw_session *js, u8 *buf, int len)
{
	struct hostapd_data *hapd;
	struct sta_info *sta;
	
	hapd = js->hapd;
	sta = js->sta;	   
	hostapd_send_eapol(hapd, sta->addr, buf, len, 0);
	return 0;
}

int
jswP1SendJdsOffer(struct jsw_session *js)
{
	int len, kdeLen = 0;
	u8 tlv[JSW_MAX_KEYDATA_LEN];

	kdeLen += jswBuildCapsMenuTlv(js, tlv);
	kdeLen += jswBuildDhTlv((tlv + kdeLen), js->dh->g, KEYDATA_DH_PARAM_G);
	kdeLen += jswBuildDhTlv((tlv + kdeLen), js->dh->p, KEYDATA_DH_PARAM_P);

	if ((len = jswBuildCommon(js->buf,
				  MAX_ETHER_BODY_SIZE,
				  JSW_OP_JDS_OFFER,
				  js->key_replay_counter,
				  NULL,                     /* nonce */
				  1,                        /* ack */
				  0,                        /* mic */
				  NULL,
				  0,                        /* encrypt */
				  NULL,
				  kdeLen,
				  tlv)) == -1)
	{
        	return -1;
	}

	return(jswSend(js, js->buf, len));
}

int
jswSendJdsAck(struct jsw_session *js)
{
	int len, kdeLen = 0;
	u8 tlv[JSW_MAX_KEYDATA_LEN];

	kdeLen += jswBuildCapsChoiceTlv(js, tlv);
	kdeLen += jswBuildDhTlv((tlv+ kdeLen), js->dh->pub_key, 
				KEYDATA_DH_PUB_KEY);

	if ((len = jswBuildCommon(js->buf,
				  MAX_ETHER_BODY_SIZE,
				  JSW_OP_JDS_ACK,
				  js->key_replay_counter,
				  js->nonce,
				  1,                        /* ack */
				  1,                        /* mic */
				  js->secret.micKey,
				  0,                        /* encrypt */
				  NULL,
				  kdeLen,
				  tlv)) == -1)
	{
		return -1;
	}

	return(jswSend(js, js->buf, len));
}

int
jswSendPassReq(struct jsw_session *js)
{
	int len;

	if ((len = jswBuildCommon(js->buf,
				  MAX_ETHER_BODY_SIZE,
				  JSW_OP_JSS_PASSWORD_REQ,
				  js->key_replay_counter,
				  NULL,                     /* nonce */
				  1,                        /* ack */
				  1,                        /* mic */
				  js->secret.micKey,
				  0,                        /* encrypt */
				  NULL,
				  0,
				  NULL)) == -1)
	{
		return -1;
	}

	return(jswSend(js, js->buf, len));
}

int
jswSendJdsPassAck(struct jsw_session *js)
{
	int len;

	if ((len = jswBuildCommon(js->buf,
				  MAX_ETHER_BODY_SIZE,
				  JSW_OP_JSS_PASSWORD_ACK,
				  js->key_replay_counter,
				  NULL,                     /* nonce */
				  1,                        /* ack */
				  1,                        /* mic */
				  js->secret.micKey,
				  0,                        /* encrypt */
				  NULL,
				  0,
				  NULL)) == -1)
	{
		return -1;
	}

	return(jswSend(js, js->buf, len));
}

int
jswP2SendJdsOffer(struct jsw_session *js)
{
	int len, kdeLen = 0;
	u8 tlv[JSW_MAX_KEYDATA_LEN];
	struct jsw_profile *jp;
	struct hostapd_data *hapd;
	
	hapd = js->hapd;
	jp = hapd->jsw_profile;

	kdeLen += jswBuildCapsMenuTlv(js, tlv);
	kdeLen += jswBuildDhTlv(&tlv[kdeLen], js->dh->g,       
				KEYDATA_DH_PARAM_G);
	kdeLen += jswBuildDhTlv(&tlv[kdeLen], js->dh->p,       
				KEYDATA_DH_PARAM_P);
	kdeLen += jswBuildDhTlv(&tlv[kdeLen], js->dh->pub_key, 
				KEYDATA_DH_PUB_KEY);
	kdeLen += jswBuildTlv(&tlv[kdeLen], jp->salt,
			      JSW_NONCE_SIZE, KEYDATA_SALT);

	if ((len = jswBuildCommon(js->buf,
				  MAX_ETHER_BODY_SIZE,
				  JSW_OP_JDS_OFFER,
				  js->key_replay_counter,
				  js->nonce,           /* nonce */
				  0,                   /* ack */
				  0,                   /* mic */
				  NULL,
				  0,                   /* encrypt */
				  NULL,
				  kdeLen,
				  tlv)) == -1) 
	{
		return -1;
	}

	return(jswSend(js, js->buf, len));
}

int
jswSendPMK(struct jsw_session *js)
{
	int len, kdeLen = 0;
	struct jsw_profile *jp;
	u8 tlv[JSW_MAX_KEYDATA_LEN];
	u8 pmk[WPA_PMK_LEN + 8];
	u8 micKey[JSW_SHA1_LEN];
	struct hostapd_data *hapd;
	
	hapd = js->hapd;
	jp = hapd->jsw_profile;
	kdeLen += jswBuildCapsChoiceTlv(js, tlv);

	aes_wrap(js->secret.KEK, sizeof(hapd->conf->wpa_psk->psk) / 8,
		 hapd->conf->wpa_psk->psk, pmk);

	kdeLen += jswBuildTlv(&tlv[kdeLen], pmk, sizeof(pmk), KEYDATA_PMKID);

	jswP2GetMic2Key(js, micKey);

	if ((len = jswBuildCommon(js->buf,
				  MAX_ETHER_BODY_SIZE,
				  JSW_OP_JSS_PMK_OFFER,
				  js->key_replay_counter,
				  NULL,                     /* nonce */
				  0,                        /* ack */
				  1,                        /* mic */
				  micKey,
				  0,                        /* encrypt */
				  NULL,
				  kdeLen,
				  tlv)) == -1)
	{
		return -1;
	}

	return(jswSend(js, js->buf, len));
}

int
jswSendPMKAck(struct jsw_session *js)
{
	int len;
	u8 micKey[JSW_SHA1_LEN];

	jswP2GetMic2Key(js, micKey);

	if ((len = jswBuildCommon(js->buf,
				  MAX_ETHER_BODY_SIZE,
				  JSW_OP_JSS_PMK_ACK,
				  js->key_replay_counter,
				  NULL,                     /* nonce */
				  0,                        /* ack */
				  1,                        /* mic */
				  micKey,
				  0,                        /* encrypt */
				  NULL,
				  0,
				  NULL)) == -1)
	{
        	return -1;
	}

	return(jswSend(js, js->buf, len));
}

/*
 * Decoding, parsing routines
 */
STATIC int
jswBuildCapsMenuTlv(struct jsw_session *js, u8 *to)
{
	struct hostapd_data *hapd;
	
	/* Copy the conf's rsn|wpa IE */	
	hapd = js->hapd;
	memcpy(to, hapd->wpa_ie, hapd->wpa_ie_len);
	return hapd->wpa_ie_len;
}

STATIC int
jswBuildCapsChoiceTlv(struct jsw_session *js, u8 *to)
{
	u8 *from = js->staCaps;
	int len;
		
	len = from[1] + 2;
	memcpy(to, from, len);

	return len;
}

STATIC int
jswBuildTlv(u8 *to, u8 *from, u8 dataLen, u8 type)
{
	struct key_data *tlv = (struct key_data *)to;

	tlv->elementID = WLAN_EID_GENERIC;
	tlv->length = dataLen + (tlv->data - tlv->oui);
	tlv->type = type;
	memcpy(&tlv->oui, ouiAtheros, sizeof(tlv->oui));
	memcpy(tlv->data, from, dataLen);

	return (tlv->length + 2);
}

STATIC int
jswBuildDhTlv(u8 *buf, BIGNUM *bn, u8 type)
{
	u8 bn_buf[(JSW_PRIME_BITS/8) + 1];
	int len;

	len = BN_bn2bin(bn, bn_buf);
	return(jswBuildTlv(buf, bn_buf, len, type));
}


STATIC struct jsw_tlv *
jswGetTlvProfile(JSW_FRAME_OPS op)
{
	int i;

	for (i = 0; i < sizeof(gTlv); i++) {
		if (gTlv[i].op == op) {
			return(&gTlv[i]);
		}
	}
	return NULL;
}

int
jswGetTlvs(struct jsw_session *js, 
	   struct ieee802_1x_hdr *eapolHdr,
	   JSW_FRAME_OPS op)
{
	int i = 0, totalLen = 0, tlvFound = 0;
	u16 key_data_length;
	struct wpa_eapol_key *eapolKeyHdr;
	u8 *p;
	struct jsw_tlv *verify;
	struct key_data *tlv;
	struct jsw_ieset ieSet;
	
	eapolKeyHdr = jswEapol2Key(eapolHdr);
	verify = jswGetTlvProfile(op);
	p = (u8 *)(eapolKeyHdr + 1);
	key_data_length = ntohs(eapolKeyHdr->key_data_length);
	memset(&ieSet, 0, sizeof(ieSet));
	
	while (totalLen < key_data_length) {
		u8 id, elen;
		u8 *pIe;
		
		pIe = p;
		id = *p++;
		elen = *p++;
		
		if ((!elen) || 
		    ((totalLen + elen + 2) > key_data_length)) {
			wpa_printf(MSG_DEBUG, 
				   "Bad TLV length %d\n", elen);
			return -1;
        	}
        	switch (id) {
		case WLAN_EID_RSN:
			tlvFound |= JSW_TLV_WPA2;
			ieSet.wpa2Ie = pIe;
			break;

		case WLAN_EID_GENERIC:
			tlv = (struct key_data *)pIe;

			if (jswTlvIsWpa(tlv)) {
				tlvFound |= JSW_TLV_WPA;
				ieSet.wpaIe = pIe;
				break;
			}
			if (!jswTlvIsAth(tlv)) {
				wpa_printf(MSG_DEBUG, "unknown oui\n");
                    		return -1;
			}
			switch (tlv->type) {
			case KEYDATA_DH_PUB_KEY:
				tlvFound |= JSW_TLV_PUB_KEY;
				ieSet.pubKey = tlv;
				break;

			case KEYDATA_SHA1_PASSWORD:
				tlvFound |= JSW_TLV_SHA1_PASSWORD;
				ieSet.passHash = tlv;
				break;

			case KEYDATA_SALT:
				tlvFound |= JSW_TLV_SALT;
				ieSet.salt = tlv;
				break;
				
			default:
                        	wpa_printf(MSG_DEBUG, 
					   "Unkown TLV type %d\n", tlv->type);
			}
			break;

		default:
                	wpa_printf(MSG_DEBUG,
				   "Unkown el id %d\n", id);
        	}

        	totalLen += elen + 2;
        	p += elen;
	}

	for (i = 0; i < verify->cnt; i++) {
		if (tlvFound == verify->val[i]) {
			return (jswParseTlvs(js, &ieSet, op));
		}
	}

	wpa_printf(MSG_DEBUG,
		   "Incorrect or incomplete Tlvs...dropping\n");
	return -1;
}

STATIC int
jswParseTlvs(struct jsw_session *js, struct jsw_ieset *ieSet, JSW_FRAME_OPS op)
{
	u8 *wpaIe;
	u8 *wpa2Ie;
	int len;
	struct key_data *tlv;
	struct jsw_profile *jp;
	u8 passHash[JSW_SHA1_LEN + 4];
	int res, wpa_ie_len;
	struct hostapd_data *hapd;
	struct sta_info *sta;
	u8 *wpa_menu = NULL;
	int version = 0;
	
	hapd = js->hapd;
	sta = js->sta;
	jp = hapd->jsw_profile;
	wpaIe = ieSet->wpaIe;
	wpa2Ie = ieSet->wpa2Ie;

	/*
	 * Caps Tlv handling depends on the opcode
	 */
	switch (op) {
	case JSW_OP_JDS_REQUEST:
		wpa_menu = wpa2Ie ? wpa2Ie : wpaIe;
		wpa_ie_len = wpa_menu[1] + 2;
		version = (wpa_menu[0] == WLAN_EID_RSN ?
				HOSTAPD_WPA_VERSION_WPA2 :
				HOSTAPD_WPA_VERSION_WPA);
				
		res = wpa_validate_wpa_ie(hapd, sta, wpa_menu, 
					  wpa_ie_len, version);
		if (res != WPA_IE_OK) {
			wpa_printf(MSG_DEBUG,"Invalid choice from sta\n");
			return -1;
		}
		js->staCaps = malloc(wpa_ie_len);
		if (!js->staCaps)
			return -1;
			
		memcpy(js->staCaps, wpa_menu, wpa_ie_len);
		break;

	case JSW_OP_JDS_ACK:
	case JSW_OP_JSS_PMK_ACK:
		if (!wpa2Ie || !wpaIe) {
			wpa_printf(MSG_DEBUG,
				   "Incomplete WPA menu\n");
			return -1;
		}  
		wpa_ie_len = wpa2Ie[1] + 2;	
		res = wpa_validate_wpa_ie(hapd, sta, wpa2Ie, 
				 	  wpa_ie_len,
					  HOSTAPD_WPA_VERSION_WPA2);
		if (res != WPA_IE_OK) {
			wpa_printf(MSG_DEBUG, 
				   "Invalid wpa2 IE from sta in ACK\n");
			return -1;
		}
		wpa_ie_len = wpaIe[1] + 2;	
		res = wpa_validate_wpa_ie(hapd, sta, wpaIe, 
					  wpa_ie_len,
					  HOSTAPD_WPA_VERSION_WPA);
		if (res != WPA_IE_OK) {
			wpa_printf(MSG_DEBUG, 
				   "Invalid wpa IE from sta in ACK\n");
			return -1;
		}
		break;
		
	default:
		break;
	}
    
	if (ieSet->pubKey) {
		tlv = ieSet->pubKey;
		len = tlv->length - (tlv->data - tlv->oui);
		BN_bin2bn(tlv->data, len, &js->staPubKey);
	}
	
	if (ieSet->passHash) {
		tlv = ieSet->passHash;
		len = tlv->length - (tlv->data - tlv->oui);
		memset(passHash, 0, sizeof(passHash));
		if (aes_unwrap(js->secret.KEK, (len >> 3) - 1, 
		    		tlv->data, passHash) == -1)
		{
			wpa_printf(MSG_DEBUG, "Cant decrypt password\n");
			return -1;
        	}
        	memcpy(jp->passHash, passHash, JSW_SHA1_LEN);
	}
	
	if (ieSet->salt) {
		tlv = ieSet->salt;
		len = tlv->length - (tlv->data - tlv->oui);
		if (len != JSW_NONCE_SIZE) {
			wpa_printf(MSG_DEBUG, "Wrong salt len %d\n", len);
			return -1;
		}
		memcpy(jp->salt, tlv->data, len);
	}
	return 0;
}

/*
 * Following rules apply:
 * 1. If Protocol 1 is not done, we process only our packets, but block *all*
 *    EAP packets.
 * 2. If Protocol 1 is done, we claim our packets but let EAP packets go
 */
int
js_claim_frame(struct hostapd_data *hapd, struct sta_info *sta,
		    u8 *data, size_t data_len)
{
	struct ieee802_1x_hdr *eapolHdr;
	struct eap_vp_header *vpHdr;
	int claimed = FALSE;

	eapolHdr = (struct ieee802_1x_hdr *)data;
	vpHdr = (struct eap_vp_header *)(eapolHdr + 1);
	
	if ((eapolHdr->type == IEEE802_1X_TYPE_EAP_PACKET) &&
	    (vpHdr->type == JSW_EAP_VP_TYPE)) {
		jswDecodePacket(hapd, sta, data, data_len);
		claimed = TRUE;
	} else if (hapd->conf->js_p1) {
		claimed = TRUE;
	}

	return claimed;
}

STATIC void
jswDecodePacket(struct hostapd_data *hapd, struct sta_info *sta,
		u8 *data, size_t len)
{
	struct ieee802_1x_hdr 	*eapolHdr;
	struct eap_vp_header 	*vpHdr;
	struct ath_eap_header 	*jswProtoHdr;
	struct jsw_op_hdr 	*jswOpHdr;
	struct wpa_eapol_key 	*eapolKeyHdr;
	struct jsw_session 	*js;
	u16 opCode;
	
	eapolHdr = (struct ieee802_1x_hdr *)data;
	vpHdr = (struct eap_vp_header *)(eapolHdr + 1);
	
	jswProtoHdr = (struct ath_eap_header *)(vpHdr->data);
	
	jswOpHdr    = (struct jsw_op_hdr *)(jswProtoHdr + 1);
	eapolKeyHdr = (struct wpa_eapol_key *)(jswOpHdr + 1);
	
	if (!jswMsgOk(len, eapolHdr, vpHdr, jswProtoHdr, eapolKeyHdr)) {
		wpa_printf(MSG_DEBUG, 
		   	   "JUMPSTART: " MACSTR
		   	   " %s: Protocol error\n",
		   	   MAC2STR(sta->addr), __func__);
		return;
	}
	
	js = sta->js_session;
	
	/* JS frame received out of any context in P1 mode */
	if ((hapd->conf->js_p1) && (!js)) {
		wpa_printf(MSG_DEBUG, 
			   "JUMPSTART: " MACSTR
			   " %s: Jumpstart frame received out of the blue\n",
		   	   MAC2STR(sta->addr), __func__);
		return;
	}

	/* 
	 * JS frame received in P2 mode, no context. 
	 * Start a session if needed 
	 */
	opCode = ntohs(jswOpHdr->op);
	if (hapd->conf->js_p2 && (!js)) {
		if (opCode != JSW_OP_JDS_DISCOVER) {
			return;
		}
		/* Clean up pending WPA session */
		wpa_free_station(sta);
		js_p2_new_station(hapd, sta);
		if ((js = sta->js_session) == NULL) 
			return;
	} else {
		if ((opCode != JSW_OP_JDS_DISCOVER) &&
		    !jswIsReplayCtrOk(js, eapolHdr, jswOpHdr->op)) {
			wpa_printf(MSG_DEBUG, 
				   "JUMPSTART: " MACSTR
				   " %s: Bad Replay Ctr.\n" 
				   "Expected %s: ",
				   MAC2STR(sta->addr), __func__,
				   hapd->conf->js_p2 ? "greater than" : "");
			hostapd_hexdump("",
					js->key_replay_counter,
					sizeof(u64));
			wpa_printf(MSG_DEBUG, 
				   "JUMPSTART: " MACSTR
				   " %s:\n"
				   "Gotten: ",
				   MAC2STR(sta->addr), __func__);
			hostapd_hexdump("", 
				(jswEapol2Key(eapolHdr))->replay_counter,
				sizeof(u64));
			return;
		}
	}

	/* Stop any pending timeout for the current STA */
	jswTimerStop(js);
	js->sm.event_arg = eapolHdr;

	switch (opCode) {
	case JSW_OP_JDS_DISCOVER:
		smSendEvent(js, JSW_EVENT_JDS_DISCOVER);
		break;

	case JSW_OP_JDS_REQUEST:
		smSendEvent(js, JSW_EVENT_JDS_REQ);
		break;

	case JSW_OP_JDS_ACK:
		smSendEvent(js, JSW_EVENT_JDS_ACK);
		break;

	case JSW_OP_JSS_PASSWORD_RESP:
		smSendEvent(js, JSW_EVENT_JSS_PW_RESP);
		break;

	case JSW_OP_JSS_PASSWORD_ACK:
		smSendEvent(js, JSW_EVENT_JSS_PW_ACK);
		break;

	case JSW_OP_JSS_PMK_ACK:
		smSendEvent(js, JSW_EVENT_JSS_PMK_ACK);
		break;

	default:
		wpa_printf(MSG_DEBUG, 
			   "JUMPSTART: " MACSTR
			   " %s: unkwown opcode %d\n",
			   MAC2STR(js->sta->addr), __func__, jswOpHdr->op);
		break;
	}
}

/*
 * Replay ctr rules:
 * 1. If in P1, except the below exception, replay should match.
 * 2. If in P1 and in jswP1PassPending, we rely on station retrying
 *    (coz the timeout is huge), so for ack-retry one less should match.
 * 3. If in P2, replayCtr should always be > current ctr
 */
STATIC int
jswIsReplayCtrOk(struct jsw_session *js, struct ieee802_1x_hdr *eapolHdr,
		 JSW_FRAME_OPS op)
{
	u8 frmCtr[8], expCtr[8];
	struct hostapd_data *hapd = js->hapd;
	
	memcpy(frmCtr, (jswEapol2Key(eapolHdr))->replay_counter,
	       sizeof(frmCtr));
	memcpy(expCtr, js->key_replay_counter, sizeof(expCtr));
	
	if (jswIsPassPending(js) && (op == JSW_OP_JDS_ACK)) {
		inc_byte_array(frmCtr, 8);
	}
	
	if (hapd->conf->js_p1) {
		return (!memcmp(frmCtr, expCtr, 8));
	}
	/*
	 * P2
	 */
	return (memcmp(frmCtr, expCtr, 8) > 0);
}

void
jswDumpFrame (u8 *buf)
{
	struct ieee802_1x_hdr *eapolHdr = (struct ieee802_1x_hdr *)buf;

	hostapd_hexdump("", (u8 *)eapolHdr, 
			ntohs(eapolHdr->length) +
			sizeof(struct ieee802_1x_hdr));
}

void
jswDumpBuffer(u8 *buf, int len)
{
	hostapd_hexdump("", buf, len);
}

STATIC int
jswMsgOk(int len, struct ieee802_1x_hdr *eapolHdr, 
	 struct eap_vp_header *vpHdr,
	 struct ath_eap_header *jswProtoHdr, 
	 struct wpa_eapol_key *eapolKeyHdr)
{
	u16 key_info, key_data_len;
	struct ath_eap_header tmpProto;
	u32 tmp;
	
	if (len < jswMinLen()) {
		printf("%s: bad length\n", __func__);
		return 0;
	}
	if (eapolHdr->version > EAPOL_VERSION) {
		printf("%s: bad eapol version\n", __func__);
		return 0;
	}
	if (eapolHdr->type != IEEE802_1X_TYPE_EAP_PACKET) {
		printf("%s: bad eapol type\n", __func__);
		return 0;
	}
	if (vpHdr->type != JSW_EAP_VP_TYPE) {
		printf("%s: bad VP type\n", __func__);
		return 0;
	}
	if (memcmp(vpHdr->smiOID, eapSmiOID, sizeof(eapSmiOID))) {
		printf("%s: bad VP OID\n", __func__);
		return 0;
	}
	memcpy(&tmp, jswProtoHdr, sizeof(tmp));
	tmp = ntohl(tmp);
	memcpy(&tmpProto, &tmp, sizeof(tmpProto));
	if (tmpProto.proto !=  ATHL2P_JUMPSTART_PROTO) {
		printf("%s: bad JS proto\n", __func__);
		return 0;
	}
	if (tmpProto.version > JSW_VERSION) {
		printf("%s: bad JS version\n", __func__);
		return 0;
	}
	if (eapolKeyHdr->type != JSW_KEY_DESC) {
		printf("%s: bad key descriptor\n", __func__);
		return 0;
	}
	key_info = ntohs(eapolKeyHdr->key_info);
	if ((key_info & WPA_KEY_INFO_TYPE_MASK) != JSW_KEY_INFO_TYPE) {
		printf("%s: bad key info\n", __func__);
		return 0;
	}
	key_data_len = ntohs(eapolKeyHdr->key_data_length);
	if (key_data_len > JSW_MAX_KEYDATA_LEN) {
		printf("%s: bad key info\n", __func__);
		return 0;
	}
	return 1;
}
