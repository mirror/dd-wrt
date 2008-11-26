
#include "voiceband/slic/proslic.h"
#include "voiceband/tdm/mvTdm.h"

extern MV_STATUS mvTdmSpiRead(MV_U32 val1, MV_U32 val2, MV_U32 cmd, MV_U8 cs, MV_U8 *data);
extern MV_STATUS mvTdmSpiWrite(MV_U32 val1, MV_U32 val2, MV_U32 cmd, MV_U8 cs);

#define TIMEOUTCODE (int) -1
char * exceptionStrings[] = 
{	"ProSLIC not communicating", 
	"Time out durring Power Up",
	"Time out durring Power Down",
	"Power is Leaking; might be a short",
	"Tip or Ring Ground Short",
	"Too Many Q1 Power Alarms",
	"Too Many Q2 Power Alarms",
	"Too Many Q3 Power Alarms", 
	"Too Many Q4 Power Alarms",
	"Too Many Q5 Power Alarms",
	"Too Many Q6 Power Alarms"
};

typedef struct _mv_slic_dev {
	unsigned char ch;
	unsigned char txSample;
	unsigned char rxSample;
	unsigned char dcval; /* daisy chain value */
}MV_SLIC_DEV;

static int work_mode;
static int interrupt_mode;

void exception (enum exceptions e)
{
	mvOsPrintf( "\n\tE X C E P T I O N: %s\n",exceptionStrings[e] );
	mvOsPrintf( " SLIC Interface Error\n");
}


int Si3215_Flag = 0;
int TestForSi3215(unsigned int slic_dev)
{
	return ((readDirectReg(slic_dev, 1) & 0x80)==0x80);
}

unsigned char readDirectReg(unsigned int slic_dev, unsigned char address)
{

	unsigned char data;
	MV_SLIC_DEV *pSlicDev = (MV_SLIC_DEV *)slic_dev;
	unsigned int val1, val2 = 0, cmd;

	address |= 0x80;

	if(work_mode) {
		val1 = (unsigned int)((address<<8) | pSlicDev->dcval);
		cmd = TRANSFER_BYTES(2) | ENDIANESS_MSB_MODE | RD_MODE | READ_1_BYTE | CLK_SPEED_LO_DIV;
	}
	else {
		val1 = address;
		cmd = TRANSFER_BYTES(1) | ENDIANESS_MSB_MODE | RD_MODE | READ_1_BYTE | CLK_SPEED_LO_DIV;
	}

	mvTdmSpiRead(val1, val2, cmd, pSlicDev->ch, &data);

	return data;
}

void writeDirectReg(unsigned int slic_dev, unsigned char address, unsigned char data)
{

	MV_SLIC_DEV *pSlicDev = (MV_SLIC_DEV *)slic_dev;
	unsigned int val1, val2 = 0, cmd;


	if(work_mode) {
		val1 = (unsigned int)((address<<8) | pSlicDev->dcval);
		val2 = data;
		cmd = TRANSFER_BYTES(3) | ENDIANESS_MSB_MODE | WR_MODE | CLK_SPEED_LO_DIV;
	}
	else {
		val1 = (data<<8) | address;
		cmd = TRANSFER_BYTES(2) | ENDIANESS_MSB_MODE | WR_MODE | CLK_SPEED_LO_DIV;
	}

	mvTdmSpiWrite(val1, val2, cmd, pSlicDev->ch);


}

void waitForIndirectReg(unsigned int slic_dev)
{
	while (readDirectReg(slic_dev, I_STATUS));
}

unsigned short readIndirectReg(unsigned int slic_dev, unsigned char address)
{ 
	if (Si3215_Flag) 
	{
		/* 
		** re-map the indirect registers for Si3215.
		*/
		if (address < 13 ) return 0;
		if ((address >= 13) && (address <= 40))
			address -= 13;
		else if ((address == 41) || (address == 43))
			address += 23;
		else if ((address >= 99) && (address <= 104))
			address -= 30;
	}

	waitForIndirectReg(slic_dev);
	writeDirectReg(slic_dev, IAA,address); 
	waitForIndirectReg(slic_dev);
	return ( readDirectReg(slic_dev, IDA_LO) | (readDirectReg (slic_dev, IDA_HI))<<8);
}

void writeIndirectReg(unsigned int slic_dev, unsigned char address, unsigned short data)
{
	if (Si3215_Flag)
	{
		/* 
		** re-map the indirect registers for Si3215.
		*/
		if (address < 13 ) return;
		if ((address >= 13) && (address <= 40))
			address -= 13;
		else if ((address == 41) || (address == 43))
			address += 23;
		else if ((address >= 99) && (address <= 104))
			address -= 30;
	}

	waitForIndirectReg(slic_dev);
	writeDirectReg(slic_dev, IDA_LO,(unsigned char)(data & 0xFF));
	writeDirectReg(slic_dev, IDA_HI,(unsigned char)((data & 0xFF00)>>8));
	writeDirectReg(slic_dev, IAA,address);
}


/*
The following Array contains:
*/
indirectRegister  indirectRegisters[] =
{
/* Reg#			Label		Initial Value  */

{	0,	"DTMF_ROW_0_PEAK",	0x55C2	},
{	1,	"DTMF_ROW_1_PEAK",	0x51E6	},
{	2,	"DTMF_ROW2_PEAK",	0x4B85	},
{	3,	"DTMF_ROW3_PEAK",	0x4937	},
{	4,	"DTMF_COL1_PEAK",	0x3333	},
{	5,	"DTMF_FWD_TWIST",	0x0202	},
{	6,	"DTMF_RVS_TWIST",	0x0202	},
{	7,	"DTMF_ROW_RATIO",	0x0198	},
{	8,	"DTMF_COL_RATIO",	0x0198	},
{	9,	"DTMF_ROW_2ND_ARM",	0x0611	},
{	10,	"DTMF_COL_2ND_ARM",	0x0202	},
{	11,	"DTMF_PWR_MIN_",	0x00E5	},
{	12,	"DTMF_OT_LIM_TRES",	0x0A1C	},
{	13,	"OSC1_COEF",		0x7b30	},
{	14,	"OSC1X",			0x0063	},
{	15,	"OSC1Y",			0x0000	},
{	16,	"OSC2_COEF",		0x7870	},
{	17,	"OSC2X",			0x007d	},
{	18,	"OSC2Y",			0x0000	},
{	19,	"RING_V_OFF",		0x0000	},
{	20,	"RING_OSC",			0x7EF0	},
{	21,	"RING_X",			0x0160	},
{	22,	"RING_Y",			0x0000	},
{	23,	"PULSE_ENVEL",		0x2000	},
{	24,	"PULSE_X",			0x2000	},
{	25,	"PULSE_Y",			0x0000	},
{	26,	"RECV_DIGITAL_GAIN",0x4000	},
{	27,	"XMIT_DIGITAL_GAIN",0x4000	},
{	28,	"LOOP_CLOSE_TRES",	0x1000	},
{	29,	"RING_TRIP_TRES",	0x3600	},
{	30,	"COMMON_MIN_TRES",	0x1000	},
{	31,	"COMMON_MAX_TRES",	0x0200	},
{	32,	"PWR_ALARM_Q1Q2",	0x7c0   },
{	33,	"PWR_ALARM_Q3Q4",	0x2600	},
{	34,	"PWR_ALARM_Q5Q6",	0x1B80	},
{	35,	"LOOP_CLSRE_FlTER",	0x8000	},
{	36,	"RING_TRIP_FILTER",	0x0320	},
{	37,	"TERM_LP_POLE_Q1Q2",0x08c	},
{	38,	"TERM_LP_POLE_Q3Q4",0x0100	},
{	39,	"TERM_LP_POLE_Q5Q6",0x0010	},
{	40,	"CM_BIAS_RINGING",	0x0C00	},
{	41,	"DCDC_MIN_V",		0x0C00	},
{	43,	"LOOP_CLOSE_TRES Low",	0x1000	},
{	99,	"FSK 0 FREQ PARAM",	0x00DA	},
{	100,"FSK 0 AMPL PARAM",	0x6B60	},
{	101,"FSK 1 FREQ PARAM",	0x0074	},
{	102,"FSK 1 AMPl PARAM",	0x79C0	},
{	103,"FSK 0to1 SCALER",	0x1120	},
{	104,"FSK 1to0 SCALER",	0x3BE0	},
{	97,	"RCV_FLTR",			0},
{	0,"",0},
};


void stopTone(unsigned int slic_dev) 
{
  writeDirectReg(slic_dev, 32, INIT_DR32	);//0x00	Oper. Oscillator 1 Controltone generation
  writeDirectReg(slic_dev, 33, INIT_DR33	);//0x00	Oper. Oscillator 2 Controltone generation

  writeIndirectReg(slic_dev, 13,INIT_IR13);
  writeIndirectReg(slic_dev, 14,INIT_IR14);
  writeIndirectReg(slic_dev, 16,INIT_IR16);
  writeIndirectReg(slic_dev, 17,INIT_IR17);
  writeDirectReg(slic_dev, 36,  INIT_DR36);
  writeDirectReg(slic_dev, 37,  INIT_DR37);
  writeDirectReg(slic_dev, 38,  INIT_DR38);
  writeDirectReg(slic_dev, 39,  INIT_DR39);
  writeDirectReg(slic_dev, 40,  INIT_DR40);
  writeDirectReg(slic_dev, 41,  INIT_DR41);
  writeDirectReg(slic_dev, 42,  INIT_DR42);
  writeDirectReg(slic_dev, 43,  INIT_DR43);

//  writeDirectReg(slic_dev, 32, INIT_DR32	);//0x00	Oper. Oscillator 1 Controltone generation
//  writeDirectReg(slic_dev, 33, INIT_DR33	);//0x00	Oper. Oscillator 2 Controltone generation
}

