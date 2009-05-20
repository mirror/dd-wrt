/*
 * IPWireless 3G UMTS TDD Modem driver (USB connected)
 *
 *   Copyright (C) 2004 Roelf Diedericks <roelfd@inet.co.za>
 *   Copyright (C) 2004 Greg Kroah-Hartman <greg@kroah.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 * All information about the device was acquired using SnoopyPro
 * on MSFT's O/S, and examing the MSFT drivers' debug output 
 * (insanely left _on_ in the enduser version)
 *
 * It was written out of frustration with the IPWireless USB modem
 * supplied by Axity3G/Sentech South Africa not supporting
 * Linux whatsoever.
 *
 * Nobody provided any proprietary information that was not already 
 * available for this device.
 * 
 * The modem adheres to the "3GPP TS  27.007 AT command set for 3G 
 * User Equipment (UE)" standard, available from 
 * http://www.3gpp.org/ftp/Specs/html-info/27007.htm
 *
 * The code was only tested the IPWireless handheld modem distributed
 * in South Africa by Sentech.
 * 
 * It may work for Woosh Inc in .nz too, as it appears they use the
 * same kit.
 *
 * There is still some work to be done in terms of handling 
 * DCD, DTR, RTS, CTS which are currently faked.
 * It's good enough for PPP at this point. It's based off all kinds of
 * code found in usb/serial and usb/class
 *
 * Revision history:
 *	Date:		Ver:	Notes:
 *	2004/04/24	0.1	First public release
 *	2004/04/29	0.2	use softirq for rx processing, as needed by tty layer
 *				change completely to using spinlocks instead of down()
 *	2005/09/10	0.3	Updated for >=2.6.11 kernels. Fix uninitialized spinlock
 *	2005/10/10	0.4	Added functionality for modem's "console" device
 *				Changed initialisation on device open() to toggle DTR,
 *				this appears to reset the AT command parser to a sane state.
 *				Modprobe argument "debug=1" will now provide debugging on 
 *				demand.
 *
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/usb.h>
#include <asm/uaccess.h>

#ifdef CONFIG_USB_SERIAL_DEBUG
        static int debug = 1;
#else
        static int debug;
#endif

#include "usb-serial.h"
#include "ipw.h"

/*
 * Version Information
 */
#define DRIVER_VERSION	"v0.4a"
#define DRIVER_AUTHOR	"Roelf Diedericks"
#define DRIVER_DESC	"IPWireless tty driver"

#define IPW_TTY_MAJOR	240	/* real device node major id, experimental range */
#define IPW_TTY_MINORS	256	/* we support 256 devices, dunno why, it'd be insane :) */

#define USB_IPW_MAGIC	0x6d02	/* magic number for ipw struct */
#define IPW_URB_TIMEOUT (HZ * 200)


/* Message sizes */
#define EVENT_BUFFER_SIZE       0xFF
#define CHAR2INT16(c1,c0)       (((u32)((c1) & 0xff) << 8) + (u32)((c0) & 0xff))
#define NUM_BULK_URBS           24
#define NUM_CONTROL_URBS        16


static struct usb_device_id usb_ipw_ids[] = {
	{ USB_DEVICE(IPW_VID, IPW_PID1) },
	{ USB_DEVICE(IPW_VID, IPW_PID2) },
	{ },
};

MODULE_DEVICE_TABLE(usb, usb_ipw_ids);

struct ipw_private {
	int		magic;

	spinlock_t	lock;
	int		write_urb_busy;

	u16		ifnum;   /* type of interface: 0=modem, or 1=console */
	unsigned int    control_lines; /* dsr/cts/rts status bits */
};

static LIST_HEAD(usb_serial_driver_list);

static void show_status(struct usb_serial_port *port)
{
	struct usb_device *dev = port->serial->dev;
	struct ipw_private *priv = usb_get_serial_port_data(port);
	unsigned char buf[10];
	int response;

	response = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			IPW_SIO_POLL,
			USB_TYPE_VENDOR|USB_DIR_IN|USB_RECIP_INTERFACE,  // Request type
			0x0000,
			priv->ifnum,
			buf, sizeof(buf), IPW_URB_TIMEOUT);
	if (response >= 1)
		dbg("status = 0x%02d", (int) buf[0]);
	else
		info("failed to read status: %d\n", (int) response);
}

