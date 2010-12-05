//=============================================================================
//
//      ser16c550c.c
//
//      Simple driver for the 16c550c serial controllers on AR7240
//
//=============================================================================

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/hal/hal_arch.h>           // SAVE/RESTORE GP macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // interface API
#include <cyg/hal/hal_intr.h>           // HAL_ENABLE/MASK/UNMASK_INTERRUPTS
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED

#include <cyg/hal/ar7240_soc.h>

extern void hal_ar7240_sys_frequency(void);
extern cyg_uint32 ar7240_ahb_freq;

//-----------------------------------------------------------------------------
// Define the serial registers. The AR7240 is equipped with a 16550C
// serial chip.
#define SER_16550_RBR 0x00*4   // receiver buffer register, read, dlab = 0
#define SER_16550_THR 0x00*4   // transmitter holding register, write, dlab = 0
#define SER_16550_DLL 0x00*4   // divisor latch (LS), read/write, dlab = 1
#define SER_16550_IER 0x01*4   // interrupt enable reg, read/write, dlab = 0
#define SER_16550_DLM 0x01*4   // divisor latch (MS), read/write, dlab = 1
#define SER_16550_IIR 0x02*4   // interrupt identification reg, read, dlab = 0
#define SER_16550_FCR 0x02*4   // fifo control register, write, dlab = 0
#define SER_16550_AFR 0x02*4   // alternate function reg, read/write, dlab = 1
#define SER_16550_LCR 0x03*4   // line control register, read/write
#define SER_16550_MCR 0x04*4   // modem control register, read/write
#define SER_16550_LSR 0x05*4   // line status register, read
#define SER_16550_MSR 0x06*4   // modem status register, read
#define SER_16550_SCR 0x07*4   // scratch pad register

// The interrupt enable register bits.
#define SIO_IER_ERDAI   0x01            // enable received data available irq
#define SIO_IER_ETHREI  0x02            // enable THR empty interrupt
#define SIO_IER_ELSI    0x04            // enable receiver line status irq
#define SIO_IER_EMSI    0x08            // enable modem status interrupt

// The interrupt identification register bits.
#define SIO_IIR_IP      0x01            // 0 if interrupt pending
#define SIO_IIR_ID_MASK 0x0e            // mask for interrupt ID bits

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

/////////////////////////////////////////
// Interrupt Enable Register
#define IER_RCV 0x01
#define IER_XMT 0x02
#define IER_LS  0x04
#define IER_MS  0x08

// Line Control Register
#define LCR_WL5 0x00    // Word length
#define LCR_WL6 0x01
#define LCR_WL7 0x02
#define LCR_WL8 0x03
#define LCR_SB1 0x00    // Number of stop bits
#define LCR_SB1_5 0x04  // 1.5 -> only valid with 5 bit words
#define LCR_SB2 0x04
#define LCR_PN  0x00    // Parity mode - none
#define LCR_PE  0x0C    // Parity mode - even
#define LCR_PO  0x08    // Parity mode - odd
#define LCR_PM  0x28    // Forced "mark" parity
#define LCR_PS  0x38    // Forced "space" parity
#define LCR_DL  0x80    // Enable baud rate latch

// Line Status Register
#define LSR_RSR 0x01
#define LSR_THE 0x20

// Modem Control Register
#define MCR_DTR 0x01
#define MCR_RTS 0x02
#define MCR_INT 0x08   // Enable interrupts
#define MCR_AFE 0x20

// Interrupt status register
#define ISR_None             0x01
#define ISR_Rx_Line_Status   0x06
#define ISR_Rx_Avail         0x04
#define ISR_Rx_Char_Timeout  0x0C
#define ISR_Tx_Empty         0x02
#define IRS_Modem_Status     0x00

// FIFO control register
#define FCR_ENABLE     0x01
#define FCR_CLEAR_RCVR 0x02
#define FCR_CLEAR_XMIT 0x04

//-----------------------------------------------------------------------------
typedef struct {
    CYG_ADDRWORD base;
    cyg_int32 msec_timeout;
    int isr_vector;
} channel_data_t;

