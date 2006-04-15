/*
 *  Copyright (c) 2002, Atheros Communications Inc., All Rights Reserved
 */

#ifndef EAP_JS_H
#define EAP_JS_H

/* data format for JS EAPOL key frames */
struct key_data {
	u8 elementID;
	u8 length;
	u8 oui[3];
	u8 type;
	u8 data[1];
} __attribute__ ((packed));

enum {
	KEYDATA_RESVD = 0,
	KEYDATA_GRPKEY,
	KEYDATA_STAKEY,
	KEYDATA_MACADDR,
	KEYDATA_PMKID,

	KEYDATA_DH_PARAM_G,
	KEYDATA_DH_PARAM_P,
	KEYDATA_DH_PUB_KEY,
	KEYDATA_SHA1_PASSWORD,
	KEYDATA_SALT,
	KEYDATA_IDENT,
};

/*
 * EAP Vendor Private header
 */
struct eap_vp_header {
	u8     code;
	u8     identifier;
	u16    length;
	u8     type;
	u8     smiOID[3];
	u8     data[1];
} __attribute__ ((packed));

#endif /* EAP_JS_H */
