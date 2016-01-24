/* *****************************************************************************
   Beg of hello_world.h header file which will be used in the following examples
   ****************************************************************************/
#ifndef HELLO_WORLD_H
#define HELLO_WORLD_H

/* ************************************************************************** */
/* headers: Vstr (and all supporting system headers), plus extra ones we need */
/* ************************************************************************** */

#define VSTR_COMPILE_INCLUDE 1 /* make Vstr include it's system headers */
#include <vstr.h>
#include <errno.h>
#include <err.h> /* BSD/Linux header see: man errx */
#include <unistd.h> /* for STDOUT_FILENO */

/* ********************************* */
/* generic POSIX IO helper functions */
/* ********************************* */

#define IO_OK    0
/* the missing values will be explained later in the tutorial... */
#define IO_NONE  3

static int io_put(Vstr_base *io_w, int fd)
{ /* assumes POSIX */
  if (!io_w->len)
    return (IO_NONE);
  
  if (!vstr_sc_write_fd(io_w, 1, io_w->len, fd, NULL))
  {
    if (errno != EINTR)
      err(EXIT_FAILURE, "write");
  }

  return (IO_OK);
}

/* ************************ */
/* generic helper functions */
/* ************************ */

/* hello world init function, init library and create a string */
static Vstr_base *hw_init(void)
{
  Vstr_base *s1 = NULL;

  if (!vstr_init())
    errno = ENOMEM, err(EXIT_FAILURE, "init");

  if (!(s1 = vstr_make_base(NULL))) /* create an empty string */
    errno = ENOMEM, err(EXIT_FAILURE, "Create string");

  return (s1);
}

/* hello world exit function, cleanup what was allocated in hw_init() */
static int hw_exit(Vstr_base *s1)
{
  vstr_free_base(s1);

  vstr_exit();

  return (EXIT_SUCCESS);
}

#endif
/* *****************************************************************************
   End of hello_world.h header file which will be used in the following examples
   ****************************************************************************/
