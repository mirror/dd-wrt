/* SPDX-License-Identifier: GPL-2.0-or-later */
/*****************************************************************************
  Copyright (c) 2006 EMC Corporation.
  Copyright (c) 2011 Factor-SPE

  Authors: Srinivas Aji <Aji_Srinivas@emc.com>
  Authors: Vitalii Demianets <dvitasgs@gmail.com>

******************************************************************************/

#include <config.h>

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "log.h"
#include "epoll_loop.h"
#include "bridge_ctl.h"
#include "clock_gettime.h"

/* globals */
static int epoll_fd = -1;
static struct timespec nexttimeout;

int init_epoll(void)
{
    int r = epoll_create(128);
    if(r < 0)
    {
        ERROR("epoll_create failed: %m");
        return -1;
    }
    epoll_fd = r;
    return 0;
}

int add_epoll(struct epoll_event_handler *h)
{
    struct epoll_event ev =
    {
        .events = EPOLLIN,
        .data.ptr = h,
    };
    h->ref_ev = NULL;
    int r = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, h->fd, &ev);
    if(r < 0)
    {
        ERROR("epoll_ctl_add: %m");
        return -1;
    }
    return 0;
}

int remove_epoll(struct epoll_event_handler *h)
{
    int r = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, h->fd, NULL);
    if(r < 0)
    {
        ERROR("epoll_ctl_del: %m");
        return -1;
    }
    if(h->ref_ev && h->ref_ev->data.ptr == h)
    {
        h->ref_ev->data.ptr = NULL;
        h->ref_ev = NULL;
    }
    return 0;
}

void clear_epoll(void)
{
    if(epoll_fd >= 0)
        close(epoll_fd);
}

static inline int time_diff(struct timespec *second, struct timespec *first)
{
    return (second->tv_sec - first->tv_sec) * 1000
            + (second->tv_nsec - first->tv_nsec) / 1000000;
}

static inline void run_timeouts(void)
{
    bridge_one_second();
    ++(nexttimeout.tv_sec);
}

int epoll_main_loop(volatile bool *quit)
{
    clock_gettime(CLOCK_MONOTONIC, &nexttimeout);
    ++(nexttimeout.tv_sec);
#define EV_SIZE 8
    struct epoll_event ev[EV_SIZE];

    while(!*quit)
    {
        int r, i;
        int timeout;

        struct timespec tv;
        clock_gettime(CLOCK_MONOTONIC, &tv);
        timeout = time_diff(&nexttimeout, &tv);
        if(timeout < 0 || timeout > 1000)
        {
            run_timeouts();
            /*
             * Check if system time has changed.
             */
            if(timeout < -4000 || timeout > 1000)
            {
                /* Most probably, system time has changed */
                nexttimeout.tv_nsec = tv.tv_nsec;
                nexttimeout.tv_sec = tv.tv_sec + 1;
            }
            timeout = 0;
        }

        r = epoll_wait(epoll_fd, ev, EV_SIZE, timeout);
        if(r < 0 && errno != EINTR)
        {
            ERROR("epoll_wait: %m");
            return -1;
        }
        for(i = 0; i < r; ++i)
        {
            struct epoll_event_handler *p = ev[i].data.ptr;
            if(p != NULL)
                p->ref_ev = &ev[i];
        }
        for (i = 0; i < r; ++i)
        {
            struct epoll_event_handler *p = ev[i].data.ptr;
            if(p && p->handler)
                p->handler(ev[i].events, p);
        }
        for (i = 0; i < r; ++i)
        {
            struct epoll_event_handler *p = ev[i].data.ptr;
            if(p != NULL)
                p->ref_ev = NULL;
        }
    }

    return 0;
}
