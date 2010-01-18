/* hcan2.c
 * Linux CAN-bus device driver.
 * This software is released under the GPL-License.
 */

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/hcan2.h"

#define myDEBUG 0

#if myDEBUG
	#define DEBUGMSG(fmt,args...) can_printk(KERN_ERR "lincan (debug): " fmt,##args)
#endif


#define MAX_TRANSMIT_WAIT_LOOPS 20
#define MAX_SETTING_WAIT_LOOPS 25		/* maximal loop count while checking Chip reply to action */
#define MAX_IRQ_WAIT_LOOPS 25

void hcan2_irq_read_handler(struct canchip_t *chip, struct msgobj_t *obj);
void hcan2_irq_write_handler(struct canchip_t *chip, struct msgobj_t *obj);

void hcan2_clear_irq_flags(struct canchip_t *chip);
void hcan2_clear_mbox(struct canchip_t *chip, int msgobj_idx);

void hcan2_setup_mbox4write(struct msgobj_t * obj, struct canmsg_t * msg);
void hcan2_setup_mbox4write_data(struct msgobj_t * obj, struct canmsg_t * msg);
void hcan2_setup_mbox4read(struct msgobj_t * obj);

void hcan2_setup_ctrl_regs(struct canmsg_t * msg, uint16_t * ctrl0, uint16_t * ctrl1, uint16_t * ctrl2);

int hcan2_compare_msg(struct msgobj_t * obj, struct canmsg_t * msg);
void hcan2_notifyRXends(struct msgobj_t * obj, int what);
/* Enable folowing IRQs
 * HCAN2_IRR_DFRI = Data Frame Received Interrupt Flag
 * HCAN2_IRR_MBEI = Mailbox Empty Interrupt Flag
 *
 * and Errors:
 * Bus Off, Error Passive, Message Overrun/Overwrite
 */
uint16_t IRQs = ~(HCAN2_IRR_DFRI + HCAN2_IRR_MBEI + HCAN2_IRR_BOI + HCAN2_IRR_EPI + HCAN2_IRR_MOOI);
/* 1 - mask interrupt, 0 - interrupt not masked */

int hcan2_chip_config(struct canchip_t *chip)
{
	DEBUGMSG("Configuring chip...\n");
	
	if (hcan2_enable_configuration(chip))
		return -ENODEV;

	if (!chip->baudrate)
		chip->baudrate=1000000;
	if (hcan2_baud_rate(chip, chip->baudrate,chip->clock,0,75,0))
		return -ENODEV;

	hcan2_config_irqs(chip, IRQs);

	if (hcan2_disable_configuration(chip))
		return -ENODEV;

/*	DEBUGMSG("Chip configured\n"); */
	return 0;
}

int hcan2_enable_configuration(struct canchip_t *chip)
{
	int i = 0;
	uint16_t gsr;

	DEBUGMSG("Enabling configuration...\n");

	/* Disable interrupt */
	can_disable_irq(chip->chip_irq);

	/* Halt mode - disable CAN activity after completing current operation */
	gsr = can_read_reg_w(chip, HCAN2_GSR);
	if (gsr & (HCAN2_GSR_BOFF | HCAN2_GSR_RESET))	/* chip is already in config mode */
		return 0;

	can_write_reg_w(chip, HCAN2_MCR_HALT, HCAN2_MCR);

	/* Waits until chip enters Halt mode - finishes current  TX/RX operation */
	gsr = can_read_reg_w(chip, HCAN2_GSR);
	while ( !(gsr & HCAN2_GSR_HSS) && ((i++) <= MAX_SETTING_WAIT_LOOPS) ) {
		udelay(200);
		gsr = can_read_reg_w(chip, HCAN2_GSR);
	}

	if (i >= MAX_SETTING_WAIT_LOOPS) {
		CANMSG("Error entering HALT mode (enable configuration) \n");
		can_enable_irq(chip->chip_irq);
		return -ENODEV;
	}

/*	DEBUGMSG("Configuration mode is ENABLED\n"); */
	return 0;
}

int hcan2_disable_configuration(struct canchip_t *chip)
{
	int i = 0;
	uint16_t gsr, mcr;

	DEBUGMSG("Disabling configuration mode...\n");

	/* Halt mode - disable CAN activity after completing current operation */
	mcr = can_read_reg_w(chip, HCAN2_MCR);
	gsr = can_read_reg_w(chip, HCAN2_GSR);

	/* if configuration already disabled */
	if (!(gsr & HCAN2_GSR_HSS) && !(mcr & HCAN2_MCR_HALT))
	    return 0;

	can_write_reg_w(chip, mcr & ~HCAN2_MCR_HALT, HCAN2_MCR);

	/* Waits until chip leaves Halt mode */
	gsr = can_read_reg_w(chip, HCAN2_GSR);
	while ( (gsr & HCAN2_GSR_BOFF) && ((i++) <= MAX_SETTING_WAIT_LOOPS) ) {
		udelay(200);
		gsr = can_read_reg_w(chip, HCAN2_GSR);
	}

	if (i >= MAX_SETTING_WAIT_LOOPS) {
		CANMSG("Error leaving HALT mode (enable configuration) \n");
		return -ENODEV;
	}

	/* Enable interrupt */
	can_enable_irq(chip->chip_irq);

/*	DEBUGMSG("Configuration mode is DISABLED\n"); */
	return 0;
}

