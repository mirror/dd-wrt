/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
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
 *
 ****************************************************************************/

// @file    shmem_datamgmt.c
// @author  Pramod Chandrashekar <pramod@sourcefire.com>

#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <assert.h>

#include "shmem_config.h"
#include "shmem_common.h"

#define MANIFEST_SEPARATORS         ",\r\n"
#define MIN_MANIFEST_COLUMNS         3

#define WHITE_TYPE_KEYWORD       "white"
#define BLACK_TYPE_KEYWORD        "block"
#define MONITOR_TYPE_KEYWORD      "monitor"


static const char* const MODULE_NAME = "ShmemFileMgmt";

// FIXME eliminate these globals
ShmemDataFileList **filelist_ptr = NULL;
int filelist_size = 0; // Number of slots in the filelist
int filelist_count = 0; // Number of 'used' slots in the file list 

static inline bool FileListFull( )
{
    return (filelist_count == filelist_size);
}

static int StringCompare(const void *elem1, const void *elem2)
{
    ShmemDataFileList * const *a = elem1;
    ShmemDataFileList * const *b = elem2;

    return strcmp((*a)->filename,(*b)->filename);
}

/* (Re)allocate *filelist to the next bucket size.
 * See `man 3 realloc' for deeper insight.
 */
static ShmemDataFileList**
ShmemDataFileList_Realloc ( ShmemDataFileList *filelist[] )
{
    ShmemDataFileList **temp;
    int _size = filelist_size + FILE_LIST_BUCKET_SIZE;

    // Don't call this function unnecessarily.
    assert( filelist_size == 0 || filelist_size > filelist_count );

    // Use errno to communicate any problems 
    errno = 0;

    if ( filelist_size >= MAX_IPLIST_FILES )
    {
        errno = ENOSPC;
        return NULL;
    }

    if ( _size > MAX_IPLIST_FILES )
        _size = MAX_IPLIST_FILES;

    // Rely on errno being set by realloc. 
    temp = realloc(filelist, _size * sizeof(*filelist));
    if ( temp == NULL )
        return NULL;

    filelist_size = _size;
    return temp;
}

static void FreeShmemDataFileListFiles()
{
    int i;

    if ( !filelist_count || !filelist_ptr )
        return;

    for(i = 0; i < filelist_count; i++)
    {
        free(filelist_ptr[i]->filename);
        free(filelist_ptr[i]);
        filelist_ptr[i] = NULL;
    }
    filelist_count = 0;
}

void FreeShmemDataFileList()
{
    if ( !filelist_ptr )
        return;

    FreeShmemDataFileListFiles( );

    free(filelist_ptr);
    filelist_ptr = NULL;
    filelist_size = 0;
}

static int ReadShmemDataFilesWithoutManifest()
{
    struct dirent *de;
    DIR *dd;

    FreeShmemDataFileListFiles();

    if ((dd = opendir(shmusr_ptr->path)) == NULL)
    {
        _dpd.errMsg("%s: Could not access %s: %s\n",
                MODULE_NAME, shmusr_ptr->path, strerror(errno));
        return SF_EINVAL;
    }

    while (( de = readdir(dd) ))
    {
        char filename[PATH_MAX];
        ShmemDataFileList *listinfo;
        const char *ext;
        int type;

        ext = strrchr(de->d_name , '.');
        if ( ext == NULL || *(ext + 1) == '\0' )
            continue;

        if ( strcasecmp(ext, ".blf") == 0 )
            type = BLACK_LIST;
        else if ( strcasecmp(ext, ".wlf") == 0 )
            type = WHITE_LIST;
        else 
            continue;

        if ( FileListFull() )
        {
            ShmemDataFileList **_temp = ShmemDataFileList_Realloc( filelist_ptr );

            if ( !_temp )
            {
                closedir(dd);

                if ( errno == ENOSPC )
                {
                    _dpd.errMsg("%s: Cannot load more than %u ip lists.",
                            MODULE_NAME, MAX_IPLIST_FILES);
                    return SF_ENOSPC;
                }

                DynamicPreprocessorFatalMessage("%s: Failed to allocate filelist_ptr: %s\n",
                        MODULE_NAME, strerror(errno));
                return SF_ENOMEM;
            }

            filelist_ptr = _temp;
        }
        
        listinfo = (ShmemDataFileList *)calloc(1, sizeof(*listinfo));
        if ( listinfo == NULL )
        {
            DynamicPreprocessorFatalMessage(
                "%s: Cannot allocate memory to store file information.\n",
                MODULE_NAME);
        }

        snprintf(filename, sizeof(filename), "%s/%s", shmusr_ptr->path, de->d_name);
        listinfo->filename = strdup(filename);
        if ( listinfo->filename == NULL )
        {
            free(listinfo);
            closedir(dd);
            DynamicPreprocessorFatalMessage("%s: Error resolving filename: %s\n",
                MODULE_NAME, strerror(errno));
        }

        listinfo->filetype = type;
        listinfo->listid = 0;
        memset(listinfo->zones, true, MAX_NUM_ZONES);

        filelist_ptr[filelist_count] = listinfo;
        filelist_count++;
    }

    closedir(dd);
    return SF_SUCCESS;
}

