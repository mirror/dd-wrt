/*
<:copyright-gpl 
 Copyright 2002 Broadcom Corp. All Rights Reserved. 
 
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
/***************************************************************************
 * File Name  : board.c
 *
 * Description: This file contains Linux character device driver entry 
 *              for the board related ioctl calls: flash, get free kernel
 *              page and dump kernel memory, etc.
 *
 * Created on :  2/20/2002  seanl:  use cfiflash.c, cfliflash.h (AMD specific)
 *
 ***************************************************************************/


/* Includes. */
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/capability.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/pagemap.h>
#include <asm/uaccess.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/if.h>

#include <bcm_map_part.h>
#include <board.h>
#include <bcmTag.h>
#include "boardparms.h"
#include "cfiflash.h"
#include "bcm_intr.h"
#include "board.h"
#include "bcm_map_part.h"

/* Typedefs. */
#if defined (NON_CONSECUTIVE_MAC)
// used to be the last octet. Now changed to the first 5 bits of the the forth octet
// to reduced the duplicated MAC addresses.
#define CHANGED_OCTET   3
#define SHIFT_BITS      3
#else
#define CHANGED_OCTET   1
#define SHIFT_BITS      0
#endif

#if defined (WIRELESS)
#define SES_BTN_PRESSED 0x00000001
#define SES_EVENTS      SES_BTN_PRESSED /*OR all values if any*/
#define SES_LED_OFF     0
#define SES_LED_ON      1
#define SES_LED_BLINK   2
#endif

typedef struct
{
    unsigned long ulId;
    char chInUse;
    char chReserved[3];
} MAC_ADDR_INFO, *PMAC_ADDR_INFO;

typedef struct
{
    unsigned long ulSdramSize;
    unsigned long ulPsiSize;
    unsigned long ulNumMacAddrs;
    unsigned long ucaBaseMacAddr[NVRAM_MAC_ADDRESS_LEN];
    MAC_ADDR_INFO MacAddrs[1];
} NVRAM_INFO, *PNVRAM_INFO;

typedef struct
{
    unsigned long eventmask;    
} BOARD_IOC, *PBOARD_IOC;


/*Dyinggasp callback*/
typedef void (*cb_dgasp_t)(void *arg);
typedef struct _CB_DGASP__LIST
{
    struct list_head list;
    char name[IFNAMSIZ];
    cb_dgasp_t cb_dgasp_fn;
    void *context;
}CB_DGASP_LIST , *PCB_DGASP_LIST;


static LED_MAP_PAIR LedMapping[] =
{   // led name     Initial state       physical pin (ledMask)
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0}, 
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0}, 
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0},     
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0} // NOTE: kLedEnd has to be at the end.
};

/* Externs. */
extern struct file fastcall *fget_light(unsigned int fd, int *fput_needed);
extern unsigned int nr_free_pages (void);
extern const char *get_system_type(void);
extern void kerSysFlashInit(void);
extern unsigned long get_nvram_start_addr(void);
extern unsigned long get_scratch_pad_start_addr(void);
extern unsigned long getMemorySize(void);
extern void __init boardLedInit(PLED_MAP_PAIR);
extern void boardLedCtrl(BOARD_LED_NAME, BOARD_LED_STATE);
extern void kerSysLedRegisterHandler( BOARD_LED_NAME ledName,
    HANDLE_LED_FUNC ledHwFunc, int ledFailType );

/* Prototypes. */
void __init InitNvramInfo( void );
static int board_open( struct inode *inode, struct file *filp );
static int board_ioctl( struct inode *inode, struct file *flip, unsigned int command, unsigned long arg );
static ssize_t board_read(struct file *filp,  char __user *buffer, size_t count, loff_t *ppos); 
static unsigned int board_poll(struct file *filp, struct poll_table_struct *wait);
static int board_release(struct inode *inode, struct file *filp);                        

static BOARD_IOC* borad_ioc_alloc(void);
static void borad_ioc_free(BOARD_IOC* board_ioc);

/* DyingGasp function prototype */
static void __init kerSysDyingGaspMapIntr(void);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
static irqreturn_t kerSysDyingGaspIsr(int irq, void * dev_id, struct pt_regs * regs);
#else
static unsigned int kerSysDyingGaspIsr(void);
#endif
static void __init kerSysInitDyingGaspHandler( void );
static void __exit kerSysDeinitDyingGaspHandler( void );
/* -DyingGasp function prototype - */


#if defined (WIRELESS)
static irqreturn_t sesBtn_isr(int irq, void *dev_id, struct pt_regs *ptregs);
static void __init sesBtn_mapGpio(void);
static void __init sesBtn_mapIntr(int context);
static unsigned int sesBtn_poll(struct file *file, struct poll_table_struct *wait);
static ssize_t sesBtn_read(struct file *file,  char __user *buffer, size_t count, loff_t *ppos);
static void __init sesLed_mapGpio(void);
static void sesLed_ctrl(int action);
static void __init ses_board_init(void);
static void __exit ses_board_deinit(void);
#endif

static PNVRAM_INFO g_pNvramInfo = NULL;
static int g_ledInitialized = 0;
static wait_queue_head_t g_board_wait_queue;
static CB_DGASP_LIST *g_cb_dgasp_list_head = NULL;

static int g_wakeup_monitor = 0;
static struct file *g_monitor_file = NULL;
static struct task_struct *g_monitor_task = NULL;
static unsigned int (*g_orig_fop_poll)
    (struct file *, struct poll_table_struct *) = NULL;

static struct file_operations board_fops =
{
  open:       board_open,
  ioctl:      board_ioctl,
  poll:       board_poll,
  read:       board_read,
  release:    board_release,
};

uint32 board_major = 0;

#if defined (WIRELESS)
static unsigned short sesBtn_irq = BP_NOT_DEFINED;
static unsigned short sesBtn_gpio = BP_NOT_DEFINED;
static unsigned short sesLed_gpio = BP_NOT_DEFINED;
#endif

#if defined(MODULE)
int init_module(void)
{
    return( brcm_board_init() );              
}

void cleanup_module(void)
{
    if (MOD_IN_USE)
        printk("brcm flash: cleanup_module failed because module is in use\n");
    else
        brcm_board_cleanup();
}
#endif //MODULE 



