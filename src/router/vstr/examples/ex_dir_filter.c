
#include "ex_utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>

#include "opt.h"

#include "bag.h"

static Bag *filters = NULL;

#define CSTREQ(x, y) (strcmp(x, y) == 0)


#define FILTER_MATCH(name, cmp)                                 \
    (CSTREQ(obj->key, name) && cmp (s2, vpos, vlen, obj->val))

static int vprefix(Vstr_base *s1, size_t pos, size_t len, const char *s2)
{
  size_t clen = strlen(s2);

  if (clen > len)
    return (FALSE);
  
  return (vstr_cmp_bod_buf_eq(s1, pos, len, s2, clen));
}

static int vsuffix(Vstr_base *s1, size_t pos, size_t len, const char *s2)
{
  size_t clen = strlen(s2);

  if (clen > len)
    return (FALSE);
  
  return (vstr_cmp_eod_buf_eq(s1, pos, len, s2, clen));
}

static int vany(Vstr_base *s1, size_t pos, size_t len, const char *s2)
{
  size_t clen = strlen(s2);

  if (clen > len)
    return (FALSE);

  return (vstr_srch_buf_fwd(s1, pos, len, s2, clen) != 0);
}

static int ex_dir_filter_process(Vstr_base *s1, Vstr_base *s2,
                                 int *parsed_header)
{
  size_t pos = 0;
  size_t len = 0;
  size_t ns1 = vstr_parse_netstr(s2, 1, s2->len, &pos, &len);
  
  if (!ns1)
  {
    if ((len     > EX_MAX_R_DATA_INCORE) || 
        (s2->len > EX_MAX_R_DATA_INCORE))
      errx(EXIT_FAILURE, "bad input");
  
    return (FALSE);
  }

  if (!*parsed_header)
  {
    size_t vpos = 0;
    size_t vlen = 0;
    size_t nst  = 0;
    
    if (!(nst = vstr_parse_netstr(s2, pos, len, &vpos, &vlen)))
      errx(EXIT_FAILURE, "bad input");
    pos += nst; len -= nst;
    if (!vstr_cmp_cstr_eq(s2, vpos, vlen, "version"))
      errx(EXIT_FAILURE, "bad input");
    if (!(nst = vstr_parse_netstr(s2, pos, len, &vpos, &vlen)))
      errx(EXIT_FAILURE, "bad input");
    pos += nst; len -= nst;
    if (!vstr_cmp_cstr_eq(s2, vpos, vlen, "1"))
      errx(EXIT_FAILURE, "Unsupported version");
    *parsed_header = TRUE;
    len = 0;
  }
  
  while (len)
  {
    size_t kpos = 0;
    size_t klen = 0;
    size_t vpos = 0;
    size_t vlen = 0;
    size_t nst  = 0;
    Bag_iter iter[1];
    const Bag_obj *obj = NULL;
    
    if (!(nst = vstr_parse_netstr(s2, pos, len, &kpos, &klen)))
      errx(EXIT_FAILURE, "bad input");
    pos += nst; len -= nst;
    
    if (!(nst = vstr_parse_netstr(s2, pos, len, &vpos, &vlen)))
      errx(EXIT_FAILURE, "bad input");    
    pos += nst; len -= nst;

    if (!vstr_cmp_cstr_eq(s2, kpos, klen, "name"))
      continue; /* only names atm. */
    
    if (!(obj = bag_iter_beg(filters, iter)))
      break;

    do
    {
      int keep = TRUE;

      if (0) { /* do nothing */ }
      else if (FILTER_MATCH("acpt-name-eq", vstr_cmp_cstr_eq)) keep = TRUE;
      else if (FILTER_MATCH("deny-name-eq", vstr_cmp_cstr_eq)) keep = FALSE;
      else if (FILTER_MATCH("acpt-name-beg", vprefix))         keep = TRUE;
      else if (FILTER_MATCH("deny-name-beg", vprefix))         keep = FALSE;
      else if (FILTER_MATCH("acpt-name-end", vsuffix))         keep = TRUE;
      else if (FILTER_MATCH("deny-name-end", vsuffix))         keep = FALSE;
      else if (FILTER_MATCH("acpt-name-any", vany))            keep = TRUE;
      else if (FILTER_MATCH("deny-name-any", vany))            keep = FALSE;
      else if (CSTREQ(obj->key, "acpt-all"))                   keep = TRUE;
      else if (CSTREQ(obj->key, "deny-all"))                   keep = FALSE;
      else
        continue;

      if (keep)
        break;
      else
      { /* skip */
        vstr_del(s2, 1, ns1);
        return (TRUE);
      }
    } while ((obj = bag_iter_nxt(iter)));
  }

  vstr_add_vstr(s1, s1->len, s2, 1, ns1, 0);
  vstr_del(s2, 1, ns1);

  return (TRUE);
}

