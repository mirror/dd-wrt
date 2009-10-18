/*==============================================================================
 * spi.c - routines that communicate with the card using spi
 * 
 * Methods in this file perform all I/O with the media card.
 *
 * This file is broken into 2 sections:
 *
 * 1) Unoptimized methods
 *    
 *    -Methods that have not been optimized for maximum performance. 
 *
 *    -Honour the maximum frequency setting (if set).
 *
 *    -There's little payback trying to optimize these, as they are only used
 *     during card initialization.
 *
 * 2) Optimized methods
 *    
 *    -Methods that have been optimized for maximum performance.
 *
 *    -Don't honour the maximum frequency setting and are coded to run as fast
 *     as possible - don't call any from code that does card initialization
 *     (max 400khz frequency).
 *
 *    -There are different variants of methods to help optimize throughput.
 *     For example, there is rarely a need to both send and receive data at
 *     the same time, so instead of using just spi_io, 3 variants can be
 *     created that have better performance.
 *
 *         r = spi_io(0xab)   - send argument, return data out bits.
 *         spi_io_v_o(0xab)   - send argument, ignore data out bits.
 *         r = spi_io_ff_o()  - send 0xff, return data out bits.
 *         spi_io_ff_v_o()    - send 0xff, ignore data out bits.
 *
 *    -Efficiency was tuned by having the compiler dump the generated MIPS
 *     assembler instructions, then adjusting the C source to minimize the
 *     number of MIPS instructions generated (wherever possible).
 *
 *    -Looping is avoided or reduced where possible to improve efficiency. This
 *     does increase module size.
 *
 *    -The compiler, with optimization on, winds up inlining the vast majority
 *     of the code, so even though we have more methods, code size is not
 *     greatly impacted.
 *
 * Conventions:
 *
 *    -All methods begin with "spi_".
 *    -Optimized methods are suffixed with "_o" to make them easy to identify.
 *    -Variants of methods that return no (void) value are suffixed with "_v".
 *    -Only variables used exclusively by methods in this module are defined here.
 *     All begin with "spi_". 
 *============================================================================*/

// Handy to turn off inlining globally when analyzing generated MIPS assembler code
#define INLINE inline

// Function prototypes - non optimized functions
static int spi_init(void);
static int spi_freq_max(int);
static void spi_freq_wait(cycles_t);
static void spi_cs_ass(void); 
static void spi_cs_dea(void); 
static unsigned char spi_io(unsigned char);
static unsigned char spi_mmc_cmd_r(unsigned char, unsigned int, unsigned char, unsigned char *, int);
static unsigned char spi_mmc_read_blk(unsigned char, unsigned int, unsigned char, unsigned char *, int);

// Function prototypes - optimized functions
static inline void spi_cs_ass_o(void); 
static inline void spi_cs_dea_o(void); 
static inline void spi_io_ff_v_o(void);
static inline void spi_io_2ff_v_o(void);
static inline void spi_io_6ff_v_o(void);
static inline unsigned char spi_io_ff_o(void);
static inline void spi_io_v_o(unsigned char);
static inline unsigned char spi_mmc_write_multi_o(unsigned int, unsigned char *, int, unsigned char, unsigned char);
static inline unsigned char spi_mmc_read_multi_o(unsigned int, unsigned char *, int, unsigned char, unsigned char);

// Variables used to set GPIO port state (cs low = asserted)
static unsigned char spi_ps_d0_c1;			// data low, clock hi, cs low 
static unsigned char spi_ps_d0_c0;			// data low, clock low, cs low
static unsigned char spi_ps_d1_c1;			// data hi, clock hi, cs low
static unsigned char spi_ps_d1_c0;			// data hi, clock low, cs low

// Masks and bit numbers - spi->GPIO mappings
static unsigned char spi_di_mask;
static unsigned char spi_do_mask;
static unsigned char spi_do_pin;
static unsigned char spi_cl_mask;
static unsigned char spi_cs_mask;

// Vars that are used to control the max clock frequency
static cycles_t spi_clk_delay = 0;			// set max clock frequency (khz)
static cycles_t spi_clk_last = 0;			// time of last clock transition


