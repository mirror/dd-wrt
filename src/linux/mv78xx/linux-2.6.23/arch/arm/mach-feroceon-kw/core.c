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

#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "boardEnv/mvBoardEnvLib.h"
#include "mvDebug.h"
#include "mvSysHwConfig.h"
#include "pex/mvPexRegs.h"
#include "cntmr/mvCntmr.h"
#include "gpp/mvGpp.h"

#if defined(CONFIG_MV_INCLUDE_SDIO)
#include "ctrlEnv/sys/mvSysSdmmc.h"
#endif
#if defined(CONFIG_MV_INCLUDE_CESA)
#include "cesa/mvCesa.h"
#endif
#ifdef CONFIG_I2C_MV64XXX
#include <linux/i2c.h>	
#include <linux/mv643xx_i2c.h>
#include "ctrlEnv/mvCtrlEnvSpec.h"
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
extern u32 mv_crypo_base_get(void);
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
		memcpy(mvMacAddr[i], tag->u.mv_uboot.macAddr[i], 6);
		mvMtu[i] = tag->u.mv_uboot.mtu[i];
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

    return (char *)(mv_crypo_base_get() + used_size);
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
 * I2C(TWSI)
 ****************************************************************************/

/*Platform devices list*/
#ifdef CONFIG_I2C_MV64XXX

static struct mv64xxx_i2c_pdata kw_i2c_pdata = {
       .freq_m         = 8, /* assumes 166 MHz TCLK */
       .freq_n         = 3,
       .timeout        = 1000, /* Default timeout of 1 second */
};

static struct resource kw_i2c_resources[] = {
       {
               .name   = "i2c base",
               .start  = INTER_REGS_BASE + TWSI_SLAVE_ADDR_REG,
               .end    = INTER_REGS_BASE + TWSI_SLAVE_ADDR_REG + 0x20 -1,
               .flags  = IORESOURCE_MEM,
       },
       {
               .name   = "i2c irq",
               .start  = IRQ_TWSI,
               .end    = IRQ_TWSI,
               .flags  = IORESOURCE_IRQ,
       },
};

static struct platform_device kw_i2c = {
       .name           = MV64XXX_I2C_CTLR_NAME,
       .id             = 0,
       .num_resources  = ARRAY_SIZE(kw_i2c_resources),
       .resource       = kw_i2c_resources,
       .dev            = {
               .platform_data = &kw_i2c_pdata,
       },
};

#endif

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
/* warning: support 2 UART ports, initialize only 1 */
	.num_resources		= 2, /*ARRAY_SIZE(mv_uart_resources),*/
	.resource		= mv_uart_resources,
};


static void serial_initialize(void)
{
	mv_uart_data[0].uartclk = mv_uart_data[1].uartclk = mvTclk;
	platform_device_register(&mv_uart);
}

#ifdef CONFIG_MV_INCLUDE_AUDIO

typedef struct {
	unsigned int base;
	unsigned int size;
} _audio_mem_info;

_audio_mem_info mem_array[MV_DRAM_MAX_CS + 1];

static struct resource mv_snd_resources[] = {
	[0] = {
		.start	= INTER_REGS_BASE + AUDIO_REG_BASE,
		.end	= INTER_REGS_BASE + AUDIO_REG_BASE + SZ_16K -1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_AUDIO_INT,
		.end	= IRQ_AUDIO_INT,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.start	= NR_IRQS,	/* should obtained from board information*/
		.end	= NR_IRQS,	/* should obtained from board information */
		.flags	= IORESOURCE_IRQ,
	}

};

static u64 mv_snd_dmamask = 0xFFFFFFFFUL;

static struct platform_device mv_snd_device = {
	.name		= "mv88fx_snd",
	.id		= -1,
	.dev		= {
		.dma_mask = &mv_snd_dmamask,
		.coherent_dma_mask = 0xFFFFFFFF,
		.platform_data = mem_array,
	},
	.num_resources	= ARRAY_SIZE(mv_snd_resources),
	.resource	= mv_snd_resources,
};

