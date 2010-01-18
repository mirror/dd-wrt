/* sja1000p.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Added by T.Motylewski@bfad.de
 * See app. note an97076.pdf from Philips Semiconductors 
 * and SJA1000 data sheet
 * PELICAN mode
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

int sja1000p_chip_config(struct canchip_t *chip);
int sja1000p_extended_mask(struct canchip_t *chip, unsigned long code, unsigned long mask);
int sja1000p_baud_rate(struct canchip_t *chip, int rate, int clock, int sjw,
		int sampl_pt, int flags);
int sja1000p_pre_read_config(struct canchip_t *chip, struct msgobj_t *obj);
int sja1000p_pre_write_config(struct canchip_t *chip, struct msgobj_t *obj,
		struct canmsg_t *msg);
int sja1000p_send_msg(struct canchip_t *chip, struct msgobj_t *obj,
		struct canmsg_t *msg);
int sja1000p_fill_chipspecops(struct canchip_t *chip);
int sja1000p_irq_handler(int irq, struct canchip_t *chip);


/* PeliCAN mode */
enum SJA1000_PeliCAN_regs {
	SJAMOD	= 0x00,
/// Command register
	SJACMR 	= 0x01,
/// Status register
	SJASR	= 0x02,
/// Interrupt register
	SJAIR	= 0x03,
/// Interrupt Enable
	SJAIER	= 0x04,
/// Bus Timing register 0
	SJABTR0 = 0x06,
/// Bus Timing register 1
	SJABTR1	= 0x07,
/// Output Control register
	SJAOCR	= 0x08,
/// Arbitration Lost Capture
	SJAALC	= 0x0b,
/// Error Code Capture
	SJAECC	= 0x0c,
/// Error Warning Limit
	SJAEWLR = 0x0d,
/// RX Error Counter
	SJARXERR = 0x0e,
/// TX Error Counter
	SJATXERR0 = 0x0e,
	SJATXERR1 = 0x0f,
/// Rx Message Counter (number of msgs. in RX FIFO
	SJARMC	= 0x1d,
/// Rx Buffer Start Addr. (address of current MSG)
	SJARBSA	= 0x1e,
/// Transmit Buffer (write) Receive Buffer (read) Frame Information
	SJAFRM = 0x10,
/// ID bytes (11 bits in 0 and 1 or 16 bits in 0,1 and 13 bits in 2,3 (extended))
	SJAID0 = 0x11, SJAID1 = 0x12, 
/// ID cont. for extended frames
	SJAID2 = 0x13, SJAID3 = 0x14,
/// Data start standard frame
	SJADATS = 0x13,
/// Data start extended frame
	SJADATE = 0x15,
/// Acceptance Code (4 bytes) in RESET mode
	SJAACR0	= 0x10,
/// Acceptance Mask (4 bytes) in RESET mode
	SJAAMR0	= 0x14,
/// 4 bytes
	SJA_PeliCAN_AC_LEN = 4, 
/// Clock Divider
	SJACDR = 0x1f
};

/** Mode Register 0x00 */
enum sja1000_PeliCAN_MOD {
	sjaMOD_SM = 1<<4,  // Sleep Mode (writable only in OPERATING mode)
	sjaMOD_AFM= 1<<3,  // Acceptance Filter Mode (writable only in RESET)
	sjaMOD_STM= 1<<2,  // Self Test Mode (writable only in RESET)
	sjaMOD_LOM= 1<<1,  // Listen Only Mode (writable only in RESET)
	sjaMOD_RM = 1	   // Reset Mode
};

/** Command Register 0x01 */
enum sja1000_PeliCAN_CMR { 
	sjaCMR_SRR= 1<<4,  // Self Reception Request (GoToSleep in BASIC mode)
	sjaCMR_CDO= 1<<3,  // Clear Data Overrun
	sjaCMR_RRB= 1<<2,  // Release Receive Buffer
	sjaCMR_AT = 1<<1,  // Abort Transmission
	sjaCMR_TR = 1 };   // Transmission Request

/** Status Register 0x02 */
enum sja1000_SR {
	sjaSR_BS  = 1<<7,  // Bus Status
	sjaSR_ES  = 1<<6,  // Error Status
	sjaSR_TS  = 1<<5,  // Transmit Status
	sjaSR_RS  = 1<<4,  // Receive Status
	sjaSR_TCS = 1<<3,  // Transmission Complete Status
	sjaSR_TBS = 1<<2,  // Transmit Buffer Status
	sjaSR_DOS = 1<<1,  // Data Overrun Status
	sjaSR_RBS = 1 };   // Receive Buffer Status