//======================= UNOPTIMIZED METHODS ==================================


/*------------------------------------------------------------------------------
 * spi_init
 * 
 * Initialize spi hardware - specify which IO pins used for SPI communications
 *
 * Set the required state, IO mode, and control mode for SPI.
 *
 *----------------------------------------------------------------------------*/
static int spi_init() {

    // Convert gpio pin nums to bit mask used to set/clear appropriate GPIO bit.
    spi_di_mask = (1 << din);
    spi_do_mask = (1 << dout);
    spi_cl_mask = (1 << clk);
    spi_cs_mask = (1 << cs);
    spi_do_pin  = dout;

    // Disable weak pullups on all GPIO pins used by this module
    *gpio_control &= ~( spi_di_mask | spi_cl_mask | spi_cs_mask | spi_do_mask);

    // Set din, clk and cs GPIO pins to outputs, dout GPIO pin to input
    *gpio_enable = (*gpio_enable | spi_di_mask | spi_cl_mask | spi_cs_mask) & ~spi_do_mask;

    // Initial setup of port state variables
    spi_ps_d1_c0 = ((*gpio_output | spi_di_mask) & ~spi_cl_mask) & ~spi_cs_mask;	// din hi, clock low
    spi_ps_d1_c1 = spi_ps_d1_c0 | spi_cl_mask;			// din hi, clock hi
    spi_ps_d0_c1 = spi_ps_d1_c1 & ~spi_di_mask;			// din low, clock hi
    spi_ps_d0_c0 = spi_ps_d0_c1 & ~spi_cl_mask;			// din low, clock hi

    // Initial state - data hi, clock low, cs hi - restores output state of unused pins
    *gpio_output = spi_ps_d1_c0 | spi_cs_mask;

    return 0;
}


/*------------------------------------------------------------------------------
 * SPI Freq
 *
 * Set the maximum frequency (in KHz) the spi clock may run at.
 * A value of zero means no maximum frequency - as fast as possible.
 *
 * Notes: Complete
 *----------------------------------------------------------------------------*/
static int spi_freq_max(int freq) {

    /* calculate and save the period of the specified frequency */
    if ( freq > 0 ) {
	spi_clk_delay = (loops_per_jiffy*(HZ<<1)) / (freq*1000*2);
    } else {
	spi_clk_delay = 0;
    }

    return 0;
}


/*------------------------------------------------------------------------------
 * SPI Freq Wait
 *
 * Used to ensure enough time has elapsed between clock transitions so clock
 * frequency remains below the set maximum.
 *
 * Notes:
 * 
 *   -spi_clk_last is global. It is also set in the spi_cs_ass method...
 *----------------------------------------------------------------------------*/
static void spi_freq_wait(cycles_t wait) {
    cycles_t now, delta, future;

    // Calculate how many cycles have elapsed since clock last toggled
    now = get_cycles();
    delta = (now > spi_clk_last) ? (now - spi_clk_last) : (spi_clk_last - now);   // abs(now - last)

    if (delta < wait) {

	// Less than requested wait - calc future time we must wait until
	future = now + (wait - delta);	/* can overflow and wrap arround */

	// Now wait until that future time - handle overflow...
	if (future < now) {
	    // Value overflowed - wait till now <= future 
	    while (get_cycles() > future) ;
	}
	while (get_cycles() < future) ;

    }

    // Save current cycles value so we know when next transition allowed.
    spi_clk_last = get_cycles();
}


/*------------------------------------------------------------------------------
 * spi_cs_ass
 *
 * Assert CS signal. Unoptimized version that supports max frequency setting.
 *
 * Notes: Complete
 *----------------------------------------------------------------------------*/
static INLINE void spi_cs_ass(void) {
    spi_clk_last = get_cycles();			// assume clock just transitioned
    spi_ps_d1_c0 = ((*gpio_output | spi_di_mask) & ~spi_cl_mask) & ~spi_cs_mask;	// din hi, clock low
    spi_ps_d1_c1 = spi_ps_d1_c0 | spi_cl_mask;		// din hi, clock hi
    spi_ps_d0_c1 = spi_ps_d1_c1 & ~spi_di_mask;		// din low, clock hi
    spi_ps_d0_c0 = spi_ps_d0_c1 & ~spi_cl_mask;		// din low, clock hi
    *gpio_output = spi_ps_d1_c0;			// assert cs
}