static int __init brcm_board_init( void )
{
    typedef int (*BP_LED_FUNC) (unsigned short *);
    static struct BpLedInformation
    {
        BOARD_LED_NAME ledName;
        BP_LED_FUNC bpFunc;
        BP_LED_FUNC bpFuncFail;
    } bpLedInfo[] =
    {{kLedAdsl, BpGetAdslLedGpio, BpGetAdslFailLedGpio},
     {kLedWireless, BpGetWirelessLedGpio, NULL},
     {kLedUsb, BpGetUsbLedGpio, NULL},
     {kLedHpna, BpGetHpnaLedGpio, NULL},
     {kLedWanData, BpGetWanDataLedGpio, NULL},
     {kLedPPP, BpGetPppLedGpio, BpGetPppFailLedGpio},
     {kLedVoip, BpGetVoipLedGpio, NULL},
     {kLedSes, BpGetWirelessSesLedGpio, NULL},     
     {kLedEnd, NULL, NULL}
    };

    int ret;
        
    ret = register_chrdev(BOARD_DRV_MAJOR, "bcrmboard", &board_fops );
    if (ret < 0)
        printk( "brcm_board_init(major %d): fail to register device.\n",BOARD_DRV_MAJOR);
    else 
    {
        PLED_MAP_PAIR pLedMap = LedMapping;
        unsigned short gpio;
        struct BpLedInformation *pInfo;

        printk("brcmboard: brcm_board_init entry\n");
        board_major = BOARD_DRV_MAJOR;
        InitNvramInfo();

        for( pInfo = bpLedInfo; pInfo->ledName != kLedEnd; pInfo++ )
        {
            if( pInfo->bpFunc && (*pInfo->bpFunc) (&gpio) == BP_SUCCESS )
            {
                pLedMap->ledName = pInfo->ledName;
                pLedMap->ledMask = GPIO_NUM_TO_MASK(gpio);
                pLedMap->ledActiveLow = (gpio & BP_ACTIVE_LOW) ? 1 : 0;
            }
            if( pInfo->bpFuncFail && (*pInfo->bpFuncFail) (&gpio) == BP_SUCCESS )
            {
                pLedMap->ledName = pInfo->ledName;
                pLedMap->ledMaskFail = GPIO_NUM_TO_MASK(gpio);
                pLedMap->ledActiveLowFail = (gpio & BP_ACTIVE_LOW) ? 1 : 0;
            }
            if( pLedMap->ledName != kLedEnd )
                pLedMap++;
        }
        
        init_waitqueue_head(&g_board_wait_queue);
#if defined (WIRELESS)
        ses_board_init();
#endif        
        kerSysInitDyingGaspHandler();
        kerSysDyingGaspMapIntr();

        boardLedInit(LedMapping);
        g_ledInitialized = 1;
    }

    return ret;
} 

void __init InitNvramInfo( void )
{
    PNVRAM_DATA pNvramData = (PNVRAM_DATA) get_nvram_start_addr();
    unsigned long ulNumMacAddrs = pNvramData->ulNumMacAddrs;

    if( ulNumMacAddrs > 0 && ulNumMacAddrs <= NVRAM_MAC_COUNT_MAX )
    {
        unsigned long ulNvramInfoSize =
            sizeof(NVRAM_INFO) + ((sizeof(MAC_ADDR_INFO) - 1) * ulNumMacAddrs);

        g_pNvramInfo = (PNVRAM_INFO) kmalloc( ulNvramInfoSize, GFP_KERNEL );

        if( g_pNvramInfo )
        {
            unsigned long ulPsiSize;
            if( BpGetPsiSize( &ulPsiSize ) != BP_SUCCESS )
                ulPsiSize = NVRAM_PSI_DEFAULT;
            memset( g_pNvramInfo, 0x00, ulNvramInfoSize );
            g_pNvramInfo->ulPsiSize = ulPsiSize * 1024;
            g_pNvramInfo->ulNumMacAddrs = pNvramData->ulNumMacAddrs;
            memcpy( g_pNvramInfo->ucaBaseMacAddr, pNvramData->ucaBaseMacAddr,
                NVRAM_MAC_ADDRESS_LEN );
            g_pNvramInfo->ulSdramSize = getMemorySize();
        }
        else
            printk("ERROR - Could not allocate memory for NVRAM data\n");
    }
    else
        printk("ERROR - Invalid number of MAC addresses (%ld) is configured.\n",
            ulNumMacAddrs);
}

void __exit brcm_board_cleanup( void )
{
    printk("brcm_board_cleanup()\n");
	
    if (board_major != -1) 
    {
#if defined (WIRELESS)    	
    	ses_board_deinit();
#endif    	
        kerSysDeinitDyingGaspHandler();
        unregister_chrdev(board_major, "board_ioctl");
    }
} 

static BOARD_IOC* borad_ioc_alloc(void)
{
    BOARD_IOC *board_ioc =NULL;
    board_ioc = (BOARD_IOC*) kmalloc( sizeof(BOARD_IOC) , GFP_KERNEL );
    if(board_ioc)
    {
        memset(board_ioc, 0, sizeof(BOARD_IOC));
    }
    return board_ioc;
}

static void borad_ioc_free(BOARD_IOC* board_ioc)
{
    if(board_ioc)
    {
        kfree(board_ioc);
    }	
}


static int board_open( struct inode *inode, struct file *filp )
{
    filp->private_data = borad_ioc_alloc();

    if (filp->private_data == NULL)
        return -ENOMEM;
            
    return( 0 );
} 

static int board_release(struct inode *inode, struct file *filp)
{
    BOARD_IOC *board_ioc = filp->private_data;
    
    wait_event_interruptible(g_board_wait_queue, 1);    
    borad_ioc_free(board_ioc);

    return( 0 );
} 


static unsigned int board_poll(struct file *filp, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
#if defined (WIRELESS)        	
    BOARD_IOC *board_ioc = filp->private_data;    	
#endif
    	
    poll_wait(filp, &g_board_wait_queue, wait);
#if defined (WIRELESS)        	
    if(board_ioc->eventmask & SES_EVENTS){
        mask |= sesBtn_poll(filp, wait);
    }			
#endif    

    return mask;
}


static ssize_t board_read(struct file *filp,  char __user *buffer, size_t count, loff_t *ppos)
{
#if defined (WIRELESS)    
    BOARD_IOC *board_ioc = filp->private_data;
    if(board_ioc->eventmask & SES_EVENTS){
    	return sesBtn_read(filp, buffer, count, ppos);
    }
#endif    
    return 0;
}

//**************************************************************************************
// Utitlities for dump memory, free kernel pages, mips soft reset, etc.
//**************************************************************************************

/***********************************************************************
 * Function Name: dumpaddr
 * Description  : Display a hex dump of the specified address.
 ***********************************************************************/
