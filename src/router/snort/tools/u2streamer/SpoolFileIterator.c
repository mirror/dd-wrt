/*
 *  Copyright (C) 2003-2007 Sourcefire. Inc.     All Rights Reserved
 */

/* System includes */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <syslog.h>

/* Local includes */
#include <Unified2.h>
#include <UnifiedLog.h>
#include <Unified2File.h>

#include "SpoolFileIterator.h"
#include "TimestampedFile.h"
#include "sf_error.h"

#define STATUS_BAD  -1
#define STATUS_OK   0

#define SPOOL_FILE_TYPE_UNIFIED 1
#define SPOOL_FILE_TYPE_UNIFIED2 2

#define TO_IP(x) x >> 24, (x >> 16) & 0xff, (x >> 8) & 0xff, x & 0xff

/* Snort Unfied Log Iterator API **********************************************/

int SpoolFileIterator_Destroy(SpoolFileIterator *iterator);
int SpoolFileIterator_SetPosition(SpoolFileIterator *iterator,
        uint32_t extension, uint32_t position);
static int LoadData(SpoolFileIterator *iterator);
static int OpenFile(char *filepath, SpoolFileIterator *iterator);
static void CloseFile(SpoolFileIterator *iterator);
static int UpdateBookmark(SpoolFileIterator *iterator, uint32_t timestamp,
        uint32_t position);

static int BuildFilepath(SpoolFileIterator *iterator, char *file_prefix,
                uint32_t extension, char **filepath)
{
    char *tmp = NULL;
    ssize_t filepath_len;
    /* construct the complete filepath for the next file */
    filepath_len = strlen(iterator->directory) + 1 + strlen(file_prefix) + 16 + 1;
    if(!(tmp = (char *)calloc(filepath_len, sizeof(char))))
    {
        fprintf(stderr, "Out of memory (wanted %zu bytes)\n", filepath_len + 1);
        return SF_ENOMEM;
    }

    snprintf(tmp, filepath_len, "%s/%s%u", iterator->directory, file_prefix, extension);

    *filepath = tmp;
    return 0;
}


int SpoolFileIterator_New(const char *directory, char *file_prefix,
        const char *bookmark_file, SpoolFileIterator **p_iterator)
{
    SpoolFileIterator *iterator = NULL;
    int rval = 0;

    if(!directory || !file_prefix || !p_iterator)
        return SF_EINVAL;

    if(!(iterator = (SpoolFileIterator *)calloc(1,
                    sizeof(SpoolFileIterator))))
    {
        fprintf(stderr, "Out of memory (wanted %zu bytes)\n",
                sizeof(SpoolFileIterator));
        return SF_ENOMEM;
    }
    iterator->bookmark_fd = -1;
    iterator->file_prefix = file_prefix;

    if(!(iterator->directory = strdup(directory)))
    {
        fprintf(stderr, "Out of memory (wanted %zu bytes)\n",
                strlen(directory) + 1);
        rval = SF_ENOMEM;
        goto exit;
    }

    if(bookmark_file)
    {
        iterator->bookmark_file = strdup(bookmark_file);
        //create a file
    }

    iterator->search_mode = 0;
    iterator->tail_mode = 1;
    iterator->timestamp = 0;

exit:
    if(rval != 0)
        SpoolFileIterator_Destroy(iterator);
    else
        *p_iterator = iterator;

    return rval;
}

int SpoolFileIterator_Destroy(SpoolFileIterator *iterator)
{
    if(!iterator)
        return SF_EINVAL;

    if(iterator->bookmark_fd != -1)
        close(iterator->bookmark_fd);

    if(iterator->u2f)
        Unified2File_Close(iterator->u2f);

    free(iterator->bookmark_file);
    free(iterator->directory);
    free(iterator->filepath);

    if(iterator->unified2_record)
        Unified2Record_Destroy(iterator->unified2_record);

    free(iterator);

    return 0;
}

