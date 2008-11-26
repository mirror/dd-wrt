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

#include <linux/tty.h>
#include <linux/platform_device.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>
#include <linux/serial_reg.h>
#include <asm/serial.h>

#include <asm/arch/serial.h>
#include <asm/string.h>

#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "cpu/mvCpu.h"
#include "boardEnv/mvBoardEnvLib.h"
#include "mvDebug.h"
#include "mvSysHwConfig.h"
#include "pex/mvPexRegs.h"
#include "cntmr/mvCntmr.h"
#include "gpp/mvGpp.h"

#if defined(CONFIG_MV_INCLUDE_CESA)
#include "cesa/mvCesa.h"
#endif

extern unsigned int irq_int_type[];

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


extern void __init mv_map_io(void);
extern void __init mv_init_irq(void);
extern struct sys_timer mv_timer;
extern MV_CPU_DEC_WIN* mv_sys_map(void);
#if defined(CONFIG_MV_INCLUDE_CESA)
extern u32 mv_crypto_base_get(void);
#endif
unsigned int support_wait_for_interrupt = 0x1;

u32 mvTclk = 166666667;
u32 mvSysclk = 200000000;
u32 mvIsUsbHost = 1;


u8	mvMacAddr[CONFIG_MV_ETH_PORTS_NUM][6];
u16	mvMtu[CONFIG_MV_ETH_PORTS_NUM] = {0};
extern MV_U32 gBoardId; 
extern unsigned int elf_hwcap;
 
static int __init parse_tag_mv_uboot(const struct tag *tag)
{
    	unsigned int mvUbootVer = 0;
	int i = 0;
 
	mvUbootVer = tag->u.mv_uboot.uboot_version;
	mvIsUsbHost = tag->u.mv_uboot.isUsbHost;

        printk("Using UBoot passing parameters structure\n");
  
	gBoardId =  (mvUbootVer & 0xff);
	for (i = 0; i < CONFIG_MV_ETH_PORTS_NUM; i++) {
#if defined (CONFIG_MV78XX0_Z0) || defined (CONFIG_MV78XX0_OVERRIDE_CMDLINE_ETH)
		memset(&mvMacAddr[i][0], 0, 6);
		mvMtu[i] = 0;
#else			
		memcpy(&mvMacAddr[i][0], &tag->u.mv_uboot.macAddr[i][0], 6);
		mvMtu[i] = tag->u.mv_uboot.mtu[i];
#endif    	
	}
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

    return (char *)(mv_crypto_base_get() + used_size);
}
#endif


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


/*****************************************************************************
 * UART
 ****************************************************************************/
static struct resource mv_uart_resources[] = {
	{
		.start		= PORT_BASE(0),
		.end		= PORT_BASE(0) + 0xff,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start          = IRQ_UART(0),
		.end            = IRQ_UART(0),
		.flags          = IORESOURCE_IRQ,
	},
	{
		.start		= PORT_BASE(1),
		.end		= PORT_BASE(1) + 0xff,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start          = IRQ_UART(1),
		.end            = IRQ_UART(1),
		.flags          = IORESOURCE_IRQ,
	},
};

static struct plat_serial8250_port mv_uart_data[] = {
	{
		.mapbase	= PORT_BASE(0),
		.membase	= (char *)PORT_BASE(0),
		.irq		= IRQ_UART(0),
		.flags		= UPF_SKIP_TEST | UPF_BOOT_AUTOCONF,
		.iotype		= UPIO_MEM,
		.regshift	= 2,
	},
	{
		.mapbase	= PORT_BASE(1),
		.membase	= (char *)PORT_BASE(1),
		.irq		= IRQ_UART(1),
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
/* warning: support 2 UART ports, initialize only 1 */
	.num_resources		= 2, /*ARRAY_SIZE(mv_uart_resources),*/
	.resource		= mv_uart_resources,
};


static void serial_initialize(void)
{
#if defined(CONFIG_MV78200)
	/*One UART per CPU*/	
	if (MV_TRUE == mvSocUnitIsMappedToThisCpu(UART0))
	{		
		mv_uart.resource = &mv_uart_resources[0]; 
		mv_uart.dev.platform_data = &mv_uart_data[0];
		mv_uart_data[1].flags = 0;
		mv_uart_data[0].uartclk = mvTclk;
		platform_device_register(&mv_uart);
	}
	else if (MV_TRUE == mvSocUnitIsMappedToThisCpu(UART1))
	{	
		mv_uart.resource = &mv_uart_resources[2]; 
		mv_uart.dev.platform_data = &mv_uart_data[1];
		mv_uart_data[1].uartclk = mvTclk;
		platform_device_register(&mv_uart);
	}	
#else
	mv_uart_data[0].uartclk = mv_uart_data[1].uartclk = mvTclk;
	platform_device_register(&mv_uart);
#endif
}

static void __init mv_vfp_init(void)
{

#if defined CONFIG_VFP_FASTVFP
	printk("VFP initialized to Run Fast Mode.\n");
#endif
}


static void __init mv_init(void)
{
        /* init the Board environment */
	mvBoardEnvInit();

        /* init the controller environment */
        if (mvCtrlEnvInit() ) {
            printk( "Controller env initialization failed.\n" );
            return;
        }


	/* Init the CPU windows setting and the access protection windows. */
	if (mvCpuIfInit(mv_sys_map())) {

		printk( "Cpu Interface initialization failed.\n" );
		return;
	}
#if defined (CONFIG_MV78XX0_Z0)
	mvCpuIfBridgeReorderWAInit();
#endif

    	/* Init Tclk & SysClk */
    	mvTclk = mvBoardTclkGet();
   	mvSysclk = mvBoardSysClkGet();
	
        support_wait_for_interrupt = 1;
  
#ifdef CONFIG_JTAG_DEBUG
            support_wait_for_interrupt = 0; /*  for Lauterbach */
#endif
	mv_vfp_init();	
	elf_hwcap &= ~HWCAP_JAVA;

   	serial_initialize();

	/* At this point, the CPU windows are configured according to default definitions in mvSysHwConfig.h */
	/* and cpuAddrWinMap table in mvCpuIf.c. Now it's time to change defaults for each platform.         */
	mvCpuIfAddrDecShow(whoAmI());

    	print_board_info();
}

#if defined(CONFIG_MV78200)


static void __init early_resmap_cpu0(char **p)
{
	char* tmp = strchr(*p, ' ');
	if (tmp) *tmp = '\0';
	if (MV_FALSE == mvSocUnitMapFillTable(*p, 0, strstr))		
		printk(KERN_ERR "Invalid command line for CPU0\n");
	if (tmp) *tmp = ' ';
}

static void __init early_resmap_cpu1(char **p)
{
	char* tmp = strchr(*p, ' ');
	if (tmp) *tmp = '\0';
	if (MV_FALSE == mvSocUnitMapFillTable(*p, 1, strstr))
		printk(KERN_ERR "Invalid command line for CPU1\n");
	if (tmp) *tmp = ' ';
}

__early_param("cpu0=", early_resmap_cpu0);
__early_param("cpu1=", early_resmap_cpu1);

#endif


MACHINE_START(FEROCEON_MV78XX0, "Feroceon-MV78XX0")
    /* MAINTAINER("MARVELL") */
    .phys_io = 0xf1000000,
    .io_pg_offst = ((0xf1000000) >> 18) & 0xfffc,
    .boot_params = 0x00000100,
    .map_io = mv_map_io,
    .init_irq = mv_init_irq,
    .timer = &mv_timer,
    .init_machine = mv_init,
MACHINE_END

