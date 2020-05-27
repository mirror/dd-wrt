/******************************************************************************
 *
 * Name:    fwapp.h
 * Project: fwapp
 * Version: $Revision: #6 $
 * Date:    $Date: 2010/11/04 $
 * Purpose: firmware application support
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	LICENSE:
 *	(C)Copyright Marvell.
 *	
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *	
 *	The information in this file is provided "AS IS" without warranty.
 *	/LICENSE
 *
 ******************************************************************************/

#ifndef __SK_FWAPP_H__
#define __SK_FWAPP_H__

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

extern int     FwAppInit0(SK_AC *, SK_IOC);
extern int     FwAppInit1(SK_AC *, SK_IOC);
extern int     FwAppInit2(SK_AC *, SK_IOC);
extern char    *FwAppGetImageName(SK_AC *);
extern int     FwAppPatchImage(SK_AC *, SK_IOC);
extern SK_BOOL FwAppIsVersionOk(SK_AC *);
extern void    FwMainStateMachine(SK_AC *, SK_IOC);
extern void    FwDriverHello(SK_AC *, SK_IOC);
extern void    FwDriverGoodbye(SK_AC *, SK_IOC);
extern void    FwRecoverFirmware(SK_AC *, SK_IOC);
extern void    FwCheckLinkMode(SK_AC *, SK_IOC, int);
extern void    FwCheckLinkState(SK_AC *, SK_IOC, int, SK_U16 *);
extern int     FwHandleRmt(SK_AC *, SK_U32, char *, SK_U32);
extern int     FwWriteRmt(SK_AC *);

#define FW_API_VERSION      "V0.04"
#define FW_API_VERSION_SIZE 5

/*
 * Recovery Message Table (RMT) definitions
 */
#define	RMT_MSG_ID_DEFAULT		0x00000000	/* Do just pass MSG to FW */
#define	RMT_MSG_ID_RESET		0xffffffff	/* Clear RMT and pass MG to FW */

#define FW_RMT_MESSAGE_SIZE		1024
#define FW_RMT_ARRAY_SIZE		32
#define FW_RMT_ARRAY_LAST_INDEX	(FW_RMT_ARRAY_SIZE - 1)

typedef struct s_FW_RMT_MESSAGE FW_RMT_MESSAGE;
struct s_FW_RMT_MESSAGE {
	SK_U32	MessageIndex;
	SK_U32	MessageID;
	SK_U32	MessageLength;
	SK_U8	Message[FW_RMT_MESSAGE_SIZE];
};

typedef struct s_FW_RMT FW_RMT;
struct s_FW_RMT {
	SK_BOOL			RmtFilled;
	SK_BOOL			RecoverMessageTable;
	FW_RMT_MESSAGE	RmtMessage[FW_RMT_ARRAY_SIZE];
};

#define FW_MAX_READTRY	10

/* FW Recovery Message Table (RMT) debugging */
#if 0
#define FW_RMT_DEBUG
#endif
#if 0
#define FW_RMT_VERBOSE_DEBUG
#endif

/* Defines for FW debugging */
#define DEBUGFW								0x0001  /*   Dont't reset Processor (eg. during  check alive) */
#define DEBUGFW_INIT_STOP					0x0002  /*   BRK in the ASF init function */
#define DEBUGFW_FORCE_FW_DNLD				0x0004  /*   Force FW download */
#define DEBUGFW_SMBUS_ANALYZER				0x0008  /*   Load SMBus analyzer firmware */
#define DEBUGFW_ERASE_CONFIG		    	0x0010  /*   Load SMBus analyzer firmware */
#define DEBUGFW_INIT_SKIP_ASF				0x0020
#define DEBUGFW_INIT_STOP_CPU				0x0040  /*   BRK in the ASF init function */
#define DEBUGFW_INIT_ENA_JTAG				0x0080  /*   Activates JTAG interface by writing 0x01 to 0xe67 register */
#define DEBUGFW_DASH_MODE					0x0100  /*   As long as we have no GUI for setting the DASH mode we use this bit */
#define DEBUGFW_SET_TEST_PATTERN			0x0200  /*   enables certain pattern for firmware testing */
#define DEBUGFW_DISABLE_HW_WD           	0x0400  /*   Disables HW watchdog (needed for debugging) */
#define DEBUGFW_DISABLE_API_RESET       	0x0800  /*   Disables API HW Reset function (needed for debugging) */
#define DEBUGFW_ERASE_FLASH_DURING_UNLOAD   0x1000  /*   Removes firmware from flash during driver unload */
#define DEBUGFW_SSL_CERTIFICATE_PATCH       0x2000  /*   Not used anymore !!! */

