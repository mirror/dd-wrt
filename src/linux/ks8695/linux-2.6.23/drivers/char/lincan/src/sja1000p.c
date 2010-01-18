/* sja1000.c
 * Linux CAN-bus device driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Changed for PeliCan mode SJA1000 by Tomasz Motylewski (BFAD GmbH)
 * T.Motylewski@bfad.de
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/sja1000p.h"

#ifdef CONFIG_OC_LINCAN_DETAILED_ERRORS

static const char *sja1000_ecc_errc_str[]={
	"bit error",
	"form error",
	"stuff error",
	"other type of error"
};

static const char *sja1000_ecc_seg_str[]={
	"?0?",
	"?1?",
	"ID.28 to ID.21",
	"start of frame",
	"bit SRTR",
	"bit IDE",
	"ID.20 to ID.18",
	"ID.17 to ID.13",
	"CRC sequence",
	"reserved bit 0",
	"data field",
	"data length code",
	"bit RTR",
	"reserved bit 1",
	"ID.4 to ID.0",
	"ID.12 to ID.5",
	"?16?"
	"active error flag",
	"intermission",
	"tolerate dominant bits",
	"?20?",
	"?21?",
	"passive error flag",
	"error delimiter",
	"CRC delimiter",
	"acknowledge slot",
	"end of frame",
	"acknowledge delimiter",
	"overload flag",
	"?29?",
	"?30?",
	"?31?"
};

#endif /*CONFIG_OC_LINCAN_DETAILED_ERRORS*/

static int sja1000_report_error_limit_counter;

static void sja1000_report_error(struct canchip_t *chip,
				unsigned sr, unsigned ir, unsigned ecc)
{
	if(sja1000_report_error_limit_counter>=100)
		return;

	CANMSG("Error: status register: 0x%x irq_register: 0x%02x error: 0x%02x\n",
		sr, ir, ecc);

	sja1000_report_error_limit_counter+=10;

	if(sja1000_report_error_limit_counter>=100){
		sja1000_report_error_limit_counter+=10;
		CANMSG("Error: too many errors, reporting disabled\n");
		return;
	}

#ifdef CONFIG_OC_LINCAN_DETAILED_ERRORS
	CANMSG("SR: BS=%c  ES=%c  TS=%c  RS=%c  TCS=%c TBS=%c DOS=%c RBS=%c\n",
		sr&sjaSR_BS?'1':'0',sr&sjaSR_ES?'1':'0',
		sr&sjaSR_TS?'1':'0',sr&sjaSR_RS?'1':'0',
		sr&sjaSR_TCS?'1':'0',sr&sjaSR_TBS?'1':'0',
		sr&sjaSR_DOS?'1':'0',sr&sjaSR_RBS?'1':'0');
	CANMSG("IR: BEI=%c ALI=%c EPI=%c WUI=%c DOI=%c EI=%c  TI=%c  RI=%c\n",
		sr&sjaIR_BEI?'1':'0',sr&sjaIR_ALI?'1':'0',
		sr&sjaIR_EPI?'1':'0',sr&sjaIR_WUI?'1':'0',
		sr&sjaIR_DOI?'1':'0',sr&sjaIR_EI?'1':'0',
		sr&sjaIR_TI?'1':'0',sr&sjaIR_RI?'1':'0');
	if((sr&sjaIR_EI) || 1){
		CANMSG("EI: %s %s %s\n",
		       sja1000_ecc_errc_str[(ecc&(sjaECC_ERCC1|sjaECC_ERCC0))/sjaECC_ERCC0],
		       ecc&sjaECC_DIR?"RX":"TX",
		       sja1000_ecc_seg_str[ecc&sjaECC_SEG_M]
		      );
	}
#endif /*CONFIG_OC_LINCAN_DETAILED_ERRORS*/
}


/**
 * sja1000p_enable_configuration - enable chip configuration mode
 * @chip: pointer to chip state structure
 */
int sja1000p_enable_configuration(struct canchip_t *chip)
{
	int i=0;
	enum sja1000_PeliCAN_MOD flags;

	can_disable_irq(chip->chip_irq);

	flags=can_read_reg(chip,SJAMOD);

	while ((!(flags & sjaMOD_RM)) && (i<=10)) {
 		can_write_reg(chip, sjaMOD_RM, SJAMOD);
// TODO: configurable sjaMOD_AFM (32/16 bit acceptance filter)
// config sjaMOD_LOM (listen only)
		udelay(100);
		i++;
		flags=can_read_reg(chip, SJAMOD);
	}
	if (i>=10) {
		CANMSG("Reset error\n");
		can_enable_irq(chip->chip_irq);
		return -ENODEV;
	}

	return 0;
}

