/*
   Copyright (C) 2011 bg <bg_one@mail.ru>
*/
#ifndef CHAN_DONGLE_PDISCOVERY_H_INCLUDED
#define CHAN_DONGLE_PDISCOVERY_H_INCLUDED

#include "export.h"			/* EXPORT_DECL EXPORT_DEF */

enum INTERFACE_TYPE {
	INTERFACE_TYPE_DATA	= 0,
	INTERFACE_TYPE_VOICE,
//	INTERFACE_TYPE_COM,
	INTERFACE_TYPE_NUMBERS,
};

struct pdiscovery_ports {
	char		* ports[INTERFACE_TYPE_NUMBERS];
};

struct pdiscovery_result {
	char			* imei;
	char			* imsi;
	struct pdiscovery_ports	ports;
};

struct pdiscovery_cache_item;

EXPORT_DECL void pdiscovery_init();
EXPORT_DECL void pdiscovery_fini();
/* return non-zero if found */
EXPORT_DECL int pdiscovery_lookup(const char * device, const char * imei, const char * imsi, char ** dport, char ** aport);
EXPORT_DECL const struct pdiscovery_result * pdiscovery_list_begin(const struct pdiscovery_cache_item ** opaque);
EXPORT_DECL const struct pdiscovery_result * pdiscovery_list_next(const struct pdiscovery_cache_item ** opaque);
EXPORT_DECL void pdiscovery_list_end();

#endif /* CHAN_DONGLE_PDISCOVERY_H_INCLUDED */
