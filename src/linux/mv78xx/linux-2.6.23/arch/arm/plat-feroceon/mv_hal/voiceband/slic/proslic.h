/************************************  PROSLIC.H   *************************************************/
#ifndef _PROSLIC_H_
#define _PROSLIC_H_

#ifdef MV_88F5181L
#include "voiceband/tdm/mvTdm.h"
#endif
#include "mvOs.h"

/* PROTOTYPE */
unsigned char family (unsigned int slic_dev);
int slicStart(unsigned int slic_dev);
//void cadenceRingPhone(ringStruct ringRegs);
int groundShort(unsigned int slic_dev);
void delay( unsigned long wait);
void goActive(unsigned int slic_dev);
int calibrate(unsigned int slic_dev);
unsigned char powerUp(unsigned int slic_dev);
unsigned char powerLeakTest(unsigned int slic_dev);

void standardRinging(unsigned int slic_dev) ;
 	
void initializeDirectRegisters(unsigned int slic_dev);
void clearInterrupts(unsigned int slic_dev);
void converterOn(unsigned int slic_dev);
void ringAndWait(unsigned int slic_dev);
void enablePCMhighway(unsigned int slic_dev);
void disablePCMhighway(unsigned int slic_dev);
void disableOscillators(unsigned int slic_dev);
void printIndirectRegisters(unsigned int slic_dev);
void initializeIndirectRegisters(unsigned int slic_dev);
int verifyIndirectRegisters(unsigned int slic_dev);
unsigned char readDirectReg(unsigned int slic_dev, unsigned char address);
void writeDirectReg(unsigned int slic_dev, unsigned char address, unsigned char data);
void writeIndirectReg(unsigned int slic_dev, unsigned char address, unsigned short data);
unsigned short readIndirectReg(unsigned int slic_dev, unsigned char address);
void stopRinging(unsigned int slic_dev);
unsigned short manualCalibrate(unsigned int slic_dev);
void dialTone(unsigned int slic_dev);
void stopTone(unsigned int slic_dev);
void busyTone(unsigned int slic_dev);
void reorderTone(unsigned int slic_dev);
void congestionTone(unsigned int slic_dev);
void ringbackPbxTone(unsigned int slic_dev);
void ringBackTone(unsigned int slic_dev);
void clearAlarmBits(unsigned int slic_dev);
unsigned long interruptBits (unsigned int slic_dev);
void activateRinging(unsigned int slic_dev);
void callerid( void);
void initializeLoopDebounceReg(unsigned int slic_dev);
void setupNeon(void);
unsigned char version(unsigned int slic_dev);
unsigned char chipType (unsigned int slic_dev);
unsigned char loopStatus(unsigned int slic_dev);
void printLoopState(unsigned int slic_dev);
unsigned char version(unsigned int slic_dev);
unsigned char digit(unsigned int slic_dev);
void printFreq_Revision(unsigned int slic_dev);
void ringBackJapan(unsigned int slic_dev);
void busyJapan(unsigned int slic_dev);
//int ProSlic_reset_initialize(void); 
int slic_start(unsigned int slic_dev);
int printSlicInfo(unsigned int slic_dev);
int initSlic(unsigned int *slicFxDev, unsigned int ch, int workMode, int intMode);
void enableSlicInterrupts(unsigned int slic_dev);
void disableSlicInterrupts(unsigned int slic_dev);
void setSlicToLinearMode(unsigned int slic_dev);
int isDaisyChain(unsigned int slic_dev);
void setSlicPcmStartCountRegs(unsigned int slic_dev);
int checkSlicInterrupts(unsigned int slic_dev);
MV_STATUS slicRingCadence(unsigned int arg, unsigned int slic_dev);
MV_STATUS slicSendNTTCRATone(unsigned int slic_dev);         /* Timor */
void slicFreeChannel(unsigned int slic_dev);
   
/********************************************STRUCTURES***********************************************/

/* STRUCTURES */
typedef struct {
	unsigned char address;
	char *name;
	unsigned short initial;
} indirectRegister;

// enum state_type { BUSY, CALL_BACK, CALLING_BACK };
typedef struct {
	//unsigned char chip_number;
	enum{ MAKEbUSY, STATEcHANGE,DIALtONE,INITIALIZING,POWERuP,CALIBRATE,PROGRAMMEDrING,POWERdOWN,POWERlEAKtEST,
		MAKErINGbACKjAPAN, MAKEbUSYjAPAN, RINGbACKjAPAN, MAKErINGbACK,RINGbACK,MAKErEORDER,REORDER,MAKEcONGESTION,CONGESTION,PRENEON,NEON,
		CALLBACKpROGRAMMED, BUSY, CALLBACK, CALLING, MAKEoFFHOOK,ONHOOK, OFFHOOK, DIGITDECODING,
	LOOPtRANSITION, FIRSTrING, DEFEREDcALLBACKpROGRAMMED,CALLERiD,RINGING,DTMFtRANISTION} state,newState,previousState;
	int digit_count;
	char DTMF_digits[20];
	unsigned long interrupt;
	unsigned char eventEnable;
	unsigned char hook_status;
	unsigned long On_Hook_time;
	unsigned long Off_Hook_time;
	char	version,type;
	struct{ enum { TYPE1, TYPE2, TYPE3 } ringType;
			int nextCadenceEntryIndex;
		}  ringCadenceCordinates;
	unsigned char ringCount;
	int qLog[6];
	unsigned long eventNumber;
} chipStruct;


typedef struct {
	unsigned short coeff;
	unsigned short x;
	unsigned short y;
	union {
		unsigned short onTime;
		struct	{unsigned char onLowByte;unsigned char onHiByte;} s1 ;

	} u1 ;

	union {
		unsigned short offTime;
		struct {unsigned char offLowByte;unsigned char offHiByte;} s2;
	} u2 ;

	}  ringStruct;


typedef struct {
	unsigned short coeff;
	unsigned short x;
	unsigned short y;
	unsigned char on_hi_byte;
	unsigned char on_low_byte;
	unsigned char off_hi_byte;
	unsigned char off_low_byte;
} Oscillator;




typedef struct {
	Oscillator osc1;
	Oscillator osc2;
} tone_struct;  

typedef struct {
unsigned short frequency;
unsigned short coeff; 
unsigned short x; 
unsigned short y; 
} coeffData;
void genTone(tone_struct tone, unsigned int slic_dev) ; 

typedef unsigned char DRDATA;

/*#define $VERSION  "PreRelease 2.0 ProSLIC Driver"*/

/* CONSTANTS */

#define LPT 0x378

#define IDA_LO  28
#define IDA_HI	29

#define IAA 30
#define CLOCK_MASK 0x04
#define ID_ACCES_STATUS 31
#define IAS_BIT 1

#define	I_STATUS	31

#define	SPI_MODE	0
#define	PCM_MODE	1
#define	PCM_XMIT_START_COUNT_LSB	2
#define	PCM_XMIT_START_COUNT_MSB	3
#define	PCM_RCV_START_COUNT_LSB	4
#define	PCM_RCV_START_COUNT_MSB	5
#define	DIO	6

#define	AUDIO_LOOPBACK	8
#define	AUDIO_GAIN	9
#define	LINE_IMPEDANCE	10
#define	HYBRID	11
#define	RESERVED12	12
#define	RESERVED13	13
#define	PWR_DOWN1	14
#define	PWR_DOWN2	15
#define	RESERVED16	16
#define	RESERVED17	17
#define	INTRPT_STATUS1	18
#define	INTRPT_STATUS2	19
#define	INTRPT_STATUS3	20
#define	INTRPT_MASK1	21
#define	INTRPT_MASK2	22
#define	INTRPT_MASK3	23
#define	DTMF_DIGIT	24
#define	RESERVED25	25
#define	RESERVED26	26
#define	RESERVED27	27
#define	I_DATA_LOW	28
#define	I_DATA_HIGH	29
#define	I_ADDRESS	30
#define	I_STATUS	31
#define	OSC1	32
#define	OSC2	33
#define	RING_OSC_CTL	34
#define	PULSE_OSC	35
#define	OSC1_ON__LO	36
#define	OSC1_ON_HI	37
#define	OSC1_OFF_LO	38
#define	OSC1_OFF_HI	39
#define	OSC2_ON__LO	40
#define	OSC2_ON_HI	41
#define	OSC2_OFF_LO	42
#define	OSC2_OFF_HI	43
#define	PULSE_ON__LO	44
#define	PULSE_ON_HI	45
#define	PULSE_OFF_LO	46
#define	PULSE_OFF_HI	47
#define	RING_ON__LO	48
#define	RING_ON_HI	49
#define	RING_OFF_LO	50
#define	RING_OFF_HI	51
#define	FSK_DATA	52	/*		0								fsk_data	*/
#define	RESERVED53	53
#define	RESERVED54	54
#define	RESERVED55	55
#define	RESERVED56	56
#define	RESERVED57	57
#define	RESERVED58	58
#define	RESERVED59	59
#define	RESERVED60	60
#define	RESERVED61	61
#define	RESERVED62	62
#define	RESERVED63	63
#define	LINE_STATE	64
#define			ACTIVATE_LINE 0x11
#define			RING_LINE     0x44
#define	BIAS_SQUELCH	65
#define	BAT_FEED	66
#define	AUTO_STATE	67
#define	LOOP_STAT	68
#define	LOOP_DEBOUCE	69
#define	RT_DEBOUCE	70
#define	LOOP_I_LIMIT	71
#define	OFF_HOOK_V	72
#define	COMMON_V	73
#define	BAT_V_HI	74
#define	BAT_V_LO	75
#define	PWR_STAT_DEV	76
#define	PWR_STAT	77
#define	LOOP_V_SENSE	78
#define	LOOP_I_SENSE	79
#define	TIP_V_SENSE	80
#define	RING_V_SENSE	81
#define	BAT_V_HI_SENSE	82
#define	BAT_V_LO_SENSE	83
#define	IQ1	84
#define	IQ2	85
#define	IQ3	86
#define	IQ4	87
#define	IQ5	88
#define	IQ6	89
#define	RESERVED90	90
#define	RESERVED91	91
#define	DCDC_PWM_OFF	92
#define	DCDC	93
#define	DCDC_PW_OFF	94
#define	RESERVED95	95
#define	CALIBR1	96
#define CALIBRATE_LINE 0x78
#define NORMAL_CALIBRATION_COMPLETE 0x20
#define	CALIBR2	97
#define	RING_GAIN_CAL	98
#define	TIP_GAIN_CAL	99
#define	DIFF_I_CAL	100
#define	COMMON_I_CAL	101
#define	I_LIMIT_GAIN_CAL	102
#define	ADC_OFFSET_CAL	103
#define	DAC_ADC_OFFSET	104
#define	DAC_OFFSET_CAL	105
#define	COMMON_BAL_CAL	106
#define	DC_PEAK_CAL	107

