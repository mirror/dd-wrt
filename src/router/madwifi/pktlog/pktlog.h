/*
 * Copyright (c) 2000-2003, Atheros Communications Inc.
 *
 * See the corresponding .c file for additional usage information.
 *
 * $Id: //depot/sw/releases/linuxsrc/src/802_11/madwifi/madwifi/pktlog/pktlog.h#1 $
 */

#ifndef _PKTLOG_H_
#define _PKTLOG_H_

#ifdef __linux__
#include <linux/config.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>

// #include <../net80211/if_media.h>
// #include <../net80211/ieee80211_var.h>

// #include "if_athrate.h"
#include "ath/if_athvar.h"
#include "ath_rate/ratectrl/ratectrl.h"
#elif __FreeBSD__
#include <sys/param.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/errno.h>

#include <sys/socket.h>

#include <net/if.h>
#include <net/if_media.h>
#include <net80211/ieee80211_var.h>

#include <netinet/in.h> 
#include <netinet/if_ether.h>
#include <dev/ath/if_athrate.h>
#include <dev/ath/if_athvar.h>
#else
#error "Don't know how to handle your operating system!"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// #define ATH_PKT_LOG
#ifdef ATH_PKT_LOG

/*
 * Version info:
 * 10001 - Logs (rssiLast, rssiLastPrev)
 * 10002 - Logs (rssiLast, rssiLastPrev, rssiLastPrev2)
 */
#define CURRENT_PKT_LOG_VER     10002
#define LOG_TYPE_RX             1
#define LOG_TYPE_TX             2
#define LOG_TYPE_RATE_GET       3
#define LOG_TYPE_RATE_UPDATE    4
#define LOG_TYPE_ANI            5

#ifdef __linux__
#define A_DRIVER_MALLOC(s)	kmalloc((s), GFP_ATOMIC);
#define A_DRIVER_FREE(p, s)	kfree((p));
#endif

extern int pktLogEnable;
/*
 * NOTE: the Perl script that processes the packet log data
 *       structures has hardcoded structure offsets.
 *
 * When adding new fields it is important to:
 * - assign a newer verison number (CURRENT_VER_PKT_LOG)
 * - Add fields to the end of the structure.
 * - Observe proper memory alignment of fields. This
 *     eases the process of adding the structure offsets
 *     to the Perl script.
 * - note the new size of the pkt log data structure
 * - Update the Perl script to reflect the new structure and version/size changes
 */
typedef struct TxRateCtrlLog_s {
    A_UINT8 logType;
    struct TxRateCtrl_s rc;
    union {
        struct {
            A_UINT16    frameLen;
            A_UINT8     rate;
            A_UINT8     defAnt;
            A_UINT8     sibAnt;
        } g;
        struct {
            A_UINT8     rate;
            A_UINT8     xRetry;
            A_UINT8     retries;
            A_UINT16    frameLen;
            A_RSSI      rssiAck;
            A_UINT8     defAnt;
            A_UINT8     sibAnt;
        } t;
    } u;
} TxRateCtrlLog;

typedef struct {
    A_UINT32 timeStamp;
    union {
        struct {
            A_UINT8         logType;
            A_UINT8         rate;
            A_RSSI          rssi;
            A_UINT8         status;
            A_UINT16        length;
            A_UINT16        seqCtrl;
            A_UINT16        frameCtrl;
            A_UINT8         antRx;
            A_UINT8         dummy;
            A_UINT16        timeStamp;
        } rx;
        struct {
            A_UINT8         logType;
            A_UINT8         rate;
            A_UINT8         retries;
            A_UINT8         status;
            A_UINT16        length;
            A_UINT16        seqCtrl;
            A_UINT16        frameCtrl;
            A_RSSI          ackRssi;
            A_UINT8         antTx;
            A_UINT16        timeStamp;
        } tx;
/* ANI log */
        struct {
            A_UINT8        logType;
            A_UINT8        phyStatsDisable;
            A_UINT8        noiseImmunLvl;
            A_UINT8        spurImmunLvl;
            A_UINT16       dummy1;
            A_UINT16       dummy2;
            A_UINT16       dummy3;
            A_UINT8        ofdmWeakDet;
            A_UINT8        cckWeakThr;
            A_UINT16       firLvl;

            A_UINT16       listenTime;
            A_UINT32       cycleCount;
            A_UINT32       msTick;
            A_UINT32       ofdmPhyErrCount;
            A_UINT32       cckPhyErrCount;
            A_INT32        rssi;
            A_UINT32       dummy4;
            A_UINT32       dummy5;
            A_UINT32       dummy6;
        } ani;
    TxRateCtrlLog rate;
    } d;
} LogInfo;

