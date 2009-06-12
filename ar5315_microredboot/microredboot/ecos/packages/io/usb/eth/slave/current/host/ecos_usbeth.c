//==========================================================================
//
//      ecos_usbeth.c
//
//      Linux device driver for eCos-based USB ethernet peripherals.
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####                                             
//
// Author(s):           bartv
// Contributors:        bartv
// Date:                2000-11-12
//
//####DESCRIPTIONEND####
//==========================================================================

#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#ifdef MODULE
MODULE_AUTHOR("Bart Veer <bartv@redhat.com>");
MODULE_DESCRIPTION("USB ethernet driver for eCos-based peripherals");
#endif

// This array identifies specific implementations of eCos USB-ethernet
// devices. All implementations should add their vendor and device
// details.

typedef struct ecos_usbeth_impl {
    const char* name;
    __u16       vendor;
    __u16       id;
} ecos_usbeth_impl;

const static ecos_usbeth_impl ecos_usbeth_implementations[] = {
    { "eCos ether",     0x4242, 0x4242 },
    { (const char*) 0,       0,      0 }
};


// Constants. These have to be kept in sync with the target-side
// code.
#define ECOS_USBETH_MAXTU                           1516
#define ECOS_USBETH_MAX_CONTROL_TU                     8
#define ECOS_USBETH_CONTROL_GET_MAC_ADDRESS         0x01
#define ECOS_USBETH_CONTROL_SET_PROMISCUOUS_MODE    0x02

// The main data structure. It keeps track of both the USB
// and network side of things, and provides buffers for
// the various operations.
//
// NOTE: currently this driver only supports a single
// plugged-in device. Theoretically multiple eCos-based
// USB ethernet devices could be plugged in to a single
// host and each one would require an ecos_usbeth
// structure.
typedef struct ecos_usbeth {
    spinlock_t                  usb_lock;
    int                         target_promiscuous;
    struct usb_device*          usb_dev;
    struct net_device*          net_dev;
    struct net_device_stats     stats;
    struct urb                  rx_urb;
    struct urb                  tx_urb;
    unsigned char               rx_buffer[ECOS_USBETH_MAXTU];
    unsigned char               tx_buffer[ECOS_USBETH_MAXTU];
} ecos_usbeth;


// open()
// Invoked by the TCP/IP stack when the interface is brought up.
// This just starts a receive operation.
static int
ecos_usbeth_open(struct net_device* net)
{
    ecos_usbeth* usbeth = (ecos_usbeth*) net->priv;
    int          res;

    netif_start_queue(net);
    res = usb_submit_urb(&(usbeth->rx_urb));
    if (0 != res) {
        printk("ecos_usbeth: failed to start USB receives, %d\n", res);
    }
    MOD_INC_USE_COUNT;
    return 0;
}

// close()
// Invoked by the TCP/IP stack when the interface is taken down.
// Any active USB operations need to be cancelled. During
// a disconnect this may get called twice, once for the
// disconnect and once for the network interface being
// brought down.
static int
ecos_usbeth_close(struct net_device* net)
{
    ecos_usbeth* usbeth = (ecos_usbeth*) net->priv;

    if (0 != netif_running(net)) {
        netif_stop_queue(net);
        net->start = 0;

        if (-EINPROGRESS == usbeth->rx_urb.status) {
            usb_unlink_urb(&(usbeth->rx_urb));
        }
        if (-EINPROGRESS == usbeth->tx_urb.status) {
            usb_unlink_urb(&(usbeth->tx_urb));
        }
        MOD_DEC_USE_COUNT;
    }
    
    return 0;
}

