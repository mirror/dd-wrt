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
#include <linux/clocksource.h>
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
#include <linux/platform_device.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>
//#include <linux/serialP.h>
#include <linux/serial_reg.h>
#include <asm/serial.h>

#include <asm/arch/serial.h>

#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "boardEnv/mvBoardEnvLib.h"
#include "mvDebug.h"
#include "mvSysHwConfig.h"
#include "pex/mvPexRegs.h"
#include "cntmr/mvCntmr.h"
#include "gpp/mvGpp.h"
#if defined(CONFIG_MV_INCLUDE_CESA)
#include "cesa/mvCesa.h"
#endif
#ifdef CONFIG_MV_INCLUDE_IDMA
#   include "ctrlEnv/sys/mvSysIdma.h"
#endif

/* for debug putstr */
#include <asm/arch/uncompress.h> 
static char arr[256];

#ifdef MV_INCLUDE_EARLY_PRINTK
void mv_early_printk(char *fmt,...)
{
	va_list args;
	va_start(args, fmt);
	vsprintf(arr,fmt,args);
	va_end(args);
	putstr(arr);
}
#endif

#if (CONFIG_MV_GPP_MAX_PINS == 32)
extern unsigned int irq_int_type[];
#else
extern unsigned int irq_int_type[];
#endif

extern void __init mv_map_io(void);
extern void __init mv_init_irq(void);
extern struct sys_timer mv_timer;
extern MV_CPU_DEC_WIN* mv_sys_map(void);
#if defined(CONFIG_MV_INCLUDE_CESA)
extern u32 mv_crypo_base_get(void);
#endif
unsigned int mv_orion_ver = 0x0;
unsigned int mv_orion_has_vfp = 0x0;
unsigned int support_wait_for_interrupt = 0x1;

u32 mvTclk = 166666667;
u32 mvSysclk = 200000000;
u32 mvIsUsbHost = 1;


u32 overEthAddr = 0;
u8	mvMacAddr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
extern MV_U32 gBoardId; 
extern unsigned int elf_hwcap;
 
static int __init parse_tag_mv_uboot(const struct tag *tag)
{
    	unsigned int mvUbootVer = 0;
 
	mvUbootVer = tag->u.mv_uboot.uboot_version;
	mvIsUsbHost = tag->u.mv_uboot.isUsbHost;

        printk("Using UBoot passing parameters structure\n");
  
	gBoardId =  (mvUbootVer & 0xff);
		
	overEthAddr = tag->u.mv_uboot.overEthAddr;        
	memcpy(mvMacAddr, tag->u.mv_uboot.macAddr, 6);
    	return 0;
}
                                                                                                                             
__tagtable(ATAG_MV_UBOOT, parse_tag_mv_uboot);

#ifdef CONFIG_MV_INCLUDE_CESA
unsigned char*  mv_sram_usage_get(int* sram_size_ptr)
{
    int used_size = 0;

#if defined(CONFIG_MV_CESA)
    used_size = sizeof(MV_CESA_SRAM_MAP);
#endif

    if(sram_size_ptr != NULL)
        *sram_size_ptr = _8K - used_size;

    return (char *)(mv_crypo_base_get() + used_size);
}
#endif

#ifdef CONFIG_MV_INCLUDE_IDMA  /* TBD - should be change */
/* Return number of free IDMA engines */
int mv_idma_usage_get(int* free_map)
{
    int idma, free;

    free = MV_IDMA_MAX_CHAN;
    for(idma=0; idma<MV_IDMA_MAX_CHAN; idma++)
    {
        free_map[idma] = 1;
    }
#if defined(CONFIG_MV_CESA)
    /* CESA uses Idma channel #0. Idma engine #1 used when CESA use two channels */
    for(idma=0; idma<MV_CESA_MAX_CHAN; idma++)
    {
        free_map[idma] = 0;
        free--;
    }
#endif /* CONFIG_MV_CESA */

#ifdef CONFIG_MV_DMA_COPYUSER
    free_map[2] = 0;
    free_map[3] = 0;
    free -= 2;
#endif /* CONFIG_MV_DMA_COPYUSER */

    return free;
}
#endif /* CONFIG_MV_INCLUDE_IDMA */


void print_board_info(void)
{
    char name_buff[50];
    printk("\n  Marvell Development Board (LSP Version %s)",LSP_VERSION);

    mvBoardNameGet(name_buff);
    printk("-- %s ",name_buff);

    mvCtrlModelRevNameGet(name_buff);
    printk(" Soc: %s",  name_buff);
#if defined(MV_CPU_LE)
	printk(" LE");
#else
	printk(" BE");
#endif
    printk("\n\n");
    printk(" Detected Tclk %d and SysClk %d \n",mvTclk, mvSysclk);
}


/*Platform devices list*/
/*****************************************************************************
 * UART
 ****************************************************************************/
static struct resource mv_uart_resources[] = {
	{
		.start		= PORT0_BASE,
		.end		= PORT0_BASE + 0xff,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start          = IRQ_UART0,
		.end            = IRQ_UART0,
		.flags          = IORESOURCE_IRQ,
	},
	{
		.start		= PORT1_BASE,
		.end		= PORT1_BASE + 0xff,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start          = IRQ_UART1,
		.end            = IRQ_UART1,
		.flags          = IORESOURCE_IRQ,
	},
};

