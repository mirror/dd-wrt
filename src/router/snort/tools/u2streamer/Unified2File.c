
/* System includes */
#include <sys/types.h>
#ifdef LINUX
#include <stdint.h>
#endif
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

/* Sourcefire includes */
#include <sf_error.h>
#include <TimestampedFile.h>

/* Local includes */
#include "Unified2.h"
#include "Unified2File.h"

#define U2FILE_STATUS_NOT_READY                 0
#define U2FILE_STATUS_HEADER_READY              1
#define U2FILE_STATUS_HEADER_PARTIAL            2
#define U2FILE_STATUS_EXTENDED_HEADER_READY     3
#define U2FILE_STATUS_EXTENDED_HEADER_PARTIAL   4
#define U2FILE_STATUS_DATA_READY                5
#define U2FILE_STATUS_DATA_PARTIAL              6

#ifndef MAX_U2_MESSAGE
#define  MAX_U2_MESSAGE (16*1024*1024)
#endif
#define U2R_EXTENDED_HEADER_BIT 0x80000000


/* Unified2 File API **********************************************************/
int Unified2File_Open(char *filepath, Unified2File **u2_file)
{
    Unified2File *tmp;

    if(!filepath || !u2_file)
        return SF_EINVAL;

    if(!(tmp = (Unified2File *)calloc(1, sizeof(Unified2File))))
    {
        fprintf(stderr, "Out of memory (wanted %zu bytes)", sizeof(Unified2File));
        return SF_ENOMEM;
    }

    tmp->fd = -1;
    tmp->read_status = U2FILE_STATUS_HEADER_READY;
    tmp->read_errno = 0;
    tmp->read_offset = 0;
    tmp->u2_record = NULL;

    if((tmp->fd = open(filepath, O_RDONLY)) == -1)
    {
        fprintf(stderr, "Unable to open file '%s': %s",
                filepath, strerror(errno));
        free(tmp);
        return SF_EOPEN;  /* XXX better return code */
    }

    *u2_file = tmp;

    return SF_SUCCESS;
}

int Unified2File_Close(Unified2File *u2_file)
{
    if(!u2_file)
        return SF_EINVAL;

    if(u2_file->u2_record)
        Unified2Record_Destroy(u2_file->u2_record);
    u2_file->u2_record = NULL;

    if(u2_file->fd != -1)
        close(u2_file->fd);
    u2_file->fd = -1;

    u2_file->read_status = 0;

    free(u2_file);

    return SF_SUCCESS;
}


