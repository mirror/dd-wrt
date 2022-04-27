/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Atheros: //depot/sw/src/include/xr.h#11 $
 * $Id: //depot/sw/branches/sam_hal/ah_xr.h#1 $
 */
#ifndef _ATH_AH_XR_H_
#define _ATH_AH_XR_H_

/*
 * values supplied by HW design - should be
 * considered to be part of the XR spec!
 */
#define XR_SLOT_DELAY                   30      /* 30usec */
#define XR_DATA_DETECT_DELAY            40      /* 40usec */
#define XR_CHIRP_DUR                    24      /* 24usec */

#define INIT_AIFS_XR                    0
#define INIT_CWMIN_XR                   3
#define INIT_CWMAX_XR                   7

#define XR_MAX_FRAME_SIZE               AR_BUF_SIZE

/*
 * Parameters biasing the XR BSS to be a second
 * rate citizen
 */
#define XR_DNLINK_QUEUE_AIFS_SHIFT      7
#define XR_DNLINK_QUEUE_CWMIN_SHIFT     4
#define XR_BEACON_FACTOR                4
#define XR_DEFAULT_RATE                 1000    /* 1 Mb/s */

/*
 * CTS protection frame is needed to protect from other stations in
 * the normal BSS - so it needs to go out at the multicast rate for
 * the normal BSS - both for XR mode and maybe 'g' mode operation
 */
#define XR_CTS_RATE(_pDev)                              \
(                                                       \
    (_pDev)->baseBss.pRateTable->info[                  \
        (_pDev)->baseBss.pRateTable->info[              \
            (_pDev)->baseBss.defaultRateIndex           \
        ].controlRate                                   \
    ].rateCode                                          \
)

/*
 * Should account for sifs + grp poll time + max backoff
 * time at the station + uplink XR data frame + sifs +
 * downlink XR ack
 */
#define XR_UPLINK_TRANSACTION_TIME(_pDev)               \
(                                                       \
    PHY_COMPUTE_TX_TIME(                                \
        (_pDev)->xrBss.pRateTable,                      \
        sizeof(WLAN_DATA_MAC_HEADER3) + FCS_FIELD_SIZE, \
        (_pDev)->xrBss.defaultRateIndex, 0              \
    )                                                   \
    + ((INIT_AIFS_XR + INIT_CWMIN_XR) * XR_SLOT_DELAY)  \
    + PHY_COMPUTE_TX_TIME(                              \
        (_pDev)->xrBss.pRateTable,                      \
        XR_MAX_FRAME_SIZE,                              \
        (_pDev)->xrBss.defaultRateIndex, 0              \
    )                                                   \
    + PHY_COMPUTE_TX_TIME(                              \
        (_pDev)->xrBss.pRateTable,                      \
        WLAN_CTRL_FRAME_SIZE,                           \
        (_pDev)->xrBss.defaultRateIndex, 0              \
    )                                                   \
)

#define XR_HACKERY                      1
#define XR_HACKERY_MACADDR              0x9999  /* Define it to be byte order agnostic */
#define XR_HACKERY_BSSID_MASK           0x00    /* Maybe 0x80 */

/* commenting the following out will turn on XR support */
#undef  XR_HACKERY

#if defined(BUILD_AP) && defined(XR_HACKERY)
#define isXrAp(_pDev)                   ((_pDev)->hwRateTable[WIRELESS_MODE_XR])
#else
#define isXrAp(_pDev)                   FALSE
#endif

#define setXrBssId(_pBssId)             do { (_pBssId)->octets[2] ^= XR_HACKERY_BSSID_MASK; } while (0)
#define isXrBssMacAddr(_pMacAddr)       ((_pMacAddr)->st.half == XR_HACKERY_MACADDR)
#define isXrBssSib(_pDevInfo, _pSib)    ((_pSib)->pOpBss == &((_pDevInfo)->xrBss))
#define isXrBssRxFrame(_pDev, _pHdr)    ( isXrAp(_pDev) && isXrBssMacAddr(&((_pHdr)->address2)) )

#endif /* _ATH_AH_XR_H_ */
