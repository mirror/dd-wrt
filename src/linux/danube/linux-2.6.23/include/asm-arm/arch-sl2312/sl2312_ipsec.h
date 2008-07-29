#ifndef _IPSEC_DIAG_H
#define _IPSEC_DIAG_H

#include <linux/scatterlist.h>

#define BIG_ENDIAN    0

#define IPSEC_TEST    0
#define ZERO_COPY     1

#define UINT unsigned int
#define BYTE unsigned char

/* define cipher algorithm */
enum CIPHER {
	DES_ECB_E	=20,
	TDES_ECB_E	=21,
	AES_ECB_E	=22,
	DES_CBC_E	=24,
	TDES_CBC_E	=25,
	AES_CBC_E	=26,

	DES_ECB_D	=27,
	TDES_ECB_D	=28,
	AES_ECB_D	=29,
	DES_CBC_D	=31,
	TDES_CBC_D	=32,
	AES_CBC_D	=33,
	A_SHA1      =12,
	A_HMAC_SHA1 =13,
	A_MD5       =14,
	A_HMAC_MD5  =15,
};

// opMode
#define CIPHER_ENC    0x1
#define CIPHER_DEC    0x3
#define AUTH          0x4
#define ENC_AUTH      0x5
#define AUTH_DEC      0x7

// cipherAlgorithm
#define CBC_DES       0x4
#define CBC_3DES      0x5
#define CBC_AES       0x6
#define ECB_DES       0x0
#define ECB_3DES      0x1
#define ECB_AES       0x2

// authAlgorithm
#define SHA1         0
#define MD5          1
#define HMAC_SHA1    2
#define HMAC_MD5     3
#define FCS          4

//cipher mode
#define ECB          0
#define CBC          1

// authMode
#define AUTH_APPEND  0
#define AUTH_CHKVAL  1

/******************************************************/
/*          the offset of IPSEC DMA register          */
/******************************************************/
enum IPSEC_DMA_REGISTER {
	IPSEC_DMA_DEVICE_ID		= 0xff00,
	IPSEC_DMA_STATUS		= 0xff04,
	IPSEC_TXDMA_CTRL 	 	= 0xff08,
	IPSEC_TXDMA_FIRST_DESC 	= 0xff0c,
	IPSEC_TXDMA_CURR_DESC	= 0xff10,
	IPSEC_RXDMA_CTRL		= 0xff14,
	IPSEC_RXDMA_FIRST_DESC	= 0xff18,
	IPSEC_RXDMA_CURR_DESC	= 0xff1c,
	IPSEC_TXDMA_BUF_ADDR    = 0xff28,
	IPSEC_RXDMA_BUF_ADDR    = 0xff38,
	IPSEC_RXDMA_BUF_SIZE		= 0xff30,
};

#define IPSEC_STATUS_REG    0x00a8
#define IPSEC_RAND_NUM_REG  0x00ac

/******************************************************/
/* the field definition of IPSEC DMA Module Register  */
/******************************************************/
typedef union
{
	unsigned int bits32;
	struct bit2_ff00
	{
#if (BIG_ENDIAN==1)
		unsigned int p_wclk 		:  4;	/* DMA_APB write clock period */
		unsigned int p_rclk 		:  4;	/* DMA_APB read clock period */
		unsigned int 				:  8;
		unsigned int device_id		: 12;
		unsigned int revision_id	:  4;
#else
		unsigned int revision_id	:  4;
		unsigned int device_id		: 12;
		unsigned int 				:  8;
		unsigned int p_rclk 		:  4;	/* DMA_APB read clock period */
		unsigned int p_wclk	    	:  4;	/* DMA_APB write clock period */
#endif
	} bits;
} IPSEC_DMA_DEVICE_ID_T;

