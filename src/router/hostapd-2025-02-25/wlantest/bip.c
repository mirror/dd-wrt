/*
 * BIP
 * Copyright (c) 2010-2012, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "utils/includes.h"

#include "utils/common.h"
#include "common/ieee802_11_defs.h"
#include "crypto/aes_wrap.h"
#include "wlantest.h"


u8 * bip_protect(const u8 *igtk, size_t igtk_len, u8 *frame, size_t len,
		 u64 ipn, int keyid, size_t *prot_len)
{
	u8 *prot, *pos, *buf;
	u8 mic[16];
	u16 fc;
	struct ieee80211_hdr *hdr;
	size_t plen;

	plen = len + (igtk_len == 32 ? 26 : 18);
	prot = os_malloc(plen);
	if (prot == NULL)
		return NULL;
	os_memcpy(prot, frame, len);
	pos = prot + len;
	*pos++ = WLAN_EID_MMIE;
	*pos++ = igtk_len == 32 ? 24 : 16;
	WPA_PUT_LE16(pos, keyid);
	pos += 2;
	WPA_PUT_LE48(pos, ipn);
	pos += 6;
	os_memset(pos, 0, igtk_len == 32 ? 16 : 8); /* MIC */

	buf = os_malloc(plen + 20 - 24);
	if (buf == NULL) {
		os_free(prot);
		return NULL;
	}

	/* BIP AAD: FC(masked) A1 A2 A3 */
	hdr = (struct ieee80211_hdr *) frame;
	fc = le_to_host16(hdr->frame_control);
	fc &= ~(WLAN_FC_RETRY | WLAN_FC_PWRMGT | WLAN_FC_MOREDATA);
	WPA_PUT_LE16(buf, fc);
	os_memcpy(buf + 2, hdr->addr1, 3 * ETH_ALEN);
	os_memcpy(buf + 20, prot + 24, plen - 24);
	wpa_hexdump(MSG_MSGDUMP, "BIP: AAD|Body(masked)", buf, plen + 20 - 24);
	/* MIC = L(AES-128-CMAC(AAD || Frame Body(masked)), 0, 64) */
	if (omac1_aes_128(igtk, buf, plen + 20 - 24, mic) < 0) {
		os_free(prot);
		os_free(buf);
		return NULL;
	}
	os_free(buf);

	os_memcpy(pos, mic, igtk_len == 32 ? 16 : 8);
	wpa_hexdump(MSG_DEBUG, "BIP MMIE MIC", pos, igtk_len == 32 ? 16 : 8);

	*prot_len = plen;
	return prot;
}


#define MME_LEN_LESS_MIC	10
#define MIC_ELEM_LEN_LESS_MIC	2

