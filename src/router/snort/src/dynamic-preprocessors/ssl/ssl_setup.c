/*
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2007-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "ssl_setup.h"

#ifdef DUMP_BUFFER
#include "ssl_buffer_dump.h"
#endif

#define SetupSSLPP DYNAMIC_PREPROC_SETUP
const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 1;
const int BUILD_VERSION = 4;
const char *PREPROC_NAME = "SF_SSLPP";

void SetupSSLPP(void)
{
#ifndef SNORT_RELOAD
    _dpd.registerPreproc( "ssl", SSLPP_init);
#else
    _dpd.registerPreproc("ssl", SSLPP_init, SSLReload,
             SSLReloadVerify, SSLReloadSwap, SSLReloadSwapFree);
#endif
#ifdef DUMP_BUFFER
    _dpd.registerBufferTracer(getSSLBuffers, SSL_BUFFER_DUMP_FUNC);
    dumpBufferInit();
#endif

}


