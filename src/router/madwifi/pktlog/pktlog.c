/*************************************************************************
 * Copyright © 2000-2005 Atheros Communications, Inc., All Rights Reserved
 *
 * Atheros and the Atheros logo and design are trademarks of Atheros
 * Communications, Inc.
 *
 * $Id: //depot/sw/releases/linuxsrc/src/802_11/madwifi/madwifi/pktlog/pktlog.c#1 $
 *
 * Packet logging routines are defined here.
 *
 * Notes for use
 * -------------
 * By default, packet logging feature is available in the production release
 * as of 12/15/03.  We used to have a separate build to include packet 
 * logging feature.  Even though, the feature is available by default, it
 * is not enabled by default.  The feature gets triggered based on a 
 * variable "pktLogEnable".  
 *
 * At the station side, this variable need to be defined in the registry.
 * If this variable is non-zero, packets get logged into the buffer.  By
 * default this variable is set to 0.
 *
 * At AP side, set this variable using "set" command as follows:
 * set pktLogEnable 1.  
 */
#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

#include <linux/module.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/proc_fs.h>
#include <linux/sysctl.h>
#include "../../ratectrl/ratectrl.h"
#include "pktlog.h"
#include <asm/uaccess.h>

#ifndef REMOVE_PKT_LOG

static A_INT32 LogIdx   = -1;
static LogInfo myLogData[LOG_MAX];
static LogInfo *LogData = &myLogData[0];

void
pktLogEventDelete(void)
{
    if (LogData) {
        A_DRIVER_FREE(LogData, sizeof(LogInfo) * LOG_MAX);
        LogData = NULL;
    }
    LogIdx = -1;
}

void
pktLogEventClear(void)
{
    if (!LogData) {
        LogData = (LogInfo*) A_DRIVER_MALLOC(sizeof(LogInfo) * LOG_MAX);
        if (!LogData) {
            LogIdx = -1;
            return;
        }
    }

    memset(LogData, 0, sizeof(LogInfo) * LOG_MAX);
    LogIdx = 0;
}

void
pktLogEventAni(A_UINT8 phyStatsDisable, A_UINT8 noiseImmunLvl, A_UINT8 spurImmunLvl,
               A_UINT8 ofdmWeakDet, A_UINT8 cckWeakThr, A_UINT16 firLvl,
               A_UINT16 listenTime, A_UINT32 cycleCount, A_UINT32 msTick,
               A_UINT32 ofdmPhyErrCount, A_UINT32 cckPhyErrCount, A_INT32 rssi)
{
    LogInfo log;

    if (LogIdx < 0) {
        pktLogEventClear();
        if (LogIdx < 0) {
            return;
        }
    }

    log.timeStamp             = A_MS_TICKGET();
    log.d.ani.logType         = LOG_TYPE_ANI;
    log.d.ani.phyStatsDisable = phyStatsDisable;
    log.d.ani.noiseImmunLvl   = noiseImmunLvl;
    log.d.ani.spurImmunLvl    = spurImmunLvl;
    log.d.ani.ofdmWeakDet     = ofdmWeakDet;
    log.d.ani.cckWeakThr      = cckWeakThr;
    log.d.ani.firLvl          = firLvl;

    log.d.ani.listenTime      = (A_UINT16)listenTime;
    log.d.ani.cycleCount      = cycleCount;
    log.d.ani.msTick          = msTick;
    log.d.ani.ofdmPhyErrCount = ofdmPhyErrCount;
    log.d.ani.cckPhyErrCount  = cckPhyErrCount;
    log.d.ani.rssi            = rssi;

    LogData[LogIdx++]         = log;
    if (LogIdx >= LOG_MAX) {
        LogIdx = 0;
    }
}

