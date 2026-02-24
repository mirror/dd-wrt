/*
 * Copyright 2024 Morse Micro
 */
#include <linux/string.h>
#include <linux/ktime.h>
#include "bus_trace.h"

static const char *trace_id_to_string(enum bus_trace_event_id id)
{
	switch (id) {
	case BUS_TRACE_EVENT_ID_SET_REG_BASE_ADDRESS:
		return "set reg base";
	case BUS_TRACE_EVENT_ID_SET_BULK_BASE_ADDRESS:
		return "set bulk base";
	case BUS_TRACE_EVENT_ID_EN_IRQ:
		return "en irq";
	case BUS_TRACE_EVENT_ID_HANDLE_IRQ:
		return "irq";
	case BUS_TRACE_EVENT_ID_RESET_BASE_ADDRESSES:
		return "reset base";
	case BUS_TRACE_EVENT_ID_REG_WRITE:
		return "reg write";
	case BUS_TRACE_EVENT_ID_REG_READ:
		return "reg read";
	case BUS_TRACE_EVENT_ID_BULK_WRITE:
		return "bulk write";
	case BUS_TRACE_EVENT_ID_BULK_READ:
		return "bulk read";
	case BUS_TRACE_EVENT_ID_BUS_EN:
		return "bus en";
	}

	return "unknown";
}

void bus_trace_dump(const struct bus_trace *trace, ktime_t reference)
{
	unsigned int count;
	unsigned int i = trace->trace_index; /* start at oldest */

	for (count = 0; count < ARRAY_SIZE(trace->events); count++) {
		const struct bus_trace_event *event = &trace->events[i];

		pr_info("[%9llu] fn[%d] %-14s : 0x%08x (%4d)",
			ktime_to_us(ktime_sub(reference, event->ts)),
			event->fn,
			trace_id_to_string(event->id),
			event->address,
			event->len);

		i = (i + 1) % ARRAY_SIZE(trace->events);
	}
}

void bus_trace_log(struct bus_trace *trace, enum bus_trace_event_id id,
		   unsigned int fn, unsigned int address, unsigned int len)
{
	unsigned int index = trace->trace_index;

	trace->events[index].ts = ktime_get_boottime();
	trace->events[index].id = id;
	trace->events[index].fn = fn;
	trace->events[index].address = address;
	trace->events[index].len = len;
	trace->trace_index = (index + 1) % ARRAY_SIZE(trace->events);
}

void bus_trace_init(struct bus_trace *trace)
{
	trace->trace_index = 0;
	memset(&trace->events, 0,  sizeof(trace->events));
}
