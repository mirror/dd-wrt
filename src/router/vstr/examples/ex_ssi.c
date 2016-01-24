/* quick but useful static ssi processor.
*/
#include "ex_utils.h"
#include "opt.h"

#include <sys/time.h>
#include <time.h>

#include <pwd.h>

#include <dirent.h>

#include <getopt.h>

#define USE_POPEN 1 /* hacky ... */

#if USE_POPEN
#include <stdio.h>
#else
/* spawn.h doesn't exist on MacOSX ... *sigh*/
#include <spawn.h>
#endif

#define EX_SSI_FAILED(x) do {                                           \
        vstr_add_fmt(s1, s1->len,                                       \
                     "<!-- \"%s\" SSI command FAILED -->\n", (x));      \
        goto ssi_parse_failed;                                          \
    }                                                                   \
    while (FALSE)

#define EX_SSI_OK(x, sf, p, l, end) do {                                \
      vstr_add_fmt(s1, s1->len, "<!-- \"%s ${vstr:%p%zu%zu%u}"          \
                   "\" SSI command OK -->%s",                           \
                   (x), (sf), (p), (l), VSTR_TYPE_ADD_ALL_REF,          \
                   (end) ? "\n" : "");                                  \
    }                                                                   \
    while (FALSE)




static char *timespec = NULL;
static int use_size_abbrev = TRUE;

static void ex_ssi_cat_read_fd_write_stdout(Vstr_base *s1, int fd)
{
  while (TRUE)
  {
    int io_w_state = IO_OK;
    int io_r_state = io_get(s1, fd);
 
    if (io_r_state == IO_EOF)
      break;
                                                                                
    io_w_state = io_put(s1, 1);
                                                                                
    io_limit(io_r_state, fd, io_w_state, 1, s1);
  }
}

static size_t ex_ssi_srch_end(Vstr_base *s2, size_t pos, size_t len)
{
  int instr = FALSE;
  
  while (TRUE)
  {
    size_t srch = vstr_cspn_cstr_chrs_fwd(s2, pos, len, "\\\"-");
      
    pos += srch;
    len -= srch;

    if (!len)
      break;

    if (0) { }
    else if (!instr && vstr_export_chr(s2, pos) == '-')
    { /* see if it's the end... */
      if (len < strlen("-->"))
        return (0);
      
      if (vstr_cmp_bod_cstr_eq(s2, pos, len, "-->"))
        return (pos + strlen("->"));
    }
    else if (!instr && vstr_export_chr(s2, pos) == '\\')
    { /* do nothing */ }
    else if ( instr && vstr_export_chr(s2, pos) == '\\')
    {
      if (len > 1)
      {
        ++pos;
        --len;
      }
    }
    else if (vstr_export_chr(s2, pos) == '"')
    { instr = !instr; }

    ++pos;
    --len;
  }
  
  return (0);
}

static inline void ex_ssi_skip_val(Vstr_base *s2, size_t *srch, size_t val)
{
  vstr_del(s2, 1, val);
  *srch        -= val;  
}

static inline void ex_ssi_skip_wsp(Vstr_base *s2, size_t *srch)
{
  size_t val = vstr_spn_cstr_chrs_fwd(s2, 1, *srch, " ");

  ex_ssi_skip_val(s2, srch, val);
}

static inline void ex_ssi_skip_str(Vstr_base *s2, size_t *srch,
                                   const char *passed_val)
{
  size_t val = strlen(passed_val);
  
  ex_ssi_skip_val(s2, srch, val);
}

static size_t ex_ssi_attr_val(Vstr_base *s2, size_t *srch)
{
  size_t pos = 1;
  size_t len = *srch;
  size_t ret = 0;
  
  while (TRUE)
  {
    size_t scan = vstr_cspn_cstr_chrs_fwd(s2, pos, len, "\\\"");

    if (scan == len)
      return (0);

    ret += scan;
    
    if (vstr_export_chr(s2, pos + scan) != '\\')
      break;

    if (scan == (len - 1))
      return (0);

    ++ret;
    
    vstr_del(s2, pos + scan, 1);
    pos += scan + 1;
    len -= scan + 1;
    --*srch;

    ASSERT(len);
  }

  return (ret);
}

