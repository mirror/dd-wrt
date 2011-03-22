/*
 *   bcmwpa.c - shared WPA-related functions
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: bcmwpa.c,v 1.44.18.3 2010/02/18 23:34:50 Exp $
 */

#include <bcmendian.h>

/* include wl driver config file if this file is compiled for driver */
#ifdef BCMDRIVER
#include <osl.h>
#elif defined(BCMEXTSUP)
#include <string.h>
#include <bcm_osl.h>
#else
#if defined(__GNUC__)
extern void bcopy(const void *src, void *dst, uint len);
extern int bcmp(const void *b1, const void *b2, uint len);
extern void bzero(void *b, uint len);
#else
#define	bcopy(src, dst, len)	memcpy((dst), (src), (len))
#define	bcmp(b1, b2, len)	memcmp((b1), (b2), (len))
#define	bzero(b, len)		memset((b), 0, (len))
#endif /* defined(__GNUC__) */

#endif /* BCMDRIVER */


#include <wlioctl.h>
#include <proto/802.11.h>
#if defined(BCMSUP_PSK) || defined(BCMSUPPL)
#include <proto/eapol.h>
#endif	/* defined(BCMSUP_PSK) || defined(BCMSUPPL) */
#include <bcmutils.h>
#include <bcmwpa.h>

#ifdef	BCMSUP_PSK

#include <bcmcrypto/prf.h>
#include <bcmcrypto/rc4.h>

void
BCMROMFN(wpa_calc_pmkid)(struct ether_addr *auth_ea, struct ether_addr *sta_ea,
                         uint8 *pmk, uint pmk_len, uint8 *pmkid, uint8 *data, uint8 *digest)
{
	/* PMKID = HMAC-SHA1-128(PMK, "PMK Name" | AA | SPA) */
	char prefix[] = "PMK Name";
	int data_len = 0;

	/* create the the data portion */
	bcopy(prefix, data, strlen(prefix));
	data_len += strlen(prefix);
	bcopy((uint8 *)auth_ea, &data[data_len], ETHER_ADDR_LEN);
	data_len += ETHER_ADDR_LEN;
	bcopy((uint8 *)sta_ea, &data[data_len], ETHER_ADDR_LEN);
	data_len += ETHER_ADDR_LEN;

	/* generate the pmkid */
	hmac_sha1(data, data_len, pmk, pmk_len, digest);
	bcopy(digest, pmkid, WPA2_PMKID_LEN);
}

