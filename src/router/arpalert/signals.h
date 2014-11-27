/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: arpalert.c 421 2006-11-04 10:56:25Z thierry $
 *
 */

#ifndef __SIGNALS_H__
#define __SIGNALS_H__

// init signals system
void signals_init(void);

// get next signal
void *signals_next(struct timeval *tv);

#endif

