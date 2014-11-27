/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: capture.h 690 2008-03-31 18:36:43Z  $
 *
 */

#ifndef __CAPTURE_H__
#define __CAPTURE_H__

#include <pcap.h>

// init capture system
void cap_init(void);

// get device name and return pointeur to her struct capt
struct capt *cap_get_interface(char *device);
		  
// set bitfield with the capture descriptors
int cap_gen_bitfield(fd_set *bf);

// launch capture system
void cap_sniff(fd_set *bf);

// reset global flood detection
void cap_abus(void);

// return the next timeout
void *cap_next(struct timeval *tv);
		
#endif