void genTone(tone_struct tone, unsigned int slic_dev) 
{ 
	// Uber function to extract values for oscillators from tone_struct
	// place them in the registers, and enable the oscillators.
	unsigned char osc1_ontimer_enable=0, osc1_offtimer_enable=0,osc2_ontimer_enable=0,osc2_offtimer_enable=0;
	int enable_osc2=0;

	//loopBackOff();
	disableOscillators(slic_dev); // Make sure the oscillators are not already on.

	if (tone.osc1.coeff == 0 || tone.osc1.x == 0) {
		// Error!
		mvOsPrintf("You passed me a invalid struct!\n");
		return;
	}
	// Setup osc 1
	writeIndirectReg(slic_dev,  OSC1_COEF, tone.osc1.coeff);
	writeIndirectReg(slic_dev,  OSC1X, tone.osc1.x);
	writeIndirectReg(slic_dev,  OSC1Y, tone.osc1.y);
	//mvOsPrintf("OUt-> 0x%04x\n",tone.osc1.coeff);
	// Active Timer


	if (tone.osc1.on_hi_byte != 0) {
		writeDirectReg(slic_dev,  OSC1_ON__LO, tone.osc1.on_low_byte);
		writeDirectReg(slic_dev,  OSC1_ON_HI, tone.osc1.on_hi_byte);
		osc1_ontimer_enable = 0x10;
	}
	// Inactive Timer
	if (tone.osc1.off_hi_byte != 0) {
		writeDirectReg(slic_dev,  OSC1_OFF_LO, tone.osc1.off_low_byte);
		writeDirectReg(slic_dev,  OSC1_OFF_HI, tone.osc1.off_hi_byte);
		osc1_offtimer_enable = 0x08;
	}
	
	if (tone.osc2.coeff != 0) {
		// Setup OSC 2
		writeIndirectReg(slic_dev,  OSC2_COEF, tone.osc2.coeff);
		writeIndirectReg(slic_dev,  OSC2X, tone.osc2.x);
		writeIndirectReg(slic_dev,  OSC2Y, tone.osc2.y);
		
		// Active Timer
		if (tone.osc1.on_hi_byte != 0) {
			writeDirectReg(slic_dev,  OSC2_ON__LO, tone.osc2.on_low_byte);
			writeDirectReg(slic_dev,  OSC2_ON_HI, tone.osc2.on_hi_byte);
			osc2_ontimer_enable = 0x10;
		}
		// Inactive Timer
		if (tone.osc1.off_hi_byte != 0) {
			writeDirectReg(slic_dev,  OSC2_OFF_LO, tone.osc2.off_low_byte);
			writeDirectReg(slic_dev,  OSC2_OFF_HI, tone.osc2.off_hi_byte);
			osc2_offtimer_enable = 0x08;
		}
		enable_osc2 = 1;
	}

	writeDirectReg(slic_dev,  OSC1, (unsigned char)(0x06 | osc1_ontimer_enable | osc1_offtimer_enable));
	if (enable_osc2)
		writeDirectReg(slic_dev,  OSC2, (unsigned char)(0x06 | osc2_ontimer_enable | osc2_offtimer_enable));
	return;
}


void dialTone(unsigned int slic_dev)
{
  if (Si3215_Flag)
  {
	  writeIndirectReg(slic_dev, 13,SI3215_DIALTONE_IR13);
	  writeIndirectReg(slic_dev, 14,SI3215_DIALTONE_IR14);
	  writeIndirectReg(slic_dev, 16,SI3215_DIALTONE_IR16);
	  writeIndirectReg(slic_dev, 17,SI3215_DIALTONE_IR17);
  }
  else
  {
	  writeIndirectReg(slic_dev, 13,SI3210_DIALTONE_IR13);
	  writeIndirectReg(slic_dev, 14,SI3210_DIALTONE_IR14);
	  writeIndirectReg(slic_dev, 16,SI3210_DIALTONE_IR16);
	  writeIndirectReg(slic_dev, 17,SI3210_DIALTONE_IR17);
  }

  writeDirectReg(slic_dev, 36,  DIALTONE_DR36);
  writeDirectReg(slic_dev, 37,  DIALTONE_DR37);
  writeDirectReg(slic_dev, 38,  DIALTONE_DR38);
  writeDirectReg(slic_dev, 39,  DIALTONE_DR39);
  writeDirectReg(slic_dev, 40,  DIALTONE_DR40);
  writeDirectReg(slic_dev, 41,  DIALTONE_DR41);
  writeDirectReg(slic_dev, 42,  DIALTONE_DR42);
  writeDirectReg(slic_dev, 43,  DIALTONE_DR43);

  writeDirectReg(slic_dev, 32,  DIALTONE_DR32);
  writeDirectReg(slic_dev, 33,  DIALTONE_DR33);
}


void busyTone(unsigned int slic_dev)
{
  if (Si3215_Flag)
  {
	  writeIndirectReg(slic_dev, 13,SI3215_BUSYTONE_IR13);
	  writeIndirectReg(slic_dev, 14,SI3215_BUSYTONE_IR14);
	  writeIndirectReg(slic_dev, 16,SI3215_BUSYTONE_IR16);
	  writeIndirectReg(slic_dev, 17,SI3215_BUSYTONE_IR17);
  }
  else
  {
	  writeIndirectReg(slic_dev, 13,SI3210_BUSYTONE_IR13);
	  writeIndirectReg(slic_dev, 14,SI3210_BUSYTONE_IR14);
	  writeIndirectReg(slic_dev, 16,SI3210_BUSYTONE_IR16);
	  writeIndirectReg(slic_dev, 17,SI3210_BUSYTONE_IR17);
  }
  writeDirectReg(slic_dev, 36,  BUSYTONE_DR36);
  writeDirectReg(slic_dev, 37,  BUSYTONE_DR37);
  writeDirectReg(slic_dev, 38,  BUSYTONE_DR38);
  writeDirectReg(slic_dev, 39,  BUSYTONE_DR39);
  writeDirectReg(slic_dev, 40,  BUSYTONE_DR40);
  writeDirectReg(slic_dev, 41,  BUSYTONE_DR41);
  writeDirectReg(slic_dev, 42,  BUSYTONE_DR42);
  writeDirectReg(slic_dev, 43,  BUSYTONE_DR43);
  
  writeDirectReg(slic_dev, 32,  BUSYTONE_DR32);
  writeDirectReg(slic_dev, 33,  BUSYTONE_DR33);
}

void reorderTone(unsigned int slic_dev)
{
  if (Si3215_Flag)
  {
	  writeIndirectReg(slic_dev, 13,SI3215_REORDERTONE_IR13);
	  writeIndirectReg(slic_dev, 14,SI3215_REORDERTONE_IR14);
	  writeIndirectReg(slic_dev, 16,SI3215_REORDERTONE_IR16);
	  writeIndirectReg(slic_dev, 17,SI3215_REORDERTONE_IR17);
  }
  else {
	  writeIndirectReg(slic_dev, 13,SI3210_REORDERTONE_IR13);
	  writeIndirectReg(slic_dev, 14,SI3210_REORDERTONE_IR14);
	  writeIndirectReg(slic_dev, 16,SI3210_REORDERTONE_IR16);
	  writeIndirectReg(slic_dev, 17,SI3210_REORDERTONE_IR17);
  }

  writeDirectReg(slic_dev, 36,  REORDERTONE_DR36);
  writeDirectReg(slic_dev, 37,  REORDERTONE_DR37);
  writeDirectReg(slic_dev, 38,  REORDERTONE_DR38);
  writeDirectReg(slic_dev, 39,  REORDERTONE_DR39);
  writeDirectReg(slic_dev, 40,  REORDERTONE_DR40);
  writeDirectReg(slic_dev, 41,  REORDERTONE_DR41);
  writeDirectReg(slic_dev, 42,  REORDERTONE_DR42);
  writeDirectReg(slic_dev, 43,  REORDERTONE_DR43);
  
  writeDirectReg(slic_dev, 32,  REORDERTONE_DR32);
  writeDirectReg(slic_dev, 33,  REORDERTONE_DR33);
 
}

void congestionTone(unsigned int slic_dev)
{
  if (Si3215_Flag)
  {
	  writeIndirectReg(slic_dev, 13,SI3215_CONGESTIONTONE_IR13);
	  writeIndirectReg(slic_dev, 14,SI3215_CONGESTIONTONE_IR14);
	  writeIndirectReg(slic_dev, 16,SI3215_CONGESTIONTONE_IR16);
	  writeIndirectReg(slic_dev, 17,SI3215_CONGESTIONTONE_IR17);
  }
  else {
	  writeIndirectReg(slic_dev, 13,SI3210_CONGESTIONTONE_IR13);
	  writeIndirectReg(slic_dev, 14,SI3210_CONGESTIONTONE_IR14);
	  writeIndirectReg(slic_dev, 16,SI3210_CONGESTIONTONE_IR16);
	  writeIndirectReg(slic_dev, 17,SI3210_CONGESTIONTONE_IR17);
  }

  writeDirectReg(slic_dev, 36,  CONGESTIONTONE_DR36);
  writeDirectReg(slic_dev, 37,  CONGESTIONTONE_DR37);
  writeDirectReg(slic_dev, 38,  CONGESTIONTONE_DR38);
  writeDirectReg(slic_dev, 39,  CONGESTIONTONE_DR39);
  writeDirectReg(slic_dev, 40,  CONGESTIONTONE_DR40);
  writeDirectReg(slic_dev, 41,  CONGESTIONTONE_DR41);
  writeDirectReg(slic_dev, 42,  CONGESTIONTONE_DR42);
  writeDirectReg(slic_dev, 43,  CONGESTIONTONE_DR43);

  writeDirectReg(slic_dev, 32,  CONGESTIONTONE_DR32);
  writeDirectReg(slic_dev, 33,  CONGESTIONTONE_DR33);

}

void ringbackPbxTone(unsigned int slic_dev)
{
  if (Si3215_Flag)
  {
	  writeIndirectReg(slic_dev, 13,SI3215_RINGBACKPBXTONE_IR13);
	  writeIndirectReg(slic_dev, 14,SI3215_RINGBACKPBXTONE_IR14);
	  writeIndirectReg(slic_dev, 16,SI3215_RINGBACKPBXTONE_IR16);
	  writeIndirectReg(slic_dev, 17,SI3215_RINGBACKPBXTONE_IR17);
  }
  else {
	  writeIndirectReg(slic_dev, 13,SI3210_RINGBACKPBXTONE_IR13);
	  writeIndirectReg(slic_dev, 14,SI3210_RINGBACKPBXTONE_IR14);
	  writeIndirectReg(slic_dev, 16,SI3210_RINGBACKPBXTONE_IR16);
	  writeIndirectReg(slic_dev, 17,SI3210_RINGBACKPBXTONE_IR17);
  }

  writeDirectReg(slic_dev, 36,  RINGBACKPBXTONE_DR36);
  writeDirectReg(slic_dev, 37,  RINGBACKPBXTONE_DR37);
  writeDirectReg(slic_dev, 38,  RINGBACKPBXTONE_DR38);
  writeDirectReg(slic_dev, 39,  RINGBACKPBXTONE_DR39);
  writeDirectReg(slic_dev, 40,  RINGBACKPBXTONE_DR40);
  writeDirectReg(slic_dev, 41,  RINGBACKPBXTONE_DR41);
  writeDirectReg(slic_dev, 42,  RINGBACKPBXTONE_DR42);
  writeDirectReg(slic_dev, 43,  RINGBACKPBXTONE_DR43);

  writeDirectReg(slic_dev, 32,  RINGBACKPBXTONE_DR32);
  writeDirectReg(slic_dev, 33,  RINGBACKPBXTONE_DR33);

}


