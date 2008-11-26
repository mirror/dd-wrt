/******************************************************************************
 *
 * Name:    skgeasf.h
 * Project: asf/ipmi
 * Version: $Revision: 1.1.2.2 $
 * Date:    $Date: 2007/06/27 15:55:09 $
 * Purpose: asf/ipmi interface in windows driver
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  (C)Copyright 1998-2002 SysKonnect.
 *  (C)Copyright 2002-2003 Marvell.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  The information in this file is provided "AS IS" without warranty.
 *
 ******************************************************************************/

#ifndef _INC_SKGEASF_H_
#define _INC_SKGEASF_H_

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* DEFINES */
#define ASF_FW_ARP_RESOLVE                  //  FW is resolving the destination IP addr
#define ASF_FW_WD                           //  Driver is watching the FW
#define ASF_CHECK_HIDDEN_ID                 //  ASF init checks hidden id

// modes for the asf driver
#define SK_GEASF_MODE_UNKNOWN               0       // unknown operation mode (initial)
#define SK_GEASF_MODE_ASF                   1       // asfec.bin binary found -> ASF operation
#define SK_GEASF_MODE_IPMI                  2       // ipmiy2.bin binary found -> IPMI operation
#define SK_GEASF_MODE_DASH                  3       // dashex.bin binary found -> DASH operation

// chip modes for asf driver
#define SK_GEASF_CHIP_UNKNOWN               0       // bad chip id / hidden id
#define SK_GEASF_CHIP_EC                    1       // EC: ASF
#define SK_GEASF_CHIP_Y2                    2       // Yukon2
#define SK_GEASF_CHIP_EX                    3       // Yukon Extreme

// dual link mode
#define SK_GEASF_Y2_SINGLEPORT              1       // Yukon2 sigle link adapter
#define SK_GEASF_Y2_DUALPORT                2       // Yukon2 dual link adapter

#define ASF_GUI_TSF                         10      /* Time Scale Factor: 1s(GUI) <-> 10*100ms(FW) */

#define ASF_MAX_STRLEN                      64

// lengths used in get oid
#define ASF_ACPI_MAXBUFFLENGTH              256     // max bytes for holding ACPI table
#define ASF_SMBUS_MAXBUFFLENGTH             128     // max bytes for holding SMBus info
#define ASF_FWVER_MAXBUFFLENGTH             40      // max stringlen for firmware version

#define SK_ASF_EVT_TIMER_EXPIRED            1       /* Counter overflow */
#define SK_ASF_EVT_TIMER_SPI_EXPIRED        2       /* Counter overflow */
#define SK_ASF_EVT_TIMER_HCI_EXPIRED        3       /* Counter overflow */

/*  Return codes to the PNMI module  */
#define SK_ASF_PNMI_ERR_OK              0
#define SK_ASF_PNMI_ERR_GENERAL         1
#define SK_ASF_PNMI_ERR_TOO_SHORT       2
#define SK_ASF_PNMI_ERR_BAD_VALUE       3
#define SK_ASF_PNMI_ERR_READ_ONLY       4
#define SK_ASF_PNMI_ERR_UNKNOWN_OID     5
#define SK_ASF_PNMI_ERR_UNKNOWN_INST    6
#define SK_ASF_PNMI_ERR_UNKNOWN_NET     7
#define SK_ASF_PNMI_ERR_NOT_SUPPORTED   10

#define REG_ASF_MAC_ADDR        0x0f24
#define REG_ASF_MY_IP           0x0f50
#define REG_ASF_STATUS_CMD      0x0e68
#define REG_ASF_SMBUS_CFG       0x0e40

#define ASF_CPU_STATE_UNKNOWN       0
#define ASF_CPU_STATE_RESET         1
#define ASF_CPU_STATE_RUNNING       2


/*  ASF MIB default values */
#define ASF_DEF_RETRANS_COUNT_MIN   0           // x1
#define ASF_DEF_RETRANS_COUNT_MAX   255         // x1
#define ASF_DEF_WATCHDOG_ENA        0           //
#define ASF_DEF_WATCHDOG_TIME       1200        // *100ms
#define ASF_DEF_WATCHDOG_TIME_MAX   36000       // *100ms
#define ASF_DEF_WATCHDOG_TIME_MIN   600         // *100ms
#define ASF_DEF_RETRANS_INT_MIN     0           // *100ms
#define ASF_DEF_RETRANS_INT_MAX     2550        // *100ms
#define ASF_DEF_HB_INT_MIN          10          // *100ms
#define ASF_DEF_HB_INT_MAX          2550        // *100ms
#define ASF_DEF_ASF_ENA             0
#define ASF_DEF_RETRANS             2
#define ASF_DEF_RETRANS_INT         10          // *100ms
#define ASF_DEF_HB_ENA              0
#define ASF_DEF_HB_INT              600         // *100ms

