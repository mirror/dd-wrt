/*
 * linux/include/asm/arch-samsung/uncompress.c
 * 2001 Mac Wang <mac@os.nctu.edu.tw> 
 */

#include <asm/hardware.h>

static int s3c4510b_decomp_setup()
{
	CSR_WRITE(DEBUG_UARTLCON_BASE, DEBUG_ULCON_REG_VAL);
	CSR_WRITE(DEBUG_UARTCONT_BASE, DEBUG_UCON_REG_VAL);
	CSR_WRITE(DEBUG_UARTBRD_BASE,  DEBUG_UBRDIV_REG_VAL);
}

static int s3c4510b_putc(char c)
{
	CSR_WRITE(DEBUG_TX_BUFF_BASE, c);
	while(!(CSR_READ(DEBUG_CHK_STAT_BASE) & DEBUG_TX_DONE_CHECK_BIT));

	if(c == '\n')
		s3c4510b_putc('\r');
}

static void s3c4510b_puts(const char *s)
{
	while(*s != '\0')
		s3c4510b_putc(*s++);
}