/*------------------------------------------------------------------------------
 * spi_cs_dea
 *
 * Deassert CS signal - send 8 clocks to release DO
 *
 * Notes: Complete
 *----------------------------------------------------------------------------*/
static INLINE void spi_cs_dea(void) {
    *gpio_output = spi_ps_d1_c0 | spi_cs_mask;	// deassert cs (cs line hi) and clock low
    spi_io(0xff);					// 8 clocks
}


/*------------------------------------------------------------------------------
 * spi_io
 *
 * Send/Receive 8 bits of data at same time.
 *
 * Notes: Complete
 *----------------------------------------------------------------------------*/
static unsigned char spi_io(unsigned char data_out) {
    volatile unsigned char * out = gpio_output;
    volatile unsigned char * in = gpio_input;
    unsigned char r = 0;
    int i;
  
    for (i = 7; i >= 0; i--) {

	// Set data bit appropriately and toggle clock hi 
	if (data_out & (1 << i)) {
	    *out |= spi_di_mask;
        } else {
	    *out &= ~spi_di_mask;
  	}
	*out |= spi_cl_mask;
    
	// read data from card...
	r <<= 1;
	r |= ((*in & spi_do_mask) ? 1 : 0);

	// toggle clock low
	if (spi_clk_delay) spi_freq_wait(spi_clk_delay);
	*out &= ~spi_cl_mask;
    
    }
  
    return r;
}


/*------------------------------------------------------------------------------
 * spi_mmc_cmd_r
 *
 * Send a command and retrieve the command response
 *
 * Supports commands that return r1, r3 and r7 response formats. 
 *
 * The first byte of the response (the r1 data) is the subroutine return value. 
 * If executing commands that return additional bytes in the response (r3 and r7 formats), 
 * pass a buffer and the additional number of bytes to read in the buf and len values. 
 *
 * Notes: Complete
 *----------------------------------------------------------------------------*/
static unsigned char spi_mmc_cmd_r(unsigned char cmd, unsigned int param, unsigned char crc7, unsigned char *buf, int len) {
    int i;
    unsigned char r = 0;
  
    // Assert CS
    spi_cs_ass();

    // send command, arguments, crc
    spi_io(0x40 | cmd);
    spi_io(param>>24);
    spi_io(param>>16);
    spi_io(param>>8);
    spi_io(param);
    spi_io(crc7);

    // wait up to 8 bytes for command r1 response
    for (i = 0; i < 8; i++) {
	r = spi_io(0xff);
	if (r != 0xff) break;
    }

    // return additional response bytes in buffer if requested
    for (i = 0; i < len; i++) buf[i] = spi_io(0xff);

    spi_cs_dea();
  
    return r;
}


/*------------------------------------------------------------------------------
 * spi_mmc_read_blk
 *
 * Send an mmc command that returns a single block of data
 *----------------------------------------------------------------------------*/
static unsigned char spi_mmc_read_blk(unsigned char cmd, unsigned int param, unsigned char crc7, unsigned char *buf, int len) {
    int i;
    unsigned char r = 0;

    // Assert CS
    spi_cs_ass();
  
    // send command, arguments, crc
    spi_io(0x40 | cmd);
    spi_io(param>>24);
    spi_io(param>>16);
    spi_io(param>>8);
    spi_io(param);
    spi_io(crc7);

    // wait up to 8 bytes for command response
    for (i = 0; i < 8; i++) {
	r = spi_io(0xff);
	if (r == 0x00) break;
    }
    if (r != 0x00) goto err1;

    // wait for start of data token
    for (i = 0; i < 1000000; i++) {
	r = spi_io(0xff);
	if (r == 0xfe) break;
    }
    if (r != 0xfe) goto err1;

    // Read in data 
    for (i = 0; i < len; i++) buf[i] = spi_io(0xff);

    // Read and discard the data packet crc bytes
    spi_io(0xff);
    spi_io(0xff);

    // Deassert CS - send 8 bits to release DO
    spi_cs_dea();
  
    return 0;

err1:
    spi_cs_dea();
    return r;
}


