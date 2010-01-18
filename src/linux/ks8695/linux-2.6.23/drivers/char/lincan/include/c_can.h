/* c_can.h - Hynix HMS30c7202 ARM generic C_CAN module handling
 * Linux CAN-bus device driver.
 * Written by Sebastian Stolzenberg email:stolzi@sebastian-stolzenberg.de
 * Based on code from Arnaud Westenberg email:arnaud@wanadoo.nl
 * and Ake Hedman, eurosource, akhe@eurosource.se
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

/* 
 * optimized inline version, may it be, that it can be too fast for the chip
 */
extern inline void c_can_write_reg_w(const struct canchip_t *pchip, u16 data, unsigned reg)
{
	can_ioptr_t address = pchip->chip_base_addr + reg;
    #ifndef CONFIG_OC_LINCAN_DYNAMICIO
	writew(data,address);
    #else /*CONFIG_OC_LINCAN_DYNAMICIO*/
	pchip->write_register(data, address);
    #endif /*CONFIG_OC_LINCAN_DYNAMICIO*/
}

extern inline u16 c_can_read_reg_w(const struct canchip_t *pchip, unsigned reg)
{
	can_ioptr_t address = pchip->chip_base_addr + reg;
    #ifndef CONFIG_OC_LINCAN_DYNAMICIO
	return readw(address);
    #else /*CONFIG_OC_LINCAN_DYNAMICIO*/
	return pchip->read_register(address);
    #endif /*CONFIG_OC_LINCAN_DYNAMICIO*/
}

extern can_spinlock_t c_can_spwlock; // Spin lock for write operations
extern can_spinlock_t c_can_sprlock; // Spin lock for read operations
extern can_spinlock_t c_can_if1lock; // spin lock for the if1 register
extern can_spinlock_t c_can_if2lock; // spin lcok for the if2 register

int c_can_if1_busycheck(struct canchip_t *pchip);
int c_can_if2_busycheck(struct canchip_t *pchip);

int c_can_enable_configuration(struct canchip_t *pchip);
int c_can_disable_configuration(struct canchip_t *pchip);
int c_can_chip_config(struct canchip_t *pchip);
int c_can_baud_rate(struct canchip_t *chip, int rate, int clock,
			int sjw, int sampl_pt, int flags);
int c_can_mask(struct msgobj_t *pmsgobj,
	       u32 mask,
	       u16 usedirbit);
int c_can_use_mask(struct msgobj_t *pmsgobj,
		   u16 useflag);
int c_can_clear_objects(struct canchip_t *pchip);
int c_can_config_irqs(struct canchip_t *pchip,
                      u16 irqs);
int c_can_pre_read_config(struct canchip_t *chip, struct msgobj_t *obj);
int c_can_send_msg(struct canchip_t *pchip, struct msgobj_t *pmsgobj,
			struct canmsg_t *pmsg);
int c_can_remote_request(struct canchip_t *pchip, struct msgobj_t *pmsgobj );
int c_can_set_btregs(struct canchip_t *chip,
                     u16 btr0,
		     u16 btr1);
int c_can_start_chip(struct canchip_t *pchip);
int c_can_stop_chip(struct canchip_t *pchip);
int c_can_check_tx_stat(struct canchip_t *pchip);

int c_can_register(struct chipspecops_t *chipspecops);

void c_can_registerdump(struct canchip_t *pchip);

void c_can_if1_registerdump(struct canchip_t *pchip);

void c_can_irq_sync_activities(struct canchip_t *chip, struct msgobj_t *obj);

int c_can_irq_handler(int irq, struct canchip_t *pchip);

int c_can_fill_chipspecops(struct canchip_t *pchip);

/* The CCCE register is not implemented in version 1.2 of C_CAN */
#undef C_CAN_WITH_CCCE

/* The mask of C_CAN registers offsets */
#define C_CAN_REGOFFS_MASK	0xFF

