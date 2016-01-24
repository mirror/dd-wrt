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
/* date text generating functions ... stop glibc some spending all the time
 * doing stat() on /etc/localtime */
   
#define EX_UTILS_NO_FUNCS 1
#include "ex_utils.h"

#include "date.h"

#include "mk.h"

#include <limits.h> /* CHAR_BIT */

#ifndef FALSE
# define FALSE 0
#endif

#ifndef TRUE
# define TRUE 1
#endif

/* FIXME: fix this to use constant data which is always in the C locale */

void date_free(Date_store *data)
{
  F(data);
}

Date_store *date_make(void)
{
  Date_store *data = MK(sizeof(Date_store));
  unsigned int num = 0;

  if (!data)
    return (NULL);
  
  data->saved_count = DATE__CACHE_NUM - 1;

  while (num < DATE__CACHE_NUM)
    data->saved_val[num++] = -1;

  return (data);
}

const struct tm *date_gmtime(Date_store *data, time_t val)
{
  unsigned int num = 0;

  ASSERT(malloc_check_sz_mem(data, sizeof(Date_store)) || TRUE);
  
  while (num < DATE__CACHE_NUM)
  {
    if (data->saved_val[num] == val)
      return (data->saved_tm + num);
    
    ++num;
  }
  
  num = (data->saved_count + 1) % DATE__CACHE_NUM;
  
  data->saved_val[num] = -1;
  if (!gmtime_r(&val, data->saved_tm + num))
    return (NULL);
  data->saved_val[num] = val;
  data->saved_count    = num;
  
  return (data->saved_tm + num);
}


static const char date__days_shrt[7][4] =
{"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char date__days_full[7][10] =
{"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday",
 "Saturday"};
static const char date__months[12][4] =
{"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
 "Sep", "Oct", "Nov", "Dec"};

/* stupid roll your own string API, but what ya gonna do... */
#define CP_BEG() do { ptr = data->ret_buf;                              \
      ASSERT(memset(ptr, 0xbe, DATE__RET_SZ));                          \
    } while (FALSE)
#define CP(x, n) do {                                                   \
      memcpy(ptr, x, n); ptr += (n);                                    \
      ASSERT((ptr >=  data->ret_buf) &&                                 \
             (ptr <= (data->ret_buf + DATE__RET_SZ))); } while (FALSE)
#define CP_LEN(x) CP(x, strlen(x))
#define CP_NUM(x) do {                                  \
      unsigned int num = (x);                           \
      char buf[sizeof(unsigned int) * CHAR_BIT];        \
      char *tmp = buf + sizeof(buf);                    \
                                                        \
      while (num)                                       \
      {                                                 \
        *--tmp = '0' + (num % 10);                      \
        num /= 10;                                      \
      }                                                 \
      if ((tmp - buf) != sizeof(buf))                   \
        CP(tmp, sizeof(buf) - (tmp - buf));             \
      else                                              \
        CP_LEN("0");                                    \
    } while (FALSE)
#define CP_02NUM(x) do {                        \
      if ((x) < 10)                             \
        CP_LEN("0");                            \
      CP_NUM(x);                                \
    } while (FALSE)
#define CP__2NUM(x) do {                        \
      if ((x) < 10)                             \
        CP_LEN(" ");                            \
      CP_NUM(x);                                \
    } while (FALSE)

#define CP_END() CP("", 1)

/* we do all of this "locally" due to POSIX requiring strftime to call tzset()
   Glibc then compounds that by stat()'ing */
const char *date_rfc1123(Date_store *data, time_t val)
{
  const struct tm *tm = NULL;
  char *ptr = NULL;
  
  if (!(tm = date_gmtime(data, val)))
    err(EXIT_FAILURE, "gmtime_r(%s)", "%a, %d %h %Y %T GMT");

  CP_BEG();
  CP(date__days_shrt[tm->tm_wday], 3); /* %a */
  CP_LEN(", ");
  CP_02NUM(tm->tm_mday); /* %d */
  CP_LEN(" ");
  CP(date__months[tm->tm_mon], 3); /* %h */
  CP_LEN(" ");
  CP_NUM(tm->tm_year + 1900); /* %Y */
  CP_LEN(" ");
  CP_02NUM(tm->tm_hour); /* %T */
  CP_LEN(":");
  CP_02NUM(tm->tm_min);
  CP_LEN(":");
  CP_02NUM(tm->tm_sec);
  CP_LEN(" GMT");
  CP_END();
  
  return (data->ret_buf);
}
const char *date_rfc850(Date_store *data, time_t val)
{
  const struct tm *tm = NULL;
  char *ptr = NULL;
  
  if (!(tm = date_gmtime(data, val)))
    err(EXIT_FAILURE, "gmtime_r(%s)", "%A, %d-%h-%y %T GMT");
  
  CP_BEG();
  CP_LEN(date__days_full[tm->tm_wday]); /* %A */
  CP_LEN(", ");
  CP_02NUM(tm->tm_mday); /* %d */
  CP_LEN("-");
  CP(date__months[tm->tm_mon], 3); /* %h */
  CP_LEN("-");
  CP_02NUM(tm->tm_year % 100); /* %y */
  CP_LEN(" ");
  CP_02NUM(tm->tm_hour); /* %T */
  CP_LEN(":");
  CP_02NUM(tm->tm_min);
  CP_LEN(":");
  CP_02NUM(tm->tm_sec);
  CP_LEN(" GMT");
  CP_END();
  
  return (data->ret_buf);
}
const char *date_asctime(Date_store *data, time_t val)
{
  const struct tm *tm = NULL;
  char *ptr = NULL;
  
  if (!(tm = date_gmtime(data, val)))
    err(EXIT_FAILURE, "gmtime_r(%s)", "%a %h %e %T %Y");

  CP_BEG();
  CP(date__days_shrt[tm->tm_wday], 3); /* %a */
  CP_LEN(" ");
  CP(date__months[tm->tm_mon], 3); /* %h */
  CP_LEN(" ");
  CP__2NUM(tm->tm_mday); /* %e */
  CP_LEN(" ");
  CP_02NUM(tm->tm_hour); /* %T */
  CP_LEN(":");
  CP_02NUM(tm->tm_min);
  CP_LEN(":");
  CP_02NUM(tm->tm_sec);
  CP_LEN(" ");
  CP_NUM(tm->tm_year + 1900); /* %Y */
  CP_END();
  
  return (data->ret_buf);
}

