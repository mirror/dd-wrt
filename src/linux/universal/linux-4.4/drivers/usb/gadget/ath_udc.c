/*
 * Atheros USB Device Controller Driver
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

#define ATH_USB_DEBUG 1
#define TRIP_WIRE

#include "ath_udc.h"

/*
 * debug level zones can be enabled individually for different level of
 * debugging.  Debug messages can delay the init sequence if the prints are
 * redirected to serial console, and result in device detection failure.
 * Enable them only if required.
 */
#ifdef ATH_USB_DEBUG
static int ath_usb_debug_level = (
	//ATH_USB_DEBUG_FUNCTION	|
	//ATH_USB_DEBUG_INTERRUPT	|
	//ATH_USB_DEBUG_ENDPOINT	|
	//ATH_USB_DEBUG_PORTSTATUS	|
	//ATH_USB_DEBUG_DEVICE		|
	//ATH_USB_DEBUG_MEMORY		|
	//ATH_USB_DEBUG_QUEUEHEAD	|
	//ATH_USB_DEBUG_DTD		|
	0x0);
static const char *ep_direction[] = { "OUT", "IN" };
#endif

#define DRIVER_DESC	"Atheros UDC Driver"

/*
 * There are a maximum of 32 endpoints (16 IN and 16 OUT).  But currently only
 * 12 (6 IN and 6 OUT) endpoints are enabled.
 */
#define ATH_USB_MAX_EP			(6)
#define ATH_USB_MAX_EP_IN_SYSTEM	(32)
#define ATH_USB_MAX_DTD			(32)
#define ATH_USB_REMOTE_WKUP		(0x02)

static struct proc_dir_entry *ath_usb_proc;

#define USB_RECV			(0)
#define USB_SEND			(1)

#define ATH_USB_CTRL_EP			(1 << 0)
#define DMA_ADDR_INVALID		((dma_addr_t)~0)

/*
 * The platform device in ATH_USB still reflects the old ar7100 name.  So we
 * maintain two names here - one for the platform driver and other for local
 * reference
 */
#if defined(CONFIG_MACH_AR7240) || defined(CONFIG_MACH_HORNET)
static const char driver_name [] = "ar7240-ehci";
#else
static const char driver_name[] = "ath-ehci";
#endif
static const char device_name[] = "ath_udc";
static struct ath_usb_udc *ap_gadget;

unsigned long int_count, case1, case2;
unsigned long actual_data_count;
unsigned long complete_count;
unsigned long queue_count;
unsigned long wipe_count;
unsigned long alloc_init_dtd_count;
unsigned long start_trans_count;
unsigned long isr_count;
unsigned long retire_dtd_count;

struct ath_usb_ep {
	struct usb_ep ep;
	char name[15];
	const struct usb_endpoint_descriptor *ep_desc;
	struct list_head queue;
	struct list_head skipped_queue;
	__u8 bEndpointAddress;
	__u8 bmAttributes;
	__u16 maxpacket;
	struct ep_qhead *ep_qh;
	struct ath_usb_udc *udc;
};

struct ath_usb_req {
	struct usb_request req;
	struct list_head queue;
	struct ep_dtd *ep_dtd;
	unsigned mapped:1;
};

struct ath_usb_udc {
	struct usb_gadget gadget;
	struct usb_gadget_driver *ga_driver;
	struct device *dev;
	struct ath_usb __iomem *op_base;
	struct ath_usb_ep ep[32];
	struct ath_usb_otg *ath_usb_otg;
	struct ep_qhead *ep_queue_head;
	struct dma_pool *dtd_pool;
	struct list_head dtd_list[ATH_USB_MAX_EP_IN_SYSTEM];
	struct ep_dtd *dtd_heads[ATH_USB_MAX_EP * 2];
	struct ep_dtd *dtd_tails[ATH_USB_MAX_EP * 2];
	struct usb_request *ctrl_req;
	void *ctrl_buf;
	void __iomem *reg_base;
	spinlock_t lock;
	__u16 usbState;
	__u16 usbDevState;
	__u8 usbCurrConfig;
	__u8 devAddr;
	__u8 ep0setup;
	dma_addr_t qh_dma;
};

static void ath_usb_start_udc(struct ath_usb_udc *udc);
static void ath_usb_stop_udc(struct ath_usb_udc *udc);
static void ath_usb_stop_activity(struct ath_usb_udc *udc);
static void ath_usb_init_device(struct ath_usb_udc *udc);
static int ath_usb_setup(struct ath_usb_udc *udc);
static int ath_usb_udc_mem_init(struct ath_usb_udc *udc);
static void ath_usb_udc_mem_free(struct ath_usb_udc *udc);
static void ath_usb_udc_ep_wipe(struct ath_usb_ep *ep, int status);
static void ath_usb_free_dtd(struct ath_usb_udc *udc, struct ep_dtd *ep_dtd, __u32 index);
static void ath_usb_complete_transfer(struct ath_usb_ep *ep, struct ath_usb_req *req,
	__u8 epdir, int status);
static void ath_usb_send_data(struct ath_usb_udc *udc, __u8 epno, __u8 * buff,
	__u32 size);
static void ath_usb_stall_endpoint(struct ath_usb_udc *udc, __u8 epno,
	__u8 epdir);
static void ath_usb_unstall_endpoint(struct ath_usb_udc *udc, __u8 epno,
	__u8 epdir);
//#define ATH_USB_UDC_DEBUG	1
#ifdef ATH_USB_UDC_DEBUG
/*
 * Print Queue Head information. EP0 information is not printed right now
 * as it interferes in Device attach/detection
 */
static void ath_usb_print_qh(struct ep_qhead *ep_qh, __u16 epno, __u16 epdir,
	char *str)
{
	if ((epno >= ATH_USB_MAX_EP) && (epno < 1)) {
		return;
	}
	ath_usb_debug_qh("%s Endpoint %d-%s Queue %p\n\tmaxPacketLen %x\n\t"
		"curr_dtd %x\n\tnext_dtd %x\n\tstatus %x\n\tBuff0 %x\n",
		str, epno, ep_direction[epdir], ep_qh,
		le32_to_cpu(ep_qh->maxPacketLen),
		le32_to_cpu(ep_qh->curr_dtd),
		le32_to_cpu(ep_qh->next_dtd),
		le32_to_cpu(ep_qh->size_ioc_int_status),
		le32_to_cpu(ep_qh->buff[0]));
}

/*
 * Print Device Transfer Descriptor information. EP0 information is not
 * printed right now as it interferes in Device attach/detection.
 */
static void ath_usb_print_dtd(struct ep_dtd *ep_dtd, __u16 epno, __u16 epdir,
	char *str)
{
	if ((epno >= ATH_USB_MAX_EP) && (epno < 1)) {
		return;
	}
	ath_usb_debug_dtd("%s Endpoint %d-%s Descriptor %p\n\tnextTr %x\n\t"
		"status %x\n\tBuff0 %x\n\tBuff1 %x\n",
		str, epno, ep_direction[epdir], ep_dtd,
		le32_to_cpu(ep_dtd->next_dtd),
		le32_to_cpu(ep_dtd->size_ioc_status),
		le32_to_cpu(ep_dtd->buff[0]),
		le32_to_cpu(ep_dtd->buff[1]));
}
#else
#define ath_usb_print_qh(ep_qh, epno, epdir, str)	\
	do { (void)(epno); } while (0)
#define ath_usb_print_dtd(ep_dtd, epno, epdir, str)	\
	do { (void)(epno); } while (0)
#endif

void ath_usb_dbg_buf(struct usb_request *req)
{
	int i;
	unsigned char *tmp_buf = (unsigned char *)req->buf;
	for (i = 0; i < req->length; i++) {
		printk(" %02x ", *(tmp_buf++));
		if ((i % 64) == 0) {
			printk("\n");
		}
	}
}

void ath_usb_fill_buf(struct usb_request *req)
{
	int i, j = 0;
	unsigned char *tmp_buf = (unsigned char *)req->buf;
	for (i = 0; i < req->length; i++) {
		*tmp_buf++ = j++;
		if ((j % 256) == 0) { j = 0; }
	}
}

/* Allocate an USB Request - used by gadget drivers */
static struct usb_request *ath_usb_alloc_request(struct usb_ep *ep,
						 gfp_t gfp_flags)
{
	struct ath_usb_req *req;
	ath_usb_debug_fn("__enter %s\n", __func__);

	req = (struct ath_usb_req *)kmalloc(sizeof(struct ath_usb_req), GFP_ATOMIC);
	if (req) {
		memset(req, 0, sizeof(struct ath_usb_req));
		req->req.dma = DMA_ADDR_INVALID;
		req->ep_dtd = NULL;
		INIT_LIST_HEAD(&req->queue);
	} else {
		return NULL;
	}
	return &req->req;
}

/* Free an USB Request - used by gadget drivers */
static void ath_usb_free_request(struct usb_ep *ep, struct usb_request *_req)
{
	struct ath_usb_req *req;
	ath_usb_debug_fn("__enter %s\n", __func__);
	if (_req) {
		req = container_of(_req, struct ath_usb_req, req);
		kfree(req);
	}
}

/* Allocate data buffer for use by USB request - used by gadget drivers */
static void *ath_usb_alloc_buffer(struct usb_ep *_ep, unsigned bytes,
	dma_addr_t * dma, gfp_t gfp_flags)
{
	struct ath_usb_ep *ep = container_of(_ep, struct ath_usb_ep, ep);

	ath_usb_debug_fn("__enter %s\n", __func__);
	/*
	 * Small memory allocation wastes memory :
	 * dma_coherent go for one page
	 */
	return dma_alloc_coherent(ep->udc->gadget.dev.parent,
		bytes, dma, gfp_flags);
}

/* Free data buffer - used by gadget drivers */
static void ath_usb_free_buffer(struct usb_ep *_ep, void *buf, dma_addr_t dma,
	unsigned bytes)
{
	struct ath_usb_ep *ep = container_of(_ep, struct ath_usb_ep, ep);
	ath_usb_debug_fn("__enter %s\n", __func__);
	dma_free_coherent(ep->udc->dev, bytes, buf, dma);
}

/*
 * Enable an endpoint.
 * The endpoint is configured using the ep descriptor properties.  The selected
 * endpoint already has endpoint types configured properly.  This function
 * primarily enables the endpoint in hardware and sets the max packet size.
 */
