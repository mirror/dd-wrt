/* ar531Xplus.h - ar531xPlus reference board header */

/* Copyright 1984-1999 Wind River Systems, Inc. */

/*
 *  Copyright © 2001-2004 Atheros Communications, Inc.,  All Rights Reserved.
 */

/*
 * This file contains I/O addresses and related constants for the
 * ar531XPlus BSP.
 */

#ifndef __ar531Xplus_h
#define __ar531Xplus_h

#ident "ACI $Header: //depot/sw/branches/mBSSID_dev/src/ap/os/vxworks/target/config/ar531xPlus/ar531xPlus.h#1 $"

#define BUS             NONE            /* no off-board bus interface */
#define N_SIO_CHANNELS  2               /* 1 on ar531Xplus */

/* timer constants */
#define SYS_CLK_RATE_MIN        30      /* minimum system clock rate */
#define SYS_CLK_RATE_MAX        1000    /* maximum system clock rate */
#define AUX_CLK_RATE_MIN        30      /* minimum auxiliary clock rate */
#define AUX_CLK_RATE_MAX        5000    /* maximum auxiliary clock rate */

#if !defined(SYS_CLK_RATE)
#define SYS_CLK_RATE 60
#endif

/* memory configuration */
#define LOCAL_MEM_LOCAL_ADRS    K0BASE  /* K0, but DMA ignores upper bits */

/*
 * Miscellaneous definitions go here. For example, macro definitions
 * for various devices.
 */

#undef  INCLUDE_EGL             /* configAll.h defines this for MIPS */

/* Directly connected MIPS interrupts -- usually starts @ 33 + 1 */
#define IV_MISCIO_VEC           (IV_SWTRAP1_VEC + 1)
#define IV_WLAN0_VEC            (IV_SWTRAP1_VEC + 2)
#define IV_ENET0_VEC            (IV_SWTRAP1_VEC + 3)
#define IV_LOCALPCI_VEC         (IV_SWTRAP1_VEC + 4)
#define IV_WMACPOLL_VEC         (IV_SWTRAP1_VEC + 5)
#define IV_COMPARE_VEC          (IV_SWTRAP1_VEC + 6)

/* Misc IO vectored interrupts */
#define IV_UART0_VEC            (IV_SWTRAP1_VEC + 7)
#define IV_I2C_RSVD_VEC         (IV_SWTRAP1_VEC + 8)
#define IV_SPI_VEC              (IV_SWTRAP1_VEC + 9)
#define IV_AHB_VEC              (IV_SWTRAP1_VEC + 10)
#define IV_APB_VEC              (IV_SWTRAP1_VEC + 11)   
#define IV_TIMER_VEC            (IV_SWTRAP1_VEC + 12)
#define IV_GPIO_VEC             (IV_SWTRAP1_VEC + 13)
#define IV_WD_VEC               (IV_SWTRAP1_VEC + 14)   
#define IV_IR_RSVD_VEC          (IV_SWTRAP1_VEC + 15)   
#define IV_PCI_SLOT0            (IV_SWTRAP1_VEC + 16)   
#define IV_PCI_SLOT1            (IV_SWTRAP1_VEC + 17)   
#define IV_PCI_MASTER_ABORT     (IV_SWTRAP1_VEC + 18)   


#define IV_PCI_SLOT             (IV_PCI_SLOT0)   

#define AR531XPLUS_SR           (SR_CU0 | SR_IE |      \
                                 AR531XPLUS_INTR_MISCIO | \
                                 AR531XPLUS_INTR_COMPARE)
/* "NVRAM" in flash. */
#if AR5315
#define NVRAM_INITIALIZED       "AR531XPLUS"
#endif
#define NVRAM_INIT_SIZE         (sizeof(NVRAM_INITIALIZED)-1)
#define NVRAM_BOOT_OFFSET       NVRAM_INIT_SIZE
#define NVRAM_SIZE              0x100           /* limited my memory map */

#undef INCLUDE_VME              /* no VME bus */
#undef INCLUDE_MMU_BASIC        /* no MMU support */
#undef INCLUDE_MMU_FULL         /* no MMU support */
#undef INCLUDE_MMU              /* no MMU support */
#undef INCLUDE_HW_FP            /* no FPU */

