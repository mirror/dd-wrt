/**
 * @file IxTimeSyncAcc_p.h
 *
 * @author Intel Corporation
 * @date 29 November 2004
 *
 * @brief  Private header file for IXP400 Access Layer to IEEE 1588(TM)
 * Precision Clock Synchronisation Protocol Hardware Assist.
 *
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

/*
 * #defines and macros used in this file.
 */

/* Inline or Non-Inlined function declaration/definition macro */
#ifdef NO_INLINE_APIS
    #define IXP400_TIMESYNCACC_INLINE  /* empty define */
#else /* else of ifdef NO_INLINE_APIS */
    #define IXP400_TIMESYNCACC_INLINE  __inline__
#endif /* end of ifdef NO_INLINE_APIS */

/*
 * SET, CLEAR & CHECK macros for bit manipulation on the 
 * memory mapped registers' contents
 */
#define IXP400_TIMESYNCACC_BIT_SET(regAddr, bitMask) \
do { \
    /* *(regAddr) |= (bitMask); */ \
    IX_OSAL_WRITE_LONG(regAddr, \
                       IX_OSAL_READ_LONG(regAddr) | bitMask); \
} while (0) /* Don't include ';' here */

#define IXP400_TIMESYNCACC_BIT_CLEAR(regAddr, bitMask) \
do { \
    /* *(regAddr) &= ~(bitMask); */ \
    IX_OSAL_WRITE_LONG(regAddr, \
                       IX_OSAL_READ_LONG(regAddr) & ~(bitMask)); \
} while (0) /* Don't include ';' here */

#define IXP400_TIMESYNCACC_BIT_SET_CHECK(regAddr, bitMask) \
    ((IX_OSAL_READ_LONG(regAddr)  & (bitMask)) == (bitMask))
/* Don't include ';' here */

/*
 * READ, WRITE macros for memory mapped registers
 */
#define IXP400_TIMESYNCACC_REG_READ(regAddr, varRef) \
do { \
    *(varRef) = IX_OSAL_READ_LONG(regAddr); \
} while (0) /* Don't include ';' here */

#define IXP400_TIMESYNCACC_REG_WRITE(regAddr, varValue) \
do { \
    IX_OSAL_WRITE_LONG(regAddr, varValue); \
} while (0) /* Don't include ';' here */

/* Masks to extract High and Low SHORTs from UINT32 values */
#define IXP400_TIMESYNCACC_MSB_SHORT_MASK  (0xFFFF0000)
#define IXP400_TIMESYNCACC_LSB_SHORT_MASK  (0x0000FFFF)

/* Location of SeqID in the register */
#define IXP400_TIMESYNCACC_SID_LOC         (16)

/* IRQ Level */
#define IRQ_IXP400_INTC_TSYNC IX_OSAL_IXP400_TSYNC_IRQ_LVL

/* Max Ports */
#define IXP400_TIMESYNCACC_MAX_1588PTP_PORT  (0x03)

/* Base Addresses for Block and Port Level Registers */
#define IXP400_TIMESYNCACC_BLREGS_BASEADDR   (0xC8010000)
#define IXP400_TIMESYNCACC_PLREGS_BASEADDR   (0xC8010040)

/* Size of the each Block / Port Level Register */
#define IXP400_TIMESYNCACC_BLPLREG_SIZE  (0x04)

/* Number of Block and Port Level Registers */
#define IXP400_TIMESYNCACC_BLREGS_COUNT  (0x10)
#define IXP400_TIMESYNCACC_PLREGS_COUNT  (0x08)

/* Address Ranges for Block and Port Level Registers */
#define IXP400_TIMESYNCACC_BLREGS_MEMMAP_SIZE  \
            (IXP400_TIMESYNCACC_BLPLREG_SIZE * \
             IXP400_TIMESYNCACC_BLREGS_COUNT)
#define IXP400_TIMESYNCACC_PLREGS_MEMMAP_SIZE  \
            (IXP400_TIMESYNCACC_BLPLREG_SIZE * \
             IXP400_TIMESYNCACC_PLREGS_COUNT * \
             IXP400_TIMESYNCACC_MAX_1588PTP_PORT)

/*
 * Block Level Registers Offset Values
 *
 * Please refer to the struct - IxTimeSyncAccBlockLevelRegisters defined
 * to hold the virtual addresses of the various block level registers of
 * time sync hardware 
 */
#define IXP400_TIMESYNCACC_TSC_OFFSET    (0x00)
#define IXP400_TIMESYNCACC_TSE_OFFSET    (0x04)
#define IXP400_TIMESYNCACC_ADD_OFFSET    (0x08)
#define IXP400_TIMESYNCACC_ACC_OFFSET    (0x0C)
/* TimeSync will not make use of the following four reserved registers but 
 * will be counted for correct implementation of offsets in the memory map
 *
 * Reserved for Testing
 * Reserved Unused
 * Reserved for NPE use
 * Reserved for NPE use
 */