//		Indirect Register (decimal)
#define	DTMF_ROW_0_PEAK	0
#define	DTMF_ROW_1_PEAK	1
#define	DTMF_ROW2_PEAK	2
#define	DTMF_ROW3_PEAK	3
#define	DTMF_COL1_PEAK	4
#define	DTMF_FWD_TWIST	5
#define	DTMF_RVS_TWIST	6
#define	DTMF_ROW_RATIO_THRESH	7
#define	DTMF_COL_RATIO_THRESH	8
#define	DTMF_ROW_2ND_HARM	9
#define	DTMF_COL_2ND_HARM	10
#define	DTMF_PWR_MIN_THRESH	11
#define	DTMF_HOT_LIM_THRESH	12
#define	OSC1_COEF	13
#define	OSC1X	14
#define	OSC1Y	15
#define	OSC2_COEF	16
#define	OSC2X	17
#define	OSC2Y	18
#define	RING_V_OFF	19
#define	RING_OSC_COEF	20
#define	RING_X	21
#define	RING_Y	22
#define	PULSE_ENVEL	23
#define	PULSE_X	24
#define	PULSE_Y	25
#define	RECV_DIGITAL_GAIN	26
#define	XMIT_DIGITAL_GAIN	27
#define	LOOP_CLOSE_THRESH	28
#define	RING_TRIP_THRESH	29
#define	COMMON_MIN_THRESH	30
#define	COMMON_MAX_THRESH	31
#define	PWR_ALARM_Q1Q2	32
#define	PWR_ALARM_Q3Q4	33
#define	PWR_ALARM_Q5Q6	34
#define	LOOP_CLOSURE_FILTER	35
#define	RING_TRIP_FILTER	36
#define	THERM_LP_POLE_Q1Q2	37
#define	THERM_LP_POLE_Q3Q4	38
#define	THERM_LP_POLE_Q5Q6	39
#define	CM_BIAS_RINGING	40
#define	DCDC_MIN_V	41
#define	DCDC_XTRA	42
#define ALL_CHIPS 0x09
#define NO_CHIPS 0
#define	REVC	108	/*		0	ilim_max	fsk_revc	dc_err_en	zs_ext	batsel_pd	lcr_sense	en_subtr	hyst_en	*/
#define	FSK_X_0		99	/*	x	sign				fsk_x_0[15:0]												*/
#define	FSK_COEFF_0	100	/*	x	sign				fsk_coeff_0[15:0]												*/
#define	FSK_X_1		101	/*	x	sign				fsk_x_1[15:0]												*/
#define	FSK_COEFF_1	102	/*	x	sign				fsk_coeff_1[15:0]												*/
#define	FSK_X_01	103	/*	x	sign				fsk_x_01[15:0]												*/
#define	FSK_X_10	104	/*	x	sign				fsk_x_10[15:0]												*/

#ifdef REALNAMES


/*      OPTIONAL CONSTANTS */

/**********************SETUP*****************************************/													
#define	SPI_MODE	0	/*		b00xxxxxx	daisy_chn	spi_mode	part_num[1:0]			revision[3:0]			*/
#define	PCM_MODE	1	/*		8			pcm_en	pcm_fmt[1:0]		pcm_size	pcm_gci	pcm_tri_0	*/
#define	TX_START_LO	2	/*		0				tx_start[7:0]					*/
#define	TX_START_HI	3	/*		0						 	tx_start[9:8]		*/
#define	RX_START_LO	4	/*		0				rx_start[7:0]					*/
#define	RX_START_HI	5	/*		0						 	rx_start[9:8]		*/
//#define	GPIO		6	/*		0				relay_drv	gpio1_dir	gpio0_dir	gpio1	gpio0	*/
 //			/*											*/
///***********************AUDIO****************************************/											*/
#define	LOOPBACK	8	/*		2						ana_lpbk2	dig_lpbk	ana_lpbk	*/
#define	AUD_GAIN	9	/*		0	rx_hpf_dis	tx_hpf_dis	adc_mute	dac_mute	atx[1:0]		arx[1:0]		*/
#define	ZSYNTH		10	/*		8		 	clcomp[1:0]		zsynth_en		zsynth[2:0]		*/
#define	HYBRID_CNTL	11	/*		h44			hybp[2:0]				hyba[2:0]		*/
#define	POWER DOWN		/*											*/
#define	PDN1		14	/*		b010000			pm_on	dc_off	mon_off	pll_off	bias_off	slic_off	*/
#define	PDN2		15	/*		0		 	adc_man	a_on_offb	dac_man	d_on_offb	gm_man	g_on_offb	*/



/***********************INTERRUPTS***********************************/
#define	IRQ_VEC1	18	/*		0	pulse_t2	pulse_t1	ring_t2	ring_t1	osc2_t2	osc2_t1	osc1_t2	osc1_t1	*/
#define	IRQ_VEC2	19	/*		0	pq6	pq5	pq4	pq3	pq2	pq1	loop_stat	ring_trip	*/
#define	IRQ_VEC3	20	/*		0						cal_cm_bal	indirect	dtmf	*/
#define	IRQ_MASK1	21	/*		0	pulse_t2	pulse_t1	ring_t2	ring_t1	osc2_t2	osc2_t1	osc1_t2	osc1_t1	*/
#define	IRQ_MASK2	22	/*		0	pq6	pq5	pq4	pq3	pq2	pq1	loop_stat	ring_trip	*/
#define	IRQ_MASK3	23	/*		0						cal_cm_bal	indirect	dtmf	*/
#define	DTMF		24	/*		0				valid		dtmf_digit[3:0]		 	*/
#define	PASS_LO		25	/*		0				dtmf_pass[7:0]					*/
#define	PASS_HI		26	/*		0							dtmf_pass[9:8]		*/

/******************INDIRECT REGISTER ACCESS	***************************/
#define	IND_DATA_LO	28	/*		0			ind_data[7:0]						*/
#define	IND_DATA_HI	29	/*		0			ind_data[15:8]						*/
#define	IND_ADDR	30	/*		0			ind_add[7:0]						*/
#define	IND_STAT	31	/*		0	ind_add[8]							ind_stat	*/


/**************************OSCILLATORS**********************************/
#define	OSC1_CNTL	32	/*		0	en_sync	reload	zero_en	t1_en	t2_en	enable	routing[1:0]		*/
#define	OSC2_CNTL	33	/*		0	en_sync		zero_en	t1_en	t2_en	enable	routing[1:0]		*/
#define	RING_CNTL	34	/*		0	en_sync		ring_dac	t1_en	t2_en	ring_en	offset	trap	*/
#define	PULSE_CNTL	35	/*		0	en_sync		zero_en	t1_en	t2_en	enable	env_en		*/
#define	OSC1_T1_LO	36	/*		0				osc1_t1[7:0]					*/
#define	OSC1_T1_HI	37	/*		0				osc1_t1[15:8]					*/
#define	OSC1_T2_LO	38	/*		0				osc1_t2[7:0]					*/
#define	OSC1_T2_HI	39	/*		0				osc1_t2[15:8]					*/
#define	OSC2_T1_LO	40	/*		0				osc2_t1[7:0]					*/
#define	OSC2_T1_HI	41	/*		0				osc2_t1[15:8]					*/
#define	OSC2_T2_LO	42	/*		0				osc2_t2[7:0]					*/
#define	OSC2_T2_HI	43	/*		0				osc2_t2[15:8]					*/
#define	PULSE_T1_LO	44	/*		0				pulse_t1[7:0]					*/
#define	PULSE_T1_HI	45	/*		0				pulse_t1[15:8]					*/
#define	PULSE_T2_LO	46	/*		0				pulse_t2[7:0]					*/
#define	PULSE_T2_HI	47	/*		0				pulse_t2[15:8]					*/
#define	RING_T1_LO	48	/*		0				ring_t1[7:0]					*/
#define	RING_T1_HI	49	/*		0				ring_t1[15:8]					*/
#define	RING_T2_LO	50	/*		0				ring_t2[7:0]					*/
#define	RING_T2_HI	51	/*		0				ring_t2[15:8]					*/
#define	FSK_DATA	52	/*		0								fsk_data	*/

/*****************************	SLIC *********************************************/

#define	DBI_LCR_RING	63	/*		52				dbi_lcr_ring[6:0]					*/
#define	LINEFEED		64	/*		0			linefeed_shadow[2:0]				linefeed[2:0]		*/
#define	BJTBIAS			65	/*		b1100001		squelch	cap_bypass	en_bjtbias	bjtbias_oht[1:0]		bjtbias_act[1:0]		*/
#define	BAT_CNTL		66	/*		b1011				vov_1p5	set_vreg	extbat	batsel	track	*/
#define	AUTO			67	/*		b0011111		mancm	mandiff	en_speedup	autobat	autord	autold	autoopen	*/
#define	LCR_RTP			68	/*		0					 	dbi_raw	rtp	lcr	*/
#define	DBI_LCR			69	/*		10				dbi_lcr[6:0]					*/
#define	DBI_RTP			70	/*		10				dbi_rtp[6:0]					*/
#define	ILIM			71	/*		0							ilim[2:0]		*/
#define	VOC				72	/*		32		vocsgn			voc[5:0]				*/
#define	VCM				73	/*		2					vcm[5:0]				*/
#define	VBATH			74	/*		50					vbath[5:0]				*/
#define	VBATL			75	/*		16					vbatl[5:0]				*/
#define	PWR_PTR			76	/*		0							pwr_ptr[2:0]		*/
#define	PWR_OUT			77	/*		0				pwr_out[7:0]					*/
#define	LVS				78	/*		0				lvs[6:0]					*/
#define	LCS				79	/*		0				lcs[6:0]					*/
#define	VTIP			80	/*		0				vtip[7:0]					*/
#define	VRING			81	/*		0				vring[7:0]					*/
#define	VREG1			82	/*		0				vreg1[7:0]					*/
#define	VREG2			83	/*		0				vreg2[7:0]					*/
#define	IQ1				84	/*		0				iq1[7:0]					*/
#define	IQ2				85	/*		0				iq2[7:0]					*/
#define	IQ3				86	/*		0				iq3[7:0]					*/
#define	IQ4				87	/*		0				iq4[7:0]					*/
#define	IQ5				88	/*		0				iq5[7:0]					*/
#define	IQ6				89	/*		0				iq6[7:0]					*/

