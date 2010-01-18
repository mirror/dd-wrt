/* i82527.c
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
#include "../include/i82527.h"

void i82527_irq_rtr_handler(struct canchip_t *chip, struct msgobj_t *obj, 
			    struct rtr_id *rtr_search, unsigned long message_id);


extern int stdmask;
extern int extmask;
extern int mo15mask;

/* helper functions for segmented cards read and write configuration and status registers
   above 0xf offset */

void i82527_seg_write_reg(const struct canchip_t *chip, unsigned char data, unsigned address)
{
	if((address > 0xf) && (chip->flags & CHIP_SEGMENTED))
		canobj_write_reg(chip, chip->msgobj[(address>>4)-1],data, address & 0xf);
	else
		can_write_reg(chip, data, address);
}

unsigned i82527_seg_read_reg(const struct canchip_t *chip, unsigned address)
{
	if((address > 0xf) && (chip->flags & CHIP_SEGMENTED))
		return canobj_read_reg(chip, chip->msgobj[(address>>4)-1], address & 0xf);
	else
		return can_read_reg(chip, address);
}

int i82527_enable_configuration(struct canchip_t *chip)
{
	unsigned short flags=0;

	flags = can_read_reg(chip, iCTL) & (iCTL_IE|iCTL_SIE|iCTL_EIE);
	can_write_reg(chip, flags|iCTL_CCE, iCTL);
	
	return 0;
}

int i82527_disable_configuration(struct canchip_t *chip)
{
	unsigned short flags=0;

	flags = can_read_reg(chip, iCTL) & (iCTL_IE|iCTL_SIE|iCTL_EIE);
	can_write_reg(chip, flags, iCTL);

	return 0;
}

int i82527_chip_config(struct canchip_t *chip)
{
	can_write_reg(chip,chip->int_cpu_reg,iCPU); // Configure cpu interface
	can_write_reg(chip,(iCTL_CCE|iCTL_INI),iCTL); // Enable configuration
	i82527_seg_write_reg(chip,chip->int_clk_reg,iCLK); // Set clock out slew rates 
	i82527_seg_write_reg(chip,chip->int_bus_reg,iBUS); /* Bus configuration */
	can_write_reg(chip,0x00,iSTAT); /* Clear error status register */

	/* Check if we can at least read back some arbitrary data from the 
	 * card. If we can not, the card is not properly configured!
	 */
	canobj_write_reg(chip,chip->msgobj[1],0x25,iMSGDAT1);
	canobj_write_reg(chip,chip->msgobj[2],0x52,iMSGDAT3);
	canobj_write_reg(chip,chip->msgobj[10],0xc3,iMSGDAT6);
	if ( (canobj_read_reg(chip,chip->msgobj[1],iMSGDAT1) != 0x25) ||
	      (canobj_read_reg(chip,chip->msgobj[2],iMSGDAT3) != 0x52) ||
	      (canobj_read_reg(chip,chip->msgobj[10],iMSGDAT6) != 0xc3) ) {
		CANMSG("Could not read back from the hardware.\n");
		CANMSG("This probably means that your hardware is not correctly configured!\n");
		return -1;
	}
	else
		DEBUGMSG("Could read back, hardware is probably configured correctly\n");

	if (chip->baudrate == 0)
		chip->baudrate=1000000;

	if (i82527_baud_rate(chip,chip->baudrate,chip->clock,0,75,0)) {
		CANMSG("Error configuring baud rate\n");
		return -ENODEV;
	}
	if (i82527_standard_mask(chip,0x0000,stdmask)) {
		CANMSG("Error configuring standard mask\n");
		return -ENODEV;
	}
	if (i82527_extended_mask(chip,0x00000000,extmask)) {
		CANMSG("Error configuring extended mask\n");
		return -ENODEV;
	}
	if (i82527_message15_mask(chip,0x00000000,mo15mask)) {
		CANMSG("Error configuring message 15 mask\n");
		return -ENODEV;
	}
	if (i82527_clear_objects(chip)) {
		CANMSG("Error clearing message objects\n");
		return -ENODEV;
	}
	if (i82527_config_irqs(chip,iCTL_IE|iCTL_EIE)) { /* has been 0x0a */
		CANMSG("Error configuring interrupts\n");
		return -ENODEV;
	}

	return 0;
}