#if 0
#define VPREFIX(vstr, p, l, cstr)                                       \
    (((l) >= CLEN(cstr)) &&                                             \
     vstr_cmp_buf_eq(vstr, p, CLEN(cstr), cstr, CLEN(cstr)))

/* see rfc2616 3.3.1 -- full date parser */
static time_t date__parse_http(Date_storage *date,
                               Vstr_base *s1, size_t pos, size_t len,
                               time_t val)
{
  const struct tm *tm = date_gmtime(date, &val);
  unsigned int scan = 0;

  if (!tm) return (-1);

  switch (len)
  {
    case 4 + 1 + 9 + 1 + 8 + 1 + 3: /* rfc1123 format - should be most common */
    {
      scan = 0;
      while (scan < 7)
      {
        if (VPREFIX(s1, pos, len, http__date_days_shrt[scan]))
          break;
        ++scan;
      }
      len -= 3; pos += 3;
      
      if (!VPREFIX(s1, pos, len, ", "))
        return (-1);
      len -= CLEN(", "); pos += CLEN(", ");

      tm->tm_mday = http__date_parse_2d(s1, pos, len, 1, 31);
      
      if (!VPREFIX(s1, pos, len, " "))
        return (-1);
      len -= CLEN(" "); pos += CLEN(" ");

      scan = 0;
      while (scan < 12)
      {
        if (VPREFIX(s1, pos, len, http__date_months[scan]))
          break;
        ++scan;
      }
      len -= 3; pos += 3;
      
      tm->tm_mon = scan;
      
      if (!VPREFIX(s1, pos, len, " "))
        return (-1);
      len -= CLEN(" "); pos += CLEN(" ");

      tm->tm_year = http__date_parse_4d(s1, pos, len);

      if (!VPREFIX(s1, pos, len, " "))
        return (-1);
      len -= CLEN(" "); pos += CLEN(" ");

      tm->tm_hour = http__date_parse2d(s1, pos, len, 0, 23);
      if (!VPREFIX(s1, pos, len, ":"))
        return (-1);
      len -= CLEN(":"); pos += CLEN(":");
      tm->tm_min  = http__date_parse2d(s1, pos, len, 0, 59);
      if (!VPREFIX(s1, pos, len, ":"))
        return (-1);
      len -= CLEN(":"); pos += CLEN(":");
      tm->tm_sec  = http__date_parse2d(s1, pos, len, 0, 61);
      
      if (!VPREFIX(s1, pos, len, " GMT"))
        return (-1);
    }
    return (mktime(tm));

    case  7 + 1 + 7 + 1 + 8 + 1 + 3:
    case  8 + 1 + 7 + 1 + 8 + 1 + 3:
    case  9 + 1 + 7 + 1 + 8 + 1 + 3:
    case 10 + 1 + 7 + 1 + 8 + 1 + 3: /* rfc850 format */
    {
      size_t match_len = 0;
      
      scan = 0;
      while (scan < 7)
      {
        match_len = CLEN(http__date_days_full[scan]);
        if (VPREFIX(s1, pos, len, http__date_days_full[scan]))
          break;
        ++scan;
      }
      len -= match_len; pos += match_len;

      return (-1);
    }
    return (mktime(tm));

    case  3 + 1 + 6 + 1 + 8 + 1 + 4: /* asctime format */
    {
      scan = 0;
      while (scan < 7)
      {
        if (VPREFIX(s1, pos, len, http__date_days_shrt[scan]))
          break;
        ++scan;
      }
      len -= 3; pos += 3;
      
      return (-1);
    }
    return (mktime(tm));
  }
  
  return (-1);  
}
#endif

/* syslog is "special" as it might be called from a signal handler etc. */
const char *date_syslog(time_t val, char *buf, size_t len)
{
  struct tm store_tm_val[1];
  struct tm *tm_val = NULL;
  
  if (!(tm_val = localtime_r(&val, store_tm_val)))
    err(EXIT_FAILURE, "localtime_r(%s)", "syslog");

  strftime(buf, len, "%h %e %T", tm_val);
  
  return (buf);
}