void dumpaddr( unsigned char *pAddr, int nLen )
{
    static char szHexChars[] = "0123456789abcdef";
    char szLine[80];
    char *p = szLine;
    unsigned char ch, *q;
    int i, j;
    unsigned long ul;

    while( nLen > 0 )
    {
        sprintf( szLine, "%8.8lx: ", (unsigned long) pAddr );
        p = szLine + strlen(szLine);

        for(i = 0; i < 16 && nLen > 0; i += sizeof(long), nLen -= sizeof(long))
        {
            ul = *(unsigned long *) &pAddr[i];
            q = (unsigned char *) &ul;
            for( j = 0; j < sizeof(long); j++ )
            {
                *p++ = szHexChars[q[j] >> 4];
                *p++ = szHexChars[q[j] & 0x0f];
                *p++ = ' ';
            }
        }

        for( j = 0; j < 16 - i; j++ )
            *p++ = ' ', *p++ = ' ', *p++ = ' ';

        *p++ = ' ', *p++ = ' ', *p++ = ' ';

        for( j = 0; j < i; j++ )
        {
            ch = pAddr[j];
            *p++ = (ch > ' ' && ch < '~') ? ch : '.';
        }

        *p++ = '\0';
        printk( "%s\r\n", szLine );

        pAddr += i;
    }
    printk( "\r\n" );
} /* dumpaddr */


void kerSysMipsSoftReset(void)
{
#if defined(CONFIG_BCM96348)
    if (PERF->RevID == 0x634800A1) {
        typedef void (*FNPTR) (void);
        FNPTR bootaddr = (FNPTR) FLASH_BASE;
        int i;

        /* Disable interrupts. */
	local_irq_disable();

        /* Reset all blocks. */
        PERF->BlockSoftReset &= ~BSR_ALL_BLOCKS;
        for( i = 0; i < 1000000; i++ )
            ;
        PERF->BlockSoftReset |= BSR_ALL_BLOCKS;
        /* Jump to the power on address. */
        (*bootaddr) ();
    }
    else
        PERF->pll_control |= SOFT_RESET;    // soft reset mips
#else
    PERF->pll_control |= SOFT_RESET;    // soft reset mips
#endif
}


int kerSysGetMacAddress( unsigned char *pucaMacAddr, unsigned long ulId )
{
    int nRet = 0;
    PMAC_ADDR_INFO pMai = NULL;
    PMAC_ADDR_INFO pMaiFreeNoId = NULL;
    PMAC_ADDR_INFO pMaiFreeId = NULL;
    unsigned long i = 0, ulIdxNoId = 0, ulIdxId = 0, shiftedIdx = 0;

    for( i = 0, pMai = g_pNvramInfo->MacAddrs; i < g_pNvramInfo->ulNumMacAddrs;
        i++, pMai++ )
    {
        if( ulId == pMai->ulId || ulId == MAC_ADDRESS_ANY )
        {
            /* This MAC address has been used by the caller in the past. */
            memcpy( pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,
                NVRAM_MAC_ADDRESS_LEN );
            shiftedIdx = i;
            pucaMacAddr[NVRAM_MAC_ADDRESS_LEN - CHANGED_OCTET] += (shiftedIdx << SHIFT_BITS);
            pMai->chInUse = 1;
            pMaiFreeNoId = pMaiFreeId = NULL;
            break;
        }
        else
            if( pMai->chInUse == 0 )
            {
                if( pMai->ulId == 0 && pMaiFreeNoId == NULL )
                {
                    /* This is an available MAC address that has never been
                     * used.
                     */
                    pMaiFreeNoId = pMai;
                    ulIdxNoId = i;
                }
                else
                    if( pMai->ulId != 0 && pMaiFreeId == NULL )
                    {
                        /* This is an available MAC address that has been used
                         * before.  Use addresses that have never been used
                         * first, before using this one.
                         */
                        pMaiFreeId = pMai;
                        ulIdxId = i;
                    }
            }
    }

    if( pMaiFreeNoId || pMaiFreeId )
    {
        /* An available MAC address was found. */
        memcpy(pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,NVRAM_MAC_ADDRESS_LEN);
        if( pMaiFreeNoId )
        {
            shiftedIdx = ulIdxNoId;
            pucaMacAddr[NVRAM_MAC_ADDRESS_LEN - CHANGED_OCTET] += (shiftedIdx << SHIFT_BITS);
            pMaiFreeNoId->ulId = ulId;
            pMaiFreeNoId->chInUse = 1;
        }
        else
        {
            shiftedIdx = ulIdxId;
            pucaMacAddr[NVRAM_MAC_ADDRESS_LEN - CHANGED_OCTET] += (shiftedIdx << SHIFT_BITS);
            pMaiFreeId->ulId = ulId;
            pMaiFreeId->chInUse = 1;
        }
    }
    else
        if( i == g_pNvramInfo->ulNumMacAddrs )
            nRet = -EADDRNOTAVAIL;

    return( nRet );
} /* kerSysGetMacAddr */

int kerSysReleaseMacAddress( unsigned char *pucaMacAddr )
{
    int nRet = -EINVAL;
    unsigned long ulIdx = 0;
    int idx = (pucaMacAddr[NVRAM_MAC_ADDRESS_LEN - CHANGED_OCTET] -
        g_pNvramInfo->ucaBaseMacAddr[NVRAM_MAC_ADDRESS_LEN - CHANGED_OCTET]);

    // if overflow 255 (negitive), add 256 to have the correct index
    if (idx < 0)
        idx += 256;
    ulIdx = (unsigned long) (idx >> SHIFT_BITS);

    if( ulIdx < g_pNvramInfo->ulNumMacAddrs )
    {
        PMAC_ADDR_INFO pMai = &g_pNvramInfo->MacAddrs[ulIdx];
        if( pMai->chInUse == 1 )
        {
            pMai->chInUse = 0;
            nRet = 0;
        }
    }

    return( nRet );
} /* kerSysReleaseMacAddr */

int kerSysGetSdramSize( void )
{
    return( (int) g_pNvramInfo->ulSdramSize );
} /* kerSysGetSdramSize */


void kerSysLedCtrl(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState)
{
    if (g_ledInitialized)
      boardLedCtrl(ledName, ledState);
}

unsigned int kerSysMonitorPollHook( struct file *f, struct poll_table_struct *t)
{
    int mask = (*g_orig_fop_poll) (f, t);

    if( g_wakeup_monitor == 1 && g_monitor_file == f )
    {
        /* If g_wakeup_monitor is non-0, the user mode application needs to
         * return from a blocking select function.  Return POLLPRI which will
         * cause the select to return with the exception descriptor set.
         */
        mask |= POLLPRI;
        g_wakeup_monitor = 0;
    }

    return( mask );
}

/* Put the user mode application that monitors link state on a run queue. */
void kerSysWakeupMonitorTask( void )
{
    g_wakeup_monitor = 1;
    if( g_monitor_task )
        wake_up_process( g_monitor_task );
}