//======================== OPTIMIZED METHODS ===================================


/*------------------------------------------------------------------------------
 * spi_cs_ass_o
 *
 * Assert CS signal - don't care about max frequency setting
 *
 * Notes:
 *
 *   -Re-read port to pick up any changes to wlan/power values.
 *   -Precalculate port states - allows bit changes via assignement (1 cycle)
 *    vs by bit operations (3 cycles)
 *----------------------------------------------------------------------------*/
static INLINE void spi_cs_ass_o(void) {
    spi_ps_d1_c0 = ((*gpio_output | spi_di_mask) & ~spi_cl_mask) & ~spi_cs_mask;	// din hi, clock low
    spi_ps_d1_c1 = spi_ps_d1_c0 | spi_cl_mask;		// din hi, clock hi
    spi_ps_d0_c1 = spi_ps_d1_c1 & ~spi_di_mask;		// din low, clock hi
    spi_ps_d0_c0 = spi_ps_d0_c1 & ~spi_cl_mask;		// din low, clock hi
    *gpio_output = spi_ps_d1_c0;			// assert cs
}


/*------------------------------------------------------------------------------
 * spi_cs_dea_o 
 *
 * Deassert CS signal. Send 8 clocks to release DO.
 *
 * Notes:
 *   - Local vars out, clk_hi, clk_low improve efficiency of generated code
 *----------------------------------------------------------------------------*/
static INLINE void spi_cs_dea_o(void) {
    volatile unsigned char * out = gpio_output;
    const unsigned char clk_hi = spi_ps_d1_c1 | spi_cs_mask;
    const unsigned char clk_low = spi_ps_d1_c0 | spi_cs_mask;

    *out = clk_low;	// deassert cs (cs line hi) and clock low
    *out = clk_hi; *out = clk_low;
    *out = clk_hi; *out = clk_low;
    *out = clk_hi; *out = clk_low;
    *out = clk_hi; *out = clk_low;
    *out = clk_hi; *out = clk_low;
    *out = clk_hi; *out = clk_low;
    *out = clk_hi; *out = clk_low;
    *out = clk_hi; *out = clk_low;
}


/*------------------------------------------------------------------------------
 * spi_io_ff_v_o
 *
 * Send 0xff - don't worry about reading response
 *
 * Notes:
 *
 *   - One asm instruction per clock toggle!
 *   - Could this possibly exceed 25MHz max for clock?
 *   - Local vars out, clk_hi, clk_low improve efficiency of generated code
 *----------------------------------------------------------------------------*/
static INLINE void spi_io_ff_v_o(void) {
    volatile unsigned char * out = gpio_output;
    const unsigned char clk_hi = spi_ps_d1_c1;
    const unsigned char clk_low = spi_ps_d1_c0;

    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
}


/*------------------------------------------------------------------------------
 * spi_io_2ff_v_o
 *
 * Send 0xff 2 times - don't worry about reading response
 *
 * Notes:
 *
 *   - Ever so slightly more efficient than calling spi_io_ff_v_o 2 times 
 *----------------------------------------------------------------------------*/
static INLINE void spi_io_2ff_v_o(void) {
    volatile unsigned char * out = gpio_output;
    const unsigned char clk_hi = spi_ps_d1_c1;
    const unsigned char clk_low = spi_ps_d1_c0;

    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
}


/*------------------------------------------------------------------------------
 * spi_io_6ff_v_o
 *
 * Send 0xff 6 times - don't worry about reading response
 *
 * Notes:
 *
 *   - Ever so slightly more efficient than calling spi_io_ff_v_o 6 times 
 *----------------------------------------------------------------------------*/
