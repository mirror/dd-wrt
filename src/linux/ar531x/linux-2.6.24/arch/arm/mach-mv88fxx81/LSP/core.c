/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sysdev.h>
#include <asm/mach/time.h>
#if defined(CONFIG_MTD_PHYSMAP) 
#include <linux/mtd/physmap.h>
#endif
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/mach-types.h>

#include <asm/mach/arch.h>
#include <asm/mach/flash.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>
#include <asm/arch/system.h>
#include <asm/arch/orion_ver.h>

#include <asm/vfp.h>

#include <linux/tty.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/serialP.h>
#include <linux/serial_reg.h>
#include <asm/serial.h>

#include <asm/arch/serial.h>

#include "mvCtrlEnvLib.h"
#include "mvCpuIf.h"
#include "mvBoardEnvLib.h"
#include "mvDebug.h"

#ifdef CONFIG_ARCH_MV88f5181
#   include "mvIdma.h"
#endif /* CONFIG_ARCH_MV88f5181 */

#ifdef CONFIG_MV_CESA
#include "mvCesa.h"
#include "mvMD5.h"
#include "mvSHA1.h"
#endif /* CONFIG_MV_CESA */

extern void __init mv_map_io(void);
extern void __init mv_init_irq(void);

unsigned int mv_orion_ver = 0x0;
unsigned int support_wait_for_interrupt = 0x1;

#ifdef CONFIG_UBOOT_STRUCT

u32 mvTclk = 166000000;
u32 mvSysclk = 200000000;
u32 mvIsUsbHost = 1;
u32 overEthAddr = 0;
extern MV_U32 gBoardId; 
 
static int __init parse_tag_mv_uboot(const struct tag *tag)
{
    unsigned int mvUbootVer = 0;

        mvTclk = tag->u.mv_uboot.tclk;
    mvSysclk = tag->u.mv_uboot.sysclk;
    mvUbootVer = tag->u.mv_uboot.uboot_version;
    mvIsUsbHost = tag->u.mv_uboot.isUsbHost;

        printk("Using UBoot passing parameters structure\n");
    printk("Sys Clk = %d, Tclk = %d\n",mvSysclk ,mvTclk  );

        /* check the u-boot version */
        if( (TEST_UBOOT_VER & 0xffffff00) != (mvUbootVer & 0xffffff00) ) {
                printk( "\n\n- Warning - This LSP release was tested only with U-Boot release %d.%d.%d \n\n", \
                TEST_UBOOT_VER >> 24, (TEST_UBOOT_VER >> 16) & 0xff,(TEST_UBOOT_VER >> 8) & 0xff );
        }

        /* If POS NAS version */
        if((mvUbootVer & 0xff) == 0xab)
        {
                gBoardId = RD_88F5181_POS_NAS;
        }
        else if ((mvUbootVer & 0xff) == 0xbc)
        {
            gBoardId = RD_88F5181_VOIP; 
        }
        else if((mvUbootVer & 0xff) == 0xcd)
        {
                gBoardId = DB_88F5X81_DDR2;
        }
        else if((mvUbootVer & 0xff) == 0xce)
        {
                gBoardId = DB_88F5X81_DDR1;
        }
    #if	defined(MV_88F5182) || defined(RD_DB_88F5181L) || defined(MV_88W8660)
        else
        {
                gBoardId =  (mvUbootVer & 0xff);
        }
    #endif
  
        
        if( mvUbootVer > 0x01040100 )  /* releases after 1.4.2 */
        {
                overEthAddr = tag->u.mv_uboot.overEthAddr;
        }
        return 0;
}
                                                                                                                             
__tagtable(ATAG_MV_UBOOT, parse_tag_mv_uboot);
#else
u32 mvTclk = 166000000;
u32 mvSysclk = 200000000;
#ifdef CONFIG_MV_USB_HOST
    u32 mvIsUsbHost = 1;
#else
    u32 mvIsUsbHost = 0;
#endif
#endif

EXPORT_SYMBOL(mvTclk);
EXPORT_SYMBOL(mvSysclk);
EXPORT_SYMBOL(mvCtrlModelGet);
EXPORT_SYMBOL(mvOsIoUncachedMalloc);
EXPORT_SYMBOL(mvOsIoUncachedFree);
EXPORT_SYMBOL(mvOsIoCachedMalloc);
EXPORT_SYMBOL(mvOsIoCachedFree);
EXPORT_SYMBOL(mvOsCacheFlush);
EXPORT_SYMBOL(mvDebugMemDump);
EXPORT_SYMBOL(mvHexToBin);
EXPORT_SYMBOL(mvBinToHex);
EXPORT_SYMBOL(mvIsUsbHost);
EXPORT_SYMBOL(mvCtrlUsbMaxGet);
EXPORT_SYMBOL(mvCtrlModelRevGet);

