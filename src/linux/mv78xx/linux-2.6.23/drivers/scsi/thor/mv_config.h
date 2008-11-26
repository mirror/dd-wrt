#ifndef __MV_CONFIGURATION_H__
#define __MV_CONFIGURATION_H__

/* HBA macro definition */
#define MV_MAX_TRANSFER_SIZE    (128*1024)
#define MAX_REQUEST_NUMBER      32
#define MAX_BASE_ADDRESS        6

/* Core driver macro definition */
#define MAX_DEVICE_SUPPORTED    32
#define MAX_HD_NUMBER           32
#define MAX_SG_ENTRY            32
#define MAX_SG_ENTRY_REDUCED    16

//#define ENABLE_PATA_ERROR_INTERRUPT

#ifndef ENABLE_PATA_ERROR_INTERRUPT
	#define USE_DMA_FOR_ALL_PACKET_COMMAND
	/* It's dangerous. Never enable it unless we have to. */
	#define PRD_SIZE_WORD_ALIGN	
#endif

//#define HIBERNATION_ROUNTINE

//#define CORE_SUPPORT_API

//#define SUPPORT_SCSI_PASSTHROUGH

//#define SUPPORT_CONSOLIDATE

/* hot plug & PM */
#define SUPPORT_HOT_PLUG        1
#define SUPPORT_PM              1

//#define SUPPORT_TIMER 1
#define SUPPORT_ERROR_HANDLING  1

//#define SUPPORT_CONSOLIDATE	1

#ifdef SUPPORT_SCSI_PASSTHROUGH
#define MV_MAX_TARGET_NUMBER    21 // console
#else
#define MV_MAX_TARGET_NUMBER    22 // max 5 ports, 4 device each
#endif /* SUPPORT_SCSI_PASSTHROUGH */
#define MV_MAX_LUN_NUMBER       1

#define CONSOLE_ID (MV_MAX_TARGET_NUMBER - 1) * MV_MAX_LUN_NUMBER

/* RAID */
#ifdef RAID_DRIVER
#define BGA_SUPPORT             1
#define SOFTWARE_XOR            1
#define SUPPORT_FREE_POLICY     1
#define SUPPORT_RAID1E          1
//#define SUPPORT_SRL           1

//#define SUPPORT_RAID6
#ifdef SUPPORT_RAID6
#define USE_MATH_LIBARY
#define SUPPORT_READ_MODIFY_WRITE
#endif /* SUPPORT_RAID6 */

#endif /* RAID_DRIVER */

//#define SUPPORT_CACHE         1

#endif /* __MV_CONFIGURATION_H__ */