/**********************	DC/DC************************************************/
#define	DC_N			92	/*		255				dc_n[7:0]					*/
#define	DC_S_DELAY		93	/*		20	dcdc_cal		dc_ff			dc_s_delay[4:0]			*/
#define	DC_X			94	/*		0				dc_x[7:0]					*/
#define	DC_ERR			95	/*		x							dc_err[2:0]		*/
 
/************************CALIBRATION****************************************/
#define	CAL_R1			96	/*		h1f		cal	cal_spdup	cal_gmisr	cal_gmist	cal_ding	cal_cing	cal_ilim	*/
#define	CAL_R2			97	/*		h1f				cal_madc1	cal_madc2	cal_dac_o	cal_adc_o	cal_cm_bal	*/
#define	CAL_GMIS_ICR	98	/*		16						cal_gmis_icr[4:0]			*/
#define	CAL_GMIS_ICT	99	/*		16						cal_gmis_ict[4:0]			*/
#define	CAL_DIN_GAIN	100	/*		17						cal_din_gain[4:0]			*/
#define	CAL_CIN_GAIN	101	/*		17						cal_cin_gain[4:0]			*/
#define	CAL_ILIM		102	/*		8						cal_ilim[3:0]			*/
#define	CAL_MADC		103	/*		h88		cal_madc_gm2[3:0]				cal_madc_gm1[3:0]			*/
#define	CAL_DAC_O_A		104	/*		0		 	 	 	dac_os_p	dac_os_n	adc_os_p	adc_os_n	*/
#define	CAL_DAC_O_D		105	/*		0				dac_offset[7:0]					*/
#define	CAL_CM_BAL		106	/*		h20					cm_bal[5:0]				*/
#define	CAL_DCPK		107	/*		8					 	cal_dcpk[3:0]			*/


#define	REVC	108	/*		0	ilim_max	fsk_revc	dc_err_en	zs_ext	batsel_pd	lcr_sense	en_subtr	hyst_en	*/


/*	INDIRECT REGISTERS																				*/
																					
/*	Name	#		D	b15	b14	b13	b12	b11	b10	b9	b8	b7	b6	b5	b4	b3	b2	b1	b0	*/
																				
/*	DTMF				(Constant values provided by Silabs after DTMF performance evaluation)																*/
#define	ROW0_THR	0	/*	x	sign				row0_thr[11:0]												*/
#define	ROW1_THR	1	/*	x	sign				row1_thr[11:0]												*/
#define	ROW2_THR	2	/*	x	sign				row2_thr[11:0]												*/
#define	ROW3_THR	3	/*	x	sign				row3_thr[11:0]												*/
#define	COL_THR		4	/*	x	sign				col_thr[11:0]												*/
#define	FWD_TW_THR	5	/*	x					sign			fwd_tw_thr[7:0]									*/
#define	REV_TW_THR	6	/*	x					sign			rev_tw_thr[7:0]									*/
#define	ROW_REL_THR	7	/*	x					sign			row_rel_thr[7:0]									*/
#define	COL_REL_THR	8	/*	x					sign			col_rel_thr[7:0]									*/
#define	ROW_2ND_THR	9	/*	x					sign			row_2nd_thr[7:0]									*/
#define	COL_2ND_THR	10	/*	x					sign			col_2nd_thr[7:0]									*/
#define	PWR_MIN_THR	11	/*	x	sign				pwr_min_thr[15:0]												*/
#define	HOT_LIMIT	12	/*	x	sign				hot_limit[15:0]												*/
																					
/*	OSCILLATORS				(See descriptions of tone generation, ringing and pulse metering for quidelines on computing register values)																*/
#define	OSC1_COEFF	13	/*	x	sign				osc1_coeff[15:0]												*/
#define	OSC1_X_REG	14	/*	x	sign				osc1_x_reg[15:0]												*/
#define	OSC1_Y_REG	15	/*	x	sign				osc1_y_reg[15:0]												*/
#define	OSC2_COEFF	16	/*	x	sign				osc2_coeff[15:0]												*/
#define	OSC2_X_REG	17	/*	x	sign				osc2_x_reg[15:0]												*/
#define	OSC2_Y_REG	18	/*	x	sign				osc2_y_reg[15:0]												*/
#define	RING_OFFSET	19	/*	x	sign				ring_offset[15:0]												*/
#define	RING_COEFF	20	/*	x	sign				ring_coeff[15:0]												*/
#define	RING_X_REG	21	/*	x	sign				ring_x_reg[15:0]												*/
#define	RING_Y_REG	22	/*	x	sign				ring_y_reg[15:0]												*/
#define	PULSE_DELTA	23	/*	x	sign				pulse_delta[15:0]												*/
#define	PULSE_X_REG	24	/*	x	sign				pulse_x_reg[15:0]												*/
#define	PULSE_COEFF	25	/*	x	sign				pulse_coeff[15:0]												*/
																					
/*	PGAs				(See description of PGA for quidelines on computing register values)																*/
#define	DAC_GAIN	26	/*	x	sign				dac_gain[11:0]												*/
#define	ADC_GAIN	27	/*	x	sign				adc_gain[11:0]												*/
																					
/*	SLIC CONTROL																				*/
#define	LCR_THR		28	/*		0			lcr_thr_high[5:0]													*/
#define	RTP_THR		29	/*		0			rtp_thr[5:0]													*/
#define	CM_LOW_THR	30	/*		0	0	0			cm_low_thr[5:0]											*/
#define	CM_HIGH_THR	31	/*		0	0	0			cm_high_thr[5:0]											*/
#define	PPTHR12		32	/*		0				ppthr12[7:0]												*/
#define	PPTHR34		33	/*		0				ppthr34[7:0]												*/
#define	NPTHR56		34	/*		0				npthr56[7:0]												*/
#define	NLCR		35	/*						nclr[12:0]												*/
#define	NRTP		36	/*						nrtp[12:0]												*/
#define	NQ12		37	/*						nq12[12:0]												*/
#define	NQ34		38	/*						nq34[12:0]												*/
#define	NQ56		39	/*						nq56[12:0]												*/
#define	VCM_RING_DELTA	40	/*		0	0	0		vcm_ring_delta[3:0]												*/
#define	VMIN_DELTA	41	/*		0	0	0		vmin_delta[3:0]												*/
#define	VXTRA_DELTA	42	/*		0	0	0		vxtra_delta[3:0]												*/
#define	LCR_THR2	43	/*		0			lcr_thr_low[5:0]													*/
																					
#define	FSK_X_0		99	/*	x	sign				fsk_x_0[15:0]												*/
#define	FSK_COEFF_0	100	/*	x	sign				fsk_coeff_0[15:0]												*/
#define	FSK_X_1		101	/*	x	sign				fsk_x_1[15:0]												*/
#define	FSK_COEFF_1	102	/*	x	sign				fsk_coeff_1[15:0]												*/
#define	FSK_X_01	103	/*	x	sign				fsk_x_01[15:0]												*/
#define	FSK_X_10	104	/*	x	sign				fsk_x_10[15:0]												*/





#endif  /* REAL NAMES */


enum exceptions {
	PROSLICiNSANE,
	TIMEoUTpOWERuP,
	TIMEoUTpOWERdOWN,
	POWERlEAK,
	TIPoRrINGgROUNDsHORT,
	POWERaLARMQ1,
	POWERaLARMQ2,
	POWERaLARMQ3,
	POWERaLARMQ4, 
	POWERaLARMQ5,
	POWERaLARMQ6
};



#define	SI3210_DIALTONE_IR13	0x7b30	//tone_struct DialTone = {  /* OSC1= 350 Hz OSC2= 440 Hz .0975 Volts -18 dBm */
#define	SI3210_DIALTONE_IR14	0x0063	
#define	SI3210_DIALTONE_IR16	0x7870	
#define	SI3210_DIALTONE_IR17	0x007d	

#define	SI3215_DIALTONE_IR13	0x7eca	//tone_struct DialTone = {  /* OSC1= 350 Hz OSC2= 440 Hz .0975 Volts -18 dBm */
#define	SI3215_DIALTONE_IR14	0x0031	
#define	SI3215_DIALTONE_IR16	0x7e18	
#define	SI3215_DIALTONE_IR17	0x003e	
	
#define	DIALTONE_DR32	6	
#define	DIALTONE_DR33	6	
#define	DIALTONE_DR36	0	
#define	DIALTONE_DR37	0	
#define	DIALTONE_DR38	0	
#define	DIALTONE_DR39	0	
#define	DIALTONE_DR40	0	
#define	DIALTONE_DR41	0	
#define	DIALTONE_DR42	0	
#define	DIALTONE_DR43	0



#define	SI3210_REORDERTONE_IR13	0x7700	//"//tone_struct ReorderTone = {	/* OSC1= 480 Hz OSC2 = 620 .0975 Volts -18 dBm */"
#define	SI3210_REORDERTONE_IR14	0x0089	
#define	SI3210_REORDERTONE_IR16	0x7120	
#define	SI3210_REORDERTONE_IR17	0x00B2

