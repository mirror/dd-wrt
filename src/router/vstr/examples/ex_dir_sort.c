/* needs to be big, as all enteries have to be in core */
#define EX_MAX_R_DATA_INCORE (16 * 1024 * 1024)

#include "ex_utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <locale.h>

#include "opt.h"

#include "bag.h"

static Bag *names = NULL;

static int (*sort_cmp)(const void *, const void *) = bag_cb_sort_key_coll;


static int ex_dir_sort_process(Vstr_base *s1, Vstr_base *s2,
                               int *parsed_header)
{
  size_t scan_pos = 1;
  size_t scan_len = s2->len;

  while (scan_len)
  {
    size_t pos = 0;
    size_t len = 0;
    size_t ns1 = vstr_parse_netstr(s2, scan_pos, scan_len, &pos, &len);
  
    if (!ns1)
    {
      if (s2->len > EX_MAX_R_DATA_INCORE)
        errx(EXIT_FAILURE, "input too big");
      if (len     > EX_MAX_R_DATA_INCORE)
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
      
      vstr_add_vstr(s1, s1->len, s2, 1, ns1, 0);
      vstr_del(s2, 1, ns1);
      scan_pos = 1;
      scan_len = s2->len;
      
      continue;
    }
    
    while (len)
    {
      size_t kpos = 0;
      size_t klen = 0;
      size_t vpos = 0;
      size_t vlen = 0;
      size_t nst  = 0;
      char *key = NULL;
      Vstr_sect_node *val = malloc(sizeof(Vstr_sect_node));
      
      if (!val)
        errno = ENOMEM, err(EXIT_FAILURE, "sort");
      
      if (!(nst = vstr_parse_netstr(s2, pos, len, &kpos, &klen)))
        errx(EXIT_FAILURE, "bad input");
      pos += nst; len -= nst;
      
      if (!(nst = vstr_parse_netstr(s2, pos, len, &vpos, &vlen)))
        errx(EXIT_FAILURE, "bad input");    
      pos += nst; len -= nst;
      
      if (!vstr_cmp_cstr_eq(s2, kpos, klen, "name"))
        continue; /* only sort by names atm. */
      
      if (!(key = vstr_export_cstr_malloc(s2, vpos, vlen)))
        errno = ENOMEM, err(EXIT_FAILURE, "sort");
      
      val->pos = scan_pos;
      val->len = ns1;
      
      if (!(names = bag_add_obj(names, key, val)))
        errno = ENOMEM, err(EXIT_FAILURE, "sort");
      break;
    }

    scan_len -= ns1;
    scan_pos += ns1;
  }

  return (TRUE);
}

static void ex_dir_sort_read_fd_write_stdout(Vstr_base *s1, Vstr_base *s2,
                                             int fd)
{
  int parsed_header[1] = {FALSE};
  
  while (TRUE)
  {
    int io_w_state = IO_OK;
    int io_r_state = io_get(s2, fd);

    if (io_r_state == IO_EOF)
      break;

    io_w_state = io_put(s1, STDOUT_FILENO);
    
    io_limit(io_r_state, fd, io_w_state, STDOUT_FILENO, s1);    
  }
  
  ex_dir_sort_process(s1, s2, parsed_header);
    
  bag_sort(names, sort_cmp);
  
  {
    Bag_iter iter[1];
    const Bag_obj *obj = bag_iter_beg(names, iter);

    while (obj)
    {
      const Vstr_sect_node *val = obj->val;
      
      vstr_add_vstr(s1, s1->len, s2, val->pos, val->len, 0);
      obj = bag_iter_nxt(iter);
    }

    if (s1->conf->malloc_bad)
      errno = ENOMEM, err(EXIT_FAILURE, "sort");
    vstr_del(s2, 1, s2->len);
  }

  bag_del_all(names);
}

static void ex_dir_sort_init(Vstr_base *s1)
{
  if (!vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$') ||
      !vstr_sc_fmt_add_all(s1->conf))
    errno = ENOMEM, err(EXIT_FAILURE, "init");
  
  if (!(names = bag_make(16, bag_cb_free_malloc, bag_cb_free_malloc)))
    errno = ENOMEM, err(EXIT_FAILURE, "init");
  names->can_resize = TRUE;
}

static void usage(const char *program_name, int ret, const char *prefix)
{
  Vstr_base *out = vstr_make_base(NULL);

  if (!out)
    errno = ENOMEM, err(EXIT_FAILURE, "usage");

  vstr_add_fmt(out, 0, "%s\n"
         " Usage: %s [-hV] [FILES]\n"
         "   or: %s OPTION\n"
         " --help -h         - Print this message.\n"
         " --sort            - Use cmp, case, version or collating sorting.\n"
         " --version -V      - Print the version string.\n",
         prefix, program_name, program_name);
  
  if (io_put_all(out, ret ? STDERR_FILENO : STDOUT_FILENO) == IO_FAIL)
    err(EXIT_FAILURE, "write");
  
  exit (ret);
}

static void ex_dir_sort_cmd_line(int *passed_argc, char **passed_argv[])
{
  int    argc = *passed_argc;
  char **argv = *passed_argv;
  char optchar = 0;
  const char *program_name = NULL;
  struct option long_options[] =
  { /* allow sorting by size etc. */
   {"help", no_argument, NULL, 'h'},
   {"sort", required_argument, NULL, 1},
   {"version", no_argument, NULL, 'V'},
   {NULL, 0, NULL, 0}
  };
  Vstr_base *out = vstr_make_base(NULL);

  if (!out)
    errno = ENOMEM, err(EXIT_FAILURE, "command line");
  
  program_name = opt_program_name(argv[0], "jdir_sort");

  while ((optchar = getopt_long(argc, argv, "hV",
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

      case 1:
        if (0) {}
        else if (!strcmp(optarg, "cmp"))       sort_cmp = bag_cb_sort_key_cmp;
        else if (!strcmp(optarg, "case"))      sort_cmp = bag_cb_sort_key_case;
        else if (!strcmp(optarg, "version"))   sort_cmp = bag_cb_sort_key_vers;
        else if (!strcmp(optarg, "collating")) sort_cmp = bag_cb_sort_key_coll;
        else
          usage(program_name, EXIT_FAILURE, "");
        break;
          
      default:
        abort();
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

  setlocale(LC_ALL, "");
  
  ex_dir_sort_init(s1);

  ex_dir_sort_cmd_line(&argc, &argv);
  
  /* if no arguments are given just do stdin to stdout */
  if (count >= argc)
  {
    io_fd_set_o_nonblock(STDIN_FILENO);
    ex_dir_sort_read_fd_write_stdout(s1, s2, STDIN_FILENO);
  }
  
  /* loop through all arguments, open the dir specified
   * and do the read/write loop */
  while (count < argc)
  {
    int fd = io_open(argv[count]);

    ex_dir_sort_read_fd_write_stdout(s1, s2, fd);

    if (close(fd) == -1)
      warn("close(%s)", argv[count]);

    ++count;
  }

  bag_free(names);
    
  /* output all remaining data */
  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, s2));
}