/* Firmware commands */
#define YASF_HOSTCMD_SYS_WILL_SLEEP         ((SK_U8)20)
#define YASF_HOSTCMD_SYS_SA_UPDATE_COMPLETE ((SK_U8)21)

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
#define YASF_HOSTCMD_CFG_SET_USER_DATA			121
#define YASF_HOSTCMD_CFG_SET_INIT_SEPROM		122
#define YASF_HOSTCMD_CFG_SET_IP_INFO			123
#define YASF_HOSTCMD_CFG_SET_EXT_IP_INFO       	124

#define YASF_HOSTCMD_CFG_STORE_CONFIG           130
#define YASF_HOSTCMD_CFG_ERASE_CONFIG           131

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
#define YASF_HOSTCMD_CFG_GET_FW_STATE 		    171
#define YASF_HOSTCMD_CFG_GET_FW_MODE 		    172
#define YASF_HOSTCMD_CFG_GET_USER_DATA			173
#define YASF_HOSTCMD_CFG_GET_SSL_DATA			174
#define YASF_HOSTCMD_CFG_READ_CONFIG            180
#define YASF_HOSTCMD_CFG_GET_IP_INFO			181
#define YASF_HOSTCMD_CFG_GET_EXT_IP_INFO        182

/* ASF MIB default values */
#define ASF_DEF_RETRANS_COUNT_MIN   0           /*  x1 */
#define ASF_DEF_RETRANS_COUNT_MAX   255         /*  x1 */
#define ASF_DEF_WATCHDOG_ENA        0           /*  */
#define ASF_DEF_WATCHDOG_TIME       1200        /*  *100ms */
#define ASF_DEF_WATCHDOG_TIME_MAX   36000       /*  *100ms */
#define ASF_DEF_WATCHDOG_TIME_MIN   10          /*  *100ms */
#define ASF_DEF_RETRANS_INT_MIN     0           /*  *100ms */
#define ASF_DEF_RETRANS_INT_MAX     2550        /*  *100ms */
#define ASF_DEF_HB_INT_MIN          10          /*  *100ms */
#define ASF_DEF_HB_INT_MAX          2550        /*  *100ms */
#define ASF_DEF_ASF_ENA             0
#define ASF_DEF_RETRANS             2
#define ASF_DEF_RETRANS_INT         10          /*  *100ms */
#define ASF_DEF_HB_ENA              0
#define ASF_DEF_HB_INT              600         /*  *100ms */

/*  we need a buffer for 3 IP addresses and one char for DHCP enabled/disabled. */
/*  4 = one IP address e.g. 255.255.255.255 */
/*  THe data is stored as follows: */
/* 				- bytes 0 - 3 IP address of the network adapter */
/* 				- bytes 4 - 7 IP mask of the network adapter */
/*  				- bytes 8 - 11 IP address of the DHCP server. */
/* 				- byte 12		DHCP enable flag */
#define SK_GEASF_IP_INFO_SIZE				13

/* Defines */
#define ASF_FW_ARP_RESOLVE                  /*   FW is resolving the destination IP addr */
#define ASF_FW_WD                           /*   Driver is watching the FW */
#define ASF_CHECK_HIDDEN_ID                 /*   ASF init checks hidden id */

