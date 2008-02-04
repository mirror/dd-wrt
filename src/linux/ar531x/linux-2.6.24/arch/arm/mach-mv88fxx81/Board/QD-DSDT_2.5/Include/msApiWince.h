#include <Copyright.h>

/********************************************************************************
* msApiWince.h
*
* DESCRIPTION:
*       Wince Application need to include only this header file.
*
* DEPENDENCIES:   None
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#ifndef __msApiFunc_h
#define __msApiFunc_h

#include "msApiDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef GT_STATUS (*FGT_PRT_ATUSIZE)(ATU_SIZE);
typedef GT_STATUS (*FGT_PRT_U32_U32)(GT_U32*,GT_U32*);
typedef GT_STATUS (*FGT_VALUE_U32)(GT_U32);
typedef GT_STATUS (*FGT_PTR_U32)(GT_U32*);
typedef GT_STATUS (*FGT_PTR_U16)(GT_U16*);
typedef GT_STATUS (*FGT_PTR_U32_U32_U32)(GT_U32,GT_U32,GT_U32*);
typedef GT_STATUS (*FGT_PTR_ATUENTRY)(GT_ATU_ENTRY*);
typedef GT_STATUS (*FGT_PTR_ATUENTRY_BOOL)(GT_ATU_ENTRY*, GT_BOOL*);
typedef GT_STATUS (*FGT_VALUE_FLUSHCMD)(GT_FLUSH_CMD);
typedef GT_STATUS (*FGT_PTR_ETHERADDR)(GT_ETHERADDR*);
typedef GT_STATUS (*FGT_PTR_BOOL)(GT_BOOL*);
typedef GT_STATUS (*FGT_VALUE_BOOL)(GT_BOOL);
typedef GT_STATUS (*FGT_VALUE_PORT_STPSTATE)(GT_LPORT,GT_PORT_STP_STATE);
typedef GT_STATUS (*FGT_PTR_PORT_STPSTATE)(GT_LPORT,GT_PORT_STP_STATE*);
typedef GT_STATUS (*FGT_VALUE_PORT_EGRESSMODE)(GT_LPORT,GT_EGRESS_MODE);
typedef GT_STATUS (*FGT_PTR_PORT_EGRESSMODE)(GT_LPORT,GT_EGRESS_MODE*);
typedef GT_STATUS (*FGT_VALUE_PORT_BOOL)(GT_LPORT,GT_BOOL);
typedef GT_STATUS (*FGT_PTR_PORT_BOOL)(GT_LPORT,GT_BOOL*);
typedef GT_STATUS (*FGT_VALUE_PORT_PORTS_U8)(GT_LPORT,GT_LPORT*,GT_U8);
typedef GT_STATUS (*FGT_PTR_PORT_PORTS_U8)(GT_LPORT,GT_LPORT*,GT_U8*);
typedef GT_STATUS (*FGT_VALUE_PORT_U16)(GT_LPORT,GT_U16);
typedef GT_STATUS (*FGT_PTR_PORT_U16)(GT_LPORT,GT_U16*);
typedef GT_STATUS (*FGT_VALUE_PORT_AUTOMODE)(GT_LPORT,GT_PHY_AUTO_MODE);

typedef GT_STATUS (*FGT_VALUE_PORT)(GT_LPORT);
typedef GT_STATUS (*FGT_VALUE_U8)(GT_U8);
typedef GT_STATUS (*FGT_PTR_U8)(GT_U8*);
typedef GT_STATUS (*FGT_VALUE_PORT_U8)(GT_LPORT,GT_U8);
typedef GT_STATUS (*FGT_PTR_PORT_U8)(GT_LPORT,GT_U8*);
typedef GT_STATUS (*FGT_VALUE_PORT_INGRESSMODE)(GT_LPORT,GT_INGRESS_MODE);
typedef GT_STATUS (*FGT_PTR_PORT_INGRESSMODE)(GT_LPORT,GT_INGRESS_MODE*);
typedef GT_STATUS (*FGT_VALUE_PORT_MCRATE)(GT_LPORT,GT_MC_RATE);
typedef GT_STATUS (*FGT_PTR_PORT_MCRATE)(GT_LPORT,GT_MC_RATE*);
typedef GT_STATUS (*FGT_VALUE_CTRMODE)(GT_CTR_MODE);
typedef GT_STATUS (*FGT_PTR_CTRMODE)(GT_CTR_MODE*);
typedef GT_STATUS (*FGT_VOID)(void);
typedef GT_STATUS (*FGT_PTR_PORT_PORTSTAT)(GT_LPORT,GT_PORT_STAT*);
typedef GT_STATUS (*FGT_VALUE_U8_U8)(GT_U8,GT_U8);
typedef GT_STATUS (*FGT_PTR_U8_U8)(GT_U8,GT_U8*);
typedef GT_STATUS (*FGT_PTR_CONFIG_INFO)(GT_SYS_CONFIG*,GT_SYS_INFO*);
typedef GT_STATUS (*FGT_PTR_VERSION)(GT_VERSION*);
typedef GT_STATUS (*FGT_PTR_REGISTER)(BSP_FUNCTIONS*);
typedef GT_STATUS (*FGT_PTR_INT_HANDLER)(FGT_INT_HANDLER*);

typedef GT_STATUS (*FGT_PTR_U32_U32_U16)(GT_U32,GT_U32,GT_U16);


extern FGT_PRT_ATUSIZE 			gfdbSetAtuSize;
extern FGT_PRT_U32_U32 			gfdbGetAgingTimeRange;
extern FGT_VALUE_U32 			gfdbSetAgingTimeout;
extern FGT_PTR_U32 				gfdbGetAtuDynamicCount;
extern FGT_PTR_ATUENTRY 		gfdbGetAtuEntryFirst;
extern FGT_PTR_ATUENTRY 		gfdbGetAtuEntryNext;
extern FGT_PTR_ATUENTRY_BOOL 	gfdbFindAtuMacEntry;
extern FGT_VALUE_FLUSHCMD 		gfdbFlush;
extern FGT_PTR_ATUENTRY 		gfdbAddMacEntry; //liane
extern FGT_PTR_ETHERADDR 		gfdbDelMacEntry;
extern FGT_VALUE_BOOL 			gfdbLearnEnable;
extern FGT_VALUE_BOOL 				gstpSetMode;
extern FGT_VALUE_PORT_STPSTATE 		gstpSetPortState;
extern FGT_PTR_PORT_STPSTATE 		gstpGetPortState;
extern FGT_VALUE_PORT_EGRESSMODE 	gprtSetEgressMode;
extern FGT_PTR_PORT_EGRESSMODE 		gprtGetEgressMode;
extern FGT_VALUE_PORT_BOOL 			gprtSetVlanTunnel;
extern FGT_PTR_PORT_BOOL 			gprtGetVlanTunnel;
extern FGT_VALUE_PORT_PORTS_U8		gvlnSetPortVlanPorts;
extern FGT_PTR_PORT_PORTS_U8		gvlnGetPortVlanPorts;
extern FGT_VALUE_PORT_BOOL			gvlnSetPortUserPriLsb;
extern FGT_PTR_PORT_BOOL			gvlnGetPortUserPriLsb;
extern FGT_VALUE_PORT_U16			gvlnSetPortVid;
extern FGT_PTR_PORT_U16				gvlnGetPortVid;
extern FGT_VALUE_U32				eventSetActive;
extern FGT_PTR_U16					eventGetIntStatus;
extern FGT_VALUE_PORT				gprtPhyReset;
extern FGT_VALUE_PORT_BOOL			gprtSetPortLoopback;
extern FGT_VALUE_PORT_BOOL			gprtSetPortSpeed;
extern FGT_VALUE_PORT_BOOL			gprtPortAutoNegEnable;
extern FGT_VALUE_PORT_BOOL			gprtPortPowerDown;
extern FGT_VALUE_PORT				gprtPortRestartAutoNeg;
extern FGT_VALUE_PORT_BOOL			gprtSetPortDuplexMode;
extern FGT_VALUE_PORT_AUTOMODE		gprtSetPortAutoMode;
extern FGT_VALUE_PORT_BOOL			gprtSetPause;
extern FGT_VALUE_PORT_U16			gprtPhyIntEnable;
extern FGT_PTR_PORT_U16				gprtGetPhyIntStatus;
extern FGT_PTR_U16					gprtGetPhyIntPortSummary;
extern FGT_VALUE_PORT_BOOL			gprtSetForceFc;
extern FGT_PTR_PORT_BOOL			gprtGetForceFc;
extern FGT_VALUE_PORT_BOOL			gprtSetTrailerMode;
extern FGT_PTR_PORT_BOOL			gprtGetTrailerMode;
extern FGT_VALUE_PORT_INGRESSMODE	gprtSetIngressMode;
extern FGT_PTR_PORT_INGRESSMODE		gprtGetIngressMode;
extern FGT_VALUE_PORT_MCRATE		gprtSetMcRateLimit;
extern FGT_PTR_PORT_MCRATE			gprtGetMcRateLimit;
extern FGT_VALUE_CTRMODE			gprtSetCtrMode;
extern FGT_VOID					gprtClearAllCtr;
extern FGT_PTR_PORT_PORTSTAT	gprtGetPortCtr;
extern FGT_PTR_PORT_BOOL		gprtGetPartnerLinkPause;
extern FGT_PTR_PORT_BOOL		gprtGetSelfLinkPause;
extern FGT_PTR_PORT_BOOL		gprtGetResolve;
extern FGT_PTR_PORT_BOOL		gprtGetLinkState;
extern FGT_PTR_PORT_BOOL		gprtGetPortMode;
extern FGT_PTR_PORT_BOOL		gprtGetPhyMode;
extern FGT_PTR_PORT_BOOL		gprtGetDuplex;
extern FGT_PTR_PORT_BOOL		gprtGetSpeed;
extern FGT_VALUE_PORT_U8		gcosSetPortDefaultTc;
extern FGT_VALUE_PORT_BOOL		gqosSetPrioMapRule;
extern FGT_PTR_PORT_BOOL		gqosGetPrioMapRule;
extern FGT_VALUE_PORT_BOOL		gqosIpPrioMapEn;
extern FGT_PTR_PORT_BOOL		gqosGetIpPrioMapEn;
extern FGT_VALUE_PORT_BOOL		gqosUserPrioMapEn;
extern FGT_PTR_PORT_BOOL		gqosGetUserPrioMapEn;
extern FGT_PTR_U8_U8			gcosGetUserPrio2Tc;
extern FGT_VALUE_U8_U8			gcosSetUserPrio2Tc;
extern FGT_PTR_U8_U8			gcosGetDscp2Tc;
extern FGT_VALUE_U8_U8			gcosSetDscp2Tc;
extern FGT_PTR_CONFIG_INFO		sysConfig;
extern FGT_VOID					sysEnable;
extern FGT_VOID					gsysSwReset;
extern FGT_VALUE_BOOL			gsysSetDiscardExcessive;
extern FGT_PTR_BOOL				gsysGetDiscardExcessive;
extern FGT_VALUE_BOOL			gsysSetSchedulingMode;
extern FGT_PTR_BOOL				gsysGetSchedulingMode;
extern FGT_VALUE_BOOL			gsysSetMaxFrameSize;
extern FGT_PTR_BOOL				gsysGetMaxFrameSize;
extern FGT_VOID					gsysReLoad;
extern FGT_VALUE_BOOL			gsysSetWatchDog;
extern FGT_PTR_BOOL				gsysGetWatchDog;
extern FGT_PTR_ETHERADDR		gsysSetDuplexPauseMac;
extern FGT_PTR_ETHERADDR		gsysGetDuplexPauseMac;
extern FGT_VALUE_BOOL			gsysSetPerPortDuplexPauseMac;
extern FGT_PTR_BOOL				gsysGetPerPortDuplexPauseMac;
extern FGT_PTR_U32_U32_U32		gsysReadMiiReg;
extern FGT_PTR_VERSION			gtVersion;
extern FGT_PTR_REGISTER			gtRegister;

extern FGT_PTR_U32_U32_U16		gsysWriteMiiReg;

/*
 * This function will get the all the MS APIs and assign to local function pointers.
 */