typedef union
{
	unsigned int bits32;
	struct bit2_ff04
	{
#if (BIG_ENDIAN==1)
		unsigned int ts_finish		:  1;	/* finished tx interrupt */
		unsigned int ts_derr		:  1;   /* AHB Bus Error while tx */
		unsigned int ts_perr		:  1;   /* Tx Descriptor protocol error */
		unsigned int ts_eodi		:  1;	/* TxDMA end of descriptor interrupt */
		unsigned int ts_eofi		:  1;   /* TxDMA end of frame interrupt */
		unsigned int rs_finish		:  1;   /* finished rx interrupt */
		unsigned int rs_derr		:  1;   /* AHB Bus Error while rx */
		unsigned int rs_perr		:  1;   /* Rx Descriptor protocol error */
		unsigned int rs_eodi		:  1;	/* RxDMA end of descriptor interrupt */
		unsigned int rs_eofi		:  1;	/* RxDMA end of frame interrupt */
        unsigned int intr           :  8;   /* Peripheral interrupt */
		unsigned int dma_reset 		:  1;	/* write 1 to this bit will cause DMA HClk domain soft reset */
		unsigned int peri_reset    	:  1;   /* write 1 to this bit will cause DMA PClk domain soft reset */
		unsigned int 				:  3;
		unsigned int loop_back		:  1;	/* loopback TxDMA to RxDMA */
        unsigned int intr_enable    :  8;   /* Peripheral Interrupt Enable */
#else
        unsigned int intr_enable    :  8;   /* Peripheral Interrupt Enable */
		unsigned int loop_back		:  1;	/* loopback TxDMA to RxDMA */
		unsigned int 				:  3;
		unsigned int peri_reset    	:  1;   /* write 1 to this bit will cause DMA PClk domain soft reset */
		unsigned int dma_reset 		:  1;	/* write 1 to this bit will cause DMA HClk domain soft reset */
        unsigned int intr           :  8;   /* Peripheral interrupt */
		unsigned int rs_eofi		:  1;	/* RxDMA end of frame interrupt */
		unsigned int rs_eodi		:  1;	/* RxDMA end of descriptor interrupt */
		unsigned int rs_perr		:  1;   /* Rx Descriptor protocol error */
		unsigned int rs_derr		:  1;   /* AHB Bus Error while rx */
		unsigned int rs_finish		:  1;   /* finished rx interrupt */
		unsigned int ts_eofi		:  1;   /* TxDMA end of frame interrupt */
		unsigned int ts_eodi		:  1;	/* TxDMA end of descriptor interrupt */
		unsigned int ts_perr		:  1;   /* Tx Descriptor protocol error */
		unsigned int ts_derr		:  1;   /* AHB Bus Error while tx */
		unsigned int ts_finish		:  1;	/* finished tx interrupt */
#endif
	} bits;
} IPSEC_DMA_STATUS_T;

typedef union
{
	unsigned int bits32;
	struct bit2_ff08
	{
#if (BIG_ENDIAN==1)
		unsigned int td_start		:  1;	/* Start DMA transfer */
		unsigned int td_continue	:  1;   /* Continue DMA operation */
		unsigned int td_chain_mode	:  1;	/* Descriptor Chain Mode;1-Descriptor Chain mode, 0-Direct DMA mode*/
		unsigned int 				:  1;
		unsigned int td_prot		:  4;	/* TxDMA protection control */
		unsigned int td_burst_size  :  2;	/* TxDMA max burst size for every AHB request */
		unsigned int td_bus		    :  2;	/* peripheral bus width;0 - 8 bits;1 - 16 bits */
		unsigned int td_endian		:  1;	/* AHB Endian. 0-little endian; 1-big endian */
		unsigned int td_finish_en   :  1;	/* DMA Finish Event Interrupt Enable;1-enable;0-mask */
		unsigned int td_fail_en 	:  1;	/* DMA Fail Interrupt Enable;1-enable;0-mask */
		unsigned int td_perr_en 	:  1;	/* Protocol Failure Interrupt Enable;1-enable;0-mask */
		unsigned int td_eod_en  	:  1;	/* End of Descriptor interrupt Enable;1-enable;0-mask */
		unsigned int td_eof_en      :  1;   /* End of frame interrupt Enable;1-enable;0-mask */
		unsigned int 				: 14;
#else
		unsigned int 				: 14;
		unsigned int td_eof_en      :  1;   /* End of frame interrupt Enable;1-enable;0-mask */
		unsigned int td_eod_en  	:  1;	/* End of Descriptor interrupt Enable;1-enable;0-mask */
		unsigned int td_perr_en 	:  1;	/* Protocol Failure Interrupt Enable;1-enable;0-mask */
		unsigned int td_fail_en 	:  1;	/* DMA Fail Interrupt Enable;1-enable;0-mask */
		unsigned int td_finish_en   :  1;	/* DMA Finish Event Interrupt Enable;1-enable;0-mask */
		unsigned int td_endian		:  1;	/* AHB Endian. 0-little endian; 1-big endian */
		unsigned int td_bus		    :  2;	/* peripheral bus width;0 - 8 bits;1 - 16 bits */
		unsigned int td_burst_size  :  2;	/* TxDMA max burst size for every AHB request */
		unsigned int td_prot		:  4;	/* TxDMA protection control */
		unsigned int 				:  1;
		unsigned int td_chain_mode	:  1;	/* Descriptor Chain Mode;1-Descriptor Chain mode, 0-Direct DMA mode*/
		unsigned int td_continue	:  1;   /* Continue DMA operation */
		unsigned int td_start		:  1;	/* Start DMA transfer */
#endif
	} bits;
} IPSEC_TXDMA_CTRL_T;

