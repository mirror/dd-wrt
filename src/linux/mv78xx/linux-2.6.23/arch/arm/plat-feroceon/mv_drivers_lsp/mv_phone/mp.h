/************************************  PROSLIC.H   *************************************************/
#ifndef _MP_H_
#define _MP_H_

#include "voiceband/slic/proslic.h"
#include "voiceband/daa/daa.h"
#include "dbg-trace.h"
#include "gpp/mvGppRegs.h"
#include "voiceband/tdm/mvTdm.h"

#define MP_IRQ	TDM_IRQ_INT

/* Defines */
#define EXCEPTION_ON		1
#define EXCEPTION_OFF		0
#define BASE_ENV_PARAM_OFF	5
#define DEV_ENV_PARAM_OFF	9
#define MPP_FXO_GROUP		0
#define MPP_FXO_SELECT		0x1000
#define DAISY_CHAIN_MODE	1
#define DUAL_CHIP_SELECT_MODE   0
#define INTERRUPT_TO_MPP        1
#define INTERRUPT_TO_TDM	0

/****************
* DEBUG CONTROL *
****************/
/* Extra IOCTLs for debug (used by mv_voip_tool) */
#define MV_IOCTL
#ifdef MV_IOCTL
#define PHONE_MV_READ_SLIC_REG	_IOWR ('q', 0xB0, unsigned char)
#define PHONE_MV_WRITE_SLIC_REG	_IOW ('q', 0xB1, unsigned int)
#define PHONE_MV_READ_REG	_IOWR ('q', 0xB2, unsigned int)
#define PHONE_MV_WRITE_REG	_IOW ('q', 0xB3, unsigned int)
#define PHONE_MV_SPI_TEST	_IO ('q', 0xB4)
#endif






#endif /*_MP_H_*/

