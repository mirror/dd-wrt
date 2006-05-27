/*
 *  Copyright (c) 2000-2002 Atheros Communications, Inc., All Rights Reserved
 *
 */

#ident "$Id: //depot/sw/releases/linuxsrc/src/802_11/madwifi/ratectrl/ar5211Phy.c#3 $"

#ifdef __FreeBSD__
#include <dev/ath/ath_rate/atheros/ratectrl.h>
#else
#include "ratectrl.h"
#endif


static RATE_TABLE ar5211_11aRateTable = {
    8,  /* number of rates */
    { -1 },
    {/*                                                             short            ctrl  RssiAck  RssiAck short Preamble long Preamble*/
     /*              valid                 Kbps  uKbps  rateCode Preamble  dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration*/
     /*   6 Mb */ {  TRUE, WLAN_PHY_OFDM,  6000,  5400,   0x0b,    0x00, (0x80|12),   0,       2,       1,      60,        60},
     /*   9 Mb */ {  TRUE, WLAN_PHY_OFDM,  9000,  7800,   0x0f,    0x00,        18,   0,       3,       1,      60,        60},
     /*  12 Mb */ {  TRUE, WLAN_PHY_OFDM, 12000, 10000,   0x0a,    0x00, (0x80|24),   2,       4,       2,      48,        48},
     /*  18 Mb */ {  TRUE, WLAN_PHY_OFDM, 18000, 13900,   0x0e,    0x00,        36,   2,       6,       2,      48,        48},
     /*  24 Mb */ {  TRUE, WLAN_PHY_OFDM, 24000, 17300,   0x09,    0x00, (0x80|48),   4,      10,       3,      44,        44},
     /*  36 Mb */ {  TRUE, WLAN_PHY_OFDM, 36000, 23000,   0x0d,    0x00,        72,   4,      14,       3,      44,        44},
     /*  48 Mb */ {  TRUE, WLAN_PHY_OFDM, 48000, 27400,   0x08,    0x00,        96,   4,      19,       3,      44,        44},
     /*  54 Mb */ {  TRUE, WLAN_PHY_OFDM, 54000, 29300,   0x0c,    0x00,       108,   4,      23,       3,      44,        44},
    },
    100, /* probe interval */
    50,  /* rssi reduce interval */
    0,   /* not used for Oahu */
    0,   /* not used for Oahu */
    0,   /* not used for Oahu */
    7,   /* initial rateMax (index) */
    0,   /* # of turboRates */
};


static RATE_TABLE ar5211_TurboRateTable = {
    8,  /* number of rates */
    { -1 },
    {/*                                                              short            ctrl  RssiAck  RssiAck short Preamble long Preamble*/
     /*              valid                  Kbps  uKbps  rateCode Preamble  dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration*/
     /*   6 Mb */ {  TRUE, WLAN_PHY_TURBO,  6000,  5400,   0x0b,    0x00, (0x80|12),   0,       2,       1,      34,        34},
     /*   9 Mb */ {  TRUE, WLAN_PHY_TURBO,  9000,  7800,   0x0f,    0x00,        18,   0,       4,       1,      34,        34},
     /*  12 Mb */ {  TRUE, WLAN_PHY_TURBO, 12000, 10000,   0x0a,    0x00, (0x80|24),   2,       7,       2,      30,        30},
     /*  18 Mb */ {  TRUE, WLAN_PHY_TURBO, 18000, 13900,   0x0e,    0x00,        36,   2,       9,       2,      30,        30},
     /*  24 Mb */ {  TRUE, WLAN_PHY_TURBO, 24000, 17300,   0x09,    0x00, (0x80|48),   4,      14,       3,      26,        26},
     /*  36 Mb */ {  TRUE, WLAN_PHY_TURBO, 36000, 23000,   0x0d,    0x00,        72,   4,      17,       3,      26,        26},
     /*  48 Mb */ {  TRUE, WLAN_PHY_TURBO, 48000, 27400,   0x08,    0x00,        96,   4,      22,       3,      26,        26},
     /*  54 Mb */ {  TRUE, WLAN_PHY_TURBO, 54000, 29300,   0x0c,    0x00,       108,   4,      26,       3,      26,        26},
    },
    100, /* probe interval */
    50,  /* rssi reduce interval */
    0,   /* not used for Oahu */
    0,   /* not used for Oahu */
    0,   /* not used for Oahu */
    7,   /* initial rateMax (index) */
    8,   /* # of turboRates */
};


static RATE_TABLE ar5211_11bRateTable = {
    4,  /* number of rates */
    { -1 },
    {/*                                                             short            ctrl  RssiAck  RssiAck long Preamble short Preamble*/
     /*              valid                 Kbps  uKbps  rateCode Preamble  dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration*/
     /*   1 Mb */ {  TRUE,  WLAN_PHY_CCK,  1000,  900,  0x0b,    0x00, (0x80| 2),   0,       0,       1,        314,       314},
     /*   2 Mb */ {  TRUE,  WLAN_PHY_CCK,  2000, 1800,  0x0a,    0x04, (0x80| 4),   1,       1,       1,        258,       162},
     /* 5.5 Mb */ {  TRUE,  WLAN_PHY_CCK,  5500, 4300,  0x09,    0x04, (0x80|11),   1,       2,       2,        258,       162},
     /*  11 Mb */ {  TRUE,  WLAN_PHY_CCK, 11000, 7100,  0x08,    0x04, (0x80|22),   1,       4,     100,        258,       162},
    },
    200, /* probe interval */
    100, /* rssi reduce interval */
    0,   /* not used for Oahu */
    0,   /* not used for Oahu */
    0,   /* not used for Oahu */
    3,   /* initial rateMax (index) */
    0,   /* # of turboRates */
};


void
ar5211SetupRateTables(void)
{
    atheros_setuptable(&ar5211_11aRateTable);
    atheros_setuptable(&ar5211_TurboRateTable);
    atheros_setuptable(&ar5211_11bRateTable);
    atheros_setuptable(&ar5211_11aRateTable);
}

void
ar5211AttachRateTables(struct atheros_softc *sc)
{
    /*
     * Attach device specific rate tables; for ar5211, g is
     * really pure-g and thus corresponds to the 11a table
     */
    sc->hwRateTable[WIRELESS_MODE_11a]   = &ar5211_11aRateTable;
    sc->hwRateTable[WIRELESS_MODE_TURBO] = &ar5211_TurboRateTable;
    sc->hwRateTable[WIRELESS_MODE_11b]   = &ar5211_11bRateTable;
    sc->hwRateTable[WIRELESS_MODE_11g]   = &ar5211_11aRateTable;
    sc->hwRateTable[WIRELESS_MODE_XR]    = NULL;
}