/* Operating modes of the asf driver */
#define SK_GEASF_MODE_UNKNOWN               0       /*  unknown operation mode (initial) */
#define SK_GEASF_MODE_ASF                   1       /*  asfec.bin binary found -> ASF operation */
#define SK_GEASF_MODE_IPMI                  2       /*  ipmiy2.bin binary found -> IPMI operation */
#define SK_GEASF_MODE_DASH                  3       /*  txdashsu.bin binary found -> DASH operation */
#define SK_GEASF_MODE_DASH_ASF              4       /*  txdashsu.bin binary found -> ASF operation */
#define SK_GEASF_MODE_DASH_ASF_MIXED        5       /*  txdashsu.bin binary found -> ASF/DASH mixed operation */
#define SK_GEASF_MODE_EMBEDDED_SDK          6       /*  txbasesu.bin binary found -> PRINTER SDK FW */

/* Chip modes for asf driver */
#define SK_GEASF_CHIP_UNKNOWN               0       /*  bad chip id / hidden id */
#define SK_GEASF_CHIP_EC                    1       /*  EC: ASF */
#define SK_GEASF_CHIP_Y2                    2       /*  Yukon2 */
#define SK_GEASF_CHIP_EX                    3       /*  Yukon Extreme */
#define SK_GEASF_CHIP_SU                    4       /*  Yukon Supreme */

/* Dual link mode */
#define SK_GEASF_Y2_SINGLEPORT              1       /*  Yukon2 sigle link adapter */
#define SK_GEASF_Y2_DUALPORT                2       /*  Yukon2 dual link adapter */

#define ASF_GUI_TSF                         10      /* Time Scale Factor: 1s(GUI) <-> 10*100ms(FW) */

#define ASF_MAX_STRLEN                      64

/* Lengths used in get oid */
#define ASF_ACPI_MAXBUFFLENGTH              1024     /*  max bytes for holding ACPI table */
#define ASF_SMBIOS_MAXBUFFLENGTH            (1024*3) /*  max bytes for holding SMBIOS table */
#define ASF_SMBUS_MAXBUFFLENGTH             128      /*  max bytes for holding SMBus info */
#define ASF_FWVER_MAXBUFFLENGTH             84       /*  max stringlen for firmware version */

#define SK_ASF_EVT_TIMER_EXPIRED            1       /* Counter overflow */
#define SK_ASF_EVT_TIMER_SPI_EXPIRED        2       /* Counter overflow */
#define SK_ASF_EVT_TIMER_HCI_EXPIRED        3       /* Counter overflow */

/* ACPI module defines */
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
#define ASF_ACPI_STATE_ERROR_NO_UUID          	-14
#define ASF_ACPI_STATE_ERROR_NO_SMBIOS_SM 	    -15
#define ASF_ACPI_STATE_ERROR_NO_SMBIOS_SM_ADDR	-16
#define ASF_ACPI_STATE_ERROR_NO_SMBIOS_SYS_INFO -17
#define ASF_ACPI_STATE_ERROR_SMBIOS_TAB_LEN     -18

#define ASF_FW_VERSION_STRING_SIZE	100
#define RSP_KEYLENGTH           20

/* VPD */
#define ASF_VPD_MAX_CONFIG_SIZE	0x200  /*   512 byte */
#define ASF_VPD_CONFIG_BASE     0x340
#define ASF_VPD_CONFIG_SIZE     0x80
#define ASF_VPD_DATA_BASE       0x3c0
#define ASF_VPD_DATA_SIZE       0x40

#define ASF_FLASH_SIZE_128K     (1024*128)
#define ASF_FLASH_SIZE_256K     (1024*256)
#define ASF_FLASH_SIZE_384K     (1024*384)

#define ASF_MAX_FLASH_SIZE 		ASF_FLASH_SIZE_384K

/*  Flash (SPI) VPD present */
#define ASF_FLASH_SIZE          (1024*128)
/*  Flash (SPI) NO VPD present */
#define ASF_FLASH_NOVPD_SIZE    (1024*116)  /*   Code + ACPI data */

