/* sja1000.c
 * Linux CAN-bus device driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/sja1000.h"

void sja1000_irq_read_handler(struct canchip_t *chip, struct msgobj_t *obj);
void sja1000_irq_write_handler(struct canchip_t *chip, struct msgobj_t *obj);

int sja1000_enable_configuration(struct canchip_t *chip)
{
	int i=0;
	unsigned flags;

	can_disable_irq(chip->chip_irq);

	flags=can_read_reg(chip,SJACR);

	while ((!(flags & sjaCR_RR)) && (i<=10)) {
		can_write_reg(chip,flags|sjaCR_RR,SJACR);
		udelay(100);
		i++;
		flags=can_read_reg(chip,SJACR);
	}
	if (i>=10) {
		CANMSG("Reset error\n");
		can_enable_irq(chip->chip_irq);
		return -ENODEV;
	}

	return 0;
}

int sja1000_disable_configuration(struct canchip_t *chip)
{
	int i=0;
	unsigned flags;

	flags=can_read_reg(chip,SJACR);

	while ( (flags & sjaCR_RR) && (i<=10) ) {
		can_write_reg(chip,flags & (sjaCR_RIE|sjaCR_TIE|sjaCR_EIE|sjaCR_OIE),SJACR);
		udelay(100);
		i++;
		flags=can_read_reg(chip,SJACR);
	}
	if (i>=10) {
		CANMSG("Error leaving reset status\n");
		return -ENODEV;
	}

	can_enable_irq(chip->chip_irq);

	return 0;
}

int sja1000_chip_config(struct canchip_t *chip)
{
	if (sja1000_enable_configuration(chip))
		return -ENODEV;

	/* Set mode, clock out, comparator */
	can_write_reg(chip,chip->sja_cdr_reg,SJACDR); 
	/* Set driver output configuration */
	can_write_reg(chip,chip->sja_ocr_reg,SJAOCR); 

	if (sja1000_standard_mask(chip,0x0000, 0xffff))
		return -ENODEV;
	
	if (!chip->baudrate)
		chip->baudrate=1000000;
	if (sja1000_baud_rate(chip,chip->baudrate,chip->clock,0,75,0))
		return -ENODEV;

	/* Enable hardware interrupts */
	can_write_reg(chip,(sjaCR_RIE|sjaCR_TIE|sjaCR_EIE|sjaCR_OIE),SJACR); 

	sja1000_disable_configuration(chip);
	
	return 0;
}

int sja1000_standard_mask(struct canchip_t *chip, unsigned short code, unsigned short mask)
{
	unsigned char write_code, write_mask;

	if (sja1000_enable_configuration(chip))
		return -ENODEV;

	/* The acceptance code bits (SJAACR bits 0-7) and the eight most 
	 * significant bits of the message identifier (id.10 to id.3) must be
	 * equal to those bit positions which are marked relevant by the 
	 * acceptance mask bits (SJAAMR bits 0-7).
	 * (id.10 to id.3) = (SJAACR.7 to SJAACR.0) v (SJAAMR.7 to SJAAMR.0)
	 * (Taken from Philips sja1000 Data Sheet)
	 */
	write_code = (unsigned char) code >> 3;
	write_mask = (unsigned char) mask >> 3;
	
	can_write_reg(chip,write_code,SJAACR);
	can_write_reg(chip,write_mask,SJAAMR);

	DEBUGMSG("Setting acceptance code to 0x%lx\n",(unsigned long)code);
	DEBUGMSG("Setting acceptance mask to 0x%lx\n",(unsigned long)mask);

	sja1000_disable_configuration(chip);

	return 0;
}

/* Set communication parameters.
 * param rate baud rate in Hz
 * param clock frequency of sja1000 clock in Hz (ISA osc is 14318000)
 * param sjw synchronization jump width (0-3) prescaled clock cycles
 * param sampl_pt sample point in % (0-100) sets (TSEG1+2)/(TSEG1+TSEG2+3) ratio
 * param flags fields BTR1_SAM, OCMODE, OCPOL, OCTP, OCTN, CLK_OFF, CBP
 */