static int ipw_set_pins (struct usb_serial_port *port, unsigned int set, unsigned int clear) 
{
	struct usb_device *dev = port->serial->dev;
	struct ipw_private *priv = usb_get_serial_port_data(port);
	unsigned int control = 0;
	int response;
	unsigned long flags; /* for spinlock */

	dbg("%s", __FUNCTION__);
/*
	if (!ipw->open_count) {
		dbg ("%s - device not open", __FUNCTION__);
		return -EINVAL;
	}
*/
	// The Request 0x07 holds the modem control signals.
	// 
	//   wValue: B0 DTR State
	//           B1 RTS State
	//           B2..B7 Reserved
	//           B8 DTR Mask (if clear do not act on DTR state)
	//           B9 RTS Mask (if clear do not act on RTS state)
	//           B10..B15 Reserved
	//

	spin_lock_irqsave( &priv->lock, flags ); 
	priv->control_lines &= ~clear;
	priv->control_lines |= set;
        spin_unlock_irqrestore( &priv->lock, flags );

	if (set & TIOCM_RTS) {
		control |= 0x202;
		dbg("port %d set RTS", port->number);
	}
	if (set & TIOCM_DTR) {
		control |= 0x101;
		dbg("port %d set DTR", port->number);
	}
	if (clear & TIOCM_RTS) {
		control |= 0x200;
		dbg("port %d clear RTS", port->number);
	}
	if (clear & TIOCM_DTR) {
		control |= 0x100;
		dbg("port %d clear DTR", port->number);
	}

	if (control == 0)
		response = 0;
	else
		response = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				IPW_SIO_SET_PIN,
				USB_TYPE_VENDOR|USB_DIR_OUT|USB_RECIP_INTERFACE,  // Request type
				control,
				priv->ifnum,
				NULL, 0, IPW_URB_TIMEOUT);
	if (response < 0)
		err("tiocmset failed: response to serial pins change = %d", response);

        return response;

}


static void ipw_read_bulk_callback(struct urb *urb)
{
	struct usb_serial_port *port = urb->context;
	unsigned char *data = urb->transfer_buffer;
	struct tty_struct *tty;
	int i, result;

	dbg("%s - port %d", __FUNCTION__, port->number);

	if (urb->status) {
		dbg("%s - nonzero read bulk status received: %d", __FUNCTION__, urb->status);
		return;
	}

	usb_serial_debug_data(__FILE__, __FUNCTION__, urb->actual_length, data);

	tty = port->tty;
	if (tty && urb->actual_length) {
//2.6		tty_buffer_request_room(tty, urb->actual_length);
//2.6		tty_insert_flip_string(tty, data, urb->actual_length);
		for (i = 0; i < urb->actual_length ; ++i) {
			/* if we insert more than TTY_FLIPBUF_SIZE characters, we drop them. */
			if(tty->flip.count >= TTY_FLIPBUF_SIZE) {
				tty_flip_buffer_push(tty);
			}
			/* this doesn't actually push the data through unless tty->low_latency is set */
			tty_insert_flip_char(tty, data[i], 0);
		}
		tty_flip_buffer_push(tty);
	}

	/* Continue trying to always read  */
	usb_fill_bulk_urb (port->read_urb, port->serial->dev,
			   usb_rcvbulkpipe(port->serial->dev,
					   port->bulk_in_endpointAddress),
			   port->read_urb->transfer_buffer,
			   port->read_urb->transfer_buffer_length,
			   ipw_read_bulk_callback, port);
	result = usb_submit_urb(port->read_urb);
	if (result)
		err("%s - failed resubmitting read urb, error %d\n", __FUNCTION__, result);
	return;
}

static char* ipw_iftype_name (u16 ifnum) {
	return ifnum == 0 ? "modem" : "console";
}

