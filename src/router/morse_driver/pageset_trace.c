/*
 * Copyright 2024 Morse Micro
 */
#include <linux/ktime.h>
#include "pageset_trace.h"
#include "morse.h"
#include "firmware.h"

static const char *trace_id_to_string(enum pageset_trace_event_id id)
{
	switch (id) {
	case PAGESET_TRACE_EVENT_ID_INIT:
		return "init";
	case PAGESET_TRACE_EVENT_ID_CACHE_PUT_PAGES:
		return "cache put pages";
	case PAGESET_TRACE_EVENT_ID_POP_PAGE:
		return "pop";
	case PAGESET_TRACE_EVENT_ID_CACHE_GET:
		return "cache get";
	case PAGESET_TRACE_EVENT_ID_PUT_PAGE:
		return "put";
	case PAGESET_TRACE_EVENT_ID_STORE_PAGE_BULK:
		return "store bit bulk";
	case PAGESET_TRACE_EVENT_ID_NOTIFY:
		return "notify";
	case PAGESET_TRACE_EVENT_ID_WRITE_PAGE:
		return "write page";
	case PAGESET_TRACE_EVENT_ID_READ_PAGE:
		return "read page";
	}

	return "unknown";
}

static const char *pager_flags_to_string(u32 flags)
{
	if (flags & MORSE_PAGER_FLAGS_DIR_TO_HOST) {
		return (flags & MORSE_PAGER_FLAGS_FREE) ?
			"to_host(free)" : "to_host(populated)";
	} else if (flags & MORSE_PAGER_FLAGS_DIR_TO_CHIP) {
		return (flags & MORSE_PAGER_FLAGS_FREE) ?
			"from_host(free)" : "from_host(populated)";
	}

	return "unknown";
}

void pageset_trace_dump(const struct morse_pageset *pageset, ktime_t reference)
{
	unsigned int count;
	unsigned int i = pageset->trace.trace_index; /* start at oldest */

	for (count = 0; count < ARRAY_SIZE(pageset->trace.events); count++) {
		const struct pageset_trace_event *event = &pageset->trace.events[i];

		pr_info("[%9llu] %-20s : %15s : 0x%08x",
			ktime_to_us(ktime_sub(reference, event->ts)),
			pager_flags_to_string(event->flags),
			trace_id_to_string(event->id),
			event->argument);

		i = (i + 1) % ARRAY_SIZE(pageset->trace.events);
	}
}

void pageset_trace_log(const struct morse_pager *pager, enum pageset_trace_event_id id,
		       unsigned int argument)
{
	struct morse_pageset *pageset = (struct morse_pageset *)pager->parent;
	unsigned int index = pageset->trace.trace_index;

	pageset->trace.events[index].ts = ktime_get_boottime();
	pageset->trace.events[index].id = id;
	pageset->trace.events[index].flags = pager->flags;
	pageset->trace.events[index].argument = argument;
	pageset->trace.trace_index = (index + 1) % ARRAY_SIZE(pageset->trace.events);
}

void pageset_trace_init(struct morse_pageset *pageset)
{
	pageset->trace.trace_index = 0;
	memset(&pageset->trace.events, 0,  sizeof(pageset->trace.events));
}