static int ath_usb_ep_enable(struct usb_ep *_ep,
	const struct usb_endpoint_descriptor *desc)
{
	struct ath_usb_ep *ep = container_of(_ep, struct ath_usb_ep, ep);
	__u8 epno, epdir, qh_offset;
	__u32 bits = 0;
	__u16 maxpacket;
	__u32 qh_maxpacket;
	__u32 bit_pos;
	unsigned long flags;
	struct ath_usb_udc *udc;
	struct ep_qhead *ep_QHead;

	ath_usb_debug_fn("__enter %s\n", __func__);

	/* Sanity check between the endpoint and descriptor properties */
	if (!_ep || !desc || (desc->bDescriptorType != USB_DT_ENDPOINT)
		 || (ep->bEndpointAddress != desc->bEndpointAddress)) {
		ath_usb_warn("%s, bad ep or descriptor \n", __func__);
		return -EINVAL;
	}

	ath_usb_debug_ep("ep_enable name:%s, maxpacket:%d, ep_addr:%x\n",
			_ep->name, _ep->maxpacket, ep->bEndpointAddress);
	ath_usb_debug_ep("descriptor lenth:%x, type:%x, ep_addr:%x, maxpacket:%x\n",
		desc->bLength, desc->bDescriptorType, desc->bEndpointAddress,
		desc->wMaxPacketSize);

	/* Get endpoint number and direction */
	epno = ep->bEndpointAddress & 0x0f;
	epdir = (ep->bEndpointAddress & USB_DIR_IN) ? USB_SEND : USB_RECV;
	bit_pos = (1 << (16 * epdir + epno));

	qh_offset = (2 * epno) + epdir;
	udc = ep->udc;
	if (ep != &udc->ep[qh_offset]) {
		ath_usb_warn("%s, bad ep or descriptor \n", __func__);
		return -EINVAL;
	};

	/* Get endpoint Queue Head based on EP number and direction */
	ep_QHead = (udc->ep_queue_head + qh_offset);

	spin_lock_irqsave(&udc->lock, flags);

	/* Set max packet length for the endpoint in EP queue head */
	ep->ep_desc = desc;
	maxpacket = le16_to_cpu(desc->wMaxPacketSize);
	qh_maxpacket = le32_to_cpu(ep_QHead->maxPacketLen);
	qh_maxpacket = (qh_maxpacket & 0xF800FFFF) | (maxpacket << 16);
	ep_QHead->maxPacketLen = cpu_to_le32(qh_maxpacket);
	_ep->maxpacket = ep->maxpacket = maxpacket;
#if 0
	if (epno > 0) {
		//printk("ep->mps %d ep_Qhead %d \n", ep->maxpacket, ep_QHead->maxPacketLen);
		printk("ep_QHead %x \n", ep_QHead);
	}
#endif

	/* Enable endpoint in Hardware */
	bits = epdir ? (ATH_USB_EPCTRL_TX_ENABLE) : (ATH_USB_EPCTRL_RX_ENABLE);
	writel((readl(&udc->op_base->ep_ctrlx[epno]) | bits),
		&udc->op_base->ep_ctrlx[epno]);
#if 1
	/* Flush the endpoint and make sure that the endpoint status is zero */
	writel(bit_pos, &udc->op_base->ep_flush);
	/* Wait till flush completes */
	while (readl(&udc->op_base->ep_flush) & bit_pos) ;

	while (readl(&udc->op_base->ep_status) & bit_pos) {
		writel(bit_pos, &udc->op_base->ep_flush);
		while (readl(&udc->op_base->ep_flush) & bit_pos) ;
	}

#endif
	ath_usb_debug_ep("ep_enable ==> ep%d-%s queue:%d, name:%s, maxlen:%x, "
		"ctrl:%x\n", epno, ep_direction[epdir], qh_offset,
		ep->name, qh_maxpacket, readl(&udc->op_base->ep_ctrlx[epno]));

	spin_unlock_irqrestore(&udc->lock, flags);
	return 0;
}

/*
 * Disable an endpoint
 * If there are any pending requests on the endpoint, shut them and inform the
 * gadget driver about it.  Disable the endpoint in the hardware.
 */
static int ath_usb_ep_disable(struct usb_ep *_ep)
{
	struct ath_usb_udc *udc;
	struct ath_usb_ep *ep;
	unsigned long flags;
	__u8 epno, epdir;
	__u32 bits;

	ep = container_of(_ep, struct ath_usb_ep, ep);
	if (!_ep || !ep->ep_desc) {
		return -EINVAL;
	}

	spin_lock_irqsave(&ap_gadget->lock, flags);

	/* Cancel all current and pending requests for this endpoint */
	ep->ep_desc = NULL;
	ath_usb_udc_ep_wipe(ep, -ESHUTDOWN);
	ep->ep.maxpacket = ep->maxpacket;
	/* Get endpoint number and direction */
	epno = ep->bEndpointAddress & 0x0f;
	epdir = (ep->bEndpointAddress & USB_DIR_IN) ? USB_SEND : USB_RECV;

	/* Disable the endpoint in hardware */
	bits = epdir ? (ATH_USB_EPCTRL_TX_ENABLE) : (ATH_USB_EPCTRL_RX_ENABLE);
	udc = ep->udc;
	writel((readl(&udc->op_base->ep_ctrlx[epno]) & ~bits),
	       &udc->op_base->ep_ctrlx[epno]);

	spin_unlock_irqrestore(&ap_gadget->lock, flags);

	return 0;
}

/* for a selected endpoint cancel all pending requests and inform the upper
 * layer about cancellation */
static void ath_usb_udc_ep_wipe(struct ath_usb_ep *ep, int status)
{
	struct ath_usb_req *req;
	struct ath_usb_udc *udc;
	struct ep_dtd *ep_dtd;
	udc = ep->udc;
	while (!list_empty(&ep->queue)) {
		__u8 epdir = (ep->bEndpointAddress & USB_DIR_IN) ? USB_SEND : USB_RECV;
		__u8 epno = ep->bEndpointAddress & 0x0f;

		if ((ep_dtd = udc->dtd_heads[(epno * 2) + epdir]) != NULL) {
			udc->dtd_heads[(epno * 2) + epdir] = NULL;
		}

		if ((ep_dtd = udc->dtd_tails[(epno * 2) + epdir]) != NULL) {
			udc->dtd_tails[(epno * 2) + epdir] = NULL;
		}
		req = list_entry(ep->queue.next, struct ath_usb_req, queue);
		ath_usb_free_dtd(udc, req->ep_dtd, ((epno * 2) + epdir));

		list_del_init(&req->queue);
		ath_usb_complete_transfer(ep, req, epdir, status);
	}
	while(!list_empty(&ep->skipped_queue)) {
		__u8 epdir = (ep->bEndpointAddress & USB_DIR_IN) ? USB_SEND : USB_RECV;
		req = list_entry(ep->skipped_queue.next, struct ath_usb_req, queue);
		list_del_init(&req->queue);
		ath_usb_complete_transfer(ep, req, epdir, status);
	}
}

static struct ep_dtd *
ath_usb_alloc_init_dtd(struct ath_usb_udc *udc, struct ath_usb_ep *ep, struct ath_usb_req *req, __u32 catalyst)
{
	struct list_head *temp;
	struct ep_dtd *ep_dtd = NULL;
	alloc_init_dtd_count++;
	/* Get a free device transfer descriptor from the pre-allocated dtd list */
	if(!list_empty(&udc->dtd_list[catalyst])) {
		temp = udc->dtd_list[catalyst].next;
		ep_dtd = list_entry(temp, struct ep_dtd, tr_list);
		list_del(temp);
		memset(ep_dtd, 0, 28); /* only Hardware access fields */
		//printk("****** DTD allocation success***** %u\n", catalyst);
	} else {
		dma_addr_t dma;
		ep_dtd = dma_pool_alloc(udc->dtd_pool, GFP_ATOMIC, &dma);
		if (ep_dtd == NULL) {
			return NULL;
		}
		ath_usb_debug_mem("DTD Alloc %p, %x\n", ep_dtd, dma);
		printk("DTD Alloc %p, %x\n", ep_dtd, dma);
		ep_dtd->dtd_dma = dma;
		ep_dtd->next = NULL;
		list_add_tail(&ep_dtd->tr_list, &udc->dtd_list[catalyst]);
		ep_dtd->size_ioc_status &= cpu_to_le32(~ATH_USB_TD_RESERVED_FIELDS);
	}
	/* Initialize dtd */
	ep_dtd->next_dtd = __constant_cpu_to_le32(ATH_USB_TD_NEXT_TERMINATE);
	ep_dtd->size_ioc_status = cpu_to_le32 (
		((req->req.length)<< ATH_USB_TD_LENGTH_BIT_POS) | ATH_USB_TD_IOC |
		(ATH_USB_TD_STATUS_ACTIVE));

	/* Set Reserved Field to 0 */
	ep_dtd->size_ioc_status &= cpu_to_le32(~ATH_USB_TD_RESERVED_FIELDS);

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
	req->ep_dtd = ep_dtd;
	return ep_dtd;
}

static int ath_usb_ep_prime(struct ath_usb_udc *udc, struct ep_dtd *ep_dtd,
			struct ep_dtd *prev_dtd, __u32 bit_pos, __u32 catalyst, __u8 empty)
{
	struct ep_qhead *ep_QHead;
	__u8 transFlag;
	__u32 temp_pos;
	ep_QHead =udc->ep_queue_head + catalyst;
	if(empty) {
		ep_QHead->next_dtd = cpu_to_le32(ep_dtd->dtd_dma);
		ep_QHead->size_ioc_int_status = __constant_cpu_to_le32(0);
		writel(bit_pos, &udc->op_base->ep_prime);
		return 0;
	} else {
		//  printk("epno = %u, epdir = %u, dtd_temp= %x\n", epno, epdir, dtd_temp);
		prev_dtd->next_dtd = cpu_to_le32(ep_dtd->dtd_dma);
		if(readl(&udc->op_base->ep_prime) & bit_pos)
			return 0;
		/* Use hardware tripwire semaphore mechanism before priming the endpoint */
#ifdef TRIP_WIRE
		transFlag = 0;
		while(!transFlag) {
			writel(readl(&udc->op_base->usbcmd) |ATH_USB_CMD_ATDTW_TRIPWIRE_SET,
				&udc->op_base->usbcmd);
			temp_pos = readl(&udc->op_base->ep_status) & bit_pos;

			if(readl(&udc->op_base->usbcmd) & ATH_USB_CMD_ATDTW_TRIPWIRE_SET) {
				transFlag = 1;
			}
		}

		writel(readl(&udc->op_base->usbcmd) & ATH_USB_CMD_ATDTW_TRIPWIRE_CLEAR,
			&udc->op_base->usbcmd);
#endif
	}
	if(!temp_pos) {
		/* Initialize queue head and prime the endpoint */
		ep_QHead->next_dtd = cpu_to_le32(ep_dtd->dtd_dma);
		ep_QHead->size_ioc_int_status = __constant_cpu_to_le32(0);
		/* Prime the EndPoint */
		writel(bit_pos, &udc->op_base->ep_prime);
	}
	return 0;
}

/*
 * Start an IN or OUT transaction based on usb request and endpoint.
 */
static int ath_usb_start_trans(struct ath_usb_udc *udc, struct ath_usb_ep *ep,
	struct ath_usb_req *req, int flag)
{
	struct ep_qhead *ep_QHead;
	struct ep_dtd *ep_dtd = NULL, *dtd_temp = NULL;
	//unsigned long flags;
	__u32 catalyst, bit_pos =0;
	__u8 epno, epdir;
	__u8 empty;
	start_trans_count++;
	ath_usb_debug_fn("__enter %s\n", __func__);

	/* Get endpoint number and direction */
	epno = ep->bEndpointAddress & 0x0f;
	epdir = (ep->bEndpointAddress & USB_DIR_IN) ? USB_SEND : USB_RECV;

	catalyst = ((2 * epno) + epdir);
	bit_pos = (1 << ((16 * epdir) + epno));

	/* Get endpoint queue head based on EP number and direction */
	ep_QHead =udc->ep_queue_head + catalyst;

	if(catalyst < 0) {
		printk("Warning... wrong dtd head position \n");
	}
	/* Get a free device transfer descriptor from the pre-allocated dtd list */
	//spin_lock_irqsave(&udc->lock, flags);
	if(flag == 1) {
		ep_dtd = ath_usb_alloc_init_dtd(udc, ep, req, catalyst);
		if(!ep_dtd) {
			ath_usb_warn("__err - dtd List Empty %s\n", __func__);
			return -ENOMEM;
		}
		//printk("epno = %u, epdir = %u, ep_dtd = %x\n", epno, epdir, ep_dtd);
		/*
		 * If the endpoint is already primed we have nothing to do here; just
		 * return; TODO - attach the current dtd to the dtd list in the queue
		 * head if the endpoint is already primed.
		 */
	} else {
		if(!list_empty(&ep->skipped_queue)) {
			req = container_of(ep->skipped_queue.next, struct ath_usb_req, queue);
			list_del_init(&req->queue);
		}
		ep_dtd = req->ep_dtd;
		//printk("epno = %u, epdir = %u, ep_dtd = %x\n", epno, epdir, ep_dtd);
	}
	empty = list_empty(&ep->queue);
	if(empty) {
		//printk("epno = %u, epdir = %u ep_dtd = %x tail= %x, head = %x, catalist = %u flag = %d\n", epno, epdir, ep_dtd, udc->dtd_tails[catalyst], udc->dtd_heads[catalyst], catalyst, flag);
		udc->dtd_heads[catalyst] = ep_dtd;
		udc->dtd_tails[catalyst] = ep_dtd;
	} else {
		if(udc->dtd_tails[catalyst]) {
			dtd_temp = udc->dtd_tails[catalyst];
		} else {
			dtd_temp = udc->dtd_heads[catalyst];
		}
		if(dtd_temp) {
			dtd_temp->next = ep_dtd;
		} else {
			return 0;
		}
		udc->dtd_tails[catalyst] = ep_dtd;
		//printk("Changing dtd_tails of epno = %u, epdir = %u tail= %x, head = %x, catalist = %u\n", epno, epdir, udc->dtd_tails[catalyst], udc->dtd_heads[catalyst], catalyst);
	}
#if 0
	if((readl(&udc->op_base->ep_complete) & bit_pos) &&
	   (!(readl(&udc->op_base->ep_prime) & bit_pos)) &&
	   (readl(&udc->op_base->ep_status) & bit_pos)) {

		//printk("epno = %u, epdir = %u, ep_dtd = %x dtd_temp = %x\n", epno, epdir, ep_dtd, dtd_temp);
		list_add_tail(&req->queue, &ep->skipped_queue);
	} else {
#endif
		ath_usb_ep_prime(udc, ep_dtd, dtd_temp, bit_pos, catalyst, empty);
#if 0
	}
#endif
	//spin_unlock_irqrestore(&udc->lock, flags);