// Reception.
// probe() fills in an rx_urb. When the net device is brought up
// the urb is activated, and this callback gets run for incoming
// data.
static void
ecos_usbeth_rx_callback(struct urb* urb)
{
    ecos_usbeth*        usbeth  = (ecos_usbeth*) urb->context;
    struct net_device*  net     = usbeth->net_dev;
    struct sk_buff*     skb;
    int                 len;
    int                 res;

    if (0 != urb->status) {
        // This happens numerous times during a disconnect. Do not
        // issue a warning, but do clear the status field or things
        // get confused when resubmitting.
        //
        // Some host hardware does not distinguish between CRC errors
        // (very rare) and timeouts (perfectly normal). Do not
        // increment the error count if it might have been a timeout.
        if (USB_ST_CRC != urb->status) {
            usbeth->stats.rx_errors++;
        }
        urb->status = 0;
    } else if (2 > urb->actual_length) {
        // With some hardware the target may have to send a bogus
        // first packet. Just ignore those.

    } else {
        len = usbeth->rx_buffer[0] + (usbeth->rx_buffer[1] << 8);
        if (len > (urb->actual_length - 2)) {
            usbeth->stats.rx_errors++;
            usbeth->stats.rx_length_errors++;
            printk("ecos_usbeth: warning, packet size mismatch, got %d bytes, expected %d\n",
                   urb->actual_length, len);
        } else {
            skb = dev_alloc_skb(len + 2);
            if ((struct sk_buff*)0 == skb) {
                printk("ecos_usbeth: failed to alloc skb, dropping packet\n");
                usbeth->stats.rx_dropped++;
            } else {
#if 0
                {
                    int i;
                    printk("--------------------------------------------------------------\n");
                    printk("ecos_usbeth RX: total size %d\n", len);
                    for (i = 0; (i < len) && (i < 128); i+= 8) {
                        printk("rx  %x %x %x %x %x %x %x %x\n",
                               usbeth->rx_buffer[i+0], usbeth->rx_buffer[i+1], usbeth->rx_buffer[i+2], usbeth->rx_buffer[i+3],
                               usbeth->rx_buffer[i+4], usbeth->rx_buffer[i+5], usbeth->rx_buffer[i+6], usbeth->rx_buffer[i+7]);
                    }
                    printk("--------------------------------------------------------------\n");
                }
#endif
                skb->dev        = net;
                eth_copy_and_sum(skb, &(usbeth->rx_buffer[2]), len, 0);
                skb_put(skb, len);
                skb->protocol   = eth_type_trans(skb, net);
                netif_rx(skb);
                usbeth->stats.rx_packets++;
                usbeth->stats.rx_bytes += len;
            }
        }
    }

    if (0 != netif_running(net)) {
        res = usb_submit_urb(&(usbeth->rx_urb));
        if (0 != res) {
            printk("ecos_usbeth: failed to restart USB receives after packet, %d\n", res);
        }
    }
}

// start_tx().
// Transmit a single packet. The relevant USB protocol requires a
// 2-byte length field at the start, the incoming buffer has no space
// for this, and the URB API does not support any form of
// scatter/gather. Therefore unfortunately the whole packet has to be
// copied. The callback function is specified when the URB is filled
// in by probe().
static void
ecos_usbeth_tx_callback(struct urb* urb)
{
    ecos_usbeth* usbeth = (ecos_usbeth*) urb->context;
    spin_lock(&usbeth->usb_lock);
    if (0 != netif_running(usbeth->net_dev)) {
        netif_wake_queue(usbeth->net_dev);
    }
    spin_unlock(&usbeth->usb_lock);
}

