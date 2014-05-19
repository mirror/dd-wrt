/*
 * Atheros AR9130 USB Device Controller Driver
 */

#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/dmapool.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>

#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/unaligned.h>

#define AR9130_USB_DEBUG 1
#define TRIP_WIRE

#include "ar9130_udc.h"

#ifdef CONFIG_USB_AR9130_OTG_MODULE
#define CONFIG_USB_AR9130_OTG
#endif

/*
 * debug level zones can be enabled individually for different level of
 * debugging.  Debug messages can delay the init sequence if the prints are 
 * redirected to serial console, and result in device detection failure.
 * Enable them only if required.
 */
#ifdef AR9130_USB_DEBUG
static int ar9130_debug_level = (
//        AR9130_DEBUG_FUNCTION   |
//        AR9130_DEBUG_INTERRUPT  |
//        AR9130_DEBUG_ENDPOINT   |
//        AR9130_DEBUG_PORTSTATUS |
//        AR9130_DEBUG_DEVICE     |
//        AR9130_DEBUG_MEMORY     |
//        AR9130_DEBUG_QUEUEHEAD  |
//        AR9130_DEBUG_DTD        |
        0x0
        );
static const char *ep_direction[] = {"OUT", "IN"};
#endif

#define DRIVER_DESC                             "Atheros UDC Driver"

/*
 * There are a maximum of 32 endpoints (16 IN and 16 OUT).  But currently only
 * 12 (6 IN and 6 OUT) endpoints are enabled.
 */
#define AR9130_MAX_END_POINTS                   (6)
#define AR9130_MAX_DTDS                         (32)
#define AR9130_USB_REMOTE_WKUP                  (0x02)

#define USB_RECV                                (0)
#define USB_SEND                                (1)

#define AR9130_CTRL_EP                          (1 << 0)
#define DMA_ADDR_INVALID                        ((dma_addr_t)~0)

/*
 * The platform device in AR9130 still reflects the old ar7100 name.  So we 
 * maintain two names here - one for the platform driver and other for local
 * reference
 */
#ifdef CONFIG_MACH_AR7240
static const char driver_name [] = "ar7240-ehci";
#else
static const char driver_name [] = "ar7100-ehci";
#endif

static const char device_name [] = "ar9130_udc";
static struct ar9130_udc *ap_gadget;

struct ar9130_ep {
    struct usb_ep ep;
    char name[15];
    const struct usb_endpoint_descriptor *ep_desc;
    struct list_head queue;
    __u8 bEndpointAddress;
    __u8 bmAttributes;
    __u16 maxpacket;
    struct ep_qhead *ep_qh;
    struct ar9130_udc *udc;
};

struct ar9130_req {
    struct usb_request req;
    struct list_head queue;
    unsigned mapped:1;
};

struct ar9130_udc {
    struct usb_gadget gadget;
    struct usb_gadget_driver *ga_driver;
    struct device *dev;
    struct ar9130_usb __iomem *op_base;
    struct ar9130_ep ep[32];
    struct ar9130_otg *ar9130_otg;
    struct ep_qhead *ep_queue_head;
    struct dma_pool *dtd_pool;
    struct list_head dtd_list;
    struct ep_dtd *dtd_heads[AR9130_MAX_END_POINTS * 2];
    struct usb_request *ctrl_req;
    void   *ctrl_buf;
    void __iomem *reg_base;
    spinlock_t lock;
    __u16 usbState;
    __u16 usbDevState;
    __u8  usbCurrConfig;
    __u8  devAddr;
    __u8  ep0setup;
    dma_addr_t qh_dma;
};

static void start_udc(struct ar9130_udc *udc);
static void stop_udc(struct ar9130_udc *udc);
static void stop_activity(struct ar9130_udc *udc);
static void ar9130_init_device(struct ar9130_udc *udc);
static int  ar9130_setup(struct ar9130_udc *udc);
static int  udc_mem_init (struct ar9130_udc *udc);
static void udc_mem_free (struct ar9130_udc *udc);
static void udc_ep_wipe (struct ar9130_ep *ep, int status);
static void ar9130_free_dtd(struct ar9130_udc *udc,struct ep_dtd *ep_dtd);
static void complete_transfer(struct ar9130_ep *ep, struct ar9130_req *req,
        __u8 epdir, int status);
static void usb_send_data(struct ar9130_udc *udc, __u8 epno, __u8 *buff,
        __u32 size);
static void ar9130_stall_endpoint(struct ar9130_udc *udc, __u8 epno,
        __u8 epdir);
static void ar9130_unstall_endpoint(struct ar9130_udc *udc, __u8 epno,
        __u8 epdir);

#ifdef AR9130_UDC_DEBUG
/*
 * Print Queue Head information.  EP0 information is not printed right now
 * as it interferes in Device attach/detection
 */
static void ar9130_print_qh (struct ep_qhead *ep_qh, __u16 epno, __u16 epdir,
        char *str)
{
    if ((epno >= AR9130_MAX_END_POINTS) && (epno < 1)) {
        return;
    }
    ar9130_debug_qh ("%s Endpoint %d-%s Queue %p\n\tmaxPacketLen %x\n\t"
            "curr_dtd %x\n\tnext_dtd %x\n\tstatus %x\n\tBuff0 %x\n",
            str, epno, ep_direction[epdir], ep_qh,
            le32_to_cpu(ep_qh->maxPacketLen),
            le32_to_cpu(ep_qh->curr_dtd),
            le32_to_cpu(ep_qh->next_dtd),
            le32_to_cpu(ep_qh->size_ioc_int_status),
            le32_to_cpu(ep_qh->buff[0]));
}

/*
 * Print Device Transfer Descriptor information.  EP0 information is not
 * printed right now as it interferes in Device attach/detection.
 */
static void ar9130_print_dtd (struct ep_dtd *ep_dtd, __u16 epno, __u16 epdir,
        char *str)
{
    if ((epno >= AR9130_MAX_END_POINTS) && (epno < 1)) {
        return;
    }
    ar9130_debug_dtd ("%s Endpoint %d-%s Descriptor %p\n\tnextTr %x\n\t"
            "status %x\n\tBuff0 %x\n\tBuff1 %x\n",
            str, epno, ep_direction[epdir], ep_dtd,
            le32_to_cpu(ep_dtd->next_dtd),
            le32_to_cpu(ep_dtd->size_ioc_status),
            le32_to_cpu(ep_dtd->buff[0]),
            le32_to_cpu(ep_dtd->buff[1]));
}
#else
#define ar9130_print_qh(ep_qh,epno,epdir,str)   do { (void)(epno); } while (0)
#define ar9130_print_dtd(ep_dtd,epno,epdir,str) do { (void)(epno); } while (0)
#endif

void dbg_buf(struct usb_request *req)
{
	int i;
	unsigned char *tmp_buf = (unsigned char *)req->buf;
	for(i=0;i<req->length;i++) {
		printk(" %02x ",*(tmp_buf++));
		if((i % 64) == 0){ printk("\n"); }
	}
}

void fill_buf(struct usb_request *req)
{
	int i,j = 0;
	unsigned char *tmp_buf = (unsigned char *)req->buf;
	for(i=0;i<req->length;i++) {
		*tmp_buf++ = j++ ;
		if((j % 256) == 0) { j = 0; }
	}
}

/* Allocate an USB Request - used by gadget drivers */
static struct usb_request * ar9130_alloc_request (struct usb_ep *ep,
        gfp_t gfp_flags)
{
    struct ar9130_req *req;
    ar9130_debug_fn("__enter %s\n",__FUNCTION__);

    req = (struct ar9130_req *)kmalloc(sizeof (struct ar9130_req), GFP_ATOMIC);
    if (req) {
        memset (req, 0, sizeof(struct ar9130_req));
        req->req.dma = DMA_ADDR_INVALID;
        INIT_LIST_HEAD(&req->queue);
    } else {
        return NULL;
    }
    return &req->req;
}

/* Free an USB Request - used by gadget drivers */
static void ar9130_free_request(struct usb_ep *ep, struct usb_request *_req)
{
    struct ar9130_req *req;
    ar9130_debug_fn("__enter %s\n",__FUNCTION__);
    if(_req) {
        req = container_of(_req,struct ar9130_req,req);
        kfree(req);
    }
}

/* Allocate data buffer for use by USB request - used by gadget drivers */
static void * ar9130_alloc_buffer(struct usb_ep *_ep, unsigned bytes,
        dma_addr_t *dma, gfp_t gfp_flags)
{
    struct ar9130_ep *ep = container_of(_ep,struct ar9130_ep,ep);

    ar9130_debug_fn("__enter %s\n",__FUNCTION__);
    /*
     * Small memory allocation wastes memory :
     * dma_coherent go for one page
     */
    return dma_alloc_coherent(ep->udc->gadget.dev.parent,
            bytes,dma,gfp_flags);
}

/* Free data buffer - used by gadget drivers */
static void ar9130_free_buffer(struct usb_ep *_ep, void *buf, dma_addr_t dma,
        unsigned bytes)
{
    struct ar9130_ep * ep = container_of(_ep,struct ar9130_ep,ep);
    ar9130_debug_fn("__enter %s\n",__FUNCTION__);
    dma_free_coherent(ep->udc->dev, bytes, buf, dma);
}

/*
 * Enable an endpoint.
 * The endpoint is configured using the ep descriptor properties.  The selected
 * endpoint already has endpoint types configured properly.  This function
 * primarily enables the endpoint in hardware and sets the max packet size.
 */