int sja1000_baud_rate(struct canchip_t *chip, int rate, int clock, int sjw,
							int sampl_pt, int flags)
{
	int best_error = 1000000000, error;
	int best_tseg=0, best_brp=0, best_rate=0, brp=0;
	int tseg=0, tseg1=0, tseg2=0;
	
	if (sja1000_enable_configuration(chip))
		return -ENODEV;

	clock /=2;

	/* tseg even = round down, odd = round up */
	for (tseg=(0+0+2)*2; tseg<=(MAX_TSEG2+MAX_TSEG1+2)*2+1; tseg++) {
		brp = clock/((1+tseg/2)*rate)+tseg%2;
		if (brp == 0 || brp > 64)
			continue;
		error = rate - clock/(brp*(1+tseg/2));
		if (error < 0)
			error = -error;
		if (error <= best_error) {
			best_error = error;
			best_tseg = tseg/2;
			best_brp = brp-1;
			best_rate = clock/(brp*(1+tseg/2));
		}
	}
	if (best_error && (rate/best_error < 10)) {
		CANMSG("baud rate %d is not possible with %d Hz clock\n",
								rate, 2*clock);
		CANMSG("%d bps. brp=%d, best_tseg=%d, tseg1=%d, tseg2=%d\n",
				best_rate, best_brp, best_tseg, tseg1, tseg2);
		return -EINVAL;
	}
	tseg2 = best_tseg-(sampl_pt*(best_tseg+1))/100;
	if (tseg2 < 0)
		tseg2 = 0;
	if (tseg2 > MAX_TSEG2)
		tseg2 = MAX_TSEG2;
	tseg1 = best_tseg-tseg2-2;
	if (tseg1 > MAX_TSEG1) {
		tseg1 = MAX_TSEG1;
		tseg2 = best_tseg-tseg1-2;
	}

	DEBUGMSG("Setting %d bps.\n", best_rate);
	DEBUGMSG("brp=%d, best_tseg=%d, tseg1=%d, tseg2=%d, sampl_pt=%d\n",
					best_brp, best_tseg, tseg1, tseg2,
					(100*(best_tseg-tseg2)/(best_tseg+1)));


	can_write_reg(chip, sjw<<6 | best_brp, SJABTR0);
	can_write_reg(chip, ((flags & BTR1_SAM) != 0)<<7 | tseg2<<4 | tseg1,
								SJABTR1);
//	can_write_reg(chip, sjaOCR_MODE_NORMAL | sjaOCR_TX0_LH | sjaOCR_TX1_ZZ, SJAOCR);
	/* BASIC mode, bypass input comparator */
//	can_write_reg(chip, sjaCDR_CBP| /* sjaCDR_CLK_OFF | */ 7, SJACDR);

	sja1000_disable_configuration(chip);

	return 0;
}

int sja1000_pre_read_config(struct canchip_t *chip, struct msgobj_t *obj)
{
	int i;
	
	i=can_read_reg(chip,SJASR);
	
	if (!(i&sjaSR_RBS)) {
//Temp
		for (i=0; i<0x20; i++)
			CANMSG("0x%x is 0x%x\n",i,can_read_reg(chip,i));
			return 0;
	}
	sja1000_start_chip(chip);

    // disable interrupts for a moment
	can_write_reg(chip, 0, SJACR); 

	sja1000_irq_read_handler(chip, obj);

    // enable interrupts
	can_write_reg(chip, sjaCR_OIE | sjaCR_EIE | sjaCR_TIE | sjaCR_RIE, SJACR);


	return 1;
}

#define MAX_TRANSMIT_WAIT_LOOPS 10

int sja1000_pre_write_config(struct canchip_t *chip, struct msgobj_t *obj, 
							struct canmsg_t *msg)
{
	int i=0, id=0;
	int len;

	sja1000_start_chip(chip); //sja1000 goes automatically into reset mode on errors

	/* Wait until Transmit Buffer Status is released */
	while ( !(can_read_reg(chip, SJASR) & sjaSR_TBS) && 
						i++<MAX_TRANSMIT_WAIT_LOOPS) {
		udelay(i);
	}
	
	if (!(can_read_reg(chip, SJASR) & sjaSR_TBS)) {
		CANMSG("Transmit timed out, cancelling\n");
		can_write_reg(chip, sjaCMR_AT, SJACMR);
		i=0;
		while ( !(can_read_reg(chip, SJASR) & sjaSR_TBS) &&
				i++<MAX_TRANSMIT_WAIT_LOOPS) {
			udelay(i);
		}
		if (!(can_read_reg(chip, SJASR) & sjaSR_TBS)) {
			CANMSG("Could not cancel, please reset\n");
			return -EIO;
		}
	}

	len = msg->length;
	if(len > CAN_MSG_LENGTH) len = CAN_MSG_LENGTH;
	id = (msg->id<<5) | ((msg->flags&MSG_RTR)?sjaID0_RTR:0) | len;

	can_write_reg(chip, id>>8, SJATXID1);
	can_write_reg(chip, id & 0xff , SJATXID0);

	for (i=0; i<len; i++)
		can_write_reg(chip, msg->data[i], SJATXDAT0+i);

	return 0;
}

int sja1000_send_msg(struct canchip_t *chip, struct msgobj_t *obj, 
							struct canmsg_t *msg)
{
	can_write_reg(chip, sjaCMR_TR, SJACMR);

	return 0;
}

