
/*
* Copyright c                  Realtek Semiconductor Corporation, 2006  
* All rights reserved.                                                    
* 
* Program : The header file of all error numbers
* Abstract :                                                           
* Author : Abel Hsu (abelshie@realtek.com.tw)               
* $Id: rtl8368s_errno.h,v 1.4 2008-05-02 08:52:30 hmchung Exp $
*/

#ifndef _RTL8368S_ERRNO_H
#define _RTL8368S_ERRNO_H

typedef struct ASICDRV_ERRMSG_S
{
	int  errCode; /*error code number*/
	char msg[255];/*error message body*/
} ASICDRV_ERRMSG_T;

extern ASICDRV_ERRMSG_T drvErrMsg[];
#define PRINT_ERRMSG( i, code ) { for(i=0; drvErrMsg[i].errCode!=0; i++)\
													if(code == drvErrMsg[i].errCode)\
														rtlglue_printf("ASIC Driver Error: %s\n",drvErrMsg[i].msg); }

#define PRINT_ERRMSG_RETURN( i, code ) { for(i=0; drvErrMsg[i].errCode!=0; i++)\
													if(code == drvErrMsg[i].errCode)\
														rtlglue_printf("ASIC Driver Error: %s\n",drvErrMsg[i].msg); return code; }

/*=============== error codes for ASIC driver ===============*/

#define ERRNO_INPUT							-1000/* invalid input parameter */
#define ERRNO_SMI							-1001/* SMI access error */
#define ERRNO_REGUNDEFINE					-1002/* out of register address */
#define ERRNO_PORT_NUM						-1003/* Invalid port number */
#define ERRNO_PRIORITY						-1004/* Invalid priority */
#define ERRNO_QUEUE_ID						-1005/* Invalid queue id */
#define ERRNO_CVIDX							-1006/* Invalid cvlan vidx */
#define ERRNO_SVIDX							-1007/* invalid svlan vidx */
#define ERRNO_VID							-1008/* invalid vid(12bits) */
#define ERRNO_QUEUE_NUMBER				-1009/* invalid queue number*/
#define ERRNO_FID							-1010/* invalid fid*/
#define ERRNO_MBR							-1011/* invalid member set*/
#define ERRNO_UTAG							-1012/* invalid untag set*/
#define ERRNO_SMBR							-1013/* invalid svlan member set*/
#define ERRNO_PORT_MSK						-1014/* Invalid port mask */
#define ERRNO_FID_MSK						-1015/* invalid fid mask*/
#define ERRNO_STP_STATE					-1016/* invalid STP state */
#define ERRNO_PPBVIDX						-1017/* invalid proptocol base group database index */
#define ERRNO_ACLIDX						-1018/* invalid acl rule index */
#define ERRNO_MAC							-1019/*invalid MAC address*/
#define ERRNO_LUT							-1020/*invalid lookup table*/
#define ERRNO_LUTIDX						-1021/*invalid LUT index*/
#define ERRNO_CAMIDX						-1022/*invalid CAM index*/
#define ERRNO_OAM_PARACT					-1030/*invalid OAM Parser State*/
#define ERRNO_OAM_MULACT					-1031/*invalid OAM Multiplexer State*/


#define ERRNO_ACL_INVALIDRULEIDX			-2000/* invalid acl rule index */
#define ERRNO_ACL_INVALIDMETERIDX		-2001/* invalid acl meter index */
#define ERRNO_ACL_INVALIDPORT				-2002/* invalid port number */

#define ERRNO_VLAN_VIDX					-2100/* invalid VLAN member configuration index */
#define ERRNO_VLAN_INVALIDVID				-2101/* invalid VID */
#define ERRNO_VLAN_INVALIDPRIORITY		-2102/* invalid Priority */
#define ERRNO_VLAN_INVALIDFID				-2103/* invalid FID */
#define ERRNO_VLAN_INVALIDPPBIDX			-2104/* invalid proptocol base group database index */
#define ERRNO_VLAN_INVALIDPORTMSK		-2105/* invalid port mask */
#define ERRNO_VLAN_INVALIDPORT			-2106/* invalid port number*/

#define ERRNO_STP_INVALIDFID				-2200/* invalid FID */
#define ERRNO_STP_INVALIDSTATE			-2201/* invalid STP state */

#define ERRNO_LUT_INVALIDIDX				-2300/* invalid L2 entry index */
#define ERRNO_CAM_INVALIDIDX				-2301/* invalid CAM entry index */


#define ERRNO_LA_INVALIDHASHVAL			-2500/* invalid link aggragation hash value */
#define ERRNO_LA_INVALIDHASHSEL			-2501/* invalid link aggragation hash algorithm selection */
#define ERRNO_LA_INVALIDPORTS				-2502/* invalid link aggragation port numbers */

#define ERRNO_SC_INVALIDPERIOD			-2600/* invalid storm control reference period  */
#define ERRNO_SC_INVALIDCOUNT				-2601/* invalid storm control drop packet number threshold */

#define ERRNO_MIB_INVALIDIDX				-2700/* invalid MIB counter index */
#define ERRNO_MIB_BUSY						-2701/* MIB busy */
#define ERRNO_MIB_RESET					-2702/* MIB resetting */
#define ERRNO_MIB_INVALIDPORT				-2703/* invalid MIB counter index */

#define ERROR_1X_ORTBASEDPNEN				-2800/* 1x Port-based enable port error */
#define ERROR_1X_PORTBASEDAUTH			-2801/* 1x Port-based auth port error */
#define ERROR_1X_PORTBASEDOPDIR			-2802/* 1x Port-based opdir error */
#define ERROR_1X_MACBASEDPNEN			-2803/* 1x MAC-based enable port error */
#define ERROR_1X_MACBASEDOPDIR			-2804/* 1x MAC-based opdir error */
#define ERROR_1X_PROC						-2805/* 1x unauthorized behavior error */
#define ERROR_1X_GVLANIDX					-2806/* 1x guest vlan index error */
#define ERROR_1X_GVLANTALK					-2807/* 1x guest vlan OPDIR error */

#define ERRNO_QOS_INVALIDPRIORITY			-2903/* invalid priority */
#define ERRNO_QOS_INVALIDDSCP				-2904/* invalid DSCP value */
#define ERRNO_QOS_INVALIDQUEUETYPE		-2905/* invalid queue type */
#define ERRNO_QOS_INVALIDQUEUEWEIGHT	-2906/* invalid queeu weight */

#define ERROR_PHY_INVALIDPHYNO			-3000/* Invalid PHY number */
#define ERROR_PHY_INVALIDPHYPAGE			-3001/*  Invalid PHY page*/
#define ERROR_PHY_INVALIDREG				-3002/*  Invalid PHY register*/
#define ERROR_MAC_INVALIDNO				-3003/* Invalid MAC number */
#define ERROR_MAC_INVALIDSPEED			-3004/* Invalid MAC number */
#define ERROR_PHY_ACCESSBUSY				-3005/* PHY Acces BUSY*/


#define ERROR_RMA_INDEX					-3104/* Invalid RMA index */


/* error code for high-level API */
#define ERRNO_API_INPUT					-4000/* invalid parameter */
#define ERRNO_API_LUTFULL					-4001/* LUT 4-way full */
#define ERRNO_API_LUTNOTEXIST				-4002/* No such LUT entry */
#define ERRNO_API_CPUNOTSET				-4003/* No CPU port specified */
#define ERRNO_API_RTCT_NOT_FINISHED		-4004/*RTCT Test not finished*/

#endif /* _RTL8368S_ERRNO_H */