/*Ignore the space characters from string*/
static char *ignoreStartSpace(char *str)
{
    while((*str) && (isspace((int)*str)))
    {
        str++;
    }
    return str;
}

/*Get file type */
static int getFileTypeFromName (char *typeName)
{
    int type = UNKNOWN_LIST;

    /* Trim the starting spaces */
    if (!typeName)
        return type;

    typeName = ignoreStartSpace(typeName);

    if (strncasecmp(typeName, WHITE_TYPE_KEYWORD, strlen(WHITE_TYPE_KEYWORD)) == 0)
    {
        type = WHITE_LIST;
        typeName += strlen(WHITE_TYPE_KEYWORD);
    }
    else if (strncasecmp(typeName, BLACK_TYPE_KEYWORD, strlen(BLACK_TYPE_KEYWORD)) == 0)
    {
        type = BLACK_LIST;
        typeName += strlen(BLACK_TYPE_KEYWORD);
    }
    else if (strncasecmp(typeName, MONITOR_TYPE_KEYWORD, strlen(MONITOR_TYPE_KEYWORD)) == 0)
    {
        type = MONITOR_LIST;
        typeName += strlen(MONITOR_TYPE_KEYWORD);
    }

    if (UNKNOWN_LIST != type )
    {
        /*Ignore spaces in the end*/
        typeName = ignoreStartSpace(typeName);

        if ( *typeName )
        {
            type = UNKNOWN_LIST;
        }

    }

    return type;

}

/*  Parse the line item in manifest file
 *
 *  The format of manifest is:
 *    file_name, list_id, action (block, white, monitor), zone information
 *
 *  If no zone information provided, this means all zones are applied.
 *
 * */

