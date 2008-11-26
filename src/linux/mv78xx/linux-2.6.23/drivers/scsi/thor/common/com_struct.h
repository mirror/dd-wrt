#ifndef COM_STRUCT_H
#define COM_STRUCT_H

#include "com_define.h"

#define GET_ALL							0xFF
#define ID_UNKNOWN						0x7F

#define MAX_NUM_ADAPTERS				2
#define MAX_LD_SUPPORTED				8
#ifdef SUPPORT_RAID6
#define MAX_HD_SUPPORTED				8
#else
#define MAX_HD_SUPPORTED				24
#endif
#ifndef _OS_BIOS
#define MAX_HD_SUPPORTED_API			32	// API is already reserved 32 HDs
#endif

#define MAX_BLOCK_PER_HD_SUPPORTED		8	//Can also be got from Adapter structure.
#define MAX_EXPANDER_SUPPORTED			0	//Can also be got from Adapter structure.
#define MAX_PM_SUPPORTED				4	//Can also be got from Adapter structure.
#define MAX_HD_BSL						32
#define MAX_BLOCK_SUPPORTED 			32

#define MAX_BGA_RATE					0xFA

#ifndef MV_GUID_SIZE
#define MV_GUID_SIZE					8
#endif

#define LD_MAX_NAME_LENGTH 				16

#define CACHE_WRITEBACK_ENABLE                   0
#define CACHE_WRITETHRU_ENABLE                   1
#define CACHE_ADAPTIVE_ENABLE                      2
#define CACHE_WRITE_POLICY                      (CACHE_WRITEBACK_ENABLE | CACHE_WRITETHRU_ENABLE | CACHE_ADAPTIVE_ENABLE)
#define CACHE_LOOKAHEAD_ENABLE                  MV_BIT(2)

#define CONSISTENCYCHECK_ONLY			0
#define CONSISTENCYCHECK_FIX			1

#define INIT_QUICK						0	//Just initialize first part size of LD
#define INIT_FULLFOREGROUND				1	//Initialize full LD size
#define INIT_FULLBACKGROUND				2	//Initialize full LD size background
#define INIT_NONE						3

#define BGA_CONTROL_START				0
#define BGA_CONTROL_RESTART				1
#define BGA_CONTROL_PAUSE				2
#define BGA_CONTROL_RESUME				3
#define BGA_CONTROL_ABORT				4
#define BGA_CONTROL_COMPLETE			5
#define BGA_CONTROL_IN_PROCESS			6
#define BGA_CONTROL_TERMINATE_IMMEDIATE	7
#define BGA_CONTROL_AUTO_PAUSE			8

#define LD_STATUS_FUNCTIONAL			0
#define LD_STATUS_DEGRADE				1 
#define LD_STATUS_DELETED				2
#define LD_STATUS_MISSING				3//LD missing in system.
#define LD_STATUS_OFFLINE				4
#define LD_STATUS_PARTIALLYOPTIMAL		5 //Specially for RAID 6 lost one HD
#define LD_STATUS_MIGRATION             MV_BIT(3)
#define LD_STATUS_INVALID				0xFF

#define LD_BGA_NONE						0
#define LD_BGA_REBUILD					MV_BIT(0)
#define LD_BGA_CONSISTENCY_FIX			MV_BIT(1)
#define LD_BGA_CONSISTENCY_CHECK		MV_BIT(2)
#define LD_BGA_INIT_QUICK				MV_BIT(3)
#define LD_BGA_INIT_BACK				MV_BIT(4)
#define LD_BGA_MIGRATION				MV_BIT(5)

#define LD_BGA_STATE_NONE				0
#define LD_BGA_STATE_RUNNING			1
#define LD_BGA_STATE_ABORTED			2
#define LD_BGA_STATE_PAUSED				3
#define LD_BGA_STATE_AUTOPAUSED			4
#define LD_BGA_STATE_DDF_PENDING		MV_BIT(7)

