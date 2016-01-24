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
/* Vectored logging APIs */

#define CONF_USE_HEXDUMP TRUE

#define _GNU_SOURCE 1

#include <vstr.h>

#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <err.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>

#include <limits.h>

#include <signal.h>


#define VLG_COMPILE_INLINE 0
#include "vlg.h"

/* FIXME: could possibly work on other OSes ? */
#ifdef __linux__
# include <sys/mount.h>
#endif

#ifdef MS_BIND
# define CONF_USE_MOUNT_BIND TRUE
# define BIND_MOUNT(x, y) mount(x, y, "", MS_BIND, "")
#else
# define BIND_MOUNT(x, y) -1 /* do nothing */
# define CONF_USE_MOUNT_BIND FALSE
#endif

#define CONF_USE_INTERNAL_SYSLOG TRUE

#define EX_UTILS_NO_FUNCS 1
#include "ex_utils.h"

/* how much memory should we preallocate so it's "unlikely" we'll get mem errors
 * when writting a log entry */
#define VLG_MEM_PREALLOC (4 * 1024)

#ifndef FALSE
# define FALSE 0
#endif

#ifndef TRUE
# define TRUE 1
#endif

#if CONF_USE_HEXDUMP
# include "hexdump.h"
#else
# define ex_hexdump_reset() /* do nothing */
# define ex_hexdump_process(x1, x2, x3, x4, x5, x6, x7, x8, x9) FALSE
#endif

static Vstr_conf *vlg__conf = NULL;
static Vstr_conf *vlg__sig_conf = NULL;
static int vlg__done_syslog_init = FALSE;


static int vlg__syslog_con(Vlg *vlg, int alt)
{
  int type = alt ? SOCK_STREAM : SOCK_DGRAM;
  int fd = -1;
  const char *fname = _PATH_LOG;
  size_t len = strlen(fname) + 1;
  struct sockaddr_un tmp_sun;
  struct sockaddr *sa = (struct sockaddr *)&tmp_sun;
  socklen_t alloc_len = 0;

  if (!CONF_USE_INTERNAL_SYSLOG)
    goto conf_fail;
  if (vlg->syslog_fd != -1)
    return (TRUE);
  
  tmp_sun.sun_path[0] = 0;
  alloc_len = SUN_LEN(&tmp_sun) + len;
  tmp_sun.sun_family = AF_LOCAL;
  memcpy(tmp_sun.sun_path, fname, len);

  if (vlg->syslog_stream)
    type = alt ? SOCK_DGRAM : SOCK_STREAM;

  if ((fd = socket(PF_LOCAL, type, 0)) == -1)
    goto sock_fail;

  if (fcntl(fd, F_SETFD, TRUE) == -1)
    goto fcntl_fail;
  
  if (connect(fd, sa, alloc_len) == -1)
    goto connect_fail;

  vlg->syslog_fd = fd;
  
  return (TRUE);
  
 connect_fail:
  if (!alt)
    return (vlg__syslog_con(vlg, TRUE));
  
 fcntl_fail:
  close(fd);
 sock_fail:
 conf_fail:
  if (!vlg__done_syslog_init)
    openlog(vlg->prog_name, LOG_PID | LOG_NDELAY, LOG_DAEMON);
  vlg__done_syslog_init = TRUE;
  return (FALSE);
}

static void vlg__syslog_close(Vlg *vlg)
{
  if (vlg->syslog_fd != -1)
    close(vlg->syslog_fd);
  vlg->syslog_fd = -1;
}

