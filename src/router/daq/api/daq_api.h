/*
** Copyright (C) 2010 Sourcefire, Inc.
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef _DAQ_API_H
#define _DAQ_API_H

#include <daq_common.h>

struct _daq_dict_entry
{
    char *key;
    char *value;
    struct _daq_dict_entry *next;
};

struct _daq_module
{
    /* The version of the API this module implements.
       This *must* be the first element in the structure. */
    const uint32_t api_version;
    /* The version of the DAQ module itself - can be completely arbitrary. */
    const uint32_t module_version;
    /* The name of the module (sfpacket, xvnim, pcap, etc.) */
    const char *name;
    /* Various flags describing the module and its capabilities (Inline-capabale, etc.) */
    const uint32_t type;
    /* Initialize the device for packet acquisition with the supplied configuration.
       This should not start queuing packets for the application. */
    int (*initialize) (const DAQ_Config_t *config, void **ctxt_ptr, char *errbuf, size_t len);
    /* Set the module's BPF based on the given string */
    int (*set_filter) (void *handle, const char *filter);
    /* Complete device opening and begin queuing packets if they have not been already. */
    int (*start) (void *handle);
    /* Acquire up to <cnt> packets and call <callback> for each with <user> as the final argument.
       The return value of the callback will determine the action taken by the DAQ for each packet.
       If <cnt> is 0, packets will continue to be acquired until some other factor breaks the
       acquisition loop. */
    int (*acquire) (void *handle, int cnt, DAQ_Analysis_Func_t callback, void *user);
    /* Inject a new packet going either the same or opposite direction as the specified packet. */
    int (*inject) (void *handle, const DAQ_PktHdr_t *hdr, const uint8_t *packet_data, uint32_t len, int reverse);
    /* Force breaking out of the acquisition loop after the current iteration. */
    int (*breakloop) (void *handle);
    /* Stop queuing packets, if possible */
    int (*stop) (void *handle);
    /* Close the device and clean up */
    void (*shutdown) (void *handle);
    /* Get the status of the module (one of DAQ_STATE_*). */
    DAQ_State (*check_status) (void *handle);
    /* Populates the <stats> structure with the current DAQ stats.  These stats are cumulative. */
    int (*get_stats) (void *handle, DAQ_Stats_t *stats);
    /* Resets the DAQ module's internal stats. */
    void (*reset_stats) (void *handle);
    /* Return the configured snaplen */
    int (*get_snaplen) (void *handle);
    /* Return a bitfield of the device's capabilities */
    uint32_t (*get_capabilities) (void *handle);
    /* Return the instance's Data Link Type */
    int (*get_datalink_type) (void *handle);
    /* Return a pointer to the module's internal error buffer */
    const char * (*get_errbuf) (void *handle);
    /* Write a string to the module instance's internal error buffer */
    void (*set_errbuf) (void *handle, const char *string);
    /* Return the index of the given named device if possible. */
    int (*get_device_index) (void *handle, const char *device);
};

#define DAQ_API_VERSION    0x00010001

#define DAQ_ERRBUF_SIZE 256
/* This is a convenience macro for safely printing to DAQ error buffers.  It must be called on a known-size character array. */

#ifdef WIN32
inline void DPE(char *var, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    snprintf(var, sizeof(var), ap);

    va_end(ap);
}
#else
#define DPE(var, ...) snprintf(var, sizeof(var), __VA_ARGS__)
#endif

#endif /* _DAQ_API_H */
