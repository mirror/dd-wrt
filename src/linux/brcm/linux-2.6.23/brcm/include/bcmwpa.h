/*
 * bcmwpa.h - interface definitions of shared WPA-related functions
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: bcmwpa.h,v 13.32.4.2.6.1 2008/11/19 04:37:38 Exp $
 */

#ifndef _BCMWPA_H_
#define _BCMWPA_H_

#include <proto/wpa.h>
#if defined(BCMSUP_PSK) || defined(BCMSUPPL)
#include <proto/eapol.h>
#endif 
#include <proto/802.11.h>
#include <wlioctl.h>

/* Field sizes for WPA key hierarchy */
#define WPA_MIC_KEY_LEN		16
#define WPA_ENCR_KEY_LEN	16
#define WPA_TEMP_ENCR_KEY_LEN	16
#define WPA_TEMP_TX_KEY_LEN	8
#define WPA_TEMP_RX_KEY_LEN	8

#define PMK_LEN			32
#define TKIP_PTK_LEN		64
#define TKIP_TK_LEN		32
#define AES_PTK_LEN		48
#define AES_TK_LEN		16

/* limits for pre-shared key lengths */
#define WPA_MIN_PSK_LEN		8
#define WPA_MAX_PSK_LEN		64

#define WLC_SW_KEYS(wlc, bsscfg) ((((wlc)->wsec_swkeys) || \
	((bsscfg)->wsec & WSEC_SWFLAG)))


#define WSEC_WEP_ENABLED(wsec)	((wsec) & WEP_ENABLED)
#define WSEC_TKIP_ENABLED(wsec)	((wsec) & TKIP_ENABLED)
#define WSEC_AES_ENABLED(wsec)	((wsec) & AES_ENABLED)
#define WSEC_ENABLED(wsec)	((wsec) & (WEP_ENABLED | TKIP_ENABLED | AES_ENABLED))
#define WSEC_SES_OW_ENABLED(wsec)	((wsec) & SES_OW_ENABLED)

#define IS_WPA_AUTH(auth)	((auth) == WPA_AUTH_NONE || \
				 (auth) == WPA_AUTH_UNSPECIFIED || \
				 (auth) == WPA_AUTH_PSK)
#define INCLUDES_WPA_AUTH(auth)	\
			((auth) & (WPA_AUTH_NONE | WPA_AUTH_UNSPECIFIED | WPA_AUTH_PSK))

#ifdef BCMWPA2
#define IS_WPA2_AUTH(auth)	((auth) == WPA2_AUTH_UNSPECIFIED || \
				 (auth) == WPA2_AUTH_PSK || \
				 (auth) == BRCM_AUTH_PSK || \
				 (auth) == BRCM_AUTH_DPT)
#define INCLUDES_WPA2_AUTH(auth) \
			((auth) & (WPA2_AUTH_UNSPECIFIED | \
				   WPA2_AUTH_PSK | \
				   BRCM_AUTH_PSK | \
				   BRCM_AUTH_DPT))
#endif /* BCMWPA2 */


#define IS_WPA_AKM(akm)	((akm) == RSN_AKM_NONE || \
				 (akm) == RSN_AKM_UNSPECIFIED || \
				 (akm) == RSN_AKM_PSK)
#ifdef BCMWPA2
#define IS_WPA2_AKM(akm)	((akm) == RSN_AKM_UNSPECIFIED || \
				 (akm) == RSN_AKM_PSK)
#endif /* BCMWPA2 */

/* Broadcom(OUI) authenticated key managment suite */
#define BRCM_AKM_NONE           0
#define BRCM_AKM_PSK            1       /* Proprietary PSK AKM */
#define BRCM_AKM_DPT            2       /* Proprietary DPT PSK AKM */

#define IS_BRCM_AKM(akm)        ((akm) == BRCM_AKM_PSK)

#define MAX_ARRAY 1
#define MIN_ARRAY 0

/* convert wsec to WPA mcast cipher. algo is needed only when WEP is enabled. */
#define WPA_MCAST_CIPHER(wsec, algo)	(WSEC_WEP_ENABLED(wsec) ? \
		((algo) == CRYPTO_ALGO_WEP128 ? WPA_CIPHER_WEP_104 : WPA_CIPHER_WEP_40) : \
			WSEC_TKIP_ENABLED(wsec) ? WPA_CIPHER_TKIP : \
			WSEC_AES_ENABLED(wsec) ? WPA_CIPHER_AES_CCM : \
			WPA_CIPHER_NONE)