static int ipw_startup(struct usb_serial *serial)
{
	struct ipw_private *priv;
	u16 ifnum = serial->interface->altsetting->bInterfaceNumber;
	int i;

        for (i = 0; i < serial->num_ports; ++i) {                               
 		priv = kmalloc (sizeof (struct ipw_private), GFP_KERNEL);
		if (!priv)
			goto cleanup;
		memset (priv, 0x00, sizeof (struct ipw_private));
		priv->magic = USB_IPW_MAGIC;
		priv->ifnum = ifnum;
		spin_lock_init(&priv->lock);
		usb_set_serial_port_data(&serial->port[i], priv);
	}
	info("IPWireless USB (%s) found",ipw_iftype_name(ifnum));
	return 0;

cleanup:
	for (--i; i>=0; --i) {
		priv = usb_get_serial_port_data(&serial->port[i]);              
		kfree(priv);                                                    
		usb_set_serial_port_data(&serial->port[i], NULL);               
	}                                                                       
	return -ENOMEM;                                                         
}

static int ipw_open(struct usb_serial_port *port, struct file *filp)
{
	struct usb_device *dev = port->serial->dev;
	struct ipw_private *priv = usb_get_serial_port_data(port);
	u8 buf_flow_init[16] = IPW_BYTES_FLOWINIT;
	u8 buf_setchars_init[6] = IPW_BYTES_SETCHARSINIT_NONE;
	unsigned char buf[32];
	int result;

	dbg("%s", __FUNCTION__);

	if (port->tty)
		port->tty->low_latency = 1;

	/* --1: Tell the modem to initialize (we think) From sniffs this is always the
	 * first thing that gets sent to the modem during opening of the device */
	dbg("%s: Sending SIO_INIT (we guess)",__FUNCTION__);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev,0),
				 IPW_SIO_INIT,
				 USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
				 0,
				 priv->ifnum, /* index */
				 NULL,
				 0,
				 100000);
	if (result < 0)
		err("%s Init of modem failed (error = %d)", __FUNCTION__, result);

	/* reset the bulk pipes */
	usb_clear_halt(dev, usb_rcvbulkpipe(dev, port->bulk_in_endpointAddress));
	usb_clear_halt(dev, usb_sndbulkpipe(dev, port->bulk_out_endpointAddress));

	/*--2: Start reading from the device */	
	dbg("%s: setting up bulk read callback",__FUNCTION__);
	usb_fill_bulk_urb(port->read_urb, dev,
			  usb_rcvbulkpipe(dev, port->bulk_in_endpointAddress),
			  port->read_urb->transfer_buffer,
			  port->read_urb->transfer_buffer_length,
			  ipw_read_bulk_callback, port);
	result = usb_submit_urb(port->read_urb);
	if (result < 0)
		dbg("%s - usb_submit_urb(read bulk) failed with status %d", __FUNCTION__, result);

	/*--3: Tell the modem to open the floodgates on the rx bulk channel */
	dbg("%s:asking modem for RxRead (RXBULK_ON)",__FUNCTION__);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				 IPW_SIO_RXCTL,
				 USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
				 IPW_RXBULK_ON,
				 priv->ifnum, /* index */
				 NULL,
				 0,
				 100000);
	if (result < 0) 
		err("%s - Enabling bulk RxRead failed (error = %d)", __FUNCTION__, result);

	/*--4: setup the initial flowcontrol */
	dbg("%s:setting init flowcontrol (%s)",__FUNCTION__,buf_flow_init);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				 IPW_SIO_HANDFLOW,
				 USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
				 0,
				 priv->ifnum, /* index */
				 buf_flow_init,
				 0x10,
				 200000);
	if (result < 0)
		err("%s - initial flowcontrol failed (error = %d)", __FUNCTION__, result);

	show_status(port);
	if (priv->ifnum == 0x01) { //console 
		dbg("raise dtr for modem");
		dbg("%s:raising dtr",__FUNCTION__);
		result = ipw_set_pins(port, TIOCM_DTR, 0); //raise DTR
		if (result<0)
			err("raising dtr failed (error = %d)", result);

	} else {
		dbg("not raising dtr for console");
	}

	/*--5: fetch some unkn buffer */
	dbg("%s:fetching unk_buffer",__FUNCTION__);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				IPW_SIO_FETCH_UNK,
				USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_IN,
				0x0000,
				priv->ifnum, /* index */
				buf,
				sizeof(buf),
				IPW_URB_TIMEOUT);
	if (result>=0)
		info("fetch unk_buffer got %d bytes.", result);
	else
		dbg("fetch unk_buffer failed (error = %d)", result);

	/*--6: purge */
	dbg("%s:sending purge",__FUNCTION__);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				IPW_SIO_PURGE,
				USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
				0x0f,
				priv->ifnum,
				NULL,
				0,
				IPW_URB_TIMEOUT);
	if (result<0)
		err("purge failed (error = %d)", result);

	/*--7: set bitrate*/
	dbg("%s:setting bitrate ",__FUNCTION__);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				IPW_SIO_SET_BAUD,
				USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
				0x0180,
				priv->ifnum,
				NULL,
				0,
				IPW_URB_TIMEOUT);
	if (result<0)
		err("setting bitrate failed (error = %d)", result);


	/*--11: raise the rts */
	dbg("%s:raising rts",__FUNCTION__);
	result = ipw_set_pins(port, TIOCM_RTS, 0); //raise RTS
	if (result<0)
		err("raising rts failed (error = %d)", result);

	dbg("%s:raising dtr",__FUNCTION__);
	result = ipw_set_pins(port, TIOCM_DTR, 0); //raise DTR
	if (result<0)
		err("raising dtr failed (error = %d)", result);


	/*--8: set parity/databits*/
	dbg("%s:setting parity ",__FUNCTION__);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				IPW_SIO_SET_LINE,
				USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
				0x800,
				priv->ifnum,
				NULL,
				0,
				IPW_URB_TIMEOUT);
	if (result<0)
		err("setting bitrate failed (error = %d)", result);


	/*--9: setup the flow control characters*/
	dbg("%s:setting init setflow characters",__FUNCTION__);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				IPW_SIO_SETCHARS,
				USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
				0x0000,
				priv->ifnum, /* index */
				buf_setchars_init,
				sizeof(buf_setchars_init),
				IPW_URB_TIMEOUT);
	if (result<0)
		err("initial setflow characters failed (error = %d)", result);

	dbg("%s:setting second flowcontrol",__FUNCTION__);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				IPW_SIO_HANDFLOW,
				USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
				0x0000,
				priv->ifnum, /* index */
				buf_flow_init,
				sizeof(buf_flow_init),
				IPW_URB_TIMEOUT);

	return result;
}

