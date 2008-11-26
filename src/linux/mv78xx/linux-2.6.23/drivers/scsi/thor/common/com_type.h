#if !defined(COM_TYPE_H)
#define COM_TYPE_H

#include "mv_os.h"
#include "com_list.h"
#include "com_define.h"

/*
 * Data Structure
 */
#define MAX_CDB_SIZE		16

struct _MV_SG_Table;
typedef struct _MV_SG_Table MV_SG_Table, *PMV_SG_Table;

struct _MV_SG_Entry;
typedef struct _MV_SG_Entry MV_SG_Entry, *PMV_SG_Entry;

struct _MV_Request;
typedef struct _MV_Request MV_Request, *PMV_Request;

#ifdef RAID_DRIVER
typedef struct _MV_XOR_Request MV_XOR_Request, *PMV_XOR_Request;
#endif

#define REQ_STATUS_SUCCESS				0x0
#define REQ_STATUS_NOT_READY			0x1
#define REQ_STATUS_MEDIA_ERROR			0x2
#define REQ_STATUS_BUSY					0x3
#define REQ_STATUS_INVALID_REQUEST		0x4
#define REQ_STATUS_INVALID_PARAMETER	0x5
#define REQ_STATUS_NO_DEVICE			0x6
#define REQ_STATUS_HAS_SENSE			0x7		/* Sense data structure is the SCSI "Fixed format sense datat" format. */
#define REQ_STATUS_ERROR				0x8		/* Generic error. No more to say. */

#define REQ_STATUS_ERROR_WITH_SENSE		0x10	/* Error with sense. Used to report error status to application */

#define REQ_STATUS_PENDING				0x80	/* Request initiator must set the status to REQ_STATUS_PENDING. */
#define REQ_STATUS_RETRY				0x81
#define REQ_STATUS_REQUEST_SENSE		0x82

/* SG Table and SG Entry */
struct _MV_SG_Entry
{
	MV_U32 Base_Address;
	MV_U32 Base_Address_High;
	MV_U32 Reserved0;
	MV_U32 Size;
};

struct _MV_SG_Table
{
	MV_U8 Max_Entry_Count;
	MV_U8 Valid_Entry_Count;
	MV_U8 Flag;
	MV_U8 Reserved0;
	MV_U32 Byte_Count;
//	MV_SG_Entry Entry[MAX_SG_ENTRY];
	PMV_SG_Entry Entry_Ptr;
};

/* 
 * MV_Request is the general request type passed through different modules. 
 * Must be 64 bit aligned.
 */
struct _MV_Request {
	List_Head Queue_Pointer;

#if 0
	union {
		struct {
			MV_U8	Target_Id; 
			MV_U8	Lun;
		};
		MV_U16 Device_Id;
	};
#else
	MV_U16 Device_Id;
#endif

	MV_U16 Req_Flag;         /* Check the REQ_FLAG definition */
	MV_U8 Scsi_Status;
	MV_U8 Tag;               /* Request tag */
	MV_U8 Req_Type;		 /* Check the REQ_TYPE definition */
#ifdef _OS_WINDOWS
	MV_U8 Splited_Count;                            
#elif defined(SUPPORT_ERROR_HANDLING) && defined(_OS_LINUX)/* _OS_WINDOWS */ 
	MV_U8 eh_flag;	/* mark a req after it is re-inserted into
			 * waiting_list due to error handling.
			 */
	MV_U8 Splited_Count;
	MV_U8 Reserved0[7]; 
#else
	MV_U8 Reserved0[1]; 
#endif /* _OS_WINDOWS */

	MV_PVOID Cmd_Initiator;		/* Which module(extension pointer) creates this request. */

	MV_U8 Sense_Info_Buffer_Length;
	MV_U8 Reserved1[3];
	MV_U32 Data_Transfer_Length; 

	MV_U8 Cdb[MAX_CDB_SIZE]; 
	MV_PVOID Data_Buffer; 
	MV_PVOID Sense_Info_Buffer;

	//TBD:PMV_SG_Table SG_Table;
	MV_SG_Table SG_Table;

	MV_PVOID Org_Req;	/* The original request. */ 
	MV_PVOID Context;
	MV_PVOID pRaid_Request;
	MV_PVOID Reserved2;	/* fill in the blank */

	MV_LBA LBA;
	MV_U32 Sector_Count;
	MV_U32 Cmd_Flag;
#if defined(_OS_LINUX) && defined(SUPPORT_ERROR_HANDLING)
	MV_DECLARE_TIMER(eh_timeout);
#endif /* _OS_LINUX && SUPPORT_ERROR_HANDLING */
#ifdef __AC_PROF__
	MV_ULONG stamp;
#endif /* __AC_PROF__ */
	void (*Completion)(MV_PVOID,PMV_Request);/* call back function */

};

#define MV_REQUEST_SIZE		sizeof(MV_Request)
/*
 * Request flag is the flag for the MV_Request data structure.
 */
#define REQ_FLAG_LBA_VALID		MV_BIT(0)		/* LBA and Sector_Count variable are valid */
#define REQ_FLAG_CMD_FLAG_VALID	MV_BIT(1)		/* Cmd_Flag is valid */
#define REQ_FLAG_RETRY			MV_BIT(2)
#define REQ_FLAG_INTERNAL_SG	MV_BIT(3)		/* SG Table is constructed by Scratch Buffer */
//will be removed

