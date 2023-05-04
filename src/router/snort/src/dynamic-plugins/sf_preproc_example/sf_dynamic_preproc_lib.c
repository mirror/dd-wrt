/* $Id$ */
/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2005-2013 Sourcefire, Inc.
 **
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License Version 2 as
 ** published by the Free Software Foundation.  You may not use, modify or
 ** distribute this program under any other version of the GNU General
 ** Public License.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "sf_dynamic_define.h"
#include "sf_preproc_info.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preproc_lib.h"
#include "sf_dynamic_meta.h"
#include "sf_dynamic_preprocessor.h"
#include "sf_dynamic_common.h"

DynamicPreprocessorData _dpd;

NORETURN void DynamicPreprocessorFatalMessage(const char *format, ...)
{
    char buf[STD_BUF];
    va_list ap;

    va_start(ap, format);
    vsnprintf(buf, STD_BUF, format, ap);
    va_end(ap);

    buf[STD_BUF - 1] = '\0';

    _dpd.fatalMsg("%s", buf);

    exit(1);
}


PREPROC_LINKAGE int InitializePreprocessor(DynamicPreprocessorData *dpd)
{
    if (dpd->version < PREPROCESSOR_DATA_VERSION)
    {
        printf("ERROR version %d < %d\n", dpd->version,
            PREPROCESSOR_DATA_VERSION);
        return -1;
    }

    if (dpd->size != sizeof(DynamicPreprocessorData))
    {
        printf("ERROR size %d != %u\n", dpd->size, (unsigned)sizeof(*dpd));
        return -2;
    }

    _dpd = *dpd;
    DYNAMIC_PREPROC_SETUP();
    return 0;
}

PREPROC_LINKAGE int LibVersion(DynamicPluginMeta *dpm)
{

    dpm->type  = TYPE_PREPROCESSOR;
    dpm->major = MAJOR_VERSION;
    dpm->minor = MINOR_VERSION;
    dpm->build = BUILD_VERSION;
    strncpy(dpm->uniqueName, PREPROC_NAME, MAX_NAME_LEN-1);
    dpm->uniqueName[MAX_NAME_LEN-1] = '\0';
    return 0;
}

