/*
 * Copyright 2022-2024 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "linux/jiffies.h"
#include <linux/module.h>
#include <linux/usb.h>

#include "morse.h"
#include "mac.h"
#include "debug.h"
#include "bus.h"

#define MORSE_USB_INTERRUPT_INTERVAL	4	/* High speed USB 2^(4-1) * 125usec = 1msec */
#define USB_MAX_TRANSFER_SIZE		(16 * 1024)	/* Max bytes per USB read/write */
#define MORSE_EP_INT_BUFFER_SIZE	8

/* Define these values to match your devices */
#define MORSE_VENDOR_ID			0x325b
#define MORSE_MM610X_PRODUCT_ID		0x6100
#define MORSE_MM810X_PRODUCT_ID		0x8100

/** Power management runtime auto-suspend delay value in milliseconds */
#define PM_RUNTIME_AUTOSUSPEND_DELAY_MS 100

/**
 * URB timeout in milliseconds. If an URB does not complete within this time, it will be killed.
 *
 * This timeout needs to account for USB suspend and resume occurring before the URB can be
 * transferred, and it also needs to account for transferring USB_MAX_TRANSFER_SIZE bytes over a
 * potentially slow, congested USB Full Speed link.
 */
#define URB_TIMEOUT_MS                  250

#define MORSE_USB_DBG(_m, _f, _a...)		morse_dbg(FEATURE_ID_USB, _m, _f, ##_a)
#define MORSE_USB_INFO(_m, _f, _a...)		morse_info(FEATURE_ID_USB, _m, _f, ##_a)
#define MORSE_USB_WARN(_m, _f, _a...)		morse_warn(FEATURE_ID_USB, _m, _f, ##_a)
#define MORSE_USB_ERR(_m, _f, _a...)		morse_err(FEATURE_ID_USB, _m, _f, ##_a)

enum morse_usb_endpoints {
	MORSE_EP_CMD = 0,	/* Commands endpoint */
	MORSE_EP_INT,		/* IRQ interrupt endpoint */
	MORSE_EP_MEM_RD,	/* Memory read endpoint */
	MORSE_EP_MEM_WR,	/* Memory write endpoint */
	MORSE_EP_REG_RD,	/* Register read endpoint */
	MORSE_EP_REG_WR,	/* Register write endpoint */
	MORSE_EP_EP_MAX,
};

struct morse_usb_endpoint {
	unsigned char *buffer;	/* the buffer to send/receive data */
	struct urb *urb;	/* the urb to read/write data with */
	__u8 addr;		/* Address of endpoint */
	int size;		/* Size of endpoint */
};

enum morse_usb_flags {
	MORSE_USB_FLAG_ATTACHED,
	MORSE_USB_FLAG_SUSPENDED
};

struct morse_usb {
	struct usb_device *udev;		/* the usb device for this device */
	struct usb_interface *interface;	/* the interface for this device */

	/* Morse USB endpoints */
	struct morse_usb_endpoint endpoints[MORSE_EP_EP_MAX];

	/* Track errors in USB callbacks */
	int errors;

	/* protects concurrent access */
	struct mutex lock;

	/* for claim and release bus */
	struct mutex bus_lock;

	/* indicate if CMD urb is in progress */
	bool ongoing_cmd;

	/* for synchronisation between commands and transfers */
	bool ongoing_rw;	/* a command is going on */
	wait_queue_head_t rw_in_wait;	/* to wait for an ongoing command */

	/* Bitmask of flags for state of USB device */
	unsigned long flags;
};

/*
 * Morse USB Read/Write command format
 */
enum morse_usb_command_direction {
	MORSE_USB_WRITE	= 0x00,
	MORSE_USB_READ	= 0x80,
	MORSE_USB_RESET	= 0x02,
};

struct morse_usb_command {
	__le32 dir;		/* Next BULK direction */
	__le32 address;		/* Next BULK address */
	__le32 length;		/* Next BULK size */
} __packet;