#define REQ_FLAG_USE_PHYSICAL_SG MV_BIT(4)
#define REQ_FLAG_USE_LOGICAL_SG MV_BIT(5)

/*
 * Request Type is the type of MV_Request.
 */
#define REQ_TYPE_OS				0
#define REQ_TYPE_SUBLD			1
#define REQ_TYPE_SUBBGA			2

/*
 * Command flag is the flag for the CDB command itself 
 */
/* The first 16 bit can be determined by the initiator. */
#define CMD_FLAG_NON_DATA		MV_BIT(0)		/* 1-non data; 0-data command */
#define CMD_FLAG_DMA			MV_BIT(1)		/* 1-DMA; 0-PIO */
#define CMD_FLAG_DATA_IN		MV_BIT(2)		/* 1-host read data; 0-host write data*/

/* The last 16 bit only can be set by the target. Only core driver knows the device characteristic. */
#define CMD_FLAG_NCQ			MV_BIT(16)
#define CMD_FLAG_TCQ			MV_BIT(17)
#define CMD_FLAG_48BIT			MV_BIT(18)
#define CMD_FLAG_PACKET			MV_BIT(19)		/* ATAPI packet command */

#ifdef RAID_DRIVER
/* XOR request types */
#define	XOR_REQUEST_WRITE		0
#define	XOR_REQUEST_COMPARE		1
#define	XOR_REQUEST_DMA			2

/* XOR request status */
#define XOR_STATUS_SUCCESS				0
#define XOR_STATUS_INVALID_REQUEST		1
#define XOR_STATUS_ERROR				2
#define XOR_STATUS_INVALID_PARAMETER	3

#ifdef SUPPORT_RAID6
#define XOR_SOURCE_SG_COUNT				11	//TBD	support 8 disks RAID5
#define XOR_TARGET_SG_COUNT				1	//TBD
typedef MV_U8	XOR_COEF, *PXOR_COEF;		/* XOR Coefficient */
#endif

struct _MV_XOR_Request {
	List_Head Queue_Pointer;

#if 0
	union {
		struct {
			MV_U8	Target_Id; 
			MV_U8	Lun;
		};
		MV_U16 Device_Id;
	};
#else
	MV_U16 Device_Id;
#endif
	MV_U8 Request_Type;						
	MV_U8 Request_Status;

	MV_U8 Source_SG_Table_Count;		/* how many items in the SG_Table_List */
	MV_U8 Target_SG_Table_Count;
	MV_U8 Reserved[2];
	
#ifndef SUPPORT_RAID6
	PMV_SG_Table Source_SG_Table_List[2];
	List_Head Target_SG_Table_List;
#else
	MV_SG_Table Source_SG_Table_List[XOR_SOURCE_SG_COUNT];
	MV_SG_Table Target_SG_Table_List[XOR_TARGET_SG_COUNT];
	XOR_COEF	Coef[XOR_TARGET_SG_COUNT][XOR_SOURCE_SG_COUNT];	//TBD: Use one task or several tasks.
#endif

	MV_U32 Error_Offset;	/* byte, not sector */
	MV_PVOID Cmd_Initiator;				/* Which module(extension pointer) creates this request. */
	MV_PVOID Context;
	void (*Completion)(MV_PVOID, PMV_XOR_Request);	/* call back function */
};
#endif

/* Resource type */
enum Resource_Type
{
	RESOURCE_CACHED_MEMORY = 0,
	RESOURCE_UNCACHED_MEMORY
};

/* Module event type */
enum Module_Event
{
	EVENT_MODULE_ALL_STARTED = 0,
#ifdef CACHE_MODULE_SUPPORT
	EVENT_DEVICE_CACHE_MODE_CHANGED,
#endif
	EVENT_DEVICE_ARRIVAL,
	EVENT_DEVICE_REMOVAL
};

/* 
 * Don't change the order here. 
 * Module_StartAll will start from big id to small id. 
 * Make sure module_set setting matches the Module_Id 
 * MODULE_HBA must be the first one. Refer to Module_AssignModuleExtension.
 * And HBA_GetNextModuleSendFunction has an assumption that the next level has larger ID.
 */
enum Module_Id
{
	MODULE_HBA = 0,
#ifdef CACHE_MODULE_SUPPORT
	MODULE_CACHE,
#endif

#ifdef RAID_DRIVER
	MODULE_RAID,
#endif
	MODULE_CORE,
	MAX_MODULE_NUMBER
};

/*
 * Exposed Functions
 */

/*
 *
 * Miscellaneous Definitions
 *
 */
/* Rounding */

/* Packed */

#define MV_MAX(x,y)		(((x) > (y)) ? (x) : (y))
#define MV_MIN(x,y)		(((x) < (y)) ? (x) : (y))

#define MV_MAX_U64(x, y)	(((x.value) > (y.value)) ? (x) : (y))	// added by xxp
#define MV_MIN_U64(x, y)	(((x.value) < (y.value)) ? (x) : (y))	// added by xxp

#define MV_MAX_U8			0xFF
#define MV_MAX_U16			0xFFFF
#define MV_MAX_U32			0xFFFFFFFFL	/* L postfix is necessary for 16 bit compiler */

#define OFFSET_OF(type,member)	((MV_U32)(MV_PTR_INTEGER)&(((type *)0)->member))

#define ROUNDING(value, align)	( ((value)+(align)-1)/(align)*(align) )

#endif