#define IXP400_TIMESYNCACC_STL_OFFSET    (0x20)
#define IXP400_TIMESYNCACC_STH_OFFSET    (0x24)
#define IXP400_TIMESYNCACC_TTL_OFFSET    (0x28)
#define IXP400_TIMESYNCACC_TTH_OFFSET    (0x2C)
#define IXP400_TIMESYNCACC_ASSL_OFFSET   (0x30)
#define IXP400_TIMESYNCACC_ASSH_OFFSET   (0x34)
#define IXP400_TIMESYNCACC_AMSL_OFFSET   (0x38)
#define IXP400_TIMESYNCACC_AMSH_OFFSET   (0x3C)

/*
 * Port Level Registers Offset Values
 *
 * The following offset macros work as explained below.
 * 
 * Effective Address:= Starting Virtual Address + 
 *                     Vertical Offset + Block Offset for Port
 *
 * NOTE: a) Starting Virtual Address will be obtained using OSAL macro
 *       b) portNum (0 -> Max IXP400_TIMESYNCACC_MAX_1588PTP_PORT)
 *       c) Block Offset for Port starts from Zero
 *
 * The example assumes that Starting Virtual Address has been 0x40.
 *
 * Eg., CC0:  0x40 + (0x04 * 0x00) + (0x20 * 0x00) = 0x40
 *      XSH1: 0x40 + (0x04 * 0x03) + (0x20 * 0x01) = 0x6C
 *      RSH2: 0x40 + (0x04 * 0x05) + (0x20 * 0x02) = 0x94
 */

/* Vertical/Relative Offset of a given Port Level Register 
 * within a single set/block for each PTP Port
 *
 * Please refer to the struct - IxTimeSyncAccPortLevelRegisters defined
 * to hold the virtual addresses of the various block level registers of
 * time sync hardware
 */
#define IXP400_TIMESYNCACC_CC_VOFFSET     (0x00)
#define IXP400_TIMESYNCACC_CE_VOFFSET     (0x04)
#define IXP400_TIMESYNCACC_XSL_VOFFSET    (0x08)
#define IXP400_TIMESYNCACC_XSH_VOFFSET    (0x0C)
#define IXP400_TIMESYNCACC_RSL_VOFFSET    (0x10)
#define IXP400_TIMESYNCACC_RSH_VOFFSET    (0x14)
#define IXP400_TIMESYNCACC_UID_VOFFSET    (0x18)
#define IXP400_TIMESYNCACC_SID_VOFFSET    (0x1C)

/* Block wise offset of each Port Level Registers for a given PTP Port
 */
#define IXP400_TIMESYNCACC_PLREGS_BOFFSET      \
            (IXP400_TIMESYNCACC_BLPLREG_SIZE * \
             IXP400_TIMESYNCACC_PLREGS_COUNT)

/* Compounded Offsets for each of the Port Level Registers as explained
 * in the NOTE of the above comments
 */
#define IXP400_TIMESYNCACC_CC_OFFSET(portNum)  \
            (IXP400_TIMESYNCACC_CC_VOFFSET +   \
             IXP400_TIMESYNCACC_PLREGS_BOFFSET * (portNum))
#define IXP400_TIMESYNCACC_CE_OFFSET(portNum)  \
            (IXP400_TIMESYNCACC_CE_VOFFSET +   \
             IXP400_TIMESYNCACC_PLREGS_BOFFSET * (portNum))
#define IXP400_TIMESYNCACC_XSL_OFFSET(portNum) \
            (IXP400_TIMESYNCACC_XSL_VOFFSET +  \
             IXP400_TIMESYNCACC_PLREGS_BOFFSET * (portNum))
#define IXP400_TIMESYNCACC_XSH_OFFSET(portNum) \
            (IXP400_TIMESYNCACC_XSH_VOFFSET +  \
             IXP400_TIMESYNCACC_PLREGS_BOFFSET * (portNum))
#define IXP400_TIMESYNCACC_RSL_OFFSET(portNum) \
            (IXP400_TIMESYNCACC_RSL_VOFFSET +  \
             IXP400_TIMESYNCACC_PLREGS_BOFFSET * (portNum))
#define IXP400_TIMESYNCACC_RSH_OFFSET(portNum) \
            (IXP400_TIMESYNCACC_RSH_VOFFSET +  \
             IXP400_TIMESYNCACC_PLREGS_BOFFSET * (portNum))
#define IXP400_TIMESYNCACC_UID_OFFSET(portNum) \
            (IXP400_TIMESYNCACC_UID_VOFFSET +  \
             IXP400_TIMESYNCACC_PLREGS_BOFFSET * (portNum))
#define IXP400_TIMESYNCACC_SID_OFFSET(portNum) \
            (IXP400_TIMESYNCACC_SID_VOFFSET +  \
             IXP400_TIMESYNCACC_PLREGS_BOFFSET * (portNum))

/* 
 * Bit Masks of Block Level Control Register
 */
/* Auxiliary Master Mode snapshot Interrupt Mask */
#define IXP400_TIMESYNCACC_TSC_AMMS_MASK    (1 << 3)
/* Auxiliary Slave Mode snapshot Interrupt Mask */
#define IXP400_TIMESYNCACC_TSC_ASMS_MASK    (1 << 2)
/* Target Time Interrupt Mask */
#define IXP400_TIMESYNCACC_TSC_TTM_MASK     (1 << 1)
/* Hardware Assist Reset */
#define IXP400_TIMESYNCACC_TSC_RESET        (1 << 0)