int qdGetMSApiFunc();

GT_U32 gtStrlen
(
    IN const void * source
);

//*****************************************************************************
//  I O C T L S
//*****************************************************************************
#include "windev.h"

typedef struct _GT_IOCTL_PARAM
{
	union 
	{
		GT_LPORT	portList[8];
		GT_LPORT	port;
		GT_U8  		u8Data;
		GT_U16  	u16Data;
		GT_U32  	u32Data;
		GT_BOOL 	boolData;

		GT_CTR_MODE	ctrMode;
		GT_PORT_STP_STATE	stpState;
		GT_EGRESS_MODE		egressMode;
		GT_INGRESS_MODE		ingressMode;
		GT_MC_RATE		mcRate;
		GT_PORT_STAT	portStat;
		ATU_SIZE 		atuSize;
		GT_FLUSH_CMD 	flushCmd;

		GT_ATU_ENTRY 	atuEntry;
		GT_ETHERADDR 	etherAddr;
		GT_SYS_CONFIG 	sysConfig;
		GT_SYS_INFO		sysInfo;

	} FirstParam;

	union 
	{
		GT_LPORT	port;
		GT_LPORT	portList[8];
		GT_U8		u8Data;
		GT_U16		u16Data;
		GT_U32		u32Data;
		GT_BOOL		boolData;
		GT_PORT_STP_STATE	stpState;
		GT_EGRESS_MODE		egressMode;
		GT_INGRESS_MODE		ingressMode;
		GT_MC_RATE		mcRate;

		GT_PORT_STAT	portStat;

		GT_PHY_AUTO_MODE	phyAutoMode;

	} SecondParam;

	union
	{
		GT_U8	u8Data;
		GT_U16	u16Data;
		GT_U32	u32Data;

	} ThirdParam;

} GT_IOCTL_PARAM, *PGT_IOCTL_PARAM;

