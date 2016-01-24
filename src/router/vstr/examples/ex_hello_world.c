/* hello world - Self contained, using a single piece of data at
 *               initialisation time (no data copying) */ 

#define VSTR_COMPILE_INCLUDE 1
#include <vstr.h>
#include <errno.h>  /* errno variable */
#include <err.h>    /* BSD/Linux header see: man errx */
#include <unistd.h> /* for STDOUT_FILENO */

int main(void)
{
  Vstr_base *s1 = NULL;

  if (!vstr_init()) /* initialize the library */
    err(EXIT_FAILURE, "init");

  /* create a string with data */
  if (!(s1 = vstr_dup_cstr_buf(NULL, "Hello World\n")))
    err(EXIT_FAILURE, "Create string");

  /* output the data to the user --
   *    assumes POSIX, assumes blocking IO but should work without */
  while (s1->len)
    if (!vstr_sc_write_fd(s1, 1, s1->len, STDOUT_FILENO, NULL))
    {
      if ((errno != EAGAIN) && (errno != EINTR))
        err(EXIT_FAILURE, "write");
    }

  /* cleanup allocated resources */
  vstr_free_base(s1);

  vstr_exit();

  exit (EXIT_SUCCESS);
}