static void ex_dir_filter_process_limit(Vstr_base *s1, Vstr_base *s2,
                                           int *parsed_header)
{
  while (s2->len)
  { /* Finish processing read data (try writing if we need memory) */
    int proc_data = ex_dir_filter_process(s1, s2, parsed_header);

    if (!proc_data && (io_put(s1, STDOUT_FILENO) == IO_BLOCK))
      io_block(-1, STDOUT_FILENO);
  }
}

static void ex_dir_filter_read_fd_write_stdout(Vstr_base *s1, Vstr_base *s2,
                                                  int fd)
{
  int parsed_header[1] = {FALSE};
  
  while (TRUE)
  {
    int io_w_state = IO_OK;
    int io_r_state = io_get(s2, fd);

    if (io_r_state == IO_EOF)
      break;

    ex_dir_filter_process(s1, s2, parsed_header);
    
    io_w_state = io_put(s1, STDOUT_FILENO);

    io_limit(io_r_state, fd, io_w_state, STDOUT_FILENO, s1);    
  }
  
  ex_dir_filter_process_limit(s1, s2, parsed_header);
}

static void ex_dir_filter_init(Vstr_base *s1)
{
  if (!vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$') ||
      !vstr_sc_fmt_add_all(s1->conf))
    errno = ENOMEM, err(EXIT_FAILURE, "init");
}

static void usage(const char *program_name, int ret, const char *prefix)
{
  Vstr_base *out = vstr_make_base(NULL);

  if (!out)
    errno = ENOMEM, err(EXIT_FAILURE, "usage");

  vstr_add_fmt(out, 0, "%s\n"
          "Usage: %s [-hV] [FILES]\n"
          "   or: %s OPTION\n"
          " --accept-name-eq\n"
          " --acpt-name-eq   -A - Accept names equal to argument.\n"
          " --deny-name-eq   -D - Deny names equal to argument.\n"
          " --accept-name-beg\n"
          " --acpt-name-beg     - Accept names begining with the argument.\n"
          " --deny-name-beg     - Deny names begining with the argument.\n"
          " --accept-name-end\n"
          " --acpt-name-end     - Accept names ending with the argument.\n"
          " --deny-name-end     - Deny names ending with the argument.\n"
          " --accept-name-any\n"
          " --acpt-name-any     - Accept names with the argument.\n"
          " --deny-name-any     - Deny names with the argument.\n"
          " --accept-all\n"
          " --acpt-all          - Accept everything.\n"
          " --deny-all          - Deny everything.\n"
          " --help -h           - Print this message.\n"
          " --version -V        - Print the version string.\n",
          prefix, program_name, program_name);
  
  if (io_put_all(out, ret ? STDERR_FILENO : STDOUT_FILENO) == IO_FAIL)
    err(EXIT_FAILURE, "write");
  
  exit (ret);
}