void ringBackTone(unsigned int slic_dev)
{
  if (Si3215_Flag)
  {
	  writeIndirectReg(slic_dev, 13,SI3215_RINGBACKTONE_IR13);
	  writeIndirectReg(slic_dev, 14,SI3215_RINGBACKTONE_IR14);
	  writeIndirectReg(slic_dev, 16,SI3215_RINGBACKTONE_IR16);
	  writeIndirectReg(slic_dev, 17,SI3215_RINGBACKTONE_IR17);
  }
  else {
	  writeIndirectReg(slic_dev, 13,SI3210_RINGBACKTONE_IR13);
	  writeIndirectReg(slic_dev, 14,SI3210_RINGBACKTONE_IR14);
	  writeIndirectReg(slic_dev, 16,SI3210_RINGBACKTONE_IR16);
	  writeIndirectReg(slic_dev, 17,SI3210_RINGBACKTONE_IR17);
  }

  writeDirectReg(slic_dev, 36,  RINGBACKTONE_DR36);
  writeDirectReg(slic_dev, 37,  RINGBACKTONE_DR37);
  writeDirectReg(slic_dev, 38,  RINGBACKTONE_DR38);
  writeDirectReg(slic_dev, 39,  RINGBACKTONE_DR39);
  writeDirectReg(slic_dev, 40,  RINGBACKTONE_DR40);
  writeDirectReg(slic_dev, 41,  RINGBACKTONE_DR41);
  writeDirectReg(slic_dev, 42,  RINGBACKTONE_DR42);
  writeDirectReg(slic_dev, 43,  RINGBACKTONE_DR43);
  
  writeDirectReg(slic_dev, 32,  RINGBACKTONE_DR32);
  writeDirectReg(slic_dev, 33,  RINGBACKTONE_DR33);
 
}

void ringBackToneSi3216(unsigned int slic_dev)
{
  writeIndirectReg(slic_dev, 13,RINGBACKTONE_SI3216_IR13);
  writeIndirectReg(slic_dev, 14,RINGBACKTONE_SI3216_IR14);
  writeIndirectReg(slic_dev, 16,RINGBACKTONE_SI3216_IR16);
  writeIndirectReg(slic_dev, 17,RINGBACKTONE_SI3216_IR17);
  writeDirectReg(slic_dev, 36,  RINGBACKTONE_SI3216_DR36);
  writeDirectReg(slic_dev, 37,  RINGBACKTONE_SI3216_DR37);
  writeDirectReg(slic_dev, 38,  RINGBACKTONE_SI3216_DR38);
  writeDirectReg(slic_dev, 39,  RINGBACKTONE_SI3216_DR39);
  writeDirectReg(slic_dev, 40,  RINGBACKTONE_SI3216_DR40);
  writeDirectReg(slic_dev, 41,  RINGBACKTONE_SI3216_DR41);
  writeDirectReg(slic_dev, 42,  RINGBACKTONE_SI3216_DR42);
  writeDirectReg(slic_dev, 43,  RINGBACKTONE_SI3216_DR43);
  
  writeDirectReg(slic_dev, 32,  RINGBACKTONE_SI3216_DR32);
  writeDirectReg(slic_dev, 33,  RINGBACKTONE_SI3216_DR33);
 
}


void ringBackJapanTone(unsigned int slic_dev)
{
  if (Si3215_Flag)
  {
	  writeIndirectReg(slic_dev, 13,SI3215_RINGBACKJAPANTONE_IR13);
	  writeIndirectReg(slic_dev, 14,SI3215_RINGBACKJAPANTONE_IR14);
	  writeIndirectReg(slic_dev, 16,SI3215_RINGBACKJAPANTONE_IR16);
	  writeIndirectReg(slic_dev, 17,SI3215_RINGBACKJAPANTONE_IR17);
  }
  else {
	  writeIndirectReg(slic_dev, 13,SI3210_RINGBACKJAPANTONE_IR13);
	  writeIndirectReg(slic_dev, 14,SI3210_RINGBACKJAPANTONE_IR14);
	  writeIndirectReg(slic_dev, 16,SI3210_RINGBACKJAPANTONE_IR16);
	  writeIndirectReg(slic_dev, 17,SI3210_RINGBACKJAPANTONE_IR17);
  }

  writeDirectReg(slic_dev, 36,  RINGBACKJAPANTONE_DR36);
  writeDirectReg(slic_dev, 37,  RINGBACKJAPANTONE_DR37);
  writeDirectReg(slic_dev, 38,  RINGBACKJAPANTONE_DR38);
  writeDirectReg(slic_dev, 39,  RINGBACKJAPANTONE_DR39);
  writeDirectReg(slic_dev, 40,  RINGBACKJAPANTONE_DR40);
  writeDirectReg(slic_dev, 41,  RINGBACKJAPANTONE_DR41);
  writeDirectReg(slic_dev, 42,  RINGBACKJAPANTONE_DR42);
  writeDirectReg(slic_dev, 43,  RINGBACKJAPANTONE_DR43);
  
  writeDirectReg(slic_dev, 32,  RINGBACKJAPANTONE_DR32);
  writeDirectReg(slic_dev, 33,  RINGBACKJAPANTONE_DR33);
 
}


void busyJapanTone(unsigned int slic_dev)
{
  if (Si3215_Flag)
  {
	  writeIndirectReg(slic_dev, 13,SI3215_BUSYJAPANTONE_IR13);
	  writeIndirectReg(slic_dev, 14,SI3215_BUSYJAPANTONE_IR14);
	  writeIndirectReg(slic_dev, 16,SI3215_BUSYJAPANTONE_IR16);
	  writeIndirectReg(slic_dev, 17,SI3215_BUSYJAPANTONE_IR17);
  }
  else {
	  writeIndirectReg(slic_dev, 13,SI3210_BUSYJAPANTONE_IR13);
	  writeIndirectReg(slic_dev, 14,SI3210_BUSYJAPANTONE_IR14);
	  writeIndirectReg(slic_dev, 16,SI3210_BUSYJAPANTONE_IR16);
	  writeIndirectReg(slic_dev, 17,SI3210_BUSYJAPANTONE_IR17);
  }

  writeDirectReg(slic_dev, 36,  BUSYJAPANTONE_DR36);
  writeDirectReg(slic_dev, 37,  BUSYJAPANTONE_DR37);
  writeDirectReg(slic_dev, 38,  BUSYJAPANTONE_DR38);
  writeDirectReg(slic_dev, 39,  BUSYJAPANTONE_DR39);
  writeDirectReg(slic_dev, 40,  BUSYJAPANTONE_DR40);
  writeDirectReg(slic_dev, 41,  BUSYJAPANTONE_DR41);
  writeDirectReg(slic_dev, 42,  BUSYJAPANTONE_DR42);
  writeDirectReg(slic_dev, 43,  BUSYJAPANTONE_DR43);
  
  writeDirectReg(slic_dev, 32,  BUSYJAPANTONE_DR32);
  writeDirectReg(slic_dev, 33,  BUSYJAPANTONE_DR33);
 
}

void standardRinging(unsigned int slic_dev) { 	
	// Enables ringing mode on ProSlic for standard North American ring
	//	RING_ON__LO	48
	//	RING_ON_HI	49
	//	RING_OFF_LO	50
	//	RING_OFF_HI	51
	// Active Timer

	writeDirectReg(slic_dev,  RING_ON__LO, 0x80); // low reg 48
	writeDirectReg(slic_dev,  RING_ON_HI, 0x3E); // hi reg 49
	// Inactive Timer
	writeDirectReg(slic_dev,  RING_OFF_LO, 0x00); // low reg 50
	writeDirectReg(slic_dev,  RING_OFF_HI, 0x7D); // hi reg 51
	// Enable timers for ringing oscillator
	writeDirectReg(slic_dev,  34, 0x18);

}

unsigned char powerUp(unsigned int slic_dev)
{ 
	unsigned char vBat ; 
	int i=0;


	if (chipType(slic_dev) == 3)  // M version correction
	{
		writeDirectReg(slic_dev, 92,INIT_SI3210M_DR92);// M version
		writeDirectReg(slic_dev, 93,INIT_SI3210M_DR93);// M version
	}
	else	
	{
		/* MA */
		writeDirectReg(slic_dev, 93, 12); 
		writeDirectReg(slic_dev, 92, 0xff); /* set the period of the DC-DC converter to 1/64 kHz  START OUT SLOW*/

	}

	writeDirectReg(slic_dev, 14, 0); /* Engage the DC-DC converter */
  
	while ((vBat=readDirectReg(slic_dev, 82)) < 0xc0)
	{ 
		mvOsDelay (1000);
		++i;
		if (i > 200) return 0;
	}
  	
	if (chipType(slic_dev) == 3)  // M version correction
	{
		writeDirectReg(slic_dev, 92,0x80 +INIT_SI3210M_DR92);// M version
	}
	else
		writeDirectReg(slic_dev, 93, 0x8c);  /* DC-DC Calibration  */ /* MA */

	while(0x80 & readDirectReg(slic_dev, 93));  // Wait for DC-DC Calibration to complete

	return vBat;
}
 
unsigned char powerLeakTest(unsigned int slic_dev)
{ 
	unsigned char vBat ;
	
	writeDirectReg(slic_dev, 64,0);

	writeDirectReg(slic_dev, 14, 0x10); 
	mvOsDelay (1000);

	if ((vBat=readDirectReg(slic_dev, 82)) < 0x4)  // 6 volts
	 return 0;

	return vBat;
}


