/* hello world - multiple sources of data (still no data copying) */

#include "ex_hello_world.h" /* helper functions */

int main(void)
{
  Vstr_base *s1 = hw_init();

  vstr_add_cstr_ptr(s1, s1->len, "Hello");
  vstr_add_cstr_ptr(s1, s1->len, " ");
  vstr_add_cstr_ptr(s1, s1->len, "World\n");

  /* we are checking whether any of the above three functions failed here */
  if (s1->conf->malloc_bad)
    errno = ENOMEM, err(EXIT_FAILURE, "Add string data");

  /* loop until all data is output... */
  while (io_put(s1, STDOUT_FILENO) != IO_NONE) {}

  exit (hw_exit(s1));
}
