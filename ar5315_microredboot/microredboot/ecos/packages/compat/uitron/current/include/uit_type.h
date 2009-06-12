#ifndef CYGONCE_COMPAT_UITRON_UIT_TYPE_H
#define CYGONCE_COMPAT_UITRON_UIT_TYPE_H
//===========================================================================
//
//      uit_type.h
//
//      uITRON specific data types as required by the API
//
//===========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   hmt
// Contributors:        hmt
// Date:        1998-03-13
// Purpose:     uITRON specific data types as required by the API
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include <pkgconf/uitron.h>             // uITRON setup CYGNUM_UITRON_SEMAS
                                        // CYGPKG_UITRON et al

#ifdef CYGPKG_UITRON

// ------------------------------------------------------------------------
// uITRON types
//

// from this section of the uITRON 3.02 manual:
// ***********************************************************************
// ***    6.4 Data Types                                               ***
// ***********************************************************************
//
// ***  General-Purpose Data Types ***************************************

typedef cyg_int8    B;          // signed 8-bit integer
typedef cyg_int16   H;          // signed 16-bit integer
typedef cyg_int32   W;          // signed 32-bit integer
typedef cyg_uint8   UB;         // unsigned 8-bit integer
typedef cyg_uint16  UH;         // unsigned 16-bit integer
typedef cyg_uint32  UW;         // unsigned 32-bit integer
                                // 
typedef cyg_uint32  VW;         // unpredictable data type (32-bit size)
typedef cyg_uint16  VH;         // unpredictable data type (16-bit size)
typedef cyg_uint8   VB;         // unpredictable data type (8-bit size)
                                
typedef void *      VP;         // pointer to an unpredictable data type
                                
typedef CYG_ADDRWORD FP;        // program start address

// * The difference between VB, VH and VW and B, H and W is that only the
//   number of bits is known for the former, not the data type of the
//   contents.  The latter clearly represent integers.
//
// ***  Data Types Dependent on ITRON Specification ***
//
// In order to clarify the meanings of certain parameters, the following
// names are used for data types which appear frequently and/or have
// special meanings.

typedef cyg_int32  INT; // Signed integer (bit width of processor)
typedef cyg_uint32 UINT;   // Unsigned integer (bit width of processor)
typedef cyg_int32  BOOL;   // Boolean value.  TRUE (1) or FALSE (0).
typedef cyg_uint16 FN;     // Function code.  Signed integer.  Maximum 2 bytes.
typedef INT        ID;     // Object ID number (???id)
typedef INT        BOOL_ID;// Boolean value or ID number
typedef INT        HNO;    // Handler number
typedef INT        RNO;    // Rendezvous number
typedef INT        NODE;   // Node Number.  Usually a signed integer.
typedef UINT       ATR;    // Object or handler attribute.  An unsigned integer.
typedef INT        ER;     // Error code.  A signed integer.
typedef INT        PRI;    // Task priority.  A signed integer.
typedef UB         T_MSG;  // Message packet data structure used for mailboxes
typedef INT        TMO;    // Timeout value.  A signed integer.
                           // TMO_POL = 0 indicates polling,
                           // while TMO_FEVR = -1 indicates wait forever.

typedef cyg_uint64 CYGTM;
typedef CYGTM      SYSTIME;// Data types used for specifying times.
typedef CYGTM      CYCTIME;// Often split into upper and lower sections.
typedef CYGTM      ALMTIME;// For details, see the chapter giving system
typedef CYGTM      DLYTIME;// call descriptions;.

// ***********************************************************************
// ***    6.7 Error Codes                                              ***
// ***********************************************************************

