/*
 **
 **
 **  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 **  Copyright (C) 2012-2013 Sourcefire, Inc.
 **
 **  This program is free software; you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License Version 2 as
 **  published by the Free Software Foundation.  You may not use, modify or
 **  distribute this program under any other version of the GNU General
 **  Public License.
 **
 **  This program is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with this program; if not, write to the Free Software
 **  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **
 **  Author(s):  Hui Cao <hcao@sourcefire.com>
 **
 **  NOTES
 **  5.25.2012 - Initial Source Code. Hcao
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include "sf_types.h"
#include "util.h"
#include "mstring.h"
#include "memory_stats.h"
#include "parser.h"

#include "file_service_config.h"
#include "file_config.h"
#include "file_lib.h"
#include "file_capture.h"
#include "file_ss.h"

#define FILE_SERVICE_OPT__TYPE_DEPTH            "file_type_depth"
#define FILE_SERVICE_OPT__SIG_DEPTH             "file_signature_depth"
#define FILE_SERVICE_OPT__BLOCK_TIMEOUT         "file_block_timeout"
#define FILE_SERVICE_OPT__LOOKUP_TIMEOUT        "file_lookup_timeout"
#define FILE_SERVICE_OPT__BLOCK_TIMEOUT_LOOKUP  "block_timeout_lookup"
#define FILE_SERVICE_OPT__CAPTURE_MEMCAP        "file_capture_memcap"
#define FILE_SERVICE_OPT__CAPTURE_MAX_SIZE      "file_capture_max"
#define FILE_SERVICE_OPT__CAPTURE_MIN_SIZE      "file_capture_min"
#define FILE_SERVICE_OPT__CAPTURE_BLOCK_SIZE    "file_capture_block_size"

#define FILE_SERVICE_TYPE_DEPTH_MIN           0
#define FILE_SERVICE_TYPE_DEPTH_MAX           UINT32_MAX
#define FILE_SERVICE_SIG_DEPTH_MIN            0
#define FILE_SERVICE_SIG_DEPTH_MAX            UINT32_MAX
#define FILE_SERVICE_BLOCK_TIMEOUT_MIN        0
#define FILE_SERVICE_BLOCK_TIMEOUT_MAX        UINT32_MAX
#define FILE_SERVICE_LOOKUP_TIMEOUT_MIN       0
#define FILE_SERVICE_LOOKUP_TIMEOUT_MAX       UINT32_MAX
#define FILE_SERVICE_CAPTURE_MEMCAP_MIN       1
#define FILE_SERVICE_CAPTURE_MEMCAP_MAX       UINT32_MAX
#define FILE_SERVICE_CAPTURE_MAX_SIZE_MIN     0
#define FILE_SERVICE_CAPTURE_MAX_SIZE_MAX     UINT32_MAX
#define FILE_SERVICE_CAPTURE_MIN_SIZE_MIN     0
#define FILE_SERVICE_CAPTURE_MIN_SIZE_MAX     UINT32_MAX
#define FILE_SERVICE_CAPTURE_BLOCK_SIZE_MIN   8     /*at least 64bits*/
#define FILE_SERVICE_CAPTURE_BLOCK_SIZE_MAX   UINT32_MAX

#define DEFAULT_FILE_TYPE_DEPTH             1460        // 1460 B
#define DEFAULT_FILE_SIGNATURE_DEPTH        10485760    // 10 MiB
#define DEFAULT_FILE_SHOW_DATA_DEPTH        100         // 100 B 
#define DEFAULT_FILE_BLOCK_TIMEOUT          86400       // 1 day
#define DEFAULT_FILE_LOOKUP_TIMEOUT         2           // 2 seconds
#define DEFAULT_FILE_CAPTURE_MEM            100         // 100 MiB 
#define DEFAULT_FILE_CAPTURE_MAX_SIZE       1048576     // 1 MiB
#define DEFAULT_FILE_CAPTURE_MIN_SIZE       0           // 0
#define DEFAULT_FILE_CAPTURE_BLOCK_SIZE     32768       // 32 KiB