/* ********************************************* */
int hcan2_baud_rate(struct canchip_t *chip, int rate, int clock, int sjw, int sampl_pt, int flags)
{
	/* Set communication parameters.
	 * param rate baud rate in Hz
	 * param clock frequency of hcan2 clock in Hz (on sh7760 most probably 27.5MHz)
	 * param sjw synchronization jump width (0-3) prescaled clock cycles
	 * param sampl_pt sample point in % (0-100) sets (TSEG1 + 1)/(TSEG1 + TSEG2 + 1) ration
	 * param flags fields BTR1_SAM, OCMODE, OCPOL, OCTP, OCTN, CLK_OFF, CBP
	 */


	/* rate = clock / ((tseg1 + tseg2 + 1) * brp ) */

	int best_error = 1000000000, error;
	int best_tseg = 0, best_brp = 0, best_rate = 0, brp = 0;
	int tseg, tseg1 = 0, tseg2 = 0;		/* tseg = TSEG1 + TSEG2 + 1*/
	uint16_t bcr0 = 0, bcr1 = 0;

	DEBUGMSG("Seting Baud rate...\n");

	for (tseg = TSEG_MIN; tseg <= TSEG_MAX; tseg++)
	{
		brp = 10 * clock/(tseg * rate);
		brp = brp % 10 > 4 ? brp / 10 + 1: brp / 10;	/* round */

		if (brp == 0 || brp > 256)
			continue;

		error = rate - clock/(brp * tseg);
		if (error < 0)
			error = -error;
		if (error <= best_error) {
			best_error = error;
			best_tseg = tseg;
			best_brp = brp;
			best_rate = clock/(brp * tseg);
		}
	}

	tseg2 = best_tseg - (sampl_pt * best_tseg)/100;
	if (tseg2 < TSEG2_MIN)		/* tseg2 <= sjw +1 , TSEG2_MIN = 4 */
		tseg2 = TSEG2_MIN;
	if (tseg2 > TSEG2_MAX)
		tseg2 = TSEG2_MAX;
	tseg1 = best_tseg - tseg2 - 1;
	if (tseg1 > TSEG1_MAX) {
		tseg1 = TSEG1_MAX;
		tseg2 = best_tseg - tseg1 - 1;
	}

	if (best_error && (rate/best_error < 10)) {
		CANMSG("baud rate %d is not possible with %d Hz clock\n",
			rate, clock);
		CANMSG("%d bps. brp=%d, best_tseg=%d, tseg1=%d, tseg2=%d\n",
			best_rate, best_brp, best_tseg, tseg1, tseg2);
		return -EINVAL;
	}

	DEBUGMSG("Setting %d bps.\n", best_rate);
/*	DEBUGMSG("brp=%d, best_tseg=%d, tseg1=%d, tseg2=%d, sampl_pt=%d\n",
		best_brp, best_tseg, tseg1, tseg2,
		(100 * tseg1 / best_tseg));
*/
	/*
	 * EG	= 0b0	- Resynchronization at falling edge
	 * BSP	= 0b00	- bit sampling at one point (end of TSEG1)
	 */

	bcr1 = (((tseg1 - 1) & 0x000f) << 12) + (((tseg2 - 1) & 0x0007) << 8) + ((sjw & 0x0003) << 4);
	bcr0 = (best_brp - 1) & 0x00ff;

	hcan2_set_btregs(chip, bcr0, bcr1);	

	hcan2_disable_configuration(chip);

/*	DEBUGMSG("Baud rate set successfully\n"); */
	return 0;
}

int hcan2_set_btregs(struct canchip_t *chip, unsigned short bcr0, unsigned short bcr1)
{
/*	DEBUGMSG("Seting BCR0 and BCR1.\n"); */

	/* masks words to correct format */
	bcr0 &= 0x00ff;
	bcr1 &= 0xf733;

	can_write_reg_w(chip, bcr1, HCAN2_BCR1);
	can_write_reg_w(chip, bcr0, HCAN2_BCR0);
	
/*	DEBUGMSG("BCR0 and BCR1 successfully set.\n"); */
	return 0;
}

/* ********************************************* */
int hcan2_start_chip(struct canchip_t *chip)
{
/*	DEBUGMSG("Starting chip %d...\n", chip->chip_idx); */

	/* Stop chip turns chip to HALT mode - traffic on CAN bus is ignored after completing curent operation.
	 * Start chip only turn chip back from HALT state - using disable_config
	 */
	hcan2_disable_configuration(chip);

	DEBUGMSG("Chip [%d] started\n", chip->chip_idx);
	return 0;
}

int hcan2_stop_chip(struct canchip_t *chip)
{
/*	DEBUGMSG("Stopping chip %d...\n", chip->chip_idx); */

	/* Stop chip turns chip to HALT mode - traffic on CAN bus is ignored after completing curent operation.
	 * - using enable_config
	 */
	hcan2_enable_configuration(chip);

	DEBUGMSG("Chip [%d] stopped\n", chip->chip_idx);
	return 0;
}

int hcan2_attach_to_chip(struct canchip_t *chip)
{
/*	DEBUGMSG("Attaching to chip %d.\n", chip->chip_idx); */
	
	/* initialize chip */	

	if (hcan2_enable_configuration(chip))
		return -ENODEV;

	/* Clear all Mailboxes */
	if (hcan2_clear_objects(chip))
		return -ENODEV;

	/* set Baudrate and Interrupts */
	if (hcan2_chip_config(chip))
		return -ENODEV;

	if (hcan2_disable_configuration(chip))
		return -ENODEV;

	/* Enable interrupt */
	can_enable_irq(chip->chip_irq);
	can_enable_irq(chip->chip_irq);

	CANMSG("Successfully attached to chip [%02d].\n", chip->chip_idx);
	return 0;
}