/* Set communication parameters.
 * param rate baud rate in Hz
 * param clock frequency of i82527 clock in Hz (ISA osc is 14318000)
 * param sjw synchronization jump width (0-3) prescaled clock cycles
 * param sampl_pt sample point in % (0-100) sets (TSEG1+2)/(TSEG1+TSEG2+3) ratio
 * param flags fields BTR1_SAM, OCMODE, OCPOL, OCTP, OCTN, CLK_OFF, CBP
 */
int i82527_baud_rate(struct canchip_t *chip, int rate, int clock, int sjw,
							int sampl_pt, int flags)
{
	int best_error = 1000000000, error;
	int best_tseg=0, best_brp=0, best_rate=0, brp=0;
	int tseg=0, tseg1=0, tseg2=0;
	
	if (i82527_enable_configuration(chip))
		return -ENODEV;

	if(chip->int_cpu_reg & iCPU_DSC)
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
	if (tseg1>MAX_TSEG1) {
		tseg1 = MAX_TSEG1;
		tseg2 = best_tseg-tseg1-2;
	}

	DEBUGMSG("Setting %d bps.\n", best_rate);
	DEBUGMSG("brp=%d, best_tseg=%d, tseg1=%d, tseg2=%d, sampl_pt=%d\n",
					best_brp, best_tseg, tseg1, tseg2,
					(100*(best_tseg-tseg2)/(best_tseg+1)));
					
				
	i82527_seg_write_reg(chip, sjw<<6 | best_brp, iBT0);
	can_write_reg(chip, ((flags & BTR1_SAM) != 0)<<7 | tseg2<<4 | tseg1,
								iBT1);
	DEBUGMSG("Writing 0x%x to iBT0\n",(sjw<<6 | best_brp));
	DEBUGMSG("Writing 0x%x to iBT1\n",((flags & BTR1_SAM) != 0)<<7 | 
							tseg2<<4 | tseg1);

	i82527_disable_configuration(chip);

	return 0;
}

int i82527_standard_mask(struct canchip_t *chip, unsigned short code, unsigned short mask)
{
	unsigned char mask0, mask1;

	mask0 = (unsigned char) (mask >> 3);
	mask1 = (unsigned char) (mask << 5);
	
	can_write_reg(chip,mask0,iSGM0);
	can_write_reg(chip,mask1,iSGM1);

	DEBUGMSG("Setting standard mask to 0x%lx\n",(unsigned long)mask);

	return 0;
}

int i82527_extended_mask(struct canchip_t *chip, unsigned long code, unsigned long mask)
{
	unsigned char mask0, mask1, mask2, mask3;

	mask0 = (unsigned char) (mask >> 21);
	mask1 = (unsigned char) (mask >> 13);
	mask2 = (unsigned char) (mask >> 5);
	mask3 = (unsigned char) (mask << 3);

	can_write_reg(chip,mask0,iEGM0);
	can_write_reg(chip,mask1,iEGM1);
	can_write_reg(chip,mask2,iEGM2);
	can_write_reg(chip,mask3,iEGM3);

	DEBUGMSG("Setting extended mask to 0x%lx\n",(unsigned long)mask);

	return 0;
}

int i82527_message15_mask(struct canchip_t *chip, unsigned long code, unsigned long mask)
{
	unsigned char mask0, mask1, mask2, mask3;

	mask0 = (unsigned char) (mask >> 21);
	mask1 = (unsigned char) (mask >> 13);
	mask2 = (unsigned char) (mask >> 5);
	mask3 = (unsigned char) (mask << 3);

	can_write_reg(chip,mask0,i15M0);
	can_write_reg(chip,mask1,i15M1);
	can_write_reg(chip,mask2,i15M2);
	can_write_reg(chip,mask3,i15M3);

	DEBUGMSG("Setting message 15 mask to 0x%lx\n",mask);

	return 0;


}

