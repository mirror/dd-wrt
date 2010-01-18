/* c_can_irq.c - Hynix HMS30c7202 ARM IRQ handling code
 * Linux CAN-bus device driver.
 * Written by Sebastian Stolzenberg email:stolzi@sebastian-stolzenberg.de
 * Based on code from Arnaud Westenberg email:arnaud@wanadoo.nl
 * and Ake Hedman, eurosource, akhe@eurosource.se
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/c_can.h"

union c_can_data {
	unsigned short wdata[4];
	unsigned char bdata[8];
};

// prototypes
inline void c_can_irq_read_handler(struct canchip_t *pchip, int idxobj,
				   u32 msgid);

inline void c_can_irq_write_handler(struct canchip_t *pchip, int idxobj);

void c_can_irq_rtr_handler(struct canchip_t *pchip, int idxobj, u32 msgid);

u16 readMaskCM = IFXCM_ARB | IFXCM_CNTRL | IFXCM_CLRINTPND
    | IFXCM_TRND | IFXCM_DA | IFXCM_DB;

u16 msgLstReadMaskCM = IFXCM_CNTRL;
u16 msgLstWriteMaskCM = IFXCM_CNTRL | IFXCM_WRRD;

///////////////////////////////////////////////////////////////////////////////
// c_can_irq_write_handler
//
// Send a message from the output fifo ( if any ).
//

inline void c_can_irq_write_handler(struct canchip_t *pchip, int idxobj)
{
	int cmd;
	struct msgobj_t *pmsgobj = pchip->msgobj[idxobj];

	DEBUGMSG("(c%dm%d)calling c_can_irq_write_handler(...)\n",
		 pchip->chip_idx, pmsgobj->object);

	if (pmsgobj->tx_slot) {
		/* Do local transmitted message distribution if enabled */
		if (processlocal) {
			/* fill CAN message timestamp */
			can_filltimestamp(&pmsgobj->tx_slot->msg.timestamp);

			pmsgobj->tx_slot->msg.flags |= MSG_LOCAL;
			canque_filter_msg2edges(pmsgobj->qends,
						&pmsgobj->tx_slot->msg);
		}
		/* Free transmitted slot */
		canque_free_outslot(pmsgobj->qends, pmsgobj->tx_qedge,
				    pmsgobj->tx_slot);
		pmsgobj->tx_slot = NULL;
	}
	// Get ready to send next message
	spin_lock(&c_can_spwlock);

	cmd =
	    canque_test_outslot(pmsgobj->qends, &pmsgobj->tx_qedge,
				&pmsgobj->tx_slot);
	if (cmd < 0) {
		DEBUGMSG("(c%dm%d)Nothin to write\n",
			 pchip->chip_idx, pmsgobj->object);
		spin_unlock(&c_can_spwlock);
		return;
	}
	// Send the message
	if (pchip->chipspecops->
	    send_msg(pchip, pmsgobj, &pmsgobj->tx_slot->msg)) {
		pmsgobj->ret = -1;
		canque_notify_inends(pmsgobj->tx_qedge,
				     CANQUEUE_NOTIFY_ERRTX_SEND);
		canque_free_outslot(pmsgobj->qends, pmsgobj->tx_qedge,
				    pmsgobj->tx_slot);
		pmsgobj->tx_slot = NULL;
		spin_unlock(&c_can_spwlock);
		DEBUGMSG("(c%dm%d)c_can_irq_handler: Unable to send message\n",
			 pchip->chip_idx, pmsgobj->object);
		return;
	} else {
		// Another message sent
#ifdef CAN_WITH_STATISTICS
		pchip->stat.cntTxPkt++;
		pchip->stat.cntTxData += pmsgobj->tx_slot->length;
#endif /*CAN_WITH_STATISTICS */
	}
	spin_unlock(&c_can_spwlock);

	// Wake up any waiting writer
	return;
}

///////////////////////////////////////////////////////////////////////////////
// c_can_irq_read_handler
//
// Message received form the line. Write it in the input fifo->
//