int hcan2_release_chip(struct canchip_t *chip)
{
	hcan2_stop_chip(chip);
	can_disable_irq(chip->chip_irq);
	
	hcan2_clear_objects(chip);
	
	DEBUGMSG("Chip released [%02d]\n", chip->chip_idx);
	return 0;
}

/* ********************************************* */
int hcan2_standard_mask(struct canchip_t *chip, unsigned short code, unsigned short mask)
{
	uint16_t ctrl0, lafm0;
	struct msgobj_t * obj;
	int obj_idx = (int) (chip->chip_data);

	if (code & 0x1ffff800)	
	    return hcan2_extended_mask(chip, code, mask);

	
	if (obj_idx > 0 && obj_idx <= 32)
	    obj = chip->msgobj[obj_idx - 1];
	else
	    return -ENODEV;

	chip->chip_data = (void*)0;	/* reset mbox number */


	ctrl0 = ((code & 0x07ff) << 4);
	lafm0 = ((mask & 0x07ff) << 4);
	lafm0 |= 0x0003;		/* ignore Ext ID 17:16  */
	
	can_write_reg_w(chip, ctrl0, (int) obj->obj_base_addr + HCAN2_MB_CTRL0);
	can_write_reg_w(chip, 0x0000, (int) obj->obj_base_addr + HCAN2_MB_CTRL1);
	can_write_reg_w(chip, lafm0, (int) obj->obj_base_addr + HCAN2_MB_MASK);
	can_write_reg_w(chip, 0xffff, (int) obj->obj_base_addr + HCAN2_MB_MASK + 2);

	DEBUGMSG("MB%02d: Set standard_mask [id:0x%04x, m:0x%04x]\n", obj_idx, code, lafm0);
	return 0;
}

int hcan2_extended_mask(struct canchip_t *chip, unsigned long code, unsigned long mask)
{
	uint16_t ctrl0, ctrl1, lafm0, lafm1;

	struct msgobj_t * obj;

	int obj_idx = (int) (chip->chip_data);
	
	if (obj_idx > 0 && obj_idx <= 32)
	    obj = chip->msgobj[obj_idx - 1];
	else
	    return -ENODEV;

	chip->chip_data = (void*)0; /* reset mbox number */

	ctrl0 = ((code & 0x1ffc0000) >> 14);
	ctrl0 |=((code & 0x00030000) >> 16);
	ctrl0 |= HCAN2_MBCT0_IDE;	/* set IDE flag */
	ctrl1 =  (code & 0x0000ffff);

	lafm0 = ((mask & 0x1ffc0000) >> 14);
	lafm0 |=((mask & 0x00030000) >> 16);
	lafm1 =  (mask & 0x0000ffff);
	
	can_write_reg_w(chip, ctrl0, (int) obj->obj_base_addr + HCAN2_MB_CTRL0);
	can_write_reg_w(chip, ctrl1, (int) obj->obj_base_addr + HCAN2_MB_CTRL1);
	can_write_reg_w(chip, lafm0, (int) obj->obj_base_addr + HCAN2_MB_MASK);
	can_write_reg_w(chip, lafm1, (int) obj->obj_base_addr + HCAN2_MB_MASK + 2);
	
	DEBUGMSG("MB%02d: Set extended_mask [id:0x%08x, m:0x%08x]\n", obj_idx, (uint32_t)code, (uint32_t)mask);
    
        return 0;
}

/* ********************************************* */
int hcan2_pre_read_config(struct canchip_t *chip, struct msgobj_t *obj)
{
	DEBUGMSG("Pre read config\n");

	hcan2_enable_configuration(chip);

	/* clears mailbox and setup LFA to accept all Exted Messages */
	hcan2_setup_mbox4read(obj);
	
	hcan2_disable_configuration(chip);

	return 0;
}

int hcan2_pre_write_config(struct canchip_t *chip, struct msgobj_t *obj, struct canmsg_t *msg)
{
	DEBUGMSG("Pre write config\n");

	/* change Mailbox header only if neccessary */	
	/* otherwise change only data */
	if (hcan2_compare_msg(obj, msg))
	{
		if (hcan2_enable_configuration(chip))
			return -ENODEV;
	
		hcan2_setup_mbox4write(obj, msg);
		
		if (hcan2_disable_configuration(chip))
			return -ENODEV;
	}
	else
		hcan2_setup_mbox4write_data(obj, msg);

	return 0;
}

int hcan2_send_msg(struct canchip_t *chip, struct msgobj_t *obj, struct canmsg_t *msg)
{
	unsigned obj_bit;
	int b_addr = ((obj->object - 1) / 16) * (-2); 

	obj_bit = (1 << ((obj->object - 1) % 16));
	
/*	CANMSG("Sending message [obj: %d]\n", obj->object - 1); */

	can_write_reg_w(chip, obj_bit, b_addr + HCAN2_TXPR0);

/*	CANMSG("Message sent [obj: %d]\n", obj->object - 1); */
	return 0;
}

int hcan2_remote_request(struct canchip_t *chip, struct msgobj_t *obj)
{
	CANMSG("hcan2_remote_request not implemented\n");
	return -ENOSYS;
}

