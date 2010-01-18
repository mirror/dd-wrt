/* i82527.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

int i82527_enable_configuration(struct canchip_t *chip);
int i82527_disable_configuration(struct canchip_t *chip);
int i82527_chip_config(struct canchip_t *chip);
int i82527_baud_rate(struct canchip_t *chip, int rate, int clock, int sjw, 
						int sampl_pt, int flags);
int i82527_standard_mask(struct canchip_t *chip, unsigned short code, 
							unsigned short mask);
int i82527_extended_mask(struct canchip_t *chip, unsigned long code, 
							unsigned long mask);
int i82527_message15_mask(struct canchip_t *chip, unsigned long code, 
							unsigned long mask);
int i82527_clear_objects(struct canchip_t *chip);
int i82527_config_irqs(struct canchip_t *chip, short irqs);
int i82527_pre_read_config(struct canchip_t *chip, struct msgobj_t *obj);
int i82527_pre_write_config(struct canchip_t *chip, struct msgobj_t *obj,
							struct canmsg_t *msg);
int i82527_send_msg(struct canchip_t *chip, struct msgobj_t *obj,
							struct canmsg_t *msg);
int i82527_remote_request(struct canchip_t *chip, struct msgobj_t *obj);
int i82527_set_btregs(struct canchip_t *chip, unsigned short btr0, 
							unsigned short btr1);
int i82527_start_chip(struct canchip_t *chip);
int i82527_stop_chip(struct canchip_t *chip);
int i82527_check_tx_stat(struct canchip_t *chip);
int i82527_irq_handler(int irq, struct canchip_t *chip);
int i82527_fill_chipspecops(struct canchip_t *chip);


#define MSG_OFFSET(object) ((object)*0x10)

#define iCTL 0x00		// Control Register
#define iSTAT 0x01		// Status Register
#define iCPU 0x02		// CPU Interface Register
#define iHSR 0x04		// High Speed Read
#define iSGM0 0x06		// Standard Global Mask byte 0
#define iSGM1 0x07
#define iEGM0 0x08		// Extended Global Mask byte 0
#define iEGM1 0x09
#define iEGM2 0x0a
#define iEGM3 0x0b
#define i15M0 0x0c		// Message 15 Mask byte 0
#define i15M1 0x0d
#define i15M2 0x0e
#define i15M3 0x0f
#define iCLK 0x1f		// Clock Out Register
#define iBUS 0x2f		// Bus Configuration Register
#define iBT0 0x3f		// Bit Timing Register byte 0
#define iBT1 0x4f
#define iIRQ 0x5f		// Interrupt Register
#define iP1C 0x9f		// Port 1 Register
#define iP2C 0xaf		// Port 2 Register
#define iP1I 0xbf		// Port 1 Data In Register
#define iP2I 0xcf		// Port 2 Data In Register
#define iP1O 0xdf		// Port 1 Data Out Register
#define iP2O 0xef		// Port 2 Data Out Register
#define iSRA 0xff		// Serial Reset Address

#define iMSGCTL0	0x00	/* First Control register */
#define iMSGCTL1	0x01	/* Second Control register */
#define iMSGID0		0x02	/* First Byte of Message ID */
#define iMSGID1		0x03
#define iMSGID2		0x04
#define iMSGID3		0x05
#define iMSGCFG		0x06	/* Message Configuration */
#define iMSGDAT0	0x07	/* First Data Byte */
#define iMSGDAT1	0x08
#define iMSGDAT2	0x09
#define iMSGDAT3	0x0a
#define iMSGDAT4	0x0b
#define iMSGDAT5	0x0c
#define iMSGDAT6	0x0d
#define iMSGDAT7	0x0e

/* Control Register (0x00) */
enum i82527_iCTL {
	iCTL_INI = 1,		// Initialization
	iCTL_IE  = 1<<1,	// Interrupt Enable
	iCTL_SIE = 1<<2,	// Status Interrupt Enable
	iCTL_EIE = 1<<3,	// Error Interrupt Enable
	iCTL_CCE = 1<<6		// Change Configuration Enable
};

