#if !defined(CORE_SATA_H)
#define CORE_SATA_H

#include "core_ata.h"
#include "com_tag.h"

#define MAX_SATA_PORT_NUMBER   4
#if /*(VER_OEM==VER_OEM_ASUS) || */(VER_OEM == VER_OEM_INTEL)
#define MAX_PATA_PORT_NUMBER   0
#else
#define MAX_PATA_PORT_NUMBER   1
#endif
#define MAX_PORT_NUMBER        (MAX_SATA_PORT_NUMBER + MAX_PATA_PORT_NUMBER)

#ifdef SUPPORT_PM
#define MAX_DEVICE_PER_PORT    5
#else
#ifdef _OS_BIOS
#define MAX_DEVICE_PER_PORT    5 /* following WinDriver, but bios not suppport PM currently*/
#else
#define MAX_DEVICE_PER_PORT    2
#endif
#endif

#define MAX_DEVICE_NUMBER			(MAX_PORT_NUMBER*MAX_DEVICE_PER_PORT)

#ifdef _OS_BIOS
#define MAX_SLOT_NUMBER				1
#else
#define MAX_SLOT_NUMBER				32
#endif

#define INTERNAL_REQ_COUNT			MAX_DEVICE_SUPPORTED

//TBD: Checking the following naming
#define SATA_CMD_LIST_SIZE			(32 * MAX_SLOT_NUMBER)
#define SATA_RX_FIS_SIZE			256

#define SATA_CMD_TABLE_HEADER_SIZE	0x80
#define SATA_CMD_TABLE_SG_SIZE		(MAX_SG_ENTRY * 16)
#define SATA_CMD_TABLE_SIZE			(SATA_CMD_TABLE_HEADER_SIZE + SATA_CMD_TABLE_SG_SIZE)

#define SATA_SCRATCH_BUFFER_SIZE	sizeof(ATA_Identify_Data)

/*
*struct _MV_Command_Header;
*typedef struct _MV_Command_Header MV_Command_Header, *PMV_Command_Header;
*/

/* various functions for master/slave support */
#define PATA_MapDeviceId(ID)		( ID % MAX_DEVICE_PER_PORT )
#define PATA_MapPortId(ID)			( ID / MAX_DEVICE_PER_PORT )	//TBD: Change the name

#define DEVICE_TYPE_ATAPI						MV_BIT(0)

/* Device initialization state */
#define DEVICE_STATE_IDLE						0x0
#define DEVICE_STATE_RESET_DONE					0x1
#define DEVICE_STATE_IDENTIFY_DONE				0x2
#define DEVICE_STATE_SET_UDMA_DONE				0x3
#define DEVICE_STATE_SET_PIO_DONE				0x4
#define DEVICE_STATE_ENABLE_WRITE_CACHE_DONE	0x5
#define DEVICE_STATE_ENABLE_READ_AHEAD_DONE		0x6
#define DEVICE_STATE_INIT_DONE					0xFF

/* Device status */
#define DEVICE_STATUS_NO_DEVICE					MV_BIT(0)
#define DEVICE_STATUS_EXISTING					MV_BIT(1)
#define DEVICE_STATUS_FUNCTIONAL				MV_BIT(2)

/* 3G and TCQ */
#define DEVICE_CAPACITY_48BIT_SUPPORTED			MV_BIT(0)
#define	DEVICE_CAPACITY_SMART_SUPPORTED			MV_BIT(1)
#define	DEVICE_CAPACITY_WRITECACHE_SUPPORTED	MV_BIT(2)
#define DEVICE_CAPACITY_NCQ_SUPPORTED			MV_BIT(3)
#define DEVICE_CAPACITY_RATE_1_5G			MV_BIT(4)
#define DEVICE_CAPACITY_RATE_3G				MV_BIT(5)	
#define DEVICE_CAPACITY_READLOGEXT_SUPPORTED		MV_BIT(6)


#define DEVICE_SETTING_SMART_ENABLED			MV_BIT(0)
#define DEVICE_SETTING_WRITECACHE_ENABLED		MV_BIT(1)

struct _Domain_Device {
	MV_U16 Id;
	MV_U8 Device_Type;				/* ATA or ATAPI */
	MV_U8 State;					/* DEVICE_STATE_XXX */
	