/* ********************************************* */
int hcan2_irq_handler(int irq, struct canchip_t *chip)
{
	uint16_t irq_reg, idx;
	short loop_cnt = MAX_IRQ_WAIT_LOOPS;
	uint32_t rxdf, txdf;

/*
	HCAN2_IRR_TCMI	- Time Compare Match Register
	HCAN2_IRR_TOI	- Time Overrun Interrupt
	HCAN2_IRR_WUBA	- Wake-up on Bus Activity
	HCAN2_IRR_MOOI	- Message Overrun/Overwrite Interrupt Flag
	HCAN2_IRR_MBEI	- Messagebox Empty Interrupt Flag
	HCAN2_IRR_OF	- Overload Frame
	HCAN2_IRR_BOI	- Bus Off Interrupt Flag
	HCAN2_IRR_EPI	- Error Passive Interrupt Flag
	HCAN2_IRR_ROWI	- Receive Overload Warning Interrupt Flag
	HCAN2_IRR_TOWI	- Transmit Overload Warining Interrupt Flag
	HCAN2_IRR_RFRI	- Remote Frame Request Interrupt Flag
	HCAN2_IRR_DFRI	- Data Frame Received Interrupt Flag
	HCAN2_IRR_RHSI	- Reset/Halt/Sleep Interrupt Flag */

	irq_reg = can_read_reg_w(chip, HCAN2_IRR);
	DEBUGMSG("irq: %d, chip base addr: 0x%08x\n", irq, (uint32_t)chip->chip_base_addr);
	DEBUGMSG("IRQ Handler: HCAN2_IRR: 0x%04x\n", irq_reg); //*/

	do {

		if(!loop_cnt--) {
			CANMSG("hcan2_irq_handler IRQ %d stuck\n", irq);
			return CANCHIP_IRQ_STUCK;
		}

		/* Received message */
		if (irq_reg & HCAN2_IRR_DFRI)
		{
			rxdf = (can_read_reg_w(chip, HCAN2_RXPR1) << 16) +
				can_read_reg_w(chip, HCAN2_RXPR0);

			while(rxdf) {
				DEBUGMSG("Received message [0x%08x]\n", rxdf);

				/* find the message object */
				for (idx = 0; (idx < chip->max_objects) && rxdf; idx++)
					if ((rxdf & (1<<idx))) {
						hcan2_irq_read_handler(chip, chip->msgobj[idx]);
						/* RXPR flag for this msgobj is cleared during irq_read_handler*/
						rxdf &= ~(1 << idx);
					}


				DEBUGMSG("Before reset flags [0x%08x]\n", rxdf);
				rxdf = (can_read_reg_w(chip, HCAN2_RXPR1) << 16) +
					can_read_reg_w(chip, HCAN2_RXPR0);
			}
		}

		/* Error: Bus Off */
		if (irq_reg & HCAN2_IRR_BOI) {
			CANMSG("Error: entering BUS OFF state\nstatus register: 0x%02x irq register: 0x%02x\n",
				can_read_reg_w(chip, HCAN2_GSR), irq_reg);

			/* notify all RX/TX ends */
			for (idx = 0; idx < chip->max_objects; idx++) {
				/* notify TX */
				chip->msgobj[idx]->ret=-1;
				if(chip->msgobj[idx]->tx_slot)
					canque_notify_inends(chip->msgobj[idx]->tx_qedge,
						CANQUEUE_NOTIFY_ERROR);
				/* notify RX */
				hcan2_notifyRXends(chip->msgobj[idx], CANQUEUE_NOTIFY_ERROR);
			}

			/* reset flag - by writing '1' */
			can_write_reg_w(chip, HCAN2_IRR_BOI, HCAN2_IRR);
		}

		/* Warning: Error Passive */
		if (irq_reg & HCAN2_IRR_EPI) {
			uint16_t tecrec;
			tecrec = can_read_reg_w(chip, HCAN2_TECREC);

			CANMSG("Warning: entering ERROR PASSIVE state\nTEC: %d REC: %d\n",
				(uint16_t)((tecrec >> 8) & 0x00ff), (uint16_t)(tecrec & 0x00ff));
			
			/* Show warning only */

			/* reset flag - by writing '1' */
			can_write_reg_w(chip, HCAN2_IRR_EPI, HCAN2_IRR);
		}

		/* Message Overrun/Overwritten */
		if (irq_reg & HCAN2_IRR_MOOI) { 
			/* put get Unread Message Status Register */
			rxdf =	(can_read_reg_w(chip, HCAN2_UMSR1) << 16) + can_read_reg_w(chip, HCAN2_UMSR0);

			/* find the message object */
			for (idx = 0; (idx < chip->max_objects) && !(rxdf & (1<<idx)); idx++) { }
			
			CANMSG("Error: MESSAGE OVERRUN/OVERWRITTEN [MB: %d]\n",idx);
			
			/* notify only injured RXqueue-end */
			if (idx < chip->max_objects) 
				hcan2_notifyRXends(chip->msgobj[idx], CANQUEUE_NOTIFY_ERROR);

			/* reset flag */
			can_write_reg_w(chip, (1 << (idx % 16)), HCAN2_UMSR0 - 2 * (idx / 16));
		}

		/* Mailbox empty - after message was sent */
		if (irq_reg & HCAN2_IRR_MBEI)
		{
		    txdf = (can_read_reg_w(chip, HCAN2_TXACK1) << 16) +
			can_read_reg_w(chip, HCAN2_TXACK0);

		    /* find the message object */
		    for (idx = 0; (idx < chip->max_objects) && !(txdf & (1<<idx)); idx++) { }

		    /* realy i got one? */
		    if (idx >= chip->max_objects) {
			/* IRQ is caused by Aborted transmition */
			can_write_reg_w(chip, 0xffff, HCAN2_ABACK0);
			can_write_reg_w(chip, 0xffff, HCAN2_ABACK1);
			return CANCHIP_IRQ_HANDLED;
		    } 

		    /* Clear TXACK flag */		    
		    can_write_reg_w(chip, 1 << (idx % 16), HCAN2_TXACK0 - 2 * (idx / 16));

		    /* sends message */	    
		    hcan2_wakeup_tx(chip, chip->msgobj[idx]);
		}

		irq_reg = can_read_reg_w(chip, HCAN2_IRR);
	} while(irq_reg & ~IRQs);

	return CANCHIP_IRQ_HANDLED;
}

