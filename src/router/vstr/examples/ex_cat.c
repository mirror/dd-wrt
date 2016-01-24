/* This is a _simple_ cat program.
 * Reads from stdin if no args are given.
 *
 * This shows how to use the Vstr library at it's simpelest,
 * for easy and fast IO. Note however that all needed error detection is
 * included.
 *
 * This file is more commented than normal code, so as to make it easy to follow
 * while knowing almost nothing about Vstr or Linux IO programming.
 */
#include "ex_utils.h"

#define CONF_USE_MMAP_DEF FALSE

/*  Keep reading on the file descriptor until there is no more data (ERR_EOF)
 * abort if there is an error reading or writing */
static void ex_cat_read_fd_write_stdout(Vstr_base *s1, int fd)
{
  while (TRUE)
  {
    int io_w_state = IO_OK;
    int io_r_state = io_get(s1, fd);

    if (io_r_state == IO_EOF)
      break;
    
    io_w_state = io_put(s1, STDOUT_FILENO);

    io_limit(io_r_state, fd, io_w_state, STDOUT_FILENO, s1);    
  }
}

static void ex_cat_limit(Vstr_base *s1)
{
  while ((s1->len >= EX_MAX_W_DATA_INCORE) || (s1->len >= EX_MAX_R_DATA_INCORE))
  {
    if (io_put(s1, STDOUT_FILENO) == IO_BLOCK)
      io_block(-1, STDOUT_FILENO);
  }
}

/* This is "cat", using non-blocking IO and Vstr for buffer space */
int main(int argc, char *argv[])
{
  Vstr_base *s1 = ex_init(NULL); /* init the library etc. */
  int count = 1; /* skip the program name */
  int use_mmap = CONF_USE_MMAP_DEF;  
  
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
jcat 1.0.0\n\
Written by James Antill\n\
\n\
Uses Vstr string library.\n\
");
      goto out;
    }
    else if (!strcmp("--help", argv[count]))
    { /* print version and exit */
      vstr_add_fmt(s1, 0, "%s", "\
Usage: jcat [FILENAME]...\n\
   or: jcat OPTION\n\
Output filenames.\n\
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
    ex_cat_read_fd_write_stdout(s1, STDIN_FILENO);
  }
  
  /* loop through all arguments, open the file specified
   * and do the read/write loop */
  while (count < argc)
  {
    unsigned int ern = 0;

    if (use_mmap)
      vstr_sc_mmap_file(s1, s1->len, argv[count], 0, 0, &ern);

    if (!use_mmap ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_FSTAT_ERRNO) ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_MMAP_ERRNO) ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_TOO_LARGE))
    { /* if mmap didn't work ... do a read/alter/write loop */
      int fd = io_open(argv[count]);

      ex_cat_read_fd_write_stdout(s1, fd);

      if (close(fd) == -1)
        warn("close(%s)", argv[count]);
    }
    else if (ern && (ern != VSTR_TYPE_SC_MMAP_FILE_ERR_CLOSE_ERRNO))
      err(EXIT_FAILURE, "add");
    else /* mmap worked */
      ex_cat_limit(s1);

    ++count;
  }

  /* output all remaining data */
 out:
  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, NULL));
}