	return 0;
}

/*
 * Endpoint queue.
 * Queue a request to the endpoint and transer it immediately if possible
 */
static int ath_usb_ep_queue(struct usb_ep *_ep, struct usb_request *_req,
				gfp_t gfp_flags)
{
	struct ath_usb_req *req;
	struct ath_usb_ep *ep;
	struct ath_usb_udc *udc;
	unsigned long flags;
	//__u8 empty;

	ath_usb_debug_fn("__enter %s\n", __func__);

	ath_usb_debug_ep("_ep->name :%s, _ep->maxpacket :%d\n", _ep->name,
			 _ep->maxpacket);

	/* Sanity checks */
	req = container_of(_req, struct ath_usb_req, req);
	if (!_req || !req->req.buf || !list_empty(&req->queue)) {
		ath_usb_error("%s, Invalid Params %p %d, %d\n", __func__,
				_req->buf, _req->length, list_empty(&req->queue));
		return -EINVAL;
	}

	ep = container_of(_ep, struct ath_usb_ep, ep);
	if (!_ep || (!ep->ep_desc && (ep->bEndpointAddress & 0x0F))) {
		ath_usb_error("%s, Invalid Endpoint %p %x\n", __func__,
				ep->ep_desc, ep->bEndpointAddress);
		return -EINVAL;
	}

	udc = ep->udc;
	if (!udc->ga_driver || udc->gadget.speed == USB_SPEED_UNKNOWN) {
		ath_usb_error("%s, Driver Error \n", __func__);
		return -ESHUTDOWN;
	}
	/* If the usb request contains data transfer, then synchronize the buffer
	 * for DMA Transfer */
	if (_req->length) {
		/* DMA for All Trans */
		if (_req->dma == DMA_ADDR_INVALID) {
			_req->dma = dma_map_single(ep->udc->gadget.dev.parent,
						_req->buf, _req->length,
						(ep->bEndpointAddress & USB_DIR_IN) ?
							DMA_TO_DEVICE : DMA_FROM_DEVICE);
			req->mapped = 1;
		} else {
			dma_sync_single_for_device(ep->udc->gadget.dev.parent,
						_req->dma, _req->length,
						(ep->bEndpointAddress & USB_DIR_IN) ?
						DMA_TO_DEVICE : DMA_FROM_DEVICE);
			req->mapped = 0;
		}
	} else {
		_req->dma = DMA_ADDR_INVALID;
	}

	_req->status = -EINPROGRESS;
	_req->actual = 0;

#if 0
	empty = list_empty(&ep->queue);
	if(empty) {
#if 0
		if((ep->bEndpointAddress & 0x0f) > 0)
			printk("S ");
#endif
		ath_usb_start_trans(udc, ep, req);
	} else {
		printk("List not empty\n\n");
	}
#endif

	spin_lock_irqsave (&ap_gadget->lock, flags);
	ath_usb_start_trans(udc, ep, req, 1);
	/*
	 * Add the request to Endpoint queue. If there are no transfers happening
	 * right now, start the current transfer
	 */

	list_add_tail(&req->queue, &ep->queue);
	spin_unlock_irqrestore (&ap_gadget->lock, flags);

	return 0;
}

static int ath_usb_ep_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct ath_usb_ep *ep = container_of(_ep, struct ath_usb_ep, ep);
	struct ath_usb_req *req;
	unsigned long flags;
	__u8 epdir;

	if (!_ep || !_req) {
		return -EINVAL;
	}

	ath_usb_debug_fn("__enter %s\n", __func__);
	ath_usb_debug_ep("_ep->name :%s, _ep->maxpacket :%d\n",
			 _ep->name, _ep->maxpacket);

	spin_lock_irqsave(&ap_gadget->lock, flags);
	/* make sure it's actually queued on this endpoint */
	list_for_each_entry(req, &ep->queue, queue) {
		if (&req->req == _req)
			break;
	}
	if (&req->req != _req) {
		spin_unlock_irqrestore(&ap_gadget->lock, flags);
		return -EINVAL;
	}

	epdir = (ep->bEndpointAddress & USB_DIR_IN) ? USB_SEND : USB_RECV;
	if (ep->queue.next == &req->queue) {
		list_del_init(&req->queue);
		ath_usb_complete_transfer(ep, req, epdir, -ECONNRESET);
	}
	spin_unlock_irqrestore(&ap_gadget->lock, flags);
	return 0;
}

