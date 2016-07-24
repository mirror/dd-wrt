/*
 * Copyright (c) 2004-2006 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2006-2014 Sippy Software, Inc., http://www.sippysoft.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "rtpp_types.h"
#include "rtpp_defines.h"
#include "rtpp_list.h"
#include "rtpp_log.h"
#include "rtpp_cfg_stable.h"
#include "rtpp_controlfd.h"
#include "rtpp_network.h"

#include "config_pp.h"

#if !defined(NO_ERR_H)
#include <err.h>
#else
#include "rtpp_util.h"
#endif

#ifdef HAVE_SYSTEMD_DAEMON
#include <systemd/sd-daemon.h>
#endif

static int
controlfd_init_systemd(void)
{
#ifdef HAVE_SYSTEMD_DAEMON
    int nfd, controlfd;

    nfd = sd_listen_fds(0);
    if (nfd > 1) {
        errx(1, "Too many file descriptors received.");
    }
    if (nfd == 1) {
        return (SD_LISTEN_FDS_START + 0);
    }
#else
    errx(1, "systemd is not supported or not detected on your system, "
      "please consider filing report or submitting a patch");
#endif
    return (-1);
}

static int
controlfd_init_ifsun(struct cfg *cf, const char *cmd_sock)
{
    int controlfd;
    struct sockaddr_un ifsun;

    unlink(cmd_sock);
    memset(&ifsun, '\0', sizeof ifsun);
#if defined(HAVE_SOCKADDR_SUN_LEN)
    ifsun.sun_len = strlen(cmd_sock);
#endif
    ifsun.sun_family = AF_LOCAL;
    strcpy(ifsun.sun_path, cmd_sock);
    controlfd = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (controlfd == -1)
        err(1, "can't create socket");
    setsockopt(controlfd, SOL_SOCKET, SO_REUSEADDR, &controlfd,
      sizeof controlfd);
    if (bind(controlfd, sstosa(&ifsun), sizeof ifsun) < 0)
        err(1, "can't bind to a socket: %s", cmd_sock);
    if ((cf->stable->run_uname != NULL || cf->stable->run_gname != NULL) &&
      chown(cmd_sock, cf->stable->run_uid, cf->stable->run_gid) == -1)
        err(1, "can't set owner of the socket: %s", cmd_sock);
    if ((cf->stable->run_gname != NULL) && cf->stable->sock_mode != 0 &&
      (chmod(cmd_sock, cf->stable->sock_mode) == -1))
        err(1, "can't allow rw acces to group");
    if (listen(controlfd, 32) != 0)
        err(1, "can't listen on a socket: %s", cmd_sock);

    return (controlfd);
}

static int
controlfd_init_udp(struct cfg *cf, struct rtpp_ctrl_sock *csp)
{
    struct sockaddr_storage ifsin;
    char *cp;
    int controlfd, so_rcvbuf, i;

    cp = strrchr(csp->cmd_sock, ':');
    if (cp != NULL) {
        *cp = '\0';
        cp++;
    }
    if (cp == NULL || *cp == '\0')
        cp = CPORT;
    csp->port_ctl = atoi(cp);
    i = (csp->type == RTPC_UDP6) ? AF_INET6 : AF_INET;
    if (setbindhost(sstosa(&ifsin), i, csp->cmd_sock, cp) != 0)
        exit(1);
    controlfd = socket(i, SOCK_DGRAM, 0);
    if (controlfd == -1)
        err(1, "can't create socket");
    so_rcvbuf = 16 * 1024;
    if (setsockopt(controlfd, SOL_SOCKET, SO_RCVBUF, &so_rcvbuf, sizeof(so_rcvbuf)) == -1)
        rtpp_log_ewrite(RTPP_LOG_ERR, cf->stable->glog, "unable to set 16K receive buffer size on controlfd");
    if (bind(controlfd, sstosa(&ifsin), SS_LEN(&ifsin)) < 0)
        err(1, "can't bind to a socket");

    return (controlfd);
}

int
rtpp_controlfd_init(struct cfg *cf)
{
    int controlfd_in, controlfd_out, flags;
    struct rtpp_ctrl_sock *ctrl_sock;

    for (ctrl_sock = RTPP_LIST_HEAD(cf->stable->ctrl_socks);
      ctrl_sock != NULL; ctrl_sock = RTPP_ITER_NEXT(ctrl_sock)) {
        switch (ctrl_sock->type) {
        case RTPC_SYSD:
            controlfd_in = controlfd_out = controlfd_init_systemd();
            break;

        case RTPC_IFSUN:
        case RTPC_IFSUN_C:
            controlfd_in = controlfd_out = controlfd_init_ifsun(cf, ctrl_sock->cmd_sock);
            break;

        case RTPC_UDP4:
        case RTPC_UDP6:
            controlfd_in = controlfd_out = controlfd_init_udp(cf, ctrl_sock);
            break;

        case RTPC_STDIO:
            controlfd_in = fileno(stdin);
            controlfd_out = fileno(stdout);
            break;
        }
        if (controlfd_in < 0 || controlfd_out < 0) {
            return (-1);
        }
        flags = fcntl(controlfd_in, F_GETFL);
        fcntl(controlfd_in, F_SETFL, flags | O_NONBLOCK);
        ctrl_sock->controlfd_in = controlfd_in;
        ctrl_sock->controlfd_out = controlfd_out;
    }

    return (0);
}

void
rtpp_controlfd_cleanup(struct cfg *cf)
{
    struct rtpp_ctrl_sock *ctrl_sock;

    for (ctrl_sock = RTPP_LIST_HEAD(cf->stable->ctrl_socks);
      ctrl_sock != NULL; ctrl_sock = RTPP_ITER_NEXT(ctrl_sock)) {
        if (RTPP_CTRL_ISUNIX(ctrl_sock) == 0)
            continue;
        unlink(ctrl_sock->cmd_sock);
    }
}

struct rtpp_ctrl_sock *
rtpp_ctrl_sock_parse(const char *optarg)
{
    struct rtpp_ctrl_sock *rcsp;

    rcsp = malloc(sizeof(struct rtpp_ctrl_sock));
    if (rcsp == NULL) {
        return (NULL);
    }
    memset(rcsp, '\0', sizeof(struct rtpp_ctrl_sock));
    rcsp->type= RTPC_IFSUN;
    if (strncmp("udp:", optarg, 4) == 0) {
        rcsp->type= RTPC_UDP4;
        optarg += 4;
    } else if (strncmp("udp6:", optarg, 5) == 0) {
        rcsp->type= RTPC_UDP6;
        optarg += 5;
    } else if (strncmp("unix:", optarg, 5) == 0) {
        rcsp->type= RTPC_IFSUN;
        optarg += 5;
    } else if (strncmp("cunix:", optarg, 6) == 0) {
        rcsp->type= RTPC_IFSUN_C;
        optarg += 6;
    }  else if (strncmp("systemd:", optarg, 8) == 0) {
        rcsp->type= RTPC_SYSD;
        optarg += 8;
    } else if (strncmp("stdio:", optarg, 6) == 0) {
        rcsp->type= RTPC_STDIO;
        optarg += 6;
    }
    rcsp->cmd_sock = optarg;

    return (rcsp);
}

const char *
rtpp_ctrl_sock_describe(struct rtpp_ctrl_sock *rcsp)
{

    switch (rcsp->type) {
    case RTPC_IFSUN:
        return "unix";

    case RTPC_UDP4:
        return "udp";

    case RTPC_UDP6:
        return "udp6";

    case RTPC_IFSUN_C:
        return "cunix";

    case RTPC_SYSD:
        return "systemd";

    case RTPC_STDIO:
        return "stdio";

    default:
        abort();
    }
}