void xcalibrate(unsigned int slic_dev)
{ unsigned char i ;

	writeDirectReg(slic_dev, 97, 0x1e); /* Do all of the Calibarations */
	writeDirectReg(slic_dev, 96, 0x47); /* Start the calibration No Speed Up */
	
	manualCalibrate(slic_dev);

	writeDirectReg(slic_dev, 23,4);  // enable interrupt for the balance Cal
	writeDirectReg(slic_dev, 97,0x1);
	writeDirectReg(slic_dev, 96,0x40);


	while (readDirectReg(slic_dev, 96) != 0 );
	mvOsPrintf("\nCalibration Vector Registers 98 - 107: ");
	
	for (i=98; i < 107; i++)  mvOsPrintf( "%X.", readDirectReg(slic_dev, i));
	mvOsPrintf("%X \n\n",readDirectReg(slic_dev, 107));

}

void goActive(unsigned int slic_dev)

{
	writeDirectReg(slic_dev, 64,1);	/* LOOP STATE REGISTER SET TO ACTIVE */
				/* Active works for on-hook and off-hook see spec. */
				/* The phone hook-switch sets the off-hook and on-hook substate*/
}


unsigned char version(unsigned int slic_dev)
{
	return 0xf & readDirectReg(slic_dev, 0); 
}

unsigned char chipType (unsigned int slic_dev)
{
	return (0x30 & readDirectReg(slic_dev, 0)) >> 4; 
}

unsigned char family (unsigned int slic_dev)
{
        return (readDirectReg (slic_dev, 1) & 0x80);
}

int selfTest(unsigned int slic_dev)
{
	/*  Begin Sanity check  Optional */
	if (readDirectReg(slic_dev, 8) !=2) {exception(PROSLICiNSANE); return 0;}
	if (readDirectReg(slic_dev, 64) !=0) {exception(PROSLICiNSANE); return 0;}
	if (readDirectReg(slic_dev, 11) !=0x33) {exception(PROSLICiNSANE); return 0;}
	/* End Sanity check */
	return 1;
}

int slicStart(unsigned int slic_dev)
{
	volatile unsigned char t,v;
	volatile unsigned short i;

	// Test if Si3210 or Si3215 is used.
	Si3215_Flag = TestForSi3215(slic_dev);

	if (Si3215_Flag)
		mvOsPrintf ("Proslic Si3215 Detected.\n");
	else
		mvOsPrintf ("Proslic Si3210 detected.\n");

	/*  Another Quarter of a Second */
	if (!selfTest(slic_dev))
		return 0;

	v = version(slic_dev);
	mvOsPrintf("Si321x Vesion = %x\n", v);

	t = chipType(slic_dev);
	mvOsPrintf("chip Type t=%d\n", t);

	initializeIndirectRegisters(slic_dev);

	i=verifyIndirectRegisters (slic_dev);
	if (i != 0) {
		mvOsPrintf ("verifyIndirect failed\n");
		return 0;
	}

	writeDirectReg (slic_dev, 8, 0);
	if (v == 5)
		writeDirectReg (slic_dev, 108, 0xeb); /* turn on Rev E features. */

	
	if (   t == 0 ) // Si3210 not the Si3211 or Si3212	
	{
		writeDirectReg(slic_dev, 67,0x17); // Make VBat switch not automatic 
		// The above is a saftey measure to prevent Q7 from accidentaly turning on and burning out.
		//  It works in combination with the statement below.  Pin 34 DCDRV which is used for the battery switch on the
		//  Si3211 & Si3212 
	
		writeDirectReg(slic_dev, 66,1);  //    Q7 should be set to OFF for si3210
	}

	if (v <=2 ) {  //  REVISION B   
		writeDirectReg(slic_dev, 73,4);  // set common mode voltage to 6 volts
	}

	// PCm Start Count. 
	writeDirectReg (slic_dev, 2, 0);
	writeDirectReg (slic_dev, 4, 0);


	/* Do Flush durring powerUp and calibrate */
	if (t == 0 || t==3) //  Si3210
	{
		mvOsPrintf("Powerup the Slic\n");

		// Turn on the DC-DC converter and verify voltage.
		if (!powerUp(slic_dev))
			return 0;
	}

	initializeDirectRegisters(slic_dev);

	if (!calibrate(slic_dev)) {
		mvOsPrintf("calibrate failed\n");
		return 0;
	}

	mvOsPrintf ("Register 98 = %x\n", readDirectReg(slic_dev, 98));
	mvOsPrintf ("Register 99 = %x\n", readDirectReg(slic_dev, 99));

	clearInterrupts(slic_dev);

	mvOsPrintf("Slic is Active\n");
	goActive(slic_dev);

//#define TORTURE_INTERFACE_TEST
#ifdef TORTURE_INTERFACE_TEST
#define SPI_ITERATIONS 10000
{
	int j, i=0;
	volatile unsigned char WriteIn, ReadBack;
	mvOsPrintf("SPI test Write/Read %d times...\n", SPI_ITERATIONS);
	for (j=0,i=0;j<SPI_ITERATIONS;j++,i++) {
		i &= 0xff;
//		mvOsPrintf("Writing %d\n",i);
		writeDirectReg (2, i);
		mvOsDelay( 1 );
		ReadBack = readDirectReg(slic_dev, 2);
		if (ReadBack != i) {
			mvOsPrintf("Wrote %x, ReadBack = %x \n", i, ReadBack);
			break;
		}
/*		
		i++;
		i = i & 0xff;
*/
	}
	if(j == SPI_ITERATIONS)
		mvOsPrintf("SPI test ok\n");
	else
		mvOsPrintf("SPI test failed\n");
}	
#endif


#ifdef SLIC_TEST_LOOPSTATUS_AND_DIGIT_DETECTION
{
	volatile unsigned char WriteIn, ReadBack;
	while (1) {
		volatile int HookStatus;
		volatile short ReadBack, CurrentReadBack;
		if (loopStatus () != HookStatus) {
			HookStatus = loopStatus();
			mvOsPrintf ("HOOK Status = %x\n", HookStatus);
		}
		ReadBack = readDirectReg(slic_dev, 64);
		if (ReadBack != CurrentReadBack) {
			mvOsPrintf ("ReadBack = %x\n", ReadBack);
			CurrentReadBack = ReadBack;
		}

		/*
		** Check digit detection.
		*/
		digit_decode_status = digit() ;
		if (digit_decode_status & DTMF_DETECT) {

			/* compare with the last detect status. */
			if ((digit_decode_status & DTMF_DIGIT_MASK) != FXS_Signaling_current_digit) {

				mvOsPrintf("Digit %x detected.\n", digit_decode_status & 0xF);

				// New digit detected.
				FXS_Signaling_current_digit =  (digit_decode_status & DTMF_DIGIT_MASK);
			}
		}
		else
			FXS_Signaling_current_digit = DTMF_NONE;
	}
}
#endif

	return 1;

}


void stopRinging(unsigned int slic_dev)
{
	
	if ((0xf & readDirectReg(slic_dev, 0))<=2 )  // if REVISION B  
        writeDirectReg(slic_dev, 69,10);   // Loop Debounce Register  = initial value
    
	goActive(slic_dev);
	
}

unsigned short manualCalibrate(unsigned int slic_dev)
{ 
unsigned char x,y,i,progress=0; // progress contains individual bits for the Tip and Ring Calibrations

	//Initialized DR 98 and 99 to get consistant results.
	// 98 and 99 are the results registers and the search should have same intial conditions.
	writeDirectReg(slic_dev, 98,0x10); // This is necessary if the calibration occurs other than at reset time
	writeDirectReg(slic_dev, 99,0x10);

	for ( i=0x1f; i>0; i--)
	{
		writeDirectReg(slic_dev, 98,i);
		mvOsDelay(200);
		if((readDirectReg(slic_dev, 88)) == 0)
		{	progress|=1;
		x=i;
		break;
		}
	} // for



	for ( i=0x1f; i>0; i--)
	{
		writeDirectReg(slic_dev, 99,i);
		mvOsDelay(200);
		if((readDirectReg(slic_dev, 89)) == 0){
			progress|=2;
			y=i;
		break;
		}
	
	}//for

	return progress;
}


void cadenceRingPhone(ringStruct ringRegs, unsigned int slic_dev) { 
	
	// Enables ringing mode on ProSlic for standard North American ring
	//	RING_ON__LO	48
	//	RING_ON_HI	49
	//	RING_OFF_HI	51
	//	RING_OFF_LO	50


	mvOsPrintf ("\n on= %d ms off =%d ms", ringRegs.u1.onTime/8, ringRegs.u2.offTime/8);
	writeDirectReg(slic_dev,  RING_ON__LO,  ringRegs.u1.s1.onLowByte); // lo reg 48
	writeDirectReg(slic_dev,  RING_ON_HI,   ringRegs.u1.s1.onHiByte); // hi reg 49
	// Inactive Timer
	writeDirectReg(slic_dev,  RING_OFF_LO, ringRegs.u2.s2.offLowByte); // low reg 50
	writeDirectReg(slic_dev,  RING_OFF_HI, ringRegs.u2.s2.offHiByte); // hi reg 51
	// Enable timers for ringing oscillator
		writeDirectReg(slic_dev,  34, 0x18);
	}


int groundShort(unsigned int slic_dev)
{ 
	int rc;

	rc= ( (readDirectReg(slic_dev, 80) < 2) || readDirectReg(slic_dev, 81) < 2);
		
		if (rc) {
			exception(TIPoRrINGgROUNDsHORT);
			return -1;
		}
		return rc;
}

void clearAlarmBits(unsigned int slic_dev)
{
	writeDirectReg(slic_dev, 19,0xFC); //Clear Alarm bits
}


unsigned long interruptBits (unsigned int slic_dev) {
	// Determines which interrupt bit is set
//	int  count = 1;
//	unsigned char j = 0x01 ;
	union {
		unsigned char reg_data[3];
		long interrupt_bits;
	} u ;
	u.interrupt_bits=0;
	
// ONLY CLEAR the ACTIVE INTERRUPT or YOU WILL CREATE CRITICAL SECTION ERROR of LEAVING
// THE TIME OPEN BETWEEN THE Nth BIT and the N+1thbit within the same BYTE.
// eg. if the inactive oscillators are firing at nearly the same time
// you would only see one.


	u.reg_data[0] = readDirectReg(slic_dev,  18);
	writeDirectReg(slic_dev,  18,u.reg_data[0]);

	u.reg_data[1] = readDirectReg(slic_dev,  19);
	if ((u.reg_data[1]) && (0xfc != 0)) {
		mvOsPrintf ("Power alarm = %x\n",(u.reg_data[1] & 0xfc));
		clearAlarmBits (slic_dev);
	} 
	writeDirectReg(slic_dev,  19,u.reg_data[1] );

	u.reg_data[2] = readDirectReg(slic_dev,  20);
	writeDirectReg(slic_dev,  20,u.reg_data[2]);


	return u.interrupt_bits ;
}