int hcan2_irq_accept(int irq, struct canchip_t *chip)
{
	CANMSG("hcan2_irq_accept NOT IMPLEMENTED\n");
	return -ENOSYS;
}

int hcan2_config_irqs(struct canchip_t *chip, short irqs)
{
	hcan2_clear_irq_flags(chip);

	can_write_reg_w(chip, irqs, HCAN2_IMR);
	
	/* allow all mailboxes to generate IRQ */
	can_write_reg_w(chip, 0, HCAN2_MBIMR0);
	can_write_reg_w(chip, 0, HCAN2_MBIMR1);
	
/*	CANMSG("IRQ Mask set [0x%02x]\n", irqs); */
	return 0;
}


/* ********************************************* */
int hcan2_clear_objects(struct canchip_t *chip)
{
	int i;
	for (i = 0; i < chip->max_objects; i++)
		hcan2_clear_mbox(chip, i);

	return 0;
}

int hcan2_check_tx_stat(struct canchip_t *chip)
{
/*	DEBUGMSG("Check TX stat\n"); */
	/* If Transmition is complete return 0 - no error */
	if (can_read_reg_w(chip, HCAN2_GSR) & HCAN2_GSR_TXC)
		return 0;
	else
		return 1;
}

/* Note: this checks TX status of concrete messagebox */
int hcan2_check_MB_tx_stat(struct canchip_t *chip, struct msgobj_t *obj)
{
	/* Transmition is complete return 0 - no error */
	
	/* MB1-MB15 are in CANTXPR0 and MB16-MB31 are in CANTXPR1
	   CANTXPR0 = CANTXPR1 + 0x0002
	   MB0 - receive only */

	char id = obj->object - 1;
	return (can_read_reg_w(chip, HCAN2_TXPR0 - 2 * (id / 16)) & (1 << (id & 0x00ff)));
}

int hcan2_wakeup_tx(struct canchip_t *chip, struct msgobj_t *obj)
{
	DEBUGMSG("WakeUP TX\n");

	if (obj->object == 1)	/* msgbox 0 cant transmit only receive ! */
	    return -ENODEV;
    
	can_preempt_disable();
	
	can_msgobj_set_fl(obj,TX_REQUEST);
	if(!can_msgobj_test_and_set_fl(obj,TX_LOCK) &&
		!hcan2_check_MB_tx_stat(chip, obj))
	{	/* enable transmition only if MB is empty */
		can_msgobj_clear_fl(obj,TX_REQUEST);

		hcan2_irq_write_handler(chip, obj);
	
		can_msgobj_clear_fl(obj,TX_LOCK);
	}
	else
		can_msgobj_clear_fl(obj,TX_REQUEST);


	can_preempt_enable();

/*	DEBUGMSG("WakeUP TX - END\n"); */
	return 0;
}

int hcan2_filtch_rq(struct canchip_t *chip, struct msgobj_t * obj)
{
	struct canfilt_t filter;


#if myDEBUG
	int num = canqueue_ends_filt_conjuction(obj->qends, &filter);
#else
	canqueue_ends_filt_conjuction(obj->qends, &filter);
#endif

	/* in structure chip->chip_data is Mailbox number */
	chip->chip_data = (void*)(obj->object);

	/* HCAN2 uses oposite logic for LAFM: 1-ignore bit, 0-use bit as mask */

	DEBUGMSG("CNT: %d ID: 0x%08x MASK: 0x%08x\n", num, (uint32_t) (filter.id) & 0x1fffffff, (uint32_t) (~filter.mask) & 0x1fffffff);

	if (filter.flags & MSG_EXT)		/* Extended ID */
		return hcan2_extended_mask(chip, filter.id, ~filter.mask);
	else					/* Standard ID */
		return hcan2_standard_mask(chip, filter.id, ~filter.mask);
}

