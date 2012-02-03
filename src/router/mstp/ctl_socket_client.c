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
#include <sys/poll.h>

#include "ctl_functions.h"
#define NO_DAEMON
#include "log.h"

static int fd = -1;

int ctl_client_init(void)
{
    struct sockaddr_un sa_svr;
    int s;
    TST(strlen(MSTP_SERVER_SOCK_NAME) < sizeof(sa_svr.sun_path), -1);

    if(0 > (s = socket(PF_UNIX, SOCK_DGRAM, 0)))
    {
        ERROR("Couldn't open unix socket: %m");
        return -1;
    }

    set_socket_address(&sa_svr, MSTP_SERVER_SOCK_NAME);

    struct sockaddr_un sa;
    char tmpname[64];
    sprintf(tmpname, "MSTPCTL_%d", getpid());
    set_socket_address(&sa, tmpname);
    /* We need this bind. The autobind on connect isn't working properly.
     * The server doesn't get a proper sockaddr in recvmsg if we don't do this.
     */
    if(0 != bind(s, (struct sockaddr *)&sa, sizeof(sa)))
    {
        ERROR("Couldn't bind socket: %m");
        close(s);
        return -1;
    }

    if(0 != connect(s, (struct sockaddr *)&sa_svr, sizeof(sa_svr)))
    {
        ERROR("Couldn't connect to server");
        close(s);
        return -1;
    }
    fd = s;

    return 0;
}

void ctl_client_cleanup(void)
{
    if(fd >= 0)
    {
        close(fd);
        fd = -1;
    }
}

int send_ctl_message(int cmd, void *inbuf, int lin, void *outbuf, int lout,
                     LogString *log, int *res)
{
    struct ctl_msg_hdr mhdr;
    struct msghdr msg;
    struct iovec iov[3];
    int l;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 3;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;

    mhdr.cmd = cmd;
    mhdr.lin = lin;
    mhdr.lout = lout;
    mhdr.llog = sizeof(log->buf) - 1;
    iov[0].iov_base = &mhdr;
    iov[0].iov_len = sizeof(mhdr);
    iov[1].iov_base = (void *)inbuf;
    iov[1].iov_len = lin;
    iov[2].iov_base = log->buf;
    iov[2].iov_len = 0;

    l = sendmsg(fd, &msg, 0);
    if(0 > l)
    {
        ERROR("Error sending message to server: %m");
        return -1;
    }
    if(l != sizeof(mhdr) + lin)
    {
        ERROR("Error sending message to server: Partial write");
        return -1;
    }

    iov[1].iov_base = outbuf;
    iov[1].iov_len = lout;
    iov[2].iov_base = log->buf;
    iov[2].iov_len = sizeof(log->buf);

    struct pollfd pfd;
    int timeout = 5000; /* 5 s */
    int r;

    pfd.fd = fd;
    pfd.events = POLLIN;
    do
    {
        if(0 == (r = poll(&pfd, 1, timeout)))
        {
            ERROR("Error getting message from server: Timeout");
            return -1;
        }
        if(0 > r)
        {
            ERROR("Error getting message from server: poll error: %m");
            return -1;
        }
    }while(0 == (pfd.revents & (POLLERR | POLLHUP | POLLNVAL | POLLIN)));

    l = recvmsg(fd, &msg, 0);
    if(0 > l)
    {
        ERROR("Error getting message from server: %m");
        return -1;
    }
    if((sizeof(mhdr) > l)
       || (l != sizeof(mhdr) + mhdr.lout + mhdr.llog)
       || (mhdr.cmd != cmd)
      )
    {
        ERROR("Error getting message from server: Bad format");
        return -1;
    }
    if(mhdr.lout != lout)
    {
        ERROR("Error, unexpected result length %d, expected %d\n",
              mhdr.lout, lout);
        return -1;
    }
    if(sizeof(log->buf) <= mhdr.llog)
    {
        ERROR("Invalid log message length %d", mhdr.llog);
        return -1;
    }
    if(res)
        *res = mhdr.res;
    log->buf[mhdr.llog] = 0;
    return 0;
}