void activateRinging(unsigned int slic_dev)
{
	writeDirectReg(slic_dev,  LINE_STATE, RING_LINE); // REG 64,4
}


void converterOn(unsigned int slic_dev){
	writeDirectReg(slic_dev, 14,	0);
}


void disableOscillators(unsigned int slic_dev) 
{ 
	// Turns of OSC1 and OSC2
	unsigned char i;

	//mvOsPrintf("Disabling Oscillators!!!\n");
	for ( i=32; i<=45; i++) 
		if (i !=34)  // Don't write to the ringing oscillator control
		writeDirectReg(slic_dev, i,0);
}

void initializeLoopDebounceReg(unsigned int slic_dev)
{
	writeDirectReg(slic_dev, 69,10);   // Loop Debounce Register  = initial value
}

void printLoopState(unsigned int slic_dev)
{

mvOsPrintf(readDirectReg(slic_dev, 68)&4?"On hook":"Off hook");
}


unsigned char loopStatus(unsigned int slic_dev)
{
static int ReadBack;

	// check for power alarms
	if ((readDirectReg(slic_dev, 19) & 0xfc) != 0) {
		mvOsPrintf ("Power alarm = %x\n",(readDirectReg(slic_dev, 19) & 0xfc));
		clearAlarmBits (slic_dev);
	} 
	ReadBack = readDirectReg (slic_dev, 68);
	return (ReadBack & 0x3);

}

unsigned char digit(unsigned int slic_dev)
{
	return readDirectReg(slic_dev, 24) & 0x1f;
}


void printFreq_Revision(unsigned int slic_dev)
{

	int freq;
	char* freqs[ ] = {"8192","4028","2048","1024","512","256","1536","768","32768"};

	// Turn on all interrupts
	freq=readDirectReg(slic_dev, 13)>>4;  /* Read the frequency */
	mvOsPrintf("PCM clock =  %s KHz   Rev %c \n",  freqs[freq], 'A'-1 + version(slic_dev)); 
}

int calibrate(unsigned int slic_dev)
{ 
	unsigned short i=0;
	volatile unsigned char   DRvalue;
	int timeOut,nCalComplete;

	/* Do Flush durring powerUp and calibrate */
	writeDirectReg(slic_dev, 21,DISABLE_ALL_DR21);//(0)  Disable all interupts in DR21
	writeDirectReg(slic_dev, 22,DISABLE_ALL_DR22);//(0)	Disable all interupts in DR21
	writeDirectReg(slic_dev, 23,DISABLE_ALL_DR23);//(0)	Disabel all interupts in DR21
	writeDirectReg(slic_dev, 64,OPEN_DR64);//(0)
  
	writeDirectReg(slic_dev, 97,0x1e); //(0x18)Calibrations without the ADC and DAC offset and without common mode calibration.
	writeDirectReg(slic_dev, 96,0x47); //(0x47)	Calibrate common mode and differential DAC mode DAC + ILIM
	
	mvOsPrintf("Calibration vBat = %x\n ", readDirectReg(slic_dev, 82));
 
	i=0;
	do 
	{
		DRvalue = readDirectReg(slic_dev, 96);
		nCalComplete = DRvalue==CAL_COMPLETE_DR96;// (0)  When Calibration completes DR 96 will be zero
		timeOut= i++>MAX_CAL_PERIOD;
		mvOsDelay(1);
	}
	while (!nCalComplete&&!timeOut);

	if (timeOut) {
	    mvOsPrintf ("Error in Caliberatation: timeOut\n");
	    return 0;
	}        

	/* Perform manual calibration anyway */
	mvOsPrintf("Performing Manual Calibration\n");
	manualCalibrate (slic_dev);
	

    
/*Initialized DR 98 and 99 to get consistant results.*/
/* 98 and 99 are the results registers and the search should have same intial conditions.*/
/*******The following is the manual gain mismatch calibration******/
/*******This is also available as a function **********************/
	// Wait for any power alarms to settle. 
	mvOsDelay(1000);

	writeIndirectReg(slic_dev, 88,0);
	writeIndirectReg(slic_dev, 89,0);
	writeIndirectReg(slic_dev, 90,0);
	writeIndirectReg(slic_dev, 91,0);
	writeIndirectReg(slic_dev, 92,0);
	writeIndirectReg(slic_dev, 93,0);


	goActive(slic_dev);

	if  (loopStatus(slic_dev) & 4) {
		mvOsPrintf ("Error in Caliberate:  ERRORCODE_LONGBALCAL\n");
		return 0 ;
	}

	writeDirectReg(slic_dev, 64,OPEN_DR64);

	writeDirectReg(slic_dev, 23,ENB2_DR23);  // enable interrupt for the balance Cal
	writeDirectReg(slic_dev, 97,BIT_CALCM_DR97); // this is a singular calibration bit for longitudinal calibration
	writeDirectReg(slic_dev, 96,0x40);

	DRvalue = readDirectReg(slic_dev, 96);
	i=0;
	do 
	{
       	DRvalue = readDirectReg(slic_dev, 96);
        nCalComplete = DRvalue==CAL_COMPLETE_DR96;// (0)  When Calibration completes DR 96 will be zero
		timeOut= i++>MAX_CAL_PERIOD;// (800) MS
		mvOsDelay(1);
	}
	while (!nCalComplete&&!timeOut);
	  
	if (timeOut) {
	    mvOsPrintf ("Error in Caliberate:  timeOut\n");
	    return 0;
	}

	mvOsDelay(1000);

	for (i=88; i<=95; i++) {
		writeIndirectReg (slic_dev, i, 0);
	}
	writeIndirectReg (slic_dev, 97, 0);

	for (i=193; i<=211; i++) {
		writeIndirectReg (slic_dev, i, 0);
	}
    		
   	writeDirectReg(slic_dev, 21,INIT_DR21);
	writeDirectReg(slic_dev, 22,INIT_DR22);
	writeDirectReg(slic_dev, 23,INIT_DR23);

/**********************************The preceding is the longitudinal Balance Cal***********************************/


	mvOsPrintf ("Caliberation done\n");
	return(1);

}// End of calibration

void initializeIndirectRegisters(unsigned int slic_dev)										
{										
if (!Si3215_Flag)
{
	writeIndirectReg(slic_dev, 	0	,	INIT_IR0		);	//	0x55C2	DTMF_ROW_0_PEAK
	writeIndirectReg(slic_dev, 	1	,	INIT_IR1		);	//	0x51E6	DTMF_ROW_1_PEAK
	writeIndirectReg(slic_dev, 	2	,	INIT_IR2		);	//	0x4B85	DTMF_ROW2_PEAK
	writeIndirectReg(slic_dev, 	3	,	INIT_IR3		);	//	0x4937	DTMF_ROW3_PEAK
	writeIndirectReg(slic_dev, 	4	,	INIT_IR4		);	//	0x3333	DTMF_COL1_PEAK
	writeIndirectReg(slic_dev, 	5	,	INIT_IR5		);	//	0x0202	DTMF_FWD_TWIST
	writeIndirectReg(slic_dev, 	6	,	INIT_IR6		);	//	0x0202	DTMF_RVS_TWIST
	writeIndirectReg(slic_dev, 	7	,	INIT_IR7		);	//	0x0198	DTMF_ROW_RATIO
	writeIndirectReg(slic_dev, 	8	,	INIT_IR8		);	//	0x0198	DTMF_COL_RATIO
	writeIndirectReg(slic_dev, 	9	,	INIT_IR9		);	//	0x0611	DTMF_ROW_2ND_ARM
	writeIndirectReg(slic_dev, 	10	,	INIT_IR10		);	//	0x0202	DTMF_COL_2ND_ARM
	writeIndirectReg(slic_dev, 	11	,	INIT_IR11		);	//	0x00E5	DTMF_PWR_MIN_
	writeIndirectReg(slic_dev, 	12	,	INIT_IR12		);	//	0x0A1C	DTMF_OT_LIM_TRES
}

	writeIndirectReg(slic_dev, 	13	,	INIT_IR13		);	//	0x7b30	OSC1_COEF
	writeIndirectReg(slic_dev, 	14	,	INIT_IR14		);	//	0x0063	OSC1X
	writeIndirectReg(slic_dev, 	15	,	INIT_IR15		);	//	0x0000	OSC1Y
	writeIndirectReg(slic_dev, 	16	,	INIT_IR16		);	//	0x7870	OSC2_COEF
	writeIndirectReg(slic_dev, 	17	,	INIT_IR17		);	//	0x007d	OSC2X
	writeIndirectReg(slic_dev, 	18	,	INIT_IR18		);	//	0x0000	OSC2Y
	writeIndirectReg(slic_dev, 	19	,	INIT_IR19		);	//	0x0000	RING_V_OFF
	writeIndirectReg(slic_dev, 	20	,	INIT_IR20		);	//	0x7EF0	RING_OSC
	writeIndirectReg(slic_dev, 	21	,	INIT_IR21		);	//	0x0160	RING_X
	writeIndirectReg(slic_dev, 	22	,	INIT_IR22		);	//	0x0000	RING_Y
	writeIndirectReg(slic_dev, 	23	,	INIT_IR23		);	//	0x2000	PULSE_ENVEL
	writeIndirectReg(slic_dev, 	24	,	INIT_IR24		);	//	0x2000	PULSE_X
	writeIndirectReg(slic_dev, 	25	,	INIT_IR25		);	//	0x0000	PULSE_Y
	writeIndirectReg(slic_dev, 	26	,	INIT_IR26		);	//	0x4000	RECV_DIGITAL_GAIN
	writeIndirectReg(slic_dev, 	27	,	INIT_IR27		);	//	0x4000	XMIT_DIGITAL_GAIN
	writeIndirectReg(slic_dev, 	28	,	INIT_IR28		);	//	0x1000	LOOP_CLOSE_TRES
	writeIndirectReg(slic_dev, 	29	,	INIT_IR29		);	//	0x3600	RING_TRIP_TRES
	writeIndirectReg(slic_dev, 	30	,	INIT_IR30		);	//	0x1000	COMMON_MIN_TRES
	writeIndirectReg(slic_dev, 	31	,	INIT_IR31		);	//	0x0200	COMMON_MAX_TRES
	writeIndirectReg(slic_dev, 	32	,	INIT_IR32		);	//	0x7c0  	PWR_ALARM_Q1Q2
	writeIndirectReg(slic_dev, 	33	,	INIT_IR33		);	//	0x2600	PWR_ALARM_Q3Q4
	writeIndirectReg(slic_dev, 	34	,	INIT_IR34		);	//	0x1B80	PWR_ALARM_Q5Q6
	writeIndirectReg(slic_dev, 	35	,	INIT_IR35		);	//	0x8000	LOOP_CLSRE_FlTER
	writeIndirectReg(slic_dev, 	36	,	INIT_IR36		);	//	0x0320	RING_TRIP_FILTER
	writeIndirectReg(slic_dev, 	37	,	INIT_IR37		);	//	0x08c	TERM_LP_POLE_Q1Q2
	writeIndirectReg(slic_dev, 	38	,	INIT_IR38		);	//	0x0100	TERM_LP_POLE_Q3Q4
	writeIndirectReg(slic_dev, 	39	,	INIT_IR39		);	//	0x0010	TERM_LP_POLE_Q5Q6
	writeIndirectReg(slic_dev, 	40	,	INIT_IR40		);	//	0x0C00	CM_BIAS_RINGING
	writeIndirectReg(slic_dev, 	41	,	INIT_IR41		);	//	0x0C00	DCDC_MIN_V
	writeIndirectReg(slic_dev, 	43	,	INIT_IR43		);	//	0x1000	LOOP_CLOSE_TRES Low
	writeIndirectReg(slic_dev, 	99	,	INIT_IR99		);	//	0x00DA	FSK 0 FREQ PARAM
	writeIndirectReg(slic_dev, 	100	,	INIT_IR100		);	//	0x6B60	FSK 0 AMPL PARAM
	writeIndirectReg(slic_dev, 	101	,	INIT_IR101		);	//	0x0074	FSK 1 FREQ PARAM
	writeIndirectReg(slic_dev, 	102	,	INIT_IR102		);	//	0x79C0	FSK 1 AMPl PARAM
	writeIndirectReg(slic_dev, 	103	,	INIT_IR103		);	//	0x1120	FSK 0to1 SCALER
	writeIndirectReg(slic_dev, 	104	,	INIT_IR104		);	//	0x3BE0	FSK 1to0 SCALER
	writeIndirectReg(slic_dev, 	97	,	INIT_IR97		);	//	0x0000	TRASMIT_FILTER
}										

	