typedef union
{
	unsigned int bits32;
	struct bit2_ff0c
	{
#if (BIG_ENDIAN==1)
		unsigned int td_first_des_ptr	: 28;/* first descriptor address */
		unsigned int td_busy			:  1;/* 1-TxDMA busy; 0-TxDMA idle */
		unsigned int 					:  3;
#else
		unsigned int 					:  3;
		unsigned int td_busy			:  1;/* 1-TxDMA busy; 0-TxDMA idle */
		unsigned int td_first_des_ptr	: 28;/* first descriptor address */
#endif
	} bits;
} IPSEC_TXDMA_FIRST_DESC_T;

typedef union
{
	unsigned int bits32;
	struct bit2_ff10
	{
#if (BIG_ENDIAN==1)
		unsigned int ndar			: 28;	/* next descriptor address */
		unsigned int eofie			:  1;	/* end of frame interrupt enable */
		unsigned int dec			:  1;	/* AHB bus address increment(0)/decrement(1) */
		unsigned int sof_eof		:  2;
#else
		unsigned int sof_eof		:  2;
		unsigned int dec			:  1;	/* AHB bus address increment(0)/decrement(1) */
		unsigned int eofie			:  1;	/* end of frame interrupt enable */
		unsigned int ndar			: 28;	/* next descriptor address */
#endif
	} bits;
} IPSEC_TXDMA_CURR_DESC_T;