int Unified2File_Read(Unified2File *u2_file, Unified2Record **u2_record)
{
    ssize_t bytes_read;
    ssize_t bytes_wanted;
    int     error_count = 0;

    if(!u2_file || !u2_record)
        return SF_EINVAL;

    if(u2_file->read_status == U2FILE_STATUS_NOT_READY)
        return SF_EREAD;

    /* allocate a new record */
    if(!u2_file->u2_record)
    {
        /* XXX we should check that we are in the HEADER_READY state */
        if(!(u2_file->u2_record = (Unified2Record *)calloc(1,
                        sizeof(Unified2Record))))
        {
            fprintf(stderr, "Out of memory (wanted %zu bytes)",
                    sizeof(Unified2Record));
            return SF_ENOMEM;
        }
        u2_file->read_offset = 0;
        u2_file->read_status = U2FILE_STATUS_HEADER_READY;
    }

    if(u2_file->read_status == U2FILE_STATUS_HEADER_READY
            || u2_file->read_status == U2FILE_STATUS_HEADER_PARTIAL)
    {
read_again:
        /* read the header */
        bytes_wanted = sizeof(Serial_Unified2_Header);
        bytes_read = read(u2_file->fd,
                ((u_int8_t *)&u2_file->s_u2_hdr) + u2_file->read_offset,
                bytes_wanted - u2_file->read_offset);

        /* end of file **************************/
        if(bytes_read == 0)
        {
            if(u2_file->read_status == U2FILE_STATUS_HEADER_PARTIAL)
            {
                fprintf(stderr, "End of file within header");

                if(errno) return SF_EREAD_TRUNCATED;
                return SF_EREAD_PARTIAL;
            }

            //fprintf(stderr, "End of file on record boundary");

            return SF_END_OF_FILE;
        }
        /* Read error **************************/
        if(bytes_read == -1)
        {
            /* read error */
            fprintf(stderr, "Read error: %s",
                    strerror(errno));
            u2_file->read_errno = errno;
            u2_file->read_status = U2FILE_STATUS_NOT_READY;
            return SF_EREAD;
        }

        /* check for partial read *************/
        if(bytes_read + u2_file->read_offset < bytes_wanted)
        {
            u2_file->read_offset += bytes_read;
            u2_file->read_status = U2FILE_STATUS_HEADER_PARTIAL;
            fprintf(stderr, "Partial header read (%u of %zu bytes)",
                    u2_file->read_offset, bytes_wanted);
            return SF_EREAD_PARTIAL;
        }

        /* basic header read is complete */

        /* process basic header data */
        u2_file->u2_record->type = ntohl(u2_file->s_u2_hdr.type);
        u2_file->u2_record->length = ntohl(u2_file->s_u2_hdr.length);
        /* XXX we have enough info now to allocate storage for the data */

        if(!u2_file->u2_record->length || (u2_file->u2_record->length >= MAX_U2_MESSAGE))
        {
            /* Seek back to where we started, in case we want to try again */
            off_t rval = lseek(u2_file->fd, (0 - bytes_read), SEEK_CUR);
            fprintf(stderr, "Seek backwards %zu bytes, seek returns %ld", bytes_read, rval);

            error_count++;
            usleep(100);
            if (error_count >= 10)
            {
                fprintf(stderr, "Unsupported length: Tried to read (%d bytes - allowed %d) at offset %ld Type %u",
                        u2_file->u2_record->length,
                        MAX_U2_MESSAGE,
                        rval,
                        u2_file->u2_record->type & ~U2R_EXTENDED_HEADER_BIT);
                return SF_EBADLEN;
            }
            else
            {
                goto read_again;
            }
        }

        /* check to see if we have an extended header */
        if(u2_file->u2_record->type & U2R_EXTENDED_HEADER_BIT)
        {
            u2_file->read_status = U2FILE_STATUS_EXTENDED_HEADER_READY;
            u2_file->u2_record->type &= ~U2R_EXTENDED_HEADER_BIT;
        }
        else
        {
            u2_file->u2_record->timestamp = 0;
            u2_file->checksum = 0;
            u2_file->read_status = U2FILE_STATUS_DATA_READY;
        }

        u2_file->read_offset = 0;
    }
    if(error_count > 0)
    {
        fprintf(stderr, "Bogus corrupt file, re-read %d times before file valid.", error_count);
    }

    if(u2_file->read_status == U2FILE_STATUS_EXTENDED_HEADER_READY
            || u2_file->read_status == U2FILE_STATUS_EXTENDED_HEADER_PARTIAL)
    {
        /* read the header extensions */
        bytes_wanted = sizeof(Serial_Unified2HeaderExtension);
        bytes_read = read(u2_file->fd,
                ((u_int8_t *)&u2_file->s_u2_hdr_ext) + u2_file->read_offset,
                bytes_wanted - u2_file->read_offset);

        /* end of file **************************/
        if(bytes_read == 0)
        {
            fprintf(stderr, "End of file within header");

            if(errno) return SF_EREAD_TRUNCATED;
            return SF_EREAD_PARTIAL;
        }
        /* Read error **************************/
        if(bytes_read == -1)
        {
            /* read error */
            fprintf(stderr, "Read error: %s",
                    strerror(errno));
            u2_file->read_errno = errno;
            u2_file->read_status = U2FILE_STATUS_NOT_READY;
            return SF_EREAD;
        }

        /* check for partial read *************/
        if(bytes_read + u2_file->read_offset < bytes_wanted)
        {
            u2_file->read_offset += bytes_read;
            u2_file->read_status = U2FILE_STATUS_EXTENDED_HEADER_PARTIAL;
            fprintf(stderr, "Partial header read (%u of %zu bytes)",
                    u2_file->read_offset, bytes_wanted);
            return SF_EREAD_PARTIAL;
        }

        /* header extension read complete */
        /* process header extenstion data */
        //VLAD we do have an extended header?
        u2_file->u2_record->timestamp = ntohl(u2_file->s_u2_hdr_ext.timestamp);
        u2_file->checksum = ntohl(u2_file->s_u2_hdr_ext.checksum);

        u2_file->read_status = U2FILE_STATUS_DATA_READY;
        u2_file->read_offset = 0;
    }

    /* we should not have any of these, but just in case */
    if(u2_file->u2_record->length == 0)
    {
        u2_file->read_offset = 0;
        u2_file->read_status = U2FILE_STATUS_HEADER_READY;
        *u2_record = u2_file->u2_record;
        u2_file->u2_record = NULL;
        return SF_SUCCESS;
    }

    /* XXX some other length sanity checking may be desirable */

    /* read the actual data ***********************/
    if(u2_file->read_status == U2FILE_STATUS_DATA_READY
            || u2_file->read_status == U2FILE_STATUS_DATA_PARTIAL)
    {
        /* allocate memory if we have not done so yet */
        if(!u2_file->u2_record->data)
        {
            if(!u2_file->u2_record->length || (u2_file->u2_record->length >= MAX_U2_MESSAGE))
            {
                fprintf(stderr, "Unsupported length: Tried to read (%d bytes - allowed %d) Type %u",
                       u2_file->u2_record->length,
                       MAX_U2_MESSAGE,
                       u2_file->u2_record->type & ~U2R_EXTENDED_HEADER_BIT);
                return SF_EBADLEN;
            }

            /* allocate the buffer (we could do this earlier) */
            if(!(u2_file->u2_record->data = calloc(u2_file->u2_record->length,sizeof(uint8_t))))
            {
                fprintf(stderr,"Out of memory (wanted %u bytes)",u2_file->u2_record->length);
                return SF_ENOMEM;
                /* Amazingly enough, this is not a fatal error.  if the user
                 * frees up some memory, we can try again
                 */
            }
        }

        /* read the actual data */
        bytes_wanted = u2_file->u2_record->length;
        bytes_read = read(u2_file->fd,
                ((u_int8_t *)u2_file->u2_record->data) + u2_file->read_offset,
                bytes_wanted - u2_file->read_offset);

        /* end of file **************************/
        if(bytes_read == 0)
        {
            fprintf(stderr, "End of file reading data");

            if(errno) return SF_EREAD_TRUNCATED;
            return SF_EREAD_PARTIAL;
        }
        /* Read error **************************/
        if(bytes_read == -1)
        {
            /* read error */
            fprintf(stderr, "Read error: %s",
                    strerror(errno));
            u2_file->read_errno = errno;
            u2_file->read_status = U2FILE_STATUS_NOT_READY;
            return SF_EREAD;
        }

        /* check for partial read *************/
        if(bytes_read + u2_file->read_offset < bytes_wanted)
        {
            u2_file->read_offset += bytes_read;
            u2_file->read_status = U2FILE_STATUS_DATA_PARTIAL;
            fprintf(stderr, "Partial header read (%u of %zu bytes)",
                    u2_file->read_offset, bytes_wanted);
            return SF_EREAD_PARTIAL;
        }

        /* data read complete */

        if(u2_file->checksum != 0)
        {
            /* XXX validation code goes here */
        }

        u2_file->read_offset = 0;
        u2_file->read_status = U2FILE_STATUS_HEADER_READY;
        *u2_record = u2_file->u2_record;
        u2_file->u2_record = NULL;
        return SF_SUCCESS;
    }

    /* We should never get here */
    return SF_EINVAL;
}