/* ASF HCI Commands */
#define YASF_HOSTCMD_ASF_INFO                   1
#define YASF_HOSTCMD_LEG_CONF                   2
#define YASF_HOSTCMD_ASF_CONF                   3
#define YASF_HOSTCMD_RCTRL_CONF                 4
#define YASF_HOSTCMD_KEEP_ALIVE                 5
#define YASF_HOSTCMD_NEW_SEPROM_CONFIG          6
#define YASF_HOSTCMD_ENTER_RAM_IDLE             7
#define YASF_HOSTCMD_LEAVE_RAM_IDLE             8
#define YASF_HOSTCMD_RUN_DIAG                   9
#define YASF_HOSTCMD_RESET_STATE                10
#define YASF_HOSTCMD_RESET                      11
#define YASF_HOSTCMD_CHECK_ALIVE                12
#define YASF_HOSTCMD_DRV_HELLO                  13
#define YASF_HOSTCMD_DRV_GOODBYE                14
#define YASF_HOSTCMD_DRV_STANDBY                15
#define YASF_HOSTCMD_UPDATE_OWN_MACADDR         16
#define YASF_HOSTCMD_ARP_RESOLVE                17
#define YASF_HOSTCMD_RESET_COLD                 18
#define YASF_HOSTCMD_ACPI_RMCP_DATA             19
#define YASF_HOSTCMD_ACPI_ERROR                 20

#define YASF_HOSTCMD_CFG_SET_ASF_ENABLE         100
#define YASF_HOSTCMD_CFG_SET_RSP_ENABLE         101
#define YASF_HOSTCMD_CFG_SET_RETRANS            102
#define YASF_HOSTCMD_CFG_SET_RETRANS_INT        103
#define YASF_HOSTCMD_CFG_SET_HB_ENABLE          104
#define YASF_HOSTCMD_CFG_SET_HB_INT             105
#define YASF_HOSTCMD_CFG_SET_IP_DESTINATION     106
#define YASF_HOSTCMD_CFG_SET_IP_SOURCE          107
#define YASF_HOSTCMD_CFG_SET_MAC_DESTINATION    108
#define YASF_HOSTCMD_CFG_SET_COMMUNITY_NAME     109
#define YASF_HOSTCMD_CFG_SET_RSP_KEY_1          110
#define YASF_HOSTCMD_CFG_SET_RSP_KEY_2          111
#define YASF_HOSTCMD_CFG_SET_RSP_KEY_3          112
#define YASF_HOSTCMD_CFG_SET_DRWD_ENABLE        113
#define YASF_HOSTCMD_CFG_SET_DRWD_INT           114
#define YASF_HOSTCMD_CFG_SET_WD_ENABLE          115
#define YASF_HOSTCMD_CFG_SET_WD_INT             116
#define YASF_HOSTCMD_CFG_SET_ASF_RAMSIZE        117
#define YASF_HOSTCMD_CFG_SET_ACTIVE_PORT        118
#define YASF_HOSTCMD_CFG_SET_DASH_ENABLE        119
#define YASF_HOSTCMD_CFG_SET_DEBUG_INFO         120

#define YASF_HOSTCMD_CFG_STORE_CONFIG           130