/* table of devices that work with this driver */
static const struct usb_device_id morse_usb_table[] = {
	{
	 USB_DEVICE(MORSE_VENDOR_ID, MORSE_MM810X_PRODUCT_ID),
	 .driver_info = (unsigned long)&mm81xx_chip_series },
	{ }			/* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, morse_usb_table);

#ifdef CONFIG_MORSE_USER_ACCESS
struct uaccess *morse_usb_uaccess;
#endif

static void morse_usb_buff_log(struct morse *mors, const char *buf, int length, const char *prefix)
{
	int i, n = 0;
	u8 *hex_buf = NULL;

	if (!morse_log_is_enabled(FEATURE_ID_USB, MORSE_MSG_DEBUG))
		return;

	hex_buf = kzalloc(length * 3, GFP_KERNEL);

	for (i = 0; i < length; i++) {
		sprintf(&hex_buf[n], "%02X ", buf[i]);
		n += 3;
	}

	if (prefix)
		MORSE_USB_DBG(mors, "%s (%d) %s\n", prefix, length, hex_buf);
	else
		MORSE_USB_DBG(mors, "%s\n", hex_buf);

	kfree(hex_buf);
}

static void morse_usb_irq_work(struct work_struct *work)
{
	struct morse *mors = container_of(work, struct morse, usb_irq_work);

	morse_usb_buff_log(mors,
			   ((struct morse_usb *)mors->drv_priv)->endpoints[MORSE_EP_INT].buffer,
			   MORSE_EP_INT_BUFFER_SIZE, "YAPS STAT: ");
	morse_claim_bus(mors);
	morse_hw_irq_handle(mors);
	morse_release_bus(mors);
}

/*
 * See https://www.kernel.org/doc/html/v5.15/driver-api/usb/error-codes.html
 * Error codes returned by in urb->status which indicate disconnect.
 */
static bool morse_usb_urb_status_is_disconnect(const struct urb *urb)
{
	return ((urb->status == -EPROTO) || (urb->status == -EILSEQ) ||
		(urb->status == -ETIME) || (urb->status == -EPIPE));
}

static void morse_usb_int_handler(struct urb *urb)
{
	int ret;
	struct morse *mors = urb->context;
	struct morse_usb *musb = (struct morse_usb *)mors->drv_priv;

	if (!test_bit(MORSE_USB_FLAG_ATTACHED, &musb->flags))
		return;

	if (urb->status) {
		if (morse_usb_urb_status_is_disconnect(urb)) {
			clear_bit(MORSE_USB_FLAG_ATTACHED, &musb->flags);
			set_bit(MORSE_STATE_FLAG_CHIP_UNRESPONSIVE, &mors->state_flags);
			MORSE_USB_INFO(mors, "USB sudden disconnect detected in %s\n", __func__);
			return;
		}

		if (!(urb->status == -ENOENT ||
		      urb->status == -ECONNRESET || urb->status == -ESHUTDOWN))
			MORSE_USB_ERR(mors,
				      "%s - nonzero read status received: %d\n",
				      __func__, urb->status);
	}

	ret = usb_submit_urb(urb, GFP_ATOMIC);

	/* usb_kill_urb has been called */
	if (ret == -EPERM)
		return;
	else if (ret)
		MORSE_USB_ERR(mors, "error: resubmit urb %p err code %d\n", urb, ret);

	queue_work(mors->chip_wq, &mors->usb_irq_work);
}

static int morse_usb_enable_int(struct morse *mors)
{
	int ret = 0;
	struct morse_usb *musb = (struct morse_usb *)mors->drv_priv;
	struct urb *urb;

	if (!test_bit(MORSE_USB_FLAG_ATTACHED, &musb->flags))
		return -ENODEV;

	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb) {
		ret = -ENOMEM;
		goto out;
	}
	musb->endpoints[MORSE_EP_INT].urb = urb;

	musb->endpoints[MORSE_EP_INT].buffer =
	    usb_alloc_coherent(musb->udev, MORSE_EP_INT_BUFFER_SIZE, GFP_KERNEL,
			       &urb->transfer_dma);
	if (!musb->endpoints[MORSE_EP_INT].buffer) {
		MORSE_USB_ERR(mors, "couldn't allocate transfer_buffer\n");
		ret = -ENOMEM;
		goto error_set_urb_null;
	}

	usb_fill_int_urb(musb->endpoints[MORSE_EP_INT].urb, musb->udev,
			 usb_rcvintpipe(musb->udev, musb->endpoints[MORSE_EP_INT].addr),
			 musb->endpoints[MORSE_EP_INT].buffer, MORSE_EP_INT_BUFFER_SIZE,
			 morse_usb_int_handler, mors, MORSE_USB_INTERRUPT_INTERVAL);
	urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	ret = usb_submit_urb(urb, GFP_KERNEL);
	if (ret) {
		MORSE_USB_ERR(mors, "Couldn't submit urb. Error number %d\n", ret);
		goto error;
	}

	return 0;

error:
	usb_free_coherent(musb->udev, MORSE_EP_INT_BUFFER_SIZE,
			  musb->endpoints[MORSE_EP_INT].buffer, urb->transfer_dma);
error_set_urb_null:
	musb->endpoints[MORSE_EP_INT].urb = NULL;
	usb_free_urb(urb);
out:
	return ret;
}

static void morse_usb_int_stop(struct morse *mors)
{
	struct morse_usb *musb = (struct morse_usb *)mors->drv_priv;

	usb_kill_urb(musb->endpoints[MORSE_EP_INT].urb);
	cancel_work_sync(&mors->usb_irq_work);
}

static void morse_usb_cmd_callback(struct urb *urb)
{
	struct morse *mors = urb->context;
	struct morse_usb *musb = (struct morse_usb *)mors->drv_priv;

	MORSE_USB_DBG(mors, "%s status: %d\n", __func__, urb->status);
	/* sync/async unlink faults aren't errors */
	if (urb->status) {
		if (!(urb->status == -ENOENT ||
		      urb->status == -ECONNRESET || urb->status == -ESHUTDOWN))
			MORSE_USB_ERR(mors,
				      "%s - nonzero write bulk status received: %d\n",
				      __func__, urb->status);

		musb->errors = urb->status;
	}

	musb->ongoing_cmd = 0;
	wake_up(&musb->rw_in_wait);
}

