/*
 * Console support for hndrte.
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: hndrte_cons.c,v 1.62.2.1 2008/05/05 01:02:15 Exp $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <hndrte.h>
#include <bcmutils.h>
#include <siutils.h>
#include <sbchipc.h>
#include <hndchipc.h>
#include <bcmdevs.h>

#define	VIRTUAL_UART	((serial_dev_t *)0xffffffff)

#define CBUF_LEN	(128)

#ifdef	BCMDBG
#define LOG_BUF_LEN	(16 * 1024)
#else
#define LOG_BUF_LEN	(1024)
#endif
#define LOG_BUF_MASK	(LOG_BUF_LEN-1)

typedef struct _ccmd {
	char		*name;
	cons_fun_t	fun;
	uint32		arg;
	struct _ccmd	*next;
} ccmd_t;

typedef struct _serial_struct {
	int	baud_base;
	int	irq;
	unsigned char	*reg_base;
	unsigned short	reg_shift;
} serial_dev_t;

/* Console description. */
typedef struct _cons_state {
	/* Virtual (SHM) uart control */
	volatile uint	vcons_in;
	volatile uint	vcons_out;

	/* For QT scripting through dhd */
	char   *log_buf_addr;
	uint	log_buf_size;

	/* Output (logging) buffer */
	uint	log_idx;
	char	*log_buf;

	/* Input buffer */
	uint	cbuf_idx;
	char	cbuf[CBUF_LEN];

	/* uart device, or VIRTUAL_UART */
	serial_dev_t	*uart;

	/* Console command processing */
	ccmd_t		*ccmd;
} cons_soft_t;


static serial_dev_t hndrte_uart;

/* FXIME, allocate dynamically if multiple console support is desired. */
static cons_soft_t *cons0 = NULL;
static cons_soft_t *active_cons = NULL;


/* serial_in: read a uart register */
static inline int
serial_in(serial_dev_t *info, int offset)
{
	return ((int)R_REG(NULL, (uint8 *)(info->reg_base + (offset << info->reg_shift))));
}

/* serial_out: write a uart register */
static inline void
serial_out(serial_dev_t *info, int offset, int value)
{
	W_REG(NULL, (uint8 *)(info->reg_base + (offset << info->reg_shift)), value);
}

/* local functions */
static void *add_console(si_t *sih);
static void process_ccmd(char *line, uint len);
static char **process_cmdline(char *cmd_line, uint *argc);
#ifndef EXT_CBALL
static void serial_add(void *regs, uint irq, uint baud_base, uint reg_shift);
#endif
#ifdef	BCMDBG
static void serial_dump_cmds(uint32 arg, uint argc, char *argv[]);
#endif

#ifndef EXT_CBALL
/* serial_add: callback to initialize the uart structure */
static void
BCMINITFN(serial_add)(void *regs, uint irq, uint baud_base, uint reg_shift)
{
	int quot;

	if (hndrte_uart.reg_base)
		return;

	hndrte_uart.reg_base = regs;
	hndrte_uart.irq = irq;
	hndrte_uart.baud_base = baud_base / 16;
	hndrte_uart.reg_shift = reg_shift;

	/* Set baud and 8N1 */
	quot = (hndrte_uart.baud_base + 57600) / 115200;
	serial_out(&hndrte_uart, UART_LCR, UART_LCR_DLAB);
	serial_out(&hndrte_uart, UART_DLL, quot & 0xff);
	serial_out(&hndrte_uart, UART_DLM, quot >> 8);
	serial_out(&hndrte_uart, UART_LCR, UART_LCR_WLEN8);

	/* enable interrupts for rx data */
	serial_out(&hndrte_uart, UART_IER, UART_IER_ERBFI);
	serial_out(&hndrte_uart, UART_MCR, UART_MCR_OUT2);

	/* enable FIFOs */
	serial_out(&hndrte_uart, UART_FCR, UART_FCR_FIFO_ENABLE);

	/* According to the Synopsys website: "the serial clock
	 * modules must have time to see new register values
	 * and reset their respective state machines. This
	 * total time is guaranteed to be no more than
	 * (2 * baud divisor * 16) clock cycles of the slower
	 * of the two system clocks. No data should be transmitted
	 * or received before this maximum time expires."
	 */
	OSL_DELAY(1000);
}
#endif	/* EXT_CBALL */