#define YASF_HOSTCMD_CFG_GET_ASF_ENABLE         150
#define YASF_HOSTCMD_CFG_GET_RSP_ENABLE         151
#define YASF_HOSTCMD_CFG_GET_RETRANS            152
#define YASF_HOSTCMD_CFG_GET_RETRANS_INT        153
#define YASF_HOSTCMD_CFG_GET_HB_ENABLE          154
#define YASF_HOSTCMD_CFG_GET_HB_INT             155
#define YASF_HOSTCMD_CFG_GET_IP_DESTINATION     156
#define YASF_HOSTCMD_CFG_GET_IP_SOURCE          157
#define YASF_HOSTCMD_CFG_GET_MAC_DESTINATION    158
#define YASF_HOSTCMD_CFG_GET_COMMUNITY_NAME     159
#define YASF_HOSTCMD_CFG_GET_RSP_KEY_1          160
#define YASF_HOSTCMD_CFG_GET_RSP_KEY_2          161
#define YASF_HOSTCMD_CFG_GET_RSP_KEY_3          162
#define YASF_HOSTCMD_CFG_GET_DRWD_ENABLE        163
#define YASF_HOSTCMD_CFG_GET_DRWD_INT           164
#define YASF_HOSTCMD_CFG_GET_WD_ENABLE          165
#define YASF_HOSTCMD_CFG_GET_WD_INT             166
#define YASF_HOSTCMD_CFG_GET_ASF_RAMSIZE        167
#define YASF_HOSTCMD_CFG_GET_FW_VERSION_STRING  168
#define YASF_HOSTCMD_CFG_GET_SMBUS_INFOS        169
#define YASF_HOSTCMD_CFG_GET_ACTIVE_PORT        170
#define YASF_HOSTCMD_CFG_READ_CONFIG            180


/* ASF HCI Master */
#define ASF_HCI_READ            0x08000000
#define ASF_HCI_WRITE           0x04000000
#define ASF_HCI_CMD_RD_READY    0x02000000
#define ASF_HCI_CMD_WR_READY    0x01000000
#define ASF_HCI_UNSUCCESS       0x00800000
#define ASF_HCI_OFFSET          0x000000ff

#define ASF_HCI_CMDREG          0x0e70
#define ASF_HCI_DATAREG         0x0e78

#define ASF_HCI_WAIT            1
#define ASF_HCI_NOWAIT          0

#define ASF_HCI_TO              100     /* 1s */

#define HCI_EN_CMD_IDLE             0
#define HCI_EN_CMD_WRITING          1
#define HCI_EN_CMD_READING          2
#define HCI_EN_CMD_WAIT             3
#define HCI_EN_CMD_READY            4
#define HCI_EN_CMD_ERROR            5

#define ASF_HCI_REC_BUF_SIZE    128
#define ASF_HCI_TRA_BUF_SIZE    128


/*  SEPROM  (VPD) */
#define ASF_VPD_CONFIG_BASE     0x340
#define ASF_VPD_CONFIG_SIZE     0x80
#define ASF_VPD_DATA_BASE       0x3c0
#define ASF_VPD_DATA_SIZE       0x40

/*  Flash (SPI)*/
#define ASF_FLASH_SIZE          (1024*64)
#define ASF_FLASH_OFFS          0x20000
#define ASF_FLASH_OFFS_VER      0x1fc00
#define ASF_FLASH_OFFS_REV      0x1fc0b
#define ASF_FLASH_OFFS_CS       0x1fffc
#define ASF_FLASH_OFFS_GUID     0x1f000
#define ASF_FLASH_OFFS_ACPI     0x1f010
#define ASF_DASH_FLASH_SIZE_1   65536
#define ASF_DASH_FLASH_SIZE_2   53248

/*  Yukon Extreme  */
#define ASF_FLASH_EX_OFFS          0x20000
#define ASF_FLASH_EX_OFFS_VER      0x1cc00
#define ASF_FLASH_EX_OFFS_REV      0x1cc0b
#define ASF_FLASH_EX_OFFS_CS       0x1cffc
#define ASF_FLASH_EX_OFFS_GUID     0x1c000
#define ASF_FLASH_EX_OFFS_ACPI     0x1c010


#define ASF_RESET_HOT           0
#define ASF_RESET_COLD          1

#define ASF_INIT_UNDEFINED      0
#define ASF_INIT_OK             1
#define ASF_INIT_ERROR          2
#define ASF_INIT_ERROR_CHIP_ID  3
#define ASF_INIT_ERROR_OPMODE   4

#define RSP_KEYLENGTH           20

//  ACPI module defines
#define ASF_ACPI_STATE_OK                       1
#define ASF_ACPI_STATE_UNDEFINED                0
#define ASF_ACPI_STATE_ERROR                    -1
#define ASF_ACPI_STATE_ERROR_NO_RSDPTR          -2
#define ASF_ACPI_STATE_ERROR_RSDT               -3
#define ASF_ACPI_STATE_ERROR_XSDT               -4
#define ASF_ACPI_STATE_ERROR_RSDT_NO_TABLE      -5
#define ASF_ACPI_STATE_ERROR_RSDT_HEADER        -6
#define ASF_ACPI_STATE_ERROR_ASF                -7
#define ASF_ACPI_STATE_ERROR_ASF_HEADER         -8
#define ASF_ACPI_STATE_ERROR_RSDT_NO_ASF_TABLE  -9
#define ASF_ACPI_STATE_ERROR_FILE_OPEN          -10
#define ASF_ACPI_STATE_ERROR_FILE_MAP           -11
#define ASF_ACPI_STATE_ERROR_FILE_SIZE          -12
#define ASF_ACPI_STATE_ERROR_FILE_CS            -13