/* SSEE C_CAN Memory map */
/* BasicCAN offsets are multiplied by two */
#define CCCR		0x00	/* Control Register */
#define CCSR		0x02	/* Status Register */
#define CCEC		0x04	/* Error Counting Register */
#define CCBT		0x06	/* Bit Timing Register */
#define CCINTR		0x08	/* Interrupt Register */
#define CCTR		0x0A	/* Test Register */
#define CCBRPE 		0x0C	/* Baud Rate Prescaler Extension Register */

#ifdef C_CAN_WITH_CCCE
#define CCCE		0x0E	/* CAN Enable Register */
#endif /*C_CAN_WITH_CCCE*/

#define CCIF1CR		0x10	/* Interface 1 Command Request Register */
#define CCIF1CM		0x12	/* IF1 Command Mask Register */
#define CCIF1M1		0x14	/* IF1 Mask 1 Register */
#define CCIF1M2		0x16	/* IF1 Mask 2 Register */
#define CCIF1A1		0x18	/* IF1 Arbitration 1 Register */
#define CCIF1A2		0x1A	/* IF1 Arbitration 2 Register */
#define CCIF1DMC	0x1C	/* IF1 Message Control Register */
#define CCIF1DA1	0x1E	/* IF1 Data A 1 Register */
#define CCIF1DA2	0x20	/* IF1 Data A 2 Register */
#define CCIF1DB1	0x22	/* IF1 Data B 1 Register */
#define CCIF1DB2	0x24	/* IF1 Data B 2 Register */

#define CCIF2CR		0x40	/* Interface 2 Command Request Register */
#define CCIF2CM		0x42	/* IF2 Command Mask Register */
#define CCIF2M1		0x44	/* IF2 Mask 1 Register */
#define CCIF2M2		0x46	/* IF2 Mask 2 Register */
#define CCIF2A1		0x48	/* IF2 Arbitration 1 Register */
#define CCIF2A2		0x4A	/* IF2 Arbitration 2 Register */
#define CCIF2DMC	0x4C	/* IF2 Message Control Register */
#define CCIF2DA1	0x4E	/* IF2 Data A 1 Register */
#define CCIF2DA2	0x50	/* IF2 Data A 2 Register */
#define CCIF2DB1	0x52	/* IF2 Data B 1 Register */
#define CCIF2DB2	0x54	/* IF2 Data B 2 Register */

#define CCTREQ1 	0x80	/* Transmission Request 1 Register */
#define CCTREQ2 	0x82	/* Transmission Request 2 Register */

#define CCND1		0x90	/* New Data 1 Register */
#define CCND2		0x92	/* New Data 2 Register */

#define CCINTP1 	0xA0	/* Interrupt Pending 1 Register */
#define CCINTP2		0xA2	/* Interrupt Pending 2 Register */

#define CCIMV1		0xB0	/* Message Valid 1 Register   */
#define CCIMV2		0xB2	/* Message Valid 2 Register   */

/* Control register */
enum c_can_BASIC_CR
{
   CR_INIT = 1,		// Internal Initialization Pending
     CR_MIE  = 1<<1,  // Module Interrupt Enable
     CR_SIE  = 1<<2,	// Status-change Interrupt Enable
     CR_EIE  = 1<<3,	// Error Interrupt Enable
     CR_DAR  = 1<<5,	// Disable Automatic Retransmission
     CR_CCE  = 1<<6,  // Configuration Change Enable
     CR_TEST = 1<<7   // Test Mode Enable
};

/* Status Register */
enum c_can_BASIC_SR
{
   SR_TXOK  = 1<<3,	// Transmitted a Message Successfully
     SR_RXOK  = 1<<4,	// Received a Message Successfully
     SR_EPASS = 1<<5,	// Error Passive
     SR_EWARN = 1<<6,	// Error Warning Status
     SR_BOFF  = 1<<7,  // Bus Off Status
};

/* Status Register Last Error Codes */
enum c_can_BASIC_SRLEC
{
   SRLEC_NE = 0,     // Last Error Code: No Error
     SRLEC_SE = 1,     // LEC: Stuff Error
     SRLEC_FE = 2,     // LEC: Form Error
     SRLEC_AE = 3,     // LEC: Acknowledgement Error
     SRLEC_B1 = 4,     // LEC: Bit1 Error
     SRLEC_B0 = 5,     // LEC: Bit0 Error
     SRLEC_CR = 6      // LEC: CRC Error
};