//********************************************************************************************
// misc. ioctl calls come to here. (flash, led, reset, kernel memory access, etc.)
//********************************************************************************************
static int board_ioctl( struct inode *inode, struct file *flip,
                        unsigned int command, unsigned long arg )
{
    int ret = 0;
    BOARD_IOCTL_PARMS ctrlParms;
    unsigned char ucaMacAddr[NVRAM_MAC_ADDRESS_LEN];
    int allowedSize;

    switch (command) 
    {
        case BOARD_IOCTL_FLASH_INIT:
            // not used for now.  kerSysBcmImageInit();
            break;


        case BOARD_IOCTL_FLASH_WRITE:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
            {
                NVRAM_DATA SaveNvramData;
                PNVRAM_DATA pNvramData = (PNVRAM_DATA) get_nvram_start_addr();

                switch (ctrlParms.action)
                {
                    case SCRATCH_PAD:
                        ret = kerSysScratchPadSet(ctrlParms.string, ctrlParms.buf, ctrlParms.offset);
                        break;

                    case PERSISTENT:
                        ret = kerSysPersistentSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;
                
                    case NVRAM:
                        ret = kerSysNvRamSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;

                    case BCM_IMAGE_CFE:
                        if( ctrlParms.strLen <= 0 || ctrlParms.strLen > FLASH45_LENGTH_BOOT_ROM )
                        {
                            printk("Illegal CFE size [%d]. Size allowed: [%d]\n",
                                ctrlParms.strLen, FLASH45_LENGTH_BOOT_ROM);
                            ret = -1;
                            break;
                        }

                        // save NVRAM data into a local structure
                        memcpy( &SaveNvramData, pNvramData, sizeof(NVRAM_DATA) );

                        // set memory type field
                        BpGetSdramSize( (unsigned long *) &ctrlParms.string[SDRAM_TYPE_ADDRESS_OFFSET] );

                        ret = kerSysBcmImageSet(ctrlParms.offset, ctrlParms.string, ctrlParms.strLen);

                        // if nvram is not valid, restore the current nvram settings
                        if( BpSetBoardId( pNvramData->szBoardId ) != BP_SUCCESS &&
                            *(unsigned long *) pNvramData == NVRAM_DATA_ID )
                        {
                            kerSysNvRamSet((char *) &SaveNvramData, sizeof(SaveNvramData), 0);
                        }
                        break;
                        
                    case BCM_IMAGE_FS:
                        allowedSize = (int) flash_get_total_size() - \
                            FLASH_RESERVED_AT_END - TAG_LEN - FLASH45_LENGTH_BOOT_ROM;
                        if( ctrlParms.strLen <= 0 || ctrlParms.strLen > allowedSize)
                        {
                            printk("Illegal root file system size [%d]. Size allowed: [%d]\n",
                                ctrlParms.strLen,  allowedSize);
                            ret = -1;
                            break;
                        }
                        ret = kerSysBcmImageSet(ctrlParms.offset, ctrlParms.string, ctrlParms.strLen);
                        kerSysMipsSoftReset();
                        break;

                    case BCM_IMAGE_KERNEL:  // not used for now.
                        break;
                    case BCM_IMAGE_WHOLE:
                        if(ctrlParms.strLen <= 0)
                        {
                            printk("Illegal flash image size [%d].\n", ctrlParms.strLen);
                            ret = -1;
                            break;
                        }

                        // save NVRAM data into a local structure
                        memcpy( &SaveNvramData, pNvramData, sizeof(NVRAM_DATA) );

                        ret = kerSysBcmImageSet(ctrlParms.offset, ctrlParms.string, ctrlParms.strLen);

                        // if nvram is not valid, restore the current nvram settings
                        if( BpSetBoardId( pNvramData->szBoardId ) != BP_SUCCESS &&
                            *(unsigned long *) pNvramData == NVRAM_DATA_ID )
                        {
                            kerSysNvRamSet((char *) &SaveNvramData, sizeof(SaveNvramData), 0);
                        }

                        kerSysMipsSoftReset();
                        break;

                    default:
                        ret = -EINVAL;
                        printk("flash_ioctl_command: invalid command %d\n", ctrlParms.action);
                        break;
                }
                ctrlParms.result = ret;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            }
            else
                ret = -EFAULT;
            break;

        case BOARD_IOCTL_FLASH_READ:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                switch (ctrlParms.action)
                {
                    case SCRATCH_PAD:
                        ret = kerSysScratchPadGet(ctrlParms.string, ctrlParms.buf, ctrlParms.offset);
                        break;

                    case PERSISTENT:
                        ret = kerSysPersistentGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;

                    case NVRAM:
                        ret = kerSysNvRamGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;

                    case FLASH_SIZE:
                        ret = kerSysFlashSizeGet();
                        break;

                    default:
                        ret = -EINVAL;
                        printk("Not supported.  invalid command %d\n", ctrlParms.action);
                        break;
                }
                ctrlParms.result = ret;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            }
            else
                ret = -EFAULT;
            break;

        case BOARD_IOCTL_GET_NR_PAGES:
            ctrlParms.result = nr_free_pages() + get_page_cache_size();
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
            break;

        case BOARD_IOCTL_DUMP_ADDR:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                dumpaddr( (unsigned char *) ctrlParms.string, ctrlParms.strLen );
                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }
            else
                ret = -EFAULT;
            break;

        case BOARD_IOCTL_SET_MEMORY:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                unsigned long  *pul = (unsigned long *)  ctrlParms.string;
                unsigned short *pus = (unsigned short *) ctrlParms.string;
                unsigned char  *puc = (unsigned char *)  ctrlParms.string;
                switch( ctrlParms.strLen )
                {
                    case 4:
                        *pul = (unsigned long) ctrlParms.offset;
                        break;
                    case 2:
                        *pus = (unsigned short) ctrlParms.offset;
                        break;
                    case 1:
                        *puc = (unsigned char) ctrlParms.offset;
                        break;
                }
                dumpaddr( (unsigned char *) ctrlParms.string, sizeof(long) );
                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }
            else
                ret = -EFAULT;
            break;
      
        case BOARD_IOCTL_MIPS_SOFT_RESET:
            kerSysMipsSoftReset();
            break;

        case BOARD_IOCTL_LED_CTRL:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
	            kerSysLedCtrl((BOARD_LED_NAME)ctrlParms.strLen, (BOARD_LED_STATE)ctrlParms.offset);
	            ret = 0;
	        }
            break;

        case BOARD_IOCTL_GET_ID:
            if (copy_from_user((void*)&ctrlParms, (void*)arg,
                sizeof(ctrlParms)) == 0) 
            {
                if( ctrlParms.string )
                {
                    char *p = (char *) get_system_type();
                    if( strlen(p) + 1 < ctrlParms.strLen )
                        ctrlParms.strLen = strlen(p) + 1;
                    __copy_to_user(ctrlParms.string, p, ctrlParms.strLen);
                }

                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                    sizeof(BOARD_IOCTL_PARMS));
            }
            break;

        case BOARD_IOCTL_GET_MAC_ADDRESS:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                ctrlParms.result = kerSysGetMacAddress( ucaMacAddr,
                    ctrlParms.offset );

                if( ctrlParms.result == 0 )
                {
                    __copy_to_user(ctrlParms.string, ucaMacAddr,
                        sizeof(ucaMacAddr));
                }

                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                    sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }
            else
                ret = -EFAULT;
            break;

        case BOARD_IOCTL_RELEASE_MAC_ADDRESS:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                if (copy_from_user((void*)ucaMacAddr, (void*)ctrlParms.string, \
                     NVRAM_MAC_ADDRESS_LEN) == 0) 
                {
                    ctrlParms.result = kerSysReleaseMacAddress( ucaMacAddr );
                }
                else
                {
                    ctrlParms.result = -EACCES;
                }

                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                    sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }
            else
                ret = -EFAULT;
            break;

        case BOARD_IOCTL_GET_PSI_SIZE:
            ctrlParms.result = (int) g_pNvramInfo->ulPsiSize;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
            break;

        case BOARD_IOCTL_GET_SDRAM_SIZE:
            ctrlParms.result = (int) g_pNvramInfo->ulSdramSize;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
            break;

        case BOARD_IOCTL_GET_BASE_MAC_ADDRESS:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                __copy_to_user(ctrlParms.string, g_pNvramInfo->ucaBaseMacAddr, NVRAM_MAC_ADDRESS_LEN);
                ctrlParms.result = 0;

                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                    sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }
            else
                ret = -EFAULT;
            break;

        case BOARD_IOCTL_GET_CHIP_ID:
            ctrlParms.result = (int) (PERF->RevID & 0xFFFF0000) >> 16;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
            break;

        case BOARD_IOCTL_GET_NUM_ENET: {
            ETHERNET_MAC_INFO EnetInfos[BP_MAX_ENET_MACS];
            int i, numeth = 0;
            if (BpGetEthernetMacInfo(EnetInfos, BP_MAX_ENET_MACS) == BP_SUCCESS) {
            for( i = 0; i < BP_MAX_ENET_MACS; i++) {
                if (EnetInfos[i].ucPhyType != BP_ENET_NO_PHY) {
                numeth++;
                }
            }
            ctrlParms.result = numeth;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,	 sizeof(BOARD_IOCTL_PARMS));   
            ret = 0;
            }
	        else {
	            ret = -EFAULT;
	        }
	        break;
            }

        case BOARD_IOCTL_GET_CFE_VER:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                char *vertag =  (char *)(FLASH_BASE + CFE_VERSION_OFFSET);
                if (ctrlParms.strLen < CFE_VERSION_SIZE) {
                    ctrlParms.result = 0;
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                    ret = -EFAULT;
                }
                else if (strncmp(vertag, "cfe-v", 5)) { // no tag info in flash
                    ctrlParms.result = 0;
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                    ret = 0;
                }
                else {
                    ctrlParms.result = 1;
                    __copy_to_user(ctrlParms.string, vertag+CFE_VERSION_MARK_SIZE, CFE_VERSION_SIZE);
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                    ret = 0;
                }
            }
            else {
                ret = -EFAULT;
            }
            break;

        case BOARD_IOCTL_GET_ENET_CFG:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                ETHERNET_MAC_INFO EnetInfos[BP_MAX_ENET_MACS];
                if (BpGetEthernetMacInfo(EnetInfos, BP_MAX_ENET_MACS) == BP_SUCCESS) {
                    if (ctrlParms.strLen == sizeof(EnetInfos)) {
                        __copy_to_user(ctrlParms.string, EnetInfos, sizeof(EnetInfos));
                        ctrlParms.result = 0;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));   
                        ret = 0;
                    } else
	                    ret = -EFAULT;
                }
	            else {
	                ret = -EFAULT;
	            }
	            break;
            }
            else {
                ret = -EFAULT;
            }
            break;            