/* Status Register (0x01) */
enum i82527_iSTAT {
	iSTAT_TXOK = 1<<3,	// Transmit Message Successfully
	iSTAT_RXOK = 1<<4,	// Receive Message Successfully
	iSTAT_WAKE = 1<<5,	// Wake Up Status
	iSTAT_WARN = 1<<6,	// Warning Status
	iSTAT_BOFF = 1<<7	// Bus Off Status
};

/* CPU Interface Register (0x02) */
enum i82527_iCPU {
	iCPU_CEN = 1,		// Clock Out Enable
	iCPU_MUX = 1<<2,	// Multiplex
	iCPU_SLP = 1<<3,	// Sleep
	iCPU_PWD = 1<<4,	// Power Down Mode
	iCPU_DMC = 1<<5,	// Divide Memory Clock
	iCPU_DSC = 1<<6,	// Divide System Clock
	iCPU_RST = 1<<7		// Hardware Reset Status
};

/* Clock Out Register (0x1f) */
enum i82527_iCLK {
	iCLK_CD0 = 1,		// Clock Divider bit 0
	iCLK_CD1 = 1<<1,
	iCLK_CD2 = 1<<2,
	iCLK_CD3 = 1<<3,
	iCLK_SL0 = 1<<4,	// Slew Rate bit 0
	iCLK_SL1 = 1<<5
};

/* Bus Configuration Register (0x2f) */
enum i82527_iBUS {
	iBUS_DR0 = 1,		// Disconnect RX0 Input
	iBUS_DR1 = 1<<1,	// Disconnect RX1 Input
	iBUS_DT1 = 1<<3,	// Disconnect TX1 Output
	iBUS_POL = 1<<5,	// Polarity
	iBUS_CBY = 1<<6		// Comparator Bypass
};

#define RESET 1			// Bit Pair Reset Status
#define SET 2			// Bit Pair Set Status
#define UNCHANGED 3		// Bit Pair Unchanged

/* Message Control Register 0 (Base Address + 0x0) */
enum i82527_iMSGCTL0 {
	INTPD_SET = SET,		// Interrupt pending
	INTPD_RES = RESET,		// No Interrupt pending
	INTPD_UNC = UNCHANGED,
	RXIE_SET  = SET<<2,		// Receive Interrupt Enable
	RXIE_RES  = RESET<<2,		// Receive Interrupt Disable
	RXIE_UNC  = UNCHANGED<<2,
	TXIE_SET  = SET<<4,		// Transmit Interrupt Enable
	TXIE_RES  = RESET<<4,		// Transmit Interrupt Disable
	TXIE_UNC  = UNCHANGED<<4,
	MVAL_SET  = SET<<6,		// Message Valid
	MVAL_RES  = RESET<<6,		// Message Invalid
	MVAL_UNC  = UNCHANGED<<6
};

/* Message Control Register 1 (Base Address + 0x01) */
enum i82527_iMSGCTL1 {
	NEWD_SET = SET,			// New Data
	NEWD_RES = RESET,		// No New Data
	NEWD_UNC = UNCHANGED,
	MLST_SET = SET<<2,		// Message Lost
	MLST_RES = RESET<<2,		// No Message Lost
	MLST_UNC = UNCHANGED<<2,
	CPUU_SET = SET<<2,		// CPU Updating
	CPUU_RES = RESET<<2,		// No CPU Updating
	CPUU_UNC = UNCHANGED<<2,
	TXRQ_SET = SET<<4,		// Transmission Request
	TXRQ_RES = RESET<<4,		// No Transmission Request
	TXRQ_UNC = UNCHANGED<<4,
	RMPD_SET = SET<<6,		// Remote Request Pending
	RMPD_RES = RESET<<6,		// No Remote Request Pending
	RMPD_UNC = UNCHANGED<<6
};

/* Message Configuration Register (Base Address + 0x06) */
enum i82527_iMSGCFG {
	MCFG_XTD = 1<<2,		// Extended Identifier
	MCFG_DIR = 1<<3			// Direction is Transmit
};

void i82527_seg_write_reg(const struct canchip_t *chip, unsigned char data, unsigned address);
unsigned i82527_seg_read_reg(const struct canchip_t *chip, unsigned address);
