// SPDX-License-Identifier: GPL-2.0+
/*
 * Conversion routines for schedules
 *
 * Copyright (c) 2020 Heinrich Schuchardt
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sispm_ctl.h"

#define PMS2_BUFFER_SIZE 0x28

static unsigned char *pms2_write_block(uint8_t action, uint32_t time, unsigned char *ptr)
{
	int i;

	*ptr++ = action;
	for (i = 0; i < 4; ++i) {
		*ptr++ = (uint8_t)time;
		time >>= 8;
	}
	return ptr;
}

/**
 * pms2_read_block() - read a single action for EG-PMS2
 *
 * @action:	action to be taken
 * @time:	time when to take the action
 * @ptr:	pointer to current action
 * Return:	pointer to next action
 */
static const unsigned char *pms2_read_block(uint8_t *action, uint32_t *time, const unsigned char *ptr)
{
	int i;

	*action = *ptr++;
	*time = 0;
	for (i = 0; i < 4; ++i) {
		*time >>= 8;
		*time += (uint32_t)*ptr++ << 24;
	}
	return ptr;
}

/**
 * pms2_schedule_to_buffer() - convert schedule to device buffer
 *
 * @schedule:	schedule
 * @buffer:	40 character buffer
 *
 * Return:	0 = success
 */
int pms2_schedule_to_buffer(const struct plannif *schedule, unsigned char *buffer)
{
	unsigned char *ptr = buffer;
	uint32_t loop_ref, start = (uint32_t)schedule->timeStamp;
	unsigned char action;
	int i;

	memset(buffer, 0, PMS2_BUFFER_SIZE);

	ptr = pms2_write_block(3 * schedule->socket + 1, start, ptr);

	for (i = 0; i < 7; ++i) {
		action = schedule->actions[i + 1].switchOn;
		start += 60 * schedule->actions[i].timeForNext;
		if (!i)
			loop_ref = start;
		if (action > 1)
			break;
		ptr = pms2_write_block(action, start, ptr);
	}
	if (action <= 1) {
		fprintf(stderr, "Schedule has too many items\n");
		return -1;
	}
	if (schedule->actions[i].timeForNext)
		start -= loop_ref;
	else
		start = 0;
	pms2_write_block(0xe5, start, ptr);
	if (start) {
		/* set loop flag */
		for (ptr -= 5; ptr > buffer; ptr -= 5)
			*ptr |= 2;
	}
	return 0;
}

/**
 * pms2_buffer_to_schedule() - device buffer to schedule
 *
 * @buffer:	40 character buffer
 * @schedule:	schedule
 */
void pms2_buffer_to_schedule(const unsigned char *buffer, struct plannif *schedule)
{
	const unsigned char *ptr = buffer;
	uint32_t start, last, loop_ref, time;
	unsigned char action;
	int i;

	plannif_reset(schedule);

	ptr = pms2_read_block(&action, &start, ptr);
	schedule->actions[0].switchOn = 0;
	schedule->socket = (action - 1) / 3;
	schedule->timeStamp = start;
	last = start;
	for (i = 0; i < 7; ++i) {
		ptr = pms2_read_block(&action, &time, ptr);
		if (!i)
			loop_ref = time;
		if (action > 3)
			break;
		schedule->actions[i + 1].switchOn = action & 1;
		schedule->actions[i].timeForNext = ((int32_t)time - (int32_t)last) / 60;
		last = time;
	}
	if (time)
		schedule->actions[i].timeForNext = ((int32_t)loop_ref + time - (int32_t)last) / 60;
}