#ifdef CONFIG_MV_CESA
EXPORT_SYMBOL(mvCesaInit);
EXPORT_SYMBOL(mvCesaSessionOpen);
EXPORT_SYMBOL(mvCesaSessionClose);
EXPORT_SYMBOL(mvCesaAction);
EXPORT_SYMBOL(mvCesaReadyGet);
EXPORT_SYMBOL(mvCesaChanInProcessGet);
EXPORT_SYMBOL(mvCesaCopyFromMbuf);
EXPORT_SYMBOL(mvCesaCopyToMbuf);
EXPORT_SYMBOL(mvCesaMbufCopy);
EXPORT_SYMBOL(mvCesaCryptoIvSet);
EXPORT_SYMBOL(mvMD5);
EXPORT_SYMBOL(mvSHA1);

EXPORT_SYMBOL(mvCesaDebugCacheIdx);
EXPORT_SYMBOL(mvCesaDebugQueue);
EXPORT_SYMBOL(mvCesaDebugSram);
EXPORT_SYMBOL(mvCesaDebugSAD);
EXPORT_SYMBOL(mvCesaDebugStatus);
EXPORT_SYMBOL(mvCesaDebugMbuf);
EXPORT_SYMBOL(mvCesaDebugChan);
EXPORT_SYMBOL(mvCesaDebugSA);
#endif /* CONFIG_MV_CESA */

void print_board_info(void)
{
    char name_buff[50];
#ifdef CONFIG_ARCH_MV88f1181 
    printk("\n  Marvell Development Board (LSP Version %s)\n\n",LSP_VERSION);
#else
    printk("\n  Marvell Development Board (LSP Version %s)",LSP_VERSION);

    mvBoardNameGet(name_buff);
    printk("-- %s ",name_buff);
    printk("\n\n");
#endif
    printk(" Detected Tclk %d and SysClk %d \n",mvTclk, mvSysclk);
}
extern int early_serial_setup(struct uart_port *port);

/* Add the uart to the console list (ttyS0) . */
static void serial_initialize(void)
{
        struct uart_port        serial_req;

        memset(&serial_req, 0, sizeof(serial_req));
        serial_req.line = 0;
        serial_req.uartclk = BASE_BAUD * 16;
        serial_req.irq = IRQ_UART0;
        serial_req.flags = STD_COM_FLAGS;
        serial_req.iotype = SERIAL_IO_MEM;
        serial_req.membase = (char *)PORT0_BASE;
        serial_req.regshift = 2;

        if (early_serial_setup(&serial_req) != 0) {
                printk("Early serial init of port 0 failed\n");
        }

        return;
}
#if defined(CONFIG_MTD_PHYSMAP) 
static void mv_mtd_initialize(void)
{
    u32 size = 0, boardId = 0;
    u32 base = DEVICE_CS1_BASE;
    u32 bankwidth = mvBoardGetDeviceBusWidth(1,BOARD_DEV_NOR_FLASH) / 8;
    
    boardId = mvBoardIdGet();
    
    switch(boardId) {
        case(RD_88F5181_VOIP):
            size = _8M;
            break;
        case(DB_88F5X81_DDR2):
        case(DB_88F5X81_DDR1):
            size = _32M;
            break;
        case(RD_88F5181_POS_NAS):
            size = 0;
            break;
        case(DB_88F5181_5281_DDR1):
        case(DB_88F5181_5281_DDR2):
            size = _16M;
            break;  
        case(DB_88F5182_DDR2):
        case(DB_88W8660_DDR2):
            size = _16M;
            break;
        case(RD_88F5182_2XSATA):
            size = _256K;
            base = mvCpuIfTargetWinBaseLowGet(DEV_BOOCS);
	    bankwidth = mvBoardGetDeviceBusWidth(0,BOARD_DEV_NOR_FLASH) / 8;
            break;
        case(RD_88F5181L_VOIP_FE):
        case(RD_88F5181L_VOIP_GE):
#ifdef CONFIG_SCM_SUPPORT            
            size = _8M;
#else            
            size = mvCpuIfTargetWinSizeGet(DEV_BOOCS);
#endif
            base =  mvCpuIfTargetWinBaseLowGet(DEV_BOOCS);
            bankwidth = mvBoardGetDeviceBusWidth(0,BOARD_DEV_NOR_FLASH) / 8;
            printk("Flash bankwidth %d, base %x, size %x\n", bankwidth, base, size); 
            break;
  
        default:
            printk(" %s Error : Unknown board \n", __FUNCTION__);
            size = 0;
    }
    if(size == 0)
        return;

    physmap_configure(base, size, bankwidth, NULL);

    return;
}
#endif

