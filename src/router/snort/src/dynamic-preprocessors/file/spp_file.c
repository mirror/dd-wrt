/* $Id */

/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2013-2013 Sourcefire, Inc.
 **
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


/*
 * File preprocessor
 * Author: Hui Cao
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "sf_types.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include "sf_snort_plugin_api.h"
#include "snort_debug.h"

#include "preprocids.h"
#include "spp_file.h"
#include "sf_preproc_info.h"

#include <stdio.h>
#include <syslog.h>
#include <string.h>
#ifndef WIN32
#include <strings.h>
#include <sys/time.h>
#endif
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include "file_agent.h"
#include "file_inspect_config.h"

const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 1;
const int BUILD_VERSION = 1;
const char *PREPROC_NAME = "SF_FILE";
#define FILE_PREPROC_NAME  "file_inspect"

#define SetupFileInspect DYNAMIC_PREPROC_SETUP

/*
 * Function prototype(s)
 */

static void FileInit( struct _SnortConfig*, char* );

static void print_file_stats(int exiting);

static void FileFreeConfig(tSfPolicyUserContextId config);
static int FileCheckConfig(struct _SnortConfig *);
static void FileCleanExit(int, void *);
static void FileUpdateConfig(FileInspectConf *, tSfPolicyUserContextId);
int FileInspectPrintMemStats(FILE *fd, char* buffer, PreprocMemInfo *meminfo);

/** File configuration per Policy
 */
tSfPolicyUserContextId file_config = NULL;

#ifdef SNORT_RELOAD
static void FileReload(struct _SnortConfig *, char *, void **);
static int FileReloadVerify(struct _SnortConfig *, void *);
static void * FileReloadSwap(struct _SnortConfig *, void *);
static void FileReloadSwapFree(void *);
#endif

File_Stats file_inspect_stats;

/* Called at preprocessor setup time. Links preprocessor keyword
 * to corresponding preprocessor initialization function.
 *
 * PARAMETERS:	None.
 *
 * RETURNS:	Nothing.
 *
 */
void SetupFileInspect(void)
{
    /* Link preprocessor keyword to initialization function
     * in the preprocessor list. */
#ifndef SNORT_RELOAD
    _dpd.registerPreproc( "file_inspect", FileInit );
#else
    _dpd.registerPreproc("file_inspect", FileInit, FileReload, FileReloadVerify,
            FileReloadSwap, FileReloadSwapFree);
#endif
}

/* Initializes the File preprocessor module and registers
 * it in the preprocessor list.
 *
 * PARAMETERS:
 *
 * argp:        Pointer to argument string to process for config
 *                      data.
 *
 * RETURNS:     Nothing.
 */
static void FileInit(struct _SnortConfig *sc, char *argp)
{
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    FileInspectConf *pPolicyConfig = NULL;

    if (file_config == NULL)
    {
        /*create a context*/
        file_config = sfPolicyConfigCreate();
        if (file_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Failed to allocate memory "
                    "for File config.\n");
        }

        if (_dpd.streamAPI == NULL)
        {
            DynamicPreprocessorFatalMessage("SetupFile(): The Stream preprocessor must be enabled.\n");
        }

        _dpd.addPreprocConfCheck(sc, FileCheckConfig);
        _dpd.registerPreprocStats(FILE_PREPROC_NAME, print_file_stats);
        _dpd.registerMemoryStatsFunc(PP_FILE_INSPECT, FileInspectPrintMemStats);
        _dpd.addPreprocExit(FileCleanExit, NULL, PRIORITY_LAST, PP_FILE_INSPECT);

    }

    sfPolicyUserPolicySet (file_config, policy_id);
    pPolicyConfig = (FileInspectConf *)sfPolicyUserDataGetCurrent(file_config);
    if (pPolicyConfig != NULL)
    {
        DynamicPreprocessorFatalMessage("File preprocessor can only be "
                "configured once.\n");
    }

    pPolicyConfig = (FileInspectConf *)_dpd.snortAlloc(1, sizeof(FileInspectConf), PP_FILE_INSPECT,
            PP_MEM_CATEGORY_CONFIG);
    if (!pPolicyConfig)
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory for "
                "File preprocessor configuration.\n");
    }

    sfPolicyUserDataSetCurrent(file_config, pPolicyConfig);

    file_config_parse(pPolicyConfig, (u_char *)argp);
    FileUpdateConfig(pPolicyConfig, file_config);
    file_agent_init(sc, pPolicyConfig);
    _dpd.addPostConfigFunc(sc, file_agent_thread_init, pPolicyConfig);

}

static void FileUpdateConfig(FileInspectConf *pPolicyConfig, tSfPolicyUserContextId context)
{

    FileInspectConf *defaultConfig =
            (FileInspectConf *)sfPolicyUserDataGetDefault(context);

    if (pPolicyConfig == defaultConfig)
    {
        if (!pPolicyConfig->file_capture_queue_size)
            pPolicyConfig->file_capture_queue_size = FILE_CAPTURE_QUEUE_SIZE_DEFAULT;
        if (!pPolicyConfig->capture_disk_size)
            pPolicyConfig->capture_disk_size = FILE_CAPTURE_DISK_SIZE_DEFAULT;
    }
    else if (defaultConfig == NULL)
    {
        if (pPolicyConfig->file_capture_queue_size)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => File inspect: "
                    "file capture queue size must be configured "
                    "in the default config.\n",
                    *(_dpd.config_file), *(_dpd.config_line));
        }
    }
    else
    {
        pPolicyConfig->file_capture_queue_size = defaultConfig->file_capture_queue_size;

    }
}