#define LOG_MAX   4000


void pktLogEventGetHeader(A_UINT32 *len, A_UINT32 *logVer);
void pktLogEventGet(A_UINT32 *len, A_UINT32 *maxCnt, A_UINT32 *currIdx,
                    A_UINT32 getIdx, void *data);
void pktLogEventClear(void);
void pktLogEventDelete(void);
void pktLogEventRxPkt(A_UINT32 length, A_UINT32 rate, A_RSSI32 rssi,
                      A_UINT32 frameType, A_UINT32 seqNum, A_UINT32 status, A_UINT32 antRx,
                      A_UINT16 timeStamp);
void pktLogEventTxPkt(A_UINT32 length, A_UINT32 rate, A_RSSI32 ackRssi,
                      A_UINT32 retries, A_UINT32 status, A_UINT32 frameCtrl, A_UINT32 seqCtrl,
                      A_UINT32 antTx, A_UINT16 timeStamp);
void pktLogEventRateCtrl(TxRateCtrlLog *rateLog) ;
void pktLogChangeSlotTime(A_UINT32 aSlotTime);
void pktLogRetrieveFromAP(void) ;

/* ANI log */
void pktLogEventAni(A_UINT8 phyStatsDisable, A_UINT8 noiseImmunLvl, A_UINT8 spurImmunLvl,
                    A_UINT8 ofdmWeakDet, A_UINT8 cckWeakThr, A_UINT16 firLvl,
                    A_UINT16 listenTime, A_UINT32 cycleCount, A_UINT32 msTick,
                    A_UINT32 ofdmPhyErrCount, A_UINT32 cckPhyErrCount, A_INT32 rssi);

#define PKTLOG_TX_PKT(pdev, length, rate, rssi, retrycnt, status,  \
                      framecontrol, seqno, ant, timestamp)         \
    if (pktLogEnable) {                            		   \
        pktLogEventTxPkt(length, rate, rssi, retrycnt, status,     \
                         framecontrol, seqno, ant, timestamp);     \
    }

#define PKTLOG_RX_PKT(pdev, length, rate, rssi, frameType, seqNum, \
                      status, antRx, timeStamp)                    \
    if (pktLogEnable) {                            \
        pktLogEventRxPkt(length, rate, rssi, frameType, seqNum,    \
                         status, antRx, timeStamp);                \
    }

#define PKTLOG_EVENT_DELETE(sc)                              \
    if (pktLogEnable) {                        \
        pktLogEventDelete();                                       \
    }

#define PKTLOG_ENABLE_SET(gDrvInfo, devIndex)                      \
        gDrvInfo.pDev[devIndex]->staConfig.pktLogEnable =          \
            pCfg->radio[devIndex].pktLogEnable;

#define PKTLOG_EVENT_GET_HEADER(sc, buffer, bytesWritten)    \
    if (pktLogEnable) {                        \
        pktLogEventGetHeader(((A_UINT32*)buffer) + 1,              \
                             ((A_UINT32*)InformationBuffer));      \
        bytesWritten = 2 * sizeof(A_UINT32);                       \
        Status = NDIS_STATUS_SUCCESS;                              \
    }

#define PKTLOG_EVENT_GET(sc, buffer, bytesWritten) {         \
    static A_UINT32 rdIdx;                                         \
    int             i;                                             \
                                                                   \
    if (pktLogEnable) {                        \
        pktLogEventGet(((A_UINT32*)buffer) + 2,                    \
                       ((A_UINT32*)buffer) + 3,                    \
                       ((A_UINT32*)buffer) + 0, rdIdx,             \
                       ((A_UINT32*)buffer) + 4);                   \
        ((A_UINT32*)buffer)[1] = rdIdx;                            \
        rdIdx++;                                                   \
        if (rdIdx >= ((A_UINT32*)buffer)[3] ||                     \
            ((A_UINT32*)buffer)[4] == 0)                           \
        {                                                          \
            pktLogEventClear();                                    \
            rdIdx = 0;                                             \
        }                                                          \
        bytesWritten = 4 * sizeof(A_UINT32) +                      \
                       ((A_UINT32*)buffer)[2];                     \
        Status       = NDIS_STATUS_SUCCESS;                        \
    }                                                              \
}