#define	SI3215_REORDERTONE_IR13	0x7DBB	//"//tone_struct ReorderTone = {	/* OSC1= 480 Hz OSC2 = 620 .0975 Volts -18 dBm */"
#define	SI3215_REORDERTONE_IR14	0x0044	
#define	SI3215_REORDERTONE_IR16	0x7C39	
#define	SI3215_REORDERTONE_IR17	0x0058

	
#define	REORDERTONE_DR32	0x1E	
#define	REORDERTONE_DR33	0x1E	
#define	REORDERTONE_DR36	0x60	
#define	REORDERTONE_DR37	0x09	
#define	REORDERTONE_DR38	0x40	
#define	REORDERTONE_DR39	0x06	
#define	REORDERTONE_DR40	0x60	
#define	REORDERTONE_DR41	0x09	
#define	REORDERTONE_DR42	0x40	
#define	REORDERTONE_DR43	0x06	


//one_struct BusySignal = { /* OSC1= 480  OSC2 = 620 .0975 Voltz -18 dBm 8*/
//	{0x7700,0x0089,0,0x0f,0xa0,0x0f,0xa0},{0x7120,0x00b2,0,0x0f,0xa0,0x0f,0xa0}
//};
#define	SI3210_BUSYTONE_IR13	0x7700	//tone_struct BusySignal = { /* OSC1= 480  OSC2 = 620 .0975 Voltz -18 dBm 8*/
#define	SI3210_BUSYTONE_IR14	0x0089	
#define	SI3210_BUSYTONE_IR16	0x7120	
#define	SI3210_BUSYTONE_IR17	0x00B2	

#define	SI3215_BUSYTONE_IR13	0x7DBB	//tone_struct BusySignal = { /* OSC1= 480  OSC2 = 620 .0975 Voltz -18 dBm 8*/
#define	SI3215_BUSYTONE_IR14	0x0044	
#define	SI3215_BUSYTONE_IR16	0x7C39	
#define	SI3215_BUSYTONE_IR17	0x0058	

#define	BUSYTONE_DR32	0x1E	
#define	BUSYTONE_DR33	0x1E	
#define	BUSYTONE_DR36	0xa0	
#define	BUSYTONE_DR37	0x0f	
#define	BUSYTONE_DR38	0xa0	
#define	BUSYTONE_DR39	0x0f	
#define	BUSYTONE_DR40	0xa0	
#define	BUSYTONE_DR41	0x0f	
#define	BUSYTONE_DR42	0xa0	
#define	BUSYTONE_DR43	0x0f	

//tone_struct RingbackNormal = { /* OSC1 = 440 Hz OSC2 = 480 .0975 Volts -18 dBm */
#define	SI3210_RINGBACKTONE_IR13	0x7870	
#define	SI3210_RINGBACKTONE_IR14	0x007D	
#define	SI3210_RINGBACKTONE_IR16	0x7700	
#define	SI3210_RINGBACKTONE_IR17	0x0089	

#define	SI3215_RINGBACKTONE_IR13	0x7E18	
#define	SI3215_RINGBACKTONE_IR14	0x003E	
#define	SI3215_RINGBACKTONE_IR16	0x7DBB	
#define	SI3215_RINGBACKTONE_IR17	0x0044	

#define	RINGBACKTONE_DR32	0x1E	
#define	RINGBACKTONE_DR33	0x1E	
#define	RINGBACKTONE_DR36	0x80	
#define	RINGBACKTONE_DR37	0x3E	
#define	RINGBACKTONE_DR38	0x0	
#define	RINGBACKTONE_DR39	0x7d	
#define	RINGBACKTONE_DR40	0x80	
#define	RINGBACKTONE_DR41	0x3E	
#define	RINGBACKTONE_DR42	0x0	
#define	RINGBACKTONE_DR43	0x7d	
			
#define	SI3210_RINGBACKPBXTONE_IR13	0x7870 //	"//tone_struct RingbackPBX = {	/* OSC1 = 440 Hz OSC2= 480 .0975 Volts -18 dBM */"
#define	SI3210_RINGBACKPBXTONE_IR14	0x007D	
#define	SI3210_RINGBACKPBXTONE_IR16	0x7700	
#define	SI3210_RINGBACKPBXTONE_IR17	0x0089	

#define	SI3215_RINGBACKPBXTONE_IR13	0x7E18 //	"//tone_struct RingbackPBX = {	/* OSC1 = 440 Hz OSC2= 480 .0975 Volts -18 dBM */"
#define	SI3215_RINGBACKPBXTONE_IR14	0x003E	
#define	SI3215_RINGBACKPBXTONE_IR16	0x7DBB	
#define	SI3215_RINGBACKPBXTONE_IR17	0x0044

#define	RINGBACKPBXTONE_DR32	0x1E	
#define	RINGBACKPBXTONE_DR33	0x1E	
#define	RINGBACKPBXTONE_DR36	0x40	
#define	RINGBACKPBXTONE_DR37	0x1f	
#define	RINGBACKPBXTONE_DR38	0xc0	
#define	RINGBACKPBXTONE_DR39	0x5d	
#define	RINGBACKPBXTONE_DR40	0x40	
#define	RINGBACKPBXTONE_DR41	0x1f	
#define	RINGBACKPBXTONE_DR42	0xc0	
#define	RINGBACKPBXTONE_DR43	0x5d	

#define	SI3210_CONGESTIONTONE_IR13	0x7700	//tone_struct CongestionTone = { /* OSC1= 480 Hz OSC2 = 620 .0975 Volts -18 dBM */
#define	SI3210_CONGESTIONTONE_IR14	0x0089	
#define	SI3210_CONGESTIONTONE_IR16	0x7120	
#define	SI3210_CONGESTIONTONE_IR17	0x00B2	

#define	SI3215_CONGESTIONTONE_IR13	0x7DBB	//tone_struct CongestionTone = { /* OSC1= 480 Hz OSC2 = 620 .0975 Volts -18 dBM */
#define	SI3215_CONGESTIONTONE_IR14	0x0044	
#define	SI3215_CONGESTIONTONE_IR16	0x7C39	
#define	SI3215_CONGESTIONTONE_IR17	0x0058	

#define	CONGESTIONTONE_DR32	0x1E	
#define	CONGESTIONTONE_DR33	0x1E	
#define	CONGESTIONTONE_DR36	0x40	
#define	CONGESTIONTONE_DR37	0x06	
#define	CONGESTIONTONE_DR38	0x60	
#define	CONGESTIONTONE_DR39	0x09	
#define	CONGESTIONTONE_DR40	0x40	
#define	CONGESTIONTONE_DR41	0x06	
#define	CONGESTIONTONE_DR42	0x60	
#define	CONGESTIONTONE_DR43	0x09	
			
			
#define	HZ350_IR13	0x7b30	//350    /* HERTZ */, 0x7b30, 0x0063,
#define	HZ350_IR14	0x0063	
			
#define	HZ440_IR13	0x7870	//440    /* HERTZ */ ,0x7870, 0x007d,
#define	HZ440_IR14	0x007d	
			
#define	HZ480_IR13	0x7700	//480    /* HERTZ */, 0x7700, 0x0089,
#define	HZ480_IR14	0x0089	
			
#define	HZ620_IR13	0x7120	//620    /* HERTZ */, 0x7120, 0x00b2,
#define	HZ620_IR14	0x00b2	
			
#define	HZ200_IR13	0x7e70	//200    /* HERTZ */, 0x7e70, 0x0038,
#define	HZ200_IR14	0x0038	
			
#define	HZ300_IR13	0x7c70	//300    /* HERTZ */, 0x7c70, 0x0055,
#define	HZ300_IR14	0x0055	
			
#define	HZ400_IR13	0x79c0	//400    /* HERTZ */, 0x79c0, 0x0071
#define	HZ400_IR14	0x0071	
			

#define	DTMF0_IR13	0x5ea0	"//	{{0x5ea0,0x08a5,0,0x03,0x20,0,0}, {0x3fc0,0x0cec,0,0x03,0x20,0,0}}, // DTMF 0"
#define	DTMF0_IR14	0x08a5	
#define	DTMF0_IR16	0x3fc0	
#define	DTMF0_IR17	0x0cec	
#define	DTMF0_DR32	0x1E	
#define	DTMF0_DR33	0x1E	
#define	DTMF0_DR36	0x03	
#define	DTMF0_DR37	0x20	
#define	DTMF0_DR38	0x03	
#define	DTMF0_DR39	0x60	
#define	DTMF0_DR40	0x40	
#define	DTMF0_DR41	0x20	
#define	DTMF0_DR42	0x60	
#define	DTMF0_DR43	0x09	

#define	DTMF1_IR13	0x6d50	"//	{{0x6d50,0x0643,0,0x03,0x20,0,0}, {0x4a80,0x0b79,0,0x03,0x20,0,0}}, // DTMF 1"
#define	DTMF1_IR14	0x0643	
#define	DTMF1_IR16	0x4a80	
#define	DTMF1_IR17	0x0b79	
#define	DTMF1_DR32	0x1E	
#define	DTMF1_DR33	0x1E	
#define	DTMF1_DR36	0x20	 
#define	DTMF1_DR37	0x03	
#define	DTMF1_DR38	0	
#define	DTMF1_DR39	0	
#define	DTMF1_DR40	0x20	
#define	DTMF1_DR41	0x03	
#define	DTMF1_DR42	0	
#define	DTMF1_DR43	0	

#define	DTMF2_IR13	0x6d50	"//	{{0x6d50,0x0643,0,0x03,0x20,0,0}, {0x3fc0,0x0cec,0,0x03,0x20,0,0}}, // DTMF 2"
#define	DTMF2_IR14	0x0643	
#define	DTMF2_IR16	0x3fc0	
#define	DTMF2_IR17	0x0cec	
#define	DTMF2_DR32	0x1E	
#define	DTMF2_DR33	0x1E	
#define	DTMF2_DR36	0x20	
#define	DTMF2_DR37	0x03	
#define	DTMF2_DR38	0	
#define	DTMF2_DR39	0	
#define	DTMF2_DR40	0x20	
#define	DTMF2_DR41	0x03	
#define	DTMF2_DR42	0	
#define	DTMF2_DR43	0	