/*
 * Bit Masks of Block Level Event Register
 */
/* Auxiliary Master Mode snapshot Event */
#define IXP400_TIMESYNCACC_TSE_SNM          (1 << 3)
/* Auxiliary Slave Mode snapshot Event */
#define IXP400_TIMESYNCACC_TSE_SNS          (1 << 2)
/* Target Time Interrupt Pending Event */
#define IXP400_TIMESYNCACC_TSE_TTIPEND      (1 << 1)

/*
 * Bit Masks of Channel/Port Level Control Register
 */
/* Timestamp All Messages Control Flag */
#define IXP400_TIMESYNCACC_CC_TA            (1 << 1)
/* Timestamp Master or Slave Mode Control Flag */
#define IXP400_TIMESYNCACC_CC_MM            (1 << 0)

/*
 * Bit Masks of Channel/Port Level Event Register
 */
/* Receive Snapshot Locked Indicator Flag */
#define IXP400_TIMESYNCACC_CE_RXS           (1 << 1)
/* Transmit Snapshot Locked Indicator Flag */
#define IXP400_TIMESYNCACC_CE_TXS           (1 << 0)

/* TimeSync Init Check Macro */
#define IXP400_TIMESYNCACC_INIT_CHECK() \
do { \
    if (FALSE == ixTs1588HardwareAssistEnabled) \
    { \
        /* \
         * Check for IXP46X device NPEs fused-out condition \
         * and Initialise Mem Map \
         */ \
        IxTimeSyncAccInitStatus initStatus = ixTimeSyncAccInitCheck(); \
        if (IX_SUCCESS != (IX_STATUS) initStatus) \
        { \
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, \
                "ixTimeSyncAccInitCheck(): TimeSync Initialisation Failed (%x)\n", \
                initStatus,   \
                0,0,0,0,0); \
            return IX_TIMESYNCACC_FAILED; \
        } /* end of if (IX_SUCCESS != (IX_STATUS) initStatus) */ \
    } /* end of if (FALSE == ixTs1588HardwareAssistEnabled) */ \
} while (0) /* Don't include ';' here */

/* NPE Fused-Out Status Check Macro */
#define IXP400_TIMESYNCACC_NPE_FUSED_STATUS_CHECK(ptpPort) \
do { \
    /* Check NPEs Fused-Out status */ \
    if (TRUE != ixTsNpeEnabled[(ptpPort)]) \
{ \
        return IX_TIMESYNCACC_FAILED; \
    } /* end of if (TRUE == ixTsNpeEnabled[(ptpPort)]) */ \
} while (0) /* Don't include ';' here */

/*
 * Macros to handle mutexes in functions other than InitCheck
 */

/* Lock Mutex Macro to be used by functions other than InitCheck */
#define IXP400_TIMESYNCACC_MUTEX_LOCK(tsMutex) \
do { \
    if (IX_SUCCESS != ixOsalMutexLock(&tsMutex, IX_OSAL_WAIT_FOREVER)) \
    { \
        return IX_TIMESYNCACC_FAILED;  \
    } /* end of if (IX_SUCCESS != ixOsalMutexLock(&tsMutex, IX_OSAL_WAIT_FOREVER)) */ \
} while (0) /* Don't include ';' here */

/* Unlock Mutex Macro to be used by functions other than InitCheck */
#define IXP400_TIMESYNCACC_MUTEX_UNLOCK(tsMutex) \
do { \
    if (IX_SUCCESS != ixOsalMutexUnlock(&tsMutex)) \
    { \
        return IX_TIMESYNCACC_FAILED;  \
    } /* end of if (IX_SUCCESS != ixOsalMutexUnlock(&tsMutex)) */ \
} while (0) /* Don't include ';' here */

/*
 * Macro to be used by InitCheck function
 */

/* Release Mutex Macro */
#define IXP400_TIMESYNCACC_MUTEX_RELEASE(tsMutex) \
do { \
    ixOsalMutexUnlock(&tsMutex);  \
    ixOsalMutexDestroy(&tsMutex); \
} while (0) /* Don't include ';' here */

/*
 * Typedefs used in this file
 */

/* Block Level Registers */
typedef struct
{
    UINT32 tsControl;   /* Time Sync Control */
    UINT32 tsEvent;     /* Time Sync Event */
    UINT32 tsAddend;    /* Addend */
    UINT32 tsAccum;     /* Accumulator */
    UINT32 tsSysTimeLo; /* SystemTime_Low */
    UINT32 tsSysTimeHi; /* SystemTime_High */
    UINT32 tsTrgtLo;    /* TargetTime_Low */
    UINT32 tsTrgtHi;    /* TargetTime_High */
    UINT32 tsASMSLo;    /* AuxSlaveModeSnap_Low */
    UINT32 tsASMSHi;    /* AuxSlaveModeSnap_High */
    UINT32 tsAMMSLo;    /* AuxMasterModeSnap_Low */
    UINT32 tsAMMSHi;    /* AuxMasterModeSnap_High */
} IxTimeSyncAccBlockLevelRegisters;

