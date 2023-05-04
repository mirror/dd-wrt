/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2012-2013 Sourcefire, Inc.
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
 * appId_ss.c
 *
 * Authors: Hareesh Vutla <hvutla@cisco.com>
 *
 * Description:
 *
 * AppId state sharing support.
 *
 **************************************************************************/

#ifdef SIDE_CHANNEL

#include "appId_ss.h"
#include "appIdConfig.h"
#include "common_util.h"
#include "hostPortAppCache.h"
#include "profiler.h"
#include <errno.h>
#include <fcntl.h>

#ifdef REG_TEST
typedef struct _MsgHeader
{
    uint32_t type;
    uint32_t data_length;
} MsgHeader;
#endif

typedef struct
{
    uint32_t messages_received;
    uint32_t messages_sent;
} AppIdSSStats;

static int runtime_output_fd = -1;
static uint8_t file_io_buffer[UINT16_MAX];
static AppIdSSStats appId_ss_stats;

void AppIdPrintSSStats(void)
{
    _dpd.logMsg("    AppId State Sharing:\n");
    _dpd.logMsg("        Messages Received: %u\n", appId_ss_stats.messages_received);
    _dpd.logMsg("        Messages Sent: %u\n", appId_ss_stats.messages_sent);
}

void AppIdResetSSStats(void)
{
    memset(&appId_ss_stats, 0, sizeof(appId_ss_stats));
}

void AppIdPrintSSConfig(AppIdSSConfig *appId_ss_config)
{
    if (appId_ss_config == NULL)
        return;

#ifdef REG_TEST
    _dpd.logMsg("    AppId SS config:\n");
    if (appId_ss_config->startup_input_file)
        _dpd.logMsg("        Startup Input File:    %s\n", appId_ss_config->startup_input_file);
    if (appId_ss_config->runtime_output_file)
        _dpd.logMsg("        Runtime Output File:   %s\n", appId_ss_config->runtime_output_file);
#endif
}

void AppIdSSConfigFree(AppIdSSConfig *appId_ss_config)
{
    if (appId_ss_config == NULL)
        return;

#ifdef REG_TEST
    if (appId_ss_config->startup_input_file)
        free(appId_ss_config->startup_input_file);

    if (appId_ss_config->runtime_output_file)
        free(appId_ss_config->runtime_output_file);
#endif

    _dpd.snortFree(appId_ss_config, sizeof(*appId_ss_config),
        PP_APP_ID, PP_MEM_CATEGORY_CONFIG);
}

static int ConsumeAppIdSSMsg(uint32_t type, const uint8_t *msg, uint32_t msglen)
{
    int rval = 1;

    switch(type)
    {
        case SC_MSG_TYPE_APPID_SS_HOST_CACHE:
            rval = ConsumeSSHostCache(msg, msglen);
            break;
        default:
            break;
    }

    appId_ss_stats.messages_received++;
    return rval;
}

#ifdef REG_TEST
/*
 * File I/O
 */
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
            _dpd.errMsg("Error reading from AppId SS message file: %s (%d)\n", strerror(errno), errno);
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
            _dpd.errMsg("Error writing to AppId SS message file: %s (%d)\n", strerror(errno), errno);
            break;
        }
    }

    return -1;
}