static size_t ex_ssi_file_attr(Vstr_base *s2, size_t *srch)
{
  size_t ret = 0;
  
  ex_ssi_skip_wsp(s2, srch);
  
  if (vstr_cmp_case_bod_cstr_eq(s2, 1, *srch, "file=\""))
    ex_ssi_skip_str(s2, srch, "file=\"");
  else
    return (0);

  if (!(ret = ex_ssi_attr_val(s2, srch)))
    return (0);
  
  return (ret);
}

#if !USE_POPEN
static int ex_ssi_spawn_r(const char *prog, pid_t *pid,
                          char *argv[], char *env[])
{
  posix_spawn_file_actions_t acts[1];
  int fds[2];
  char *dummy_env[] = {NULL};

  if (!env)
    env = dummy_env;
  
  if ((errno = posix_spawn_file_actions_init(acts)))
    err(EXIT_FAILURE, "spawn_make");

  if (pipe(fds) == -1)
    err(EXIT_FAILURE, "pipe");

  if ((errno = posix_spawn_file_actions_adddup2(attr, fds[1], FILENO_STDOUT)))
    err(EXIT_FAILURE, "spawn_dup2");
  if ((errno = posix_spawn_file_actions_addclose(attr, fds[0])))
    err(EXIT_FAILURE, "spawn_close");
  if ((errno = posix_spawn_file_actions_addclose(attr, fds[1])))
    err(EXIT_FAILURE, "spawn_close");

  if ((errno = posix_spawnp(pid, prog, acts, NULL, argv, NULL)))
    err(EXIT_FAILURE, "spawn");
  
  if ((errno = posix_spawn_file_actions_destroy(acts)))
    err(EXIT_FAILURE, "spawn_free");

  close(fds[1]);

  return (fds[0]);
}
#endif

static void ex_ssi_exec(Vstr_base *s1,
                        Vstr_base *s2, size_t pos, size_t len)
{
#if USE_POPEN
  FILE *fp = NULL; /* FIXME: hack job */
  /*  struct stat64 sbuf[1];
   * 
   * if (stat64(vstr_export_cstr_ptr(s2, pos, len), sbuf))
   *   err(EXIT_FAILURE, "stat(%s)", vstr_export_cstr_ptr(s2, pos, len));
   */
  if (!(fp = popen(vstr_export_cstr_ptr(s2, pos, len), "r")))
    err(EXIT_FAILURE, "popen(%s)", vstr_export_cstr_ptr(s2, pos, len));

  ex_ssi_cat_read_fd_write_stdout(s1, fileno(fp));

  pclose(fp);
#else
  Vstr_sects *sects = vstr_sects_make(4);
  size_t srch = 0;
  size_t tpos = pos;
  size_t tlen = len;
  pid_t pid;
  
  while ((srch < tlen) &&
         (tmp = vstr_cspn_cstr_chrs_fwd(s2, , tlen, " ")))
  {
  }

  /* FIXME: doesn't handle <foo "bar baz" arg2>... */
  vstr_split_cstr_buf(s2, pos, len, " ", sects, 0, VSTR_FLAG_SPLIT_NO_RET);
  
  ex_ssi_cat_read_fd_write_stdout(s1,
                                  ex_ssi_spawn_r(argv[0], &pid, argv, NULL));
  waitpid(pid, NULL, 0);
#endif
}

static const char *ex_ssi_strftime(time_t val, int use_gmt)
{
  static char ret[4096];
  const char *spec = timespec;
  struct tm *tm_val = NULL;
  
  if (!spec)  spec = "%c";

  if (use_gmt)
    tm_val = gmtime(&val);
  else
    tm_val = localtime(&val);
  
  if (!tm_val)
    err(EXIT_FAILURE, "gmtime");
      
  strftime(ret, sizeof(ret), spec, tm_val);

  return (ret);
}

static const char *ex_ssi_getpwuid_name(uid_t uid)
{
  struct passwd *pw = getpwuid(uid);

  if (!pw)
    return (":unknown:");

  return (pw->pw_name);
}