int sja1000_check_tx_stat(struct canchip_t *chip)
{
	if (can_read_reg(chip,SJASR) & sjaSR_TCS)
		return 0;
	else
		return 1;
}

int sja1000_set_btregs(struct canchip_t *chip, unsigned short btr0, 
							unsigned short btr1)
{
	if (sja1000_enable_configuration(chip))
		return -ENODEV;

	can_write_reg(chip, btr0, SJABTR0);
	can_write_reg(chip, btr1, SJABTR1);

	sja1000_disable_configuration(chip);

	return 0;
}

int sja1000_start_chip(struct canchip_t *chip)
{
	unsigned short flags = 0;

	flags = can_read_reg(chip, SJACR) & (sjaCR_RIE|sjaCR_TIE|sjaCR_EIE|sjaCR_OIE);
	can_write_reg(chip, flags, SJACR);

	return 0;
}

int sja1000_stop_chip(struct canchip_t *chip)
{
	unsigned short flags = 0;

	flags = can_read_reg(chip, SJACR) & (sjaCR_RIE|sjaCR_TIE|sjaCR_EIE|sjaCR_OIE);
	can_write_reg(chip, flags|sjaCR_RR, SJACR);

	return 0;
}

int sja1000_attach_to_chip(struct canchip_t *chip)
{
	return 0;
}

int sja1000_release_chip(struct canchip_t *chip)
{
	sja1000_stop_chip(chip);
	can_write_reg(chip,sjaCR_RR,SJACR);

	return 0;
}

int sja1000_remote_request(struct canchip_t *chip, struct msgobj_t *obj)
{
	CANMSG("sja1000_remote_request not implemented\n");
	return -ENOSYS;
}

int sja1000_extended_mask(struct canchip_t *chip, unsigned long code,
		unsigned long mask)
{
	CANMSG("sja1000_extended_mask not implemented\n");
	return -ENOSYS;
}

int sja1000_clear_objects(struct canchip_t *chip)
{
	CANMSG("sja1000_clear_objects not implemented\n");
	return -ENOSYS;
}

int sja1000_config_irqs(struct canchip_t *chip, short irqs)
{
	CANMSG("sja1000_config_irqs not implemented\n");
	return -ENOSYS;
}


int sja1000_irq_handler(int irq, struct canchip_t *chip)
{
	unsigned irq_register;
	struct msgobj_t *obj=chip->msgobj[0];
	int loop_cnt=CHIP_MAX_IRQLOOP;

	irq_register=can_read_reg(chip, SJAIR);
//	DEBUGMSG("sja1000_irq_handler: SJAIR:%02x\n",irq_register);
//	DEBUGMSG("sja1000_irq_handler: SJASR:%02x\n",
//					can_read_reg(chip, SJASR));

	if ((irq_register & (sjaIR_WUI|sjaIR_DOI|sjaIR_EI|sjaIR_TI|sjaIR_RI)) == 0)
		return CANCHIP_IRQ_NONE;

	do {

		if(!loop_cnt--) {
			CANMSG("sja1000_irq_handler IRQ %d stuck\n",irq);
			return CANCHIP_IRQ_STUCK;
		}

		if ((irq_register & sjaIR_RI) != 0) 
			sja1000_irq_read_handler(chip, obj);

		if ((irq_register & sjaIR_TI) != 0) { 
			can_msgobj_set_fl(obj,TX_REQUEST);
			while(!can_msgobj_test_and_set_fl(obj,TX_LOCK)){
				can_msgobj_clear_fl(obj,TX_REQUEST);

				if (can_read_reg(chip, SJASR) & sjaSR_TBS)
					sja1000_irq_write_handler(chip, obj);

				can_msgobj_clear_fl(obj,TX_LOCK);
				if(!can_msgobj_test_fl(obj,TX_REQUEST)) break;
			}
		}

		if ((irq_register & (sjaIR_EI|sjaIR_DOI)) != 0) { 
			// Some error happened
// FIXME: chip should be brought to usable state. Transmission cancelled if in progress.
// Reset flag set to 0 if chip is already off the bus. Full state report
			CANMSG("Error: status register: 0x%x irq_register: 0x%02x\n",
				can_read_reg(chip, SJASR), irq_register);
			obj->ret=-1;

			if(obj->tx_slot){
				canque_notify_inends(obj->tx_qedge, CANQUEUE_NOTIFY_ERRTX_BUS);
				/*canque_free_outslot(obj->qends, obj->tx_qedge, obj->tx_slot);
				obj->tx_slot=NULL;*/
			}
		}

		irq_register=can_read_reg(chip, SJAIR);

	} while(irq_register & (sjaIR_WUI|sjaIR_DOI|sjaIR_EI|sjaIR_TI|sjaIR_RI));
	
	return CANCHIP_IRQ_HANDLED;
}