inline void c_can_irq_read_handler(struct canchip_t *pchip,
				   int idxobj, u32 msgid)
{
	int i = 0;
	u16 bDataAvail = 1;
	u16 msgCntlReg = 0;
	union c_can_data readData;
	struct msgobj_t *pmsgobj = pchip->msgobj[idxobj];

	DEBUGMSG("(c%dm%d)calling c_can_irq_read_handler(...)\n",
		 pchip->chip_idx, pmsgobj->object);

	while (bDataAvail) {

#ifdef CAN_WITH_STATISTICS
		pchip->stat.cntRxFifoOvr++;
#endif /*CAN_WITH_STATISTICS */
		// Message length
		msgCntlReg = c_can_read_reg_w(pchip, CCIF1DMC);

		pmsgobj->rx_msg.length = msgCntlReg & 0x000F;

		// Message id
		pmsgobj->rx_msg.id = (u32) msgid;

		// Fetch message bytes
		if (pmsgobj->rx_msg.length > 0)
			readData.wdata[0] = c_can_read_reg_w(pchip, CCIF1DA1);
		if (pmsgobj->rx_msg.length > 2)
			readData.wdata[1] = c_can_read_reg_w(pchip, CCIF1DA2);
		if (pmsgobj->rx_msg.length > 4)
			readData.wdata[2] = c_can_read_reg_w(pchip, CCIF1DB1);
		if (pmsgobj->rx_msg.length > 6)
			readData.wdata[3] = c_can_read_reg_w(pchip, CCIF1DB2);

		for (i = 0; i < pmsgobj->rx_msg.length; i++) {
			pmsgobj->rx_msg.data[i] = readData.bdata[i];
		}
		DEBUGMSG("(c%dm%d)Received Message:\n",
			 pchip->chip_idx, pmsgobj->object);
		DEBUGMSG(" id = %ld\n", pmsgobj->rx_msg.id);
		DEBUGMSG(" length = %d\n", pmsgobj->rx_msg.length);
		for (i = 0; i < pmsgobj->rx_msg.length; i++)
			DEBUGMSG(" data[%d] = 0x%.2x\n", i,
				 pmsgobj->rx_msg.data[i]);

		/* fill CAN message timestamp */
		can_filltimestamp(&pmsgobj->rx_msg.timestamp);

		canque_filter_msg2edges(pmsgobj->qends, &pmsgobj->rx_msg);

#ifdef CAN_WITH_STATISTICS
		// Another received packet
		pchip->stat.cntRxPkt++;

		// Add databytes read to statistics block
		pchip->stat.cntRxData += pmsgobj->rx_msg.length;
#endif /*CAN_WITH_STATISTICS */

		// Check if new data arrived
		if (c_can_if1_busycheck(pchip)) ;
		c_can_write_reg_w(pchip, readMaskCM, CCIF1CM);
		c_can_write_reg_w(pchip, idxobj + 1, CCIF1CR);
		if (c_can_if1_busycheck(pchip)) ;
		if (!((bDataAvail = c_can_read_reg_w(pchip, CCIF1DMC)) &
		      IFXMC_NEWDAT)) {
			break;
		}

		if (bDataAvail & IFXMC_MSGLST) {
			CANMSG("(c%dm%d)c-can fifo full: Message lost!\n",
			       pchip->chip_idx, pmsgobj->object);
		}

	}
	// while
}

///////////////////////////////////////////////////////////////////////////////
// c_can_irq_update_filter
//
// update acceptance filter for given object.
//

void c_can_irq_update_filter(struct canchip_t *pchip, struct msgobj_t *obj)
{
	struct canfilt_t filt;

	if(canqueue_ends_filt_conjuction(obj->qends, &filt)) {
		obj->rx_preconfig_id=filt.id;

		if (filt.flags&MSG_EXT)
			can_msgobj_set_fl(obj,RX_MODE_EXT);
		else
			can_msgobj_clear_fl(obj,RX_MODE_EXT);

		c_can_mask(obj, filt.mask, 0);

		c_can_pre_read_config(pchip, obj);

		CANMSG("c_can_irq_update_filter: obj #%d\n",obj->object);
	}
}

///////////////////////////////////////////////////////////////////////////////
// c_can_irq_sync_activities
//
// ensure, that not requests for object activities are serialized.
//

