/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
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
 *****************************************************************************/

/**************************************************************************
 *
 * file_ss.c
 *
 * Authors: Bhargava Jandhyala <bjandhya@cisco.com>
 *
 * Description:
 *
 * File cache sharing support.
 *
 **************************************************************************/

#ifdef SIDE_CHANNEL

#include "file_ss.h"
#include "sidechannel.h"
#include "file_resume_block.h"
#include "packet_time.h"
#include <errno.h>
#include <fcntl.h>

#ifdef REG_TEST
typedef struct _MsgHeader
{
    uint32_t type;
    uint32_t data_length;
} MsgHeader;

#endif

typedef struct _FileSSStats
{
    uint32_t messages_received;
    uint32_t messages_sent;
} FileSSStats;

static uint8_t file_io_buffer[UINT16_MAX];
static FileSSStats file_ss_stats;

#ifdef REG_TEST
static int runtime_output_fd = -1;
static uint32_t WriteFileSSMsgHeader(uint8_t *, uint32_t , uint32_t );
static inline ssize_t Read(int , void *, size_t );
static inline ssize_t Write(int , const void *, size_t );
static int ReadFileSSMessagesFromFile(const char *);
static uint32_t WriteFileSSMsgHeader(uint8_t *, uint32_t , uint32_t );
#endif

void FilePrintSSStats(void)
{
    LogMessage("    File Cache Sharing:\n");
    LogMessage("        Messages Received: %u\n", file_ss_stats.messages_received);
    LogMessage("        Messages Sent: %u\n", file_ss_stats.messages_sent);
}

static int ConsumeFileSSMsg(uint32_t type, const uint8_t *msg, uint32_t msglen)
{
    int rval = 1;

    switch(type)
    {
        case SC_MSG_TYPE_FILE_SS_HOST_CACHE:
            rval = ConsumeSSFileCache(msg, msglen);
            break;
        default:
            break;
    }

    file_ss_stats.messages_received++;
    return rval;
}


#ifndef REG_TEST

static int FileSSSCMsgHandler(SCMsgHdr *hdr, const uint8_t *msg, uint32_t msglen)
{
    int rval = 1;

    if(!hdr)
        return rval;

    rval = ConsumeFileSSMsg(hdr->type, msg, msglen);

    return rval;
}

/*Message will be logged within 600 seconds*/
static ThrottleInfo error_throttleInfo = {0,600,0};

/* Caller should proceed writing to the 'data_ptr' ONLY upon successful return i.e 0 */
int CreateFileSSUpdate(void **msg_handle, void **hdr_ptr, void **data_ptr, uint32_t type, uint32_t data_len)
{
    FileConfig *file_config;
    SCMsgHdr *schdr;
    uint8_t *msg;
    int rval;
    file_config =  (FileConfig *)(snort_conf->file_config);
    if (!msg_handle || !hdr_ptr || !data_ptr)
    {
      ErrorMessage("Param %s is null for side channel update\n", msg_handle ? 
          (hdr_ptr ? "data pointer" : "header pointer") : "Message handle"); 
      return -1;
    }


    if (file_config->use_side_channel)
    {
        /* Allocate space for the message. */
        if ((rval = SideChannelPreallocMessageTX(data_len, &schdr, &msg, msg_handle)) != 0)
        {
            ErrorMessage("Unable to allocate memory for enqueing File SS message\n");
            return -1;
        }

        *hdr_ptr = (void *)schdr;
        *data_ptr = (void *)msg;
    }
    else
    {
#if defined(DAQ_VERSION) && DAQ_VERSION > 6
      if(ScSideChannelEnabled())
      {
        file_config->use_side_channel = true;
        ErrorMessage("Enabling file side channel runtime \n");
        /* Allocate space for the message. */
        if ((rval = SideChannelPreallocMessageTX(data_len, &schdr, &msg, msg_handle)) != 0)
        {
          ErrorMessage("Unable to allocate memory for enqueing File SS message after at runtime\n");
          return -1;
        }

        *hdr_ptr = (void *)schdr;
        *data_ptr = (void *)msg;
      }
      else
#endif
      {
        ErrorMessageThrottled(&error_throttleInfo, "side channel infra is not up\n");
        return -1;
      }
    }
    return 0;
}
int SendFileSSUpdate(void *msg_handle, void *hdr_ptr, void *data_ptr, uint32_t type, uint32_t data_len)
{
   FileConfig *file_config;
   file_config =  (FileConfig *)(snort_conf->file_config);
   SCMsgHdr *schdr;
   uint8_t *msg;

   if (!msg_handle || !hdr_ptr || !data_ptr)
   {
        return -1;
   }

    if (file_config->use_side_channel)
    {
        schdr = (SCMsgHdr *)hdr_ptr;
        msg = (uint8_t *)data_ptr;

        schdr->type = type;
        schdr->timestamp = packet_time();

        SideChannelEnqueueMessageTX(schdr, msg, data_len, msg_handle, NULL);
        file_ss_stats.messages_sent++;
    }
    return 0;
}

void FileSSConfigInit(void)
{
    int rval;
    FileConfig *file_config;
    file_config =  (FileConfig *)(snort_conf->file_config);
    if ((rval = SideChannelRegisterRXHandler(SC_MSG_TYPE_FILE_SS_HOST_CACHE, FileSSSCMsgHandler, NULL)) != 0)
    {
          ErrorMessage("Unable to register File SS message handler\n");
    }
    return;
}
#endif
#ifdef REG_TEST
/*
 * File I/O for regression only 
 */