void sja1000_irq_read_handler(struct canchip_t *chip, struct msgobj_t *obj)
{
	int i=0, id=0, len;

	do {
		id = can_read_reg(chip, SJARXID0) | (can_read_reg(chip, SJARXID1)<<8);
		obj->rx_msg.length = len = id & 0x0f;
		obj->rx_msg.flags = id&sjaID0_RTR ? MSG_RTR : 0;
		obj->rx_msg.cob = 0;
		obj->rx_msg.id = id>>5;

		if(len > CAN_MSG_LENGTH) len = CAN_MSG_LENGTH;
		for (i=0; i<len; i++)
			obj->rx_msg.data[i]=can_read_reg(chip, SJARXDAT0 + i);

		can_write_reg(chip, sjaCMR_RRB, SJACMR);

		/* fill CAN message timestamp */
		can_filltimestamp(&obj->rx_msg.timestamp);

		canque_filter_msg2edges(obj->qends, &obj->rx_msg);
	} while(can_read_reg(chip, SJASR) & sjaSR_RBS);
}

void sja1000_irq_write_handler(struct canchip_t *chip, struct msgobj_t *obj)
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

	cmd=canque_test_outslot(obj->qends, &obj->tx_qedge, &obj->tx_slot);
	if(cmd<0)
		return;

	if (chip->chipspecops->pre_write_config(chip, obj, &obj->tx_slot->msg)) {
		obj->ret = -1;
		canque_notify_inends(obj->tx_qedge, CANQUEUE_NOTIFY_ERRTX_PREP);
		canque_free_outslot(obj->qends, obj->tx_qedge, obj->tx_slot);
		obj->tx_slot=NULL;
		return;
	}
	if (chip->chipspecops->send_msg(chip, obj, &obj->tx_slot->msg)) {
		obj->ret = -1;
		canque_notify_inends(obj->tx_qedge, CANQUEUE_NOTIFY_ERRTX_SEND);
		canque_free_outslot(obj->qends, obj->tx_qedge, obj->tx_slot);
		obj->tx_slot=NULL;
		return;
	}
}

/**
 * sja1000_wakeup_tx: - wakeups TX processing
 * @chip: pointer to chip state structure
 * @obj: pointer to message object structure
 *
 * Return Value: negative value reports error.
 * File: src/sja1000.c
 */
int sja1000_wakeup_tx(struct canchip_t *chip, struct msgobj_t *obj)
{
	can_preempt_disable();
	
	can_msgobj_set_fl(obj,TX_REQUEST);
	while(!can_msgobj_test_and_set_fl(obj,TX_LOCK)){
		can_msgobj_clear_fl(obj,TX_REQUEST);

		if (can_read_reg(chip, SJASR) & sjaSR_TBS)
			sja1000_irq_write_handler(chip, obj);
	
		can_msgobj_clear_fl(obj,TX_LOCK);
		if(!can_msgobj_test_fl(obj,TX_REQUEST)) break;
	}

	can_preempt_enable();
	return 0;
}

int sja1000_register(struct chipspecops_t *chipspecops)
{
	chipspecops->chip_config = sja1000_chip_config;
	chipspecops->baud_rate = sja1000_baud_rate;
	chipspecops->standard_mask = sja1000_standard_mask;
	chipspecops->extended_mask = sja1000_extended_mask;
	chipspecops->message15_mask = sja1000_extended_mask;
	chipspecops->clear_objects = sja1000_clear_objects;
	chipspecops->config_irqs = sja1000_config_irqs;
	chipspecops->pre_read_config = sja1000_pre_read_config;
	chipspecops->pre_write_config = sja1000_pre_write_config;
	chipspecops->send_msg = sja1000_send_msg;
	chipspecops->check_tx_stat = sja1000_check_tx_stat;
	chipspecops->wakeup_tx=sja1000_wakeup_tx;
	chipspecops->remote_request = sja1000_remote_request;
	chipspecops->enable_configuration = sja1000_enable_configuration;
	chipspecops->disable_configuration = sja1000_disable_configuration;
	chipspecops->set_btregs = sja1000_set_btregs;
	chipspecops->attach_to_chip=sja1000_attach_to_chip;
	chipspecops->release_chip=sja1000_release_chip;
	chipspecops->start_chip = sja1000_start_chip;
	chipspecops->stop_chip = sja1000_stop_chip;
	chipspecops->irq_handler = sja1000_irq_handler;
	chipspecops->irq_accept = NULL;
	return 0;
}

int sja1000_fill_chipspecops(struct canchip_t *chip)
{
	chip->chip_type="sja1000";
	chip->max_objects=1;
	sja1000_register(chip->chipspecops);
	return 0;
}