static int ex_ssi_process(Vstr_base *s1, Vstr_base *s2, time_t last_modified,
                          int last)
{
  size_t srch = 0;
  int ret = FALSE;
  
  /* we don't want to create more data, if we are over our limit */
  if (s1->len > EX_MAX_W_DATA_INCORE)
    return (FALSE);
  
  while (s2->len >= strlen("<!--#"))
  {
    if (!(srch = vstr_srch_cstr_buf_fwd(s2, 1, s2->len, "<!--#")))
    {
      if (last)
        break;
      
      ret = TRUE;
      vstr_mov(s1, s1->len, s2, 1, s2->len - strlen("<!--#"));
      break;
    }
    
    if (srch > 1)
    {
      ret = TRUE;
      vstr_add_vstr(s1, s1->len, s2, 1, srch - 1, VSTR_TYPE_ADD_BUF_REF);
      vstr_del(s2, 1, srch - 1);
    }

    if (!(srch = ex_ssi_srch_end(s2, 1, s2->len)))
      break;
    
    ret = TRUE;

    if (0) { }
    else if (vstr_cmp_case_bod_cstr_eq(s2, 1, srch, "<!--#include"))
    {
      int fd = -1;
      size_t tmp = 0;

      ex_ssi_skip_str(s2, &srch, "<!--#include");

      if (!(tmp = ex_ssi_file_attr(s2, &srch)))
        EX_SSI_FAILED("include");

      EX_SSI_OK("include", s2, 1, tmp, TRUE);

      if (s1->conf->malloc_bad)
        errno = ENOMEM, err(EXIT_FAILURE, "add data");

      fd = io_open(vstr_export_cstr_ptr(s2, 1, tmp));

      ex_ssi_cat_read_fd_write_stdout(s1, fd);

      if (close(fd) == -1)
        warn("close(%s)", vstr_export_cstr_ptr(s2, 1, tmp));
    }
    else if (vstr_cmp_case_bod_cstr_eq(s2, 1, s2->len, "<!--#exec"))
    {
      size_t tmp = 0;
      
      ex_ssi_skip_str(s2, &srch, "<!--#exec");
      ex_ssi_skip_wsp(s2, &srch);

      if (vstr_cmp_case_bod_cstr_eq(s2, 1, srch, "cmd=\""))
        ex_ssi_skip_str(s2, &srch, "cmd=\"");
      else
        EX_SSI_FAILED("exec");

      if (!(tmp = ex_ssi_attr_val(s2, &srch)))
        EX_SSI_FAILED("exec");

      if (s1->conf->malloc_bad)
        errno = ENOMEM, err(EXIT_FAILURE, "add data");

      EX_SSI_OK("exec", s2, 1, tmp, TRUE);

      ex_ssi_exec(s1, s2, 1, tmp);
    }
    else if (vstr_cmp_case_bod_cstr_eq(s2, 1, s2->len, "<!--#config"))
    {
      size_t tmp = 0;
      enum { tERR = -1,
             tsize, ttime } type = tERR;
      const char *tname[2] = {"config sizefmt", "config timefmt"};
      
      ex_ssi_skip_str(s2, &srch, "<!--#config");
      ex_ssi_skip_wsp(s2, &srch);

      if (0) { }
      else if (vstr_cmp_case_bod_cstr_eq(s2, 1, srch, "timefmt=\""))
        ex_ssi_skip_str(s2, &srch, "timefmt=\""), type = ttime;
      else if (vstr_cmp_case_bod_cstr_eq(s2, 1, srch, "sizefmt=\""))
        ex_ssi_skip_str(s2, &srch, "sizefmt=\""), type = tsize;
      else
        EX_SSI_FAILED("config");

      ASSERT(type >= 0);
      
      if (!(tmp = ex_ssi_attr_val(s2, &srch)))
        EX_SSI_FAILED(tname[type]);

      EX_SSI_OK(tname[type], s2, 1, tmp, FALSE);

      switch (type)
      {
        case ttime:
          free(timespec);
          timespec = vstr_export_cstr_malloc(s2, 1, tmp);
          break;
        case tsize:
          if (0){ }
          else if (vstr_cmp_cstr_eq(s2, 1, tmp, "bytes"))
            use_size_abbrev = FALSE;
          else if (vstr_cmp_cstr_eq(s2, 1, tmp, "abbrev"))
            use_size_abbrev = TRUE;
          break;
        default:
          ASSERT(FALSE);
      }

      /* <!--#config errmsg="foo" --> ? */
    }
    else if (vstr_cmp_case_bod_cstr_eq(s2, 1, s2->len, "<!--#echo"))
    {
      size_t tmp = 0;
      
      ex_ssi_skip_str(s2, &srch, "<!--#echo");
      ex_ssi_skip_wsp(s2, &srch);

      if (vstr_cmp_case_bod_cstr_eq(s2, 1, srch, "encoding=\""))
        ex_ssi_skip_str(s2, &srch, "encoding=\"");
      else
        EX_SSI_FAILED("echo");

      if (!(tmp = ex_ssi_attr_val(s2, &srch)))
        EX_SSI_FAILED("echo");

      if (!vstr_cmp_cstr_eq(s2, 1, tmp, "none"))
        EX_SSI_FAILED("echo");

      srch -= tmp + 1;
      vstr_del(s2, 1, tmp + 1);
      ex_ssi_skip_wsp(s2, &srch);
      
      if (vstr_cmp_case_bod_cstr_eq(s2, 1, srch, "var=\""))
        ex_ssi_skip_str(s2, &srch, "var=\"");
      else
        EX_SSI_FAILED("echo");

      if (!(tmp = ex_ssi_attr_val(s2, &srch)))
        EX_SSI_FAILED("echo");

      if (0) { }
      else if (vstr_cmp_cstr_eq(s2, 1, tmp, "LAST_MODIFIED"))
        vstr_add_cstr_buf(s1, s1->len, ex_ssi_strftime(last_modified, FALSE));
      else if (vstr_cmp_cstr_eq(s2, 1, tmp, "DATE_GMT"))
        vstr_add_cstr_buf(s1, s1->len, ex_ssi_strftime(time(NULL), TRUE));
      else if (vstr_cmp_cstr_eq(s2, 1, tmp, "DATE_LOCAL"))
        vstr_add_cstr_buf(s1, s1->len, ex_ssi_strftime(time(NULL), FALSE));
      else if (vstr_cmp_cstr_eq(s2, 1, tmp, "USER_NAME"))
        vstr_add_cstr_buf(s1, s1->len, ex_ssi_getpwuid_name(getuid()));
      else
        EX_SSI_FAILED("echo");

      EX_SSI_OK("echo", s2, 1, tmp, FALSE);

      
      /* <!--#echo encoding="none" var="LAST_MODIFIED" --> */
      /* <!--#echo encoding="url" var="LAST_MODIFIED" --> */
      /* <!--#echo encoding="entity" var="LAST_MODIFIED" --> */
      
      /* <!--#echo var="DOCUMENT_NAME" --> */
      /* <!--#echo var="DOCUMENT_URI" --> */
      
    }
    else if (vstr_cmp_case_bod_cstr_eq(s2, 1, s2->len, "<!--#fsize"))
    {
      size_t tmp = 0;
      struct stat64 sbuf[1];
      
      ex_ssi_skip_str(s2, &srch, "<!--#fsize");

      if (!(tmp = ex_ssi_file_attr(s2, &srch)))
        EX_SSI_FAILED("fsize");

      EX_SSI_OK("fsize", s2, 1, tmp, FALSE);

      if (s1->conf->malloc_bad)
        errno = ENOMEM, err(EXIT_FAILURE, "add data");

      if (stat64(vstr_export_cstr_ptr(s2, 1, tmp), sbuf))
        err(EXIT_FAILURE, "stat(%s)", vstr_export_cstr_ptr(s2, 1, tmp));

      if (use_size_abbrev)
        vstr_add_fmt(s1, s1->len, "${BKMG.ju:%ju}",
                     (VSTR_AUTOCONF_uintmax_t)sbuf->st_size);
      else
        vstr_add_fmt(s1, s1->len, "%ju",
                     (VSTR_AUTOCONF_uintmax_t)sbuf->st_size);
    }
    else if (vstr_cmp_case_bod_cstr_eq(s2, 1, s2->len, "<!--#flastmod"))
    {
      size_t tmp = 0;
      struct stat64 sbuf[1];
      
      ex_ssi_skip_str(s2, &srch, "<!--#flastmod");

      if (!(tmp = ex_ssi_file_attr(s2, &srch)))
        EX_SSI_FAILED("flastmod");

      EX_SSI_OK("flastmod", s2, 1, tmp, FALSE);

      if (s1->conf->malloc_bad)
        errno = ENOMEM, err(EXIT_FAILURE, "add data");

      if (stat64(vstr_export_cstr_ptr(s2, 1, tmp), sbuf))
        err(EXIT_FAILURE, "stat(%s)", vstr_export_cstr_ptr(s2, 1, tmp));
        
      vstr_add_cstr_buf(s1, s1->len, ex_ssi_strftime(sbuf->st_mtime, TRUE));
    }
    else if (0 && vstr_cmp_case_bod_cstr_eq(s2, 1, s2->len, "<!--#if"))
    {
      ex_ssi_skip_str(s2, &srch, "<!--#if");
      ex_ssi_skip_wsp(s2, &srch);

      /* <!--#if expr="foo" --> ? */
      /* <!--#elif expt="foo" --> ? */
      /* <!--#else --> ? */
      /* <!--#endif --> ? */
      
    }
    else
      vstr_add_cstr_ptr(s1, s1->len, "<!-- UNKNOWN SSI command -->\n");

    ASSERT(vstr_export_chr(s2, srch) == '>');
    
    vstr_del(s2, 1, srch);
  }
  
 ssi_parse_failed:

  if (last && s2->len)
  {
    ret = TRUE;
    vstr_mov(s1, s1->len, s2, 1, s2->len);
  }
  
  return (ret);  
}