#define PKTLOG_MAGIC_NUMBER 7735225

#define PKTLOG_RATE_CTL_GET(sc, pRcmisc, pRc, aFrameLen,     \
                            aRate, aDefAnt, aSibAnt)               \
    if (pktLogEnable) {                        \
        TxRateCtrlLog        log;                                  \
                                                                   \
        log.rc.misc[4]      = pRcmisc[4];                          \
        log.rc.misc[5]      = pRcmisc[5];                          \
        log.rc.misc[6]      = pRcmisc[6];                          \
        log.rc.misc[7]      = pRcmisc[7];                          \
        log.rc.misc[8]      = pRcmisc[8];                          \
        log.rc.misc[9]      = pRcmisc[9];                          \
        log.rc.misc[10]     = pRcmisc[10];                         \
        log.rc.misc[11]     = pRcmisc[11];                         \
        log.rc.misc[12]     = pRcmisc[12];                         \
        log.rc              = pRc;                                 \
        log.logType         = LOG_TYPE_RATE_GET;                   \
        log.u.g.frameLen    = aFrameLen;                           \
        log.u.g.rate        = aRate;                               \
        log.u.g.defAnt      = aDefAnt;                             \
        log.u.g.sibAnt      = aSibAnt;                             \
        pktLogEventRateCtrl(&log);                                 \
    }

#define PKTLOG_RATE_CTL_UPDATE(sc, aRc, numTxPending, boostState,\
                               useTurboPrime, rateTableSize, aRate, Xretries, aRetries, \
                               aFrameLen, aRssiAck, aDefAnt, aSibAnt) \
    if (pktLogEnable) {                        \
        TxRateCtrlLog        log;                                  \
                                                                   \
        log.rc              = aRc;                                 \
        log.rc.misc[0]      = numTxPending;                        \
        log.rc.misc[1]      = boostState;                          \
        log.rc.misc[2]      = useTurboPrime;                       \
        log.rc.misc[3]      = rateTableSize;                       \
        log.logType         = LOG_TYPE_RATE_UPDATE;                \
        log.u.t.rate        = aRate;                               \
        log.u.t.xRetry      = Xretries;                            \
        log.u.t.retries     = aRetries;                            \
        log.u.t.frameLen    = aFrameLen;                           \
        log.u.t.rssiAck     = aRssiAck;                            \
        log.u.t.defAnt      = aDefAnt;                             \
        log.u.t.sibAnt      = aSibAnt;                             \
        pktLogEventRateCtrl(&log);                                 \
    }

#define PKTLOG_CHANGE_SLOT_TIME(sc, aSlotTime)               \
    if (pktLogEnable)                          				   \
        pktLogChangeSlotTime(aSlotTime)


#else // #ifdef ATH_PKT_LOG




#define PKTLOG_TX_PKT(pdev, length, rate, rssi, retrycnt, status,  \
                      framecontrol, seqno, ant, timestamp)
#define PKTLOG_RX_PKT(pdev, length, rate, rssi, frameType, seqNum, \
                      status, antRx, timeStamp)
#define PKTLOG_ENABLE_SET(gDrvInfo, devIndex)
#define PKTLOG_EVENT_DELETE(sc)
#define PKTLOG_EVENT_GET_HEADER(sc, buffer, bytesWritten)    \
    bytesWritten = 0;
#define PKTLOG_CHANGE_SLOT_TIME(sc, aSlotTime)
#define PKTLOG_EVENT_GET(sc, buffer, bytesWritten)           \
    bytesWritten = 0;
#define PKTLOG_RATE_CTL_GET(sc, pRcmisc, pRc, aFrameLen,     \
                            aRate, aDefAnt, aSibAnt)
#define PKTLOG_RATE_CTL_UPDATE(sc, aRc, numTxPending, boostState,\
                               useTurboPrime, rateTableSize, aRate, Xretries, aRetries, \
                               aFrameLen, aRssiAck, aDefAnt, aSibAnt)

#endif // #ifdef ATH_PKT_LOG

#ifdef __cplusplus
}
#endif

#endif // _PKTLOG_H_