static void vlg__flush(Vlg *vlg, int type, int out_err)
{
  Vstr_base *dlg = vlg->out_vstr;
  time_t now = (*vlg->tm_get)();
  
  ASSERT(vstr_export_chr(dlg, dlg->len) == '\n');

  if (now != vlg->tm_time)
  {
    vlg->tm_time = now;
    date_syslog(vlg->tm_time, vlg->tm_data, sizeof(vlg->tm_data));
  }
  
  if (vlg->daemon_mode)
  {
    if (!vlg__syslog_con(vlg, 0))
    { /* ignoring borken syslog()'s that overflow here use a real OS */
      const char *tmp = vstr_export_cstr_ptr(dlg, 1, dlg->len - 1);
      
      if (!tmp)
        errno = ENOMEM, err(EXIT_FAILURE, "vlog__flush");
      
      syslog(type, "%s", tmp);
    }
    else
    {
      pid_t pid = getpid();
      int fd = vlg->syslog_fd;
      size_t beg_len = 0;

      vstr_add_fmt(dlg, 0, "<%u>%s %s[%lu]: ", type | LOG_DAEMON,
                   vlg->tm_data, vlg->prog_name, (unsigned long)pid);
      
      if (vlg->syslog_stream)
        vstr_sub_buf(dlg, dlg->len, 1, "", 1);
      else
        vstr_sc_reduce(dlg, 1, dlg->len, 1); /* remove "\n" */
      
      if (dlg->conf->malloc_bad)
        errno = ENOMEM, err(EXIT_FAILURE, "vlog__flush");

      beg_len = dlg->len;
      while (dlg->len)
        if (!vstr_sc_write_fd(dlg, 1, dlg->len, fd, NULL) && (errno != EAGAIN))
        {
          vlg__syslog_close(vlg);
          if (beg_len != dlg->len) /* we have sent _some_ data, and it died */
            break;
          if (!vlg__syslog_con(vlg, 0))
            err(EXIT_FAILURE, "vlg__syslog_con");
        }
    }
    
    vstr_del(dlg, 1, dlg->len);
  }
  else
  {
    int fd = out_err ? STDERR_FILENO : STDOUT_FILENO;
    
    if (vlg->log_prefix_console)
    {
      /* Note: we add the begining backwards, it's easier that way */
      if ((type == LOG_WARNING) && !vstr_add_cstr_ptr(dlg, 0, "WARN: "))
        errno = ENOMEM, err(EXIT_FAILURE, "warn");
      if ((type == LOG_ALERT) && !vstr_add_cstr_ptr(dlg, 0, "ERR: "))
        errno = ENOMEM, err(EXIT_FAILURE, "err");
      if ((type == LOG_DEBUG) && !vstr_add_cstr_ptr(dlg, 0, "DEBUG: "))
        errno = ENOMEM, err(EXIT_FAILURE, "vlog_vdbg");
      
      if (!vlg->log_pid)
      {
        if (!vstr_add_cstr_ptr(dlg, 0, "]: "))
          errno = ENOMEM, err(EXIT_FAILURE, "prefix");
      }
      else
      {
        pid_t pid = getpid();
        
        if (!vstr_add_fmt(dlg, 0, "] %lu: ", (unsigned long)pid))
          errno = ENOMEM, err(EXIT_FAILURE, "prefix");
      }
      
      if (!vstr_add_cstr_ptr(dlg, 0, vlg->tm_data) ||
          !vstr_add_cstr_ptr(dlg, 0, "["))
        errno = ENOMEM, err(EXIT_FAILURE, "prefix");      
    }
    
    while (dlg->len)
      if (!vstr_sc_write_fd(dlg, 1, dlg->len, fd, NULL) && (errno != EAGAIN))
        err(EXIT_FAILURE, "vlg__flush");
  }
}

static void vlg__add_chk_flush(Vlg *vlg, const char *fmt, va_list ap,
                               int type, int out_err)
{
  Vstr_base *dlg = vlg->out_vstr;

  if (!vstr_add_vfmt(dlg, dlg->len, fmt, ap))
  {
    if (dlg->conf->malloc_bad)
      errno = ENOMEM, err(EXIT_FAILURE, "chk_flush");
    return;
  }
  
  if (vstr_export_chr(dlg, dlg->len) == '\n')
    vlg__flush(vlg, type, out_err);
}


/* because vlg goes away quickly it's likely we'll want to just use _BUF_PTR
   for Vstr data to save the copying. So here is a helper. */