#if defined (WIRELESS)
        case BOARD_IOCTL_GET_WLAN_ANT_INUSE:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                unsigned short antInUse = 0;
                if (BpGetWirelessAntInUse(&antInUse) == BP_SUCCESS) {
                    if (ctrlParms.strLen == sizeof(antInUse)) {
                        __copy_to_user(ctrlParms.string, &antInUse, sizeof(antInUse));
                        ctrlParms.result = 0;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));   
                        ret = 0;
                    } else
	                    ret = -EFAULT;
                }
	        else {
	           ret = -EFAULT;
	        }
	        break;
            }
            else {
                ret = -EFAULT;
            }
            break;            
#endif            
        case BOARD_IOCTL_SET_TRIGGER_EVENT:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {            	
            	BOARD_IOC *board_ioc = (BOARD_IOC *)flip->private_data;            	
                ctrlParms.result = -EFAULT;
                ret = -EFAULT;
                if (ctrlParms.strLen == sizeof(unsigned long)) {                 	                    
                    board_ioc->eventmask |= *((int*)ctrlParms.string);                    
#if defined (WIRELESS)                    
                    if((board_ioc->eventmask & SES_EVENTS)) {
                        if(sesBtn_irq != BP_NOT_DEFINED) {
                            BcmHalInterruptEnable(sesBtn_irq);
                            ctrlParms.result = 0;
                            ret = 0;
                        }                                                
                    } 
#endif                                                
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));                        
                }
	        break;
            }
            else {
                ret = -EFAULT;
            }
            break;                        

        case BOARD_IOCTL_GET_TRIGGER_EVENT:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
            	BOARD_IOC *board_ioc = (BOARD_IOC *)flip->private_data;
                if (ctrlParms.strLen == sizeof(unsigned long)) {
                    __copy_to_user(ctrlParms.string, &board_ioc->eventmask, sizeof(unsigned long));
                    ctrlParms.result = 0;
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));   
                    ret = 0;
                } else
	            ret = -EFAULT;

	        break;
            }
            else {
                ret = -EFAULT;
            }
            break;                
            
        case BOARD_IOCTL_UNSET_TRIGGER_EVENT:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                if (ctrlParms.strLen == sizeof(unsigned long)) {
                    BOARD_IOC *board_ioc = (BOARD_IOC *)flip->private_data;                	
                    board_ioc->eventmask &= (~(*((int*)ctrlParms.string)));                  
                    ctrlParms.result = 0;
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));   
                    ret = 0;
                } else
	            ret = -EFAULT;

	        break;
            } 
            else {
                ret = -EFAULT;
            }
            break;            
#if defined (WIRELESS)
        case BOARD_IOCTL_SET_SES_LED:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                if (ctrlParms.strLen == sizeof(int)) {
                    sesLed_ctrl(*(int*)ctrlParms.string);
                    ctrlParms.result = 0;
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));   
                    ret = 0;
                } else
	            ret = -EFAULT;

	        break;
            }
            else {
                ret = -EFAULT;
            }
            break;            
