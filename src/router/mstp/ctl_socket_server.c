/* SPDX-License-Identifier: GPL-2.0-or-later */
/*****************************************************************************
  Copyright (c) 2006 EMC Corporation.
  Copyright (c) 2011 Factor-SPE

  Authors: Srinivas Aji <Aji_Srinivas@emc.com>
  Authors: Vitalii Demianets <dvitasgs@gmail.com>

******************************************************************************/

#include <sys/un.h>
#include <unistd.h>

#include "ctl_socket_client.h"
#include "epoll_loop.h"
#include "log.h"

static int server_socket(void)
{
    struct sockaddr_un sa;
    int s, creds;

    TST(strlen(MSTP_SERVER_SOCK_NAME) < sizeof(sa.sun_path), -1);

    if(0 > (s = socket(PF_UNIX, SOCK_DGRAM, 0)))
    {
        ERROR("Couldn't open unix socket: %m");
        return -1;
    }

    set_socket_address(&sa, MSTP_SERVER_SOCK_NAME);

    creds = 1;
    if(0 != setsockopt(s, SOL_SOCKET, SO_PASSCRED, &creds, sizeof(creds)))
    {
        ERROR("Couldn't enable SO_PASSCRED: %m");
        close(s);
        return -1;
    }

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
            int brcount = br_array[0];
            /*
             * Accept more data for compatibility with old API
             * where interfaces belonging to the bridge were provided
             * by clients implementing their own mstpctl application.
             */
            if(lin < ((brcount + 1) * sizeof(int)))
            {
                LOG("Bad sizes: lin %d < %d", lin,
                    (brcount + 1) * sizeof(int));
                return -1;
            }
            int r = CTL_add_bridges(br_array);
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
static unsigned char msg_ctlbuf[CMSG_SPACE(sizeof(struct ucred))];

static bool ctl_access_ok(const struct ucred *creds, int cmd)
{
    switch(cmd)
    {
        case CMD_CODE_get_cist_bridge_status:
        case CMD_CODE_get_msti_bridge_status:
        case CMD_CODE_get_cist_port_status:
        case CMD_CODE_get_msti_port_status:
        case CMD_CODE_get_mstilist:
        case CMD_CODE_get_mstconfid:
        case CMD_CODE_get_vids2fids:
        case CMD_CODE_get_fids2mstids:
            return true;
        default:
            return creds->uid == 0;
    }
}

static void ctl_rcv_handler(uint32_t events, struct epoll_event_handler *p)
{
    struct ctl_msg_hdr mhdr;
    struct msghdr msg;
    struct sockaddr_un sa;
    struct iovec iov[3];
    struct cmsghdr *cmsg;
    struct ucred *creds;
    int l;

    msg.msg_name = &sa;
    msg.msg_namelen = sizeof(sa);
    msg.msg_iov = iov;
    msg.msg_iovlen = 3;
    msg.msg_control = msg_ctlbuf;
    msg.msg_controllen = sizeof(msg_ctlbuf);
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

    cmsg = CMSG_FIRSTHDR(&msg);

    if(!cmsg || (cmsg->cmsg_level != SOL_SOCKET)
       || (cmsg->cmsg_type != SCM_CREDENTIALS)
       || (cmsg->cmsg_len != CMSG_LEN(sizeof(struct ucred)))
      )
    {
        ERROR("CTL: No creds or unexpected control message. Ignoring");
        return;
    }

    creds = (struct ucred *)CMSG_DATA(cmsg);

    msg_log_offset = 0;
    ctl_in_handler = 1;
    if(ctl_access_ok(creds, mhdr.cmd)) {
        if(!(mhdr.cmd & RESPONSE_FIRST_HANDLE_LATER))
            mhdr.res = handle_message(mhdr.cmd, msg_inbuf, mhdr.lin,
                                      msg_outbuf, mhdr.lout);
        else
            mhdr.res = 0;

    } else {
        ERROR("Operation not permitted");
        mhdr.res = -1;
    }
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

    if(mhdr.res == 0 && mhdr.cmd & RESPONSE_FIRST_HANDLE_LATER)
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
