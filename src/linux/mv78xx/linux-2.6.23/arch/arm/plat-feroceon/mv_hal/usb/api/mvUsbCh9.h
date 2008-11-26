/*******************************************************************************

This software file (the "File") is distributed by Marvell International Ltd. 
or its affiliate(s) under the terms of the GNU General Public License Version 2, 
June 1991 (the "License").  You may use, redistribute and/or modify this File 
in accordance with the terms and conditions of the License, a copy of which 
is available along with the File in the license.txt file or by writing to the 
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 
or on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.

(C) Copyright 2004 - 2007 Marvell Semiconductor Israel Ltd. All Rights Reserved.
(C) Copyright 1999 - 2004 Chipidea Microelectronica, S.A. All Rights Reserved.

*******************************************************************************/

#ifndef __mvUsbCh9_h__
#define __mvUsbCh9_h__

#include "mvUsbTypes.h"
/*----------------------------------------------------------------**
** Chapter 9.4 Standard Device Requests -- all devices            **
** See Table 9-3 p. 250 of USB 2.0 spec for combinations          **
**   of request type bitfields with requests, WVALUE, WINDEX etc. **
**----------------------------------------------------------------*/
#define REQ_RECIP_MASK          0x1f
#define REQ_RECIP_DEVICE        0x00
#define REQ_RECIP_INTERFACE     0x01
#define REQ_RECIP_ENDPOINT      0x02
#define REQ_RECIP_OTHER         0x03

/* Also for class requests set the following bit */
#define REQ_TYPE_OFFSET         5
#define REQ_TYPE_MASK			(0x03 << REQ_TYPE_OFFSET)
#define REQ_TYPE_STANDARD		(0x00 << REQ_TYPE_OFFSET)
#define REQ_TYPE_CLASS			(0x01 << REQ_TYPE_OFFSET)
#define REQ_TYPE_VENDOR			(0x02 << REQ_TYPE_OFFSET)
#define REQ_TYPE_RESERVED		(0x03 << REQ_TYPE_OFFSET)

/* Combine one of the 3 above with one of the following 2 */
#define REQ_DIR_OFFSET         7
#define REQ_DIR_IN             (1 << REQ_DIR_OFFSET)
#define REQ_DIR_OUT            (0 << REQ_DIR_OFFSET)

/* Standard USB requests, see Chapter 9 */
#define REQ_GET_STATUS          0
#define REQ_CLEAR_FEATURE       1
#define REQ_SET_FEATURE         3
#define REQ_SET_ADDRESS         5
#define REQ_GET_DESCRIPTOR      6
#define REQ_SET_DESCRIPTOR      7
#define REQ_GET_CONFIGURATION   8
#define REQ_SET_CONFIGURATION   9
#define REQ_GET_INTERFACE       10
#define REQ_SET_INTERFACE       11
#define REQ_SYNCH_FRAME         12

#define DESC_TYPE_DEVICE         0x1
#define DESC_TYPE_CONFIG         0x2
#define DESC_TYPE_STRING         0x3
#define DESC_TYPE_INTERFACE      0x4
#define DESC_TYPE_ENDPOINT       0x5
#define DESC_TYPE_QUALIFIER      0x6
#define DESC_TYPE_OTHER_SPEED    0x7
#define DESC_TYPE_INTF_POWER     0x8
#define DESC_TYPE_OTG            0x9

/*******************************************************************
**
** Values specific to CLEAR FEATURE commands (must go to common.h later)
*/

#define  ENDPOINT_HALT          0
#define  DEVICE_SELF_POWERED    0
#define  DEVICE_REMOTE_WAKEUP   1
#define  DEVICE_TEST_MODE       2


/* States of device instances on the device list */

/* initial device state */
#define  DEVSTATE_INITIAL        0x00

/* device descriptor [0..7]*/
#define  DEVSTATE_DEVDESC8       0x01

/* address set */
#define  DEVSTATE_ADDR_SET       0x02

/* full device descriptor */
#define  DEVSTATE_DEV_DESC       0x03

/* config descriptor [0..7] */
#define  DEVSTATE_GET_CFG9       0x04

/* config set */
#define  DEVSTATE_SET_CFG        0x05

/* full config desc. read in */
#define  DEVSTATE_CFG_READ       0x06

/* application callbacks */
#define  DEVSTATE_APP_CALL       0x07

/* Select interface done */
#define  DEVSTATE_SET_INTF       0x08

#define  DEVSTATE_ENUM_OK        0x09

#define  DEVSTATE_CHK_OTG        0x0A

/* Event codes for attach/detach etc. callback */
#define  USB_ATTACH_EVENT        1   /* device attach */
#define  USB_DETACH_EVENT        2   /* device detach */
#define  USB_CONFIG_EVENT        3   /* device reconfigured */
#define  USB_INTF_EVENT          4   /* device interface selected */

#endif /* __mvUsbCh9_h__ */

/* EOF */
