/*
 **  $Id$
 **
 **  snort_dump_packets.c
 **
 **  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 **  Copyright (C) 2002-2013 Sourcefire, Inc.
 **  Author(s):  Ron Dempster <rdempster@sourcefire.com>
 **
 **  NOTES
 **  3.4.14 - Initial Source Code. Dempster
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
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdint.h>
#include "sfcontrol.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

struct _CS_MESSAGE
{
    CSMessageHeader hdr;
    uint8_t msg[0];
} __attribute__((packed));

typedef struct _CS_MESSAGE CSMessage;

struct _CS_RESPONSE_MESSAGE
{
    CSMessageHeader hdr;
    CSMessageDataHeader msg_hdr;
    uint8_t msg[4096];
} __attribute__((packed));

typedef struct _CS_RESPONSE_MESSAGE CSResponseMessage;

static void DisplayUsage(const char *progname)
{
    fprintf(stderr, "Usage %s <snort log dir> [-a daq address space id (0-65535)] <snort log dir> [<file name> [<bpf>]]\n", progname);
}

static int SendMessage(int socket_fd, const CSMessage *msg, uint32_t len)
{
    ssize_t numsent;
    unsigned total_len = sizeof(*msg) + len;
    unsigned total = 0;

    do
    {
        numsent = write(socket_fd, ((unsigned char *)msg) + total, total_len - total);
        if (!numsent)
            return 0;
        else if (numsent > 0)
            total += numsent;
        else if (errno != EINTR && errno != EAGAIN)
            return -1;
    } while (total < total_len);
    return 1;
}

static int ReadData(int socket_fd, uint8_t *buffer, uint32_t length)
{
    ssize_t numread;
    unsigned total = 0;

    do
    {
        numread = read(socket_fd, buffer + total, length - total);
        if (!numread)
            return 0;
        else if (numread > 0)
            total += numread;
        else if (errno != EINTR && errno != EAGAIN)
            return -1;
    } while (total < length);
    if (total < length)
        return 0;
    return 1;
}

static int ReadResponse(int socket_fd, CSMessageHeader *hdr)
{
    ssize_t numread;
    unsigned total = 0;

    do
    {
        numread = read(socket_fd, ((unsigned char *)hdr) + total, sizeof(*hdr) - total);
        if (!numread)
            return 0;
        else if (numread > 0)
            total += numread;
        else if (errno != EINTR && errno != EAGAIN)
            return -1;
    } while (total < sizeof(*hdr));
    if (total < sizeof(*hdr))
        return 0;

    hdr->length = ntohl(hdr->length);
    hdr->version = ntohs(hdr->version);
    hdr->type = ntohs(hdr->type);
    return 1;
}

static void ConnectToUnixSocket(const char * const name, int * const psock)
{
    struct sockaddr_un sunaddr;
    int sock = -1;
    int rval;

    memset(&sunaddr, 0, sizeof(sunaddr));
    rval = snprintf(sunaddr.sun_path, sizeof(sunaddr.sun_path), "%s", name);
    if (rval < 0 || (size_t)rval >= sizeof(sunaddr.sun_path))
    {
        fprintf(stderr, "Socket name '%s' is too long\n", name);
        exit(-1);
    }

    sunaddr.sun_family = AF_UNIX;

    /* open the socket */
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Error opening socket: %s\n", strerror(errno));
        exit(-1);
    }

    if (connect(sock, (struct sockaddr *) &sunaddr, sizeof(sunaddr)) == -1)
    {
        fprintf(stderr, "Unable to connect to UNIX socket at %s: %s\n", name, strerror(errno));
        close(sock);
        exit(-1);
    }

    *psock = sock;
}

