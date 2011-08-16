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

#ifndef _DAQ_H
#define _DAQ_H

#include <stdio.h>
#include <daq_common.h>

/* Definition of the structures returned by daq_get_module_list(). */
typedef struct {
    char *name;         /* Module name */
    uint32_t version;   /* Module version */
    uint32_t type;      /* Module capabilities */
} DAQ_Module_Info_t;

/* Functions for loading, handling, and unloading DAQ modules. */
DAQ_LINKAGE void daq_set_verbosity(int level);
DAQ_LINKAGE int daq_load_modules(const char *module_dirs[]);
DAQ_LINKAGE const DAQ_Module_t *daq_find_module(const char *name);
DAQ_LINKAGE int daq_get_module_list(DAQ_Module_Info_t *list[]);
DAQ_LINKAGE void daq_free_module_list(DAQ_Module_Info_t *list, int size);
DAQ_LINKAGE void daq_unload_modules(void);
DAQ_LINKAGE void daq_print_stats(DAQ_Stats_t *stats, FILE *fp);

/* Enumeration to String translation functions. */
DAQ_LINKAGE const char *daq_mode_string(DAQ_Mode mode);
DAQ_LINKAGE const char *daq_state_string(DAQ_State state);
DAQ_LINKAGE const char *daq_verdict_string(DAQ_Verdict verdict);

/* DAQ Configuration Dictionary Functions */
DAQ_LINKAGE const char *daq_config_get_value(DAQ_Config_t *config, const char *key);
DAQ_LINKAGE void daq_config_set_value(DAQ_Config_t *config, const char *key, const char *value);
DAQ_LINKAGE void daq_config_clear_value(DAQ_Config_t *config, const char *key);
DAQ_LINKAGE void daq_config_clear_values(DAQ_Config_t *config);

/* DAQ Module functions. */
DAQ_LINKAGE const char *daq_get_name(const DAQ_Module_t *module);
DAQ_LINKAGE uint32_t daq_get_type(const DAQ_Module_t *module);

/* DAQ Module Instance functions */
DAQ_LINKAGE int daq_initialize(const DAQ_Module_t *module, const DAQ_Config_t *config, void **handle, char *errbuf, size_t len);
DAQ_LINKAGE int daq_set_filter(const DAQ_Module_t *module, void *handle, const char *filter);
DAQ_LINKAGE int daq_start(const DAQ_Module_t *module, void *handle);
DAQ_LINKAGE int daq_acquire(const DAQ_Module_t *module, void *handle, int cnt, DAQ_Analysis_Func_t callback, void *user);
DAQ_LINKAGE int daq_inject(const DAQ_Module_t *module, void *handle, const DAQ_PktHdr_t *hdr, const uint8_t *packet_data, uint32_t len, int reverse);
DAQ_LINKAGE int daq_breakloop(const DAQ_Module_t *module, void *handle);
DAQ_LINKAGE int daq_stop(const DAQ_Module_t *module, void *handle);
DAQ_LINKAGE int daq_shutdown(const DAQ_Module_t *module, void *handle);
DAQ_LINKAGE DAQ_State daq_check_status(const DAQ_Module_t *module, void *handle);
DAQ_LINKAGE int daq_get_stats(const DAQ_Module_t *module, void *handle, DAQ_Stats_t *stats);
DAQ_LINKAGE void daq_reset_stats(const DAQ_Module_t *module, void *handle);
DAQ_LINKAGE int daq_get_snaplen(const DAQ_Module_t *module, void *handle);
DAQ_LINKAGE uint32_t daq_get_capabilities(const DAQ_Module_t *module, void *handle);
DAQ_LINKAGE int daq_get_datalink_type(const DAQ_Module_t *module, void *handle);
DAQ_LINKAGE const char *daq_get_error(const DAQ_Module_t *module, void *handle);
DAQ_LINKAGE void daq_clear_error(const DAQ_Module_t *module, void *handle);

#endif /* _DAQ_H */