static void __init mv_init(void)
{
        /* init the Board environment */
        mvBoardEnvInit();

        /* init the controller environment */
        if( mvCtrlEnvInit() )
        {
                printk( "Controller env initialization failed.\n" );
                return;
        }

        /* Init the CPU windows setting and the access protection windows. */
        if( mvCpuIfInit() )
        {
                printk( "Cpu Interface initialization failed.\n" );
                return;
        }
    	/* adjust CPU windows for PCI IO ramap */
    	if( MV_OK != mvAhbToMbusWinTargetSwap(PEX0_IO , PEX0_MEM) )
    	{
                printk( "SWAP windows failed.\n" );
                return; 
    	}
#ifdef CONFIG_ARCH_MV88f5181
        if( MV_OK != mvAhbToMbusWinTargetSwap(PCI0_IO , PCI0_MEM) )
        {
                printk( "SWAP windows failed.\n" );
                return;
        }
#else
        if( MV_OK != mvAhbToMbusWinTargetSwap(PEX1_IO , PEX1_MEM) )
        {
                printk( "SWAP windows failed.\n" );
                return;
        }
#endif

    	if(mvCtrlModelGet() == MV_5281_DEV_ID)
        	mv_orion_ver = MV_ORION2; /* Orion II */    
    	else
        	mv_orion_ver = MV_ORION1; /* Orion I */ 

        /* Implement workaround for FEr# CPU-C16: Wait for interrupt command */ 
        /* is not processed properly, the workaround is not to use this command */
        /* the erratum is relevant for 5281 devices with revision less than C0 */
        if((mvCtrlModelGet() == MV_5281_DEV_ID)
         && (mvCtrlRevGet() < MV_5281_C0_REV))
        {
            support_wait_for_interrupt = 0;
        }

#ifdef CONFIG_VFP_RUN_FAST_MODE
#define vfpreg(_vfp_) #_vfp_

#define fmrx(_vfp_) ({			\
	u32 __v;			\
	asm("mrc%? p10, 7, %0, " vfpreg(_vfp_) ", cr0, 0 @ fmrx	%0, " #_vfp_	\
	    : "=r" (__v));		\
	__v;				\
 })

#define fmxr(_vfp_,_var_)		\
	asm("mcr%? p10, 7, %0, " vfpreg(_vfp_) ", cr0, 0 @ fmxr	" #_vfp_ ", %0"	\
	   : : "r" (_var_))

        if( mv_orion_ver == MV_ORION2)
        {
            /* init and Enable VFP to Run Fast Mode */
            printk("VFP initialized to Run Fast Mode.\n");
            /* Enable */
            fmxr(FPEXC, fmrx(FPEXC) | FPEXC_ENABLE);

            /* Run Fast Mode */
            fmxr(FPSCR, fmrx(FPSCR) | (FPSCR_DEFAULT_NAN | FPSCR_FLUSHTOZERO));

        }
#endif

#ifndef CONFIG_UBOOT_STRUCT
    	mvTclk = mvBoardTclkGet();
    	mvSysclk = mvBoardSysClkGet();
#endif
    	serial_initialize();

#ifdef RD_DB_88F5181L
        /* in voip RD2 boards map bootcs flash to 0xf4000000*/

	if( MV_OK != mvCpuIfTargetWinEnable(DEVICE_CS1 , 0) )
	{
                printk( "disable win failed.\n" );
                return;
	}
        {
            MV_CPU_DEC_WIN bootcs_win;

            if( MV_OK != mvCpuIfTargetWinGet(DEV_BOOCS, &bootcs_win) )
	    {
                printk( "get window failed.\n" );
                return;
	    }

            bootcs_win.addrWin.baseLow = 0xf4000000;
            bootcs_win.enable = 1;
            bootcs_win.addrWin.size = _16M;
            if( MV_OK != mvCpuIfTargetWinSet(DEV_BOOCS, &bootcs_win) )
	    {
                printk( "set window failed.\n" );
                return;
	    }
        }
#endif /*RD_DB_88F5181L*/

#if defined(CONFIG_MTD_PHYSMAP)
    	mv_mtd_initialize();
#endif

    	print_board_info();

#ifdef CONFIG_MV_CESA
	{
	MV_CPU_DEC_WIN crypt_win;

	if( MV_OK != mvCpuIfTargetWinGet(DEVICE_CS2, &crypt_win) )
	{
                printk( "get window failed.\n" );
                return;
	}
	crypt_win.addrWin.baseLow = CRYPT_ENG_BASE;
	crypt_win.addrWin.baseHigh = 0;
	crypt_win.addrWin.size = CRYPT_ENG_SIZE;
	crypt_win.enable = 1;
	if( MV_OK != mvCpuIfTargetWinEnable(DEVICE_CS2, 0) )
	{
                printk( "disable win failed.\n" );
                return;
	}
	if( MV_OK != mvCpuIfTargetWinSet(CRYPT_ENG, &crypt_win) )
	{
                printk( "set window failed.\n" );
                return;
	}
	mvDmaInit();
	}
#endif /* CONFIG_MV_CESA */

    return;
}

#include "asm/arch/time.h"

static void __init mv_init_timer(void)
{
        mv_time_init();
}

static struct sys_timer mv_timer = {
        .init           = mv_init_timer,
        .offset         = mv_gettimeoffset,
};

MACHINE_START(AVILA, "MV-88fxx81")
	/* Maintainer: MontaVista Software, Inc. */
	.phys_io	= 0xf1000000,
	.io_pg_offst	= ((0xf1100000) >> 18) & 0xfffc,
	.map_io		= mv_map_io,
	.init_irq	= mv_init_irq,
	.timer		= &mv_timer,
	.boot_params	= 0x00000100,
	.init_machine	= mv_init,
MACHINE_END



