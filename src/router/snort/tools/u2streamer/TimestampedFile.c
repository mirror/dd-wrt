
/* System includes */
#include <sys/types.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef LINUX
#include <stdint.h>
#endif

/* local includes */
#include "sf_error.h"
#include "TimestampedFile.h"

#define MODULE_NAME "TimestampedFile"

int FindNextTimestampedFile(char *directory, char *file_prefix,
        uint32_t timestamp, int mode, uint32_t *next_timestamp)
{
    DIR *dir = NULL;
    struct dirent *entry;
    unsigned long file_timestamp;
    unsigned long selected_timestamp = 0;
    char *extension;

    /* check arguments */
    if(!directory || !file_prefix)
        return SF_EINVAL;

    if(!(dir = opendir(directory)))
    {
        fprintf(stderr, "Unable to open directory '%s': %s", directory, strerror(errno));
        return SF_EOPEN;
    }

    /* Reset errno */
    errno = 0;
    while((entry = readdir(dir)))
    {
        char *ts_test = NULL;

        if(strncmp(entry->d_name, file_prefix, strlen(file_prefix)) != 0)
        {
            continue;
        }

        /* Make sure timestamp comes right after prefix - necessary for snapshot file
            at least */
        ts_test = entry->d_name + strlen(file_prefix);
        if (ts_test && !isdigit(ts_test[0]))
        {
            continue;
        }

        ts_test = strrchr(entry->d_name, '.');
        if(ts_test && !isdigit(ts_test[1]))
        {
            fprintf(stderr, "Skip validating file '%s' (%s)", entry->d_name,ts_test);
            continue;
        }
        
        extension = entry->d_name + strlen(file_prefix);

        if((file_timestamp = strtoul(extension, NULL, 10)) == 0)
        {
            fprintf(stderr, "Failed to extract timestamp from '%s'",
                    entry->d_name);
            continue;
        }
        if(mode == 2)   /* return smallest timestamp >= specified */
        {
            if(file_timestamp < timestamp)
                continue;
            if((selected_timestamp != 0) && file_timestamp > selected_timestamp)
                continue;
            selected_timestamp = file_timestamp;
        }
        else if(mode == 1)   /* return timestamp > specified */
        {
            if(file_timestamp <= timestamp)
                continue;
            if((selected_timestamp != 0) && file_timestamp > selected_timestamp)
                continue;
            selected_timestamp = file_timestamp;
        }
        else            /* return timestamp <= specified */
        {
            if(selected_timestamp == 0)
            {
                selected_timestamp = file_timestamp;
                continue;
            }

            if(file_timestamp > timestamp)
            {
                if(selected_timestamp > file_timestamp)
                {
                    selected_timestamp = file_timestamp;
                }
            }
            else /* file_timestamp <= timestamp */
            {
                if(selected_timestamp <= file_timestamp)
                {
                    selected_timestamp = file_timestamp;
                }
            }
        }
    }

    if(errno == EBADF)
    {
        fprintf(stderr, "Error reading directory %s", strerror(errno));
        closedir(dir);
        return SF_EREAD;
    }

    closedir(dir);

    if(selected_timestamp == 0) /* no file found */
        return SF_ENOENT;


    if(next_timestamp)
        *next_timestamp = selected_timestamp;

    return 0;
}

