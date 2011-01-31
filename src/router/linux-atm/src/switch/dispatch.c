/* dispatch.c - Event dispatcher */

/* Written 1998 by Werner Almesberger, EPFL ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>

#include "atmd.h"
#include "dispatch.h"


typedef struct _fd_entry {
    int fd;
    void (*callback)(int fd,void *user);
    void *user;
    struct _fd_entry *next;
} FD_ENTRY;

static FD_ENTRY *fd_idle = NULL,*fd_active = NULL;
static int fds = 0;
static fd_set r_set;


void dsp_fd_add(int fd,void (*callback)(int fd,void *user),void *user)
{
    FD_ENTRY *entry;

    for (entry = fd_idle; entry; entry = entry->next)
	if (entry->fd == fd) break;
    if (!entry)
	for (entry = fd_active; entry; entry = entry->next)
	    if (entry->fd == fd) break;
    if (entry) {
	fprintf(stderr,"dsp_fd_add: duplicate fd %d\n",fd);
	exit(1);
    }
    if (fd >= fds) fds = fd+1;
    FD_SET(fd,&r_set);
    entry = alloc_t(FD_ENTRY);
    entry->fd = fd;
    entry->callback = callback;
    entry->user = user;
    entry->next = fd_idle;
    fd_idle = entry;
}


void dsp_fd_remove(int fd)
{
    FD_ENTRY **walk,*next;

    FD_CLR(fd,&r_set);
    for (walk = &fd_idle; *walk; walk = &(*walk)->next) {
	if ((*walk)->fd != fd) continue;
	next = (*walk)->next;
	free(*walk);
	*walk = next;
	return;
    }
    for (walk = &fd_active; *walk; walk = &(*walk)->next) {
	if ((*walk)->fd != fd) continue;
	next = (*walk)->next;
	free(*walk);
	*walk = next;
	return;
    }
    fprintf(stderr,"dsp_fd_remove: fd %d not found\n",fd);
    exit(1);
}


void dsp_init(void)
{
    FD_ZERO(&r_set);
}


void dsp_poll(void)
{
    FD_ENTRY **walk,*next,*entry;
    fd_set r_poll;
    int num;

    while (!fd_active) {
	r_poll = r_set;
	num = select(fds+1,&r_poll,NULL,NULL,NULL);
	if (num < 0) {
	    if (errno != EINTR) perror("select");
	    continue;
	}
	for (walk = &fd_idle; num;) {
	    next = (*walk)->next;
	    if (!FD_ISSET((*walk)->fd,&r_poll)) {
		walk = &(*walk)->next;
		continue;
	    }
	    (*walk)->next = fd_active;
	    fd_active = *walk;
	    *walk = next;
	    num--;
	}
    }
    entry = fd_active;
    fd_active = entry->next;
    entry->next = fd_idle;
    fd_idle = entry;
    entry->callback(entry->fd,entry->user);
}
