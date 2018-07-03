/***************************************************************************
***
***    Copyright 2007  Hon Hai Precision Ind. Co. Ltd.
***    All Rights Reserved.
***    No portions of this material shall be reproduced in any form without the
***    written permission of Hon Hai Precision Ind. Co. Ltd.
***
***    All information contained in this document is Hon Hai Precision Ind.  
***    Co. Ltd. company private, proprietary, and trade secret property and 
***    are protected by international intellectual property laws and treaties.
***
****************************************************************************/

#ifndef __WPS_LED_H__
#define __WPS_LED_H__

//#include "ambitCfg.h"
#include "../wps/common/include/wps_led.h"

#if (defined GPIO_EXT_CTRL)
#include <ext_led.h>
#endif

#define WPS_LED_MAJOR_NUM       253

#define DEV_WPS_LED             "/dev/wps_led"
#define WPS_LED_IOCTL_NUM       'W'

#define WPS_LED_BLINK_NORMAL   _IOWR(WPS_LED_IOCTL_NUM, 0, int *)
#define WPS_LED_BLINK_QUICK    _IOWR(WPS_LED_IOCTL_NUM, 1, int *)
#define WPS_LED_BLINK_OFF      _IOWR(WPS_LED_IOCTL_NUM, 2, int *)
#define WPS_LED_CHANGE_GREEN   _IOWR(WPS_LED_IOCTL_NUM, 3, int *)
#define WPS_LED_CHANGE_AMBER   _IOWR(WPS_LED_IOCTL_NUM, 4, int *)
#define WPS_LED_BLINK_QUICK2   _IOWR(WPS_LED_IOCTL_NUM, 5, int *)

#define WLAN_N_RADIO_ON        _IOWR(WPS_LED_IOCTL_NUM, 6, int *)
#define WLAN_N_RADIO_OFF       _IOWR(WPS_LED_IOCTL_NUM, 7, int *)
#define WLAN_G_RADIO_ON        _IOWR(WPS_LED_IOCTL_NUM, 8, int *)
#define WLAN_G_RADIO_OFF       _IOWR(WPS_LED_IOCTL_NUM, 9, int *)

#define USB_LED_STATE_ON       _IOWR(WPS_LED_IOCTL_NUM, 10, int *)
#define USB_LED_STATE_OFF      _IOWR(WPS_LED_IOCTL_NUM, 11, int *)

#define DOME_N_RADIO_ON        _IOWR(WPS_LED_IOCTL_NUM, 12, int *)
#define DOME_N_RADIO_OFF       _IOWR(WPS_LED_IOCTL_NUM, 13, int *)
#define DOME_G_RADIO_ON        _IOWR(WPS_LED_IOCTL_NUM, 14, int *)
#define DOME_G_RADIO_OFF       _IOWR(WPS_LED_IOCTL_NUM, 15, int *)

/* For USB2 LED */
#define USB2_LED_STATE_ON      _IOWR(WPS_LED_IOCTL_NUM, 16, int *)
#define USB2_LED_STATE_OFF     _IOWR(WPS_LED_IOCTL_NUM, 17, int *)

#define WPS_LED_BLINK_AP_LOCKDOWN   _IOWR(WPS_LED_IOCTL_NUM, 18, int *)


#define LED_CONTROL_ENABLE_BLINK    _IOWR(WPS_LED_IOCTL_NUM, 19, int *)
#define LED_CONTROL_DISABLE_BLINK   _IOWR(WPS_LED_IOCTL_NUM, 20, int *)
#define LED_CONTROL_TURN_OFF        _IOWR(WPS_LED_IOCTL_NUM, 21, int *)

//#if defined(R8000)
#define WLAN_G_2_RADIO_ON        _IOWR(WPS_LED_IOCTL_NUM, 22, int *)
#define WLAN_G_2_RADIO_OFF       _IOWR(WPS_LED_IOCTL_NUM, 23, int *)
//#endif

#define WPS_LED_STOP_NO         (0)
#define WPS_LED_STOP_RADIO_OFF  (1)
#define WPS_LED_STOP_DISABLED   (2)

#endif