	MV_U8 Status;					/* DEVICE_STATUS_XXX */
	MV_BOOLEAN Is_Slave;
#ifndef BIOS_NOT_SUPPORT
	MV_BOOLEAN Need_Notify;			/* added for PM hot plug */
	MV_U8 Reserved0;
#endif

	struct _Domain_Port * PPort;	/* Shortcut to the port. */

	/* 
	 * Different device should have a different struct here. 
	 * Now it's SATA device only.
	 */
	MV_U16 Capacity;				/* Be able to support NCQ, 48 bit LBA. */
	MV_U16 Setting;					/* The supported features are enabled or not. */

	MV_U8 PM_Number;
	MV_U8 PIO_Mode;
	MV_U8 MDMA_Mode;
	MV_U8 UDMA_Mode;

	MV_U64 Max_LBA;
#ifndef BIOS_NOT_SUPPORT
	MV_U8 Queue_Depth;
#endif
	MV_U8 Timer_ID;					/* for error handling */
	MV_U8 Outstanding_Req;			/* for error handling */
	MV_U8 Reset_Count;
	MV_U8 Reserved1[4];


#ifndef BIOS_NOT_SUPPORT
	MV_U8 Serial_Number[20];
#endif
	MV_U8 Model_Number[40];
#ifndef BIOS_NOT_SUPPORT
	MV_U8 Firmware_Revision[8];
#endif
	MV_U32 WWN;

	/* The scratch buffer used for initialization like identify */
	MV_PVOID Scratch_Buffer;
	MV_PHYSICAL_ADDR Scratch_Buffer_DMA;

	/* Internal request used in device initialization */
	PMV_Request Internal_Req;

};

/* Port initialization state */

#define PORT_STATE_IDLE					0x00
#define PORT_STATE_INIT_DONE			0xFF

#define PORT_TYPE_SATA					0
#define PORT_TYPE_PATA					1
#define PORT_TYPE_PM					4 /*PM Support, lily tested*/

#define PORT_CAPACITY_NCQ_SUPPORTED		MV_BIT(0)

#define PORT_SETTING_NCQ_RUNNING		MV_BIT(0)
#define PORT_SETTING_PM_EXISTING		MV_BIT(1)
#define PORT_SETTING_PM_FUNCTIONAL		MV_BIT(2)	// added by Harriet for PM hot plug 
#define PORT_SETTING_DURING_RETRY		MV_BIT(3)

struct _Domain_Port {
	MV_PVOID Core_Extension;

	MV_U8 Id;
	MV_U8 Port_State;
	MV_U8 Type;					/* PORT_TYPE_XXX */
	MV_U8 Capacity;				/* PORT_CAPACITY_XXX */
	MV_U8 Setting;				/* PORT_SETTING_XXX */
	MV_U8 Device_Number;		/* How many devices this port has now? */	
	MV_U16 PM_Vendor_Id;
	MV_U16 PM_Device_Id;
	MV_U8 PM_Product_Revision;
	MV_U8 PM_Spec_Revision;
	MV_U8 PM_Num_Ports;
#ifndef BIOS_NOT_SUPPORT
	MV_U8 Reserved0[3];
#endif
	MV_LPVOID Mmio_Base;		/* Base address for SATA Port Registers */
#ifndef _OS_BIOS
	MV_LPVOID Mmio_SCR;			/* Base address for sata register(SCR) */
#endif
	MV_PVOID Cmd_List;			/* Can be PMV_PATA_Command_Header or PMV_Command_Header */
	MV_PHYSICAL_ADDR Cmd_List_DMA;

	/* Received FIS */
	MV_PVOID RX_FIS;
	MV_PHYSICAL_ADDR RX_FIS_DMA;

	/* The 32 command tables. */
	MV_PVOID Cmd_Table;
	MV_PHYSICAL_ADDR Cmd_Table_DMA;

	/* Running MV_Requests are linked together. */	//TBD: Too much memory?
	PMV_Request Running_Req[MAX_SLOT_NUMBER];	

	/* Which slot has requests running. */
	MV_U32	Running_Slot;
#ifndef BIOS_NOT_SUPPORT
//	MV_U32	Reserved1;
	MV_U32	VS_RegC;
#endif

	struct _Domain_Device Device[MAX_DEVICE_PER_PORT];	

	//Timer: for time out checking.