u8 * bip_protect_s1g_beacon(const u8 *igtk, size_t igtk_len, const u8 *frame,
			    size_t len, const u8 *ipn, int keyid, bool bce,
			    size_t *prot_len)
{
	u8 *prot, *pos, *buf;
	u8 mic[16];
	u16 fc;
	struct ieee80211_hdr_s1g_beacon *hdr;
	struct ieee80211_s1g_beacon_compat *bc;
	size_t plen, mic_len, element_len, buf_len;
	size_t hdr_add_len = 0, bce_add_len = 0;

	mic_len = igtk_len == 32 ? 16 : 8;
	if (bce)
		element_len = MIC_ELEM_LEN_LESS_MIC + mic_len;
	else
		element_len = MME_LEN_LESS_MIC + mic_len;

	plen = len + element_len; /* add element length */
	prot = os_malloc(plen);
	if (!prot)
		return NULL;
	os_memcpy(prot, frame, len);
	pos = prot + len;

	/* Add MME/MIC element to the end of the frame */
	if (bce) {
		bce_add_len = 6; /* AAD increases by 6 bytes for BCE */
		*pos++ = WLAN_EID_MIC; /* Element ID */
		*pos++ = element_len - 2; /* Length field */
	} else {
		*pos++ = WLAN_EID_MMIE; /* Element ID */
		*pos++ = element_len - 2; /* Length field */
		WPA_PUT_LE16(pos, keyid); /* KeyID */
		pos += 2;
		os_memcpy(pos, ipn, 6); /* BIPN */
		pos += 6;
	}
	os_memset(pos, 0, mic_len); /* MIC */

	/* Duration (2) and Timestamp (4) are omitted from AAD, BIPN (6) is
	 * added if BCE is used */
	buf_len = plen - 6 + bce_add_len;
	buf = os_malloc(buf_len);
	if (!buf) {
		os_free(prot);
		return NULL;
	}

	/* BIP AAD: FC SA ChangeSeq NextTBTT(if present)
	 * CompressedSSID(if present) ANO(if present), BIPN(if using BCE) */
	hdr = (struct ieee80211_hdr_s1g_beacon *) frame;
	fc = le_to_host16(hdr->frame_control);
	if (WLAN_FC_GET_TYPE(fc) != WLAN_FC_TYPE_EXT ||
	    WLAN_FC_GET_STYPE(fc) != WLAN_FC_STYPE_S1G_BEACON) {
		wpa_printf(MSG_ERROR, "Frame is not an S1G Beacon");
		os_free(prot);
		os_free(buf);
		return NULL;
	}

	WPA_PUT_LE16(buf, fc);
	os_memcpy(buf + 2, hdr->sa, ETH_ALEN);
	os_memcpy(buf + 8, hdr->change_seq, 1);
	if (fc & WLAN_FC_S1G_BEACON_NEXT_TBTT)
		hdr_add_len += 3;
	if (fc & WLAN_FC_S1G_BEACON_COMP_SSID)
		hdr_add_len += 4;
	if (fc & WLAN_FC_S1G_BEACON_ANO)
		hdr_add_len++;
	os_memcpy(buf + 9, prot + IEEE80211_HDRLEN_S1G_BEACON, hdr_add_len);
	if (bce)
		os_memcpy(buf + 9 + hdr_add_len, ipn, bce_add_len);
	os_memcpy(buf + 9 + hdr_add_len + bce_add_len,
		  prot + IEEE80211_HDRLEN_S1G_BEACON + hdr_add_len,
		  plen - (IEEE80211_HDRLEN_S1G_BEACON + hdr_add_len));
	/* The S1G Beacon Compatibility element, when present, is the first
	 * element in the S1G Beacon frame body */
	if (len >= IEEE80211_HDRLEN_S1G_BEACON + hdr_add_len +
	    sizeof(struct ieee80211_s1g_beacon_compat)) {
		bc = (struct ieee80211_s1g_beacon_compat *)
			(buf + 9 + hdr_add_len + bce_add_len);
		if (bc->element_id == WLAN_EID_S1G_BCN_COMPAT) {
			wpa_printf(MSG_DEBUG,
				   "S1G Beacon Compatibility element found, masking TSF Completion field");
			os_memset(&bc->tsf_completion, 0,
				  sizeof(bc->tsf_completion));
			if (bce && keyid - 6 != (bc->compat_info & 0x80) >> 7) {
				wpa_printf(MSG_ERROR,
					   "Key ID does not match BIGTK Key ID Index in S1G Beacon Compatibility element");
				os_free(prot);
				os_free(buf);
				return NULL;
			}
		}
	}

	wpa_hexdump(MSG_MSGDUMP, "S1G BIP-CMAC AAD|Body(masked)", buf, buf_len);

	/* MIC = L(AES-128-CMAC(AAD || Frame Body), 0, 64) */
	if (omac1_aes_128(igtk, buf, buf_len, mic) < 0) {
		os_free(prot);
		os_free(buf);
		return NULL;
	}
	os_free(buf);

	os_memcpy(pos, mic, mic_len);
	wpa_hexdump(MSG_DEBUG, "S1G BIP-CMAC MIC", pos, mic_len);

	*prot_len = plen;
	return prot;
}


