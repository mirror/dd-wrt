/* hello world - multiple pieces of data, includes substitution and
 *               inserting data into the middle of a string */

#include "ex_hello_world.h" /* helper functions */

int main(void)
{
  Vstr_base *s1 = hw_init();
  Vstr_ref *ref = NULL;

  vstr_add_cstr_ptr(s1, s1->len, "Hello");

  vstr_add_rep_chr(s1, s1->len, 'W', 5); /* add "WWWWWW" */

  if (s1->conf->malloc_bad)
    errno = ENOMEM, err(EXIT_FAILURE, "Add string data");

  /* substitute an 'o' for a 'W' */
  if (!vstr_sub_rep_chr(s1, strlen("HelloWW"), 1, 'o', 1))
    errno = ENOMEM, err(EXIT_FAILURE, "Substitute string data");

  /* substitute "WWW" for a "rld\n" -- */
  if (!vstr_sub_cstr_buf(s1, strlen("HelloWoW"), strlen("WWW"), "rld\n"))
    errno = ENOMEM, err(EXIT_FAILURE, "Substitute string data");

  if (!(ref = vstr_ref_make_ptr((char *)"XYZ ", vstr_ref_cb_free_ref)))
    errno = ENOMEM, err(EXIT_FAILURE, "Create data reference");
  /* now ref->ptr is "XYZ " */

  /* add space after "Hello", by skipping "XYZ" in reference */
  vstr_add_ref(s1, strlen("Hello"), ref, strlen("XYZ"), strlen(" "));

  vstr_ref_del(ref); /* delete our reference to the Vstr_ref */

  if (s1->conf->malloc_bad)
    errno = ENOMEM, err(EXIT_FAILURE, "Add string data");

  while (io_put(s1, STDOUT_FILENO) != IO_NONE) {}

  exit (hw_exit(s1));
}
