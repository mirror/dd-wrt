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
#include <linux/pci.h>

#include <bcm_map_part.h>
#include <board.h>
#define  BCMTAG_EXE_USE
#include <bcmTag.h>
#include "boardparms.h"
#include "flash_api.h"
#include "bcm_intr.h"
#include "board.h"
#include "bcm_map_part.h"

/* Typedefs. */

#if defined (WIRELESS)
#define SES_EVENT_BTN_PRESSED      0x00000001
#define SES_EVENTS                 SES_EVENT_BTN_PRESSED /*OR all values if any*/
#define SES_LED_OFF                0
#define SES_LED_ON                 1
#define SES_LED_BLINK              2

#define WLAN_ONBOARD_SLOT    1 /* Corresponds to IDSEL -- EBI_A11/PCI_AD12 */ 
#define BRCM_VENDOR_ID	     0x14e4
#define BRCM_WLAN_DEVICE_IDS 0x4300  
#endif

typedef struct
{
    unsigned long ulId;
    char chInUse;
    char chReserved[3];
} MAC_ADDR_INFO, *PMAC_ADDR_INFO;

typedef struct
{
    unsigned long ulNumMacAddrs;
    unsigned char ucaBaseMacAddr[NVRAM_MAC_ADDRESS_LEN];
    MAC_ADDR_INFO MacAddrs[1];
} MAC_INFO, *PMAC_INFO;

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
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0},   
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0},   
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0},   
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0},   
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0} // NOTE: kLedEnd has to be at the end.
};

/* Externs. */
extern struct file fastcall *fget_light(unsigned int fd, int *fput_needed);
extern unsigned int nr_free_pages (void);
extern const char *get_system_type(void);
extern unsigned long get_nvram_start_addr(void);
extern unsigned long getMemorySize(void);
extern void __init boardLedInit(PLED_MAP_PAIR);
extern void boardLedCtrl(BOARD_LED_NAME, BOARD_LED_STATE);
extern void kerSysLedRegisterHandler( BOARD_LED_NAME ledName,
    HANDLE_LED_FUNC ledHwFunc, int ledFailType );

/* Prototypes. */
static void set_mac_info( void );
static int board_open( struct inode *inode, struct file *filp );
static int board_ioctl( struct inode *inode, struct file *flip, unsigned int command, unsigned long arg );
static ssize_t board_read(struct file *filp,  char __user *buffer, size_t count, loff_t *ppos); 
static unsigned int board_poll(struct file *filp, struct poll_table_struct *wait);
static int board_release(struct inode *inode, struct file *filp);                        

static BOARD_IOC* borad_ioc_alloc(void);
static void borad_ioc_free(BOARD_IOC* board_ioc);

static void writeNvramData(PNVRAM_DATA pNvramData);
static int readNvramData(PNVRAM_DATA pNvramData);

/* DyingGasp function prototype */
static void __init kerSysDyingGaspMapIntr(void);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
static irqreturn_t kerSysDyingGaspIsr(int irq, void * dev_id);
#else
static unsigned int kerSysDyingGaspIsr(void);
#endif
static void __init kerSysInitDyingGaspHandler( void );
static void __exit kerSysDeinitDyingGaspHandler( void );
/* -DyingGasp function prototype - */

static int ConfigCs(BOARD_IOCTL_PARMS *parms);
static void SetGpio(int gpio, GPIO_STATE_t state);

#if defined (WIRELESS)
static irqreturn_t sesBtn_isr(int irq, void *dev_id);
static void __init sesBtn_mapIntr(int context);
static Bool sesBtn_pressed(void);
static unsigned int sesBtn_poll(struct file *file, struct poll_table_struct *wait);
static ssize_t sesBtn_read(struct file *file,  char __user *buffer, size_t count, loff_t *ppos);
static void __init sesLed_mapGpio(void);
static void sesLed_ctrl(int action);
static void __init ses_board_init(void);
static void __exit ses_board_deinit(void);
static void __init kerSysScreenPciDevices(void);
#endif

static irqreturn_t reset_isr(int irq, void *dev_id);

static PMAC_INFO g_pMacInfo = NULL;
static unsigned long g_ulSdramSize;
static unsigned long g_ulPsiSize;
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

