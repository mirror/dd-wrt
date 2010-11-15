#include <linux/init.h>
#include <linux/tty.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/serial.h>
#include <linux/console.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/pci.h>

#include <asm/reboot.h>
#include <asm/io.h>
#include <asm/time.h>
#include <asm/pgtable.h>
#include <asm/processor.h>
#include <asm/reboot.h>
#include <asm/system.h>
#include <asm/serial.h>
#include <asm/traps.h>
#include <linux/serial_core.h>
#include <asm/bootinfo.h>

#include "ar7240.h"

#ifdef CONFIG_AR7240_EMULATION
#define         AG7240_CONSOLE_BAUD (9600)
#else
#define         AG7240_CONSOLE_BAUD (115200)
#endif

uint32_t ar7240_cpu_freq = 0, ar7240_ahb_freq, ar7240_ddr_freq;

static int __init ar7240_init_ioc(void);
void Uart16550Init(void);
void serial_print(char *fmt, ...);
void writeserial(char *str,int count);
static void ar7240_sys_frequency(void);
u8 Uart16550GetPoll(void);
/* 
 * Export AHB freq value to be used by Ethernet MDIO.
 */
EXPORT_SYMBOL(ar7240_ahb_freq);

void
ar7240_restart(char *command)
{
    for(;;) {
        ar7240_reg_wr(AR7240_RESET, AR7240_RESET_FULL_CHIP);
    }
}

void
ar7240_halt(void)
{
        printk(KERN_NOTICE "\n** You can safely turn off the power\n");
        while (1);
}

void
ar7240_power_off(void)
{
        ar7240_halt();
}


const char 
*get_system_type(void)
{
char *chip;
u32 id;
u32 rev=0;
static char str[64];
id = ar7240_reg_rd(AR7240_REV_ID) & AR7240_REV_ID_MASK;

switch (id) {
	case AR7240_REV_ID_AR7130:
		chip = "7130";
		break;
	case AR7240_REV_ID_AR7141:
		chip = "7141";
		break;
	case AR7240_REV_ID_AR7161:
		chip = "7161";
		break;
	case AR7240_REV_1_0:
		chip = "7240";
		rev = 0;
		break;
	case AR7240_REV_1_1:
		chip = "7240";
		rev = 1;
		break;
	case AR7240_REV_1_2:
		chip = "7240";
		rev = 2;
		break;
	case AR7241_REV_1_0:
		chip = "7241";
		rev = 0;
		break;
	case AR7242_REV_1_0:
		chip = "7242";
		rev = 0;
		break;
	case AR7241_REV_1_1:
		chip = "7241";
		rev = 1;
		break;
	case AR7242_REV_1_1:
		chip = "7242";
		rev = 1;
		break;
	default:
		chip = "724x";
	}
sprintf(str, "Atheros AR%s rev 1.%u (0x%04x)",
		chip, rev, id);
return str;
}

EXPORT_SYMBOL(get_system_type);
/*
 * The bootloader musta set cpu_pll_config.
 * We extract the pll divider, multiply it by the base freq 40.
 * The cpu and ahb are divided off of that.
 */
//#define FB50 1
static void
ar7240_sys_frequency(void)
{

#ifdef CONFIG_AR7240_EMULATION
#ifdef FB50
    ar7240_cpu_freq = 66000000;
    ar7240_ddr_freq = 66000000;
    ar7240_ahb_freq = 33000000;
#else
#if 1
    ar7240_cpu_freq = 300000000;
    ar7240_ddr_freq = 300000000;
    ar7240_ahb_freq = 150000000;
#else
    ar7240_cpu_freq = 62500000;
    ar7240_ddr_freq = 62500000;
    ar7240_ahb_freq = 31250000;
#endif

#endif
    return;
#else
    uint32_t pll, pll_div, ahb_div, ddr_div, freq, ref_div;

    if (ar7240_cpu_freq)
        return;

    pll = ar7240_reg_rd(AR7240_PLL_CONFIG);

    pll_div  = ((pll >> PLL_DIV_SHIFT) & PLL_DIV_MASK);
    ref_div  = (pll >> REF_DIV_SHIFT) & REF_DIV_MASK;
    ddr_div  = ((pll >> DDR_DIV_SHIFT) & DDR_DIV_MASK) + 1;
    ahb_div  = (((pll >> AHB_DIV_SHIFT) & AHB_DIV_MASK) + 1)*2;

    freq     = pll_div * ref_div * 5000000;

    ar7240_cpu_freq = freq;
    ar7240_ddr_freq = freq/ddr_div;
    ar7240_ahb_freq = ar7240_cpu_freq/ahb_div;
#endif
}
extern int early_serial_setup(struct uart_port *port);

