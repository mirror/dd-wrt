
/*
* Copyright c                  Realtek Semiconductor Corporation, 2006  
* All rights reserved.                                                    
* 
* Program : The header file of all error numbers
* Abstract :                                                           
* Author : Abel Hsu (abelshie@realtek.com.tw)               
* $Id: rtl8366s_errno.h,v 1.5 2007/09/13 05:04:06 abelshie Exp $
*/

#ifndef _RTL8366S_ERRNO_H
#define _RTL8366S_ERRNO_H

#define ERRNO_INVALIDINPUT					-1000/* invalid input parameter */
#define ERRNO_SMIERROR						-1001/* SMI access error */
#define ERRNO_REGUNDEFINE					-1002/* out of register address */
#define ERRNO_INVALIDPORT					-1003/* In valid port number */

#define ERRNO_ACL_INVALIDRULEIDX			-2000/* invalid acl rule index */
#define ERRNO_ACL_INVALIDMETERIDX			-2001/* invalid acl meter index */
#define ERRNO_ACL_INVALIDPORT				-2002/* invalid port number */

#define ERRNO_VLAN_INVALIDMBRCONFIDX		-2100/* invalid VLAN member configuration index */
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

#define ERRNO_SVLAN_INVALIDIDX			-2400/* invalid svlan index */

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

#define ERRNO_QOS_INVALIDPORT				-2900/* invalid port number*/
#define ERRNO_QOS_INVALIDQUEUEID			-2901/* invalid queue id*/
#define ERRNO_QOS_INVALIDQUEUENUM		-2902/* invalid queue number*/
#define ERRNO_QOS_INVALIDPRIORITY			-2903/* invalid priority */
#define ERRNO_QOS_INVALIDDSCP				-2904/* invalid DSCP value */
#define ERRNO_QOS_INVALIDQUEUETYPE		-2905/* invalid queue type */
#define ERRNO_QOS_INVALIDQUEUEWEIGHT	-2906/* invalid queeu weight */

#define ERROR_PHY_INVALIDPHYNO			-3000/* Invalid PHY number */
#define ERROR_PHY_INVALIDPHYPAGE			-3001/*  Invalid PHY page*/
#define ERROR_PHY_INVALIDPHYADDR			-3002/*  Invalid PHY address*/
#define ERROR_MAC_INVALIDNO				-3003/* Invalid MAC number */
#define ERROR_MAC_INVALIDSPEED			-3004/* Invalid MAC number */




/* error code for high-level API */
#define ERRNO_API_INVALIDPARAM			-4000/* invalid parameter */
#define ERRNO_API_LUTFULL					-4001/* LUT 4-way full */
#define ERRNO_API_LUTNOTEXIST				-4002/* No such LUT entry */
#define ERRNO_API_CPUNOTSET				-4003/* No CPU port specified */
#define ERRNO_API_RTCT_NOT_FINISHED		-4005/* RTCT detection is still running */

#endif /* _RTL8366S_ERRNO_H */

