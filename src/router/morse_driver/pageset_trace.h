/*
 * Copyright 2024 Morse Micro
 */
#ifndef _MORSE_PAGESET_TRACE_H_
#define _MORSE_PAGESET_TRACE_H_

#include <linux/types.h>
#include <linux/ktime.h>

enum pageset_trace_event_id {
	PAGESET_TRACE_EVENT_ID_INIT,
	PAGESET_TRACE_EVENT_ID_CACHE_PUT_PAGES,
	PAGESET_TRACE_EVENT_ID_POP_PAGE,
	PAGESET_TRACE_EVENT_ID_CACHE_GET,
	PAGESET_TRACE_EVENT_ID_PUT_PAGE,
	PAGESET_TRACE_EVENT_ID_STORE_PAGE_BULK,
	PAGESET_TRACE_EVENT_ID_NOTIFY,
	PAGESET_TRACE_EVENT_ID_WRITE_PAGE,
	PAGESET_TRACE_EVENT_ID_READ_PAGE,
};

#ifdef CONFIG_MORSE_PAGESET_TRACE
struct pageset_trace_event {
	ktime_t ts;
	u32 flags;
	enum pageset_trace_event_id id;
	unsigned int argument;
};

struct pageset_trace {
	unsigned int trace_index;
	struct pageset_trace_event events[PAGESET_TRACE_DEPTH];
};

struct morse_pageset;
struct morse_pager;

/**
 * pageset_trace_init() - Initialise a pageset's trace event log.
 *
 * @pageset: The pageset to initialise.
 */
void pageset_trace_init(struct morse_pageset *pageset);

/**
 * pageset_trace_log() - Log a trace event on the given pager.
 *
 * @pager: The pager/pageset to log the event on.
 * @id: Event trace id.
 * @argument: Additional argument to supply.
 */
void pageset_trace_log(const struct morse_pager *pager, enum pageset_trace_event_id id,
		       unsigned int argument);

/**
 * pageset_trace_dump() - Print trace event log.
 *
 * @pageset: The pageset that contains the trace event log to dump.
 * @reference: Current kernel boot time reference.
 */
void pageset_trace_dump(const struct morse_pageset *pageset, ktime_t reference);
#else
#define pageset_trace_init(...)
#define pageset_trace_log(...)
#define pageset_trace_dump(...)
#endif

#endif /* _MORSE_PAGESET_TRACE_H_ */
