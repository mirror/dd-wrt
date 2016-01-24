/* hello world - Using a single piece of data (no data copying) */

#include "ex_hello_world.h" /* helper functions */

int main(void)
{
  Vstr_base *s1 = hw_init();

  vstr_add_cstr_buf(s1, s1->len, "Hello World\n");

  if (s1->conf->malloc_bad)
    errno = ENOMEM, err(EXIT_FAILURE, "Add string data");

  while (io_put(s1, STDOUT_FILENO) != IO_NONE) {}

  exit (hw_exit(s1));
}
