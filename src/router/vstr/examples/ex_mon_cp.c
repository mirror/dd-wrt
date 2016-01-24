/* This monitors a copy to a file, and prints nice stats */

#define EX_UTILS_NO_USE_INPUT 1
#include "ex_utils.h"

#include <sys/time.h>
#include <time.h>

#define CUR_RATE_NUM_SECS 60

int main(int argc, char *argv[])
{
  Vstr_base *s1 = NULL;
  Vstr_base *s2 = ex_init(&s1);
  int fd = -1;
  struct stat stat_buf;
  time_t beg_time;
  unsigned int beg_sz = 0;
  unsigned int last_sz = 0;
  unsigned int count = 0;
  struct
  {
   time_t timestamp;
   unsigned int sz;
  } cur[CUR_RATE_NUM_SECS];

  if (argc != 2)
  {
    size_t pos = 0;

    VSTR_ADD_CSTR_PTR(s1, 0, argc ? argv[0] : "mon_cp");

    if ((pos = vstr_srch_chr_rev(s1, 1, s1->len, '/')))
      vstr_del(s1, 1, pos);

    VSTR_ADD_CSTR_PTR(s1, 0, " Format: ");
    VSTR_ADD_CSTR_PTR(s1, s1->len, " <filename>\n");

    io_put_all(s1, STDERR_FILENO);

    exit (EXIT_FAILURE);
  }

  fd = io_open(argv[1]);

  if (fstat(fd, &stat_buf) == -1)
    err(EXIT_FAILURE, "fstat");

  vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_cntl_conf(s2->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_sc_fmt_add_bkmg_Byte_uint(s1->conf, "{BKMG:%u}");
  vstr_sc_fmt_add_bkmg_Byte_uint(s2->conf, "{BKMG:%u}");
  vstr_sc_fmt_add_bkmg_Bytes_uint(s2->conf, "{BKMG/s:%u}");

  if (s1->conf->malloc_bad || s2->conf->malloc_bad)
    errno = ENOMEM, err(EXIT_FAILURE, "conf");
  
  beg_time = time(NULL); --beg_time;
  beg_sz = last_sz = stat_buf.st_size;
  while (count < CUR_RATE_NUM_SECS)
  {
    cur[count].timestamp = beg_time;
    cur[count].sz = beg_sz;
    ++count;
  }

  vstr_add_fmt(s1, 0, "Start size = ${BKMG:%u}, current period = %u seconds.\n",
               beg_sz, CUR_RATE_NUM_SECS);

  count = 0;
  while (count < (5 * 60)) /* same for 5 minutes */
  {
    size_t prev_len = s2->len;
    time_t now;

    if (fstat(fd, &stat_buf) == -1)
      err(EXIT_FAILURE, "fstat:");

    ++count;
    if (last_sz != (unsigned int)stat_buf.st_size)
      count = 0;
    if (last_sz > (unsigned int)stat_buf.st_size)
      break;
    last_sz = stat_buf.st_size;

    now = time(NULL);
    memmove(cur, cur + 1, sizeof(cur[0]) * (CUR_RATE_NUM_SECS - 1));
    cur[CUR_RATE_NUM_SECS - 1].timestamp = now;
    cur[CUR_RATE_NUM_SECS - 1].sz = last_sz;

    vstr_del(s2, 1, s2->len);
    vstr_add_fmt(s2, 0,
                 "CP = $8{BKMG:%u} | "
                 "Rate = $10{BKMG/s:%u} | "
                 "CP(C) = $8{BKMG:%u} | "
                 "Rate(C) = $10{BKMG/s:%u}",
                 (last_sz - beg_sz),
                 (unsigned int)((last_sz - beg_sz) /
                                (now - beg_time)),
                 (last_sz - cur[0].sz),
                 (unsigned int)((last_sz - cur[0].sz) /
                                (now - cur[0].timestamp)));

    if (s2->len < prev_len)
    {
      vstr_add_rep_chr(s1, s1->len, '\b', prev_len - s2->len);
      vstr_add_rep_chr(s1, s1->len, ' ',  prev_len - s2->len);
    }

    vstr_add_rep_chr(s1, s1->len, '\b', prev_len);
    vstr_add_vstr(s1, s1->len, s2, 1, s2->len, 0);

    if (s1->conf->malloc_bad || s2->conf->malloc_bad)
      errno = ENOMEM, err(EXIT_FAILURE, "data");
  
    io_put_all(s1, STDOUT_FILENO);

    sleep(1);
  }

  close(fd);

  vstr_add_rep_chr(s1, s1->len, '\n', 1);

  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, s2));
}
