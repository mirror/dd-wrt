/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2010-2013 Sourcefire, Inc.
** Author: Michael R. Altizer <maltizer@sourcefire.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include "daq.h"
#include "daq_api.h"

/*
 * Functions that apply to instances of DAQ modules go here.
 */
DAQ_LINKAGE int daq_initialize(const DAQ_Module_t *module, const DAQ_Config_t *config, void **handle, char *errbuf, size_t len)
{
    /* Don't do this. */
    if (!errbuf)
        return DAQ_ERROR;

    if (!module)
        return DAQ_ERROR_NOMOD;

    if (!config)
    {
        snprintf(errbuf, len, "Can't initialize without a configuration!");
        return DAQ_ERROR_INVAL;
    }

    if (!handle)
    {
        snprintf(errbuf, len, "Can't initialize without a context pointer!");
        return DAQ_ERROR_INVAL;
    }

    if ((config->mode == DAQ_MODE_PASSIVE && !(module->type & DAQ_TYPE_INTF_CAPABLE)) ||
        (config->mode == DAQ_MODE_INLINE && !(module->type & DAQ_TYPE_INLINE_CAPABLE)) ||
        (config->mode == DAQ_MODE_READ_FILE && !(module->type & DAQ_TYPE_FILE_CAPABLE)))
    {
        snprintf(errbuf, len, "The %s DAQ module does not support %s mode!", module->name, daq_mode_string(config->mode));
        return DAQ_ERROR_INVAL;
    }


    return module->initialize(config, handle, errbuf, len);
}

DAQ_LINKAGE int daq_set_filter(const DAQ_Module_t *module, void *handle, const char *filter)
{
    if (!module)
        return DAQ_ERROR_NOMOD;

    if (!handle)
        return DAQ_ERROR_NOCTX;

    if (!filter)
    {
        module->set_errbuf(handle, "No filter string specified!");
        return DAQ_ERROR_INVAL;
    }

    return module->set_filter(handle, filter);
}

DAQ_LINKAGE int daq_start(const DAQ_Module_t *module, void *handle)
{
    if (!module)
        return DAQ_ERROR_NOMOD;

    if (!handle)
        return DAQ_ERROR_NOCTX;

    if (module->check_status(handle) != DAQ_STATE_INITIALIZED)
    {
        module->set_errbuf(handle, "Can't start an instance that isn't initialized!");
        return DAQ_ERROR;
    }

    return module->start(handle);
}

DAQ_LINKAGE int daq_acquire(const DAQ_Module_t *module, void *handle, int cnt,
                            DAQ_Analysis_Func_t callback, void *user)
{
    if (!module)
        return DAQ_ERROR_NOMOD;

    if (!handle)
        return DAQ_ERROR_NOCTX;

    if (module->check_status(handle) != DAQ_STATE_STARTED)
    {
        module->set_errbuf(handle, "Can't acquire packets from an instance that isn't started!");
        return DAQ_ERROR;
    }

    return module->acquire(handle, cnt, callback, NULL, user);
}

DAQ_LINKAGE int daq_acquire_with_meta(const DAQ_Module_t *module, void *handle, int cnt,
                                      DAQ_Analysis_Func_t callback,
                                      DAQ_Meta_Func_t metaback, void *user)
{
    if (!module)
        return DAQ_ERROR_NOMOD;

    if (!handle)
        return DAQ_ERROR_NOCTX;

    if (module->check_status(handle) != DAQ_STATE_STARTED)
    {
        module->set_errbuf(handle, "Can't acquire packets from an instance that isn't started!");
        return DAQ_ERROR;
    }

    return module->acquire(handle, cnt, callback, metaback, user);
}

DAQ_LINKAGE int daq_inject(const DAQ_Module_t *module, void *handle, const DAQ_PktHdr_t *hdr, const uint8_t *packet_data, uint32_t len, int reverse)
{
    if (!module)
        return DAQ_ERROR_NOMOD;

    if (!handle)
        return DAQ_ERROR_NOCTX;

    if (!hdr)
    {
        module->set_errbuf(handle, "No originating packet header specified!");
        return DAQ_ERROR_INVAL;
    }

    if (!packet_data)
    {
        module->set_errbuf(handle, "No packet data specified!");
        return DAQ_ERROR_INVAL;
    }

    return module->inject(handle, hdr, packet_data, len, reverse);
}

DAQ_LINKAGE int daq_breakloop(const DAQ_Module_t *module, void *handle)
{
    if (!module)
        return DAQ_ERROR_NOMOD;

    if (!handle)
        return DAQ_ERROR_NOCTX;

    return module->breakloop(handle);
}

DAQ_LINKAGE int daq_stop(const DAQ_Module_t *module, void *handle)
{
    if (!module)
        return DAQ_ERROR_NOMOD;

    if (!handle)
        return DAQ_ERROR_NOCTX;

    if (module->check_status(handle) != DAQ_STATE_STARTED)
    {
        module->set_errbuf(handle, "Can't stop an instance that hasn't started!");
        return DAQ_ERROR;
    }

    return module->stop(handle);
}