typedef union
{
	unsigned int bits32;
	struct bit2_ff14
	{
#if (BIG_ENDIAN==1)
		unsigned int rd_start		:  1;	/* Start DMA transfer */
		unsigned int rd_continue	:  1;   /* Continue DMA operation */
		unsigned int rd_chain_mode	:  1;	/* Descriptor Chain Mode;1-Descriptor Chain mode, 0-Direct DMA mode*/
		unsigned int 				:  1;
		unsigned int rd_prot		:  4;	/* DMA protection control */
		unsigned int rd_burst_size  :  2;	/* DMA max burst size for every AHB request */
		unsigned int rd_bus		    :  2;	/* peripheral bus width;0 - 8 bits;1 - 16 bits */
		unsigned int rd_endian		:  1;	/* AHB Endian. 0-little endian; 1-big endian */
		unsigned int rd_finish_en   :  1;	/* DMA Finish Event Interrupt Enable;1-enable;0-mask */
		unsigned int rd_fail_en  	:  1;	/* DMA Fail Interrupt Enable;1-enable;0-mask */
		unsigned int rd_perr_en 	:  1;	/* Protocol Failure Interrupt Enable;1-enable;0-mask */
		unsigned int rd_eod_en  	:  1;	/* End of Descriptor interrupt Enable;1-enable;0-mask */
		unsigned int rd_eof_en      :  1;   /* End of frame interrupt Enable;1-enable;0-mask */
		unsigned int 				: 14;
#else
		unsigned int 				: 14;
		unsigned int rd_eof_en      :  1;   /* End of frame interrupt Enable;1-enable;0-mask */
		unsigned int rd_eod_en  	:  1;	/* End of Descriptor interrupt Enable;1-enable;0-mask */
		unsigned int rd_perr_en 	:  1;	/* Protocol Failure Interrupt Enable;1-enable;0-mask */
		unsigned int rd_fail_en  	:  1;	/* DMA Fail Interrupt Enable;1-enable;0-mask */
		unsigned int rd_finish_en   :  1;	/* DMA Finish Event Interrupt Enable;1-enable;0-mask */
		unsigned int rd_endian		:  1;	/* AHB Endian. 0-little endian; 1-big endian */
		unsigned int rd_bus		    :  2;	/* peripheral bus width;0 - 8 bits;1 - 16 bits */
		unsigned int rd_burst_size  :  2;	/* DMA max burst size for every AHB request */
		unsigned int rd_prot		:  4;	/* DMA protection control */
		unsigned int 				:  1;
		unsigned int rd_chain_mode	:  1;	/* Descriptor Chain Mode;1-Descriptor Chain mode, 0-Direct DMA mode*/
		unsigned int rd_continue	:  1;   /* Continue DMA operation */
		unsigned int rd_start		:  1;	/* Start DMA transfer */
#endif
	} bits;
} IPSEC_RXDMA_CTRL_T;

typedef union
{
	unsigned int bits32;
	struct bit2_ff18
	{
#if (BIG_ENDIAN==1)
		unsigned int rd_first_des_ptr	: 28;/* first descriptor address */
		unsigned int rd_busy			:  1;/* 1-RxDMA busy; 0-RxDMA idle */
		unsigned int 					:  3;
#else
		unsigned int 					:  3;
		unsigned int rd_busy			:  1;/* 1-RxDMA busy; 0-RxDMA idle */
		unsigned int rd_first_des_ptr	: 28;/* first descriptor address */
#endif
	} bits;
} IPSEC_RXDMA_FIRST_DESC_T;

typedef union
{
	unsigned int bits32;
	struct bit2_ff1c
	{
#if (BIG_ENDIAN==1)
		unsigned int ndar			: 28;	/* next descriptor address */
		unsigned int eofie			:  1;	/* end of frame interrupt enable */
		unsigned int dec			:  1;	/* AHB bus address increment(0)/decrement(1) */
		unsigned int sof_eof		:  2;
#else
		unsigned int sof_eof		:  2;
		unsigned int dec			:  1;	/* AHB bus address increment(0)/decrement(1) */
		unsigned int eofie			:  1;	/* end of frame interrupt enable */
		unsigned int ndar			: 28;	/* next descriptor address */
#endif
	} bits;
} IPSEC_RXDMA_CURR_DESC_T;



/******************************************************/
/*    the field definition of IPSEC module Register   */
/******************************************************/
typedef union
{
	unsigned int id;
	struct bit_0000
	{
#if (BIG_ENDIAN==1)
		unsigned int device_id		: 28;
		unsigned int revision_id	:  4;
#else
		unsigned int revision_id	:  4;
		unsigned int device_id		: 28;
#endif
	} bits;
} IPSEC_ID_T;