/* ********************************************* */
void hcan2_irq_read_handler(struct canchip_t *chip, struct msgobj_t *obj)
{
	int i, len;
	unsigned ctrl0, ctrl2, data;
	unsigned long flag_addr;
	uint16_t mb_offset;
	

	mb_offset = (int ) obj->obj_base_addr;

/*	DEBUGMSG("------IRQ Read Handler\n"); */

	ctrl0 = can_read_reg_w(chip, mb_offset + HCAN2_MB_CTRL0);
	ctrl2 = can_read_reg_w(chip, mb_offset + HCAN2_MB_CTRL2);

	obj->rx_msg.length = len = ctrl2 & HCAN2_MBCT2_DLC;
	obj->rx_msg.flags = (ctrl0 & HCAN2_MBCT0_RTR) ? MSG_RTR : 0;
	obj->rx_msg.cob = obj->object - 1;
		
	/* get ID of received message */
	if (ctrl0 & HCAN2_MBCT0_IDE)
	{
	    DEBUGMSG("EXTENDED ID\n");
	    obj->rx_msg.id = (ctrl0 & HCAN2_MBCT0_STDID) << (18 - 4);
	    obj->rx_msg.id |= can_read_reg_w(chip, mb_offset + HCAN2_MB_CTRL1);
    	    obj->rx_msg.id |= ((ctrl0 & HCAN2_MBCT0_EXTID) << 16);
	}
	else
	    obj->rx_msg.id = (ctrl0 & HCAN2_MBCT0_STDID)>>4;


	if(len > CAN_MSG_LENGTH) len = CAN_MSG_LENGTH;
	for (i = 0; i < len; i++)
	{
		/* rcanqueue_ends_filt_conjuctionead 16bit data - two data bytes*/
		data = can_read_reg_w(chip, (int) obj->obj_base_addr + HCAN2_MB_DATA1 + i);
		obj->rx_msg.data[i] = (data & 0xff00) >> 8;		// one data byte
		if (++i < len) obj->rx_msg.data[i] = data & 0x00ff;	// second data byte
	}

	/* Computes correct offset address of register from MSGBOX_IDX and RTR flag
	 * result is one of these:
	 * HCAN2_RXPR1, HCAN2_RXPR0, HCAN2_RFPR1, HCAN2_RFPR0
	 */
	flag_addr = HCAN2_RXPR0 - (int)((obj->object - 1) / 16) * 2;
		
	/* Reset flag by writing 1 to its position */
	can_write_reg_w(chip, (1 << ((obj->object - 1) % 16)), flag_addr);

	/* fill CAN message timestamp */
	can_filltimestamp(&obj->rx_msg.timestamp);

	canque_filter_msg2edges(obj->qends, &obj->rx_msg);
}

void hcan2_irq_write_handler(struct canchip_t *chip, struct msgobj_t *obj)
{
	int cmd;

	if(obj->tx_slot){
		/* Do local transmitted message distribution if enabled */
		if (processlocal){
			/* fill CAN message timestamp */
			can_filltimestamp(&obj->tx_slot->msg.timestamp);

			obj->tx_slot->msg.flags |= MSG_LOCAL;
			canque_filter_msg2edges(obj->qends, &obj->tx_slot->msg);
		}
		/* Free transmitted slot */
		canque_free_outslot(obj->qends, obj->tx_qedge, obj->tx_slot);
		obj->tx_slot=NULL;
	}

	cmd = canque_test_outslot(obj->qends, &obj->tx_qedge, &obj->tx_slot);
	if(cmd < 0)
		return;

	if (chip->chipspecops->pre_write_config(chip, obj, &obj->tx_slot->msg)) {
		obj->ret = -1;
		canque_notify_inends(obj->tx_qedge, CANQUEUE_NOTIFY_ERRTX_PREP);
		canque_free_outslot(obj->qends, obj->tx_qedge, obj->tx_slot);
		obj->tx_slot = NULL;
		return;
	}
	if (chip->chipspecops->send_msg(chip, obj, &obj->tx_slot->msg)) {
		obj->ret = -1;
		canque_notify_inends(obj->tx_qedge, CANQUEUE_NOTIFY_ERRTX_SEND);
		canque_free_outslot(obj->qends, obj->tx_qedge, obj->tx_slot);
		obj->tx_slot = NULL;
		return;
	}
}

/* ********************************************* */
int hcan2_register(struct chipspecops_t *chipspecops)
{
	chipspecops->chip_config = hcan2_chip_config;
	chipspecops->enable_configuration = hcan2_enable_configuration;
	chipspecops->disable_configuration = hcan2_disable_configuration;

	chipspecops->baud_rate = hcan2_baud_rate;
	chipspecops->set_btregs = hcan2_set_btregs;

	chipspecops->start_chip = hcan2_start_chip;
	chipspecops->stop_chip = hcan2_stop_chip;
	chipspecops->attach_to_chip = hcan2_attach_to_chip;
	chipspecops->release_chip = hcan2_release_chip;

	chipspecops->standard_mask = hcan2_standard_mask;
	chipspecops->extended_mask = hcan2_extended_mask;
	chipspecops->message15_mask = NULL; /* hcan2_message15_mask; */

	chipspecops->pre_read_config = hcan2_pre_read_config;
	chipspecops->pre_write_config = hcan2_pre_write_config;
	chipspecops->send_msg = hcan2_send_msg;
	chipspecops->remote_request = hcan2_remote_request;

	chipspecops->irq_handler = hcan2_irq_handler;
	chipspecops->irq_accept = NULL; /* hcan2_irq_accept; */
	chipspecops->config_irqs = hcan2_config_irqs;

	chipspecops->clear_objects = hcan2_clear_objects;
	chipspecops->check_tx_stat = hcan2_check_tx_stat;
	chipspecops->wakeup_tx = hcan2_wakeup_tx;
	chipspecops->filtch_rq = hcan2_filtch_rq;
	return 0;
}

int hcan2_fill_chipspecops(struct canchip_t *chip)
{
	chip->chip_type = "hcan2";
	chip->max_objects = 32;
	chip->write_register = chip->hostdevice->hwspecops->write_register;
	chip->read_register = chip->hostdevice->hwspecops->read_register;
	
	/*
	chip->flags;
	chip->baudrate;
	chip->msgobj;
	chip->chip_data;
	chip->chip_lock;

	chip->sja_cdr_reg;
	chip->sja_ocr_reg;
	chip->int_cpu_reg;
	chip->int_clk_reg;
	chip->int_bus_reg;

	#ifdef CAN_WITH_RTL
	chip->worker_thread;
	chip->pend_flags;
	#endif
	*/

	hcan2_register(chip->chipspecops);
	return 0;
}