#define ASF_RECORD_INFO                 0x00
#define ASF_RECORD_ALRT                 0x01
#define ASF_RECORD_RCTL                 0x02
#define ASF_RECORD_RMCP                 0x03
#define ASF_RECORD_ADDR                 0x04

#define TABLE_HEADER_LENGTH             36
#define SEC_COMMIT                      0x08000000


// endianess depended macros

#define REVERSE_16(x)   ((((x)<<8)&0xff00) + (((x)>>8)&0x00ff))

#define REVERSE_32(x)   ( ((((SK_U32)(x))<<24UL)&0xff000000UL) + \
                          ((((SK_U32)(x))<< 8UL)&0x00ff0000UL) + \
                          ((((SK_U32)(x))>> 8UL)&0x0000ff00UL) + \
                          ((((SK_U32)(x))>>24UL)&0x000000ffUL) )

#ifdef SK_LITTLE_ENDIAN
#define NTOHS(x) REVERSE_16(x)
#define HTONS(x) REVERSE_16(x)
#define NTOHL(x) REVERSE_32(x)
#define HTONL(x) REVERSE_32(x)
#else
#define NTOHS(x) (x)
#define HTONS(x) (x)
#define NTOHL(x) (x)
#define HTONL(x) (x)
#endif

/*
 *  ASF MIB structure
 */
struct _STR_PET_DAT
{
    SK_U8   EventSensorType;
    SK_U8   EventType;
    SK_U8   EventOffset;
    SK_U8   TrapSourceType;
    SK_U8   EventSourceType;
    SK_U8   EventSeverity;
    SK_U8   SensorDevice;
    SK_U8   SensorNumber;
    SK_U8   Entity;
    SK_U8   EntityInstance;
    SK_U8   EventData [8];
    SK_U8   LanguageCode;
    SK_U8   OemCustomField [64];
    //  83 Bytes  so far
};
typedef struct _STR_PET_DAT STR_PET_DAT;

// structure for ACPI data for reporting to GUI
struct _STR_ASF_ACPI
{
    SK_U8  buffer [ASF_ACPI_MAXBUFFLENGTH];
    SK_U32 length;
};
typedef struct _STR_ASF_ACPI STR_ASF_ACPI;

// structure for SMBus data for reporting to GUI
struct _STR_ASF_SMBUSINFO
{
    SK_U8  UpdateReq;
    SK_U8  buffer [ASF_SMBUS_MAXBUFFLENGTH];
    SK_U32 length;
};
typedef struct _STR_ASF_SMBUSINFO STR_ASF_SMBUSINFO;

struct _STR_ASF_MIB
{
    SK_U8               WriteToFlash;
    SK_U8               ConfigChange;
    //  Configuration parameter related to registers
    SK_U8               NewParam;
    SK_U8               Ena;
    SK_U16              Retrans;
    SK_U32              RetransInt;
    SK_U8               HbEna;
    SK_U32              HbInt;
    SK_U8               WdEna;
    SK_U32              WdTime;
    SK_U8               IpSource        [4];
    SK_U8               MacSource       [6];
    SK_U8               IpDest          [4];
    SK_U8               MacDest         [6];
    SK_U8               CommunityName   [64];
    SK_U8               Guid            [16];
    SK_U8               RspEnable;
    SK_U32              RetransCountMin;
    SK_U32              RetransCountMax;
    SK_U32              RetransIntMin;
    SK_U32              RetransIntMax;
    SK_U32              HbIntMin;
    SK_U32              HbIntMax;
    SK_U32              WdTimeMax;
    SK_U32              WdTimeMin;
    SK_U8               KeyOperator         [RSP_KEYLENGTH];
    SK_U8               KeyAdministrator    [RSP_KEYLENGTH];
    SK_U8               KeyGenerator        [RSP_KEYLENGTH];
    STR_ASF_ACPI        Acpi;
    STR_ASF_SMBUSINFO   SMBus;
    SK_U8               RlmtMode;
    SK_U8               Reserved            [6];    // reserved bytes in vpd
    SK_U8               PattUpReq;
};
typedef struct _STR_ASF_MIB STR_ASF_MIB;

