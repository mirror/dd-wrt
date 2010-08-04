/*
 * rdma.h - CNS3xxx hardware RAID acceleration
 */
#ifndef _CNS3XXX_RDMA_H_
#define _CNS3XXX_RDMA_H_

#include <mach/hardware.h>

#define	RDMA_REGS_PHYS(x)	((u32)(CNS3XXX_RAID_BASE + (x)))
#define	RDMA_REGS_VIRT(x)	((u32 volatile *)(CNS3XXX_RAID_BASE_VIRT + (x)))
#define	RDMA_REGS_VALUE(x)	(*((u32 volatile *)(CNS3XXX_RAID_BASE_VIRT + (x))))


#define	GENERIC_ALIGN			0x8				/* 64-bits */
#define	GENERIC_ALIGN_MASK		0xFFFFFFF8UL
#define	QUEUE_DEPTH_ALIGN_MUL	0x10	/* 16 bytes; ALIGNMENT == QDEPTH * 16 bytes */


/* Register Offset */
#define	REG_PARA_OFFSET		0x00UL	/* Parameter */
#define	REG_BLSZ_OFFSET		0x04UL	/* Block Size */
#define	REG_SGAD_OFFSET		0x08UL	/* SG Address */
#define	REG_STAT_OFFSET		0x0CUL	/* Status */
#define	REG_FRNT_OFFSET		0x10UL	/* FP */
#define	REG_BACK_OFFSET		0x14UL	/* BP */
#define	REG_QPAR_OFFSET		0x18UL	/* Queue Parameter */


/* 0x00: PARA */
#define	REG_PARA_ENABLE				0x80000000UL	/* 31 */
#define	REG_PARA_XFER_END			0x40000000UL	/* 30 */
#define	REG_PARA_MEMORY_WR_DISABLE	0x20000000UL	/* 29 */
#define	REG_PARA_FAULTY_DISKS_CNT	0x08000000UL	/* 28:27 */

#define	REG_PARA_CALC				0x01000000UL	/* 26:24 */
	#define	REG_PARA_CALC_DATA		0x00000000UL	
	#define	REG_PARA_CALC_P			0x01000000UL	
	#define	REG_PARA_CALC_Q			0x02000000UL	
	#define	REG_PARA_CALC_R			0x04000000UL	
	#define	REG_PARA_CALC_PQ		0x03000000UL	
	#define	REG_PARA_CALC_PR		0x05000000UL	
	#define	REG_PARA_CALC_QR		0x06000000UL	
	#define	REG_PARA_CALC_PQR		0x07000000UL	

#define	REG_PARA_FDISK_3_R_IDX		0x00010000UL	/* 23:16 */
#define	REG_PARA_FDISK_2_Q_IDX		0x00000100UL	/* 15:8 */
#define	REG_PARA_FDISK_1_P_IDX		0x00000001UL	/* 7:0 */

/* 0x04: BLSZ */
#define	REG_BLSZ_SHIFT				3				/* 19:3 */
#define	REG_BLSZ_MASK				0x000FFFF8UL	/* N * 8bytes */

/* 0x08: SGAD */
#define	REG_SGAD_SHIFT				0

/* 0x0C: STAT */
#define	REG_STAT_XFER_COMPLETE		0x80000000UL	/* 31 */
#define	REG_STAT_SLAVE_ERROR		0x40000000UL	/* 30 */
#define	REG_STAT_DECODER_ERROR		0x20000000UL	/* 29 */
#define	REG_STAT_R_FLAG				0x00080000UL	/* 19 */
#define	REG_STAT_Q_FLAG				0x00040000UL	/* 18 */
#define	REG_STAT_P_FLAG				0x00020000UL	/* 17 */
#define	REG_STAT_CMD_QUEUE_ENABLE	0x00000002UL	/* 1 */
#define	REG_STAT_INTERRUPT_FLAG		0x00000001UL	/* 0 */

/* 0x10/14: FRNT/BACK */
#define	REG_FRNT_SHIFT				0
#define	REG_BACK_SHIFT				0