	Tag_Stack Tag_Pool;
};

/*
 * Hardware related format. Never change their size. Must follow hardware specification.
 */
/* AHCI a little difference */
typedef struct _MV_Command_Header
{
#ifdef __MV_BIG_ENDIAN_BITFIELD__
	MV_U8	Reserved0 : 2;
	MV_U8	Packet_Command : 1;
	MV_U8	FIS_Length : 5;
			
	MV_U8	PM_Port : 4;
	MV_U8	NCQ : 1;
	MV_U8	Reserved1: 2;
	MV_U8	Reset : 1;
#else /* default to __MV_LITTLE_ENDIAN_BITFIELD__ */
	MV_U8	FIS_Length : 5;		/* Command FIS Length in DWORD */
	MV_U8	Packet_Command : 1;	/* ATAPI packet command */
	MV_U8	Reserved0 : 2;

	MV_U8	Reset : 1;
	MV_U8	Reserved1: 2;
	MV_U8	NCQ : 1;
	MV_U8	PM_Port : 4;
#endif /* __MV_BIG_ENDIAN_BITFIELD__ */
	MV_U16	PRD_Entry_Count : 16;

	MV_U32	Reserved2;
	MV_U32	Table_Address;
	MV_U32	Table_Address_High;

	MV_U32	Reserved3[4];
} MV_Command_Header, *PMV_Command_Header;

typedef struct _MV_PATA_Command_Header
{
#ifdef __MV_BIG_ENDIAN_BITFIELD__
	MV_U8	Packet_Command : 1;	
	MV_U8	TCQ : 1;		
	MV_U8	Controller_Command : 1;	
	MV_U8	PIO_Sector_Count : 5;   

	MV_U8	Is_Slave : 1;		
	MV_U8	Reset : 1;		
	MV_U8	Diagnostic_Command : 1;	
	MV_U8	Is_48Bit : 1;		
	MV_U8	PIO_Sector_Command : 1;	
	MV_U8	Non_Data : 1;		
	MV_U8	Data_In : 1;		
	MV_U8	DMA : 1;		
#else /* __MV_BIG_ENDIAN_BITFIELD__ */
	MV_U8	PIO_Sector_Count : 5;   /* PIO command data block size in sector */
	MV_U8	Controller_Command : 1;	/* If 1, command is for the controller instead of the device */
	MV_U8	TCQ : 1;		/* TCQ command */
	MV_U8	Packet_Command : 1;	/* ATAPI packet command */

	MV_U8	DMA : 1;		/* DMA command */
	MV_U8	Data_In : 1;		/* Data is from device to host. */
	MV_U8	Non_Data : 1;		/* Non data command */
	MV_U8	PIO_Sector_Command : 1;	/* PIO multiple sectors commands including read/write sector, read/write multiple. */
	MV_U8	Is_48Bit : 1;		/* 48 bit command */
	MV_U8	Diagnostic_Command : 1;	/* Execute device diagnostic command */
	MV_U8	Reset : 1;		/* Device reset command */
	MV_U8	Is_Slave : 1;		/* 0 for master and 1 for slave */
#endif /* __MV_BIG_ENDIAN_BITFIELD__ */
	MV_U16	PRD_Entry_Count;


	MV_U32	Reserved0;
	MV_U32	Table_Address;
	MV_U32	Table_Address_High;

	MV_U32	Reserved3[4];
} MV_PATA_Command_Header, *PMV_PATA_Command_Header;

/* SATA Command Table: same with AHCI */
typedef struct _MV_Command_Table
{
	MV_U8	FIS[64];								/* Command FIS */
	MV_U8	ATAPI_CDB[32];							/* ATAPI CDB */
	MV_U8	Reserve0[32];
	MV_SG_Entry PRD_Entry[MAX_SG_ENTRY];		/* 32 */
} MV_Command_Table, *PMV_Command_Table;

#define DIMMSGTABLE_SIZE sizeof(AHCI_DIMM_SG_TABLE)

#define	MV_PCI_BAR			 5
#define	MV_CMD_ATAPI		 (1L << 5)
#define	MV_CMD_WRITE		 (1L << 6)

#define	RX_FIS_D2H_REG		 0x40	/* offset of D2H Register FIS data */

	/* global controller registers */
