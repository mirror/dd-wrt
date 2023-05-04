/* 
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2003-2013 Sourcefire, Inc.     All Rights Reserved
 * Test program for streaming a unified log file 
 */

#include <signal.h>

#include "UnifiedLog.h"
#include "SpoolFileIterator.h"

#include <sf_error.h>

#include <stdio.h>
#include <sys/types.h>
#ifdef LINUX
#include <stdint.h>
#endif
#include <netinet/in.h>
#include <getopt.h>
#include <string.h>
#include <syslog.h>
#include <sf_error.h>

struct _config
{
    char *name;
    char *path;
    uint32_t priority;
}config;

static int ParseCommandLine(int argc, char *argv[]);


bool stop_processing = false;
#if 1

static void HandleSignal(int signal)
{
    stop_processing = true;
}

#endif

#define BOOKMARK_FILE_SIZE  128

int main(int argc, char *argv[])
{
    //UnifiedLog *unified_log = NULL;
    SpoolFileIterator *iterator = NULL;
    Unified2Record *record = NULL;
    int rval = 0;
    char bookmark[BOOKMARK_FILE_SIZE];
    uint32_t file, position;

#if 0
    sflog_enable_details();
    sflog_enable_stderr();
    sflog_disable_syslog();
    sflog_set_current_log_level(SFLOG_DEBUG);
#endif
    signal(SIGTERM, HandleSignal);

    if((rval = ParseCommandLine(argc, argv))
            != SF_SUCCESS)
    {
        return rval;
    }

    snprintf(bookmark, BOOKMARK_FILE_SIZE, "%s/%sbookmark", config.path, config.name);

    if((rval = SpoolFileIterator_New(config.path, config.name, bookmark, &iterator))
            != SF_SUCCESS)
    {
        fprintf(stderr, "Failed to create iterator: %s\n", sf_strerror(rval));
        return rval;
    }


    while (!stop_processing)
    {
        /* Get another record for this iterator */
        rval = SpoolFileIterator_GetNext(iterator, &record, &file, &position);
        if(rval != SF_SUCCESS && rval != SF_EAGAIN && rval != SF_ENOENT)
        {
            fprintf(stderr, "Error getting record from iterator: %s",sf_strerror(rval));
            return rval;
        }
    }
    fprintf(stderr, "GetNext returned: %s\n", sf_strerror(rval));
    
    if(iterator)
    {
        SpoolFileIterator_Destroy(iterator);
    }
    free(config.path);
    free(config.name);

    return rval;
}
static void usage(char *binaryName)
{
    printf("Usage: %s [options] --name=<base file name>\n", binaryName);
    printf("   --path: directory containing the binary files.\n");
    printf("   --help: This text.\n");
}


static int ParseCommandLine(int argc, char *argv[])
{
    int c;
    int option_index = 0;
    static struct option long_options[] =
    {
        {"name",  1, NULL, 'n'},
        {"path",  1, NULL, 'p'},
        {"help",  0, NULL, 0},
        {NULL,    0, NULL, 0}
    };

    memset(&config, 0, sizeof(config));
    while((c = getopt_long(argc, argv, "n:p:", long_options,
                    &option_index)) != -1)
    {
        switch(c)
        {
            case 0:
                if(strcasecmp("help", long_options[option_index].name) == 0)
                {
                    usage(argv[0]);
                    exit(0);
                }
                else
                {
                    fprintf(stderr, "Unknown command line option: %s",
                            long_options[option_index].name);
                    return SF_EINVAL;
                }
                break;

            case 'n':
                config.name = malloc(strlen(optarg)+2);
                if(!(config.name))
                {
                    fprintf(stderr, "Out of memory processing command line");
                    return SF_ENOMEM;
                }
                strcpy(config.name, optarg);
                strcat(config.name,".");
                break;

            case 'p':
                if(!(config.path = strdup(optarg)))
                {
                    fprintf(stderr, "Out of memory processing command line");
                    return SF_ENOMEM;
                }
                break;

            default:
                return SF_EINVAL;
        }
    }
    if (!config.name || !config.path)
    {
        usage(argv[0]);
        exit(-1);
    }
    config.priority = (LOG_INFO | LOG_INFO);
    return 0;
}