#define AR71XX_UART_FLAGS (UPF_BOOT_AUTOCONF | UPF_SKIP_TEST | UPF_IOREMAP)
void __init
serial_setup(void)
{
	struct uart_port p;

	memset(&p, 0, sizeof(p));

	p.flags     = AR71XX_UART_FLAGS;
	p.iotype    = UPIO_MEM32;
	p.uartclk   = ar7240_ahb_freq;
	p.irq       = AR7240_MISC_IRQ_UART;
	p.regshift  = 2;
	p.membase   = (u8 *)KSEG1ADDR(AR7240_UART_BASE);

	if (early_serial_setup(&p) != 0)
		printk(KERN_ERR "early_serial_setup failed\n");
	
}
void __init plat_time_init(void)
{
    /* 
     * to generate the first CPU timer interrupt
     */
    write_c0_count(0);
    write_c0_compare(0xffff);

    mips_hpt_frequency =  ar7240_cpu_freq/2;
}


#define compare_change_hazard() \
	do { \
		irq_disable_hazard(); \
		irq_disable_hazard(); \
		irq_disable_hazard(); \
		irq_disable_hazard(); \
	} while (0)





int 
ar7240_be_handler(struct pt_regs *regs, int is_fixup)
{
#if 0
    if (!is_fixup && (regs->cp0_cause & 4)) {
        /* Data bus error - print PA */
        printk("DBE physical address: %010Lx\n",
                __read_64bit_c0_register($26, 1));
    }
#endif
#ifdef CONFIG_PCI
    int error = 0, status, trouble = 0;
    error = ar7240_reg_rd(AR7240_PCI_ERROR) & 3;

    if (error) {
        printk("PCI error %d at PCI addr 0x%x\n", 
                error, ar7240_reg_rd(AR7240_PCI_ERROR_ADDRESS));
        ar7240_reg_wr(AR7240_PCI_ERROR, error);
        ar7240_local_read_config(PCI_STATUS, 2, &status);
        printk("PCI status: %#x\n", status);
        trouble = 1;
    }

    error = 0;
    error = ar7240_reg_rd(AR7240_PCI_AHB_ERROR) & 1;

    if (error) {
        printk("AHB error at AHB address 0x%x\n", 
                  ar7240_reg_rd(AR7240_PCI_AHB_ERROR_ADDRESS));
        ar7240_reg_wr(AR7240_PCI_AHB_ERROR, error);
        ar7240_local_read_config(PCI_STATUS, 2, &status);
        printk("PCI status: %#x\n", status);
        trouble = 1;
    }
#endif

    printk("ar7240 data bus error: cause %#x\n", read_c0_cause());
    return (is_fixup ? MIPS_BE_FIXUP : MIPS_BE_FATAL);
}

void disable_early_printk(void)
{
}

#define AR71XX_MEM_SIZE_MIN	0x0200000
#define AR71XX_MEM_SIZE_MAX	0x8000000

static void __init ar71xx_detect_mem_size(void)
{
	volatile u8 *p;
	u8 memsave;
	u32 size;

	p = (volatile u8 *) KSEG1ADDR(0);
	memsave = *p;
	for (size = AR71XX_MEM_SIZE_MIN;
	     size <= (AR71XX_MEM_SIZE_MAX >> 1); size <<= 1) {
		volatile u8 *r;

		r = (p + size);
		*p = 0x55;
		if (*r == 0x55) {
			/* Mirrored data found, try another pattern */
			*p = 0xAA;
			if (*r == 0xAA) {
				/* Mirrored data found again, stop detection */
				break;
			}
		}
	}
	*p = memsave;

	add_memory_region(0, size, BOOT_MEM_RAM);
}

unsigned int __cpuinit get_c0_compare_irq(void)
{
	return CP0_LEGACY_COMPARE_IRQ;
}

unsigned int __cpuinit get_c0_compare_int(void)
{
    //printk("%s: returning timer irq : %d\n",__func__, AR7240_CPU_IRQ_TIMER);
    return AR7240_CPU_IRQ_TIMER;
}



int is_ar9000;

void __init plat_mem_setup(void)
{

#if 0
    board_be_handler = ar7240_be_handler;
#endif
//    board_timer_setup   =  ar7240_timer_setup;
    _machine_restart    =  ar7240_restart;
    _machine_halt       =  ar7240_halt;
    pm_power_off = ar7240_power_off;


    /* 
    ** early_serial_setup seems to conflict with serial8250_register_port() 
    ** In order for console to work, we need to call register_console().
    ** We can call serial8250_register_port() directly or use
    ** platform_add_devices() function which eventually calls the 
    ** register_console(). AP71 takes this approach too. Only drawback
    ** is if system screws up before we register console, we won't see
    ** any msgs on the console.  System being stable now this should be
    ** a special case anyways. Just initialize Uart here.
    */ 
    ar71xx_detect_mem_size();

    Uart16550Init();
    serial_print("Booting AR7240(Python)...\n");
    is_ar9000=1;
//#if 0
//    serial_setup();
//#endif
}