static INLINE void spi_io_6ff_v_o(void) {
    volatile unsigned char * out = gpio_output;
    const unsigned char clk_hi = spi_ps_d1_c1;
    const unsigned char clk_low = spi_ps_d1_c0;

    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
    *out = clk_low; *out = clk_hi;
}


/*------------------------------------------------------------------------------
 * spi_io_ff_o
 *
 * Send 0xff - read and return response.
 *
 * Notes:
 *
 *   - Local vars improve efficiency of generated code
 *   - 'r' must be an int (because of the way shifting is done). Approach
 *     allows efficient read with gpio assignment via module parm. Added
 *     bonus: switch to int saves one instruction per read (5 to 4).
 *   - Final shift fixes up return value.
 *----------------------------------------------------------------------------*/
static INLINE unsigned char spi_io_ff_o(void) {
    volatile unsigned char *out = gpio_output;
    volatile unsigned char *in = gpio_input;
    const unsigned char clk_hi = spi_ps_d1_c1;
    const unsigned char clk_low = spi_ps_d1_c0;
    const unsigned char do_mask = spi_do_mask;
    unsigned int r = 0;

    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask);

    // Fixup result - shift right based on gpio pin dout assigned to...
    r >>= spi_do_pin;

    return r;
}

/*------------------------------------------------------------------------------
 * spi_io_4ff_o
 *
 * Send 0xffffff - read and return 32 bit response (word).
 *
 * Notes:
 *
 *   - Local vars improve efficiency of generated code
 *   - Slightly more efficient than 4 calls to spi_io_ff_o
 *   - The MIPS architecture is little endian, so bytes have to be reversed in
 *     the register before the word is written to memory
 *     byte order while reading, so when bytes are reversed when the word is written
 *     to memory, they are stored correctly!
 *
 *     e.g. Card sends the characters "abcd" as 4 bytes.
 *          Algorithm reads it into the 32 bit register as "dcba" (bytes reversed).
 *          Memory write reverses the bytes - storing "abcd" in ascending locations.
 *----------------------------------------------------------------------------*/
static INLINE unsigned int spi_io_4ff_o(void) {
    volatile unsigned char *out = gpio_output;
    volatile unsigned char *in = gpio_input;
    const unsigned char clk_hi = spi_ps_d1_c1;
    const unsigned char clk_low = spi_ps_d1_c0;
    const unsigned char do_mask = spi_do_mask;
    const unsigned char do_pin  = spi_do_pin;
    unsigned int r = 0;
    unsigned int w = 0;

    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); 
    r >>= do_pin; w |= r; r = 0;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); 
    r >>= do_pin; r <<= 8; w |= r; r = 0;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); 
    r >>= do_pin; r <<= 16; w |= r; r = 0;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); r <<= 1;
    *out = clk_low; *out=clk_hi; r |= (*in & do_mask); 
    r >>= do_pin; r <<= 24; w |= r;

    return w;
}


/*------------------------------------------------------------------------------
 * spi_io_v_o
 *
 * Send 8 bits of data - don't worry about reading response.
 *
 * Notes:
 *
 *   - Local vars improve efficiency of generated code
 *   - if structures - larger code size, but less instructions per bit written
 *     than attempting to use bit shift/and/or instructions...
 *----------------------------------------------------------------------------*/
static INLINE void spi_io_v_o(unsigned char data_out) {
    volatile unsigned char * out = gpio_output;
    const unsigned int d1_c0 = spi_ps_d1_c0;
    const unsigned int d1_c1 = spi_ps_d1_c1;
    const unsigned int d0_c0 = spi_ps_d0_c0;
    const unsigned int d0_c1 = spi_ps_d0_c1;

    if (data_out & (1 << 7)) { *out = d1_c0; *out = d1_c1; } else { *out = d0_c0; *out = d0_c1; }
    if (data_out & (1 << 6)) { *out = d1_c0; *out = d1_c1; } else { *out = d0_c0; *out = d0_c1; }
    if (data_out & (1 << 5)) { *out = d1_c0; *out = d1_c1; } else { *out = d0_c0; *out = d0_c1; }
    if (data_out & (1 << 4)) { *out = d1_c0; *out = d1_c1; } else { *out = d0_c0; *out = d0_c1; }
    if (data_out & (1 << 3)) { *out = d1_c0; *out = d1_c1; } else { *out = d0_c0; *out = d0_c1; }
    if (data_out & (1 << 2)) { *out = d1_c0; *out = d1_c1; } else { *out = d0_c0; *out = d0_c1; }
    if (data_out & (1 << 1)) { *out = d1_c0; *out = d1_c1; } else { *out = d0_c0; *out = d0_c1; }
    if (data_out & (1 << 0)) { *out = d1_c0; *out = d1_c1; } else { *out = d0_c0; *out = d0_c1; }
}


