/* hcan2.h
* Header file for the Linux CAN-bus driver.
* This software is released under the GPL-License.
*/

int hcan2_chip_config(struct canchip_t *chip);
int hcan2_enable_configuration(struct canchip_t *chip);
int hcan2_disable_configuration(struct canchip_t *chip);

int hcan2_baud_rate(struct canchip_t *chip, int rate, int clock, int sjw,	int sampl_pt, int flags);
int hcan2_set_btregs(struct canchip_t *chip, unsigned short btr0, unsigned short btr1);

int hcan2_start_chip(struct canchip_t *chip);
int hcan2_stop_chip(struct canchip_t *chip);
int hcan2_attach_to_chip(struct canchip_t *chip);
int hcan2_release_chip(struct canchip_t *chip);

int hcan2_standard_mask(struct canchip_t *chip, unsigned short code, unsigned short mask);
int hcan2_extended_mask(struct canchip_t *chip, unsigned long code, unsigned long mask);
int hcan2_message15_mask(int irq, struct canchip_t *chip);

int hcan2_pre_read_config(struct canchip_t *chip, struct msgobj_t *obj);
int hcan2_pre_write_config(struct canchip_t *chip, struct msgobj_t *obj, struct canmsg_t *msg);
int hcan2_send_msg(struct canchip_t *chip, struct msgobj_t *obj, struct canmsg_t *msg);
int hcan2_remote_request(struct canchip_t *chip, struct msgobj_t *obj);

int hcan2_irq_handler(int irq, struct canchip_t *chip);
int hcan2_irq_accept(int irq, struct canchip_t *chip);
int hcan2_config_irqs(struct canchip_t *chip, short irqs);

int hcan2_clear_objects(struct canchip_t *chip);
int hcan2_check_tx_stat(struct canchip_t *chip);
int hcan2_check_MB_tx_stat(struct canchip_t *chip, struct msgobj_t *obj);
int hcan2_wakeup_tx(struct canchip_t *chip, struct msgobj_t *obj);
int hcan2_filtch_rq(struct canchip_t *chip, struct msgobj_t * obj);

int hcan2_register(struct chipspecops_t *chipspecops);
int hcan2_fill_chipspecops(struct canchip_t *chip);

int hcan2_reset_chip(struct canchip_t *chip);


extern inline void can_write_reg_w(const struct canchip_t *pchip, uint16_t data, unsigned reg)
{
	can_ioptr_t address = pchip->chip_base_addr + reg;
    #ifndef CONFIG_OC_LINCAN_DYNAMICIO
	writew(data,address);
    #else /*CONFIG_OC_LINCAN_DYNAMICIO*/
	pchip->write_register(data, address);
    #endif /*CONFIG_OC_LINCAN_DYNAMICIO*/
}

extern inline uint16_t can_read_reg_w(const struct canchip_t *pchip, unsigned reg)
{
	can_ioptr_t address = pchip->chip_base_addr + reg;
    #ifndef CONFIG_OC_LINCAN_DYNAMICIO
	return readw(address);
    #else /*CONFIG_OC_LINCAN_DYNAMICIO*/
	return pchip->read_register(address);
    #endif /*CONFIG_OC_LINCAN_DYNAMICIO*/
}