#define	LD_MODE_RAID0					0x0
#define	LD_MODE_RAID1					0x1
#define	LD_MODE_RAID1E					0x11
#define	LD_MODE_RAID5					0x5
#define	LD_MODE_RAID6					0x6
#define	LD_MODE_JBOD					0x0f
#define	LD_MODE_RAID10					0x10	//TBD
#define	LD_MODE_RAID50					0x50	//TBD
#define	LD_MODE_RAID60					0x60	//TBD

#define	LD_RAID0_SUPPORTED				MV_BIT(0)
#define	LD_RAID1_SUPPORTED				MV_BIT(1)
#define	LD_RAID1E_SUPPORTED				MV_BIT(2)
#define	LD_RAID5_SUPPORTED				MV_BIT(3)
#define	LD_RAID6_SUPPORTED				MV_BIT(4)
#define	LD_JBOD_SUPPORTED				MV_BIT(5)
#define	LD_RAID10_SUPPORTED				MV_BIT(6)
#define	LD_RAID50_SUPPORTED				MV_BIT(7)

#define HD_WIPE_MDD						0
#define HD_WIPE_FORCE					1

#define ROUNDING_SCHEME_NONE			0          // no rounding
#define ROUNDING_SCHEME_1GB				1          // 1 GB rounding
#define ROUNDING_SCHEME_10GB			2          // 10 GB rounding

#define DEVICE_TYPE_NONE				0
#define DEVICE_TYPE_HD					1
#define DEVICE_TYPE_PM					2
#define DEVICE_TYPE_EXPANDER			3
#define DEVICE_TYPE_PORT				0xFF

#define HD_STATUS_FREE					MV_BIT(0)
#define HD_STATUS_ASSIGNED				MV_BIT(1)
#define HD_STATUS_SPARE					MV_BIT(2)
#define HD_STATUS_OFFLINE				MV_BIT(3)
#define HD_STATUS_SMARTCHECKING			MV_BIT(4)
#define HD_STATUS_MP					MV_BIT(5)

#define HD_BGA_STATE_NONE				LD_BGA_STATE_NONE
#define HD_BGA_STATE_RUNNING			LD_BGA_STATE_RUNNING
#define HD_BGA_STATE_ABORTED			LD_BGA_STATE_ABORTED
#define HD_BGA_STATE_PAUSED				LD_BGA_STATE_PAUSED
#define HD_BGA_STATE_AUTOPAUSED			LD_BGA_STATE_AUTOPAUSED

#define GLOBAL_SPARE_DISK				MV_BIT(2)

#define PD_DDF_VALID					MV_BIT(0)
#define PD_DISK_VALID					MV_BIT(1)
#define PD_DDF_CLEAN					MV_BIT(2)
#define PD_NEED_UPDATE					MV_BIT(3)
#define PD_MBR_VALID					MV_BIT(4)

#define PD_STATE_ONLINE					MV_BIT(0)
#define PD_STATE_FAILED					MV_BIT(1)
#define PD_STATE_REBUILDING				MV_BIT(2)
#define PD_STATE_TRANSITION				MV_BIT(3)
#define PD_STATE_SMART_ERROR			MV_BIT(4)
#define PD_STATE_READ_ERROR				MV_BIT(5)
#define PD_STATE_MISSING				MV_BIT(6)

#define HD_STATUS_SETONLINE				0
#define HD_STATUS_SETOFFLINE			1

#define HD_TYPE_SATA					MV_BIT(0)
#define HD_TYPE_PATA					MV_BIT(1)
#define HD_TYPE_SAS						MV_BIT(2)
#define HD_TYPE_ATAPI					MV_BIT(3)	//SATA, PATA, can co-exist with ATAPI

#define HD_FEATURE_NCQ					MV_BIT(0)
#define HD_FEATURE_TCQ					MV_BIT(1)
#define HD_FEATURE_1_5G					MV_BIT(2)
#define HD_FEATURE_3G					MV_BIT(3)
#define HD_FEATURE_WRITE_CACHE			MV_BIT(4)
#define HD_FEATURE_48BITS				MV_BIT(5)
#define HD_FEATURE_SMART				MV_BIT(6)

