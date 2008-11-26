
#include "voiceband/daa/daa.h"
#include "voiceband/tdm/mvTdm.h"


extern MV_STATUS mvTdmSpiRead(MV_U32 val1, MV_U32 val2, MV_U32 cmd, MV_U8 cs, MV_U8 *data);
extern MV_STATUS mvTdmSpiWrite(MV_U32 val1, MV_U32 val2, MV_U32 cmd, MV_U8 cs);

typedef struct _mv_daa_dev {
	unsigned char ch;
	unsigned char txSample;
	unsigned char rxSample;
	unsigned char hook;
	unsigned char ring;
	unsigned char reverse_polarity;
	unsigned char drop_out;
	unsigned char dcval; /* daisy chain value */

}MV_DAA_DEV;

#define COUNTRY_INDEX   29 /* SET TO ISRAEL */
#define NUM_OF_REGS	59

static int work_mode;
static int interrupt_mode;

static struct daa_mode {
	char *name;
	int ohs;
	int ohs2;
	int rz;
	int rt;
	int ilim;
	int dcv;
	int mini;
	int acim;
} daa_modes[] =
{
	{ "ARGENTINA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "AUSTRALIA", 1, 0, 0, 0, 0, 0, 0x3, 0x3, },
	{ "AUSTRIA", 0, 1, 0, 0, 1, 0x3, 0, 0x3, },
	{ "BAHRAIN", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "BELGIUM", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "BRAZIL", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "BULGARIA", 0, 0, 0, 0, 1, 0x3, 0x0, 0x3, },
	{ "CANADA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "CHILE", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "CHINA", 0, 0, 0, 0, 0, 0, 0x3, 0xf, },
	{ "COLUMBIA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "CROATIA", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "CYRPUS", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "CZECH", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "DENMARK", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "ECUADOR", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "EGYPT", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "ELSALVADOR", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "FINLAND", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "FRANCE", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "GERMANY", 0, 1, 0, 0, 1, 0x3, 0, 0x3, },
	{ "GREECE", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "GUAM", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "HONGKONG", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "HUNGARY", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "ICELAND", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "INDIA", 0, 0, 0, 0, 0, 0x3, 0, 0x4, },
	{ "INDONESIA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "IRELAND", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "ISRAEL", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "ITALY", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "JAPAN", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "JORDAN", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "KAZAKHSTAN", 0, 0, 0, 0, 0, 0x3, 0, },
	{ "KUWAIT", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "LATVIA", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "LEBANON", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "LUXEMBOURG", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "MACAO", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "MALAYSIA", 0, 0, 0, 0, 0, 0, 0x3, 0, },	/* Current loop >= 20ma */
	{ "MALTA", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "MEXICO", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "MOROCCO", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "NETHERLANDS", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "NEWZEALAND", 0, 0, 0, 0, 0, 0x3, 0, 0x4, },
	{ "NIGERIA", 0, 0, 0, 0, 0x1, 0x3, 0, 0x2, },
	{ "NORWAY", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "OMAN", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "PAKISTAN", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "PERU", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "PHILIPPINES", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "POLAND", 0, 0, 1, 1, 0, 0x3, 0, 0, },
	{ "PORTUGAL", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "ROMANIA", 0, 0, 0, 0, 0, 3, 0, 0, },
	{ "RUSSIA", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "SAUDIARABIA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "SINGAPORE", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "SLOVAKIA", 0, 0, 0, 0, 0, 0x3, 0, 0x3, },
	{ "SLOVENIA", 0, 0, 0, 0, 0, 0x3, 0, 0x2, },
	{ "SOUTHAFRICA", 1, 0, 1, 0, 0, 0x3, 0, 0x3, },
	{ "SOUTHKOREA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "SPAIN", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "SWEDEN", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "SWITZERLAND", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "TAIWAN", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "TBR21", 0, 0, 0, 0, 1, 0x3, 0, 0x2/*, 0x7e6c, 0x023a, */},
	/* Austria, Belgium, Denmark, Finland, France, Germany, Greece, Iceland, Ireland, Italy, Luxembourg, Netherlands,
	Norway, Portugal, Spain, Sweden, Switzerland, and UK */
	{ "THAILAND", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "UAE", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "UK", 0, 1, 0, 0, 1, 0x3, 0, 0x5, },
	{ "USA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "YEMEN", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "FCC", 0, 0, 0, 1, 0, 0x3, 0, 0, }, 	/* US, Canada */

};


unsigned char readDaaDirectReg(unsigned int daa_dev, unsigned char address)
{

	unsigned char data, control = 0;
	MV_DAA_DEV *pDaaDev = (MV_DAA_DEV *)daa_dev;
	unsigned int val1 = 0, val2 = 0, cmd = 0;

	control |= 0x60;

	if(work_mode)
		control |= pDaaDev->ch;       

	val1 =  (unsigned int)((address<<8) | control);
	cmd = TRANSFER_BYTES(2) | ENDIANESS_MSB_MODE | RD_MODE | READ_1_BYTE | CLK_SPEED_LO_DIV;


	mvTdmSpiRead(val1, val2, cmd, pDaaDev->ch, &data);
	return data;
}

void writeDaaDirectReg(unsigned int daa_dev, unsigned char address, unsigned char data)
{
	/*unsigned short addr;*/
	MV_DAA_DEV *pDaaDev = (MV_DAA_DEV *)daa_dev;
	unsigned char control = 0;
	unsigned int val1 = 0, val2 = 0, cmd = 0;


	control = 0x20;

	if(work_mode)
		control |= pDaaDev->ch;       

	val1 = (unsigned int)((address<<8) | control);
	val2 = (unsigned int)data;
	cmd = TRANSFER_BYTES(3) | ENDIANESS_MSB_MODE | WR_MODE | CLK_SPEED_LO_DIV;



	mvTdmSpiWrite(val1, val2, cmd, pDaaDev->ch);
}

void printDaaInfo(unsigned int daa_dev)
{
	unsigned char reg11;

	reg11 =  readDaaDirectReg(daa_dev, DAA_SYSTEM_AND_LINE_SIDE_REV_REG);
	if(reg11)
	{
		mvOsPrintf("DAA Si3050 Detected.\n");

		switch((reg11 & DAA_LINE_SIDE_ID_MASK))
		{
			case DAA_SI3018 :	mvOsPrintf("Line Side ID: Si3018\n");
						break;

			case DAA_SI3019 :	mvOsPrintf("Line Side ID: Si3019\n");
						break;

			default:		mvOsPrintf("Unkown Line Side ID\n");
						break;
		}
      
		switch((reg11 & DAA_SYSTEM_SIDE_REVISION_MASK))
		{
			case DAA_SYSTEM_SIDE_REV_A:	mvOsPrintf("System Side Revision: A\n");
							break;
		
			case DAA_SYSTEM_SIDE_REV_B:	mvOsPrintf("System Side Revision: B\n");
							break;
		
			case DAA_SYSTEM_SIDE_REV_C:	mvOsPrintf("System Side Revision: C\n");
							break;
		
			case DAA_SYSTEM_SIDE_REV_D:	mvOsPrintf("System Side Revision: D\n");
							break;
		
			default:			mvOsPrintf("Unkonwn System Side Revision\n");
							break;
		}

		mvOsPrintf("DAA Country Settings: %s\n",daa_modes[COUNTRY_INDEX].name);
	}
	else
		mvOsPrintf("Error, unable to communicate with DAA\n");


}

int initDaa(unsigned int *daaDev, unsigned int ch, int workMode, int intMode)
{
	unsigned char reg2 = 0, reg24;
	long newDelay;
	MV_DAA_DEV *pDaaDev;
	int lineSideId;

	pDaaDev = (MV_DAA_DEV *)mvOsMalloc(sizeof(MV_DAA_DEV));
	if(!pDaaDev)
	{
		mvOsPrintf("%s: error malloc failed\n",__FUNCTION__);
		return 1;
	}

	work_mode = workMode;
	interrupt_mode = intMode;

	pDaaDev->ch = ch;
	if(work_mode)
        	pDaaDev->dcval = ch + 1;

	
	pDaaDev->hook = 0;
	pDaaDev->ring = 0;
	pDaaDev->reverse_polarity = 0;
	pDaaDev->drop_out = 0;
		
       *daaDev = (unsigned int)pDaaDev;
	
	
	/* Software reset */
	writeDaaDirectReg(*daaDev, 1, 0x80);

	/* Wait just a bit */	
	mvOsDelay(100);

	setDaaDigitalHybrid(COUNTRY_INDEX, *daaDev);


	/* Set Transmit/Receive timeslot */
	/* Configure tx/rx sample in DAA */
	pDaaDev->txSample = ((pDaaDev->ch==0) ? CH0_TX_SLOT : CH1_TX_SLOT);
	pDaaDev->rxSample = ((pDaaDev->ch==0) ? CH0_RX_SLOT : CH1_RX_SLOT);
	mvOsPrintf("FXO-%d: RX sample %d, TX sample %d\n",pDaaDev->ch, pDaaDev->rxSample, pDaaDev->txSample);
	pDaaDev->txSample *= 8;
	pDaaDev->rxSample *= 8;

	setDaaPcmStartCountRegs(*daaDev);

#ifdef MV_TDM_LINEAR_MODE

	/* Enable PCM, linear 16 bit */
	writeDaaDirectReg(*daaDev, 33, 0x38);
#else
	/* Enable PCM, m-Law 8 bit */
	writeDaaDirectReg(*daaDev, 33, 0x28);
#endif

	/* Enable full wave rectifier */
	writeDaaDirectReg(*daaDev, DAA_INTERNATIONAL_CONTROL_3, DAA_RFWE);

	/* Disable interrupts */
	disableDaaInterrupts(*daaDev);

	/* Set AOUT/INT pin as hardware interrupt */
	reg2 = readDaaDirectReg(*daaDev, DAA_CONTROL_2_REG);
	writeDaaDirectReg(*daaDev, DAA_CONTROL_2_REG, (reg2 | DAA_INTE));

	
	/* Enable ISO-Cap */
	writeDaaDirectReg(*daaDev, DAA_DAA_CONTROL_2, 0);

	/* Wait 2000ms for ISO-cap to come up */
	newDelay = 2000;
	while(newDelay > 0 && !(readDaaDirectReg(*daaDev, DAA_SYSTEM_AND_LINE_SIDE_REV_REG) & 0xf0))
	{	
		mvOsDelay(100);
		newDelay -= 100;
	}

	lineSideId = readDaaDirectReg(*daaDev, DAA_SYSTEM_AND_LINE_SIDE_REV_REG);

	if (!(lineSideId & 0xf0)) {
		mvOsPrintf("Error: FXO did not bring up ISO link properly!\n");
		return 1;
	}

	/* Perform ADC manual calibration */
	daaCalibration(*daaDev);

	/* Enable Ring Validation */
	reg24 = readDaaDirectReg(*daaDev, DAA_RING_VALIDATION_CONTROL_3);

	writeDaaDirectReg(*daaDev,DAA_RING_VALIDATION_CONTROL_3 , reg24 | DAA_RNGV);

	
	/* Print DAA info */
	printDaaInfo(*daaDev);

	MV_TRC_REC("ISO-Cap is now up, line side: %02x rev %02x\n", 
		        readDaaDirectReg(*daaDev, 11) >> 4,
		       ( readDaaDirectReg(*daaDev, 13) >> 2) & 0xf);
	
	
	/* raise tx gain by 7 dB for NEWZEALAND */
	if (!strcmp(daa_modes[COUNTRY_INDEX].name, "NEWZEALAND")) {
		mvOsPrintf("Adjusting gain\n");
		writeDaaDirectReg(*daaDev, 38, 0x7);
	}

	return 0;

}

void enableDaaInterrupts(unsigned int daa_dev)
{
	MV_TRC_REC("enable daa-%d interrupts\n",((MV_DAA_DEV*)(daa_dev))->ch);
	/* first, clear source register */
	writeDaaDirectReg(daa_dev, DAA_INTERRUPT_SOURCE_REG, 0);
	/* enable interrupts in mask register */
	writeDaaDirectReg(daa_dev, DAA_INTERRUPT_MASK_REG, (DAA_DODM | DAA_RDTM));		
}

void disableDaaInterrupts(unsigned int daa_dev)
{
	MV_TRC_REC("disable daa-%d interrupts\n",((MV_DAA_DEV*)(daa_dev))->ch);
	writeDaaDirectReg(daa_dev, DAA_INTERRUPT_MASK_REG, 0);

}



void setDaaPcmStartCountRegs(unsigned int daa_dev)
{
	MV_DAA_DEV *pDaaDev = (MV_DAA_DEV*)daa_dev;
	writeDaaDirectReg(daa_dev, 34 , pDaaDev->txSample&0xff);
	writeDaaDirectReg(daa_dev, 35 , (pDaaDev->txSample>>8)&0x3);
	writeDaaDirectReg(daa_dev, 36 , pDaaDev->rxSample&0xff);
	writeDaaDirectReg(daa_dev, 37 , (pDaaDev->rxSample>>8)&0x3);

}

int checkDaaInterrupts(unsigned int daa_dev)
{
	unsigned char cause , mask, control, cause_and_mask;
	MV_DAA_DEV *pDaaDev = (MV_DAA_DEV *)daa_dev;


	cause = readDaaDirectReg(daa_dev, DAA_INTERRUPT_SOURCE_REG);
	mask = readDaaDirectReg(daa_dev, DAA_INTERRUPT_MASK_REG);
	cause_and_mask = cause & mask;


	/* check ring */
	if(cause_and_mask & DAA_RDTI)
	{
		MV_TRC_REC("*** Ring Detected ***\n");	
		pDaaDev->ring = 1;
		writeDaaDirectReg(daa_dev, DAA_INTERRUPT_MASK_REG, (mask & ~DAA_POLM));
		control = readDaaDirectReg(daa_dev, DAA_DAA_CONTROL_2);
		writeDaaDirectReg(daa_dev, DAA_DAA_CONTROL_2, (control & (~DAA_PDL)));	
	}

	/* check parallel phone detection */
	if(cause_and_mask & DAA_DODI)
	{
		MV_TRC_REC("*** Drop Out Detected ***\n");
		pDaaDev->drop_out = 1;
	}
	/* check reverse polarity */
	if(cause_and_mask & DAA_POLI)
	{
		MV_TRC_REC("*** Reverse Polarity Detected ***\n");
		pDaaDev->reverse_polarity = 1;	
	}

	/* clear interrupts */
	writeDaaDirectReg(daa_dev, DAA_INTERRUPT_SOURCE_REG, 0);

	
	if(cause_and_mask)	
		return 1;
	else
		return 0;
}


void setPstnState(long state, unsigned int daa_dev)
{
	MV_DAA_DEV *pDaaDev = (MV_DAA_DEV *)daa_dev;
	unsigned char reg3, reg5;

	if(state == PSTN_OFF_HOOK)
	{
		/* Set OFFHOOK */
		reg5 = readDaaDirectReg(daa_dev, DAA_DAA_CONTROL_1);
		writeDaaDirectReg(daa_dev, DAA_DAA_CONTROL_1,(reg5 | DAA_OH));
		pDaaDev->hook = 1; 	
	}
	else if(state == PSTN_ON_HOOK)
	{
		/* First disable Drop Out interrupt */
		reg3 = readDaaDirectReg(daa_dev, DAA_INTERRUPT_MASK_REG);
		writeDaaDirectReg(daa_dev, DAA_INTERRUPT_MASK_REG, (reg3 & ~DAA_DODM));
		/* Set ONHOOK */
		reg5 = readDaaDirectReg(daa_dev, DAA_DAA_CONTROL_1);		
		writeDaaDirectReg(daa_dev, DAA_DAA_CONTROL_1, (reg5 & ~DAA_OH));
		/* Enable Drop Out interrupt */
		writeDaaDirectReg(daa_dev, DAA_INTERRUPT_MASK_REG, reg3);
		pDaaDev->hook = 0;
	}
	else
		mvOsPrintf("Unknown PSTN state\n");
}

void daaCalibration(unsigned int daa_dev)
{
	unsigned char reg17;

	mvOsPrintf("Start ADC calibration\n");
	
	/* Set CALD bit */
	reg17 = readDaaDirectReg(daa_dev, DAA_INTERNATIONAL_CONTROL_2);
	writeDaaDirectReg(daa_dev, DAA_INTERNATIONAL_CONTROL_2, (reg17 | DAA_CALD));

	/* Set MCAL bit */
	reg17 = readDaaDirectReg(daa_dev, DAA_INTERNATIONAL_CONTROL_2);
	writeDaaDirectReg(daa_dev, DAA_INTERNATIONAL_CONTROL_2, (reg17 | DAA_MCAL));

	/* Reset MCAL bit - start ADC calibration */
	reg17 = readDaaDirectReg(daa_dev, DAA_INTERNATIONAL_CONTROL_2);
	writeDaaDirectReg(daa_dev, DAA_INTERNATIONAL_CONTROL_2, (reg17 & ~DAA_MCAL));

	/* Must wait at least 256 ms */
	mvOsDelay(300);

	mvOsPrintf("ADC calibration completed\n");
	


}

void setDaaCidState(long state, unsigned int daa_dev)
{
	unsigned char reg5;

	if(state == CID_ON)
	{
		/* Enable on-hook line monitor */
		writeDaaDirectReg(daa_dev, DAA_DAA_CONTROL_1, DAA_ONHM);
	}
	else if(state == CID_OFF)
	{
		reg5 = readDaaDirectReg(daa_dev, DAA_DAA_CONTROL_1);
		writeDaaDirectReg(daa_dev, DAA_DAA_CONTROL_1,(reg5 & ~DAA_ONHM));	
	}
	else
		mvOsPrintf("%s: Unknown CID state\n",__FUNCTION__);
		
}

void enableDaaReveresePolarity(long enable, unsigned int daa_dev)
{
	unsigned char mask;

	if(enable)
	{
		mask = readDaaDirectReg(daa_dev, DAA_INTERRUPT_MASK_REG);
		writeDaaDirectReg(daa_dev, DAA_INTERRUPT_MASK_REG, (mask | DAA_POLM));
	}
	else
	{
		mask = readDaaDirectReg(daa_dev, DAA_INTERRUPT_MASK_REG);
		writeDaaDirectReg(daa_dev, DAA_INTERRUPT_MASK_REG, (mask & ~DAA_POLM));

	}
	
}

void daaEventTypeGet(unsigned char *eventType, unsigned int daa_dev)
{	
	MV_DAA_DEV *pDaaDev = (MV_DAA_DEV *)daa_dev;

	if(pDaaDev->ring)
	{
		*eventType |= DAA_RDTI;
		pDaaDev->ring = 0;
	}
	
	if(pDaaDev->reverse_polarity)
	{
		*eventType |= DAA_POLI;
		pDaaDev->reverse_polarity = 0;
	}

	if(pDaaDev->drop_out)
	{
		*eventType |= DAA_DODI;
		pDaaDev->drop_out = 0;
	}


		
	return;
}


void setDaaDigitalHybrid(unsigned int index, unsigned int daa_dev)
{
	unsigned char reg16 = 0, reg26 = 0, reg30 = 0, reg31 = 0;
	
	/* Set On-hook speed, Ringer impedence, and ringer threshold */
	reg16 |= (daa_modes[index].ohs << 6);
	reg16 |= (daa_modes[index].rz << 1);
	reg16 |= (daa_modes[index].rt);
	writeDaaDirectReg(daa_dev, 16, reg16);
	
	/* Set DC Termination:
	   Tip/Ring voltage adjust, minimum operational current, current limitation */
	reg26 |= (daa_modes[index].dcv << 6);
	reg26 |= (daa_modes[index].mini << 4);
	reg26 |= (daa_modes[index].ilim << 1);
	writeDaaDirectReg(daa_dev, 26, reg26);

	/* Set AC Impedence */
	reg30 = (daa_modes[index].acim);
	writeDaaDirectReg(daa_dev, 30, reg30);

	/* Misc. DAA parameters */
	reg31 = 0xa3;
	reg31 |= (daa_modes[index].ohs2 << 3);
	writeDaaDirectReg(daa_dev, 31, reg31);

}


int daaGetLineVoltage(unsigned int daa_dev)
{
	return (int)(readDaaDirectReg(daa_dev, DAA_LINE_VOLTAGE_STATUS));

}

void daaFreeChannel(unsigned int daa_dev)
{
	MV_DAA_DEV *pDaaDev = (MV_DAA_DEV *)daa_dev;
	mvOsFree(pDaaDev);
}

void dumpDaaRegs(unsigned int daa_dev)
{
	int i;
	
	for(i=1; i < NUM_OF_REGS; i++)
	   MV_TRC_REC("Register %d: 0x%x\n",i, readDaaDirectReg(daa_dev, i));
}