/* BasicCAN mode address map */
#define HCAN2_MCR		  0x00000000	/* Master control register */
#define HCAN2_GSR		  0x00000002	/* General status register */
#define HCAN2_BCR1		  0x00000004	/* Bit configuration register 1 */
#define HCAN2_BCR0		  0x00000006	/* Bit configuration register 0 */
#define HCAN2_IRR		  0x00000008	/* Interrupt request register */
#define HCAN2_IMR		  0x0000000a	/* Interrupt mask register */
#define HCAN2_TECREC	  0x0000000c	/* 15:8 Transmit error counter 7:0 Receive error counter */
#define HCAN2_TXPR1		  0x00000020	/* Transmit pending request register 1 */
#define HCAN2_TXPR0		  0x00000022	/* Transmit pending request register 0 */
#define HCAN2_TXCR1		  0x00000028	/* Transmit cancel register 1 */
#define HCAN2_TXCR0		  0x0000002a	/* Transmit cancel register 0 */
#define HCAN2_TXACK1	  0x00000030	/* Transmit acknowledge register 1 */
#define HCAN2_TXACK0	  0x00000032	/* Transmit acknowledge register 0 */
#define HCAN2_ABACK1	  0x00000038	/* Abort acknowledge register 1 */
#define HCAN2_ABACK0	  0x0000003a	/* Abort acknowledge register 0 */
#define HCAN2_RXPR1		  0x00000040	/* Receive data frame pending register 1 */
#define HCAN2_RXPR0		  0x00000042	/* Receive data frame pending register 0 */
#define HCAN2_RFPR1		  0x00000048	/* Remote frame request pending register 1 */
#define HCAN2_RFPR0		  0x0000004a	/* Remote frame request pending register 0 */
#define HCAN2_MBIMR1	  0x00000050	/* Mailbox interrupt mask register 1 */
#define HCAN2_MBIMR0	  0x00000052	/* Mailbox interrupt mask register 0 */
#define HCAN2_UMSR1		  0x00000058	/* Unread message status register 1 */
#define HCAN2_UMSR0		  0x0000005a	/* Unread message status register 0 */
#define HCAN2_TCNTR		  0x00000080	/* Timer counter register */
#define HCAN2_TCR		  0x00000082	/* Timer control register */
#define HCAN2_TCMR		  0x00000084	/* Timer Compare Match register */
#define HCAN2_TDCR		  0x00000086
#define HCAN2_LOSR		  0x00000088
#define HCAN2_ICR1		  0x0000008e
#define HCAN2_TCMR0		  0x00000090	/* Timer compare match register */
#define HCAN2_TCMR1		  0x00000092
#define HCAN2_TCMR2		  0x00000094
#define HCAN2_CCR		  0x00000096
#define HCAN2_CMAX		  0x00000098
#define HCAN2_TMR		  0x0000009a

/* BaudRate minimal and maximal TSEG values */
#define TSEG_MIN	8
#define	TSEG_MAX	25
#define TSEG1_MIN	4
#define	TSEG1_MAX	16
#define TSEG2_MIN	2
#define	TSEG2_MAX	8

enum hcan2_mcr {
	/* bits 15 to 8 are used for test mode */
	HCAN2_MCR_AWAKE	= 1 << 7,		/* Auto Wake Mode */
	HCAN2_MCR_SLEEP = 1 << 5,		/* Sleep Mode */
	HCAN2_MCR_TXP	= 1 << 2,		/* Transmition Priority 0-message ID number priority, 1-mailbox number piority*/
	HCAN2_MCR_HALT	= 1 << 1,		/* Halt Request */
	HCAN2_MCR_RESET = 1 << 0,		/* Reset Request */
};

enum hcan2_gsr {
	HCAN2_GSR_EPS	= 1 << 5,		/* Error Passive Status */
	HCAN2_GSR_HSS	= 1 << 4,		/* Halt/Sleep Status */
	HCAN2_GSR_RESET	= 1 << 3,		/* Reset Status */
	HCAN2_GSR_TXC	= 1 << 2,		/* Message Transmission Complete Flag */
	HCAN2_GSR_TXRXW	= 1 << 1,		/* Transmit/Receive Warning Flag */
	HCAN2_GSR_BOFF	= 1 << 0,		/* Buss Off Flag */
};