#define	HOST_CAP			 0x00 	/* host capabilities */
#define	HOST_CTL			 0x04	/* global host control */
#define	HOST_IRQ_STAT		 0x08 	/* interrupt status */
#define	HOST_PORTS_IMPL		 0x0c 	/* bitmap of implemented ports */
#define	HOST_VERSION		 0x10 	/* AHCI spec. version compliancy */

	/* HOST_CTL bits */
#define	HOST_RESET			(1L << 0)  /* reset controller; self-clear */
#define	HOST_IRQ_EN		 	(1L << 1)  /* global IRQ enable */
#define	HOST_MVL_EN		 	(1L << 31) /* AHCI enabled */

	/* HOST_CAP bits */
#define	HOST_CAP_64			(1L << 31) /* PCI DAC (64-bit DMA) support */

	/* Vendor specific register */
#define VENDOR_DETECT		0xA4	/* PATA device/PM detection */
	/* VENDOR_DETECT bits */
#define VENDOR_DETECT_PATA	(1L << 10)	/* PATA device detection (bit10) (0 - default) */
#define VENDOR_DETECT_PM	(1L << 11)	/* PM device detection (bit11) (0 - default) */

	/* registers for each SATA port */
#define	PORT_LST_ADDR		0x00 /* command list DMA addr */
#define	PORT_LST_ADDR_HI	0x04 /* command list DMA addr hi */
#define	PORT_FIS_ADDR		0x08 /* FIS rx buf addr */
#define	PORT_FIS_ADDR_HI	0x0c /* FIS rx buf addr hi */
#define	PORT_IRQ_STAT		0x10 /* interrupt status */
#define	PORT_IRQ_MASK		0x14 /* interrupt enable/disable mask */
#define	PORT_CMD			0x18 /* port command */

	/* For SATA port */
#define	PORT_TFDATA			0x20	/* taskfile data */
#define	PORT_SIG			0x24	/* device TF signature */
#define	PORT_CMD_ISSUE		0x38 	/* command issue */
#define	PORT_FIFO_CTL		0x44	/* vendor unique FIFO control */
#define	PORT_SCR			0x28 	/* SATA phy register block */
#define	PORT_SCR_STAT		0x28 	/* SATA phy register: SStatus */
#define	PORT_SCR_CTL		0x2c 	/* SATA phy register: SControl */
#define	PORT_SCR_ERR		0x30 	/* SATA phy register: SError */
#define	PORT_SCR_ACT		0x34 	/* SATA phy register: SActive */
#define	PORT_PM_FIS_0		0x3c	/* port multiplier FIS content 0 */
#define	PORT_PM_FIS_1		0x40	/* port multiplier FIS content 1 */

/* #if (VER_OEM==VER_OEM_ASUS) */
#define	PORT_VSR_ADDR		0x78	/* port Vendor Specific Register Address */
#define	PORT_VSR_DATA		0x7c	/* port Vendor Specific Register Data */
/* #endif */
#define VS_REG_SIG			0xab

	/* For PATA port */
#define	PORT_MASTER_TF0		0x20
#define	PORT_MASTER_TF1		0x24
#define	PORT_MASTER_TF2		0x28
#define	PORT_SLAVE_TF0		0x30
#define	PORT_SLAVE_TF1		0x3c
#define	PORT_SLAVE_TF2		0x40
#define	PORT_INTERNAL_STATE_MACHINE	0x48


#ifdef AHCI
	/* PORT_IRQ_{STAT,MASK} bits */
#define	PORT_IRQ_COLD_PRES		(1L << 31)	/* cold presence detect */
#define	PORT_IRQ_TF_ERR			(1L << 30)	/* task file error */
#define	PORT_IRQ_HBUS_ERR		(1L << 29)	/* host bus fatal error */
#define	PORT_IRQ_HBUS_DATA_ERR	(1L << 28)	/* host bus data error */
#define	PORT_IRQ_IF_ERR			(1L << 27)	/* interface fatal error */
#define	PORT_IRQ_IF_NONFATAL	(1L << 26)	/* interface non-fatal error */
#define	PORT_IRQ_OVERFLOW		(1L << 24)	/* xfer exhausted available S/G */
#define	PORT_IRQ_BAD_PMP		(1L << 23)	/* incorrect port multiplier */