/* Port Level Registers */
typedef struct
{
    UINT32 tsChControl;  /* TS Channel Control */
    UINT32 tsChEvent;    /* TS Channel Event */
    UINT32 tsTxSnapLo;   /* XMIT Snaphot Low */
    UINT32 tsTxSnapHi;   /* XMIT Snapshot High */
    UINT32 tsRxSnapLo;   /* RECV Snapshot Low */
    UINT32 tsRxSnapHi;   /* RECV Snapshot High */
    UINT32 tsSrcUUIDLo;  /* SourceUUID Low */
    UINT32 tsSrcUUIDHi;  /* SequenceID0/SourceUUID High */
} IxTimeSyncAccPortLevelRegisters;

/* Virtual addresses of time sync hardware registers */
typedef struct
{
    /* Block Level Registers */
    IxTimeSyncAccBlockLevelRegisters  blRegisters;
    /* Port Level Registers */
    IxTimeSyncAccPortLevelRegisters   plRegisters[
        IXP400_TIMESYNCACC_MAX_1588PTP_PORT];
} IxTimeSyncAccRegisters;

/* NPE Identifiers */
typedef enum {
    IXP400_TIMESYNCACC_NPEA = 0,      /* NPE - A (PTP Port #0) */
    IXP400_TIMESYNCACC_NPEB,          /* NPE - B (PTP Port #1) */
    IXP400_TIMESYNCACC_NPEC,          /* NPE - C (PTP Port #2) */
    IXP400_TIMESYNCACC_NPE_UNKNOWN =  /* NPE - Unknown (Max PTP Port) */
        IXP400_TIMESYNCACC_MAX_1588PTP_PORT
} IxTimeSyncAccNpeId;

/* Initialisation Status */
typedef enum {
    IXP400_TIMESYNCACC_INIT_SUCCESS = IX_SUCCESS, /* Init successful */
    IXP400_TIMESYNCACC_INIT_FAIL    = IX_FAIL,    /* Init failed */
    IXP400_TIMESYNCACC_INIT_MUTEX_FAIL,       /* Fail to secure mutex */
    IXP400_TIMESYNCACC_INIT_TS_SUPPORT_FAIL,  /* TimeSync not supported */
    IXP400_TIMESYNCACC_INIT_TS_DISABLED_FAIL, /* TimeSync feature disabled */
    IXP400_TIMESYNCACC_INIT_ISR_BIND_FAIL,    /* ISR not installed */
    IXP400_TIMESYNCACC_INIT_MEM_ASGN_FAIL     /* Virtual mem assignment fail */
} IxTimeSyncAccInitStatus;

/*
 * Variable declarations global to TimeSync access component
 */

/* 1588 Hardware Assist enabled / disabled as found by Feature Control */
static BOOL ixTs1588HardwareAssistEnabled = FALSE;  /* Assume it is Disabled */

/* TimeSync Hardware Registers */
static IxTimeSyncAccRegisters ixTsRegisters;
/*
 * The array of NPE Enabled variables will be set to TRUE (enabled) or 
 * FALSE (disabled) of the NPE fused-out status with the help of the 
 * IxFeatureCtrlAcc component.
 */
static BOOL ixTsNpeEnabled[IXP400_TIMESYNCACC_MAX_1588PTP_PORT] =
{
    FALSE,  /* Assume NPE-A disabled */
    FALSE,  /* Assume NPE-B disabled */
    FALSE   /* Assume NPE-C disabled */
};

/*
 * Mutexes to protect
 * a) the system time from frequency scaling value change during the 
 *    client request to set system time.
 * b) initialisation sequence completion protection mechanism
 */
static  IxOsalMutex ixTsSysTimeMutex;      /* (a) */
static  IxOsalMutex ixTsInitChkMutex;      /* (b) */

/*
 * Client registered callback routines for 
 * a) the target time reached or exceeded interrupt notification
 * b) the auxiliary time stamps availability interrupt notification
 */
static IxTimeSyncAccTargetTimeCallback  ixTsTargetTimeCallback    = NULL; /*(a)*/
static IxTimeSyncAccAuxTimeCallback     ixTsAuxMasterTimeCallback = NULL; /*(b)*/
static IxTimeSyncAccAuxTimeCallback     ixTsAuxSlaveTimeCallback  = NULL; /*(b)*/

/*
 * The transmit and receive timestamp statistics 
 */
static IxTimeSyncAccStats ixTsStats = { 0,0 };


/*
 * Support functions declarations
 */

/* TimeSync Interrupt Service Routine declaration */
void
ixTimeSyncAccIsr(void);

/* Initialise the base address registers */
PRIVATE IX_STATUS
ixTimeSyncAccBlPlBaseAddressesSet (void);

/* Initialsation Check */
PRIVATE IxTimeSyncAccInitStatus
ixTimeSyncAccInitCheck (void);

PRIVATE IxTimeSyncAcc1588PTPPortMode
ixTimeSyncAccPTPPortModeGet(IxTimeSyncAcc1588PTPPort ptpPort);