typedef struct s_Hci
{
    SK_U32      To;
    SK_U8       Status;
    SK_U8       OldStatus;
    SK_U32      OldCmdReg;
    SK_U8       SendIndex;
    SK_U8       ReceiveIndex;
    SK_U8       SendLength;
    SK_U8       ReceiveLength;
    SK_U8       ExpectResponse;
    SK_U8       Cycles;
    SK_U64      Time;
    SK_U8       ReceiveBuf  [ASF_HCI_REC_BUF_SIZE];
    SK_U8       TransmitBuf [ASF_HCI_TRA_BUF_SIZE];
    SK_TIMER    AsfTimerHci;
} STR_HCI;

/*
 * ASF specific adapter context structure
 */
typedef struct s_AsfData
{
    SK_U8       CurrentMacAddr[6];
    SK_U8       IpAddress[4];
    SK_TIMER    AsfTimer;
    SK_TIMER    AsfTimerWrSpi;
    SK_U8       StateHci;
    SK_U8       StateWrSpi;
    SK_U8       DriverVersion   [5];
    SK_U8       FlashFwVersion  [5];
    SK_U8       FlashFwRev;
    SK_U8       FileFwVersion   [5];
    SK_U8       FileFwRev;
//VSz
    SK_U8       FlashBuffer     [ASF_FLASH_SIZE];
    SK_U8       VpdConfigBuf    [ASF_VPD_CONFIG_SIZE];
    STR_ASF_MIB Mib;
    STR_HCI     Hci;
    SK_U8       GlHciState;
    SK_U8       LastGlHciState;
    SK_U8       InitState;
    SK_U8       VpdInitOk;
    SK_U32      FwError;
    SK_U8       CpuAlive;
    SK_U16      FwWdIntervall;
    SK_U16      FwRamSize;
    SK_U8       ActivePort;
    SK_U8       PrefPort;
    SK_U8       FwVersionString [80];
    SK_U8       NewGuid;
    SK_U8       OpMode;                 // ASF or IPMI operation mode - see SkAsfInit
    SK_U8       ChipMode;               // relevant for ASF or IPMI operation mode
    SK_U8       DualMode;
}SK_ASF_DATA;

#define MAX_EVENT_DATA      8

struct _STR_EVENT_DATA  {
    struct _STR_EVENT_DATA  *next;
    SK_U8                   SensorType;               // SNMP (Specific Trap)
    SK_U8                   Type;                     // SNMP (Specific Trap)
    SK_U8                   Offset;                   // SNMP (Specific Trap)
    SK_U8                   SourceType;               // PET
    SK_U8                   Severity;                 // PET
    SK_U8                   SensorDevice;             // PET
    SK_U8                   SensorNumber;             // PET
    SK_U8                   Entity;                   // PET
    SK_U8                   EntityInstance;           // PET
    SK_U8                   DataLen;
    SK_U8                   Data [MAX_EVENT_DATA];    // PET
};
typedef struct _STR_EVENT_DATA STR_EVENT_DATA;


/* Functions provided by SkGeAsf */

/* ANSI/C++ compliant function prototypes */

/*
 * Public Function prototypes
 */
extern int SkAsfDeInit(SK_AC *pAC, SK_IOC IoC );
extern int SkAsfInit (SK_AC *pAC , SK_IOC IoC , int level);
extern int SkAsfDeInitStandBy( SK_AC *pAC, SK_IOC IoC );
extern int SkAsfInitStandBy( SK_AC *pAC, SK_IOC IoC, int Level );
extern int SkAsfGet (SK_AC *pAC , SK_IOC IoC , SK_U8 *pBuf, unsigned int *pLen);
extern int SkAsfPreSet (SK_AC *pAC , SK_IOC IoC , SK_U8 *pBuf, unsigned int *pLen);
extern int SkAsfSet (SK_AC *pAC , SK_IOC IoC , SK_U8 *pBuf, unsigned int *pLen);
extern int SkAsfEvent (SK_AC *pAC , SK_IOC IoC , SK_U32 Event , SK_EVPARA Param);
extern int SkAsfSetOid(SK_AC *pAC, SK_IOC IoC, SK_U32 Id, SK_U32 Inst, SK_U8 *pBuf, unsigned int *pLen);
extern int SkAsfPreSetOid(SK_AC *pAC, SK_IOC IoC, SK_U32 Id, SK_U32 Inst, SK_U8 *pBuf, unsigned int *pLen);
extern int SkAsfGetOid(SK_AC *pAC, SK_IOC IoC, SK_U32 Id, SK_U32 Inst, SK_U8 *pBuf, unsigned int *pLen);
extern int SkAsfRestorePattern(SK_AC *pAC , SK_IOC IoC);


