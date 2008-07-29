/*
*  linux/include/asm-arm/xor.h
*
*  Copyright (C) 2001 Storlink Semi.
* 	Jason Lee <jason@storlink.com.tw>
*
*/
#include <asm/arch/sl2312.h>
#include <asm/io.h>
//#include <linux/compatmac.h>

#undef BIG_ENDIAN
#define CPU 		0
#define DMA 		1

#define DESC_NO	8
#define TX_DESC_NUM		DESC_NO
#define RX_DESC_NUM		DESC_NO

#define RAID_BASE_ADDR	IO_ADDRESS(SL2312_RAID_BASE)

#define SRAM_PAR_0k		0
#define SRAM_PAR_4k		1
#define SRAM_PAR_8k		2
#define SRAM_PAR_16k		3
#define SRAM_PAR_SIZE	SRAM_PAR_8k

#define RUNNING	 	0x1
#define COMPLETE 	0x2
#define ERROR 		0x4

#define CMD_XOR		0x0
#define CMD_FILL	0x1
#define CMD_CPY		0x3
#define CMD_CHK		0x4

enum RAID_DMA_REGISTER {
	RAID_DMA_DEVICE_ID		= 0xff00,
	RAID_DMA_STATUS			= 0xff04,
	RAID_FCHDMA_CTRL		= 0xff08,
	RAID_FCHDMA_FIRST_DESC	= 0xff0C,
	RAID_FCHDMA_CURR_DESC	= 0xff10,
	RAID_STRDMA_CTRL		= 0xff14,
	RAID_STRDMA_FIRST_DESC	= 0xff18,
	RAID_STRDMA_CURR_DESC	= 0xff1C,
	RAID_TX_FLG_REG			= 0xff24,
	RAID_RX_FLG_REG			= 0xff34,
	RAID_PCR				= 0xff50,
	SMC_CMD_REG				= 0xff60,
	SMC_STATUS_REG			= 0xff64
	};

enum RAID_FUNC_MODE {
	RAID_XOR			= 0,
	RAID_MIX			= 2,
	RAID_SRAM			= 3,
	RAID_ENDIAN			= 4,
	RAID_MEM_BLK		= 5,
	RAID_MEM2MEM		= 7,
	RAID_BUF_SIZE		= 8,
	RAID_ERR_TEST		= 9,
	RAID_BURST			= 10,
	RAID_BUS			= 11
	};

typedef struct reg_info {
	int mask;
	char err[32];
	int offset;
} REG_INFO;

/********************************************************/
/* 	the definition of RAID DMA Module Register      */
/********************************************************/
typedef union
{
	unsigned int bit32;
	struct bits_ff00
	{
		#ifdef BIG_ENDIAN
		unsigned int  				: 8;
		unsigned int teytPerr		: 4; /* define protocol error under tsPErrI*/
		unsigned int reytPerr		: 14; /* define protocol error under rsPErrI */
		unsigned int device_id		: 12;
		unsigned int revision_id	: 4;
		#else
		unsigned int revision_id	: 4;
		unsigned int device_id		: 12;
		unsigned int reytPerr		: 14; /* define protocol error under rsPErrI */
		unsigned int teytPerr		: 4; /* define protocol error under tsPErrI*/
		unsigned int 				: 8;
		#endif
	} bits;
} RAID_DMA_DEVICE_ID_T;