static int vlg__fmt__add_vstr_add_vstr(Vstr_base *base, size_t pos,
                                       Vstr_fmt_spec *spec)
{
  Vstr_base *sf          = VSTR_FMT_CB_ARG_PTR(spec, 0);
  size_t sf_pos          = VSTR_FMT_CB_ARG_VAL(spec, size_t, 1);
  size_t sf_len          = VSTR_FMT_CB_ARG_VAL(spec, size_t, 2);
  unsigned int sf_flags  = VSTR_TYPE_ADD_BUF_PTR;
  
  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &sf_len,
                          VSTR_FLAG_SC_FMT_CB_BEG_OBJ_STR))
    return (FALSE);
  
  if (!vstr_add_vstr(base, pos, sf, sf_pos, sf_len, sf_flags))
    return (FALSE);
  
  if (!vstr_sc_fmt_cb_end(base, pos, spec, sf_len))
    return (FALSE);
  
  return (TRUE);
}

static int vlg__fmt_add_vstr_add_vstr(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vlg__fmt__add_vstr_add_vstr,
                       VSTR_TYPE_FMT_PTR_VOID,
                       VSTR_TYPE_FMT_SIZE_T,
                       VSTR_TYPE_FMT_SIZE_T,
                       VSTR_TYPE_FMT_END));
}

/* because you have to do (size_t)1 for varargs it's annoying when you want
   the entire Vstr ... helper */
static int vlg__fmt__add_vstr_add_all_vstr(Vstr_base *base, size_t pos,
                                           Vstr_fmt_spec *spec)
{
  Vstr_base *sf          = VSTR_FMT_CB_ARG_PTR(spec, 0);
  size_t sf_pos          = 1;
  size_t sf_len          = sf->len;
  unsigned int sf_flags  = VSTR_TYPE_ADD_BUF_PTR;
  
  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &sf_len,
                          VSTR_FLAG_SC_FMT_CB_BEG_OBJ_STR))
    return (FALSE);
  
  if (!vstr_add_vstr(base, pos, sf, sf_pos, sf_len, sf_flags))
    return (FALSE);
  
  if (!vstr_sc_fmt_cb_end(base, pos, spec, sf_len))
    return (FALSE);
  
  return (TRUE);
}

static int vlg__fmt_add_vstr_add_all_vstr(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vlg__fmt__add_vstr_add_all_vstr,
                       VSTR_TYPE_FMT_PTR_VOID,
                       VSTR_TYPE_FMT_END));
}

/* also a helper for printing sects --
 * should probably have some in Vstr itself */
static int vlg__fmt__add_vstr_add_sect_vstr(Vstr_base *base, size_t pos,
                                            Vstr_fmt_spec *spec)
{
  Vstr_base *sf          = VSTR_FMT_CB_ARG_PTR(spec, 0);
  Vstr_sects *sects      = VSTR_FMT_CB_ARG_PTR(spec, 1);
  unsigned int num       = VSTR_FMT_CB_ARG_VAL(spec, unsigned int, 2);
  size_t sf_pos          = VSTR_SECTS_NUM(sects, num)->pos;
  size_t sf_len          = VSTR_SECTS_NUM(sects, num)->len;
  unsigned int sf_flags  = VSTR_TYPE_ADD_BUF_PTR;
  
  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &sf_len,
                          VSTR_FLAG_SC_FMT_CB_BEG_OBJ_STR))
    return (FALSE);
  
  if (!vstr_add_vstr(base, pos, sf, sf_pos, sf_len, sf_flags))
    return (FALSE);
  
  if (!vstr_sc_fmt_cb_end(base, pos, spec, sf_len))
    return (FALSE);
  
  return (TRUE);
}

static int vlg__fmt_add_vstr_add_sect_vstr(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vlg__fmt__add_vstr_add_sect_vstr,
                       VSTR_TYPE_FMT_PTR_VOID,
                       VSTR_TYPE_FMT_PTR_VOID,
                       VSTR_TYPE_FMT_UINT,
                       VSTR_TYPE_FMT_END));
}