#define EXP_SSP							MV_BIT(0)
#define EXP_STP							MV_BIT(1)
#define EXP_SMP							MV_BIT(2)

#define HD_DMA_NONE						0
#define HD_DMA_1						1
#define HD_DMA_2						2
#define HD_DMA_3						3
#define HD_DMA_4						4
#define HD_DMA_5						5
#define HD_DMA_6						6
#define HD_DMA_7						7
#define HD_DMA_8						8
#define HD_DMA_9						9

#define HD_PIO_NONE						0
#define HD_PIO_1						1
#define HD_PIO_2						2
#define HD_PIO_3						3
#define HD_PIO_4						4
#define HD_PIO_5						5

#define HD_XCQ_OFF						0
#define HD_NCQ_ON						1
#define HD_TCQ_ON						2

#define SECTOR_LENGTH					512
#define SECTOR_WRITE					0
#define SECTOR_READ						1

#define DBG_LD2HD						0
#define DBG_HD2LD						1

#define DRIVER_LENGTH					1024*16
#define FLASH_DOWNLOAD					0xf0
#define FLASH_UPLOAD					0xf
#define	FLASH_TYPE_CONFIG				0
#define	FLASH_TYPE_BIN					1
#define	FLASH_TYPE_BIOS					2
#define	FLASH_TYPE_FIRMWARE				3

#define BLOCK_INVALID					0
#define BLOCK_VALID						MV_BIT(0)	//Free block can be used to create LD.
#define BLOCK_ASSIGNED					MV_BIT(1)	//Block used for LD
#ifdef _OS_BIOS
#define FREE_BLOCK(Flags)	(Flags&(BLOCK_VALID)==Flags)
#define ASSIGN_BLOCK(Flags)	(Flags&(BLOCK_VALID|BLOCK_ASSIGNED)==Flags)
#define INVALID_BLOCK(Flags)	(Flags&(BLOCK_VALID|BLOCK_ASSIGNED)==0)
#endif
//#define BLOCK_STATUS_NORMAL				0
//#define BLOCK_STATUS_REBUILDING			MV_BIT(0)
//#define BLOCK_STATUS_CONSISTENTCHECKING	MV_BIT(1)
//#define BLOCK_STATUS_INITIALIZING		MV_BIT(2)
//#define BLOCK_STATUS_MIGRATING			MV_BIT(3)
//#define BLOCK_STATUS_OFFLINE			MV_BIT(4)

#ifndef _OS_BIOS
#pragma pack(8)
#endif

typedef struct _Version_Info
{
	MV_U32		VerMajor;
	MV_U32		VerMinor;
	MV_U32		VerOEM;
	MV_U32		VerBuild;
}Version_Info, *PVersion_Info;

#define BASE_ADDRESS_MAX_NUM 6

#define SUPPORT_LD_MODE_RAID0		MV_BIT(0)
#define SUPPORT_LD_MODE_RAID1		MV_BIT(1)
#define SUPPORT_LD_MODE_RAID10		MV_BIT(2)
#define SUPPORT_LD_MODE_RAID1E		MV_BIT(3)
#define SUPPORT_LD_MODE_RAID5		MV_BIT(4)
#define SUPPORT_LD_MODE_RAID6		MV_BIT(5)
#define SUPPORT_LD_MODE_RAID50		MV_BIT(6)
#define SUPPORT_LD_MODE_JBOD		MV_BIT(7)