/**
 * sja1000p_disable_configuration - disable chip configuration mode
 * @chip: pointer to chip state structure
 */
int sja1000p_disable_configuration(struct canchip_t *chip)
{
	int i=0;
	enum sja1000_PeliCAN_MOD flags;

	flags=can_read_reg(chip,SJAMOD);

	while ( (flags & sjaMOD_RM) && (i<=50) ) {
// could be as long as 11*128 bit times after buss-off
		can_write_reg(chip, 0, SJAMOD);
// TODO: configurable sjaMOD_AFM (32/16 bit acceptance filter)
// config sjaMOD_LOM (listen only)
		udelay(100);
		i++;
		flags=can_read_reg(chip, SJAMOD);
	}
	if (i>=10) {
		CANMSG("Error leaving reset status\n");
		return -ENODEV;
	}

	can_enable_irq(chip->chip_irq);

	return 0;
}

/**
 * sja1000p_chip_config: - can chip configuration
 * @chip: pointer to chip state structure
 *
 * This function configures chip and prepares it for message
 * transmission and reception. The function resets chip,
 * resets mask for acceptance of all messages by call to
 * sja1000p_extended_mask() function and then 
 * computes and sets baudrate with use of function sja1000p_baud_rate().
 * Return Value: negative value reports error.
 * File: src/sja1000p.c
 */
int sja1000p_chip_config(struct canchip_t *chip)
{
	int i;
	unsigned char n, r;
	
	if (sja1000p_enable_configuration(chip))
		return -ENODEV;

	/* Set mode, clock out, comparator */
	can_write_reg(chip,sjaCDR_PELICAN|chip->sja_cdr_reg,SJACDR); 

	/* Ensure, that interrupts are disabled even on the chip level now */
	can_write_reg(chip, sjaDISABLE_INTERRUPTS, SJAIER);

	/* Set driver output configuration */
	can_write_reg(chip,chip->sja_ocr_reg,SJAOCR); 
	
	/* Simple check for chip presence */
	for (i=0, n=0x5a; i<8; i++, n+=0xf) {
		can_write_reg(chip,n,SJAACR0+i);
	}
	for (i=0, n=0x5a; i<8; i++, n+=0xf) {
		r = n^can_read_reg(chip,SJAACR0+i);
		if (r) {
			CANMSG("sja1000p_chip_config: chip connection broken,"
				" readback differ 0x%02x\n", r);
			return -ENODEV;
		}
	}
	

	if (sja1000p_extended_mask(chip,0x00000000, 0xffffffff))
		return -ENODEV;
	
	if (!chip->baudrate)
		chip->baudrate=1000000;
	if (sja1000p_baud_rate(chip,chip->baudrate,chip->clock,0,75,0))
		return -ENODEV;

	/* Enable hardware interrupts */
	can_write_reg(chip, sjaENABLE_INTERRUPTS, SJAIER); 

	sja1000p_disable_configuration(chip);

	return 0;
}

/**
 * sja1000p_extended_mask: - setup of extended mask for message filtering
 * @chip: pointer to chip state structure
 * @code: can message acceptance code
 * @mask: can message acceptance mask
 *
 * Return Value: negative value reports error.
 * File: src/sja1000p.c
 */
int sja1000p_extended_mask(struct canchip_t *chip, unsigned long code, unsigned  long mask)
{
	int i;

	if (sja1000p_enable_configuration(chip))
		return -ENODEV;

// LSB to +3, MSB to +0	
	for(i=SJA_PeliCAN_AC_LEN; --i>=0;) {
		can_write_reg(chip,code&0xff,SJAACR0+i);
		can_write_reg(chip,mask&0xff,SJAAMR0+i);
		code >>= 8;
		mask >>= 8;
	}

	DEBUGMSG("Setting acceptance code to 0x%lx\n",(unsigned long)code);
	DEBUGMSG("Setting acceptance mask to 0x%lx\n",(unsigned long)mask);

	sja1000p_disable_configuration(chip);

	return 0;
}

