/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2012-2013 Sourcefire, Inc.
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
 **
 ** Date: 01-27-2012
 ** Author: Hui Cao <hcao@sourcefire.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"

#include "output_api.h"
#include "output_lib.h"


extern char *file_name;
extern int file_line;

DynamicOutputData _dod;


OUTPUT_SO_PUBLIC int initOutputPlugins(void *dod)
{
    DynamicOutputData* outputData = (DynamicOutputData*)dod;
    /*Check the version*/
    if (!dod)
        return OUTPUT_ERROR;

    if (outputData->majorVersion != OUTPUT_DATA_MAJOR_VERSION)
    {
        printf("ERROR major version %d != %d\n", outputData->majorVersion,
                OUTPUT_DATA_MAJOR_VERSION);
        return OUTPUT_ERROR;
    }

    if (outputData->minorVersion < OUTPUT_DATA_MINOR_VERSION)
    {
        printf("ERROR minor version %d < %d\n", outputData->minorVersion,
                OUTPUT_DATA_MINOR_VERSION);
        return OUTPUT_ERROR;
    }

    if (outputData->size < sizeof(DynamicOutputData))
    {
        printf("ERROR size %d != %u\n", outputData->size, (unsigned)sizeof(*outputData));
        return OUTPUT_ERROR;
    }

    _dod = *((DynamicOutputData*)dod);
    return OUTPUT_SUCCESS;

}

void init_output_module(struct _SnortConfig *sc, Output_Module_t *om, char *args)
{
    void *config;

    if (!om)
        return;

    printf("Initializing module %s \n", om->name);
    om->parse_args(&config, args, om->default_file);
    if (om->alert_output)
    {
        _dod.addOutputModule(sc, om->alert_output, DYNAMIC_OUTPUT_TYPE_FLAG__ALERT, config);
    }
    if (om->log_output)
    {
        _dod.addOutputModule(sc, om->log_output, DYNAMIC_OUTPUT_TYPE_FLAG__LOG, config);
    }
#ifdef SNORT_RELOAD
    if (om->rotate)
    {
        _dod.addReload(om->rotate, config);
    }
#endif
    if (om->postconfig)
    {
        _dod.addPostconfig(sc, om->postconfig, config);
    }
    if (om->shutdown)
    {
        _dod.addCleanExit(om->shutdown, config);
    }
}