typedef struct _Adapter_Info {

	Version_Info	DriverVersion;
	Version_Info	BIOSVersion;
	MV_U64			Reserved1[2];//Reserve for firmware

	MV_U32			SystemIOBusNumber;
	MV_U32			SlotNumber;
	MV_U32			InterruptLevel;
	MV_U32			InterruptVector;
	
	MV_U32			VenDevID;
	MV_U32			SubVenDevID;
	
	MV_U8			PortCount;		//How many ports, like 4 ports,  or 4S1P.
	MV_U8			PortSupportType;		//Like SATA port, SAS port, PATA port, use MV_BIT
	MV_BOOLEAN		RAMSupport;
	MV_U8			Reserved2[13];
	
	MV_BOOLEAN		AlarmSupport;
	MV_U8			MaxBlockPerPD;	//E.g one HD support 8 block at maximum.
	MV_U8			MaxHD;		//E.g 16 HD support
	MV_U8			MaxExpander;	//E.g 4 Expander support
	MV_U8			MaxPM;		//E.g 4 PM support
	MV_U8			MaxLogicalDrive;
	MV_U16			LogicalDriverMode;	// check SUPPORT_LD_MODE definition
	MV_U8			WWN[8];			//For future VDS use.

} Adapter_Info, *PAdapter_Info;

typedef struct _Adapter_Config {
	MV_BOOLEAN		AlarmOn;
	MV_BOOLEAN		AutoRebuildOn;
	MV_U8			BGARate;
	MV_BOOLEAN		PollSMARTStatus;
	MV_U8			Reserved[4];
} Adapter_Config, *PAdapter_Config;

typedef struct _HD_Info
{
	MV_U8			Type;			/*Refer to DEVICE_TYPE_xxx*/
	MV_U8			ParentType;		//Refer to DEVICE_TYPE_xxx
	MV_U16			ID;			/* ID should be unique*/
	MV_U16			ParentID;		//Point to Port, PM or Expander ID
	MV_U8			AdapterID;
	MV_U8			PhyID;	/* Means HD attached to which Phy of the Expander or PM or Adapter.*/

	MV_U8			Status;		/*Refer to HD_STATUS_XXX*/
	MV_U8			HDType;	/*For HD type, refer to HD_Type_xxx*/
	MV_U8			PIOMode;
	MV_U8			MDMAMode;
	MV_U8			UDMAMode;	
	MV_U8			Reserved1[3];

	MV_U32			FeatureSupport;	/*Support 1.5G, 3G, TCQ, NCQ, and etc, MV_BIT related*/

	MV_U8			Model[40];
	MV_U8			SerialNo[20];
	MV_U8			FWVersion[8];

	MV_U8			WWN[64];	/*ATA/ATAPI-8 has such definitions for the identify buffer*/
	MV_U8			Reserved3[64];

	MV_U64		   	Size;		//unit: 1KB
}
HD_Info, *PHD_Info;

typedef struct _HD_MBR_Info
{
	MV_U8 			HDCount;
	MV_U8			Reserved[7];
	MV_U16	 		HDIDs[MAX_HD_SUPPORTED_API];	
	MV_BOOLEAN	 	hasMBR[MAX_HD_SUPPORTED_API];
} HD_MBR_Info, *PHD_MBR_Info;


typedef struct _HD_FreeSpaceInfo
{
	MV_U16			ID;			/* ID should be unique*/
	MV_U8			AdapterID;
	MV_U8			Reserved[4];
	MV_BOOLEAN		isFixed;

	MV_U64		   	Size;		//unit: 1KB
}
HD_FreeSpaceInfo, *PHD_FreeSpaceInfo;


typedef struct _HD_Block_Info
{
	MV_U16			ID;			/* ID in the HD_Info*/
	MV_U8			Type;			/*Refer to DEVICE_TYPE_xxx*/
	MV_U8			Reserved1[5];

	MV_U16			BlockIDs[MAX_BLOCK_PER_HD_SUPPORTED];  // Free is 0xff
}
HD_Block_Info, *PHD_Block_Info;