static ShmemDataFileList* processLineInManifest(char *manifest, char *line, int linenumber)
{
    char* token;
    int tokenIndex = 0;
    ShmemDataFileList* listItem = NULL;
    char* nextPtr = line;
    char filename[PATH_MAX];
    bool hasZone = false;

    if ((listItem = (ShmemDataFileList*)calloc(1,sizeof(ShmemDataFileList))) == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Cannot allocate memory to "
                "store reputation manifest file information\n", manifest, linenumber);
        return NULL;
    }

    while((token = strtok_r(nextPtr, MANIFEST_SEPARATORS, &nextPtr)) != NULL)
    {
        char *endStr;
        long zone_id;
        long list_id;

        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Process reputation list token: %s\n",token ););

        switch (tokenIndex)
        {
        case 0:    /* File name */
            DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Reputation list filename: %s\n",token ););

            snprintf(filename, sizeof(filename), "%s/%s", shmusr_ptr->path,token);
            listItem->filename = strdup(filename);
            if (listItem->filename == NULL)
            {
                free(listItem);
                listItem = NULL;
                DynamicPreprocessorFatalMessage(
                        "%s(%d) => Error resolving filename: %s\n",
                        manifest, linenumber, strerror(errno));
            }
            break;

        case 1:    /* List ID */

            list_id = _dpd.SnortStrtol( token, &endStr, 10);

            /*Ignore spaces in the end*/
            endStr = ignoreStartSpace(endStr);

            if ( *endStr )
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Bad value (%s) specified for listID. "
                        "Please specify an integer between %d and %li.\n",
                        manifest, linenumber, token, 0, MAX_LIST_ID);
            }

            if ((list_id < 0)  || (list_id > MAX_LIST_ID) || (errno == ERANGE))
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => Value specified (%s) is out of "
                        "bounds.  Please specify an integer between %d and %li.\n",
                        manifest, linenumber, token, 0, MAX_LIST_ID);
            }
            listItem->listid = (uint32_t) list_id;
            break;

        case 2:    /* Action */
            token = ignoreStartSpace(token);
            listItem->filetype = getFileTypeFromName(token);
            if (UNKNOWN_LIST == listItem->filetype)
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => Unknown action specified (%s)."
                        " Please specify a value: %s | %s | %s.\n", manifest, linenumber, token,
                        WHITE_TYPE_KEYWORD, BLACK_TYPE_KEYWORD, MONITOR_TYPE_KEYWORD);
            }
            break;

        default:

            /*Ignore spaces in the beginning*/
            token= ignoreStartSpace(token);
            if (!(*token))
               break;

            zone_id = _dpd.SnortStrtol( token, &endStr, 10);

            /*Ignore spaces in the end*/
            endStr = ignoreStartSpace(endStr);

            if ( *endStr )
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Bad value (%s) specified for zone. "
                        "Please specify an integer between %d and %li.\n",
                        manifest, linenumber, token, 0, MAX_NUM_ZONES - 1);
            }
            if ((zone_id < 0)  || (zone_id >= MAX_NUM_ZONES ) || (errno == ERANGE))
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => Value specified (%s) for zone is "
                        "out of bounds. Please specify an integer between %d and %li.\n",
                        manifest, linenumber, token, 0, MAX_NUM_ZONES - 1);
            }

            listItem->zones[zone_id] = true;
            hasZone = true;
        }
        tokenIndex++;
    }

    if ( tokenIndex < MIN_MANIFEST_COLUMNS )
    {
        free(listItem);

        if ( tokenIndex > 0 )
        {
            DynamicPreprocessorFatalMessage("%s(%d) => Too few columns in line: %s.\n",
                    manifest, linenumber, line);
        }
        return NULL;
    }

    if (false == hasZone)
    {
        memset(listItem->zones, true, MAX_NUM_ZONES);
    }
    return listItem;
}

/*Parse the manifest file*/
static int ReadShmemDataFilesWithManifest()
{
    FILE *fp;
    char line[MAX_MANIFEST_LINE_LENGTH];
    char manifest_file[PATH_MAX];
    int  line_number = 0;

    snprintf(manifest_file, sizeof(manifest_file),
            "%s/%s",shmusr_ptr->path, MANIFEST_FILENAME);

    if ((fp = fopen(manifest_file, "r")) == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
                "Error opening file at: %s\n", manifest_file););
        return SF_ENOENT;
    }

    FreeShmemDataFileListFiles();

    while (fgets(line, sizeof(line),fp))
    {
        char* nextPtr = NULL;
        ShmemDataFileList* listItem;

        line_number++;

        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Reputation manifest: %s\n",line ););
        /* remove comments */

        if( (nextPtr = strchr(line, '#')) )
            *nextPtr = '\0';

        if ( FileListFull() )
        {
            ShmemDataFileList **_temp = ShmemDataFileList_Realloc( filelist_ptr );

            if ( !_temp )
            {
                fclose(fp);

                if ( errno == ENOSPC )
                {
                    _dpd.errMsg("%s: Cannot load more than %u ip lists.",
                            MODULE_NAME, MAX_IPLIST_FILES);
                    return SF_ENOSPC;
                }

                DynamicPreprocessorFatalMessage("%s: Failed to allocate filelist_ptr: %s\n",
                        MODULE_NAME, strerror(errno));
                return SF_ENOMEM;
            }

            filelist_ptr = _temp;
        }

        /*Processing the line*/
        listItem = processLineInManifest(manifest_file, line, line_number);
        if ( listItem )
        {
            filelist_ptr[filelist_count] = listItem;
            filelist_count++;
        }
    }

    fclose(fp);
    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
            "Successfully processed manifest file: %s\n", MANIFEST_FILENAME););

    return SF_SUCCESS;
}

