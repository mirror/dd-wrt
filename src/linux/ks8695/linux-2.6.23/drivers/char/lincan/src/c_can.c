/* c_can.c - Hynix HMS30c7202 ARM generic C_CAN module handling
 * Linux CAN-bus device driver.
 * Written by Sebastian Stolzenberg email:stolzi@sebastian-stolzenberg.de
 * Based on code from Arnaud Westenberg email:arnaud@wanadoo.nl
 * and Ake Hedman, eurosource, akhe@eurosource.se
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#define __NO_VERSION__

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/c_can.h"

extern int stdmask;
extern int extmask;

CAN_DEFINE_SPINLOCK(c_can_spwlock);	// Spin lock for write operations
CAN_DEFINE_SPINLOCK(c_can_sprlock);	// Spin lock for read operations
CAN_DEFINE_SPINLOCK(c_can_if1lock);	// spin lock for the if1 register
CAN_DEFINE_SPINLOCK(c_can_if2lock);	// spin lcok for the if2 register

/**
 * c_can_enable_configuration - enable chip configuration mode
 * @pchip: pointer to chip state structure
 */
int c_can_enable_configuration(struct canchip_t *pchip)
{
	int i = 0;
	u16 flags;
	DEBUGMSG("(c%d)calling c_can_enable_configuration(...)\n",
		 pchip->chip_idx);
/*
   DEBUGMSG("Trying disable_irq(...) : ");
   //disable IRQ
	disable_irq(chip->chip_irq);
*/
	//read Control Register
	flags = c_can_read_reg_w(pchip, CCCR);
	//set Init-Bit in the Control Register (10 tries)
	while ((!(flags & CR_INIT)) && (i <= 10)) {
		c_can_write_reg_w(pchip, flags | CR_INIT, CCCR);
		udelay(1000);
		i++;
		flags = c_can_read_reg_w(pchip, CCCR);
	}
	if (i >= 10) {
		CANMSG("Reset error\n");
		//enable_irq(chip->chip_irq);
		return -ENODEV;
	}

	DEBUGMSG("-> ok\n");
	return 0;
}

///////////////////////////////////////////////////////////////////////
int c_can_disable_configuration(struct canchip_t *pchip)
{
	int i = 0;
	u16 flags;

	DEBUGMSG("(c%d)calling c_can_disable_configuration(...)\n",
		 pchip->chip_idx);
	//read Control Register
	flags = c_can_read_reg_w(pchip, CCCR);

	//reset Init-Bit in the Control Register (10 tries)
	while ((flags & CR_INIT) && (i <= 10)) {
		c_can_write_reg_w(pchip, flags & ~CR_INIT, CCCR);
		udelay(1000);	//100 microseconds
		i++;
		flags = c_can_read_reg_w(pchip, CCCR);
	}
	if (i >= 10) {
		CANMSG("Error leaving reset status\n");
		return -ENODEV;
	}
	//enable IRQ
	//enable_irq(chip->chip_irq);
	DEBUGMSG("-> ok\n");
	return 0;
}

///////////////////////////////////////////////////////////////////////
int c_can_chip_config(struct canchip_t *pchip)
{

	DEBUGMSG("(c%d)calling c_can_chip_config(...)\n", pchip->chip_idx);
	// Validate pointer
	if (NULL == pchip)
		return -1;

	if (pchip->baudrate == 0)
		pchip->baudrate = 1000000;

	if (c_can_baud_rate
	    (pchip, pchip->baudrate, pchip->clock, 0, 75, 0)) {
		CANMSG("Error configuring baud rate\n");
		return -ENODEV;
	}
	/*if (extended){
	   if (c_can_extended_mask(pchip,0x0000000,extmask)) {
	   CANMSG("Error configuring extended mask\n");
	   return -ENODEV;
	   }
	   }else{
	   if (c_can_standard_mask(pchip,0x0000,stdmask)) {
	   CANMSG("Error configuring standard mask\n");
	   return -ENODEV;
	   }
	   } */
	if (c_can_clear_objects(pchip)) {
		CANMSG("Error clearing message objects\n");
		return -ENODEV;
	}
	if (c_can_config_irqs(pchip, CR_MIE | CR_SIE | CR_EIE)) {
		CANMSG("Error configuring interrupts\n");
		return -ENODEV;
	}

	DEBUGMSG("-> Configured successfully\n");

#ifdef REGDUMP
	c_can_registerdump(pchip);
#endif

	return 0;
}

///////////////////////////////////////////////////////////////////////
/*
 * Checks if the Busy-Bit in the IF1-Command-Request Register is set
 */