typedef union
{
	unsigned int bits32;
	struct bits_ff04
	{
		#ifdef BIG_ENDIAN
		unsigned int tsFinishI		: 1; /* owner bit error interrupt */
		unsigned int tsDErrI		: 1; /* AHB bus error interrupt */
		unsigned int tsPErrI		: 1; /* RAID XOR fetch descriptor protocol error interrupt */
		unsigned int tsEODI			: 1; /* RAID XOR fetch DMA end of descriptor interrupt */
		unsigned int tsEOFI			: 1; /* RAID XOR fetch DMA end of frame interrupt */
		unsigned int rsFinishI		: 1; /* owner bit error interrupt */
		unsigned int rsDErrI 		: 1; /* AHB bus error while RAID XOR store interrupt */
		unsigned int rsPErrI		: 1; /* RAID XOR store descriptor protocol error interrupt */
		unsigned int rsEODI			: 1; /* RAID XOR store DMA end of descriptor interrupt */
		unsigned int rsEOFI			: 1; /* RAID XOR store DMA end of frame interrupt */
		unsigned int inter			: 8; /* pattern check error interrupt */
		unsigned int 				: 5;
		unsigned int Loopback		: 1; /* loopback */
		unsigned int intEnable		: 8; /*pattern check error interrupt enable */
		#else
		unsigned int intEnable		: 8; /*pattern check error interrupt enable */
		unsigned int Loopback		: 1; /* loopback */
		unsigned int 				: 5;
		unsigned int inter			: 8; /* pattern check error interrupt */
		unsigned int rsEOFI			: 1; /* RAID XOR store DMA end of frame interrupt */
		unsigned int rsEODI			: 1; /* RAID XOR store DMA end of descriptor interrupt */
		unsigned int rsPErrI		: 1; /* RAID XOR store descriptor protocol error interrupt */
		unsigned int rsDErrI 		: 1; /* AHB bus error while RAID XOR store interrupt */
		unsigned int rsFinishI		: 1; /* owner bit error interrupt */
		unsigned int tsEOFI			: 1; /* RAID XOR fetch DMA end of frame interrupt */
		unsigned int tsEODI			: 1; /* RAID XOR fetch DMA end of descriptor interrupt */
		unsigned int tsPErrI		: 1; /* RAID XOR fetch descriptor protocol error interrupt */
		unsigned int tsDErrI		: 1; /* AHB bus error interrupt */
		unsigned int tsFinishI		: 1; /* owner bit error interrupt */
		#endif
	} bits;
} RAID_DMA_STATUS_T;


