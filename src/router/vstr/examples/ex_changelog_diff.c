/* This is a _really_simple_ program, to extract the difference between two
 * ChangeLog files. Assumptions are that the new stuff is _always_ added at the
 * front of the new file.
 *
 * Assumes mmap() will work on both files, at once.
 *
 * Assumes first 20 bytes of old ChangeLog don't change, and that any changes
 * total less than 10 bytes.
 *
 */
#define EX_UTILS_NO_USE_GET   1
#define EX_UTILS_NO_USE_LIMIT 1
#define EX_UTILS_NO_USE_OPEN  1

#include "ex_utils.h"

#define LINE_DIFF_LEN 10

static size_t add(Vstr_base *s1, const char *fname)
{
  size_t old = s1->len;
  unsigned int ern = 0;
  
  if (!vstr_sc_mmap_file(s1, s1->len, fname, 0, 0, &ern))
    err(EXIT_FAILURE, "mmap(%s)", fname);
  if (old == s1->len)
    errx(EXIT_FAILURE, "mmap(%s) is an empty file", fname);
    
  return (vstr_sc_posdiff(++old, s1->len));
}

int main(int argc, char *argv[])
{
  Vstr_base *s1 = ex_init(NULL); /* init the library etc. */
  size_t old_f_pos = 0;
  size_t old_f_len = 0;
  size_t new_f_pos = 0;
  size_t new_f_len = 0;
  size_t srch_pos = 0;
  size_t srch_len = 0;
  size_t old_first_line_len = 0;
  size_t found = 0;
  
  /* if no arguments are given just do stdin to stdout */
  if (argc != 3)
    errx(EXIT_FAILURE, "Format: %s <old ChangeLog> <new ChangeLog>", argv[0]);

  old_f_len = add(s1, argv[1]);
  new_f_len = add(s1, argv[2]);

  old_f_pos = 1;
  new_f_pos = old_f_len + 1;

  if (old_f_len > new_f_len)
    errx(EXIT_FAILURE, "old ChangeLog is larger than new ChangeLog");

  srch_pos = old_f_pos + old_first_line_len;
  srch_len = old_f_len - old_first_line_len;
  old_first_line_len += vstr_spn_cstr_chrs_fwd(s1,  srch_pos, srch_len, "\n");
  srch_pos = old_f_pos + old_first_line_len;
  srch_len = old_f_len - old_first_line_len;
  old_first_line_len += vstr_cspn_cstr_chrs_fwd(s1, srch_pos, srch_len, "\n");
  srch_pos = old_f_pos + old_first_line_len;
  srch_len = old_f_len - old_first_line_len;
  old_first_line_len += vstr_spn_cstr_chrs_fwd(s1,  srch_pos, srch_len, "\n");

  srch_pos = new_f_pos;
  srch_len = new_f_len;
  while ((found = vstr_srch_vstr_fwd(s1, srch_pos, srch_len,
                                     s1, old_f_pos, old_first_line_len)))
  {
    size_t oln_len = vstr_sc_posdiff(found, s1->len);

    if (oln_len > old_f_len)
    { if ((oln_len   - old_f_len) <= LINE_DIFF_LEN) break; }
    else
    { if ((old_f_len - oln_len)   <= LINE_DIFF_LEN) break; }
    
    srch_pos = found + 1;
    srch_len = vstr_sc_posdiff(srch_pos, s1->len);
  }

  if (!found)
    errx(EXIT_FAILURE, "old ChangeLog doesn't have a line of data");
  
  vstr_del(s1, found, vstr_sc_posdiff(found, s1->len));
  vstr_del(s1, old_f_pos, old_f_len);

  /* get rid of blanks from beg. and end */
  vstr_del(s1, 1, vstr_spn_cstr_chrs_fwd(s1, 1, s1->len, "\n"));
  srch_len = vstr_spn_cstr_chrs_rev(s1, 1, s1->len, "\n");
  if (srch_len)
    vstr_sc_reduce(s1, 1, s1->len, srch_len - 1);
  
  /* output all remaining data */
  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, NULL));
}