#define	DTMF3_IR13	0x6d50	"//	{{0x6d50,0x0643,0,0x03,0x20,0,0}, {0x3320,0x0ea0,0,0x03,0x20,0,0}}, // DTMF 3"
#define	DTMF3_IR14	0x0643	
#define	DTMF3_IR16	0x3320	
#define	DTMF3_IR17	0x0ea0	
#define	DTMF3_DR32	0x1E	
#define	DTMF3_DR33	0x1E	
#define	DTMF3_DR36	0x20	
#define	DTMF3_DR37	0x03	
#define	DTMF3_DR38	0	
#define	DTMF3_DR39	0	
#define	DTMF3_DR40	0x20	
#define	DTMF3_DR41	0x03	
#define	DTMF3_DR42	0	
#define	DTMF3_DR43	0	

#define	DTMF4_IR13	0x6950	"//	{{0x6950,0x06f6,0,0x03,0x20,0,0}, {0x4a80,0x0b79,0,0x03,0x20,0,0}}, // DTMF 4"
#define	DTMF4_IR14	0x0616	
#define	DTMF4_IR16	0x4a80	
#define	DTMF4_IR17	0x0b79	
#define	DTMF4_DR32	0x1E	
#define	DTMF4_DR33	0x1E	
#define	DTMF4_DR36	0x20	
#define	DTMF4_DR37	0x03	
#define	DTMF4_DR38	0	
#define	DTMF4_DR39	0	
#define	DTMF4_DR40	0x20	
#define	DTMF4_DR41	0x03	
#define	DTMF4_DR42	0	
#define	DTMF4_DR43	0	

#define	DTMF5_IR13	0x6950	"//	{{0x6950,0x06f6,0,0x03,0x20,0,0}, {0x3fc0,0x0cec,0,0x03,0x20,0,0}}, // DTMF 5"
#define	DTMF5_IR14	0x0616	
#define	DTMF5_IR16	0x3fc0	
#define	DTMF5_IR17	0x0cec	
#define	DTMF5_DR32	0x1E	
#define	DTMF5_DR33	0x1E	
#define	DTMF5_DR36	0x20	
#define	DTMF5_DR37	0x03	
#define	DTMF5_DR38	0	
#define	DTMF5_DR39	0	
#define	DTMF5_DR40	0x20	
#define	DTMF5_DR41	0x03	
#define	DTMF5_DR42	0	
#define	DTMF5_DR43	0	

#define	DTMF6_IR13	0x6950	"//	{{0x6950,0x06f6,0,0x03,0x20,0,0}, {0x3320,0x0ea0,0,0x03,0x20,0,0}}, // DTMF 6"
#define	DTMF6_IR14	0x0616	
#define	DTMF6_IR16	0x3320	
#define	DTMF6_IR17	0xea0	
#define	DTMF6_DR32	0x1E	
#define	DTMF6_DR33	0x1E	
#define	DTMF6_DR36	0x20	
#define	DTMF6_DR37	0x03	
#define	DTMF6_DR38	0	
#define	DTMF6_DR39	0	
#define	DTMF6_DR40	0x20	
#define	DTMF6_DR41	0x03	
#define	DTMF6_DR42	0	
#define	DTMF6_DR43	0	

#define	DTMF7_IR13	0x6460	"//	{{0x6460,0x07c3,0,0x03,0x20,0,0}, {0x4a80,0x0b79,0,0x03,0x20,0,0}}, // DTMF 7"
#define	DTMF7_IR14	0x07c3	
#define	DTMF7_IR16	0x4a80	
#define	DTMF7_IR17	0x0b79	
#define	DTMF7_DR32	0x1E	
#define	DTMF7_DR33	0x1E	
#define	DTMF7_DR36	0x20	
#define	DTMF7_DR37	0x03	
#define	DTMF7_DR38	0	
#define	DTMF7_DR39	0	
#define	DTMF7_DR40	0x20	
#define	DTMF7_DR41	0x03	
#define	DTMF7_DR42	0	
#define	DTMF7_DR43	0	

#define	DTMF8_IR13	0x6460	"//	{{0x6460,0x07c3,0,0x03,0x20,0,0}, {0x3fc0,0x0cec,0,0x03,0x20,0,0}}, // DTMF 8"
#define	DTMF8_IR14	0x07c3	
#define	DTMF8_IR16	0x3fc0	
#define	DTMF8_IR17	0x0cec	
#define	DTMF8_DR32	0x1E	
#define	DTMF8_DR33	0x1E	
#define	DTMF8_DR36	0x20	
#define	DTMF8_DR37	0x03	
#define	DTMF8_DR38	0	
#define	DTMF8_DR39	0	
#define	DTMF8_DR40	0x20	
#define	DTMF8_DR41	0x03	
#define	DTMF8_DR42	0	
#define	DTMF8_DR43	0	

#define	DTMF9_IR13	0x6460	"//	{{0x6460,0x07c3,0,0x03,0x20,0,0}, {0x3320,0x0ea0,0,0x03,0x20,0,0}}, // DTMF 9"
#define	DTMF9_IR14	0x07c3	
#define	DTMF9_IR16	0x3320	
#define	DTMF9_IR17	0x0ea0	
#define	DTMF9_DR32	0x1E	
#define	DTMF9_DR33	0x1E	
#define	DTMF9_DR36	0x20	
#define	DTMF9_DR37	0x03	
#define	DTMF9_DR38	0	
#define	DTMF9_DR39	0	
#define	DTMF9_DR40	0x20	
#define	DTMF9_DR41	0x03	
#define	DTMF9_DR42	0	
#define	DTMF9_DR43	0	

#define	DTMFA_IR13	0x6d50	"//	{{0x6d50,0x0643,0,0x03,0x20,0,0}, {0x2460,0x10ab,0,0x03,0x20,0,0}}, // DTMF A"
#define	DTMFA_IR14	0x0643	
#define	DTMFA_IR16	0x2460	
#define	DTMFA_IR17	0x10ab	
#define	DTMFA_DR32	0x1E	
#define	DTMFA_DR33	0x1E	
#define	DTMFA_DR36	0x20	
#define	DTMFA_DR37	0x03	
#define	DTMFA_DR38	0	
#define	DTMFA_DR39	0	
#define	DTMFA_DR40	0x20	
#define	DTMFA_DR41	0x03	
#define	DTMFA_DR42	0	
#define	DTMFA_DR43	0	

#define	DTMFB_IR13	0x6950	"//	{{0x6950,0x06f6,0,0x03,0x20,0,0}, {0x2460,0x10ab,0,0x03,0x20,0,0}}, // DTMF B"
#define	DTMFB_IR14	0x0616	
#define	DTMFB_IR16	0x2460	
#define	DTMFB_IR17	0x10ab	
#define	DTMFB_DR32	0x1E	
#define	DTMFB_DR33	0x1E	
#define	DTMFB_DR36	0x20	
#define	DTMFB_DR37	0x03	
#define	DTMFB_DR38	0	
#define	DTMFB_DR39	0	
#define	DTMFB_DR40	0x20	
#define	DTMFB_DR41	0x03	
#define	DTMFB_DR42	0	
#define	DTMFB_DR43	0	

#define	DTMFC_IR13	0x6460	"//	{{0x6460,0x07c3,0,0x03,0x20,0,0}, {0x2460,0x10ab,0,0x03,0x20,0,0}}, // DTMF C"
#define	DTMFC_IR14	0x07c3	
#define	DTMFC_IR16	0x2460	
#define	DTMFC_IR17	0x10ab	
#define	DTMFC_DR32	0x1E	
#define	DTMFC_DR33	0x1E	
#define	DTMFC_DR36	0x20	
#define	DTMFC_DR37	0x03	
#define	DTMFC_DR38	0	
#define	DTMFC_DR39	0	
#define	DTMFC_DR40	0x20	
#define	DTMFC_DR41	0x03	
#define	DTMFC_DR42	0	
#define	DTMFC_DR43	0	

#define	DTMFD_IR13	0x5ea0	"//	{{0x5ea0,0x08a5,0,0x03,0x20,0,0}, {0x2460,0x10ab,0,0x03,0x20,0,0}}  // DTMF D"
#define	DTMFD_IR14	0x08a5	
#define	DTMFD_IR16	0x2460	
#define	DTMFD_IR17	0x10ab	
#define	DTMFD_DR32	0x1E	
#define	DTMFD_DR33	0x1E	
#define	DTMFD_DR36	0x20	
#define	DTMFD_DR37	0x03	
#define	DTMFD_DR38	0	
#define	DTMFD_DR39	0	
#define	DTMFD_DR40	0x20	
#define	DTMFD_DR41	0x03	
#define	DTMFD_DR42	0	
#define	DTMFD_DR43	0	

#define	DTMFstar_IR13	0x5ea0	"//	{{0x5ea0,0x08a5,0,0x03,0x20,0,0}, {0x4a80,0x0b79,0,0x03,0x20,0,0}}, // DTMF star"
#define	DTMFstar_IR14	0x8a50	
#define	DTMFstar_IR16	0x4a80	
#define	DTMFstar_IR17	0x0b79	
#define	DTMFstar_DR32	0x1E	
#define	DTMFstar_DR33	0x1E	
#define	DTMFstar_DR36	0x20	
#define	DTMFstar_DR37	0x03	
#define	DTMFstar_DR38	0	
#define	DTMFstar_DR39	0	
#define	DTMFstar_DR40	0x20	
#define	DTMFstar_DR41	0x03	
#define	DTMFstar_DR42	0	
#define	DTMFstar_DR43	0	

#define	DTMFhash_IR13	0x5ae0	"//	{{0x5ea0,0x08a5,0,0x03,0x20,0,0}, {0x3320,0x0ea0,0,0x03,0x20,0,0}}, // DTMF #"
#define	DTMFhash_IR14	0x08a5	
#define	DTMFhash_IR16	0x3320	
#define	DTMFhash_IR17	0x0ea0	
#define	DTMFhash_DR32	0x1E	
#define	DTMFhash_DR33	0x1E	
#define	DTMFhash_DR36	0x20	
#define	DTMFhash_DR37	0x03	
#define	DTMFhash_DR38	0	
#define	DTMFhash_DR39	0	
#define	DTMFhash_DR40	0x20	
#define	DTMFhash_DR41	0x03	
#define	DTMFhash_DR42	0	
#define	DTMFhash_DR43	0	