static int ar9130_ep_enable(struct usb_ep *_ep,
        const struct usb_endpoint_descriptor *desc)
{
    struct ar9130_ep *ep = container_of(_ep,struct ar9130_ep,ep); 
    __u8 epno,epdir,qh_offset;
    __u32 bits = 0;
    __u16 maxpacket;
    __u32 qh_maxpacket;
    __u32 bit_pos;
    unsigned long flags;
    struct ar9130_udc *udc;
    struct ep_qhead *ep_QHead;

    ar9130_debug_fn("__enter %s\n",__FUNCTION__);

    /* Sanity check between the endpoint and descriptor properties */
    if(!_ep || !desc || (desc->bDescriptorType != USB_DT_ENDPOINT)
            || (ep->bEndpointAddress != desc->bEndpointAddress))
    {
        ar9130_warn("%s, bad ep or descriptor \n",__FUNCTION__);
        return -EINVAL;
    }

    ar9130_debug_ep("ep_enable name:%s, maxpacket:%d, ep_addr:%x\n",
            _ep->name, _ep->maxpacket, ep->bEndpointAddress);
    ar9130_debug_ep("descriptor lenth:%x, type:%x, ep_addr:%x, maxpacket:%x\n",
            desc->bLength ,desc->bDescriptorType, desc->bEndpointAddress,
            desc->wMaxPacketSize);

    /* Get endpoint number and direction */
    epno  = ep->bEndpointAddress & 0x0f;
    epdir = (ep->bEndpointAddress & USB_DIR_IN) ? USB_SEND : USB_RECV;
    bit_pos = (1 << (16 * epdir + epno));

    qh_offset  = (2 * epno) + epdir;
    udc = ep->udc;
    if (ep != &udc->ep[qh_offset]) {
        ar9130_warn("%s, bad ep or descriptor \n",__FUNCTION__);
        return -EINVAL;
    };

    /* Get endpoint Queue Head based on EP number and direction */
    ep_QHead  = (udc->ep_queue_head + qh_offset);

    spin_lock_irqsave(&udc->lock, flags);

    /* Set max packet length for the endpoint in EP queue head */
    ep->ep_desc = desc;
    maxpacket = le16_to_cpu(desc->wMaxPacketSize);
    qh_maxpacket = le32_to_cpu(ep_QHead->maxPacketLen);
    qh_maxpacket = (qh_maxpacket & 0xF800FFFF) | (maxpacket << 16);
    ep_QHead->maxPacketLen = cpu_to_le32(qh_maxpacket);
    _ep->maxpacket = ep->maxpacket = maxpacket;
#if 0
	if(epno > 0)
	//	printk("ep->mps %d ep_Qhead %d \n",ep->maxpacket,ep_QHead->maxPacketLen);
	printk("ep_QHead %x \n",ep_QHead);
#endif

    /* Enable endpoint in Hardware */
    bits = epdir  ? (AR9130_EPCTRL_TX_ENABLE) : (AR9130_EPCTRL_RX_ENABLE);
    writel((readl(&udc->op_base->ep_ctrlx[epno]) | bits ),
            &udc->op_base->ep_ctrlx[epno]);
#if 1
    /* Flush the endpoint and make sure that the endpoint status is zero */
    writel(bit_pos,&udc->op_base->ep_flush);
    /* Wait till flush completes */
    while(readl(&udc->op_base->ep_flush) & bit_pos);

    while(readl(&udc->op_base->ep_status) & bit_pos){
        writel(bit_pos,&udc->op_base->ep_flush);
        while(readl(&udc->op_base->ep_flush) & bit_pos);
    }

#endif
    ar9130_debug_ep("ep_enable ==> ep%d-%s queue:%d, name:%s, maxlen:%x, "
            "ctrl:%x\n", epno, ep_direction[epdir], qh_offset, ep->name,
            qh_maxpacket, readl(&udc->op_base->ep_ctrlx[epno]));

    spin_unlock_irqrestore(&udc->lock, flags);
    return 0;
}

/*
 * Disable an endpoint
 * If there are any pending requests on the endpoint, shut them and inform the
 * gadget driver about it.  Disable the endpoint in the hardware.
 */
static int ar9130_ep_disable(struct usb_ep *_ep)
{
    struct ar9130_udc *udc;
    struct ar9130_ep *ep;
    unsigned long flags;
    __u8 epno,epdir;
    __u32 bits;

    ep = container_of(_ep,struct ar9130_ep,ep);
    if(!_ep || !ep->ep_desc) {
        return -EINVAL;
    }

    spin_lock_irqsave(&ap_gadget->lock,flags);

    /* Cancel all current and pending requests for this endpoint */
    ep->ep_desc = NULL;
    udc_ep_wipe(ep,-ESHUTDOWN);
    ep->ep.maxpacket = ep->maxpacket;
    /* Get endpoint number and direction */
    epno  = ep->bEndpointAddress & 0x0f;
    epdir = (ep->bEndpointAddress & USB_DIR_IN) ? USB_SEND : USB_RECV;

    /* Disable the endpoint in hardware */
    bits = epdir  ? (AR9130_EPCTRL_TX_ENABLE) : (AR9130_EPCTRL_RX_ENABLE);
    udc = ep->udc;
    writel((readl(&udc->op_base->ep_ctrlx[epno]) & ~bits),
            &udc->op_base->ep_ctrlx[epno]);

    spin_unlock_irqrestore(&ap_gadget->lock,flags);

    return 0;
}

/* for a selected endpoint cancel all pending requests and inform the upper
 * layer about cancellation */
static void udc_ep_wipe (struct ar9130_ep *ep, int status)
{
    struct ar9130_req *req;
    struct ar9130_udc *udc;
    struct ep_dtd *ep_dtd;
    udc = ep->udc;
    while(!list_empty(&ep->queue)) {
        __u8 epdir = (ep->bEndpointAddress & USB_DIR_IN) ? USB_SEND : USB_RECV;
        __u8 epno  = ep->bEndpointAddress & 0x0f;
        if ((ep_dtd = udc->dtd_heads[(epno * 2) + epdir]) != NULL) {
            udc->dtd_heads[(epno * 2) + epdir] = NULL;
#if 0
			if(epno > 0)
				printk("wp_ep %x \n",ep_dtd);
#endif
            ar9130_free_dtd(udc, ep_dtd);
        }
        req = list_entry(ep->queue.next, struct ar9130_req, queue);
        complete_transfer(ep, req, epdir, status);
    }
}

/*
 * Start an IN or OUT transaction based on usb request and endpoint.
 */
static int ar9130_start_trans(struct ar9130_udc *udc,struct ar9130_ep *ep,
        struct ar9130_req *req)
{
    struct ep_qhead *ep_QHead;
    struct ep_dtd *ep_dtd = NULL;
    struct list_head *temp;
    unsigned long flags;
    __u32 catalyst,bit_pos =0,temp_pos = 0;
    __u8 epno,epdir,transFlag;

    ar9130_debug_fn("__enter %s\n",__FUNCTION__);

    /* Get endpoint number and direction */
    epno  = ep->bEndpointAddress & 0x0f;
    epdir = (ep->bEndpointAddress & USB_DIR_IN) ? USB_SEND : USB_RECV;

    catalyst = ((2 * epno) + epdir);
    bit_pos = (1 << ((16 * epdir) + epno));

    spin_lock_irqsave(&udc->lock,flags);
    
    /* Get endpoint queue head based on EP number and direction */
    ep_QHead =udc->ep_queue_head + catalyst;
	
    /* Get a free device transfer descriptor from the pre-allocated dtd list */
    if(!list_empty(&udc->dtd_list)) {
        temp = udc->dtd_list.next;
        ep_dtd = list_entry(temp,struct ep_dtd,tr_list);
		list_del(temp);
		memset(ep_dtd,0,28); /* only Hardware access fields */	
	ar9130_debug_fn("%x-%s",epno,epdir ? "IN":"OUT");
#if 0
	if(epno > 0)
			printk("%x-%s -> len %d \n",epno,epdir ? "IN":"OUT",req->req.length);
	else
		printk("%x-%s \n",epno,epdir ? "IN":"OUT");

	if(epno == 1 && epdir == 0){	
	//	printk("*");
	}
#endif
    } else {
        ar9130_warn("__err  - dtd List Empty %s\n",__FUNCTION__);
        /* ep_dtd == NULL TODO: allocate a new dtd */
        spin_unlock_irqrestore(&udc->lock,flags);
        return -ENOMEM;
    }

    /* Store the allocated DTD for feature reference */
    udc->dtd_heads[catalyst] = ep_dtd;
    if(catalyst < 0) {
		printk("Warning... wrong dtd head position \n");
    }
    spin_unlock_irqrestore(&udc->lock,flags);

    /* Initialize dtd */
    ep_dtd->next_dtd = __constant_cpu_to_le32(AR9130_TD_NEXT_TERMINATE);
    ep_dtd->size_ioc_status = cpu_to_le32 (
            ((req->req.length)<< AR9130_TD_LENGTH_BIT_POS) | AR9130_TD_IOC |
            (AR9130_TD_STATUS_ACTIVE));

    /* Set Reserved Field to 0 */
    ep_dtd->size_ioc_status &= cpu_to_le32(~AR9130_TD_RESERVED_FIELDS);

#if 0
 if(epno > 0)  
//	printk("ep_dtd->size_ioc_status %x \n",ep_dtd->size_ioc_status); 
//	printk("epno %d : epdir %s \n",epno,epdir > 0 ? "IN":"OUT");
	printk("ep_dtd %x loc %d req len %x ep_QHead %x \n",ep_dtd,catalyst,ep_dtd->size_ioc_status,
			ep_QHead);
#endif

	/* Debug Zero Device */
	if((epno > 0) && (epdir > 0)) {
		fill_buf(&req->req); 
	/*	dbg_buf(&req->req); */
	}
	/* Debug Zero Device */

    /* We can transfer a maximum of 20K using a single dtd.  Fill in the dtd
     * transfer buffers accordingly */
    if (req->req.length) {
        ep_dtd->buff[0] = cpu_to_le32((__u32)(req->req.dma));
        ep_dtd->buff[1] = cpu_to_le32(((__u32)(req->req.dma)) + 4096);
        ep_dtd->buff[2] = cpu_to_le32(((__u32)(req->req.dma)) + (4096 * 2));
        ep_dtd->buff[3] = cpu_to_le32(((__u32)(req->req.dma)) + (4096 * 3));
        ep_dtd->buff[4] = cpu_to_le32(((__u32)(req->req.dma)) + (4096 * 4));
    } else {
        ep_dtd->buff[0] = 0;
        ep_dtd->buff[1] = ep_dtd->buff[2] = ep_dtd->buff[3] =
            ep_dtd->buff[4] = cpu_to_le32(4096);
    }

    /*
     * If the endpoint is already primed we have nothing to do here; just
     * return; TODO - attach the current dtd to the dtd list in the queue
     * head if the endpoint is already primed.
     */
    if(readl(&udc->op_base->ep_prime) & bit_pos) {
        ar9130_warn("Endpoint already being primed %x\n", bit_pos);
        goto complete;
    }

