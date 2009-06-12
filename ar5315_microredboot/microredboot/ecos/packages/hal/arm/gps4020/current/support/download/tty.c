//
// TTY support code
//

struct uart {
    unsigned char control;
    unsigned char _fill0[3];
    unsigned char mode;
    unsigned char _fill1[3];
    unsigned char baud;
    unsigned char _fill2[3];
    unsigned char status;
    unsigned char _fill3[3];
    unsigned char TxRx;
    unsigned char _fill4[3];
    unsigned char modem_control;
    unsigned char _fill5[3+8];
    unsigned char modem_status;
    unsigned char _fill6[3];
};

// Serial control
#define SCR_MIE      0x80   // Modem interrupt enable
#define SCR_EIE      0x40   // Error interrupt enable
#define SCR_TIE      0x20   // Transmit interrupt enable
#define SCR_RIE      0x10   // Receive interrupt enable
#define SCR_FCT      0x08   // Flow type 0=>software, 1=>hardware
#define SCR_CLK      0x04   // Clock source 0=internal, 1=external
#define SCR_TEN      0x02   // Transmitter enabled
#define SCR_REN      0x01   // Receiver enabled
// Serial mode
#define SMR_DIV(x)   ((x)<<4) // Clock divisor
#define SMR_STOP     0x08   // Stop bits 0=>one, 1=>two
#define   SMR_STOP_1   0x00
#define   SMR_STOP_2   0x08
#define SMR_PARITY   0x04   // Parity mode 0=>even, 1=odd
#define   SMR_PARITY_EVEN 0x00
#define   SMR_PARITY_ODD  0x04
#define SMR_PARITY_ON 0x02  // Parity checked
#define   SMR_PARITY_OFF  0x00
#define SMR_LENGTH    0x01  // Character length
#define    SMR_LENGTH_8 0x00
#define    SMR_LENGTH_7 0x01
// Serial status
#define SSR_MSS      0x80   // Modem status has changed
#define SSR_OE       0x40   // Overrun error
#define SSR_FE       0x20   // Framing error
#define SSR_PE       0x10   // Parity error
#define SSR_TxActive 0x08   // Transmitter is active
#define SSR_RxActive 0x04   // Receiver is active
#define SSR_TxEmpty  0x02   // Tx buffer is empty
#define SSR_RxFull   0x01   // Rx buffer contains data
// Modem control
#define SMR_CFG      0x08   // Configuration 0=>normal, 1=>null
#define SMR_MSU      0x04   // Modem status update 1=>enable
#define SMR_DTR      0x02   // Assert DTR
#define SMR_RTS      0x01   // Assert RTS

#define GP4020_UART1 0xE0018000
#define GP4020_UART2 0xE0019000

//
// Initialize a TTY port
//
static void
_tty_init(volatile struct uart *uart)
{
    uart->mode = SMR_STOP_1 | SMR_PARITY_OFF | SMR_LENGTH_8;
    uart->baud = 0x15;  // Magic for 57600
    uart->modem_control = SMR_DTR | SMR_RTS;
    uart->control = SCR_TEN | SCR_REN;
}

//
// Output a character to a TTY port
//
static void
_tty_putc(volatile struct uart *uart, char c)
{
    // Wait for space for character
    do {
    } while ((uart->status & SSR_TxEmpty) == 0);
    uart->TxRx = c;
}

//
// Read a character from a TTY port
//
static char
_tty_getc(volatile struct uart *uart)
{
    do {
#if 0
        if ((uart->status & 0xF0) != 0) {
            tty_puts("\nErr = ");
            tty_puthex(uart->TxRx, 2);
        }
#endif
    } while ((uart->status & SSR_RxFull) == 0);
    return uart->TxRx;
}

//
// Initialize the TTY ports
//
volatile struct uart *uarts[] = {
    (volatile struct uart *)GP4020_UART1,
    (volatile struct uart *)GP4020_UART2
};

void
tty_init(void)
{

#if 0
    tty_puts("\nUart: ");
    tty_puthex(uart->control, 2);
    tty_puts(", ");
    tty_puthex(uart->mode, 2);
    tty_puts(", ");
    tty_puthex(uart->baud, 2);
    tty_puts(", ");
    tty_puthex(uart->TxRx, 2);
    tty_puts("\n");
    tty_puts("\nStat at ");  tty_puthex(&uart->status, 8);  tty_puts("\n");
#endif
    _tty_init(uarts[0]);
    _tty_init(uarts[1]);
}

//
// Write a character to the selected TTY
//
void
tty_putc(int chan, char c)
{
    _tty_putc(uarts[chan], c);
}

//
// Read a character from the selected TTY
//
char
tty_getc(int chan)
{
    return _tty_getc(uarts[chan]);
}

//
// Display a string on the selected TTY
//
void
tty_puts(int chan, char *s)
{
    char c;

    while ((c = *s++) != '\0') {
        if (c == '\n') {
            tty_putc(chan, '\r');
        }
        tty_putc(chan, c);
    }
}

//
// Read characters into a buffer, terminated by a '\n' character
// Note: the '\n' character is not stored
//
int
tty_getline(int chan, char *buf)
{
    char c;
    int len = 0;

    while (((c = tty_getc(chan)) != '\n') && (c != '\r')) {
        tty_putc(chan, c);
        *buf++ = c;
        len++;
    }
    *buf = '\0';
    return len;
}

//
// Display a number in hex
//
void
tty_puthex(int chan, unsigned long val, int length)
{
    char hex[] = "0123456789ABCDEF";
    char str[16];
    char *s = &str[length+3];
    int i;

    *--s = '\0';
    for (i = 0;  i < length;  i++) {
        *--s = hex[(val & 0x0F)];
        val >>= 4;
    }
    *--s = 'x';
    *--s = '0';
    tty_puts(chan, s);
}