#undef INCLUDE_SCSI             /* no SCSI or related disk support */
#undef INCLUDE_SCSI_DMA
#undef INCLUDE_SCSI_BOOT
#undef INCLUDE_DOSFS
#undef INCLUDE_TAPEFS
#undef INCLUDE_CDROMFS
#undef SYS_SCSI_CONFIG
#undef INCLUDE_SCSI2

/* PCI configuration & Configuration space access macros */
#define INCLUDE_PCI
#define INCLUDE_PCI_AUTOCONF
#define PCI_MAX_DEV 6

#define INCLUDE_SYS_HW_INIT_0
#define SYS_HW_INIT_0() sysHwInit0()

#ifdef _ASMLANGUAGE

/*
 * Assembly helpers
 */
#define LEAF(name)      \
        .globl name;    \
        .ent name;      \
name:

#define XLEAF(name)     \
        .globl name;    \
name:

#define END(name)       \
        .end    name

#define ASM_PUTS(str)                   \
        .data;                          \
99:;                                    \
        .asciz str;                     \
        .text;                          \
        la      a0, 99b;                \
        jal     asm_puts

#define ROM_PUTS(str)                   \
        b       99f;                    \
98:;                                    \
        .asciz str;                     \
        .align 2;                       \
99:;                                    \
        la      a0, 98b;                \
        sub     a0, RAM_LOW_ADRS;       \
        or      a0, 0xbfc00000;         \
        bal     rom_puts

#else /* _ASMLANGUAGE */


#ifndef PCI_INTERFACE
/*
 * Reset Codes used by halSetResetReg to determine
 * which component(s) to reset.
 */
#define MAC_RC_PCI   0x00  /* For backwards compatibility */
#define MAC_RC_BB    0x01  /* Reset BaseBand */
#define MAC_RC_MAC   0x02  /* Reset Media Access Controller */
#endif

/*
 * BSP specific prototypes
 */
unsigned int sysConfig1Get(void);
void sysWbFlush(void);

/* spirit performance counter helpers */
int  sysPerfSupported(void);            /* performance counters supported? */
void sysPerfStart(int);                 /* start counters with mode */
#define SYSPERF_DC      0               /* watch data cache */
#define SYSPERF_IC      1               /* watch instruction cache */
#define SYSPERF_WB      2               /* watch write buffer */
#define SYSPERF_CH      3               /* watch cache hits */
#define SYSPERF_CM      4               /* watch cache misses */
void sysPerfUpdate(void);               /* sample + accumulate counters */
void sysPerfShow(void);                 /* display results */
void sysPerfStop(void);                 /* stop counters */

void asm_puts(char *);                  /* low level console output */
void asm_puthex(UINT32);

void sysReset(void);                    /* hardware reset of the system */
void sysHwInit0(void);                  /* very early init C code */

void sysUDelay(int);                    /* micro second delay */
void sysMsDelay(int);                   /* milli second delay */
void sysDelay(void);                    /* delay to allow PIO to finish */

void sysToFactory(void);                /* set-up for factory defaults */

STATUS sysLanEnetAddrGet(int, char *);  /* get network MAC address */

int sysTffsMount(void);                 /* mount flash filesystem */

void sysGpioCtrlOutput(int gpio);       /* set gpio for output */
void sysGpioCtrlInput(int gpio);        /* set gpio for input */
void sysGpioCtrlIntr(int gpio, int intnum, int intlevel); 
                                        /* set gpio for interrupt */
void sysGpioConnect(int gpio, FUNCPTR cb, UINT32 arg,  int intnum, int intlevel);
void sysGpioSet(int gpio, int val);     /* set output gpio to val */
int  sysGpioGet(int gpio);              /* get input gpio val */

void cacheDataInvalidate(void *p, int len);
void cacheDescInvalidate(void *p);

void sysLedBlink(void);
void sysLed(int on);

/* PCI funtions */
#ifdef PCI_INTERFACE 
void sysPciIntrAck(void);
void sysPciAbortAck(void);
#endif

/* hardware watchdog support */
#define SYSWATCHDOG
void sysWatchdogPeriodSet(int ticks);
void sysWatchdogUpdate(void);
void sysWatchdogEnable(void);
void sysWatchdogDisable(void);
void sysWatchdogExtendTime(void);