#endif                                                            

        case BOARD_IOCTL_SET_MONITOR_FD:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                int fput_needed = 0;

                g_monitor_file = fget_light( ctrlParms.offset, &fput_needed );
                if( g_monitor_file ) {
                    /* Hook this file descriptor's poll function in order to set
                     * the exception descriptor when there is a change in link
                     * state.
                     */
                    g_monitor_task = current;
                    g_orig_fop_poll = g_monitor_file->f_op->poll;
                    g_monitor_file->f_op->poll = kerSysMonitorPollHook;
                }
            }
            break;

        case BOARD_IOCTL_WAKEUP_MONITOR_TASK:
            kerSysWakeupMonitorTask();
            break;

        default:
            ret = -EINVAL;
            ctrlParms.result = 0;
            printk("board_ioctl: invalid command %x, cmd %d .\n",command,_IOC_NR(command));
            break;

  } /* switch */

  return (ret);

} /* board_ioctl */

/***************************************************************************
 * SES Button ISR/GPIO/LED functions.
 ***************************************************************************/
#if defined (WIRELESS) 
static irqreturn_t sesBtn_isr(int irq, void *dev_id, struct pt_regs *ptregs)
{   
#if defined(_BCM96338_) || defined(CONFIG_BCM96338)
    unsigned long gpio_mask = GPIO_NUM_TO_MASK(sesBtn_gpio);
    volatile unsigned long *gpio_reg = &GPIO->GPIOio;
#endif
#if defined(_BCM96345_) || defined(CONFIG_BCM96345)
    unsigned short gpio_mask = GPIO_NUM_TO_MASK(sesBtn_gpio);
    volatile unsigned short *gpio_reg = &GPIO->GPIOio;
#endif
#if defined(_BCM96348_) || defined (CONFIG_BCM96348)
    unsigned long gpio_mask = GPIO_NUM_TO_MASK(sesBtn_gpio);
    volatile unsigned long *gpio_reg = &GPIO->GPIOio;

    if( (sesBtn_gpio & ~BP_ACTIVE_MASK) >= 32 )
    {
        gpio_mask = GPIO_NUM_TO_MASK_HIGH(sesBtn_gpio);
        gpio_reg = &GPIO->GPIOio_high;
    }
#endif 
    		
    if (!(*gpio_reg & gpio_mask)){
        wake_up_interruptible(&g_board_wait_queue);
        return IRQ_RETVAL(1);
    } else {
        return IRQ_RETVAL(0);    	
    }
}

static void __init sesBtn_mapGpio()
{	
    if( BpGetWirelessSesBtnGpio(&sesBtn_gpio) == BP_SUCCESS )
    {
        printk("SES: Button GPIO 0x%x is enabled\n", sesBtn_gpio);    
    }
}

static void __init sesBtn_mapIntr(int context)
{    	
    if( BpGetWirelessSesExtIntr(&sesBtn_irq) == BP_SUCCESS )
    {
    	printk("SES: Button Interrupt 0x%x is enabled\n", sesBtn_irq);
    }
    else
    	return;
    	    
    sesBtn_irq += INTERRUPT_ID_EXTERNAL_0;	
    		
    if (BcmHalMapInterrupt((FN_HANDLER)sesBtn_isr, context, sesBtn_irq)) {
    	printk("SES: Interrupt mapping failed\n");
    }    
    BcmHalInterruptEnable(sesBtn_irq);
}


static unsigned int sesBtn_poll(struct file *file, struct poll_table_struct *wait)
{
#if defined(_BCM96338_) || defined(CONFIG_BCM96338)
    unsigned long gpio_mask = GPIO_NUM_TO_MASK(sesBtn_gpio);
    volatile unsigned long *gpio_reg = &GPIO->GPIOio;
#endif
#if defined(_BCM96345_) || defined(CONFIG_BCM96345)
    unsigned short gpio_mask = GPIO_NUM_TO_MASK(sesBtn_gpio);
    volatile unsigned short *gpio_reg = &GPIO->GPIOio;
#endif
#if defined(_BCM96348_) || defined (CONFIG_BCM96348)
    unsigned long gpio_mask = GPIO_NUM_TO_MASK(sesBtn_gpio);
    volatile unsigned long *gpio_reg = &GPIO->GPIOio;

    if( (sesBtn_gpio & ~BP_ACTIVE_MASK) >= 32 )
    {
        gpio_mask = GPIO_NUM_TO_MASK_HIGH(sesBtn_gpio);
        gpio_reg = &GPIO->GPIOio_high;
    }
#endif 
    		
    if (!(*gpio_reg & gpio_mask)){
	return POLLIN;
    }	
    return 0;
}

static ssize_t sesBtn_read(struct file *file,  char __user *buffer, size_t count, loff_t *ppos)
{
    volatile unsigned int event=0;
    ssize_t ret=0;	

#if defined(_BCM96338_) || defined (CONFIG_BCM96338)
    unsigned long gpio_mask = GPIO_NUM_TO_MASK(sesBtn_gpio);
    volatile unsigned long *gpio_reg = &GPIO->GPIOio;
#endif
#if defined(_BCM96345_) || defined (CONFIG_BCM96345)
    unsigned short gpio_mask = GPIO_NUM_TO_MASK(sesBtn_gpio);
    volatile unsigned short *gpio_reg = &GPIO->GPIOio;
#endif
#if defined(_BCM96348_) || defined (CONFIG_BCM96348)
    unsigned long gpio_mask = GPIO_NUM_TO_MASK(sesBtn_gpio);
    volatile unsigned long *gpio_reg = &GPIO->GPIOio;
    
    if( (sesBtn_gpio & ~BP_ACTIVE_MASK) >= 32 )
    {
        gpio_mask = GPIO_NUM_TO_MASK_HIGH(sesBtn_gpio);
        gpio_reg = &GPIO->GPIOio_high;
    }
#endif 

    if(*gpio_reg & gpio_mask){
	BcmHalInterruptEnable(sesBtn_irq);		
	return ret;
    }	
    event = SES_EVENTS;
    __copy_to_user((char*)buffer, (char*)&event, sizeof(event));	
    BcmHalInterruptEnable(sesBtn_irq);	
    count -= sizeof(event);
    buffer += sizeof(event);
    ret += sizeof(event);	
    return ret;	
}

static void __init sesLed_mapGpio()
{	
    if( BpGetWirelessSesBtnGpio(&sesLed_gpio) == BP_SUCCESS )
    {
        printk("SES: LED GPIO 0x%x is enabled\n", sesBtn_gpio);    
    }
}