static int vlg__fmt__add_vstr_add_hexdump_vstr(Vstr_base *base, size_t pos,
                                               Vstr_fmt_spec *spec)
{
  Vstr_base *sf          = VSTR_FMT_CB_ARG_PTR(spec, 0);
  size_t sf_pos          = VSTR_FMT_CB_ARG_VAL(spec, size_t, 1);
  size_t sf_len          = VSTR_FMT_CB_ARG_VAL(spec, size_t, 2);
  size_t orig_len = base->len;
  
  ex_hexdump_reset();
  ex_hexdump_process(base, pos, sf, sf_pos, sf_len, PRNT_NONE,
                     UINT_MAX, FALSE, TRUE);
  if (base->conf->malloc_bad)
    return (FALSE);

  sf_len = base->len - orig_len;
  
  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &sf_len,
                          VSTR_FLAG_SC_FMT_CB_BEG_OBJ_ATOM))
    return (FALSE);
  
  if (!vstr_sc_fmt_cb_end(base, pos, spec, sf_len))
    return (FALSE);
  
  return (TRUE);
}

static int vlg__fmt_add_vstr_add_hexdump_vstr(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vlg__fmt__add_vstr_add_hexdump_vstr,
                       VSTR_TYPE_FMT_PTR_VOID,
                       VSTR_TYPE_FMT_SIZE_T,
                       VSTR_TYPE_FMT_SIZE_T,
                       VSTR_TYPE_FMT_END));
}

/* also a helper for printing any network address */
static int vlg__fmt__add_vstr_add_sa(Vstr_base *base, size_t pos,
                                     Vstr_fmt_spec *spec)
{
  struct sockaddr *sa = VSTR_FMT_CB_ARG_PTR(spec, 0);
  size_t obj_len = 0;
  char buf1[128 + 1];
  char buf2[sizeof(short) * CHAR_BIT + 1];
  const char *ptr1 = NULL;
  size_t len1 = 0;
  const char *ptr2 = NULL;
  size_t len2 = 0;

  assert(sizeof(buf1) >= INET_ADDRSTRLEN);
  assert(sizeof(buf1) >= INET6_ADDRSTRLEN);

  if (!sa)
  {
    ptr1 = "<none>";
    len1 = strlen(ptr1);
  }
  else
  switch (sa->sa_family)
  {
    case AF_INET:
    {
      struct sockaddr_in *sin4 = (void *)sa;
      ptr1 = inet_ntop(AF_INET, &sin4->sin_addr, buf1, sizeof(buf1));
      if (!ptr1) ptr1 = "<unknown>";
      len1 = strlen(ptr1);
      ptr2 = buf2;
      len2 = vstr_sc_conv_num10_uint(buf2, sizeof(buf2), ntohs(sin4->sin_port));
    }
    break;
      
    case AF_INET6:
    {
      struct sockaddr_in6 *sin6 = (void *)sa;
      ptr1 = inet_ntop(AF_INET6, &sin6->sin6_addr, buf1, sizeof(buf1));
      if (!ptr1) ptr1 = "<unknown>";
      len1 = strlen(ptr1);
      ptr2 = buf2;
      len2 = vstr_sc_conv_num10_uint(buf2,sizeof(buf2), ntohs(sin6->sin6_port));
    }
    break;
    
    case AF_LOCAL:
    { /* struct sockaddr_un *sun = (void *)sa; */
      struct sockaddr_un *sun = (void *)sa;
      ptr1 = "local";
      len1 = strlen(ptr1);
      ptr2 = sun->sun_path;
      len2 = strlen(ptr2);
    }
    break;
    
    default: ASSERT_NOT_REACHED();
  }

  obj_len = len1 + !!len2 + len2;
  
  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &obj_len,
                          VSTR_FLAG_SC_FMT_CB_BEG_OBJ_ATOM))
    return (FALSE);
  ASSERT(obj_len == (len1 + !!len2 + len2));
  
  if (!vstr_add_buf(base, pos, ptr1, len1))
    return (FALSE);
  if (ptr2 && (!vstr_add_rep_chr(base, pos + len1, '@', 1) ||
               !vstr_add_buf(    base, pos + len1 + 1, ptr2, len2)))
    return (FALSE);
                                                                                
  if (!vstr_sc_fmt_cb_end(base, pos, spec, obj_len))
    return (FALSE);
                                                                                
  return (TRUE);
}