    /* Use hardware tripwire semaphore mechanism before priming the endpoint */
#ifdef TRIP_WIRE
    transFlag = 0;
    while(!transFlag) {
        writel(readl(&udc->op_base->usbcmd) |AR9130_CMD_ATDTW_TRIPWIRE_SET,
                &udc->op_base->usbcmd);
        temp_pos = readl(&udc->op_base->ep_status) & bit_pos;

        if(readl(&udc->op_base->usbcmd) & AR9130_CMD_ATDTW_TRIPWIRE_SET) {
            transFlag = 1;
        }
    }

    writel(readl(&udc->op_base->usbcmd) & AR9130_CMD_ATDTW_TRIPWIRE_CLEAR,
            &udc->op_base->usbcmd);
#endif
    if(!temp_pos) {
        /* Initialize queue head and prime the endpoint */
        ep_QHead->next_dtd = cpu_to_le32(ep_dtd->dtd_dma);
        ep_QHead->size_ioc_int_status = __constant_cpu_to_le32(0);

        ar9130_print_qh (ep_QHead, catalyst/2, (catalyst % 2), "trans");
        ar9130_print_dtd (ep_dtd, catalyst/2, (catalyst % 2), "trans");

	        /* Prime the EndPoint */
        writel(bit_pos,&udc->op_base->ep_prime);
    } else {
			printk("Err -> temp_pos %d:%s \n",epno,epdir ? "IN":"OUT");
    }

complete:
    return 0;
}

/*
 * Endpoint queue.
 * Queue a request to the endpoint and transer it immediately if possible
 */
static int ar9130_ep_queue(struct usb_ep *_ep,struct usb_request *_req,
        gfp_t gfp_flags)
{
    struct ar9130_req *req;
    struct ar9130_ep *ep;
    struct ar9130_udc *udc;
	unsigned long flags;
    __u8 empty;

    ar9130_debug_fn("__enter %s\n",__FUNCTION__);

    ar9130_debug_ep("_ep->name :%s, _ep->maxpacket :%d\n", _ep->name,
            _ep->maxpacket);

    /* Sanity checks */
    req = container_of(_req,struct ar9130_req,req);
    if (!_req || !req->req.buf || !list_empty(&req->queue)) {
        ar9130_error("%s, Invalid Params %p %d, %d\n", __FUNCTION__, _req->buf,
                _req->length, list_empty(&req->queue));
        return -EINVAL;
    }

    ep = container_of(_ep,struct ar9130_ep,ep);
    if (!_ep || (!ep->ep_desc && (ep->bEndpointAddress & 0x0F))) {
        ar9130_error("%s, Invalid Endpoint %p %x\n", __FUNCTION__, ep->ep_desc,
                ep->bEndpointAddress);
        return -EINVAL;
    }

    udc = ep->udc;
    if (!udc->ga_driver || udc->gadget.speed == USB_SPEED_UNKNOWN) {
        ar9130_error("%s, Driver Error \n", __FUNCTION__);
        return -ESHUTDOWN;
    }
    /* If the usb request contains data transfer, then synchronize the buffer
     * for DMA Transfer */
    if (_req->length) {
        /* DMA for All Trans */
        if(_req->dma == DMA_ADDR_INVALID){
            _req->dma = dma_map_single(
                    ep->udc->gadget.dev.parent,
                    _req->buf,_req->length,
                    (ep->bEndpointAddress & USB_DIR_IN) ?
                    DMA_TO_DEVICE:DMA_FROM_DEVICE);
            req->mapped = 1;
        }else{
            dma_sync_single_for_device(ep->udc->gadget.dev.parent,
                    _req->dma,_req->length,
                    (ep->bEndpointAddress & USB_DIR_IN) ?
                    DMA_TO_DEVICE:DMA_FROM_DEVICE);
            req->mapped = 0;
        }
    } else {
        _req->dma = DMA_ADDR_INVALID;
    }

    _req->status = -EINPROGRESS;
    _req->actual = 0;

    /*
     * Add the request to Endpoint queue.  If there are no transfers happening
     * right now, start the current transfer
     */

    spin_lock_irqsave (&ap_gadget->lock, flags);
    empty = list_empty(&ep->queue);
    list_add_tail(&req->queue,&ep->queue);
    spin_unlock_irqrestore (&ap_gadget->lock, flags);

    if(empty) {
#if 0
		if((ep->bEndpointAddress & 0x0f) > 0)
			printk("S ");
#endif
        ar9130_start_trans(udc,ep,req);
    }

    return 0;
}

static int ar9130_ep_dequeue(struct usb_ep *_ep,struct usb_request * _req)
{
    struct ar9130_ep * ep = container_of(_ep,struct ar9130_ep,ep);
    struct ar9130_req *req;
    unsigned long flags;
    __u8 epdir;

    if(!_ep || !_req) {
        return -EINVAL;
    }

    ar9130_debug_fn("__enter %s\n",__FUNCTION__);
    ar9130_debug_ep("_ep->name :%s, _ep->maxpacket :%d\n",
            _ep->name, _ep->maxpacket);

    spin_lock_irqsave (&ap_gadget->lock, flags);
    /* make sure it's actually queued on this endpoint */
    list_for_each_entry (req, &ep->queue, queue) {
        if (&req->req == _req)
            break;
    }
    if (&req->req != _req) {
        spin_unlock_irqrestore (&ap_gadget->lock, flags);
        return -EINVAL;
    }

    epdir = (ep->bEndpointAddress & USB_DIR_IN) ? USB_SEND : USB_RECV;
    if(ep->queue.next == &req->queue) {
        complete_transfer(ep, req, epdir, -ECONNRESET);
    }
    spin_unlock_irqrestore (&ap_gadget->lock, flags);
    return 0;
}

static void complete_transfer(struct ar9130_ep *ep, struct ar9130_req *req,
        __u8 epdir, int status)
{
    ar9130_debug_fn("__enter %s\n",__FUNCTION__);

    list_del_init(&req->queue);
#if 0
	if((ep->bEndpointAddress & 0x0f))
		printk("ep_no :%d \n",(ep->bEndpointAddress & 0x0f));
#endif

    if(req->req.status == -EINPROGRESS) {
        req->req.status = status;
    } else {
        status = req->req.status;
    }

    if (req->req.length) {
        if (req->mapped) {
            dma_unmap_single(ep->udc->gadget.dev.parent,
                    req->req.dma, req->req.length,
                    (ep->bEndpointAddress & USB_DIR_IN)
                    ? DMA_TO_DEVICE
                    : DMA_FROM_DEVICE);
            req->req.dma = DMA_ADDR_INVALID;
            req->mapped = 0;
        } else {
            dma_sync_single_for_cpu(ep->udc->gadget.dev.parent,
                    req->req.dma, req->req.length,
                    (ep->bEndpointAddress & USB_DIR_IN)
                    ? DMA_TO_DEVICE
                    : DMA_FROM_DEVICE);
        }
    }

    spin_unlock(&ep->udc->lock);
    if (req->req.complete) {
        req->req.complete(&ep->ep,&req->req);
    }
    spin_lock(&ep->udc->lock);
}

static int ar9130_udc_get_frame(struct usb_gadget *_gadget)
{
    ar9130_debug_fn("__enter %s\n",__FUNCTION__);
    return 0;
}

static int ar9130_udc_wakeup(struct usb_gadget * _gadget)
{
    struct ar9130_udc *udc = ap_gadget;
    ar9130_debug_fn("__enter %s\n",__FUNCTION__);

#ifdef CONFIG_USB_AR9130_OTG
	if(readl(&udc->op_base->portscx[0]) &  AR9130_PORTSCX_PORT_SUSPEND) {
		/*TODO: Do Remote Wake up */								
	}else{
#if 0
		if(udc->transceiver){
			otg_start_srp(udc->transceiver);
		}	
#endif
	}
#endif
    return 0;
}

static int ar9130_udc_pullup(struct usb_gadget * _gadget,int is_active)
{
    struct ar9130_udc *udc = ap_gadget;
    ar9130_debug_fn("__enter %s\n",__FUNCTION__);
    if (is_active) {
        start_udc(udc);
    } else {
        stop_udc(udc);
        stop_activity(udc);
    }
    return 0;
}

static int ar9130_udc_vbus_session(struct usb_gadget * _gadget,int is_active)
{
    struct ar9130_udc *udc = ap_gadget;
    ar9130_debug_fn("__enter %s\n",__FUNCTION__);
    if (is_active) {
        start_udc(udc);
    } else {
	    printk ("VBUS Reset\n");
	    stop_udc(udc);
	    stop_activity(udc);
    }
    return 0;
}

static int ar9130_ep_set_halt(struct usb_ep *_ep,int value)
{
    ar9130_debug_fn("__enter %s\n",__FUNCTION__);
    return 0;
}

static const struct usb_gadget_ops ar9130_udc_ops = {
    .get_frame      = ar9130_udc_get_frame,
    .wakeup         = ar9130_udc_wakeup,
    .vbus_session   = ar9130_udc_vbus_session,
    .pullup         = ar9130_udc_pullup,
};

static struct usb_ep_ops ar9130_ep_ops = {
    .enable         = ar9130_ep_enable,
    .disable        = ar9130_ep_disable,
    .alloc_request  = ar9130_alloc_request,
    .free_request   = ar9130_free_request,
//    .alloc_buffer   = ar9130_alloc_buffer,
//    .free_buffer    = ar9130_free_buffer,
    .queue          = ar9130_ep_queue,
    .dequeue        = ar9130_ep_dequeue,
    .set_halt       = ar9130_ep_set_halt,
};

/* Used by EP0 status phase */
static void usb_send_data(struct ar9130_udc *udc, __u8 epno, __u8 *buff,
        __u32 size)
{
    struct usb_request *_req;
    unsigned long flags;

    ar9130_debug_fn("__enter %s\n",__FUNCTION__);
    _req = udc->ctrl_req;
    _req->zero = 0;
    if (size) {
        memcpy(_req->buf, buff, size);
    }
    _req->length = size;
    if (ar9130_ep_queue(&udc->ep[1].ep, _req, GFP_ATOMIC) < 0) {
        ar9130_error ("send setup phase failed\n");
        spin_lock_irqsave(&udc->lock,flags);
        udc_ep_wipe(&udc->ep[1],-ESHUTDOWN);
        spin_unlock_irqrestore(&udc->lock,flags);
    }
}

