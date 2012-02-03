/*****************************************************************************
  Copyright (c) 2006 EMC Corporation.
  Copyright (c) 2011 Factor-SPE

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.

  Authors: Srinivas Aji <Aji_Srinivas@emc.com>
  Authors: Vitalii Demianets <vitas@nppfactor.kiev.ua>

******************************************************************************/

#include <sys/un.h>
#include <unistd.h>

#include "ctl_socket_client.h"
#include "epoll_loop.h"
#include "log.h"

static int server_socket(void)
{
    struct sockaddr_un sa;
    int s;

    TST(strlen(MSTP_SERVER_SOCK_NAME) < sizeof(sa.sun_path), -1);

    if(0 > (s = socket(PF_UNIX, SOCK_DGRAM, 0)))
    {
        ERROR("Couldn't open unix socket: %m");
        return -1;
    }

    set_socket_address(&sa, MSTP_SERVER_SOCK_NAME);

    if(0 != bind(s, (struct sockaddr *)&sa, sizeof(sa)))
    {
        ERROR("Couldn't bind socket: %m");
        close(s);
        return -1;
    }

    return s;
}

static int handle_message(int cmd, void *inbuf, int lin,
                          void *outbuf, int lout)
{
    switch(cmd)
    {
        SERVER_MESSAGE_CASE(get_cist_bridge_status);
        SERVER_MESSAGE_CASE(get_msti_bridge_status);
        SERVER_MESSAGE_CASE(set_cist_bridge_config);
        SERVER_MESSAGE_CASE(set_msti_bridge_config);
        SERVER_MESSAGE_CASE(get_cist_port_status);
        SERVER_MESSAGE_CASE(get_msti_port_status);
        SERVER_MESSAGE_CASE(set_cist_port_config);
        SERVER_MESSAGE_CASE(set_msti_port_config);
        SERVER_MESSAGE_CASE(port_mcheck);
        SERVER_MESSAGE_CASE(set_debug_level);
        SERVER_MESSAGE_CASE(get_mstilist);
        SERVER_MESSAGE_CASE(create_msti);
        SERVER_MESSAGE_CASE(delete_msti);
        SERVER_MESSAGE_CASE(get_mstconfid);
        SERVER_MESSAGE_CASE(set_mstconfid);
        SERVER_MESSAGE_CASE(get_vids2fids);
        SERVER_MESSAGE_CASE(get_fids2mstids);
        SERVER_MESSAGE_CASE(set_vid2fid);
        SERVER_MESSAGE_CASE(set_fid2mstid);
        SERVER_MESSAGE_CASE(set_vids2fids);
        SERVER_MESSAGE_CASE(set_fids2mstids);

        case CMD_CODE_add_bridges:
        {
            if(0 != lout)
            {
                LOG("Bad sizes: lout %d != 0", lout);
                return -1;
            }
            if(sizeof(int) > lin)
            {
                LOG("Bad sizes: lin == 0");
                return -1;
            }
            int *br_array = inbuf;
            int i, serialized_data_count, chunk_count, brcount = br_array[0];
            int *ptr = br_array + (serialized_data_count = (brcount + 1));
            if(lin < ((serialized_data_count + 1) * sizeof(int)))
            {
bad_lin1:       LOG("Bad sizes: lin %d < %d", lin,
                    (serialized_data_count + 1) * sizeof(int));
                return -1;
            }
            for(i = 0; i < brcount; ++i)
            {
                serialized_data_count += (chunk_count = *ptr + 1);
                if(i < (brcount - 1))
                {
                    if(lin < ((serialized_data_count + 1) * sizeof(int)))
                        goto bad_lin1;
                    ptr += chunk_count;
                }
                else
                {
                    if(lin != (serialized_data_count * sizeof(int)))
                    {
                        LOG("Bad sizes: lin %d != %d", lin,
                            serialized_data_count * sizeof(int));
                        return -1;
                    }
                }
            }
            int* *ifaces_lists = malloc(brcount * sizeof(int*));
            if(NULL == ifaces_lists)
            {
                LOG("out of memory, brcount = %d\n", brcount);
                return -1;
            }
            ptr = br_array + (brcount + 1);
            for(i = 0; i < brcount; ++i)
            {
                ifaces_lists[i] = ptr;
                ptr += ifaces_lists[i][0] + 1;
            }
            int r = CTL_add_bridges(br_array, ifaces_lists);
            free(ifaces_lists);
            if(r)
                return r;
            return r;
        }

        case CMD_CODE_del_bridges:
        {
            if(0 != lout)
            {
                LOG("Bad sizes: lout %d != 0", lout);
                return -1;
            }
            if(sizeof(int) > lin)
            {
                LOG("Bad sizes: lin == 0");
                return -1;
            }
            int *br_array = inbuf;
            int brcount = br_array[0];
            if(((brcount + 1) * sizeof(int)) != lin)
            {
                LOG("Bad sizes: lin %d != %d", lin,
                    (brcount + 1) * sizeof(int));
                return -1;
            }
            int r = CTL_del_bridges(br_array);
            if(r)
                return r;
            return r;
        }

        default:
            ERROR("CTL: Unknown command %d", cmd);
            return -1;
    }
}