static int vlg__fmt_add_vstr_add_sa(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vlg__fmt__add_vstr_add_sa,
                       VSTR_TYPE_FMT_PTR_VOID,
                       VSTR_TYPE_FMT_END));
}

int vlg_sc_fmt_add_all(Vstr_conf *conf)
{
  return (VSTR_SC_FMT_ADD(conf, vlg__fmt_add_vstr_add_vstr,
                          "<vstr", "p%zu%zu", ">") &&
          VSTR_SC_FMT_ADD(conf, vlg__fmt_add_vstr_add_all_vstr,
                          "<vstr.all", "p", ">") &&
          VSTR_SC_FMT_ADD(conf, vlg__fmt_add_vstr_add_hexdump_vstr,
                          "<vstr.hexdump", "p%zu%zu", ">") &&
          VSTR_SC_FMT_ADD(conf, vlg__fmt_add_vstr_add_sect_vstr,
                          "<vstr.sect", "p%p%u", ">") &&
          VSTR_SC_FMT_ADD(conf, vlg__fmt_add_vstr_add_sa,
                          "<sa", "p", ">"));
}

void vlg_sc_bind_mount(const char *chroot_dir)
{ /* make sure we can reconnect to syslog */
  Vstr_base *tmp = NULL;
  const char *src = "/dev/log";
  const char *dst = NULL;
  struct stat64 st_src[1];
  struct stat64 st_dst[1];
  
  if (!CONF_USE_MOUNT_BIND || !chroot_dir)
    return;
  
  if (!(tmp = vstr_make_base(NULL)))
    errno = ENOMEM, err(EXIT_FAILURE, "bind-mount");
    
  vstr_add_fmt(tmp, 0, "%s%s", chroot_dir, "/dev/log");
  dst = vstr_export_cstr_ptr(tmp, 1, tmp->len);
  if (tmp->conf->malloc_bad)
    errno = ENOMEM, err(EXIT_FAILURE, "bind-mount");
  
  if (stat64(src, st_src) == -1)
    err(EXIT_FAILURE, "stat(%s)", src);
  if (stat64(dst, st_dst) == -1)
    err(EXIT_FAILURE, "stat(%s)", dst);
  
  if ((st_src->st_ino != st_dst->st_ino) ||
      (st_src->st_dev != st_dst->st_dev))
  {
    umount(dst); /* NOTE: You can't bind mount over a bind mount,
                  * so if syslog is restarted we need to try this */
    if (BIND_MOUNT(src, dst) == -1)
      err(EXIT_FAILURE, "bind-mount(%s, %s)", src, dst);
  }
    
  vstr_free_base(tmp);
}