/* Used by EP0 status phase */
static void usb_recv_data(struct ar9130_udc *udc, __u8 epno, __u8 *buff,
        __u32 size)
{
    ar9130_debug_fn("__enter %s\n",__FUNCTION__);
    struct usb_request *_req;
    unsigned long flags;

    _req = udc->ctrl_req;
    _req->zero = 0;
    _req->length = size;
    if (ar9130_ep_queue(&udc->ep[0].ep, _req, GFP_ATOMIC) < 0) {
        ar9130_error ("recv setup phase failed\n");
        spin_lock_irqsave(&udc->lock,flags);
        udc_ep_wipe(&udc->ep[0],-ESHUTDOWN);
        spin_unlock_irqrestore(&udc->lock,flags);
    }
}

void read_setup_data(struct ar9130_udc *udc,__u8 *dest,int noBytes)
{
    struct ep_qhead *qHead;
    qHead = udc->ep_queue_head;
    int cal, read_safe ;

    ar9130_debug_fn("__enter %s\n",__FUNCTION__);

    /* if semaphore mechanism is used the following code is compiled in */
    read_safe = 0;                      
    while(!read_safe) {
        /* CI 8.4.3.1.2 step 2 */
        writel((readl(&udc->op_base->usbcmd) | AR9130_CMD_SETUP_TRIPWIRE_SET),
                &udc->op_base->usbcmd);
        /* CI 8.4.3.1.2 step 3 */
        for(cal=0;cal < noBytes;cal++) {
            *(dest+cal) = qHead->setup_buff[cal];
        }
        /* CI 8.4.3.1.2 step 4 */
        if(readl(&udc->op_base->usbcmd) & AR9130_CMD_SETUP_TRIPWIRE_SET) {
            read_safe = 1; /* we can proceed exiting out of loop*/
        }
    }

    /* CI 8.4.3.1.2 step 5 */
    writel((readl(&udc->op_base->usbcmd) & AR9130_CMD_SETUP_TRIPWIRE_CLEAR),
            &udc->op_base->usbcmd);
}

static void ar9130_stall_endpoint(struct ar9130_udc *udc,__u8 epno,__u8 epdir)
{
    struct ep_qhead *qHead;
    qHead = udc->ep_queue_head + (2 * epno) + epdir;

    ar9130_debug_fn("__enter %s\n",__FUNCTION__);
    if(epno > 0)
	    printk("__enter %s\n",__FUNCTION__);

    if(le32_to_cpu(qHead->maxPacketLen) & AR9130_EP_QUEUE_HEAD_IOS) {
        writel(readl(&udc->op_base->ep_ctrlx[epno]) |
                (AR9130_EPCTRL_TX_EP_STALL|AR9130_EPCTRL_RX_EP_STALL),
                &udc->op_base->ep_ctrlx[epno]);
    } else {
        writel(readl(&udc->op_base->ep_ctrlx[epno]) |
                (epdir ? AR9130_EPCTRL_TX_EP_STALL : AR9130_EPCTRL_RX_EP_STALL),
                &udc->op_base->ep_ctrlx[epno]);
    }
}

static void ar9130_unstall_endpoint(struct ar9130_udc *udc,__u8 epno,__u8 epdir)
{
    /* Enable the endpoint for Rx or Tx and set the endpoint type */
    writel(readl(&udc->op_base->ep_ctrlx[epno]) & 
            (epdir ? ~AR9130_EPCTRL_TX_EP_STALL : ~AR9130_EPCTRL_RX_EP_STALL),
            &udc->op_base->ep_ctrlx[epno]);
}

static int getStatus(struct ar9130_udc *udc, struct usb_ctrlrequest *ctrl)
{
    __u16 status;
    __u8 intStatus,ep;

    ar9130_debug_fn("__enter %s\n",__FUNCTION__);
    switch(ctrl->bRequestType ) {
        case (USB_DIR_IN | USB_RECIP_DEVICE):
			printk("USB_RECIP_DEVICE\n");
            status = udc->usbDevState;
            usb_send_data(udc,0,(__u8 *)&status,sizeof(status));
            break;

        case (USB_DIR_IN | USB_RECIP_INTERFACE):
		printk("USB_RECIP_INTERFACE\n");
		return 1;
#if 0 
             intStatus = USB_IF_ALT[ctrl->wIndex & 0x00ff];
             usb_send_data(udc,0,(__u8 *)&intStatus,sizeof(intStatus));
#endif
            break;

        case (USB_DIR_IN | USB_RECIP_ENDPOINT):
		printk("USB_RECIP_ENDPOINT\n");
            ep = le16_to_cpu(ctrl->wIndex) & 0x8f;
            /* ep&0x0f ->ep_num;ep&0x80->ep_dir*/
            intStatus = readl(&udc->op_base->ep_ctrlx[ep & 0x0f]);
            status = intStatus & ((ep & 0x80) ? 1 : 0);
            usb_send_data(udc,0,(__u8 *)&status,sizeof(status));
            break;

        default:
		return 1;
#if 0
	    ar9130_stall_endpoint(udc,0,0);
#endif
    }
	return 0;
}

static void setFeature(struct ar9130_udc *udc, struct usb_ctrlrequest *ctrl)
{
    __u16 status;
    __u8 intStatus,ep;

    ar9130_debug_fn("__enter %s\n",__FUNCTION__);
    switch(ctrl->bRequestType) {
        case USB_RECIP_DEVICE:
            switch(le16_to_cpu(ctrl->wValue)) {
                case USB_DEVICE_REMOTE_WAKEUP:
                    /* Set Remote Wakeup */
                    status = udc->usbDevState;
                    status |= AR9130_USB_REMOTE_WKUP;
                    udc->usbDevState = status;
                    break;
				case USB_DEVICE_B_HNP_ENABLE:
						udc->gadget.b_hnp_enable = 1;
						break;
				case USB_DEVICE_A_HNP_SUPPORT:
						udc->gadget.a_hnp_support = 1;
						break;
				case USB_DEVICE_A_ALT_HNP_SUPPORT:
						udc->gadget.a_alt_hnp_support = 1;
						break;
            }
            break;
        case USB_RECIP_ENDPOINT:
            if(le16_to_cpu(ctrl->wValue) != 0) {
                ar9130_stall_endpoint(udc,0,0);
            }
            ep= le16_to_cpu(ctrl->wIndex) & 0x8F;
            /* Get Endpoint Status */
            intStatus = readl(&udc->op_base->ep_ctrlx[ep & 0x0f]);
            status = intStatus & ((ep & 0x80) ? 1 : 0);
            /* Set Endpoint Status ; set stall */
            writel(readl(&udc->op_base->ep_ctrlx[ep&0x0f])|
                    (AR9130_EPCTRL_TX_EP_STALL|AR9130_EPCTRL_RX_EP_STALL),
                    &udc->op_base->ep_ctrlx[ep&0x0f]);
            break;
        default:
	    printk("Stall Endpoints \n");
            ar9130_stall_endpoint(udc,0,0);
    }
    usb_send_data(udc, 0, 0, 0);
}

static void clrFeature(struct ar9130_udc *udc, struct usb_ctrlrequest *ctrl)
{
    __u16 status;
    __u8 intStatus,ep;

    ar9130_debug_fn("__enter %s\n",__FUNCTION__);
    if((udc->usbState != USB_STATE_CONFIGURED) &&
            (udc->usbState != USB_STATE_ADDRESS))
    {
        ar9130_stall_endpoint(udc,0,0);
        return;
    }

    switch(ctrl->bRequestType) {
        case USB_RECIP_DEVICE:
            if(le16_to_cpu(ctrl->wValue) == 1) {
                /* clear remove wakeup */
                status = udc->usbDevState;
                status &= ~USB_DEVICE_REMOTE_WAKEUP;
                udc->usbDevState = status;
            } else {
                ar9130_stall_endpoint(udc,0,0);
                return;
            }
            break;
        case USB_RECIP_ENDPOINT:
            if(le16_to_cpu(ctrl->wValue) != 0) {
                ar9130_stall_endpoint(udc,0,0);
            }
            ep= le16_to_cpu(ctrl->wIndex) & 0x8F;
            /* Get Endpoint Status */
            intStatus = readl(&udc->op_base->ep_ctrlx[ep & 0x0f]);
            status = intStatus & ((ep & 0x80) ? 1 : 0);
            /* Set Endpoint Status ; unstall */
            writel(readl(&udc->op_base->ep_ctrlx[ep&0x0f]) &
                    ~(AR9130_EPCTRL_TX_EP_STALL|AR9130_EPCTRL_RX_EP_STALL),
                    &udc->op_base->ep_ctrlx[ep&0x0f]);
            break;
        default:
            ar9130_stall_endpoint(udc,0,0);
    }
    usb_send_data(udc, 0, 0, 0);
}

static void setConfiguration(struct ar9130_udc *udc,
        struct usb_ctrlrequest *ctrl)
{
    __u16 usbStatus, wValue;

    ar9130_debug_fn("__enter %s\n",__FUNCTION__);

    wValue = le16_to_cpu(ctrl->wValue);

    if(le16_to_cpu(ctrl->wLength) != 0) {
        ar9130_stall_endpoint(udc,0,0);
        goto end;
    }
    if((wValue & 0x00ff) == 0) {
        usbStatus = udc->usbState;
        if((usbStatus == USB_STATE_CONFIGURED) ||
                (usbStatus == USB_STATE_ADDRESS))
        {
            /* Clear current config values */
            udc->usbCurrConfig = 0;
            udc->usbState = USB_STATE_ADDRESS;
        }else{
            ar9130_stall_endpoint(udc,0,0);
        }
        goto end;
    }
    /* Read the current Conf Value */
    usbStatus = udc->usbCurrConfig;
    if(usbStatus != (wValue & 0x00ff)) {
        udc->usbCurrConfig = (wValue & 0x00ff);
        udc->usbState = USB_STATE_CONFIGURED;
        goto end;
    }
    udc->usbState = USB_STATE_CONFIGURED;
end:
    return;
}