u8 * bip_gmac_protect(const u8 *igtk, size_t igtk_len, u8 *frame, size_t len,
		      u8 *ipn, int keyid, size_t *prot_len)
{
	u8 *prot, *pos, *buf;
	u16 fc;
	struct ieee80211_hdr *hdr;
	size_t plen;
	u8 nonce[12], *npos;

	plen = len + 26;
	prot = os_malloc(plen);
	if (prot == NULL)
		return NULL;
	os_memcpy(prot, frame, len);
	pos = prot + len;
	*pos++ = WLAN_EID_MMIE;
	*pos++ = 24;
	WPA_PUT_LE16(pos, keyid);
	pos += 2;
	os_memcpy(pos, ipn, 6);
	pos += 6;
	os_memset(pos, 0, 16); /* MIC */

	buf = os_malloc(plen + 20 - 24);
	if (buf == NULL) {
		os_free(prot);
		return NULL;
	}

	/* BIP AAD: FC(masked) A1 A2 A3 */
	hdr = (struct ieee80211_hdr *) frame;
	fc = le_to_host16(hdr->frame_control);
	fc &= ~(WLAN_FC_RETRY | WLAN_FC_PWRMGT | WLAN_FC_MOREDATA);
	WPA_PUT_LE16(buf, fc);
	os_memcpy(buf + 2, hdr->addr1, 3 * ETH_ALEN);
	os_memcpy(buf + 20, prot + 24, plen - 24);
	wpa_hexdump(MSG_MSGDUMP, "BIP-GMAC: AAD|Body(masked)",
		    buf, plen + 20 - 24);

	/* Nonce: A2 | IPN */
	os_memcpy(nonce, hdr->addr2, ETH_ALEN);
	npos = nonce + ETH_ALEN;
	*npos++ = ipn[5];
	*npos++ = ipn[4];
	*npos++ = ipn[3];
	*npos++ = ipn[2];
	*npos++ = ipn[1];
	*npos++ = ipn[0];
	wpa_hexdump(MSG_EXCESSIVE, "BIP-GMAC: Nonce", nonce, sizeof(nonce));

	/* MIC = AES-GMAC(AAD || Frame Body(masked)) */
	if (aes_gmac(igtk, igtk_len, nonce, sizeof(nonce),
		     buf, plen + 20 - 24, pos) < 0) {
		os_free(prot);
		os_free(buf);
		return NULL;
	}
	os_free(buf);

	wpa_hexdump(MSG_DEBUG, "BIP-GMAC MMIE MIC", pos, 16);

	*prot_len = plen;
	return prot;
}