static channel_data_t channels[1] = {
    { .base = (CYGARC_UNCACHED_ADDRESS(AR7240_UART_BASE)), 
      .msec_timeout = 1000,
      .isr_vector = 0},
};

//-----------------------------------------------------------------------------
// Set the baud rate

static void
cyg_hal_plf_serial_set_baud(cyg_uint8* port, cyg_uint16 baud_divisor)
{
    cyg_uint8 _lcr;

    HAL_READ_UINT32(port+SER_16550_LCR, _lcr);
    _lcr |= LCR_DL;
    HAL_WRITE_UINT32(port+SER_16550_LCR, _lcr);

    HAL_WRITE_UINT32(port+SER_16550_DLM,(baud_divisor >> 8) & 0xff);
    HAL_WRITE_UINT32(port+SER_16550_DLL, baud_divisor & 0xff);

    _lcr &= ~LCR_DL;
    HAL_WRITE_UINT32(port+SER_16550_LCR, _lcr);
}

//-----------------------------------------------------------------------------
// The minimal init, get and put functions. All by polling.

void
cyg_hal_plf_serial_init_channel(void* __ch_data)
{
    CYG_ADDRWORD port;
    cyg_uint8 _lcr;
    cyg_uint32 freq;

    hal_ar7240_sys_frequency();
    freq = ar7240_ahb_freq;

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    port = ((channel_data_t*)__ch_data)->base;

    /*
     * undocumented. confirm, why write to GPIO for uart?
     */
    ar7240_reg_wr(AR7240_GPIO_OE, 0xcff);
    ar7240_reg_wr(AR7240_GPIO_OUT, 0x3b);
    ar7240_reg_wr(AR7240_GPIO_FUNCTIONS, (ar7240_reg_rd(AR7240_GPIO_FUNCTIONS) | 0x48002));
    ar7240_reg_wr(AR7240_GPIO_OUT, 0x2f);
    

    // Disable port interrupts while changing hardware
    HAL_WRITE_UINT32(port+SER_16550_IER, 0);

    // Set databits, stopbits and parity.
    _lcr = LCR_WL8 | LCR_SB1 | LCR_PN;
    HAL_WRITE_UINT32(port+SER_16550_LCR, _lcr);

    // Set baud rate.
    cyg_hal_plf_serial_set_baud(port, freq / (16 *
                      CYGNUM_HAL_VIRTUAL_VECTOR_CHANNELS_DEFAULT_BAUD));

    // Enable and clear FIFO
    HAL_WRITE_UINT32(port+SER_16550_FCR, (FCR_ENABLE | FCR_CLEAR_RCVR | FCR_CLEAR_XMIT));

#ifdef NOTANYMORE
    // enable RTS to keep host side happy. Also allow interrupts
    HAL_WRITE_UINT32( port+SER_16550_MCR, MCR_DTR | MCR_RTS | MCR_INT);
#endif

    // Don't allow interrupts.
    HAL_WRITE_UINT32(port+SER_16550_IER, 0);
}

void
cyg_hal_plf_serial_putc(void* __ch_data, cyg_uint8 __ch)
{
    CYG_ADDRWORD port;
    cyg_uint8 _lsr;

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    port = ((channel_data_t*)__ch_data)->base;

    CYGARC_HAL_SAVE_GP();

    do {
        HAL_READ_UINT32(port+SER_16550_LSR, _lsr);
    } while ((_lsr & SIO_LSR_THRE) == 0);

    // Now, the transmit buffer is empty
    HAL_WRITE_UINT32(port+SER_16550_THR, __ch);


    CYGARC_HAL_RESTORE_GP();
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    CYG_ADDRWORD port;
    cyg_uint8 _lsr;

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    port = ((channel_data_t*)__ch_data)->base;

    HAL_READ_UINT32(port+SER_16550_LSR, _lsr);
    if ((_lsr & SIO_LSR_DR) == 0)
        return false;

    HAL_READ_UINT32(port+SER_16550_RBR, *ch);

    return true;
}

cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data)
{
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    while(!cyg_hal_plf_serial_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();
    return ch;
}

static void
cyg_hal_plf_serial_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    while(__len-- > 0)
        cyg_hal_plf_serial_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_plf_serial_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_serial_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}


cyg_bool
cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    channel_data_t* chan;
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    chan = (channel_data_t*)__ch_data;

    delay_count = chan->msec_timeout * 10; // delay in .1 ms steps

    for(;;) {
        res = cyg_hal_plf_serial_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        CYGACC_CALL_IF_DELAY_US(100);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static int
cyg_hal_plf_serial_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
#if 0  /* No interrupts yet */
    static int irq_state = 0;
#endif
    channel_data_t* chan;
    cyg_uint8 ier;
    int ret = 0;
    CYGARC_HAL_SAVE_GP();

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    chan = (channel_data_t*)__ch_data;

    switch (__func) {
#if 0  /* No interrupts yet */
    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;

        HAL_READ_UINT32(chan->base + SER_16550_IER, ier);
        ier |= SIO_IER_ERDAI;
        HAL_WRITE_UINT32(chan->base + SER_16550_IER, ier);

        HAL_INTERRUPT_SET_LEVEL(chan->isr_vector, 1);
        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;

        HAL_READ_UINT32(chan->base + SER_16550_IER, ier);
        ier &= ~SIO_IER_ERDAI;
        HAL_WRITE_UINT32(chan->base + SER_16550_IER, ier);

        HAL_INTERRUPT_MASK(chan->isr_vector);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = chan->isr_vector;
        break;
#endif
    case __COMMCTL_SET_TIMEOUT:
    {
        va_list ap;

        va_start(ap, __func);

        ret = chan->msec_timeout;
        chan->msec_timeout = va_arg(ap, cyg_uint32);

        va_end(ap);
    }        
    break;
    case __COMMCTL_SETBAUD:
    {
        cyg_uint32 baud_rate;
        cyg_uint16 baud_divisor;
        CYG_ADDRWORD port = chan->base;
        va_list ap;

        va_start(ap, __func);
        baud_rate = va_arg(ap, cyg_uint32);
        va_end(ap);

        baud_divisor = (ar7240_ahb_freq / 16 / baud_rate);

        // Disable port interrupts while changing hardware
        HAL_READ_UINT32(port+SER_16550_IER, ier);
        HAL_WRITE_UINT32(port+SER_16550_IER, 0);

        // Set baud rate.
        cyg_hal_plf_serial_set_baud(port, baud_divisor);

        // Reenable interrupts if necessary
        HAL_WRITE_UINT32(port+SER_16550_IER, ier);
    }
    break;

    case __COMMCTL_GETBAUD:
        break;
    default:
        break;
    }
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

static int
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
#if 0  /* No interrupts yet */
    int res = 0;
    cyg_uint8 _iir, c;
    channel_data_t* chan;
    CYGARC_HAL_SAVE_GP();

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    chan = (channel_data_t*)__ch_data;

    HAL_INTERRUPT_ACKNOWLEDGE(chan->isr_vector);

    HAL_READ_UINT32(chan->base + SER_16550_IIR, _iir);
    _iir &= SIO_IIR_ID_MASK;

    *__ctrlc = 0;
    if ((_iir == ISR_Rx_Avail) || (_iir == ISR_Rx_Char_Timeout)) {

        HAL_READ_UINT32(chan->base + SER_16550_RBR, c);
    
        if( cyg_hal_is_break( &c , 1 ) )
            *__ctrlc = 1;

        res = CYG_ISR_HANDLED;
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
#endif
    return 0;
}

static void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur;

    cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
#if 0 /* No interrupts yet */
    // Disable interrupts.
    HAL_INTERRUPT_MASK(channels[0].isr_vector);
#endif

    // Init channels
    cyg_hal_plf_serial_init_channel((void*)&channels[0]);

    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &channels[0]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);

#ifdef NOTANYMORE
    // Set channel 1
    CYGACC_CALL_IF_SET_CONSOLE_COMM(1);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &channels[1]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);
#endif

    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
}

void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    cyg_hal_plf_serial_init();
}

//-----------------------------------------------------------------------------
// end of ser16c550c.c