int i82527_clear_objects(struct canchip_t *chip)
{
	int i=0,id=0,data=0;
	struct msgobj_t *obj;

	DEBUGMSG("Cleared all message objects on chip\n");

	for (i=0; i<chip->max_objects; i++) {
		obj=chip->msgobj[i];
		canobj_write_reg(chip,obj,(INTPD_RES|RXIE_RES|TXIE_RES|MVAL_RES),iMSGCTL0);
		canobj_write_reg(chip,obj,(NEWD_RES|MLST_RES|TXRQ_RES|RMPD_RES), iMSGCTL1);
		for (data=0x07; data<0x0f; data++)
			canobj_write_reg(chip,obj,0x00,data);
		for (id=2; id<6; id++) {
			canobj_write_reg(chip,obj,0x00,id);
		}
		if (extended==0) {
			canobj_write_reg(chip,obj,0x00,iMSGCFG);
		}
		else {
			canobj_write_reg(chip,obj,MCFG_XTD,iMSGCFG);
		}
	}
	if (extended==0)
		DEBUGMSG("All message ID's set to standard\n");
	else
		DEBUGMSG("All message ID's set to extended\n");
	
	return 0;
}

int i82527_config_irqs(struct canchip_t *chip, short irqs)
{
	can_write_reg(chip,irqs,iCTL);
	DEBUGMSG("Configured hardware interrupt delivery\n");
	return 0;
}

int i82527_pre_read_config(struct canchip_t *chip, struct msgobj_t *obj)
{
	unsigned long id=obj->rx_preconfig_id;

	can_msgobj_set_fl(obj,RX_MODE);

	if (extended || can_msgobj_test_fl(obj,RX_MODE_EXT)) {
		id<<=3;
		canobj_write_reg(chip,obj,id,iMSGID3);
		canobj_write_reg(chip,obj,id>>8,iMSGID2);
		canobj_write_reg(chip,obj,id>>16,iMSGID1);
		canobj_write_reg(chip,obj,id>>24,iMSGID0);
		canobj_write_reg(chip,obj,MCFG_XTD,iMSGCFG);
	} else {
		id<<=5;
		canobj_write_reg(chip,obj,id,iMSGID1);
		canobj_write_reg(chip,obj,id>>8,iMSGID0);
		canobj_write_reg(chip,obj,0x00,iMSGCFG);
	}

	canobj_write_reg(chip,obj,(NEWD_RES|MLST_RES|TXRQ_RES|RMPD_RES), iMSGCTL1);
	canobj_write_reg(chip,obj,(MVAL_SET|TXIE_RES|RXIE_SET|INTPD_RES),iMSGCTL0);

	DEBUGMSG("i82527_pre_read_config: configured obj at 0x%08lx\n",obj->obj_base_addr);

	return 0;
}

int i82527_pre_write_config(struct canchip_t *chip, struct msgobj_t *obj,
							struct canmsg_t *msg)
{
	int i=0,id0=0,id1=0,id2=0,id3=0;
	int len;
	
	len = msg->length;
	if(len > CAN_MSG_LENGTH) len = CAN_MSG_LENGTH;

	can_msgobj_clear_fl(obj,RX_MODE);

	canobj_write_reg(chip,obj,(MVAL_SET|TXIE_SET|RXIE_RES|INTPD_RES),iMSGCTL0);
	canobj_write_reg(chip,obj,(RMPD_RES|TXRQ_RES|CPUU_SET|NEWD_RES),iMSGCTL1);

