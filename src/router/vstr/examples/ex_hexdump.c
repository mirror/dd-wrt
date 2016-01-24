/* This is a fairly simple hexdump program, it has command line options to
 * enable printing of high latin symbols, and/or use mmap() to load the input
 * data.
 *
 * Reads from stdin if no args are given.
 *
 * This shows how to use the Vstr library for simple data convertion.
 *
 * This file is more commented than normal code, so as to make it easy to follow
 * while knowing almost nothing about Vstr or Linux IO programming.
 */

#include "ex_utils.h"

#define CONF_USE_MMAP_DEF FALSE
#define CONF_PRNT_TYPE    PRNT_SPAC

#include "hexdump.h"

/* configure what ASCII characters we print */
static unsigned int prnt_high_chars = CONF_PRNT_TYPE;

static void ex_hexdump_process_limit(Vstr_base *s1, Vstr_base *s2,
                                     unsigned int lim)
{
  while (s2->len > lim)
  { /* Finish processing read data (try writing if we need memory) */
    int proc_data = ex_hexdump_process(s1, s1->len, s2, 1, s2->len,
                                       prnt_high_chars, EX_MAX_W_DATA_INCORE,
                                       TRUE, !lim);

    if (s1->conf->malloc_bad)
      errno = ENOMEM, err(EXIT_FAILURE, "adding data");

    if (!proc_data && (io_put(s1, STDOUT_FILENO) == IO_BLOCK))
      io_block(-1, STDOUT_FILENO);
  }
}

/* we process an entire file at a time... */
static void ex_hexdump_read_fd_write_stdout(Vstr_base *s1, Vstr_base *s2,
                                            int fd)
{
  /* read/process/write loop */
  while (TRUE)
  {
    int io_w_state = IO_OK;
    int io_r_state = io_get(s2, fd);
    
    if (io_r_state == IO_EOF)
      break;
    
    ex_hexdump_process(s1, s1->len, s2, 1, s2->len,
                       prnt_high_chars, EX_MAX_W_DATA_INCORE,
                       TRUE, FALSE);

    if (s1->conf->malloc_bad)
      errno = ENOMEM, err(EXIT_FAILURE, "adding data");

    io_w_state = io_put(s1, STDOUT_FILENO);

    io_limit(io_r_state, fd, io_w_state, STDOUT_FILENO, s1);
  }

  /* write out all of the end of the file,
   * so the next file starts on a new line */
  ex_hexdump_process_limit(s1, s2, 0);
}


int main(int argc, char *argv[])
{ /* This is "hexdump", as it should be by default */
  Vstr_base *s2 = NULL;
  Vstr_base *s1 = ex_init(&s2); /* init the library, and create two strings */
  int count = 1; /* skip the program name */
  unsigned int use_mmap = CONF_USE_MMAP_DEF;

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
    
    else if (!strcmp("--none", argv[count])) /* choose what is displayed */
      prnt_high_chars = PRNT_NONE; /* just simple 7 bit ASCII, no spaces */
    else if (!strcmp("--space", argv[count]))
      prnt_high_chars = PRNT_SPAC; /* allow spaces */
    else if (!strcmp("--high", argv[count]))
      prnt_high_chars = PRNT_HIGH; /* allow high bit characters */
    
    else if (!strcmp("--version", argv[count]))
    { /* print version and exit */
      vstr_add_fmt(s1, 0, "%s", "\
jhexdump 1.0.0\n\
Written by James Antill\n\
\n\
Uses Vstr string library.\n\
");
      goto out;
    }
    else if (!strcmp("--help", argv[count]))
    { /* print version and exit */
      vstr_add_fmt(s1, 0, "%s", "\
Usage: jhexdump [FILENAME]...\n\
   or: jhexdump OPTION\n\
Output filenames in human hexdump format.\n\
\n\
      --help     Display this help and exit\n\
      --version  Output version information and exit\n\
      --high     Allow space and high characters in ASCII output\n\
      --none     Allow only small amount of characters ASCII output\n\
      --space    Allow space characters in ASCII output (default)\n\
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
    ex_hexdump_read_fd_write_stdout(s1, s2, STDIN_FILENO);
  }

  /* loop through all arguments, open the file specified
   * and do the read/write loop */
  while (count < argc)
  {
    unsigned int ern = 0;

    ASSERT(!s2->len); /* all input is fully processed before each new file */
    
    /* try to mmap the file */
    if (use_mmap)
      vstr_sc_mmap_file(s2, s2->len, argv[count], 0, 0, &ern);

    if (!use_mmap ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_FSTAT_ERRNO) ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_MMAP_ERRNO) ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_TOO_LARGE))
    { /* if mmap didn't work ... do a read/alter/write loop */
      int fd = io_open(argv[count]);
      
      ex_hexdump_read_fd_write_stdout(s1, s2, fd);

      if (close(fd) == -1)
        warn("close(%s)", argv[count]);
    }
    else if (ern && (ern != VSTR_TYPE_SC_MMAP_FILE_ERR_CLOSE_ERRNO))
      err(EXIT_FAILURE, "add");
    else /* mmap worked so processes the entire file at once */
      ex_hexdump_process_limit(s1, s2, 0);
    
    ++count;
  }
  ASSERT(!s2->len); /* all input is fully processed before each new file */
  
  /* Cleanup... */
 out:
  io_put_all(s1, STDOUT_FILENO);
  
  exit (ex_exit(s1, s2));
}