/**
 * sja1000p_baud_rate: - set communication parameters.
 * @chip: pointer to chip state structure
 * @rate: baud rate in Hz
 * @clock: frequency of sja1000 clock in Hz (ISA osc is 14318000)
 * @sjw: synchronization jump width (0-3) prescaled clock cycles
 * @sampl_pt: sample point in % (0-100) sets (TSEG1+1)/(TSEG1+TSEG2+2) ratio
 * @flags: fields %BTR1_SAM, %OCMODE, %OCPOL, %OCTP, %OCTN, %CLK_OFF, %CBP
 *
 * Return Value: negative value reports error.
 * File: src/sja1000p.c
 */
int sja1000p_baud_rate(struct canchip_t *chip, int rate, int clock, int sjw,
							int sampl_pt, int flags)
{
	int best_error = 1000000000, error;
	int best_tseg=0, best_brp=0, best_rate=0, brp=0;
	int tseg=0, tseg1=0, tseg2=0;
	
	if (sja1000p_enable_configuration(chip))
		return -ENODEV;

	clock /=2;

	/* tseg even = round down, odd = round up */
	for (tseg=(0+0+2)*2; tseg<=(sjaMAX_TSEG2+sjaMAX_TSEG1+2)*2+1; tseg++) {
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
	if (tseg2 > sjaMAX_TSEG2)
		tseg2 = sjaMAX_TSEG2;
	tseg1 = best_tseg-tseg2-2;
	if (tseg1>sjaMAX_TSEG1) {
		tseg1 = sjaMAX_TSEG1;
		tseg2 = best_tseg-tseg1-2;
	}

	DEBUGMSG("Setting %d bps.\n", best_rate);
	DEBUGMSG("brp=%d, best_tseg=%d, tseg1=%d, tseg2=%d, sampl_pt=%d\n",
					best_brp, best_tseg, tseg1, tseg2,
					(100*(best_tseg-tseg2)/(best_tseg+1)));


	can_write_reg(chip, sjw<<6 | best_brp, SJABTR0);
	can_write_reg(chip, ((flags & BTR1_SAM) != 0)<<7 | (tseg2<<4) 
					| tseg1, SJABTR1);

	sja1000p_disable_configuration(chip);

	return 0;
}

/**
 * sja1000p_read: - reads and distributes one or more received messages
 * @chip: pointer to chip state structure
 * @obj: pinter to CAN message queue information
 *
 * File: src/sja1000p.c
 */
void sja1000p_read(struct canchip_t *chip, struct msgobj_t *obj) {
	int i, flags, len, datastart;

#ifdef CAN_USE_LEDS
	// turn data LED on
	chip->canStatusLED |= DATA_LED;
	can_write_reg(chip, chip->canStatusLED, LED_REGISTER);
#endif

	do {
		flags = can_read_reg(chip,SJAFRM);
		if(flags&sjaFRM_FF) {
			obj->rx_msg.id =
				(can_read_reg(chip,SJAID0)<<21) +
				(can_read_reg(chip,SJAID1)<<13) +
				(can_read_reg(chip,SJAID2)<<5) +
				(can_read_reg(chip,SJAID3)>>3);
			datastart = SJADATE;
		} else {
			obj->rx_msg.id =
				(can_read_reg(chip,SJAID0)<<3) +
				(can_read_reg(chip,SJAID1)>>5);
			datastart = SJADATS;
		}
		obj->rx_msg.flags =
			((flags & sjaFRM_RTR) ? MSG_RTR : 0) |
			((flags & sjaFRM_FF) ? MSG_EXT : 0);
		len = flags & sjaFRM_DLC_M;
		obj->rx_msg.length = len;
		if(len > CAN_MSG_LENGTH) len = CAN_MSG_LENGTH;
		for(i=0; i< len; i++) {
			obj->rx_msg.data[i]=can_read_reg(chip,datastart+i);
		}

		/* fill CAN message timestamp */
		can_filltimestamp(&obj->rx_msg.timestamp);

		if(canque_filter_msg2edges(obj->qends, &obj->rx_msg) == 0)
		{
		  chip->canStatus |= CAN_IOCTL_FLAG_RX_FIFO_FULL;
#ifdef CAN_USE_LEDS
		  if(!(chip->canStatus & CAN_IOCTL_FLAG_BUS_ERROR) && !(chip->canStatusLED & ERROR_LED_FLASHING))
		  {
		    chip->canStatusLED |= ERROR_LED_FLASHING;
		    can_write_reg(chip, chip->canStatusLED, LED_REGISTER);
		  }
#endif
		}

		can_write_reg(chip, sjaCMR_RRB, SJACMR);

	} while (can_read_reg(chip, SJASR) & sjaSR_RBS);
}

/**
 * sja1000p_pre_read_config: - prepares message object for message reception
 * @chip: pointer to chip state structure
 * @obj: pointer to message object state structure
 *
 * Return Value: negative value reports error.
 *	Positive value indicates immediate reception of message.
 * File: src/sja1000p.c
 */
int sja1000p_pre_read_config(struct canchip_t *chip, struct msgobj_t *obj)
{
	int status;
	status=can_read_reg(chip,SJASR);
	
	if(status  & sjaSR_BS) {
		/* Try to recover from error condition */
		DEBUGMSG("sja1000p_pre_read_config bus-off recover 0x%x\n",status);
		sja1000p_enable_configuration(chip);
		can_write_reg(chip, 0, SJARXERR);
		can_write_reg(chip, 0, SJATXERR1);
		can_read_reg(chip, SJAECC);
		sja1000p_disable_configuration(chip);
	}

	if (!(status&sjaSR_RBS)) {
		return 0;
	}

	can_write_reg(chip, sjaDISABLE_INTERRUPTS, SJAIER); //disable interrupts for a moment
	sja1000p_read(chip, obj);
	can_write_reg(chip, sjaENABLE_INTERRUPTS, SJAIER); //enable interrupts
	return 1;
}

#define MAX_TRANSMIT_WAIT_LOOPS 10
/**
 * sja1000p_pre_write_config: - prepares message object for message transmission
 * @chip: pointer to chip state structure
 * @obj: pointer to message object state structure
 * @msg: pointer to CAN message
 *
 * This function prepares selected message object for future initiation
 * of message transmission by sja1000p_send_msg() function.
 * The CAN message data and message ID are transfered from @msg slot
 * into chip buffer in this function.
 * Return Value: negative value reports error.
 * File: src/sja1000p.c
 */
int sja1000p_pre_write_config(struct canchip_t *chip, struct msgobj_t *obj, 
							struct canmsg_t *msg)
{
	int i=0; 
	unsigned int id;
	int status;
	int len;
#if defined(VSCAN_CHECK_WRITTEN_FRAME)
	int rc = 0;
	canmsg_id_t test_id;
#endif /* defined(VSCAN_CHECK_WRITTEN_FRAME) */

	/* Wait until Transmit Buffer Status is released */
	while ( !((status=can_read_reg(chip, SJASR)) & sjaSR_TBS) && 
						i++<MAX_TRANSMIT_WAIT_LOOPS) {
		udelay(i);
	}
	
	if(status & sjaSR_BS) {
		/* Try to recover from error condition */
		DEBUGMSG("sja1000p_pre_write_config bus-off recover 0x%x\n",status);
		sja1000p_enable_configuration(chip);
		can_write_reg(chip, 0, SJARXERR);
		can_write_reg(chip, 0, SJATXERR1);
		can_read_reg(chip, SJAECC);
		sja1000p_disable_configuration(chip);
	}
	if (!(can_read_reg(chip, SJASR) & sjaSR_TBS)) {
		CANMSG("Transmit timed out, cancelling\n");
// here we should check if there is no write/select waiting for this
// transmit. If so, set error ret and wake up.
// CHECKME: if we do not disable sjaIER_TIE (TX IRQ) here we get interrupt
// immediately
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
	/* len &= sjaFRM_DLC_M; ensured by above condition already */
	can_write_reg(chip, ((msg->flags&MSG_EXT)?sjaFRM_FF:0) |
		((msg->flags & MSG_RTR) ? sjaFRM_RTR : 0) | len, SJAFRM);
	if(msg->flags&MSG_EXT) {
		id=msg->id<<3;
		can_write_reg(chip, id & 0xff, SJAID3);
		id >>= 8;
		can_write_reg(chip, id & 0xff, SJAID2);
		id >>= 8;
		can_write_reg(chip, id & 0xff, SJAID1);
		id >>= 8;
		can_write_reg(chip, id, SJAID0);
		for(i=0; i < len; i++) {
			can_write_reg(chip, msg->data[i], SJADATE+i);
		}
	} else {
		id=msg->id<<5;
		can_write_reg(chip, (id >> 8) & 0xff, SJAID0);
		can_write_reg(chip, id & 0xff, SJAID1);
		for(i=0; i < len; i++) {
			can_write_reg(chip, msg->data[i], SJADATS+i);
		}
	}

#if defined(VSCAN_CHECK_WRITTEN_FRAME)
	test_id = 0xff & can_read_reg(chip, SJAID3 + 80);
	test_id += (0xff & can_read_reg(chip, SJAID2 + 80)) << 8; 
	test_id += (0xff & can_read_reg(chip, SJAID1 + 80)) << 16; 
	test_id += (0xff & can_read_reg(chip, SJAID0 + 80)) << 24; 
	test_id >>= 3;

	if(test_id != msg->id)
		rc = 0x01;
	//printk("Soll ID: 0x%08X, ist ID: 0x%08X\n", msg->id, test_id);
	for(i=0; i < len; i++)
	{
		if(msg->data[i] != (0xff & can_read_reg(chip, SJADATE + 80 + i)))
			rc |= 0x02;
		//printk("Data soll [%d] 0x%02X, data ist 0x%02X\n", i, msg->data[i], 0xff & can_read_reg(chip, SJADATE + 80 + i));
	}

	if(rc != 0)
	{
		printk("ID %s, data %s\n", (rc & 0x01)?"error":"O.K.", (rc & 0x02)?"error":"O.K.");
	}
#endif /* defined(VSCAN_CHECK_WRITTEN_FRAME) */
	return 0;
}

/**
 * sja1000p_send_msg: - initiate message transmission
 * @chip: pointer to chip state structure
 * @obj: pointer to message object state structure
 * @msg: pointer to CAN message
 *
 * This function is called after sja1000p_pre_write_config() function,
 * which prepares data in chip buffer.
 * Return Value: negative value reports error.
 * File: src/sja1000p.c
 */
int sja1000p_send_msg(struct canchip_t *chip, struct msgobj_t *obj, 
							struct canmsg_t *msg)
{
#ifdef CAN_USE_LEDS
	// turn data LED on
	chip->canStatusLED |= DATA_LED;
	can_write_reg(chip, chip->canStatusLED, LED_REGISTER);
#endif
	can_write_reg(chip, sjaCMR_TR, SJACMR);

	return 0;
}

/**
 * sja1000p_check_tx_stat: - checks state of transmission engine
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 *	Positive return value indicates transmission under way status.
 *	Zero value indicates finishing of all issued transmission requests.
 * File: src/sja1000p.c
 */
int sja1000p_check_tx_stat(struct canchip_t *chip)
{
	if (can_read_reg(chip,SJASR) & sjaSR_TCS)
		return 0;
	else
		return 1;
}

/**
 * sja1000p_set_btregs: -  configures bitrate registers
 * @chip: pointer to chip state structure
 * @btr0: bitrate register 0
 * @btr1: bitrate register 1
 *
 * Return Value: negative value reports error.
 * File: src/sja1000p.c
 */
int sja1000p_set_btregs(struct canchip_t *chip, unsigned short btr0, 
							unsigned short btr1)
{
	if (sja1000p_enable_configuration(chip))
		return -ENODEV;

	can_write_reg(chip, btr0, SJABTR0);
	can_write_reg(chip, btr1, SJABTR1);

	sja1000p_disable_configuration(chip);

	return 0;
}

/**
 * sja1000p_start_chip: -  starts chip message processing
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/sja1000p.c
 */
int sja1000p_start_chip(struct canchip_t *chip)
{
	enum sja1000_PeliCAN_MOD flags;

	flags = can_read_reg(chip, SJAMOD) & (sjaMOD_LOM|sjaMOD_STM|sjaMOD_AFM|sjaMOD_SM);
	can_write_reg(chip, flags, SJAMOD);

	sja1000_report_error_limit_counter=0;

	return 0;
}

/**
 * sja1000p_stop_chip: -  stops chip message processing
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/sja1000p.c
 */
int sja1000p_stop_chip(struct canchip_t *chip)
{
	enum sja1000_PeliCAN_MOD flags;

	flags = can_read_reg(chip, SJAMOD) & (sjaMOD_LOM|sjaMOD_STM|sjaMOD_AFM|sjaMOD_SM);
	can_write_reg(chip, flags|sjaMOD_RM, SJAMOD);

	return 0;
}

/**
 * sja1000p_attach_to_chip: - attaches to the chip, setups registers and state
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/sja1000p.c
 */
int sja1000p_attach_to_chip(struct canchip_t *chip)
{
	return 0;
}

/**
 * sja1000p_release_chip: - called before chip structure removal if %CHIP_ATTACHED is set
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/sja1000p.c
 */
int sja1000p_release_chip(struct canchip_t *chip)
{
	sja1000p_stop_chip(chip);
	can_write_reg(chip, sjaDISABLE_INTERRUPTS, SJAIER);

	return 0;
}

/**
 * sja1000p_remote_request: - configures message object and asks for RTR message
 * @chip: pointer to chip state structure
 * @obj: pointer to message object structure
 *
 * Return Value: negative value reports error.
 * File: src/sja1000p.c
 */
int sja1000p_remote_request(struct canchip_t *chip, struct msgobj_t *obj)
{
	CANMSG("sja1000p_remote_request not implemented\n");
	return -ENOSYS;
}

/**
 * sja1000p_standard_mask: - setup of mask for message filtering
 * @chip: pointer to chip state structure
 * @code: can message acceptance code
 * @mask: can message acceptance mask
 *
 * Return Value: negative value reports error.
 * File: src/sja1000p.c
 */
int sja1000p_standard_mask(struct canchip_t *chip, unsigned short code,
		unsigned short mask)
{
	CANMSG("sja1000p_standard_mask not implemented\n");
	return -ENOSYS;
}

/**
 * sja1000p_clear_objects: - clears state of all message object residing in chip
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/sja1000p.c
 */
int sja1000p_clear_objects(struct canchip_t *chip)
{
	CANMSG("sja1000p_clear_objects not implemented\n");
	return -ENOSYS;
}

/**
 * sja1000p_config_irqs: - tunes chip hardware interrupt delivery
 * @chip: pointer to chip state structure
 * @irqs: requested chip IRQ configuration
 *
 * Return Value: negative value reports error.
 * File: src/sja1000p.c
 */
int sja1000p_config_irqs(struct canchip_t *chip, short irqs)
{
	CANMSG("sja1000p_config_irqs not implemented\n");
	return -ENOSYS;
}

/**
 * sja1000p_irq_write_handler: - part of ISR code responsible for transmit events
 * @chip: pointer to chip state structure
 * @obj: pointer to attached queue description
 *
 * The main purpose of this function is to read message from attached queues
 * and transfer message contents into CAN controller chip.
 * This subroutine is called by
 * sja1000p_irq_write_handler() for transmit events.
 * File: src/sja1000p.c
 */
void sja1000p_irq_write_handler(struct canchip_t *chip, struct msgobj_t *obj)
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
	
	can_msgobj_clear_fl(obj,TX_PENDING);
	cmd=canque_test_outslot(obj->qends, &obj->tx_qedge, &obj->tx_slot);
	if(cmd<0)
		return;

	can_msgobj_set_fl(obj,TX_PENDING);

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

#define MAX_RETR 10

/**
 * sja1000p_irq_handler: - interrupt service routine
 * @irq: interrupt vector number, this value is system specific
 * @chip: pointer to chip state structure
 * 
 * Interrupt handler is activated when state of CAN controller chip changes,
 * there is message to be read or there is more space for new messages or
 * error occurs. The receive events results in reading of the message from
 * CAN controller chip and distribution of message through attached
 * message queues.
 * File: src/sja1000p.c
 */
int sja1000p_irq_handler(int irq, struct canchip_t *chip)
{
	int irq_register, status, error_code;
	struct msgobj_t *obj=chip->msgobj[0];
	int loop_cnt=CHIP_MAX_IRQLOOP;


	irq_register=can_read_reg(chip,SJAIR);
	if(!can_msgobj_test_fl(obj,OPENED))
		return CANCHIP_IRQ_HANDLED;
	if(irq_register & sjaIR_ALI)
		chip->canStatus |= CAN_IOCTL_FLAG_ARBIT_LOST;

#ifdef CAN_USE_LEDS
	// let error LED flashing
	if(can_msgobj_test_fl(obj,OPENED) 
		&& (chip->canStatus & CAN_IOCTL_FLAG_ARBIT_LOST) 
		&& !(chip->canStatus & CAN_IOCTL_FLAG_BUS_ERROR) 
		&& !(chip->canStatusLED & ERROR_LED_FLASHING))
	{
		chip->canStatusLED |= ERROR_LED_FLASHING;
		can_write_reg(chip, chip->canStatusLED, LED_REGISTER);
	}
#endif

//	DEBUGMSG("sja1000_irq_handler: SJAIR:%02x\n",irq_register);
//	DEBUGMSG("sja1000_irq_handler: SJASR:%02x\n",
//					can_read_reg(chip,SJASR));

	if ((irq_register & (sjaIR_BEI|sjaIR_EPI|sjaIR_DOI|sjaIR_EI|sjaIR_TI|sjaIR_RI)) == 0)
		return CANCHIP_IRQ_NONE;

	if(!(chip->flags&CHIP_CONFIGURED)) {
		// Don't display this message, cause CAN interface shares interrupt with UARTs
		//CANMSG("sja1000p_irq_handler: called for non-configured device, irq_register 0x%02x\n", irq_register);
		return CANCHIP_IRQ_NONE;
	}

	status=can_read_reg(chip,SJASR);

	do {

		if(!loop_cnt--) {
			CANMSG("sja1000p_irq_handler IRQ %d stuck\n",irq);
			return CANCHIP_IRQ_STUCK;
		}

		/* (irq_register & sjaIR_RI) */
		/*	old variant using SJAIR, collides with intended use with irq_accept */
		if (status & sjaSR_RBS) {
			DEBUGMSG("sja1000_irq_handler: RI or RBS\n");
			sja1000p_read(chip,obj);
			obj->ret = 0;
		}

		/* (irq_register & sjaIR_TI) */
		/*	old variant using SJAIR, collides with intended use with irq_accept */
		if (((status & sjaSR_TBS) && can_msgobj_test_fl(obj,TX_PENDING))||
		    (can_msgobj_test_fl(obj,TX_REQUEST))) {
			DEBUGMSG("sja1000_irq_handler: TI or TX_PENDING and TBS\n");
			obj->ret = 0;
			can_msgobj_set_fl(obj,TX_REQUEST);
			while(!can_msgobj_test_and_set_fl(obj,TX_LOCK)){
				can_msgobj_clear_fl(obj,TX_REQUEST);

				if (can_read_reg(chip, SJASR) & sjaSR_TBS)
					sja1000p_irq_write_handler(chip, obj);

				can_msgobj_clear_fl(obj,TX_LOCK);
				if(!can_msgobj_test_fl(obj,TX_REQUEST)) break;
				DEBUGMSG("TX looping in sja1000_irq_handler\n");
			}
		}
		if ((irq_register & (sjaIR_EI|sjaIR_BEI|sjaIR_EPI|sjaIR_DOI)) != 0) { 
			if(irq_register & sjaIR_EI)
				chip->canStatus |= CAN_IOCTL_FLAG_ERR_WARNING;
			if(irq_register & sjaIR_BEI)
				chip->canStatus |= CAN_IOCTL_FLAG_BUS_ERROR;
			if(irq_register & sjaIR_EPI)
				chip->canStatus |= CAN_IOCTL_FLAG_ERR_PASSIVE;
			if(irq_register & sjaIR_DOI)
				chip->canStatus |= CAN_IOCTL_FLAG_DATA_OVERRUN;

#ifdef CAN_USE_LEDS
			// let error LED flashing or turn it on
			if(can_msgobj_test_fl(obj,OPENED) 
				&& !(chip->canStatus & CAN_IOCTL_FLAG_BUS_ERROR) 
				&& (chip->canStatus & (CAN_IOCTL_FLAG_ERR_WARNING | CAN_IOCTL_FLAG_ERR_PASSIVE | CAN_IOCTL_FLAG_DATA_OVERRUN)) 
				&& !(chip->canStatusLED & ERROR_LED_FLASHING))
			{
				chip->canStatusLED |= ERROR_LED_FLASHING;
				can_write_reg(chip, chip->canStatusLED, LED_REGISTER);
			}
			else if(can_msgobj_test_fl(obj,OPENED) && (chip->canStatus & CAN_IOCTL_FLAG_BUS_ERROR) && !(chip->canStatusLED & ERROR_LED))
			{
				chip->canStatusLED |= ERROR_LED;
				can_write_reg(chip, chip->canStatusLED, LED_REGISTER);
			}
#endif

			// Some error happened
			error_code=can_read_reg(chip,SJAECC);
			// disable error reporting
			//sja1000_report_error(chip, status, irq_register, error_code);
// FIXME: chip should be brought to usable state. Transmission cancelled if in progress.
// Reset flag set to 0 if chip is already off the bus. Full state report
			obj->ret=-1;
		
			if(error_code == 0xd9) {
				obj->ret= -ENXIO;
				/* no such device or address - no ACK received */
			}
			if(obj->tx_retry_cnt++>MAX_RETR) {
				can_write_reg(chip, sjaCMR_AT, SJACMR); // cancel any transmition
				obj->tx_retry_cnt = 0;
			}
			if(status&sjaSR_BS) {
				CANMSG("bus-off, resetting sja1000p\n");
				can_write_reg(chip, 0, SJAMOD);
			}

			if(obj->tx_slot){
				canque_notify_inends(obj->tx_qedge, CANQUEUE_NOTIFY_ERRTX_BUS);
				/*canque_free_outslot(obj->qends, obj->tx_qedge, obj->tx_slot);
				obj->tx_slot=NULL;*/
			}

		} else {
			if(sja1000_report_error_limit_counter)
				sja1000_report_error_limit_counter--;
			obj->tx_retry_cnt=0;
		}

		irq_register=can_read_reg(chip,SJAIR);

		status=can_read_reg(chip,SJASR);

		if(((status & sjaSR_TBS) && can_msgobj_test_fl(obj,TX_PENDING)) ||
		   (irq_register & sjaIR_TI))
			 can_msgobj_set_fl(obj,TX_REQUEST);

	} while((irq_register & (sjaIR_BEI|sjaIR_EPI|sjaIR_DOI|sjaIR_EI|sjaIR_RI)) ||
	        (can_msgobj_test_fl(obj,TX_REQUEST) && !can_msgobj_test_fl(obj,TX_LOCK)) ||
		(status & sjaSR_RBS));

	return CANCHIP_IRQ_HANDLED;
}

/**
 * sja1000p_wakeup_tx: - wakeups TX processing
 * @chip: pointer to chip state structure
 * @obj: pointer to message object structure
 *
 * Function is responsible for initiating message transmition.
 * It is responsible for clearing of object TX_REQUEST flag
 *
 * Return Value: negative value reports error.
 * File: src/sja1000p.c
 */
int sja1000p_wakeup_tx(struct canchip_t *chip, struct msgobj_t *obj)
{
	
	can_preempt_disable();
	
	can_msgobj_set_fl(obj,TX_PENDING);
	can_msgobj_set_fl(obj,TX_REQUEST);
	while(!can_msgobj_test_and_set_fl(obj,TX_LOCK)){
		can_msgobj_clear_fl(obj,TX_REQUEST);

		if (can_read_reg(chip, SJASR) & sjaSR_TBS){
			obj->tx_retry_cnt=0;
			sja1000p_irq_write_handler(chip, obj);
		}
	
		can_msgobj_clear_fl(obj,TX_LOCK);
		if(!can_msgobj_test_fl(obj,TX_REQUEST)) break;
		DEBUGMSG("TX looping in sja1000p_wakeup_tx\n");
	}

	can_preempt_enable();
	return 0;
}

int sja1000p_register(struct chipspecops_t *chipspecops)
{
	printk(KERN_INFO "lincan: initializing sja1000p chip operations\n");
	chipspecops->chip_config=sja1000p_chip_config;
	chipspecops->baud_rate=sja1000p_baud_rate;
	chipspecops->standard_mask=sja1000p_standard_mask;
	chipspecops->extended_mask=sja1000p_extended_mask;
	chipspecops->message15_mask=sja1000p_extended_mask;
	chipspecops->clear_objects=sja1000p_clear_objects;
	chipspecops->config_irqs=sja1000p_config_irqs;
	chipspecops->pre_read_config=sja1000p_pre_read_config;
	chipspecops->pre_write_config=sja1000p_pre_write_config;
	chipspecops->send_msg=sja1000p_send_msg;
	chipspecops->check_tx_stat=sja1000p_check_tx_stat;
	chipspecops->wakeup_tx=sja1000p_wakeup_tx;
	chipspecops->remote_request=sja1000p_remote_request;
	chipspecops->enable_configuration=sja1000p_enable_configuration;
	chipspecops->disable_configuration=sja1000p_disable_configuration;
	chipspecops->attach_to_chip=sja1000p_attach_to_chip;
	chipspecops->release_chip=sja1000p_release_chip;
	chipspecops->set_btregs=sja1000p_set_btregs;
	chipspecops->start_chip=sja1000p_start_chip;
	chipspecops->stop_chip=sja1000p_stop_chip;
	chipspecops->irq_handler=sja1000p_irq_handler;
	chipspecops->irq_accept=NULL;
	return 0;
}

/**
 * sja1000p_fill_chipspecops - fills chip specific operations
 * @chip: pointer to chip representation structure
 *
 * The function fills chip specific operations for sja1000 (PeliCAN) chip.
 *
 * Return Value: returns negative number in the case of fail
 */
int sja1000p_fill_chipspecops(struct canchip_t *chip)
{
	chip->chip_type="sja1000p";
	chip->max_objects=1;
	sja1000p_register(chip->chipspecops);
	return 0;
}
