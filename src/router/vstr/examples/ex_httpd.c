/*
 *  Copyright (C) 2004, 2005  James Antill
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  email: james@and.org
 */
/* conditionally compliant HTTP/1.1 server. */
#define _GNU_SOURCE 1 /* strsignal() / posix_fadvise64 */
#include <vstr.h>

#include <socket_poll.h>
#include <timer_q.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <err.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <grp.h>

#define EX_UTILS_NO_USE_INIT  1
#define EX_UTILS_NO_USE_EXIT  1
#define EX_UTILS_NO_USE_LIMIT 1
#define EX_UTILS_NO_USE_OPEN  1
#define EX_UTILS_NO_USE_GET   1
#define EX_UTILS_NO_USE_IO_FD 1
#define EX_UTILS_RET_FAIL     1
#include "ex_utils.h"

#include "mk.h"

MALLOC_CHECK_DECL();

#include "cntl.h"
#include "date.h"

#define HTTPD_HAVE_GLOBAL_OPTS 1

#include "httpd.h"
#include "httpd_policy.h"

#define CLEN VSTR__AT_COMPILE_STRLEN

/* is the cstr a prefix of the vstr */
#define VPREFIX(vstr, p, l, cstr)                                       \
    (((l) >= CLEN(cstr)) &&                                             \
     vstr_cmp_buf_eq(vstr, p, CLEN(cstr), cstr, CLEN(cstr)))
/* is the cstr a suffix of the vstr */
#define VSUFFIX(vstr, p, l, cstr)                                       \
    (((l) >= CLEN(cstr)) &&                                             \
     vstr_cmp_eod_buf_eq(vstr, p, l, cstr, CLEN(cstr)))

/* is the cstr a prefix of the vstr, no case */
#define VIPREFIX(vstr, p, l, cstr)                                      \
    (((l) >= CLEN(cstr)) &&                                             \
     vstr_cmp_case_buf_eq(vstr, p, CLEN(cstr), cstr, CLEN(cstr)))

/* for simplicity */
#define VEQ(vstr, p, l, cstr) vstr_cmp_cstr_eq(vstr, p, l, cstr)

static Vlg *vlg = NULL;

