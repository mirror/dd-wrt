/* Unix nl command */
#include "ex_utils.h"

#define CONF_USE_MMAP_DEF FALSE

#define EX_NL_SECTS_LOOP 32 /* how many sections to split into per loop */
#define EX_NL_USE_SRCH_MOV 0
#define EX_NL_USE_SPLIT 0

#if EX_NL_USE_SPLIT /* this version using split shouldn't be any slower */
static int ex_nl_process(Vstr_base *s1, Vstr_base *s2, int last)
{
  static unsigned int count = 0;
  int ret = FALSE;
  
  /* we don't want to create more data, if we are over our limit */
  if (s1->len > EX_MAX_W_DATA_INCORE)
    return (FALSE);

  while (s2->len)
  {
    const int flags = (VSTR_FLAG_SPLIT_REMAIN |
                       VSTR_FLAG_SPLIT_BEG_NULL |
                       VSTR_FLAG_SPLIT_MID_NULL |
                       VSTR_FLAG_SPLIT_END_NULL |
                       VSTR_FLAG_SPLIT_NO_RET);
    VSTR_SECTS_DECL(sects, EX_NL_SECTS_LOOP);
    unsigned int num = 0;

    VSTR_SECTS_DECL_INIT(sects);
    vstr_split_buf(s2, 1, s2->len, "\n", 1, sects, sects->sz, flags);

    if ((sects->num != sects->sz) && !last)
      --sects->num; /* last one isn't full line */

    if (!sects->num)
      return (ret);

    while (++num <= sects->num)
    {
      size_t split_pos = VSTR_SECTS_NUM(sects, num)->pos;
      size_t split_len = VSTR_SECTS_NUM(sects, num)->len;

      vstr_add_fmt(s1, s1->len, "% 6d\t", ++count);
      if (split_len)
        vstr_add_vstr(s1, s1->len, s2, split_pos, split_len,
                      VSTR_TYPE_ADD_BUF_REF);
      vstr_add_cstr_buf(s1, s1->len, "\n");

      if (s1->conf->malloc_bad) /* checks all three above */
        errno = ENOMEM, err(EXIT_FAILURE, "adding data");
    }

    if (sects->num != sects->sz)
      vstr_del(s2, 1, s2->len);
    else
    {
      size_t pos = VSTR_SECTS_NUM(sects, sects->num)->pos;
      size_t len = VSTR_SECTS_NUM(sects, sects->num)->len;
      vstr_del(s2, 1, vstr_sc_poslast(pos, len + 1));
    }

    ret = TRUE;
    
    if (s1->len > EX_MAX_W_DATA_INCORE)
      return (ret);
  }

  return (FALSE);
}
#else
static int ex_nl_process(Vstr_base *s1, Vstr_base *s2, int last)
{
  static unsigned int count = 0;
  size_t pos = 0;
  int ret = FALSE;
  
  /* we don't want to create more data, if we are over our limit */
  if (s1->len > EX_MAX_W_DATA_INCORE)
    return (FALSE);

  if (!s2->len)
    return (FALSE);

  while ((pos = vstr_srch_chr_fwd(s2, 1, s2->len, '\n')))
  {
    ret = TRUE;
    
    vstr_add_fmt(s1, s1->len, "% 6d\t", ++count);

    if (EX_NL_USE_SRCH_MOV)
      vstr_mov(s1, s1->len, s2, 1, pos);
    else
    { /* The flag turns _BUF nodes into sharable _REF nodes */
      vstr_add_vstr(s1, s1->len, s2, 1, pos, VSTR_TYPE_ADD_BUF_REF);
      vstr_del(s2, 1, pos);
    }
    
    if (s1->conf->malloc_bad) /* checks all three above */
      errno = ENOMEM, err(EXIT_FAILURE, "adding data");
    
    if (s1->len > EX_MAX_W_DATA_INCORE)
      return (ret);
  }

  if (s2->len && last)
  {
    ret = TRUE;
    
    vstr_add_fmt(s1, s1->len, "% 6d\t", ++count);
    if (s2->len)
      vstr_mov(s1, s1->len, s2, 1, s2->len);
    vstr_add_cstr_buf(s1, s1->len, "\n");

    if (s1->conf->malloc_bad) /* checks all three above */
      errno = ENOMEM, err(EXIT_FAILURE, "adding data");
  }

  return (ret);
}
#endif

/* files are merged */
static void ex_nl_read_fd_write_stdout(Vstr_base *s1, Vstr_base *s2, int fd)
{
  while (TRUE)
  {
    int io_w_state = IO_OK;
    int io_r_state = io_get(s2, fd);

    if (io_r_state == IO_EOF)
      break;

    ex_nl_process(s1, s2, FALSE);

    io_w_state = io_put(s1, 1);

    io_limit(io_r_state, fd, io_w_state, STDOUT_FILENO, s1);
  }
}

static void ex_nl_process_limit(Vstr_base *s1, Vstr_base *s2, unsigned int lim)
{
  while (s2->len > lim)
  {
    int proc_data = ex_nl_process(s1, s2, !lim);
    if (!proc_data && (io_put(s1, STDOUT_FILENO) == IO_BLOCK))
      io_block(-1, STDOUT_FILENO);
  }
}

int main(int argc, char *argv[])
{
  Vstr_base *s2 = NULL;
  Vstr_base *s1 = ex_init(&s2);
  int count = 1;
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
jnl 1.0.0\n\
Written by James Antill\n\
\n\
Uses Vstr string library.\n\
");
      goto out;
    }
    else if (!strcmp("--help", argv[count]))
    { /* print version and exit */
      vstr_add_fmt(s1, 0, "%s", "\
Usage: jnl [FILENAME]...\n\
   or: jnl OPTION\n\
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
    ex_nl_read_fd_write_stdout(s1, s2, STDIN_FILENO);
  }

  /* loop through all arguments, open the file specified
   * and do the read/write loop */
  while (count < argc)
  {
    unsigned int ern = 0;

    if (use_mmap)
      vstr_sc_mmap_file(s2, s2->len, argv[count], 0, 0, &ern);

    if (!use_mmap ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_FSTAT_ERRNO) ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_MMAP_ERRNO) ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_TOO_LARGE))
    {
      int fd = io_open(argv[count]);

      ex_nl_read_fd_write_stdout(s1, s2, fd);

      if (close(fd) == -1)
        warn("close(%s)", argv[count]);
    }
    else if (ern && (ern != VSTR_TYPE_SC_MMAP_FILE_ERR_CLOSE_ERRNO))
      err(EXIT_FAILURE, "add");
    else
      ex_nl_process_limit(s1, s2, EX_MAX_R_DATA_INCORE);
    
    ++count;
  }

  ex_nl_process_limit(s1, s2, 0);

 out:
  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, s2));
}