void initializeDirectRegisters(unsigned int slic_dev)
{

if(work_mode)
	writeDirectReg(slic_dev, 0,	INIT_DR0_DC	);//0X00	Serial Interface
else
	writeDirectReg(slic_dev, 0,	INIT_DR0	);//0X00	Serial Interface

writeDirectReg(slic_dev, 1,	INIT_DR1	);//0X08	PCM Mode - INIT TO disable
writeDirectReg(slic_dev, 2,	INIT_DR2	);//0X00	PCM TX Clock Slot Low Byte (1 PCLK cycle/LSB)
writeDirectReg(slic_dev, 3,	INIT_DR3	);//0x00	PCM TX Clock Slot High Byte
writeDirectReg(slic_dev, 4,	INIT_DR4	);//0x00	PCM RX Clock Slot Low Byte (1 PCLK cycle/LSB)
writeDirectReg(slic_dev, 5,	INIT_DR5	);//0x00	PCM RX Clock Slot High Byte
writeDirectReg(slic_dev, 8,	INIT_DR8	);//0X00	Loopbacks (digital loopback default)
writeDirectReg(slic_dev, 9,	INIT_DR9	);//0x00	Transmit and receive path gain and control
writeDirectReg(slic_dev, 10,	INIT_DR10	);//0X28	Initialization Two-wire impedance (600  and enabled)
writeDirectReg(slic_dev, 11,	INIT_DR11	);//0x33	Transhybrid Balance/Four-wire Return Loss
writeDirectReg(slic_dev, 18,	INIT_DR18	);//0xff	Normal Oper. Interrupt Register 1 (clear with 0xFF)
writeDirectReg(slic_dev, 19,	INIT_DR19	);//0xff	Normal Oper. Interrupt Register 2 (clear with 0xFF)
writeDirectReg(slic_dev, 20,	INIT_DR20	);//0xff	Normal Oper. Interrupt Register 3 (clear with 0xFF)
writeDirectReg(slic_dev, 21,	INIT_DR21	);//0xff	Interrupt Mask 1
writeDirectReg(slic_dev, 22,	INIT_DR22	);//0xff	Initialization Interrupt Mask 2
writeDirectReg(slic_dev, 23,	INIT_DR23	);//0xff	 Initialization Interrupt Mask 3
writeDirectReg(slic_dev, 32,	INIT_DR32	);//0x00	Oper. Oscillator 1 Controltone generation
writeDirectReg(slic_dev, 33,	INIT_DR33	);//0x00	Oper. Oscillator 2 Controltone generation
writeDirectReg(slic_dev, 34,	INIT_DR34	);//0X18	34 0x22 0x00 Initialization Ringing Oscillator Control
writeDirectReg(slic_dev, 35,	INIT_DR35	);//0x00	Oper. Pulse Metering Oscillator Control
writeDirectReg(slic_dev, 36,	INIT_DR36	);//0x00	36 0x24 0x00 Initialization OSC1 Active Low Byte (125 탎/LSB)
writeDirectReg(slic_dev, 37,	INIT_DR37	);//0x00	37 0x25 0x00 Initialization OSC1 Active High Byte (125 탎/LSB)
writeDirectReg(slic_dev, 38,	INIT_DR38	);//0x00	38 0x26 0x00 Initialization OSC1 Inactive Low Byte (125 탎/LSB)
writeDirectReg(slic_dev, 39,	INIT_DR39	);//0x00	39 0x27 0x00 Initialization OSC1 Inactive High Byte (125 탎/LSB)
writeDirectReg(slic_dev, 40,	INIT_DR40	);//0x00	40 0x28 0x00 Initialization OSC2 Active Low Byte (125 탎/LSB)
writeDirectReg(slic_dev, 41,	INIT_DR41	);//0x00	41 0x29 0x00 Initialization OSC2 Active High Byte (125 탎/LSB)
writeDirectReg(slic_dev, 42,	INIT_DR42	);//0x00	42 0x2A 0x00 Initialization OSC2 Inactive Low Byte (125 탎/LSB)
writeDirectReg(slic_dev, 43,	INIT_DR43	);//0x00	43 0x2B 0x00 Initialization OSC2 Inactive High Byte (125 탎/LSB)
writeDirectReg(slic_dev, 44,	INIT_DR44	);//0x00	44 0x2C 0x00 Initialization Pulse Metering Active Low Byte (125 탎/LSB)
writeDirectReg(slic_dev, 45,	INIT_DR45	);//0x00	45 0x2D 0x00 Initialization Pulse Metering Active High Byte (125 탎/LSB)
writeDirectReg(slic_dev, 46,	INIT_DR46	);//0x00	46 0x2E 0x00 Initialization Pulse Metering Inactive Low Byte (125 탎/LSB)
writeDirectReg(slic_dev, 47,	INIT_DR47	);//0x00	47 0x2F 0x00 Initialization Pulse Metering Inactive High Byte (125 탎/LSB)
writeDirectReg(slic_dev, 48,	INIT_DR48	);//0X80	48 0x30 0x00 0x80 Initialization Ringing Osc. Active Timer Low Byte (2 s,125 탎/LSB)
writeDirectReg(slic_dev, 49,	INIT_DR49	);//0X3E	49 0x31 0x00 0x3E Initialization Ringing Osc. Active Timer High Byte (2 s,125 탎/LSB)
writeDirectReg(slic_dev, 50,	INIT_DR50	);//0X00	50 0x32 0x00 0x00 Initialization Ringing Osc. Inactive Timer Low Byte (4 s, 125 탎/LSB)
writeDirectReg(slic_dev, 51,	INIT_DR51	);//0X7D	51 0x33 0x00 0x7D Initialization Ringing Osc. Inactive Timer High Byte (4 s, 125 탎/LSB)
writeDirectReg(slic_dev, 52,	INIT_DR52	);//0X00	52 0x34 0x00 Normal Oper. FSK Data Bit
writeDirectReg(slic_dev, 63,	INIT_DR63	);//0X54	63 0x3F 0x54 Initialization Ringing Mode Loop Closure Debounce Interval
writeDirectReg(slic_dev, 64,	INIT_DR64	);//0x00	64 0x40 0x00 Normal Oper. Mode Byte뾭rimary control
writeDirectReg(slic_dev, 65,	INIT_DR65	);//0X61	65 0x41 0x61 Initialization External Bipolar Transistor Settings
writeDirectReg(slic_dev, 66,	INIT_DR66	);//0X03	66 0x42 0x03 Initialization Battery Control
writeDirectReg(slic_dev, 67,	INIT_DR67	);//0X1F	67 0x43 0x1F Initialization Automatic/Manual Control
writeDirectReg(slic_dev, 69,	INIT_DR69	);//0X0C	69 0x45 0x0A 0x0C Initialization Loop Closure Debounce Interval (1.25 ms/LSB)
writeDirectReg(slic_dev, 70,	INIT_DR70	);//0X0A	70 0x46 0x0A Initialization Ring Trip Debounce Interval (1.25 ms/LSB)
writeDirectReg(slic_dev, 71,	INIT_DR71	);//0X01	71 0x47 0x00 0x01 Initialization Off-Hook Loop Current Limit (20 mA + 3 mA/LSB)
writeDirectReg(slic_dev, 72,	INIT_DR72	);//0X20	72 0x48 0x20 Initialization On-Hook Voltage (open circuit voltage) = 48 V(1.5 V/LSB)
writeDirectReg(slic_dev, 73,	INIT_DR73	);//0X02	73 0x49 0x02 Initialization Common Mode VoltageVCM = 3 V(1.5 V/LSB)
writeDirectReg(slic_dev, 74,	INIT_DR74	);//0X32	74 0x4A 0x32 Initialization VBATH (ringing) = 75 V (1.5 V/LSB)
writeDirectReg(slic_dev, 75,	INIT_DR75	);//0X10	75 0x4B 0x10 Initialization VBATL (off-hook) = 24 V (TRACK = 0)(1.5 V/LSB)
if (chipType(slic_dev) != 3)
writeDirectReg(slic_dev, 92,	INIT_DR92	);//0x7f	92 0x5C 0xFF 7F Initialization DCDC Converter PWM Period (61.035 ns/LSB)
else
writeDirectReg(slic_dev, 92,	INIT_SI3210M_DR92	);//0x7f	92 0x5C 0xFF 7F Initialization DCDC Converter PWM Period (61.035 ns/LSB)


writeDirectReg(slic_dev, 93,	INIT_DR93	);//0x14	93 0x5D 0x14 0x19 Initialization DCDC Converter Min. Off Time (61.035 ns/LSB)
writeDirectReg(slic_dev, 96,	INIT_DR96	);//0x00	96 0x60 0x1F Initialization Calibration Control Register 1(written second and starts calibration)
writeDirectReg(slic_dev, 97,	INIT_DR97	);//0X1F	97 0x61 0x1F Initialization Calibration Control Register 2(written before Register 96)
writeDirectReg(slic_dev, 98,	INIT_DR98	);//0X10	98 0x62 0x10 Informative Calibration result (see data sheet)
writeDirectReg(slic_dev, 99,	INIT_DR99	);//0X10	99 0x63 0x10 Informative Calibration result (see data sheet)
writeDirectReg(slic_dev, 100,	INIT_DR100	);//0X11	100 0x64 0x11 Informative Calibration result (see data sheet)
writeDirectReg(slic_dev, 101,	INIT_DR101	);//0X11	101 0x65 0x11 Informative Calibration result (see data sheet)
writeDirectReg(slic_dev, 102,	INIT_DR102	);//0x08	102 0x66 0x08 Informative Calibration result (see data sheet)
writeDirectReg(slic_dev, 103,	INIT_DR103	);//0x88	103 0x67 0x88 Informative Calibration result (see data sheet)
writeDirectReg(slic_dev, 104,	INIT_DR104	);//0x00	104 0x68 0x00 Informative Calibration result (see data sheet)
writeDirectReg(slic_dev, 105,	INIT_DR105	);//0x00	105 0x69 0x00 Informative Calibration result (see data sheet)
writeDirectReg(slic_dev, 106,	INIT_DR106	);//0x20	106 0x6A 0x20 Informative Calibration result (see data sheet)
writeDirectReg(slic_dev, 107,	INIT_DR107	);//0x08	107 0x6B 0x08 Informative Calibration result (see data sheet)
writeDirectReg(slic_dev, 108,	INIT_DR108	);//0xEB	108 0x63 0x00 0xEB Initialization Feature enhancement register
}
   