#define	PULSE_METERING_IR24	0x3190	
#define	PULSE_METERING_IR25	0x4FF	
#define	PULSE_METERING_DR35	0x10	
#define	PULSE_METERING_DR44	0x60	0x04
#define	PULSE_METERING_DR45	0x04	0x60
#define	PULSE_METERING_DR46	0x60	0x04
#define	PULSE_METERING_DR47	0x04	0x60

#define	RINGING_20HZ_IR19	0	//offset voltage
#define	RINGING_20HZ_IR20	0x7F00	
#define	RINGING_20HZ_IR21	0x013D	
#define	RINGING_20HZ_DR34	0X18	
#define	RINGING_20HZ_DR48	0x80	
#define	RINGING_20HZ_DR49	0x3e	
#define	RINGING_20HZ_DR50	0x7d	
#define	RINGING_20HZ_DR51	0	






//tone_struct DialTone = {  /* OSC1= 350 Hz OSC2= 440 Hz .0975 Volts -18 dBm */
//	{0x7ed0,0x0031,0,0,0,0,0},{0x7e20,0x003d,0,0,0,0,0}
//};

#define	DIALTONE_SI3216_IR13	0x7ed0	
#define	DIALTONE_SI3216_IR14	0x0031	
#define	DIALTONE_SI3216_IR16	0x7e20	
#define	DIALTONE_SI3216_IR17	0x003d	
#define	DIALTONE_SI3216_DR32	6	
#define	DIALTONE_SI3216_DR33	6	
#define	DIALTONE_SI3216_DR36	0	
#define	DIALTONE_SI3216_DR37	0	
#define	DIALTONE_SI3216_DR38	0	
#define	DIALTONE_SI3216_DR39	0	
#define	DIALTONE_SI3216_DR40	0	
#define	DIALTONE_SI3216_DR41	0	
#define	DIALTONE_SI3216_DR42	0	
#define	DIALTONE_SI3216_DR43	0	

//tone_struct ReorderTone = {	/* OSC1= 480 Hz OSC2 = 620 .0975 Volts -18 dBm */
//	{0x7dc0,0x0043,0,0x09,0x60,0x06,0x40},{0x7c40,0x0057,0,0x09,0x60,0x06,0x40}
//};

#define	REORDERTONE_SI3216_IR13	0x7dc0	
#define	REORDERTONE_SI3216_IR14	0x0043	
#define	REORDERTONE_SI3216_IR16	0x7c40	
#define	REORDERTONE_SI3216_IR17	0x0057	
#define	REORDERTONE_SI3216_DR32	0x1E	
#define	REORDERTONE_SI3216_DR33	0x1E	
#define	REORDERTONE_SI3216_DR36	0x60	
#define	REORDERTONE_SI3216_DR37	0x09	
#define	REORDERTONE_SI3216_DR38	0x40	
#define	REORDERTONE_SI3216_DR39	0x06	
#define	REORDERTONE_SI3216_DR40	0x60	
#define	REORDERTONE_SI3216_DR41	0x09	
#define	REORDERTONE_SI3216_DR42	0x40	
#define	REORDERTONE_SI3216_DR43	0x06

//tone_struct CongestionTone = { /* OSC1= 480 Hz OSC2 = 620 .0975 Volts -18 dBM */
//	{0x7dc0,0x0043,0,0x06,0x40,0x09,0x60},{0x7c40,0x0057,0,0x06,0x40,0x09,0x60}
//};
	

#define	BUSYTONE_SI3216_IR13	0x7dc0	
#define	BUSYTONE_SI3216_IR14	0x0043	
#define	BUSYTONE_SI3216_IR16	0x7c40	
#define	BUSYTONE_SI3216_IR17	0x0057	
#define	BUSYTONE_SI3216_DR32	0x1E	
#define	BUSYTONE_SI3216_DR33	0x1E	
#define	BUSYTONE_SI3216_DR36	0xa0	
#define	BUSYTONE_SI3216_DR37	0x0f	
#define	BUSYTONE_SI3216_DR38	0xa0	
#define	BUSYTONE_SI3216_DR39	0x0f	
#define	BUSYTONE_SI3216_DR40	0xa0	
#define	BUSYTONE_SI3216_DR41	0x0f	
#define	BUSYTONE_SI3216_DR42	0xa0	
#define	BUSYTONE_SI3216_DR43	0x0f


//tone_struct RingbackPBX = {	/* OSC1 = 440 Hz OSC2= 480 .0975 Volts -18 dBM */
//	{0x7e20,0x003d,0,0x1f,0x40,0x5d,0xc0},{0x7dc0,0x0043,0,0x1f,0x40,0x5d,0xc0}	
			
#define	RINGBACKTONE_SI3216_IR13	0x7e20	
#define	RINGBACKTONE_SI3216_IR14	0x003d	
#define	RINGBACKTONE_SI3216_IR16	0x7dc0	
#define	RINGBACKTONE_SI3216_IR17	0x0043	
#define	RINGBACKTONE_SI3216_DR32	0x1E	
#define	RINGBACKTONE_SI3216_DR33	0x1E	
#define	RINGBACKTONE_SI3216_DR36	0x80	
#define	RINGBACKTONE_SI3216_DR37	0x3E	
#define	RINGBACKTONE_SI3216_DR38	0x0	
#define	RINGBACKTONE_SI3216_DR39	0x7d	
#define	RINGBACKTONE_SI3216_DR40	0x80	
#define	RINGBACKTONE_SI3216_DR41	0x3E	
#define	RINGBACKTONE_SI3216_DR42	0x0	
#define	RINGBACKTONE_SI3216_DR43	0x7d	


//tone_struct RingbackPBX = {	/* OSC1 = 440 Hz OSC2= 480 .0975 Volts -18 dBM */
//	{0x7e20,0x003d,0,0x1f,0x40,0x5d,0xc0},{0x7dc0,0x0043,0,0x1f,0x40,0x5d,0xc0}
//};

			
#define	RINGBACKPBXTONE_SI3216_IR13	0x7e20	
#define	RINGBACKPBXTONE_SI3216_IR14	0x003d	
#define	RINGBACKPBXTONE_SI3216_IR16	0x7dc0	
#define	RINGBACKPBXTONE_SI3216_IR17	0x0043	
#define	RINGBACKPBXTONE_SI3216_DR32	0x1E	
#define	RINGBACKPBXTONE_SI3216_DR33	0x1E	
#define	RINGBACKPBXTONE_SI3216_DR36	0x40	
#define	RINGBACKPBXTONE_SI3216_DR37	0x1f	
#define	RINGBACKPBXTONE_SI3216_DR38	0xc0	
#define	RINGBACKPBXTONE_SI3216_DR39	0x5d	
#define	RINGBACKPBXTONE_SI3216_DR40	0x40	
#define	RINGBACKPBXTONE_SI3216_DR41	0x1f	
#define	RINGBACKPBXTONE_SI3216_DR42	0xc0	
#define	RINGBACKPBXTONE_SI3216_DR43	0x5d	


//tone_struct CongestionTone = { /* OSC1= 480 Hz OSC2 = 620 .0975 Volts -18 dBM */
//	{0x7dc0,0x0043,0,0x06,0x40,0x09,0x60},{0x7c40,0x0057,0,0x06,0x40,0x09,0x60}
//};


#define	CONGESTIONTONE_SI3216_IR13	0x7dc0
#define	CONGESTIONTONE_SI3216_IR14	0x0043	
#define	CONGESTIONTONE_SI3216_IR16	0x7c40	
#define	CONGESTIONTONE_SI3216_IR17	0x0057	
#define	CONGESTIONTONE_SI3216_DR32	0x1E	
#define	CONGESTIONTONE_SI3216_DR33	0x1E	
#define	CONGESTIONTONE_SI3216_DR36	0x40	
#define	CONGESTIONTONE_SI3216_DR37	0x06	
#define	CONGESTIONTONE_SI3216_DR38	0x60	
#define	CONGESTIONTONE_SI3216_DR39	0x09	
#define	CONGESTIONTONE_SI3216_DR40	0x40	
#define	CONGESTIONTONE_SI3216_DR41	0x06	
#define	CONGESTIONTONE_SI3216_DR42	0x60	
#define	CONGESTIONTONE_SI3216_DR43	0x09


//tone_struct RingbackJapan = { /* OSC1 = 400 Hz OSC2 = 435 .0975 Volts -18 dBm */
//	{0x79c0,0x00e9,0,0x1f,0x40,0x3e,0x80},{0x7940,0x00f2,0,0x1f,0x40,0x3e,0x80}

//};

#define	SI3210_RINGBACKJAPANTONE_IR13	0x79C0	
#define	SI3210_RINGBACKJAPANTONE_IR14	0xe9	
#define	SI3210_RINGBACKJAPANTONE_IR16	0x7940	
#define	SI3210_RINGBACKJAPANTONE_IR17	0xf2

#define	SI3215_RINGBACKJAPANTONE_IR13	0x7E6C	
#define	SI3215_RINGBACKJAPANTONE_IR14	0x38	
#define	SI3215_RINGBACKJAPANTONE_IR16	0x7E23	
#define	SI3215_RINGBACKJAPANTONE_IR17	0x3D
	
#define	RINGBACKJAPANTONE_DR32	0x1E	
#define	RINGBACKJAPANTONE_DR33	0x1E	
#define	RINGBACKJAPANTONE_DR36	0x40	
#define	RINGBACKJAPANTONE_DR37	0x1f	
#define	RINGBACKJAPANTONE_DR38	0x80
#define	RINGBACKJAPANTONE_DR39	0x3e	
#define	RINGBACKJAPANTONE_DR40	0x40	
#define	RINGBACKJAPANTONE_DR41	0x1f	
#define	RINGBACKJAPANTONE_DR42	0x80
#define	RINGBACKJAPANTONE_DR43	0x3e	



//tone_struct BusyJapan = { /* OSC1 = 400 Hz OSC2 = 435 .0975 Volts -18 dBm */
//	{0x79c0,0x00e9,0,0x0f,0xa0,0x0f,0xa0},{0,0,0,0,0,0,0}

//};