/* 0x18: QPAR */
#define	MAX_Q_DEPTH				256
#define	REG_QPAR_DEPTH_256		0xFF
#define	REG_QPAR_DEPTH_128		0x7F
#define	REG_QPAR_DEPTH_64		0x3F
#define	REG_QPAR_DEPTH_32		0x1F
#define	REG_QPAR_DEPTH_16		0xF
#define	REG_QPAR_DEPTH_8		0x7
#define	REG_QPAR_DEPTH_4		0x3
#define	REG_QPAR_DEPTH_2		0x1

/* len = 32 */
#define	CURR_Q_DEPTH			(REG_QPAR_DEPTH_32 + 1)
#define	CURR_Q_DEPTH_ALIGN		((CURR_Q_DEPTH) * (QUEUE_DEPTH_ALIGN_MUL))	/* 0x200 */
#define	CURR_Q_DEPTH_ALIGN_MASK	0xFFFFFE00UL


#define	MAX_SG					32   // cf. CURR_Q_DEPTH or MAX_Q_DEPTH
#define	MAX_ENTRIES_PER_SG		32

/* SG Table */
#define	SG_ADDR_MASK		0x00000000FFFFFFFFULL

#define	SG_READ_IDX_MASK	0x000000FF00000000ULL
#define	SG_IDX_SHIFT		32

// ---------------------- 7654321076543210
#define	SG_RW_MASK		0x00000F0000000000ULL
#define	RWI_RD_D		0x0000000000000000ULL
#define	RWI_RD_P		0x0000010000000000ULL
#define	RWI_RD_Q		0x0000020000000000ULL
#define	RWI_RD_R		0x0000030000000000ULL
#define	RWI_W_D1		0x0000040000000000ULL
#define	RWI_W_P1		0x0000050000000000ULL
#define	RWI_W_Q1		0x0000060000000000ULL
#define	RWI_W_R1		0x0000070000000000ULL
#define	RWI_W_D2		0x0000080000000000ULL
#define	RWI_W_P2		0x0000090000000000ULL
#define	RWI_W_Q2		0x00000A0000000000ULL
#define	RWI_W_R2		0x00000B0000000000ULL
#define	RWI_W_D3		0x00000C0000000000ULL
#define	RWI_W_P3		0x00000D0000000000ULL
#define	RWI_W_Q3		0x00000E0000000000ULL
#define	RWI_W_R3		0x00000F0000000000ULL


#define	SG_STATUS_FREE			0x00000001UL
#define	SG_STATUS_ACQUIRED		0x00000002UL
#define	SG_STATUS_SCHEDULED		0x00000004UL
#define SG_STATUS_DONE			0x00000008UL
#define	SG_STATUS_ERROR			0x00000010UL

#define SG_ENTRY_BYTES	(8 * MAX_ENTRIES_PER_SG)

typedef struct rdma_sgtable sg_t;
struct rdma_sgtable {
	u64	entry[MAX_ENTRIES_PER_SG];

	struct list_head lru;			/* list_add_tail/list_del to/from process_q when schedule or isr */
	wait_queue_head_t wait;
	
	u32 status;
};

/* Command Queue: cmdq_t */
typedef	struct rdma_cmdq cmdq_t;
struct rdma_cmdq {
	volatile u32 parameter;
	volatile u32 block_size;
	volatile u32 sg_addr;
	volatile u32 reserved;
};
 
struct ctrl_regs {
	volatile u32 *para;
	volatile u32 *blsz;
	volatile u32 *sgad;
	volatile u32 *stat;
	volatile u32 *frnt;
	volatile u32 *back;
	volatile u32 *qpar;
};

/* channel */
#define	RDMA_CHANNEL_COUNT			1
typedef struct rdma_channel rdma_chan_t;
struct rdma_channel 
{
	struct list_head process_q;
	spinlock_t process_lock;	/* process queue lock */

	int irq;					
	const char *irq_str;		
	
	/* cmd queue start address */
	volatile cmdq_t *q_virt;			
	volatile u32 q_first_phys;
	volatile u32 q_last_phys;

	/* control regs */
	struct ctrl_regs *cregs;

	// wait_queue_head_t wait;
};

int __init cns_rdma_init(void);

#endif