static void ex_ssi_process_limit(Vstr_base *s1, Vstr_base *s2,
                                 time_t last_modified, unsigned int lim)
{
  while (s2->len > lim)
  {
    int proc_data = ex_ssi_process(s1, s2, last_modified, !lim);
    if (!proc_data && (io_put(s1, STDOUT_FILENO) == IO_BLOCK))
      io_block(-1, STDOUT_FILENO);
  }
}

static void ex_ssi_read_fd_write_stdout(Vstr_base *s1, Vstr_base *s2, int fd)
{
  struct stat64 sbuf[1];
  time_t last_modified = time(NULL);
  
  if (fstat64(fd, sbuf))
    warn("fstat");
  else
    last_modified = sbuf->st_mtime;
  
  while (TRUE)
  {
    int io_w_state = IO_OK;
    int io_r_state = io_get(s2, fd);

    if (io_r_state == IO_EOF)
      break;

    ex_ssi_process(s1, s2, last_modified, FALSE);
    
    io_w_state = io_put(s1, 1);

    io_limit(io_r_state, fd, io_w_state, 1, s1);    
  }

  ex_ssi_process_limit(s1, s2, last_modified, 0);
}

static void ex_ssi_fin(Vstr_base *s1, time_t timestamp, const char *fname)
{
  free(timespec);
  timespec = NULL;

  vstr_add_fmt(s1, s1->len,
               "<!-- SSI processing of %s -->\n"
               "<!--   done on %s -->\n"
               "<!--   done by jssi -->\n",
               fname, ex_ssi_strftime(timestamp, FALSE));
}