#if defined(DEBUG_MSGS) || defined (REG_TEST)
#define FILE_SERVICE_OPT__TYPE              "type_id"
#define FILE_SERVICE_OPT__SIG               "signature"
#define FILE_SERVICE_OPT__SHOW_DATA_DEPTH   "show_data_depth"
#include "file_api.h"
#endif

/*Set default values for file config*/
static inline void file_service_config_defaults(FileConfig *file_config)
{
    file_config->file_type_depth = DEFAULT_FILE_TYPE_DEPTH;
    file_config->file_signature_depth = DEFAULT_FILE_SIGNATURE_DEPTH;
    file_config->file_block_timeout = DEFAULT_FILE_BLOCK_TIMEOUT;
    file_config->file_lookup_timeout = DEFAULT_FILE_LOOKUP_TIMEOUT;
    file_config->block_timeout_lookup = false;

#if defined(DEBUG_MSGS) || defined (REG_TEST)
    file_config->show_data_depth = DEFAULT_FILE_SHOW_DATA_DEPTH;
#endif

    file_config->file_capture_memcap = DEFAULT_FILE_CAPTURE_MEM;
    file_config->file_capture_max_size = DEFAULT_FILE_CAPTURE_MAX_SIZE;
    file_config->file_capture_min_size = DEFAULT_FILE_CAPTURE_MIN_SIZE;
    file_config->file_capture_block_size = DEFAULT_FILE_CAPTURE_BLOCK_SIZE;
}

FileConfig* file_service_config_create(void)
{
    FileConfig *file_config = SnortPreprocAlloc(1, sizeof(*file_config), PP_FILE, PP_MEM_CATEGORY_CONFIG);

    file_service_config_defaults(file_config);
    return file_config;
}

#if defined (SIDE_CHANNEL)
void check_sidechannel_enabled(void *config)
{
  FileConfig *file_config = (FileConfig *)config;
  if(ScSideChannelEnabled())
  {
    file_config->use_side_channel = true;

    LogMessage("File service config: \n");

    LogMessage("    File side channel enabled =  %d \n",file_config->use_side_channel);
  }
  return;
}
#endif
#if defined (SIDE_CHANNEL) && defined (REG_TEST)
void FileSSConfigFree(void *config)
{
  FileConfig *file_config = (FileConfig *)config;
  FileSSConfig *file_ss_config = file_config->file_ss_config;
    if (file_ss_config == NULL)
        return;
    /* Not changing the free here because memory is allocated using strdup */
    if (file_ss_config->startup_input_file)
        free(file_ss_config->startup_input_file);

    /* Not changing the free here because memory is allocated using strdup */
    if (file_ss_config->runtime_output_file)
        free(file_ss_config->runtime_output_file);

    SnortPreprocFree(file_ss_config, sizeof(FileSSConfig), PP_FILE, PP_MEM_CATEGORY_CONFIG);
    return;
}
#endif

#ifdef SNORT_RELOAD
/* Verify the file service configuration, changing memory settings and depth
 * settings requires snort restart
 */
int file_sevice_config_verify(SnortConfig *old, SnortConfig *new)
{
    FileConfig *curr = (FileConfig *)old->file_config;
    FileConfig *next = (FileConfig *)new->file_config;
    FileConfig tmp;

    /*no file config on both*/
    if ((!curr) &&  (!next))
        return 0;

    /* use default value if nothing sets*/
    if (!curr)
    {
        curr = &tmp;
        file_service_config_defaults(curr);
    }

    if (!next)
    {
        next = &tmp;
        file_service_config_defaults(next);
    }
#if defined (SIDE_CHANNEL) && !defined (REG_TEST)
    check_sidechannel_enabled(curr);
#endif

    /* check configurations */
    if (curr->file_capture_memcap != next->file_capture_memcap)
    {
        ErrorMessage("File service: Changing file capture memcap"
                " requires a restart.\n");
        return -1;
    }

    if (curr->file_capture_block_size != next->file_capture_block_size)
    {
        ErrorMessage("File service: Changing file capture block size"
                " requires a restart.\n");
        return -1;
    }

    return 0;
}
#endif

