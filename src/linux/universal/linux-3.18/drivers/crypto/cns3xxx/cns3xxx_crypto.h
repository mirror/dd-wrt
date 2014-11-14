#ifndef __CNS3XXX_CRYPTO_H__

#define AES_KEY_LEN	(8 * 4)

struct sec_accel_config {
	
	u32 config;
#define CFG_OP_MAC_ONLY		0
#define CFG_OP_CRYPT_ONLY	1
#define CFG_OP_MAC_CRYPT	2
#define CFG_OP_CRYPT_MAC	3
#define CFG_MACM_MD5		(4 << 4)
#define CFG_MACM_SHA1		(5 << 4)
#define CFG_MACM_HMAC_MD5	(6 << 4)
#define CFG_MACM_HMAC_SHA1	(7 << 4)
#define CFG_ENCM_DES		(1 << 8)
#define CFG_ENCM_3DES		(2 << 8)
#define CFG_ENCM_AES		(3 << 8)
#define CFG_DIR_ENC		(0 << 12)
#define CFG_DIR_DEC		(1 << 12)
#define CFG_ENC_MODE_ECB	(0 << 16)
#define CFG_ENC_MODE_CBC	(1 << 16)
#define CFG_3DES_EEE		(0 << 20)
#define CFG_3DES_EDE		(1 << 20)
#define CFG_AES_LEN_128		(0 << 24)
#define CFG_AES_LEN_192		(1 << 24)
#define CFG_AES_LEN_256		(2 << 24)
#define CFG_NOT_FRAG		(0 << 30)
#define CFG_FIRST_FRAG		(1 << 30)
#define CFG_LAST_FRAG		(2 << 30)
#define CFG_MID_FRAG		(3 << 30)

	u32 enc_p;
#define ENC_P_SRC(x)		(x)
#define ENC_P_DST(x)		((x) << 16)

	u32 enc_len;
#define ENC_LEN(x)		(x)

	u32 enc_key_p;
#define ENC_KEY_P(x)		(x)

	u32 enc_iv;
#define ENC_IV_POINT(x)		((x) << 0)
#define ENC_IV_BUF_POINT(x)	((x) << 16)

	u32 mac_src_p;
#define MAC_SRC_DATA_P(x)	(x)
#define MAC_SRC_TOTAL_LEN(x)	((x) << 16)

	u32 mac_digest;
#define MAC_DIGEST_P(x)	(x)
#define MAC_FRAG_LEN(x)	((x) << 16)
	u32 mac_iv;
#define MAC_INNER_IV_P(x)	(x)
#define MAC_OUTER_IV_P(x)	((x) << 16)
}__attribute__ ((packed));
	/*
	 * /-----------\ 0
	 * | ACCEL CFG |	4 * 8
	 * |-----------| 0x20
	 * | CRYPT KEY |	8 * 4
	 * |-----------| 0x40
	 * |  IV   IN  |	4 * 4
	 * |-----------| 0x40 (inplace)
	 * |  IV BUF   |	4 * 4
	 * |-----------| 0x80
	 * |  DATA IN  |	16 * x (max ->max_req_size)
	 * |-----------| 0x80 (inplace operation)
	 * |  DATA OUT |	16 * x (max ->max_req_size)
	 * \-----------/ SRAM size
	 */

	/* Hashing memory map:
	 * /-----------\ 0
	 * | ACCEL CFG |        4 * 8
	 * |-----------| 0x20
	 * | Inner IV  |        5 * 4
	 * |-----------| 0x34
	 * | Outer IV  |        5 * 4
	 * |-----------| 0x48
	 * | Output BUF|        5 * 4
	 * |-----------| 0x80
	 * |  DATA IN  |        64 * x (max ->max_req_size)
	 * \-----------/ SRAM size
	 */

#define IN_DATA_IV_P		0
#define IN_DATA_KEY_P		16
#define IN_DATA_P(key_len)	16+key_len

#define MAX_INCNT    32
#define MAX_OUTCNT   32
#define MAX_BUFCNT   MAX_INCNT


typedef void (*CallBackFn)(int, void *);

/*! \enum AesType AES128CBC AES192CBC AES256CBC*/
typedef enum
{ AES_128 = 5, AES_192 = 6, AES_256 = 7}
AesType;

typedef struct
{
  u16 bufcnt;
  u32 *bufptr[MAX_BUFCNT];
  u32 bufsize[MAX_BUFCNT];
}
Csp1ScatterBuffer, n1_scatter_buffer;



extern u64 Csp1AllocContext(void);
u64 n1_alloc_context(void *);
void n1_free_context(void *, u64 context);
#endif