void
pktLogEventRxPkt(A_UINT32 length, A_UINT32 rate, A_RSSI32 rssi,
              A_UINT32 frameCtrl, A_UINT32 seqCtrl, A_UINT32 status,
              A_UINT32 antRx, A_UINT16 timeStamp)
{
    LogInfo log;

    if (LogIdx < 0) {
        pktLogEventClear();
        if (LogIdx < 0) {
            return;
        }
    }

    log.timeStamp       = A_MS_TICKGET();
    log.d.rx.logType    = LOG_TYPE_RX;
    log.d.rx.rate       = (A_UINT8)rate;
    log.d.rx.rssi       = (A_RSSI)rssi;
    log.d.rx.frameCtrl  = (A_UINT16)frameCtrl;
    log.d.rx.seqCtrl    = (A_UINT16)seqCtrl;
    log.d.rx.status     = (A_UINT8)status;
    log.d.rx.length     = (A_UINT16)length;
    log.d.rx.antRx      = (A_UINT8)antRx;
    log.d.rx.timeStamp  = timeStamp;
    LogData[LogIdx++]   = log;
    if (LogIdx >= LOG_MAX) {
        LogIdx = 0;
    }
}

void
pktLogEventTxPkt(A_UINT32 length, A_UINT32 rate, A_RSSI32 ackRssi,
              A_UINT32 retries, A_UINT32 status, A_UINT32 frameCtrl,
              A_UINT32 seqCtrl, A_UINT32 antTx, A_UINT16 timeStamp)
{
    LogInfo log;

    if (LogIdx < 0) {
        pktLogEventClear();
        if (LogIdx < 0) {
            return;
        }
    }
    log.timeStamp       = A_MS_TICKGET();
    log.d.tx.logType    = LOG_TYPE_TX;
    log.d.tx.rate       = (A_UINT8)rate;
    log.d.tx.length     = (A_UINT16)length;
    log.d.tx.retries    = (A_UINT8)retries;
    log.d.tx.status     = (A_UINT8)status;
    log.d.tx.ackRssi    = (A_RSSI)ackRssi;
    log.d.tx.antTx      = (A_UINT8)antTx;
    log.d.tx.frameCtrl  = (A_UINT16)frameCtrl;
    log.d.tx.seqCtrl    = (A_UINT16)seqCtrl;
    log.d.tx.timeStamp  = timeStamp;
    LogData[LogIdx++]   = log;
    if (LogIdx >= LOG_MAX) {
        LogIdx = 0;
    }
}

void
pktLogEventRateCtrl(TxRateCtrlLog *rateLog)
{
    LogInfo log;

    if (LogIdx < 0) {
        pktLogEventClear();
        if (LogIdx < 0) {
            return;
        }
    }

    log.timeStamp     = A_MS_TICKGET();
    log.d.rate        = *rateLog;
    LogData[LogIdx++] = log;

    if (LogIdx >= LOG_MAX) {
        LogIdx = 0;
    }
}

void
pktLogEventGet(A_UINT32 *len, A_UINT32 *maxCnt, A_UINT32 *currIdx,
            A_UINT32 getIdx, void *data)
{
    if (LogIdx < 0) {
        *len     = sizeof(LogInfo);
        *maxCnt  = 0;
        *currIdx = 0;
        return;
    }
    *len     = sizeof(LogInfo);
    *maxCnt  = LOG_MAX;
    *currIdx = LogIdx;
    if (getIdx >= LOG_MAX) {
        getIdx = LOG_MAX - 1;
    }
    if (LogIdx >= 0) {
        *(LogInfo*)data = LogData[getIdx];
    }
}

void
pktLogEventGetHeader(A_UINT32 *len, A_UINT32 *logVer)
{
    *len     = sizeof(LogInfo);
    *logVer  = CURRENT_PKT_LOG_VER;
}

A_UINT32 savedIndex = 0;
int savedPktLogState = 0;