static void usage(const char *program_name, int ret, const char *prefix)
{
  Vstr_base *out = vstr_make_base(NULL);

  if (!out)
    errno = ENOMEM, err(EXIT_FAILURE, "usage");

  vstr_add_fmt(out, 0, "%s\n\
 Format: %s [options] <dir>\n\
  Daemon options\n\
    --daemon          - Toggle becoming a daemon%s.\n\
    --chroot          - Change root.\n\
    --drop-privs      - Toggle droping privilages%s.\n\
    --priv-uid        - Drop privilages to this uid.\n\
    --priv-gid        - Drop privilages to this gid.\n\
    --pid-file        - Log pid to file.\n\
    --cntl-file       - Create control file.\n\
    --accept-filter-file\n\
                      - Load Linux Socket Filter code for accept().\n\
    --processes       - Number of processes to use (default: 1).\n\
    --debug -d        - Raise debug level (can be used upto 3 times).\n\
    --host -H         - IPv4 address to bind (default: \"all\").\n\
    --help -h         - Print this message.\n\
    --max-connections\n\
                  -M  - Max connections allowed (0 = no limit).\n\
    --nagle -n        - Toggle usage of nagle TCP option%s.\n\
    --port -P         - Port to bind to.\n\
    --idle-timeout -t - Timeout (usecs) for connections that are idle.\n\
    --defer-accept    - Time to defer dataless connections (default: 8s)\n\
    --version -V      - Print the version string.\n\
\n\
  HTTPD options\n\
    --mmap            - Toggle use of mmap() to load files%s.\n\
    --sendfile        - Toggle use of sendfile() to load files%s.\n\
    --keep-alive      - Toggle use of Keep-Alive handling%s.\n\
    --keep-alive-1.0  - Toggle use of Keep-Alive handling for HTTP/1.0%s.\n\
    --virtual-hosts\n\
    --vhosts          - Toggle use of directory virtual hostnames%s.\n\
    --range           - Toggle use of partial responces%s.\n\
    --range-1.0       - Toggle use of partial responces for HTTP/1.0%s.\n\
    --public-only     - Toggle use of public only privilages%s.\n\
    --directory-filename\n\
    --dir-filename    - Filename to use when requesting directories.\n\
    --server-name     - Contents of server header used in replies.\n\
    --gzip-content-replacement\n\
                      - Toggle use of gzip content replacement%s.\n\
    --mime-types-main - Main mime types filename (default: /etc/mime.types).\n\
    --mime-types-xtra - Additional mime types filename.\n\
    --error-406       - Toggle sending 406 responses%s.\n\
    --canonize-host   - Strip leading 'www.', strip trailing '.'%s.\n\
    --error-host-400  - Give an 400 error for a bad host%s.\n\
    --check-host      - Whether we check host headers at all%s.\n\
    --unspecified-hostname\n\
                      - Used for req with no Host header (default is hostname).\n\
    --max-header-sz   - Max size of http header (0 = no limit).\n\
",
               prefix, program_name,
               opt_def_toggle(FALSE), opt_def_toggle(FALSE),
               opt_def_toggle(EVNT_CONF_NAGLE),
               opt_def_toggle(HTTPD_CONF_USE_MMAP),
               opt_def_toggle(HTTPD_CONF_USE_SENDFILE),
               opt_def_toggle(HTTPD_CONF_USE_KEEPA),
               opt_def_toggle(HTTPD_CONF_USE_KEEPA_1_0),
               opt_def_toggle(HTTPD_CONF_USE_VHOSTS_NAME),
               opt_def_toggle(HTTPD_CONF_USE_RANGE),
               opt_def_toggle(HTTPD_CONF_USE_RANGE_1_0),
               opt_def_toggle(HTTPD_CONF_USE_PUBLIC_ONLY),
               opt_def_toggle(HTTPD_CONF_USE_ENC_CONTENT_REPLACEMENT),
               opt_def_toggle(HTTPD_CONF_USE_ERR_406),
               opt_def_toggle(HTTPD_CONF_USE_CANONIZE_HOST),
               opt_def_toggle(HTTPD_CONF_USE_HOST_ERR_400),
               opt_def_toggle(HTTPD_CONF_USE_HOST_ERR_CHK));

  if (io_put_all(out, ret ? STDERR_FILENO : STDOUT_FILENO) == IO_FAIL)
    err(EXIT_FAILURE, "write");
  
  exit (ret);
}

static void serv_init(void)
{
  if (!vstr_init()) /* init the library */
    errno = ENOMEM, err(EXIT_FAILURE, "init");

  vlg_init();

  if (!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_TYPE_GRPALLOC_CACHE,
                      VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR))
    errno = ENOMEM, err(EXIT_FAILURE, "init");

  if (!vstr_cntl_conf(NULL,
                      VSTR_CNTL_CONF_SET_NUM_BUF_SZ, OPT_SERV_CONF_BUF_SZ))
    errno = ENOMEM, err(EXIT_FAILURE, "init");
  
  /* no passing of conf to evnt */
  if (!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$') ||
      !vstr_sc_fmt_add_all(NULL) ||
      !vlg_sc_fmt_add_all(NULL) ||
      !VSTR_SC_FMT_ADD(NULL, http_fmt_add_vstr_add_vstr,
                       "<http-esc.vstr", "p%zu%zu", ">") ||
      !VSTR_SC_FMT_ADD(NULL, http_fmt_add_vstr_add_sect_vstr,
                       "<http-esc.vstr.sect", "p%p%u", ">"))
    errno = ENOMEM, err(EXIT_FAILURE, "init");

  if (!(vlg = vlg_make()))
    errno = ENOMEM, err(EXIT_FAILURE, "init");

  if (!VSTR_SC_FMT_ADD(vlg->out_vstr->conf, http_fmt_add_vstr_add_vstr,
                       "<http-esc.vstr", "p%zu%zu", ">") ||
      !VSTR_SC_FMT_ADD(vlg->out_vstr->conf, http_fmt_add_vstr_add_sect_vstr,
                       "<http-esc.vstr.sect", "p%p%u", ">"))
    errno = ENOMEM, err(EXIT_FAILURE, "init");
  if (!VSTR_SC_FMT_ADD(vlg->sig_out_vstr->conf, http_fmt_add_vstr_add_vstr,
                       "<http-esc.vstr", "p%zu%zu", ">") ||
      !VSTR_SC_FMT_ADD(vlg->sig_out_vstr->conf, http_fmt_add_vstr_add_sect_vstr,
                       "<http-esc.vstr.sect", "p%p%u", ">"))
    errno = ENOMEM, err(EXIT_FAILURE, "init");

  if (!socket_poll_init(0, SOCKET_POLL_TYPE_MAP_DIRECT))
      errno = ENOMEM, err(EXIT_FAILURE, "init");
  
  evnt_logger(vlg);
  evnt_poll_init();
  evnt_timeout_init();

  vlg_time_set(vlg, evnt_sc_time);
  
  opt_serv_logger(vlg);

  httpd_init(vlg);
  
  opt_serv_sc_signals();
}

