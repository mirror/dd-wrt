/* sja1000.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

int sja1000_enable_configuration(struct canchip_t *chip);
int sja1000_disable_configuration(struct canchip_t *chip);
int sja1000_chip_config(struct canchip_t *chip);
int sja1000_standard_mask(struct canchip_t *chip, unsigned short code, unsigned short mask);
int sja1000_baud_rate(struct canchip_t *chip, int rate, int clock, int sjw, 
						int sampl_pt, int flags);
int sja1000_pre_read_config(struct canchip_t *chip, struct msgobj_t *obj);
int sja1000_pre_write_config(struct canchip_t *chip, struct msgobj_t *obj, 
							struct canmsg_t *msg);
int sja1000_send_msg(struct canchip_t *chip, struct msgobj_t *obj, 
							struct canmsg_t *msg);
int sja1000_check_tx_stat(struct canchip_t *chip);
int sja1000_set_btregs(struct canchip_t *chip, unsigned short btr0, 
							unsigned short btr1);
int sja1000_start_chip(struct canchip_t *chip);
int sja1000_stop_chip(struct canchip_t *chip);
int sja1000_irq_handler(int irq, struct canchip_t *chip);
int sja1000_fill_chipspecops(struct canchip_t *chip);

/* BasicCAN mode address map */
#define SJACR		0x00	/* Control register */
#define SJACMR		0x01	/* Command register */
#define SJASR		0x02	/* Status register */
#define SJAIR		0x03	/* Interrupt register */
#define SJAACR		0x04	/* Acceptance Code register */
#define SJAAMR		0x05	/* Acceptance Mask Register */
#define SJABTR0		0x06	/* Bus Timing register 0 */
#define SJABTR1		0x07 	/* Bus Timing register 1 */
#define SJAOCR		0x08	/* Output Control register */
#define SJACDR		0x1f	/* Clock Divider register */

#define SJATXID1	0x0a	/* Identifier byte 1 */
#define SJATXID0	0x0b	/* Identifier byte 0 */
#define SJATXDAT0	0x0c	/* First data byte */
#define SJATXDAT1	0x0d
#define SJATXDAT2	0x0e
#define SJATDDAT3	0x0f
#define SJATXDAT4	0x10
#define SJATXDAT5	0x11
#define SJATXDAT6	0x12
#define SJATXDAT7	0x13

#define SJARXID1	0x14	/* Identifier byte 1 */
#define SJARXID0	0x15	/* Identifier byte 0 */
#define SJARXDAT0	0x16	/* First data byte */
#define SJARXDAT1	0x17
#define SJARXDAT2	0x18
#define SJARXDAT3	0x19
#define SJARXDAT4	0x1a
#define SJARXDAT5	0x1b
#define SJARXDAT6	0x1c
#define SJARXDAT7	0x1d

/* Command register */
enum sja1000_BASIC_CMR {
	sjaCMR_TR  = 1,		// Transmission request
	sjaCMR_AT  = 1<<1,	// Abort Transmission
	sjaCMR_RRB = 1<<2,	// Release Receive Buffer
	sjaCMR_CDO = 1<<3,	// Clear Data Overrun
	sjaCMR_GTS = 1<<4	// Go To Sleep
};

/* Status Register */
enum sja1000_BASIC_SR {
	sjaSR_RBS = 1,		// Receive Buffer Status
	sjaSR_DOS = 1<<1,	// Data Overrun Status
	sjaSR_TBS = 1<<2,	// Transmit Buffer Status
	sjaSR_TCS = 1<<3,	// Transmission Complete Status
	sjaSR_RS  = 1<<4,	// Receive Status
	sjaSR_TS  = 1<<5,	// Transmit Status
	sjaSR_ES  = 1<<6,	// Error Status
	sjaSR_BS  = 1<<7	// Bus Status
};

/* Control Register */
enum sja1000_BASIC_CR {
	sjaCR_RR  = 1,		// Reset Request
	sjaCR_RIE = 1<<1,	// Receive Interrupt Enable
	sjaCR_TIE = 1<<2,	// Transmit Interrupt Enable
	sjaCR_EIE = 1<<3,	// Error Interrupt Enable
	sjaCR_OIE = 1<<4	// Overrun Interrupt Enable
};

/* Interrupt (status) Register */
enum sja1000_BASIC_IR {
	sjaIR_RI  = 1,		// Receive Interrupt
	sjaIR_TI  = 1<<1,	// Transmit Interrupt
	sjaIR_EI  = 1<<2,	// Error Interrupt
	sjaIR_DOI = 1<<3,	// Data Overrun Interrupt
	sjaIR_WUI = 1<<4	// Wake-Up Interrupt
};

/* Clock Divider Register */
enum sja1000_CDR {
	/* f_out = f_osc/(2*(CDR[2:0]+1)) or f_osc if CDR[2:0]==7 */
	sjaCDR_CLKOUT_DIV1 = 7,
	sjaCDR_CLKOUT_DIV2 = 0,
	sjaCDR_CLKOUT_DIV4 = 1,
	sjaCDR_CLKOUT_DIV6 = 2,
	sjaCDR_CLKOUT_DIV8 = 3,
	sjaCDR_CLKOUT_DIV10 = 4,
	sjaCDR_CLKOUT_DIV12 = 5,
	sjaCDR_CLKOUT_DIV14 = 6,
	sjaCDR_CLKOUT_MASK = 7,
	sjaCDR_CLK_OFF = 1<<3,	// Clock Off
	sjaCDR_RXINPEN = 1<<5,	// TX1 output is RX irq output
	sjaCDR_CBP = 1<<6,	// Input Comparator By-Pass
	sjaCDR_PELICAN = 1<<7	// PeliCAN Mode 
};

/* Output Control Register */
enum sja1000_OCR {
	sjaOCR_MODE_BIPHASE = 0,
	sjaOCR_MODE_TEST = 1,
	sjaOCR_MODE_NORMAL = 2,
	sjaOCR_MODE_CLOCK = 3,
// TX0 push-pull not inverted
	sjaOCR_TX0_LH = 0x18,
// TX0 push-pull inverted
	sjaOCR_TX0_HL = 0x1c,
// TX1 floating (off)
	sjaOCR_TX1_ZZ = 0,
// TX1 pull-down not inverted
	sjaOCR_TX1_LZ = 0x40
};

/** Frame format information 0x11 */
enum sja1000_BASIC_ID0 {
	sjaID0_RTR = 1<<4,	// Remote request
	sjaID0_DLC_M = (1<<4)-1	// Length Mask
};