static int
ecos_usbeth_start_tx(struct sk_buff* skb, struct net_device* net)
{
    ecos_usbeth* usbeth = (ecos_usbeth*) net->priv;
    int          res;

    if ((skb->len + 2) > ECOS_USBETH_MAXTU) {
        printk("ecos_usbeth: oversized packet of %d bytes\n", skb->len);
        return 0;
    }

    if (netif_queue_stopped(net)) {
        // Another transmission already in progress.
        // USB bulk operations should complete within 5s.
        int current_delay = jiffies - net->trans_start;
        if (current_delay < (5 * HZ)) {
            return 1;
        } else {
            // There has been a timeout. Discard this message.
            //printk("transmission timed out\n");
            usbeth->stats.tx_errors++;
            dev_kfree_skb(skb);
            return 0;
        }
    }

    spin_lock(&usbeth->usb_lock);
    usbeth->tx_buffer[0]        = skb->len & 0x00FF;
    usbeth->tx_buffer[1]        = (skb->len >> 8) & 0x00FF;
    memcpy(&(usbeth->tx_buffer[2]), skb->data, skb->len);
    usbeth->tx_urb.transfer_buffer_length = skb->len + 2;

    // Some targets are unhappy about receiving 0-length packets, not
    // just sending them.
    if (0 == (usbeth->tx_urb.transfer_buffer_length % 64)) {
        usbeth->tx_urb.transfer_buffer_length++;
    }
#if 0
    {
        int i;
        printk("--------------------------------------------------------------\n");
        printk("ecos_usbeth start_tx: len %d\n", skb->len + 2);
        for (i = 0; (i < (skb->len + 2)) && (i < 128); i+= 8) {
            printk("tx  %x %x %x %x %x %x %x %x\n",
                   usbeth->tx_buffer[i], usbeth->tx_buffer[i+1], usbeth->tx_buffer[i+2], usbeth->tx_buffer[i+3],
                   usbeth->tx_buffer[i+4], usbeth->tx_buffer[i+5], usbeth->tx_buffer[i+6], usbeth->tx_buffer[i+7]);
        }
        printk("--------------------------------------------------------------\n");
    }
#endif
    res = usb_submit_urb(&(usbeth->tx_urb));
    if (0 == res) {
        netif_stop_queue(net);
        net->trans_start        = jiffies;
        usbeth->stats.tx_packets++;
        usbeth->stats.tx_bytes  += skb->len;
    } else {        
        printk("ecos_usbeth: failed to start USB packet transmission, %d\n", res);
        usbeth->stats.tx_errors++;
    }
    
    spin_unlock(&usbeth->usb_lock);
    dev_kfree_skb(skb);
    return 0;
}


// set_rx_mode()
// Invoked by the network stack to enable/disable promiscuous mode or
// for multicasting. The latter is not yet supported on the target
// side. The former involves a USB control message. The main call
// is not allowed to block.
static void
ecos_usbeth_set_rx_mode_callback(struct urb* urb)
{
    kfree(urb->setup_packet);
    usb_free_urb(urb);
}

static void
ecos_usbeth_set_rx_mode(struct net_device* net)
{
    ecos_usbeth* usbeth         = (ecos_usbeth*) net->priv;
    __u16        promiscuous    = net->flags & IFF_PROMISC;
    int          res;
    
    if (promiscuous != usbeth->target_promiscuous) {
        devrequest*     req;
        urb_t*          urb;

        urb = usb_alloc_urb(0);
        if ((urb_t*)0 == urb) {
            return;
        }
        req = kmalloc(sizeof(devrequest), GFP_KERNEL);
        if ((devrequest*)0 == req) {
            usb_free_urb(urb);
            return;
        }
        req->requesttype        = USB_TYPE_CLASS | USB_RECIP_DEVICE;
        req->request            = ECOS_USBETH_CONTROL_SET_PROMISCUOUS_MODE;
        req->value              = cpu_to_le16p(&promiscuous);
        req->index              = 0;
        req->length             = 0;

        FILL_CONTROL_URB(urb,
                         usbeth->usb_dev,
                         usb_sndctrlpipe(usbeth->usb_dev, 0),
                         (unsigned char*) req,
                         (void*) 0,
                         0,
                         &ecos_usbeth_set_rx_mode_callback,
                         (void*) usbeth);
        res = usb_submit_urb(urb);
        if (0 != res) {
            kfree(req);
            usb_free_urb(urb);
        } else {
            usbeth->target_promiscuous = promiscuous;
        }
    }
}

// netdev_stats()
// Supply the current network statistics. These are held in
// the stats field of the ecos_usbeth structure
static struct net_device_stats*
ecos_usbeth_netdev_stats(struct net_device* net)
{
    ecos_usbeth* usbeth = (ecos_usbeth*) net->priv;
    return &(usbeth->stats);
}

// ioctl()
// Currently none of the network ioctl()'s are supported
static int
ecos_usbeth_ioctl(struct net_device* net, struct ifreq* request, int command)
{
    return -EINVAL;
}