bool
wpa_encr_key_data(eapol_wpa_key_header_t *body, uint16 key_info, uint8 *ekey,
	uint8 *gtk,	uint8 *data, uint8 *encrkey, rc4_ks_t *rc4key)
{
	uint16 len;

	switch (key_info & (WPA_KEY_DESC_V1 | WPA_KEY_DESC_V2)) {
	case WPA_KEY_DESC_V1:
		if (gtk)
			len = ntoh16_ua((uint8 *)&body->key_len);
		else
			len = ntoh16_ua((uint8 *)&body->data_len);

		/* create the iv/ptk key */
		bcopy(body->iv, encrkey, 16);
		bcopy(ekey, &encrkey[16], 16);
		/* encrypt the key data */
		prepare_key(encrkey, 32, rc4key);
		rc4(data, WPA_KEY_DATA_LEN_256, rc4key); /* dump 256 bytes */
		rc4(body->data, len, rc4key);
		break;
	case WPA_KEY_DESC_V2:
		len = ntoh16_ua((uint8 *)&body->data_len);
		/* pad if needed - min. 16 bytes, 8 byte aligned */
		/* padding is 0xdd followed by 0's */
		if (len < 2*AKW_BLOCK_LEN) {
			body->data[len] = WPA2_KEY_DATA_PAD;
			bzero(&body->data[len+1], 2*AKW_BLOCK_LEN - (len+1));
			len = 2*AKW_BLOCK_LEN;
		} else if (len % AKW_BLOCK_LEN) {
			body->data[len] = WPA2_KEY_DATA_PAD;
			bzero(&body->data[len+1], AKW_BLOCK_LEN - ((len+1) % AKW_BLOCK_LEN));
			len += AKW_BLOCK_LEN - (len % AKW_BLOCK_LEN);
		}
		if (aes_wrap(WPA_MIC_KEY_LEN, ekey, len, body->data, body->data)) {
			return FALSE;
		}
		len += 8;
		hton16_ua_store(len, (uint8 *)&body->data_len);
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

/* Decrypt a key data from a WPA key message */
bool
BCMROMFN(wpa_decr_key_data)(eapol_wpa_key_header_t *body, uint16 key_info, uint8 *ekey,
                            uint8 *gtk, uint8 *data, uint8 *encrkey, rc4_ks_t *rc4key)
{
	uint16 len;

	switch (key_info & (WPA_KEY_DESC_V1 | WPA_KEY_DESC_V2)) {
	case WPA_KEY_DESC_V1:
		bcopy(body->iv, encrkey, WPA_MIC_KEY_LEN);
		bcopy(ekey, &encrkey[WPA_MIC_KEY_LEN], WPA_MIC_KEY_LEN);
		/* decrypt the key data */
		prepare_key(encrkey, WPA_MIC_KEY_LEN*2, rc4key);
		rc4(data, WPA_KEY_DATA_LEN_256, rc4key); /* dump 256 bytes */
		if (gtk)
			len = ntoh16_ua((uint8 *)&body->key_len);
		else
			len = ntoh16_ua((uint8 *)&body->data_len);
		rc4(body->data, len, rc4key);
		if (gtk)
			bcopy(body->data, gtk, len);
		break;

	case WPA_KEY_DESC_V2:
		len = ntoh16_ua((uint8 *)&body->data_len);
		if (aes_unwrap(WPA_MIC_KEY_LEN, ekey, len, body->data,
		               gtk ? gtk : body->data)) {
			return FALSE;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

void
BCMROMFN(wpa_calc_ptk)(struct ether_addr *auth_ea, struct ether_addr *sta_ea,
                       uint8 *anonce, uint8* snonce, uint8 *pmk, uint pmk_len,
                       uint8 *ptk, uint ptk_len)
{
	uchar data[128], prf_buff[PRF_OUTBUF_LEN];
	char prefix[] = "Pairwise key expansion";
	uint data_len = 0;

	/* Create the the data portion:
	 * the lesser of the EAs, followed by the greater of the EAs,
	 * followed by the lesser of the the nonces, followed by the
	 * greater of the nonces.
	 */
	bcopy(wpa_array_cmp(MIN_ARRAY, (uint8 *)auth_ea, (uint8 *)sta_ea,
	                    ETHER_ADDR_LEN),
	      (char *)&data[data_len], ETHER_ADDR_LEN);
	data_len += ETHER_ADDR_LEN;
	bcopy(wpa_array_cmp(MAX_ARRAY, (uint8 *)auth_ea, (uint8 *)sta_ea,
	                    ETHER_ADDR_LEN),
	      (char *)&data[data_len], ETHER_ADDR_LEN);
	data_len += ETHER_ADDR_LEN;
	bcopy(wpa_array_cmp(MIN_ARRAY, snonce, anonce,
	                    EAPOL_WPA_KEY_NONCE_LEN),
	      (char *)&data[data_len], EAPOL_WPA_KEY_NONCE_LEN);
	data_len += EAPOL_WPA_KEY_NONCE_LEN;
	bcopy(wpa_array_cmp(MAX_ARRAY, snonce, anonce,
	                    EAPOL_WPA_KEY_NONCE_LEN),
	      (char *)&data[data_len], EAPOL_WPA_KEY_NONCE_LEN);
	data_len += EAPOL_WPA_KEY_NONCE_LEN;

	/* generate the PTK */
	ASSERT(strlen(prefix) + data_len + 1 <= PRF_MAX_I_D_LEN);
	fPRF(pmk, (int)pmk_len, (uchar *)prefix, strlen(prefix), data, data_len,
	     prf_buff, (int)ptk_len);
	bcopy(prf_buff, (char*)ptk, ptk_len);
}

/* Decrypt a group transient key from a WPA key message */
bool
BCMROMFN(wpa_decr_gtk)(eapol_wpa_key_header_t *body, uint16 key_info, uint8 *ekey,
	uint8 *gtk, uint8 *data, uint8 *encrkey, rc4_ks_t *rc4key)
{
	return wpa_decr_key_data(body, key_info, ekey, gtk, data, encrkey, rc4key);
}

/* Compute Message Integrity Code (MIC) over EAPOL message */
bool
BCMROMFN(wpa_make_mic)(eapol_header_t *eapol, uint key_desc, uint8 *mic_key, uchar *mic)
{
	int mic_length;

	/* length of eapol pkt from the version field on */
	mic_length = 4 + ntoh16_ua((uint8 *)&eapol->length);

	/* Create the MIC for the pkt */
	switch (key_desc) {
	case WPA_KEY_DESC_V1:
		hmac_md5(&eapol->version, mic_length, mic_key,
		         EAPOL_WPA_KEY_MIC_LEN, mic);
		break;
	case WPA_KEY_DESC_V2:
		hmac_sha1(&eapol->version, mic_length, mic_key,
		          EAPOL_WPA_KEY_MIC_LEN, mic);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

/* Check MIC of EAPOL message */
bool
BCMROMFN(wpa_check_mic)(eapol_header_t *eapol, uint key_desc, uint8 *mic_key)
{
	eapol_wpa_key_header_t *body = (eapol_wpa_key_header_t *)eapol->body;
	uchar digest[PRF_OUTBUF_LEN];
	uchar mic[EAPOL_WPA_KEY_MIC_LEN];

	/* save MIC and clear its space in message */
	bcopy((char*)&body->mic, mic, EAPOL_WPA_KEY_MIC_LEN);
	bzero((char*)&body->mic, EAPOL_WPA_KEY_MIC_LEN);

	if (!wpa_make_mic(eapol, key_desc, mic_key, digest)) {
		return FALSE;
	}
	return !bcmp(digest, mic, EAPOL_WPA_KEY_MIC_LEN);
}
#endif	/* BCMSUP_PSK */

/* Convert WPA/WPA2 IE cipher suite to locally used value */
static bool
rsn_cipher(wpa_suite_t *suite, ushort *cipher, const uint8 *std_oui, bool wep_ok)
{
	bool ret = TRUE;

	if (!bcmp((const char *)suite->oui, std_oui, DOT11_OUI_LEN)) {
		switch (suite->type) {
		case WPA_CIPHER_TKIP:
			*cipher = CRYPTO_ALGO_TKIP;
			break;
		case WPA_CIPHER_AES_CCM:
			*cipher = CRYPTO_ALGO_AES_CCM;
			break;
		case WPA_CIPHER_WEP_40:
			if (wep_ok)
				*cipher = CRYPTO_ALGO_WEP1;
			else
				ret = FALSE;
			break;
		case WPA_CIPHER_WEP_104:
			if (wep_ok)
				*cipher = CRYPTO_ALGO_WEP128;
			else
				ret = FALSE;
			break;
		default:
			ret = FALSE;
			break;
		}
		return ret;
	}

	/* check for other vendor OUIs */
	return FALSE;
}

bool
BCMROMFN(wpa_cipher)(wpa_suite_t *suite, ushort *cipher, bool wep_ok)
{
	return rsn_cipher(suite, cipher, (const uchar*)WPA_OUI, wep_ok);
}

bool
BCMROMFN(wpa2_cipher)(wpa_suite_t *suite, ushort *cipher, bool wep_ok)
{
	return rsn_cipher(suite, cipher, (const uchar*)WPA2_OUI, wep_ok);
}

/* Is this body of this tlvs entry a WFA entry? If
 * not update the tlvs buffer pointer/length.
 */
bool
bcm_is_wfa_ie(uint8 *ie, uint8 **tlvs, uint *tlvs_len, uint8 type)
{
	/* If the contents match the WFA_OUI and type */
	if ((ie[TLV_LEN_OFF] > (WFA_OUI_LEN+1)) &&
	    !bcmp(&ie[TLV_BODY_OFF], WFA_OUI, WFA_OUI_LEN) &&
	    type == ie[TLV_BODY_OFF + WFA_OUI_LEN]) {
		return TRUE;
	}

	/* point to the next ie */
	ie += ie[TLV_LEN_OFF] + TLV_HDR_LEN;
	/* calculate the length of the rest of the buffer */
	*tlvs_len -= (int)(ie - *tlvs);
	/* update the pointer to the start of the buffer */
	*tlvs = ie;

	return FALSE;
}

wpa_ie_fixed_t *
BCMROMFN(bcm_find_wpaie)(uint8 *parse, uint len)
{
	bcm_tlv_t *ie;

	while ((ie = bcm_parse_tlvs(parse, len, DOT11_MNG_VS_ID))) {
		if (bcm_is_wpa_ie((uint8*)ie, &parse, &len)) {
			return (wpa_ie_fixed_t *)ie;
		}
	}
	return NULL;
}

wpa_ie_fixed_t *
bcm_find_wpsie(uint8 *parse, uint len)
{
	bcm_tlv_t *ie;

	while ((ie = bcm_parse_tlvs(parse, len, DOT11_MNG_VS_ID))) {
		if (bcm_is_wps_ie((uint8*)ie, &parse, &len)) {
			return (wpa_ie_fixed_t *)ie;
		}
	}
	return NULL;
}

#ifdef WLP2P
wifi_p2p_ie_t *
bcm_find_p2pie(uint8 *parse, uint len)
{
	bcm_tlv_t *ie;

	while ((ie = bcm_parse_tlvs(parse, len, DOT11_MNG_VS_ID))) {
		if (bcm_is_p2p_ie((uint8*)ie, &parse, &len)) {
			return (wifi_p2p_ie_t *)ie;
		}
	}
	return NULL;
}
#endif

#if defined(BCMSUP_PSK) || defined(BCMSUPPL)
/* Is this body of this tlvs entry a WPA entry? If */
/* not update the tlvs buffer pointer/length */
static bool
wpa_is_kde(bcm_tlv_t *ie, uint8 **tlvs, uint *tlvs_len, uint8 type)
{
	/* If the contents match the 802.11i OUI and desired subtype */
	if ((ie->len > (DOT11_OUI_LEN+1)) &&
	    !bcmp(ie->data, WPA2_OUI, DOT11_OUI_LEN) &&
	    (ie->data[DOT11_OUI_LEN] == type)) {
		return TRUE;
	}

	/* point to the next ie */
	ie = (bcm_tlv_t *)((uint8 *)ie + ie->len + TLV_HDR_LEN);
	/* calculate the length of the rest of the buffer */
	*tlvs_len -= (int)((uint8*)ie - *tlvs);
	/* update the pointer to the start of the buffer */
	*tlvs = (uint8*)ie;

	return FALSE;
}

eapol_wpa2_encap_data_t *
BCMROMFN(wpa_find_kde)(uint8 *parse, uint len, uint8 type)
{
	bcm_tlv_t *ie;

	while ((ie = bcm_parse_tlvs(parse, len, DOT11_MNG_PROPR_ID))) {
		if (wpa_is_kde(ie, &parse, &len, type)) {
			return (eapol_wpa2_encap_data_t *)ie;
		}
	}
	return NULL;
}

bool
BCMROMFN(wpa_is_gtk_encap)(uint8 *ie, uint8 **tlvs, uint *tlvs_len)
{
	return wpa_is_kde((bcm_tlv_t*)ie, tlvs, tlvs_len, WPA2_KEY_DATA_SUBTYPE_GTK);
}

eapol_wpa2_encap_data_t *
BCMROMFN(wpa_find_gtk_encap)(uint8 *parse, uint len)
{
	return wpa_find_kde(parse, len, WPA2_KEY_DATA_SUBTYPE_GTK);
}
#endif /* defined(BCMSUP_PSK) || defined(BCMSUPPL) */

uint8 *
BCMROMFN(wpa_array_cmp)(int max_array, uint8 *x, uint8 *y, uint len)
{
	uint i;
	uint8 *ret = x;

	for (i = 0; i < len; i++)
		if (x[i] != y[i])
			break;

	if (i == len) {
		return NULL;
	}
	if (max_array && (y[i] > x[i]))
		ret = y;
	if (!max_array && (y[i] < x[i]))
		ret = y;

	return (ret);
}

void
BCMROMFN(wpa_incr_array)(uint8 *array, uint len)
{
	int i;

	for (i = (len-1); i >= 0; i--)
		if (array[i]++ != 0xff) {
			break;
		}
}

/* map akm suite to internal WPA_AUTH_XXXX */
/* akms points to 4 byte suite (oui + type) */
bool
BCMROMFN(bcmwpa_akm2WPAauth)(uint8 *akm, uint32 *auth, bool sta_iswpa)
{
	if (!bcmp(akm, WPA2_OUI, DOT11_OUI_LEN)) {
		switch (akm[DOT11_OUI_LEN]) {
		case RSN_AKM_NONE:
			*auth = WPA_AUTH_NONE;
			break;
		case RSN_AKM_UNSPECIFIED:
			*auth = WPA2_AUTH_UNSPECIFIED;
			break;
		case RSN_AKM_PSK:
			*auth = WPA2_AUTH_PSK;
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	else
	if (!bcmp(akm, WPA_OUI, DOT11_OUI_LEN)) {
		switch (akm[DOT11_OUI_LEN]) {
		case RSN_AKM_NONE:
			*auth = WPA_AUTH_NONE;
			break;
		case RSN_AKM_UNSPECIFIED:
			*auth = WPA_AUTH_UNSPECIFIED;
			break;
		case RSN_AKM_PSK:
			*auth = WPA_AUTH_PSK;
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

/* map cipher suite to internal WSEC_XXXX */
/* cs points 4 byte cipher suite, and only the type is used for non CCX ciphers */
bool
BCMROMFN(bcmwpa_cipher2wsec)(uint8 *cipher, uint32 *wsec)
{
	switch (cipher[DOT11_OUI_LEN]) {
	case WPA_CIPHER_NONE:
		*wsec = 0;
		break;
	case WPA_CIPHER_WEP_40:
	case WPA_CIPHER_WEP_104:
		*wsec = WEP_ENABLED;
		break;
	case WPA_CIPHER_TKIP:
		*wsec = TKIP_ENABLED;
		break;
	case WPA_CIPHER_AES_CCM:
		*wsec = AES_ENABLED;
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