enum {
//------------------------------------------------------
//Mnemonic Value      Description
//------------------------------------------------------
E_OK     = 0,      // Normal completion
// - - - - - - - - // - - - - - - - - - - - - - - - - - -
E_SYS    = (-5),   // System error
// - - - - - - - - // - - - - - - - - - - - - - - - - - -
E_NOMEM  = (-10),  // Insufficient memory
// - - - - - - - - // - - - - - - - - - - - - - - - - - -
E_NOSPT  = (-17),  // Feature not supported
E_INOSPT = (-18),  // Feature not supported by ITRON/FILE specification
E_RSFN   = (-20),  // Reserved function code number
E_RSATR  = (-24),  // Reserved attribute
// - - - - - - - - // - - - - - - - - - - - - - - - - - -
E_PAR    = (-33),  // Parameter error
E_ID     = (-35),  // Invalid ID number
// - - - - - - - - // - - - - - - - - - - - - - - - - - -
E_NOEXS  = (-52),  // Object does not exist
E_OBJ    = (-63),  // Invalid object state
// - - - - - - - - // - - - - - - - - - - - - - - - - - -
E_MACV   = (-65),  // Memory access disabled or memory access violation
E_OACV   = (-66),  // Object access violation
// - - - - - - - - // - - - - - - - - - - - - - - - - - -
E_CTX    = (-69),  // Context error
// - - - - - - - - // - - - - - - - - - - - - - - - - - -
E_QOVR   = (-73),  // Queuing or nesting overflow
// - - - - - - - - // - - - - - - - - - - - - - - - - - -
E_DLT    = (-81),  // Object being waited for was deleted
// - - - - - - - - // - - - - - - - - - - - - - - - - - -
E_TMOUT  = (-85),  // Polling failure or timeout exceeded
E_RLWAI  = (-86),  // WAIT state was forcibly released
// - - - - - - - - // - - - - - - - - - - - - - - - - - - -
#if 0 // CONNECTION FUNCTIONS ARE NOT SUPPORTED
EN_NOND  = (-113), // Target node does not exist or cannot be accessed
EN_OBJNO = (-114), // Specifies an object number which could not be
                   // accessed on the target node
EN_PROTO = (-115), // Protocol not supported on target node
EN_RSFN  = (-116), // System call or function not supported on target node
EN_COMM  = (-117), // No response from target node
EN_RLWAI = (-118), // Connection function response wait state was forcibly
                   // released
EN_PAR   = (-119), // A value outside the range supported by the target
                   // node and/or transmission packet format was specified
                   // as a parameter
EN_RPAR  = (-120), // A value outside the range supported by the issuing
                   // node and/or transmission packet format was returned
                   // as a return parameter
EN_CTXID = (-121), // An object on another node was specified to a system
                   // call issued from a task in dispatch disabled state
                   // or from a task-independent portion
EN_EXEC  = (-122), // System call could not be executed due to
                   // insufficient resources on the target node
EN_NOSPT = (-123), // Connection function not supported
#endif // 0 CONNECTION FUNCTIONS ARE NOT SUPPORTED
// - - - - - - - - // - - - - - - - - - - - - - - - - - - -
};


// *******************************************************************
// ***    6.6 Common Constants and Data Structure Packet Formats   ***
// *******************************************************************

/* --- overall ----------------------- */

/* invalid address or pointer value */
#define NADR      ((void *)(-1)) 

enum {
        TRUE  =   1,    /* true */
        FALSE =   0,    /* false */
};

/*    TMO tmout:   */
enum {
        TMO_POL  =    0,    /* polling */
        TMO_FEVR =  (-1)    /* wait forever */
};

/* --- for task management functions ----------------------- */

// cre_tsk:
        typedef struct t_ctsk {
                VP    exinf;     /* extended information */
                ATR   tskatr;    /* task attributes */
                FP    task;      /* task start address */
                PRI   itskpri;   /* initial task priority */
                INT   stksz;     /* stack size */
                // ...
            /* additional information may be included depending on the
               implementation */
                // ...
        } T_CTSK;

//    tskatr:
enum {
        TA_ASM    = 0x00,     /* program written in assembly language */
        TA_HLNG   = 0x01,     /* program written in high-level language */
        TA_COP0   = 0x8000,   /* uses coprocessor having ID = 0 */
        TA_COP1   = 0x4000,   /* uses coprocessor having ID = 1 */
        TA_COP2   = 0x2000,   /* uses coprocessor having ID = 2 */
        TA_COP3   = 0x1000,   /* uses coprocessor having ID = 3 */
        TA_COP4   = 0x0800,   /* uses coprocessor having ID = 4 */
        TA_COP5   = 0x0400,   /* uses coprocessor having ID = 5 */
        TA_COP6   = 0x0200,   /* uses coprocessor having ID = 6 */
        TA_COP7   = 0x0100,   /* uses coprocessor having ID = 7 */
};