void vlg_init(void)
{
  unsigned int buf_sz = 0;
  
  if (!(vlg__conf     = vstr_make_conf()))
    goto malloc_err_vstr_conf;
  if (!(vlg__sig_conf = vstr_make_conf()))
    goto malloc_err_vstr_conf;

  if (!vstr_cntl_conf(vlg__conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$') ||
      !vstr_sc_fmt_add_all(vlg__conf) ||
      !vlg_sc_fmt_add_all(vlg__conf) ||
      FALSE)
    goto malloc_err_vstr_fmt_all;
  if (!vstr_cntl_conf(vlg__sig_conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$') ||
      !vstr_sc_fmt_add_all(vlg__sig_conf) ||
      !vlg_sc_fmt_add_all(vlg__sig_conf) ||
      FALSE)
    goto malloc_err_vstr_fmt_all;

  vstr_cntl_conf(vlg__conf,     VSTR_CNTL_CONF_GET_NUM_BUF_SZ, &buf_sz);
  vstr_cntl_conf(vlg__sig_conf, VSTR_CNTL_CONF_GET_NUM_BUF_SZ, &buf_sz);

  /* don't bother with _NON nodes */
  if (!vstr_make_spare_nodes(vlg__conf, VSTR_TYPE_NODE_BUF,
                             (VLG_MEM_PREALLOC / buf_sz) + 1) ||
      !vstr_make_spare_nodes(vlg__conf, VSTR_TYPE_NODE_PTR,
                             (VLG_MEM_PREALLOC / buf_sz) + 1) ||
      !vstr_make_spare_nodes(vlg__conf, VSTR_TYPE_NODE_REF,
                             (VLG_MEM_PREALLOC / buf_sz) + 1))
    goto malloc_err_vstr_spare;
  if (!vstr_make_spare_nodes(vlg__sig_conf, VSTR_TYPE_NODE_BUF,
                             (VLG_MEM_PREALLOC / buf_sz) + 1) ||
      !vstr_make_spare_nodes(vlg__sig_conf, VSTR_TYPE_NODE_PTR,
                             (VLG_MEM_PREALLOC / buf_sz) + 1) ||
      !vstr_make_spare_nodes(vlg__sig_conf, VSTR_TYPE_NODE_REF,
                             (VLG_MEM_PREALLOC / buf_sz) + 1))
    goto malloc_err_vstr_spare;

  return;
  
 malloc_err_vstr_spare:
 malloc_err_vstr_fmt_all:
  vstr_free_conf(vlg__conf);
  vstr_free_conf(vlg__sig_conf);
 malloc_err_vstr_conf:
  errno = ENOMEM; err(EXIT_FAILURE, "vlg_init");
}

void vlg_exit(void)
{
  if (vlg__done_syslog_init)
    closelog();
  
  vstr_free_conf(vlg__conf);     vlg__conf     = NULL;
  vstr_free_conf(vlg__sig_conf); vlg__sig_conf = NULL;
}

static time_t vlg__tm_get(void)
{
  return time(NULL);
}

Vlg *vlg_make(void)
{
  Vlg *vlg = malloc(sizeof(Vlg));

  if (!vlg)
    goto malloc_err_vlg;

  if (!(vlg->out_vstr = vstr_make_base(vlg__conf)))
    goto malloc_err_vstr_base;
  
  if (!(vlg->sig_out_vstr = vstr_make_base(vlg__sig_conf)))
    goto malloc_err_vstr_base;
  
  vlg->prog_name          = NULL;
  vlg->syslog_fd          = -1;
  
  vlg->tm_time            = -1;
  vlg->tm_get             = vlg__tm_get;
  
  vlg->syslog_stream      = FALSE;
  vlg->log_pid            = FALSE;
  vlg->out_dbg            = 0;
  vlg->daemon_mode        = FALSE;
  vlg->log_prefix_console = TRUE;
  
  return (vlg);

 malloc_err_vstr_base:
  free(vlg);
 malloc_err_vlg:
  
  return (NULL);
}

/* don't actually free ... this shouldn't happen until exit time anyway */
void vlg_free(Vlg *vlg)
{
  vstr_free_base(vlg->out_vstr);     vlg->out_vstr     = NULL;
  vstr_free_base(vlg->sig_out_vstr); vlg->sig_out_vstr = NULL;
}

void vlg_daemon(Vlg *vlg, const char *name)
{
  ASSERT(name);

  vlg->prog_name   = name;
  vlg->log_pid     = TRUE;
  vlg->daemon_mode = TRUE;
  
  vlg__syslog_con(vlg, 0);
}

void vlg_debug(Vlg *vlg)
{
  if (vlg->out_dbg >= 3)
    return;

  ++vlg->out_dbg;
}

void vlg_undbg(Vlg *vlg)
{
  if (!vlg->out_dbg)
    return;

  --vlg->out_dbg;
}

int vlg_pid_set(Vlg *vlg, int pid)
{
  int old = vlg->log_pid;

  vlg->log_pid = !!pid;

  return (old);
}

int vlg_prefix_set(Vlg *vlg, int prefix)
{
  int old = vlg->log_prefix_console;

  vlg->log_prefix_console = prefix;

  return (old);
}

void vlg_time_set(Vlg *vlg, time_t (*func)(void))
{
  vlg->tm_get = func;
}

void vlg_pid_file(Vlg *vlg, const char *pid_file)
{
  Vstr_base *out = vlg->out_vstr;
  
  if (out->len)
    vlg_err(vlg, EXIT_FAILURE, "Data in vlg for pid_file\n");
  
  if (!vstr_add_fmt(out, out->len, "%lu", (unsigned long)getpid()))
  {
    vstr_del(out, 1, out->len);
    vlg_err(vlg, EXIT_FAILURE, "vlg_pid_file: %m\n");
  }

  if (!vstr_sc_write_file(out, 1, out->len,
                          pid_file, O_WRONLY | O_CREAT | O_TRUNC, 0644, 0,NULL))
  {
    vstr_del(out, 1, out->len);
    vlg_err(vlg, EXIT_FAILURE, "vlg_pid_file(%s): %m\n", pid_file);
  }
}

/* ================== actual logging functions ================== */

/* ---------- va_list ---------- */
void vlg_vabort(Vlg *vlg, const char *fmt, va_list ap)
{
  vlg__add_chk_flush(vlg, fmt, ap, LOG_ALERT, TRUE);
  abort();
}

void vlg_verr(Vlg *vlg, int exit_code, const char *fmt, va_list ap)
{
  vlg__add_chk_flush(vlg, fmt, ap, LOG_ALERT, TRUE);
  _exit(exit_code);
}

void vlg_vwarn(Vlg *vlg, const char *fmt, va_list ap)
{
  vlg__add_chk_flush(vlg, fmt, ap, LOG_WARNING, TRUE);
}

void vlg_vinfo(Vlg *vlg, const char *fmt, va_list ap)
{
  vlg__add_chk_flush(vlg, fmt, ap, LOG_NOTICE, FALSE);
}

void vlg_vdbg1(Vlg *vlg, const char *fmt, va_list ap)
{
  if (vlg->out_dbg < 1)
    return;

  vlg__add_chk_flush(vlg, fmt, ap, LOG_DEBUG, TRUE);
}

void vlg_vdbg2(Vlg *vlg, const char *fmt, va_list ap)
{
  if (vlg->out_dbg < 2)
    return;

  vlg__add_chk_flush(vlg, fmt, ap, LOG_DEBUG, TRUE);
}

void vlg_vdbg3(Vlg *vlg, const char *fmt, va_list ap)
{
  if (vlg->out_dbg < 3)
    return;

  vlg__add_chk_flush(vlg, fmt, ap, LOG_DEBUG, TRUE);
}

/* ---------- ... ---------- */
void vlg_abort(Vlg *vlg, const char *fmt, ... )
{
  va_list ap;

  va_start(ap, fmt);
  vlg_vabort(vlg, fmt, ap);
  va_end(ap);
  
  ASSERT_NOT_REACHED();
}

void vlg_err(Vlg *vlg, int exit_code, const char *fmt, ... )
{
  va_list ap;

  va_start(ap, fmt);
  vlg_verr(vlg, exit_code, fmt, ap);
  va_end(ap);
  
  ASSERT_NOT_REACHED();
}

void vlg_warn(Vlg *vlg, const char *fmt, ... )
{
  va_list ap;

  va_start(ap, fmt);
  vlg_vwarn(vlg, fmt, ap);
  va_end(ap);
}

void vlg_info(Vlg *vlg, const char *fmt, ... )
{
  va_list ap;

  va_start(ap, fmt);
  vlg_vinfo(vlg, fmt, ap);
  va_end(ap);
}

void vlg_dbg1(Vlg *vlg, const char *fmt, ... )
{
  va_list ap;

  va_start(ap, fmt);
  vlg_vdbg1(vlg, fmt, ap);
  va_end(ap);
}

void vlg_dbg2(Vlg *vlg, const char *fmt, ... )
{
  va_list ap;

  va_start(ap, fmt);
  vlg_vdbg2(vlg, fmt, ap);
  va_end(ap);
}

void vlg_dbg3(Vlg *vlg, const char *fmt, ... )
{
  va_list ap;

  va_start(ap, fmt);
  vlg_vdbg3(vlg, fmt, ap);
  va_end(ap);
}

/* ---------- signal ... ---------- */

static volatile sig_atomic_t vlg__in_signal = FALSE;

/* due to multiple signals hitting while we are inside vlg_*() we have
   signal safe varients, that:
   
   block all signals (apart from SEGV and ABRT)
   do their thing
   make sure they have flushed
   restore signal mask
   
   ...note that this does mean if we've crashed inside vlg, we are screwed
   and just abort().
*/

#define VLG__SIG_BLOCK_BEG() do {                                       \
      sigset_t oset;                                                    \
      sigset_t nset;                                                    \
                                                                        \
      if (sigfillset(&nset)                      == -1) abort();        \
      if (sigdelset(&nset, SIGSEGV)              == -1) abort();        \
      if (sigdelset(&nset, SIGABRT)              == -1) abort();        \
      if (sigprocmask(SIG_SETMASK, &nset, &oset) == -1) abort();        \
                                                                        \
      if (vlg__in_signal) abort();                                      \
      else {                                                            \
        Vstr_base *tmp = vlg->out_vstr;                                 \
                                                                        \
        vlg__in_signal = TRUE;                                          \
        vlg->out_vstr     = vlg->sig_out_vstr;                          \
        vlg->sig_out_vstr = NULL;                                       \
        if (vlg->out_vstr->len) abort()

#define VLG__SIG_BLOCK_END()                                            \
        if (vlg->out_vstr->len) abort();                                \
        vlg->sig_out_vstr = vlg->out_vstr;                              \
        vlg->out_vstr     = tmp;                                        \
        vlg__in_signal = FALSE;                                         \
      }                                                                 \
                                                                        \
      if (sigprocmask(SIG_SETMASK, &oset, NULL) == -1) abort();         \
    } while (FALSE)

void vlg_sig_abort(Vlg *vlg, const char *fmt, ... )
{
  va_list ap;

  VLG__SIG_BLOCK_BEG();
  
  va_start(ap, fmt);
  vlg_vabort(vlg, fmt, ap);
  va_end(ap);
  
  VLG__SIG_BLOCK_END();
  
  ASSERT_NOT_REACHED();
}

void vlg_sig_err(Vlg *vlg, int exit_code, const char *fmt, ... )
{
  va_list ap;

  VLG__SIG_BLOCK_BEG();
  
  va_start(ap, fmt);
  vlg_verr(vlg, exit_code, fmt, ap);
  va_end(ap);

  VLG__SIG_BLOCK_END();
  
  ASSERT_NOT_REACHED();
}

void vlg_sig_warn(Vlg *vlg, const char *fmt, ... )
{
  va_list ap;

  VLG__SIG_BLOCK_BEG();
  
  va_start(ap, fmt);
  vlg_vwarn(vlg, fmt, ap);
  va_end(ap);

  VLG__SIG_BLOCK_END();
}

void vlg_sig_info(Vlg *vlg, const char *fmt, ... )
{
  va_list ap;

  VLG__SIG_BLOCK_BEG();
  
  va_start(ap, fmt);
  vlg_vinfo(vlg, fmt, ap);
  va_end(ap);

  VLG__SIG_BLOCK_END();
}

void vlg_sig_dbg1(Vlg *vlg, const char *fmt, ... )
{
  va_list ap;

  VLG__SIG_BLOCK_BEG();
  
  va_start(ap, fmt);
  vlg_vdbg1(vlg, fmt, ap);
  va_end(ap);

  VLG__SIG_BLOCK_END();
}

void vlg_sig_dbg2(Vlg *vlg, const char *fmt, ... )
{
  va_list ap;

  VLG__SIG_BLOCK_BEG();
  
  va_start(ap, fmt);
  vlg_vdbg2(vlg, fmt, ap);
  va_end(ap);

  VLG__SIG_BLOCK_END();
}

void vlg_sig_dbg3(Vlg *vlg, const char *fmt, ... )
{
  va_list ap;

  VLG__SIG_BLOCK_BEG();
  
  va_start(ap, fmt);
  vlg_vdbg3(vlg, fmt, ap);
  va_end(ap);

  VLG__SIG_BLOCK_END();
}

