/*
 * OLSRd Quagga plugin
 *
 * Copyright (C) 2006-2008 Immo 'FaUl' Wehrenberg <immo@chaostreff-dortmund.de>
 * Copyright (C) 2007-2010 Vasilis Tsiligiannis <acinonyxs@yahoo.gr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation or - at your option - under
 * the terms of the GNU General Public Licence version 2 but can be
 * linked to any BSD-Licenced Software with public available sourcecode
 *
 */

/* -------------------------------------------------------------------------
 * File               : quagga.h
 * Description        : header file for quagga.c
 * ------------------------------------------------------------------------- */

#include "routing_table.h"

/* Zebra socket */
#ifndef ZEBRA_SOCKPATH
#define ZEBRA_SOCKPATH "/var/run/quagga/zserv.api"
#endif

/* Quagga plugin flags */

void zebra_init(void);
void zebra_fini(void);
int zebra_addroute(const struct rt_entry *);
int zebra_delroute(const struct rt_entry *);
void zebra_redistribute(uint16_t cmd);

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
