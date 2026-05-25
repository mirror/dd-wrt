/* ISC license. */

#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include <skalibs/sgetopt.h>
#include <skalibs/types.h>
#include <skalibs/strerr.h>
#include <skalibs/direntry.h>
#include <skalibs/djbunix.h>

#include "mdevd-internal.h"

#define USAGE "mdevd-coldplug [ -v verbosity ] [ -s slashsys ] [ -O nlgroup ] [ -b kbufsz ]"
#define dieusage() strerr_dieusage(100, USAGE)

static char subsystem[PATH_MAX] = "" ;
static char mdev[PATH_MAX] = "" ;

static int scan_subdir (int fdat, char const *pathat, char const *list)
{
  int r = 0 ;
  DIR *dir ;
  direntry *d = 0 ;
  direntry *lastd ;
  int fdlist = openat(fdat, list, O_RDONLY | O_DIRECTORY) ;
  if (fdlist < 0) strerr_diefu4sys(111, "open ", pathat, "/", list) ;
  dir = fdopendir(fdlist) ;
  if (!dir) strerr_diefu4sys(111, "fdopendir ", pathat, "/", list) ;
  for (;;)
  {
    lastd = d ;
    errno = 0 ;
    d = readdir(dir) ;
    if (!d) break ;
    if (d->d_name[0] == '.') continue ;
    {
      int fd ;
      size_t dlen = strlen(d->d_name) ;
      char fn[dlen + 8] ;
      memcpy(fn, d->d_name, dlen) ;
      memcpy(fn + dlen, "/uevent", 8) ;
      fd = openat(fdlist, fn, O_WRONLY) ;
      if (fd < 0) continue ;
      if (write(fd, "add\n", 4) < 4)
        strerr_warnwu6sys("write to ", pathat, "/", list, "/", fn) ;
      close(fd) ;
      r = 1 ;
    }
  }
  if (errno) strerr_diefu4sys(111, "readdir ", pathat, "/", list) ;
  if (r) strncpy(mdev, lastd->d_name, PATH_MAX) ;
  dir_close(dir) ;
  return r ;
}

static int scan_dir (char const *path, int add_devices)
{
  DIR *dir ;
  int fdpath = open2(path, O_RDONLY | O_DIRECTORY) ;
  if (fdpath < 0) return 0 ;
  dir = fdopendir(fdpath) ;
  if (!dir) strerr_diefu2sys(111, "fdopendir ", path) ;
  for (;;)
  {
    direntry *d ;
    errno = 0 ;
    d = readdir(dir) ;
    if (!d) break ;
    if (d->d_name[0] == '.') continue ;
    if (add_devices)
    {
      size_t dlen = strlen(d->d_name) ;
      char fn[dlen + 9] ;
      memcpy(fn, d->d_name, dlen) ;
      memcpy(fn + dlen, "/devices", 9) ;
      if (scan_subdir(fdpath, path, fn))
        strncpy(subsystem, d->d_name, PATH_MAX) ;
    }
    else if (scan_subdir(fdpath, path, d->d_name))
      strncpy(subsystem, d->d_name, PATH_MAX) ;
  }
  if (errno) strerr_diefu2sys(111, "readdir ", path) ;
  dir_close(dir) ;
  return 1 ;
}


int main (int argc, char const *const *argv, char const *const *envp)
{
  char const *slashsys = "/sys" ;
  unsigned int verbosity = 1 ;
  unsigned int nlgroup = 0 ;
  unsigned int kbufsz = 1048576 ;
  int nlfd = -1 ;
  PROG = "mdevd-coldplug" ;
  {
    subgetopt l = SUBGETOPT_ZERO ;
    for (;;)
    {
      int opt = subgetopt_r(argc, argv, "v:s:O:b:", &l) ;
      if (opt == -1) break ;
      switch (opt)
      {
        case 'v' : if (!uint0_scan(l.arg, &verbosity)) dieusage() ; break ;
        case 's' : slashsys = l.arg ; break ;
        case 'O' : if (!uint0_scan(l.arg, &nlgroup)) dieusage() ; break ;
        case 'b' : if (!uint0_scan(l.arg, &kbufsz)) dieusage() ; break ;
        default : dieusage() ;
      }
    }
    argc -= l.ind ; argv += l.ind ;
  }

  nlgroup &= ~1 ;
  if (nlgroup)
  {
    nlfd = mdevd_netlink_init(nlgroup, kbufsz) ;
    if (nlfd == -1)
      strerr_diefu1sys(111, "init netlink") ;
    if (ndelay_off(nlfd) == -1)
      strerr_diefu1sys(111, "make netlink socket non-blocking") ;
  }

 /* Trigger the uevents */
  {
    size_t slashsyslen = strlen(slashsys) ;
    char fn[slashsyslen + 11] ;
    memcpy(fn, slashsys, slashsyslen) ;
    memcpy(fn + slashsyslen, "/subsystem", 11) ;
    if (!scan_dir(fn, 1))
    {
      memcpy(fn + slashsyslen + 1, "class", 6) ;
      if (!scan_dir(fn, 0)) strerr_diefu2sys(111, "open ", fn) ;
      memcpy(fn + slashsyslen + 1, "bus", 4) ;
      if (!scan_dir(fn, 1)) strerr_diefu2sys(111, "open ", fn) ;
    }
  }

 /* Wait for the last triggered uevent to be processed */
  if (nlgroup && subsystem[0] && mdev[0])
  {
    struct uevent_s event ;
    for (;;) if (mdevd_uevent_read(nlfd, &event, 0, verbosity))
    {
      char const *x = mdevd_uevent_getvar(&event, "ACTION") ;
      if (strcmp(x, "add")) continue ;
      x = mdevd_uevent_getvar(&event, "SUBSYSTEM") ;
      if (strcmp(x, subsystem)) continue ;
      x = strrchr(mdevd_uevent_getvar(&event, "DEVPATH"), '/') + 1 ;
      if (!strcmp(x, mdev)) break ;
    }
  }

  return 0 ;
}