/** Interrupt Enable Register 0x04 */
enum sja1000_PeliCAN_IER {
	sjaIER_BEIE= 1<<7, // Bus Error Interrupt Enable
	sjaIER_ALIE= 1<<6, // Arbitration Lost Interrupt Enable
	sjaIER_EPIE= 1<<5, // Error Passive Interrupt Enable
	sjaIER_WUIE= 1<<4, // Wake-Up Interrupt Enable
	sjaIER_DOIE= 1<<3, // Data Overrun Interrupt Enable
	sjaIER_EIE = 1<<2, // Error Warning Interrupt Enable
	sjaIER_TIE = 1<<1, // Transmit Interrupt Enable
	sjaIER_RIE = 1,    // Receive Interrupt Enable
	sjaENABLE_INTERRUPTS = sjaIER_BEIE|sjaIER_EPIE|sjaIER_DOIE|sjaIER_EIE|sjaIER_TIE|sjaIER_RIE,
	sjaDISABLE_INTERRUPTS = 0
// WARNING: the chip automatically enters RESET (bus off) mode when 
	// error counter > 255
};

/** Arbitration Lost Capture Register 0x0b.
 * Counting starts from 0 (bit1 of ID). Bits 5-7 reserved*/
enum sja1000_PeliCAN_ALC {
	sjaALC_SRTR = 0x0b,// Arbitration lost in bit SRTR
	sjaALC_IDE  = 0x1c, // Arbitration lost in bit IDE
	sjaALC_RTR  = 0x1f, // Arbitration lost in RTR
};

/** Error Code Capture Register 0x0c*/
enum sja1000_PeliCAN_ECC {
	sjaECC_ERCC1 = 1<<7,
	sjaECC_ERCC0 = 1<<6,
	sjaECC_BIT   = 0,
	sjaECC_FORM  = sjaECC_ERCC0,
	sjaECC_STUFF = sjaECC_ERCC1,
	sjaECC_OTHER = sjaECC_ERCC0 | sjaECC_ERCC1,
	sjaECC_DIR   = 1<<5,	// 1 == RX, 0 == TX
	sjaECC_SEG_M = (1<<5) -1 // Segment mask, see page 37 of SJA1000 Data Sheet
};

/** Frame format information 0x10 */
enum sja1000_PeliCAN_FRM {
	sjaFRM_FF = 1<<7, // Frame Format 1 == extended, 0 == standard
	sjaFRM_RTR = 1<<6, // Remote request
	sjaFRM_DLC_M = (1<<4)-1 // Length Mask
};


/** Interrupt (status) Register 0x03 */
enum sja1000_PeliCAN_IR {
	sjaIR_BEI = 1<<7,  // Bus Error Interrupt
	sjaIR_ALI = 1<<6,  // Arbitration Lost Interrupt
	sjaIR_EPI = 1<<5,  // Error Passive Interrupt (entered error passive state or error active state)
	sjaIR_WUI = 1<<4,  // Wake-Up Interrupt
	sjaIR_DOI = 1<<3,  // Data Overrun Interrupt
	sjaIR_EI  = 1<<2,  // Error Interrupt
	sjaIR_TI  = 1<<1,  // Transmit Interrupt
	sjaIR_RI  = 1      // Receive Interrupt
};

/** Bus Timing 1 Register 0x07 */
enum sja1000_BTR1 {
	sjaMAX_TSEG1 = 15,
	sjaMAX_TSEG2 = 7
};

/** Output Control Register 0x08 */
enum sja1000_OCR {
	sjaOCR_MODE_BIPHASE = 0,
	sjaOCR_MODE_TEST = 1,
	sjaOCR_MODE_NORMAL = 2,
	sjaOCR_MODE_CLOCK = 3,
/// TX0 push-pull not inverted
	sjaOCR_TX0_LH = 0x18,
/// TX1 floating (off)
	sjaOCR_TX1_ZZ = 0
};

/** Clock Divider register 0x1f */
enum sja1000_CDR {
	sjaCDR_PELICAN = 1<<7,
/// bypass input comparator
	sjaCDR_CBP = 1<<6,
/// switch TX1 to generate RX INT
	sjaCDR_RXINPEN = 1<<5,
	sjaCDR_CLK_OFF = 1<<3,
/// f_out = f_osc/(2*(CDR[2:0]+1)) or f_osc if CDR[2:0]==7
	sjaCDR_CLKOUT_DIV1 = 7,
	sjaCDR_CLKOUT_DIV2 = 0,
	sjaCDR_CLKOUT_DIV4 = 1,
	sjaCDR_CLKOUT_DIV6 = 2,
	sjaCDR_CLKOUT_DIV8 = 3,
	sjaCDR_CLKOUT_DIV10 = 4,
	sjaCDR_CLKOUT_DIV12 = 5,
	sjaCDR_CLKOUT_DIV14 = 6,
	sjaCDR_CLKOUT_MASK = 7
};

/** flags for sja1000_baud_rate */
#define BTR1_SAM (1<<1)