static void ipw_close(struct usb_serial_port *port, struct file * filp)
{
	struct usb_device *dev = port->serial->dev;
	int result;

	if (tty_hung_up_p(filp)) {
		dbg("%s: tty_hung_up_p ...", __FUNCTION__);
		return;
	}

	/*--1: drop the dtr */
	dbg("%s:dropping dtr",__FUNCTION__);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				 IPW_SIO_SET_PIN,
				 USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
				 IPW_PIN_CLRDTR,
				 0,
				 NULL,
				 0,
				 200000);
	if (result < 0)
		err("%s - dropping dtr failed (error = %d)", __FUNCTION__, result);

	/*--2: drop the rts */
	dbg("%s:dropping rts",__FUNCTION__);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				 IPW_SIO_SET_PIN, USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
				 IPW_PIN_CLRRTS,
				 0,
				 NULL,
				 0,
				 200000);
	if (result < 0)
		err("%s - dropping rts failed (error = %d)", __FUNCTION__, result);


	/*--3: purge */
	dbg("%s:sending purge",__FUNCTION__);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				 IPW_SIO_PURGE, USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
				 0x03,
				 0,
				 NULL,
				 0,
				 200000);
	if (result < 0)
		err("%s - purge failed (error = %d)", __FUNCTION__, result);


	/* send RXBULK_off (tell modem to stop transmitting bulk data on rx chan) */
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				 IPW_SIO_RXCTL,
				 USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
				 IPW_RXBULK_OFF,
				 0, /* index */
				 NULL,
				 0,
				 100000);

	if (result < 0)
		err("%s - Disabling bulk RxRead failed (error = %d)", __FUNCTION__, result);

	/* shutdown any in-flight urbs that we know about */
	usb_unlink_urb(port->read_urb);
	usb_unlink_urb(port->write_urb);
}

static int ipw_tiocmset(struct usb_serial_port *port, struct file *file, 
			unsigned int set, unsigned int clear)                
{                                                                               
	return ipw_set_pins(port, set, clear);
}