static void usage(const char *program_name, int ret, const char *prefix)
{
  Vstr_base *out = vstr_make_base(NULL);

  if (!out)
    errno = ENOMEM, err(EXIT_FAILURE, "usage");

  vstr_add_fmt(out, 0, "%s\n"
          " Format: %s [-hV]\n"
          " --help -h         - Print this message.\n"
          " --prefix-path     - Prefix path with argument.\n"
          " --suffix-path     - Suffix path with argument.\n"
          " --version -V      - Print the version string.\n",
          prefix, program_name);
  
  if (io_put_all(out, ret ? STDERR_FILENO : STDOUT_FILENO) == IO_FAIL)
    err(EXIT_FAILURE, "write");
  
  exit (ret);
}

static void merge_path(const char *beg, const char *end, const char *name)
{
  Vstr_base *tmp = vstr_dup_cstr_ptr(NULL, beg);
  
  if (!tmp)
    errno = ENOMEM, err(EXIT_FAILURE, "%s", name);
  
  vstr_add_cstr_ptr(tmp, tmp->len, ":");
  vstr_add_cstr_ptr(tmp, tmp->len, end);
  
  if (tmp->conf->malloc_bad || !vstr_export_cstr_ptr(tmp, 1, tmp->len))
    errno = ENOMEM, err(EXIT_FAILURE, "%s", name);
  
  setenv("PATH", vstr_export_cstr_ptr(tmp, 1, tmp->len), TRUE);
  vstr_free_base(tmp);
}

