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

/* $Id: olsrd_plugin.c,v 1.11 2005/05/29 12:47:42 br1 Exp $ */

/*
 * Dynamic linked library for olsr.org olsrd
 */

#include <stdio.h>
#include <string.h>

#include "olsrd_plugin.h"
#include "nameservice.h"


#define PLUGIN_NAME    "OLSRD nameservice plugin"
#define PLUGIN_VERSION "0.2"
#define PLUGIN_AUTHOR   "Bruno Randolf"
#define MOD_DESC PLUGIN_NAME " " PLUGIN_VERSION


static void __attribute__ ((constructor)) 
my_init(void);

static void __attribute__ ((destructor)) 
my_fini(void);


int 
olsrd_plugin_interface_version()
{
	return OLSRD_PLUGIN_INTERFACE_VERSION;
}


int olsrd_plugin_init()
{
	return name_init();
}


static void
my_init()
{
	/* Print plugin info to stdout */
	printf("%s\n", MOD_DESC);

	name_constructor();
  
	return;
}


static void
my_fini()
{
	name_destructor();
}
