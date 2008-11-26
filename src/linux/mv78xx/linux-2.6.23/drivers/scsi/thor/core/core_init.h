#if !defined(CORE_INIT_H)
#define CORE_INIT_H

#include "core_inter.h"

typedef enum mvAdapterState
{
    ADAPTER_INITIALIZING,
    ADAPTER_READY,
    ADAPTER_FATAL_ERROR
} MV_ADAPTER_STATE;

typedef enum mvChannelState	//TBD
{
    CHANNEL_NOT_CONNECTED,
    CHANNEL_CONNECTED,
    CHANNEL_IN_SRST,
    CHANNEL_PM_STAGGERED_SPIN_UP,
    CHANNEL_PM_SRST_DEVICE,
    CHANNEL_READY,
    CHANNEL_PM_HOT_PLUG,
} MV_CHANNEL_STATE;

MV_BOOLEAN mvAdapterStateMachine(
	MV_PVOID This
	);

void SATA_PortReset(
	PDomain_Port pPort,
	MV_BOOLEAN hardReset
	);

void PATA_PortReset(
	PDomain_Port pPort,
	MV_BOOLEAN hardReset
	);

MV_BOOLEAN SATA_DoSoftReset(PDomain_Port pPort, MV_U8 PMPort);

#define SATA_PortDeviceDetected(port)	\
	 ( MV_REG_READ_DWORD(port->Mmio_Base, PORT_SCR_STAT) & 0x01 )

#define SATA_PortDeviceReady(port)		\
	(								\
		( ( (MV_REG_READ_DWORD(port->Mmio_Base, PORT_SCR_STAT) & 0x0F00 ) >> 8) != PORT_SSTATUS_IPM_NO_DEVICE )		\
	)

#define FIS_REG_H2D_SIZE_IN_DWORD	5

/* PM related - move elsewhere? */
#define MV_ATA_COMMAND_PM_READ_REG              0xe4
#define MV_ATA_COMMAND_PM_WRITE_REG             0xe8

#define MV_SATA_GSCR_ID_REG_NUM                 0
#define MV_SATA_GSCR_REVISION_REG_NUM           1
#define MV_SATA_GSCR_INFO_REG_NUM               2
#define MV_SATA_GSCR_ERROR_REG_NUM              32
#define MV_SATA_GSCR_ERROR_ENABLE_REG_NUM       33
#define MV_SATA_GSCR_FEATURES_REG_NUM           64
#define MV_SATA_GSCR_FEATURES_ENABLE_REG_NUM    96

#define MV_SATA_PSCR_SSTATUS_REG_NUM            0
#define MV_SATA_PSCR_SERROR_REG_NUM             1
#define MV_SATA_PSCR_SCONTROL_REG_NUM           2
#define MV_SATA_PSCR_SACTIVE_REG_NUM            3

#define MV_Read_Reg  1
#define MV_Write_Reg 0

void mvPMDevReWrReg(
	PDomain_Port pPort, 
	MV_U8 read, 
	MV_U8 PMreg, 
	MV_U32 regVal, 
	MV_U8 PMport, 
	MV_BOOLEAN control
	);

void SATA_InitPM (
    PDomain_Port pPort
	);

void SATA_InitPMPort (
	PDomain_Port pPort,
	MV_U8 portNum
	);

MV_BOOLEAN SATA_SoftResetDevice(
	PDomain_Port pPort, 
	MV_U8 portNum
	);

MV_BOOLEAN SATA_PortSoftReset( 
	PCore_Driver_Extension pCore, 
	PDomain_Port pPort 
	);

void SATA_PortReportNoDevice (
    PCore_Driver_Extension pCore, 
	PDomain_Port pPort
	);

PMV_Request GetInternalReqFromPool( 
	PCore_Driver_Extension pCore
	);

void ReleaseInternalReqToPool( 
	PCore_Driver_Extension pCore, 
	PMV_Request pReq
	);

#define mvDisableIntr(portMmio, old_stat) do{ \
		old_stat = MV_REG_READ_DWORD(portMmio, PORT_IRQ_MASK); \
		MV_REG_WRITE_DWORD(portMmio, PORT_IRQ_MASK, 0);\
		}while(0)

#define mvEnableIntr(portMmio, old_stat)	MV_REG_WRITE_DWORD(portMmio, PORT_IRQ_MASK, old_stat)

#define CORE_MAX_RESET_COUNT	10

#endif