/* ********************************************* */
int hcan2_reset_chip(struct canchip_t *chip)
{
	/* After reset and reconfig (start_chip) Chip waits for
	 * 11 recessive bits to join CAN bus activity
	 */

	int i; 
	unsigned gsr_reset;

	DEBUGMSG("Resetting HCAN2 chip %d...\n", chip->chip_idx);

	/* send Reset Request */
	can_write_reg_w(chip, HCAN2_MCR_RESET, HCAN2_MCR );

	/* Check hardware reset status */ 
	i = 0;
	gsr_reset = can_read_reg_w(chip, HCAN2_GSR) & HCAN2_GSR_RESET;
	while (!(gsr_reset) && ((i++) <= MAX_SETTING_WAIT_LOOPS))
	{
		udelay(10000);
		gsr_reset = can_read_reg_w(chip, HCAN2_GSR) & HCAN2_GSR_RESET;
	}

	if (i >= MAX_SETTING_WAIT_LOOPS) {
		CANMSG("Reset status timeout! (enter Reset Mode)\n");
		return -ENODEV;
	}

	/* Clear Reset request flag */
	can_write_reg_w(chip, can_read_reg_w(chip, HCAN2_MCR) & (~HCAN2_MCR_RESET), HCAN2_MCR);


	/* Clear Reset Interrupt Flag IRR 0 */
	can_write_reg_w(chip, HCAN2_IRR_RHSI, HCAN2_IRR);

/* 	DEBUGMSG("Chips reset status ok.\n"); */

	return 0;
}

/* !!! Functions below doesn't call enable/disable chip config !!! */
/* !!! Usable only in block, where enable/diable config is called explicitly !!! */
void hcan2_clear_irq_flags(struct canchip_t *chip)
{
	uint16_t irr;
	DEBUGMSG("Clearing IRQ flags...\n");
	DEBUGMSG("IRR: %04x\n",can_read_reg_w(chip, HCAN2_IRR));

	irr = HCAN2_IRR_TCMI | HCAN2_IRR_TOI | HCAN2_IRR_WUBA |
		HCAN2_IRR_OF | HCAN2_IRR_BOI | HCAN2_IRR_EPI |
		HCAN2_IRR_ROWI | HCAN2_IRR_TOWI | HCAN2_IRR_RHSI;
	can_write_reg_w(chip, irr, HCAN2_IRR);

	/* Other IRQ flags are cleared through other registers - see below */

	/* Meseage Overrun/Overwrite interrupt */
	can_write_reg_w(chip, 0, HCAN2_UMSR0);
	can_write_reg_w(chip, 0, HCAN2_UMSR1);

	/* Mailbox Empty Interrupt */
	can_write_reg_w(chip, 0, HCAN2_TXACK0);
	can_write_reg_w(chip, 0, HCAN2_TXACK1);
	can_write_reg_w(chip, 0, HCAN2_ABACK0);
	can_write_reg_w(chip, 0, HCAN2_ABACK1);

	/* Remote Frame Request Interrupt */
	can_write_reg_w(chip, 0, HCAN2_RFPR0);
	can_write_reg_w(chip, 0, HCAN2_RFPR1);

	/* Data Frame Received Interupt Flag */
	can_write_reg_w(chip, 0, HCAN2_RXPR0);
	can_write_reg_w(chip, 0, HCAN2_RXPR1);

	DEBUGMSG("clear_irq_flags - OK\n");
}

void hcan2_clear_mbox(struct canchip_t *chip, int msgobj_idx)
{
	unsigned long mb_start_addr = HCAN2_MB0 + msgobj_idx * HCAN2_MB_OFFSET;

/*	DEBUGMSG("Clearing message object %d\n", msgobj_idx); */

	/* STDID = 0
	 * Standard Identifier format (0)
	 * Data Frame (0)
	 * EXTID{17,16} = 0
	 */
	can_write_reg_w(chip, 0, mb_start_addr + HCAN2_MB_CTRL0);

	/* EXTID {15:0} = 0 */
	can_write_reg_w(chip, 0, mb_start_addr + HCAN2_MB_CTRL1);

	/* NMC: overwrite stored message (1)
	 * ATDF: No message is transmited after receiving Remote Frame (0)
	 * DARTX: disable Automatic Retransmition: yes (1)
	 * MBC = 111 - not used: HCAN2_MBCT2_MBC default value correspond to 'Not Used'
	 * CAN Bus error - 0
	 * Data Length = 0
	 */
	can_write_reg_w(chip, (uint16_t) (HCAN2_MBCT2_NMC | HCAN2_MBCT2_DART | HCAN2_MBCT2_MBC), mb_start_addr + HCAN2_MB_CTRL2);

	/* TimeStamp */
	can_write_reg_w(chip, 0, mb_start_addr + HCAN2_MB_TSTP);

	/* Data: all bytes 0xff */
	can_write_reg_w(chip, (uint16_t) 0xffff, mb_start_addr + HCAN2_MB_DATA0);
	can_write_reg_w(chip, (uint16_t) 0xffff, mb_start_addr + HCAN2_MB_DATA2);
	can_write_reg_w(chip, (uint16_t) 0xffff, mb_start_addr + HCAN2_MB_DATA4);
	can_write_reg_w(chip, (uint16_t) 0xffff, mb_start_addr + HCAN2_MB_DATA6);

	/* Local Acceptance Filter Mask - all bits of STDID and EXTID must match values set in mailbox */
	can_write_reg_w(chip, 0, mb_start_addr + HCAN2_MB_MASK);
	can_write_reg_w(chip, 0, mb_start_addr + HCAN2_MB_MASK + 2);		/* Mask is 4 bytes */


	DEBUGMSG("Mailbox [%d] cleared.\n", msgobj_idx);
}