void clearInterrupts(unsigned int slic_dev)
{
	writeDirectReg(slic_dev, 	18	,	INIT_DR18	);//0xff	Normal Oper. Interrupt Register 1 (clear with 0xFF)
	writeDirectReg(slic_dev, 	19	,	INIT_DR19	);//0xff	Normal Oper. Interrupt Register 2 (clear with 0xFF)
	writeDirectReg(slic_dev, 	20	,	INIT_DR20	);//0xff	Normal Oper. Interrupt Register 3 (clear with 0xFF)
}

int verifyIndirectReg(unsigned int slic_dev, unsigned char address, unsigned short should_be_value)
{ 
	int error_flag ;
	unsigned short value;
		value = readIndirectReg(slic_dev, address);
		error_flag = (should_be_value != value);
		
		if ( error_flag )
		{
			mvOsPrintf("\n   iREG %d = %X  should be %X ",address,value,should_be_value );			
		}	
		return error_flag;
}


int verifyIndirectRegisters(unsigned int slic_dev)										
{		
	int error=0;

	if (!Si3215_Flag)
	{

	error |= verifyIndirectReg(slic_dev   ,    	0	,	INIT_IR0		);	//	0x55C2	DTMF_ROW_0_PEAK
	error |= verifyIndirectReg(slic_dev   ,    	1	,	INIT_IR1		);	//	0x51E6	DTMF_ROW_1_PEAK
	error |= verifyIndirectReg(slic_dev   ,    	2	,	INIT_IR2		);	//	0x4B85	DTMF_ROW2_PEAK
	error |= verifyIndirectReg(slic_dev   ,    	3	,	INIT_IR3		);	//	0x4937	DTMF_ROW3_PEAK
	error |= verifyIndirectReg(slic_dev   ,    	4	,	INIT_IR4		);	//	0x3333	DTMF_COL1_PEAK
	error |= verifyIndirectReg(slic_dev   ,    	5	,	INIT_IR5		);	//	0x0202	DTMF_FWD_TWIST
	error |= verifyIndirectReg(slic_dev   ,    	6	,	INIT_IR6		);	//	0x0202	DTMF_RVS_TWIST
	error |= verifyIndirectReg(slic_dev   ,    	7	,	INIT_IR7		);	//	0x0198	DTMF_ROW_RATIO
	error |= verifyIndirectReg(slic_dev   ,    	8	,	INIT_IR8		);	//	0x0198	DTMF_COL_RATIO
	error |= verifyIndirectReg(slic_dev   ,    	9	,	INIT_IR9		);	//	0x0611	DTMF_ROW_2ND_ARM
	error |= verifyIndirectReg(slic_dev   ,    	10	,	INIT_IR10		);	//	0x0202	DTMF_COL_2ND_ARM
	error |= verifyIndirectReg(slic_dev   ,    	11	,	INIT_IR11		);	//	0x00E5	DTMF_PWR_MIN_
	error |= verifyIndirectReg(slic_dev   ,    	12	,	INIT_IR12		);	//	0x0A1C	DTMF_OT_LIM_TRES
	}
	error |= verifyIndirectReg(slic_dev   ,    	13	,	INIT_IR13		);	//	0x7b30	OSC1_COEF
	error |= verifyIndirectReg(slic_dev   ,    	14	,	INIT_IR14		);	//	0x0063	OSC1X
	error |= verifyIndirectReg(slic_dev   ,    	15	,	INIT_IR15		);	//	0x0000	OSC1Y
	error |= verifyIndirectReg(slic_dev   ,    	16	,	INIT_IR16		);	//	0x7870	OSC2_COEF
	error |= verifyIndirectReg(slic_dev   ,    	17	,	INIT_IR17		);	//	0x007d	OSC2X
	error |= verifyIndirectReg(slic_dev   ,    	18	,	INIT_IR18		);	//	0x0000	OSC2Y
	error |= verifyIndirectReg(slic_dev   ,    	19	,	INIT_IR19		);	//	0x0000	RING_V_OFF
	error |= verifyIndirectReg(slic_dev   ,    	20	,	INIT_IR20		);	//	0x7EF0	RING_OSC
	error |= verifyIndirectReg(slic_dev   ,    	21	,	INIT_IR21		);	//	0x0160	RING_X
	error |= verifyIndirectReg(slic_dev   ,    	22	,	INIT_IR22		);	//	0x0000	RING_Y
	error |= verifyIndirectReg(slic_dev   ,    	23	,	INIT_IR23		);	//	0x2000	PULSE_ENVEL
	error |= verifyIndirectReg(slic_dev   ,    	24	,	INIT_IR24		);	//	0x2000	PULSE_X
	error |= verifyIndirectReg(slic_dev   ,    	25	,	INIT_IR25		);	//	0x0000	PULSE_Y
	error |= verifyIndirectReg(slic_dev   ,    	26	,	INIT_IR26		);	//	0x4000	RECV_DIGITAL_GAIN
	error |= verifyIndirectReg(slic_dev   ,    	27	,	INIT_IR27		);	//	0x4000	XMIT_DIGITAL_GAIN
	error |= verifyIndirectReg(slic_dev   ,    	28	,	INIT_IR28		);	//	0x1000	LOOP_CLOSE_TRES
	error |= verifyIndirectReg(slic_dev   ,    	29	,	INIT_IR29		);	//	0x3600	RING_TRIP_TRES
	error |= verifyIndirectReg(slic_dev   ,    	30	,	INIT_IR30		);	//	0x1000	COMMON_MIN_TRES
	error |= verifyIndirectReg(slic_dev   ,    	31	,	INIT_IR31		);	//	0x0200	COMMON_MAX_TRES
	error |= verifyIndirectReg(slic_dev   ,    	32	,	INIT_IR32		);	//	0x7c0  	PWR_ALARM_Q1Q2
	error |= verifyIndirectReg(slic_dev   ,    	33	,	INIT_IR33		);	//	0x2600	PWR_ALARM_Q3Q4
	error |= verifyIndirectReg(slic_dev   ,    	34	,	INIT_IR34		);	//	0x1B80	PWR_ALARM_Q5Q6
	error |= verifyIndirectReg(slic_dev   ,    	35	,	INIT_IR35		);	//	0x8000	LOOP_CLSRE_FlTER
	error |= verifyIndirectReg(slic_dev   ,    	36	,	INIT_IR36		);	//	0x0320	RING_TRIP_FILTER
	error |= verifyIndirectReg(slic_dev   ,    	37	,	INIT_IR37		);	//	0x08c	TERM_LP_POLE_Q1Q2
	error |= verifyIndirectReg(slic_dev   ,    	38	,	INIT_IR38		);	//	0x0100	TERM_LP_POLE_Q3Q4
	error |= verifyIndirectReg(slic_dev   ,    	39	,	INIT_IR39		);	//	0x0010	TERM_LP_POLE_Q5Q6
	error |= verifyIndirectReg(slic_dev   ,    	40	,	INIT_IR40		);	//	0x0C00	CM_BIAS_RINGING
	error |= verifyIndirectReg(slic_dev   ,    	41	,	INIT_IR41		);	//	0x0C00	DCDC_MIN_V
	error |= verifyIndirectReg(slic_dev   ,    	43	,	INIT_IR43		);	//	0x1000	LOOP_CLOSE_TRES Low
	error |= verifyIndirectReg(slic_dev   ,    	99	,	INIT_IR99		);	//	0x00DA	FSK 0 FREQ PARAM
	error |= verifyIndirectReg(slic_dev   ,    	100	,	INIT_IR100		);	//	0x6B60	FSK 0 AMPL PARAM
	error |= verifyIndirectReg(slic_dev   ,    	101	,	INIT_IR101		);	//	0x0074	FSK 1 FREQ PARAM
	error |= verifyIndirectReg(slic_dev   ,    	102	,	INIT_IR102		);	//	0x79C0	FSK 1 AMPl PARAM
	error |= verifyIndirectReg(slic_dev   ,    	103	,	INIT_IR103		);	//	0x1120	FSK 0to1 SCALER
	error |= verifyIndirectReg(slic_dev   ,    	104	,	INIT_IR104		);	//	0x3BE0	FSK 1to0 SCALER
	
	return error;
}