DAQ_LINKAGE int daq_shutdown(const DAQ_Module_t *module, void *handle)
{
    if (!module)
       return DAQ_ERROR_NOMOD;
    if (!handle)
        return DAQ_ERROR_NOCTX;

    module->shutdown(handle);

    return DAQ_SUCCESS;
}

DAQ_LINKAGE DAQ_State daq_check_status(const DAQ_Module_t *module, void *handle)
{
    if (!module || !handle)
        return DAQ_STATE_UNKNOWN;

    return module->check_status(handle);
}

DAQ_LINKAGE int daq_get_stats(const DAQ_Module_t *module, void *handle, DAQ_Stats_t *stats)
{
    if (!module)
        return DAQ_ERROR_NOMOD;

    if (!handle)
        return DAQ_ERROR_NOCTX;

    if (!stats)
    {
        module->set_errbuf(handle, "No place to put the statistics!");
        return DAQ_ERROR_INVAL;
    }

    return module->get_stats(handle, stats);
}

DAQ_LINKAGE void daq_reset_stats(const DAQ_Module_t *module, void *handle)
{
    if (module && handle)
        module->reset_stats(handle);
}

DAQ_LINKAGE int daq_get_snaplen(const DAQ_Module_t *module, void *handle)
{
    if (!module)
        return DAQ_ERROR_NOMOD;

    if (!handle)
        return DAQ_ERROR_NOCTX;

    return module->get_snaplen(handle);
}

DAQ_LINKAGE uint32_t daq_get_capabilities(const DAQ_Module_t *module, void *handle)
{
    if (!module)
        return DAQ_ERROR_NOMOD;

    if (!handle)
        return DAQ_ERROR_NOCTX;

    return module->get_capabilities(handle);
}

DAQ_LINKAGE int daq_get_datalink_type(const DAQ_Module_t *module, void *handle)
{
    if (!module)
        return DAQ_ERROR_NOMOD;

    if (!handle)
        return DAQ_ERROR_NOCTX;

    return module->get_datalink_type(handle);
}

DAQ_LINKAGE const char *daq_get_error(const DAQ_Module_t *module, void *handle)
{
    if (!module || !handle)
        return NULL;

    return module->get_errbuf(handle);
}

DAQ_LINKAGE void daq_clear_error(const DAQ_Module_t *module, void *handle)
{
    if (!module || !handle)
        return;

    module->set_errbuf(handle, "");
}

DAQ_LINKAGE int daq_get_device_index(const DAQ_Module_t *module, void *handle, const char *device)
{
    if (!module)
        return DAQ_ERROR_NOMOD;

    if (!handle)
        return DAQ_ERROR_NOCTX;

    if (!device)
    {
        module->set_errbuf(handle, "No device name to find the index of!");
        return DAQ_ERROR_INVAL;
    }

    return module->get_device_index(handle, device);
}

DAQ_LINKAGE int daq_hup_prep(const DAQ_Module_t *module, void *handle, void **new_config)
{
    if (!module)
        return DAQ_ERROR_NOMOD;

    if (!handle)
        return DAQ_ERROR_NOCTX;

    if (!module->hup_prep)
    {
        if (!module->hup_apply)
            return 1;
        return DAQ_SUCCESS;
    }

    return module->hup_prep(handle, new_config);
}

DAQ_LINKAGE int daq_hup_apply(const DAQ_Module_t *module, void *handle, void *new_config, void **old_config)
{
    if (!module)
        return DAQ_ERROR_NOMOD;

    if (!handle)
        return DAQ_ERROR_NOCTX;

    if (!module->hup_apply)
        return DAQ_SUCCESS;

    return module->hup_apply(handle, new_config, old_config);
}

DAQ_LINKAGE int daq_hup_post(const DAQ_Module_t *module, void *handle, void *old_config)
{
    if (!module)
        return DAQ_ERROR_NOMOD;

    if (!handle)
        return DAQ_ERROR_NOCTX;

    if (!module->hup_post)
        return DAQ_SUCCESS;

    return module->hup_post(handle, old_config);
}

DAQ_LINKAGE int daq_modify_flow(const DAQ_Module_t *module, void *handle, const DAQ_PktHdr_t *hdr, DAQ_ModFlow_t *modify)
{
    if (!module)
        return DAQ_ERROR_NOMOD;

    if (!module->modify_flow)
        return DAQ_SUCCESS;

    return module->modify_flow(handle, hdr, modify);
}

/*
 * Functions that apply to DAQ modules themselves go here.
 */
DAQ_LINKAGE const char *daq_get_name(const DAQ_Module_t *module)
{
    if (!module)
        return NULL;

    return module->name;
}

DAQ_LINKAGE uint32_t daq_get_type(const DAQ_Module_t *module)
{
    if (!module)
        return DAQ_ERROR_NOMOD;

    return module->type;
}

DAQ_LINKAGE int daq_dp_add_dc(const DAQ_Module_t *module, void *handle, const DAQ_PktHdr_t *hdr, DAQ_DP_key_t *dp_key, const uint8_t *packet_data)
{
    if (!module)
        return DAQ_ERROR_NOMOD;

    if (!handle)
        return DAQ_ERROR_NOCTX;

    if (!module->dp_add_dc)
        return DAQ_SUCCESS;

    return module->dp_add_dc(handle, hdr, dp_key, packet_data);
}