static int ReadAppIdSSMessagesFromFile(const char *filename)
{
    MsgHeader *msg_header;
    uint8_t *msg;
    int rval = 0, fd, offset = 0;

    fd = open(filename, O_RDONLY , 0664);
    if (fd < 0)
    {
        if (errno == ENOENT)
            return 0;

        DynamicPreprocessorFatalMessage("Could not open %s for reading AppId SS messages from: %s (%d)\n", filename, strerror(errno), errno);
    }

    _dpd.logMsg("Reading AppId SS messages from '%s'...\n", filename);
    msg = file_io_buffer;

    while ((rval = Read(fd, msg, sizeof(*msg_header))) == 0)
    {
        msg_header = (MsgHeader *) msg;
        offset = sizeof(*msg_header);

        if ((rval = Read(fd, msg + offset, msg_header->data_length)) != 0)
        {
            _dpd.errMsg("Error reading the remaining %zu bytes of AppId SS message from file: %s (%d)\n",
                msg_header->data_length, strerror(errno), errno);

            close(fd);
            return rval;
        }

        if ((rval = ConsumeAppIdSSMsg(msg_header->type, msg + offset, msg_header->data_length)) != 0)
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

static uint32_t WriteAppIdSSMsgHeader(uint8_t *msg, uint32_t type, uint32_t data_len)
{
    MsgHeader *msg_hdr;

    msg_hdr = (MsgHeader *) msg;
    msg_hdr->type = type;
    msg_hdr->data_length = data_len;

    return 0;
}
#endif

static int AppIdSSSCMsgHandler(SCMsgHdr *hdr, const uint8_t *msg, uint32_t msglen)
{
    int rval = 1;

    if(!hdr)
        return rval;

    rval = ConsumeAppIdSSMsg(hdr->type, msg, msglen);

    return rval;
}

/* Caller should proceed writing to the 'data_ptr' ONLY upon successful return i.e 0 */
int CreateAppIdSSUpdate(void **msg_handle, void **hdr_ptr, void **data_ptr, uint32_t type, uint32_t data_len)
{
#ifdef REG_TEST
    if (!data_ptr)
        return -1;

    if (runtime_output_fd >= 0)
    {
        WriteAppIdSSMsgHeader(file_io_buffer, type, data_len);
        *data_ptr = (void *)(&file_io_buffer[0] + sizeof(MsgHeader));
    }
#else
    if (!msg_handle || !hdr_ptr || !data_ptr)
        return -1;

    SCMsgHdr *schdr;
    uint8_t *msg;
    int rval;

    if (appidStaticConfig && appidStaticConfig->send_state_sharing_updates &&
        appidStaticConfig->appId_ss_config && appidStaticConfig->appId_ss_config->use_side_channel)
    {
        /* Allocate space for the message. */
        if ((rval = _dpd.scAllocMessageTX(data_len, &schdr, &msg, msg_handle)) != 0)
        {
            _dpd.errMsg("Unable to allocate memory for enqueing AppId SS message\n");
            return -1;
        }

        *hdr_ptr = (void *)schdr;
        *data_ptr = (void *)msg;
    }
#endif
    else
    {
        return -1;
    }
    return 0;
}

int SendAppIdSSUpdate(void *msg_handle, void *hdr_ptr, void *data_ptr, uint32_t type, uint32_t data_len)
{
#ifdef REG_TEST
    if (runtime_output_fd >= 0)
    {
        if (Write(runtime_output_fd, file_io_buffer, sizeof(MsgHeader) + data_len) == -1)
        {
            /* Already reported Error inside the 'Write' call */
        }
        else
        {
            appId_ss_stats.messages_sent++;
        }
    }
#else
   if (!msg_handle || !hdr_ptr || !data_ptr)
        return -1;

    SCMsgHdr *schdr;
    uint8_t *msg;

    if (appidStaticConfig && appidStaticConfig->appId_ss_config &&
            appidStaticConfig->appId_ss_config->use_side_channel)
    {
        schdr = (SCMsgHdr *)hdr_ptr;
        msg = (uint8_t *)data_ptr;

        schdr->type = type;
        schdr->timestamp = _dpd.pktTime();

        _dpd.scEnqueueMessageTX(schdr, msg, data_len, msg_handle, NULL);
        appId_ss_stats.messages_sent++;
    }
#endif
    return 0;
}

void AppIdSSPostConfigInit(struct _SnortConfig *sc, int unused, void *arg)
{
    int rval, i;

#ifdef REG_TEST
    /* Probably need to do this through a different function for Reload. But we're okay for now. */
    if (appidStaticConfig && appidStaticConfig->appId_ss_config)
    {
        if (appidStaticConfig->appId_ss_config->startup_input_file)
        {
            if ((rval = ReadAppIdSSMessagesFromFile(appidStaticConfig->appId_ss_config->startup_input_file)) != 0)
            {
                _dpd.errMsg("Errors were encountered while reading AppId SS messages from file '%s'\n",
                        appidStaticConfig->appId_ss_config->startup_input_file);
            }
        }

        if (appidStaticConfig->appId_ss_config->runtime_output_file)
        {
            runtime_output_fd = open(appidStaticConfig->appId_ss_config->runtime_output_file, O_WRONLY | O_CREAT | O_TRUNC, 0664);
            if (runtime_output_fd < 0)
            {
                DynamicPreprocessorFatalMessage("Could not open %s for writing AppId SS messages: %s (%d)\n",
                        appidStaticConfig->appId_ss_config->runtime_output_file, strerror(errno), errno);
            }
        }
    }
#endif

    if (!appidStaticConfig->appId_ss_config->use_side_channel)
    {
        _dpd.logMsg("Could not register AppId SS message handlers. Need Side channel configuration to do so.\n");
        return;
    }

    for (i = SC_MSG_TYPE_APPID_SS_MIN + 1; i < SC_MSG_TYPE_APPID_SS_MAX; ++i)
    {
        if ((rval = _dpd.scRegisterRXHandler(i, AppIdSSSCMsgHandler, NULL)) != 0)
        {
            DynamicPreprocessorFatalMessage("Unable to register AppId SS message handler\n");
        }
    }
}

void AppIdCleanSS(void)
{
    if (runtime_output_fd >= 0)
    {
        close(runtime_output_fd);
        runtime_output_fd = -1;
    }
}
#endif /* SIDE_CHANNEL */