int GetSortedListOfShmemDataFiles()
{
    int rval;

    if ((rval = ReadShmemDataFilesWithManifest()) == SF_ENOENT)
    {
        if ((rval = ReadShmemDataFilesWithoutManifest()) != SF_SUCCESS)
        {
            return rval;
        }

        // Only sort when not using a manifest file; manifest ip-lists
        // need to be used in the order specified used as-is.
        qsort(filelist_ptr, filelist_count, sizeof(*filelist_ptr), StringCompare);
    }

    return SF_SUCCESS;
}

//valid version values are 1 through UINT_MAX
int GetLatestShmemDataSetVersionOnDisk(uint32_t* shmemVersion)
{
    unsigned long tmpVersion;
    FILE *fp;
    char line[PATH_MAX];
    char version_file[PATH_MAX];
    const char *const key = "VERSION";
    char* keyend_ptr      = NULL;

    snprintf(version_file, sizeof(version_file),
            "%s/%s",shmusr_ptr->path,VERSION_FILENAME);

    if ((fp = fopen(version_file, "r")) == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
                "Error opening file at: %s\n", version_file););
        return SF_ENOENT;
    }

    while (fgets(line,sizeof(line),fp))
    {
        char *strptr;

        if ( *line == '#' )
            continue;

        if ( (strptr = strstr(line, key )) && (strptr == line) )
        {
            if ( strlen(line) <= (strlen(key) + 1) )
                break;

            keyend_ptr  = line;
            keyend_ptr += strlen(key) + 1;
            tmpVersion  = strtoul(keyend_ptr,NULL,0);
            break;
        }
    }

    if (!keyend_ptr)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
                "Invalid file format %s\n", version_file););
        fclose(fp);
        return SF_ENOENT;
    }

    if (tmpVersion > UINT_MAX) //someone tampers with the file
        *shmemVersion = 1;
    else
        *shmemVersion = (uint32_t)tmpVersion;

    fclose(fp);

    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
            "version information being returned is %u\n", *shmemVersion););

    return SF_SUCCESS;
}

#ifdef DEBUG_MSGS
void PrintListInfo (bool *zones, uint32_t listid)
{
    char zonesInfo[MAX_MANIFEST_LINE_LENGTH];
    int zone_id;

    int buf_len = sizeof(zonesInfo);
    char *out_buf = zonesInfo;
    for (zone_id = 0; zone_id < MAX_NUM_ZONES; zone_id++)
    {
        int bytesOutput;

        if (!zones[zone_id])
            continue;

        bytesOutput = snprintf(out_buf, buf_len, "%d,",zone_id);
        out_buf += bytesOutput;
        buf_len -= bytesOutput;

    }
    DebugMessage(DEBUG_REPUTATION, "List %li has zones defined: %s \n",
            listid, zonesInfo);
}

void PrintDataFiles()
{
    int i;

    for (i=0;i< filelist_count;i++)
    {
        DebugMessage(DEBUG_REPUTATION, "File %s of type %d found \n",
                filelist_ptr[i]->filename, filelist_ptr[i]->filetype);

        if (filelist_ptr[i]->listid)
        {
            PrintListInfo(filelist_ptr[i]->zones, filelist_ptr[i]->listid);
        }
    }
}
#endif /* DEBUG_MSGS */