void hcan2_setup_mbox4write(struct msgobj_t * obj, struct canmsg_t * msg)
{
	int mb_offset;
	uint16_t ctrl0, ctrl1, ctrl2; 

	struct canchip_t * chip = obj->hostchip;

	DEBUGMSG("Change Header\n");

	mb_offset = (int) obj->obj_base_addr;
	
	hcan2_setup_ctrl_regs(msg, &ctrl0, &ctrl1, &ctrl2);

	can_write_reg_w(chip, ctrl0, mb_offset + HCAN2_MB_CTRL0);
	can_write_reg_w(chip, ctrl1, mb_offset + HCAN2_MB_CTRL1);	/* set 0 if not using EXT format */
	can_write_reg_w(chip, ctrl2, mb_offset + HCAN2_MB_CTRL2);

	/* data */
	hcan2_setup_mbox4write_data(obj, msg);
}

void hcan2_setup_mbox4write_data(struct msgobj_t * obj, struct canmsg_t * msg)
{
	int len,i, mb_offset;
	uint16_t data; 
	
	struct canchip_t * chip = obj->hostchip;

	DEBUGMSG("Change Data\n");

	mb_offset = (int) obj->obj_base_addr;
	
	len = msg->length;
	if(len > CAN_MSG_LENGTH) len = CAN_MSG_LENGTH;
	
	for (i = 0; i < len; i+=2)
	{
		data = (msg->data[i] << 8) + (i+1 < len ? msg->data[i+1] : 0);
		can_write_reg_w(chip, data, mb_offset + HCAN2_MB_DATA1 + i);
	}
}

void hcan2_setup_mbox4read(struct msgobj_t * obj)
{
	struct canchip_t * chip = obj->hostchip;
    
	hcan2_clear_mbox(chip, obj->object - 1);

	// in structure chip->chip_data is Mailbox number
	chip->chip_data = (void*)(obj->object);
	hcan2_extended_mask(chip, 2048, 0x1fffffff);	/* accept all */

	can_write_reg_w(chip, HCAN2_MBCT2_DART + (HCAN2_MBMOD_RXDR << 8),
	    (int) obj->obj_base_addr + HCAN2_MB_CTRL2);
}

void hcan2_setup_ctrl_regs(struct canmsg_t * msg, uint16_t * ctrl0, uint16_t * ctrl1, uint16_t * ctrl2)
{
	uint8_t len;
	uint32_t id = msg->id;

	len = msg->length;
	if(len > CAN_MSG_LENGTH) len = CAN_MSG_LENGTH;

	*ctrl0 = (msg->flags & MSG_RTR ? HCAN2_MBCT0_RTR : 0);

	if (msg->flags & MSG_EXT)
	{
		/* Extended ID */
		*ctrl0 |= ((id & 0x1ffc0000) << 14);
		*ctrl0 |= ((id & 0x00030000) >> 16) | HCAN2_MBCT0_IDE;	/* get bits {17:16} from EXTID */
		*ctrl1 = (id & 0x0000ffff);
	}
	else
	{
		/* Standard ID */
		*ctrl0 |= ((id & 0x01ff) << 4);
		*ctrl1 = 0;
	}

	*ctrl2 = HCAN2_MBCT2_DART + HCAN2_MBMOD_TXDR + (len & HCAN2_MBCT2_DLC);
}

int hcan2_compare_msg(struct msgobj_t * obj, struct canmsg_t * msg)
{
	/* Check if mailbox header content change is neccessary */
	/* Comparing only Control regs */
	uint16_t ctrl0, ctrl1, ctrl2;
	uint16_t mb_offset;
	uint16_t c0,c1,c2;
	
	struct canchip_t * chip = obj->hostchip;

	mb_offset = (int) obj->obj_base_addr;

	c0 = can_read_reg_w(chip, mb_offset + HCAN2_MB_CTRL0);
	c1 = can_read_reg_w(chip, mb_offset + HCAN2_MB_CTRL1);
	c2 = can_read_reg_w(chip, mb_offset + HCAN2_MB_CTRL2);

	hcan2_setup_ctrl_regs(msg, &ctrl0, &ctrl1, &ctrl2);

	/* if using EXT ID conpare also ctrl1 */
	if (msg->flags & MSG_EXT && ctrl1 ^ c1)
		return 1;
		

	DEBUGMSG("C0 0x%04x HW: 0x%04x\n", ctrl0, c0);
	DEBUGMSG("C1 0x%04x HW: 0x%04x\n", ctrl1, c1);
	DEBUGMSG("C2 0x%04x HW: 0x%04x\n", ctrl2, c2);

	return ((ctrl0 ^ c0) || (ctrl2 ^ c2));
}

void hcan2_notifyRXends(struct msgobj_t * obj, int what){
	struct canque_edge_t * edge;
	canque_for_each_inedge(obj->qends, edge){
		canque_notify_outends(edge, what);
	}
}