static int map_external_irq (int irq)
{
   int map_irq ;

   switch (irq) {
      case BP_EXT_INTR_0   :
         map_irq = INTERRUPT_ID_EXTERNAL_0;
         break ;
      case BP_EXT_INTR_1   :
         map_irq = INTERRUPT_ID_EXTERNAL_1;
         break ;
      case BP_EXT_INTR_2   :
         map_irq = INTERRUPT_ID_EXTERNAL_2;
         break ;
      case BP_EXT_INTR_3   :
         map_irq = INTERRUPT_ID_EXTERNAL_3;
         break ;
#if defined(CONFIG_BCM96358)
      case BP_EXT_INTR_4   :
         map_irq = INTERRUPT_ID_EXTERNAL_4;
         break ;
      case BP_EXT_INTR_5   :
         map_irq = INTERRUPT_ID_EXTERNAL_5;
         break ;
#endif
      default           :
         printk ("Invalid External Interrupt definition \n") ;
         map_irq = 0 ;
         break ;
   }
     return (map_irq) ;
}

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
     {kLedHpna, BpGetHpnaLedGpio, NULL},
     {kLedWanData, BpGetWanDataLedGpio, NULL},
     {kLedPPP, BpGetPppLedGpio, BpGetPppFailLedGpio},
     {kLedSes, BpGetWirelessSesLedGpio, NULL},
     {kLedVoip, BpGetVoipLedGpio, NULL},
     {kLedVoip1, BpGetVoip1LedGpio, NULL },     
     {kLedVoip2, BpGetVoip2LedGpio, NULL },     
     {kLedPots, BpGetPotsLedGpio, NULL }, 
     {kLedSecAdsl, BpGetSecAdslLedGpio, BpGetSecAdslFailLedGpio},
     {kLedEnd, NULL, NULL}
    };

    unsigned short rstToDflt_irq;	
    unsigned short vdslRelayGpio;	
    int ret;
        
    ret = register_chrdev(BOARD_DRV_MAJOR, "bcrmboard", &board_fops );
    if (ret < 0)
        printk( "brcm_board_init(major %d): fail to register device.\n",BOARD_DRV_MAJOR);
    else 
    {
        PLED_MAP_PAIR pLedMap = LedMapping;
        unsigned short gpio;
        struct BpLedInformation *pInfo;
        NVRAM_DATA nvramData;
        ETHERNET_MAC_INFO EnetInfo;

        printk("brcmboard: brcm_board_init entry\n");
        board_major = BOARD_DRV_MAJOR;

        readNvramData(&nvramData);

        if ((nvramData.ulPsiSize != -1) && (nvramData.ulPsiSize != 0))
            g_ulPsiSize = nvramData.ulPsiSize * ONEK;
        else
            g_ulPsiSize = DEFAULT_PSI_SIZE * ONEK;

        g_ulSdramSize = getMemorySize();

        set_mac_info();

        for( pInfo = bpLedInfo; pInfo->ledName != kLedEnd; pInfo++ )
        {
            if( pInfo->bpFunc && (*pInfo->bpFunc) (&gpio) == BP_SUCCESS &&
                gpio != BP_HW_DEFINED_AH && gpio != BP_HW_DEFINED_AL )
            {
                pLedMap->ledName = pInfo->ledName;
                pLedMap->ledMask = GPIO_NUM_TO_MASK(gpio);
                pLedMap->ledActiveLow = (gpio & BP_ACTIVE_LOW) ? 1 : 0;
                pLedMap->ledSerial = (gpio & BP_GPIO_SERIAL) ? 1 : 0;
            }
            if( pInfo->bpFuncFail && (*pInfo->bpFuncFail)(&gpio)==BP_SUCCESS &&
                gpio != BP_HW_DEFINED_AH && gpio != BP_HW_DEFINED_AL )
            {
                pLedMap->ledName = pInfo->ledName;
                pLedMap->ledMaskFail = GPIO_NUM_TO_MASK(gpio);
                pLedMap->ledActiveLowFail = (gpio & BP_ACTIVE_LOW) ? 1 : 0;
                pLedMap->ledSerialFail = (gpio & BP_GPIO_SERIAL) ? 1 : 0;
            }
            if( pLedMap->ledName != kLedEnd )
                pLedMap++;
        }

        if( BpGetEthernetMacInfo( &EnetInfo, 1 ) == BP_SUCCESS )
        {
            if( EnetInfo.usGpioPhyLinkSpeed != BP_NOT_DEFINED )
            {
                /* The internal Ethernet PHY has a GPIO for 10/100 link speed. */
                gpio = EnetInfo.usGpioPhyLinkSpeed;
                pLedMap->ledName = kLedEphyLinkSpeed;
                pLedMap->ledMask = GPIO_NUM_TO_MASK(gpio);
                pLedMap->ledActiveLow = (gpio & BP_ACTIVE_LOW) ? 1 : 0;
                pLedMap->ledSerial = (gpio & BP_GPIO_SERIAL) ? 1 : 0;
                pLedMap++;
            }
        }
        
        init_waitqueue_head(&g_board_wait_queue);
#if defined (WIRELESS)
        kerSysScreenPciDevices();   
        ses_board_init();
#endif        
        kerSysInitDyingGaspHandler();
        kerSysDyingGaspMapIntr();

        boardLedInit(LedMapping);
        g_ledInitialized = 1;

        if( BpGetResetToDefaultExtIntr(&rstToDflt_irq) == BP_SUCCESS )
        {
            rstToDflt_irq = map_external_irq (rstToDflt_irq) ;
            BcmHalMapInterrupt((FN_HANDLER)reset_isr, 0, rstToDflt_irq);
            BcmHalInterruptEnable(rstToDflt_irq);
        }

        if((vdslRelayGpio = BpGetVcopeGpio(VCOPE_RELAY_GPIO)) != BP_NOT_DEFINED)
            SetGpio( vdslRelayGpio, GPIO_LOW ); /* set relay to ADSL */
    }

    return ret;
} 