/* Error Counting Register */
enum c_can_BASIC_EC
{
   EC_REP = 1<<15		// Receive Error Passive
};

/* Interrupt Register */
enum c_can_BASIC_INT
{
   INT_NOINT = 0,		   // No Interrupt is pending
     INT_STAT  = 0x8000   // Status Interrupt
};

/* CAN Test Register */
enum c_can_BASIC_TR
{
   TR_BASIC = 1<<2,  // Basic Mode
     TR_SLNT  = 1<<3,  // Silent Mode
     TR_LOOPB = 1<<4,  // Loop Back Mode
     TR_RX    = 1<<7   // Receive (CAN_RX Pin)
};

/* CAN Test Register TX Control*/
enum c_can_BASIC_TRTX
{
   TRTX_RST = 0,     // Reset value, CAN_TX is controlled by the CAN Core
     TRTX_MON = 1,     // Sample Point can be monitored at CAN_TX pin
     TRTX_DOM = 2,     // CAN_TX pin drives a dominant('0') value
     TRTX_REC = 3      // CAN_TX pin drives a recessive('1') value
};

/* CAN Enable Register */
enum c_can_BASIC_CE
{
   CE_EN  = 1   		// CAN Enable Bit
};

/* Interface X Command Request Register */
enum c_can_BASIC_IFXCR
{
   IFXCR_BUSY = 1<<15   // Busy Flag (Write Access only when Busy='0')
};

/* Interface X Command Mask Register */
enum c_can_BASIC_IFXCM
{
   IFXCM_DB        = 1,		// R/W Data Byte 4-7
     IFXCM_DA        = 1<<1,	// R/W Data Byte 0-3
     IFXCM_TRND      = 1<<2,	// Transmit Request (WRRD=1) or Reset New Date Bit (WRRD=0)
     IFXCM_CLRINTPND = 1<<3,	// Clear Interrupt Pending Bit when reading the Message Object
     IFXCM_CNTRL     = 1<<4,	// Access Interface X Message Control Bits
     IFXCM_ARB       = 1<<5,	// Access Interface X Arbitration
     IFXCM_MASK      = 1<<6,	// Access Interface X Mask Bits
     IFXCM_WRRD      = 1<<7	// Read/Write (write data from Interface Registers to Message Object if ='1')
     //            (read data from Message Object to Interface Registers if ='0')
};

/* Interface X Mask 2 Register */
enum c_can_BASIC_IFXMSK2
{
   IFXMSK2_MDIR = 1<<14, // Mask Message Direction (message direction bit(RTR) used for acceptance filt. or not)
     IFXMSK2_MXTD = 1<<15  // Mask Extended Identifier (extended id bit(IDE) used for acceptance filt. or not)
};

/* Interface X Arbitration 2 Register */
enum c_can_BASIC_IFXARB2
{
   IFXARB2_DIR  = 1<<13,  // Message Direction (transmit='1')
     IFXARB2_XTD  = 1<<14,  // Use Extended Identifier
     IFXARB2_MVAL = 1<<15   // Message Validation
};

/* Interface X Message Control Register */
enum c_can_BASIC_IFXMC
{
   IFXMC_EOB    = 1<<7,    // End of Buffer (marks last Message Object of FIFO Buffer)
     IFXMC_TXRQST = 1<<8,    // Transmit Request
     IFXMC_RMTEN  = 1<<9,    // Remote Enable
     IFXMC_RXIE   = 1<<10,   // Receive Interrupt Enable
     IFXMC_TXIE   = 1<<11,   // Transmit Interrupt Enable
     IFXMC_UMASK  = 1<<12,   // Use Identifier Mask
     IFXMC_INTPND = 1<<13,   // Interrupt Pending
     IFXMC_MSGLST = 1<<14,   // Message Lost (Only valid for direction = receive)
     IFXMC_NEWDAT = 1<<15    // New Data
};

