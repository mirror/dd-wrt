#ifndef OPT_H
#define OPT_H

#include <vstr.h>
#include <getopt.h>
#include <string.h>

#define OPT_SC_EXPORT_CSTR(e, x, mbs, desc) do {                          \
      if (vstr_srch_chr_fwd(x, 1, (x)->len, 0))                         \
        usage(program_name, EXIT_FAILURE,                               \
              " The value for " desc " must not include a NIL character.\n"); \
      if ((mbs) && !(x)->len)                                           \
        usage(program_name, EXIT_FAILURE,                               \
              " The value for " desc " can't be set to an empty string.\n"); \
      if ((x)->len && !((e) = vstr_export_cstr_ptr(x, 1, (x)->len)))    \
        errno = ENOMEM, err(EXIT_FAILURE, "options");                   \
    } while (FALSE)

#ifndef CONF_FULL_STATIC
# include <pwd.h>
# include <grp.h>
# include <sys/types.h>
# define OPT_SC_RESOLVE_UID(opts) do {                                  \
      const char *name  = NULL;                                         \
      struct passwd *pw = NULL;                                         \
                                                                        \
      OPT_SC_EXPORT_CSTR(name, opts->vpriv_uid, FALSE, "privilage uid"); \
                                                                        \
      if (name && (pw = getpwnam(name)))                                \
        (opts)->priv_uid = pw->pw_uid;                                  \
    } while (FALSE)
# define OPT_SC_RESOLVE_GID(opts) do {                                  \
      const char *name = NULL;                                          \
      struct group *gr = NULL;                                          \
                                                                        \
      OPT_SC_EXPORT_CSTR(name, opts->vpriv_gid, FALSE, "privilage gid"); \
                                                                        \
      if (name && (gr = getgrnam(name)))                                \
        (opts)->priv_uid = gr->gr_gid;                                  \
    } while (FALSE)
#else
# define OPT_SC_RESOLVE_UID(opts)
# define OPT_SC_RESOLVE_GID(opts)
#endif

#define OPT_TOGGLE_ARG(val) (val = opt_toggle(val, optarg))
#define OPT_NUM_ARG(val, desc, min, max, range_desc) do {               \
      Vstr_base *opt__parse_num = vstr_dup_cstr_ptr(NULL, optarg);      \
      unsigned int opt__nflags = VSTR_FLAG02(PARSE_NUM, OVERFLOW, SEP); \
      int opt__num = 0;                                                 \
                                                                        \
      if  (!opt__parse_num)                                             \
        errno = ENOMEM, err(EXIT_FAILURE, "parse_num(%s)", desc);       \
                                                                        \
      opt__num = vstr_parse_ulong(opt__parse_num, 1, opt__parse_num->len, \
                                  opt__nflags, NULL, NULL);             \
      if ((opt__num < min) || (opt__num > max))                         \
        usage(program_name, EXIT_FAILURE,                               \
              " The value for " desc " must be in the range "           \
              #min " to " #max range_desc ".\n");                       \
      val = opt__num;                                                   \
                                                                        \
      vstr_free_base(opt__parse_num);                                   \
    } while (FALSE)
#define OPT_NUM_NR_ARG(val, desc) do {                                  \
      Vstr_base *opt__parse_num = vstr_dup_cstr_ptr(NULL, optarg);      \
      unsigned int opt__nflags = VSTR_FLAG02(PARSE_NUM, OVERFLOW, SEP); \
      int opt__num = 0;                                                 \
                                                                        \
      if  (!opt__parse_num)                                             \
        errno = ENOMEM, err(EXIT_FAILURE, "parse_num(%s)", desc);       \
                                                                        \
      opt__num = vstr_parse_ulong(opt__parse_num, 1, opt__parse_num->len, \
                                  opt__nflags, NULL, NULL);             \
      val = opt__num;                                                   \
                                                                        \
      vstr_free_base(opt__parse_num);                                   \
    } while (FALSE)
#define OPT_VSTR_ARG(val) do {                                          \
      if (!vstr_sub_cstr_ptr((val), 1, (val)->len, optarg))             \
        errno = ENOMEM, err(EXIT_FAILURE, "parse_string(%s)", optarg);  \
    } while (FALSE)

extern int opt_toggle(int, const char *);

/* get program name ... but ignore "lt-" libtool prefix */
extern const char *opt_program_name(const char *, const char *);

extern const char *opt_def_toggle(int);

#endif