/*
 * Local functions definitions.
 */

/*
 * ------------------------------------------------------------------ *
 * Block level configuration support functions definitions
 * ------------------------------------------------------------------ *
 */

/* Enable Auxiliary Master Mode Snapshot Interrupt */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccControlAmmsInterruptMaskSet(void)
{
    /* SET the amms bit */
    IXP400_TIMESYNCACC_BIT_SET(ixTsRegisters.blRegisters.tsControl,
                              IXP400_TIMESYNCACC_TSC_AMMS_MASK);
} /* end of ixTimeSyncAccControlAmmsInterruptMaskSet() function */

/* Enable Auxiliary Slave Mode Snapshot Interrupt */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccControlAsmsInterruptMaskSet(void)
{
    /* SET the asms bit */
    IXP400_TIMESYNCACC_BIT_SET(ixTsRegisters.blRegisters.tsControl,
                              IXP400_TIMESYNCACC_TSC_ASMS_MASK);
} /* end of ixTimeSyncAccControlAsmsInterruptMaskSet() function */

/* Enable Target Time Interrupt */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccControlTtmInterruptMaskSet(void)
{
    /* SET the ttm bit */
    IXP400_TIMESYNCACC_BIT_SET(ixTsRegisters.blRegisters.tsControl,
                               IXP400_TIMESYNCACC_TSC_TTM_MASK);
} /* end of ixTimeSyncAccControlTtmInterruptMaskSet() function */

/* Get Auxiliary Master Mode Snapshot Interrupt Mask value */
PRIVATE IXP400_TIMESYNCACC_INLINE BOOL 
ixTimeSyncAccControlAmmsInterruptMaskGet(void)
{
    /* Is the amms bit SET? */
    return IXP400_TIMESYNCACC_BIT_SET_CHECK(ixTsRegisters.blRegisters.tsControl,
                                            IXP400_TIMESYNCACC_TSC_AMMS_MASK);
} /* end of ixTimeSyncAccControlAmmsInterruptMaskGet() function */

/* Get Auxiliary Slave Mode Snapshot Interrupt  Mask value */
PRIVATE IXP400_TIMESYNCACC_INLINE BOOL
ixTimeSyncAccControlAsmsInterruptMaskGet(void)
{
    /* Is the asms bit SET? */
    return IXP400_TIMESYNCACC_BIT_SET_CHECK(ixTsRegisters.blRegisters.tsControl,
                                            IXP400_TIMESYNCACC_TSC_ASMS_MASK);
} /* end of ixTimeSyncAccControlAsmsInterruptMaskGet() function */

/* Get Target Time Interrupt Mask value */
PRIVATE IXP400_TIMESYNCACC_INLINE BOOL
ixTimeSyncAccControlTtmInterruptMaskGet(void)
{
    /* Is the ttm bit SET? */
    return IXP400_TIMESYNCACC_BIT_SET_CHECK(ixTsRegisters.blRegisters.tsControl,
                                            IXP400_TIMESYNCACC_TSC_TTM_MASK);
} /* end of ixTimeSyncAccControlTtmInterruptMaskGet() function */

/* Disable Auxiliary Master Mode Snapshot Interrupt */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccControlAmmsInterruptMaskClear(void)
{
    /* CLEAR the amms bit */
    IXP400_TIMESYNCACC_BIT_CLEAR(ixTsRegisters.blRegisters.tsControl,
                                 IXP400_TIMESYNCACC_TSC_AMMS_MASK);
} /* end of ixTimeSyncAccControlAmmsInterruptMaskClear() function */

/* Disable Auxiliary Slave Mode Snapshot Interrupt */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccControlAsmsInterruptMaskClear(void)
{
    /* CLEAR the asms bit */
    IXP400_TIMESYNCACC_BIT_CLEAR(ixTsRegisters.blRegisters.tsControl,
                                 IXP400_TIMESYNCACC_TSC_ASMS_MASK);
} /* end of ixTimeSyncAccControlAsmsInterruptMaskClear() function */

/* Disable Target Time Interrupt */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccControlTtmInterruptMaskClear(void)
{
    /* CLEAR the ttm bit */
    IXP400_TIMESYNCACC_BIT_CLEAR(ixTsRegisters.blRegisters.tsControl,
                                 IXP400_TIMESYNCACC_TSC_TTM_MASK);
} /* end of ixTimeSyncAccControlTtmInterruptMaskClear() function */

/* Reset Hardware Assist block */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccControlReset(void)
{
    /* SET the rst bit */
    IXP400_TIMESYNCACC_BIT_SET(ixTsRegisters.blRegisters.tsControl,
                               IXP400_TIMESYNCACC_TSC_RESET);
    /* CLEAR the rst bit */
    IXP400_TIMESYNCACC_BIT_CLEAR(ixTsRegisters.blRegisters.tsControl,
                                 IXP400_TIMESYNCACC_TSC_RESET);
} /* end of ixTimeSyncAccControlReset() function */