static int morse_usb_cmd(struct morse_usb *musb, const char *user_buffer, size_t writesize)
{
	int retval = 0;
	struct morse *mors = usb_get_intfdata(musb->interface);
	struct morse_usb_endpoint *ep = &musb->endpoints[MORSE_EP_CMD];

	if (!test_bit(MORSE_USB_FLAG_ATTACHED, &musb->flags))
		return -ENODEV;

	memcpy(ep->buffer, user_buffer, writesize);

	/* initialize the urb properly */
	usb_fill_bulk_urb(ep->urb, musb->udev, usb_sndbulkpipe(musb->udev, ep->addr),
			  ep->buffer, writesize, morse_usb_cmd_callback, mors);
	ep->urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	musb->ongoing_cmd = 1;
	/* send the data out the bulk port */
	retval = usb_submit_urb(ep->urb, GFP_KERNEL);
	if (retval) {
		MORSE_USB_ERR(mors, "%s - failed submitting write urb, error %d\n",
			      __func__, retval);

		goto error;
	}
	retval = wait_event_interruptible_timeout(musb->rw_in_wait, (!musb->ongoing_cmd),
						  msecs_to_jiffies(URB_TIMEOUT_MS));
	if (retval < 0) {
		MORSE_USB_ERR(mors, "%s: error waiting for urb %d\n", __func__, retval);
		goto error;
	} else if (retval == 0) {
		MORSE_USB_ERR(mors, "%s: timed out waiting for urb\n", __func__);
		usb_kill_urb(ep->urb);
		retval = -ETIMEDOUT;
		goto error;
	}

	musb->ongoing_cmd = 0;

	return writesize;

error:
	musb->ongoing_cmd = 0;
	return retval;
}

/* Non-destructive USB reset */
int morse_usb_ndr_reset(struct morse *mors)
{
	int ret;
	struct morse_usb *musb = (struct morse_usb *)mors->drv_priv;
	struct morse_usb_command cmd;

	mutex_lock(&musb->lock);

	musb->ongoing_rw = 1;
	musb->errors = 0;

	cmd.dir = cpu_to_le32(MORSE_USB_RESET);
	cmd.address = cpu_to_le32(0);
	cmd.length = cpu_to_le32(0);

	ret = morse_usb_cmd(musb, (const char *)&cmd, sizeof(cmd));

	if (ret < 0)
		MORSE_ERR(mors, "morse_usb_cmd (MORSE_USB_RESET) error %d\n", ret);
	else
		ret = 0;

	musb->ongoing_rw = 0;
	mutex_unlock(&musb->lock);

	return ret;
}

static void morse_usb_mem_rw_callback(struct urb *urb)
{
	struct morse *mors = urb->context;
	struct morse_usb *musb = (struct morse_usb *)mors->drv_priv;

	MORSE_USB_DBG(mors, "%s status: %d\n", __func__, urb->status);
	/* sync/async unlink faults aren't errors */
	if (urb->status) {
		if (!(urb->status == -ENOENT ||
		      urb->status == -ECONNRESET || urb->status == -ESHUTDOWN))
			MORSE_USB_ERR(mors,
				      "%s - nonzero write bulk status received: %d\n",
				      __func__, urb->status);
		musb->errors = urb->status;
	}

	musb->ongoing_rw = 0;
	wake_up(&musb->rw_in_wait);
}

static int morse_usb_mem_read(struct morse_usb *musb, u32 address, u8 *data, ssize_t size)
{
	int ret;
	struct morse_usb_command cmd;
	struct morse *mors = usb_get_intfdata(musb->interface);

	if (!test_bit(MORSE_USB_FLAG_ATTACHED, &musb->flags))
		return -ENODEV;

	mutex_lock(&musb->lock);

	musb->ongoing_rw = 1;
	musb->errors = 0;

	/* Send command ahead to prepare for Tokens */
	cmd.dir = cpu_to_le32(MORSE_USB_READ);
	cmd.address = cpu_to_le32(address);
	cmd.length = cpu_to_le32(size);

	morse_usb_buff_log(mors, (const char *)&cmd, sizeof(cmd), "CMDBUF: ");

	ret = morse_usb_cmd(musb, (const char *)&cmd, sizeof(cmd));
	if (ret < 0) {
		MORSE_USB_ERR(mors, "morse_usb_cmd error %d\n", ret);
		goto error;
	}

	/* Let's be fast push the next URB, don't wait until command is done */
	usb_fill_bulk_urb(musb->endpoints[MORSE_EP_MEM_RD].urb,
			  musb->udev,
			  usb_rcvbulkpipe(musb->udev,
					  musb->endpoints[MORSE_EP_MEM_RD].addr),
			  musb->endpoints[MORSE_EP_MEM_RD].buffer,
			  size, morse_usb_mem_rw_callback, mors);

	/* do it */
	ret = usb_submit_urb(musb->endpoints[MORSE_EP_MEM_RD].urb, GFP_ATOMIC);
	if (ret < 0) {
		MORSE_USB_ERR(mors, "%s - failed submitting read urb, error %d\n", __func__, ret);
		ret = (ret == -ENOMEM) ? ret : -EIO;
		goto error;
	}

	ret = wait_event_interruptible_timeout(musb->rw_in_wait, (!musb->ongoing_rw),
					       msecs_to_jiffies(URB_TIMEOUT_MS));
	if (ret < 0) {
		MORSE_USB_ERR(mors, "%s: wait_event_interruptible: error %d\n", __func__, ret);
		goto error;
	} else if (ret == 0) {
		/* Timed out. */
		usb_kill_urb(musb->endpoints[MORSE_EP_MEM_RD].urb);
	}

	if (musb->errors) {
		ret = musb->errors;
		MORSE_USB_ERR(mors, "%s error %d\n", __func__, ret);
		goto error;
	}

	memcpy(data, musb->endpoints[MORSE_EP_MEM_RD].buffer, size);

	morse_usb_buff_log(mors, (const char *)data, size, "RD-DATA: ");

	ret = size;

error:
	musb->ongoing_rw = 0;
	mutex_unlock(&musb->lock);

	return ret;
}

