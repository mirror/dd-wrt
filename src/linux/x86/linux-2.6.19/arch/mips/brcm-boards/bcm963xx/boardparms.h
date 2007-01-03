/*
<:copyright-gpl 

 Copyright 2003 Broadcom Corp. All Rights Reserved. 
 
 This program is free software; you can distribute it and/or modify it 
 under the terms of the GNU General Public License (Version 2) as 
 published by the Free Software Foundation. 
 
 This program is distributed in the hope it will be useful, but WITHOUT 
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 for more details. 
 
 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA. 

:>
*/
/**************************************************************************
 * File Name  : boardparms.h
 *
 * Description: This file contains definitions and function prototypes for
 *              the BCM63xx board parameter access functions.
 * 
 * Updates    : 07/14/2003  Created.
 ***************************************************************************/

#if !defined(_BOARDPARMS_H)
#define _BOARDPARMS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Return codes. */
#define BP_SUCCESS                              0
#define BP_BOARD_ID_NOT_FOUND                   1
#define BP_VALUE_NOT_DEFINED                    2
#define BP_BOARD_ID_NOT_SET                     3

/* Values for EthernetMacInfo PhyType. */
#define BP_ENET_NO_PHY                          0
#define BP_ENET_INTERNAL_PHY                    1
#define BP_ENET_EXTERNAL_PHY                    2
#define BP_ENET_EXTERNAL_SWITCH                 3

/* Values for EthernetMacInfo Configuration type. */
#define BP_ENET_CONFIG_MDIO                     0       /* Internal PHY, External PHY, Switch+(no GPIO, no SPI, no MDIO Pseudo phy */
#define BP_ENET_CONFIG_MDIO_PSEUDO_PHY          1
#define BP_ENET_CONFIG_SPI_SSB_0                2
#define BP_ENET_CONFIG_SPI_SSB_1                3
#define BP_ENET_CONFIG_SPI_SSB_2                4
#define BP_ENET_CONFIG_SPI_SSB_3                5

/* Values for EthernetMacInfo Reverse MII. */
#define BP_ENET_NO_REVERSE_MII                  0
#define BP_ENET_REVERSE_MII                     1

/* Values for VoIPDSPInfo DSPType. */
#define BP_VOIP_NO_DSP                          0
#define BP_VOIP_DSP                             1
#define BP_VOIP_MIPS                            2

/* Values for GPIO pin assignments (AH = Active High, AL = Active Low). */
#define BP_GPIO_NUM_MASK                        0x00FF
#define BP_ACTIVE_LOW                           0x8000
#define BP_GPIO_SERIAL                          0x4000