static void *
BCMINITFN(add_console)(si_t *sih)
{
	uint i;

	if (active_cons == NULL) {
		if (cons0 == NULL)
			hndrte_log_init();
		active_cons = cons0;
	}
	if (active_cons == NULL)
		return NULL;

#ifdef BCMQT
		active_cons->vcons_in = 0;
		active_cons->vcons_out = 0;
		active_cons->uart = NULL;
		return active_cons;
#endif

#ifndef EXT_CBALL
	if ((sih->chippkg != HDLSIM_PKG_ID) && (sih->chippkg != HWSIM_PKG_ID))
		si_serial_init(sih, serial_add);
#endif

	if (hndrte_uart.reg_base)
		active_cons->uart = &hndrte_uart;
	else {
		active_cons->vcons_in = 0;
		active_cons->vcons_out = 0;

		active_cons->uart = VIRTUAL_UART;
	}

	/* dump whatever that is already in the logbuf */
	for (i = 0; i < active_cons->log_idx; i ++) {
		if (active_cons->uart != VIRTUAL_UART) {
			while (!(serial_in(active_cons->uart, UART_LSR) & UART_LSR_THRE))
				;
			serial_out(active_cons->uart, UART_TX, active_cons->log_buf[i]);
		}
	}

#ifdef	BCMDBG
	hndrte_cons_addcmd("cmds", serial_dump_cmds, (uint32)active_cons);
	hndrte_cons_addcmd("help", serial_dump_cmds, (uint32)active_cons);
	hndrte_cons_addcmd("?", serial_dump_cmds, (uint32)active_cons);
#endif /* BCMDBG */

	return active_cons;
}

static void
hndrte_cons_isr(hndrte_dev_t *dev)
{
	cons_soft_t *cd = active_cons;
	uint idx;

	if (cd->uart == NULL)
		return;

	if (cd->uart != VIRTUAL_UART) {
		if ((serial_in(cd->uart, UART_IIR) & UART_IIR_INT_MASK) == UART_IIR_NOINT) {
			return;
		}
	}

	if (cd->uart == VIRTUAL_UART) {
		uint i;

		/* Wait for input */
		if ((idx = cd->vcons_in) == 0) {
			return;
		}

		cd->cbuf_idx = idx;

		/* echo it synchronously */
		for (i = 0; i < idx; i++)
			putc(cd->cbuf[i]);
		putc('\n');
	} else {
		int c;

		/* Input available? */
		if (!(serial_in(cd->uart, UART_LSR) & UART_LSR_RXRDY)) {
			return;
		}

		/* Get the char */
		c = serial_in(cd->uart, UART_RX);

		if (c == '\r')
			c = '\n';

		/* Backspace */
		if (c == '\b' || c == 0177) {
			if (cd->cbuf_idx > 0) {
				cd->cbuf_idx--;
				putc('\b');
				putc(' ');
				putc('\b');
			}
			return;
		}

		/* echo it synchronously */
		putc(c);

		idx = cd->cbuf_idx;
		if (c != '\n') {
			/* Save it */
			cd->cbuf[idx++] = c;

			/* If not a carriage return, and still space in buffer;
			 * return from the poll to continue waiting.
			 */
			if ((cd->cbuf_idx = idx) < CBUF_LEN) {
				return;
			}
		} else
			cd->cbuf[idx] = '\0';
	}

	/* OK, process the input and call the proper command processor */
	process_ccmd(cd->cbuf, idx);

	/* After we are done, setup for next */
	cd->cbuf_idx = 0;
	if (cd->uart == VIRTUAL_UART)
		cd->vcons_in = 0;
	putc('>');
	putc(' ');
}

static void
hndrte_uart_isr(void *cbdata, uint intstatus)
{
	hndrte_cons_isr((hndrte_dev_t *)cbdata);
}

/* hndrte_cons_init(sih): Initialize the console subsystem */
void
BCMINITFN(hndrte_cons_init)(si_t *sih)
{
	si_cc_register_isr(sih, hndrte_uart_isr, CI_UART, NULL);
	hndrte_uart.reg_base = NULL;
	add_console(sih);
}

/*
 * hdnrte.
 *
 * Console i/o:
 *	putc(c): Write out a byte to the console
 *	getc(): Wait for and read a byte from the console
 *	keypressed(): Check for key pressed
 */

void
putc(int c)
{
	cons_soft_t *cd = active_cons;

	/* ready? */
	if (cd == NULL)
		return;

	/* CR before LF */
	if (c == '\n')
		putc('\r');

	if (cd->log_buf != NULL) {
		int idx = cd->log_idx;

		/* Store in log buffer */
		cd->log_buf[idx] = (char) c;
		cd->log_idx = (idx + 1) & LOG_BUF_MASK;
	}

	/* No UART */
	if (cd->uart == NULL)
		return;
	else if (cd->uart == VIRTUAL_UART) {
	} else {
#ifdef HNDRTE_CONSOLE
		while (!(serial_in(cd->uart, UART_LSR) & UART_LSR_THRE));
			serial_out(cd->uart, UART_TX, c);
#endif
	}
}