SK_I8 SkAsfReadSpiConfigData( SK_AC *pAC );
SK_I8 SkAsfWriteSpiConfigData( SK_AC *pAC );
SK_I8 SkAsfUpdateSpiConfigData(SK_AC *pAC, SK_U8 *data , SK_U32 off , SK_U32 len, SK_U32 ClrCnt );
SK_I8 SkAsfUpdateConfDat( SK_AC *pAC, SK_U8 Pig, SK_U16 RegOffs, SK_U8 ByteEnable, SK_U32 Val, SK_U8 ForceNewEntry );
SK_I8 SkAsfReadConfDat( SK_AC *pAC, SK_U8 Pig, SK_U16 RegOffs, SK_U8 ByteEnable, SK_U32 *Val );
SK_I8 SkAsfWriteDeferredFlash( SK_AC *pAC, SK_IOC IoC );
SK_I8 SkAsfStartWriteDeferredFlash( SK_AC *pAC, SK_IOC IoC );
void SkAsfTimer( SK_AC *pAC, SK_IOC IoC );
void SkAsfShowMib( SK_AC *pAC );
void AsfResetCpu( SK_AC *pAC, SK_IOC IoC );
void AsfRunCpu( SK_AC *pAC, SK_IOC IoC );
SK_U8 AsfCheckAliveCpu( SK_AC *pAC, SK_IOC IoC );
SK_I8 SkAsfSeprom2Mib( SK_AC *pAC, SK_IOC IoC );
SK_I8 SkAsfMib2Seprom( SK_AC *pAC, SK_IOC IoC );
SK_U8 AsfSmartResetCpu( SK_AC *pAC, SK_IOC IoC, SK_U8 Cold );
SK_U8 AsfSmartResetStateCpu( SK_AC *pAC, SK_IOC IoC );
SK_U8 AsfCpuState( SK_AC *pAC, SK_IOC Ioc );

SK_U8 AsfHciGetData( SK_AC    *pAC, SK_U8 **pHciRecBuf );
SK_U8 AsfHciGetState( SK_AC   *pAC );
SK_U8 AsfHciSendCommand( SK_AC *pAC, SK_IOC IoC, SK_U8 Command, SK_U8 Par1, SK_U8 Par2, SK_U8 ExpectResponse, SK_U8 Wait, SK_U8 Retry );
SK_U8 AsfHciSendData( SK_AC *pAC, SK_IOC IoC,  SK_U8 *Buffer, SK_U8 ExpectResponse, SK_U8 Wait, SK_U8 Retry );
SK_U8 AsfHciSendMessage( SK_AC *pAC, SK_IOC IoC, SK_U8 *message, SK_U8 length, SK_U8 ExpectResponse, SK_U8 Wait );
void AsfLockSpi( SK_AC *pAC, SK_IOC IoC );
void AsfUnlockSpi( SK_AC *pAC, SK_IOC IoC );
void SkAsfHci( SK_AC *pAC, SK_IOC IoC, SK_U8 ToEna );
void AsfWatchCpu( SK_AC *pAC, SK_IOC IoC, SK_U32 par );
void AsfEnable(SK_AC *pAC, SK_IOC IoC );
void AsfDisable(SK_AC *pAC, SK_IOC IoC );
void AsfSetOsPresentBit( SK_AC *pAC, SK_IOC IoC );
void AsfResetOsPresentBit( SK_AC *pAC, SK_IOC IoC );
void AsfEnableFlushFifo( SK_AC *pAC, SK_IOC IoC  );
void AsfDisableFlushFifo( SK_AC *pAC, SK_IOC IoC);


void AsfSetUpPattern(SK_AC *pAC, SK_IOC IoC, SK_U8 port  );
SK_I8 AsfWritePatternRam( SK_AC *pAC,
                                 SK_IOC IoC,
                                 SK_U8 Port,
                                 SK_U8 PatternId1,
                                 SK_U8 PatternId2,
                                 SK_U8 Length1,
                                 SK_U8 Length2,
                                 SK_U8 *pMask1,
                                 SK_U8 *pPattern1,
                                 SK_U8 *pMask2,
                                 SK_U8 *pPattern2 );