typedef struct _Exp_Info
{
	MV_U8			Type;			/*Refer to DEVICE_TYPE_xxx */
	MV_U8			ParentType;		//Refer to DEVICE_TYPE_xxx
	MV_U16 			ID;			//ID should be unique
	MV_U16			ParentID;		//Point to Port or Expander ID
	MV_U8			AdapterID;
	MV_U8			PhyID;	/* Means this Expander attached to which Phy of the Expander or Adapter.*/

	MV_U8			SAS_Address[8];

	MV_BOOLEAN		Configuring;	
	MV_BOOLEAN		RouteTableConfigurable;
	MV_U16			Reserved1[2];
	MV_U8 			PhyCount;
	MV_U8			Reserved2[1];

	MV_U16			ExpChangeCount;
	MV_U16			MaxRouteIndexes;
	MV_U32			Reserved3[1];

	char			VendorID[8+1];
	char			ProductID[16+1];
	char			ProductRev[4+1];
	char			ComponentVendorID[8+1];
	MV_U16			ComponentID;
	MV_U8			ComponentRevisionID;

	MV_U8			Reserved4[8];
}Exp_Info, * PExp_Info;

typedef  struct _PM_Info{
	MV_U8     		Type;			// Refer to DEVICE_TYPE_xxx 
	MV_U8			ParentType;		// Refer to DEVICE_TYPE_xxx
	MV_U16			ID;				// the same as port ID it attached
	MV_U16			ParentID;
	MV_U16			VendorId;

	MV_U16			DeviceId;
	MV_U8			AdapterID;
	MV_U8			ProductRevision;
   	MV_U8			PMSpecRevision;	// 10 means 1.0, 11 means 1.1 etc
	MV_U8			NumberOfPorts;
	MV_U8			PhyID;	/* Means this PM attached to which Phy of the Expander or Adapter.*/
	MV_U8			Reserved[1];
}PM_Info, *PPM_Info;

typedef struct _HD_CONFIG
{
	MV_BOOLEAN		WriteCacheOn;		/* 1: enable write cache */
	MV_BOOLEAN		SMARTOn;			/* 1: enable S.M.A.R.T */
	MV_BOOLEAN		Online;				/* 1: to set HD online */
	MV_U8			Reserved[3];
	MV_U16			HDID;
}
HD_Config, *PHD_Config;

typedef struct  _HD_STATUS
{
	MV_BOOLEAN		SmartThresholdExceeded;		
	MV_U8      		Reserved[1];
	MV_U16			HDID;
}
HD_Status, *PHD_Status;

typedef struct  _SPARE_STATUS
{
	MV_U16			HDID;
	MV_U16			LDID;
	MV_U8			Status;		// HD_STATUS_SPARE
	MV_U8      		Reserved[3];
}
Spare_Status, *PSpare_Status;

typedef struct  _BSL{
	MV_U64     	LBA;		//Bad sector LBA for the HD.

	MV_U32			Count;		//How many serial bad sectors 
	MV_BOOLEAN		Flag;		//Fake bad sector or not.
	MV_U8      	Reserved[3];
}
BSL,*PBSL;

typedef struct _BLOCK_INFO
{
	MV_U16      	ID;
	MV_U16			HDID;		//ID in the HD_Info
	MV_U16 		Flags;		/*Refer to BLOCK_XXX definition*/
	MV_U16			LDID;		/*Belong to which LD*/

	MV_U8			Status;		/* Refer to BLOCK_STATUS_XXX*/
	MV_U8 			Reserved[7];

	MV_U64 		StartLBA;	//unit: 1KB
	MV_U64 		Size;		//unit: 1KB
}
Block_Info, *PBlock_Info;

typedef struct _LD_Info
{
	MV_U16			ID;
	MV_U8 			Status;	/* Refer to LD_STATUS_xxx */
	MV_U8			BGAStatus; /* Refer to LD_BGA_STATE_xxx */
	MV_U16			StripeBlockSize;	//unit: 1KB
	MV_U8			RaidMode;			
	MV_U8			HDCount;

	MV_U8			CacheMode;/*Default is CacheMode_Default, see above*/	
	MV_U8 			LD_GUID[MV_GUID_SIZE];
	MV_U8 			Reserved[7];

	MV_U64			Size;			/* LD size, unit: 1KB */

	MV_U8			Name[LD_MAX_NAME_LENGTH];

	MV_U16			BlockIDs[MAX_HD_SUPPORTED_API];		/* 32 */
//According to BLOCK ID, to get the related HD ID, then WMRU can draw the related graph like above.
	MV_U8			SubLDCount;   //for raid 10, 50,60
	MV_U8			NumParityDisk; //For RAID 6.
	MV_U8			Reserved1[6];
}
LD_Info, *PLD_Info;