#define BP_GPIO_0_AH                            (0)
#define BP_GPIO_0_AL                            (0  | BP_ACTIVE_LOW)
#define BP_GPIO_1_AH                            (1)
#define BP_GPIO_1_AL                            (1  | BP_ACTIVE_LOW)
#define BP_GPIO_2_AH                            (2)
#define BP_GPIO_2_AL                            (2  | BP_ACTIVE_LOW)
#define BP_GPIO_3_AH                            (3)
#define BP_GPIO_3_AL                            (3  | BP_ACTIVE_LOW)
#define BP_GPIO_4_AH                            (4)
#define BP_GPIO_4_AL                            (4  | BP_ACTIVE_LOW)
#define BP_GPIO_5_AH                            (5)
#define BP_GPIO_5_AL                            (5  | BP_ACTIVE_LOW)
#define BP_GPIO_6_AH                            (6)
#define BP_GPIO_6_AL                            (6  | BP_ACTIVE_LOW)
#define BP_GPIO_7_AH                            (7)
#define BP_GPIO_7_AL                            (7  | BP_ACTIVE_LOW)
#define BP_GPIO_8_AH                            (8)
#define BP_GPIO_8_AL                            (8  | BP_ACTIVE_LOW)
#define BP_GPIO_9_AH                            (9)
#define BP_GPIO_9_AL                            (9  | BP_ACTIVE_LOW)
#define BP_GPIO_10_AH                           (10)
#define BP_GPIO_10_AL                           (10 | BP_ACTIVE_LOW)
#define BP_GPIO_11_AH                           (11)
#define BP_GPIO_11_AL                           (11 | BP_ACTIVE_LOW)
#define BP_GPIO_12_AH                           (12)
#define BP_GPIO_12_AL                           (12 | BP_ACTIVE_LOW)
#define BP_GPIO_13_AH                           (13)
#define BP_GPIO_13_AL                           (13 | BP_ACTIVE_LOW)
#define BP_GPIO_14_AH                           (14)
#define BP_GPIO_14_AL                           (14 | BP_ACTIVE_LOW)
#define BP_GPIO_15_AH                           (15)
#define BP_GPIO_15_AL                           (15 | BP_ACTIVE_LOW)
#define BP_GPIO_16_AH                           (16)
#define BP_GPIO_16_AL                           (16 | BP_ACTIVE_LOW)
#define BP_GPIO_17_AH                           (17)
#define BP_GPIO_17_AL                           (17 | BP_ACTIVE_LOW)
#define BP_GPIO_18_AH                           (18)
#define BP_GPIO_18_AL                           (18 | BP_ACTIVE_LOW)
#define BP_GPIO_19_AH                           (19)
#define BP_GPIO_19_AL                           (19 | BP_ACTIVE_LOW)
#define BP_GPIO_20_AH                           (20)
#define BP_GPIO_20_AL                           (20 | BP_ACTIVE_LOW)
#define BP_GPIO_21_AH                           (21)
#define BP_GPIO_21_AL                           (21 | BP_ACTIVE_LOW)
#define BP_GPIO_22_AH                           (22)
#define BP_GPIO_22_AL                           (22 | BP_ACTIVE_LOW)
#define BP_GPIO_23_AH                           (23)
#define BP_GPIO_23_AL                           (23 | BP_ACTIVE_LOW)
#define BP_GPIO_24_AH                           (24)
#define BP_GPIO_24_AL                           (24 | BP_ACTIVE_LOW)
#define BP_GPIO_25_AH                           (25)
#define BP_GPIO_25_AL                           (25 | BP_ACTIVE_LOW)
#define BP_GPIO_26_AH                           (26)
#define BP_GPIO_26_AL                           (26 | BP_ACTIVE_LOW)
#define BP_GPIO_27_AH                           (27)
#define BP_GPIO_27_AL                           (27 | BP_ACTIVE_LOW)
#define BP_GPIO_28_AH                           (28)
#define BP_GPIO_28_AL                           (28 | BP_ACTIVE_LOW)
#define BP_GPIO_29_AH                           (29)
#define BP_GPIO_29_AL                           (29 | BP_ACTIVE_LOW)
#define BP_GPIO_30_AH                           (30)
#define BP_GPIO_30_AL                           (30 | BP_ACTIVE_LOW)
#define BP_GPIO_31_AH                           (31)
#define BP_GPIO_31_AL                           (31 | BP_ACTIVE_LOW)
#define BP_GPIO_32_AH                           (32)
#define BP_GPIO_32_AL                           (32 | BP_ACTIVE_LOW)
#define BP_GPIO_33_AH                           (33)
#define BP_GPIO_33_AL                           (33 | BP_ACTIVE_LOW)
#define BP_GPIO_34_AH                           (34)
#define BP_GPIO_34_AL                           (34 | BP_ACTIVE_LOW)
#define BP_GPIO_35_AH                           (35)
#define BP_GPIO_35_AL                           (35 | BP_ACTIVE_LOW)
#define BP_GPIO_36_AH                           (36)
#define BP_GPIO_36_AL                           (36 | BP_ACTIVE_LOW)
#define BP_GPIO_37_AH                           (37)
#define BP_GPIO_37_AL                           (37 | BP_ACTIVE_LOW)

#define BP_SERIAL_GPIO_0_AH                     (0  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_0_AL                     (0  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_1_AH                     (1  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_1_AL                     (1  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_2_AH                     (2  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_2_AL                     (2  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_3_AH                     (3  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_3_AL                     (3  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_4_AH                     (4  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_4_AL                     (4  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_5_AH                     (5  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_5_AL                     (5  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_6_AH                     (6  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_6_AL                     (6  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_7_AH                     (7  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_7_AL                     (7  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)