/* configuration in flash support */
void sysFlashConfigInit(void *ptr, INT32, INT32, INT32);
void sysFlashConfigErase(int fcl);
UINT8 sysFlashConfigRead(int fcl , int offset);
void sysFlashConfigWrite(int fcl, int offset, UINT8 *data, int len);
int  sysRomSize(void);
#define NFLC            3               /* number of flash config segments */
#define FLC_BOOTLINE    0
#define FLC_BOARDDATA   1
#define FLC_RADIOCFG    2

/* lost from dosFsLib.h in Tornado 2.1 */
STATUS dosFsMkfsOptionsSet(UINT options);

#define DELTA(a,b)      (((UINT32)(a)) - ((UINT32)(b)))

/*
 * Board configuration data to allow software to determine
 * configuration at run time.  Stored in a read only flash
 * configuration block.
 */
struct ar531xPlus_boarddata {
    UINT32 magic;                       /* board data is valid */
#define BD_MAGIC        0x35333131      /* "5311", for all 531x platforms */
    UINT16 cksum;                       /* checksum (starting with BD_REV 2) */
    UINT16 rev;                         /* revision of this struct */
#define BD_REV  5
    char   boardName[64];               /* Name of board */
    UINT16 major;                       /* Board major number */
    UINT16 minor;                       /* Board minor number */
    UINT32 config;                      /* Board configuration */
#define BD_ENET0        0x00000001      /* ENET0 is stuffed */
#define BD_ENET1        0x00000002      /* ENET1 is stuffed */
#define BD_UART1        0x00000004      /* UART1 is stuffed */
#define BD_UART0        0x00000008      /* UART0 is stuffed (dma) */
#define BD_RSTFACTORY   0x00000010      /* Reset factory defaults stuffed */
#define BD_SYSLED       0x00000020      /* System LED stuffed */
#define BD_EXTUARTCLK   0x00000040      /* External UART clock */
#define BD_CPUFREQ      0x00000080      /* cpu freq is valid in nvram */
#define BD_SYSFREQ      0x00000100      /* sys freq is set in nvram */
#define BD_WLAN0        0x00000200      /* use WLAN0 */
#define BD_MEMCAP       0x00000400      /* CAP SDRAM @ memCap for testing */
#define BD_DISWATCHDOG  0x00000800      /* disable system watchdog */
#define BD_WLAN1        0x00001000      /* Enable WLAN1 (ar5212) */
#define BD_ISCASPER     0x00002000      /* FLAG for low cost ar5212 */
#define BD_WLAN0_2G_EN  0x00004000      /* FLAG for radio0_2G */
#define BD_WLAN0_5G_EN  0x00008000      /* FLAG for radio0_2G */
#define BD_WLAN1_2G_EN  0x00020000      /* FLAG for radio0_2G */
#define BD_WLAN1_5G_EN  0x00040000      /* FLAG for radio0_2G */
#define BD_LOCALBUS     0x00080000      /* Enable Local Bus */
#define BD_I2CBUS       0x00100000      /* Enable I2C bus */
#define BD_PCIBUS       0x00200000      /* Enable PCI Bus */
    UINT16 resetConfigGpio;             /* Reset factory GPIO pin */
    UINT16 sysLedGpio;                  /* System LED GPIO pin */

    UINT32 cpuFreq;                     /* CPU core frequency in Hz */
    UINT32 sysFreq;                     /* System frequency in Hz */
    UINT32 cntFreq;                     /* Calculated C0_COUNT frequency */

    UINT8  wlan0Mac[6];
    UINT8  enet0Mac[6];
    UINT8  enet1Mac[6];

    UINT16 pciId;                       /* Pseudo PCIID for common code */
    UINT16 memCap;                      /* cap bank1 in MB */

#define SIZEOF_BD_REV2  120             /* padded by 2 bytes */

    /* version 3 */
    UINT8  wlan1Mac[6];                 /* (ar5212) */
    UINT8  lbMac[6];
    UINT8  pciMac[6];
};
extern struct ar531xPlus_boarddata sysBoardData;

int bdChange(void), bdShow(struct ar531xPlus_boarddata *);
UINT16 sysBoardDataChecksum(struct ar531xPlus_boarddata *bd);       /* for MDK */

#ifndef VXWORKS55
#ifdef INCLUDE_TELNET
/* need a place to insert prototypes for prjConfig.c */
STATUS cliTelnetHook(int, UINT32, int *, int *);
#endif
#endif /* VXWORKS55 */

int loattach();                         /* vxWorks missing prototype */

#endif /* _ASMLANGUAGE */

#endif  /* __AR531XPLUS_H */