int ReadBookmarkData(SpoolFileIterator *iterator)
{
    int fd = -1;
    char buffer[256];   /* This is the most we would write */
    char *s_position = NULL;
    unsigned long position;
    unsigned long extension;

    ssize_t bytes_read;
    int rval = 0;

    if(!iterator || !iterator->bookmark_file)
        return SF_EINVAL;

    if((fd = open(iterator->bookmark_file, O_RDONLY)) == -1)
    {
        if (UpdateBookmark(iterator, 0, 0))
        {
            rval = errno;
            fprintf(stderr, "Failed to create file '%s': %s\n", iterator->bookmark_file, strerror(rval));
            return rval;
        }
        if((fd = open(iterator->bookmark_file, O_RDONLY)) == -1)
        {

            rval = errno;
            fprintf(stderr, "Failed to open file '%s': %s\n", iterator->bookmark_file, strerror(rval));
            return rval;
        }
    }

    errno = 0;
    /* Read from the file */
    memset(buffer, 0, sizeof(buffer));
    if((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) == -1)
    {
        if(fd != -1)
           close(fd);
        fd = -1;
        rval = errno;
        fprintf(stderr, "Failed to read from file '%s': %s\n",
                iterator->bookmark_file, strerror(rval));
        return rval;
    }

    /* Remove trailing newline */
    if((s_position = strchr(buffer, '\n')))
        *s_position = '\0';

    /* Parse the position */
    if(!(s_position = strchr(buffer, ',')))
    {
        fprintf(stderr, "Syntax error processing bookmark data '%s'\n",
                buffer);
        goto exit;
    }
    *s_position = '\0';
    s_position++;

    position = strtoul(s_position, NULL,0);
    if(errno)
    {
        fprintf(stderr, "Failed to parse position '%s': %s\n",
                s_position, strerror(errno));
        rval = errno;
        goto exit;
    }

    /* Parse the extension */
    extension = strtoul(buffer, NULL,0);
    if(errno)
    {
        fprintf(stderr, "Failed to parse extension '%s': %s\n",
                buffer, strerror(errno));
        rval = errno;
        goto exit;
    }

    /* Set the iterator start position */
    if((rval = SpoolFileIterator_SetPosition(iterator, extension, position))
            != 0)
    {
        fprintf(stderr, "Failed to set position: %s\n", strerror(rval));
    }

exit:
    if(fd != -1)
        close(fd);
    fd = -1;

    return rval;
}