SK_I8 AsfWritePatternRamEx( SK_AC *pAC,
			    SK_IOC IoC,
			    SK_U8 Port,
			    SK_U8 PatternId,
			    SK_U8 Length1, 
			    SK_U8 *pMask1, 
			    SK_U8 *pPattern1 );

SK_I8 YlciEnablePattern (SK_AC *pAC, SK_IOC IoC, SK_U8 port, SK_U8 pattno );
SK_I8 YlciDisablePattern (SK_AC *pAC, SK_IOC IoC, SK_U8 port, SK_U8 pattno );

//  ACPI and "ASF!" stuff
SK_I8 SkAsfAcpi( SK_AC *pAC, SK_IOC IoC, SK_U8 *pImage );
//SK_I8 SkAsfAcpiRsdt( SK_AC *pAC, SK_IOC IoC, SK_U8 *pImage, HANDLE SectionHandle, SK_U32 PhysAddr );
//SK_I8 SkAsfAcpiXsdt( SK_AC *pAC, SK_IOC IoC, SK_U8 *pImage, HANDLE SectionHandle, SK_U64 PhysAddr );
//SK_I8 SkAsfAcpiAsf( SK_AC *pAC, SK_IOC IoC, SK_U8 *pImage, HANDLE SectionHandle, SK_U32 PhysAddr );
SK_I8 SkAsfPatchAsfTable( SK_AC *pAC, SK_IOC IoC, SK_U8 *pImage, SK_U8 *pAsfTable, SK_U32 TableLength );
SK_I8 SkAsfPatchGuid( SK_AC *pAC, SK_IOC IoC, SK_U8 *pImage, SK_U8 *pGuid );
void SkAsfExamineAsfTable( SK_AC *pAC, SK_IOC IoC, SK_U8 *pAsf, SK_U32 TableLength );
SK_I8 SkAsfSendRmcpData(SK_AC *pAC, SK_IOC IoC,SK_U8 *pData, SK_U8 Length );

// ipmi
SK_I8 AsfWriteIpmiPattern(SK_AC *pAC, SK_IOC IoC, SK_U8 port);

/* in file skspilole.c */
void spi_init_pac( SK_AC *pAC );

// for cleaning up smbus register
void AsfSetSMBusRegister(SK_IOC IoC);

#define SKERR_ASF_E001      (SK_ERRBASE_ASF)
#define SKERR_ASF_E001MSG   "SkAsfInit() error: wrong HCI version"
#define SKERR_ASF_E002      (SKERR_ASF_E001+1)
#define SKERR_ASF_E002MSG   "SkAsfInit() error: flash read"
#define SKERR_ASF_E003      (SKERR_ASF_E001+2)
#define SKERR_ASF_E003MSG   "SkAsfInit() error: flash erase"
#define SKERR_ASF_E004      (SKERR_ASF_E001+3)
#define SKERR_ASF_E004MSG   "SkAsfInit() error: flash write"
#define SKERR_ASF_E005      (SKERR_ASF_E001+4)
#define SKERR_ASF_E005MSG   "SkAsfInit() error: map FW image"
#define SKERR_ASF_E006      (SKERR_ASF_E001+5)
#define SKERR_ASF_E006MSG   "SkAsfInit() error: flash reread"
#define SKERR_ASF_E007      (SKERR_ASF_E001+6)
#define SKERR_ASF_E007MSG   "SkAsfInit() error: flash compare"
#define SKERR_ASF_E008      (SKERR_ASF_E001+7)
#define SKERR_ASF_E008MSG   "SkAsfInit() flash successfully updated"
#define SKERR_ASF_E009      (SKERR_ASF_E001+8)
#define SKERR_ASF_E009MSG   "SkAsfInit() updating flash"


#define ASF_YEC_YTB_BASE_WOL_CTRL1          ((SK_U32)0x0f20)                    // YTB WOL CTRL register link 1
#define ASF_YEC_PATTRAM_CLUSTER_BYTES       ((SK_U8)4)    // 4 bytes is a word
#define ASF_YEC_PATTRAM_CLUSTER_WORDS       ((SK_U8)4)    // 4 words in a cluster
#define ASF_YEC_PATTRAM_CLUSTER_SIZE        ((SK_U8)64)   // pattern ram has 64 cluster