static int serv_cb_func_send(struct Evnt *evnt)
{
  struct Con *con = (struct Con *)evnt;
  
  assert(HTTPD_CONF_SEND_CALL_LIMIT >= 1);
  con->io_limit_num = HTTPD_CONF_SEND_CALL_LIMIT;
  return (httpd_serv_send(con));
}

static int serv_cb_func_recv(struct Evnt *evnt)
{
  struct Con *con = (struct Con *)evnt;
  
  assert(HTTPD_CONF_RECV_CALL_LIMIT >= 1);
  con->io_limit_num = HTTPD_CONF_RECV_CALL_LIMIT;
  return (httpd_serv_recv(con));
}

static void serv_cb_func_free(struct Evnt *evnt)
{
  struct Con *con = (struct Con *)evnt;
  
  httpd_fin_fd_close(con);

  opt_serv_sc_free_beg(evnt, con->acpt_sa_ref);

  ASSERT(con->fs && !con->fs_num && !con->fs_off && (con->fs->fd == -1));

  vstr_free_base(con->mpbr_ct);
  
  if (con->fs_sz > 1)
  {
    ASSERT(con->fs != con->fs_store);
    F(con->fs);
  }
  else
    ASSERT(con->fs == con->fs_store);

  F(con);
}

static struct Evnt *serv_cb_func_accept(struct Evnt *from_evnt, int fd,
                                        struct sockaddr *sa, socklen_t len)
{
  struct Acpt_listener *acpt_listener = (struct Acpt_listener *)from_evnt;
  struct Con *con = MK(sizeof(struct Con));

  if (sa->sa_family != AF_INET) /* only support IPv4 atm. */
    goto sa_fail;
  
  if (!con || !evnt_make_acpt_dup(con->evnt, fd, sa, len))
    goto make_acpt_fail;

  con->evnt->cbs->cb_func_recv       = serv_cb_func_recv;
  con->evnt->cbs->cb_func_send       = serv_cb_func_send;
  con->evnt->cbs->cb_func_free       = serv_cb_func_free;

  if (!httpd_con_init(con, acpt_listener))
    goto evnt_fail;

  if (!evnt_sc_timeout_via_mtime(con->evnt,
                                 con->policy->s->idle_timeout * 1000))
    VLG_WARNNOMEM_GOTO(evnt_fail, (vlg, "timeout: %m\n"));

  if (!opt_serv_sc_acpt_end(con->policy->s, from_evnt, con->evnt))
    goto evnt_fail;

  ASSERT(!con->evnt->flag_q_closed);

  return (con->evnt);
  
 evnt_fail:
  evnt_close(con->evnt);
  return (con->evnt);
  
 make_acpt_fail:
  F(con);
  VLG_WARNNOMEM_RET(NULL, (vlg, "%s: %m\n", "accept"));
  