typedef union
{
	unsigned int bits32;
	struct bits_ff08
	{
		#ifdef BIG_ENDIAN
		unsigned int td_start		:  1;	/* Start DMA transfer */
		unsigned int td_continue	:  1;   /* Continue DMA operation */
		unsigned int td_chain_mode	:  1;	/* Descriptor Chain Mode;1-Descriptor Chain mode, 0-Direct DMA mode*/
		unsigned int 				:  1;
		unsigned int td_prot		:  4;	/* DMA protection control */
		unsigned int td_burst_size  :  2;	/* DMA max burst size for every AHB request */
		unsigned int td_bus		    :  2;	/* peripheral bus width */
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
} RAID_TXDMA_CTRL_T;

typedef union
{
	unsigned int bits32;
	struct bits_ff0c
	{
		#ifdef BIG_ENDIAN
		unsigned int td_first_des_ptr	: 28;/* first descriptor address */
		unsigned int td_busy			:  1;/* 1-TxDMA busy; 0-TxDMA idle */
		unsigned int 					:  3;
		#else
		unsigned int 					:  3;
		unsigned int td_busy			:  1;/* 1-TxDMA busy; 0-TxDMA idle */
		unsigned int td_first_des_ptr	: 28;/* first descriptor address */
		#endif
	} bits;
} RAID_TXDMA_FIRST_DESC_T;

typedef union
{
	unsigned int bits32;
	struct bits_ff10
	{
		#ifdef BIG_ENDIAN
		unsigned int ndar			: 28;	/* next descriptor address */
		unsigned int eofie			:  1;	/* end of frame interrupt enable */
		unsigned int    			:  1;
		unsigned int sof_eof    	:  2;
		#else
		unsigned int sof_eof		:  2;
		unsigned int 				:  1;
		unsigned int eofie			:  1;	/* end of frame interrupt enable */
		unsigned int ndar			: 28;	/* next descriptor address */
		#endif
	} bits;
} RAID_TXDMA_CURR_DESC_T;

typedef union
{
	unsigned int bits32;
	struct bits_ff14
	{
		#ifdef BIG_ENDIAN
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
} RAID_RXDMA_CTRL_T;

typedef union
{
	unsigned int bits32;
	struct bits_ff18
	{
		#ifdef BIG_ENDIAN
		unsigned int rd_first_des_ptr	: 28;/* first descriptor address */
		unsigned int rd_busy			:  1;/* 1-RxDMA busy; 0-RxDMA idle */
		unsigned int 					:  3;
		#else
		unsigned int 					:  3;
		unsigned int rd_busy			:  1;/* 1-RxDMA busy; 0-RxDMA idle */
		unsigned int rd_first_des_ptr	: 28;/* first descriptor address */
		#endif
	} bits;
} RAID_RXDMA_FIRST_DESC_T;

typedef union
{
	unsigned int bits32;
	struct bits_ff1c
	{
		#ifdef BIG_ENDIAN
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
} RAID_RXDMA_CURR_DESC_T;

typedef union
{
	unsigned int bit32;
	struct bits_ff50
	{
		unsigned int pat			: 32; /* data for pattern check */
	} bits;
} RAID_PACR_T;

/******************************************************/
/*	the definition of DMA Descriptor Register     */
/******************************************************/
typedef struct raid_descriptor_t
{
	union func_ctrl_t
	{
		unsigned int bit32;
		struct bits_0000
		{
			#ifdef BIG_ENDIAN
			unsigned int own				: 1; /* owner bit */
			unsigned int derr				: 1;	/* data error during processing this descriptor */
			unsigned int perr				: 1;	/* protocol error during processing this descriptor */
			unsigned int raid_ctrl_status	: 7; /* pass RAID XOR fetch/store control status to CPU */
			unsigned int desc_cnt			: 6;
			unsigned int buffer_size		: 16;	/* transfer buffer size associated with current description*/
			#else
			unsigned int buffer_size		: 16;	/* transfer buffer size associated with current description*/
			unsigned int desc_cnt			: 6;
			unsigned int raid_ctrl_status	: 7; /* pass RAID XOR fetch/store control status to CPU */
			unsigned int perr				: 1;	/* protocol error during processing this descriptor */
			unsigned int derr				: 1;	/* data error during processing this descriptor */
			unsigned int own				: 1; /* owner bit */
			#endif
		} bits;
	} func_ctrl;

	union flg_status_t
	{
		unsigned int bits32;
		struct bit_004
		{
			#ifdef BIG_ENDIAN
			unsigned int bcc 		: 16;
			unsigned int 			: 13
			unsigned int mode		: 3;
			#else
			unsigned int mode		: 3;
			unsigned int 			: 13;
			unsigned int bcc		: 16;
			#endif
		} bits_cmd_status;
	} flg_status;  //Sanders

	unsigned int buf_addr;

	union next_desc_addr_t
	{
		unsigned int bits32;
		struct bits_000c
		{
			#ifdef BIG_ENDIAN
			unsigned int ndar 		: 28; /* next descriptor address */
			unsigned int eofie		: 1; /* end of frame interrupt enable */
			unsigned int 			: 1;
			unsigned int sof_eof	: 2; /* the position of the descriptor in chain */
			#else
			unsigned int sof_eof	: 2; /* the position of the descriptor in chain */
			unsigned int 			: 1;
			unsigned int eofie		: 1; /* end of frame interrupt enable */
			unsigned int ndar 		: 28; /* next descriptor address */
			#endif
		} bits;
	} next_desc_addr;
} RAID_DESCRIPTOR_T;

/******************************************************/
/*	the offset of RAID SMC register		      */
/******************************************************/
enum RAID_SMC_REGISTER {
	RAID_SMC_CMD_REG		= 0xff60,
	RAID_SMC_STATUS_REG		= 0xff64
	};

/******************************************************/
/*	the definition of RAID SMC module register    */
/******************************************************/
typedef union
{
	unsigned int bits32;
	struct bits_ff60
	{
		#ifdef BIG_ENDIAN
		unsigned int pat_mode		: 2; /* partition mode selection */
		unsigned int 				: 14;
		unsigned int device_id		: 12;
		unsigned int revision_id	: 4;
		#else
		unsigned int revision_id	: 4;
		unsigned int device_id		: 12;
		unsigned int 				: 14;
		unsigned int pat_mode		: 2; /* partition mode selection */
		#endif
	} bits;
} RAID_SMC_CMD;

typedef union
{
	unsigned int bits32;
	struct bits_ff64
	{
		#ifdef BIG_ENDIAN
		unsigned int addr_err1		: 1; /* address is out of range for controller 1 */
		unsigned int ahb_err1		: 1; /* AHB bus error for controller 1 */
		unsigned int 				: 14;
		unsigned int addr_err2		: 1;	/* address is out of range for controller 2 */
		unsigned int ahb_err2		: 1; /* AHB bus error for controller 2 */
		unsigned int 				: 14;
		#else
		unsigned int 				: 14;
		unsigned int ahb_err2		: 1; /* AHB bus error for controller 2 */
		unsigned int addr_err2		: 1;	/* address is out of range for controller 2 */
		unsigned int 				: 14;
		unsigned int ahb_err1		: 1; /* AHB bus error for controller 1 */
		unsigned int addr_err1		: 1; /* address is out of range for controller 1 */
		#endif
	} bits;
} RAID_SMC_STATUS;

typedef struct RAID_S
{
	const char *device_name;
	wait_queue_head_t wait;
	unsigned int busy;
	int irq;
	unsigned int status;
	RAID_DESCRIPTOR_T *tx_desc;  /*   point to virtual TX descriptor address */
	RAID_DESCRIPTOR_T *rx_desc;  /* point ot virtual RX descriptor address */
	RAID_DESCRIPTOR_T *tx_cur_desc; /* current TX descriptor */
	RAID_DESCRIPTOR_T *rx_cur_desc; /* current RX descriptor */
	RAID_DESCRIPTOR_T *tx_finished_desc;
	RAID_DESCRIPTOR_T *rx_finished_desc;
	RAID_DESCRIPTOR_T *tx_first_desc;
	RAID_DESCRIPTOR_T *rx_first_desc;

//	unsigned int *tx_buf[TX_DESC_NUM];
	unsigned int *rx_desc_dma;			// physical address of rx_descript
	unsigned int *tx_desc_dma;			// physical address of tx_descript
	unsigned int *rx_bufs_dma;
	unsigned int *tx_bufs_dma;

} RAID_T;

struct reg_ioctl
{
	unsigned int reg_addr;
	unsigned int val_in;
	unsigned int val_out;
};

typedef struct dma_ctrl {
	int sram;
	int prot;
	int burst;
	int bus;
	int endian;
	int mode;
} DMA_CTRL;


#ifdef XOR_SW_FILL_IN

#define __XOR(a1, a2) a1 ^= a2

#define GET_BLOCK_2(dst) \
	__asm__("ldmia	%0, {%1, %2}" \
		: "=r" (dst), "=r" (a1), "=r" (a2) \
		: "0" (dst))

#define GET_BLOCK_4(dst) \
	__asm__("ldmia	%0, {%1, %2, %3, %4}" \
		: "=r" (dst), "=r" (a1), "=r" (a2), "=r" (a3), "=r" (a4) \
		: "0" (dst))

#define XOR_BLOCK_2(src) \
	__asm__("ldmia	%0!, {%1, %2}" \
		: "=r" (src), "=r" (b1), "=r" (b2) \
		: "0" (src)); \
	__XOR(a1, b1); __XOR(a2, b2);

#define XOR_BLOCK_4(src) \
	__asm__("ldmia	%0!, {%1, %2, %3, %4}" \
		: "=r" (src), "=r" (b1), "=r" (b2), "=r" (b3), "=r" (b4) \
		: "0" (src)); \
	__XOR(a1, b1); __XOR(a2, b2); __XOR(a3, b3); __XOR(a4, b4)

#define PUT_BLOCK_2(dst) \
	__asm__ __volatile__("stmia	%0!, {%2, %3}" \
		: "=r" (dst) \
		: "0" (dst), "r" (a1), "r" (a2))

#define PUT_BLOCK_4(dst) \
	__asm__ __volatile__("stmia	%0!, {%2, %3, %4, %5}" \
		: "=r" (dst) \
		: "0" (dst), "r" (a1), "r" (a2), "r" (a3), "r" (a4))

static void
xor_arm4regs_2(unsigned long bytes, unsigned long *p1, unsigned long *p2)
{
	unsigned int lines = bytes / sizeof(unsigned long) / 4;
	register unsigned int a1 __asm__("r4");
	register unsigned int a2 __asm__("r5");
	register unsigned int a3 __asm__("r6");
	register unsigned int a4 __asm__("r7");
	register unsigned int b1 __asm__("r8");
	register unsigned int b2 __asm__("r9");
	register unsigned int b3 __asm__("ip");
	register unsigned int b4 __asm__("lr");

	do {
		GET_BLOCK_4(p1);
		XOR_BLOCK_4(p2);
		PUT_BLOCK_4(p1);
	} while (--lines);
}

static void
xor_arm4regs_3(unsigned long bytes, unsigned long *p1, unsigned long *p2,
		unsigned long *p3)
{
	unsigned int lines = bytes / sizeof(unsigned long) / 4;
	register unsigned int a1 __asm__("r4");
	register unsigned int a2 __asm__("r5");
	register unsigned int a3 __asm__("r6");
	register unsigned int a4 __asm__("r7");
	register unsigned int b1 __asm__("r8");
	register unsigned int b2 __asm__("r9");
	register unsigned int b3 __asm__("ip");
	register unsigned int b4 __asm__("lr");

	do {
		GET_BLOCK_4(p1);
		XOR_BLOCK_4(p2);
		XOR_BLOCK_4(p3);
		PUT_BLOCK_4(p1);
	} while (--lines);
}

static void
xor_arm4regs_4(unsigned long bytes, unsigned long *p1, unsigned long *p2,
		unsigned long *p3, unsigned long *p4)
{
	unsigned int lines = bytes / sizeof(unsigned long) / 2;
	register unsigned int a1 __asm__("r8");
	register unsigned int a2 __asm__("r9");
	register unsigned int b1 __asm__("ip");
	register unsigned int b2 __asm__("lr");

	do {
		GET_BLOCK_2(p1);
		XOR_BLOCK_2(p2);
		XOR_BLOCK_2(p3);
		XOR_BLOCK_2(p4);
		PUT_BLOCK_2(p1);
	} while (--lines);
}

static void
xor_arm4regs_5(unsigned long bytes, unsigned long *p1, unsigned long *p2,
		unsigned long *p3, unsigned long *p4, unsigned long *p5)
{
	unsigned int lines = bytes / sizeof(unsigned long) / 2;
	register unsigned int a1 __asm__("r8");
	register unsigned int a2 __asm__("r9");
	register unsigned int b1 __asm__("ip");
	register unsigned int b2 __asm__("lr");

	do {
		GET_BLOCK_2(p1);
		XOR_BLOCK_2(p2);
		XOR_BLOCK_2(p3);
		XOR_BLOCK_2(p4);
		XOR_BLOCK_2(p5);
		PUT_BLOCK_2(p1);
	} while (--lines);
}
#endif	//XOR_SW_FILL_IN

