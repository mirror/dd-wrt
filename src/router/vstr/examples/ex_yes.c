/* Unix "yes" command */

#define EX_UTILS_NO_USE_INPUT 1

#include "ex_utils.h"

#include <limits.h>

#define OUT_DEF_NUM 2 /* how many lines to add at once, by default */

#define OUT_LIM_DEF_NUM 0 /* how many lines to do in total, by default */

#define OUT_PTR 1 /* copy ptr's to data */
#define OUT_BUF 2 /* copy data to */
#define OUT_REF 3 /* copy data once, to coalese */

#define MAX_W_DATA_INCORE (1024 * 128)

static unsigned int sz = MAX_W_DATA_INCORE;

int main(int argc, char *argv[])
{ /* This is "yes", without any command line options */
  Vstr_base *cmd_line = NULL;
  Vstr_base *s1 = ex_init(&cmd_line);
  int count = 1; /* skip program name */
  unsigned int out_type = OUT_PTR;
  unsigned int out_num = OUT_DEF_NUM;
  unsigned int lim_num = OUT_LIM_DEF_NUM;
  const char *out_filename = NULL;
  int out_fd = -1;

  while (count < argc)
  { /* quick hack getopt_long */
    if (!strcmp("--", argv[count]))
    {
      ++count;
      break;
    }
    else if (!strcmp("--ptr", argv[count]))
      out_type = OUT_PTR;
    else if (!strcmp("--buf", argv[count]))
      out_type = OUT_BUF;
    else if (!strcmp("--ref", argv[count]))
      out_type = OUT_REF;
    EX_UTILS_GETOPT_NUM("sz",    sz);
    EX_UTILS_GETOPT_NUM("num",   out_num);
    EX_UTILS_GETOPT_NUM("limit", lim_num);
    EX_UTILS_GETOPT_NUM("lim",   lim_num);
    EX_UTILS_GETOPT_CSTR("file",  out_filename);
    else if (!strcmp("--version", argv[count]))
    {
      vstr_add_fmt(s1, 0, "%s", "\
yes 1.0.0\n\
Written by James Antill\n\
\n\
Uses Vstr string library.\n\
");
      goto out;
    }
    else if (!strcmp("--help", argv[count]))
    {
      vstr_add_fmt(s1, 0, "%s", "\
Usage: yes [STRING]...\n\
   or: yes OPTION\n\
Repeatedly output a line with all specified STRING(s), or `y'.\n\
\n\
      --help     display this help and exit\n\
      --version  output version information and exit\n\
      --ptr      output using pointers to data\n\
      --buf      output using copies of data\n\
      --ref      output using single copy of data, with references\n\
      --num      number of times to add the line, between output calls\n\
      --sz       output maximum size, lower bound\n\
      --         Use rest of cmd line input regardless of if it's an option\n\
\n\
Report bugs to James Antill <james@and.org>.\n\
");
      goto out;
    }
    else
      break;
    ++count;
  }

  if (count >= argc)
    VSTR_ADD_CSTR_PTR(cmd_line, cmd_line->len, "y");

  {
    int added = FALSE;

    while (count < argc)
    {
      if (added) /* already done one argument */
        VSTR_ADD_CSTR_PTR(cmd_line, cmd_line->len, " ");
      VSTR_ADD_CSTR_PTR(cmd_line, cmd_line->len, argv[count]);

      ++count;
      added = TRUE;
    }
  }

  VSTR_ADD_CSTR_PTR(cmd_line, cmd_line->len, "\n");

  if (0)
  { ASSERT(FALSE); }
  else if (out_type == OUT_PTR)
  { /* do nothing */ }
  else if (out_type == OUT_BUF)
  {
    size_t len = cmd_line->len;
    vstr_add_vstr(cmd_line, len, cmd_line, 1, len, VSTR_TYPE_ADD_ALL_BUF);
    vstr_del(cmd_line, 1, len);
  }
  else if (out_type == OUT_REF)
  {
    size_t len = cmd_line->len;
    vstr_add_vstr(cmd_line, len, cmd_line, 1, len, VSTR_TYPE_ADD_ALL_REF);
    vstr_del(cmd_line, 1, len);
  }
  else
    errx(EXIT_SUCCESS, "INTERNAL ERROR ... bad out_type");

  if (cmd_line->conf->malloc_bad)
    errno = ENOMEM, err(EXIT_SUCCESS, "Creating output string: %m\n");

  if (!out_num)
    ++out_num;

  if (!out_filename)
    out_fd = 1;
  else
    out_fd = io_open(out_filename);

  /* BEG: main loop */

  do
  {
    count = out_num;

    while ((s1->len < sz) && (--count >= 0))
      vstr_add_vstr(s1, s1->len, cmd_line, 1, cmd_line->len, VSTR_TYPE_ADD_DEF);

    if ((io_put(s1, out_fd) == IO_BLOCK) && (s1->len >= sz))
      io_block(-1, out_fd);
    
    /* 0 == infinity */
    if    (lim_num == 1) break;
    if    (lim_num >  1) --lim_num;
  } while (TRUE);

 out:
  io_put_all(s1, out_fd);

  exit (ex_exit(s1, cmd_line));
}