int main(int argc, char *argv[])
{
    int rval;
    char socket_fn[PATH_MAX];
    int socket_fd;
    int current_arg;
    uint16_t address_space_id = 0;
    unsigned address_space_id_len = 0;
    char *p;
    ssize_t len;
    const char *sep;
    const char* dump_file_name = NULL;
    unsigned dump_file_name_len = 0;
    const char* bpf = NULL;
    unsigned bpf_len = 0;
    CSMessage *message;
    uint32_t extra_len;
    CSResponseMessage response;

    if (argc < 2)
    {
        DisplayUsage(argv[0]);
        exit(-1);
    }

    current_arg = 1;
    if (strcmp(argv[current_arg], "-a") == 0)
    {
        unsigned long tmp;

        current_arg++;
        if (current_arg >= argc)
        {
            DisplayUsage(argv[0]);
            exit(-1);
        }
        tmp = strtoul(argv[current_arg], &p, 0);
        if (*p || tmp > UINT16_MAX)
        {
            DisplayUsage(argv[0]);
            exit(-1);
        }
        address_space_id = (uint16_t)tmp;
        current_arg++;
    }

    if (current_arg >= argc)
    {
        DisplayUsage(argv[0]);
        exit(-1);
    }
    len = strlen(argv[current_arg]);
    if (len && argv[current_arg][len - 1] == '/')
        sep = "";
    else
        sep = "/";
    snprintf(socket_fn, sizeof(socket_fn), "%s%s%s", argv[current_arg], sep, CONTROL_FILE);
    current_arg++;

    if (current_arg < argc)
    {
        address_space_id_len = sizeof(address_space_id);
        dump_file_name = argv[current_arg];
        dump_file_name_len = strlen(dump_file_name) + 1;
        current_arg++;
        if (current_arg < argc)
        {
            bpf = argv[current_arg];
            bpf_len = strlen(bpf) + 1;
        }
        else
            bpf_len = 1;
    }

    extra_len = address_space_id_len + dump_file_name_len + bpf_len;
    ConnectToUnixSocket(socket_fn, &socket_fd);
    message = malloc(sizeof *message + extra_len);
    if (message == NULL)
    {
        fprintf(stderr, "snort_control: could not allocate message.\n");
        exit(-1);
    }

    message->hdr.version = htons(CS_HEADER_VERSION);
    message->hdr.type = htons((uint16_t)CS_TYPE_DUMP_PACKETS);
    message->hdr.length = 0;

    if (address_space_id_len)
    {
        uint8_t* msg = message->msg;
        message->hdr.length = htonl(extra_len);

        *((uint16_t*)msg) = address_space_id;
        msg += sizeof(address_space_id);
        snprintf((char*)msg, dump_file_name_len, "%s", dump_file_name);
        msg[dump_file_name_len - 1] = 0;
        msg += dump_file_name_len;
        if (bpf_len > 1)
        {
            snprintf((char*)msg, bpf_len, "%s", bpf);
            msg[bpf_len - 1] = 0;
        }
        else
            *msg = 0;
    }

    if ((rval = SendMessage(socket_fd, message, extra_len)) < 0)
    {
        fprintf(stderr, "Failed to send the message: %s\n", strerror(errno));
        close(socket_fd);
        exit(-1);
    }
    else if (!rval)
    {
        fprintf(stderr, "Server closed the socket\n");
        close(socket_fd);
        exit(-1);
    }
    free(message);

    do
    {
        /* Reusing the same CSMessage to capture the response */
        if ((rval = ReadResponse(socket_fd, &response.hdr)) < 0)
        {
            fprintf(stderr, "Failed to read the response: %s\n", strerror(errno));
            close(socket_fd);
            exit(-1);
        }
        else if (!rval)
        {
            fprintf(stderr, "Server closed the socket before sending a response\n");
            close(socket_fd);
            exit(-1);
        }

        if (response.hdr.version != CS_HEADER_VERSION)
        {
            printf("snort_control: bad response version\n");
            close(socket_fd);
            exit(-1);
        }

        if (response.hdr.length)
        {

            if (response.hdr.length < sizeof(response.msg_hdr))
            {
                printf("snort_control: response message is too small\n");
                close(socket_fd);
                exit(-1);
            }

            if (response.hdr.length > sizeof(response.msg))
            {
                printf("snort_control: response message is too large\n");
                close(socket_fd);
                exit(-1);
            }

            if ((rval = ReadData(socket_fd, (uint8_t *)(&response)+sizeof(response.hdr), response.hdr.length)) < 0)
            {
                fprintf(stderr, "Failed to read the response data: %s\n", strerror(errno));
                close(socket_fd);
                exit(-1);
            }
            else if (!rval)
            {
                fprintf(stderr, "Server closed the socket before sending the response data\n");
                close(socket_fd);
                exit(-1);
            }

            response.msg_hdr.code = ntohl(response.msg_hdr.code);
            response.msg_hdr.length = ntohs(response.msg_hdr.length);

            if (response.msg_hdr.length == response.hdr.length - sizeof(response.msg_hdr))
            {
                response.msg[response.msg_hdr.length-1] = 0;
                fprintf(stdout, "Response %04X with code %d (%s)\n",
                    response.hdr.type, response.msg_hdr.code, response.msg);
            }
            else
                fprintf(stdout, "Response %04X with code %d\n", response.hdr.type, response.msg_hdr.code);
        }
        else
        {
            printf("Response %04X\n", response.hdr.type);
        }
    } while (response.hdr.type == CS_HEADER_DATA);
    return 0;
}