/* Hardware assisted SET_ADDRESS */
static void setAddress(struct ar9130_udc *udc, __u16 addr)
{
    ar9130_debug_fn("__enter %s Address %d \n",__FUNCTION__,addr);

    udc->devAddr = addr;
    writel((((__u32)udc->devAddr) <<  AR9130_ADDRESS_BIT_SHIFT  |
                (0x01 << (AR9130_DEVADDR_USBADRA))),&udc->op_base->devaddr);
    udc->usbState = USB_STATE_ADDRESS;
}

static void handle_ep0_setup(struct ar9130_udc *udc)
{
    struct usb_ctrlrequest ctrl;
    int status = 0;

    ar9130_debug_fn("__enter %s\n",__FUNCTION__);

    /* Clear the bit in ep_setup_stat 8.4.3.1.2 step 1 */
    writel(AR9130_CTRL_EP,&udc->op_base->ep_setup_stat);

    read_setup_data(udc,(__u8 *)&ctrl,8);

    while (readl(&udc->op_base->ep_setup_stat) & AR9130_CTRL_EP) {
   //     udelay(1);
    }
    if (ctrl.bRequestType & USB_DIR_IN) {
        /* setup phase */
        udc->ep0setup = 1;
    } else {
        udc->ep0setup = 0;
    }
    switch(ctrl.bRequest){
        case USB_REQ_GET_STATUS:
   //         printk ("Get Status \n");
            if(getStatus(udc,&ctrl)) {
		goto cliOper; 	
	    }
            break;
        case USB_REQ_SET_FEATURE:
    //        printk ("Set Feature\n");
            setFeature(udc,&ctrl);
            break;
        case USB_REQ_CLEAR_FEATURE:
     //       printk ("Clear Feature\n");
            clrFeature(udc,&ctrl);
            break;
        case USB_REQ_SET_CONFIGURATION:
      //      printk ("Set Configuration \n");
            setConfiguration(udc,&ctrl);
            goto cliOper;
        case USB_REQ_SET_ADDRESS:
       //     printk ("Set Address \n");
            setAddress(udc,le16_to_cpu(ctrl.wValue));
            usb_send_data(udc,0,0,0);
            break;
        default:
            /*Hope Rest all Requests are Handled by CLIENT */
cliOper:
            ar9130_debug_dev("SETUP %02x.%02x v%04x i%04x l%04x\n",
                    ctrl.bRequestType, ctrl.bRequest,
                    le16_to_cpu(ctrl.wValue),le16_to_cpu(ctrl.wIndex),
                    le16_to_cpu(ctrl.wLength));

            status = udc->ga_driver->setup(&udc->gadget,&ctrl);
    }

    if(status < 0) {
        ar9130_error ("error %d, stalling endpoint\n", status);
        ar9130_stall_endpoint(udc,0,0);
    }
}

static void ar9130_free_dtd(struct ar9130_udc *udc,struct ep_dtd *ep_dtd)
{
    list_add_tail(&ep_dtd->tr_list,&udc->dtd_list);
}

static void ar9130_retire_dtd(struct ar9130_udc *udc,__u8 epno,__u8 epdir)
{
    __u32 bit_pos,tmp;
    unsigned long flags;
    struct ar9130_ep *ep;
    struct ar9130_req *req;
    struct ep_qhead *ep_QHead;
    struct ep_dtd  *ep_dtd;

    bit_pos = (1 << (16 * epdir + epno));
    tmp = (2*epno + epdir);
    ep = &udc->ep[tmp];

    ep_QHead =udc->ep_queue_head + tmp;
    ep_dtd = udc->dtd_heads[tmp];

    if(ep_dtd){
        __u32 size_ioc_status;
	
	if(epno >0)
		if(le32_to_cpu(ep_dtd->size_ioc_status) & AR9130_TD_STATUS_ACTIVE){
			printk("A\n");
		}

#if 0
        if(le32_to_cpu(ep_dtd->size_ioc_status) & AR9130_TD_STATUS_ACTIVE){
            /* Write 1 to Flush Reg */
            writel(bit_pos,&udc->op_base->ep_flush);
            /* Wait till flush completes */
            while(readl(&udc->op_base->ep_flush) & bit_pos);
            while(readl(&udc->op_base->ep_status) & bit_pos) {
                writel(bit_pos,&udc->op_base->ep_flush);
                while(readl(&udc->op_base->ep_flush) & bit_pos);
            }
        }
#endif
        size_ioc_status = le32_to_cpu(ep_dtd->size_ioc_status);
#if 0
	printk("size_ioc_status %x epno %d\n",size_ioc_status,epno);
#endif
		if (epno > 0)
		{
//	printk("RET epno %d : epdir %s \n",epno,epdir > 0 ? "IN":"OUT");
//	printk("ep_dtd_done %x loc %d \n",ep_dtd,tmp);
		}
   //     ep_dtd->size_ioc_status = __constant_cpu_to_le32(0);
        ep_dtd->next_dtd = __constant_cpu_to_le32(AR9130_TD_NEXT_TERMINATE);
        ep_QHead->next_dtd = __constant_cpu_to_le32(
                AR9130_EP_QUEUE_HEAD_NEXT_TERMINATE);
        ep_QHead->size_ioc_int_status = __constant_cpu_to_le32(0);

        spin_lock_irqsave(&udc->lock,flags);
        ar9130_free_dtd(udc,ep_dtd);

        udc->dtd_heads[tmp] = NULL;
        if(!list_empty(&ep->queue)){
            req = container_of(ep->queue.next,struct ar9130_req,queue);
            req->req.actual = req->req.length -
                ((size_ioc_status >> AR9130_TD_LENGTH_BIT_POS) & 0x7FFF);
#if 0
			if(epno > 0)
				printk("req->actual %d \n",((size_ioc_status >> AR9130_TD_LENGTH_BIT_POS) & 0x7FFF));
#endif
            complete_transfer(ep,req,epdir,0);
        }
        spin_unlock_irqrestore(&udc->lock,flags);
    }
	else {
		printk("Null ep_dtd Err \n");
	}
    if (udc->ep0setup) {
        udc->ep0setup = 0;
        if (epno == 0) {
            usb_recv_data(udc, 0, 0, 0);
        }
    } else if(!list_empty(&ep->queue)){
        req = container_of(ep->queue.next,struct ar9130_req,queue);
#if 0
		if(epno > 0)
			printk("R ");
#endif
		if(udc->dtd_heads[tmp] == NULL)
			ar9130_start_trans(udc,ep,req);
    }
}

/* Endpoint Transfer Complete interrupt handling */
static void ar9130_process_USB_Intr(struct ar9130_udc *udc)
{
    struct ep_qhead *ep_QHead;
    struct ep_dtd  *ep_dtd;
    __u8 epno = 0, epdir = 0;
    __u32 setup_stat=0,bit_pos =0,tmp,err;
    __u8 epDetect ;
    int i;

    /* EP0 Setup transfer complete */
    setup_stat = readl(&udc->op_base->ep_setup_stat);
    if(setup_stat & AR9130_CTRL_EP){
        handle_ep0_setup(udc);
    }

    bit_pos = readl(&udc->op_base->ep_complete);

    /* Clear the bit in Registers */
    writel(bit_pos,&udc->op_base->ep_complete);

    if(bit_pos){
        for (i=0;i<AR9130_MAX_END_POINTS;i++) {
            /* Based on the bit position get EP number and direction */
            epDetect = 0;
            if (bit_pos & (1 << i)){
                epno = i;
                epdir = 0;
                epDetect = 1;
            } else if(bit_pos & (1 << (i + 16))){
                epno = i;
                epdir = 1;
                epDetect = 1;
            }

            if(epDetect) {
                unsigned long flags;

                /* Based on EP number and direction Get Queue head and dtd */
                spin_lock_irqsave(&udc->lock,flags);
                tmp = ((2*epno) + epdir);
                ep_dtd = udc->dtd_heads[tmp];
	
                ep_QHead = udc->ep_queue_head + tmp;
                spin_unlock_irqrestore(&udc->lock,flags);

                if (ep_dtd) {
                    err = (le32_to_cpu(ep_dtd->size_ioc_status) &
                            AR9130_TD_ERROR_MASK);
                    if(err & AR9130_TD_STATUS_HALTED){
						printk("Descp Halted \n");
                        ep_QHead->size_ioc_int_status &= cpu_to_le32(~err);
                    }
					if(err & 0x20 || err & 0x08) {
						printk("Data Trans Err %x \n",err);
					}
					
                }else{
					printk("dtd Null \n");
				}
                /* Retire dtd & start next transfer */
                ar9130_retire_dtd(udc,epno,epdir);
            }
        }
    }

    return;
}

static void ar9130_handle_reset(struct ar9130_udc *udc)
{
    int i;

    ar9130_debug_fn("__enter %s\n",__FUNCTION__);
    ar9130_debug_ps("Port Status %x \n",readl(&udc->op_base->portscx[0]));

    stop_activity(udc);

    if(udc->gadget.speed != USB_SPEED_UNKNOWN){
        udc->gadget.speed = USB_SPEED_UNKNOWN;
    }

    /* The address bits are past bit 25-31. Set the address */
    setAddress(udc, 0);

    /* Clear all the setup token semaphores */
    writel(readl(&udc->op_base->ep_setup_stat),&udc->op_base->ep_setup_stat);

    /* Clear all the endpoint complete status bits */
    writel(readl(&udc->op_base->ep_complete),&udc->op_base->ep_complete);

    while (readl(&udc->op_base->ep_prime) & 0xFFFFFFFF) {
        /* Wait until all ENDPTPRIME bits cleared */
    } 

    /* Write 1s to the Flush register */
    writel(0xffffffff,&udc->op_base->ep_flush);

    /* Unstall all endpoints */
    for (i = 0; i < (AR9130_MAX_END_POINTS * 2); i++) {
        ar9130_unstall_endpoint(udc, i, 0);
        ar9130_unstall_endpoint(udc, i, 1);
    }

    ar9130_debug_ps("Port Status %x \n",readl(&udc->op_base->portscx[0]));
    if (readl(&udc->op_base->portscx[0])& AR9130_PORTSCX_PORT_RESET) {
        udc->usbState = USB_STATE_POWERED;
    } else { 
        ar9130_init_device(udc);
        ar9130_setup(udc);
    }
}

