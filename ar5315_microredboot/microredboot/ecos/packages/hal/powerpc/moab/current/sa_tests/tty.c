//
// TTY support code
//
//-----------------------------------------------------------------
// Copyright (C) 2003, 2004 Gary Thomas <gary@mlbassoc.com>
//-----------------------------------------------------------------

struct uart {
    unsigned char rbr;
#define thr rbr
#define dll rbr
    unsigned char dlm;
#define ier dlm
    unsigned char iir;
#define fcr iir
    unsigned char lcr;
    unsigned char mcr;
    unsigned char lsr;
    unsigned char msr;
    unsigned char scr;
};

// Control register defines
// The interrupt enable register bits.
#define SIO_IER_ERDAI   0x01            // enable received data available irq
#define SIO_IER_ETHREI  0x02            // enable THR empty interrupt
#define SIO_IER_ELSI    0x04            // enable receiver line status irq
#define SIO_IER_EMSI    0x08            // enable modem status interrupt

// The interrupt identification register bits.
#define SIO_IIR_IP      0x01            // 0 if interrupt pending
#define SIO_IIR_ID_MASK 0x0e            // mask for interrupt ID bits
#define ISR_Tx  0x02
#define ISR_Rx  0x04

// The line status register bits.
#define SIO_LSR_DR      0x01            // data ready
#define SIO_LSR_OE      0x02            // overrun error
#define SIO_LSR_PE      0x04            // parity error
#define SIO_LSR_FE      0x08            // framing error
#define SIO_LSR_BI      0x10            // break interrupt
#define SIO_LSR_THRE    0x20            // transmitter holding register empty
#define SIO_LSR_TEMT    0x40            // transmitter register empty
#define SIO_LSR_ERR     0x80            // any error condition

// The modem status register bits.
#define SIO_MSR_DCTS  0x01              // delta clear to send
#define SIO_MSR_DDSR  0x02              // delta data set ready
#define SIO_MSR_TERI  0x04              // trailing edge ring indicator
#define SIO_MSR_DDCD  0x08              // delta data carrier detect
#define SIO_MSR_CTS   0x10              // clear to send
#define SIO_MSR_DSR   0x20              // data set ready
#define SIO_MSR_RI    0x40              // ring indicator
#define SIO_MSR_DCD   0x80              // data carrier detect

// The line control register bits.
#define SIO_LCR_WLS0   0x01             // word length select bit 0
#define SIO_LCR_WLS1   0x02             // word length select bit 1
#define SIO_LCR_STB    0x04             // number of stop bits
#define SIO_LCR_PEN    0x08             // parity enable
#define SIO_LCR_EPS    0x10             // even parity select
#define SIO_LCR_SP     0x20             // stick parity
#define SIO_LCR_SB     0x40             // set break
#define SIO_LCR_DLAB   0x80             // divisor latch access bit

// The FIFO control register
#define SIO_FCR_FCR0   0x01             // enable xmit and rcvr fifos
#define SIO_FCR_FCR1   0x02             // clear RCVR FIFO
#define SIO_FCR_FCR2   0x04             // clear XMIT FIFO

void
tty_init(void)
{
    volatile struct uart *uart = (volatile struct uart *)0xEF600300;
    unsigned char lcr;

    // Reset/clear interrupts
    uart->ier = 0;

    // Disable and clear FIFOs (need to enable to clear).
    uart->fcr = (SIO_FCR_FCR0 | SIO_FCR_FCR1 | SIO_FCR_FCR2);
    uart->fcr = 0;

#if 0 // Assume that baud rate has been properly set
    // Set speed to 38400.
    uart->lcr = SIO_LCR_WLS0 | SIO_LCR_WLS1 | SIO_LCR_DLAB;
    uart->dll = 0x12;
    uart->dlm = 0;
#endif

    // 8-1-no parity.
    uart->lcr = SIO_LCR_WLS0 | SIO_LCR_WLS1;

    // Enable FIFOs (and clear them).
    uart->fcr = (SIO_FCR_FCR0 | SIO_FCR_FCR1 | SIO_FCR_FCR2);
}

void
tty_putc(char c)
{
    volatile struct uart *uart = (volatile struct uart *)0xEF600300;

    do {
    } while ((uart->lsr & SIO_LSR_TEMT) == 0);
    uart->thr = c;
}

void
tty_puts(char *s)
{
    char c;

    while ((c = *s++) != '\0') {
        tty_putc(c);
        if (c == '\n') {
            tty_putc('\r');
        }
    }
}

char
tty_getc(void)
{
    volatile struct uart *uart = (volatile struct uart *)0xEF600300;

    while ((uart->lsr & SIO_LSR_DR) == 0) ;
    return uart->rbr;
}

void
tty_puthex(unsigned long val)
{
    char hex[] = "0123456789ABCDEF";
    char str[11];
    char *s = &str[10];
    int i;

    *--s = '\0';
    for (i = 0;  i < 8;  i++) {
        *--s = hex[(val & 0x0F)];
        val >>= 4;
    }
    *--s = 'x';
    *--s = '0';
    tty_puts(s);
}