typedef union
{
    unsigned int control;
    struct bit_0004
    {
#if (BIG_ENDIAN==1)
        unsigned int op_mode            :  4; /* Operation Mode for the IPSec Module */
        unsigned int                    :  1;
        unsigned int cipher_algorithm   :  3;
        unsigned int aesnk              :  4; /* AES Key Size */
        unsigned int mix_key_sel        :  1; /* 0:use rCipherKey0-3  1:use Key Mixer */
        unsigned int                    :  2;
        unsigned int fcs_stream_copy    :  1; /* enable authentication stream copy */
        unsigned int auth_mode          :  1; /* 0-Append or 1-Check Authentication Result */
        unsigned int auth_algorithm     :  3;
        unsigned int                    :  1;
        unsigned int auth_check_len     :  3; /* Number of 32-bit words to be check or appended */
                                              /* by the authentication module */
        unsigned int process_id         :  8; /* Used to identify process.This number will be */
                                              /* copied to the descriptor status of received packet*/
#else
        unsigned int process_id         :  8; /* Used to identify process.This number will be */
                                              /* copied to the descriptor status of received packet*/
        unsigned int auth_check_len     :  3; /* Number of 32-bit words to be check or appended */
                                              /* by the authentication module */
        unsigned int                    :  1;
        unsigned int auth_algorithm     :  3;
        unsigned int auth_mode          :  1; /* 0-Append or 1-Check Authentication Result */
        unsigned int fcs_stream_copy    :  1; /* enable authentication stream copy */
        unsigned int                    :  2;
        unsigned int mix_key_sel        :  1; /* 0:use rCipherKey0-3  1:use Key Mixer */
        unsigned int aesnk              :  4; /* AES Key Size */
        unsigned int cipher_algorithm   :  3;
        unsigned int                    :  1;
        unsigned int op_mode            :  4; /* Operation Mode for the IPSec Module */
#endif
    } bits;
} IPSEC_CONTROL_T;


typedef union
{
    unsigned int cipher_packet;
    struct bit_0008
    {
#if (BIG_ENDIAN==1)
        unsigned int cipher_header_len    : 16; /* The header length to be skipped by the cipher */
        unsigned int cipher_algorithm_len : 16; /* The length of message body to be encrypted/decrypted */
#else
        unsigned int cipher_algorithm_len : 16; /* The length of message body to be encrypted/decrypted */
        unsigned int cipher_header_len    : 16; /* The header length to be skipped by the cipher */
#endif
    } bits;
} IPSEC_CIPHER_PACKET_T;

typedef union
{
    unsigned int auth_packet;
    struct bit_000c
    {
#if (BIG_ENDIAN==1)
        unsigned int auth_header_len    : 16; /* The header length that is to be skipped by the authenticator */
        unsigned int auth_algorithm_len : 16; /* The length of message body that is to be authenticated */
#else
        unsigned int auth_algorithm_len : 16; /* The length of message body that is to be authenticated */
        unsigned int auth_header_len    : 16; /* The header length that is to be skipped by the authenticator */
#endif
    } bits;
} IPSEC_AUTH_PACKET_T;

typedef union
{
    unsigned int status;
    struct bit_00a8
    {
#if (BIG_ENDIAN==1)
        unsigned int auth_cmp_rslt  :  1; /* Authentication Compare result */
        unsigned int wep_crc_ok     :  1; /* WEP ICV compare result */
        unsigned int tkip_mic_ok    :  1; /* TKIP Mic compare result */
        unsigned int ccm_mic_ok     :  1; /* CCM Mic compare result */
        unsigned int                : 16;
        unsigned int parser_err_code:  4; /* Authentication Compare result */
        unsigned int auth_err_code  :  4; /* Authentication module error code */
        unsigned int cipher_err_code:  4; /* Cipher module erroe code */
#else
        unsigned int cipher_err_code:  4; /* Cipher module erroe code */
        unsigned int auth_err_code  :  4; /* Authentication module error code */
        unsigned int parser_err_code:  4; /* Authentication Compare result */
        unsigned int                : 16;
        unsigned int ccm_mic_ok     :  1; /* CCM Mic compare result */
        unsigned int tkip_mic_ok    :  1; /* TKIP Mic compare result */
        unsigned int wep_crc_ok     :  1; /* WEP ICV compare result */
        unsigned int auth_cmp_rslt  :  1; /* Authentication Compare result */
#endif
    } bits;
} IPSEC_STATUS_T;