#define GET_FUNC_FROM_CTL_CODE(_ioctl) ((_ioctl>>2) & 0xFFF)

/*
	Microsoft allows for us to use 0x800 ~ 0xFFF
	So, our program is using 6 bits for function group,
	and 6 bits for each function.
*/
#define SUB_FUNC_MASK		0xFC0
#define SYS_CFG_FUNC_MASK	(1 << 6) | 0x800
#define SYS_CTRL_FUNC_MASK	(2 << 6) | 0x800
#define FDB_FUNC_MASK		(3 << 6) | 0x800
#define VLAN_FUNC_MASK		(4 << 6) | 0x800
#define STP_FUNC_MASK		(5 << 6) | 0x800
#define PORT_CTRL_FUNC_MASK		(6 << 6) | 0x800
#define PORT_STATUS_FUNC_MASK	(7 << 6) | 0x800
#define PORT_STATS_FUNC_MASK	(8 << 6) | 0x800
#define QOS_FUNC_MASK			(9 << 6) | 0x800
#define PHY_CTRL_FUNC_MASK		(10 << 6) | 0x800
#define SYS_EVENT_FUNC_MASK		(11 << 6) | 0x800
#define PHY_INT_FUNC_MASK		(12 << 6) | 0x800

/*
	Functions for SYS Configuration
*/
#define IOCTL_sysConfig    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_CFG_FUNC_MASK + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gsysReadMiiReg    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_CFG_FUNC_MASK + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gsysWriteMiiReg    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_CFG_FUNC_MASK + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gtVersion    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_CFG_FUNC_MASK + 3, METHOD_BUFFERED, FILE_ANY_ACCESS)

