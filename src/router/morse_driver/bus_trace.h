/*
 * Copyright 2024 Morse Micro
 */
#ifndef _MORSE_BUS_TRACE_H_
#define _MORSE_BUS_TRACE_H_

#include <linux/types.h>
#include <linux/ktime.h>

enum bus_trace_event_id {
	BUS_TRACE_EVENT_ID_SET_REG_BASE_ADDRESS,
	BUS_TRACE_EVENT_ID_SET_BULK_BASE_ADDRESS,
	BUS_TRACE_EVENT_ID_RESET_BASE_ADDRESSES,
	BUS_TRACE_EVENT_ID_EN_IRQ,
	BUS_TRACE_EVENT_ID_HANDLE_IRQ,
	BUS_TRACE_EVENT_ID_REG_WRITE,
	BUS_TRACE_EVENT_ID_REG_READ,
	BUS_TRACE_EVENT_ID_BULK_WRITE,
	BUS_TRACE_EVENT_ID_BULK_READ,
	BUS_TRACE_EVENT_ID_BUS_EN,
};

#ifdef CONFIG_MORSE_BUS_TRACE
struct bus_trace_event {
	ktime_t ts;
	enum bus_trace_event_id id;
	unsigned int fn;
	unsigned int address;
	unsigned int len;
};

struct bus_trace {
	unsigned int trace_index;
	struct bus_trace_event events[BUS_TRACE_DEPTH];
};

/**
 * bus_trace_init() - Initialise a bus trace event log.
 *
 * @trace: The bus trace to initialise.
 */
void bus_trace_init(struct bus_trace *trace);

/**
 * bus_trace_log() - Log a trace event for the bus.
 *
 * @trace: The trace to log the event on.
 * @id: Event trace id.
 * @fn: Bus function in use.
 * @address: Address of operation.
 * @len: Length (bytes) of operation.
 */
void bus_trace_log(struct bus_trace *trace, enum bus_trace_event_id id,
		   unsigned int fn, unsigned int address, unsigned int len);

/**
 * bus_trace_dump() - Print trace event log.
 *
 * @trace: The trace event log to dump.
 * @reference: Current kernel boot time reference.
 */
void bus_trace_dump(const struct bus_trace *trace, ktime_t reference);
#else
struct bus_trace {};
#define bus_trace_init(...)
#define bus_trace_log(...)
#define bus_trace_dump(...)
#endif

#endif /* _MORSE_BUS_TRACE_H_ */
