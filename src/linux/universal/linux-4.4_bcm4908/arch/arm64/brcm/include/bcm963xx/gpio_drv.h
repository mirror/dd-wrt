/***************************************************************************
***
***    Copyright 2010  Hon Hai Precision Ind. Co. Ltd.
***    All Rights Reserved.
***    No portions of this material shall be reproduced in any form without the
***    written permission of Hon Hai Precision Ind. Co. Ltd.
***
***    All information contained in this document is Hon Hai Precision Ind.  
***    Co. Ltd. company private, proprietary, and trade secret property and 
***    are protected by international intellectual property laws and treaties.
***
****************************************************************************/

#ifndef __GPIO_DRV_H__
#define __GPIO_DRV_H__

#define DEV_GPIO_DRV        "gpio_drv"

#define GPIO_IOCTL_NUM   'W'

#define IOCTL_LAN_LED_STATE       _IOWR(GPIO_IOCTL_NUM, 0, int *)
#define IOCTL_USB_LED_STATE       _IOWR(GPIO_IOCTL_NUM, 1, int *)
#define IOCTL_WPS_LED_STATE       _IOWR(GPIO_IOCTL_NUM, 2, int *)
#define IOCTL_VOIP_LED_OFF        _IOWR(GPIO_IOCTL_NUM, 3, int *)
#define IOCTL_VOIP_LED_ON         _IOWR(GPIO_IOCTL_NUM, 4, int *)
#define IOCTL_VOIP_LED_BS         _IOWR(GPIO_IOCTL_NUM, 5, int *)
#define IOCTL_VOIP_LED_BF         _IOWR(GPIO_IOCTL_NUM, 6, int *)
#define IOCTL_VOIP_LED_BN         _IOWR(GPIO_IOCTL_NUM, 7, int *)
#define IOCTL_WAN_LED_STATE       _IOWR(GPIO_IOCTL_NUM, 8, int *)
#define IOCTL_LAN_VLAN_ID         _IOWR(GPIO_IOCTL_NUM, 9, int *)
#define IOCTL_WAN_VLAN_ID         _IOWR(GPIO_IOCTL_NUM, 10, int *)
#define IOCTL_3G_LED_STATE        _IOWR(GPIO_IOCTL_NUM, 11, int *)  

#define IOCTL_PVC_DET_START       _IOWR(GPIO_IOCTL_NUM, 12, int *)  
#define IOCTL_PVC_DET_STOP        _IOWR(GPIO_IOCTL_NUM, 13, int *)
#define IOCTL_PVC_DET_RESULT      _IOWR(GPIO_IOCTL_NUM, 14, int *)

#define IOCTL_ETH_WAN_LED_STATE   _IOWR(GPIO_IOCTL_NUM, 15, int *)
#define IOCTL_INTERNET_LED_STATE  _IOWR(GPIO_IOCTL_NUM, 16, int *)
#define IOCTL_INTERNET_IFNAME     _IOWR(GPIO_IOCTL_NUM, 17, int *)

#define IOCTL_USB_1_LED_STATE       _IOWR(GPIO_IOCTL_NUM, 18, int *)
#define IOCTL_USB_2_LED_STATE       _IOWR(GPIO_IOCTL_NUM, 19, int *)


#define IOCTL_SET_TE_TEST_FLAG      _IOWR(GPIO_IOCTL_NUM, 20, int *)


#define IOCTL_PVC_PROTOCOL_DET_START    _IOWR(GPIO_IOCTL_NUM, 21, int *)  


/* for U12L216, to blink USB LED while detection devices */
#define IOCTL_USB_1_BLINK    _IOWR(GPIO_IOCTL_NUM, 22, int *) 
#define IOCTL_PWR_BLINK      _IOWR(GPIO_IOCTL_NUM, 23, int *) 
#define IOCTL_PWR_AMBER_BLINK   _IOWR(GPIO_IOCTL_NUM, 24, int *)


#define LED_CONTROL_ENABLE_BLINK    _IOWR(GPIO_IOCTL_NUM, 25, int *)
#define LED_CONTROL_DISABLE_BLINK   _IOWR(GPIO_IOCTL_NUM, 26, int *)
#define LED_CONTROL_TURN_OFF        _IOWR(GPIO_IOCTL_NUM, 27, int *)


/* USB partitions counter - design for D6400 with 2 USB ports */
#define IOCTL_USB_SDA_COUNT         _IOWR(GPIO_IOCTL_NUM, 28, int *)
#define IOCTL_USB_SDB_COUNT         _IOWR(GPIO_IOCTL_NUM, 29, int *)
#define IOCTL_USB_SDA_REMOVED_FLAG  _IOWR(GPIO_IOCTL_NUM, 30, int *)
#define IOCTL_USB_SDB_REMOVED_FLAG  _IOWR(GPIO_IOCTL_NUM, 31, int *)

typedef struct _INTERNET_IFNAME
{
    char ifname[128];
}INTERNET_IFNAME;

#endif  /* __GPIO_DRV_H__ */