/* IRR and IMR register */
enum hcan2_irr {
	HCAN2_IRR_TCMI	= 1 << 14,		/* Time Compare Match Register */
	HCAN2_IRR_TOI	= 1 << 13,		/* Time Overrun Interrupt */
	HCAN2_IRR_WUBA	= 1 << 12,		/* Wake-up on Bus Activity */
	HCAN2_IRR_MOOI	= 1 << 9,		/* Message Overrun/Overwrite Interrupt Flag */
	HCAN2_IRR_MBEI	= 1 << 8,		/* Messagebox Empty Interrupt Flag */
	HCAN2_IRR_OF	= 1 << 7,		/* Overload Frame */
	HCAN2_IRR_BOI	= 1 << 6,		/* Bus Off Interrupt Flag */
	HCAN2_IRR_EPI	= 1 << 5,		/* Error Passive Interrupt Flag */
	HCAN2_IRR_ROWI	= 1 << 4,		/* Receive Overload Warning Interrupt Flag */
	HCAN2_IRR_TOWI	= 1 << 3,		/* Transmit Overload Warining Interrupt Flag */
	HCAN2_IRR_RFRI	= 1 << 2,		/* Remote Frame Request Interrupt Flag */
	HCAN2_IRR_DFRI	= 1 << 1,		/* Data Frame Received Interrupt Flag */
	HCAN2_IRR_RHSI	= 1 << 0,		/* Reset/Halt/Sleep Interrupt Flag */
};

/* Message box 0-31 */
#define HCAN2_MB0		  0x00000100	/* RECEIVE ONLY */
#define HCAN2_MB_OFFSET	  0x00000020

/* Message box structure offsets */
#define HCAN2_MB_CTRL0	0x00000000	/* Control 0 */
#define HCAN2_MB_CTRL1	0x00000002	/* Control 1 */
#define HCAN2_MB_CTRL2	0x00000004	/* Control 2 */
#define HCAN2_MB_TSTP	0x00000006	/* Time stamp */
#define HCAN2_MB_DATA0	0x00000009	/* Data 0 */ 
#define HCAN2_MB_DATA1	0x00000008	/* Data 1 */
#define HCAN2_MB_DATA2	0x0000000b	/* Data 2 */
#define HCAN2_MB_DATA3	0x0000000a	/* Data 3 */
#define HCAN2_MB_DATA4	0x0000000d	/* Data 4 */
#define HCAN2_MB_DATA5	0x0000000c	/* Data 5 */
#define HCAN2_MB_DATA6	0x0000000f	/* Data 6 */
#define HCAN2_MB_DATA7	0x0000000e	/* Data 7 */
#define HCAN2_MB_MASK	0x00000010	/* Acceptance filter mask 4 bytes */

/* Control register 0 */
enum hcan2_mb_ctrl0{
	HCAN2_MBCT0_STDID	= 0x7ff0,	/* STD ID */
	HCAN2_MBCT0_RTR	= 0x0008,	/* Remote transmition request 0-DataFrame 1-RemoteFrame */
	HCAN2_MBCT0_IDE	= 0x0004,	/* Identifier extension 0-Standard 1-Extended */
	HCAN2_MBCT0_EXTID	= 0x0003	/* EXTID 17:16 */
};

/* Control register 1 */
/* whole register is EXTD ID 15:0 */

/* Control register 2 */
enum hcan2_mb_ctrl2{
	HCAN2_MBCT2_NMC		= 1<<13,	/* New message control */
	HCAN2_MBCT2_ATX 	= 1<<12,	/* Automatic transmition of data frame */
	HCAN2_MBCT2_DART	= 1<<11,	/* Disable automatic re-transmition */
	HCAN2_MBCT2_MBC		= 7<<8,		/* Mailbox configuration */
	HCAN2_MBCT2_CBE		= 1<<5,		/* CAN bus error */
	HCAN2_MBCT2_DLC		= 0xf		/* Data length code */
};

/* MessageBox modes */
enum hcan2_mb_mode{
	HCAN2_MBMOD_TXDR		= 0,	/* Transmit Data and Remote Frame */
	HCAN2_MBMOD_TXDR_RXR	= 1,	/* Transmit Data and Remote Frame, Receive Remote Frame */
	HCAN2_MBMOD_RXDR		= 2,	/* Receive Data and Remote Frame */
	HCAN2_MBMOD_RXD			= 3,	/* Receive Data Frame */
	HCAN2_MBMOD_TXR_RXDR	= 4,	/* Transmit Remote Frame, Receive Data and Remove Frame */
	HCAN2_MBMOD_TXR_RXD		= 5,	/* Transmit Remote Frame, Receive Data Frame */
	/* 6 is not used and prohibited */
	HCAN2_MBMOD_INNACTIVE	= 7,	/* Mailboc Innactive */
};