static int FileFreeConfigPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
)
{
    FileInspectConf *pPolicyConfig = (FileInspectConf *)pData;

    //do any housekeeping before freeing FileInspectConf
    file_config_free(pPolicyConfig);
    sfPolicyUserDataClear (config, policyId);
    _dpd.snortFree(pPolicyConfig, sizeof(FileInspectConf), PP_FILE_INSPECT, PP_MEM_CATEGORY_CONFIG);
    return 0;
}

static void FileFreeConfig(tSfPolicyUserContextId config)
{
    if (config == NULL)
        return;

    sfPolicyUserDataFreeIterate (config, FileFreeConfigPolicy);
    sfPolicyConfigDelete(config);
}

static int FileCheckPolicyConfig(struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData)
{
    _dpd.setParserPolicy(sc, policyId);

    if (!_dpd.isPreprocEnabled(sc, PP_STREAM))
    {
        DynamicPreprocessorFatalMessage("FileCheckPolicyConfig(): The Stream preprocessor must be enabled.\n");
    }
    return 0;
}

static int FileCheckConfig(struct _SnortConfig *sc)
{
    int rval;
    if ((rval = sfPolicyUserDataIterate (sc, file_config, FileCheckPolicyConfig)))
        return rval;
    return 0;
}

static void FileCleanExit(int signal, void *data)
{
    if (file_config != NULL)
    {
        file_agent_close();
        FileFreeConfig(file_config);
        file_config = NULL;
    }
}

#ifdef SNORT_RELOAD
static void FileReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId file_swap_config = (tSfPolicyUserContextId)*new_config;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    FileInspectConf * pPolicyConfig = NULL;

    if (file_swap_config == NULL)
    {
        //create a context
        file_swap_config = sfPolicyConfigCreate();
        if (file_swap_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Failed to allocate memory "
                    "for File config.\n");
        }

        if (_dpd.streamAPI == NULL)
        {
            DynamicPreprocessorFatalMessage("SetupFile(): The Stream preprocessor must be enabled.\n");
        }

        *new_config = (void *)file_swap_config;
    }

    sfPolicyUserPolicySet (file_swap_config, policy_id);
    pPolicyConfig = (FileInspectConf *)sfPolicyUserDataGetCurrent(file_swap_config);

    if (pPolicyConfig != NULL)
    {
        DynamicPreprocessorFatalMessage("File preprocessor can only be "
                "configured once.\n");
    }

    pPolicyConfig = (FileInspectConf *)_dpd.snortAlloc(1, sizeof(FileInspectConf), PP_FILE_INSPECT,
            PP_MEM_CATEGORY_CONFIG);
    if (!pPolicyConfig)
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory for "
                "File preprocessor configuration.\n");
    }
    sfPolicyUserDataSetCurrent(file_swap_config, pPolicyConfig);

    file_config_parse(pPolicyConfig, (u_char *)args);
    FileUpdateConfig(pPolicyConfig, file_config);
    file_agent_init(sc, pPolicyConfig);
}

static int FileReloadVerify(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId file_swap_config = (tSfPolicyUserContextId)swap_config;
    FileInspectConf * pPolicyConfig = NULL;
    FileInspectConf * pCurrentConfig = NULL;

    if (file_swap_config == NULL)
        return 0;

    pPolicyConfig = (FileInspectConf *)sfPolicyUserDataGet(file_swap_config, _dpd.getDefaultPolicy());

    if (!pPolicyConfig)
        return 0;


    if (file_config != NULL)
    {
        pCurrentConfig = (FileInspectConf *)sfPolicyUserDataGet(file_config, _dpd.getDefaultPolicy());
    }

    if (!pCurrentConfig)
        return 0;

    if (file_config_compare(pCurrentConfig, pPolicyConfig))
    {
        _dpd.errMsg("File inspect reload: Changing file settings requires a restart.\n");
        return -1;
    }

    if (!_dpd.isPreprocEnabled(sc, PP_STREAM))
    {
        _dpd.errMsg("SetupFile(): The Stream preprocessor must be enabled.\n");
        return -1;
    }

    return 0;
}

static int FileFreeUnusedConfigPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
)
{
    FileInspectConf *pPolicyConfig = (FileInspectConf *)pData;

    //do any housekeeping before freeing FileInspectConf
    if (pPolicyConfig->ref_count == 0)
    {
        sfPolicyUserDataClear (config, policyId);
        _dpd.snortFree(pPolicyConfig, sizeof(FileInspectConf), PP_FILE_INSPECT, PP_MEM_CATEGORY_CONFIG);
    }
    return 0;
}