void enablePCMhighway(unsigned int slic_dev)
{
	/* pcm Mode Select */
	/*
	** Bit 0:   tri-State on Positive edge of Pclk
	** Bit 1:   GCI Clock Format, 1 PCLK per data bit
	** Bit 2:   PCM Transfer size (8 bt transfer)
	** Bit 3-4:	PCM Format mu-law
	** Bit 5:   PCM Enable
	*/
	writeDirectReg(slic_dev, 1,0x28); 
}

void disablePCMhighway(unsigned int slic_dev)
{
	/* Disable the PCM */
	writeDirectReg(slic_dev, 1,0x08);
}

/* New Functions */
int printSlicInfo(unsigned int slic_dev)
{
    unsigned int type;
    unsigned char fm;
    char name[10];

    
    fm = family(slic_dev);
    type = chipType(slic_dev);
    
	if(fm == MV_SI3210_FAMILY)
	{
		if(type == MV_SI3210)
			strcpy(name, "SI3210");
		else if(type == MV_SI3211)
			strcpy(name, "SI3211");
		else if(type == MV_SI3210M)
			strcpy(name, "SI3210M");
		else 
            strcpy(name, "UNKNOWN");
	}
	else if(fm == MV_SI3215_FAMILY)
	{
		if(type == MV_SI3215)
			strcpy(name, "SI3215");
		else if(type == MV_SI3215M)
			strcpy(name, "SI3215M");
		else
            strcpy(name, "UNKNOWN");

	}
	strcat(name, "\0");
    mvOsPrintf("FXS type : %s\n",name);

    if(strcmp(name,"UNKNOWN"))
        return MV_OK;
    else
        return MV_NOT_SUPPORTED;

}

int initSlic(unsigned int *slicDev, unsigned int ch, int workMode, int intMode)
{
	MV_SLIC_DEV *pSlicDev;
	int status;
	
	pSlicDev = (MV_SLIC_DEV *)mvOsMalloc(sizeof(MV_SLIC_DEV));
	if(!pSlicDev)
	{
		mvOsPrintf("%s: error malloc failed\n",__FUNCTION__);
		return 1;
	}
	work_mode = workMode;
	interrupt_mode = intMode;

	pSlicDev->ch = ch;

	if(work_mode)
        	pSlicDev->dcval = ch + 1;

		
       *slicDev = (unsigned int)pSlicDev;	
      
	MV_TRC_REC("start slic-%d\n",pSlicDev->ch);

	/* Verify slic exists */
	if (!selfTest(*slicDev))
		return 1;

    	if(!slicStart(*slicDev))
		return 1;

	enablePCMhighway(*slicDev);

	/* print slic info */
	status = printSlicInfo(*slicDev);

	if(status == MV_OK) 
	{
		if(work_mode) {

			if(!isDaisyChain(*slicDev))
			{
				mvOsPrintf("error, ch %d is not in daisy chain mode\n",pSlicDev->ch);
				return 1;
			}
		}

		MV_TRC_REC("disable slic-%d interrupts\n",pSlicDev->ch);
		disableSlicInterrupts(*slicDev);
#ifdef MV_TDM_LINEAR_MODE
		/* Configure linear mode */
		mvOsPrintf("Slic-%d: using linear mode\n",pSlicDev->ch);
		setSlicToLinearMode(*slicDev);
          
#endif

		/* Configure tx/rx sample in SLIC */
		pSlicDev->txSample = ((pSlicDev->ch==0) ? CH0_TX_SLOT : CH1_TX_SLOT);
		pSlicDev->rxSample = ((pSlicDev->ch==0) ? CH0_RX_SLOT : CH1_RX_SLOT);
		mvOsPrintf("FXS-%d: RX sample %d, TX sample %d\n",pSlicDev->ch, pSlicDev->rxSample, pSlicDev->txSample);
		pSlicDev->txSample *= 8;
		pSlicDev->rxSample *= 8;

          
		setSlicPcmStartCountRegs(*slicDev);
	}
	else
	{
		mvOsPrintf("%s: error, unknown slic type\n",__FUNCTION__);
		return 1;
	}

	return 0;
}

void enableSlicInterrupts(unsigned int slic_dev)
{
	MV_TRC_REC("enable slic-%d interrupts\n",((MV_SLIC_DEV*)(slic_dev))->ch);
	writeDirectReg(slic_dev, INTRPT_MASK1, 0x10);		
	writeDirectReg(slic_dev, INTRPT_MASK2, 2);
	writeDirectReg(slic_dev, INTRPT_MASK3, 0);

}

void disableSlicInterrupts(unsigned int slic_dev)
{
	MV_TRC_REC("disable slic-%d interrupts\n",((MV_SLIC_DEV*)(slic_dev))->ch);
	writeDirectReg(slic_dev, INTRPT_STATUS1, 0);
	writeDirectReg(slic_dev, INTRPT_STATUS2, 0);
	writeDirectReg(slic_dev, INTRPT_STATUS3, 0);
	writeDirectReg(slic_dev, INTRPT_MASK1, 0);
	writeDirectReg(slic_dev, INTRPT_MASK2, 0);
	writeDirectReg(slic_dev, INTRPT_MASK3, 0);
}



int isDaisyChain(unsigned int slic_dev)
{
    int val;

    val = readDirectReg(slic_dev, 0);
	
    return (val & 0x80);

}

void setSlicToLinearMode(unsigned int slic_dev)
{
    int val;
    val = readDirectReg(slic_dev, PCM_MODE);
	writeDirectReg(slic_dev, PCM_MODE, val|(0x3<<3)|(1<<2)); /* linear mode + 16 bit transfer */
}

void setSlicPcmStartCountRegs(unsigned int slic_dev)
{
	MV_SLIC_DEV *pSlicDev = (MV_SLIC_DEV*)slic_dev;

	writeDirectReg(slic_dev, PCM_XMIT_START_COUNT_LSB, pSlicDev->txSample&0xff);
	writeDirectReg(slic_dev, PCM_XMIT_START_COUNT_MSB, (pSlicDev->txSample>>8)&0x3);
	writeDirectReg(slic_dev, PCM_RCV_START_COUNT_LSB, pSlicDev->rxSample&0xff);
	writeDirectReg(slic_dev, PCM_RCV_START_COUNT_MSB, (pSlicDev->rxSample>>8)&0x3);

}

int checkSlicInterrupts(unsigned int slic_dev)
{
   union {
		unsigned char reg_data[3];
		long interrupt_bits;
	} u ;
	u.interrupt_bits=0;


	u.reg_data[0] = readDirectReg(slic_dev, 18);
	writeDirectReg(slic_dev, 18, u.reg_data[0]);

	u.reg_data[1] = readDirectReg(slic_dev, 19);
	if ((u.reg_data[1] & 0xfc) != 0) {
		clearAlarmBits (slic_dev);
	} 
	writeDirectReg(slic_dev, 19, u.reg_data[1] );

	u.reg_data[2] = readDirectReg(slic_dev,  20);
	writeDirectReg(slic_dev,  20, u.reg_data[2]);

	if((u.reg_data[0] & 0x10) && (0x04  == readDirectReg(slic_dev, 68)))	
	{
		return 0;
	}

	return (u.interrupt_bits)? 1 : 0;
 
}

MV_STATUS slicRingCadence(unsigned int arg, unsigned int slic_dev)
{
    ringStruct ringRegs;
	ringRegs.u1.onTime  = (unsigned short) (0xffff & (arg>>16)); 
	ringRegs.u2.offTime = (unsigned short) (0xffff & arg); 	
	if(0 == ringRegs.u1.onTime && 0 == ringRegs.u2.offTime)
		writeDirectReg(slic_dev,  34, 0x00);	/*disable timers */
	else if (0 < ringRegs.u1.onTime && 0 < ringRegs.u2.offTime && /* check values of 0 to 8 seconds */
		8 >= ringRegs.u1.onTime && 8 >= ringRegs.u2.offTime)
	{
		ringRegs.u1.onTime  *= (1000*8);
		ringRegs.u2.offTime *= (1000*8);

		writeDirectReg(slic_dev,  RING_ON__LO,  ringRegs.u1.s1.onLowByte); // lo reg 48
		writeDirectReg(slic_dev,  RING_ON_HI,   ringRegs.u1.s1.onHiByte); // hi reg 49
		// Inactive Timer
		writeDirectReg(slic_dev,  RING_OFF_LO, ringRegs.u2.s2.offLowByte); // low reg 50
		writeDirectReg(slic_dev,  RING_OFF_HI, ringRegs.u2.s2.offHiByte); // hi reg 51
		// Enable timers for ringing oscillator
		writeDirectReg(slic_dev,  34, 0x18);
	}
	else
		return MV_ERROR; /*error*/
	return MV_OK;

}


MV_STATUS slicSendNTTCRATone(unsigned int slic_dev)         /* Timor */
{
    
    writeDirectReg(slic_dev,  RING_ON__LO, 0x80); /* low reg 48 */
    writeDirectReg(slic_dev,  RING_ON_HI, 0x3E);  /* hi reg 49 */
    /* Inactive Timer */
    writeDirectReg(slic_dev,  RING_OFF_LO, 0x00); /* low reg 50 */
    writeDirectReg(slic_dev,  RING_OFF_HI, 0x7D); /* hi reg 51 */
    /* Enable timers for ringing oscillator */
    writeDirectReg(slic_dev,  RING_OSC_CTL, 0x18);

    return MV_OK;
}

void slicFreeChannel(unsigned int slic_dev)
{
	MV_SLIC_DEV *pSlicDev = (MV_SLIC_DEV *)slic_dev;
	mvOsFree(pSlicDev);
}