/* Values for external interrupt assignments. */
#define BP_EXT_INTR_0                           0
#define BP_EXT_INTR_1                           1
#define BP_EXT_INTR_2                           2
#define BP_EXT_INTR_3                           3
#define BP_EXT_INTR_4                           4
#define BP_EXT_INTR_5                           5

/* Values for chip select assignments. */
#define BP_CS_0                                 0
#define BP_CS_1                                 1
#define BP_CS_2                                 2
#define BP_CS_3                                 3

#define BP_PCI                                  (1<<0)
#define BP_CB                                   (1<<1)
#define BP_SPI_EXT_CS                           (1<<2)
#define BP_MII2                                 (1<<3)
#define BP_UTOPIA                               (1<<4)
#define BP_UART1                                (1<<5)

/* Value for GPIO and external interrupt fields that are not used. */
#define BP_NOT_DEFINED                          0xffff
#define BP_UNEQUIPPED                           0xfff1

#define BP_HW_DEFINED                           0xfff0
#define BP_HW_DEFINED_AL                        0xfff0
#define BP_HW_DEFINED_AH                        0xfff2

/* Maximum size of the board id string. */
#define BP_BOARD_ID_LEN                         16

/* Maximum number of Ethernet MACs. */
#define BP_MAX_ENET_MACS                        2

/* Maximum number of VoIP DSPs. */
#define BP_MAX_VOIP_DSP                         2

/* Wireless Antenna Settings. */
#define BP_WLAN_ANT_MAIN                        0
#define BP_WLAN_ANT_AUX                         1
#define BP_WLAN_ANT_BOTH                        3

/* Wireless FLAGS */
#define BP_WLAN_MAC_ADDR_OVERRIDE               0x0001   /* use kerSysGetMacAddress for mac address */
#define BP_WLAN_EXCLUDE_ONBOARD                 0x0002   /* exclude onboard wireless  */

#define BP_WLAN_NVRAM_NAME_LEN			16
#define BP_WLAN_MAX_PATCH_ENTRY			8

#if !defined(__ASSEMBLER__)

/* Information about an Ethernet MAC.  If ucPhyType is BP_ENET_NO_PHY,
 * then the other fields are not valid.
 */
typedef struct EthernetMacInfo
{
    unsigned char ucPhyType;                    /* BP_ENET_xxx             */
    unsigned char ucPhyAddress;                 /* 0 to 31                 */
    unsigned short usGpioPhyReset;              /* GPIO pin or not defined */
    unsigned short usGpioPhyLinkSpeed;          /* GPIO pin or not defined */
    unsigned short numSwitchPorts;              /* Number of PHY ports */
    unsigned short usConfigType;                /* Configuration type */
    unsigned short usReverseMii;                /* Reverse MII */
} ETHERNET_MAC_INFO, *PETHERNET_MAC_INFO;

typedef struct WlanSromEntry {
    char name[BP_WLAN_NVRAM_NAME_LEN];
    unsigned short wordOffset;
    unsigned short value;
} WLAN_SROM_ENTRY;

typedef struct WlanSromPatchInfo {
    char szboardId[BP_BOARD_ID_LEN];
    unsigned short usWirelessChipId;
    unsigned short usNeededSize;    
    WLAN_SROM_ENTRY entries[BP_WLAN_MAX_PATCH_ENTRY];
} WLAN_SROM_PATCH_INFO, *PWLAN_SROM_PATCH_INFO;


/* Information about VoIP DSPs.  If ucDspType is BP_VOIP_NO_DSP,
 * then the other fields are not valid.
 */
typedef struct VoIPDspInfo
{
    unsigned char  ucDspType;
    unsigned char  ucDspAddress;
    unsigned short usExtIntrVoip;
    unsigned short usGpioVoipReset;
    unsigned short usGpioVoipIntr;
    unsigned short usGpioLedVoip;
    unsigned short usCsVoip;
    unsigned short usGpioVoip1Led;
    unsigned short usGpioSlic0Relay;
    unsigned short usGpioSlic0Reset;
    unsigned short usGpioVoip2Led;
    unsigned short usGpioSlic1Relay;
    unsigned short usGpioSlic1Reset;
    unsigned short usGpioPotsLed;
    unsigned short usGpioPotsReset;

} VOIP_DSP_INFO;