typedef struct _Create_LD_Param
{
	MV_U8 			RaidMode;
	MV_U8 			HDCount;
	MV_U8			RoundingScheme;//please refer to the definitions of  ROUNDING_SCHEME_XXX.
	MV_U8			SubLDCount;   	//for raid 10,50,60
	MV_U16			StripeBlockSize; /*In sectors unit: 1KB */
	MV_U8			NumParityDisk;  //For RAID 6.
	MV_U8			CachePolicy;//please refer to the definitions of CACHEMODE_XXXX.

	MV_U8			InitializationOption;// please refer to the definitions of INIT_XXXX.
	MV_U8  			Reserved1;
	MV_U16			LDID;			// ID of the LD to be migrated or expanded
	MV_U8  			Reserved2[4];

	MV_U16	 		HDIDs[MAX_HD_SUPPORTED_API];	/* 32 */
	MV_U8	 		Name[LD_MAX_NAME_LENGTH];

	MV_U64			Size;		/* size of LD in sectors */
} Create_LD_Param, *PCreate_LD_Param;

typedef struct _LD_STATUS
{
	MV_U8			Status;		/* Refer to LD_STATUS_xxx */
	MV_U8			Bga;		/* Refer to LD_BGA_xxx */
	MV_U16			BgaPercentage;	/* xx% */
	MV_U8			BgaState;	/* Refer to LD_BGA_STATE_xxx */
	MV_U8			Reserved[1];
	MV_U16			LDID;
} 
LD_Status, *PLD_Status;

typedef struct	_LD_Config
{
	MV_U8			CacheMode;		/* See definition 4.4.1 CacheMode_xxx */
	MV_U8			Reserved1;		
	MV_BOOLEAN		AutoRebuildOn;	/*1- AutoRebuild On*/
	MV_U8			Status;
	MV_U16			LDID;
	MV_U8			Reserved2[2];

	MV_U8 			Name[LD_MAX_NAME_LENGTH];
}
LD_Config, * PLD_Config;

typedef struct _HD_MPSTATUS
{
	MV_U64			Watermark;
	MV_U16			LoopCount;			/* loop count */
	MV_U16			ErrorCount;	/* error detected during media patrol */
	MV_U16			Percentage;	/* xx% */
	MV_U8			Status;		/* Refer to HD_BGA_STATE_xxx */
	MV_U8			Type;
	MV_U16			HDID;
	MV_U16			Reserved [3];
} 
HD_MPStatus, *PHD_MPStatus;

typedef struct _DBG_DATA
{
	MV_U64			LBA;
	MV_U64			Size;
	MV_U8			Data[SECTOR_LENGTH];
} 
DBG_Data, *PDBG_Data;

typedef struct _DBG_HD
{
	MV_U64			LBA;
	MV_U16			HDID;
	MV_BOOLEAN		isUsed;
	MV_U8			Reserved[5];
} 
DBG_HD;

typedef struct _DBG_MAP
{
	MV_U64			LBA;
	MV_U16			LDID;
	MV_BOOLEAN		isUsed;
	MV_U8			Reserved[5];
	DBG_HD		HDs[MAX_HD_SUPPORTED_API];
} 
DBG_Map, *PDBG_Map;

#ifdef CACHE_MODULE_SUPPORT
typedef struct _LD_CACHE_STATUS
{
	MV_BOOLEAN	IsOnline;
	MV_U8		CachePolicy;
	MV_U16		StripeUnitSize;
	MV_U32		StripeSize;
}
LD_CACHE_STATUS, *PLD_CACHE_STATUS;
#endif

#ifndef _OS_BIOS
#pragma pack()
#endif

#endif /* COM_STRUCT_H */

