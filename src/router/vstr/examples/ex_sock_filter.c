/* parses input from "tcpdump -dd" outputs raw filter,
 * Ie. sock filter compiler. Why can't tcpdump do this :( */
#include "ex_utils.h"
#include <limits.h>

#define CONF_USE_MMAP_DEF FALSE

/* not in glibc... */
struct sock_filter
{
 uint16_t   code;   /* Actual filter code */
 uint8_t    jt;     /* Jump true */
 uint8_t    jf;     /* Jump false */
 uint32_t   k;      /* Generic multiuse field */
};

/* not needed here ... see ex_httpd etc. */
struct sock_fprog
{
 unsigned short len;    /* Number of filter blocks */
 struct sock_filter *filter;
};

/* is the cstr a prefix of the vstr */
#define VPREFIX(vstr, p, l, cstr)                                       \
    (((l) >= strlen(cstr)) && vstr_cmp_bod_cstr_eq(vstr, p, l, cstr))


#define SOCK_FILTER_OP(x, y, j, k) else if (VPREFIX(s2, pos, len, x))   \
      do {                                                              \
        *num_len = strlen(x);                                           \
        *js = (j);                                                      \
        *ks = (k);                                                      \
        return (y);                                                     \
      } while (FALSE)

/* add ops with X */
#define SOCK_FILTER_OP_PX(x, y, j)                 \
    SOCK_FILTER_OP(x "x", y | 0x8, j, FALSE);      \
    SOCK_FILTER_OP(x, y, j, TRUE)

/* human version... */
static unsigned int ex_sock_filter_parse_op(Vstr_base *s2,
                                            size_t pos, size_t len,
                                            size_t *num_len, int *js, int *ks)
{
  size_t tmp = 0;
  
  tmp = vstr_spn_cstr_chrs_fwd(s2, pos, len, " ");
  len -= tmp; pos += tmp;
  
  if (0){ } /* FIXME: not all ops */
  SOCK_FILTER_OP("lda1",  0x30, FALSE, TRUE);
  SOCK_FILTER_OP("lda2",  0x28, FALSE, TRUE);
  SOCK_FILTER_OP("lda4",  0x20, FALSE, TRUE);

  SOCK_FILTER_OP("ldlx",  0x81, FALSE, FALSE);
  SOCK_FILTER_OP("ldl",   0x80, FALSE, FALSE);
  
  SOCK_FILTER_OP("ldmx",  0x61, FALSE, TRUE);
  SOCK_FILTER_OP("ldm",   0x60, FALSE, TRUE);
  SOCK_FILTER_OP("stmx",  0x03, FALSE, TRUE);
  SOCK_FILTER_OP("stm",   0x02, FALSE, TRUE);
  
  SOCK_FILTER_OP_PX("add",  0x04, FALSE);
  SOCK_FILTER_OP_PX("sub",  0x14, FALSE);
  SOCK_FILTER_OP_PX("mul",  0x24, FALSE);
  SOCK_FILTER_OP_PX("div",  0x34, FALSE);
  SOCK_FILTER_OP_PX("or",   0x44, FALSE);
  SOCK_FILTER_OP_PX("and",  0x54, FALSE);
  SOCK_FILTER_OP_PX("lsh",  0x64, FALSE);
  SOCK_FILTER_OP_PX("rsh",  0x74, FALSE);
  SOCK_FILTER_OP_PX("neg",  0x84, FALSE);
  
  SOCK_FILTER_OP_PX("ja",   0x05, TRUE);
  SOCK_FILTER_OP_PX("jeq",  0x15, TRUE);
  SOCK_FILTER_OP_PX("jgt",  0x25, TRUE);
  SOCK_FILTER_OP_PX("jge",  0x35, TRUE);
  SOCK_FILTER_OP_PX("jset", 0x45, TRUE);
  
  SOCK_FILTER_OP("tax",  0x07, FALSE, FALSE);
  SOCK_FILTER_OP("txa",  0x87, FALSE, FALSE);
  
  SOCK_FILTER_OP("reta", 0x16, FALSE, FALSE);
  SOCK_FILTER_OP("ret",  0x06, FALSE, TRUE);

  return (0);
}