static void * FileReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId file_swap_config = (tSfPolicyUserContextId)swap_config;
    tSfPolicyUserContextId old_config = file_config;

    if (file_swap_config == NULL)
        return NULL;

    file_config = file_swap_config;

    file_swap_config = NULL;

    sfPolicyUserDataFreeIterate (old_config, FileFreeUnusedConfigPolicy);

    if (sfPolicyUserPolicyGetActive(old_config) == 0)
    {
        /* No more outstanding configs - free the config array */
        return (void *)old_config;
    }

    return NULL;
}

static void FileReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    FileFreeConfig((tSfPolicyUserContextId)data);
}
#endif

static void print_file_stats(int exiting)
{
    _dpd.logMsg("File Preprocessor Statistics\n");

    _dpd.logMsg("  Total file type callbacks:            "FMTu64("-10")" \n",
            file_inspect_stats.file_types_total);
    _dpd.logMsg("  Total file signature callbacks:       "FMTu64("-10")" \n",
            file_inspect_stats.file_signatures_total);
    _dpd.logMsg("  Total files would saved to disk:      "FMTu64("-10")" \n",
            file_inspect_stats.files_to_disk_total);
    _dpd.logMsg("  Total files saved to disk:            "FMTu64("-10")" \n",
            file_inspect_stats.files_saved);
    _dpd.logMsg("  Total file data saved to disk:        "FMTu64("-10")"bytes\n",
            file_inspect_stats.file_data_to_disk);
    _dpd.logMsg("  Total files duplicated:               "FMTu64("-10")" \n",
            file_inspect_stats.file_duplicates_total);
    _dpd.logMsg("  Total files reserving failed:         "FMTu64("-10")" \n",
            file_inspect_stats.file_reserve_failures);
    _dpd.logMsg("  Total file capture min:               "FMTu64("-10")" \n",
            file_inspect_stats.file_capture_min);
    _dpd.logMsg("  Total file capture max:               "FMTu64("-10")" \n",
            file_inspect_stats.file_capture_max);
    _dpd.logMsg("  Total file capture memcap:            "FMTu64("-10")" \n",
            file_inspect_stats.file_capture_memcap);
    _dpd.logMsg("  Total files reading failed:           "FMTu64("-10")" \n",
            file_inspect_stats.file_read_failures);
    _dpd.logMsg("  Total file agent memcap failures:     "FMTu64("-10")" \n",
            file_inspect_stats.file_agent_memcap_failures);
    _dpd.logMsg("  Total files sent:                     "FMTu64("-10")" \n",
            file_inspect_stats.files_to_host_total);
    _dpd.logMsg("  Total file data sent:                 "FMTu64("-10")" \n",
            file_inspect_stats.file_data_to_host);
    _dpd.logMsg("  Total file transfer failures:         "FMTu64("-10")" \n",
            file_inspect_stats.file_transfer_failures);


}

int FileInspectPrintMemStats(FILE *fd, char* buffer, PreprocMemInfo *meminfo)
{
    time_t curr_time = time(NULL);
    int len = 0;

    size_t total_heap_memory = meminfo[PP_MEM_CATEGORY_SESSION].used_memory 
                              + meminfo[PP_MEM_CATEGORY_CONFIG].used_memory;
    
    if (fd)
    {
        len = fprintf(fd, ",%lu,%u,%u"
                      ",%lu,%u,%u"
                      ",%lu"
                      , meminfo[PP_MEM_CATEGORY_SESSION].used_memory
                      , meminfo[PP_MEM_CATEGORY_SESSION].num_of_alloc
                      , meminfo[PP_MEM_CATEGORY_SESSION].num_of_free
                      , meminfo[PP_MEM_CATEGORY_CONFIG].used_memory
                      , meminfo[PP_MEM_CATEGORY_CONFIG].num_of_alloc
                      , meminfo[PP_MEM_CATEGORY_CONFIG].num_of_free
                      , total_heap_memory);
       return len;
    }
   
    if (buffer)
    {
       len = snprintf(buffer, CS_STATS_BUF_SIZE, "\n\nMemory Statistics for File Inspect at: %s\n"
       "Total file data saved to disk:           "FMTu64("-10")"bytes\n"
       "Total file capture max:                  "FMTu64("-10")" \n"
       "Total file capture memcap:               "FMTu64("-10")" \n"
       ,ctime(&curr_time)
       ,file_inspect_stats.file_data_to_disk
       ,file_inspect_stats.file_capture_max
       ,file_inspect_stats.file_capture_memcap);
    }
    else
    {
        _dpd.logMsg("\n");
        _dpd.logMsg("Memory Statistics for File Inspect at: %s\n", ctime(&curr_time));
        _dpd.logMsg("Total file data saved to disk:     "FMTu64("-10")" bytes\n", 
                file_inspect_stats.file_data_to_disk);
        _dpd.logMsg("Total file capture max:                 "FMTu64("-10")" \n", 
                file_inspect_stats.file_capture_max);
        _dpd.logMsg("Total file capture memcap:              "FMTu64("-10")" \n", 
                file_inspect_stats.file_capture_memcap);
    }
    
    return len;
}