static void sesLed_ctrl(int action)
{

    //char status = ((action >> 8) & 0xff); /* extract status */
    //char event = ((action >> 16) & 0xff); /* extract event */        
    //char blinktype = ((action >> 24) & 0xff); /* extract blink type for SES_LED_BLINK  */
    
    BOARD_LED_STATE led;
    
    if(sesLed_gpio == BP_NOT_DEFINED)
        return;
    	
    action &= 0xff; /* extract led */

    //printk("blinktype=%d, event=%d, status=%d\n",(int)blinktype, (int)event, (int)status);
            	
    switch (action) 
    {
        case SES_LED_ON:
            //printk("SES: led on\n");
            led = kLedStateOn;                                          
            break;
        case SES_LED_BLINK:
            //printk("SES: led blink\n");
            led = kLedStateSlowBlinkContinues;           		
            break;
        case SES_LED_OFF:
            default:
            //printk("SES: led off\n");
            led = kLedStateOff;  						
    }	
    
    kerSysLedCtrl(kLedSes, led);
}

static void __init ses_board_init()
{
    sesBtn_mapGpio();
    sesBtn_mapIntr(0);
    sesLed_mapGpio();
}
static void __exit ses_board_deinit()
{
    if(sesBtn_irq)
        BcmHalInterruptDisable(sesBtn_irq);
}
#endif

/***************************************************************************
 * Dying gasp ISR and functions.
 ***************************************************************************/
#define KERSYS_DBG	printk

#if defined(CONFIG_BCM96345)
#define	CYCLE_PER_US	70
#elif defined(CONFIG_BCM96348) || defined(CONFIG_BCM96338)
/* The BCM6348 cycles per microsecond is really variable since the BCM6348
 * MIPS speed can vary depending on the PLL settings.  However, an appoximate
 * value of 120 will still work OK for the test being done.
 */
#define	CYCLE_PER_US	120
#endif
#define	DG_GLITCH_TO	(100*CYCLE_PER_US)
 
static void __init kerSysDyingGaspMapIntr()
{
    unsigned long ulIntr;
    	
#if defined(CONFIG_BCM96348) || defined(_BCM96348_) || defined(CONFIG_BCM96338) || defined(_BCM96338_)
    if( BpGetAdslDyingGaspExtIntr( &ulIntr ) == BP_SUCCESS ) {
		BcmHalMapInterrupt((FN_HANDLER)kerSysDyingGaspIsr, 0, INTERRUPT_ID_DG);
		BcmHalInterruptEnable( INTERRUPT_ID_DG );
    }
#elif defined(CONFIG_BCM96345) || defined(_BCM96345_)
    if( BpGetAdslDyingGaspExtIntr( &ulIntr ) == BP_SUCCESS ) {
        ulIntr += INTERRUPT_ID_EXTERNAL_0;
        BcmHalMapInterrupt((FN_HANDLER)kerSysDyingGaspIsr, 0, ulIntr);
        BcmHalInterruptEnable( ulIntr );
    }
#endif

} 

void kerSysSetWdTimer(ulong timeUs)
{
	TIMER->WatchDogDefCount = timeUs * (FPERIPH/1000000);
	TIMER->WatchDogCtl = 0xFF00;
	TIMER->WatchDogCtl = 0x00FF;
}

ulong kerSysGetCycleCount(void)
{
    ulong cnt; 
#ifdef _WIN32_WCE
    cnt = 0;
#else
    __asm volatile("mfc0 %0, $9":"=d"(cnt));
#endif
    return(cnt); 
}

static Bool kerSysDyingGaspCheckPowerLoss(void)
{
    ulong clk0;
    ulong ulIntr;

    ulIntr = 0;
    clk0 = kerSysGetCycleCount();

    UART->Data = 'D';
    UART->Data = '%';
    UART->Data = 'G';

#if defined(CONFIG_BCM96345)
    BpGetAdslDyingGaspExtIntr( &ulIntr );

    do {
        ulong clk1;
        
        clk1 = kerSysGetCycleCount();		/* time cleared */
	/* wait a little to get new reading */
        while ((kerSysGetCycleCount()-clk1) < CYCLE_PER_US*2)
            ;
    } while ((0 == (PERF->ExtIrqCfg & (1 << (ulIntr + EI_STATUS_SHFT)))) && ((kerSysGetCycleCount() - clk0) < DG_GLITCH_TO));

    if (PERF->ExtIrqCfg & (1 << (ulIntr + EI_STATUS_SHFT))) {	/* power glitch */
        BcmHalInterruptEnable( ulIntr + INTERRUPT_ID_EXTERNAL_0);
        KERSYS_DBG(" - Power glitch detected. Duration: %ld us\n", (kerSysGetCycleCount() - clk0)/CYCLE_PER_US);
        return 0;
    }
#elif (defined(CONFIG_BCM96348) || defined(CONFIG_BCM96338)) && !defined(VXWORKS)
    do {
        ulong clk1;
        
        clk1 = kerSysGetCycleCount();		/* time cleared */
	/* wait a little to get new reading */
        while ((kerSysGetCycleCount()-clk1) < CYCLE_PER_US*2)
            ;
     } while ((PERF->IrqStatus & (1 << (INTERRUPT_ID_DG - INTERNAL_ISR_TABLE_OFFSET))) && ((kerSysGetCycleCount() - clk0) < DG_GLITCH_TO));

    if (!(PERF->IrqStatus & (1 << (INTERRUPT_ID_DG - INTERNAL_ISR_TABLE_OFFSET)))) {
        BcmHalInterruptEnable( INTERRUPT_ID_DG );
        KERSYS_DBG(" - Power glitch detected. Duration: %ld us\n", (kerSysGetCycleCount() - clk0)/CYCLE_PER_US);
        return 0;
    }
#endif
    return 1;
}