bool
keypressed(void)
{
	cons_soft_t *cd = active_cons;

	/* no UART */
	if ((cd == NULL) || (cd->uart == NULL))
		return FALSE;

	if (cd->uart == VIRTUAL_UART)
		return (cd->vcons_in != 0);
	else
		return (serial_in(cd->uart, UART_LSR) & UART_LSR_RXRDY);
}

int
getc(void)
{
	cons_soft_t *cd = active_cons;

	/* no UART */
	if ((cd == NULL) || (cd->uart == NULL))
		return (0);

	if (cd->uart == VIRTUAL_UART) {
		/* If we have consumed all the previous input */
		if (cd->cbuf_idx >= cd->vcons_in)
			cd->cbuf_idx = cd->vcons_in = 0;

		/* Wait for input */
		while (cd->vcons_in == 0)
			hndrte_poll(hndrte_sih);

		return (cd->cbuf[cd->cbuf_idx++]);
	} else {
		/* Wait for input */
		while (!keypressed())
			hndrte_poll(hndrte_sih);

		return (serial_in(cd->uart, UART_RX));
	}
}

/*
 * hdnrte.
 *
 * Console command support:
 *	hndrte_cons_addcmd()
 */
void
hndrte_cons_addcmd(char *name, cons_fun_t fun, uint32 arg)
{
	cons_soft_t *cd = active_cons;
	ccmd_t *new;

	new = hndrte_malloc(sizeof(ccmd_t));
	ASSERT(new != NULL);

	new->name = name;
	new->fun = fun;
	new->arg = arg;
	new->next = cd->ccmd;

	cd->ccmd = new;
}

/* Tokenize command line into argv */
static char **
process_cmdline(char *cmd_line, uint *argc)
{
	int pass2;
	char **argv = NULL, *s, *t;
	int qquote;

	/* Consumes input line by overwriting spaces with NULs */

	for (pass2 = 0; pass2 < 2; pass2++) {
		qquote = 0;
		for (s = cmd_line, *argc = 0;;) {
			while (*s == ' ')
				s++;
			if (!*s)
				break;
			if (pass2)
				argv[*argc] = s;
			(*argc)++;
			while (*s && (qquote || *s != ' ')) {
				if (*s == '"') {
					qquote ^= 1;
					if (pass2)	/* delete quotes 2nd pass */
						for (t = s--; (t[0] = t[1]) != 0; t++);
				}
				s++;
			}
			if (pass2 && *s)
				*s++ = 0;
		}

		if (!pass2) {
			argv = (char **)hndrte_malloc((*argc + 1) * sizeof(char *));
			ASSERT(argv);
		}
	}

	argv[*argc] = NULL;

	return argv;
}

static void
process_ccmd(char *line, uint len)
{
	cons_soft_t *cd = active_cons;
	ccmd_t *ccmd = cd->ccmd;
	uint argc;
	char **argv = NULL;

	if (!line[0])
		return;

	argv = process_cmdline(line, &argc);
	if (argc == 0) {
		hndrte_free(argv);
		return;
	}

	ASSERT(argv[0]);

	while (ccmd != NULL) {
		if (strcmp(ccmd->name, argv[0]) == 0)
			break;

		ccmd = ccmd->next;
	}

	if (ccmd != NULL)
		ccmd->fun(ccmd->arg, argc, argv);
	else
		printf("?\n");

	hndrte_free(argv);
}

int
BCMINITFN(hndrte_log_init)(void)
{
	cons0 = (cons_soft_t *)hndrte_malloc(sizeof(cons_soft_t));
	if (cons0 == NULL)
		return BCME_ERROR;

	bzero(cons0, sizeof(cons_soft_t));
	cons0->log_buf = (char *)hndrte_malloc(LOG_BUF_LEN);
	if (cons0->log_buf == NULL) {
		hndrte_free((void *)cons0);
		cons0 = NULL;
		return BCME_ERROR;
	}

	cons0->log_buf = (char *)OSL_UNCACHED(cons0->log_buf);
	bzero(cons0->log_buf, LOG_BUF_LEN);
	cons0->log_buf_addr = cons0->log_buf;
	cons0->log_buf_size = LOG_BUF_LEN;

	active_cons = cons0;

	return 0;
}

#ifdef	BCMDBG
static void
serial_dump_cmds(uint32 arg, uint argc, char *argv[])
{
	cons_soft_t *cd = (cons_soft_t *)arg;
	ccmd_t *ccmd = cd->ccmd;

	while (ccmd != NULL) {
		printf("cmd \"%s\": %p(%p)\n", ccmd->name, (void *)ccmd->fun, (void *)ccmd->arg);
		ccmd = ccmd->next;
	}
}
#endif	/* BCMDBG */