/*
 * -------------------------------------------------
 * Early printk hack
 */
/* === CONFIG === */

#define		REG_OFFSET		4

/* === END OF CONFIG === */

/* register offset */
#define         OFS_RCV_BUFFER          (0*REG_OFFSET)
#define         OFS_TRANS_HOLD          (0*REG_OFFSET)
#define         OFS_SEND_BUFFER         (0*REG_OFFSET)
#define         OFS_INTR_ENABLE         (1*REG_OFFSET)
#define         OFS_INTR_ID             (2*REG_OFFSET)
#define         OFS_DATA_FORMAT         (3*REG_OFFSET)
#define         OFS_LINE_CONTROL        (3*REG_OFFSET)
#define         OFS_MODEM_CONTROL       (4*REG_OFFSET)
#define         OFS_RS232_OUTPUT        (4*REG_OFFSET)
#define         OFS_LINE_STATUS         (5*REG_OFFSET)
#define         OFS_MODEM_STATUS        (6*REG_OFFSET)
#define         OFS_RS232_INPUT         (6*REG_OFFSET)
#define         OFS_SCRATCH_PAD         (7*REG_OFFSET)

#define         OFS_DIVISOR_LSB         (0*REG_OFFSET)
#define         OFS_DIVISOR_MSB         (1*REG_OFFSET)

#define         UART16550_READ(y)   ar7240_reg_rd((AR7240_UART_BASE+y))
#define         UART16550_WRITE(x, z)  ar7240_reg_wr((AR7240_UART_BASE+x), z)

static int serial_inited = 0;

#define         MY_WRITE(y, z)  ((*((volatile u32*)(y))) = z)

void Uart16550Init()
{
    int freq, div;

    ar7240_sys_frequency();
    freq = ar7240_ahb_freq;

#if 0// CONFIG_DIR615E

    MY_WRITE(0xb8040000, 0xcff);
    MY_WRITE(0xb8040008, 0x3b);
    /* Enable UART , SPI and Disable S26 UART */ 
    MY_WRITE(0xb8040028, (ar7240_reg_rd(0xb8040028) | 0x48002));

    MY_WRITE(0xb8040008, 0x2f);
#endif
    div = freq/(AG7240_CONSOLE_BAUD*16);

//    div = 0xCB;
        /* set DIAB bit */
    UART16550_WRITE(OFS_LINE_CONTROL, 0x80);
        
    /* set divisor */
    UART16550_WRITE(OFS_DIVISOR_LSB, (div & 0xff));
    UART16550_WRITE(OFS_DIVISOR_MSB, (div >> 8) & 0xff);

    /*UART16550_WRITE(OFS_DIVISOR_LSB, 0x61);
    UART16550_WRITE(OFS_DIVISOR_MSB, 0x03);*/

    /* clear DIAB bit*/ 
    UART16550_WRITE(OFS_LINE_CONTROL, 0x00);

    /* set data format */
    UART16550_WRITE(OFS_DATA_FORMAT, 0x3);

    UART16550_WRITE(OFS_INTR_ENABLE, 0);
}


u8 Uart16550GetPoll()
{
    while((UART16550_READ(OFS_LINE_STATUS) & 0x1) == 0);
    return UART16550_READ(OFS_RCV_BUFFER);
}

void Uart16550Put(u8 byte)
{
    if (!serial_inited) {
        serial_inited = 1;
        Uart16550Init();
    }
    while (((UART16550_READ(OFS_LINE_STATUS)) & 0x20) == 0x0);
    UART16550_WRITE(OFS_SEND_BUFFER, byte);
}

extern int vsprintf(char *buf, const char *fmt, va_list args);
static char sprint_buf[1024];

void
serial_print(char *fmt, ...)
{
        va_list args;
        int n;

        va_start(args, fmt);
        n = vsprintf(sprint_buf, fmt, args);
        va_end(args);
        writeserial(sprint_buf,n);
}

void writeserial(char *str,int count)
{
  int i;
  for(i = 0 ;i <= count ; i++)
	Uart16550Put(str[i]);

	Uart16550Put('\r');
  memset(str,'\0',1024);
  return;
}

unsigned int getCPUClock(void)
{
    return ar7240_cpu_freq/1000000;
}