static int morse_usb_mem_write(struct morse_usb *musb, u32 address, u8 *data, ssize_t size)
{
	int ret;
	struct morse_usb_command cmd;
	struct morse *mors = usb_get_intfdata(musb->interface);

	if (!test_bit(MORSE_USB_FLAG_ATTACHED, &musb->flags))
		return -ENODEV;

	mutex_lock(&musb->lock);

	musb->ongoing_rw = 1;
	musb->errors = 0;

	/* Send command ahead to prepare for Tokens */
	cmd.dir = cpu_to_le32(MORSE_USB_WRITE);
	cmd.address = cpu_to_le32(address);
	cmd.length = cpu_to_le32(size);
	ret = morse_usb_cmd(musb, (const char *)&cmd, sizeof(cmd));
	if (ret < 0) {
		MORSE_USB_ERR(mors, "morse_usb_mem_read error %d\n", ret);
		goto error;
	}

	morse_usb_buff_log(mors, (const char *)data, size, "WR-DATA: ");

	memcpy(musb->endpoints[MORSE_EP_MEM_WR].buffer, data, size);

	/* prepare a read */
	usb_fill_bulk_urb(musb->endpoints[MORSE_EP_MEM_WR].urb,
			  musb->udev,
			  usb_sndbulkpipe(musb->udev,
					  musb->endpoints[MORSE_EP_MEM_WR].addr),
			  musb->endpoints[MORSE_EP_MEM_WR].buffer,
			  size, morse_usb_mem_rw_callback, mors);

	/* do it */
	ret = usb_submit_urb(musb->endpoints[MORSE_EP_MEM_WR].urb, GFP_ATOMIC);
	if (ret < 0) {
		MORSE_USB_ERR(mors, "%s - failed submitting write urb, error %d\n",
			      __func__, ret);
		ret = (ret == -ENOMEM) ? ret : -EIO;
		goto error;
	}

	ret = wait_event_interruptible_timeout(musb->rw_in_wait, (!musb->ongoing_rw),
					       msecs_to_jiffies(URB_TIMEOUT_MS));
	if (ret < 0) {
		MORSE_USB_ERR(mors, "%s error %d\n", __func__, ret);
		goto error;
	} else if (ret == 0) {
		/* Timed out. */
		usb_kill_urb(musb->endpoints[MORSE_EP_MEM_WR].urb);
	}

	if (musb->errors) {
		ret = musb->errors;
		MORSE_USB_ERR(mors, "%s error %d\n", __func__, ret);
		goto error;
	}

	ret = size;

error:
	musb->ongoing_rw = 0;
	mutex_unlock(&musb->lock);
	return ret;
}

static int morse_usb_dm_write(struct morse *mors, u32 address, const u8 *data, int len)
{
	ssize_t offset = 0;
	int ret;
	struct morse_usb *musb = (struct morse_usb *)mors->drv_priv;

	if (WARN_ON(len < 0))
		return -EINVAL;

	while (offset < len) {
		/* cast to ssize_t is so x64 build doesn't complain. */
		ret = morse_usb_mem_write(musb, address + offset, (u8 *)(data + offset),
					  min((ssize_t)(len - offset),
					      (ssize_t)USB_MAX_TRANSFER_SIZE));
		if (ret < 0) {
			MORSE_USB_ERR(mors, "%s failed (errno=%d)\n", __func__, ret);
			return ret;
		}
		offset += ret;
	}

	return 0;
}

static int morse_usb_dm_read(struct morse *mors, u32 address, u8 *data, int len)
{
	ssize_t offset = 0;
	int ret;
	struct morse_usb *musb = (struct morse_usb *)mors->drv_priv;

	if (WARN_ON(len < 0))
		return -EINVAL;

	while (offset < len) {
		/* cast to ssize_t is so x64 build doesn't complain. */
		ret = morse_usb_mem_read(musb, address + offset, (u8 *)(data + offset),
					 min((ssize_t)(len - offset),
					     (ssize_t)USB_MAX_TRANSFER_SIZE));

		if (ret < 0) {
			MORSE_USB_ERR(mors, "%s failed (errno=%d)\n", __func__, ret);
			return ret;
		}
		offset += ret;
	}

	return 0;
}

static int morse_usb_reg32_read(struct morse *mors, u32 address, u32 *val)
{
	int ret = 0;
	struct morse_usb *musb = (struct morse_usb *)mors->drv_priv;

	ret = morse_usb_mem_read(musb, address, (u8 *)val, sizeof(*val));
	if (ret == sizeof(*val)) {
		*val = le32_to_cpup((__le32 *)val);
		return 0;
	}

	MORSE_USB_ERR(mors, "%s failed %d\n", __func__, ret);
	return ret;
}

static int morse_usb_reg32_write(struct morse *mors, u32 address, u32 val)
{
	int ret = 0;
	struct morse_usb *musb = (struct morse_usb *)mors->drv_priv;
	__le32 val_le = cpu_to_le32(val);

	ret = morse_usb_mem_write(musb, address, (u8 *)&val_le, sizeof(val_le));
	if (ret == sizeof(val_le))
		return 0;

	MORSE_USB_ERR(mors, "%s failed %d\n", __func__, ret);
	return ret;
}