static int
ATH_SYSCTL_DECL(ath_dump_pktlog, ctl, write, filp, buffer, lenp, ppos)
{
#define NUM_USER_ENTRIES	10
#define COPY_TO_USER() {					\
	copy_to_user(buffer, string, strlen(string));		\
	buffer += strlen(string);				\
	}

    A_UINT8 data[256], string[100];
    A_UINT32 len, maxCnt, currIdx, count;
    A_UINT32 startIndex;
    char *bufstart = buffer;
    A_UINT32 i, j;
    A_UINT32 logVer, pktLogStartSeq;
    int validEntries = 0;

    pktLogEventGet(&len, &maxCnt, &currIdx, 0, data);
    pktLogEventGetHeader(&len, &logVer);
    pktLogStartSeq = PKTLOG_MAGIC_NUMBER;

    /* first request ? */
    if (filp->f_pos == 0) {
    	sprintf(string, "pktLog start\n");
	COPY_TO_USER();
    	sprintf(string, "%08x%08x%08x",pktLogStartSeq, logVer, len);
	COPY_TO_USER();
	savedIndex = 0;
	savedPktLogState = pktLogEnable;
    	pktLogEnable = 0; 	/* disable packet logging */
    } else {
	/* There is no more data left */
	if (currIdx == 0) {
		*lenp = 0;
		pktLogEnable = savedPktLogState;
		return 0;
	}
    }

    startIndex = savedIndex;

    if (NUM_USER_ENTRIES > (maxCnt - startIndex)) {
	count = (maxCnt - startIndex);
    } else {
	count = NUM_USER_ENTRIES;
    }

    for (i = startIndex; i < (count + startIndex); i++) {
        pktLogEventGet(&len, &maxCnt, &currIdx, i, data);
        if (*(int*)data == 0) {
            break;
        }
        for (j = 0; j < len; j++) {
            sprintf(string, "%02x", data[j]);
	    COPY_TO_USER();
        }
        sprintf(string, "\n");
	COPY_TO_USER();
    }

    validEntries += (i - startIndex);
    savedIndex += validEntries;

    *lenp = ((int)buffer - (int)bufstart);
    filp->f_pos += ((int)buffer - (int)bufstart);

    if (validEntries == 0) {
    	sprintf(string, "pktLog end\n");
	COPY_TO_USER();
    	*lenp = ((int)buffer - (int)bufstart);
    	filp->f_pos += ((int)buffer - (int)bufstart);
    	pktLogEventClear();
    }

    return 0;
}

int pktLogEnable = 0;

#define	CTL_AUTO	-2	/* cannot be CTL_ANY or CTL_NONE */

static ctl_table ath_pktlog_sysctl[] = {
	{ .ctl_name	= CTL_AUTO,
	  .procname	= "pktLog",
	  .mode		= 0644,
	  .data		= &pktLogEnable,
	  .maxlen	= sizeof(pktLogEnable),
	  .proc_handler	= proc_dointvec
	},
	{ .ctl_name	= CTL_AUTO,
	  .procname	= "pktData",
	  .mode		= 0444,
	  .proc_handler	= ath_dump_pktlog
	},
	{ 0 }
};

/*
 * Static (i.e. global) sysctls.  Note that the hal sysctls
 * are located under ours by sharing the setting for DEV_ATH.
 */
enum {
        DEV_ATH         = 9,                    /* XXX known by hal */
};

static ctl_table ath_ath_table[] = {
	{ .ctl_name	= DEV_ATH,
	  .procname	= "ath",
	  .mode		= 0555,
	  .child	= ath_pktlog_sysctl
	}, { 0 }
};

static ctl_table ath_root_table[] = {
	{ .ctl_name	= CTL_DEV,
	  .procname	= "dev",
	  .mode		= 0555,
	  .child	= ath_ath_table
	}, { 0 }
};

static struct ctl_table_header *ath_pktlog_header;

static int __init
pktlog_init(void)
{
	ath_pktlog_header = register_sysctl_table(ath_root_table, 1);
	return 0;
}

static void __exit
pktlog_exit(void)
{
	unregister_sysctl_table(ath_pktlog_header);
}

module_init(pktlog_init);
module_exit(pktlog_exit);

MODULE_AUTHOR("Atheros Communications, Inc.");
MODULE_DESCRIPTION("Support for Atheros packet log");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Atheros");
#endif

EXPORT_SYMBOL(pktLogEnable);
EXPORT_SYMBOL(pktLogEventRateCtrl);
EXPORT_SYMBOL(pktLogEventTxPkt);
EXPORT_SYMBOL(pktLogEventRxPkt);

#endif /* REMOVE_PKT_LOG */