/************************************************************************/
/*              IPSec Descriptor Format                                 */
/************************************************************************/
typedef struct descriptor_t
{
	union frame_control_t
	{
		unsigned int bits32;
		struct bits_0000
		{
#if (BIG_ENDIAN==1)
			unsigned int own 		: 1;	/* owner bit. 0-CPU, 1-DMA */
			unsigned int derr		: 1;	/* data error during processing this descriptor */
			unsigned int perr		: 1;	/* protocol error during processing this descriptor */
			unsigned int            : 1;    /* authentication compare result */
			unsigned int            : 6;    /* checksum[15:8] */
			unsigned int desc_count : 6;	/* number of descriptors used for the current frame */
			unsigned int buffer_size:16;	/* transfer buffer size associated with current description*/
#else
			unsigned int buffer_size:16;	/* transfer buffer size associated with current description*/
			unsigned int desc_count : 6;	/* number of descriptors used for the current frame */
			unsigned int            : 6;    /* checksum[15:8] */
			unsigned int            : 1;    /* authentication compare result */
			unsigned int perr		: 1;	/* protocol error during processing this descriptor */
			unsigned int derr		: 1;	/* data error during processing this descriptor */
			unsigned int own 		: 1;	/* owner bit. 0-CPU, 1-DMA */
#endif
		} bits;
	} frame_ctrl;

	union flag_status_t
	{
		unsigned int bits32;
		struct bits_0004
		{
#if (BIG_ENDIAN==1)
//            unsigned int checksum   : 8; /* checksum[7:0] */
			unsigned int            : 4;
			unsigned int auth_result: 1;
			unsigned int wep_crc_ok : 1;
			unsigned int tkip_mic_ok: 1;
			unsigned int ccmp_mic_ok: 1;
			unsigned int process_id : 8;
			unsigned int frame_count:16;
#else
			unsigned int frame_count:16;
			unsigned int process_id : 8;
			unsigned int ccmp_mic_ok: 1;
			unsigned int tkip_mic_ok: 1;
			unsigned int wep_crc_ok : 1;
			unsigned int auth_result: 1;
			unsigned int            : 4;
//            unsigned int checksum   : 8; /* checksum[7:0] */
#endif
		} bits_rx_status;

		struct bits_0005
		{
#if (BIG_ENDIAN==1)
            unsigned int            : 8;
			unsigned int process_id : 8;
			unsigned int frame_count:16;
#else
			unsigned int frame_count:16;
			unsigned int process_id : 8;
            unsigned int            : 8;
#endif
		} bits_tx_status;

		struct bits_0006
		{
#if (BIG_ENDIAN==1)
			unsigned int            :22;
			unsigned int tqflag     :10;
#else
			unsigned int tqflag     :10;
			unsigned int            :22;
#endif
		} bits_tx_flag;
	} flag_status;

	unsigned int buf_adr;	/* data buffer address */

	union next_desc_t
	{
		unsigned int next_descriptor;
		struct bits_000c
		{
#if (BIG_ENDIAN==1)
			unsigned int ndar		:28;	/* next descriptor address */
			unsigned int eofie		: 1;	/* end of frame interrupt enable */
			unsigned int dec   		: 1;	/* AHB bus address. 0-increment; 1-decrement */
			unsigned int sof_eof	: 2;	/* 00-the linking descriptor   01-the last descriptor of a frame*/
			                                /* 10-the first descriptor of a frame    11-only one descriptor for a frame*/
#else
			unsigned int sof_eof	: 2;	/* 00-the linking descriptor   01-the last descriptor of a frame*/
			                                /* 10-the first descriptor of a frame    11-only one descriptor for a frame*/
			unsigned int dec   		: 1;	/* AHB bus address. 0-increment; 1-decrement */
			unsigned int eofie		: 1;	/* end of frame interrupt enable */
			unsigned int ndar		:28;	/* next descriptor address */
#endif
		} bits;
	} next_desc;
} IPSEC_DESCRIPTOR_T;