// probe().
// This is invoked by the generic USB code when a new device has
// been detected and its configuration details have been extracted
// and stored in the usbdev structure. The interface_id specifies
// a specific USB interface, to cope with multifunction peripherals.
//
// FIXME; right now this code only copes with simple enumeration data.
// OK, to be really honest it just looks for the vendor and device ids
// in the simple test cases and ignores everything else.
//
// On success it should return a non-NULL pointer, which happens to be
// a newly allocated ecos_usbeth structure. This will get passed to
// the disconnect function. Filling in the ecos_usbeth structure will,
// amongst other things, register this as a network device driver.
// The MAC address is obtained from the peripheral via a control
// request.

static void*
ecos_usbeth_probe(struct usb_device* usbdev, unsigned int interface_id)
{
    struct net_device*  net;
    ecos_usbeth*        usbeth;
    int                 res;
    unsigned char       MAC[6];
    unsigned char       dummy[1];
    int                 tx_endpoint = -1;
    int                 rx_endpoint = -1;
    const ecos_usbeth_impl*   impl;
    int                 found_impl = 0;

    // See if this is the correct driver for this USB peripheral.
    impl = ecos_usbeth_implementations;
    while (impl->name != NULL) {
        if ((usbdev->descriptor.idVendor  != impl->vendor) ||
            (usbdev->descriptor.idProduct != impl->id)) {
            found_impl = 1;
            break;
        }
        impl++;
    }
    if (! found_impl) {
        return (void*) 0;
    }

    // For now only support USB-ethernet peripherals consisting of a single
    // configuration, with a single interface, with two bulk endpoints.
    if ((1 != usbdev->descriptor.bNumConfigurations)  ||
        (1 != usbdev->config[0].bNumInterfaces) ||
        (2 != usbdev->config[0].interface[0].altsetting->bNumEndpoints)) {
        return (void*) 0;
    }
    if ((0 == (usbdev->config[0].interface[0].altsetting->endpoint[0].bEndpointAddress & USB_DIR_IN)) &&
        (0 != (usbdev->config[0].interface[0].altsetting->endpoint[1].bEndpointAddress & USB_DIR_IN))) {
        tx_endpoint = usbdev->config[0].interface[0].altsetting->endpoint[0].bEndpointAddress;
        rx_endpoint = usbdev->config[0].interface[0].altsetting->endpoint[1].bEndpointAddress & ~USB_DIR_IN;
    }
    if ((0 != (usbdev->config[0].interface[0].altsetting->endpoint[0].bEndpointAddress & USB_DIR_IN)) &&
        (0 == (usbdev->config[0].interface[0].altsetting->endpoint[1].bEndpointAddress & USB_DIR_IN))) {
        tx_endpoint = usbdev->config[0].interface[0].altsetting->endpoint[1].bEndpointAddress;
        rx_endpoint = usbdev->config[0].interface[0].altsetting->endpoint[0].bEndpointAddress & ~USB_DIR_IN;
    }
    if (-1 == tx_endpoint) {
        return (void*) 0;
    }
           
    res = usb_set_configuration(usbdev, usbdev->config[0].bConfigurationValue);
    if (0 != res) {
        printk("ecos_usbeth: failed to set configuration, %d\n", res);
        return (void*) 0;
    }
    res = usb_control_msg(usbdev,
                          usb_rcvctrlpipe(usbdev, 0),
                          ECOS_USBETH_CONTROL_GET_MAC_ADDRESS,
                          USB_TYPE_CLASS | USB_RECIP_DEVICE | USB_DIR_IN,
                          0,
                          0,
                          (void*) MAC,
                          6,
                          5 * HZ);
    if (6 != res) {
        printk("ecos_usbeth: failed to get MAC address, %d\n", res);
        return (void*) 0;
    }
    
    res = usb_control_msg(usbdev,
                          usb_sndctrlpipe(usbdev, 0),                           // pipe
                          ECOS_USBETH_CONTROL_SET_PROMISCUOUS_MODE,             // request
                          USB_TYPE_CLASS | USB_RECIP_DEVICE,                    // requesttype
                          0,                                                    // value
                          0,                                                    // index
                          (void*) dummy,                                        // data
                          0,                                                    // size
                          5 * HZ);                                              // timeout
    if (0 != res) {
        printk("ecos_usbeth: failed to disable promiscous mode, %d\n", res);
    }
           
    usbeth = (ecos_usbeth*) kmalloc(sizeof(ecos_usbeth), GFP_KERNEL);
    if ((ecos_usbeth*)0 == usbeth) {
        printk("ecos_usbeth: failed to allocate memory for usbeth data structure\n");
        return (void*) 0;
    }
    memset(usbeth, 0, sizeof(ecos_usbeth));
    
    net                 = init_etherdev(0, 0);
    if ((struct net_device*) 0 == net) {
        kfree(usbeth);
        printk("ecos_usbeth: failed to allocate memory for net data structure\n");
        return (void*) 0;
    }

    usbeth->usb_lock    = SPIN_LOCK_UNLOCKED;
    usbeth->usb_dev     = usbdev;
    FILL_BULK_URB(&(usbeth->tx_urb), usbdev, usb_sndbulkpipe(usbdev, tx_endpoint),
                  usbeth->tx_buffer, ECOS_USBETH_MAXTU, &ecos_usbeth_tx_callback, (void*) usbeth);
    FILL_BULK_URB(&(usbeth->rx_urb), usbdev, usb_rcvbulkpipe(usbdev, rx_endpoint),
                  usbeth->rx_buffer, ECOS_USBETH_MAXTU, &ecos_usbeth_rx_callback, (void*) usbeth);
    
    usbeth->net_dev             = net;
    usbeth->target_promiscuous  = 0;
    
    net->priv                   = (void*) usbeth;
    net->open                   = &ecos_usbeth_open;
    net->stop                   = &ecos_usbeth_close;
    net->do_ioctl               = &ecos_usbeth_ioctl;
    net->hard_start_xmit        = &ecos_usbeth_start_tx;
    net->set_multicast_list     = &ecos_usbeth_set_rx_mode;
    net->get_stats              = &ecos_usbeth_netdev_stats;
    net->mtu                    = 1500; // ECOS_USBETH_MAXTU - 2;
    memcpy(net->dev_addr, MAC, 6);
    
    printk("eCos-based USB ethernet peripheral active at %s\n", net->name);
    MOD_INC_USE_COUNT;
    return (void*) usbeth;
}