int ctl_in_handler = 0;
static unsigned char msg_logbuf[LOG_STRING_LEN];
static unsigned int msg_log_offset;
void _ctl_err_log(char *fmt, ...)
{
    if((sizeof(msg_logbuf) - 1) <= msg_log_offset)
        return;
    int r;
    va_list ap;
    va_start(ap, fmt);
    r = vsnprintf((char *)msg_logbuf + msg_log_offset,
                  sizeof(msg_logbuf) - msg_log_offset,
                  fmt, ap);
    va_end(ap);
    msg_log_offset += r;
    if(sizeof(msg_logbuf) <= msg_log_offset)
    {
        msg_log_offset = sizeof(msg_logbuf) - 1;
        msg_logbuf[sizeof(msg_logbuf) - 1] = 0;
    }
}

#define MSG_BUF_LEN 10000
static unsigned char msg_inbuf[MSG_BUF_LEN];
static unsigned char msg_outbuf[MSG_BUF_LEN];

static void ctl_rcv_handler(uint32_t events, struct epoll_event_handler *p)
{
    struct ctl_msg_hdr mhdr;
    struct msghdr msg;
    struct sockaddr_un sa;
    struct iovec iov[3];
    int l;

    msg.msg_name = &sa;
    msg.msg_namelen = sizeof(sa);
    msg.msg_iov = iov;
    msg.msg_iovlen = 3;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    iov[0].iov_base = &mhdr;
    iov[0].iov_len = sizeof(mhdr);
    iov[1].iov_base = msg_inbuf;
    iov[1].iov_len = MSG_BUF_LEN;
    iov[2].iov_base = NULL;
    iov[2].iov_len = 0;
    l = recvmsg(p->fd, &msg, MSG_NOSIGNAL | MSG_DONTWAIT);
    TST(l > 0,);
    if((0 != msg.msg_flags) || (sizeof(mhdr) > l)
       || (l != sizeof(mhdr) + mhdr.lin)
       || (MSG_BUF_LEN < mhdr.lout)
       || (0 > mhdr.cmd)
      )
    {
        ERROR("CTL: Unexpected message. Ignoring");
        return;
    }

    msg_log_offset = 0;
    ctl_in_handler = 1;

    if(!(mhdr.cmd & RESPONSE_FIRST_HANDLE_LATER))
        mhdr.res = handle_message(mhdr.cmd, msg_inbuf, mhdr.lin,
                                  msg_outbuf, mhdr.lout);
    else
        mhdr.res = 0;

    ctl_in_handler = 0;
    if(0 > mhdr.res)
        memset(msg_outbuf, 0, mhdr.lout);
    if(msg_log_offset < mhdr.llog)
        mhdr.llog = msg_log_offset;

    iov[1].iov_base = msg_outbuf;
    iov[1].iov_len = mhdr.lout;
    iov[2].iov_base = msg_logbuf;
    iov[2].iov_len = mhdr.llog;
    l = sendmsg(p->fd, &msg, MSG_NOSIGNAL);
    if(0 > l)
        ERROR("CTL: Couldn't send response: %m");
    else if(l != sizeof(mhdr) + mhdr.lout + mhdr.llog)
    {
        ERROR
            ("CTL: Couldn't send full response, sent %d bytes instead of %zd.",
             l, sizeof(mhdr) + mhdr.lout + mhdr.llog);
    }

    if(mhdr.cmd & RESPONSE_FIRST_HANDLE_LATER)
        handle_message(mhdr.cmd, msg_inbuf, mhdr.lin, msg_outbuf, mhdr.lout);
}

static struct epoll_event_handler ctl_handler = {0};

int ctl_socket_init(void)
{
    int s = server_socket();
    if(0 > s)
        return -1;

    ctl_handler.fd = s;
    ctl_handler.handler = ctl_rcv_handler;

    TST(add_epoll(&ctl_handler) == 0, -1);
    return 0;
}

void ctl_socket_cleanup(void)
{
    remove_epoll(&ctl_handler);
    close(ctl_handler.fd);
}