static int UpdateBookmark(SpoolFileIterator *iterator, uint32_t timestamp,
        uint32_t position)
{
    char buffer[256];
    if(!iterator || !iterator->bookmark_file)
        return SF_EINVAL;

    if(iterator->bookmark_fd == -1)
    {
        if((iterator->bookmark_fd = open(iterator->bookmark_file,
                        //O_WRONLY | O_CREAT | O_SYNC, S_IRUSR | S_IWUSR)) == -1)
                        O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) == -1)
        {
            fprintf(stderr, "Unable to open file '%s': %s\n",
                    iterator->bookmark_file, strerror(errno));
            return -1;
        }
        /* XXX We may want to get a lock on this too */
    }

    memset(buffer, ' ', sizeof(buffer));
    snprintf(buffer, sizeof(buffer)-1, "%u, %u\n", timestamp, position);
    /* Set back to the beginning of the file */
    if (lseek(iterator->bookmark_fd, 0, SEEK_SET))
    {
        fprintf(stderr, "Unable to seek file '%s': %s\n",
                iterator->bookmark_file, strerror(errno));
        return -1;
    }
    if (write(iterator->bookmark_fd, buffer, sizeof(buffer)) < 0)
    {
        fprintf(stderr, "Unable to write file '%s': %s\n",
                iterator->bookmark_file, strerror(errno));
        return -1;
    }

    /* XXX Block signals here */
    /* XXX We may also want to check for errors */

    return 0;
}

static int FindNewestSpoolFile(SpoolFileIterator *iterator, char **found_file_prefix,
                               uint32_t *found_file_timestamp)
{
    uint32_t file_timestamp = 0;
    int rval = 0;

    while((rval = FindNextTimestampedFile(iterator->directory,
                    iterator->file_prefix, iterator->timestamp,
                    1, &file_timestamp)) == 0)
    {
        fprintf(stderr, "Found timestamp: %u\n",
                file_timestamp);

        if (found_file_prefix)
            *found_file_prefix = iterator->file_prefix;
        if (found_file_timestamp)
            *found_file_timestamp = file_timestamp;

        if(file_timestamp == iterator->timestamp)
            iterator->timestamp++;
        else
        {
            if(iterator->timestamp)
            {
                struct stat buf;
                /* Try to archive, since we found a newer one */
                if(iterator->filepath)
                    free(iterator->filepath);
                iterator->filepath = NULL;
                if((rval = BuildFilepath(iterator, iterator->file_prefix,
                                iterator->timestamp, &(iterator->filepath)))
                        != 0)
                {
                    fprintf(stderr, "Unable to build filepath: %s\n",
                            strerror(rval));
                    return SF_ENOENT;
                }

                if (stat(iterator->filepath, &buf))
                {
                    fprintf(stderr, "Unable to get file status: %s\n",
                            strerror(rval)); /* warning only */
                }
            }

            iterator->timestamp = file_timestamp;
        }

        fprintf(stderr, "Looking with timestamp: %u\n",
                iterator->timestamp);

        return SF_SUCCESS;
    }

    return SF_ENOENT;
}

static int FindNextSpoolFile(SpoolFileIterator *iterator,uint32_t timestamp, int mode,
                             char **next_file_prefix, uint32_t *next_file_timestamp)
{
    uint32_t file_timestamp = 0;
    int rval = 0;

    if((rval = FindNextTimestampedFile(iterator->directory,
                    iterator->file_prefix, timestamp, mode, &file_timestamp)) == 0)
    {
        if (next_file_prefix)
            *next_file_prefix = iterator->file_prefix;
        if (next_file_timestamp)
            *next_file_timestamp = file_timestamp;
        return 0;
    }

    return rval;
}


static int OpenNextFile(SpoolFileIterator *iterator)
{
    int rval = 0;
    char *filepath = NULL;
    char *file_prefix = NULL;
    uint32_t file_timestamp = 0;

    if(!iterator)
        return SF_EINVAL;

    if(iterator->go_to_end)
    {
        fprintf(stderr, "Looking with timestamp: %u\n",
                iterator->timestamp);

        /* Find the newest spool file */
        if ((rval = FindNewestSpoolFile(iterator, &file_prefix,
                      &file_timestamp)) != 0)
        {
            iterator->go_to_end = 0;
            return rval;
        }

        fprintf(stderr, "Using timestamp: %u\n", file_timestamp);
    }
    else
    {
        /* Find the next spool file */
        if((rval = FindNextSpoolFile(iterator, iterator->timestamp,
                        iterator->search_mode, &file_prefix,
                        &file_timestamp)) != 0)
        {
            /* No next file */
            if(rval == SF_ENOENT)  /* No files found, return */
            {
                return SF_EAGAIN;
            }
            fprintf(stderr,
                    "Error finding next timestamped file: %s\n",
                    strerror(rval));

            return rval;    /* other errors */
        }
    }

    /* We found a file, attempt to open it */
    if((rval = BuildFilepath(iterator,
                    file_prefix, file_timestamp, &filepath))
            != 0)
    {
        fprintf(stderr, "Unable to build filepath: %s\n",
                strerror(rval));
        goto exit;
    }
    //reads unified file header and just opens unified2 file to read
    rval = OpenFile(filepath, iterator);
    if(rval == 0)
    {
        fprintf(stderr, "Opened %s\n", filepath);
        if(iterator->filepath)
            free(iterator->filepath);
        iterator->filepath = filepath;
        filepath = NULL;
        iterator->timestamp = file_timestamp;
        iterator->file_extension = file_timestamp;
        iterator->file_prefix = file_prefix;
        iterator->record_number  = 0;
        iterator->search_mode = 1;
    }

exit:
    if(filepath)
    {
        free(filepath);
    }
    return rval;
}

#if 0
typedef struct _BookmarkRecord
{
    uint32_t extension;
    uint32_t position;
} BookmarkRecord;
#endif


int SpoolFileIterator_GetNext(SpoolFileIterator *iterator,
        Unified2Record **p_record, uint32_t *p_file, uint32_t *p_position)
{
    int rval = 0;
    Unified2Record *record = NULL;
    int offset;

    if((rval = LoadData(iterator)) != 0)
    {
        return rval;
    }

    /* Set pointer to record */
    record = iterator->unified2_record;

    offset = lseek(iterator->u2f->fd, 0, SEEK_CUR);
    if (offset < 0)
    {
        fprintf(stderr, "Failed to determine current file offset: %s",sf_strerror(rval));
        return rval;    
    }

    if((rval = UpdateBookmark(iterator, iterator->file_extension, offset)) != SF_SUCCESS)
    {   
        fprintf(stderr, "Failed to update bookmark: %s",sf_strerror(rval));
        return rval;    
    }   

    if(p_file)
    {
        *p_file = iterator->file_extension;
    }
    if(p_position)
    {
        *p_position = iterator->extra_data_record_number;
    }
    iterator->extra_data_record_number = 0;

    *p_record = record;

    return rval;
}

static void event3_dump(Unified2Record *record) 
{
    uint8_t *field;
    int i;

    Serial_Unified2IDSEvent event;

    memcpy(&event, record->data, sizeof(Serial_Unified2IDSEvent));

    /* network to host ordering */
    /* In the event structure, only the last 40 bits are not 32 bit fields */
    /* The first 11 fields need to be convertted */
    field = (uint8_t*)&event;
    for(i=0; i<11; i++, field+=4) {
        *(uint32_t*)field = ntohl(*(uint32_t*)field);
    }

    /* last 3 fields, with the exception of the last most since it's just one byte */
    *(uint16_t*)field = ntohs(*(uint16_t*)field); /* sport_itype */
    field += 2;
    *(uint16_t*)field = ntohs(*(uint16_t*)field); /* dport_icode */
    field +=6;
    *(uint32_t*)field = ntohl(*(uint32_t*)field); /* mpls_label */
    field += 4;
    /* policy_id and vlanid */
    for(i=0; i<2; i++, field+=2) {
        *(uint16_t*)field = ntohs(*(uint16_t*)field);
    }
    /* done changing the network ordering */


    syslog(LOG_ALERT|LOG_AUTH, "\"(Event)\""
            ",sensor_id=\"%u\",event_id=\"%u\",event_second=\"%u\",event_microsecond=\"%u\""
            ",sig_id=\"%u\",gen_id=\"%u\",revision=\"%u\",classification=\"%u\""
            ",priority=\"%u\",ip_source=\"%u.%u.%u.%u\",ip_destination=\"%u.%u.%u.%u\""
            ",src_port=\"%u\",dest_port=\"%u\",protocol=\"%u\",impact_flag=\"%u\",blocked=\"%u\""
            ",mpls_label=\"%u\",vland_id=\"%u\",policy_id=\"%u\",appid=\"%s\"\n",
             event.sensor_id, event.event_id,
             event.event_second, event.event_microsecond,
             event.signature_id, event.generator_id,
             event.signature_revision, event.classification_id,
             event.priority_id, TO_IP(event.ip_source),
             TO_IP(event.ip_destination), event.sport_itype,
             event.dport_icode, event.protocol,
             event.impact_flag, event.blocked,
             event.mpls_label, event.vlanId, event.pad2, event.app_name);

}
static void event3_6_dump(Unified2Record *record) 
{
    uint8_t *field;
    int i;
    char ip6Src[INET6_ADDRSTRLEN+1];
    char ip6Dst[INET6_ADDRSTRLEN+1];
    Serial_Unified2IDSEventIPv6 event;

    memcpy(&event, record->data, sizeof(Serial_Unified2IDSEventIPv6));

    /* network to host ordering */
    /* In the event structure, only the last 40 bits are not 32 bit fields */
    /* The first fields need to be convertted */
    field = (uint8_t*)&event;
    for(i=0; i<9; i++, field+=4) {
        *(uint32_t*)field = ntohl(*(uint32_t*)field);
    }

    field = field + 2*sizeof(struct in6_addr);

    /* last 3 fields, with the exception of the last most since it's just one byte */
    *(uint16_t*)field = ntohs(*(uint16_t*)field); /* sport_itype */
    field += 2;
    *(uint16_t*)field = ntohs(*(uint16_t*)field); /* dport_icode */
    field +=6;
    *(uint32_t*)field = ntohl(*(uint32_t*)field); /* mpls_label */
    field += 4;
    /* policy_id and vlanid */
    for(i=0; i<2; i++, field+=2) {
        *(uint16_t*)field = ntohs(*(uint16_t*)field);
    }
    /* done changing the network ordering */

    inet_ntop(AF_INET6, &event.ip_source, ip6Src, INET6_ADDRSTRLEN);
    inet_ntop(AF_INET6, &event.ip_destination, ip6Dst, INET6_ADDRSTRLEN);

    syslog(LOG_ALERT|LOG_AUTH, "\"(IPv6_Event)|\""
            ",sensor_id=\"%u\",event_id=\"%u\",event_second=\"%u\",event_microsecond=\"%u\""
            ",sig_id=\"%u\",gen_id=\"%u\",revision=\"%u\",classification=\"%u\""
            ",priority=\"%u\",ip_source=\"%s\","
            "ip_destination=\"%s\""
            ",src_port=\"%u\",dest_port=\"%u\",protocol=\"%u\",impact_flag=\"%u\",blocked=\"%u\""
            ",mpls_label=\"%u\",vland_id=\"%u\",policy_id=\"%u\",appid=\"%s\"\n",
             event.sensor_id, event.event_id,
             event.event_second, event.event_microsecond,
             event.signature_id, event.generator_id,
             event.signature_revision, event.classification_id,
             event.priority_id, ip6Src,
             ip6Dst, event.sport_itype,
             event.dport_icode, event.protocol,
             event.impact_flag, event.blocked,
             event.mpls_label, event.vlanId,event.pad2, event.app_name);

}
static void appid_dump(Unified2Record *record) {
    uint8_t *field = (uint8_t*)record->data;
    unsigned i;
    unsigned appCnt;
    unsigned statTime;

    /* network to host ordering */
    /* In the event structure, only the last 40 bits are not 32 bit fields */
    /* The first fields need to be convertted */
    statTime = ntohl(*(uint32_t*)field);
    field += 4;
    appCnt = ntohl(*(uint32_t*)field); /* mpls_label */
    field += 4;
    
    for(i=0; i<appCnt; i++) 
    {
        char appName[MAX_EVENT_APPNAME_LEN];
        memcpy(appName, field, sizeof(appName));
        field += MAX_EVENT_APPNAME_LEN;

        int txBytes = ntohl(*(uint32_t*)field);
        field += 4;
        int rxBytes = ntohl(*(uint32_t*)field);
        field += 4;

        syslog(LOG_ALERT|LOG_AUTH, "statTime=\"%u\",appName=\"%s\",txBytes=\"%u\",rxBytes=\"%u\"\n",
                statTime, appName, txBytes, rxBytes);
    }
}


static int ExtractUnified2Data(SpoolFileIterator *iterator, Unified2Record *unified2_record)
{
    if(!iterator || !unified2_record)
    {
        return SF_EINVAL;
    }

    switch (unified2_record->type) 
    {
        case UNIFIED2_IDS_EVENT_APPSTAT:
            appid_dump(unified2_record);
            break;
        case UNIFIED2_IDS_EVENT_APPID:
            event3_dump(unified2_record);
            break;
        case UNIFIED2_IDS_EVENT_APPID_IPV6:
            event3_6_dump(unified2_record);
            break;

    }

    return 0;
}

static int LoadData(SpoolFileIterator *iterator)
{
    int rval = 0;
    Unified2Record *unified2_record = NULL;

    /* validate arguments */
    if(!iterator)
    {
        return SF_EINVAL;
    }

    iterator->new_file_found = 0;

    if(iterator->status != STATUS_OK)
    {
        fprintf(stderr, "Iterator status is not OK\n");
        return -1;  /* XXX better return code */
    }

    if(!iterator->initialized && iterator->bookmark_file)
    {
        rval = ReadBookmarkData(iterator);
        if(rval != 0 && rval != SF_ENOENT)
        {
            fprintf(stderr, "Failed to process bookmark: %s\n",strerror(rval));
            return rval;
        }
        iterator->initialized = 1;
    }

    if(iterator->seek_set
            && iterator->file_extension != iterator->seek_extension)
    {
        CloseFile(iterator);
        iterator->file_extension = 0;
        iterator->record_number  = 0;
        iterator->timestamp      = iterator->seek_extension;
        iterator->search_mode    = 2;
    }

    while(1)
    {
        /* Find the file we need to use */
        if(!iterator->u2f)
        {
            if(iterator->tail_mode == 0)
            {
                return SF_ENOENT;
            }
            rval = OpenNextFile(iterator);
            /* If we could not open the next file because it only had a
             * partial header, we keep trying until we can open a file
             * completely.
             * XXX This could cause us to loop forever.  We
             * should kick the error back up to the parent so it can
             * decide when to retry.
             */
            if(rval == SF_ECONT)
            {
                //if SF_ECONT is set twice and  iterator->new_file_found == 1 and file size is zero - should archive?
                continue;
            }
            else if(rval != 0)
            {
                //fprintf(stderr, "No new file found for iterator - returning %d",rval);
                return rval;
            }

            /* Did we move past the desired file? */
            if(iterator->seek_set &&
               iterator->file_extension != iterator->seek_extension)
            {
                fprintf(stderr, "Wanted events from %u, but skipped to %u\n",
                        iterator->seek_extension, iterator->file_extension);
                iterator->seek_set = 0;
            }
        }

        /* If we get here, we have a valid open file handle */
        /* Attempt to read a record from the file */

        if (iterator->file_type == SPOOL_FILE_TYPE_UNIFIED2)
        {
            //fprintf(stderr, "NORMAL - UNIFIED2 LOG\n");
            rval = Unified2File_Read(iterator->u2f, &unified2_record);
        }

        if(rval == 0)
        {
            if (iterator->file_type == SPOOL_FILE_TYPE_UNIFIED2)
            {
                if((rval = ExtractUnified2Data(iterator, unified2_record)) != 0)
                {
                    if(rval == SF_ECORRUPT || rval == SF_EBADLEN)
                    {
                        CloseFile(iterator);
                        iterator->file_extension = 0;
                        iterator->record_number  = 0;
                    }

                    fprintf(stderr,"Failed to process unified2 record: %s\n",strerror(rval));
                    return rval;
                }

                if (iterator->unified2_record)
                    Unified2Record_Destroy(iterator->unified2_record);
                iterator->unified2_record = unified2_record;
                unified2_record = NULL;
            }

            if(iterator->go_to_end)
            {
                continue;
            }
            else if(iterator->seek_set)
            {
                iterator->seek_set = 0;
            }

            return 0;
        }

        /* Read failed */

        /* Bail out if we are not in tail mode */
        if(iterator->tail_mode == 0)
        {
            CloseFile(iterator);
            iterator->status = -1;
            iterator->file_extension = 0;
            iterator->record_number  = 0;
            return rval;
        }

        /* Invalidate the iterator on fatal errors */
        if(    rval != SF_EREAD_PARTIAL &&
               rval != SF_ENOENT        &&
               rval != SF_ECORRUPT      &&
               rval != SF_EBADLEN       &&
               rval != SF_END_OF_FILE)
        {
            /* Fatal errors */
            fprintf(stderr,"Error reading unified log record: %s\n",strerror(rval));

            CloseFile(iterator);
            iterator->status = -1;
            iterator->file_extension = 0;
            iterator->record_number  = 0;
            return rval;
        }

        //rval = SF_ECORRUPT;//test only
        /* Close the unified file on fatal file errors */
        if(rval == SF_ECORRUPT || rval == SF_EBADLEN)
        {
            CloseFile(iterator);
            iterator->file_extension = 0;
            iterator->record_number  = 0;
            return rval;
        }

        /* Drop out of finding the most recent record */
        if(iterator->go_to_end)
        {
            iterator->go_to_end = 0;
            return SF_EAGAIN;
        }

        /* Is there a new file to rotate to? */
        //we should never get here unless we have a truncated unified file (not unified2)
        if(iterator->new_file_found)
        {
            //Hey! this is it: Unified File_Read returned SF_END_OF_FILE
            if(rval == SF_END_OF_FILE)
            {
                iterator->status = STATUS_OK;
            }
            else if(rval == SF_EREAD_PARTIAL || rval == SF_EREAD_TRUNCATED)
            {
                fprintf(stderr,"SNORT(CRITICAL): File %s read is truncated (%s)\n", iterator->filepath,strerror(rval));
                //ArchiveCurrentFile (iterator, "truncated");
            }
            else
            {
                fprintf(stderr,"SNORT(UNIFIED): File %s read error:%s\n", iterator->filepath,strerror(rval));
            }
            CloseFile(iterator);
            iterator->file_extension = 0;
            iterator->record_number  = 0;

            /* Reset the new file found flag */
            iterator->new_file_found = 0;
            continue;
        }
        else
        {
            int rval_read = rval;
            rval = FindNextSpoolFile(iterator, iterator->timestamp, 1, NULL, NULL);
            if(rval == 0)
            {
                if(rval_read == SF_EREAD_PARTIAL || rval_read == SF_EREAD_TRUNCATED)
                {
                    iterator->new_file_found = 1;//need it only for the last read of a unified file
                    iterator->err_status = 0;
                    usleep(100); //give it the last chance to finish the last record
                }
                //for unified2->SF_END_OF_FILE, for unified->ENOENT --> Done reading
                else if(rval_read == SF_END_OF_FILE || rval_read == SF_ENOENT)
                {
                    CloseFile(iterator);
                    iterator->status = STATUS_OK;
                    iterator->file_extension = 0;
                    iterator->record_number  = 0;
                }
                else
                {
                    fprintf(stderr,"SNORT: File %s read error:%s\n",iterator->filepath,strerror(rval_read));
                }
                continue;
            }
            else if(rval != SF_ENOENT)
            {
                /* Fatal search error */
                fprintf(stderr,"Error finding next timestamped file %s\n",strerror(rval));
                return rval;
            }
            iterator->err_status = SF_EREAD_PARTIAL;
        }
        return SF_EAGAIN;
    }
    return 0;
}

int SpoolFileIterator_SetPosition(SpoolFileIterator *iterator,
        uint32_t extension, uint32_t position)
{
    if(!iterator)
    {
        return SF_EINVAL;
    }

    if(extension == 0)
    {
        iterator->go_to_end = 1;
        iterator->seek_set = 0;
    }
    else
    {
        iterator->seek_set = 1;
        iterator->go_to_end = 0;
        iterator->seek_extension = extension;
        iterator->seek_position  = position;
    }

    return 0;
}

static void CloseFile(SpoolFileIterator *iterator)
{
    if (iterator->file_type == SPOOL_FILE_TYPE_UNIFIED2)
    {
        Unified2File_Close(iterator->u2f);
        iterator->u2f = NULL;
    }
}

static int OpenFile(char *filepath, SpoolFileIterator *iterator)
{
    int rval = 0;

    if((rval = Unified2File_Open(filepath, &iterator->u2f)) == 0)
    {
        iterator->file_type = SPOOL_FILE_TYPE_UNIFIED2;
    }

    return rval;
}