void c_can_irq_sync_activities(struct canchip_t *pchip, struct msgobj_t *obj)
{
	while (!can_msgobj_test_and_set_fl(obj, TX_LOCK)) {

		if(can_msgobj_test_and_clear_fl(obj,TX_REQUEST)) {
			int cctreqx;
			int idxobj = obj->object-1;
			
			if(idxobj<0) {
				DEBUGMSG("c_can_irq_sync_activities wrong idxobj\n");
				break;
			}

			if(idxobj < 16)
				cctreqx = c_can_read_reg_w(pchip, CCTREQ1);
			else
				cctreqx = c_can_read_reg_w(pchip, CCTREQ2);

			if (!(cctreqx & (1 << (idxobj & 0xf)))) {
				can_msgobj_clear_fl(obj, TX_REQUEST);
				c_can_irq_write_handler(pchip, idxobj);
			}
		}

		if(!obj->tx_slot) {
			if(can_msgobj_test_and_clear_fl(obj,FILTCH_REQUEST)) {
				c_can_irq_update_filter(pchip, obj);
			}
		}

		can_msgobj_clear_fl(obj, TX_LOCK);

		mb();

		if (can_msgobj_test_fl(obj, TX_REQUEST))
			continue;
		if (can_msgobj_test_fl(obj, FILTCH_REQUEST) && !obj->tx_slot)
			continue;
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
// c_can_irq_handler
//

int c_can_irq_handler(int irq, struct canchip_t *pchip)
{
	struct rtr_id *rtr_search = hardware_p->rtr_queue;
	u16 chip_status;
	int id0 = 0, id1 = 0;
	u16 errcount = 0;
	u16 irqreg = 0;
	u32 msgid = 0;
	u16 tempCntlReg = 0;

	irqreg = c_can_read_reg_w(pchip, CCINTR);

	if (!irqreg) {
		DEBUGMSG("\n(c%d)IRQ handler: addr=%.8lx spurious interrupt\n",
			 pchip->chip_idx,
			 (long)(pchip-> /*v */ chip_base_addr /* + CCSR */ ));
		return CANCHIP_IRQ_NONE;
	}

	DEBUGMSG("\n(c%d)IRQ handler: addr=%.8lx irqreg=0x%.4x\n",
		 pchip->chip_idx,
		 (long)(pchip-> /*v */ chip_base_addr /* + CCSR */ ),
		 irqreg);

#ifdef REGDUMP
	c_can_registerdump(pchip);
#endif

	while (irqreg) {
		// Handle change in status register

		if (irqreg == INT_STAT) {
			chip_status = c_can_read_reg_w(pchip, CCSR);
			DEBUGMSG("(c%d)Status register: 0x%x\n",
				 pchip->chip_idx, chip_status);

			if (chip_status & SR_EWARN) {
				// There is an abnormal # of errors
#ifdef CAN_WITH_STATISTICS
				pchip->stat.cntWarnings++;
#endif /*CAN_WITH_STATISTICS */
				errcount = c_can_read_reg_w(pchip, CCEC);
				DEBUGMSG
				    ("(c%d)stat: c_can_irq_handler: Abnormal number of Errors Warning\n"
				     "       txErr=%d, rxErr=%d\n",
				     pchip->chip_idx, (errcount & 0x00ff),
				     ((errcount & 0x7f00) >> 8));

				/*
				   // this code deactivates the chip if the transmiterrorcounter grew above 127
				   if ((pchip->stat.cntWarnings > 100) && ((errcount&0x00ff) > 127))
				   {
				   CANMSG("(c%d)to much Errornumber warnings (>100), deactivating chip",
				   pchip->chip_idx);
				   pchip->config_irqs(pchip, 0);
				   pchip->enable_configuration(pchip);
				   pchip->clear_objects(pchip);
				   pchip->flags &= ~CHANNEL_CONFIGURED;
				   return;
				   } */
			}

			if (chip_status & SR_EPASS) {
				// There is an abnormal # of errors
#ifdef CAN_WITH_STATISTICS
				pchip->stat.cntErrPassive++;
#endif /*CAN_WITH_STATISTICS */
				DEBUGMSG
				    ("(c%d)stat: c_can_irq_handler: Chip entering Error Passive Mode\n",
				     pchip->chip_idx);
			}

			if (chip_status & SR_BOFF) {
				// We have a bus off condition
#ifdef CAN_WITH_STATISTICS
				pchip->stat.cntBusOff++;
#endif /*CAN_WITH_STATISTICS */
				//pchip->fifo->tx_in_progress = 0;
				//reset init bit
				CANMSG
				    ("(c%d)stat: c_can_irq_handler: Bus Off\n",
				     pchip->chip_idx);
				/*if (pchip->stat.cntBusOff > 100)
				   {
				   CANMSG("(c%d)to much busoff warnings (>100), deactivating chip",
				   pchip->chip_idx);
				   pchip->config_irqs(pchip, 0);
				   pchip->enable_configuration(pchip);
				   pchip->clear_objects(pchip);
				   pchip->flags &= ~CHANNEL_CONFIGURED;
				   return;
				   }
				   else */
				CANMSG("(c%d)try to reconnect",
				       pchip->chip_idx);
				pchip->chipspecops->
				    disable_configuration(pchip);
			}

			if (chip_status & SR_TXOK) {
				DEBUGMSG
				    ("(c%d)stat: Transmitted a Message successfully\n",
				     pchip->chip_idx);
				c_can_write_reg_w(pchip, chip_status & ~SR_TXOK,
						  CCSR);
			}

			if (chip_status & SR_RXOK) {
				DEBUGMSG
				    ("(c%d)stat: Received a Message successfully\n",
				     pchip->chip_idx);
				c_can_write_reg_w(pchip, chip_status & ~SR_RXOK,
						  CCSR);
			}
#ifdef CAN_WITH_STATISTICS
			// Errors to statistics
			switch (chip_status & 0x07) {
			case SRLEC_NE:	// No error
				break;
			case SRLEC_SE:	// Stuff error
				pchip->stat.cntStuffErr++;
				break;
			case SRLEC_FE:	// Form error
				pchip->stat.cntFormErr++;
				break;
			case SRLEC_AE:	// Ack error
				pchip->stat.cntAckErr++;
				break;
			case SRLEC_B1:	// Bit 1 error
				pchip->stat.cntBit1Err++;
				break;
			case SRLEC_B0:	// Bit 0 error
				pchip->stat.cntBit0Err++;
				break;
			case SRLEC_CR:	// CRC error
				pchip->stat.cntCrcErr++;
				break;
			case 7:	// unused
				break;
			}
#endif /*CAN_WITH_STATISTICS */
			//return; // continue?
		} else {
			if (irqreg >= 1 && irqreg <= 32) {
				struct msgobj_t *pmsgobj;
				int idxobj;

				//get id
				idxobj = irqreg - 1;
				pmsgobj = pchip->msgobj[idxobj];

				//DEBUGMSG( "Interrupt handler: addr=%lx devid=%lx irqreq=%x status=0x%x\n",
				//            (unsigned long)pchip->vbase_addr + iIRQ,
				//      (unsigned long)dev_id,
				//      irqreg,
				//      statreg );
				//
				spin_lock(&c_can_if1lock);

				//Message Lost Check
				if (c_can_if1_busycheck(pchip)) ;	/*?????????? */
				c_can_write_reg_w(pchip, msgLstReadMaskCM,
						  CCIF1CM);
				c_can_write_reg_w(pchip, idxobj + 1, CCIF1CR);

				if (c_can_if1_busycheck(pchip)) ;	/*?????????? */
				tempCntlReg = c_can_read_reg_w(pchip, CCIF1DMC);

				if (tempCntlReg & IFXMC_MSGLST) {
					CANMSG("(c%dm%d)Chip lost a message\n",
					       pchip->chip_idx,
					       pmsgobj->object);
#ifdef CAN_WITH_STATISTICS
					pchip->stat.cntMsgLst++;
#endif /*CAN_WITH_STATISTICS */

					//Reset Message Lost Bit
					tempCntlReg =
					    tempCntlReg & (~IFXMC_MSGLST);
					c_can_write_reg_w(pchip, tempCntlReg,
							  CCIF1DMC);
					c_can_write_reg_w(pchip,
							  msgLstWriteMaskCM,
							  CCIF1CM);
					c_can_write_reg_w(pchip, idxobj + 1,
							  CCIF1CR);
				}
				//transfer Message Object to IF1 Buffer
				if (c_can_if1_busycheck(pchip)) ;

				c_can_write_reg_w(pchip, readMaskCM, CCIF1CM);
				c_can_write_reg_w(pchip, idxobj + 1, CCIF1CR);

				if (c_can_if1_busycheck(pchip)) ;
				if (c_can_read_reg_w(pchip, CCIF1A2) &
				    IFXARB2_DIR) {
					DEBUGMSG("c_can_irq_write_handler idxobj=%d, msgid=%d\n",idxobj,msgid);
					spin_unlock(&c_can_if1lock);
					c_can_irq_write_handler(pchip, idxobj);

					if(!pmsgobj->tx_slot){
						if(can_msgobj_test_and_clear_fl(pmsgobj, FILTCH_REQUEST)) {
							c_can_irq_update_filter(pchip, pmsgobj);
						}
					}
				} else {
					if (can_msgobj_test_fl
					    (pmsgobj, RX_MODE_EXT)) {
						id0 =
						    c_can_read_reg_w(pchip,
								     CCIF1A1);
						id1 =
						    (c_can_read_reg_w
						     (pchip,
						      CCIF1A2) & 0x1FFF) << 16;
						msgid = id0 | id1;
					} else {
						msgid =
						    ((c_can_read_reg_w
						      (pchip,
						       CCIF1A2) & 0x1FFC) >> 2)
						    & 0x7FF;
					}
					spin_unlock(&c_can_if1lock);

					spin_lock(&hardware_p->rtr_lock);
					while (rtr_search != NULL) {
						if (rtr_search->id == msgid) {
							break;
						}
						rtr_search = rtr_search->next;
					}
					spin_unlock(&hardware_p->rtr_lock);

					spin_lock(&c_can_if1lock);

					//transfer Message Object to IF1 Buffer
					if (c_can_if1_busycheck(pchip)) ;
					c_can_write_reg_w(pchip, readMaskCM,
							  CCIF1CM);
					c_can_write_reg_w(pchip, idxobj + 1,
							  CCIF1CR);

					if (c_can_if1_busycheck(pchip)) ;

					if ((rtr_search != NULL)
					    && (rtr_search->id == msgid)) {
						c_can_irq_rtr_handler(pchip,
								      idxobj,
								      msgid);
					} else {
						c_can_irq_read_handler(pchip,
								       idxobj,
								       msgid);
					}
					spin_unlock(&c_can_if1lock);

				}
				//else
			}
			//if (irqreg >= 1 && irqreg <= 32)
		}
		// Get irq status again
		irqreg = c_can_read_reg_w(pchip, CCINTR);
	}
	return CANCHIP_IRQ_HANDLED;
}

///////////////////////////////////////////////////////////////////////////////
// c_can_irq_rtr_handler
//

void c_can_irq_rtr_handler(struct canchip_t *pchip, int idxobj, u32 msgid)
{
	short int i = 0;
	struct rtr_id *prtr_search = hardware_p->rtr_queue;
	union c_can_data rtrData;

	spin_lock(&hardware_p->rtr_lock);

	prtr_search->rtr_message->id = msgid;
	prtr_search->rtr_message->length =
	    (c_can_read_reg_w(pchip, CCIF1DMC) & 0x000f);

	// Fetch message bytes
	if (prtr_search->rtr_message->length > 0)
		rtrData.wdata[0] = c_can_read_reg_w(pchip, CCIF1DA1);
	if (prtr_search->rtr_message->length > 2)
		rtrData.wdata[1] = c_can_read_reg_w(pchip, CCIF1DA2);
	if (prtr_search->rtr_message->length > 4)
		rtrData.wdata[2] = c_can_read_reg_w(pchip, CCIF1DB1);
	if (prtr_search->rtr_message->length > 6)
		rtrData.wdata[3] = c_can_read_reg_w(pchip, CCIF1DB2);

	for (i = 0; i < prtr_search->rtr_message->length; i++) {
		prtr_search->rtr_message->data[i] = rtrData.bdata[i];
	}

	spin_unlock(&hardware_p->rtr_lock);
	wake_up_interruptible(&prtr_search->rtr_wq);
	return;
}
