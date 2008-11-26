/************************************  DAA.H   *************************************************/
#ifndef _DAA_H_
#define _DAA_H_

#ifdef MV_88F5181L
#include "voiceband/tdm/mvTdm.h"
#endif

#include "mvOs.h"

/* PROTOTYPE */
unsigned char readDaaDirectReg(unsigned int daa_dev, unsigned char address);
void writeDaaDirectReg(unsigned int daa_dev, unsigned char address, unsigned char data);
unsigned char lineSideIdGet (void);
void printDaaInfo(unsigned int daa_dev);
int initDaa(unsigned int *daaDev, unsigned int ch, int workMode, int intMode);
void enableDaaInterrupts(unsigned int daa_dev);
void disableDaaInterrupts(unsigned int daa_dev);
void setDaaPcmStartCountRegs(unsigned int daa_dev);
int checkDaaInterrupts(unsigned int daa_dev); 
void setPstnState(long state, unsigned int daa_dev);
void dumpDaaRegs(unsigned int daa_dev);
void daaCalibration(unsigned int daa_dev);
void setDaaCidState(long state, unsigned int daa_dev);
void enableDaaReveresePolarity(long enable, unsigned int daa_dev);
void daaEventTypeGet(unsigned char *eventType, unsigned int daa_dev);
void setDaaDigitalHybrid(unsigned int index, unsigned int daa_dev);
int daaGetLineVoltage(unsigned int daa_dev);
void daaFreeChannel(unsigned int daa_dev);
  
/********************************************DEFINES***********************************************/

/* General */
#define PSTN_ON_HOOK		0
#define PSTN_RINGING		1
#define PSTN_OFF_HOOK		2
#define PSTN_PULSE_DIAL		3

#define CID_OFF			0
#define CID_ON			1

/* Registers */
#define DAA_CONTROL_1_REG				1
#define DAA_CONTROL_2_REG				2
#define DAA_INTERRUPT_MASK_REG				3
#define DAA_INTERRUPT_SOURCE_REG			4
#define DAA_DAA_CONTROL_1				5
#define DAA_DAA_CONTROL_2				6
#define DAA_CONTROL_3_REG				10
#define DAA_SYSTEM_AND_LINE_SIDE_REV_REG		11
#define DAA_INTERNATIONAL_CONTROL_2			17
#define DAA_INTERNATIONAL_CONTROL_3			18
#define DAA_RING_VALIDATION_CONTROL_3			24
#define DAA_LINE_VOLTAGE_STATUS				29

/* Bits */
/* Register  11*/
#define DAA_SYSTEM_SIDE_REVISION_MASK			0xF
#define DAA_LINE_SIDE_ID_MASK				0xF0
#define DAA_SYSTEM_SIDE_REV_A				0
#define DAA_SYSTEM_SIDE_REV_B				0x2
#define DAA_SYSTEM_SIDE_REV_C				0x3
#define DAA_SYSTEM_SIDE_REV_D				0x4
#define DAA_SI3018					0x10
#define DAA_SI3019					0x30

/* Register 2 */
#define DAA_INTE					1 << 7
#define DAA_INTP					1 << 6

/* Register 3 */
#define DAA_POLM					1 << 0
#define DAA_DODM					1 << 3
#define DAA_RDTM					1 << 7

/* Register 4 */
#define DAA_POLI					1 << 0
#define DAA_DODI					1 << 3
#define DAA_RDTI					1 << 7

/* Register 5 */
#define DAA_ONHM					1 << 3
#define DAA_OH						1 << 0
#define DAA_RDT						1 << 2
#define DAA_RDTP					1 << 5
#define DAA_RDTN					1 << 6

/* Register 6 */
#define DAA_PDL						1 << 4

/* Register 17 */
#define DAA_CALD					1 << 5
#define DAA_MCAL					1 << 6

/* Register 18 */
#define DAA_RFWE					1 << 1

/* Register 24 */
#define DAA_RNGV					1 << 7

#endif /*_DAA_H_*/