#define	PORT_IRQ_PHYRDY			(1L << 22)	 /* PhyRdy changed */
#define PORT_IRQ_ASYNC_NOTIF	(1L << 20)	 /* Asynchronous Notification, SDB FIS */
#define	PORT_IRQ_DEV_ILCK		(1L << 7)		/* device interlock */
#define	PORT_IRQ_CONNECT		(1L << 6)		/* port connect change status */
#define	PORT_IRQ_SG_DONE		(1L << 5)		/* descriptor processed */
#define	PORT_IRQ_UNK_FIS		(1L << 4)		/* unknown FIS rx'd */
#define	PORT_IRQ_SDB_FIS		(1L << 3)		/* Set Device Bits FIS rx'd */
#define	PORT_IRQ_DMAS_FIS		(1L << 2)		/* DMA Setup FIS rx'd */
#define	PORT_IRQ_PIOS_FIS		(1L << 1)		/* PIO Setup FIS rx'd */
#define	PORT_IRQ_D2H_REG_FIS	(1L << 0)		/* D2H Register FIS rx'd */

#define	PORT_IRQ_FATAL	(PORT_IRQ_TF_ERR |\
				  		PORT_IRQ_HBUS_ERR |\
				  		PORT_IRQ_HBUS_DATA_ERR |\
				  		PORT_IRQ_IF_ERR)
#define	DEF_PORT_IRQ	(PORT_IRQ_FATAL | PORT_IRQ_PHYRDY |\
				  		PORT_IRQ_CONNECT | PORT_IRQ_SG_DONE |\
				  		PORT_IRQ_UNK_FIS | PORT_IRQ_SDB_FIS |\
				  		PORT_IRQ_DMAS_FIS | PORT_IRQ_PIOS_FIS |\
				  		PORT_IRQ_D2H_REG_FIS)
#else
	/* PORT_IRQ_{STAT,MASK} bits for SATA port */
#define	PORT_IRQ_SIGNATURE_FIS	(1L << 31)	/* Signature FIS received */
#define	PORT_IRQ_TF_ERR			(1L << 30)	/* task file error */
#define	PORT_IRQ_PHYRDY			(1L << 22)	/* PhyRdy changed */

#define	PORT_IRQ_BIST			(1L << 21)	/* BIST activate FIS received */
#define	PORT_IRQ_ASYNC_NOTIF	(1L << 20)	/* Asynchronous notification received */
#define	PORT_IRQ_LINK_RECEIVE_ERROR	(1L << 7)
#define	PORT_IRQ_LINK_TRANSMIT_ERROR (1L << 6)
#define	PORT_IRQ_PIO_DONE		(1L << 5)		/* PIO Data-in Done */
#define	PORT_IRQ_UNK_FIS		(1L << 4)		/* unknown FIS rx'd */
#define	PORT_IRQ_SDB_FIS		(1L << 3)		/* Set Device Bits FIS rx'd */
#define	PORT_IRQ_DMAS_FIS		(1L << 2)		/* DMA Setup FIS rx'd */
#define	PORT_IRQ_PIOS_FIS		(1L << 1)		/* PIO Setup FIS rx'd */
#define	PORT_IRQ_D2H_REG_FIS	(1L << 0)		/* D2H Register FIS rx'd */

#if 0
#define	DEF_PORT_IRQ		 (MV_U32)(\
				PORT_IRQ_SIGNATURE_FIS | PORT_IRQ_TF_ERR |\
				PORT_IRQ_PHYRDY | \
				PORT_IRQ_BIST |	PORT_IRQ_ASYNC_NOTIF | \
				PORT_IRQ_LINK_RECEIVE_ERROR | PORT_IRQ_LINK_TRANSMIT_ERROR |\
				PORT_IRQ_PIO_DONE | PORT_IRQ_UNK_FIS | PORT_IRQ_SDB_FIS | PORT_IRQ_D2H_REG_FIS)
#else
#define	DEF_PORT_IRQ		 (MV_U32)(\
				PORT_IRQ_SIGNATURE_FIS | PORT_IRQ_TF_ERR |\
				PORT_IRQ_PHYRDY | \
				PORT_IRQ_BIST |	PORT_IRQ_ASYNC_NOTIF | \
				PORT_IRQ_PIO_DONE | PORT_IRQ_UNK_FIS | PORT_IRQ_SDB_FIS | PORT_IRQ_D2H_REG_FIS)