	if (extended || (msg->flags&MSG_EXT)) {
		canobj_write_reg(chip,obj,(len<<4)|(MCFG_DIR|MCFG_XTD),iMSGCFG);
		id0 = (unsigned char) (msg->id<<3);
		id1 = (unsigned char) (msg->id>>5);
		id2 = (unsigned char) (msg->id>>13);
		id3 = (unsigned char) (msg->id>>21);
		canobj_write_reg(chip,obj,id0,iMSGID3);
		canobj_write_reg(chip,obj,id1,iMSGID2);
		canobj_write_reg(chip,obj,id2,iMSGID1);
		canobj_write_reg(chip,obj,id3,iMSGID0);
	}
	else {
		canobj_write_reg(chip,obj,(len<<4)|MCFG_DIR,iMSGCFG);
		id1 = (unsigned char) (msg->id<<5);
		id0 = (unsigned char) (msg->id>>3);
		canobj_write_reg(chip,obj,id1,iMSGID1);
		canobj_write_reg(chip,obj,id0,iMSGID0);
	}
	canobj_write_reg(chip,obj,RMPD_UNC|TXRQ_UNC|CPUU_SET|NEWD_SET,iMSGCTL1);
	for (i=0; i<len; i++) {
		canobj_write_reg(chip,obj,msg->data[i],iMSGDAT0+i);
	}

	return 0;
}

int i82527_send_msg(struct canchip_t *chip, struct msgobj_t *obj,
							struct canmsg_t *msg)
{
	canobj_write_reg(chip,obj,(MVAL_SET|TXIE_SET|RXIE_RES|INTPD_RES),iMSGCTL0);

	if (msg->flags & MSG_RTR) {
		canobj_write_reg(chip,obj,(RMPD_RES|TXRQ_RES|CPUU_RES|NEWD_SET),iMSGCTL1);
	}
	else {
		canobj_write_reg(chip,obj,(RMPD_RES|TXRQ_SET|CPUU_RES|NEWD_SET),iMSGCTL1);
	}

	return 0;
}

int i82527_check_tx_stat(struct canchip_t *chip)
{
	if (can_read_reg(chip,iSTAT) & iSTAT_TXOK) {
		can_write_reg(chip,0x0,iSTAT);
		return 0;
	}
	else {
		can_write_reg(chip,0x0,iSTAT);
		return 1;
	}
}

int i82527_remote_request(struct canchip_t *chip, struct msgobj_t *obj)
{
	canobj_write_reg(chip,obj,(MVAL_SET|TXIE_RES|RXIE_SET|INTPD_RES),iMSGCTL0);
	canobj_write_reg(chip,obj,(RMPD_RES|TXRQ_SET|MLST_RES|NEWD_RES),iMSGCTL1);
	
	return 0;
}

int i82527_set_btregs(struct canchip_t *chip, unsigned short btr0,
							unsigned short btr1)
{
	if (i82527_enable_configuration(chip))
		return -ENODEV;

	i82527_seg_write_reg(chip, btr0, iBT0);
	i82527_seg_write_reg(chip, btr1, iBT1);

	i82527_disable_configuration(chip);

	return 0;
}

int i82527_start_chip(struct canchip_t *chip)
{
	unsigned short flags = 0;

	flags = can_read_reg(chip, iCTL) & (iCTL_IE|iCTL_SIE|iCTL_EIE);
	can_write_reg(chip, flags, iCTL);
	
	return 0;
}

int i82527_stop_chip(struct canchip_t *chip)
{
	unsigned short flags = 0;

	flags = can_read_reg(chip, iCTL) & (iCTL_IE|iCTL_SIE|iCTL_EIE);
	can_write_reg(chip, flags|(iCTL_CCE|iCTL_INI), iCTL);

	return 0;
}

int i82527_attach_to_chip(struct canchip_t *chip)
{
	return 0;
}

int i82527_release_chip(struct canchip_t *chip)
{
	i82527_stop_chip(chip);
	can_write_reg(chip, (iCTL_CCE|iCTL_INI), iCTL);

	return 0;
}

static inline 
void i82527_irq_write_handler(struct canchip_t *chip, struct msgobj_t *obj)
{
	int cmd;

	canobj_write_reg(chip,obj,(MVAL_RES|TXIE_RES|RXIE_RES|INTPD_RES),iMSGCTL0);

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
	return;
}