static void ath_usb_complete_transfer(struct ath_usb_ep *ep, struct ath_usb_req *req,
	__u8 epdir, int status)
{
	ath_usb_debug_fn("__enter %s\n", __func__);

	complete_count++;
	//list_del_init(&req->queue);
	//if(((ep->bEndpointAddress & 0x0f) == 1) || ((ep->bEndpointAddress & 0x0f) == 2))
	//printk("%s ep_no :%d  dir = %u status = %d ep_dtd = %x entry = %d\n", __func__, (ep->bEndpointAddress & 0x0f), epdir, status, req->ep_dtd, list_empty(&ep->queue));
#if 0
	if ((ep->bEndpointAddress & 0x0f))
		printk("ep_no :%d \n", (ep->bEndpointAddress & 0x0f));
#endif

	if (req->req.status == -EINPROGRESS) {
		req->req.status = status;
	} else {
		status = req->req.status;
	}

	if (req->req.length) {
		if (req->mapped) {
			dma_unmap_single(ep->udc->gadget.dev.parent,
					 req->req.dma, req->req.length,
					 (ep->bEndpointAddress & USB_DIR_IN)
					 ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
			req->req.dma = DMA_ADDR_INVALID;
			req->mapped = 0;
		} else {
			dma_sync_single_for_cpu(ep->udc->gadget.dev.parent,
						req->req.dma, req->req.length,
						(ep->bEndpointAddress & USB_DIR_IN)
						? DMA_TO_DEVICE : DMA_FROM_DEVICE);
		}
	}
	if(status != 0) {
		spin_unlock(&ep->udc->lock);
		if (req->req.complete) {
			req->req.complete(&ep->ep, &req->req);
		}
		spin_lock(&ep->udc->lock);
	}
}

static int ath_usb_udc_get_frame(struct usb_gadget *_gadget)
{
	ath_usb_debug_fn("__enter %s\n", __func__);
	return 0;
}

static int ath_usb_udc_wakeup(struct usb_gadget *_gadget)
{
#ifdef CONFIG_USB_ATH_OTG
	struct ath_usb_udc *udc = ap_gadget;
#endif
	ath_usb_debug_fn("__enter %s\n", __func__);

#ifdef CONFIG_USB_ATH_OTG
	if (readl(&udc->op_base->portscx[0]) & ATH_USB_PORTSCX_PORT_SUSPEND) {
		/*TODO: Do Remote Wake up */
	} else {
#if 0
		if (udc->transceiver) {
			otg_start_srp(udc->transceiver);
		}
#endif
	}
#endif
	return 0;
}

static int ath_usb_udc_pullup(struct usb_gadget *_gadget, int is_active)
{
	struct ath_usb_udc *udc = ap_gadget;
	ath_usb_debug_fn("__enter %s\n", __func__);
	if (is_active) {
		ath_usb_start_udc(udc);
	} else {
		ath_usb_stop_udc(udc);
		ath_usb_stop_activity(udc);
	}
	return 0;
}

static int ath_usb_udc_vbus_session(struct usb_gadget *_gadget, int is_active)
{
	struct ath_usb_udc *udc = ap_gadget;
	ath_usb_debug_fn("__enter %s\n", __func__);
	if (is_active) {
		ath_usb_start_udc(udc);
	} else {
		printk("VBUS Reset\n");
		ath_usb_stop_udc(udc);
		ath_usb_stop_activity(udc);
	}
	return 0;
}

static int ath_usb_ep_set_halt(struct usb_ep *_ep, int value)
{
	ath_usb_debug_fn("__enter %s\n", __func__);
	return 0;
}

static const struct usb_gadget_ops ath_usb_udc_ops = {
	.get_frame	= ath_usb_udc_get_frame,
	.wakeup		= ath_usb_udc_wakeup,
	.vbus_session	= ath_usb_udc_vbus_session,
	.pullup		= ath_usb_udc_pullup,
};

static struct usb_ep_ops ath_usb_ep_ops = {
	.enable		= ath_usb_ep_enable,
	.disable	= ath_usb_ep_disable,
	.alloc_request	= ath_usb_alloc_request,
	.free_request	= ath_usb_free_request,
	//.alloc_buffer	= ath_usb_alloc_buffer,
	//.free_buffer 	= ath_usb_free_buffer,
	.queue		= ath_usb_ep_queue,
	.dequeue	= ath_usb_ep_dequeue,
	.set_halt	= ath_usb_ep_set_halt,
};

/* Used by EP0 status phase */
static void ath_usb_send_data(struct ath_usb_udc *udc, __u8 epno, __u8 * buff,
			__u32 size)
{
	struct usb_request *_req;
	unsigned long flags;

	ath_usb_debug_fn("__enter %s\n", __func__);
	_req = udc->ctrl_req;
	_req->zero = 0;
	if (size) {
		memcpy(_req->buf, buff, size);
	}
	_req->length = size;
	if (ath_usb_ep_queue(&udc->ep[1].ep, _req, GFP_ATOMIC) < 0) {
		ath_usb_error("send setup phase failed\n");
		spin_lock_irqsave(&udc->lock, flags);
		ath_usb_udc_ep_wipe(&udc->ep[1], -ESHUTDOWN);
		spin_unlock_irqrestore(&udc->lock, flags);
	}
}

/* Used by EP0 status phase */
static void usb_recv_data(struct ath_usb_udc *udc, __u8 epno, __u8 * buff,
			__u32 size)
{
	struct usb_request *_req;
	unsigned long flags;

	ath_usb_debug_fn("__enter %s\n", __func__);
	_req = udc->ctrl_req;
	_req->zero = 0;
	_req->length = size;
	if (ath_usb_ep_queue(&udc->ep[0].ep, _req, GFP_ATOMIC) < 0) {
		ath_usb_error("recv setup phase failed\n");
		spin_lock_irqsave(&udc->lock, flags);
		ath_usb_udc_ep_wipe(&udc->ep[0], -ESHUTDOWN);
		spin_unlock_irqrestore(&udc->lock, flags);
	}
}

void read_setup_data(struct ath_usb_udc *udc, __u8 * dest, int noBytes)
{
	struct ep_qhead *qHead;
	int cal, read_safe;

	ath_usb_debug_fn("__enter %s\n", __func__);

	qHead = udc->ep_queue_head;
	/* if semaphore mechanism is used the following code is compiled in */
	read_safe = 0;
	while (!read_safe) {
		/* CI 8.4.3.1.2 step 2 */
		writel((readl(&udc->op_base->usbcmd) | ATH_USB_CMD_SETUP_TRIPWIRE_SET),
			&udc->op_base->usbcmd);
		/* CI 8.4.3.1.2 step 3 */
		for (cal = 0; cal < noBytes; cal++) {
			*(dest + cal) = qHead->setup_buff[cal];
		}
		/* CI 8.4.3.1.2 step 4 */
		if (readl(&udc->op_base->usbcmd) & ATH_USB_CMD_SETUP_TRIPWIRE_SET) {
			read_safe = 1;	/* we can proceed exiting out of loop */
		}
	}

	/* CI 8.4.3.1.2 step 5 */
	writel((readl(&udc->op_base->usbcmd) & ATH_USB_CMD_SETUP_TRIPWIRE_CLEAR),
			&udc->op_base->usbcmd);
}

static void ath_usb_stall_endpoint(struct ath_usb_udc *udc, __u8 epno,
				__u8 epdir)
{
	struct ep_qhead *qHead;
	qHead = udc->ep_queue_head + (2 * epno) + epdir;

	ath_usb_debug_fn("__enter %s\n", __func__);
	if (epno > 0)
		printk("__enter %s\n", __func__);

	if (le32_to_cpu(qHead->maxPacketLen) & ATH_USB_EP_QUEUE_HEAD_IOS) {
		writel(readl(&udc->op_base->ep_ctrlx[epno]) |
			(ATH_USB_EPCTRL_TX_EP_STALL | ATH_USB_EPCTRL_RX_EP_STALL),
			&udc->op_base->ep_ctrlx[epno]);
	} else {
		writel(readl(&udc->op_base->ep_ctrlx[epno]) |
			(epdir ? ATH_USB_EPCTRL_TX_EP_STALL : ATH_USB_EPCTRL_RX_EP_STALL),
			&udc->op_base->ep_ctrlx[epno]);
	}
}

static void ath_usb_unstall_endpoint(struct ath_usb_udc *udc, __u8 epno,
				__u8 epdir)
{
	/* Enable the endpoint for Rx or Tx and set the endpoint type */
	writel(readl(&udc->op_base->ep_ctrlx[epno]) &
		(epdir ? ~ATH_USB_EPCTRL_TX_EP_STALL : ~ATH_USB_EPCTRL_RX_EP_STALL),
		&udc->op_base->ep_ctrlx[epno]);
}

static int getStatus(struct ath_usb_udc *udc, struct usb_ctrlrequest *ctrl)
{
	__u16 status;
	__u8 intStatus, ep;

	ath_usb_debug_fn("__enter %s\n", __func__);
	switch (ctrl->bRequestType) {
	case (USB_DIR_IN | USB_RECIP_DEVICE):
		printk("USB_RECIP_DEVICE\n");
		status = udc->usbDevState;
		ath_usb_send_data(udc, 0, (__u8 *) & status, sizeof(status));
		break;

	case (USB_DIR_IN | USB_RECIP_INTERFACE):
		printk("USB_RECIP_INTERFACE\n");
		return 1;
#if 0
		intStatus = USB_IF_ALT[ctrl->wIndex & 0x00ff];
		ath_usb_send_data(udc, 0, (__u8 *) & intStatus, sizeof(intStatus));
#endif
		break;

	case (USB_DIR_IN | USB_RECIP_ENDPOINT):
		printk("USB_RECIP_ENDPOINT\n");
		ep = le16_to_cpu(ctrl->wIndex) & 0x8f;
		/* ep&0x0f ->ep_num;ep&0x80->ep_dir */
		intStatus = readl(&udc->op_base->ep_ctrlx[ep & 0x0f]);
		status = intStatus & ((ep & 0x80) ? 1 : 0);
		ath_usb_send_data(udc, 0, (__u8 *) & status, sizeof(status));
		break;

	default:
		return 1;
#if 0
		ath_usb_stall_endpoint(udc, 0, 0);
#endif
	}
	return 0;
}

static void setFeature(struct ath_usb_udc *udc, struct usb_ctrlrequest *ctrl)
{
	__u16 status;
	__u8 intStatus, ep;

	ath_usb_debug_fn("__enter %s\n", __func__);
	switch (ctrl->bRequestType) {
	case USB_RECIP_DEVICE:
		switch (le16_to_cpu(ctrl->wValue)) {
		case USB_DEVICE_REMOTE_WAKEUP:
			/* Set Remote Wakeup */
			status = udc->usbDevState;
			status |= ATH_USB_REMOTE_WKUP;
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
		if (le16_to_cpu(ctrl->wValue) != 0) {
			ath_usb_stall_endpoint(udc, 0, 0);
		}
		ep = le16_to_cpu(ctrl->wIndex) & 0x8F;
		/* Get Endpoint Status */
		intStatus = readl(&udc->op_base->ep_ctrlx[ep & 0x0f]);
		status = intStatus & ((ep & 0x80) ? 1 : 0);
		/* Set Endpoint Status ; set stall */
		writel(readl(&udc->op_base->ep_ctrlx[ep & 0x0f]) |
			(ATH_USB_EPCTRL_TX_EP_STALL | ATH_USB_EPCTRL_RX_EP_STALL),
			&udc->op_base->ep_ctrlx[ep & 0x0f]);
		break;
	default:
		printk("Stall Endpoints\n");
		ath_usb_stall_endpoint(udc, 0, 0);
	}
	ath_usb_send_data(udc, 0, 0, 0);
}

static void clrFeature(struct ath_usb_udc *udc, struct usb_ctrlrequest *ctrl)
{
	__u16 status;
	__u8 intStatus, ep;

	ath_usb_debug_fn("__enter %s\n", __func__);
	if ((udc->usbState != USB_STATE_CONFIGURED) &&
	    (udc->usbState != USB_STATE_ADDRESS)) {
		ath_usb_stall_endpoint(udc, 0, 0);
		return;
	}

	switch (ctrl->bRequestType) {
	case USB_RECIP_DEVICE:
		if (le16_to_cpu(ctrl->wValue) == 1) {
			/* clear remove wakeup */
			status = udc->usbDevState;
			status &= ~USB_DEVICE_REMOTE_WAKEUP;
			udc->usbDevState = status;
		} else {
			ath_usb_stall_endpoint(udc, 0, 0);
			return;
		}
		break;
	case USB_RECIP_ENDPOINT:
		if (le16_to_cpu(ctrl->wValue) != 0) {
			ath_usb_stall_endpoint(udc, 0, 0);
		}
		ep = le16_to_cpu(ctrl->wIndex) & 0x8F;
		/* Get Endpoint Status */
		intStatus = readl(&udc->op_base->ep_ctrlx[ep & 0x0f]);
		status = intStatus & ((ep & 0x80) ? 1 : 0);
		/* Set Endpoint Status ; unstall */
		writel(readl(&udc->op_base->ep_ctrlx[ep & 0x0f]) &
			~(ATH_USB_EPCTRL_TX_EP_STALL | ATH_USB_EPCTRL_RX_EP_STALL),
			&udc->op_base->ep_ctrlx[ep & 0x0f]);
		break;
	default:
		ath_usb_stall_endpoint(udc, 0, 0);
	}
	ath_usb_send_data(udc, 0, 0, 0);
}

static void setConfiguration(struct ath_usb_udc *udc,
				struct usb_ctrlrequest *ctrl)
{
	__u16 usbStatus, wValue;

	ath_usb_debug_fn("__enter %s\n", __func__);

	wValue = le16_to_cpu(ctrl->wValue);

	if (le16_to_cpu(ctrl->wLength) != 0) {
		ath_usb_stall_endpoint(udc, 0, 0);
		goto end;
	}
	if ((wValue & 0x00ff) == 0) {
		usbStatus = udc->usbState;
		if ((usbStatus == USB_STATE_CONFIGURED) ||
		    (usbStatus == USB_STATE_ADDRESS)) {
			/* Clear current config values */
			udc->usbCurrConfig = 0;
			udc->usbState = USB_STATE_ADDRESS;
		} else {
			ath_usb_stall_endpoint(udc, 0, 0);
		}
		goto end;
	}
	/* Read the current Conf Value */
	usbStatus = udc->usbCurrConfig;
	if (usbStatus != (wValue & 0x00ff)) {
		udc->usbCurrConfig = (wValue & 0x00ff);
		udc->usbState = USB_STATE_CONFIGURED;
		goto end;
	}
	udc->usbState = USB_STATE_CONFIGURED;
end:
	return;
}

/* Hardware assisted SET_ADDRESS */
static void setAddress(struct ath_usb_udc *udc, __u16 addr)
{
	ath_usb_debug_fn("__enter %s Address %d \n", __func__, addr);

	udc->devAddr = addr;
	writel((((__u32) udc->devAddr) << ATH_USB_ADDRESS_BIT_SHIFT |
		(0x01 << (ATH_USB_DEVADDR_USBADRA))), &udc->op_base->devaddr);
	udc->usbState = USB_STATE_ADDRESS;
}

static void handle_ep0_setup(struct ath_usb_udc *udc)
{
	struct usb_ctrlrequest ctrl;
	int status = 0;

	ath_usb_debug_fn("__enter %s\n", __func__);

	/* Clear the bit in ep_setup_stat 8.4.3.1.2 step 1 */
	writel(ATH_USB_CTRL_EP, &udc->op_base->ep_setup_stat);

	read_setup_data(udc, (__u8 *) & ctrl, 8);

	while (readl(&udc->op_base->ep_setup_stat) & ATH_USB_CTRL_EP) {
		//udelay(1);
	}
	if (ctrl.bRequestType & USB_DIR_IN) {
		/* setup phase */
		udc->ep0setup = 1;
	} else {
		udc->ep0setup = 0;
	}
	switch (ctrl.bRequest) {
	case USB_REQ_GET_STATUS:
		//printk ("Get Status \n");
		if (getStatus(udc, &ctrl)) {
			goto cliOper;
		}
		break;
	case USB_REQ_SET_FEATURE:
		//printk ("Set Feature\n");
		setFeature(udc, &ctrl);
		break;
	case USB_REQ_CLEAR_FEATURE:
		//printk ("Clear Feature\n");
		clrFeature(udc, &ctrl);
		break;
	case USB_REQ_SET_CONFIGURATION:
		//printk ("Set Configuration \n");
		setConfiguration(udc, &ctrl);
		goto cliOper;
	case USB_REQ_SET_ADDRESS:
		//printk ("Set Address \n");
		setAddress(udc, le16_to_cpu(ctrl.wValue));
		ath_usb_send_data(udc, 0, 0, 0);
		break;
	default:
		/*Hope Rest all Requests are Handled by CLIENT */
cliOper:
		ath_usb_debug_dev("SETUP %02x.%02x v%04x i%04x l%04x\n",
				ctrl.bRequestType, ctrl.bRequest,
				le16_to_cpu(ctrl.wValue), le16_to_cpu(ctrl.wIndex),
				le16_to_cpu(ctrl.wLength));

		status = udc->ga_driver->setup(&udc->gadget, &ctrl);
	}

	if (status < 0) {
		ath_usb_error("error %d, stalling endpoint\n", status);
		ath_usb_stall_endpoint(udc, 0, 0);
	}
}

static void ath_usb_free_dtd(struct ath_usb_udc *udc, struct ep_dtd *ep_dtd, __u32 index)
{
	list_add_tail(&ep_dtd->tr_list, &udc->dtd_list[index]);
}

static struct ath_usb_req *
ath_usb_retire_dtd(struct ath_usb_udc *udc, struct ep_dtd  *ep_dtd, __u8 epno, __u8 epdir)
{
	__u32 bit_pos, tmp;
	//unsigned long flags;
	struct ath_usb_ep *ep;
	struct ath_usb_req *req;
	struct ep_qhead *ep_QHead;
	//struct ep_dtd *temp;

	bit_pos = (1 << (16 * epdir + epno));
	tmp = (2 * epno + epdir);
	ep = &udc->ep[tmp];

	ep_QHead =udc->ep_queue_head + tmp;

	retire_dtd_count++;
	if (ep_dtd) {
		__u32 size_ioc_status;

		if (epno > 0) {
			if (le32_to_cpu(ep_dtd->size_ioc_status) & ATH_USB_TD_STATUS_ACTIVE) {
				//printk("%s:epno = %d, epdir = %d ep_dtd = %x next = %x QueueH_Curr =%x next = %x Active\n", __func__, epno, epdir, ep_dtd, ep_dtd->next_dtd, ep_QHead->curr_dtd, ep_QHead->next_dtd);
				return NULL;
#if 0
				/*Repriming */
				temp = (struct ep_dtd *)ep_dtd->next_dtd;
				ep_QHead->next_dtd = cpu_to_le32(temp->dtd_dma);
				ep_QHead->size_ioc_int_status = __constant_cpu_to_le32(0);
				writel(bit_pos, &udc->op_base->ep_prime);
#endif
			}
		}

		ath_usb_print_dtd(ep_dtd, epno, epdir, "nit");
		size_ioc_status = le32_to_cpu(ep_dtd->size_ioc_status);
#if 0
		if(size_ioc_status)
			printk("size_ioc_status %x epno %d\n", size_ioc_status, epno);
#endif
		ep_dtd->size_ioc_status = __constant_cpu_to_le32(0);
		ath_usb_free_dtd(udc, ep_dtd, tmp);
		if (!list_empty(&ep->queue)) {
			req = container_of(ep->queue.next, struct ath_usb_req, queue);
			req->req.actual = req->req.length -
				((size_ioc_status >> ATH_USB_TD_LENGTH_BIT_POS) & 0x7FFF);
			actual_data_count += req->req.actual;
			list_del_init(&req->queue);
#if 1
			if(list_empty(&ep->queue)) {
				udc->dtd_heads[tmp] = NULL;
			} else {
				udc->dtd_heads[tmp] = (struct ep_dtd *)(udc->dtd_heads[tmp]->next);
				//printk("dtd_heads = %x, dtd_tails = %x, ep_dtd = %x\n", udc->dtd_heads[tmp], udc->dtd_tails[tmp], ep_dtd);
			}
#endif
			ath_usb_complete_transfer(ep, req, epdir, 0);
		}
	} else {
		printk("Null ep_dtd Err \n");
	}
	if (udc->ep0setup) {
		udc->ep0setup = 0;
		if (epno == 0) {
			usb_recv_data(udc, 0, 0, 0);
		}
	}
	return req;
}

/* Endpoint Transfer Complete interrupt handling */
static void ath_usb_process_USB_Intr(struct ath_usb_udc *udc)
{
	struct ep_qhead *ep_QHead;
	struct ep_dtd *ep_dtd;
	__u8 epno = 0, epdir = 0;
	__u32 setup_stat = 0, bit_pos = 0, tmp, err;
	__u8 epDetect;
	int i, count, prev_count = 0;

	int_count++;
	/* EP0 Setup transfer complete */
	setup_stat = readl(&udc->op_base->ep_setup_stat);
	if (setup_stat & ATH_USB_CTRL_EP) {
		handle_ep0_setup(udc);
	}

	bit_pos = readl(&udc->op_base->ep_complete);

	/* Clear the bit in Registers */
	writel(bit_pos, &udc->op_base->ep_complete);

	if (bit_pos) {
		for (i = 0; i < ATH_USB_MAX_EP; i++) {
			/* Based on the bit position get EP number and direction */
			epDetect = 0;
				epno = i;
			if (bit_pos & (1 << i)) {
				epdir = 0;
				epDetect = 1;
			}
			if (bit_pos & (1 << (i + 16))) {
				if(!epDetect) {
					epdir = 1;
				}
				epDetect++;
			}
			for(;epDetect > 0; epDetect--) {
				count = 0;
				ep_dtd = NULL;
				if(epDetect) {
					unsigned long flags;
					struct ath_usb_req *req;
					struct ath_usb_ep *ep;

					/* Based on EP number and direction Get Queue head and dtd */
					tmp = ((2 * epno) + epdir);
					ep = &udc->ep[tmp];

					ep_QHead = udc->ep_queue_head + tmp;
					/*Searching for all the inactive DTDs*/
					do {
						spin_lock_irqsave(&udc->lock, flags);
						if (!ep_dtd) {
							ep_dtd = udc->dtd_heads[tmp];
							if (ep_dtd) {
								if(le32_to_cpu(ep_dtd->size_ioc_status) & ATH_USB_TD_STATUS_ACTIVE) {
									//printk("Hitting here ep_dtd = %x\n", ep_dtd);
									spin_unlock_irqrestore(&udc->lock, flags);
									break;
								}
							} else {
								//printk("dtd Null tmp = %d, epno = %u, epdir = %u count = %u\n", tmp, epno, epdir);
								spin_unlock_irqrestore(&udc->lock, flags);
								break;
							}
						}
						if (ep_dtd) {
							if ((err = (le32_to_cpu(ep_dtd->size_ioc_status) &
									ATH_USB_TD_ERROR_MASK))) {
								if (err & ATH_USB_TD_STATUS_HALTED) {
									printk("Descp Halted \n");
									ep_QHead->size_ioc_int_status &= cpu_to_le32(~err);
								}
								if (err & 0x20) {
									printk("Data Buffer Err %x \n", err);
								}
								if (err & 0x08) {
									printk("Data Trans Err %x \n", err);
								}
							}
						}
						count++;
						/* Retire dtd & start next transfer */
						req = ath_usb_retire_dtd(udc, ep_dtd, epno, epdir);
						ep_dtd = udc->dtd_heads[tmp];
						spin_unlock_irqrestore(&udc->lock, flags);
						if (req) {
							if (req->req.complete) {
								req->req.complete(&ep->ep, &req->req);
							}
						} /* else {
							printk("bit_pos = %x epno = %u epdir = %u epDetect = %u tmp = %u count = %d prev_count = %d\n", bit_pos, epno, epdir, epDetect, tmp, count, prev_count);
						} */
						if (!ep_dtd) /*If no discriptor in the queue*/
							break;
					} while (!(le32_to_cpu(ep_dtd->size_ioc_status) & ATH_USB_TD_STATUS_ACTIVE));
				}
				epdir = 1;
				prev_count = count;
			}
		}
	}
	return;
}

static void ath_usb_handle_reset(struct ath_usb_udc *udc)
{
	int i;

	ath_usb_debug_fn("__enter %s\n", __func__);
	ath_usb_debug_ps("Port Status %x \n", readl(&udc->op_base->portscx[0]));

	ath_usb_stop_activity(udc);

	if (udc->gadget.speed != USB_SPEED_UNKNOWN) {
		udc->gadget.speed = USB_SPEED_UNKNOWN;
	}

	/* The address bits are past bit 25-31. Set the address */
	setAddress(udc, 0);

	/* Clear all the setup token semaphores */
	writel(readl(&udc->op_base->ep_setup_stat), &udc->op_base->ep_setup_stat);

	/* Clear all the endpoint complete status bits */
	writel(readl(&udc->op_base->ep_complete), &udc->op_base->ep_complete);

	while (readl(&udc->op_base->ep_prime) & 0xFFFFFFFF) {
		/* Wait until all ENDPTPRIME bits cleared */
	}

	/* Write 1s to the Flush register */
	writel(0xffffffff, &udc->op_base->ep_flush);

	/* Unstall all endpoints */
	for (i = 0; i < (ATH_USB_MAX_EP * 2); i++) {
		ath_usb_unstall_endpoint(udc, i, 0);
		ath_usb_unstall_endpoint(udc, i, 1);
	}

	ath_usb_debug_ps("Port Status %x \n", readl(&udc->op_base->portscx[0]));
	if (readl(&udc->op_base->portscx[0]) & ATH_USB_PORTSCX_PORT_RESET) {
		udc->usbState = USB_STATE_POWERED;
	} else {
		ath_usb_init_device(udc);
		ath_usb_setup(udc);
	}
}

static void ath_usb_process_port_change(struct ath_usb_udc *udc)
{
	unsigned int rddata;
	ath_usb_debug_fn("__enter %s\n", __func__);

	if (!(readl(&udc->op_base->portscx[0]) & ATH_USB_PORTSCX_PORT_RESET)) {
		/* Get the speed */
		if (readl(&udc->op_base->portscx[0]) & ATH_USB_PORTSCX_PORT_HIGH_SPEED) {
			udc->gadget.speed = USB_SPEED_HIGH;
		} else {
			udc->gadget.speed = USB_SPEED_FULL;
		}
	}

	if (readl(&udc->op_base->portscx[0]) & ATH_USB_PORTSCX_PORT_SUSPEND) {
		if ((udc->gadget.speed != USB_SPEED_UNKNOWN) &&
		    (udc->ga_driver->suspend)) {
#if 0
			/* Issue PHY Low Power Suspend - Stops the phy clock */
			writel((readl(&udc->op_base->portscx[0]) | (1 << 23)),
				&udc->op_base->portscx[0]);
#endif
			spin_unlock(&udc->lock);
			udc->ga_driver->suspend(&udc->gadget);
			spin_lock(&udc->lock);
			if(is_wasp()) {
				ap_usb_led_off();
			}
		}
	}

	if (!(readl(&udc->op_base->portscx[0]) & ATH_USB_PORTSCX_PORT_SUSPEND)) {
		if ((udc->gadget.speed != USB_SPEED_UNKNOWN) &&
		    (udc->ga_driver->resume)) {
			/* Resume - starts the phy clock(not necessary when host issues resume) */
#if 1
			writel((readl(&udc->op_base->portscx[0]) & ~(1 << 23)),
				&udc->op_base->portscx[0]);
#endif
			spin_unlock(&udc->lock);
			udc->ga_driver->resume(&udc->gadget);
			spin_lock(&udc->lock);
			if(is_wasp()) {
				ap_usb_led_on();
			}
		}
	}
}

#define ATH_USB_SUSPEND_ENTRY_COUNT 	0xff /*TODO: Value needs to be checked*/
#define ATH_USB_SUSPEND_EXIT_COUNT 	2000 /*TODO: Value needs to be checked*/

#define ATH_USB_GPIO_USB_SUSP_POLARITY	(1u << 2)
#define ATH_USB_CHIP_RESET_ON_RESUME	(1u << 1)
#define ATH_USB_MASTER_SUSPEND_ENABLE	(1u)

#define ATH_SELF_REFRESH (ATH_DDR_CTL_BASE+0x110)
#define EN_SELF_REFRESH (1 << 31)
#define EN_AUTO_SF_EXIT (1 << 30)
#define CUT_CLK (1 << 28)
#define WASP_SRAM_BASE 0xbd000000
#define WASP_DEV_PTR ((struct ath_usb **)(WASP_SRAM_BASE+0x1000))
#define TEMP_INT ((unsigned int *)(WASP_SRAM_BASE+0x1020))
#define TEMP_INT1 ((unsigned int *)(WASP_SRAM_BASE+0x1040))

#define ATH_ENABLE_DDR_SELFREFRESH 1
void ath_usb_suspend(void)
{
	//ath_usb_reg_wr(0xb80600b8, (ath_usb_reg_rd(0xb80600b8) | (1u<<31)));
#if 1
	/*
	ath_usb_reg_wr(ATH_USB_RESET,
		(ath_usb_reg_rd(ATH_USB_RESET) &
			~(ATH_USB_RESET_USB_HOST | ATH_USB_RESET_USB_PHY |
			  AR7240_RESET_FULL_CHIP | AR7240_RESET_DDR)) |
			  ATH_USB_RESET_USBSUS_OVRIDE);
	*/
#endif
#ifndef ATH_ENABLE_DDR_SELFREFRESH
	//ath_usb_reg_wr(ATH_USB_RESET, 0xffffffff);
	ath_usb_reg_wr(0xbb000184, (ath_usb_reg_rd(0xbb000184) | (1u<<23)));
	/*TODO: Put all PLLs in Bypass mode and then power down all of them*/
	ath_usb_reg_wr(ATH_USB_SUSPEND_RESUME_CNTR, ((ATH_USB_SUSPEND_ENTRY_COUNT << 24) \
						| ATH_USB_SUSPEND_EXIT_COUNT));
	ath_usb_reg_wr(ATH_USB_DEV_SUSPEND_CTRL, (ath_usb_reg_rd(ATH_USB_DEV_SUSPEND_CTRL) \
								| ATH_USB_CHIP_RESET_ON_RESUME));
	/*TODO: Programs the Switcher to be in Discontinuous Mode*/

	ath_usb_reg_wr(ATH_USB_DEV_SUSPEND_CTRL, (ath_usb_reg_rd(ATH_USB_DEV_SUSPEND_CTRL) \
								| ATH_USB_MASTER_SUSPEND_ENABLE));
	ath_usb_reg_wr(0xb8030004, (ath_usb_reg_rd(0xb8030004) & ~(1u<<4)));
	ath_usb_reg_wr(0xb80600b8, (ath_usb_reg_rd(0xb80600b8) | (1u<<31)));

	while(1);
#else
	ath_usb_reg_wr(&(*WASP_DEV_PTR)->portscx[0], (ath_usb_reg_rd(&(*WASP_DEV_PTR)->portscx[0]) | (1u<<23)));
	ath_usb_reg_wr(ATH_SELF_REFRESH, (ath_usb_reg_rd(ATH_SELF_REFRESH) | (EN_SELF_REFRESH | EN_AUTO_SF_EXIT)));
        ath_usb_reg_rmw_set(ATH_DDR_CLK_CTRL, (1<<2));
        ath_usb_reg_rmw_set(ATH_DDR_CLK_CTRL, (1<<4));
        ath_usb_reg_rmw_set(ATH_DDR_CLK_CTRL, (1<<3));

        ath_usb_reg_wr(ATH_DDR_PLL_CONFIG, (ath_usb_reg_rd(ATH_DDR_PLL_CONFIG) | (1u<<30))); //DDR PLL control register: Bit 30(Power down)
        ath_usb_reg_wr(ATH_PLL_CONFIG, (ath_usb_reg_rd(ATH_PLL_CONFIG) | (1u<<30))); //CPU PLL control register: Bit 30(Power down)
        ath_usb_reg_wr(ATH_USB_CONFIG, (ath_usb_reg_rd(ATH_USB_CONFIG) & ~(1u<<4)));
        ath_usb_reg_wr(ATH_USB_DEV_SUSPEND_CTRL, (ath_usb_reg_rd(ATH_USB_DEV_SUSPEND_CTRL) \
                                                                | (ATH_USB_MASTER_SUSPEND_ENABLE)));
        while(!((ath_usb_reg_rd(&(*WASP_DEV_PTR)->usbsts) & ATH_USB_EHCI_STS_PORT_CHANGE) &&
                (!(ath_usb_reg_rd(&(*WASP_DEV_PTR)->portscx[0]) & ATH_USB_PORTSCX_PORT_SUSPEND))));

        ath_usb_reg_wr(ATH_DDR_PLL_CONFIG, (ath_usb_reg_rd(ATH_DDR_PLL_CONFIG) & ~(1u<<30))); //DDR PLL control register: Bit 30(Power down)
        ath_usb_reg_wr(ATH_PLL_CONFIG, (ath_usb_reg_rd(ATH_PLL_CONFIG) & ~(1u<<30))); //CPU PLL control register: Bit 30(Power down)

        ath_usb_reg_wr(ATH_DDR_CLK_CTRL, 0x01308000); //PLL Bypass register :Bit 2,3,4 (CPU, DDR)
        ath_usb_reg_rmw_clear(ATH_SELF_REFRESH, EN_SELF_REFRESH);
        while((ath_usb_reg_rd(ATH_SELF_REFRESH) & (1<<29)));
        ath_usb_reg_wr(ATH_USB_DEV_SUSPEND_CTRL, (ath_usb_reg_rd(ATH_USB_DEV_SUSPEND_CTRL) \
                                                                & ~(ATH_USB_MASTER_SUSPEND_ENABLE)));

        ath_usb_reg_wr(ATH_USB_CONFIG, (ath_usb_reg_rd(ATH_USB_CONFIG) | (1u<<4)));

#endif
	/*
	ath_usb_reg_wr(0xb80600b8, (ath_usb_reg_rd(0xb80600b8) & ~(1u<<31)));
	ath_usb_reg_wr(ATH_USB_DEV_SUSPEND_CTRL, (ath_usb_reg_rd(ATH_USB_DEV_SUSPEND_CTRL) \
								& ~(ATH_USB_MASTER_SUSPEND_ENABLE)));
	*/
}

void ath_usb_switch_to_sram(void)
{
#if 0
	printk("Going to Suspend mode\n\n");
	void (*foo)(void) = 0xbd000000;
	memcpy(0xbd000000, usb_suspend, 200);
	foo();
	printk("Going to Resume\n\n");
#else
	printk("Skipping Suspend mode\n\n");
#endif
}
/*
 * UDC Driver Interrupt handler.  This is called directly by the UDC or
 * indirectly by OTG driver
 */
static int suspend_flag;
irqreturn_t ath_usb_udc_isr(int irq, void *_udc)
{
	struct ath_usb_udc *udc = (struct ath_usb_udc *)_udc;
	__u32 status = 0, setupstat = 0;

	isr_count++;
	if(!udc){
		printk("udc null condition \n");
		return IRQ_NONE;
	}

	if (!udc->op_base) {
		printk("udc_isr null values %p, %p\n", udc, (udc) ? udc->op_base : NULL);
		return IRQ_NONE;
	}
	/*
	 * Avoid delays during device attach times by handling all interrupts
	 * at once
	 */
	for (;;) {
		status = readl(&udc->op_base->usbsts);
		setupstat = readl(&udc->op_base->ep_setup_stat);

		if (!(status & readl(&udc->op_base->usbintr))) {
			/* Nothing to do - exit */
			break;
		}

		/* Clear all interrupts */
		// writel(status, &udc->op_base->usbsts);

		/* USB Port Reset Event */
		if (status & ATH_USB_EHCI_STS_RESET) {
			ath_usb_debug_int("Port Reset Interrupt\n");
			ath_usb_handle_reset(udc);
		}

		/* USB Port Change Event */
		if (status & ATH_USB_EHCI_STS_PORT_CHANGE) {
			ath_usb_debug_int("Port Change Interrupt\n");
			ath_usb_process_port_change(udc);
		}

		if (status & ATH_USB_EHCI_STS_ERR) {
			ath_usb_error("Error Interrupt\n");
			printk("Error Interrupt\n");
			/* Not Handled */
		}

		if (status & ATH_USB_EHCI_STS_SOF) {
			/* Not Handled - Nothing to do */
		}

		/* Endpoint Transfer Complete Events */
		if (status & ATH_USB_EHCI_STS_INT) {
			ath_usb_debug_int("USB Interrupt\n");
			ath_usb_process_USB_Intr(udc);
		}

		/* USB Port Suspend Event */
		if (status & ATH_USB_EHCI_STS_SUSPEND) {
#if 0
			__u32 otgsc = readl(&udc->op_base->otgsc);
			ath_usb_debug_int("USB Suspend\n");
			if (!(otgsc & (1 << 20))) {
				//printk("OTG_Detach Event \n");
				/* USB Device detach event - inform gadget driver */
				ath_usb_stop_activity(udc);
			} else if (udc->ga_driver->suspend) {
				/* USB Device suspend event - inform gadget driver */
				writel((1 << 20), &udc->op_base->otgsc);
				udc->ga_driver->suspend(&udc->gadget);
			}
			if (suspend_flag)
				ath_usb_switch_to_sram();
			suspend_flag = 1;
#else
			writel ((1 << 20), &udc->op_base->otgsc);
			ath_reg_rmw_set(ATH_GPIO_OUT, (1<<11));
			//ath_usb_handle_reset(udc);
			udc->ga_driver->suspend(&udc->gadget);
			ath_usb_stop_activity(udc);
			//printk("%s: Skipping ATH_USB_EHCI_STS_SUSPEND\n", __func__);
#endif
		}

		/* Clear all interrupts */
		writel(status, &udc->op_base->usbsts);
	}
	return IRQ_HANDLED;
}

/* Endpoint Initialization */
static int ath_usb_endpoint_setup(char *ep_name, __u8 ep_addr, __u8 ep_type,
	__u16 maxPack, struct ath_usb_udc *udc)
{
	struct ath_usb_ep *ep;
	struct usb_ep *_ep;
	struct ep_qhead *ep_QHead;
	__u8 epno, epdir, qh_offset;
	__u32 bits = 0;
	__u32 xferFlags = 0;

	ath_usb_debug_fn("__enter %s\n", __func__);

	/* Get endpoint number and direction */
	epno = ep_addr & 0x0f;
	epdir = (ep_addr & USB_DIR_IN) ? USB_SEND : USB_RECV;
	qh_offset = (2 * epno) + epdir;

	/* Select the Queue Head based on EP number and direction */
	ep = &udc->ep[qh_offset];
	ep_QHead = (udc->ep_queue_head + qh_offset);
	ep->ep_qh = ep_QHead;
	strlcpy(ep->name, ep_name, sizeof ep->name);
	ep->udc = udc;
	ep->bEndpointAddress = ep_addr;
	ep->bmAttributes = ep_type;
	INIT_LIST_HEAD(&ep->queue);
	INIT_LIST_HEAD(&ep->skipped_queue);

	/* Initialize EndPoint queue head params based on EP type */
	switch (ep_type) {
	case USB_ENDPOINT_XFER_ISOC:
		xferFlags = (1 << ATH_USB_EP_QUEUE_HEAD_MULT_POS);
		break;
	case USB_ENDPOINT_XFER_CONTROL:
		xferFlags = ATH_USB_EP_QUEUE_HEAD_IOS;
		break;
	default:
		xferFlags = ATH_USB_EP_QUEUE_HEAD_ZERO_LEN_TER_SEL;
		break;
	}

	/*
	 * We select a default max packet length now.  This is later modified as
	 * required when the endpoint is enabled by gadget drivers
	 */
	ep_QHead->maxPacketLen = cpu_to_le32((maxPack << 16) | xferFlags);

	ath_usb_debug_ep("ep_setup ==> ep%d-%s queue:%d, name:%s, maxlen:%d, "
			 "ctrl:%x\n", epno, ep_direction[epdir], qh_offset,
			 ep->name, maxPack, readl(&udc->op_base->ep_ctrlx[epno]));

	_ep = &ep->ep;
	_ep->name = ep->name;	/* ep- (name, type, direction) */
	_ep->ops = &ath_usb_ep_ops;
	_ep->maxpacket = ep->maxpacket = maxPack;

	/*
	 * Only configure the endpoint properties in the control register, but do
	 * not enable them.  The endpoints are enabled by gadget drivers
	 */
	bits = (((epdir) ? (ATH_USB_EPCTRL_TX_DATA_TOGGLE_RST) :
				(ATH_USB_EPCTRL_RX_DATA_TOGGLE_RST)) |
				(ep_type << (epdir ? (ATH_USB_EPCTRL_TX_EP_TYPE_SHIFT) :
						(ATH_USB_EPCTRL_RX_EP_TYPE_SHIFT))));

	writel((readl(&udc->op_base->ep_ctrlx[epno]) | bits),
		&udc->op_base->ep_ctrlx[epno]);

	ath_usb_debug_ep("ep_setup ==> ep%d-%s queue:%d, name:%s, maxlen:%d, "
			 "ctrl:%x, %x\n", epno, ep_direction[epdir], qh_offset,
			 ep->name, maxPack, readl(&udc->op_base->ep_ctrlx[epno]), bits);

	/* EP0 not added to the gadget endpoint list */
	if (epno > 0) {
		list_add_tail(&_ep->ep_list, &udc->gadget.ep_list);
	}
	return 0;
}

static int ath_usb_setup(struct ath_usb_udc *udc)
{
	ath_usb_debug_fn("__enter %s \n", __func__);

	/*Init EndPoint 0 Properties */
	ath_usb_debug_ep("Init Endpoint 0 \n");
	writel((ATH_USB_EPCTRL_TX_DATA_TOGGLE_RST | ATH_USB_EPCTRL_RX_DATA_TOGGLE_RST),
		&udc->op_base->ep_ctrlx[0]);
	writel((readl(&udc->op_base->ep_ctrlx[0]) &
		~(ATH_USB_EPCTRL_TX_EP_STALL | ATH_USB_EPCTRL_RX_EP_STALL)),
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
	ath_usb_endpoint_setup("ep0out", 0, USB_ENDPOINT_XFER_CONTROL, 64, udc);
	ath_usb_endpoint_setup("ep0in", 0 | USB_DIR_IN,
				USB_ENDPOINT_XFER_CONTROL, 64, udc);

	/* Init Bulk EndPoints, Set Required Block maxBlockSize later */
	ath_usb_endpoint_setup("ep1out-bulk", 1, USB_ENDPOINT_XFER_BULK, 0x400, udc);
	ath_usb_endpoint_setup("ep1in-bulk", 1 | USB_DIR_IN,
				USB_ENDPOINT_XFER_BULK, 0x400, udc);

	ath_usb_endpoint_setup("ep2out-bulk", 2, USB_ENDPOINT_XFER_BULK, 0x400, udc);
	ath_usb_endpoint_setup("ep2in-bulk", 2 | USB_DIR_IN,
				USB_ENDPOINT_XFER_BULK, 0x400, udc);

	ath_usb_endpoint_setup("ep3out-iso", 3, USB_ENDPOINT_XFER_ISOC, 0x400, udc);
	ath_usb_endpoint_setup("ep3in-iso", 3 | USB_DIR_IN,
				USB_ENDPOINT_XFER_ISOC, 0x400, udc);

	ath_usb_endpoint_setup("ep4out-int", 4, USB_ENDPOINT_XFER_INT, 0x400, udc);
	ath_usb_endpoint_setup("ep4in-int", 4 | USB_DIR_IN,
				USB_ENDPOINT_XFER_INT, 0x400, udc);

	return 0;
}

static void ath_usb_udc_release(struct device *dev)
{
	ath_usb_debug_fn("__enter %s\n", __func__);
#if 0
	kfree(ap_gadget);
	ap_gadget = NULL;
#endif
}

static void ath_usb_init_device(struct ath_usb_udc *udc)
{
	ath_usb_debug_fn("__enter %s \n", __func__);

	/*
	 * Device controller Initialization
	 */
	ath_usb_debug_dev("STOP UDC\n");
	writel(~ATH_USB_CMD_RUN_STOP, &udc->op_base->usbcmd);
	udelay(100);

	ath_usb_debug_dev("RESET UDC\n");
	writel(ATH_USB_CMD_CTRL_RESET, &udc->op_base->usbcmd);
	udelay(100);

	ath_usb_debug_dev("Waiting for Reset to complete \n");
	while (readl(&udc->op_base->usbcmd) & ATH_USB_CMD_CTRL_RESET) ;

	ath_usb_debug_dev("Setting Device Mode \n");

	/* Set Device Mode */
//	writel((ATH_USB_SET_DEV_MODE | ATH_USB_MODE_SLOM | ATH_USB_MODE_SDIS), &udc->op_base->usbmode);
	writel((ATH_USB_SET_DEV_MODE | ATH_USB_MODE_SLOM), &udc->op_base->usbmode);

	writel(0, &udc->op_base->ep_setup_stat);

	/* Initialize EndPointList Addr */
	writel(udc->qh_dma, &udc->op_base->ep_list_addr);

#if 0				// TODO OTG
	if (readl(&udc->op_base->hcs_params) &
	    ATH_USB_HCS_PARAMS_PORT_POWER_CONTROL_FLAG) {
		__u32 port_control;
		port_control = readl(&udc->op_base.portscx[0]);
		port_control &= (~EHCI_PORTSCX_W1C_BITS | ~EHCI_PORTSCX_PORT_POWER);
		writel(port_control, &udc->op_base.portscx[0]);
	}
#endif
#if 0
	/* Force to Full Speed - shekar(Nov 29) */
	ath_usb_reg_rmw_set(&udc->op_base->portscx[0], (1 << 24));
	printk("Port Status %x\n", readl(&udc->op_base->portscx[0]));
#endif
	if(is_wasp()) {
		ath_reg_wr(&udc->op_base->tx_filltuning, 
				(ath_reg_rd(&udc->op_base->tx_filltuning) | (0x2 <<16)));
		ath_reg_wr(&udc->op_base->burst_size, \
			(ath_reg_rd(&udc->op_base->burst_size) | (0x20 <<8) | (0x20)));
		ath_reg_wr(0xb80000c4, 0x72224222);
		ath_reg_wr(0xb80000c8, 0x22224222);
	}
	ath_usb_debug_fn("__exit %s \n", __func__);
}

static void ath_usb_udc_mem_free(struct ath_usb_udc *udc)
{
	struct ep_dtd *ep_dtd;
	int count;
	//int ret = 0;
	ath_usb_debug_fn("ath_usb_udc_mem_free \n");
	printk("ath_usb_udc_mem_free \n");
	writel(0, &udc->op_base->ep_list_addr);
	if (udc->dtd_pool) {
		for(count = 0; count < ATH_USB_MAX_EP_IN_SYSTEM; count++) {
			while (!list_empty(&udc->dtd_list[count])) {
				struct list_head *temp;
				temp = udc->dtd_list[count].next;
				ep_dtd = list_entry(temp, struct ep_dtd, tr_list);
				dma_pool_free(udc->dtd_pool, ep_dtd, ep_dtd->dtd_dma);
				list_del(temp);
			}
		}
		dma_pool_destroy(udc->dtd_pool);
		udc->dtd_pool = NULL;
	}

	if (udc->ep_queue_head) {
		dma_free_coherent(udc->dev,
				(sizeof(struct ep_qhead) * ATH_USB_MAX_EP * 2),
				udc->ep_queue_head, udc->qh_dma);
		udc->ep_queue_head = NULL;
	}

	if (udc->ctrl_req) {
		ath_usb_free_request(NULL, udc->ctrl_req);
		udc->ctrl_req = NULL;
	}

	if (udc->ctrl_buf) {
		kfree(udc->ctrl_buf);
		udc->ctrl_buf = NULL;
	}
}

static int ath_usb_udc_mem_init(struct ath_usb_udc *udc)
{
	int count, i;
	struct ep_dtd *ep_dtd;

	/* Allocate pool for device transfer descriptors(DTD) -DTDs are DMA-able */
	udc->dtd_pool = dma_pool_create(
				"udc_dtd",
				udc->dev,
				sizeof(struct ep_dtd),
				(ATH_USB_MAX_EP_IN_SYSTEM * ATH_USB_MAX_DTD) /* byte alignment (for hw parts) */ ,
				4096 * ATH_USB_MAX_DTD /* can't cross 4K */);
	if (!udc->dtd_pool) {
		ath_usb_error("ath_usb_udc: dtd dma_pool_create failure\n");
		return -ENOMEM;
	}

	/* Allocate Queue Heads for transfer -QHs are DMA-able */
	udc->ep_queue_head = dma_alloc_coherent(
					udc->dev,
					(sizeof(struct ep_qhead) * ATH_USB_MAX_EP * 2),
					&udc->qh_dma, 0);
	if (!udc->ep_queue_head) {
		ath_usb_udc_mem_free(udc);
		return -ENOMEM;
	}
	ath_usb_debug_mem("queue head %p %x Allocated\n", udc->ep_queue_head,
				udc->qh_dma);

	printk("queue head %p %x Allocated\n", udc->ep_queue_head,
		udc->qh_dma);
	/* Pre-allocate a transfer request and buffer for EP0 operations */
	udc->ctrl_req = ath_usb_alloc_request(NULL, GFP_ATOMIC);
	udc->ctrl_buf = kmalloc(64, GFP_ATOMIC);
	if (!udc->ctrl_req || !udc->ctrl_buf) {
		ath_usb_udc_mem_free(udc);
		return -ENOMEM;
	}
	udc->ctrl_req->buf = udc->ctrl_buf;

	/* Pre-allocate DTDs */
	for (count = 0; count < ATH_USB_MAX_EP_IN_SYSTEM; count++) {
		for (i = 0; i < ATH_USB_MAX_DTD; i++) {
			dma_addr_t dma;
        
			ep_dtd = dma_pool_alloc(udc->dtd_pool, GFP_ATOMIC, &dma);
			if (ep_dtd == NULL) {
				ath_usb_udc_mem_free(udc);
				return -ENOMEM;
			}
			ath_usb_debug_mem("DTD Alloc %p, %x\n", ep_dtd, dma);
			ep_dtd->dtd_dma = dma;
			list_add_tail(&ep_dtd->tr_list, &udc->dtd_list[count]);
			ep_dtd->size_ioc_status &= cpu_to_le32(~ATH_USB_TD_RESERVED_FIELDS);
			ep_dtd->next_dtd = __constant_cpu_to_le32(ATH_USB_TD_NEXT_TERMINATE);
		}
	}

	return 0;
}

/* Provides a graceful exit for the gadget/udc driver */
static void ath_usb_stop_activity(struct ath_usb_udc *udc)
{
	struct ath_usb_ep *ep = NULL;
	unsigned long flags;

	spin_lock_irqsave(&udc->lock, flags);
	udc->gadget.speed = USB_SPEED_UNKNOWN;

	/* Cancel any EP0 IN/OUT transfers */
	ath_usb_udc_ep_wipe(&udc->ep[0], -ESHUTDOWN);
	ath_usb_udc_ep_wipe(&udc->ep[1], -ESHUTDOWN);

	/* Cancel all other EP IN/OUT transfers used by gadget driver */
	list_for_each_entry(ep, &udc->gadget.ep_list, ep.ep_list) {
		ath_usb_udc_ep_wipe(ep, -ESHUTDOWN);
	}
	spin_unlock_irqrestore(&udc->lock, flags);

	/* Disconnect event to Gadget driver */
	if (udc->ga_driver->disconnect) {
		udc->ga_driver->disconnect(&udc->gadget);
	}
}

/* Start ATH_USB device controller hardware */
static void ath_usb_start_udc(struct ath_usb_udc *udc)
{
	ath_usb_debug_dev("Starting Device Controller ...\n");

	ath_usb_init_device(udc);
	ath_usb_setup(udc);

	/* Enable Interrupts */
	writel((ATH_USB_INTR_INT_EN | ATH_USB_INTR_ERR_INT_EN |
		ATH_USB_INTR_PORT_CHANGE_DETECT_EN | ATH_USB_INTR_RESET_EN |
		/*ATH_USB_INTR_SOF_UFRAME_EN | */ ATH_USB_INTR_DEVICE_SUSPEND),
		&udc->op_base->usbintr);

	/* Start Device Controller */
	writel(ATH_USB_CMD_RUN_STOP, &udc->op_base->usbcmd);
    if (!is_ar933x())
	    ath_reg_rmw_set(ATH_GPIO_FUNCTIONS, ATH_GPIO_FUNCTION_USB_LED);
	udelay(100);
}

/* Stop ATH_USB device controller hardware */
static void ath_usb_stop_udc(struct ath_usb_udc *udc)
{
	ath_usb_debug_dev("Stoping Device Controller ...\n");

	/* Disable Interrupts */
	writel(0, &udc->op_base->usbintr);

	/* Stop Device Controller */
	writel(~ATH_USB_CMD_RUN_STOP, &udc->op_base->usbcmd);
    if (!is_ar933x())
        ath_reg_rmw_clear(ATH_GPIO_FUNCTIONS, ATH_GPIO_FUNCTION_USB_LED);
	suspend_flag = 0;
	udelay(100);
}

/*
 * Gadget driver registration
 * The device controller driver starts the actual operation only after a
 * gadget driver is registered.  This is where we enable the UDC interrupts
 */
int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	struct ath_usb_udc *udc = ap_gadget;
	int ret;

	ath_usb_debug_fn("__enter %s\n", __func__);

	/* Sanity checks */
	if (!udc) {
		ath_usb_error("no udc %p\n", udc);
		return -ENODEV;
	}

	if (!driver || driver->speed != USB_SPEED_HIGH || !driver->bind
	    /*|| !driver->unbind */ || !driver->setup) {
		ath_usb_error("gadget driver does not match udc\n");
		return -EINVAL;
	}

	/* hook up the driver */
	driver->driver.bus = NULL;
	udc->ga_driver = driver;
	udc->gadget.dev.driver = &driver->driver;

	/* Bind the gadget driver */
	ret = driver->bind(&udc->gadget);
	if (ret) {
		ath_usb_error("unable to bind driver %s --> %d\n",
				driver->driver.name, ret);
		udc->ga_driver = NULL;
		udc->gadget.dev.driver = NULL;
		return ret;
	}
#ifdef CONFIG_USB_ATH_OTG
	udc->ath_usb_otg->udc = udc;
	udc->ath_usb_otg->udc_isr = ath_usb_udc_isr;
	/* Enable peripheral mode in OTG */
	if (otg_set_peripheral(&udc->ath_usb_otg->otg, &udc->gadget)) {
		if (driver->unbind) {
			driver->unbind(&udc->gadget);
		}
		udc->gadget.dev.driver = NULL;
		udc->ga_driver = NULL;
		return -EINVAL;
	}
#else
	/* Everything is fine - start the device controller */
	ath_usb_start_udc(udc);
#endif

	ath_usb_debug_fn("__exit %s\n", __func__);
	return 0;
}

EXPORT_SYMBOL(usb_gadget_register_driver);

int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	struct ath_usb_udc *udc = ap_gadget;

	if (!udc) {
		return -ENODEV;
	}

	if (!driver || driver != udc->ga_driver) {
		return -EINVAL;
	}

	ath_usb_stop_udc(udc);
	ath_usb_stop_activity(udc);

#ifdef CONFIG_USB_ATH_OTG
	/* Disable peripheral mode in OTG */
	printk("set peripheral null\n");
	otg_set_peripheral(&udc->ath_usb_otg->otg, NULL);
	udc->ath_usb_otg->udc = NULL;
	udc->ath_usb_otg->udc_isr = NULL;
#endif

	if (driver->unbind) {
		driver->unbind(&udc->gadget);
	}
	udc->gadget.dev.driver = NULL;
	udc->ga_driver = NULL;

	return 0;
}

EXPORT_SYMBOL(usb_gadget_unregister_driver);

static int ath_usb_udc_init(struct ath_usb_udc *udc, struct device *dev)
{
	struct ep_qhead *ep_queue_head;
	int temp;

	udc->dev = dev;
	spin_lock_init(&udc->lock);
	for(temp = 0; temp < ATH_USB_MAX_EP_IN_SYSTEM; temp++)
		INIT_LIST_HEAD(&udc->dtd_list[temp]);
	if (ath_usb_udc_mem_init(udc) != 0) {
		return -ENOMEM;
	}

	/* Initialize all device Q Head */
	ep_queue_head = udc->ep_queue_head;
	ath_usb_debug_dev("QHead Size %x : eTD Size %x \n",
				sizeof(struct ep_qhead), sizeof(struct ep_dtd));

	ath_usb_debug_dev("Initialize Dev Trans Descp \n");
	for (temp = 0; temp < (ATH_USB_MAX_EP * 2); temp++) {
		(ep_queue_head + temp)->maxPacketLen = cpu_to_le32(0x400);
		(ep_queue_head + temp)->next_dtd =
			cpu_to_le32(ATH_USB_EP_QUEUE_HEAD_NEXT_TERMINATE);
	}

	udc->gadget.ops = &ath_usb_udc_ops;
	udc->gadget.ep0 = &udc->ep[1].ep;
	INIT_LIST_HEAD(&udc->gadget.ep_list);
	udc->gadget.speed = USB_SPEED_UNKNOWN;
	udc->gadget.name = device_name;

	device_initialize(&udc->gadget.dev);
	//strcpy(udc->gadget.dev.bus_id, "gadget");
	udc->gadget.dev.release = ath_usb_udc_release;
	udc->gadget.dev.parent = dev;

	ap_gadget = udc;
	ath_usb_debug_dev("UDC %p\n", ap_gadget);

	/* Setup all endpoints */
	ath_usb_setup(udc);
	device_add(&udc->gadget.dev);
	return 0;
}

#ifndef CONFIG_USB_ATH_OTG
static int ath_usb_udc_probe(struct platform_device *pdev)
{
	struct ath_usb_udc *udc;
	void __iomem *reg_base;
	int retval;

	ath_usb_debug_fn("__enter %s \n", __func__);

	udc = (struct ath_usb_udc *)kmalloc(sizeof(struct ath_usb_udc), GFP_ATOMIC);
	if (udc == NULL) {
		ath_usb_error("Unable to allocate udc device\n");
		return -ENOMEM;
	}
	memset(udc, 0, sizeof(struct ath_usb_udc));

	/* Allocate and map resources */
	if (!request_mem_region(pdev->resource[0].start,
				pdev->resource[0].end - pdev->resource[0].start + 1,
				driver_name)) {
		ath_usb_error("ath_usb_udc: controller already in use\n");
		retval = -EBUSY;
		goto err1;
	}

	reg_base = ioremap(pdev->resource[0].start,
			pdev->resource[0].end - pdev->resource[0].start + 1);
	if (!reg_base) {
		ath_usb_error("ath_usb_udc: error mapping memory\n");
		retval = -EFAULT;
		goto err2;
	}

	udc->reg_base = reg_base;
	reg_base += 0x140;
	udc->op_base = reg_base;

	/* Device Initialization - start */
	ath_usb_debug_dev("Device Initialization\n");

#if 0
	/*Setting to 8-bit 6th March */
	ath_usb_reg_rmw_clear(ATH_USB_RESET, ATH_USB_RESET_USB_HOST);
	ath_usb_reg_rmw_set(ATH_USB_RESET, ATH_USB_RESET_USB_PHY); //PHY RESET
#endif

	if (is_wasp() || is_ar7242() || is_ar7241() || is_ar933x()) {
		ath_usb_reg_rmw_set(ATH_USB_RESET, ATH_USB_RESET_USBSUS_OVRIDE);
		mdelay(10);
		ath_usb_reg_wr(ATH_USB_RESET,
				((ath_usb_reg_rd(ATH_USB_RESET) &
				 ~(ATH_USB_RESET_USB_HOST)) |
				ATH_USB_RESET_USBSUS_OVRIDE));
		mdelay(10);
		ath_usb_reg_wr(ATH_USB_RESET,
				((ath_usb_reg_rd(ATH_USB_RESET) &
				 ~(ATH_USB_RESET_USB_PHY)) |
				ATH_USB_RESET_USBSUS_OVRIDE));
		mdelay(10);
	} else {

		ath_usb_reg_rmw_clear(ATH_USB_RESET, ATH_USB_RESET_USB_PHY);	//PHY CLEAR RESET
		ath_usb_debug_dev("ATH_USB_RESET %x \n", ath_usb_reg_rd(ATH_USB_RESET));
		mdelay(10);
		ath_usb_reg_rmw_clear(ATH_USB_RESET, ATH_USB_RESET_USB_HOST);	// 6th March
		mdelay(10);
	}

	/* Setting 16-bit mode */
	ath_usb_reg_rmw_set(&udc->op_base->portscx[0], (1 << 28));
	ath_usb_debug_dev("PORT_STATUS[0] %x\n", readl(&udc->op_base->portscx[0]));
	mdelay(10);

	/* Clear Host Mode */
	if (is_wasp() || is_ar7242() || is_ar7241() || is_ar933x()) {
		ath_usb_reg_rmw_clear(ATH_USB_CONFIG, (1 << 8));
	} else {
		ath_usb_reg_rmw_clear(ATH_USB_CONFIG, (1 << 2));
	}
	ath_usb_debug_dev("Usb Config Reg %x\n", ath_usb_reg_rd(ATH_USB_CONFIG));
	mdelay(10);

	/*Debug Info */
	ath_usb_debug_dev("Platform Device Info:\n");
	ath_usb_debug_dev("pdev->resource[0].start %x\n", pdev->resource[0].start);
	ath_usb_debug_dev("pdev->resource[1].start %u\n", pdev->resource[1].start);
	ath_usb_debug_dev("reg_base :%p udc->op_base :%p\n", reg_base, udc->op_base);

	/* Interrupt Request */
	if ((retval = request_irq(pdev->resource[1].start, ath_usb_udc_isr,
				IRQF_SHARED, driver_name, udc)) != 0) {
		ath_usb_error("request interrupt %x failed\n", pdev->resource[1].start);
		retval = -EBUSY;
		goto err3;
	}

	ath_usb_debug_dev("PORT_STATUS[0] %x\n", readl(&udc->op_base->portscx[0]));
	if (ath_usb_udc_init(udc, &pdev->dev) == 0) {
		return 0;
	}

	free_irq(pdev->resource[1].start, udc);
err3:
	iounmap(reg_base);
err2:
	release_mem_region(pdev->resource[0].start,
				pdev->resource[0].end - pdev->resource[0].start + 1);
err1:
	ap_gadget = NULL;
	kfree(udc);
	return retval;
}

static int ath_usb_udc_remove(struct platform_device *pdev)
{
	struct ath_usb_udc *udc = ap_gadget;

	ath_usb_udc_mem_free(udc);
	free_irq(pdev->resource[1].start, udc);
	iounmap(udc->reg_base);
	release_mem_region(pdev->resource[0].start,
			pdev->resource[0].end - pdev->resource[0].start + 1);
	device_unregister(&udc->gadget.dev);
	ap_gadget = NULL;
	kfree(udc);

	return 0;
}

static struct platform_driver ath_usb_udc_drv = {
	.probe	= ath_usb_udc_probe,
	.remove	= ath_usb_udc_remove,
	.driver	= {
		.name = (char *)driver_name,
		.owner = THIS_MODULE,
	},
};

#else

static int ath_usb_udc_probe(void)
{
	struct ath_usb_otg *ath_usb_otg;
	struct ath_usb_udc *udc;

	ath_usb_debug_fn("__enter %s \n", __func__);

	if ((ath_usb_otg = ath_usb_get_otg()) == NULL) {
		return -ENODEV;
	}

	udc = (struct ath_usb_udc *)kmalloc(sizeof(struct ath_usb_udc), GFP_ATOMIC);
	if (udc == NULL) {
		ath_usb_error("Unable to allocate udc device\n");
		return -ENOMEM;
	}
	memset(udc, 0, sizeof(struct ath_usb_udc));

	udc->ath_usb_otg = ath_usb_otg;
	udc->gadget.is_otg = 1;
	udc->op_base = ath_usb_otg->usb_reg;
	if (ath_usb_udc_init(udc, ath_usb_otg->dev) < 0) {
		kfree(udc);
		return -ENODEV;
	}
	return 0;
}

static int ath_usb_udc_remove(void)
{
	struct ath_usb_udc *udc = ap_gadget;

	if (udc) {
		ath_usb_udc_mem_free(udc);
		udc->ath_usb_otg = NULL;
		device_unregister(&udc->gadget.dev);
		kfree(udc);
	}
	ap_gadget = NULL;

	return 0;
}
#endif

static int ath_usb_udc_read_procmem(char *buf, char **start, off_t offset,
					int count, int *eof, void *data)
{
	return sprintf(buf,
			"Total interrupt count = %li\n"
			"Total complete count = %li\n"
			"Total actual data count = %li\n"
			"DTD alloc count = %li\n"
			"Start transfer count = %li\n"
			"Endpoint queue count = %li\n"
			"Retire dtd count = %li\n"
			"Case1 = %li Case2 = %li\n",
				int_count, complete_count, actual_data_count,
				alloc_init_dtd_count, start_trans_count,
				queue_count, retire_dtd_count,
				case1, case2);
}

static int __init ath_usb_init(void)
{
	ath_usb_debug_fn("__enter %s\n", __func__);
	create_proc_read_entry("udc", 0, ath_usb_proc,
				ath_usb_udc_read_procmem, NULL);
#ifdef CONFIG_MACH_HORNET
    printk("%s: id: %lx\n", __func__, 
        (unsigned long) ar7240_reg_rd(AR7240_REV_ID));
#else
    printk("%s: id: %lx\n", __func__, 
        (unsigned long) ath_reg_rd(ATH_REV_ID));
#endif
#ifdef CONFIG_USB_ATH_OTG
	return (ath_usb_udc_probe());
#else
	return platform_driver_register(&ath_usb_udc_drv);
#endif
}

static void __exit ath_usb_exit(void)
{
	ath_usb_debug_fn("__enter %s\n", __func__);
#ifdef CONFIG_USB_ATH_OTG
	ath_usb_udc_remove();
#else
	platform_driver_unregister(&ath_usb_udc_drv);
#endif
	if (ath_usb_proc) {
		remove_proc_entry("udc", ath_usb_proc);
	}
}

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

arch_initcall(ath_usb_init);
module_exit(ath_usb_exit);