/* Yukon EC etc. */
#define ASF_FLASH_EC_OFFS          0x20000
#define ASF_FLASH_EC_OFFS_VER      0x1fc00
#define ASF_FLASH_EC_OFFS_REV      0x1fc0b
#define ASF_FLASH_EC_OFFS_CS       0x1fffc
#define ASF_FLASH_EC_OFFS_GUID     0x1f000
#define ASF_FLASH_EC_OFFS_ACPI     0x1f010

/*  Yukon Extreme  */
#define ASF_FLASH_EX_OFFS          0x20000
#define ASF_FLASH_EX_OFFS_VER      0x1cc00
#define ASF_FLASH_EX_OFFS_REV      0x1cc0b
#define ASF_FLASH_EX_OFFS_CS       0x1cffc
#define ASF_FLASH_EX_OFFS_GUID     0x1c000
#define ASF_FLASH_EX_OFFS_ACPI     0x1c010

/*  Yukon Supreme  */
#define ASF_FLASH_SU_SIZE    	   (1024*384)
#define ASF_FLASH_SU_OFFS          0x00000
#define ASF_FLASH_SU_OFFS_SMBIOS   0x5eb00
#define ASF_FLASH_SU_OFFS_SSL_CERT 0x5e300
/* #define ASF_FLASH_SU_OFFS_GUID     0x5f300 */
/* #define ASF_FLASH_SU_OFFS_ACPI     0x5f310 */
#define ASF_FLASH_SU_OFFS_GUID     0x5f700
#define ASF_FLASH_SU_OFFS_ACPI     0x5f710
#define ASF_FLASH_SU_OFFS_VER      0x5ff00
#define ASF_FLASH_SU_OFFS_REV      0x5ff0b
#define ASF_FLASH_SU_OFFS_CS       0x5fffc

#define ASF_RESET_HOT           0
#define ASF_RESET_COLD          1

#define ASF_INIT_UNDEFINED      0
#define ASF_INIT_OK             1
#define ASF_INIT_ERROR          2
#define ASF_INIT_ERROR_CHIP_ID  3
#define ASF_INIT_ERROR_OPMODE   4

/* Defines for FW state */
#define ASF_FW_STATE_NEEDS_CONFIG				BIT_0
#define ASF_FW_STATE_DISABLE_LAN_PORT			BIT_1
#define ASF_FW_STATE_UPDATE_SSL_CERTIFICATE		BIT_2

#define ASF_SSL_BUFFER_SIZE						1024
#define	ASF_PFLASH_PART_INFO					0
#define	ASF_PFLASH_PART_MAIN					1

#define ASF_PORT_TRAFFIC_UNCHANGED		0
#define ASF_PORT_TRAFFIC_STARTED		1
#define ASF_PORT_TRAFIC_STOPPED			2

struct _STR_ASF_MIB
{
    SK_U8       NewParam;
    SK_U8       Ena;
    SK_U8       RspEnable;
};
typedef struct _STR_ASF_MIB STR_ASF_MIB;

/*
 * FW application specific adapter context structure
 */
typedef struct s_FwApp {
    SK_TIMER    AsfTimer;
    SK_U8       DriverVersion[FW_API_VERSION_SIZE+1];
    SK_U8       *FlashBuffer;
    char        VpdConfigBuf[ASF_VPD_MAX_CONFIG_SIZE];
    STR_ASF_MIB Mib;
    STR_HCI     Hci;
    SK_U8       GlHciState;
    SK_U8       InitState;
    SK_U8       VpdInitOk;
    SK_U8       CpuAlive;
    SK_U8       ActivePort;
    SK_U8       OpMode;
    SK_U8       ChipMode;
    SK_U8       DualMode;
    SK_U8       StandBy; 
    SK_U8       PxeSPI;
    SK_U8       FwState;
    SK_U8       ToPatternCheck;
    SK_U32      FlashSize;
    SK_U32      FlashOffs;
    SK_U32      SPIFlashSize;
    FW_RMT      *pRMT;
} SK_FWAPP;

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* __SK_FWAPP_H__ */

