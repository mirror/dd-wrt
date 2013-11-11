#ifndef IPTRAF_NG_FLTSELECT_H
#define IPTRAF_NG_FLTSELECT_H

/***

othfilter.h - declarations for the non-TCP filter module

 ***/

#include "fltdefs.h"

struct filterstate {
	char filename[FLT_FILENAME_MAX];
	int filtercode;
	struct filterlist fl;

	unsigned int arp:1, rarp:1, nonip:1, padding:13;
};

extern struct filterstate ofilter;

void config_filters(void);
void loadfilters(void);
void savefilters(void);
int nonipfilter(unsigned int protocol);

#endif	/* IPTRAF_NG_FLTSELECT_H */