static void __init set_mac_info( void )
{
    NVRAM_DATA nvramData;
    unsigned long ulNumMacAddrs;

    readNvramData(&nvramData);
    ulNumMacAddrs = nvramData.ulNumMacAddrs;

    if( ulNumMacAddrs > 0 && ulNumMacAddrs <= NVRAM_MAC_COUNT_MAX )
    {
        unsigned long ulMacInfoSize =
            sizeof(MAC_INFO) + ((sizeof(MAC_ADDR_INFO) - 1) * ulNumMacAddrs);

        g_pMacInfo = (PMAC_INFO) kmalloc( ulMacInfoSize, GFP_KERNEL );

        if( g_pMacInfo )
        {
            memset( g_pMacInfo, 0x00, ulMacInfoSize );
            g_pMacInfo->ulNumMacAddrs = nvramData.ulNumMacAddrs;
            memcpy( g_pMacInfo->ucaBaseMacAddr, nvramData.ucaBaseMacAddr,
                NVRAM_MAC_ADDRESS_LEN );
        }
        else
            printk("ERROR - Could not allocate memory for MAC data\n");
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

/***************************************************************************
// Function Name: getCrc32
// Description  : caculate the CRC 32 of the given data.
// Parameters   : pdata - array of data.
//                size - number of input data bytes.
//                crc - either CRC32_INIT_VALUE or previous return value.
// Returns      : crc.
****************************************************************************/
static UINT32 getCrc32(byte *pdata, UINT32 size, UINT32 crc) 
{
    while (size-- > 0)
        crc = (crc >> 8) ^ Crc32_table[(crc ^ *pdata++) & 0xff];

    return crc;
}

// write the nvramData struct to nvram after CRC is calculated 
static void writeNvramData(PNVRAM_DATA pNvramData)
{
    UINT32 crc = CRC32_INIT_VALUE;
    
    pNvramData->ulCheckSum = 0;
    crc = getCrc32((char *)pNvramData, sizeof(NVRAM_DATA), crc);      
    pNvramData->ulCheckSum = crc;
    kerSysNvRamSet((char *)pNvramData, sizeof(NVRAM_DATA), 0);
}

// read the nvramData struct from nvram 
// return -1:  crc fail, 0 ok
static int readNvramData(PNVRAM_DATA pNvramData)
{
    UINT32 crc = CRC32_INIT_VALUE, savedCrc;
    
    kerSysNvRamGet((char *)pNvramData, sizeof(NVRAM_DATA), 0);
    savedCrc = pNvramData->ulCheckSum;
    pNvramData->ulCheckSum = 0;
    crc = getCrc32((char *)pNvramData, sizeof(NVRAM_DATA), crc);      
    if (savedCrc != crc)
        return -1;
    
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
    const unsigned long constMacAddrIncIndex = 3;
    int nRet = 0;
    PMAC_ADDR_INFO pMai = NULL;
    PMAC_ADDR_INFO pMaiFreeNoId = NULL;
    PMAC_ADDR_INFO pMaiFreeId = NULL;
    unsigned long i = 0, ulIdxNoId = 0, ulIdxId = 0, baseMacAddr = 0;

    /* baseMacAddr = last 3 bytes of the base MAC address treated as a 24 bit integer */
    memcpy((unsigned char *) &baseMacAddr,
        &g_pMacInfo->ucaBaseMacAddr[constMacAddrIncIndex],
        NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex);
    baseMacAddr >>= 8;

    for( i = 0, pMai = g_pMacInfo->MacAddrs; i < g_pMacInfo->ulNumMacAddrs;
        i++, pMai++ )
    {
        if( ulId == pMai->ulId || ulId == MAC_ADDRESS_ANY )
        {
            /* This MAC address has been used by the caller in the past. */
            baseMacAddr = (baseMacAddr + i) << 8;
            memcpy( pucaMacAddr, g_pMacInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
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
        memcpy(pucaMacAddr, g_pMacInfo->ucaBaseMacAddr,NVRAM_MAC_ADDRESS_LEN);
        if( pMaiFreeNoId )
        {
            baseMacAddr = (baseMacAddr + ulIdxNoId) << 8;
            memcpy( pucaMacAddr, g_pMacInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMaiFreeNoId->ulId = ulId;
            pMaiFreeNoId->chInUse = 1;
        }
        else
        {
            baseMacAddr = (baseMacAddr + ulIdxId) << 8;
            memcpy( pucaMacAddr, g_pMacInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMaiFreeId->ulId = ulId;
            pMaiFreeId->chInUse = 1;
        }
    }
    else
        if( i == g_pMacInfo->ulNumMacAddrs )
            nRet = -EADDRNOTAVAIL;

    return( nRet );
} /* kerSysGetMacAddr */

int kerSysReleaseMacAddress( unsigned char *pucaMacAddr )
{
    const unsigned long constMacAddrIncIndex = 3;
    int nRet = -EINVAL;
    unsigned long ulIdx = 0;
    unsigned long baseMacAddr = 0;
    unsigned long relMacAddr = 0;

    /* baseMacAddr = last 3 bytes of the base MAC address treated as a 24 bit integer */
    memcpy((unsigned char *) &baseMacAddr,
        &g_pMacInfo->ucaBaseMacAddr[constMacAddrIncIndex],
        NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex);
    baseMacAddr >>= 8;

    /* Get last 3 bytes of MAC address to release. */
    memcpy((unsigned char *) &relMacAddr, &pucaMacAddr[constMacAddrIncIndex],
        NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex);
    relMacAddr >>= 8;

    ulIdx = relMacAddr - baseMacAddr;

    if( ulIdx < g_pMacInfo->ulNumMacAddrs )
    {
        PMAC_ADDR_INFO pMai = &g_pMacInfo->MacAddrs[ulIdx];
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
    return( (int) g_ulSdramSize );
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

static PFILE_TAG getTagFromPartition(int imageNumber)
{
    static unsigned char sectAddr1[sizeof(FILE_TAG)];
    static unsigned char sectAddr2[sizeof(FILE_TAG)];
    int blk = 0;
    UINT32 crc;
    PFILE_TAG pTag = NULL;
    unsigned char *pBase = flash_get_memptr(0);
    unsigned char *pSectAddr = NULL;

    /* The image tag for the first image is always after the boot loader.
     * The image tag for the second image, if it exists, is at one half
     * of the flash size.
     */
    if( imageNumber == 1 )
    {
        blk = flash_get_blk((int) (pBase + FLASH_LENGTH_BOOT_ROM));
        pSectAddr = sectAddr1;
    }
    else
        if( imageNumber == 2 )
        {
            blk = flash_get_blk((int) (pBase + (flash_get_total_size() / 2)));
            pSectAddr = sectAddr2;
        }

    if( blk )
    {
        memset(pSectAddr, 0x00, sizeof(FILE_TAG));
        flash_read_buf((unsigned short) blk, 0, pSectAddr, sizeof(FILE_TAG));
        crc = CRC32_INIT_VALUE;
        crc = getCrc32(pSectAddr, (UINT32)TAG_LEN-TOKEN_LEN, crc);      
        pTag = (PFILE_TAG) pSectAddr;
        if (crc != (UINT32)(*(UINT32*)(pTag->tagValidationToken)))
            pTag = NULL;
    }

    return( pTag );
}

static int getPartitionFromTag( PFILE_TAG pTag )
{
    int ret = 0;

    if( pTag )
    {
        PFILE_TAG pTag1 = getTagFromPartition(1);
        PFILE_TAG pTag2 = getTagFromPartition(2);
        int sequence = simple_strtoul(pTag->imageSequence,  NULL, 10);
        int sequence1 = (pTag1) ? simple_strtoul(pTag1->imageSequence, NULL, 10)
            : -1;
        int sequence2 = (pTag2) ? simple_strtoul(pTag2->imageSequence, NULL, 10)
            : -1;

        if( pTag1 && sequence == sequence1 )
            ret = 1;
        else
            if( pTag2 && sequence == sequence2 )
                ret = 2;
    }

    return( ret );
}

static PFILE_TAG getBootImageTag(void)
{
    PFILE_TAG pTag = NULL;
    PFILE_TAG pTag1 = getTagFromPartition(1);
    PFILE_TAG pTag2 = getTagFromPartition(2);

    if( pTag1 && pTag2 )
    {
        /* Two images are flashed. */
        int sequence1 = simple_strtoul(pTag1->imageSequence, NULL, 10);
        int sequence2 = simple_strtoul(pTag2->imageSequence, NULL, 10);
        char *p;
        char bootPartition = BOOT_LATEST_IMAGE;
        PNVRAM_DATA pNvramData;

        pNvramData = (PNVRAM_DATA)get_nvram_start_addr();

        for( p = pNvramData->szBootline; p[2] != '\0'; p++ ) 
        {
            if( p[0] == 'p' && p[1] == '=' )
            {
                bootPartition = p[2];
                break;
            }
        }

        if( bootPartition == BOOT_LATEST_IMAGE )
            pTag = (sequence2 > sequence1) ? pTag2 : pTag1;
        else /* Boot from the image configured. */
            pTag = (sequence2 < sequence1) ? pTag2 : pTag1;
    }
    else
        /* One image is flashed. */
        pTag = (pTag2) ? pTag2 : pTag1;

    return( pTag );
}

static void UpdateImageSequenceNumber( unsigned char *imageSequence )
{
    int newImageSequence = 0;
    PFILE_TAG pTag = getTagFromPartition(1);

    if( pTag )
        newImageSequence = simple_strtoul(pTag->imageSequence, NULL, 10);

    pTag = getTagFromPartition(2);
    if(pTag && simple_strtoul(pTag->imageSequence, NULL, 10) > newImageSequence)
        newImageSequence = simple_strtoul(pTag->imageSequence, NULL, 10);

    newImageSequence++;
    sprintf(imageSequence, "%d", newImageSequence);
}

static int flashFsKernelImage( int destAddr, unsigned char *imagePtr,
    int imageLen )
{
    int status = 0;
    PFILE_TAG pTag = (PFILE_TAG) imagePtr;
    int rootfsAddr = simple_strtoul(pTag->rootfsAddress, NULL, 10) + BOOT_OFFSET;
    int kernelAddr = simple_strtoul(pTag->kernelAddress, NULL, 10) + BOOT_OFFSET;
    char *p;
    char *tagFs = imagePtr;
    unsigned int baseAddr = (unsigned int) flash_get_memptr(0);
    unsigned int totalSize = (unsigned int) flash_get_total_size();
    unsigned int availableSizeOneImg = totalSize -
        ((unsigned int) rootfsAddr - baseAddr) - FLASH_RESERVED_AT_END;
    unsigned int reserveForTwoImages =
        (FLASH_LENGTH_BOOT_ROM > FLASH_RESERVED_AT_END)
        ? FLASH_LENGTH_BOOT_ROM : FLASH_RESERVED_AT_END;
    unsigned int availableSizeTwoImgs =
        (totalSize / 2) - reserveForTwoImages;
    unsigned int newImgSize = simple_strtoul(pTag->rootfsLen, NULL, 10) +
        simple_strtoul(pTag->kernelLen, NULL, 10);
    PFILE_TAG pCurTag = getBootImageTag();
    UINT32 crc = CRC32_INIT_VALUE;
    unsigned int curImgSize = 0;
    NVRAM_DATA nvramData;

    readNvramData(&nvramData);

    if( pCurTag )
    {
        curImgSize = simple_strtoul(pCurTag->rootfsLen, NULL, 10) +
            simple_strtoul(pCurTag->kernelLen, NULL, 10);
    }

    if( newImgSize > availableSizeOneImg)
    {
        printk("Illegal image size %d.  Image size must not be greater "
            "than %d.\n", newImgSize, availableSizeOneImg);
        return -1;
    }

    // If the current image fits in half the flash space and the new
    // image to flash also fits in half the flash space, then flash it
    // in the partition that is not currently being used to boot from.
    if( curImgSize <= availableSizeTwoImgs &&
        newImgSize <= availableSizeTwoImgs &&
        getPartitionFromTag( pCurTag ) == 1 )
    {
        // Update rootfsAddr to point to the second boot partition.
        int offset = (totalSize / 2) + TAG_LEN;

        sprintf(((PFILE_TAG) tagFs)->kernelAddress, "%lu",
            (unsigned long) IMAGE_BASE + offset + (kernelAddr - rootfsAddr));
        kernelAddr = baseAddr + offset + (kernelAddr - rootfsAddr);

        sprintf(((PFILE_TAG) tagFs)->rootfsAddress, "%lu",
            (unsigned long) IMAGE_BASE + offset);
        rootfsAddr = baseAddr + offset;
    }

    UpdateImageSequenceNumber( ((PFILE_TAG) tagFs)->imageSequence );
    crc = getCrc32((unsigned char *)tagFs, (UINT32)TAG_LEN-TOKEN_LEN, crc);      
    *(unsigned long *) &((PFILE_TAG) tagFs)->tagValidationToken[0] = crc;

    if( (status = kerSysBcmImageSet((rootfsAddr-TAG_LEN), tagFs,
        TAG_LEN + newImgSize)) != 0 )
    {
        printk("Failed to flash root file system. Error: %d\n", status);
        return status;
    }

    for( p = nvramData.szBootline; p[2] != '\0'; p++ )
    {
        if( p[0] == 'p' && p[1] == '=' && p[2] != BOOT_LATEST_IMAGE )
        {
            // Change boot partition to boot from new image.
            p[2] = BOOT_LATEST_IMAGE;
            writeNvramData(&nvramData);
            break;
        }
    }

    return(status);
}

PFILE_TAG kerSysImageTagGet(void)
{
    return( getBootImageTag() );
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

    switch (command) 
    {
        case BOARD_IOCTL_FLASH_WRITE:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
            {
                NVRAM_DATA tmpNvramData;
                NVRAM_DATA nvramData;

                readNvramData(&nvramData);

                switch (ctrlParms.action)
                {
                    case SCRATCH_PAD:
                        if (ctrlParms.offset == -1)
                              ret =  kerSysScratchPadClearAll();
                        else
                              ret = kerSysScratchPadSet(ctrlParms.string, ctrlParms.buf, ctrlParms.offset);
                        break;

                    case PERSISTENT:
                        ret = kerSysPersistentSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;
                
                    case NVRAM:
                        ret = kerSysNvRamSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;

                    case BCM_IMAGE_CFE:
                        if( ctrlParms.strLen <= 0 || ctrlParms.strLen > FLASH_LENGTH_BOOT_ROM )
                        {
                            printk("Illegal CFE size [%d]. Size allowed: [%d]\n",
                                ctrlParms.strLen, FLASH_LENGTH_BOOT_ROM);
                            ret = -1;
                            break;
                        }

                        ret = kerSysBcmImageSet(ctrlParms.offset + BOOT_OFFSET, ctrlParms.string, ctrlParms.strLen);

                        // Check if the new image has valid NVRAM
                        if ((readNvramData(&tmpNvramData) != 0) || (BpSetBoardId(tmpNvramData.szBoardId) != BP_SUCCESS))
                            writeNvramData(&nvramData);

                        break;
                        
                    case BCM_IMAGE_FS:
                        if( (ret = flashFsKernelImage( ctrlParms.offset, ctrlParms.string, ctrlParms.strLen)) == 0 )
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

                        if (ctrlParms.offset == 0)
                            ctrlParms.offset = FLASH_BASE;

                        ret = kerSysBcmImageSet(ctrlParms.offset, ctrlParms.string, ctrlParms.strLen);

                        // Check if the new image has valid NVRAM
                        if ((readNvramData(&tmpNvramData) != 0) || (BpSetBoardId(tmpNvramData.szBoardId) != BP_SUCCESS))
                            writeNvramData(&nvramData);

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
            ctrlParms.result = (int) g_ulPsiSize;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
            break;

        case BOARD_IOCTL_GET_SDRAM_SIZE:
            ctrlParms.result = (int) g_ulSdramSize;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
            break;

        case BOARD_IOCTL_GET_BASE_MAC_ADDRESS:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                __copy_to_user(ctrlParms.string, g_pMacInfo->ucaBaseMacAddr, NVRAM_MAC_ADDRESS_LEN);
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

        case BOARD_IOCTL_GET_CHIP_REV:
            ctrlParms.result = (int) (PERF->RevID & 0x000000FF);
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
            break;

        case BOARD_IOCTL_GET_NUM_ENET_MACS:
        {
            ETHERNET_MAC_INFO EnetInfos[BP_MAX_ENET_MACS];
            int i, numEthMacs = 0;
            if (BpGetEthernetMacInfo(EnetInfos, BP_MAX_ENET_MACS) == BP_SUCCESS) {
                for( i = 0; i < BP_MAX_ENET_MACS; i++) {
                    if (EnetInfos[i].ucPhyType != BP_ENET_NO_PHY)
                        numEthMacs++;
                }
                ctrlParms.result = numEthMacs;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,	 sizeof(BOARD_IOCTL_PARMS));   
                ret = 0;
            }
            else {
                ret = -EFAULT;
            }
            break;
        }

        case BOARD_IOCTL_GET_NUM_ENET_PORTS:
        {
            ETHERNET_MAC_INFO EnetInfos[BP_MAX_ENET_MACS];
            int i, numEthPorts = 0;
            if (BpGetEthernetMacInfo(EnetInfos, BP_MAX_ENET_MACS) == BP_SUCCESS) {
                for( i = 0; i < BP_MAX_ENET_MACS; i++) {
                    if (EnetInfos[i].ucPhyType != BP_ENET_NO_PHY)
                        numEthPorts += EnetInfos[i].numSwitchPorts;
                }
                ctrlParms.result = numEthPorts;
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
/*
        case BOARD_IOCTL_SET_MONITOR_FD:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                int fput_needed = 0;

                g_monitor_file = fget_light( ctrlParms.offset, &fput_needed );
                if( g_monitor_file ) {
                    g_monitor_task = current;
                    g_orig_fop_poll = g_monitor_file->f_op->poll;
                    g_monitor_file->f_op->poll = kerSysMonitorPollHook;
                }
            }
            break;*/

        case BOARD_IOCTL_WAKEUP_MONITOR_TASK:
            kerSysWakeupMonitorTask();
            break;

        case BOARD_IOCTL_GET_VCOPE_GPIO:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                ret = ((ctrlParms.result = BpGetVcopeGpio(ctrlParms.offset)) != BP_NOT_DEFINED) ? 0 : -EFAULT;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            }
            else {
                ret = -EFAULT;  
                ctrlParms.result = BP_NOT_DEFINED;
            }

            break;

        case BOARD_IOCTL_SET_CS_PAR: 
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                ret = ConfigCs(&ctrlParms);
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            } 
            else {
                ret = -EFAULT;  
            }
            break;

        case BOARD_IOCTL_SET_GPIO:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                SetGpio(ctrlParms.strLen, ctrlParms.offset);
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            } 
            else {
                ret = -EFAULT;  
            }
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

static Bool sesBtn_pressed(void)
{
    if ((sesBtn_irq >= INTERRUPT_ID_EXTERNAL_0) && (sesBtn_irq <= INTERRUPT_ID_EXTERNAL_3)) {
        if (!(PERF->ExtIrqCfg & (1 << (sesBtn_irq - INTERRUPT_ID_EXTERNAL_0 + EI_STATUS_SHFT)))) {
            return 1;
        }
    }
#if defined(CONFIG_BCM96358)
    else if ((sesBtn_irq >= INTERRUPT_ID_EXTERNAL_4) || (sesBtn_irq <= INTERRUPT_ID_EXTERNAL_5)) {
        if (!(PERF->ExtIrqCfg1 & (1 << (sesBtn_irq - INTERRUPT_ID_EXTERNAL_4 + EI_STATUS_SHFT)))) {
            return 1;
        }
    }
#endif
    return 0;
}

static irqreturn_t sesBtn_isr(int irq, void *dev_id)
{   
    if (sesBtn_pressed()){
        wake_up_interruptible(&g_board_wait_queue);
        return IRQ_RETVAL(1);
    } else {
        return IRQ_RETVAL(0);    	
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
    	    
    sesBtn_irq = map_external_irq (sesBtn_irq) ;
    		
    if (BcmHalMapInterrupt((FN_HANDLER)sesBtn_isr, context, sesBtn_irq)) {
    	printk("SES: Interrupt mapping failed\n");
    }    
    BcmHalInterruptEnable(sesBtn_irq);
}


static unsigned int sesBtn_poll(struct file *file, struct poll_table_struct *wait)
{

    if (sesBtn_pressed()){
        return POLLIN;
    }	
    return 0;
}

static ssize_t sesBtn_read(struct file *file,  char __user *buffer, size_t count, loff_t *ppos)
{
    volatile unsigned int event=0;
    ssize_t ret=0;	

    if(!sesBtn_pressed()){
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
    if( BpGetWirelessSesLedGpio(&sesLed_gpio) == BP_SUCCESS )
    { 
        printk("SES: LED GPIO 0x%x is enabled\n", sesLed_gpio);    
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

#if defined(CONFIG_BCM96348) || defined(CONFIG_BCM96338)
/* The BCM6348 cycles per microsecond is really variable since the BCM6348
 * MIPS speed can vary depending on the PLL settings.  However, an appoximate
 * value of 120 will still work OK for the test being done.
 */
#define	CYCLE_PER_US	120
#elif defined(CONFIG_BCM96358)
#define	CYCLE_PER_US	150
#endif
#define	DG_GLITCH_TO	(100*CYCLE_PER_US)
 
static void __init kerSysDyingGaspMapIntr()
{
    unsigned long ulIntr;
    	
    if( BpGetAdslDyingGaspExtIntr( &ulIntr ) == BP_SUCCESS ) {
		BcmHalMapInterrupt((FN_HANDLER)kerSysDyingGaspIsr, 0, INTERRUPT_ID_DG);
		BcmHalInterruptEnable( INTERRUPT_ID_DG );
    }
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

    return 1;
}

static void kerSysDyingGaspShutdown( void )
{
    kerSysSetWdTimer(1000000);
#if defined(CONFIG_BCM96348)
    PERF->blkEnables &= ~(EMAC_CLK_EN | USBS_CLK_EN | USBH_CLK_EN | SAR_CLK_EN);
#elif defined(CONFIG_BCM96358) 
    PERF->blkEnables &= ~(EMAC_CLK_EN | USBS_CLK_EN | SAR_CLK_EN);
#endif
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
static irqreturn_t kerSysDyingGaspIsr(int irq, void * dev_id)
#else
static unsigned int kerSysDyingGaspIsr(void)
#endif
{	
    struct list_head *pos;
    CB_DGASP_LIST *tmp = NULL, *dsl = NULL;	

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

        // If power is going down, nothing should continue!

        while (1)
            ;
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

static int ConfigCs (BOARD_IOCTL_PARMS *parms)
{
    int                     retv = 0;
#if !defined(CONFIG_BCM96338)
    int                     cs, flags;
    cs_config_pars_t        info;

    if (copy_from_user(&info, (void*)parms->buf, sizeof(cs_config_pars_t)) == 0) 
    {
        cs = parms->offset;

        MPI->cs[cs].base = ((info.base & 0x1FFFE000) | (info.size >> 13));	

        if ( info.mode == EBI_TS_TA_MODE )     // syncronious mode
            flags = (EBI_TS_TA_MODE | EBI_ENABLE);
        else
        {
            flags = ( EBI_ENABLE | \
                (EBI_WAIT_STATES  & (info.wait_state << EBI_WTST_SHIFT )) | \
                (EBI_SETUP_STATES & (info.setup_time << EBI_SETUP_SHIFT)) | \
                (EBI_HOLD_STATES  & (info.hold_time  << EBI_HOLD_SHIFT )) );
        }
        MPI->cs[cs].config = flags;
        parms->result = BP_SUCCESS;
        retv = 0;
    }
    else
    {
        retv -= EFAULT;
        parms->result = BP_NOT_DEFINED; 
    }
#endif
    return( retv );
}

static void SetGpio(int gpio, GPIO_STATE_t state)
{
    int active_low = (gpio & BP_ACTIVE_LOW) ? 1 : 0;
    unsigned long gpio_mask = GPIO_NUM_TO_MASK(gpio);
    volatile unsigned long *gpio_io_reg = &GPIO->GPIOio;
    volatile unsigned long *gpio_dir_reg = &GPIO->GPIODir;
    
#if !defined (CONFIG_BCM96338)
    if( gpio >= 32 )
    {
        gpio_mask = GPIO_NUM_TO_MASK_HIGH(gpio);
        gpio_io_reg = &GPIO->GPIOio_high;
        gpio_dir_reg = &GPIO->GPIODir_high;
    }
#endif 

    *gpio_dir_reg |= gpio_mask;

    if(state == GPIO_HIGH && active_low == 0)
        *gpio_io_reg |= gpio_mask;
    else
        *gpio_io_reg &= ~gpio_mask;
}


static irqreturn_t reset_isr(int irq, void *dev_id)
{
    printk("\n*** Restore to Factory Default Setting ***\n\n");
    kerSysPersistentSet( "Reset Persistent", strlen("Reset Persistent"), 0 );
    kerSysMipsSoftReset();
    return 0;
}

#if defined(WIRELESS)
/***********************************************************************
 * Function Name: kerSysScreenPciDevices
 * Description  : Screen Pci Devices before loading modules
 ***********************************************************************/
static void __init kerSysScreenPciDevices(void)
{		
   unsigned short wlFlag;		

   if((BpGetWirelessFlags(&wlFlag) == BP_SUCCESS) && (wlFlag & BP_WLAN_EXCLUDE_ONBOARD)) {
     /* 
      * scan all available pci devices and delete on board BRCM wireless device
      * if external slot presents a BRCM wireless device
      */	      
      int foundPciAddOn = 0;
      struct pci_dev *pdevToExclude = NULL;
      struct pci_dev *dev = NULL;
   
      while((dev=pci_find_device(PCI_ANY_ID, PCI_ANY_ID, dev))!=NULL) {
      	 printk("kerSysScreenPciDevices: 0x%x:0x%x:(slot %d) detected\n", dev->vendor, dev->device, PCI_SLOT(dev->devfn));      	         	
         if((dev->vendor == BRCM_VENDOR_ID) && ((dev->device & 0xff00) == BRCM_WLAN_DEVICE_IDS)) {
            if(PCI_SLOT(dev->devfn) != WLAN_ONBOARD_SLOT)
               foundPciAddOn++;
            else
               pdevToExclude = dev;            	      	 
         }
      }
      
      if(foundPciAddOn && pdevToExclude) {
         printk("kerSysScreenPciDevices: 0x%x:0x%x:(onboard) deleted\n", pdevToExclude->vendor, pdevToExclude->device);
         pci_remove_bus_device(pdevToExclude);	
      }	
   }   
}
#endif

/***************************************************************************
 * MACRO to call driver initialization and cleanup functions.
 ***************************************************************************/
module_init( brcm_board_init );
module_exit( brcm_board_cleanup );

EXPORT_SYMBOL(dumpaddr);
EXPORT_SYMBOL(kerSysGetMacAddress);
EXPORT_SYMBOL(kerSysReleaseMacAddress);
EXPORT_SYMBOL(kerSysGetSdramSize);
EXPORT_SYMBOL(kerSysLedCtrl);
EXPORT_SYMBOL(kerSysLedRegisterHwHandler);
EXPORT_SYMBOL(BpGetBoardIds);
EXPORT_SYMBOL(BpGetGPIOverlays);
EXPORT_SYMBOL(BpGetEthernetMacInfo);
EXPORT_SYMBOL(BpGetRj11InnerOuterPairGpios);
EXPORT_SYMBOL(BpGetVoipResetGpio);
EXPORT_SYMBOL(BpGetVoipIntrGpio);
EXPORT_SYMBOL(BpGetRtsCtsUartGpios);
EXPORT_SYMBOL(BpGetAdslLedGpio);
EXPORT_SYMBOL(BpGetAdslFailLedGpio);
EXPORT_SYMBOL(BpGetHpnaLedGpio);
EXPORT_SYMBOL(BpGetWanDataLedGpio);
EXPORT_SYMBOL(BpGetPppLedGpio);
EXPORT_SYMBOL(BpGetPppFailLedGpio);
EXPORT_SYMBOL(BpGetVoipLedGpio);
EXPORT_SYMBOL(BpGetPotsResetGpio);
EXPORT_SYMBOL(BpGetSlicGpio);
EXPORT_SYMBOL(BpGetPotsLedGpio);
EXPORT_SYMBOL(BpGetVoip2LedGpio);
EXPORT_SYMBOL(BpGetVoip1LedGpio);
EXPORT_SYMBOL(BpGetAdslDyingGaspExtIntr);
EXPORT_SYMBOL(BpGetVoipExtIntr);
EXPORT_SYMBOL(BpGetHpnaExtIntr);
EXPORT_SYMBOL(BpGetHpnaChipSelect);
EXPORT_SYMBOL(BpGetVoipChipSelect);
EXPORT_SYMBOL(BpGetWirelessSesExtIntr);
EXPORT_SYMBOL(BpGetWirelessSesLedGpio);
EXPORT_SYMBOL(BpGetWirelessFlags);
EXPORT_SYMBOL(BpUpdateWirelessSromMap);
EXPORT_SYMBOL(BpGetSlicType);
EXPORT_SYMBOL(BpGetDAAType);
EXPORT_SYMBOL(BpGetSecAdslLedGpio);
EXPORT_SYMBOL(BpGetSecAdslFailLedGpio);
EXPORT_SYMBOL(BpGetBondingSecondaryResetGpio);
EXPORT_SYMBOL(kerSysRegisterDyingGaspHandler);
EXPORT_SYMBOL(kerSysDeregisterDyingGaspHandler);
EXPORT_SYMBOL(kerSysGetCycleCount);
EXPORT_SYMBOL(kerSysSetWdTimer);
EXPORT_SYMBOL(kerSysWakeupMonitorTask);