static void morse_usb_claim_bus(struct morse *mors)
{
	struct morse_usb *musb;

	musb = (struct morse_usb *)mors->drv_priv;
	mutex_lock(&musb->bus_lock);
}

static void morse_usb_release_bus(struct morse *mors)
{
	struct morse_usb *musb;

	musb = (struct morse_usb *)mors->drv_priv;
	mutex_unlock(&musb->bus_lock);
}

static int morse_usb_reset_bus(struct morse *mors)
{
	return 0;
}

static void morse_usb_bus_enable(struct morse *mors, bool enable)
{
	struct morse_usb *musb = (struct morse_usb *)mors->drv_priv;

	if (enable)
		usb_autopm_get_interface(musb->interface);
	else
		usb_autopm_put_interface(musb->interface);
}

static void morse_usb_set_irq(struct morse *mors, bool enable)
{
}

static const struct morse_bus_ops morse_usb_ops = {
	.dm_read = morse_usb_dm_read,
	.dm_write = morse_usb_dm_write,
	.reg32_read = morse_usb_reg32_read,
	.reg32_write = morse_usb_reg32_write,
	.set_bus_enable = morse_usb_bus_enable,
	.claim = morse_usb_claim_bus,
	.release = morse_usb_release_bus,
	.reset = morse_usb_reset_bus,
	.set_irq = morse_usb_set_irq,
	.bulk_alignment = MORSE_DEFAULT_BULK_ALIGNMENT,
};

static int morse_detect_endpoints(struct morse *mors, const struct usb_interface *intf)
{
	int ret;
	struct morse_usb *musb = (struct morse_usb *)mors->drv_priv;
	struct usb_endpoint_descriptor *ep_desc;
	struct usb_host_interface *intf_desc = intf->cur_altsetting;
	unsigned int i;

	for (i = 0; i < morse_compat_desc_num_endpoints(intf_desc->desc); i++) {
		ep_desc = &intf_desc->endpoint[i].desc;

		if (usb_endpoint_is_bulk_in(ep_desc)) {
			/* Assuming all the Endpoints are the same size, Pick Memory first */
			if (!musb->endpoints[MORSE_EP_MEM_RD].addr) {
				musb->endpoints[MORSE_EP_MEM_RD].addr = usb_endpoint_num(ep_desc);
				musb->endpoints[MORSE_EP_MEM_RD].size = usb_endpoint_maxp(ep_desc);
			} else if (!musb->endpoints[MORSE_EP_REG_RD].addr) {
				musb->endpoints[MORSE_EP_REG_RD].addr = usb_endpoint_num(ep_desc);
				musb->endpoints[MORSE_EP_REG_RD].size = usb_endpoint_maxp(ep_desc);
			}
		} else if (usb_endpoint_is_bulk_out(ep_desc)) {
			/* Assuming all the Endpoints are the same size, Pick Memory first */
			if (!musb->endpoints[MORSE_EP_MEM_WR].addr) {
				musb->endpoints[MORSE_EP_MEM_WR].addr = usb_endpoint_num(ep_desc);
				musb->endpoints[MORSE_EP_MEM_WR].size = usb_endpoint_maxp(ep_desc);
			} else if (!musb->endpoints[MORSE_EP_REG_WR].addr) {
				musb->endpoints[MORSE_EP_REG_WR].addr = usb_endpoint_num(ep_desc);
				musb->endpoints[MORSE_EP_REG_WR].size = usb_endpoint_maxp(ep_desc);
			}
		} else if (usb_endpoint_is_int_in(ep_desc)) {
			musb->endpoints[MORSE_EP_INT].addr = usb_endpoint_num(ep_desc);
			musb->endpoints[MORSE_EP_INT].size = usb_endpoint_maxp(ep_desc);
		}
	}

	MORSE_USB_INFO(mors, "\n"
		       "Memory Endpoint IN %s detected: %d size %d\n"
		       "Memory Endpoint OUT %s detected: %d size %d\n"
		       "Register Endpoint IN %s detected: %d\n"
		       "Register Endpoint OUT %s detected: %d\n"
		       "Stats IN endpoint %s detected: %d\n",
		       musb->endpoints[MORSE_EP_MEM_RD].addr ? "" : "not",
		       musb->endpoints[MORSE_EP_MEM_RD].addr, musb->endpoints[MORSE_EP_MEM_RD].size,
		       musb->endpoints[MORSE_EP_MEM_WR].addr ? "" : "not",
		       musb->endpoints[MORSE_EP_MEM_WR].addr, musb->endpoints[MORSE_EP_MEM_WR].size,
		       musb->endpoints[MORSE_EP_REG_RD].addr ? "" : "not",
		       musb->endpoints[MORSE_EP_REG_RD].addr,
		       musb->endpoints[MORSE_EP_REG_WR].addr ? "" : "not",
		       musb->endpoints[MORSE_EP_REG_WR].addr,
		       musb->endpoints[MORSE_EP_INT].addr ? "" : "not",
		       musb->endpoints[MORSE_EP_INT].addr);

	/* Verify we have an IN and OUT */
	if (!(musb->endpoints[MORSE_EP_MEM_RD].addr && musb->endpoints[MORSE_EP_MEM_WR].addr)) {
		ret = -ENODEV;
		goto err;
	}

	/* Verify the stats MORSE_EP_INT is detected */
	if (!musb->endpoints[MORSE_EP_INT].addr) {
		ret = -ENODEV;
		goto err;
	}

	/* Verify minimum interrupt status read */
	if (musb->endpoints[MORSE_EP_INT].size < 8) {
		ret = -ENODEV;
		goto err;
	}

	musb->endpoints[MORSE_EP_CMD].urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!musb->endpoints[MORSE_EP_CMD].urb) {
		ret = -ENOMEM;
		goto err_free_urb;
	}

	musb->endpoints[MORSE_EP_MEM_RD].urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!musb->endpoints[MORSE_EP_MEM_RD].urb) {
		ret = -ENOMEM;
		goto err_free_urb;
	}

	musb->endpoints[MORSE_EP_MEM_WR].urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!musb->endpoints[MORSE_EP_MEM_WR].urb) {
		ret = -ENOMEM;
		goto err_free_urb;
	}

	musb->endpoints[MORSE_EP_MEM_RD].buffer = kmalloc(USB_MAX_TRANSFER_SIZE, GFP_KERNEL);
	if (!musb->endpoints[MORSE_EP_MEM_RD].buffer) {
		ret = -ENOMEM;
		goto err_free_urb;
	}

	musb->endpoints[MORSE_EP_MEM_WR].buffer = kmalloc(USB_MAX_TRANSFER_SIZE, GFP_KERNEL);
	if (!musb->endpoints[MORSE_EP_MEM_WR].buffer) {
		ret = -ENOMEM;
		goto err_free_rd_buff;
	}

	musb->endpoints[MORSE_EP_CMD].buffer =
	    usb_alloc_coherent(musb->udev, sizeof(struct morse_usb_command), GFP_KERNEL,
			       &musb->endpoints[MORSE_EP_CMD].urb->transfer_dma);

	if (!musb->endpoints[MORSE_EP_CMD].buffer) {
		ret = -ENOMEM;
		goto err_free_wr_buff;
	}

	/* Assign command to memory out end point */
	musb->endpoints[MORSE_EP_CMD].addr = musb->endpoints[MORSE_EP_MEM_WR].addr;
	musb->endpoints[MORSE_EP_CMD].size = musb->endpoints[MORSE_EP_MEM_WR].size;

	return 0;