static inline
void i82527_irq_read_handler(struct canchip_t *chip, struct msgobj_t *obj, int objnum)
{
	int i;
	unsigned long message_id;
	int msgcfg, msgctl1;
	
	msgctl1=canobj_read_reg(chip,obj,iMSGCTL1);
	if(msgctl1 & NEWD_RES)
		return;
	
	do {
		if(objnum != 14) {
			canobj_write_reg(chip,obj,(RMPD_RES|TXRQ_RES|MLST_UNC|NEWD_RES),iMSGCTL1);
			canobj_write_reg(chip,obj,(MVAL_SET|TXIE_RES|RXIE_SET|INTPD_RES),iMSGCTL0);
		}

		msgcfg = canobj_read_reg(chip,obj,iMSGCFG);

		if (msgcfg&MCFG_XTD) {
			message_id =canobj_read_reg(chip,obj,iMSGID3);
			message_id|=canobj_read_reg(chip,obj,iMSGID2)<<8;
			message_id|=canobj_read_reg(chip,obj,iMSGID1)<<16;
			message_id|=canobj_read_reg(chip,obj,iMSGID0)<<24;
			message_id>>=3;
			obj->rx_msg.flags = MSG_EXT;

		}
		else {
			message_id =canobj_read_reg(chip,obj,iMSGID1);
			message_id|=canobj_read_reg(chip,obj,iMSGID0)<<8;
			message_id>>=5;
			obj->rx_msg.flags = 0;
		}

		obj->rx_msg.length = (msgcfg >> 4) & 0xf;
		if(obj->rx_msg.length > CAN_MSG_LENGTH) obj->rx_msg.length = CAN_MSG_LENGTH;

		obj->rx_msg.id = message_id;

		for (i=0; i < obj->rx_msg.length; i++)
			obj->rx_msg.data[i] = canobj_read_reg(chip,obj,iMSGDAT0+i);

		
		if(objnum != 14) {
			/* if NEWD is set after data read, then read data are likely inconsistent */
			msgctl1=canobj_read_reg(chip,obj,iMSGCTL1);
			if(msgctl1 & NEWD_SET) {
				CANMSG("i82527_irq_read_handler: object %d data overwritten\n",objnum);
				continue;
			}
		}
		else {
			/* this object is special and data are queued in the shadow register */
			canobj_write_reg(chip,obj,(MVAL_SET|TXIE_RES|RXIE_SET|INTPD_RES),iMSGCTL0);
			canobj_write_reg(chip,obj,(RMPD_RES|TXRQ_RES|MLST_UNC|NEWD_RES),iMSGCTL1);
			msgctl1=canobj_read_reg(chip,obj,iMSGCTL1);
		}
		

		/* fill CAN message timestamp */
		can_filltimestamp(&obj->rx_msg.timestamp);

		canque_filter_msg2edges(obj->qends, &obj->rx_msg);
		
		if (msgctl1 & NEWD_SET)
			continue;
		
		if (msgctl1 & MLST_SET) {
			canobj_write_reg(chip,obj,(RMPD_UNC|TXRQ_UNC|MLST_RES|NEWD_UNC),iMSGCTL1);
			CANMSG("i82527_irq_read_handler: object %d message lost\n",objnum);
		}
		
		return;

	} while(1);
}

/*
			if (msgcfg&MCFG_XTD) {
				message_id =canobj_read_reg(chip,obj,iMSGID3);
				message_id|=canobj_read_reg(chip,obj,iMSGID2)<<8;
				message_id|=canobj_read_reg(chip,obj,iMSGID1)<<16;
				message_id|=canobj_read_reg(chip,obj,iMSGID0)<<24;
				message_id>>=3;
			}
			else {
				message_id =canobj_read_reg(chip,obj,iMSGID1);
				message_id|=canobj_read_reg(chip,obj,iMSGID0)<<8;
				message_id>>=5;
			}

			can_spin_lock(&hardware_p->rtr_lock);
			rtr_search=hardware_p->rtr_queue;
			while (rtr_search != NULL) {
				if (rtr_search->id == message_id)
					break;
				rtr_search=rtr_search->next;
			}
			can_spin_unlock(&hardware_p->rtr_lock);
			if ((rtr_search!=NULL) && (rtr_search->id==message_id))
				i82527_irq_rtr_handler(chip, obj, rtr_search, message_id);
			else
				i82527_irq_read_handler(chip, obj, message_id); 
*/