#define	SI3210_BUSYJAPANTONE_IR13	0x79c0	
#define	SI3210_BUSYJAPANTONE_IR14	0x00e9	
#define	SI3210_BUSYJAPANTONE_IR16	0
#define	SI3210_BUSYJAPANTONE_IR17	0

#define	SI3215_BUSYJAPANTONE_IR13	0x7E6C	
#define	SI3215_BUSYJAPANTONE_IR14	0x0038	
#define	SI3215_BUSYJAPANTONE_IR16	0
#define	SI3215_BUSYJAPANTONE_IR17	0
	
#define	BUSYJAPANTONE_DR32	0x1E	
#define	BUSYJAPANTONE_DR33	0	
#define	BUSYJAPANTONE_DR36	0xa0	
#define	BUSYJAPANTONE_DR37	0x0f	
#define	BUSYJAPANTONE_DR38	0xa0
#define	BUSYJAPANTONE_DR39	0x0f
#define	BUSYJAPANTONE_DR40	0	
#define	BUSYJAPANTONE_DR41	0	
#define	BUSYJAPANTONE_DR42	0
#define	BUSYJAPANTONE_DR43	0	








#define	FLUSH_USING_IR34	0x8000	//	By seting this value to 0x8000 it accelearates data through Filter
#define	FLUSH_USING_IR35	0x8000	//	By seting this value to 0x8000 it accelearates data through Filter
#define	FLUSH_USING_IR36	0x8000	//	By seting this value to 0x8000 it accelearates data through Filter
#define	FLUSH_USING_IR37	0x8000	//	By seting this value to 0x8000 it accelearates data through Filter
#define	FLUSH_USING_IR38	0x8000	//	By seting this value to 0x8000 it accelearates data through Filter
#define	FLUSH_USING_IR39	0x8000	//	By seting this value to 0x8000 it accelearates data through Filter

#define	FLUSH_IR89	0x0000	//	Clear value in power accumulator for Q1
#define	FLUSH_IR90	0x0000	//	Clear value in power accumulator for Q2
#define	FLUSH_IR91	0x0000	//	Clear value in power accumulator for Q3
#define	FLUSH_IR92	0x0000	//	Clear value in power accumulator for Q4
#define	FLUSH_IR93	0x0000	//	Clear value in power accumulator for Q5
#define	FLUSH_IR94	0x0000	//	Clear value in power accumulator for Q6



				
#define	BIT_CALM1_DR97	0x10	//	CALM1 Monitor ADC Calibration 1.
#define	BIT_CALM2_DR97	0x08	//	CALM2 Monitor ADC Calibration 2.
#define	BIT_CALDAC_DR97	0x04	//	CALDAC DAC Calibration.
#define	BIT_ADC_DR97	0x02	//	CALADC ADC Calibration.
#define	BIT_CALCM_DR97	0x01	//	CALCM Common Mode Balance Calibration.
#define	STANDARD_CAL_DR97	0x18	//	Calibrations without the ADC and DAC offset and without common mode calibration.
#define	MASK_CAL_DR96	0x40	//	CAL Calibration Status Bit.
#define	BIT_CAL_DR96	0x40	//	CAL Calibration Control
#define	BIT_CALR_DR96	0x10	//	CALR RING Gain Mismatch Calibration.
#define	BIT_CALT_DR96	0x08	//	CALT TIP Gain Mismatch Calibration.
#define	BIT_CALD_DR96	0x04	//	CALD Differential DAC Gain Calibration.
#define	BIT_CALC_DR96	0x02	//	CALC Common Mode DAC Gain Calibration.
#define	BIT_CALIL_DR96	0x01	//	CALIL ILIM Calibration.
#define	STANDARD_CAL_DR96	0x47	//	Calibrate common mode and differential DAC mode DAC + ILIM
#define	BIT_IRQE_CMCE	0x04	//	Calibrate common mode and differential mode DAC + ILIM
#define CAL_COMPLETE_DR96 0     //  Value in dr96 after calibration is completed
#define MAX_CAL_PERIOD 800		// The longest period in ms. for a calibration to complete.

#define	OPEN_DR64	0	//	
#define	FORWARD_ACTIVE_DR64	1	//	
#define	OHT_DR64	2	//	
#define	TIP_OPEN_DR64	3	//	
#define	RINGING_DR64	4	//	
#define	REVERSE_ACTIVE_DR64	5	//	
#define	REVERSE_OHT_DR64	6	//	
#define	RING_OPEN_DR64	7	//	

#define	GROUND_THRESHOLD_DR80	2	//	.75 volts 
#define	GROUND_THRESHOLD_DR81	2	//	.75 volts
				

#define ENB2_DR23  1<<2 // enable interrupt for the balance Cal



#define	RESET_VALUE_DR8	2	//	This is the value at reset of DR8
#define	RESET_VALUE_DR64	0	//	This is the value at reset of DR64
#define	RESET_VALUE_DR11	0x33	//	This is the value at reset of DR11
				
#define	FLUSHING_DR35	0x8000		
#define	FLUSHING_DR36	0x8000		
#define	FLUSHING_DR37	0x8000		
#define	FLUSHING_DR38	0x8000		
#define	FLUSHING_DR39	0x8000		


#define DISABLE_ALL_DR21 0
#define DISABLE_ALL_DR22 0
#define DISABLE_ALL_DR23 0

#define	PULSE_METERING_IR24	0x3190	
#define	PULSE_METERING_IR25	0x4FF	
#define	PULSE_METERING_DR35	0x10	
#define	PULSE_METERING_DR44	0x60	0x04
#define	PULSE_METERING_DR45	0x04	0x60
#define	PULSE_METERING_DR46	0x60	0x04
#define	PULSE_METERING_DR47	0x04	0x60

#define	RINGING_20HZ_IR19	0	//offset voltage
#define	RINGING_20HZ_IR20	0x7F00	
#define	RINGING_20HZ_IR21	0x013D	
#define	RINGING_20HZ_DR34	0X18	
#define	RINGING_20HZ_DR48	0x80	
#define	RINGING_20HZ_DR49	0x3e	
#define	RINGING_20HZ_DR50	0x7d	
#define	RINGING_20HZ_DR51	0	

			
			
#define	INIT_IR0		0x55C2	// DTMF_ROW_0_PEAK	
#define	INIT_IR1		0x51E6  //	DTMF_ROW_0_PEAK,	
#define	INIT_IR2		0x4B85	//	DTMF_ROW_1_PEAK,	
#define	INIT_IR3		0x4937	//	DTMF_ROW2_PEAK,	
#define	INIT_IR4		0x3333	//	DTMF_ROW3_PEAK,	
#define	INIT_IR5		0x0202	//	DTMF_COL1_PEAK,	
#define	INIT_IR6		0x0202	//	DTMF_FWD_TWIST,	
#define	INIT_IR7		0x0198	//	DTMF_RVS_TWIST,	
#define	INIT_IR8		0x0198	//	DTMF_ROW_RATIO,	
#define	INIT_IR9		0x0611	//	DTMF_COL_RATIO,	
#define	INIT_IR10		0x0202	//	DTMF_ROW_2ND_ARM,	
#define	INIT_IR11		0x00E5	//	DTMF_COL_2ND_ARM,	
#define	INIT_IR12		0x0A1C	//	DTMF_PWR_MIN_,	
#define	INIT_IR13		0x7b30	//	DTMF_OT_LIM_TRES,	
#define	INIT_IR14		0x0063	//	OSC1_COEF,	
#define	INIT_IR15		0x0000	//	OSC1X,	
#define	INIT_IR16		0x7870	//	OSC1Y,	
#define	INIT_IR17		0x007d	//	OSC2_COEF,	
#define	INIT_IR18		0x0000	//	OSC2X,	
#define	INIT_IR19		0x0000	//	OSC2Y,	
#define	INIT_IR20		0x7EF0	//	RING_V_OFF,	
#define	INIT_IR21		0x0160	//	RING_OSC,	
#define	INIT_IR22		0x0000	//	RING_X,	
#define	INIT_IR23		0x2000	//	RING_Y,	
#define	INIT_IR24		0x2000	//	PULSE_ENVEL,	
#define	INIT_IR25		0x0000	//	PULSE_X,	
#define	INIT_IR26		0x4000	//	PULSE_Y,	
#define	INIT_IR27		0x4000	//	RECV_DIGITAL_GAIN,	
#define	INIT_IR28		0x1000	//	XMIT_DIGITAL_GAIN,	
#define	INIT_IR29		0x3600	//	LOOP_CLOSE_TRES,	
#define	INIT_IR30		0x1000	//	RING_TRIP_TRES,	
#define	INIT_IR31		0x0200	//	COMMON_MIN_TRES,	
#define	INIT_IR32		0x7c0  	//	COMMON_MAX_TRES,	
#define	INIT_IR33		0x2600	//	PWR_ALARM_Q1Q2,	
#define	INIT_IR34		0x1B80	//	PWR_ALARM_Q3Q4,	
#define	INIT_IR35		0x8000	//	PWR_ALARM_Q5Q6,	
#define	INIT_IR36		0x0320	//	LOOP_CLSRE_FlTER,	
#define	INIT_IR37		0x08c	//	RING_TRIP_FILTER,	
#define	INIT_IR38		0x0100	//	TERM_LP_POLE_Q1Q2,	
#define	INIT_IR39		0x010	//	TERM_LP_POLE_Q3Q4,	
#define	INIT_IR40		0x0001  // MA 0x0C00	//	TERM_LP_POLE_Q5Q6,	
#define	INIT_IR41		0x0C00	//	CM_BIAS_RINGING,	
#define	INIT_IR42		0x1000	//	DCDC_MIN_V,	
#define	INIT_IR43		0x00DA	//	"LOOP_CLOSE_TRES Low
#define	INIT_IR99		0x00DA	// FSK 0 FREQ PARAM
#define	INIT_IR100		0x6B60	// FSK 0 AMPL PARAM
#define	INIT_IR101		0x0074	// FSK 1 FREQ PARAM
#define	INIT_IR102		0x79C0	// FSK 1 AMPl PARAM
#define	INIT_IR103		0x1120	// FSK 0to1 SCALER
#define	INIT_IR104		0x3BE0	// FSK 1to0 SCALER
#define	INIT_IR97		0x0000	// TRASMIT_FILTER