/*
	Functions for ATU
*/
#define IOCTL_gfdbSetAtuSize    \
    CTL_CODE(FILE_DEVICE_NETWORK , FDB_FUNC_MASK + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gfdbGetAgingTimeRange    \
    CTL_CODE(FILE_DEVICE_NETWORK , FDB_FUNC_MASK + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gfdbSetAgingTimeout    \
    CTL_CODE(FILE_DEVICE_NETWORK , FDB_FUNC_MASK + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gfdbGetAtuDynamicCount    \
    CTL_CODE(FILE_DEVICE_NETWORK , FDB_FUNC_MASK + 3, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gfdbGetAtuEntryFirst    \
    CTL_CODE(FILE_DEVICE_NETWORK , FDB_FUNC_MASK + 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gfdbGetAtuEntryNext    \
    CTL_CODE(FILE_DEVICE_NETWORK , FDB_FUNC_MASK + 5, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gfdbFindAtuMacEntry    \
    CTL_CODE(FILE_DEVICE_NETWORK , FDB_FUNC_MASK + 6, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gfdbFlush    \
    CTL_CODE(FILE_DEVICE_NETWORK , FDB_FUNC_MASK + 7, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gfdbAddMacEntry    \
    CTL_CODE(FILE_DEVICE_NETWORK , FDB_FUNC_MASK + 8, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gfdbDelMacEntry    \
    CTL_CODE(FILE_DEVICE_NETWORK , FDB_FUNC_MASK + 9, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gfdbLearnEnable    \
    CTL_CODE(FILE_DEVICE_NETWORK , FDB_FUNC_MASK + 10, METHOD_BUFFERED, FILE_ANY_ACCESS)


/*
	Functions for STP
*/
#define IOCTL_gstpSetMode    \
    CTL_CODE(FILE_DEVICE_NETWORK , STP_FUNC_MASK + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gstpSetPortState    \
    CTL_CODE(FILE_DEVICE_NETWORK , STP_FUNC_MASK + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gstpGetPortState    \
    CTL_CODE(FILE_DEVICE_NETWORK , STP_FUNC_MASK + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

/*
	Functions for VLAN
*/
#define IOCTL_gprtSetEgressMode    \
    CTL_CODE(FILE_DEVICE_NETWORK , VLAN_FUNC_MASK + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtGetEgressMode    \
    CTL_CODE(FILE_DEVICE_NETWORK , VLAN_FUNC_MASK + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtSetVlanTunnel    \
    CTL_CODE(FILE_DEVICE_NETWORK , VLAN_FUNC_MASK + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtGetVlanTunnel    \
    CTL_CODE(FILE_DEVICE_NETWORK , VLAN_FUNC_MASK + 3, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gvlnSetPortVlanPorts    \
    CTL_CODE(FILE_DEVICE_NETWORK , VLAN_FUNC_MASK + 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gvlnGetPortVlanPorts    \
    CTL_CODE(FILE_DEVICE_NETWORK , VLAN_FUNC_MASK + 5, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gvlnSetPortUserPriLsb    \
    CTL_CODE(FILE_DEVICE_NETWORK , VLAN_FUNC_MASK + 6, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gvlnGetPortUserPriLsb    \
    CTL_CODE(FILE_DEVICE_NETWORK , VLAN_FUNC_MASK + 7, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gvlnSetPortVid    \
    CTL_CODE(FILE_DEVICE_NETWORK , VLAN_FUNC_MASK + 8, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gvlnGetPortVid    \
    CTL_CODE(FILE_DEVICE_NETWORK , VLAN_FUNC_MASK + 9, METHOD_BUFFERED, FILE_ANY_ACCESS)

/*
	Functions for System Event
*/
#define IOCTL_eventSetActive    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_EVENT_FUNC_MASK + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_eventGetIntStatus    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_EVENT_FUNC_MASK + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)

/*
	Functions for Phy Control
*/
#define IOCTL_gprtPhyReset    \
    CTL_CODE(FILE_DEVICE_NETWORK , PHY_CTRL_FUNC_MASK + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtSetPortLoopback    \
    CTL_CODE(FILE_DEVICE_NETWORK , PHY_CTRL_FUNC_MASK + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtSetPortSpeed    \
    CTL_CODE(FILE_DEVICE_NETWORK , PHY_CTRL_FUNC_MASK + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtPortAutoNegEnable    \
    CTL_CODE(FILE_DEVICE_NETWORK , PHY_CTRL_FUNC_MASK + 3, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtPortPowerDown    \
    CTL_CODE(FILE_DEVICE_NETWORK , PHY_CTRL_FUNC_MASK + 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtPortRestartAutoNeg    \
    CTL_CODE(FILE_DEVICE_NETWORK , PHY_CTRL_FUNC_MASK + 5, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtSetPortDuplexMode    \
    CTL_CODE(FILE_DEVICE_NETWORK , PHY_CTRL_FUNC_MASK + 6, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtSetPortAutoMode    \
    CTL_CODE(FILE_DEVICE_NETWORK , PHY_CTRL_FUNC_MASK + 7, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtSetPause    \
    CTL_CODE(FILE_DEVICE_NETWORK , PHY_CTRL_FUNC_MASK + 8, METHOD_BUFFERED, FILE_ANY_ACCESS)


/* 
	Functions for Phy Interrupt
*/
#define IOCTL_gprtPhyIntEnable    \
    CTL_CODE(FILE_DEVICE_NETWORK , PHY_INT_FUNC_MASK + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtGetPhyIntStatus    \
    CTL_CODE(FILE_DEVICE_NETWORK , PHY_INT_FUNC_MASK + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtGetPhyIntPortSummary    \
    CTL_CODE(FILE_DEVICE_NETWORK , PHY_INT_FUNC_MASK + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

/*
	Functions for Port Control
*/
#define IOCTL_gprtSetForceFc    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_CTRL_FUNC_MASK + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtGetForceFc    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_CTRL_FUNC_MASK + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtSetTrailerMode    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_CTRL_FUNC_MASK + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtGetTrailerMode    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_CTRL_FUNC_MASK + 3, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtSetIngressMode    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_CTRL_FUNC_MASK + 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtGetIngressMode    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_CTRL_FUNC_MASK + 5, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtSetMcRateLimit    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_CTRL_FUNC_MASK + 6, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtGetMcRateLimit    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_CTRL_FUNC_MASK + 7, METHOD_BUFFERED, FILE_ANY_ACCESS)


/*
	Functions for Port Statistics
*/
#define IOCTL_gprtSetCtrMode    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_STATS_FUNC_MASK + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtClearAllCtr    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_STATS_FUNC_MASK + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtGetPortCtr    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_STATS_FUNC_MASK + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

/*
	Functions for Port Status
*/
#define IOCTL_gprtGetPartnerLinkPause    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_STATUS_FUNC_MASK + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtGetSelfLinkPause    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_STATUS_FUNC_MASK + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtGetResolve    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_STATUS_FUNC_MASK + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtGetLinkState    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_STATUS_FUNC_MASK + 3, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtGetPortMode    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_STATUS_FUNC_MASK + 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtGetPhyMode    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_STATUS_FUNC_MASK + 5, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtGetDuplex    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_STATUS_FUNC_MASK + 6, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gprtGetSpeed    \
    CTL_CODE(FILE_DEVICE_NETWORK , PORT_STATUS_FUNC_MASK + 7, METHOD_BUFFERED, FILE_ANY_ACCESS)

/*
	Functions for QoS Mapping
*/
#define IOCTL_gcosSetPortDefaultTc    \
    CTL_CODE(FILE_DEVICE_NETWORK , QOS_FUNC_MASK + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gqosSetPrioMapRule    \
    CTL_CODE(FILE_DEVICE_NETWORK , QOS_FUNC_MASK + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gqosGetPrioMapRule    \
    CTL_CODE(FILE_DEVICE_NETWORK , QOS_FUNC_MASK + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gqosIpPrioMapEn    \
    CTL_CODE(FILE_DEVICE_NETWORK , QOS_FUNC_MASK + 3, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gqosGetIpPrioMapEn    \
    CTL_CODE(FILE_DEVICE_NETWORK , QOS_FUNC_MASK + 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gqosUserPrioMapEn    \
    CTL_CODE(FILE_DEVICE_NETWORK , QOS_FUNC_MASK + 5, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gqosGetUserPrioMapEn    \
    CTL_CODE(FILE_DEVICE_NETWORK , QOS_FUNC_MASK + 6, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gcosGetUserPrio2Tc    \
    CTL_CODE(FILE_DEVICE_NETWORK , QOS_FUNC_MASK + 7, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gcosSetUserPrio2Tc    \
    CTL_CODE(FILE_DEVICE_NETWORK , QOS_FUNC_MASK + 8, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gcosGetDscp2Tc    \
    CTL_CODE(FILE_DEVICE_NETWORK , QOS_FUNC_MASK + 9, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gcosSetDscp2Tc    \
    CTL_CODE(FILE_DEVICE_NETWORK , QOS_FUNC_MASK + 10, METHOD_BUFFERED, FILE_ANY_ACCESS)


/*
	Functions for Sys Control
*/
#define IOCTL_gsysSwReset    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_CTRL_FUNC_MASK + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gsysSetDiscardExcessive    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_CTRL_FUNC_MASK + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gsysGetDiscardExcessive    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_CTRL_FUNC_MASK + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gsysSetSchedulingMode    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_CTRL_FUNC_MASK + 3, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gsysGetSchedulingMode    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_CTRL_FUNC_MASK + 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gsysSetMaxFrameSize    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_CTRL_FUNC_MASK + 5, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gsysGetMaxFrameSize    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_CTRL_FUNC_MASK + 6, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gsysReLoad    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_CTRL_FUNC_MASK + 7, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gsysSetWatchDog    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_CTRL_FUNC_MASK + 8, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gsysGetWatchDog    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_CTRL_FUNC_MASK + 9, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gsysSetDuplexPauseMac    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_CTRL_FUNC_MASK + 10, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gsysGetDuplexPauseMac    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_CTRL_FUNC_MASK + 11, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gsysSetPerPortDuplexPauseMac    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_CTRL_FUNC_MASK + 12, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_gsysGetPerPortDuplexPauseMac    \
    CTL_CODE(FILE_DEVICE_NETWORK , SYS_CTRL_FUNC_MASK + 13, METHOD_BUFFERED, FILE_ANY_ACCESS)


#ifdef __cplusplus
}
#endif

#endif