/*   VCOPE board definitions 
 */
typedef enum
{
    BCM6505_RESET_GPIO = 1,
    VCOPE_RELAY_GPIO,
    HPI_CS,
    VCOPE_BOARD_REV, 
} VCOPE_PIN_TYPES;


/*   SLIC types 
 */
typedef enum
{
    Silicon_Labs_3230,
    Legerity_88221,
    Slic_Not_Defined 
} SLIC_TYPE;


/*   DAA types 
 */
typedef enum
{
    Silicon_Labs_3050,
    Legerity_88010, 
    DAA_Not_Defined 
} DAA_TYPE;

/*  SLIC GPIO TYPES
*/
typedef enum
{
    SLIC_GPIO_RESET,
    SLIC_GPIO_RELAY, 
    SLIC_GPIO_GENERIC 
} SLIC_GPIO_TYPE;

int BpSetBoardId( char *pszBoardId );
int BpGetBoardIds( char *pszBoardIds, int nBoardIdsSize );

int BpGetGPIOverlays( unsigned short *pusValue );

int BpGetRj11InnerOuterPairGpios( unsigned short *pusInner, unsigned short *pusOuter );
int BpGetRtsCtsUartGpios( unsigned short *pusRts, unsigned short *pusCts );
int BpGetVcopeGpio(int pio_idx);

int BpGetAdslLedGpio( unsigned short *pusValue );
int BpGetAdslFailLedGpio( unsigned short *pusValue );
int BpGetSecAdslLedGpio( unsigned short *pusValue );
int BpGetSecAdslFailLedGpio( unsigned short *pusValue );
int BpGetWirelessSesLedGpio( unsigned short *pusValue );
int BpGetHpnaLedGpio( unsigned short *pusValue );
int BpGetWanDataLedGpio( unsigned short *pusValue );
int BpGetPppLedGpio( unsigned short *pusValue );
int BpGetPppFailLedGpio( unsigned short *pusValue );
int BpGetBootloaderPowerOnLedGpio( unsigned short *pusValue );
int BpGetBootloaderStopLedGpio( unsigned short *pusValue );
int BpGetBondingSecondaryResetGpio( unsigned short *pusValue ) ;

int BpGetAdslDyingGaspExtIntr( unsigned long *pulValue );
int BpGetResetToDefaultExtIntr( unsigned short *pusValue );
int BpGetWirelessSesExtIntr( unsigned short *pusValue );
int BpGetHpnaExtIntr( unsigned long *pulValue );

int BpGetHpnaChipSelect( unsigned long *pulValue );
int BpGetHpiChipSelect( unsigned long *pulValue );

int BpGetWirelessAntInUse( unsigned short *pusValue );
int BpGetWirelessFlags( unsigned short *pusValue );
int BpUpdateWirelessSromMap(unsigned short chipID, unsigned short* pBase, int sizeInWords);

int BpGetEthernetMacInfo( PETHERNET_MAC_INFO pEnetInfos, int nNumEnetInfos );

int BpGetVoipExtIntr( unsigned char dspNum, unsigned long *pulValue );
int BpGetVoipLedGpio( unsigned short *pusValue );
int BpGetVoip1LedGpio( unsigned short *pusValue );
int BpGetVoip2LedGpio( unsigned short *pusValue );
int BpGetSlicGpio( unsigned short num, SLIC_GPIO_TYPE gpioType, unsigned short *pusValue );
int BpGetVoipChipSelect( unsigned char dspNum, unsigned long *pulValue );
int BpGetVoipResetGpio( unsigned char dspNum, unsigned short *pusValue );
int BpGetVoipIntrGpio( unsigned char dspNum, unsigned short *pusValue );
int BpGetPotsLedGpio( unsigned short *pusValue );
int BpGetPotsResetGpio( unsigned short *pusValue );
int BpGetSlicType( unsigned short *slicType );
int BpGetDAAType( unsigned short *daaType );


#endif /* __ASSEMBLER__ */

#ifdef __cplusplus
}
#endif

#endif /* _BOARDPARMS_H */