u8 * bip_gmac_protect_s1g_beacon(const u8 *igtk, size_t igtk_len,
				 const u8 *frame, size_t len, const u8 *ipn,
				 int keyid, bool bce, size_t *prot_len)
{
	u8 *prot, *pos, *buf;
	u16 fc;
	struct ieee80211_hdr_s1g_beacon *hdr;
	struct ieee80211_s1g_beacon_compat *bc;
	size_t plen, mic_len, element_len, buf_len;
	size_t hdr_add_len = 0, bce_add_len = 0;
	u8 nonce[12], *npos;

	mic_len = 16;
	if (bce)
		element_len = MIC_ELEM_LEN_LESS_MIC + mic_len;
	else
		element_len = MME_LEN_LESS_MIC + mic_len;

	plen = len + element_len; /* add element length */
	prot = os_malloc(plen);
	if (!prot)
		return NULL;
	os_memcpy(prot, frame, len);
	pos = prot + len;
	/* Add MME/MIC element to the end of the frame */
	if (bce) {
		bce_add_len = 6;
		*pos++ = WLAN_EID_MIC; /* Element ID */
		*pos++ = element_len - 2; /* Length field */
	} else {
		*pos++ = WLAN_EID_MMIE; /* Element ID */
		*pos++ = element_len - 2; /* Length field */
		WPA_PUT_LE16(pos, keyid); /* KeyID */
		pos += 2;
		os_memcpy(pos, ipn, 6); /* BIPN */
		pos += 6;
	}
	os_memset(pos, 0, mic_len); /* MIC */

	 /* Duration (2) and Timestamp (4) are omitted from AAD,
	  * BIPN (6) is added if BCE is used */
	buf_len = plen - 6 + bce_add_len;
	buf = os_malloc(buf_len);
	if (!buf) {
		os_free(prot);
		return NULL;
	}

	/* BIP AAD: FC SA ChangeSeq NextTBTT(if present)
	 * CompressedSSID(if present) ANO(if present) */
	hdr = (struct ieee80211_hdr_s1g_beacon *) frame;
	fc = le_to_host16(hdr->frame_control);
	if (WLAN_FC_GET_TYPE(fc) != WLAN_FC_TYPE_EXT ||
	    WLAN_FC_GET_STYPE(fc) != WLAN_FC_STYPE_S1G_BEACON) {
		wpa_printf(MSG_ERROR, "Frame is not an S1G Beacon");
		os_free(prot);
		os_free(buf);
		return NULL;
	}

	WPA_PUT_LE16(buf, fc);
	os_memcpy(buf + 2, hdr->sa, ETH_ALEN);
	os_memcpy(buf + 8, hdr->change_seq, 1);
	if (fc & WLAN_FC_S1G_BEACON_NEXT_TBTT)
		hdr_add_len += 3;
	if (fc & WLAN_FC_S1G_BEACON_COMP_SSID)
		hdr_add_len += 4;
	if (fc & WLAN_FC_S1G_BEACON_ANO)
		hdr_add_len++;
	os_memcpy(buf + 9, prot + IEEE80211_HDRLEN_S1G_BEACON, hdr_add_len);
	if (bce)
		os_memcpy(buf + 9 + hdr_add_len, ipn, bce_add_len);
	os_memcpy(buf + 9 + hdr_add_len + bce_add_len,
		  prot + IEEE80211_HDRLEN_S1G_BEACON + hdr_add_len,
		  plen - (IEEE80211_HDRLEN_S1G_BEACON + hdr_add_len));
	/* The S1G Beacon Compatibility element, when present, is the first
	 * element in the S1G Beacon frame body */
	if (len >= IEEE80211_HDRLEN_S1G_BEACON + hdr_add_len +
	    sizeof(struct ieee80211_s1g_beacon_compat)) {
		bc = (struct ieee80211_s1g_beacon_compat *)
			(buf + 9 + hdr_add_len + bce_add_len);
		if (bc->element_id == WLAN_EID_S1G_BCN_COMPAT) {
			wpa_printf(MSG_DEBUG,
				   "S1G Beacon Compatibility element found, masking TSF Completion field");
			os_memset(&bc->tsf_completion, 0,
				  sizeof(bc->tsf_completion));
			if (bce && keyid - 6 != (bc->compat_info & 0x80) >> 7) {
				wpa_printf(MSG_ERROR,
					   "Key ID does not match BIGTK Key ID Index in S1G Beacon Compatibility element");
				os_free(prot);
				os_free(buf);
				return NULL;
			}
		}
	}
	wpa_hexdump(MSG_MSGDUMP, "S1G BIP-GMAC AAD|Body(masked)", buf, buf_len);

	/* Nonce: SA | IPN */
	os_memcpy(nonce, hdr->sa, ETH_ALEN);
	npos = nonce + ETH_ALEN;
	*npos++ = ipn[5];
	*npos++ = ipn[4];
	*npos++ = ipn[3];
	*npos++ = ipn[2];
	*npos++ = ipn[1];
	*npos++ = ipn[0];
	wpa_hexdump(MSG_EXCESSIVE, "S1G BIP-GMAC Nonce", nonce, sizeof(nonce));

	/* MIC = AES-GMAC(AAD || Frame Body) */
	if (aes_gmac(igtk, igtk_len, nonce, sizeof(nonce), buf, buf_len, pos) <
	    0) {
		os_free(prot);
		os_free(buf);
		return NULL;
	}
	os_free(buf);

	wpa_hexdump(MSG_DEBUG, "S1G BIP-GMAC MIC", pos, 16);

	*prot_len = plen;
	return prot;
}
