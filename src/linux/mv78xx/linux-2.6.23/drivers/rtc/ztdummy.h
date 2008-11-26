/*
 * Dummy Zaptel Driver for Zapata Telephony interface
 *
 * Written by Robert Pleh <robert.pleh@hermes.si>
 *
 * Copyright (C) 2002, Hermes Softlab
 *
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
*/

#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,19)
#define USB2420
#endif

struct ztdummy {
	struct zt_span span;
	struct zt_chan chan;
#ifdef LINUX26
	unsigned int counter;
#ifdef USE_RTC
	spinlock_t rtclock;
	rtc_task_t rtc_task;
#endif
#endif
};


#ifndef LINUX26
/* Uhci definitions and structures - from file usb-uhci.h */
#define TD_CTRL_IOC		(1 << 24)	/* Interrupt on Complete */
#define USBSTS 2

typedef enum {
	TD_TYPE, QH_TYPE
} uhci_desc_type_t;

typedef struct {
	__u32 link;
	__u32 status;
	__u32 info;
	__u32 buffer;
} uhci_td_t, *puhci_td_t;


typedef struct {
	__u32 head;
	__u32 element;		/* Queue element pointer */
} uhci_qh_t, *puhci_qh_t;

typedef struct {
	union {
		uhci_td_t td;
		uhci_qh_t qh;
	} hw;
	uhci_desc_type_t type;
	dma_addr_t dma_addr;
	struct list_head horizontal;
	struct list_head vertical;
	struct list_head desc_list;
	int last_used;
} uhci_desc_t, *puhci_desc_t;

typedef struct {
	struct list_head desc_list;	// list pointer to all corresponding TDs/QHs associated with this request
	dma_addr_t setup_packet_dma;
	dma_addr_t transfer_buffer_dma;
	unsigned long started;
#ifdef USB2420
        struct urb *next_queued_urb;    // next queued urb for this EP
        struct urb *prev_queued_urb;
#else
        urb_t *next_queued_urb;         
        urb_t *prev_queued_urb;
#endif
	uhci_desc_t *bottom_qh;
	uhci_desc_t *next_qh;       	// next helper QH
	char use_loop;
	char flags;
} urb_priv_t, *purb_priv_t;

struct virt_root_hub {
	int devnum;		/* Address of Root Hub endpoint */
	void *urb;
	void *int_addr;
	int send;
	int interval;
	int numports;
	int c_p_r[8];
	struct timer_list rh_int_timer;
};

typedef struct uhci {
	int irq;
	unsigned int io_addr;
	unsigned int io_size;
	unsigned int maxports;
	int running;

	int apm_state;

	struct uhci *next;	// chain of uhci device contexts

	struct list_head urb_list;	// list of all pending urbs

	spinlock_t urb_list_lock;	// lock to keep consistency

	int unlink_urb_done;
	atomic_t avoid_bulk;

	struct usb_bus *bus;	// our bus

	__u32 *framelist;
	dma_addr_t framelist_dma;
	uhci_desc_t **iso_td;
	uhci_desc_t *int_chain[8];
	uhci_desc_t *ls_control_chain;
	uhci_desc_t *control_chain;
	uhci_desc_t *bulk_chain;
	uhci_desc_t *chain_end;
	uhci_desc_t *td1ms;
	uhci_desc_t *td32ms;
	struct list_head free_desc;
	spinlock_t qh_lock;
	spinlock_t td_lock;
	struct virt_root_hub rh;	//private data of the virtual root hub
	int loop_usage;            // URBs using bandwidth reclamation

	struct list_head urb_unlinked;	// list of all unlinked  urbs
	long timeout_check;
	int timeout_urbs;
	struct pci_dev *uhci_pci;
	struct pci_pool *desc_pool;
	long last_error_time;          // last error output in uhci_interrupt()
} uhci_t, *puhci_t;
#endif