static int ex_sock_filter_process(Vstr_base *s1, Vstr_base *s2)
{
  size_t srch = 0;
  int ret = FALSE;
  struct sock_filter filter[1];
  static unsigned int filter_count = 0;
  static unsigned int line_count = 0;
  
  /* we don't want to create more data, if we are over our limit */
  if (s1->len > EX_MAX_W_DATA_INCORE)
    return (FALSE);
  
  while ((srch = vstr_srch_chr_fwd(s2, 1, s2->len, '\n')))
  {
    size_t pos = 1;
    size_t len = srch;
    size_t num_len = 0;
    int js = TRUE;
    int ks = TRUE;
    unsigned int num_flags = (VSTR_FLAG_PARSE_NUM_SEP |
                              VSTR_FLAG_PARSE_NUM_OVERFLOW |
                              VSTR_FLAG_PARSE_NUM_SPACE |
                              VSTR_FLAG_PARSE_NUM_NO_NEGATIVE);
    int special_ld_cmd = FALSE;
    
    ++line_count;
    
    if (VPREFIX(s2, pos, len, "#"))
    { /* comments */
      vstr_del(s2, 1, srch);
      ret = TRUE;
      continue;
    }
    filter->code = filter->jt = filter->jf = filter->k = 0;
    
    if (filter_count == USHRT_MAX)
      errno = ENOMEM, err(EXIT_FAILURE, "too many filters");

    if (!VPREFIX(s2, pos, len, "{ "))
      errx(EXIT_FAILURE, "parse error 1, line %u", line_count);
    len -= strlen("{ "); pos += strlen("{ ");

    filter->code = vstr_parse_uint(s2, pos, len, num_flags, &num_len, NULL);
    len -= num_len; pos += num_len;

    if (!num_len)
    {
      filter->code = ex_sock_filter_parse_op(s2, pos, len, &num_len, &js, &ks);
      len -= num_len; pos += num_len;
    }
    if (!num_len)
      errx(EXIT_FAILURE, "parse error 2, line %u", line_count);

    if ((filter->code == 0x30) || (filter->code == 0x28) ||
        (filter->code == 0x20))
      special_ld_cmd = TRUE;
    
    if (js)
    {
      if (!VPREFIX(s2, pos, len, ", "))
        errx(EXIT_FAILURE, "parse error 3, line %u", line_count);
      len -= strlen(", "); pos += strlen(", ");
      
      filter->jt   = vstr_parse_uint(s2, pos, len, num_flags, &num_len, NULL);
      len -= num_len; pos += num_len;
      
      if (!VPREFIX(s2, pos, len, ", "))
        errx(EXIT_FAILURE, "parse error 4, line %u", line_count);
      len -= strlen(", "); pos += strlen(", ");
    
      filter->jf   = vstr_parse_uint(s2, pos, len, num_flags, &num_len, NULL);
      len -= num_len; pos += num_len;
    }

    if (ks)
    {
      if (!VPREFIX(s2, pos, len, ", "))
        errx(EXIT_FAILURE, "parse error 5, line %u", line_count);
      len -= strlen(", "); pos += strlen(", ");
      
      num_len = vstr_spn_cstr_chrs_fwd(s2, pos, len, " ");
      len -= num_len; pos += num_len;

      num_len = 0;
      /* see linux/filter.h ... magic off sets as we start at TCP */
      if (!special_ld_cmd) { }
      else if (VPREFIX(s2, pos, len, "ad:"))
      {
        num_len = strlen("ad:");
        filter->k = -0x1000;
      }
      else if (VPREFIX(s2, pos, len, "net:"))
      {
        num_len = strlen("net:");
        filter->k = -0x100000;
      }
      else if (VPREFIX(s2, pos, len, "ll:"))
      {
        num_len = strlen("ll:");
        filter->k = -0x200000;
      }
      len -= num_len; pos += num_len;
      
      filter->k += vstr_parse_uint(s2, pos, len, num_flags, &num_len, NULL);
      len -= num_len; pos += num_len;
    }
    
    if (!VPREFIX(s2, pos, len, " },"))
      errx(EXIT_FAILURE, "parse error 6, line %u", line_count);
    
    ++filter_count;
    vstr_del(s2, 1, srch);
    ret = TRUE;

    if (!vstr_add_buf(s1, s1->len, filter, sizeof(filter)))
      errno = ENOMEM, err(EXIT_FAILURE, "adding data");
    
    if (s1->len > EX_MAX_W_DATA_INCORE)
      return (ret);
  }
  
  return (ret);
}

static void ex_sock_filter_process_limit(Vstr_base *s1, Vstr_base *s2,
                                         unsigned int lim)
{
  while (s2->len > lim)
  { /* Finish processing read data (try writing if we need memory) */
    int proc_data = ex_sock_filter_process(s1, s2);

    if (!proc_data && (io_put(s1, STDOUT_FILENO) == IO_BLOCK))
      io_block(-1, STDOUT_FILENO);
  }
}

static void ex_sock_filter_read_fd_write_stdout(Vstr_base *s1, Vstr_base *s2,
                                                int fd)
{
  /* read/process/write loop */
  while (TRUE)
  {
    int io_w_state = IO_OK;
    int io_r_state = io_get(s2, fd);
    
    if (io_r_state == IO_EOF)
      break;
    
    ex_sock_filter_process(s1, s2);

    io_w_state = io_put(s1, STDOUT_FILENO);

    io_limit(io_r_state, fd, io_w_state, STDOUT_FILENO, s1);
  }

  /* write out all of the end of the file,
   * so the next file starts on a new line */
  ex_sock_filter_process_limit(s1, s2, 0);
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
jsock_filter 1.0.0\n\
Written by James Antill\n\
\n\
Uses Vstr string library.\n\
");
      goto out;
    }
    else if (!strcmp("--help", argv[count]))
    { /* print version and exit */
      vstr_add_fmt(s1, 0, "%s", "\
Usage: jsock_filter [FILENAME]...\n\
   or: jsock_filter OPTION\n\
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
    ex_sock_filter_read_fd_write_stdout(s1, s2, STDIN_FILENO);
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

      ex_sock_filter_read_fd_write_stdout(s1, s2, fd);

      if (close(fd) == -1)
        warn("close(%s)", argv[count]);
    }
    else if (ern && (ern != VSTR_TYPE_SC_MMAP_FILE_ERR_CLOSE_ERRNO))
      err(EXIT_FAILURE, "add");
    else
      ex_sock_filter_process_limit(s1, s2, EX_MAX_R_DATA_INCORE);
    
    ++count;
  }

  ex_sock_filter_process_limit(s1, s2, 0);

 out:
  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, s2));
}