static struct plat_serial8250_port mv_uart_data[] = {
	{
		.mapbase	= PORT0_BASE,
		.membase	= (char *)PORT0_BASE,
		.irq		= IRQ_UART0,
		.flags		= UPF_SKIP_TEST | UPF_BOOT_AUTOCONF,
		.iotype		= UPIO_MEM,
		.regshift	= 2,
	},
	{
		.mapbase	= PORT1_BASE,
		.membase	= (char *)PORT1_BASE,
		.irq		= IRQ_UART1,
		.flags		= UPF_SKIP_TEST | UPF_BOOT_AUTOCONF,
		.iotype		= UPIO_MEM,
		.regshift	= 2,
	},
	{ },
};

static struct platform_device mv_uart = {
	.name			= "serial8250",
	.id			= PLAT8250_DEV_PLATFORM,
	.dev			= {
		.platform_data	= mv_uart_data,
	},
	.num_resources		= ARRAY_SIZE(mv_uart_resources),
	.resource		= mv_uart_resources,
};


static void serial_initialize(void)
{
	mv_uart_data[0].uartclk = mv_uart_data[1].uartclk = mvTclk;
	platform_device_register(&mv_uart);
}

static void __init mv_vfp_init(void)
{

#if defined CONFIG_VFP_FASTVFP
        printk("VFP initialized to Run Fast Mode.\n");
#endif
}

#if defined(MV_88F6183)

static struct resource mvsdmmc_resources[] = {
	[0] = {
		.start	= INTER_REGS_BASE + 0x80000,
		.end	= INTER_REGS_BASE + 0x80000 + SZ_1K -1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= SDIO_IRQ_NUM,
		.end	= SDIO_IRQ_NUM,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.start	= NR_IRQS,	/* should obtained from board information*/
		.end	= NR_IRQS,	/* should obtained from board information */
		.flags	= IORESOURCE_IRQ,
	}

};

static u64 mvsdmmc_dmamask = 0xffffffffUL;

static struct platform_device mvsdmmc_device = {
	.name		= "mvsdmmc",
	.id		= -1,
	.dev		= {
		.dma_mask = &mvsdmmc_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(mvsdmmc_resources),
	.resource	= mvsdmmc_resources,
};

static struct platform_device *devices[] __initdata = {
	&mvsdmmc_device
};

#endif /* #if defined(MV_88F6183) */
static void __init mv_init(void)
{

	unsigned int temp;
        /* init the Board environment */
        if (mvBoardIdGet() != RD_88F6082_MICRO_DAS_NAS)  /* excluded for HDD power problem - to be fixed */
		mvBoardEnvInit();

        /* init the controller environment */
        if( mvCtrlEnvInit() ) {
            printk( "Controller env initialization failed.\n" );
            return;
        }

	if(mvBoardIdGet() == RD_88F5181_POS_NAS) {
		temp = MV_REG_READ(GPP_DATA_OUT_REG(0));
		temp &= ~(1 << 0x5);

		/* for host mode should be set to 0 */
		if(!mvIsUsbHost) {
			temp |= (1 << 0x5);
		}
        	MV_REG_WRITE(GPP_DATA_OUT_REG(0), temp);
	}

	/* Init the CPU windows setting and the access protection windows. */
	if( mvCpuIfInit(mv_sys_map()) ) {

		printk( "Cpu Interface initialization failed.\n" );
		return;
	}

    	/* Init Tclk & SysClk */
    	mvTclk = mvBoardTclkGet();
   	mvSysclk = mvBoardSysClkGet();

	printk("Sys Clk = %d, Tclk = %d\n",mvSysclk ,mvTclk  );
	

    	if ((mvCtrlModelGet() == MV_5281_DEV_ID) || (mvCtrlModelGet() == MV_1281_DEV_ID)
			|| (mvCtrlModelGet() == MV_6183_DEV_ID))
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

#ifdef CONFIG_JTAG_DEBUG
            support_wait_for_interrupt = 0; /*  for Lauterbach */
#endif
	mv_vfp_init();
	elf_hwcap &= ~HWCAP_JAVA;

   	serial_initialize();

	/* At this point, the CPU windows are configured according to default definitions in mvSysHwConfig.h */
	/* and cpuAddrWinMap table in mvCpuIf.c. Now it's time to change defaults for each platform.         */
	mvCpuIfAddDecShow();


#if defined(CONFIG_MTD_PHYSMAP)
	mv_mtd_initialize();
#endif
    	print_board_info();

#ifdef CONFIG_MV_INCLUDE_IDMA
    	mvDmaInit();
#endif

#if defined(MV_88F6183)
	mvsdmmc_resources[2].end = mvBoardSDIOGpioPinGet() + IRQ_GPP_START;
	mvsdmmc_resources[2].start = mvBoardSDIOGpioPinGet() + IRQ_GPP_START;
	irq_int_type[mvBoardSDIOGpioPinGet()] = GPP_IRQ_TYPE_CHANGE_LEVEL;

	platform_add_devices(devices, ARRAY_SIZE(devices));
#endif

    return;
}


MACHINE_START(FEROCEON_ORION ,"Feroceon-Orion")
    /* MAINTAINER("MARVELL") */
    .phys_io = 0xf1000000,
    .io_pg_offst = ((0xf1000000) >> 18) & 0xfffc,
    .boot_params = 0x00000100,
    .map_io = mv_map_io,
    .init_irq = mv_init_irq,
    .timer = &mv_timer,
    .init_machine = mv_init,
MACHINE_END

