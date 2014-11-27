/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: alertes.h 690 2008-03-31 18:36:43Z  $
 *
 */

#ifndef __ALERTES_H__
#define __ALERTES_H__

enum {
	AL_IP_CHANGE,
	AL_UNKNOWN_ADDRESS,
	AL_BLACK_LISTED,
	AL_NEW,
	AL_UNAUTHRQ,
	AL_RQABUS,
	AL_MAC_ERROR,
	AL_FLOOD,
	AL_NEW_MAC,
	AL_MAC_CHANGE
};

extern char *alert_type[];
/*
const char *alert_type[] = {
	"ip_change",
	"unknow_address",
	"black_listed",
	"new",  
	"unauthrq",
	"rqabus",
	"mac_error",
	"flood",
	"new_mac",
	"mac_change"
};
*/

#endif
