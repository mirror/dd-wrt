/* cat.c -- simple version of cat, a "fixed" version of...
   http://people.redhat.com/johnsonm/lad/src/cat.c.html */

#define _LARGEFILE64_SOURCE 1 /* open64 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>

static void io_fd_unset_o_nonblock(int fd)
{
  int flags = 0;

  /* see if the NONBLOCK flag is set... */
  if ((flags = fcntl(fd, F_GETFL)) == -1)
      err(EXIT_FAILURE, "fcntl(F_GETFL)");

  /* if it is remove it, or err() out */
  if (flags & O_NONBLOCK)
    if (fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) == -1)
      err(EXIT_FAILURE, "fcntl(F_SETFL, ~O_NONBLOCK)");
}

static void full_write(int fd, const char *buf, size_t len)
{
  while (len > 0)
  { /* loop until all of the write request is done */
    ssize_t ret = write(fd, buf, len);
    
    if ((ret == -1) && (errno != EINTR))
      err(EXIT_FAILURE, "write");
    if (ret == -1) continue;

    buf += (size_t)ret;
    len -= (size_t)ret;
  }   
}

static void ex_cat_read_fd_write_stdout(int fd)
{
  char buf[BUFSIZ];
  ssize_t ret = 0;

  while ((ret = read(fd, buf, sizeof(buf))) != 0)
  { /* read a bit, then write that bit */
    if ((ret == -1) && (errno != EINTR))
      err(EXIT_FAILURE, "read");
      
    full_write(STDOUT_FILENO, buf, ret);
  }
}

int main(int argc, char *argv[])
{
  int count = 1;
  
  io_fd_unset_o_nonblock(STDOUT_FILENO);
  io_fd_unset_o_nonblock(STDERR_FILENO);

  if (count >= argc)
  {
    io_fd_unset_o_nonblock(STDIN_FILENO);
    ex_cat_read_fd_write_stdout(STDIN_FILENO);
  }

  while (count < argc)
  {
    int fd = open64(argv[count], O_RDONLY);

    if (fd == -1)
      err(EXIT_FAILURE, "open(%s)", argv[count]);

    /* assumes open() produces a blocking fd */
    ex_cat_read_fd_write_stdout(fd);

    if (close(fd) == -1)
      warn("close(%s)", argv[count]);

    ++count;
  }
  
  exit (EXIT_SUCCESS);
}
