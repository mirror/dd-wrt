
/***

othfilter.h - declarations for the non-TCP filter module
Written by Gerard Paul Java
Copyright (c) Gerard Paul Java 1997
			
***/

#include <sys/types.h>
#include <netinet/udp.h>
#include "ipfilter.h"

struct filterstate {
    char filename[FLT_FILENAME_MAX];
    int filtercode;
    struct filterlist fl;

    unsigned int arp:1, rarp:1, nonip:1, padding:13;
};

void config_filters(struct filterstate *filter);
void loadfilters(struct filterstate *filter);
void savefilters(struct filterstate *filter);
int nonipfilter(struct filterstate *filter, unsigned int protocol);