#define ASF_YEC_PATTERN_ENA1                (ASF_YEC_YTB_BASE_WOL_CTRL1 + 0x02)     // enable pattern register, width:8
#define ASF_YEC_PATTERN_LENGTH_R1_L         (ASF_YEC_YTB_BASE_WOL_CTRL1 + 0x10)     // pattern length register, pattern 0-3, width: 4x8
#define ASF_YEC_PATTERN_LENGTH_R1_H         (ASF_YEC_YTB_BASE_WOL_CTRL1 + 0x14)     // pattern length register, pattern 4-6, width: 3x8
#define ASF_YEC_PATTERN_MATCHENA1           (ASF_YEC_YTB_BASE_WOL_CTRL1 + 0x0b)     // ASF/PME match enable register, width: 8
#define ASF_YEC_PATTERN_CTRL1               (ASF_YEC_YTB_BASE_WOL_CTRL1 + 0x00)     // match result, match control, wol ctrl and status

#define ASF_YEC_YTB_BASE_MACRXFIFO1         ((SK_U32)0x0c40)                        // base of receive MAC fifo registers, port 1
#define ASF_YEC_MAC_FIFO_CTRL1              (ASF_YEC_YTB_BASE_MACRXFIFO1 + 0x08)    // control/test Rx MAC, link1, 32 bit
#define ASF_YEC_MAC_FIFO_FLUSHMASK1         (ASF_YEC_YTB_BASE_MACRXFIFO1 + 0x0c)    // flush mask register Rx MAC, link1, 32 bit
#define ASF_YEC_MAC_FIFO_FLUSHTHRES1        (ASF_YEC_YTB_BASE_MACRXFIFO1 + 0x10)    // Rx MAC FIFO Flush Threshold, link1, 32 bit

#define ASF_YLCI_MACRXFIFOTHRES             8                                       // mac rx threshold in qwords


#define ASF_PATTERN_ID_RSP					0
#define ASF_PATTERN_ID_ARP					1
#define ASF_PATTERN_ID_RMCP					2


//  Yukon Extreme
#define ASF_YEX_YTB_BASE_WOL_CTRL1          ((SK_U32)0x0f20)                    // YTB WOL CTRL register link 1
#define ASF_YEX_PATTRAM_CLUSTER_BYTES       ((SK_U8)4)    // 4 bytes is a word
#define ASF_YEX_PATTRAM_CLUSTER_WORDS       ((SK_U8)4)    // 4 words in a cluster
#define ASF_YEX_PATTRAM_CLUSTER_SIZE        ((SK_U8)64)   // pattern ram has 64 cluster

#define ASF_YEX_PATTERN_ENA1                (ASF_YEX_YTB_BASE_WOL_CTRL1 + 0x02)     // enable pattern register, width:8
#define ASF_YEX_PATTERN_LENGTH_R1_L         (ASF_YEX_YTB_BASE_WOL_CTRL1 + 0x10)     // pattern length register, pattern 0-3, width: 4x8
#define ASF_YEX_PATTERN_LENGTH_R1_H         (ASF_YEX_YTB_BASE_WOL_CTRL1 + 0x14)     // pattern length register, pattern 4-6, width: 3x8
#define ASF_YEX_PATTERN_LENGTH_R1_EH        (ASF_YEX_YTB_BASE_WOL_CTRL1 + 0x18)     // pattern length register, pattern 7-8, width: 3x8
#define ASF_YEX_PATTERN_MATCHENA1           (ASF_YEX_YTB_BASE_WOL_CTRL1 + 0x0c)     // ASF/PME match enable register, width: 8
#define ASF_YEX_PATTERN_CTRL1               (ASF_YEX_YTB_BASE_WOL_CTRL1 + 0x00)     // match result, match control, wol ctrl and status

#define ASF_DASH_PATTERN_NUM_ICMP	3
#define ASF_DASH_PATTERN_NUM_ARP	4

#ifdef USE_ASF_DASH_FW
#define ASF_DASH_PATTERN_NUM_SNMP       5
#else
#define ASF_DASH_PATTERN_NUM_RMCP	5
#define ASF_DASH_PATTERN_NUM_RSP	6
#define ASF_DASH_PATTERN_NUM_TCP1	7
#define ASF_DASH_PATTERN_NUM_TCP2	8
#endif

#endif  /* _INC_SKGEASF_H_ */