/* Return address of max or min array depending first argument.
 * Return first array addr in case of a draw.
 */
extern uint8 *BCMROMFN(wpa_array_cmp)(int max_array, uint8 *x, uint8 *y, uint len);

/* Increment the array argument */
extern void BCMROMFN(wpa_incr_array)(uint8 *array, uint len);

/* Convert WPA IE cipher suite to locally used value */
extern bool BCMROMFN(wpa_cipher)(wpa_suite_t *suite, ushort *cipher, bool wep_ok);

/* Look for a WPA IE; return it's address if found, NULL otherwise */
extern wpa_ie_fixed_t *BCMROMFN(bcm_find_wpaie)(uint8 *parse, uint len);

/* Look for a WPS IE; return it's address if found, NULL otherwise */
extern wpa_ie_fixed_t *bcm_find_wpsie(uint8 *parse, uint len);

/* Check whether pointed-to IE looks like WPA. */
extern bool BCMROMFN(bcm_is_wpa_ie)(uint8 *ie, uint8 **tlvs, uint *tlvs_len);

/* Check whether pointed-to IE looks like WPS. */
extern bool bcm_is_wps_ie(uint8 *ie, uint8 **tlvs, uint *tlvs_len);

#ifdef BCMWPA2
/* Convert WPA2 IE cipher suite to locally used value */
extern bool BCMROMFN(wpa2_cipher)(wpa_suite_t *suite, ushort *cipher, bool wep_ok);

#if defined(BCMSUP_PSK) || defined(BCMSUPPL)
/* Look for an encapsulated GTK; return it's address if found, NULL otherwise */
extern eapol_wpa2_encap_data_t *BCMROMFN(wpa_find_gtk_encap)(uint8 *parse, uint len);

/* Check whether pointed-to IE looks like an encapsulated GTK. */
extern bool BCMROMFN(wpa_is_gtk_encap)(uint8 *ie, uint8 **tlvs, uint *tlvs_len);

/* Look for encapsulated key data; return it's address if found, NULL otherwise */
extern eapol_wpa2_encap_data_t *BCMROMFN(wpa_find_kde)(uint8 *parse, uint len, uint8 type);
#endif /* defined(BCMSUP_PSK) || defined(BCMSUPPL) */
#endif	/* BCMWPA2 */

#ifdef BCMSUP_PSK
/* Calculate a pair-wise transient key */
extern void BCMROMFN(wpa_calc_ptk)(struct ether_addr *auth_ea, struct ether_addr *sta_ea,
                                   uint8 *anonce, uint8* snonce, uint8 *pmk, uint pmk_len,
                                   uint8 *ptk, uint ptk_len);

/* Compute Message Integrity Code (MIC) over EAPOL message */
extern bool BCMROMFN(wpa_make_mic)(eapol_header_t *eapol, uint key_desc, uint8 *mic_key,
                                   uchar *mic);

/* Check MIC of EAPOL message */
extern bool BCMROMFN(wpa_check_mic)(eapol_header_t *eapol, uint key_desc, uint8 *mic_key);

#ifdef BCMWPA2
/* Calculate PMKID */
extern void BCMROMFN(wpa_calc_pmkid)(struct ether_addr *auth_ea, struct ether_addr *sta_ea,
                                     uint8 *pmk, uint pmk_len, uint8 *pmkid);
#endif /* BCMWPA2 */

/* Decrypt key data from a WPA key message */
extern bool BCMROMFN(wpa_decr_key_data)(eapol_wpa_key_header_t *body, uint16 key_info,
                                        uint8 *ekey, uint8 *gtk);

/* Decrypt a group transient key from a WPA key message */
extern bool BCMROMFN(wpa_decr_gtk)(eapol_wpa_key_header_t *body, uint16 key_info,
                                   uint8 *ekey, uint8 *gtk);
#endif	/* BCMSUP_PSK */

extern bool BCMROMFN(bcmwpa_akm2WPAauth)(uint8 *akm, uint32 *auth, bool sta_iswpa);

extern bool BCMROMFN(bcmwpa_cipher2wsec)(uint8 *cipher, uint32 *wsec);

#endif	/* _BCMWPA_H_ */
