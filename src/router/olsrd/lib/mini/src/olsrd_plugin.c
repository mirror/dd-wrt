/*
 * Copyright (c) 2005, Bruno Randolf <bruno.randolf@4g-systems.biz>
 * Copyright (c) 2004, Andreas Tønnesen(andreto-at-olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 * * Neither the name of the UniK olsr daemon nor the names of its contributors 
 *   may be used to endorse or promote products derived from this software 
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* $Id: olsrd_plugin.c,v 1.2 2005/05/29 12:47:41 br1 Exp $ */

 /*
 * Example plugin for olsrd.org OLSR daemon
 * Only the bare minimum
 */


#include <stdio.h>
#include <string.h>

#include "olsrd_plugin.h"
#include "olsr.h"


/****************************************************************************
 *                Functions that the plugin MUST provide                    *
 ****************************************************************************/
 
/**
 * Plugin interface version
 * Used by main olsrd to check plugin interface version
 */
int 
olsrd_plugin_interface_version()
{
	return OLSRD_PLUGIN_INTERFACE_VERSION;
}


/**
 * Register parameters from config file
 * Called for all plugin parameters
 */
int
olsrd_plugin_register_param(char *key, char *value)
{
	if(!strcmp(key, "test")) {
		printf("\n*** MINI: parameter test: %s\n", value);
		return 1;
	}
	return 0;
}


/**
 * Initialize plugin
 * Called after all parameters are passed
 */
int
olsrd_plugin_init()
{
	printf("*** MINI: plugin_init\n");
	
	/* call a function from main olsrd */
	olsr_printf(2, "*** MINI: printed this with olsr_printf\n");
	
	return 1;
}


/****************************************************************************
 *       Optional private constructor and destructor functions              *
 ****************************************************************************/

/* attention: make static to avoid name clashes */

static void __attribute__ ((constructor)) 
my_init(void);

static void __attribute__ ((destructor)) 
my_fini(void);


/**
 * Optional Private Constructor
 */
static void
my_init()
{
	printf("*** MINI: constructor\n");
}


/**
 * Optional Private Destructor
 */
static void
my_fini()
{
	printf("*** MINI: destructor\n");
}