err_free_wr_buff:
	kfree(musb->endpoints[MORSE_EP_MEM_WR].buffer);
err_free_rd_buff:
	kfree(musb->endpoints[MORSE_EP_MEM_RD].buffer);
err_free_urb:
	usb_free_urb(musb->endpoints[MORSE_EP_MEM_WR].urb);
	usb_free_urb(musb->endpoints[MORSE_EP_MEM_RD].urb);
	usb_free_urb(musb->endpoints[MORSE_EP_CMD].urb);
err:
	return ret;
}

static int morse_usb_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	int ret;
	struct morse *mors;
	struct morse_usb *musb;
	struct morse_chip_series *mors_chip_series = (struct morse_chip_series *)id->driver_info;
	const bool reset_hw = false;
	const bool reattach_hw = false;
	/* let the user know what node this device is now attached to */
	dev_info(&interface->dev,
		 "USB Morse device now attached to Morse driver (minor=%d)", interface->minor);

	mors = morse_mac_create(sizeof(*musb), &interface->dev);
	if (!mors) {
		dev_err(&interface->dev, "morse_mac_create failed\n");
		return -ENOMEM;
	}

	mors->bus_ops = &morse_usb_ops;
	mors->bus_type = MORSE_HOST_BUS_TYPE_USB;

	musb = (struct morse_usb *)mors->drv_priv;
	musb->udev = usb_get_dev(interface_to_usbdev(interface));
	musb->interface = usb_get_intf(interface);

	/* save our data pointer in this interface device */
	mutex_init(&musb->lock);
	mutex_init(&musb->bus_lock);
	init_waitqueue_head(&musb->rw_in_wait);
	usb_set_intfdata(interface, mors);

	ret = morse_detect_endpoints(mors, interface);
	if (ret) {
		MORSE_USB_ERR(mors, "morse_detect_endpoints failed (%d)\n", ret);
		goto err_ep;
	}

	set_bit(MORSE_USB_FLAG_ATTACHED, &musb->flags);

	ret = morse_chip_cfg_detect_and_init(mors, mors_chip_series);
	if (ret < 0) {
		MORSE_USB_ERR(mors, "morse_chip_cfg_detect_and_init failed: %d\n", ret);
		goto err_ep;
	}
	MORSE_USB_INFO(mors, "Morse Micro USB device found, chip ID=0x%04x\n", mors->chip_id);

	mors->cfg->mm_ps_gpios_supported = false;

#ifdef CONFIG_MORSE_ENABLE_TEST_MODES
	if (test_mode == MORSE_CONFIG_TEST_MODE_BUS) {
		morse_bus_test(mors, "USB");
		goto usb_test_fin;
	}