static void ar9130_process_port_change(struct ar9130_udc *udc)
{
    ar9130_debug_fn("__enter %s\n",__FUNCTION__);

    if (!(readl(&udc->op_base->portscx[0]) & AR9130_PORTSCX_PORT_RESET)) {
        /* Get the speed */
        if (readl(&udc->op_base->portscx[0]) & AR9130_PORTSCX_PORT_HIGH_SPEED) {
            udc->gadget.speed = USB_SPEED_HIGH;
        } else {
            udc->gadget.speed = USB_SPEED_FULL;
        }
    }

    if(readl(&udc->op_base->portscx[0]) &  AR9130_PORTSCX_PORT_SUSPEND) {
        if((udc->gadget.speed != USB_SPEED_UNKNOWN) &&
                (udc->ga_driver->suspend))
        {
#if 0
			/* Issue PHY Low Power Suspend - Stops the phy clock */
			writel((readl(&udc->op_base->portscx[0]) | (1 << 23)),
					&udc->op_base->portscx[0]);
#endif
            spin_unlock(&udc->lock);
            udc->ga_driver->suspend(&udc->gadget);
            spin_lock(&udc->lock);
        }
    }

    if(!(readl(&udc->op_base->portscx[0]) &  AR9130_PORTSCX_PORT_SUSPEND)) {
        if((udc->gadget.speed != USB_SPEED_UNKNOWN) &&
                (udc->ga_driver->resume))
        {
			/* Resume - starts the phy clock(not necessary when host issues resume) */
#if 1
			writel((readl(&udc->op_base->portscx[0]) & ~(1 << 23)),
					&udc->op_base->portscx[0]);
#endif
            spin_unlock(&udc->lock);
            udc->ga_driver->resume(&udc->gadget);
            spin_lock(&udc->lock);
        }
    }
}

/*
 * UDC Driver Interrupt handler.  This is called directly by the UDC or
 * indirectly by OTG driver
 */
irqreturn_t ar9130_udc_isr(int irq,void * _udc,struct pt_regs *r)
{
    struct ar9130_udc *udc = (struct ar9130_udc *)_udc;
    __u32 status=0,setupstat =0;
	if(!udc){
		printk("udc null condition \n");
		return IRQ_NONE;
	}	

	if (!udc->op_base) {
		printk ("udc_isr null values %p, %p\n", udc, (udc) ? udc->op_base : NULL);
		return IRQ_NONE;
	}
    /*
     * Avoid delays during device attach times by handling all interrupts
     * at once
     */
    for(;;) {
        status = readl(&udc->op_base->usbsts); 
        setupstat = readl(&udc->op_base->ep_setup_stat);

        if(!(status & readl(&udc->op_base->usbintr))) {
            /* Nothing to do - exit */
            break;
        }

        /* Clear all interrupts */
        writel(status,&udc->op_base->usbsts);

        /* USB Port Reset Event */
        if(status & AR9130_EHCI_STS_RESET) {
            ar9130_debug_int("Port Reset Interrupt\n");
            ar9130_handle_reset(udc);
        }

        /* USB Port Change Event */
        if(status & AR9130_EHCI_STS_PORT_CHANGE) {
            ar9130_debug_int("Port Change Interrupt\n");
            ar9130_process_port_change(udc);
        }

        if(status & AR9130_EHCI_STS_ERR) {
            ar9130_error("Error Interrupt\n");
			printk("Error Interrupt\n");
            /* Not Handled */
        }

        if(status & AR9130_EHCI_STS_SOF) {
            /* Not Handled - Nothing to do */
        }

        /* Endpoint Transfer Complete Events */
        if(status & AR9130_EHCI_STS_INT) {
            ar9130_debug_int("USB Interrupt\n");
            ar9130_process_USB_Intr(udc);
        }

        /* USB Port Suspend Event */
        if(status & AR9130_EHCI_STS_SUSPEND) {
            __u32 otgsc = readl(&udc->op_base->otgsc);
            ar9130_debug_int("USB Suspend\n");
            if (!(otgsc & (1 << 20))) {
		//printk("OTG_Detach Event \n");
                /* USB Device detach event - inform gadget driver */
                stop_activity(udc);
            } else if (udc->ga_driver->suspend){
                /* USB Device suspend event - inform gadget driver */
                writel ((1 << 20), &udc->op_base->otgsc);
                udc->ga_driver->suspend(&udc->gadget);
            }
        }
    }
    return IRQ_HANDLED;
}

/* Endpoint Initialization */
static int ar9130_endpoint_setup(char *ep_name,__u8 ep_addr,__u8 ep_type,
        __u16 maxPack, struct ar9130_udc *udc)
{
    struct ar9130_ep *ep;
    struct usb_ep *_ep;
    struct ep_qhead *ep_QHead;
    __u8 epno, epdir, qh_offset;
    __u32 bits = 0;
    __u32 xferFlags = 0;

    ar9130_debug_fn("__enter %s\n",__FUNCTION__);

    /* Get endpoint number and direction */
    epno  = ep_addr & 0x0f;
    epdir = (ep_addr & USB_DIR_IN) ? USB_SEND : USB_RECV;
    qh_offset  = (2 * epno) + epdir;

    /* Select the Queue Head based on EP number and direction */
    ep = &udc->ep[qh_offset];
    ep_QHead  = (udc->ep_queue_head + qh_offset);
    ep->ep_qh = ep_QHead;
    strlcpy(ep->name,ep_name,sizeof ep->name);
    ep->udc   = udc;
    ep->bEndpointAddress = ep_addr;
    ep->bmAttributes = ep_type;
    INIT_LIST_HEAD(&ep->queue);

    /* Initialize EndPoint queue head params based on EP type */
    switch (ep_type) {
        case USB_ENDPOINT_XFER_ISOC:
            xferFlags  = (1 << AR9130_EP_QUEUE_HEAD_MULT_POS);
            break;
        case USB_ENDPOINT_XFER_CONTROL:
            xferFlags = AR9130_EP_QUEUE_HEAD_IOS;
            break;
        default:
            xferFlags = AR9130_EP_QUEUE_HEAD_ZERO_LEN_TER_SEL;
            break;
    }

    /*
     * We select a default max packet length now.  This is later modified as
     * required when the endpoint is enabled by gadget drivers
     */
    ep_QHead->maxPacketLen = cpu_to_le32((maxPack << 16 ) | xferFlags);

    ar9130_debug_ep("ep_setup ==> ep%d-%s queue:%d, name:%s, maxlen:%d, "
            "ctrl:%x\n", epno, ep_direction[epdir], qh_offset, ep->name,
            maxPack, readl(&udc->op_base->ep_ctrlx[epno]));

    _ep = &ep->ep;
    _ep->name = ep->name;   /*ep- (name,type,direction)*/
    _ep->ops  = &ar9130_ep_ops;
    _ep->maxpacket = ep->maxpacket = maxPack;

    /*
     * Only configure the endpoint properties in the control register, but do
     * not enable them.  The endpoints are enabled by gadget drivers
     */
    bits = (((epdir) ? (AR9130_EPCTRL_TX_DATA_TOGGLE_RST) :
            (AR9130_EPCTRL_RX_DATA_TOGGLE_RST)) |
            (ep_type << ( epdir ? (AR9130_EPCTRL_TX_EP_TYPE_SHIFT) :
                          (AR9130_EPCTRL_RX_EP_TYPE_SHIFT))));

    writel((readl(&udc->op_base->ep_ctrlx[epno]) | bits),
            &udc->op_base->ep_ctrlx[epno]);

    ar9130_debug_ep("ep_setup ==> ep%d-%s queue:%d, name:%s, maxlen:%d, "
            "ctrl:%x,%x\n", epno, ep_direction[epdir], qh_offset, ep->name,
            maxPack, readl(&udc->op_base->ep_ctrlx[epno]),bits);

    /* EP0 not added to the gadget endpoint list */
    if(epno > 0) {
        list_add_tail(&_ep->ep_list,&udc->gadget.ep_list);
    } 
    return 0;
}

static int ar9130_setup(struct ar9130_udc *udc)
{
    ar9130_debug_fn("__enter %s \n",__FUNCTION__);

    /*Init EndPoint 0 Properties */
    ar9130_debug_ep("Init Endpoint 0 \n");
    writel((AR9130_EPCTRL_TX_DATA_TOGGLE_RST |AR9130_EPCTRL_RX_DATA_TOGGLE_RST),
            &udc->op_base->ep_ctrlx[0]);
    writel((readl(&udc->op_base->ep_ctrlx[0]) &
                ~(AR9130_EPCTRL_TX_EP_STALL|AR9130_EPCTRL_RX_EP_STALL)),
            &udc->op_base->ep_ctrlx[0]);

    /* Clear all ENDPTPRIME Status */
    writel(0, &udc->op_base->ep_prime);

    /*
     * We have a total of 5 IN/OUT endpoints, split them for different transfer
     * Control IN/OUT - 1
     * Bulk INOUT - 2
     * ISO IN/OUT - 1
     * INT IN/OUT - 1
     */

    /* Init EndPoint 0 */
    ar9130_endpoint_setup("ep0out", 0, USB_ENDPOINT_XFER_CONTROL, 64, udc);
    ar9130_endpoint_setup("ep0in", 0|USB_DIR_IN, USB_ENDPOINT_XFER_CONTROL,
            64, udc);

    /* Init Bulk EndPoints, Set Required Block maxBlockSize later */
    ar9130_endpoint_setup("ep1out-bulk", 1, USB_ENDPOINT_XFER_BULK, 0x400, udc);
    ar9130_endpoint_setup("ep1in-bulk", 1 |USB_DIR_IN, USB_ENDPOINT_XFER_BULK,
            0x400,udc);

    ar9130_endpoint_setup("ep2out-bulk", 2, USB_ENDPOINT_XFER_BULK, 0x400, udc);
    ar9130_endpoint_setup("ep2in-bulk", 2 | USB_DIR_IN, USB_ENDPOINT_XFER_BULK,
            0x400,udc);

    ar9130_endpoint_setup("ep3out-iso", 3, USB_ENDPOINT_XFER_ISOC, 0x400, udc);
    ar9130_endpoint_setup("ep3in-iso", 3 | USB_DIR_IN, USB_ENDPOINT_XFER_ISOC,
            0x400,udc);

    ar9130_endpoint_setup("ep4out-int", 4, USB_ENDPOINT_XFER_INT, 0x400, udc);
    ar9130_endpoint_setup("ep4in-int", 4 | USB_DIR_IN, USB_ENDPOINT_XFER_INT,
            0x400,udc);

    return 0;
}