#endif /* #ifdef CONFIG_MV_INCLUDE_AUDIO */

#if defined(CONFIG_MV_INCLUDE_SDIO)

static struct resource mvsdmmc_resources[] = {
	[0] = {
		.start	= INTER_REGS_BASE + MV_SDIO_REG_BASE,
		.end	= INTER_REGS_BASE + MV_SDIO_REG_BASE + SZ_1K -1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= SDIO_IRQ_NUM,
		.end	= SDIO_IRQ_NUM,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.start	= -1,	/* should obtained from board information*/
		.end	= -1,	/* should obtained from board information */
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


#endif /* #if defined(CONFIG_MV_INCLUDE_SDIO) */
static void __init mv_init(void)
{
        /* init the Board environment */
       	mvBoardEnvInit();

        /* init the controller environment */
        if( mvCtrlEnvInit() ) {
            printk( "Controller env initialization failed.\n" );
            return;
        }

	
	/* Init the CPU windows setting and the access protection windows. */
	if( mvCpuIfInit(mv_sys_map()) ) {

		printk( "Cpu Interface initialization failed.\n" );
		return;
	}

	/* Bridge Reordering  Workaround */
	mvCpuIfBridgeReorderWAInit();

    	/* Init Tclk & SysClk */
    	mvTclk = mvBoardTclkGet();
   	mvSysclk = mvBoardSysClkGet();
	
        support_wait_for_interrupt = 1;
  
#ifdef CONFIG_JTAG_DEBUG
            support_wait_for_interrupt = 0; /*  for Lauterbach */
#endif

	elf_hwcap &= ~HWCAP_JAVA;

   	serial_initialize();

	/* At this point, the CPU windows are configured according to default definitions in mvSysHwConfig.h */
	/* and cpuAddrWinMap table in mvCpuIf.c. Now it's time to change defaults for each platform.         */
	mvCpuIfAddDecShow();

    	print_board_info();


#if defined(CONFIG_MV_INCLUDE_SDIO)
       if (MV_TRUE == mvCtrlPwrClckGet(SDIO_UNIT_ID, 0)) 
       {
		
	       int irq_detect = mvBoardSDIOGpioPinGet();

		

		if (irq_detect != -1) {
               mvsdmmc_resources[2].end = mvBoardSDIOGpioPinGet() + IRQ_GPP_START;
               mvsdmmc_resources[2].start = mvBoardSDIOGpioPinGet() + IRQ_GPP_START;
	       irq_int_type[mvBoardSDIOGpioPinGet()] = GPP_IRQ_TYPE_CHANGE_LEVEL;
		}
		if (MV_OK == mvSdmmcWinInit())
	               platform_device_register(&mvsdmmc_device);
       }

#endif

#if defined(CONFIG_MV_INCLUDE_AUDIO)
       if (MV_TRUE == mvCtrlPwrClckGet(AUDIO_UNIT_ID, 0)) 
       {
		unsigned int i;
		for (i=0 ; i< MV_DRAM_MAX_CS; i++) {
			MV_DRAM_DEC_WIN win;
			mem_array[i].base = 0;
			mem_array[i].size = 0;

			mvDramIfWinGet(SDRAM_CS0 + i, &win);

			if (win.enable) {
				mem_array[i].base = win.addrWin.baseLow; 
				mem_array[i].size = win.addrWin.size;
			}
		}
		
               platform_device_register(&mv_snd_device);
       }

#endif

#ifdef CONFIG_I2C_MV64XXX 
       platform_device_register(&kw_i2c);
#endif

    return;
}


MACHINE_START(FEROCEON_KW ,"Feroceon-KW")
    /* MAINTAINER("MARVELL") */
    .phys_io = 0xf1000000,
    .io_pg_offst = ((0xf1000000) >> 18) & 0xfffc,
    .boot_params = 0x00000100,
    .map_io = mv_map_io,
    .init_irq = mv_init_irq,
    .timer = &mv_timer,
    .init_machine = mv_init,
MACHINE_END