/* Poll for Auxiliary Master Mode Snapshot Captured event */
PRIVATE IXP400_TIMESYNCACC_INLINE BOOL 
ixTimeSyncAccEventAmmsFlagGet(void)
{
    /* Is the snm bit SET? */
    return IXP400_TIMESYNCACC_BIT_SET_CHECK(ixTsRegisters.blRegisters.tsEvent,
                                            IXP400_TIMESYNCACC_TSE_SNM);
} /* end of ixTimeSyncAccEventAmmsFlagGet() function */

/* Poll for Auxiliary Slave Mode Snapshot Captured event */
PRIVATE IXP400_TIMESYNCACC_INLINE BOOL
ixTimeSyncAccEventAsmsFlagGet(void)
{
    /* Is the sns bit SET? */
    return IXP400_TIMESYNCACC_BIT_SET_CHECK(ixTsRegisters.blRegisters.tsEvent,
                                            IXP400_TIMESYNCACC_TSE_SNS);
} /* end of ixTimeSyncAccEventAsmsFlagGet() function */

/* end of Poll for Target Time Reached event function */
PRIVATE IXP400_TIMESYNCACC_INLINE BOOL
ixTimeSyncAccEventTtmFlagGet(void)
{
    /* Is the ttipend bit SET? */
    return IXP400_TIMESYNCACC_BIT_SET_CHECK(ixTsRegisters.blRegisters.tsEvent,
                                            IXP400_TIMESYNCACC_TSE_TTIPEND);
} /* end of ixTimeSyncAccEventTtmFlagGet() function */

/* Clear Auxiliary Master Mode Snapshot Captured event */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccEventAmmsFlagClear(void)
{
    /* CLEAR the snm bit by writing '1' onto it */
    IXP400_TIMESYNCACC_BIT_SET(ixTsRegisters.blRegisters.tsEvent,
                               IXP400_TIMESYNCACC_TSE_SNM);
} /* end of ixTimeSyncAccEventAmmsFlagClear() function */

/* Clear Auxiliary Slave Mode Snapshot Captured event */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccEventAsmsFlagClear(void)
{
    /* CLEAR the sns bit by writing '1' onto it */
    IXP400_TIMESYNCACC_BIT_SET(ixTsRegisters.blRegisters.tsEvent,
                               IXP400_TIMESYNCACC_TSE_SNS);
} /* end of ixTimeSyncAccEventAsmsFlagClear() function */

/* Clear Target Time Reached event */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccEventTtmFlagClear(void)
{
    /* CLEAR the ttipend bit by writing '1' onto it */
    IXP400_TIMESYNCACC_BIT_SET(ixTsRegisters.blRegisters.tsEvent,
                               IXP400_TIMESYNCACC_TSE_TTIPEND);
} /* end of ixTimeSyncAccEventTtmFlagClear() function */


/*
 * ------------------------------------------------------------------ *
 * Block level timestamp support functions definitions
 * ------------------------------------------------------------------ *
 */

/* Set System Time value */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccSystemTimeSnapshotSet (
    UINT32 systemTimeLow,
    UINT32 systemTimeHigh)
{
    /* Update the System Time Low Register contents */
    IXP400_TIMESYNCACC_REG_WRITE(ixTsRegisters.blRegisters.tsSysTimeLo,
                                 systemTimeLow);
    /* Update the System Time High Register contents */
    IXP400_TIMESYNCACC_REG_WRITE(ixTsRegisters.blRegisters.tsSysTimeHi,
                                 systemTimeHigh);
} /* end of ixTimeSyncAccSystemTimeSnapshotSet() function */

/* Get System Time Low value */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccSystemTimeSnapshotGet(
    UINT32 *systemTimeLow,
    UINT32 *systemTimeHigh)
{
    /* Fetch the System Time Low Register contents */
    IXP400_TIMESYNCACC_REG_READ(ixTsRegisters.blRegisters.tsSysTimeLo,
                                systemTimeLow);
    /* Fetch the System Time High Register contents */
    IXP400_TIMESYNCACC_REG_READ(ixTsRegisters.blRegisters.tsSysTimeHi,
                                systemTimeHigh);
} /* end of ixTimeSyncAccSystemTimeSnapshotGet() function */

/* Set Target Time value */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccTargetTimeSnapshotSet (
    UINT32 targetTimeLow,
    UINT32 targetTimeHigh)
{
    /* Update the Target Time Low Register contents */
    IXP400_TIMESYNCACC_REG_WRITE(ixTsRegisters.blRegisters.tsTrgtLo,
                                 targetTimeLow);
    /* Update the Target Time High Register contents */
    IXP400_TIMESYNCACC_REG_WRITE(ixTsRegisters.blRegisters.tsTrgtHi,
                                 targetTimeHigh);
} /* end of ixTimeSyncAccTargetTimeSnapshotSet() function */

/* Get Target Time value */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccTargetTimeSnapshotGet(
    UINT32 *targetTimeLow,
    UINT32 *targetTimeHigh)
{
    /* Fetch the Target Time Low Register contents */
    IXP400_TIMESYNCACC_REG_READ(ixTsRegisters.blRegisters.tsTrgtLo,
                                targetTimeLow);
    /* Fetch the Target Time High Register contents */
    IXP400_TIMESYNCACC_REG_READ(ixTsRegisters.blRegisters.tsTrgtHi,
                                targetTimeHigh);
} /* end of ixTimeSyncAccTargetTimeSnapshotGet() function */