static inline 
void i82527_irq_update_filter(struct canchip_t *chip, struct msgobj_t *obj)
{
	struct canfilt_t filt;

	if(canqueue_ends_filt_conjuction(obj->qends, &filt)) {
		obj->rx_preconfig_id=filt.id;
		canobj_write_reg(chip,obj,(MVAL_RES|TXIE_RES|RXIE_RES|INTPD_RES),iMSGCTL0);
		if(obj->object == 15) {
			i82527_message15_mask(chip,filt.id,filt.mask);
		}
		if (filt.flags&MSG_EXT)
			can_msgobj_set_fl(obj,RX_MODE_EXT);
		else
			can_msgobj_clear_fl(obj,RX_MODE_EXT);

		i82527_pre_read_config(chip, obj);

		CANMSG("i82527_irq_update_filter: obj at 0x%08lx\n",
			can_ioptr2ulong(obj->obj_base_addr));
	}
}


int i82527_irq_sync_activities(struct canchip_t *chip, struct msgobj_t *obj)
{
	int job_done=0;

	while(!can_msgobj_test_and_set_fl(obj,TX_LOCK)) {

		if(can_msgobj_test_and_clear_fl(obj,TX_REQUEST)) {
			if(canobj_read_reg(chip,obj,iMSGCTL1)&TXRQ_RES)
				i82527_irq_write_handler(chip, obj);
		}

		if(!obj->tx_slot) {
			if(can_msgobj_test_and_clear_fl(obj,FILTCH_REQUEST)) {
				i82527_irq_update_filter(chip, obj);
			}
		}

		job_done=1;

		mb();

		can_msgobj_clear_fl(obj,TX_LOCK);
		if(can_msgobj_test_fl(obj,TX_REQUEST))
			continue;
		if(can_msgobj_test_fl(obj,FILTCH_REQUEST) && !obj->tx_slot)
			continue;
		break;
	}

	return job_done;
}

int i82527_irq_handler(int irq, struct canchip_t *chip)
{
	unsigned char msgcfg;

	unsigned irq_register;
	unsigned status_register;
	unsigned object;
	struct msgobj_t *obj;
	int loop_cnt=CHIP_MAX_IRQLOOP;

	/*put_reg=device->hwspecops->write_register;*/
	/*get_reg=device->hwspecops->read_register;*/

	irq_register = i82527_seg_read_reg(chip, iIRQ);

	if(!irq_register) {
		DEBUGMSG("i82527: spurious IRQ\n");
		return CANCHIP_IRQ_NONE;
	}


	do {

		if(!loop_cnt--) {
			CANMSG("i82527_irq_handler IRQ %d stuck\n",irq);
			CANMSG("i82527_irq_register 0x%x\n",irq_register);
			return CANCHIP_IRQ_STUCK;
		}
		
		DEBUGMSG("i82527: iIRQ 0x%02x\n",irq_register);
		
		if (irq_register == 0x01) {
			status_register=can_read_reg(chip, iSTAT);
			CANMSG("Status register: 0x%x\n",status_register);
			continue;
			/*return CANCHIP_IRQ_NONE;*/
		}
		
		if (irq_register == 0x02)
			object = 14;
		else if(irq_register <= 13+3)
			object = irq_register-3;
		else
			return CANCHIP_IRQ_NONE;

		obj=chip->msgobj[object];
		
		msgcfg = canobj_read_reg(chip,obj,iMSGCFG);
		if (msgcfg & MCFG_DIR) {
			can_msgobj_set_fl(obj,TX_REQUEST);
			
			/* calls i82527_irq_write_handler synchronized with other invocations */
			if(i82527_irq_sync_activities(chip, obj)<=0){
				/* The interrupt has to be cleared anyway */
				canobj_write_reg(chip,obj,(MVAL_UNC|TXIE_UNC|RXIE_UNC|INTPD_RES),iMSGCTL0);

				/* 
				 * Rerun for case, that parallel activity on SMP or fully-preemptive
				 * kernel result in preparation and finished sending of message
				 * between above if and canobj_write_reg.
				 */
				i82527_irq_sync_activities(chip, obj);
			}
		}
		else { 

			i82527_irq_read_handler(chip, obj, object); 
		}
		
	} while((irq_register=i82527_seg_read_reg(chip, iIRQ)) != 0);

	return CANCHIP_IRQ_HANDLED;
}