/*------------------------------------------------------------------------------
 * spi_mmc_write_multi_o
 *
 * Write multiple 512 byte blocks from buffer with the multi block write CMD25
 *
 * Return Codes:
 *
 *    0 - Successful write
 *    1 - Busy wait timeout
 *    2 - CMD25 (multi block write) error
 *    3 - Data response token not received
 *    4 - Data response token - busy error
 *   11 - Data rejected due to crc error
 *   14 - Data rejected due to write error
 *
 * Notes:
 *
 *   - Local vars improve efficiency of generated code
 *   - Possible improvements - unsigned int array - send 32 bits at a time
 *   - Try pre-erase command and analyze effect
 *----------------------------------------------------------------------------*/
static INLINE unsigned char spi_mmc_write_multi_o(unsigned int addr, unsigned char *buf, int blocks, unsigned char first, unsigned char last) {
    volatile unsigned char *in = gpio_input;
    volatile unsigned char *out = gpio_output;
    const unsigned char clk_hi = spi_ps_d1_c1;
    const unsigned char clk_low = spi_ps_d1_c0;
    const unsigned int do_mask = spi_do_mask;
    unsigned char *p = buf;
    int err = 0;
    int r = 0;
    int i,j;

    // Only send the multi block write if first first buffer in a request
    // Otherwise it's a continuation of an already running request
    if (first) {

	// send multi block write command, address, crc
	spi_cs_ass_o();
	spi_io_v_o(0x40 | 25);
	spi_io_v_o(addr>>24);
	spi_io_v_o(addr>>16);
	spi_io_v_o(addr>>8);
	spi_io_v_o(addr);
	spi_io_ff_v_o();

	// wait up to 8 bytes for command response
	for (i = 0; i < 8; i++) {
	    r = spi_io_ff_o();
	    if (r == 0x00) break;
	}
	if (r != 0x00) return 2;
    }

    // process each block in a loop
    for (j=0; j < blocks; j++) {

	// write start of data token
	spi_io_v_o(0xfc);

	// write data block 
	for (i = 0; i < 512; i++) {
	    spi_io_v_o(*p);
	    *p++;
	}

	// write dummy crc (2 bytes)
	spi_io_2ff_v_o();

	// wait up to 9 bytes for data response token
	for (i = 0; i < 29; i++) {
	    r = spi_io_ff_o();
	    if (r != 0xff) break;
	}
	if ((r & 0x0f) != 0x05) {
	    if (r == 0xff)
		err = 3;
	    else if (r == 0x00)
		err = 4;
	    else
		err = r & 0x0f;
	    break;
	} 

	// Busy state follows data response token (dout pulled low).
	// Keep the clock running until it's done..
	*out = clk_low;
	for (i = 1; i < 1000000; i++) {
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    if (*in & do_mask) break;
	    yield();
	}
	LOG_DEBUG(DBG_BUSY, "Write_multi: Data response busy: %d clock cycles\n",i * 8);
    }

    // Only send the stop transmission if last buffer, otherwise we'll be called again
    // with the next buffer.
    if (last) {

	// send stop transmission token + 1 byte till busy flag appears
	spi_io_v_o(0xfd);
	spi_io_ff_v_o();

	// Busy state follows stop transmission token (dout pulled low).
	// Keep the clock running until it's done..
	for (i = 1; i < 1000000; i++) {
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    if (*in & do_mask) break;
	    yield();
	}
	LOG_DEBUG(DBG_BUSY, "Write_multi: Stop tran busy: %d clock cycles\n",i * 8);

	spi_cs_dea_o();
    }
  
    return err;
}