/* Display the configuration for the File preprocessor.
 *
 * PARAMETERS:  None.
 *
 * RETURNS: Nothing.
 */
static void file_service_display_conf(FileConfig *config)
{

    if (config == NULL)
        return;

    LogMessage("\n");

    LogMessage("File service config: \n");

    LogMessage("    File type depth:           "STDi64" %s \n",
            config->file_type_depth,
            config->file_type_depth == DEFAULT_FILE_TYPE_DEPTH ?
                    "(Default) bytes" : " bytes" );

    LogMessage("    File signature depth:      "STDi64" %s \n",
            config->file_signature_depth,
            config->file_signature_depth == DEFAULT_FILE_SIGNATURE_DEPTH ?
                    "(Default) bytes" : " bytes" );

    LogMessage("    File block timeout:        "STDi64" %s \n",
            config->file_block_timeout,
            config->file_block_timeout == DEFAULT_FILE_BLOCK_TIMEOUT ?
                    "(Default) seconds" : " seconds" );

    LogMessage("    File lookup timeout:       "STDi64" %s \n",
            config->file_lookup_timeout,
            config->file_lookup_timeout == DEFAULT_FILE_LOOKUP_TIMEOUT ?
                    "(Default) seconds" : " seconds" );

    LogMessage("    Action for lookup timeout: %s\n",
            config->block_timeout_lookup ? "BLOCKED":"ALLOWED (Default)");

    LogMessage("    File capture memcap:       "STDi64" %s \n",
            config->file_capture_memcap,
            config->file_capture_memcap == DEFAULT_FILE_CAPTURE_MEM ?
                    "(Default) megabytes" : " megabytes" );

    LogMessage("    File capture block size:   "STDi64" %s \n",
            config->file_capture_block_size,
            config->file_capture_block_size == DEFAULT_FILE_CAPTURE_BLOCK_SIZE ?
                    "(Default) bytes" : " bytes" );

    LogMessage("    File capture max size:     "STDi64" %s \n",
            config->file_capture_max_size,
            config->file_capture_max_size == DEFAULT_FILE_CAPTURE_MAX_SIZE ?
                    "(Default) bytes" : " bytes" );

    LogMessage("    File capture min size:     "STDi64" %s \n",
            config->file_capture_min_size,
            config->file_capture_min_size == DEFAULT_FILE_CAPTURE_MIN_SIZE ?
                    "(Default) bytes" : " bytes" );
#if defined (SIDE_CHANNEL) && defined (REG_TEST)
    FilePrintSSConfig(config->file_ss_config);
#endif

    LogMessage("\n");
}
/*The main function for parsing rule option*/
void file_service_config(struct _SnortConfig* sc, char *args, void *conf)
{
    char **toks;
    int num_toks;
    int i;
    FileConfig *file_config = (FileConfig *)conf;
#if defined(DEBUG_MSGS) || defined (REG_TEST)
    bool file_type_enabled = false;
    bool file_signature_enabled = false;
#endif
    DEBUG_WRAP(DebugMessage(DEBUG_FILE,"Loading file service configuration: %s\n", args););

    if (!file_config)
    {
        return;
    }

    file_service_config_defaults(file_config);
#if defined (SIDE_CHANNEL) && defined (REG_TEST)
    if (NULL == (file_config->file_ss_config = (FileSSConfig *)SnortPreprocAlloc(1, 
            sizeof(FileSSConfig), PP_FILE, PP_MEM_CATEGORY_CONFIG)))
      return;
    else if(ScSideChannelEnabled())
      file_config->use_side_channel = true;
#endif

    toks = mSplit(args, ",", 0, &num_toks, 0);  /* get rule option pairs */

    for (i = 0; i < num_toks; i++)
    {
        char **opts;
        int num_opts;
        char *option_args = NULL;
        unsigned long value = 0;

        DEBUG_WRAP(DebugMessage(DEBUG_FILE,"   option: %s\n", toks[i]););

        /* break out the option name from its data */
        opts = mSplit(toks[i], " ", 3, &num_opts, '\\');

        DEBUG_WRAP(DebugMessage(DEBUG_FILE,"   option name: %s\n", opts[0]););

        if (num_opts == 2)
        {
            option_args = opts[1];
            DEBUG_WRAP(DebugMessage(DEBUG_FILE,"   option args: %s\n", option_args););
        }
        if ( !strcasecmp( opts[0], FILE_SERVICE_OPT__TYPE_DEPTH ))
        {
            CheckValueInRange(option_args, FILE_SERVICE_OPT__TYPE_DEPTH,
                    FILE_SERVICE_TYPE_DEPTH_MIN,
                    FILE_SERVICE_TYPE_DEPTH_MAX,
                    &value);
            file_config->file_type_depth = (int64_t)value;
            if (file_config->file_type_depth == 0)
                file_config->file_type_depth = FILE_SERVICE_TYPE_DEPTH_MAX;
        }
        else if ( !strcasecmp( opts[0], FILE_SERVICE_OPT__SIG_DEPTH ))
        {
            CheckValueInRange(option_args, FILE_SERVICE_OPT__SIG_DEPTH,
                    FILE_SERVICE_SIG_DEPTH_MIN,
                    FILE_SERVICE_SIG_DEPTH_MAX,
                    &value);
            file_config->file_signature_depth = (int64_t)value;
            if (file_config->file_signature_depth == 0)
                file_config->file_signature_depth = FILE_SERVICE_SIG_DEPTH_MAX;
        }
        else if ( !strcasecmp( opts[0], FILE_SERVICE_OPT__BLOCK_TIMEOUT ))
        {
            CheckValueInRange(option_args, FILE_SERVICE_OPT__BLOCK_TIMEOUT,
                    FILE_SERVICE_BLOCK_TIMEOUT_MIN,
                    FILE_SERVICE_BLOCK_TIMEOUT_MAX,
                    &value);
            file_config->file_block_timeout = (int64_t)value;
            if (file_config->file_block_timeout == 0)
                file_config->file_block_timeout = FILE_SERVICE_BLOCK_TIMEOUT_MAX;
        }
        else if ( !strcasecmp( opts[0], FILE_SERVICE_OPT__LOOKUP_TIMEOUT ))
        {
            CheckValueInRange(option_args, FILE_SERVICE_OPT__LOOKUP_TIMEOUT,
                    FILE_SERVICE_LOOKUP_TIMEOUT_MIN,
                    FILE_SERVICE_LOOKUP_TIMEOUT_MAX,
                    &value);
            file_config->file_lookup_timeout = (int64_t)value;
            if (file_config->file_lookup_timeout == 0)
                file_config->file_lookup_timeout = FILE_SERVICE_LOOKUP_TIMEOUT_MAX;
        }
        else if ( !strcasecmp( opts[0], FILE_SERVICE_OPT__BLOCK_TIMEOUT_LOOKUP ))
        {
            file_config->block_timeout_lookup = true;
        }
        else if ( !strcasecmp( opts[0], FILE_SERVICE_OPT__CAPTURE_MEMCAP ))
        {
            CheckValueInRange(option_args, FILE_SERVICE_OPT__CAPTURE_MEMCAP,
                    FILE_SERVICE_CAPTURE_MEMCAP_MIN, FILE_SERVICE_CAPTURE_MEMCAP_MAX,
                    &value);
            file_config->file_capture_memcap = (int64_t) value;
        }
        else if ( !strcasecmp( opts[0], FILE_SERVICE_OPT__CAPTURE_MAX_SIZE ))
        {
            CheckValueInRange(option_args, FILE_SERVICE_OPT__CAPTURE_MAX_SIZE,
                    FILE_SERVICE_CAPTURE_MAX_SIZE_MIN,
                    FILE_SERVICE_CAPTURE_MAX_SIZE_MAX,
                    &value);
            file_config->file_capture_max_size = (int64_t) value;
        }
        else if ( !strcasecmp( opts[0], FILE_SERVICE_OPT__CAPTURE_MIN_SIZE ))
        {
            CheckValueInRange(option_args, FILE_SERVICE_OPT__CAPTURE_MIN_SIZE,
                    FILE_SERVICE_CAPTURE_MIN_SIZE_MIN,
                    FILE_SERVICE_CAPTURE_MIN_SIZE_MAX,
                    &value);
            file_config->file_capture_min_size = (int64_t)value;
        }
        else if ( !strcasecmp( opts[0], FILE_SERVICE_OPT__CAPTURE_BLOCK_SIZE ))
        {
            CheckValueInRange(option_args, FILE_SERVICE_OPT__CAPTURE_BLOCK_SIZE,
                    FILE_SERVICE_CAPTURE_BLOCK_SIZE_MIN,
                    FILE_SERVICE_CAPTURE_BLOCK_SIZE_MAX,
                    &value);
            file_config->file_capture_block_size = (int64_t) value;
        }
#if defined(DEBUG_MSGS) || defined (REG_TEST)
        else if ( !strcasecmp( opts[0], FILE_SERVICE_OPT__TYPE ))
        {
            file_type_enabled = true;

        }
        else if ( !strcasecmp( opts[0], FILE_SERVICE_OPT__SIG ))
        {
            file_signature_enabled = true;
        }
        else if ( !strcasecmp( opts[0], FILE_SERVICE_OPT__SHOW_DATA_DEPTH ))
        {
            CheckValueInRange(option_args, FILE_SERVICE_OPT__SHOW_DATA_DEPTH,
                    FILE_SERVICE_SIG_DEPTH_MIN,
                    FILE_SERVICE_SIG_DEPTH_MAX,
                    &value);
            file_config->show_data_depth = (int64_t)value;
            if (file_config->show_data_depth == 0)
                file_config->show_data_depth = FILE_SERVICE_SIG_DEPTH_MAX;
        }
#endif
#if defined (SIDE_CHANNEL) && defined (REG_TEST)
        else if(!strcasecmp(opts[0], "ss_startup_input_file"))
        {
            file_config->file_ss_config->startup_input_file = strdup(opts[1]);
        }
        else if(!strcasecmp(opts[0], "ss_runtime_output_file"))
        {
            file_config->file_ss_config->runtime_output_file = strdup(opts[1]);
        }
#endif
        else
        {
            ParseError("File service: Invalid argument: %s\n",  opts[0]);
            return;
        }
        mSplitFree(&opts, num_opts);
    }

#if defined(DEBUG_MSGS) || defined (REG_TEST)
    if (file_type_enabled)
        file_api->enable_file_type(sc, NULL);
    if (file_signature_enabled)
        file_api->enable_file_signature(sc, NULL);
#endif
    /* file capture memcap should not be larger file capture block size*/
    if (file_config->file_capture_block_size >
            (file_config->file_capture_memcap << 20))
    {
        ParseError("File service: file capture block size ("STDi64")"
                " is larger than file capture memcap ("STDi64") bytes.",
                file_config->file_capture_block_size,
                file_config->file_capture_memcap << 20);
    }

    /* file capture size should not be larger file signature depth*/
    if (file_config->file_capture_max_size > file_config->file_signature_depth)
    {
        ParseError("File service: file capture max size ("STDi64") "
                "is larger than file signature depth ("STDi64").",
                file_config->file_capture_max_size,
                file_config->file_signature_depth);
    }

    /* file capture min size should not be larger file capture max size*/
    if (file_config->file_capture_min_size > file_config->file_capture_max_size)
    {
        ParseError("File service: file capture min size ("STDi64") "
                "is larger than file capture max size ("STDi64").",
                file_config->file_capture_min_size,
                file_config->file_capture_max_size);
    }

    file_service_display_conf(file_config);

    mSplitFree(&toks, num_toks);
}