void i82527_irq_rtr_handler(struct canchip_t *chip, struct msgobj_t *obj,
			    struct rtr_id *rtr_search, unsigned long message_id)
{
	short int i=0;

	canobj_write_reg(chip,obj,(MVAL_RES|TXIE_RES|RXIE_RES|INTPD_RES),iMSGCTL0);
	canobj_write_reg(chip,obj,(RMPD_RES|TXRQ_RES|MLST_RES|NEWD_RES),iMSGCTL1);
	
	can_spin_lock(&hardware_p->rtr_lock);

	rtr_search->rtr_message->id=message_id;
	rtr_search->rtr_message->length=(canobj_read_reg(chip,obj,iMSGCFG) & 0xf0)>>4;
	for (i=0; i<rtr_search->rtr_message->length; i++)
		rtr_search->rtr_message->data[i]=canobj_read_reg(chip,obj,iMSGDAT0+i);
	
	can_spin_unlock(&hardware_p->rtr_lock);

	if (waitqueue_active(&rtr_search->rtr_wq))
		wake_up(&rtr_search->rtr_wq);
}

/**
 * i82527_wakeup_tx: - wakeups TX processing
 * @chip: pointer to chip state structure
 * @obj: pointer to message object structure
 *
 * Function is responsible for initiating message transmition.
 * It is responsible for clearing of object TX_REQUEST flag
 *
 * Return Value: negative value reports error.
 * File: src/i82527.c
 */
int i82527_wakeup_tx(struct canchip_t *chip, struct msgobj_t *obj)
{
	can_preempt_disable();
	
	can_msgobj_set_fl(obj,TX_REQUEST);

	/* calls i82527_irq_write_handler synchronized with other invocations
	  from kernel and IRQ context */
	i82527_irq_sync_activities(chip, obj);

	can_preempt_enable();
	return 0;
}

int i82527_filtch_rq(struct canchip_t *chip, struct msgobj_t *obj)
{
	can_preempt_disable();
	
	can_msgobj_set_fl(obj,FILTCH_REQUEST);

	/* setups filter synchronized with other invocations from kernel and IRQ context */
	i82527_irq_sync_activities(chip, obj);

	can_preempt_enable();
	return 0;
}

int i82527_register(struct chipspecops_t *chipspecops)
{
	chipspecops->chip_config = i82527_chip_config;
	chipspecops->baud_rate = i82527_baud_rate;
	chipspecops->standard_mask = i82527_standard_mask;
	chipspecops->extended_mask = i82527_extended_mask;
	chipspecops->message15_mask = i82527_message15_mask;
	chipspecops->clear_objects = i82527_clear_objects;
	chipspecops->config_irqs = i82527_config_irqs;
	chipspecops->pre_read_config = i82527_pre_read_config;
	chipspecops->pre_write_config = i82527_pre_write_config;
	chipspecops->send_msg = i82527_send_msg;
	chipspecops->check_tx_stat = i82527_check_tx_stat;
	chipspecops->wakeup_tx = i82527_wakeup_tx;
	chipspecops->filtch_rq = i82527_filtch_rq;
	chipspecops->remote_request = i82527_remote_request;
	chipspecops->enable_configuration = i82527_enable_configuration;
	chipspecops->disable_configuration = i82527_disable_configuration;
	chipspecops->set_btregs = i82527_set_btregs;
	chipspecops->attach_to_chip = i82527_attach_to_chip;
	chipspecops->release_chip = i82527_release_chip;
	chipspecops->start_chip = i82527_start_chip;
	chipspecops->stop_chip = i82527_stop_chip;
	chipspecops->irq_handler = i82527_irq_handler;
	chipspecops->irq_accept = NULL;
	return 0;
}

int i82527_fill_chipspecops(struct canchip_t *chip)
{
	chip->chip_type="i82527";
	chip->max_objects=15;
	i82527_register(chip->chipspecops);
	return 0;
}