/* Set Frequency Scaling Value */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccAddendFsvSet (UINT32 fsv)
{
    /* Update the Addend Register contents */
    IXP400_TIMESYNCACC_REG_WRITE(ixTsRegisters.blRegisters.tsAddend, fsv);
} /* end of ixTimeSyncAccAddendFsvSet() function */

/* Get Frequency Scaling Value */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccAddendFsvGet (UINT32 *fsv)
{
    /* Fetch the Addend Register contents */
    IXP400_TIMESYNCACC_REG_READ(ixTsRegisters.blRegisters.tsAddend, fsv);
} /* end of ixTimeSyncAccAddendFsvGet() function */

/* Get AMMS value */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccAuxMasterModeSnapshotGet (
    UINT32 *ammsLow,
    UINT32 *ammsHigh)
{
    /* Fetch the Auxiliary Master Mode Snapshot Low Register contents */
    IXP400_TIMESYNCACC_REG_READ(ixTsRegisters.blRegisters.tsAMMSLo, ammsLow);
    /* Fetch the Auxiliary Master Mode Snapshot High Register contents */
    IXP400_TIMESYNCACC_REG_READ(ixTsRegisters.blRegisters.tsAMMSHi, ammsHigh);
} /* end of ixTimeSyncAccAuxMasterModeSnapshotGet() function */

/* Get AMMS value */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccAuxSlaveModeSnapshotGet (
    UINT32 *asmsLow,
    UINT32 *asmsHigh)
{
    /* Fetch the Auxiliary Slave Mode Snapshot Low Register contents */
    IXP400_TIMESYNCACC_REG_READ(ixTsRegisters.blRegisters.tsASMSLo, asmsLow);
    /* Fetch the Auxiliary Slave Mode Snapshot High Register contents */
    IXP400_TIMESYNCACC_REG_READ(ixTsRegisters.blRegisters.tsASMSHi, asmsHigh);
} /* end of ixTimeSyncAccAuxSlaveModeSnapshotGet() function */


/*
 * ------------------------------------------------------------------ *
 * Port level configuration support functions definitions
 * ------------------------------------------------------------------ *
 */

/* Set the channel mode to 1588 Master */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccControlPTPPortMasterModeSet (
    UINT32 ptpPort,
    BOOL masterMode)
{
    /* SET or CLEAR the Master Mode */
    if (TRUE == masterMode)
    {
        /* SET the mm bit */
        IXP400_TIMESYNCACC_BIT_SET(
            ixTsRegisters.plRegisters[ptpPort].tsChControl,
            IXP400_TIMESYNCACC_CC_MM);
    }
    else /* else of if (TRUE == masterMode) */
    {
        /* CLEAR the mm bit */
        IXP400_TIMESYNCACC_BIT_CLEAR(
            ixTsRegisters.plRegisters[ptpPort].tsChControl,
            IXP400_TIMESYNCACC_CC_MM);
    } /* if (TRUE == masterMode) */
} /* end of ixTimeSyncAccControlPTPPortMasterModeSet() function */

/* Check for 1588 master mode of channel */
PRIVATE IXP400_TIMESYNCACC_INLINE BOOL
ixTimeSyncAccControlPTPPortMasterModeGet (UINT32 ptpPort)
{
    /* Is the mm bit SET? */
    return IXP400_TIMESYNCACC_BIT_SET_CHECK(
               ixTsRegisters.plRegisters[ptpPort].tsChControl,
               IXP400_TIMESYNCACC_CC_MM);
} /* end of ixTimeSyncAccControlPTPPortMasterModeGet() function */

/* Set Timestamp all or only PTP messages flag */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccControlPTPPortPTPMsgTimestampSet (
    UINT32 ptpPort,
    BOOL allMsg)
{
    /* SET or CLEAR the All Message Timestamping */
    if (TRUE == allMsg)
    {
        /* SET the ta bit */
        IXP400_TIMESYNCACC_BIT_SET(
            ixTsRegisters.plRegisters[ptpPort].tsChControl,
            IXP400_TIMESYNCACC_CC_TA);
    }
    else /* else of if (TRUE == allMsg) */
    {
    /* CLEAR the ta bit */
        IXP400_TIMESYNCACC_BIT_CLEAR(
            ixTsRegisters.plRegisters[ptpPort].tsChControl,
            IXP400_TIMESYNCACC_CC_TA);
    } /* if (TRUE == allMsg) */
} /* end of ixTimeSyncAccControlPTPPortPTPMsgTimestampSet() function */

/* Check for Timestamp all or only PTP messages flag */
PRIVATE IXP400_TIMESYNCACC_INLINE BOOL
ixTimeSyncAccControlPTPPortPTPMsgTimestampGet(UINT32 ptpPort)
{
    /* Is the ta bit SET? */
    return IXP400_TIMESYNCACC_BIT_SET_CHECK(
               ixTsRegisters.plRegisters[ptpPort].tsChControl,
               IXP400_TIMESYNCACC_CC_TA);
} /* end of ixTimeSyncAccControlPTPPortPTPMsgTimestampGet() function */