/*------------------------------------------------------------------------------
 * spi_mmc_read_multi_o
 *
 * Read multiple 512 byte blocks into a buffer with the multi block read CMD18
 *
 * Return Codes:
 *
 *    0 - Successful write
 *    1 - Busy wait timeout
 *    2 - CMD18 (multi block read) error
 *    3 - Start of data packet not received
 *   11 - Data rejected due to crc error
 *   14 - Data rejected due to write error
 *
 * Notes:
 *
 *   - Local vars improve efficiency of generated code
 *   - Possible improvements - unsigned int array - send 32 bits at a time
 *     read 32 bits at at time...
 *----------------------------------------------------------------------------*/
static INLINE unsigned char spi_mmc_read_multi_o(unsigned int addr, unsigned char *buf, int blocks, unsigned char first, unsigned char last) {
    volatile unsigned char *in = gpio_input;
    volatile unsigned char *out = gpio_output;
    const unsigned char clk_hi = spi_ps_d1_c1;
    const unsigned char clk_low = spi_ps_d1_c0;
    const unsigned int do_mask = spi_do_mask;
    unsigned int *p;
    int r = 0;
    int err = 0;
    int i,j;

    p = (unsigned int *) buf;			// Cast to an int pointer for 32bit io...

    // Only send the multi block write if first first buffer in a request
    // Otherwise it's a continuation of an already running request
    if (first) {

	// send multi block read command, address, crc
	spi_cs_ass_o();
	spi_io_v_o(0x40 | 18);
	spi_io_v_o(addr>>24);
	spi_io_v_o(addr>>16);
	spi_io_v_o(addr>>8);
	spi_io_v_o(addr);
	spi_io_ff_v_o();

	// wait up to 8 bytes for command response
	for (i = 0; i < 8; i++) {
	    r = spi_io_ff_o();
	    if (r == 0x00) break;
	}
	if (r != 0x00) goto err2;
    }

    // process each block in a loop
    for (j=0; j < blocks; j++) {

	// wait for start of data token
	for (i = 1; i < 1000000; i++) {
	    r = spi_io_ff_o();
	    if (r != 0xff) break;
	}
	if (r != 0xfe) {
	    err = 3;
	    break;
	}
	LOG_DEBUG(DBG_BUSY, "Read_multi: Data token busy: %d clock cycles\n",i * 8);

	// Read in data block a word at a time. Reduce looping overhead by doing
        // multiple reads per an iteration of the loop. 
	for (i = 0; i < 512/4/4; i++) {
	    *p = spi_io_4ff_o(); p++; 
	    *p = spi_io_4ff_o(); p++; 
	    *p = spi_io_4ff_o(); p++; 
	    *p = spi_io_4ff_o(); p++; 
	}

	// Read and discard the data packet crc bytes
	spi_io_2ff_v_o();
    }

    // Only send the stop transmission if last buffer, otherwise we'll be called again
    // with the next buffer.
    if (last) {

	// send stop transmission command, parm, crc and skip following stuff byte.
	spi_io_v_o(0x40 | 12);
	spi_io_6ff_v_o();	// parm (4) - crc (1) - stuff byte (1)

	// wait up to 8 bytes for command response
	for (i = 0; i < 8; i++) {
	    r = spi_io_ff_o();
	    if (r == 0x00) break;
	}
	if (r != 0x00) err=4;

	// Busy state can follow CMD12
	*out = clk_low;
	for (i = 1; i < 1000000; i++) {
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    *out = clk_low; *out = clk_hi;
	    if (*in & do_mask) break;
	    yield();
	}
	LOG_DEBUG(DBG_BUSY, "Read_multi: Stop tran busy: %d clock cycles\n",i * 8);
	if ((*in & do_mask) == 0) goto err1;

	spi_cs_dea_o();
    }
  
    return err;

err2:
    err++;
err1:
    err++;
    spi_cs_dea_o();
    return err;
}