//    tskid:
enum {
        TSK_SELF  = 0,  /* task specifies itself */
        /* FALSE     = 0, */ /* indicates a task-independent portion (return
                          parameters only) */
};
//    tskpri:
enum {
        TPRI_INI  = 0,  /* specifies the initial priority on task startup
                          (chg_pri) */
        TPRI_RUN  = 0,  /* specifies the highest priority during execution
                          (rot_rdq) */
};
    /* ref_tsk */
        typedef struct t_rtsk {
                VP     exinf;     /* extended information */
                PRI    tskpri;    /* current priority */
                UINT   tskstat;   /* task state */
            /* the following are represent extended features of support
               [level X] (implementation-dependent) */
#if 0 // NOT SUPPORTED
                UINT   tskwait;   /* cause of wait */
                ID     wid;       /* ID of object being waited for */
                INT    wupcnt;    /* wakeup request count */
                INT    suscnt;    /* SUSPEND request count */
                ATR    tskatr;    /* task attributes */
                FP     task;      /* task start address */
                PRI    itskpri;   /* initial task priority */
                INT    stksz;     /* stack size */
                        // ...
#endif
        } T_RTSK;

//    tskstat:
enum {
        TTS_RUN   = 0x01,  /* RUN */
        TTS_RDY   = 0x02,  /* READY */
        TTS_WAI   = 0x04,  /* WAIT */
        TTS_SUS   = 0x08,  /* SUSPEND */
        TTS_WAS   = 0x0C,  /* WAIT-SUSPEND */
        TTS_DMT   = 0x10,  /* DORMANT */
};
//    tskwait:
enum {
        TTW_SLP   = 0x0001,  /* wait due to slp_tsk or tslp_tsk */
        TTW_DLY   = 0x0002,  /* wait due to dly_tsk */
        TTW_NOD   = 0x0008,  /* connection function response wait */
        TTW_FLG   = 0x0010,  /* wait due to wai_flg or twai_flg */
        TTW_SEM   = 0x0020,  /* wait due to wai_sem or twai_sem */
        TTW_MBX   = 0x0040,  /* wait due to rcv_msg or trcv_msg */
        TTW_SMBF  = 0x0080,  /* wait due to snd_mbf or tsnd_mbf */
        TTW_MBF   = 0x0100,  /* wait due to rcv_mbf or trcv_mbf */
        TTW_CAL   = 0x0200,  /* wait for rendezvous call */
        TTW_ACP   = 0x0400,  /* wait for rendezvous accept */
        TTW_RDV   = 0x0800,  /* wait for rendezvous completion */
        TTW_MPL   = 0x1000,  /* wait due to get_blk or tget_blk */
        TTW_MPF   = 0x2000,  /* wait due to get_blf or tget_blf */
};
         /* Since the task states given by tskstat and tskwait are expressed
            by bit correspondences, they are convenient when looking for OR
            conditions (such as whether a task is in WAIT or READY state).
            */

