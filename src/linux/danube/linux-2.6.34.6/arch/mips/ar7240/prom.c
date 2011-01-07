
/*
 * Prom setup file for ar7240
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/serial_reg.h>

#include <asm/bootinfo.h>
#include <asm/addrspace.h>

#include "ar7240.h"

int __ath_flash_size;

extern void Uart16550Put(unsigned char byte);

#define UART_READ(r) \
	__raw_readl((void __iomem *)(KSEG1ADDR(AR7240_UART_BASE) + 4 * (r)))

#define UART_WRITE(r, v) \
	__raw_writel((v), (void __iomem *)(KSEG1ADDR(AR7240_UART_BASE) + 4*(r)))

void prom_putchar(unsigned char ch)
{
	while (((UART_READ(UART_LSR)) & UART_LSR_THRE) == 0);
	UART_WRITE(UART_TX, ch);
	while (((UART_READ(UART_LSR)) & UART_LSR_THRE) == 0);
}


void __init prom_init(void)
{

    printk ("flash_size passed from bootloader = %d\n", (int)fw_arg3);
    __ath_flash_size = fw_arg3;


    mips_machtype  = MACH_ATHEROS_AR7240;

}

void __init prom_free_prom_memory(void)
{
}