typedef struct IPSEC_S
{
    unsigned char       *tx_bufs;
    unsigned char       *rx_bufs;
	IPSEC_DESCRIPTOR_T	*tx_desc;	    /* point to virtual TX descriptor address*/
	IPSEC_DESCRIPTOR_T	*rx_desc;	    /* point to virtual RX descriptor address*/
	IPSEC_DESCRIPTOR_T	*tx_cur_desc;	/* point to current TX descriptor */
	IPSEC_DESCRIPTOR_T	*rx_cur_desc;	/* point to current RX descriptor */
	IPSEC_DESCRIPTOR_T  *tx_finished_desc;
	IPSEC_DESCRIPTOR_T  *rx_finished_desc;
	dma_addr_t          rx_desc_dma;	/* physical RX descriptor address */
	dma_addr_t          tx_desc_dma;    /* physical TX descriptor address */
	dma_addr_t          rx_bufs_dma;    /* physical RX descriptor address */
	dma_addr_t          tx_bufs_dma;    /* physical TX descriptor address */
} IPSEC_T;


/*=====================================================================================================*/
/*  Data Structure of IPSEC Control Packet  */
/*=====================================================================================================*/
typedef struct IPSEC_ECB_AUTH_S
{
    IPSEC_CONTROL_T         control; /* control parameter */
    IPSEC_CIPHER_PACKET_T   cipher; /* cipher packet parameter */
    IPSEC_AUTH_PACKET_T     auth;   /* authentication packet parameter */
    unsigned char           cipher_key[8*4];
    unsigned char           auth_check_val[5*4];
} IPSEC_ECB_AUTH_T;

typedef struct IPSEC_CBC_AUTH_S
{
    IPSEC_CONTROL_T         control; /* control parameter */
    IPSEC_CIPHER_PACKET_T   cipher; /* cipher packet parameter */
    IPSEC_AUTH_PACKET_T     auth;   /* authentication packet parameter */
    unsigned char           cipher_iv[4*4];
    unsigned char           cipher_key[8*4];
    unsigned char           auth_check_val[5*4];
} IPSEC_CBC_AUTH_T;

typedef struct IPSEC_ECB_HMAC_AUTH_S
{
    IPSEC_CONTROL_T         control; /* control parameter */
    IPSEC_CIPHER_PACKET_T   cipher; /* cipher packet parameter */
    IPSEC_AUTH_PACKET_T     auth;   /* authentication packet parameter */
    unsigned char           cipher_key[8*4];
    unsigned char           auth_key[16*4];
    unsigned char           auth_check_val[5*4];
} IPSEC_ECB_AUTH_HMAC_T;

typedef struct IPSEC_CBC_HMAC_AUTH_S
{
    IPSEC_CONTROL_T         control; /* control parameter */
    IPSEC_CIPHER_PACKET_T   cipher; /* cipher packet parameter */
    IPSEC_AUTH_PACKET_T     auth;   /* authentication packet parameter */
    unsigned char           cipher_iv[4*4];
    unsigned char           cipher_key[8*4];
    unsigned char           auth_key[16*4];
    unsigned char           auth_check_val[5*4];
} IPSEC_CBC_AUTH_HMAC_T;

typedef struct IPSEC_HMAC_AUTH_S
{
    IPSEC_CONTROL_T         control; /* control parameter */
    IPSEC_AUTH_PACKET_T     auth;   /* authentication packet parameter */
    unsigned char           auth_key[16*4];
    unsigned char           auth_check_val[5*4];
} IPSEC_HMAC_AUTH_T;

typedef union
{
    unsigned char auth_pkt[28];

    struct IPSEC_AUTH_S
    {
        IPSEC_CONTROL_T         control; /* control parameter(4-byte) */
        IPSEC_AUTH_PACKET_T     auth;   /* authentication packet parameter(4-byte) */
        unsigned char           auth_check_val[5*4];
    } var;
} IPSEC_AUTH_T;

typedef struct IPSEC_CIPHER_CBC_S
{
    IPSEC_CONTROL_T         control; /* control parameter */
    IPSEC_CIPHER_PACKET_T   cipher; /* cipher packet parameter */
    unsigned char           cipher_iv[4*4];
    unsigned char           cipher_key[8*4];
} IPSEC_CIPHER_CBC_T;

typedef struct IPSEC_CIPHER_ECB_S
{
    IPSEC_CONTROL_T         control; /* control parameter */
    IPSEC_CIPHER_PACKET_T   cipher; /* cipher packet parameter */
    unsigned char           cipher_key[8*4];
} IPSEC_CIPHER_ECB_T;