int c_can_if1_busycheck(struct canchip_t *pchip)
{

	int i = 0;
	unsigned short comreg = 0;

	comreg = c_can_read_reg_w(pchip, CCIF1CR);
	while ((comreg & IFXCR_BUSY) && (i <= 100)) {
		udelay(1);	//1 microseconds
		i++;
		comreg = c_can_read_reg_w(pchip, CCIF1CR);
	}
	if (i >= 10) {
		CANMSG("Error Busy-Bit stays set\n");
		return -ENODEV;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////
/*
 * Checks if the Busy-Bit in the IF2-Command-Request Register is set
 */
int c_can_if2_busycheck(struct canchip_t *pchip)
{

	int i = 0;
	unsigned short comreg = 0;

	comreg = c_can_read_reg_w(pchip, CCIF2CR);
	while ((comreg & IFXCR_BUSY) && (i <= 100)) {
		udelay(1);	//1 microseconds
		i++;
		comreg = c_can_read_reg_w(pchip, CCIF2CR);
	}
	if (i >= 10) {
		CANMSG("Error Busy-Bit stays set\n");
		return -ENODEV;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////
/*
 * Though the C-CAN Chip can handle one mask for each Message Object, this Method defines
 * one mask for all MOs. That means every MO gets the same mask.
 */

/* Set communication parameters.
 * param rate baud rate in Hz
 * param clock frequency of C-CAN clock in Hz
 * param sjw synchronization jump width (0-3) prescaled clock cycles
 * param sampl_pt sample point in % (0-100) sets (TSEG1+2)/(TSEG1+TSEG2+3) ratio
 * param flags fields BTR1_SAM, OCMODE, OCPOL, OCTP, OCTN, CLK_OFF, CBP
 */
int c_can_baud_rate(struct canchip_t *pchip, int rate, int clock,
		    int sjw, int sampl_pt, int flags)
{
	int best_error = 1000000000, error;
	int best_tseg = 0, best_brp = 0, best_rate = 0, brp = 0;
	int tseg = 0, tseg1 = 0, tseg2 = 0;

	unsigned short tempCR = 0;

	DEBUGMSG("(c%d)calling c_can_baud_rate(...)\n", pchip->chip_idx);

	if (c_can_enable_configuration(pchip))
		return -ENODEV;

	/* tseg even = round down, odd = round up */
	for (tseg = (0 + 0 + 2) * 2;
	     tseg <= (MAX_TSEG2 + MAX_TSEG1 + 2) * 2 + 1; tseg++) {
		brp = clock / ((1 + tseg / 2) * rate) + tseg % 2;
		if (brp == 0 || brp > 64)
			continue;
		error = rate - clock / (brp * (1 + tseg / 2));
		if (error < 0)
			error = -error;
		if (error <= best_error) {
			best_error = error;
			best_tseg = tseg / 2;
			best_brp = brp - 1;
			best_rate = clock / (brp * (1 + tseg / 2));
		}
	}
	if (best_error && (rate / best_error < 10)) {
		CANMSG("baud rate %d is not possible with %d Hz clock\n",
		       rate, clock);
		CANMSG("%d bps. brp=%d, best_tseg=%d, tseg1=%d, tseg2=%d\n",
		       best_rate, best_brp, best_tseg, tseg1, tseg2);
		return -EINVAL;
	}
	tseg2 = best_tseg - (sampl_pt * (best_tseg + 1)) / 100;
	if (tseg2 < 0)
		tseg2 = 0;
	if (tseg2 > MAX_TSEG2)
		tseg2 = MAX_TSEG2;
	tseg1 = best_tseg - tseg2 - 2;
	if (tseg1 > MAX_TSEG1) {
		tseg1 = MAX_TSEG1;
		tseg2 = best_tseg - tseg1 - 2;
	}

	DEBUGMSG("-> Setting %d bps.\n", best_rate);
	DEBUGMSG("->brp=%d, best_tseg=%d, tseg1=%d, tseg2=%d, sampl_pt=%d\n",
		 best_brp, best_tseg, tseg1, tseg2,
		 (100 * (best_tseg - tseg2) / (best_tseg + 1)));

	//read Control Register
	tempCR = c_can_read_reg_w(pchip, CCCR);
	//Configuration Change Enable
	c_can_write_reg_w(pchip, tempCR | CR_CCE, CCCR);
	c_can_write_reg_w(pchip,
			  ((unsigned short)tseg2) << 12 | ((unsigned short)
							   tseg1) << 8 |
			  (unsigned short)sjw << 6 | (unsigned short)best_brp,
			  CCBT);

	if (c_can_disable_configuration(pchip))
		return -ENODEV;

	return 0;
}

///////////////////////////////////////////////////////////////////////
int c_can_mask(struct msgobj_t *pmsgobj, u32 mask, u16 usedirbit)
{
	unsigned short tempreg = 0;
	unsigned short readMaskCM;
	unsigned short writeMaskCM;

	DEBUGMSG("(c%dm%d)calling c_can_mask(...)\n",
		 pmsgobj->hostchip->chip_idx, pmsgobj->object);

	readMaskCM = IFXCM_CNTRL | IFXCM_ARB | IFXCM_MASK;
	writeMaskCM = IFXCM_CNTRL | IFXCM_ARB | IFXCM_MASK | IFXCM_WRRD;

	spin_lock(&c_can_if1lock);

	//load Message Object in IF1
	if (c_can_if1_busycheck(pmsgobj->hostchip))
		return -ENODEV;
	c_can_write_reg_w(pmsgobj->hostchip, readMaskCM, CCIF1CM);
	c_can_write_reg_w(pmsgobj->hostchip, pmsgobj->object, CCIF1CR);

	//setting Message Valid Bit to zero
	if (c_can_if1_busycheck(pmsgobj->hostchip))
		return -ENODEV;
	tempreg = c_can_read_reg_w(pmsgobj->hostchip, CCIF1A2);
	c_can_write_reg_w(pmsgobj->hostchip, tempreg & ~(IFXARB2_MVAL),
			  CCIF1A2);
	c_can_write_reg_w(pmsgobj->hostchip, writeMaskCM, CCIF1CM);
	c_can_write_reg_w(pmsgobj->hostchip, pmsgobj->object, CCIF1CR);

	//setting UMask, MsgVal and Mask Register
	if (c_can_if1_busycheck(pmsgobj->hostchip))
		return -ENODEV;

	//set indication, that mask is used
	tempreg = c_can_read_reg_w(pmsgobj->hostchip, CCIF1DMC);
	c_can_write_reg_w(pmsgobj->hostchip, tempreg | IFXMC_UMASK,
		CCIF1DMC);

	//writing acceptance mask for extended or standart mode
	if (can_msgobj_test_fl(pmsgobj, RX_MODE_EXT)) {
		if (usedirbit)
			c_can_write_reg_w(pmsgobj->hostchip,
					  (mask >> 16 & 0x1FFF) | IFXMSK2_MXTD |
					  IFXMSK2_MDIR, CCIF1M2);
		else
			c_can_write_reg_w(pmsgobj->hostchip,
					  (mask >> 16 & 0x1FFF) | IFXMSK2_MXTD,
					  CCIF1M2);
		c_can_write_reg_w(pmsgobj->hostchip, (mask & 0xFFFF), CCIF1M1);
	} else {
		if (usedirbit)
			c_can_write_reg_w(pmsgobj->hostchip,
					  ((mask << 2) & 0x1FFC) | IFXMSK2_MDIR,
					  CCIF1M2);
		else
			c_can_write_reg_w(pmsgobj->hostchip,
					  ((mask << 2) & 0x1FFC), CCIF1M2);
		c_can_write_reg_w(pmsgobj->hostchip, 0, CCIF1M1);
	}

	//seting Message Valid Bit to one
	tempreg = c_can_read_reg_w(pmsgobj->hostchip, CCIF1A2);
	c_can_write_reg_w(pmsgobj->hostchip, tempreg | IFXARB2_MVAL, CCIF1A2);
	//write to chip
	c_can_write_reg_w(pmsgobj->hostchip, writeMaskCM, CCIF1CM);
	c_can_write_reg_w(pmsgobj->hostchip, pmsgobj->object, CCIF1CR);

	spin_unlock(&c_can_if1lock);

	DEBUGMSG("-> Setting acceptance mask to 0x%lx\n", (unsigned long)mask);

#ifdef REGDUMP
	c_can_registerdump(pmsgobj->hostchip);
#endif

	return 0;
}

///////////////////////////////////////////////////////////////////////
int c_can_use_mask(struct msgobj_t *pmsgobj, u16 useflag)
{
	unsigned short tempreg = 0;
	unsigned short readMaskCM;
	unsigned short writeMaskCM;

#ifdef DEBUG
	char *boolstring = "false";
	if (useflag)
		boolstring = "true";
#endif
	DEBUGMSG("(c%dm%d)calling c_can_use_mask(...)\n",
		 pmsgobj->hostchip->chip_idx, pmsgobj->object);

	readMaskCM = IFXCM_CNTRL | IFXCM_ARB;
	writeMaskCM = IFXCM_CNTRL | IFXCM_ARB | IFXCM_WRRD;;

	spin_lock(&c_can_if1lock);

	//load Message Object in IF1
	if (c_can_if1_busycheck(pmsgobj->hostchip))
		return -ENODEV;
	c_can_write_reg_w(pmsgobj->hostchip, readMaskCM, CCIF1CM);
	c_can_write_reg_w(pmsgobj->hostchip, pmsgobj->object, CCIF1CR);

	//setting Message Valid Bit to zero
	if (c_can_if1_busycheck(pmsgobj->hostchip))
		return -ENODEV;
	tempreg = c_can_read_reg_w(pmsgobj->hostchip, CCIF1A2);
	c_can_write_reg_w(pmsgobj->hostchip, tempreg & ~IFXARB2_MVAL, CCIF1A2);
	c_can_write_reg_w(pmsgobj->hostchip, writeMaskCM, CCIF1CM);
	c_can_write_reg_w(pmsgobj->hostchip, pmsgobj->object, CCIF1CR);

	//setting UMask bit
	if (c_can_if1_busycheck(pmsgobj->hostchip))
		return -ENODEV;
	if (useflag) {
		tempreg = c_can_read_reg_w(pmsgobj->hostchip, CCIF1DMC);
		c_can_write_reg_w(pmsgobj->hostchip, tempreg | IFXMC_UMASK,
				  CCIF1DMC);
	} else {
		tempreg = c_can_read_reg_w(pmsgobj->hostchip, CCIF1DMC);
		c_can_write_reg_w(pmsgobj->hostchip, tempreg & ~IFXMC_UMASK,
				  CCIF1DMC);
	}
	//seting Message Valid Bit to one
	tempreg = c_can_read_reg_w(pmsgobj->hostchip, CCIF1A2);
	c_can_write_reg_w(pmsgobj->hostchip, tempreg | IFXARB2_MVAL, CCIF1A2);
	//write to chip
	c_can_write_reg_w(pmsgobj->hostchip, writeMaskCM, CCIF1CM);
	c_can_write_reg_w(pmsgobj->hostchip, pmsgobj->object, CCIF1CR);

	spin_unlock(&c_can_if1lock);

#ifdef DEBUG
	DEBUGMSG("-> Setting umask bit to %s\n", boolstring);
#endif
#ifdef REGDUMP
	c_can_registerdump(pmsgobj->hostchip);
#endif

	return 0;
}

///////////////////////////////////////////////////////////////////////
int c_can_clear_objects(struct canchip_t *pchip)
{
	unsigned short i = 0;
	unsigned short tempreg = 0;

	unsigned short maskCM = IFXCM_ARB;

	DEBUGMSG("(c%d)calling c_can_clear_objects(...)\n", pchip->chip_idx);

	spin_lock(&c_can_if1lock);
	spin_lock(&c_can_if2lock);

	for (i = 0; i < 0x10; i++) {

		//loading Message Objects in IF1 and IF2
		if (c_can_if1_busycheck(pchip))
			return -ENODEV;
		c_can_write_reg_w(pchip, maskCM, CCIF1CM);
		c_can_write_reg_w(pchip, i, CCIF1CR);
		if (c_can_if2_busycheck(pchip))
			return -ENODEV;
		c_can_write_reg_w(pchip, maskCM, CCIF2CM);
		c_can_write_reg_w(pchip, i + 0x10, CCIF2CR);

		//setting Message Valid Bit to zero
		if (c_can_if1_busycheck(pchip))
			return -ENODEV;
		tempreg = c_can_read_reg_w(pchip, CCIF1A2);
		c_can_write_reg_w(pchip, tempreg & ~IFXARB2_MVAL, CCIF1A2);
		c_can_write_reg_w(pchip, i, CCIF1CR);
		if (c_can_if2_busycheck(pchip))
			return -ENODEV;
		tempreg = c_can_read_reg_w(pchip, CCIF2A2);
		c_can_write_reg_w(pchip, tempreg & ~IFXARB2_MVAL, CCIF2A2);
		c_can_write_reg_w(pchip, i + 0x10, CCIF2CR);
	}

	for (i = 0; i < pchip->max_objects; i++) {
		if (can_msgobj_test_fl(pchip->msgobj[i], OPENED)) {
			// In- and output buffer re-initialization
			canqueue_ends_flush_inlist(pchip->msgobj[i]->qends);
			canqueue_ends_flush_outlist(pchip->msgobj[i]->qends);

		}
	}

	spin_unlock(&c_can_if1lock);
	spin_unlock(&c_can_if2lock);

	DEBUGMSG("-> Message Objects reset\n");

	return 0;
}

///////////////////////////////////////////////////////////////////////
int c_can_config_irqs(struct canchip_t *pchip, u16 irqs)
{
	u16 tempreg;

	DEBUGMSG("(c%d)calling c_can_config_irqs(...)\n", pchip->chip_idx);

	tempreg = c_can_read_reg_w(pchip, CCCR);
	DEBUGMSG("-> CAN Control Register: 0x%4lx\n", (long)tempreg);
	c_can_write_reg_w(pchip, tempreg | (irqs & 0xe), CCCR);
	DEBUGMSG("-> Configured hardware interrupt delivery\n");
	return 0;
}

///////////////////////////////////////////////////////////////////////
int c_can_pre_read_config(struct canchip_t *pchip, struct msgobj_t *pmsgobj)
{
	unsigned short readMaskCM = IFXCM_CNTRL | IFXCM_ARB;
	unsigned short writeMaskCM = IFXCM_CNTRL | IFXCM_ARB | IFXCM_WRRD;
	unsigned short mcreg = 0;
	u32 id = pmsgobj->rx_preconfig_id;

	DEBUGMSG("(c%dm%d)calling c_can_pre_read_config(...)\n",
		 pmsgobj->hostchip->chip_idx, pmsgobj->object);

	spin_lock(&c_can_if1lock);

	if (c_can_if1_busycheck(pmsgobj->hostchip))
		goto error_enodev;

	//loading Message Object in IF1
	c_can_write_reg_w(pmsgobj->hostchip, readMaskCM, CCIF1CM);
	c_can_write_reg_w(pmsgobj->hostchip, pmsgobj->object, CCIF1CR);

	if (c_can_if1_busycheck(pmsgobj->hostchip))
		goto error_enodev;

	//setting Message Valid Bit to zero
	c_can_write_reg_w(pmsgobj->hostchip, 0, CCIF1A2);
	c_can_write_reg_w(pmsgobj->hostchip, writeMaskCM, CCIF1CM);
	c_can_write_reg_w(pmsgobj->hostchip, pmsgobj->object, CCIF1CR);

	/* Only access when the C_CAN controller is idle */
	if (c_can_if1_busycheck(pmsgobj->hostchip))
		goto error_enodev;

	//Configuring Message-Object
	mcreg = c_can_read_reg_w(pmsgobj->hostchip, CCIF1DMC);
	c_can_write_reg_w(pmsgobj->hostchip,
			  ((mcreg & IFXMC_UMASK) | IFXMC_EOB | IFXMC_RXIE),
			  CCIF1DMC);

	//writing arbitration mask for extended or standart mode
	if (can_msgobj_test_fl(pmsgobj, RX_MODE_EXT)) {
		c_can_write_reg_w(pmsgobj->hostchip,
				  IFXARB2_XTD | IFXARB2_MVAL | (id >> 16 &
								0x1FFF),
				  CCIF1A2);
		c_can_write_reg_w(pmsgobj->hostchip, id & 0xFFFF, CCIF1A1);
	} else {
		c_can_write_reg_w(pmsgobj->hostchip,
				  IFXARB2_MVAL | (id << 2 & 0x1FFC), CCIF1A2);
		//c_can_write_reg_w(pmsgobj->hostchip, 0, CCIF1A1);
	}
	c_can_write_reg_w(pmsgobj->hostchip, writeMaskCM, CCIF1CM);
	c_can_write_reg_w(pmsgobj->hostchip, pmsgobj->object, CCIF1CR);

	spin_unlock(&c_can_if1lock);

	DEBUGMSG("-> Receiving through message object %d with id=%d\n",
		 pmsgobj->object, id);
#ifdef REGDUMP
	c_can_registerdump(pmsgobj->hostchip);
#endif

	return 0;

      error_enodev:
	CANMSG("Timeout in c_can_if1_busycheck\n");
	spin_unlock(&c_can_if1lock);
	return -ENODEV;

}

///////////////////////////////////////////////////////////////////////
int c_can_pre_write_config(struct canchip_t *chip, struct msgobj_t *obj,
			   struct canmsg_t *msg)
{
	return 0;
}

 ///////////////////////////////////////////////////////////////////////
/*
 *Prepare the Chip to send specified Message over specified Messageobject
 *In this version the method also sends the message.
 */

int c_can_send_msg(struct canchip_t *pchip, struct msgobj_t *pmsgobj,
		   struct canmsg_t *pmsg)
{
	unsigned short readMaskCM =
	    IFXCM_CNTRL | IFXCM_ARB | IFXCM_DA | IFXCM_DB;
	unsigned short writeMaskCM =
	    IFXCM_CNTRL | IFXCM_ARB | IFXCM_DA | IFXCM_DB | IFXCM_WRRD;
	unsigned short writeSendMskCM =
	    IFXCM_CNTRL | IFXCM_ARB | IFXCM_DA | IFXCM_DB | IFXCM_WRRD |
	    IFXCM_TRND;
	unsigned short mcreg = 0;
	//unsigned short arbreg = 0;
	unsigned short dataA1 = 0;
	unsigned short dataA2 = 0;
	unsigned short dataB1 = 0;
	unsigned short dataB2 = 0;

	DEBUGMSG("(c%dm%d)calling c_can_send_msg(...)\n",
		 pmsgobj->hostchip->chip_idx, pmsgobj->object);

	spin_lock(&c_can_if2lock);

	can_msgobj_clear_fl(pmsgobj, RX_MODE);

	//loading Message Object in IF1
	if (c_can_if2_busycheck(pmsgobj->hostchip))
		return -ENODEV;
	c_can_write_reg_w(pmsgobj->hostchip, readMaskCM, CCIF2CM);
	c_can_write_reg_w(pmsgobj->hostchip, pmsgobj->object, CCIF2CR);
	//setting Message Valid Bit to zero
	if (c_can_if2_busycheck(pmsgobj->hostchip))
		return -ENODEV;
	c_can_write_reg_w(pmsgobj->hostchip, 0, CCIF2A2);
	c_can_write_reg_w(pmsgobj->hostchip, writeMaskCM, CCIF2CM);
	c_can_write_reg_w(pmsgobj->hostchip, pmsgobj->object, CCIF2CR);
	//Configuring MO
	if (c_can_if2_busycheck(pmsgobj->hostchip))
		return -ENODEV;
	mcreg = c_can_read_reg_w(pmsgobj->hostchip, CCIF2CM);
	//remote enable?
	//define Command Mask
	c_can_write_reg_w(pmsgobj->hostchip,
			  (mcreg & IFXMC_UMASK) | IFXMC_EOB | IFXMC_TXIE |
			  IFXMC_RMTEN | IFXMC_NEWDAT | IFXMC_TXRQST | (pmsg->
								       length &
								       0xF),
			  CCIF2DMC);
	//set Arbitration Bits
	if (can_msgobj_test_fl(pmsgobj, RX_MODE_EXT)) {
		c_can_write_reg_w(pmsgobj->hostchip, (u16) (pmsg->id), CCIF2A1);
		c_can_write_reg_w(pmsgobj->hostchip,
				  IFXARB2_XTD | IFXARB2_MVAL | IFXARB2_DIR |
				  ((u16) (pmsg->id >> 16) & 0x1FFF), CCIF2A2);
	} else {
		c_can_write_reg_w(pmsgobj->hostchip,
				  (IFXARB2_MVAL | IFXARB2_DIR |
				   ((u16) (pmsg->id << 2) & 0x1FFC)), CCIF2A2);
		c_can_write_reg_w(pmsgobj->hostchip, 0, CCIF1A1);
	}
	//write Data
	if (pmsg->length > 0) {
		dataA1 = pmsg->data[0] | (u16) pmsg->data[1] << 8;
		dataA2 = pmsg->data[2] | (u16) pmsg->data[3] << 8;
		dataB1 = pmsg->data[4] | (u16) pmsg->data[5] << 8;
		dataB2 = pmsg->data[6] | (u16) pmsg->data[7] << 8;

		c_can_write_reg_w(pmsgobj->hostchip, dataA1, CCIF2DA1);
		c_can_write_reg_w(pmsgobj->hostchip, dataA2, CCIF2DA2);
		c_can_write_reg_w(pmsgobj->hostchip, dataB1, CCIF2DB1);
		c_can_write_reg_w(pmsgobj->hostchip, dataB2, CCIF2DB2);
	}

	c_can_write_reg_w(pmsgobj->hostchip, writeSendMskCM, CCIF2CM);
	c_can_write_reg_w(pmsgobj->hostchip, pmsgobj->object, CCIF2CR);

	spin_unlock(&c_can_if2lock);

	DEBUGMSG("-> ok\n");
#ifdef REGDUMP
	c_can_registerdump(pmsgobj->hostchip);
#endif

	return 0;
}

//////////////////////////////////////////////////////////////////////
int c_can_remote_request(struct canchip_t *pchip, struct msgobj_t *pmsgobj)
{
	unsigned short readMaskCM = IFXCM_CNTRL;	// | IFXCM_ARB;
	//unsigned short writeMaskCM = IFXCM_CNTRL | IFXCM_ARB | IFXCM_WRRD;
	unsigned short mcreg = 0;

	DEBUGMSG("(c%dm%d)calling c_can_remote_request(...)\n",
		 pmsgobj->hostchip->chip_idx, pmsgobj->object);

	//Remote request is only available when the message object is in receiving mode
	if (!can_msgobj_test_fl(pmsgobj, RX_MODE)) {
		return 1;
	}

	spin_lock(&c_can_if1lock);

	//loading Message Object in IF1
	if (c_can_if1_busycheck(pmsgobj->hostchip))
		return -ENODEV;
	c_can_write_reg_w(pmsgobj->hostchip, readMaskCM, CCIF1CM);
	c_can_write_reg_w(pmsgobj->hostchip, pmsgobj->object, CCIF1CR);
	//setting Transmit-Request-Bit
	if (c_can_if1_busycheck(pmsgobj->hostchip))
		return -ENODEV;
	mcreg = c_can_read_reg_w(pmsgobj->hostchip, CCIF1DMC);
	c_can_write_reg_w(pmsgobj->hostchip, mcreg | IFXMC_TXRQST, CCIF1DMC);

	c_can_write_reg_w(pmsgobj->hostchip, pmsgobj->object, CCIF1CR);

	spin_unlock(&c_can_if1lock);

	DEBUGMSG("-> Sent remote request through message object %d\n",
		 pmsgobj->object);
#ifdef REGDUMP
	c_can_registerdump(pmsgobj->hostchip);
#endif

	return 0;
}

///////////////////////////////////////////////////////////////////////
int c_can_set_btregs(struct canchip_t *pchip, u16 btr0, u16 btr1)
{
	unsigned short tempCR = 0;

	DEBUGMSG("(c%d)calling c_can_set_btregs(...)\n", pchip->chip_idx);

	// Validate pointer
	if (NULL == pchip)
		return -1;

	if (c_can_enable_configuration(pchip))
		return -ENODEV;

	//read Control Register
	tempCR = c_can_read_reg_w(pchip, CCCR);
	//Configuration Change Enable
	c_can_write_reg_w(pchip, tempCR | CR_CCE, CCCR);
	c_can_write_reg_w(pchip, btr0 | (btr1 << 8), CCBT);

	if (c_can_disable_configuration(pchip))
		return -ENODEV;

	DEBUGMSG("-> ok\n");
	return 0;
}

///////////////////////////////////////////////////////////////////////
/*
 * Starts the Chip, by setting the CAN Enable Bit
 */
int c_can_start_chip(struct canchip_t *pchip)
{
	u16 flags = 0;

	DEBUGMSG("(c%d)calling c_can_start_chip(...)\n", pchip->chip_idx);

	// Validate pointer
	if (NULL == pchip) {
		DEBUGMSG("-> Error Chip not available.\n");
		return -1;
	}
#ifdef C_CAN_WITH_CCCE
	flags = c_can_read_reg_w(pchip, CCCE) | CE_EN;
	c_can_write_reg_w(pchip, flags, CCCE);
#endif

	DEBUGMSG("-> ok\n");
#ifdef REGDUMP
	c_can_registerdump(pchip);
#endif

	return 0;
}

///////////////////////////////////////////////////////////////////////
/*
 * Stops the Chip, by deleting the CAN Enable Bit
 */
int c_can_stop_chip(struct canchip_t *pchip)
{
	u16 flags = 0;

	DEBUGMSG("(c%d)calling c_can_stop_chip(...)\n", pchip->chip_idx);

	// Validate pointer
	if (NULL == pchip) {
		DEBUGMSG("-> Error Chip not available.\n");
		return -1;
	}
#ifdef C_CAN_WITH_CCCE
	flags = c_can_read_reg_w(pchip, CCCE) & ~CE_EN;
	c_can_write_reg_w(pchip, flags, CCCE);
#endif

	DEBUGMSG("-> ok\n");
	return 0;
}

int c_can_attach_to_chip(struct canchip_t *chip)
{
	return 0;
}

int c_can_release_chip(struct canchip_t *chip)
{
	int temp;

	temp = c_can_read_reg_w(chip, CCCR);

	/* Disable IRQ generation */
	c_can_config_irqs(chip, 0);

	temp = c_can_read_reg_w(chip, CCCR);

	/* Power-down C_CAN, except this does nothing in the version 1.2 */
	c_can_stop_chip(chip);

	return 0;
}

///////////////////////////////////////////////////////////////////////
/*
 *Check the TxOK bit of the Status Register and resets it afterwards.
 */
int c_can_check_tx_stat(struct canchip_t *pchip)
{
	unsigned long tempstat = 0;

	DEBUGMSG("(c%d)calling c_can_check_tx_stat(...)\n", pchip->chip_idx);

	// Validate pointer
	if (NULL == pchip)
		return -1;

	tempstat = c_can_read_reg_w(pchip, CCSR);

	if (tempstat & SR_TXOK) {
		c_can_write_reg_w(pchip, tempstat & ~SR_TXOK, CCSR);
		return 0;
	} else {
		return 1;
	}
}

///////////////////////////////////////////////////////////////////////
int c_can_wakeup_tx(struct canchip_t *chip, struct msgobj_t *obj)
{
	can_preempt_disable();

	can_msgobj_set_fl(obj, TX_REQUEST);

	/* calls c_can_irq_write_handler synchronized with other invocations
	   from kernel and IRQ context */
	c_can_irq_sync_activities(chip, obj);

	can_preempt_enable();
	return 0;
}

///////////////////////////////////////////////////////////////////////
int c_can_filtch_rq(struct canchip_t *chip, struct msgobj_t *obj)
{
	can_preempt_disable();

	can_msgobj_set_fl(obj, FILTCH_REQUEST);

	/* setups filter synchronized with other invocations from kernel and IRQ context */
	c_can_irq_sync_activities(chip, obj);

	can_preempt_enable();
	return 0;
}

///////////////////////////////////////////////////////////////////////
void c_can_registerdump(struct canchip_t *pchip)
{
	CANMSG("------------------------------------\n");
	CANMSG("---------C-CAN Register Dump--------\n");
	CANMSG("------------at 0x%.8lx-----------\n",
	       (unsigned long)pchip->chip_base_addr);
	CANMSG("Control Register:             0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCCR)));
	CANMSG("Status Register:              0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCSR)));
	CANMSG("Error Counting Register:      0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCEC)));
	CANMSG("Bit Timing Register:          0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCBT)));
	CANMSG("Interrupt Register:           0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCINTR)));
	CANMSG("Test Register:                0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCTR)));
	CANMSG("Baud Rate Presc. Register:    0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCBRPE)));
#ifdef C_CAN_WITH_CCCE
	CANMSG("CAN Enable Register:          0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCCE)));
#endif
	CANMSG("Transm. Req. 1 Register:      0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCTREQ1)));
	CANMSG("Transm. Req. 2 Register:      0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCTREQ2)));
	CANMSG("New Data 1 Register:          0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCND1)));
	CANMSG("New Data 2 Register:          0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCND2)));
	CANMSG("Interrupt Pend. 1 Register:   0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCINTP1)));
	CANMSG("Interrupt Pend. 2 Register:   0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCINTP2)));
	CANMSG("------------------------------------\n");
	CANMSG("IF1 Command Req. Register:    0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1CR)));
	CANMSG("IF1 Command Mask Register:    0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1CM)));
	CANMSG("IF1 Mask 1 Register:          0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1M1)));
	CANMSG("IF1 Mask 2 Register:          0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1M2)));
	CANMSG("IF1 Arbitration 1 Register:   0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1A1)));
	CANMSG("IF1 Arbitration 2 Register:   0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1A2)));
	CANMSG("IF1 Message Control Register: 0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1DMC)));
	CANMSG("IF1 Data A1 Register:         0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1DA1)));
	CANMSG("IF1 Data A2 Register:         0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1DA2)));
	CANMSG("IF1 Data B1 Register:         0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1DB1)));
	CANMSG("IF1 Data B2 Register:         0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1DB2)));
	CANMSG("------------------------------------\n");
	CANMSG("IF2 Command Req. Register:    0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF2CR)));
	CANMSG("IF2 Command Mask Register:    0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF2CM)));
	CANMSG("IF2 Mask 1 Register:          0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF2M1)));
	CANMSG("IF2 Mask 2 Register:          0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF2M2)));
	CANMSG("IF2 Arbitration 1 Register:   0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF2A1)));
	CANMSG("IF2 Arbitration 2 Register:   0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF2A2)));
	CANMSG("IF2 Message Control Register: 0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF2DMC)));
	CANMSG("IF2 Data A1 Register:         0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF2DA1)));
	CANMSG("IF2 Data A2 Register:         0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF2DA2)));
	CANMSG("IF2 Data B1 Register:         0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF2DB1)));
	CANMSG("IF2 Data B2 Register:         0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF2DB2)));
	CANMSG("------------------------------------\n");
	CANMSG("------------------------------------\n");
}

void c_can_if1_registerdump(struct canchip_t *pchip)
{
	CANMSG("----------------------------------------\n");
	CANMSG("Error Counting Register:      0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCEC)));
	CANMSG("---------C-CAN IF1 Register Dump--------\n");
	CANMSG("IF1 Command Req. Register:    0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1CR)));
	CANMSG("IF1 Command Mask Register:    0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1CM)));
	CANMSG("IF1 Mask 1 Register:          0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1M1)));
	CANMSG("IF1 Mask 2 Register:          0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1M2)));
	CANMSG("IF1 Arbitration 1 Register:   0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1A1)));
	CANMSG("IF1 Arbitration 2 Register:   0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1A2)));
	CANMSG("IF1 Message Control Register: 0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1DMC)));
	CANMSG("IF1 Data A1 Register:         0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1DA1)));
	CANMSG("IF1 Data A2 Register:         0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1DA2)));
	CANMSG("IF1 Data B1 Register:         0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1DB1)));
	CANMSG("IF1 Data B2 Register:         0x%.4lx\n",
	       (long)(c_can_read_reg_w(pchip, CCIF1DB2)));
}

///////////////////////////////////////////////////////////////////////

int c_can_register(struct chipspecops_t *chipspecops)
{
	CANMSG("initializing c_can chip operations\n");
	chipspecops->chip_config = c_can_chip_config;
	chipspecops->baud_rate = c_can_baud_rate;
	/*chipspecops->standard_mask=c_can_standard_mask;
	   chipspecops->extended_mask=c_can_extended_mask;
	   chipspecops->message15_mask=c_can_extended_mask; */
	chipspecops->clear_objects = c_can_clear_objects;
	/*chipspecops->config_irqs=c_can_config_irqs; */
	chipspecops->pre_read_config = c_can_pre_read_config;
	chipspecops->pre_write_config = c_can_pre_write_config;
	chipspecops->send_msg = c_can_send_msg;
	chipspecops->check_tx_stat = c_can_check_tx_stat;
	chipspecops->wakeup_tx = c_can_wakeup_tx;
	chipspecops->filtch_rq = c_can_filtch_rq;
	chipspecops->remote_request = c_can_remote_request;
	chipspecops->enable_configuration = c_can_enable_configuration;
	chipspecops->disable_configuration = c_can_disable_configuration;
	chipspecops->attach_to_chip = c_can_attach_to_chip;
	chipspecops->release_chip = c_can_release_chip;
	chipspecops->set_btregs = c_can_set_btregs;
	chipspecops->start_chip = c_can_start_chip;
	chipspecops->stop_chip = c_can_stop_chip;
	chipspecops->irq_handler = c_can_irq_handler;
	chipspecops->irq_accept = NULL;
	return 0;
}

int c_can_fill_chipspecops(struct canchip_t *chip)
{
	chip->chip_type = "c_can";
	if (MAX_MSGOBJS >= 32) {
		chip->max_objects = 32;
	} else {
		CANMSG("C_CAN requires 32 message objects per chip,"
		       " but only %d is compiled maximum\n", MAX_MSGOBJS);
		chip->max_objects = MAX_MSGOBJS;
	}
	c_can_register(chip->chipspecops);
	return 0;
}
