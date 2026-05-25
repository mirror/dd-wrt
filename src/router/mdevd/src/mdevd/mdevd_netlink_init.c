/* ISC license. */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include <skalibs/socket.h>
#include <skalibs/djbunix.h>

int mdevd_netlink_init (unsigned int group, unsigned int kbufsz)
{
  struct sockaddr_nl nl = { .nl_family = AF_NETLINK, .nl_pad = 0, .nl_groups = group, .nl_pid = 0 } ;
  int fd = socket_internal(AF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT, O_NONBLOCK|O_CLOEXEC) ;
  if (fd < 0) return -1 ;
  if (bind(fd, (struct sockaddr *)&nl, sizeof(struct sockaddr_nl)) < 0) goto err ;
  if (setsockopt(fd, SOL_SOCKET, SO_RCVBUFFORCE, &kbufsz, sizeof(unsigned int)) < 0)
  {
    if (errno != EPERM
     || setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &kbufsz, sizeof(unsigned int)) < 0) goto err ;
  }
  return fd ;

 err:
  fd_close(fd) ;
  return -1 ;
}