static void ar9130_udc_release(struct device *dev)
{
    ar9130_debug_fn("__enter %s\n",__FUNCTION__);
#if 0
    kfree(ap_gadget);
    ap_gadget = NULL;
#endif
}

static void ar9130_init_device(struct ar9130_udc *udc)
{
    ar9130_debug_fn("__enter %s \n",__FUNCTION__);

    /*
     * Device controller Initialization
     */
    ar9130_debug_dev("STOP UDC\n");
    writel(~AR9130_CMD_RUN_STOP, &udc->op_base->usbcmd);
    udelay(100);

    ar9130_debug_dev("RESET UDC\n");
    writel(AR9130_CMD_CTRL_RESET, &udc->op_base->usbcmd);
    udelay(100);

    ar9130_debug_dev("Waiting for Reset to complete \n");
    while(readl(&udc->op_base->usbcmd) & AR9130_CMD_CTRL_RESET);

    ar9130_debug_dev("Setting Device Mode \n");

    /* Set Device Mode */
    writel((AR9130_SET_DEV_MODE | AR9130_USBMODE_SLOM), &udc->op_base->usbmode);

    writel(0, &udc->op_base->ep_setup_stat);

    /* Initialize EndPointList Addr*/
    writel(udc->qh_dma,&udc->op_base->ep_list_addr);

#if 0 // TODO OTG
    if (readl(&udc->op_base->hcs_params) &
            AR9130_HCS_PARAMS_PORT_POWER_CONTROL_FLAG) 
    {
        __u32 port_control;
        port_control = readl(&udc->op_base.portscx[0]);
        port_control &= (~EHCI_PORTSCX_W1C_BITS | ~EHCI_PORTSCX_PORT_POWER);
        writel(port_control, &udc->op_base.portscx[0]);
    }
#endif
#if 0
	/* Force to Full Speed - shekar(Nov 29) */	
    ar9130_reg_rmw_set(&udc->op_base->portscx[0],(1 <<24)); 
	printk("Port Status %x\n",readl(&udc->op_base->portscx[0]));
#endif
    ar9130_debug_fn("__exit %s \n",__FUNCTION__);
}

static void udc_mem_free (struct ar9130_udc *udc)
{
    struct ep_dtd *ep_dtd;
//	int ret = 0;
	ar9130_debug_fn("udc_mem_free \n");
	printk("udc_mem_free \n");
    writel(0, &udc->op_base->ep_list_addr);
    if (udc->dtd_pool) {
    	while(!list_empty(&udc->dtd_list)){
    		struct list_head *temp;
			temp = udc->dtd_list.next;
       		ep_dtd = list_entry(temp,struct ep_dtd,tr_list);
			dma_pool_free(udc->dtd_pool, ep_dtd, ep_dtd->dtd_dma);
       		list_del(temp);
		}
        dma_pool_destroy(udc->dtd_pool);
        udc->dtd_pool = NULL;
    }

    if (udc->ep_queue_head) {
        dma_free_coherent (udc->dev,
            (sizeof(struct ep_qhead) * AR9130_MAX_END_POINTS * 2),
            udc->ep_queue_head, udc->qh_dma);
        udc->ep_queue_head = NULL;
    }

    if(udc->ctrl_req) {
        ar9130_free_request(NULL, udc->ctrl_req);
        udc->ctrl_req = NULL;
    }

    if(udc->ctrl_buf) {
        kfree(udc->ctrl_buf);
        udc->ctrl_buf = NULL;
    }
}

static int udc_mem_init (struct ar9130_udc *udc)
{
    int count;
    struct ep_dtd *ep_dtd;

    /* Allocate pool for device transfer descriptors(DTD) -DTDs are DMA-able */
    udc->dtd_pool = dma_pool_create ("udc_dtd", 
            udc->dev,
            sizeof (struct ep_dtd),
            32 /* byte alignment (for hw parts) */,
            4096 /* can't cross 4K */);
    if (!udc->dtd_pool) {
        ar9130_error ("ar9130_udc: dtd dma_pool_create failure\n");
        return -ENOMEM;
    }

    /* Allocate Queue Heads for transfer -QHs are DMA-able */
    udc->ep_queue_head = dma_alloc_coherent (udc->dev,
            (sizeof(struct ep_qhead) * AR9130_MAX_END_POINTS * 2),
            &udc->qh_dma, 0);
    if (!udc->ep_queue_head) {
        udc_mem_free(udc);
        return -ENOMEM;
    }
    ar9130_debug_mem("queue head %p %x Allocated\n", udc->ep_queue_head,
            udc->qh_dma);

    /* Pre-allocate a transfer request and buffer for EP0 operations */
    udc->ctrl_req = ar9130_alloc_request(NULL, GFP_ATOMIC);
    udc->ctrl_buf = kmalloc(64, GFP_ATOMIC);
    if (!udc->ctrl_req || !udc->ctrl_buf) {
        udc_mem_free(udc);
        return -ENOMEM;
    }
    udc->ctrl_req->buf = udc->ctrl_buf;

    /* Pre-allocate DTDs */
    for(count = 0; count < AR9130_MAX_DTDS; count++){
        dma_addr_t dma;
        ep_dtd = dma_pool_alloc(udc->dtd_pool, GFP_ATOMIC, &dma);
        if (ep_dtd == NULL) {
            udc_mem_free(udc);
            return -ENOMEM;
        }
        ar9130_debug_mem("DTD Alloc %p, %x\n", ep_dtd, dma);
        ep_dtd->dtd_dma = dma;
        list_add_tail(&ep_dtd->tr_list,&udc->dtd_list);
        ep_dtd->size_ioc_status &= cpu_to_le32(~AR9130_TD_RESERVED_FIELDS);
        ep_dtd->next_dtd =__constant_cpu_to_le32(AR9130_TD_NEXT_TERMINATE);
    }

    return 0;
}

/* Provides a graceful exit for the gadget/udc driver */
static void stop_activity(struct ar9130_udc * udc)
{
    struct ar9130_ep *ep = NULL;
    unsigned long flags;

    spin_lock_irqsave(&udc->lock, flags);
    udc->gadget.speed = USB_SPEED_UNKNOWN;

    /* Cancel any EP0 IN/OUT transfers */
    udc_ep_wipe(&udc->ep[0],-ESHUTDOWN);
    udc_ep_wipe(&udc->ep[1],-ESHUTDOWN);

    /* Cancel all other EP IN/OUT transfers used by gadget driver */
    list_for_each_entry(ep,&udc->gadget.ep_list,ep.ep_list){
        udc_ep_wipe(ep,-ESHUTDOWN);
    }
    spin_unlock_irqrestore(&udc->lock, flags);

    /* Disconnect event to Gadget driver */
    if (udc->ga_driver->disconnect) {
        udc->ga_driver->disconnect(&udc->gadget);
    }
}

/* Start AR9130 device controller hardware */
static void start_udc(struct ar9130_udc *udc)
{
    ar9130_debug_dev("Starting Device Controller ...\n");

    ar9130_init_device(udc);
    ar9130_setup(udc);

    /* Enable Interrupts */
    writel ((AR9130_INTR_INT_EN | AR9130_INTR_ERR_INT_EN |
                AR9130_INTR_PORT_CHANGE_DETECT_EN | AR9130_INTR_RESET_EN |
                /*AR9130_INTR_SOF_UFRAME_EN |*/AR9130_INTR_DEVICE_SUSPEND),
            &udc->op_base->usbintr);

    /* Start Device Controller */
    writel(AR9130_CMD_RUN_STOP, &udc->op_base->usbcmd);
    udelay(100);
}

/* Stop AR9130 device controller hardware */
static void stop_udc(struct ar9130_udc *udc)
{
    ar9130_debug_dev("Stoping Device Controller ...\n");

    /* Disable Interrupts */
    writel (0, &udc->op_base->usbintr);

    /* Stop Device Controller */
    writel(~AR9130_CMD_RUN_STOP, &udc->op_base->usbcmd);
    udelay(100);
}

/*
 * Gadget driver registration
 * The device controller driver starts the actual operation only after a 
 * gadget driver is registered.  This is where we enable the UDC interrupts
 */
int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
    struct ar9130_udc *udc = ap_gadget;
    int ret;

    ar9130_debug_fn("__enter %s\n",__FUNCTION__);

    /* Sanity checks */
    if(!udc) {
        ar9130_error("no udc %p\n",udc);
        return -ENODEV;
    }

    if (!driver
            || driver->speed != USB_SPEED_HIGH
            || !driver->bind
            /*|| !driver->unbind*/
            || !driver->setup)
    {
        ar9130_error("gadget driver does not match udc\n");
        return -EINVAL;
    }

    /* hook up the driver */
    driver->driver.bus = NULL;
    udc->ga_driver = driver;
    udc->gadget.dev.driver = &driver->driver;

    /* Bind the gadget driver */
    ret = driver->bind(&udc->gadget);
    if (ret) {
        ar9130_error("unable to bind driver %s --> %d\n",
                driver->driver.name, ret);
        udc->ga_driver = NULL;
        udc->gadget.dev.driver = NULL;
        return ret;
    }

#ifdef CONFIG_USB_AR9130_OTG
    udc->ar9130_otg->udc = udc;
    udc->ar9130_otg->udc_isr = ar9130_udc_isr;
    /* Enable peripheral mode in OTG */
    if (otg_set_peripheral(&udc->ar9130_otg->otg, &udc->gadget)) {
        if (driver->unbind) {
            driver->unbind(&udc->gadget);
        }
        udc->gadget.dev.driver = NULL;
        udc->ga_driver = NULL;
        return -EINVAL;
    }
#else
    /* Everything is fine - start the device controller */
    start_udc(udc);
#endif

    ar9130_debug_fn("__exit %s\n",__FUNCTION__);
    return 0;
}
EXPORT_SYMBOL(usb_gadget_register_driver);

int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
    struct ar9130_udc *udc = ap_gadget;

    if(!udc){
        return -ENODEV;
    }

    if(!driver || driver != udc->ga_driver){
        return -EINVAL;
    }

    stop_udc(udc);
    stop_activity(udc);