#endif

	mors->board_serial = serial;
	MORSE_USB_INFO(mors, "Board serial: %s", mors->board_serial);

	ret = morse_firmware_prepare_and_init(mors, reset_hw, reattach_hw);
	if (ret)
		goto err_ep;

	if (morse_test_mode_is_interactive(test_mode)) {
		mors->chip_wq = create_singlethread_workqueue("MorseChipIfWorkQ");
		if (!mors->chip_wq) {
			MORSE_USB_ERR(mors,
				      "create_singlethread_workqueue(MorseChipIfWorkQ) failed\n");
			ret = -ENOMEM;
			goto err_ep;
		}

		mors->net_wq = create_singlethread_workqueue("MorseNetWorkQ");

		if (!mors->net_wq) {
			MORSE_USB_ERR(mors,
				      "create_singlethread_workqueue(MorseNetWorkQ) failed\n");
			ret = -ENOMEM;
			goto err_net_wq;
		}

		ret = mors->cfg->ops->init(mors);
		if (ret) {
			MORSE_USB_ERR(mors, "chip_if_init failed: %d\n", ret);
			goto err_buffs;
		}

		ret = morse_firmware_parse_extended_host_table(mors);
		if (ret) {
			MORSE_USB_ERR(mors, "failed to parse extended host table: %d\n", ret);
			goto err_host_table;
		}

		INIT_WORK(&mors->usb_irq_work, morse_usb_irq_work);
		morse_usb_enable_int(mors);

		ret = morse_mac_register(mors);
		if (ret) {
			MORSE_USB_ERR(mors, "morse_mac_register failed: %d\n", ret);
			goto err_mac;
		}
	}

#ifdef CONFIG_MORSE_USER_ACCESS
	morse_usb_uaccess = uaccess_alloc();
	if (IS_ERR(morse_usb_uaccess)) {
		MORSE_PR_ERR(FEATURE_ID_USB, "uaccess_alloc() failed\n");
		return PTR_ERR(morse_usb_uaccess);
	}

	ret = uaccess_init(morse_usb_uaccess);
	if (ret) {
		MORSE_PR_ERR(FEATURE_ID_USB, "uaccess_init() failed\n");
		goto err_uaccess;
	}

	if (uaccess_device_register(mors, morse_usb_uaccess, &musb->udev->dev)) {
		MORSE_USB_ERR(mors, "uaccess_device_init() failed.\n");
		goto err_uaccess;
	}
#endif

	/* USB requires remote wakeup functionality for suspend */
	clear_bit(MORSE_USB_FLAG_SUSPENDED, &musb->flags);
	musb->interface->needs_remote_wakeup = 1;
	usb_enable_autosuspend(musb->udev);
	pm_runtime_set_autosuspend_delay(&musb->udev->dev, PM_RUNTIME_AUTOSUSPEND_DELAY_MS);

	usb_autopm_get_interface(interface);
#ifdef CONFIG_MORSE_ENABLE_TEST_MODES
usb_test_fin:
#endif
	return 0;

#ifdef CONFIG_MORSE_USER_ACCESS
err_uaccess:
	uaccess_cleanup(morse_usb_uaccess);
	if (morse_test_mode_is_interactive(test_mode))
		morse_mac_unregister(mors);
#endif
err_mac:
	if (morse_test_mode_is_interactive(test_mode))
		morse_usb_int_stop(mors);
err_host_table:
	if (morse_test_mode_is_interactive(test_mode))
		mors->cfg->ops->finish(mors);
err_buffs:
	if (morse_test_mode_is_interactive(test_mode)) {
		flush_workqueue(mors->net_wq);
		destroy_workqueue(mors->net_wq);
	}
err_net_wq:
	if (morse_test_mode_is_interactive(test_mode)) {
		flush_workqueue(mors->chip_wq);
		destroy_workqueue(mors->chip_wq);
	}
err_ep:
	morse_mac_destroy(mors);
	return ret;
}

static void morse_urb_cleanup(struct morse *mors)
{
	struct morse_usb *musb = (struct morse_usb *)mors->drv_priv;
	struct morse_usb_endpoint *int_ep = &musb->endpoints[MORSE_EP_INT];
	struct morse_usb_endpoint *rd_ep = &musb->endpoints[MORSE_EP_MEM_RD];
	struct morse_usb_endpoint *wr_ep = &musb->endpoints[MORSE_EP_MEM_WR];
	struct morse_usb_endpoint *cmd_ep = &musb->endpoints[MORSE_EP_CMD];

	usb_kill_urb(rd_ep->urb);
	usb_kill_urb(wr_ep->urb);
	usb_kill_urb(cmd_ep->urb);

	/* Locking the bus. No USB communication after this point */
	mutex_lock(&musb->lock);

	if (int_ep->urb)
		usb_free_coherent(musb->udev, MORSE_EP_INT_BUFFER_SIZE,
				  int_ep->buffer, int_ep->urb->transfer_dma);

	if (cmd_ep->urb)
		usb_free_coherent(musb->udev, sizeof(struct morse_usb_command),
				  cmd_ep->buffer, cmd_ep->urb->transfer_dma);

	kfree(wr_ep->buffer);
	kfree(rd_ep->buffer);

	usb_free_urb(int_ep->urb);
	usb_free_urb(wr_ep->urb);
	usb_free_urb(rd_ep->urb);
	usb_free_urb(cmd_ep->urb);
}

