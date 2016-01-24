/* this program does a character count of it's input */

#include "ex_utils.h"
#include <locale.h>
#include <ctype.h>

#define CONF_USE_MMAP_DEF FALSE

static void prnt_chrs(Vstr_base *s1, char chr, size_t *len)
{
  if (isprint((unsigned char)chr))
    vstr_add_fmt(s1, s1->len, " '%c' [%#04x] * %zu\n", chr, chr, *len);
  else
    vstr_add_fmt(s1, s1->len, " '?' [%#04x] * %zu\n", chr, *len);

  if (s1->conf->malloc_bad) /* checks all three above */
    errno = ENOMEM, err(EXIT_FAILURE, "adding data");
    
  *len = 0;
}

static int ex_ccount_process(Vstr_base *s1, Vstr_base *s2, int last)
{
  static size_t prev_len = 0;
  static char prev_chr = 0;
  int ret = FALSE;

  if (s1->len > EX_MAX_W_DATA_INCORE)
    return (FALSE);

  if (!s2->len && last && prev_len)
    prnt_chrs(s1, prev_chr, &prev_len);
  
  while (s2->len)
  {
    char chrs[1];
    int did_all = FALSE;
    size_t len = 0;
    
    chrs[0] = vstr_export_chr(s2, 1);
    if (prev_len && (chrs[0] != prev_chr))
      prnt_chrs(s1, prev_chr, &prev_len);
    
    len = vstr_spn_chrs_fwd(s2, 1, s2->len, chrs, 1);

    ret = TRUE;

    did_all = (len == s2->len);
    vstr_del(s2, 1, len);

    prev_len += len;
    prev_chr  = chrs[0];
      
    if (did_all && !last)
      break;

    prnt_chrs(s1, prev_chr, &prev_len);
    
    if (s1->len > EX_MAX_W_DATA_INCORE)
      return (FALSE);
  }

  return (ret);
}

static void ex_ccount_process_limit(Vstr_base *s1, Vstr_base *s2,
                                    unsigned int lim)
{
  while (s2->len > lim)
  { /* Finish processing read data (try writing if we need memory) */
    int proc_data = ex_ccount_process(s1, s2, !lim);
 
    if (!proc_data && (io_put(s1, STDOUT_FILENO) == IO_BLOCK))
      io_block(-1, STDOUT_FILENO);
  }
}

static void ex_ccount_read_fd_write_stdout(Vstr_base *s1, Vstr_base *s2, int fd)
{
  while (TRUE)
  {
    int io_w_state = IO_OK;
    int io_r_state = io_get(s2, fd);
                                                                                
    if (io_r_state == IO_EOF)
      break;

    ex_ccount_process(s1, s2, FALSE);
    
    io_w_state = io_put(s1, STDOUT_FILENO);
                                                                                
    io_limit(io_r_state, fd, io_w_state, STDOUT_FILENO, s1);
  }

  ex_ccount_process_limit(s1, s2, 0);
}
                                                                                
int main(int argc, char *argv[])
{
  Vstr_base *s2 = NULL;
  Vstr_base *s1 = ex_init(&s2); /* init the library, and create two strings */
  int count = 1; /* skip the program name */
  unsigned int use_mmap = CONF_USE_MMAP_DEF;
  unsigned int ern = 0;
  
  setlocale(LC_ALL, "");
  
  /* parse command line arguments... */
  while (count < argc)
  { /* quick hack getopt_long */
    if (!strcmp("--", argv[count]))
    {
      ++count;
      break;
    }
    else if (!strcmp("--mmap", argv[count])) /* toggle use of mmap */
      use_mmap = !use_mmap;
    else if (!strcmp("--version", argv[count]))
    { /* print version and exit */
      vstr_add_fmt(s1, 0, "%s", "\
jccount 1.0.0\n\
Written by James Antill\n\
\n\
Uses Vstr string library.\n\
");
      goto out;
    }
    else if (!strcmp("--help", argv[count]))
    { /* print version and exit */
      vstr_add_fmt(s1, 0, "%s", "\
Usage: jccount [FILENAME]...\n\
   or: jccount OPTION\n\
Output filenames in byte/character count format.\n\
\n\
      --help     Display this help and exit\n\
      --version  Output version information and exit\n\
      --mmap     Toggle use of mmap() to load input files\n\
      --         Treat rest of cmd line as input filenames\n\
\n\
Report bugs to James Antill <james@and.org>.\n\
");
      goto out;
    }
    else
      break;
    ++count;
  }
  
  /* if no arguments are given just do stdin to stdout */
  if (count >= argc)
  {
    io_fd_set_o_nonblock(STDIN_FILENO);
    ex_ccount_read_fd_write_stdout(s1, s2, STDIN_FILENO);
  }

  /* loop through all arguments, open the file specified
   * and do the read/write loop */
  while (count < argc)
  {
    /* try to mmap the file */
    if (use_mmap)
      vstr_sc_mmap_file(s2, s2->len, argv[count], 0, 0, &ern);

    if (!use_mmap ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_FSTAT_ERRNO) ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_MMAP_ERRNO) ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_TOO_LARGE))
    { /* if mmap didn't work ... do a read/alter/write loop */
      int fd = io_open(argv[count]);
                                                                                
      ex_ccount_read_fd_write_stdout(s1, s2, fd);
      
      if (close(fd) == -1)
        warn("close(%s)", argv[count]);
    }
    else if (ern && (ern != VSTR_TYPE_SC_MMAP_FILE_ERR_CLOSE_ERRNO))
      err(EXIT_FAILURE, "add");
    else /* mmap worked so processes the entire file at once */
      ex_ccount_process_limit(s1, s2, 0);

    ++count;
  }

  /* output all remaining data */
 out:
  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, NULL));
}