void FileCleanSS(void)
{
    if (runtime_output_fd >= 0)
    {
        close(runtime_output_fd);
        runtime_output_fd = -1;
    }
}
void FilePrintSSConfig(FileSSConfig *file_ss_config)
{
    if (file_ss_config == NULL)
        return;

    LogMessage("    File SS config:\n");
    if (file_ss_config->startup_input_file)
        LogMessage("        Startup Input File:    %s\n", file_ss_config->startup_input_file);
    if (file_ss_config->runtime_output_file)
        LogMessage("        Runtime Output File:   %s\n", file_ss_config->runtime_output_file);
    return;
}
int CreateFileSSUpdate(void **msg_handle, void **hdr_ptr, void **data_ptr, uint32_t type, uint32_t data_len)
{
    if (!data_ptr)
        return -1;

    if (runtime_output_fd >= 0)
    {
        WriteFileSSMsgHeader(file_io_buffer, type, data_len);
        *data_ptr = (void *)(&file_io_buffer[0] + sizeof(MsgHeader));
    }
    else
    {
        return -1;
    }
    return 0;
}
int SendFileSSUpdate(void *msg_handle, void *hdr_ptr, void *data_ptr, uint32_t type, uint32_t data_len)
{
    if (runtime_output_fd >= 0)
    {
        if (Write(runtime_output_fd, file_io_buffer, sizeof(MsgHeader) + data_len) == -1)
        {
            /* Already reported Error inside the 'Write' call */
        }
        else
        {
            file_ss_stats.messages_sent++;
        }
    }
    return 0;
}
void FileSSConfigInit(void)
{
    int rval;
    FileConfig *file_config;
    file_config =  (FileConfig *)(snort_conf->file_config);
    /* Probably need to do this through a different function for Reload. But we're okay for now. */
    if (file_config && file_config->file_ss_config)
    {
        if (file_config->file_ss_config->startup_input_file)
        {
            if ((rval = ReadFileSSMessagesFromFile(file_config->file_ss_config->startup_input_file)) != 0)
            {
                ErrorMessage("Errors were encountered while reading File SS messages from file '%s'\n",
                        file_config->file_ss_config->startup_input_file);
            }
        }

        if (file_config->file_ss_config->runtime_output_file)
        {
            runtime_output_fd = open(file_config->file_ss_config->runtime_output_file, O_WRONLY | O_CREAT | O_TRUNC, 0664);
            if (runtime_output_fd < 0)
            {
                ErrorMessage("Could not open %s for writing File SS messages: %s (%d)\n",
                        file_config->file_ss_config->runtime_output_file, strerror(errno), errno);
            }
        }
    }
}
static inline ssize_t Read(int fd, void *buf, size_t count)
{
    ssize_t n;
    errno = 0;

    while ((n = read(fd, buf, count)) <= (ssize_t) count)
    {
        if (n == (ssize_t) count)
            return 0;

        if (n > 0)
        {
            buf = (uint8_t *) buf + n;
            count -= n;
        }
        else if (n == 0)
            break;
        else if (errno != EINTR)
        {
            ErrorMessage("Error reading from File SS message file: %s (%d)\n", strerror(errno), errno);
            break;
        }
    }
    return -1;
}

static inline ssize_t Write(int fd, const void *buf, size_t count)
{
    ssize_t n;
    errno = 0;

    while ((n = write(fd, buf, count)) <= (ssize_t) count)
    {
        if (n == (ssize_t) count)
            return 0;

        if (n > 0)
            count -= n;
        else if (errno != EINTR)
        {
            ErrorMessage("Error writing to File SS message file: %s (%d)\n", strerror(errno), errno);
            break;
        }
    }

    return -1;
}

static int ReadFileSSMessagesFromFile(const char *filename)
{
    MsgHeader *msg_header;
    uint8_t *msg;
    int rval = 0, fd, offset = 0;

    fd = open(filename, O_RDONLY , 0664);
    if (fd < 0)
    {
        if (errno == ENOENT)
            return 0;

        ErrorMessage("Could not open %s for reading File SS messages from: %s (%d)\n", filename, strerror(errno), errno);
    }

    msg = file_io_buffer;

    while ((rval = Read(fd, msg, sizeof(*msg_header))) == 0)
    {
        msg_header = (MsgHeader *) msg;
        offset = sizeof(*msg_header);

        if ((rval = Read(fd, msg + offset, msg_header->data_length)) != 0)
        {
            ErrorMessage("Error reading the remaining %u bytes of File SS message from file: %s (%d)\n",
                msg_header->data_length, strerror(errno), errno);

            close(fd);
            return rval;
        }

        if ((rval = ConsumeFileSSMsg(msg_header->type, msg + offset, msg_header->data_length) != 0))
        {
            close(fd);
            return rval;
        }

        offset += msg_header->data_length;
        msg += offset;
    }
    close(fd);
    return 0;
}

static uint32_t WriteFileSSMsgHeader(uint8_t *msg, uint32_t type, uint32_t data_len)
{
    MsgHeader *msg_hdr;

    msg_hdr = (MsgHeader *) msg;
    msg_hdr->type = type;
    msg_hdr->data_length = data_len;

    return 0;
}
#endif
#endif /* SIDE_CHANNEL */