static void morse_usb_disconnect(struct usb_interface *interface)
{
	struct morse *mors = usb_get_intfdata(interface);
	struct morse_usb *musb = (struct morse_usb *)mors->drv_priv;
	int minor = interface->minor;
	struct usb_device *udev = interface_to_usbdev(interface);

	if (udev->state == USB_STATE_NOTATTACHED) {
		clear_bit(MORSE_USB_FLAG_ATTACHED, &musb->flags);
		set_bit(MORSE_STATE_FLAG_CHIP_UNRESPONSIVE, &mors->state_flags);
		MORSE_USB_INFO(mors, "USB suddenly unplugged\n");
	}

	usb_disable_autosuspend(usb_get_dev(udev));

	if (test_bit(MORSE_USB_FLAG_SUSPENDED, &musb->flags)) {
		MORSE_USB_INFO(mors, "USB was suspended: release locks\n");
		morse_usb_release_bus(mors);
		mutex_unlock(&musb->lock);
	}

	clear_bit(MORSE_USB_FLAG_SUSPENDED, &musb->flags);

#ifdef CONFIG_MORSE_USER_ACCESS
	uaccess_device_unregister(mors);
	uaccess_cleanup(morse_usb_uaccess);
#endif

	if (morse_test_mode_is_interactive(test_mode)) {
		morse_mac_unregister(mors);
		morse_usb_int_stop(mors);
		mors->cfg->ops->finish(mors);
		flush_workqueue(mors->chip_wq);
		destroy_workqueue(mors->chip_wq);
		flush_workqueue(mors->net_wq);
		destroy_workqueue(mors->net_wq);
	}

	/* No USB communication after this point */
	morse_urb_cleanup(mors);

	if (mors)
		morse_mac_destroy(mors);

	usb_autopm_put_interface(interface);
	usb_set_intfdata(interface, NULL);
	dev_info(&interface->dev, "USB Morse #%d now disconnected", minor);
	usb_put_dev(udev);
}

static int morse_usb_suspend(struct usb_interface *intf, pm_message_t message)
{
	struct morse *mors = usb_get_intfdata(intf);
	struct morse_usb *musb = (struct morse_usb *)mors->drv_priv;
	struct morse_usb_endpoint *int_ep = &musb->endpoints[MORSE_EP_INT];
	struct morse_usb_endpoint *rd_ep = &musb->endpoints[MORSE_EP_MEM_RD];
	struct morse_usb_endpoint *wr_ep = &musb->endpoints[MORSE_EP_MEM_WR];
	struct morse_usb_endpoint *cmd_ep = &musb->endpoints[MORSE_EP_CMD];

	if (!test_bit(MORSE_USB_FLAG_ATTACHED, &musb->flags))
		return -ENODEV;

	usb_kill_urb(int_ep->urb);
	usb_kill_urb(rd_ep->urb);
	usb_kill_urb(wr_ep->urb);
	usb_kill_urb(cmd_ep->urb);

	/* Locking the bus. No USB communication after this point */
	morse_usb_claim_bus(mors);
	mutex_lock(&musb->lock);

	set_bit(MORSE_USB_FLAG_SUSPENDED, &musb->flags);

	MORSE_USB_INFO(mors, "USB suspend\n");

	return 0;
}

static int morse_usb_resume(struct usb_interface *intf)
{
	struct morse *mors = usb_get_intfdata(intf);
	struct morse_usb *musb = (struct morse_usb *)mors->drv_priv;
	int ret;
	struct morse_usb_endpoint *int_ep = &musb->endpoints[MORSE_EP_INT];

	if (!test_bit(MORSE_USB_FLAG_ATTACHED, &musb->flags))
		return -ENODEV;

	ret = usb_submit_urb(int_ep->urb, GFP_KERNEL);
	if (ret)
		MORSE_USB_ERR(mors, "Couldn't submit urb. Error number %d\n", ret);

	morse_usb_release_bus(mors);
	mutex_unlock(&musb->lock);

	clear_bit(MORSE_USB_FLAG_SUSPENDED, &musb->flags);

	MORSE_USB_INFO(mors, "USB resume\n");

	return 0;
}

static int morse_usb_reset_resume(struct usb_interface *intf)
{
	struct morse *mors = usb_get_intfdata(intf);
	struct morse_usb *musb = (struct morse_usb *)mors->drv_priv;
	int ret;
	struct morse_usb_endpoint *int_ep = &musb->endpoints[MORSE_EP_INT];

	if (!test_bit(MORSE_USB_FLAG_ATTACHED, &musb->flags))
		return -ENODEV;

	dev_err(&intf->dev, "Morse USB Reset resume");

	ret = usb_submit_urb(int_ep->urb, GFP_KERNEL);
	if (ret)
		MORSE_USB_ERR(mors, "Couldn't submit urb. Error number %d\n", ret);

	morse_usb_release_bus(mors);
	mutex_unlock(&musb->lock);

	clear_bit(MORSE_USB_FLAG_SUSPENDED, &musb->flags);

	return 0;
}

static int morse_usb_pre_reset(struct usb_interface *intf)
{
	return 0;
}

static int morse_usb_post_reset(struct usb_interface *intf)
{
	return 0;
}

static struct usb_driver morse_usb_driver = {
	.name = "morse_usb",
	.probe = morse_usb_probe,
	.disconnect = morse_usb_disconnect,
	.suspend = morse_usb_suspend,
	.resume = morse_usb_resume,
	.reset_resume = morse_usb_reset_resume,
	.pre_reset = morse_usb_pre_reset,
	.post_reset = morse_usb_post_reset,
	.id_table = morse_usb_table,
	.supports_autosuspend = 1,
	.soft_unbind = 1,
};

int __init morse_usb_init(void)
{
	int ret;

	ret = usb_register(&morse_usb_driver);
	if (ret)
		MORSE_PR_ERR(FEATURE_ID_USB, "usb_register_driver() failed: %d\n", ret);
	return ret;
}

void __exit morse_usb_exit(void)
{
	usb_deregister(&morse_usb_driver);
}
