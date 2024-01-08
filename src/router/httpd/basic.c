/*
 * Basic skin (shtml)
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: basic.c,v 1.1 2005/09/26 16:58:15 seg Exp $
 */

#include <stdio.h>
#include <httpd.h>

struct mime_handler mime_handlers[] = {
	{ "**.htm", "text/html", NULL, NULL, do_file, NULL },
	{ "**.html", "text/html", NULL, NULL, do_file, NULL },
	{ "**.gif", "image/gif", NULL, NULL, do_file, NULL },
	{ "**.jpg", "image/jpeg", NULL, NULL, do_file, NULL },
	{ "**.jpeg", "image/gif", NULL, NULL, do_file, NULL },
	{ "**.png", "image/png", NULL, NULL, do_file, NULL },
	{ "**.css", "text/css", NULL, NULL, do_file, NULL },
	{ "**.au", "audio/basic", NULL, NULL, do_file, NULL },
	{ "**.wav", "audio/wav", NULL, NULL, do_file, NULL },
	{ "**.avi", "video/x-msvideo", NULL, NULL, do_file, NULL },
	{ "**.mov", "video/quicktime", NULL, NULL, do_file, NULL },
	{ "**.mpeg", "video/mpeg", NULL, NULL, do_file, NULL },
	{ "**.vrml", "model/vrml", NULL, NULL, do_file, NULL },
	{ "**.midi", "audio/midi", NULL, NULL, do_file, NULL },
	{ "**.mp3", "audio/mpeg", NULL, NULL, do_file, NULL },
	{ "**.pac", "application/x-ns-proxy-autoconfig", NULL, NULL, do_file,
	  NULL },
	{ NULL, NULL, NULL, NULL, NULL, NULL }
};

struct ej_handler ej_handlers[] = { { NULL, NULL } };