static void cl_cmd_line(int *passed_argc, char ***passed_argv)
{
  int    argc = *passed_argc;
  char **argv = *passed_argv;
  
  char optchar = 0;
  const char *program_name = NULL;
  struct option long_options[] =
  {
   {"help", no_argument, NULL, 'h'},
   {"prefix-path", required_argument, NULL, 1},
   {"suffix-path", required_argument, NULL, 2},
   {"version", no_argument, NULL, 'V'},
   {NULL, 0, NULL, 0}
  };
  Vstr_base *out = vstr_make_base(NULL);

  if (!out)
    errno = ENOMEM, err(EXIT_FAILURE, "command line");
  
  program_name = opt_program_name(argv[0], "jssi");
  
  while ((optchar = getopt_long(argc, argv, "hV", long_options, NULL)) != -1)
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
        merge_path(optarg, getenv("PATH"), "prefix-path");
        break;
        
      case 2:
        merge_path(getenv("PATH"), optarg, "suffix-path");
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
  Vstr_base *s2 = NULL;
  Vstr_base *s1 = ex_init(&s2);
  int count = 0; /* getopt reduces it by one */
  time_t now = time(NULL);
  int beg_dir = -1;
  DIR *beg_dir_obj = NULL;

  cl_cmd_line(&argc, &argv);
  
  vstr_sc_fmt_add_all(s1->conf);
  vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  
  if (count >= argc)
  {
    io_fd_set_o_nonblock(STDIN_FILENO);
    ex_ssi_read_fd_write_stdout(s1, s2, STDIN_FILENO);
    ex_ssi_fin(s1, now, "stdin");    
  }

  if (!(beg_dir_obj = opendir(".")))
    err(EXIT_FAILURE, "opendir(.)");
  beg_dir = dirfd(beg_dir_obj);
    
  while (count < argc)
  {
    int fd = io_open(argv[count]);
    size_t len = strlen(argv[count]);
    size_t dbeg = 0;
    size_t tdname_len = 0;
    
    if (!vstr_add_buf(s1, s1->len, argv[count], len))
      errno = ENOMEM, err(EXIT_FAILURE, "add data");

    if (fchdir(beg_dir) == -1)
      err(EXIT_FAILURE, "fchdir()");
    
    dbeg = s1->len - len + 1;
    vstr_sc_dirname(s1, dbeg, len, &tdname_len);
    if (tdname_len)
    {
      const char *tmp = vstr_export_cstr_ptr(s1, dbeg, tdname_len);
      
      if (chdir(tmp) == -1)
        err(EXIT_FAILURE, "chdir(%s)", tmp);
    }
    vstr_del(s1, s1->len - len + 1, len);
    
    ex_ssi_read_fd_write_stdout(s1, s2, fd);
    ex_ssi_fin(s1, now, argv[count]);    

    if (close(fd) == -1)
      warn("close(%s)", argv[count]);

    ++count;
  }
  closedir(beg_dir_obj);

  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, s2));
}