static void ex_dir_filter_cmd_line(int *passed_argc, char **passed_argv[])
{
  int    argc = *passed_argc;
  char **argv = *passed_argv;
  char optchar = 0;
  const char *program_name = NULL;
  struct option long_options[] =
  {
   /* allow filtering on size etc. */
   {"help", no_argument, NULL, 'h'},

   {"accept-name-eq", required_argument, NULL, 'A'},
   {"acpt-name-eq", required_argument, NULL, 'A'},
   {"deny-name-eq", required_argument, NULL, 'D'},
   
   {"accept-name-beg", required_argument, NULL, 1},
   {"acpt-name-beg",   required_argument, NULL, 1},
   {"deny-name-beg",   required_argument, NULL, 2},
   
   {"accept-name-end", required_argument, NULL, 3},
   {"acpt-name-end",   required_argument, NULL, 3},
   {"deny-name-end",   required_argument, NULL, 4},
   
   {"accept-name-any", required_argument, NULL, 5},
   {"acpt-name-any",   required_argument, NULL, 5},
   {"deny-name-any",   required_argument, NULL, 6},
   
   {"accept-all", no_argument, NULL, 7},
   {"acpt-all",   no_argument, NULL, 7},
   {"deny-all",   no_argument, NULL, 8},
   
   {"version", no_argument, NULL, 'V'},
   {NULL, 0, NULL, 0}
  };
  Vstr_base *out = vstr_make_base(NULL);

  if (!out)
    errno = ENOMEM, err(EXIT_FAILURE, "command line");

  program_name = opt_program_name(argv[0], "jdir_filter");

  if (!(filters = bag_make(argc, bag_cb_free_nothing, bag_cb_free_nothing)))
    errno = ENOMEM, err(EXIT_FAILURE, "init");
  
  while ((optchar = getopt_long(argc, argv, "A:D:hV",
                                long_options, NULL)) != -1)
  {
    switch (optchar)
    {
      case '?': usage(program_name, EXIT_FAILURE, "");
      case 'h': usage(program_name, EXIT_SUCCESS, "");
        
      case 'V':
        vstr_add_fmt(out, 0,"\
%s version 1.0.0, compiled on %s.\n\
Written by James Antill\n\
\n\
Uses Vstr string library.\n\
",
               program_name, __DATE__);
        
        if (io_put_all(out, STDOUT_FILENO) == IO_FAIL)
          err(EXIT_FAILURE, "write");
        
        exit (EXIT_SUCCESS);

      case 'A': bag_add_cstr(filters, "acpt-name-eq",  optarg); break;
      case 'D': bag_add_cstr(filters, "deny-name-eq",  optarg); break;
      case   1: bag_add_cstr(filters, "acpt-name-beg", optarg); break;
      case   2: bag_add_cstr(filters, "deny-name-beg", optarg); break;
      case   3: bag_add_cstr(filters, "acpt-name-end", optarg); break;
      case   4: bag_add_cstr(filters, "deny-name-end", optarg); break;
      case   5: bag_add_cstr(filters, "acpt-name-any", optarg); break;
      case   6: bag_add_cstr(filters, "deny-name-any", optarg); break;
      case   7: bag_add_cstr(filters, "acpt-all",      NULL);   break;
      case   8: bag_add_cstr(filters, "deny-all",      NULL);   break;
        
      default:
        ASSERT(FALSE);
    }
  }
  vstr_free_base(out); out = NULL;

  argc -= optind;
  argv += optind;

  *passed_argc = argc;
  *passed_argv = argv;
}

int main(int argc, char *argv[])
{
  Vstr_base *s1 = NULL;
  Vstr_base *s2 = ex_init(&s1); /* init the library etc. */
  int count = 0;

  ex_dir_filter_init(s1);

  ex_dir_filter_cmd_line(&argc, &argv);
  
  /* if no arguments are given just do stdin to stdout */
  if (count >= argc)
  {
    io_fd_set_o_nonblock(STDIN_FILENO);
    ex_dir_filter_read_fd_write_stdout(s1, s2, STDIN_FILENO);
  }
  
  /* loop through all arguments, open the dir specified
   * and do the read/write loop */
  while (count < argc)
  {
    int fd = io_open(argv[count]);

    ex_dir_filter_read_fd_write_stdout(s1, s2, fd);

    if (close(fd) == -1)
      warn("close(%s)", argv[count]);

    ++count;
  }

  bag_free(filters);
  
  /* output all remaining data */
  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, s2));
}