#define STARDARD_IR_INIT_ARRAY  unsigned short ir[] =\
{\
\
INIT_IR0	,\
INIT_IR1	,\
INIT_IR2	,\
INIT_IR3	,\
INIT_IR4	,\
INIT_IR5	,\
INIT_IR6	,\
INIT_IR7	,\
INIT_IR8	,\
INIT_IR9	,\
INIT_IR10	,\
INIT_IR11	,\
INIT_IR12	,\
INIT_IR13	,\
INIT_IR14	,\
INIT_IR15	,\
INIT_IR16	,\
INIT_IR17	,\
INIT_IR18	,\
INIT_IR19	,\
INIT_IR20	,\
INIT_IR21	,\
INIT_IR22	,\
INIT_IR23	,\
INIT_IR24	,\
INIT_IR25	,\
INIT_IR26	,\
INIT_IR27	,\
INIT_IR28	,\
INIT_IR29	,\
INIT_IR30	,\
INIT_IR31	,\
INIT_IR32	,\
INIT_IR33	,\
INIT_IR34	,\
INIT_IR35	,\
INIT_IR36	,\
INIT_IR37	,\
INIT_IR38	,\
INIT_IR39	,\
INIT_IR40	,\
INIT_IR41	,\
INIT_IR42	,\
INIT_IR43	\
};




#define ERRORCODE_LONGBALCAL -2;


#define	INIT_DR0_DC	0x80	//	Serial Interface
#define	INIT_DR0	0x00	//	Serial Interface
#define	INIT_DR1	0x08	//	PCM Mode
#define	INIT_DR2	0x00	//	PCM TX Clock Slot Low Byte (1 PCLK cycle/LSB)
#define	INIT_DR3	0x00	//	PCM TX Clock Slot High Byte
#define	INIT_DR4	0x00	//	PCM RX Clock Slot Low Byte (1 PCLK cycle/LSB)
#define	INIT_DR5	0x00	//	PCM RX Clock Slot High Byte
#define	INIT_DR6	0x00	//	DIO Control (external battery operation, Si3211/12)
#define	INIT_DR8	0x00	//	Loopbacks (digital loopback default)
#define	INIT_DR9	0x04/*0x00*/	//	Transmit and receive path gain and control
#define	INIT_DR10	0x28	//	Initialization Two-wire impedance (600  and enabled)
#define	INIT_DR11	0x33	//	Transhybrid Balance/Four-wire Return Loss
#define	INIT_DR14	0x10	//	Powerdown Control 1
#define	INIT_DR15	0x00	//	Initialization Powerdown Control 2
#define	INIT_DR18	0xff	//	Normal Oper. Interrupt Register 1 (clear with 0xFF)
#define	INIT_DR19	0xff	//	Normal Oper. Interrupt Register 2 (clear with 0xFF)
#define	INIT_DR20	0xff	//	Normal Oper. Interrupt Register 3 (clear with 0xFF)
#define	INIT_DR21	0xff	//	Interrupt Mask 1
#define	INIT_DR22	0xff	//	Initialization Interrupt Mask 2
#define	INIT_DR23	0xff	//	 Initialization Interrupt Mask 3
#define	INIT_DR32	0x00	//	Oper. Oscillator 1 Control—tone generation
#define	INIT_DR33	0x00	//	Oper. Oscillator 2 Control—tone generation
#define	INIT_DR34	0x18	//	34 0x22 0x00 Initialization Ringing Oscillator Control
#define	INIT_DR35	0x00	//	Oper. Pulse Metering Oscillator Control
#define	INIT_DR36	0x00	//	36 0x24 0x00 Initialization OSC1 Active Low Byte (125 µs/LSB)
#define	INIT_DR37	0x00	//	37 0x25 0x00 Initialization OSC1 Active High Byte (125 µs/LSB)
#define	INIT_DR38	0x00	//	38 0x26 0x00 Initialization OSC1 Inactive Low Byte (125 µs/LSB)
#define	INIT_DR39	0x00	//	39 0x27 0x00 Initialization OSC1 Inactive High Byte (125 µs/LSB)
#define	INIT_DR40	0x00	//	40 0x28 0x00 Initialization OSC2 Active Low Byte (125 µs/LSB)
#define	INIT_DR41	0x00	//	41 0x29 0x00 Initialization OSC2 Active High Byte (125 µs/LSB)
#define	INIT_DR42	0x00	//	42 0x2A 0x00 Initialization OSC2 Inactive Low Byte (125 µs/LSB)
#define	INIT_DR43	0x00	//	43 0x2B 0x00 Initialization OSC2 Inactive High Byte (125 µs/LSB)
#define	INIT_DR44	0x00	//	44 0x2C 0x00 Initialization Pulse Metering Active Low Byte (125 µs/LSB)
#define	INIT_DR45	0x00	//	45 0x2D 0x00 Initialization Pulse Metering Active High Byte (125 µs/LSB)
#define	INIT_DR46	0x00	//	46 0x2E 0x00 Initialization Pulse Metering Inactive Low Byte (125 µs/LSB)
#define	INIT_DR47	0x00	//	47 0x2F 0x00 Initialization Pulse Metering Inactive High Byte (125 µs/LSB)
#define	INIT_DR48	0x80	//	48 0x30 0x00 0x80 Initialization Ringing Osc. Active Timer Low Byte (2 s,125 µs/LSB)
#define	INIT_DR49	0x3E	//	49 0x31 0x00 0x3E Initialization Ringing Osc. Active Timer High Byte (2 s,125 µs/LSB)
#define	INIT_DR50	0x00	//	50 0x32 0x00 0x00 Initialization Ringing Osc. Inactive Timer Low Byte (4 s, 125 µs/LSB)
#define	INIT_DR51	0x7D	//	51 0x33 0x00 0x7D Initialization Ringing Osc. Inactive Timer High Byte (4 s, 125 µs/LSB)
#define	INIT_DR52	0x00	//	52 0x34 0x00 Normal Oper. FSK Data Bit
#define	INIT_DR63	0x54	//	63 0x3F 0x54 Initialization Ringing Mode Loop Closure Debounce Interval
#define	INIT_DR64	0x00	//	64 0x40 0x00 Normal Oper. Mode Byte—primary control
#define	INIT_DR65	0x61	//	65 0x41 0x61 Initialization External Bipolar Transistor Settings
#define	INIT_DR66	0x03	//	66 0x42 0x03 Initialization Battery Control
#define	INIT_DR67	0x1F	//	67 0x43 0x1F Initialization Automatic/Manual Control
#define	INIT_DR69	0x0C	//	69 0x45 0x0A 0x0C Initialization Loop Closure Debounce Interval (1.25 ms/LSB)
#define	INIT_DR70	0x0A	//	70 0x46 0x0A Initialization Ring Trip Debounce Interval (1.25 ms/LSB)
#define	INIT_DR71	0x01	//	71 0x47 0x00 0x01 Initialization Off-Hook Loop Current Limit (20 mA + 3 mA/LSB)
#define	INIT_DR72	0x20	//	72 0x48 0x20 Initialization On-Hook Voltage (open circuit voltage) = 48 V(1.5 V/LSB)
#define	INIT_DR73	0x02	//	73 0x49 0x02 Initialization Common Mode Voltage—VCM = –3 V(–1.5 V/LSB)
#define	INIT_DR74	0x2f    // MA 0x32	//	74 0x4A 0x32 Initialization VBATH (ringing) = –75 V (–1.5 V/LSB)
#define	INIT_DR75	0x10	//	75 0x4B 0x10 Initialization VBATL (off-hook) = –24 V (TRACK = 0)(–1.5 V/LSB)
#define	INIT_DR92	0xff    // MA 0x7f	//	92 0x5C  7F Initialization DC–DC Converter PWM Period (61.035 ns/LSB)
#define	INIT_DR93	0x0c    // MA 0x14	//	93 0x5D 0x14 0x19 Initialization DC–DC Converter Min. Off Time (61.035 ns/LSB)
#define	INIT_DR96	0x00	//	96 0x60 0x1F Initialization Calibration Control Register 1(written second and starts calibration)
#define	INIT_DR97	0x1F	//	97 0x61 0x1F Initialization Calibration Control Register 2(written before Register 96)
#define	INIT_DR98	0x10	//	98 0x62 0x10 Informative Calibration result (see data sheet)
#define	INIT_DR99	0x10	//	99 0x63 0x10 Informative Calibration result (see data sheet)
#define	INIT_DR100	0x11	//	100 0x64 0x11 Informative Calibration result (see data sheet)
#define	INIT_DR101	0x11	//	101 0x65 0x11 Informative Calibration result (see data sheet)
#define	INIT_DR102	0x08	//	102 0x66 0x08 Informative Calibration result (see data sheet)
#define	INIT_DR103	0x88	//	103 0x67 0x88 Informative Calibration result (see data sheet)
#define	INIT_DR104	0x00	//	104 0x68 0x00 Informative Calibration result (see data sheet)
#define	INIT_DR105	0x00	//	105 0x69 0x00 Informative Calibration result (see data sheet)
#define	INIT_DR106	0x20	//	106 0x6A 0x20 Informative Calibration result (see data sheet)
#define	INIT_DR107	0x08	//	107 0x6B 0x08 Informative Calibration result (see data sheet)
#define	INIT_DR108	0xEB	//	108 0x63 0x00 0xEB Initialization Feature enhancement register
#define INIT_SI3210M_DR92 0x60  //  92 0x60 Initialization DC–DC Converter PWM Period (61.035 ns/LSB)
#define INIT_SI3210M_DR93 0x38  //  92 0x60 Initialization DC–DC Converter PWM Period (61.035 ns/LSB)

/* New Defines */
#define MV_SI3210_FAMILY	0
#define MV_SI3210		0
#define MV_SI3211		1
#define MV_SI3210M		3
#define MV_SI3215_FAMILY	(1<<7)
#define MV_SI3215		0
#define MV_SI3215M		3

/**************************************OS.h****************************************************/

void exception (enum exceptions e);

#endif /*_PROSLIC_H_*/
