/*
 *  Copyright (c) Atheros Communications 2004 All Rights Reserved
 */
#ifndef _JSW_PROTO
#define _JSW_PROTO

/*
 * EAPOL VP type
 */
#define JSW_EAP_VP_TYPE     0xFE

struct ath_eap_header {
#ifdef WORDS_BIGENDIAN
	u32	proto:6,
		version:4,
		unused:22;
#else
	u32	unused:22,
		version:4,
		proto:6;
#endif /* WORDS_BIGENDIAN */
};

#define ATHL2P_JUMPSTART_PROTO  1
#define JSW_VERSION             1

typedef enum {
	JSW_OP_JDS_DISCOVER = 1,
	JSW_OP_JDS_OFFER,
	JSW_OP_JDS_REQUEST,
	JSW_OP_JDS_ACK,
	JSW_OP_JSS_PASSWORD_REQ,
	JSW_OP_JSS_PASSWORD_RESP,
	JSW_OP_JSS_PASSWORD_ACK,
	JSW_OP_JSS_PMK_OFFER,
	JSW_OP_JSS_PMK_ACK,
} JSW_FRAME_OPS;

struct jsw_op_hdr {
	u16 op;
	u16 length; // includes payload beginning at op, above
};


/*
 * For wpa_eapol_key
 */
#define JSW_KEY_DESC                0x01
#define JSW_KEY_INFO_TYPE           0x02

/*
 * Nonce and salts are fixed size; So is "hashed" password
 */
#define JSW_NONCE_SIZE              32
#define JSW_SHA1_LEN                20

#define JSW_MAX_RETRIES             3
#endif /* _JSW_PROTO */