 sa_fail:
  F(con);
  VLG_WARNNOMEM_RET(NULL, (vlg, "%s: HTTPD sa == ipv4 fail\n", "accept"));
}

static void serv_make_bind(const char *program_name)
{
  Opt_serv_addr_opts *addr = httpd_opts->s->addr_beg;
  
  while (addr)
  {
    const char *ipv4_address = NULL;
    const char *acpt_filter_file = NULL;
    struct Evnt *evnt = NULL;
    
    OPT_SC_EXPORT_CSTR(ipv4_address,     addr->ipv4_address,     FALSE,
                       "ipv4 address");
    OPT_SC_EXPORT_CSTR(acpt_filter_file, addr->acpt_filter_file, FALSE,
                       "accept filter file");
    
    evnt = evnt_sc_serv_make_bind(ipv4_address, addr->tcp_port,
                                  addr->q_listen_len,
                                  addr->max_connections,
                                  addr->defer_accept,
                                  acpt_filter_file);
    
    evnt->cbs->cb_func_accept = serv_cb_func_accept;

    addr = addr->next;
  }
}

#define VCMP_MT_EQ_ALL(x, y, z)               \
    vstr_cmp_eq(x -> mime_types_ ## z, 1, x -> mime_types_ ## z ->len,  \
                y -> mime_types_ ## z, 1, y -> mime_types_ ## z ->len)
static Httpd_policy_opts *serv__mime_types_eq(Httpd_policy_opts *node)
{ /* compares both mime filenames ... */
  Opt_serv_policy_opts *scan = httpd_opts->s->def_policy;

  ASSERT(node);
  
  while (scan != node->s)
  {
    Httpd_policy_opts *tmp = (Httpd_policy_opts *)scan;
    
    if (VCMP_MT_EQ_ALL(tmp, node, main) && VCMP_MT_EQ_ALL(tmp, node, xtra))
      return (tmp);
    
    scan = scan->next;
  }
  
  return (NULL);
}
#undef VCMP_MT_EQ_ALL

static void serv_mime_types(const char *program_name)
{
  Opt_serv_policy_opts *scan = httpd_opts->s->def_policy;
    
  while (scan)
  {
    Httpd_policy_opts *tmp = (Httpd_policy_opts *)scan;
    Httpd_policy_opts *prev = NULL;
    
    if (!mime_types_init(tmp->mime_types,
                         tmp->mime_types_def_ct, 1,
                         tmp->mime_types_def_ct->len))
      errno = ENOMEM, err(EXIT_FAILURE, "init");

    if ((prev = serv__mime_types_eq(tmp)))
      mime_types_combine_filedata(tmp->mime_types, prev->mime_types);
    else
    {
      const char *mime_types_main = NULL;
      const char *mime_types_xtra = NULL;
  
      OPT_SC_EXPORT_CSTR(mime_types_main, tmp->mime_types_main, FALSE,
                         "MIME types main file");
      OPT_SC_EXPORT_CSTR(mime_types_xtra, tmp->mime_types_xtra, FALSE,
                         "MIME types extra file");

      if (!mime_types_load_simple(tmp->mime_types, mime_types_main))
        err(EXIT_FAILURE, "load_mime(%s)", mime_types_main);
  
      if (!mime_types_load_simple(tmp->mime_types, mime_types_xtra))
        err(EXIT_FAILURE, "load_mime(%s)", mime_types_xtra);
    }
    
    scan = scan->next;
  }

  /* we don't need the filenames after we've loaded... */
  scan = httpd_opts->s->def_policy;
  while (scan)
  {
    Httpd_policy_opts *tmp = (Httpd_policy_opts *)scan;
    
    vstr_free_base(tmp->mime_types_main); tmp->mime_types_main = NULL;
    vstr_free_base(tmp->mime_types_xtra); tmp->mime_types_xtra = NULL;
    
    scan = scan->next;
  }
}

static void serv_canon_policies(void)
{
  Opt_serv_policy_opts *scan = httpd_opts->s->def_policy;
  
  while (scan)
  { /* check variables for things which will screw us too much */
    Httpd_policy_opts *tmp = (Httpd_policy_opts *)scan;
    Vstr_base *s1 = NULL;

    ASSERT(scan->beg == httpd_opts->s);
    
    s1 = tmp->document_root;
    if (!httpd_canon_dir_path(s1))
      VLG_ERRNOMEM((vlg, EXIT_FAILURE, "canon_dir_path($<vstr.all:%p>): %m\n",
                    s1));
  
    s1 = tmp->req_conf_dir;
    if (!httpd_canon_dir_path(s1))
      VLG_ERRNOMEM((vlg, EXIT_FAILURE, "canon_dir_path($<vstr.all:%p>): %m\n",
                    s1));

    s1 = tmp->req_err_dir;
    if (!httpd_canon_abs_dir_path(s1))
      VLG_ERRNOMEM((vlg, EXIT_FAILURE,
                    "canon_abs_dir_path($<vstr.all:%p>): %m\n", s1));

    if (!httpd_init_default_hostname(scan))
      VLG_ERRNOMEM((vlg, EXIT_FAILURE, "hostname(): %m\n"));
    
    s1 = tmp->dir_filename;
    if (!httpd_valid_url_filename(s1, 1, s1->len) &&
        !vstr_sub_cstr_ptr(s1, 1, s1->len, HTTPD_CONF_DEF_DIR_FILENAME))
      VLG_ERRNOMEM((vlg, EXIT_FAILURE, "dir_filename(): %m\n"));
    
    scan = scan->next;
  }
}

static void serv_cmd_line(int argc, char *argv[])
{
  int optchar = 0;
  const char *program_name = NULL;
  struct option long_options[] =
  {
   OPT_SERV_DECL_GETOPTS(),
   
   {"configuration-file", required_argument, NULL, 'C'},
   {"config-file",        required_argument, NULL, 'C'},
   {"configuration-data-daemon", required_argument, NULL, 143},
   {"config-data-daemon",        required_argument, NULL, 143},
   {"configuration-data-jhttpd", required_argument, NULL, 144},
   {"config-data-jhttpd",        required_argument, NULL, 144},
   
   {"sendfile", optional_argument, NULL, 31},
   {"mmap", optional_argument, NULL, 30},
   
   {"max-header-sz", required_argument, NULL, 128},
   {"keep-alive", optional_argument, NULL, 129},
   {"keep-alive-1.0", optional_argument, NULL, 130},
   {"vhosts", optional_argument, NULL, 131},
   {"virtual-hosts", optional_argument, NULL, 131},
   {"range", optional_argument, NULL, 132},
   {"range-1.0", optional_argument, NULL, 133},
   {"public-only", optional_argument, NULL, 134}, /* FIXME: rm ? */
   {"dir-filename", required_argument, NULL, 135},
   {"server-name", required_argument, NULL, 136},
   {"gzip-content-replacement", optional_argument, NULL, 137},
   /* 138 */
   {"error-406", optional_argument, NULL, 139},
   /* 140 */
   {"mime-types-main", required_argument, NULL, 141},
   {"mime-types-extra", required_argument, NULL, 142},
   {"mime-types-xtra", required_argument, NULL, 142},
   /* 143 -- config data above */
   /* 144 -- config data above */
   {"unspecified-hostname", required_argument, NULL, 145},
   {"canonize-host", optional_argument, NULL, 146},
   {"error-host-400", optional_argument, NULL, 147},
   {"check-host", optional_argument, NULL, 148},
   /* {"404-file", required_argument, NULL, 0}, */
   {NULL, 0, NULL, 0}
  };
  const char *chroot_dir = NULL;
  const char *document_root = NULL;
  Vstr_base *out = vstr_make_base(NULL);
  Httpd_policy_opts *popts = NULL;

  if (!out)
    errno = ENOMEM, err(EXIT_FAILURE, "command line");

  evnt_opt_nagle = TRUE;
  
  program_name = opt_program_name(argv[0], HTTPD_CONF_PROG_NAME);
  httpd_opts->s->name_cstr = program_name;
  httpd_opts->s->name_len  = strlen(program_name);

  httpd_opts->s->make_policy = httpd_policy_make;
  httpd_opts->s->copy_policy = httpd_policy_copy;

  if (!httpd_conf_main_init(httpd_opts))
    errno = ENOMEM, err(EXIT_FAILURE, "options");

  if (!geteuid()) /* If root */
    httpd_opts->s->addr_beg->tcp_port = HTTPD_CONF_SERV_DEF_PORT;
  else /* somewhat common unprived port */
    httpd_opts->s->addr_beg->tcp_port = 8008;
    
  popts = (Httpd_policy_opts *)httpd_opts->s->def_policy;
  while ((optchar = getopt_long(argc, argv, "C:dhH:M:nP:t:V",
                                long_options, NULL)) != -1)
  {
    switch (optchar)
    {
      case '?': usage(program_name, EXIT_FAILURE, "");
      case 'h': usage(program_name, EXIT_SUCCESS, "");
        
      case 'V':
        vstr_add_fmt(out, 0, " %s version %s.\n",
                     program_name, HTTPD_CONF_VERSION);
        
        if (io_put_all(out, STDOUT_FILENO) == IO_FAIL)
          err(EXIT_FAILURE, "write");
        
        exit (EXIT_SUCCESS);
      
      OPT_SERV_GETOPTS(httpd_opts->s);

      case 'C':
        if (!httpd_conf_main_parse_file(out, httpd_opts, optarg))
          goto out_err_conf_msg;
        break;
      case 143: /* FIXME: ... need to integrate */
        if (!opt_serv_conf_parse_cstr(out, httpd_opts->s, optarg))
          goto out_err_conf_msg;
      break;
      case 144:
        if (!httpd_conf_main_parse_cstr(out, httpd_opts, optarg))
          goto out_err_conf_msg;
        break;
        
      case 128: OPT_NUM_NR_ARG(popts->max_header_sz, "max header size"); break;
        
      case  31: OPT_TOGGLE_ARG(popts->use_sendfile);                 break;
      case  30: OPT_TOGGLE_ARG(popts->use_mmap);                     break;
        
      case 129: OPT_TOGGLE_ARG(popts->use_keep_alive);               break;
      case 130: OPT_TOGGLE_ARG(popts->use_keep_alive_1_0);           break;
      case 131: OPT_TOGGLE_ARG(popts->use_vhosts_name);              break;
      case 132: OPT_TOGGLE_ARG(popts->use_range);                    break;
      case 133: OPT_TOGGLE_ARG(popts->use_range_1_0);                break;
      case 134: OPT_TOGGLE_ARG(popts->use_public_only);              break;
      case 135: OPT_VSTR_ARG(popts->dir_filename);                   break;
      case 136: OPT_VSTR_ARG(popts->server_name);                    break;
      case 137: OPT_TOGGLE_ARG(popts->use_enc_content_replacement);  break;
      case 139: OPT_TOGGLE_ARG(popts->use_err_406);                  break;
        /* case 140: */
      case 141: OPT_VSTR_ARG(popts->mime_types_main);                break;
      case 142: OPT_VSTR_ARG(popts->mime_types_xtra);                break;
        /* case 143: */
        /* case 144: */
      case 145: OPT_VSTR_ARG(popts->default_hostname);               break;
      case 146: OPT_TOGGLE_ARG(popts->use_canonize_host);            break;
      case 147: OPT_TOGGLE_ARG(popts->use_host_err_400);             break;
      case 148: OPT_TOGGLE_ARG(popts->use_host_err_chk);
        
      
      ASSERT_NO_SWITCH_DEF();
    }
  }
  vstr_free_base(out); out = NULL;
  
  argc -= optind;
  argv += optind;

  if (argc > 1)
    usage(program_name, EXIT_FAILURE, " Too many arguments.\n");

  if (argc == 1)
  {
    Vstr_base *tmp = popts->document_root;
    vstr_sub_cstr_ptr(tmp, 1, tmp->len, argv[0]);
  }

  if (!popts->document_root->len)
    usage(program_name, EXIT_FAILURE, " Not specified a document root.\n");
  
  if (httpd_opts->s->become_daemon)
  {
    if (daemon(FALSE, FALSE) == -1)
      err(EXIT_FAILURE, "daemon");
    vlg_daemon(vlg, program_name);
  }

  if (httpd_opts->s->rlim_core_call)
    opt_serv_sc_rlim_core_num(httpd_opts->s->rlim_core_num);
  if (httpd_opts->s->rlim_file_call)
    opt_serv_sc_rlim_file_num(httpd_opts->s->rlim_file_num);
  
  {
    const char *pid_file = NULL;
    const char *cntl_file = NULL;
    Opt_serv_opts *opts = httpd_opts->s;
    
    OPT_SC_EXPORT_CSTR(pid_file,   opts->pid_file,   FALSE, "pid file");
    OPT_SC_EXPORT_CSTR(cntl_file,  opts->cntl_file,  FALSE, "control file");
    OPT_SC_EXPORT_CSTR(chroot_dir, opts->chroot_dir, FALSE, "chroot directory");
    
    serv_make_bind(program_name);

    if (!httpd_init_default_hostname(httpd_opts->s->def_policy))
      errno = ENOMEM, err(EXIT_FAILURE, "default_hostname");
  
    if (pid_file)
      vlg_pid_file(vlg, pid_file);

    if (cntl_file)
      cntl_make_file(vlg, cntl_file);

    serv_canon_policies();
  
    OPT_SC_EXPORT_CSTR(document_root, popts->document_root, TRUE,
                       "document root");
    
    serv_mime_types(program_name);
    
    if (chroot_dir)
      vlg_sc_bind_mount(chroot_dir);
  
    /* after daemon so syslog works */
    if (chroot_dir && ((chroot(chroot_dir) == -1) || (chdir("/") == -1)))
      vlg_err(vlg, EXIT_FAILURE, "chroot(%s): %m\n", chroot_dir);
    
    if (opts->drop_privs)
    {
      OPT_SC_RESOLVE_UID(opts);
      OPT_SC_RESOLVE_GID(opts);
      opt_serv_sc_drop_privs(opts);
    }

    if (opts->num_procs > 1)
      cntl_sc_multiproc(vlg, opts->num_procs, !!cntl_file, opts->use_pdeathsig);
  }

  httpd_opts->beg_time = time(NULL);
  
  /*  if (make_dumpable && (prctl(PR_SET_DUMPABLE, 1, 0, 0, 0) == -1))
   *    vlg_warn(vlg, "prctl(SET_DUMPABLE, TRUE): %m\n"); */

  {
    struct Evnt *evnt = evnt_queue("accept");
    
    while (evnt)
    {
      vlg_info(vlg, "READY [$<sa:%p>]: %s%s%s\n", EVNT_SA(evnt),
               chroot_dir ? chroot_dir : "",
               chroot_dir ?        "/" : "",
               document_root);
      evnt = evnt->next;
    }
  }
  
  opt_serv_conf_free(httpd_opts->s);
  return;

 out_err_conf_msg:
  vstr_add_cstr_ptr(out, out->len, "\n");
  if (io_put_all(out, STDERR_FILENO) == IO_FAIL)
    err(EXIT_FAILURE, "write");
  exit (EXIT_FAILURE);  
}

int main(int argc, char *argv[])
{
  serv_init();

  serv_cmd_line(argc, argv);

  while (evnt_waiting())
  {
    evnt_sc_main_loop(HTTPD_CONF_MAX_WAIT_SEND);
    opt_serv_sc_check_children();
    opt_serv_sc_cntl_resources(httpd_opts->s);
  }
  evnt_out_dbg3("E");
  
  evnt_timeout_exit();
  cntl_child_free();
  evnt_close_all();

  httpd_exit();
  
  http_req_exit();
  
  vlg_free(vlg);
  vlg_exit();

  httpd_conf_main_free(httpd_opts);
  
  vstr_exit();

  MALLOC_CHECK_EMPTY();
  
  exit (EXIT_SUCCESS);
}