/*
 * ------------------------------------------------------------------ *
 * Port level timestamp support functions definitions
 * ------------------------------------------------------------------ *
 */

/* Receive Timestamp available */
PRIVATE IXP400_TIMESYNCACC_INLINE BOOL
ixTimeSyncAccControlPTPPortRxsFlagGet (UINT32 ptpPort)
{
    /* Is the rxs bit SET? */
    return IXP400_TIMESYNCACC_BIT_SET_CHECK(
               ixTsRegisters.plRegisters[ptpPort].tsChEvent,
               IXP400_TIMESYNCACC_CE_RXS);
} /* end of ixTimeSyncAccControlPTPPortRxsFlagGet() function */

/* Transmit Timestamp available */
PRIVATE IXP400_TIMESYNCACC_INLINE BOOL
ixTimeSyncAccControlPTPPortTxsFlagGet (UINT32 ptpPort)
{
    /* Is the txs bit SET? */
    return IXP400_TIMESYNCACC_BIT_SET_CHECK(
               ixTsRegisters.plRegisters[ptpPort].tsChEvent,
               IXP400_TIMESYNCACC_CE_TXS);
} /* end of ixTimeSyncAccControlPTPPortTxsFlagGet() function */

/* Clear Receive Timestamp available event */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccControlPTPPortRxsFlagClear(UINT32 ptpPort)
{
    /* CLEAR the rxs bit by writing '1' onto it */
    IXP400_TIMESYNCACC_REG_WRITE(ixTsRegisters.plRegisters[ptpPort].tsChEvent,
                               IXP400_TIMESYNCACC_CE_RXS);
} /* end of ixTimeSyncAccControlPTPPortRxsFlagClear() function */

/* Clear Transmit Timestamp available event */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccControlPTPPortTxsFlagClear(UINT32 ptpPort)
{
    /* CLEAR the txs bit by writing '1' onto it */
    IXP400_TIMESYNCACC_REG_WRITE(ixTsRegisters.plRegisters[ptpPort].tsChEvent,
                               IXP400_TIMESYNCACC_CE_TXS);
} /* end of ixTimeSyncAccControlPTPPortTxsFlagClear() function */

/* Get PTP Port Rx Timestamp value */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccPTPPortReceiveSnapshotGet (
    UINT32 ptpPort,
    UINT32 *rxsLow,
    UINT32 *rxsHigh)
{
    /* Fetch the Receive Timestamp/Snapshot Low Register contents */
    IXP400_TIMESYNCACC_REG_READ(ixTsRegisters.plRegisters[ptpPort].tsRxSnapLo,
                                rxsLow);
    /* Fetch the Receive Timestamp/Snapshot High Register contents */
    IXP400_TIMESYNCACC_REG_READ(ixTsRegisters.plRegisters[ptpPort].tsRxSnapHi,
                                rxsHigh);
} /* end of ixTimeSyncAccPTPPortReceiveSnapshotGet() function */

/* Get PTP Port Tx Timestamp value */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccPTPPortTransmitSnapshotGet (
    UINT32 ptpPort,
    UINT32 *txsLow,
    UINT32 *txsHigh)
{
    /* Fetch the Transmit Timestamp/Snapshot Low Register contents */
    IXP400_TIMESYNCACC_REG_READ(ixTsRegisters.plRegisters[ptpPort].tsTxSnapLo,
                                txsLow);
    /* Fetch the Transmit Timestamp/Snapshot High Register contents */
    IXP400_TIMESYNCACC_REG_READ(ixTsRegisters.plRegisters[ptpPort].tsTxSnapHi,
                                txsHigh);
} /* end of ixTimeSyncAccPTPPortTransmitSnapshotGet() function */

/* Get UUID High (16-bit value) & Sequence ID (16-bit value) of PTP message */
PRIVATE IXP400_TIMESYNCACC_INLINE void
ixTimeSyncAccPTPMsgUuidSeqIdGet (
    UINT32 ptpPort,
    UINT32 *uuidLow,
    UINT16 *uuidHigh,
    UINT16 *seqId)
{
    /* Local variables */
    UINT32 seqIdUuidHigh = 0;

    /* Fetch the UUID Low Register contents */
    IXP400_TIMESYNCACC_REG_READ(ixTsRegisters.plRegisters[ptpPort].tsSrcUUIDLo,
                                uuidLow);
    /* Fetch the Sequence ID and Source UUID High Register contents */
    IXP400_TIMESYNCACC_REG_READ(ixTsRegisters.plRegisters[ptpPort].tsSrcUUIDHi,
                                &seqIdUuidHigh);

    *seqId    = (seqIdUuidHigh >> IXP400_TIMESYNCACC_SID_LOC);
    *uuidHigh = (IXP400_TIMESYNCACC_LSB_SHORT_MASK & seqIdUuidHigh);
} /* end of ixTimeSyncAccPTPMsgUuidHighSeqIdGet() function */
