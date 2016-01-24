#ifndef DATE_H
#define DATE_H

#include <vstr.h>

#if VSTR_AUTOCONF_TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if VSTR_AUTOCONF_HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#define DATE__RET_SZ    128
#define DATE__CACHE_NUM   4

typedef struct Date_store
{
 unsigned int saved_count;
 time_t    saved_val[DATE__CACHE_NUM];
 struct tm saved_tm[DATE__CACHE_NUM];
 char ret_buf[DATE__RET_SZ];
} Date_store;

extern Date_store *date_make(void);
extern void date_free(Date_store *);

extern const struct tm *date_gmtime(Date_store *, time_t);

extern const char *date_rfc1123(Date_store *, time_t);
extern const char *date_rfc850(Date_store *, time_t);
extern const char *date_asctime(Date_store *, time_t);

extern const char *date_syslog(time_t, char *, size_t);

#endif