static void kerSysDyingGaspShutdown( void )
{
    kerSysSetWdTimer(1000000);
#if defined(CONFIG_BCM96345)
    PERF->blkEnables &= ~(EMAC_CLK_EN | USB_CLK_EN | CPU_CLK_EN);
#elif defined(CONFIG_BCM96348)
    PERF->blkEnables &= ~(EMAC_CLK_EN | USBS_CLK_EN | USBH_CLK_EN | SAR_CLK_EN);
#endif
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
static irqreturn_t kerSysDyingGaspIsr(int irq, void * dev_id, struct pt_regs * regs)
#else
static unsigned int kerSysDyingGaspIsr(void)
#endif
{	
    struct list_head *pos;
    CB_DGASP_LIST *tmp, *dsl = NULL;	

    if (kerSysDyingGaspCheckPowerLoss()) {        

        /* first to turn off everything other than dsl */        
        list_for_each(pos, &g_cb_dgasp_list_head->list) {    	
            tmp = list_entry(pos, CB_DGASP_LIST, list);
    	    if(strncmp(tmp->name, "dsl", 3)) {
    	        (tmp->cb_dgasp_fn)(tmp->context); 
    	    }else {
    		dsl = tmp;    		    	
    	    }       
        }  
        
        /* now send dgasp */
        if(dsl)
            (dsl->cb_dgasp_fn)(dsl->context); 

        /* reset and shutdown system */
        kerSysDyingGaspShutdown();
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
return( IRQ_HANDLED );
#else
    return( 1 );
#endif
}

static void __init kerSysInitDyingGaspHandler( void )
{
    CB_DGASP_LIST *new_node;

    if( g_cb_dgasp_list_head != NULL) {
        printk("Error: kerSysInitDyingGaspHandler: list head is not null\n");
        return;	
    }
    new_node= (CB_DGASP_LIST *)kmalloc(sizeof(CB_DGASP_LIST), GFP_KERNEL);
    memset(new_node, 0x00, sizeof(CB_DGASP_LIST));
    INIT_LIST_HEAD(&new_node->list);    
    g_cb_dgasp_list_head = new_node; 
		
} /* kerSysInitDyingGaspHandler */

static void __exit kerSysDeinitDyingGaspHandler( void )
{
    struct list_head *pos;
    CB_DGASP_LIST *tmp; 
     	
    if(g_cb_dgasp_list_head == NULL)
        return;
        
    list_for_each(pos, &g_cb_dgasp_list_head->list) {    	
    	tmp = list_entry(pos, CB_DGASP_LIST, list);
        list_del(pos);
	kfree(tmp);
    }       

    kfree(g_cb_dgasp_list_head);	
    g_cb_dgasp_list_head = NULL;
    
} /* kerSysDeinitDyingGaspHandler */

void kerSysRegisterDyingGaspHandler(char *devname, void *cbfn, void *context)
{
    CB_DGASP_LIST *new_node;

    if( g_cb_dgasp_list_head == NULL) {
        printk("Error: kerSysRegisterDyingGaspHandler: list head is null\n");	
        return;    
    }
    
    if( devname == NULL || cbfn == NULL ) {
        printk("Error: kerSysRegisterDyingGaspHandler: register info not enough (%s,%x,%x)\n", devname, (unsigned int)cbfn, (unsigned int)context);	    	
        return;
    }
       
    new_node= (CB_DGASP_LIST *)kmalloc(sizeof(CB_DGASP_LIST), GFP_KERNEL);
    memset(new_node, 0x00, sizeof(CB_DGASP_LIST));    
    INIT_LIST_HEAD(&new_node->list);
    strncpy(new_node->name, devname, IFNAMSIZ);
    new_node->cb_dgasp_fn = (cb_dgasp_t)cbfn;
    new_node->context = context;
    list_add(&new_node->list, &g_cb_dgasp_list_head->list);
    
    printk("dgasp: kerSysRegisterDyingGaspHandler: %s registered \n", devname);
        	
} /* kerSysRegisterDyingGaspHandler */

void kerSysDeregisterDyingGaspHandler(char *devname)
{
    struct list_head *pos;
    CB_DGASP_LIST *tmp;    
    
    if(g_cb_dgasp_list_head == NULL) {
        printk("Error: kerSysDeregisterDyingGaspHandler: list head is null\n");
        return;	
    }

    if(devname == NULL) {
        printk("Error: kerSysDeregisterDyingGaspHandler: devname is null\n");
        return;	
    }
    
    printk("kerSysDeregisterDyingGaspHandler: %s is deregistering\n", devname);

    list_for_each(pos, &g_cb_dgasp_list_head->list) {    	
    	tmp = list_entry(pos, CB_DGASP_LIST, list);
    	if(!strcmp(tmp->name, devname)) {
            list_del(pos);
	    kfree(tmp);
	    printk("kerSysDeregisterDyingGaspHandler: %s is deregistered\n", devname);
	    return;
	}
    }	
    printk("kerSysDeregisterDyingGaspHandler: %s not (de)registered\n", devname);
	
} /* kerSysDeregisterDyingGaspHandler */

/***************************************************************************
 * MACRO to call driver initialization and cleanup functions.
 ***************************************************************************/
module_init( brcm_board_init );
module_exit( brcm_board_cleanup );

EXPORT_SYMBOL(kerSysNvRamGet);
EXPORT_SYMBOL(dumpaddr);
EXPORT_SYMBOL(kerSysGetMacAddress);
EXPORT_SYMBOL(kerSysReleaseMacAddress);
EXPORT_SYMBOL(kerSysGetSdramSize);
EXPORT_SYMBOL(kerSysLedCtrl);
EXPORT_SYMBOL(kerSysLedRegisterHwHandler);
EXPORT_SYMBOL(BpGetBoardIds);
EXPORT_SYMBOL(BpGetSdramSize);
EXPORT_SYMBOL(BpGetPsiSize);
EXPORT_SYMBOL(BpGetEthernetMacInfo);
EXPORT_SYMBOL(BpGetRj11InnerOuterPairGpios);
EXPORT_SYMBOL(BpGetPressAndHoldResetGpio);
EXPORT_SYMBOL(BpGetVoipResetGpio);
EXPORT_SYMBOL(BpGetVoipIntrGpio);
EXPORT_SYMBOL(BpGetPcmciaResetGpio);
EXPORT_SYMBOL(BpGetRtsCtsUartGpios);
EXPORT_SYMBOL(BpGetAdslLedGpio);
EXPORT_SYMBOL(BpGetAdslFailLedGpio);
EXPORT_SYMBOL(BpGetWirelessLedGpio);
EXPORT_SYMBOL(BpGetUsbLedGpio);
EXPORT_SYMBOL(BpGetHpnaLedGpio);
EXPORT_SYMBOL(BpGetWanDataLedGpio);
EXPORT_SYMBOL(BpGetPppLedGpio);
EXPORT_SYMBOL(BpGetPppFailLedGpio);
EXPORT_SYMBOL(BpGetVoipLedGpio);
EXPORT_SYMBOL(BpGetWirelessExtIntr);
EXPORT_SYMBOL(BpGetAdslDyingGaspExtIntr);
EXPORT_SYMBOL(BpGetVoipExtIntr);
EXPORT_SYMBOL(BpGetHpnaExtIntr);
EXPORT_SYMBOL(BpGetHpnaChipSelect);
EXPORT_SYMBOL(BpGetVoipChipSelect);
EXPORT_SYMBOL(BpGetWirelessSesBtnGpio);
EXPORT_SYMBOL(BpGetWirelessSesExtIntr);
EXPORT_SYMBOL(BpGetWirelessSesLedGpio);
EXPORT_SYMBOL(kerSysRegisterDyingGaspHandler);
EXPORT_SYMBOL(kerSysDeregisterDyingGaspHandler);
EXPORT_SYMBOL(kerSysGetCycleCount);
EXPORT_SYMBOL(kerSysSetWdTimer);
EXPORT_SYMBOL(kerSysWakeupMonitorTask);