/* --- for semaphore functions ----------------------- */

    /* cre_sem */
        typedef struct t_csem {
                VP    exinf;    /* extended information */
                ATR   sematr;   /* semaphore attributes */
            /* Following is the extended function for [level X]. */
                INT   isemcnt;   /* initial semaphore count */
            /*  INT   maxsem;  NOT SUPPORTED maximum semaphore count */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_CSEM;

    /* ref_sem */
        typedef struct t_rsem {
                VP      exinf;    /* extended information */
                BOOL_ID wtsk;     /* indicates whether or not there is a
                                     waiting task */
                INT     semcnt;   /* current semaphore count */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_RSEM;

/* --- for eventflag functions ----------------------- */

    /* cre_flg */
        typedef struct t_cflg {
                VP     exinf;     /* extended information */
                ATR    flgatr;    /* eventflag attribute */
                UINT   iflgptn;   /* initial eventflag */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_CFLG;

//    flgatr:
enum {
        TA_WSGL   = 0x00,  /* multiple tasks are not allowed to wait (Wait
                                Single Task) */
        TA_WMUL   = 0x08,  /* multiple tasks are allowed to wait (Wait
                                Multiple Task) */
};
//    wfmode:
enum {
        TWF_ANDW   = 0x00,  /* AND wait */
        TWF_ORW    = 0x02,  /* OR wait */
        TWF_CLR    = 0x01,  /* clear specification */
};
    /* ref_flg */
        typedef struct t_rflg {
                VP        exinf;      /* extended information */
                BOOL_ID   wtsk;       /* indicates whether or not there is a
                                         waiting task */
                UINT      flgptn;     /* eventflag bit pattern */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_RFLG;

/* --- for mailbox functions ----------------------- */

   /* cre_mbx */
        typedef struct t_cmbx {
                VP    exinf;    /* extended information */
                ATR   mbxatr;   /* mailbox attributes */
            /* Following is implementation-dependent function */
            /*  INT   bufcnt; NOT SUPPORTED ring buffer size IS FIXED */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_CMBX;

//    mbxatr:
enum {
        TA_TFIFO   = 0x00,  /* waiting tasks are handled by FIFO */
        TA_TPRI    = 0x01,  /* waiting tasks are handled by priority */
        TA_MFIFO   = 0x00,  /* messages are handled by FIFO */
        TA_MPRI    = 0x02,  /* messages are handled by priority */
};

    /* ref_mbx */
        typedef struct t_rmbx {
                VP        exinf;    /* extended information */
                BOOL_ID   wtsk;     /* indicates whether or not there is a
                                       waiting task */
                T_MSG*    pk_msg;   /* message to be sent next */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_RMBX;

/* --- for messagebuffer functions ----------------------- */

#if 0 // NOT SUPPORTED
    /* cre_mbf */
        typedef struct t_cmbf {
                VP    exinf;    /* extended information */
                ATR   mbfatr;   /* messagebuffer attributes */
                INT   bufsz;    /* messagebuffer size */
                INT   maxmsz;   /* maximum size of messages */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_CMBF;

//    mbfatr:
//    mbfid:
enum {
        TMBF_OS  = (-4),   /* messagebuffer used for OS error log */
        TMBF_DB  = (-3),   /* messagebuffer used for debugging */
};
    /* ref_mbf */
        typedef struct t_rmbf {
                VP        exinf;     /* extended information */
                BOOL_ID   wtsk;      /* indicates whether or not there is a
                                        task waiting to receive a message */
                BOOL_ID   stsk;      /* indicates whether or not there is a
                                        task waiting to send a message */
                INT       msgsz;     /* size of message to be sent next */
                INT       frbufsz;   /* size of free buffer */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_RMBF;

#endif
/* --- for port or rendezvous functions ----------------------- */

#if 0 // NOT SUPPORTED

    /* cre_por */
        typedef struct t_cpor {
                VP    exinf;     /* extended information */
                ATR   poratr;    /* port attributes */
                INT   maxcmsz;   /* maximum call message size */
                INT   maxrmsz;   /* maximum reply message size */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_CPOR;

//    poratr:
enum {
        TA_NULL  = 0,  /* specifies no particular attributes */
         /* TA_NULL should be used in place of zeroes to turn off all
            attribute features. */
};
    /* ref_por */
        typedef struct t_rpor {
                VP        exinf;   /* extended information */
                BOOL_ID   wtsk;    /* indicates whether or not there is a task
                                      waiting to call a rendezvous */
                BOOL_ID   atsk;    /* indicates whether or not there is a task
                                      waiting to accept a rendezvous */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_RPOR;
#endif

/* --- for interrupt management functions ----------------------- */

#if 0 // NOT SUPPORTED
    /* def_int */
        typedef struct t_dint {
                ATR   intatr;   /* interrupt handler attributes */
                FP    inthdr;   /* interrupt handler address */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_DINT;
#endif

/* --- for memorypool management functions ----------------------- */

    /* cre_mpl */
        typedef struct t_cmpl {
                VP    exinf;    /* extended information */
                ATR   mplatr;   /* memorypool attributes */
                INT   mplsz;    /* memorypool size */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_CMPL;

//    mplatr:
//    mplid:
enum {
        TMPL_OS  = (-4)   /* memorypool used by OS */
};
    /* ref_mpl */
        typedef struct t_rmpl {
                VP        exinf;    /* extended information */
                BOOL_ID   wtsk;     /* indicates whether or not there are
                                       waiting tasks */
                INT       frsz;     /* total size of free memory */
                INT       maxsz;    /* size of largest contiguous memory */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_RMPL;

    /* cre_mpf */
        typedef struct t_cmpf {
                VP    exinf;     /* extended information */
                ATR   mpfatr;    /* memorypool attributes */
                INT   mpfcnt;    /* block count for entire memorypool */
                INT   blfsz;     /* fixed-size memory block size */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_CMPF;

//    mpfatr:
    /* ref_mpf */
        typedef struct t_rmpf {
                VP        exinf;    /* extended information */
                BOOL_ID   wtsk;     /* indicates whether or not there are
                                       waiting tasks */
                INT       frbcnt;   /* free block count */
            /* additional information may be included depending on the
               implementation */
                INT       numbcnt;  /* total number of blocks */
                INT       bsize;    /* block size */

        } T_RMPF;

/* --- for time management functions ----------------------- */

#if 0 // native definition is at head of this file
    /* example for 32-bit CPUs */
        typedef struct t_systime {
                H    utime;   /* upper 16 bits */
                UW   ltime;   /* lower 32 bits */
        } SYSTIME, CYCTIME, ALMTIME;

    /* example for 16-bit CPUs */
        typedef struct t_systime {
                H    utime;   /* upper 16 bits */
                UH   mtime;   /* middle 16 bits */
                UH   ltime;   /* lower 16 bits */
        } SYSTIME, CYCTIME, ALMTIME;
#endif
         /* Member configuration depends on the bit width of the processor and
            on the implementation.  A total of 48 bits is recommended. */

    /* def_cyc */
        typedef struct t_dcyc {
                VP        exinf;    /* extended information */
                ATR       cycatr;   /* cyclic handler attributes */
                FP        cychdr;   /* cyclic handler address */
                UINT      cycact;   /* cyclic handler activation */
                CYCTIME   cyctim;   /* cyclic startup period */
        } T_DCYC;

//    cycact:
enum {
        TCY_OFF   = 0x00,  /* do not invoke cyclic handler */
        TCY_ON    = 0x01,  /* invoke cyclic handler */
        TCY_INT   = 0x02,  /* initialize cycle count */
        /* Following changed from TCY_INT to TCY_INI to match
           description in the body of the standard.  I assume TCY_INT
           is a hypercorrection/typo; keep both */
        TCY_INI   = 0x02,  /* initialize cycle count */
};
    /* ref_cyc */
        typedef struct t_rcyc {
                VP        exinf;    /* extended information */
                CYCTIME   lfttim;   /* time left before next handler startup */
                UINT      cycact;   /* cyclic handler activation */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_RCYC;

    /* def_alm */
        typedef struct t_dalm {
                VP        exinf;    /* extended information */
                ATR       almatr;   /* alarm handler attributes */
                FP        almhdr;   /* alarm handler address */
                UINT      tmmode;   /* start time specification mode */
                ALMTIME   almtim;   /* handler startup time */
        } T_DALM;

//    tmmode:
enum {
        TTM_ABS   = 0x00,  /* specified as an absolute time */
        TTM_REL   = 0x01,  /* specified as a relative time */
};
    /* ref_alm */
        typedef struct t_ralm {
                VP        exinf;    /* extended information */
                ALMTIME   lfttim;   /* time left before next handler startup */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_RALM;

/* --- for system management functions ----------------------- */

    /* get_ver */
        typedef struct t_ver {
                UH   maker;     /* vendor */
                UH   id;        /* format number */
                UH   spver;     /* specification version */
                UH   prver;     /* product version */
                UH   prno[4];   /* product control information */
                UH   cpu;       /* CPU information */
                UH   var;       /* variation descriptor */
        } T_VER;

    /* ref_sys */
        typedef struct t_rsys {
                INT   sysstat;   /* system state */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_RSYS;

//    sysstat:
enum {
        TSS_TSK   = 0,  /* normal state in which dispatching is enabled during
                          task portion execution */
        TSS_DDSP  = 1,   /* state after dis_dsp has been executed during task
                          portion execution (dispatch disabled) */
        TSS_LOC   = 3,   /* state after loc_cpu has been executed during task
                          portion execution (interrupt and dispatch disabled)
                          */
        TSS_INDP  = 4,   /* state during execution of task-independent portions
                          (interrupt and timer handlers) */
};
    /* ref_cfg */
        typedef struct t_rcfg {
            /* details concerning members are implementation dependent */
        } T_RCFG;

#if 0 // NOT SUPPORTED
    /* def_svc */
        typedef struct t_dsvc {
                ATR   svcatr;   /* extended SVC handler attributes */
                FP    svchdr;   /* extended SVC handler address */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_DSVC;

    /* def_exc */
        typedef struct t_dexc {
                ATR   excatr;   /* exception handler attributes */
                FP    exchdr;   /* exception handler address */
                        // ...
            /* additional information may be included depending on the
               implementation */
                        // ...
        } T_DEXC;
#endif

/* --- for network management functions ----------------------- */

#if 0 // NOT SUPPORTED
//    NODE srcnode, dstnode, node:
enum {
        TND_SELF  = 0,     /* specifies the local node */
        TND_OTHR  = (-1)   /* specifies default remote node */
};
#endif
/* ------------------------------------------------------ */



#endif // CYGPKG_UITRON

#endif // CYGONCE_COMPAT_UITRON_UIT_TYPE_H
// EOF uit_type.h
