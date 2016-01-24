#ifndef VLG_H
#define VLG_H 1

#include <vstr.h>
#include <stdarg.h>
#include <errno.h>

#include "date.h"

#define VLG__TM_SYSLOG_SZ 128

typedef struct Vlg
{
 struct Vstr_base *out_vstr;
 struct Vstr_base *sig_out_vstr;
 const char *prog_name;
 int syslog_fd;

 time_t tm_time;
 char tm_data[VLG__TM_SYSLOG_SZ];

 time_t (*tm_get)(void);
 
 unsigned int syslog_stream : 1;
 unsigned int log_pid : 1;
 unsigned int out_dbg : 2;
 unsigned int daemon_mode : 1;
 unsigned int log_prefix_console : 1;
} Vlg;

extern void vlg_init(void);
extern void vlg_exit(void);

extern Vlg *vlg_make(void);
extern void vlg_free(Vlg *);

extern void vlg_daemon(Vlg *, const char *);
extern void vlg_debug(Vlg *);
extern void vlg_undbg(Vlg *);

extern int  vlg_pid_set(Vlg *, int);
extern int  vlg_prefix_set(Vlg *, int);
extern void vlg_time_set(Vlg *vlg, time_t (*func)(void));

extern void vlg_pid_file(Vlg *, const char *);

extern int  vlg_sc_fmt_add_all(Vstr_conf *);
extern void vlg_sc_bind_mount(const char *);

extern void vlg_vabort(Vlg *, const char *fmt, va_list )
   VSTR__COMPILE_ATTR_FMT(2, 0);
extern void vlg_verr(Vlg *, int, const char *fmt, va_list )
   VSTR__COMPILE_ATTR_FMT(3, 0);
extern void vlg_vwarn(Vlg *, const char *fmt, va_list )
   VSTR__COMPILE_ATTR_FMT(2, 0);
extern void vlg_vinfo(Vlg *, const char *fmt, va_list )
   VSTR__COMPILE_ATTR_FMT(2, 0);
extern void vlg_vdbg1(Vlg *, const char *fmt, va_list )
   VSTR__COMPILE_ATTR_FMT(2, 0);
extern void vlg_vdbg2(Vlg *, const char *fmt, va_list )
   VSTR__COMPILE_ATTR_FMT(2, 0);
extern void vlg_vdbg3(Vlg *, const char *fmt, va_list )
   VSTR__COMPILE_ATTR_FMT(2, 0);

extern void vlg_abort(Vlg *, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(2, 3);
extern void vlg_err(Vlg *, int, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(3, 4);
extern void vlg_warn(Vlg *, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(2, 3);
extern void vlg_info(Vlg *, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(2, 3);
extern void vlg_dbg1(Vlg *, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(2, 3);
extern void vlg_dbg2(Vlg *, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(2, 3);
extern void vlg_dbg3(Vlg *, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(2, 3);

extern void vlg_sig_abort(Vlg *, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(2, 3);
extern void vlg_sig_err(Vlg *, int, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(3, 4);
extern void vlg_sig_warn(Vlg *, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(2, 3);
extern void vlg_sig_info(Vlg *, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(2, 3);
extern void vlg_sig_dbg1(Vlg *, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(2, 3);
extern void vlg_sig_dbg2(Vlg *, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(2, 3);
extern void vlg_sig_dbg3(Vlg *, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(2, 3);

#ifndef VLG_COMPILE_INLINE
# ifdef VSTR_AUTOCONF_NDEBUG
#  define VLG_COMPILE_INLINE 1
# else
#  define VLG_COMPILE_INLINE 0
# endif
#endif

#if defined(VSTR_AUTOCONF_HAVE_INLINE) && VLG_COMPILE_INLINE
extern inline void vlg_dbg1(Vlg *vlg, const char *fmt, ... )
{
  va_list ap;
  
  if (vlg->out_dbg < 1)
    return;

  va_start(ap, fmt);
  vlg_vdbg1(vlg, fmt, ap);
  va_end(ap);
}
extern inline void vlg_dbg2(Vlg *vlg, const char *fmt, ... )
{
  va_list ap;
  
  if (vlg->out_dbg < 2)
    return;

  va_start(ap, fmt);
  vlg_vdbg2(vlg, fmt, ap);
  va_end(ap);
}
extern inline void vlg_dbg3(Vlg *vlg, const char *fmt, ... )
{
  va_list ap;
  
  if (vlg->out_dbg < 3)
    return;

  va_start(ap, fmt);
  vlg_vdbg3(vlg, fmt, ap);
  va_end(ap);
}
#endif


#define VLG_WARN_GOTO(label, fmt) do {                \
      vlg_warn fmt ;                                  \
      goto label ;                                    \
    } while (0)
#define VLG_WARN_RET(val, fmt) do {                   \
      vlg_warn fmt ;                                  \
      return val ;                                    \
    } while (0)
#define VLG_WARN_RET_VOID(fmt) do {                   \
      vlg_warn fmt ;                                  \
      return;                                         \
    } while (0)

#define VLG_WARNNOMEM_GOTO(label, fmt) do {           \
      errno = ENOMEM;                                 \
      vlg_warn fmt ;                                  \
      goto label ;                                    \
    } while (0)
#define VLG_WARNNOMEM_RET(val, fmt) do {              \
      errno = ENOMEM;                                 \
      vlg_warn fmt ;                                  \
      return val ;                                    \
    } while (0)

#define VLG_ERRNOMEM(fmt) do {                        \
      errno = ENOMEM;                                 \
      vlg_err fmt ;                                   \
    } while (0)

#endif