// disconnect().
// Invoked after probe() has recognized a device but that device
// has gone away. 
static void
ecos_usbeth_disconnect(struct usb_device* usbdev, void* data)
{
    ecos_usbeth* usbeth = (ecos_usbeth*) data;
    if (!usbeth) {
        printk("ecos_usbeth: warning, disconnecting unconnected device\n");
        return;
    }
    if (0 != netif_running(usbeth->net_dev)) {
        ecos_usbeth_close(usbeth->net_dev);
    }
    unregister_netdev(usbeth->net_dev);
    if (-EINPROGRESS == usbeth->rx_urb.status) {
        usb_unlink_urb(&(usbeth->rx_urb));
    }
    if (-EINPROGRESS == usbeth->tx_urb.status) {
        usb_unlink_urb(&(usbeth->tx_urb));
    }
    kfree(usbeth);
    MOD_DEC_USE_COUNT;
}

static struct usb_driver ecos_usbeth_driver = {
    name:       "ecos_usbeth",
    probe:      ecos_usbeth_probe,
    disconnect: ecos_usbeth_disconnect,
};

// init()
// Called when the module is loaded. It just registers the device with
// the generic USB code. Nothing else can really be done until
// the USB code detects that a device has been attached.
int __init
ecos_usbeth_init(void)
{
    printk("eCos USB-ethernet device driver\n");
    return usb_register(&ecos_usbeth_driver);
}

// exit()
// Called when the module is unloaded. disconnect() will be
// invoked if appropriate.
void __exit
ecos_usbeth_exit(void)
{
    usb_deregister(&ecos_usbeth_driver);
}

module_init(ecos_usbeth_init);
module_exit(ecos_usbeth_exit);