#endif

	/* PORT_IRQ_{STAT,MASK} bits for PATA port */
#define	PORT_IRQ_PATA_DEVICE0_DONE	MV_BIT(0)
#define	PORT_IRQ_PATA_DEVICE0_ERROR	MV_BIT(1)
#define	PORT_IRQ_PATA_DEVICE1_DONE	MV_BIT(2)
#define	PORT_IRQ_PATA_DEVICE1_ERROR	MV_BIT(3)
#define	DEF_PORT_PATA_IRQ	(PORT_IRQ_PATA_DEVICE0_DONE | PORT_IRQ_PATA_DEVICE0_ERROR\
								| PORT_IRQ_PATA_DEVICE1_DONE | PORT_IRQ_PATA_DEVICE1_ERROR)
#endif

#ifdef AHCI
	/* PORT_CMD bits */
#define	PORT_CMD_LIST_ON		(1L << 15)	/* cmd list DMA engine running */
#define	PORT_CMD_FIS_ON			(1L << 14)	/* FIS DMA engine running */
#define	PORT_CMD_FIS_RX			(1L << 4)		/* Enable FIS receive DMA engine */
#define	PORT_CMD_POWER_ON		(1L << 2)		/* Power up device */
#define	PORT_CMD_SPIN_UP		(1L << 1)		/* Spin up device */
#define	PORT_CMD_START			(1L << 0)		/* Enable port DMA engine */

#define	PORT_CMD_ICC_ACTIVE		(0x1L << 28)	/* Put i/f in active state */
#define	PORT_CMD_ICC_PARTIAL	(0x2L << 28)	/* Put i/f in partial state */
#define	PORT_CMD_ICC_SLUMBER	(0x6L << 28)	/* Put i/f in slumber state */
#else
	/* PORT_CMD bits for SATA port */
#define	PORT_CMD_LIST_ON		(1L << 15)	/* cmd list DMA engine running */
#define	PORT_CMD_FIS_ON			(1L << 14)	/* FIS DMA engine running */

#define	PORT_CMD_FIS_RX			(1L << 4)		/* Enable FIS receive DMA engine */
#define	PORT_CMD_START			(1L << 0)		/* Enable port DMA engine */

	/* PORT_CMD bits for PATA port */
#define	PORT_CMD_PATA_LIST_ON	MV_BIT(15)
#define	PORT_CMD_PATA_HARD_RESET	MV_BIT(3)
#define	PORT_CMD_PATA_INTERRUPT MV_BIT(1)
#define	PORT_CMD_PATA_START		MV_BIT(0)
#endif

#define	PORT_SSTATUS_IPM_NO_DEVICE	0x0L	/* IPM: device not present or communication not established */
#define	PORT_SSTATUS_IPM_ACTIVE		0x1L	/* IPM: Interface in active state */
#define	PORT_SSTATUS_IPM_PARTIAL	0x2L	/* IPM: Interface in partical power management state */
#define	PORT_SSTATUS_IPRM_SLUMBER	0x6L	/* IPM: Interface in slumber power management state */

#define	PORT_TF_STATUS_BSY	(1L<<7)	/* Task file status: BSY */
#define	PORT_TF_STATUS_DRQ 	(1L<<3)	/* Task file status: DRQ */
#define	PORT_TF_STATUS_ERR 	(1L<<0)	/* Task file status: ERR */


typedef enum _MV_QUEUE_COMMAND_RESULT
{
    MV_QUEUE_COMMAND_RESULT_FINISHED = 0,
    MV_QUEUE_COMMAND_RESULT_FULL,
    MV_QUEUE_COMMAND_RESULT_SENDTED,
} MV_QUEUE_COMMAND_RESULT;

#define SATA_GetCommandHeader(pPort, slot)	\
	((PMV_Command_Header)pPort->Cmd_List + slot)

#define PATA_GetCommandHeader(pPort, slot)	\
	((PMV_PATA_Command_Header)pPort->Cmd_List + slot)

#define Port_GetCommandTable(pPort, slot)	\
	((PMV_Command_Table)((MV_PU8)pPort->Cmd_Table + slot * SATA_CMD_TABLE_SIZE))

#endif /* CORE_SATA_H */

