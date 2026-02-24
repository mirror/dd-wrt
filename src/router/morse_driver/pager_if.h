#ifndef _MORSE_PAGER_IF_H_
#define _MORSE_PAGER_IF_H_

/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <linux/skbuff.h>
#include <linux/workqueue.h>

#include "skbq.h"

/* This is a common interface for pagers. It includes common code between the
 * hardware and software implementations for handling interrupts between
 * the chip and host.
 */

#define MORSE_PAGER_FLAGS_DIR_TO_HOST	BIT(0)
#define MORSE_PAGER_FLAGS_DIR_TO_CHIP	BIT(1)
#define MORSE_PAGER_FLAGS_FREE		BIT(2)
#define MORSE_PAGER_FLAGS_POPULATED	BIT(3)

#define MORSE_PAGER_INT_STS(mors)	MORSE_REG_INT1_STS(mors)
#define MORSE_PAGER_INT_EN(mors)	MORSE_REG_INT1_EN(mors)
#define MORSE_PAGER_INT_SET(mors)	MORSE_REG_INT1_SET(mors)
#define MORSE_PAGER_INT_CLR(mors)	MORSE_REG_INT1_CLR(mors)

#define MORSE_PAGER_TRGR_STS(mors)	MORSE_REG_TRGR1_STS(mors)
#define MORSE_PAGER_TRGR_EN(mors)	MORSE_REG_TRGR1_EN(mors)
#define MORSE_PAGER_TRGR_SET(mors)	MORSE_REG_TRGR1_SET(mors)
#define MORSE_PAGER_TRGR_CLR(mors)	MORSE_REG_TRGR1_CLR(mors)

#define MORSE_PAGER_IRQ_MASK(ID)	(1 << (ID))

#define MORSE_PAGER_BYPASS_TX_STATUS_IRQ_NUM		(15)
#define MORSE_PAGER_IRQ_BYPASS_TX_STATUS_AVAILABLE	BIT(MORSE_PAGER_BYPASS_TX_STATUS_IRQ_NUM)
#define MORSE_PAGER_BYPASS_TX_STATUS_FIFO_DEPTH		(4)

#define MORSE_PAGER_BYPASS_CMD_RESP_IRQ_NUM		(29)
#define MORSE_PAGER_IRQ_BYPASS_CMD_RESP_AVAILABLE	BIT(MORSE_PAGER_BYPASS_CMD_RESP_IRQ_NUM)
#define MORSE_PAGER_BYPASS_CMD_RESP_FIFO_DEPTH		(2)

struct morse;
struct morse_pageset;
struct morse_page;

struct morse_pager {
	struct morse *mors;
	struct work_struct work;
	struct morse_skbq mq;

	/* Parent pageset, filled in morse_pageset_init */
	struct morse_pageset *parent;

	u8 id;			/* ID of pager */
	u8 flags;		/* Indicate direction of pager */
	int num_pages;		/* Maximum number of pages in this pager */
	int page_size_bytes;

	/* Pager implementation specific data and function pointers */
	const struct morse_pager_ops *ops;
	void *aux_data;
};

struct morse_pager_ops {
	/**
	 * Puts a page into the given pager
	 *
	 * @pager: Pointer to pager instance to insert page into
	 * @page: Page to insert
	 *
	 * @return: Error code
	 */
	int (*put)(struct morse_pager *pager, struct morse_page *page);

	/**
	 * Pops a page from the given pager
	 *
	 * @pager: Pointer to pager instance to take page from
	 * @page: Pointer to where to place popped page
	 *
	 * @return: Error code
	 *          Note: If page is not available -EAGAIN is returned
	 */
	int (*pop)(struct morse_pager *pager, struct morse_page *page);

	/**
	 * Notify the pager that there are pages available.
	 *
	 * @pager: Pointer to pager instance to notify
	 *
	 * @return: Error code
	 */
	int (*notify)(const struct morse_pager *pager);

	/**
	 * Writes bytes from an SKB into a page's memory.
	 *
	 * Will raise an error if the number of bytes is
	 * greater than the page size.
	 *
	 * @pager: Pointer to pager instance to take page from
	 * @page: Pointer to page to write into
	 * @offset: Offset in bytes of data in page to write to (must be +ve)
	 * @buff: Buffer to fetch data from
	 * @num_bytes: Number of bytes to write
	 *
	 * @return: Error code
	 */
	int (*write_page)(struct morse_pager *pager, struct morse_page *page,
			  int offset, const char *buff, int num_bytes);

	/**
	 * Reads bytes from a page's memory into an SKB.
	 *
	 * Will raise an error if the number of bytes to read is
	 * greater than the page size.
	 *
	 * @pager: Pointer to pager instance
	 * @page: Pointer to page to fetch data from
	 * @offset: Offset in bytes of data in page to read from (must be +ve)
	 * @buff: Buffer to write into
	 * @num_bytes: Number of bytes to read
	 *
	 * @return: Error code
	 */
	int (*read_page)(struct morse_pager *pager, struct morse_page *page,
			 int offset, char *buff, int num_bytes);

};

/**
 * Initialise the morse pager instance.
 * Does not perform any initialisation of the underlying pager implementation,
 * it is expected you call the implementation specific init on *pager first.
 *
 * @mors: Morse chip instance
 * @pager: Pointer to pager struct to initialise
 * @page_size: Page size in bytes
 * @flags: Pager flags (MORSE_PAGER_FLAGS_xxx)
 * @id: Unique identifier for the pager
 *
 * @return: Error code
 */
int morse_pager_init(struct morse *mors, struct morse_pager *pager, int page_size, u8 flags, u8 id);

/**
 * Prints info about the pager instance to a file.
 *
 * @mors: Morse chip instance
 * @pager: Pointer to pager struct to print
 * @file: Pointer to file to print to
 */
void morse_pager_show(struct morse *mors, struct morse_pager *pager, struct seq_file *file);

/**
 * Cleans up memory used by pager instance.
 *
 * @pager: Pointer to pager struct to delete.
 */
void morse_pager_finish(struct morse_pager *pager);

/**
 * Enables an interrupt for the given pager
 *
 * @pager: Pointer to pager interrupt to enable
 * @enable: Enable (true) or disable (false) pager interrupt
 *
 * @return: Error code
 */
int morse_pager_irq_enable(const struct morse_pager *pager, bool enable);

/**
 * morse_pager_tx_status_irq_enable() - Enables/disables interrupt for tx
 * statuses to bypass the pager
 *
 * @mors Mors object.
 * @enable Set to true for enable.
 *
 * Returns: 0 on success, else error code
 */
int morse_pager_tx_status_irq_enable(struct morse *mors, bool enable);

/**
 * morse_pager_cmd_resp_irq_enable() - Enables/disables interrupt for cmd
 * responses to bypass the pager
 *
 * @mors Mors object.
 * @enable Set to true for enable.
 *
 * Returns: 0 on success, else error code
 */
int morse_pager_cmd_resp_irq_enable(struct morse *mors, bool enable);

/**
 * Default IRQ handler for the pager
 *
 * @mors: Morse chip instance
 * @status: Interrupt status register value
 *
 * @return: Error code
 */
int morse_pager_irq_handler(struct morse *mors, u32 status);

#endif /* !_MORSE_PAGER_IF_H_ */