#ifdef CONFIG_USB_AR9130_OTG
    /* Disable peripheral mode in OTG */
	printk ("set peripheral null\n");
    otg_set_peripheral(&udc->ar9130_otg->otg, NULL);
    udc->ar9130_otg->udc = NULL;
    udc->ar9130_otg->udc_isr = NULL;
#endif

    if (driver->unbind) {
        driver->unbind(&udc->gadget);
    }
    udc->gadget.dev.driver = NULL;
    udc->ga_driver = NULL;

    return 0;
}
EXPORT_SYMBOL(usb_gadget_unregister_driver);

static int ar9130_udc_init(struct ar9130_udc *udc, struct device *dev)
{
    struct ep_qhead *ep_queue_head;
    int temp;

    udc->dev = dev;
    spin_lock_init(&udc->lock);
    INIT_LIST_HEAD(&udc->dtd_list);
    if (udc_mem_init(udc) != 0) {
        return -ENOMEM;
    }

    /* Initialize all device Q Head */
    ep_queue_head = udc->ep_queue_head;
    ar9130_debug_dev("QHead Size %x : eTD Size %x \n",
            sizeof(struct ep_qhead),sizeof(struct ep_dtd));

    ar9130_debug_dev("Initialize Dev Trans Descp \n");
    for(temp = 0; temp < (AR9130_MAX_END_POINTS * 2); temp++){
        (ep_queue_head + temp)->maxPacketLen = cpu_to_le32(0x400);
        (ep_queue_head + temp)->next_dtd =
            cpu_to_le32(AR9130_EP_QUEUE_HEAD_NEXT_TERMINATE);
    }

    udc->gadget.ops = &ar9130_udc_ops;
    udc->gadget.ep0 = &udc->ep[1].ep;
    INIT_LIST_HEAD(&udc->gadget.ep_list);
    udc->gadget.speed = USB_SPEED_UNKNOWN;
    udc->gadget.name  = device_name;

    device_initialize(&udc->gadget.dev);
    //strcpy(udc->gadget.dev.bus_id,"gadget");
    udc->gadget.dev.release = ar9130_udc_release;
    udc->gadget.dev.parent  = dev;

    ap_gadget = udc;
    ar9130_debug_dev("UDC %p\n",ap_gadget);

    /* Setup all endpoints */
    ar9130_setup(udc);
    device_add(&udc->gadget.dev);
    return 0;
}

#ifndef CONFIG_USB_AR9130_OTG
static int ar9130_udc_probe(struct platform_device *pdev)
{
    struct ar9130_udc *udc;
    void __iomem *reg_base;
    int retval;

    ar9130_debug_fn("__enter %s \n",__FUNCTION__);

    udc = (struct ar9130_udc *)kmalloc(sizeof(struct ar9130_udc), GFP_ATOMIC);
    if(udc == NULL){
        ar9130_error("Unable to allocate udc device\n");
        return -ENOMEM;
    }
    memset (udc, 0, sizeof(struct ar9130_udc));

    /* Allocate and map resources */
    if (!request_mem_region(pdev->resource[0].start, 
                pdev->resource[0].end - pdev->resource[0].start + 1,
                driver_name))
    {
        ar9130_error("ar9130_udc: controller already in use\n");
        retval = -EBUSY;
        goto err1;
    }

    reg_base = ioremap(pdev->resource[0].start,
            pdev->resource[0].end - pdev->resource[0].start +1);
    if (!reg_base) {
        ar9130_error("ar9130_udc: error mapping memory\n");
        retval = -EFAULT;
        goto err2;
    }

    udc->reg_base = reg_base;
    reg_base += 0x140;
    udc->op_base = reg_base;

    /* Device Initialization - start */
    ar9130_debug_dev("Device Initialization\n");

#if 0 /*Setting to 8-bit 6th March*/
    ar9130_reg_rmw_clear(AR9130_RESET,AR9130_RESET_USB_HOST);
    ar9130_reg_rmw_set(AR9130_RESET,AR9130_RESET_USB_PHY);  //PHY RESET
#endif

    if (is_wasp() || is_ar7242() || is_ar7241()) {
        ar9130_reg_rmw_set(AR9130_RESET,AR9130_RESET_USBSUS_OVRIDE);
        mdelay(10);
        ar9130_reg_wr(AR9130_RESET,((ar9130_reg_rd(AR9130_RESET) & ~(AR9130_RESET_USB_HOST)) |
                                                                  AR9130_RESET_USBSUS_OVRIDE));
        mdelay(10);
        ar9130_reg_wr(AR9130_RESET,((ar9130_reg_rd(AR9130_RESET) & ~(AR9130_RESET_USB_PHY)) |
                                                                   AR9130_RESET_USBSUS_OVRIDE));
        mdelay(10);
    }
    else {

        ar9130_reg_rmw_clear(AR9130_RESET,AR9130_RESET_USB_PHY);//PHY CLEAR RESET
        ar9130_debug_dev("AR9130_RESET %x \n",ar9130_reg_rd(AR9130_RESET));
        mdelay(10);
        ar9130_reg_rmw_clear(AR9130_RESET,AR9130_RESET_USB_HOST); // 6th March 
        mdelay(10);
    }


    /* Setting 16-bit mode */
    ar9130_reg_rmw_set(&udc->op_base->portscx[0],(1 <<28)); 
    ar9130_debug_dev("PORT_STATUS[0] %x\n",readl(&udc->op_base->portscx[0]));
    mdelay(10);

    /* Clear Host Mode */
    if (is_ar7242() || is_ar7241()) {
        ar9130_reg_rmw_clear(AR9130_USB_CONFIG,(1 << 8));
    }
    else {
         ar9130_reg_rmw_clear(AR9130_USB_CONFIG,(1 << 2));
    }
    ar9130_debug_dev("Usb Config Reg %x\n",ar9130_reg_rd(AR9130_USB_CONFIG));
    mdelay(10);


    /* Setting 16-bit mode */
    ar9130_reg_rmw_set(&udc->op_base->portscx[0],(1 <<28)); 
    ar9130_debug_dev("PORT_STATUS[0] %x\n",readl(&udc->op_base->portscx[0]));
    mdelay(10);

    /* Clear Host Mode */
    if (is_wasp() || is_ar7242() || is_ar7241()) {
        ar9130_reg_rmw_clear(AR9130_USB_CONFIG,(1 << 8));
    }
    else {
         ar9130_reg_rmw_clear(AR9130_USB_CONFIG,(1 << 2));
    }
    ar9130_debug_dev("Usb Config Reg %x\n",ar9130_reg_rd(AR9130_USB_CONFIG));
    mdelay(10);

    /*Debug Info*/
    ar9130_debug_dev("Platform Device Info:\n");
    ar9130_debug_dev("pdev->resource[0].start %lx\n",pdev->resource[0].start);
    ar9130_debug_dev("pdev->resource[1].start %lu\n",pdev->resource[1].start);
    ar9130_debug_dev("reg_base :%p udc->op_base :%p\n",reg_base,udc->op_base);

    /* Interrupt Request */
    if((retval = request_irq(pdev->resource[1].start, ar9130_udc_isr,
                    IRQF_SHARED, driver_name, udc)) != 0)
    {
        ar9130_error("request interrupt %lx failed\n", pdev->resource[1].start);
        retval = -EBUSY;
        goto err3;
    }

    if (ar9130_udc_init(udc, &pdev->dev) == 0) {
        return 0;
    }

    free_irq(pdev->resource[1].start, udc);
err3:
    iounmap(reg_base);
err2:
    release_mem_region(pdev->resource[0].start,
            pdev->resource[0].end - pdev->resource[0].start+1);
err1:
    ap_gadget = NULL;
    kfree(udc);
    return retval;
}

static int ar9130_udc_remove(struct platform_device *pdev)
{
    struct ar9130_udc *udc =ap_gadget;

    udc_mem_free(udc);
    free_irq(pdev->resource[1].start, udc);
    iounmap(udc->reg_base);
    release_mem_region(pdev->resource[0].start,
            pdev->resource[0].end - pdev->resource[0].start+1);
    device_unregister(&udc->gadget.dev);
    ap_gadget = NULL;
    kfree(udc);

    return 0;
}

static struct platform_driver ar9130_udc_drv = {
    .probe  = ar9130_udc_probe,
    .remove = ar9130_udc_remove,
    .driver = {
        .name  = (char *)driver_name,
        .owner = THIS_MODULE,
    },
};

#else

static int ar9130_udc_probe(void)
{
    struct ar9130_otg *ar9130_otg;
    struct ar9130_udc *udc;

    ar9130_debug_fn("__enter %s \n",__FUNCTION__);

    if ((ar9130_otg = ar9130_get_otg()) == NULL) {
        return -ENODEV;
    }

    udc = (struct ar9130_udc *)kmalloc(sizeof(struct ar9130_udc), GFP_ATOMIC);
    if(udc == NULL){
        ar9130_error("Unable to allocate udc device\n");
        return -ENOMEM;
    }
    memset (udc, 0, sizeof(struct ar9130_udc));

    udc->ar9130_otg = ar9130_otg;
	udc->gadget.is_otg = 1;
    udc->op_base = ar9130_otg->usb_reg;
    if (ar9130_udc_init(udc, ar9130_otg->dev) < 0) {
        kfree(udc);
        return -ENODEV;
    }
    return 0;
}

static int ar9130_udc_remove(void)
{
    struct ar9130_udc *udc = ap_gadget;

    if (udc) {
        udc_mem_free(udc);
        udc->ar9130_otg = NULL;
        device_unregister(&udc->gadget.dev);
        kfree(udc);
    }
    ap_gadget = NULL;

    return 0;
}
#endif

static int __init ar9130_init(void)
{
    ar9130_debug_fn("__enter %s\n",__FUNCTION__);
#ifdef CONFIG_USB_AR9130_OTG
    return (ar9130_udc_probe());
#else
    return platform_driver_register(&ar9130_udc_drv);
#endif
}

static void __exit ar9130_exit(void)
{
    ar9130_debug_fn("__enter %s\n",__FUNCTION__);
#ifdef CONFIG_USB_AR9130_OTG
    ar9130_udc_remove();
#else
    platform_driver_unregister(&ar9130_udc_drv);
#endif
}

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

arch_initcall(ar9130_init);
module_exit(ar9130_exit);