static void ipw_write_bulk_callback(struct urb *urb)
{
	struct usb_serial_port *port = urb->context;
	struct ipw_private *priv = usb_get_serial_port_data(port);

	dbg("%s", __FUNCTION__);

	priv->write_urb_busy = 0;

	if (urb->status)
		dbg("%s - nonzero write bulk status received: %d", __FUNCTION__, urb->status);

        queue_task(&port->tqueue, &tq_immediate);
        mark_bh(IMMEDIATE_BH);
}

static int ipw_write(struct usb_serial_port *port, int from_user,
		     const unsigned char *buf, int count)
{
	struct usb_device *dev = port->serial->dev;
	struct ipw_private *priv = usb_get_serial_port_data(port);
	int ret;

	dbg("%s: TOP: count=%d, in_interrupt=%d", __FUNCTION__,
		count, in_interrupt() );

	if (count == 0) {
		dbg("%s - write request of 0 bytes", __FUNCTION__);
		return 0;
	}

	spin_lock(&priv->lock);
	if (priv->write_urb_busy) {
		spin_unlock(&priv->lock);
		dbg("%s - already writing", __FUNCTION__);
		return 0;
	}
	priv->write_urb_busy = 1;
	spin_unlock(&priv->lock);

	count = min(count, port->bulk_out_size);
	memcpy(port->bulk_out_buffer, buf, count);

	dbg("%s count now:%d", __FUNCTION__, count);

	usb_fill_bulk_urb(port->write_urb, dev,
			  usb_sndbulkpipe(dev, port->bulk_out_endpointAddress),
			  port->write_urb->transfer_buffer,
			  count,
			  ipw_write_bulk_callback,
			  port);

	ret = usb_submit_urb(port->write_urb);
	if (ret != 0) {
		priv->write_urb_busy = 0;
		dbg("%s - usb_submit_urb(write bulk) failed with error = %d", __FUNCTION__, ret);
		return ret;
	}

	dbg("%s returning %d", __FUNCTION__, count);
	return count;
} 

static void ipw_disconnect(struct usb_serial *serial)
{
	struct usb_serial_port *port;
	struct ipw_private *priv;

	if (serial) {
		port = &serial->port[0];
		if (port->tty)
			tty_hangup(port->tty);
		priv = usb_get_serial_port_data(port);
		kfree(priv);
		usb_set_serial_port_data(port, NULL);
		if (serial->dev) {
                /* shutdown any bulk reads that might be going on */
			if (serial->num_bulk_out)
				usb_unlink_urb (port->write_urb);
			if (serial->num_bulk_in)
				usb_unlink_urb (port->read_urb);
		}
//2.6		usb_serial_generic_shutdown(serial);
		kfree(serial);
	}
}

static struct usb_serial_device_type ipw_device = {
//	.driver = {
		.owner =	THIS_MODULE,
		.name =		"IPWireless converter",
//	},
	.id_table =		usb_ipw_ids,
	.num_interrupt_in =	NUM_DONT_CARE,
	.num_bulk_in =		1,
	.num_bulk_out =		1,
	.num_ports =		1,
	.startup = 		ipw_startup,
	.shutdown =		ipw_disconnect,
	.open =			ipw_open,
	.close =		ipw_close,
//2.6	.port_probe = 		ipw_probe,
	.write =		ipw_write,
	.write_bulk_callback =	ipw_write_bulk_callback,
	.read_bulk_callback =	ipw_read_bulk_callback,
};



static int usb_ipw_init(void)
{
	int retval;

	retval = usb_serial_register(&ipw_device);
	if (retval)
		return retval;
	info(DRIVER_DESC " " DRIVER_VERSION);
	return 0;
}

static void usb_ipw_exit(void)
{
	usb_serial_deregister(&ipw_device);
}

module_init(usb_ipw_init);
module_exit(usb_ipw_exit);

/* Module information */
MODULE_AUTHOR( DRIVER_AUTHOR );
MODULE_DESCRIPTION( DRIVER_DESC );
MODULE_LICENSE("GPL");

MODULE_PARM(debug, "i");
MODULE_PARM_DESC(debug, "Debug enabled or not");