/****************************************************************************
 *                          Structure Definition                            *
 ****************************************************************************/
struct IPSEC_PACKET_S
{
    unsigned int    op_mode;            /* CIPHER_ENC(1),CIPHER_DEC(3),AUTH(4),ENC_AUTH(5),AUTH_DEC(7) */
    unsigned int    cipher_algorithm;   /* ECB_DES(0),ECB_3DES(1),ECB_AES(2),CBC_DES(4),CBC_3DES(5),CBC_AES(6) */
    unsigned int    auth_algorithm;     /* SHA1(0),MD5(1),HMAC_SHA1(2),HMAC_MD5(3),FCS(4) */
    unsigned int    auth_result_mode;   /* AUTH_APPEND(0),AUTH_CHKVAL(1) */
    unsigned int    process_id;         /* Used to identify the process */
    unsigned int    auth_header_len;    /* Header length to be skipped by the authenticator */
    unsigned int    auth_algorithm_len; /* Length of message body that is to be authenticated */
    unsigned int    cipher_header_len;  /* Header length to be skipped by the cipher */
    unsigned int    cipher_algorithm_len;   /* Length of message body to be encrypted or decrypted */
    unsigned char   iv[16];             /* Initial vector used for DES,3DES,AES */
    unsigned int    iv_size;            /* Initial vector size */
    unsigned char   auth_key[64];       /* authentication key */
    unsigned int    auth_key_size;      /* authentication key size */
    unsigned char   cipher_key[32];     /* cipher key */
    unsigned int    cipher_key_size;    /* cipher key size */
    struct scatterlist *in_packet;         /* input_packet buffer pointer */
    //unsigned char		*in_packet;         /* input_packet buffer pointer */
    unsigned int    pkt_len;            /* input total packet length */
    unsigned char   auth_checkval[20];  /* Authentication check value/FCS check value */
    struct IPSEC_PACKET_S *next,*prev;        /* pointer to next/previous operation to perform on buffer */
    void (*callback)(struct IPSEC_PACKET_S *); /* function to call when done authentication/cipher */
    unsigned char   *out_packet;        /* output_packet buffer pointer */
    //struct scatterlist *out_packet;        /* output_packet buffer pointer */
    unsigned int    out_pkt_len;        /* output total packet length */
    unsigned int    auth_cmp_result;    /* authentication compare result */
    unsigned int    checksum;           /* checksum value */
    unsigned int    status;             /* ipsec return status. 0:success, others:fail */
#if (IPSEC_TEST == 1)
    unsigned char    *sw_packet;         /* for test only */
    unsigned int    sw_pkt_len;         /* for test only */
#endif
} ;

/*****************************************************************************
 * Function    : ipsec_crypto_hw_process
 * Description : This function processes H/W authentication and cipher.
 *       Input : op_info - the authentication and cipher information for IPSec module.
 *      Output : none.
 *      Return : 0 - success, others - failure.
 *****************************************************************************/
int ipsec_crypto_hw_process(struct IPSEC_PACKET_S  *op_info);

int ipsec_get_cipher_algorithm(unsigned char *alg_name,unsigned int alg_mode);
int ipsec_get_auth_algorithm(unsigned char *alg_name,unsigned int alg_mode);
#if 0
void ipsec_sw_authentication(char *data,unsigned int data_len,char *authkey,char authAlgorithm,char *auth_result);
void ipsec_sw_cipher(unsigned char *pt,unsigned int pt_len, unsigned char *cipher_key, unsigned int key_size,
                            unsigned char *iv,unsigned int cipherAlgorithm,unsigned char *ct);
void ipsec_sw_auth_cipher(unsigned int op_mode,char *data,unsigned int data_len,
                                BYTE *auth_key,char authAlgorithm,char *auth_result,
                                char *pt, unsigned int pt_len,char *cipher_key, int key_size,
                                char *iv, char cipherAlgorithm,char *ct);
#endif


#endif
