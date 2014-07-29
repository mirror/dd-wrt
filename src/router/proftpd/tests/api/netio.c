/*
 * ProFTPD - FTP server testsuite
 * Copyright (c) 2008-2014 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, The ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/* NetIO API tests
 * $Id: netio.c,v 1.1 2014/01/06 06:58:23 castaglia Exp $
 */

#include "tests.h"

/* See RFC 854 for the definition of these Telnet values */

/* Telnet "Interpret As Command" indicator */
#define TELNET_IAC     255
#define TELNET_DONT    254
#define TELNET_DO      253
#define TELNET_WONT    252
#define TELNET_WILL    251
#define TELNET_IP      244
#define TELNET_DM      242

static pool *p = NULL;

static int tmp_fd = -1;
static const char *tmp_path = NULL;

static void test_cleanup(void) {
  (void) close(tmp_fd);
  tmp_fd = -1;

  if (tmp_path != NULL) {
    (void) unlink(tmp_path);
    tmp_path = NULL;
  }
}

static int open_tmpfile(void) {
  int fd;

  if (tmp_path != NULL) {
    test_cleanup();
  }

  tmp_path = "/tmp/netio-test.dat";
  fd = open(tmp_path, O_RDWR|O_CREAT, 0666);
  fail_unless(fd >= 0, "Failed to open '%s': %s", tmp_path, strerror(errno));  
  tmp_fd = fd;

  return fd;
}

static void set_up(void) {
  if (p == NULL) {
    p = permanent_pool = make_sub_pool(NULL);
  }

  init_netio();
}

static void tear_down(void) {
  if (p) {
    destroy_pool(p);
    p = permanent_pool = NULL;
  } 

  test_cleanup();
}

START_TEST (netio_open_test) {
  pr_netio_stream_t *nstrm;
  int fd;

  fd = -1;

  nstrm = pr_netio_open(NULL, PR_NETIO_STRM_CTRL, fd, PR_NETIO_IO_RD);
  fail_unless(nstrm == NULL, "Failed to handle null pool argument");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL, got %s (%d)",
    strerror(errno), errno);

  nstrm = pr_netio_open(p, 7777, fd, PR_NETIO_IO_RD);
  fail_unless(nstrm == NULL, "Failed to handle unknown stream type argument");
  fail_unless(errno == EPERM, "Failed to set errno to EPERM, got %s (%d)",
    strerror(errno), errno);

  nstrm = pr_netio_open(p, PR_NETIO_STRM_CTRL, fd, PR_NETIO_IO_RD);
  fail_unless(nstrm != NULL, "Failed to open stream on fd %d: %s", fd,
    strerror(errno));

  pr_netio_close(nstrm);
}
END_TEST

START_TEST (netio_close_test) {
  pr_netio_stream_t *nstrm;
  int res, fd;

  fd = -1;

  res = pr_netio_close(NULL);
  fail_unless(res == -1, "Failed to handle null stream argument");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL, got %s (%d)",
    strerror(errno), errno);

  nstrm = pr_netio_open(p, PR_NETIO_STRM_CTRL, fd, PR_NETIO_IO_RD);
  nstrm->strm_type = 7777;

  res = pr_netio_close(nstrm);
  fail_unless(res == -1, "Failed to handle unknown stream type argument");
  fail_unless(errno == EPERM, "Failed to set errno to EPERM, got %s (%d)",
    strerror(errno), errno);

  nstrm->strm_type = PR_NETIO_STRM_CTRL;
  res = pr_netio_close(nstrm);
  fail_unless(res == -1, "Failed to handle bad file descriptor");
  fail_unless(errno == EBADF, "Failed to set errno to EBADF, got %s (%d)",
    strerror(errno), errno);
}
END_TEST

START_TEST (netio_buffer_alloc_test) {
  pr_buffer_t *pbuf;
  pr_netio_stream_t *nstrm;

  pbuf = pr_netio_buffer_alloc(NULL);
  fail_unless(pbuf == NULL, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL, got %s (%d)",
    strerror(errno), errno);

  nstrm = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);

  pbuf = pr_netio_buffer_alloc(nstrm);
  fail_unless(pbuf != NULL, "Failed to allocate buffer: %s", strerror(errno));

  pr_netio_close(nstrm);
}
END_TEST

START_TEST (netio_telnet_gets_args_test) {
  char *buf, *res;
  pr_netio_stream_t *in, *out;

  res = pr_netio_telnet_gets(NULL, 0, NULL, NULL);
  fail_unless(res == NULL, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL, got %s (%d)",
    strerror(errno), errno);

  buf = "";
  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_WR);

  res = pr_netio_telnet_gets(buf, 0, in, out);
  fail_unless(res == NULL,
    "Failed to handle zero-length buffer length argument");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL, got %s (%d)",
    strerror(errno), errno);

  res = pr_netio_telnet_gets(buf, 1, NULL, out);
  fail_unless(res == NULL, "Failed to handle null input stream argument");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL, got %s (%d)",
    strerror(errno), errno);

  res = pr_netio_telnet_gets(buf, 1, in, NULL);
  fail_unless(res == NULL, "Failed to handle null output stream argument");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL, got %s (%d)",
    strerror(errno), errno);

  pr_netio_close(in);
  pr_netio_close(out);
}
END_TEST

START_TEST (netio_telnet_gets_single_line_test) {
  char buf[256], *cmd, *res;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_WR);

  cmd = "Hello, World!\n";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;
  len = snprintf(pbuf->buf, pbuf->buflen-1, "%s", cmd);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  pr_netio_close(in);
  pr_netio_close(out);

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strcmp(buf, cmd) == 0, "Expected string '%s', got '%s'", cmd,
    buf);
}
END_TEST

START_TEST (netio_telnet_gets_multi_line_test) {
  char buf[256], *cmd, *first_cmd, *res;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_WR);

  cmd = "Hello, World!\nHow are you?\n";
  first_cmd = "Hello, World!\n";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;
  len = snprintf(pbuf->buf, pbuf->buflen-1, "%s", cmd);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  pr_netio_close(in);
  pr_netio_close(out);

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strcmp(buf, first_cmd) == 0, "Expected string '%s', got '%s'",
    first_cmd, buf);
}
END_TEST

START_TEST (netio_telnet_gets_no_newline_test) {
  char buf[8], *cmd, *res;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_WR);

  cmd = "Hello, World!";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;
  len = snprintf(pbuf->buf, pbuf->buflen-1, "%s", cmd);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  pr_netio_close(in);
  pr_netio_close(out);

  fail_unless(res == NULL, "Read in string unexpectedly, got '%s'", buf);
  fail_unless(xerrno == E2BIG, "Failed to set errno to E2BIG, got (%d) %s",
    xerrno, strerror(xerrno));
}
END_TEST

START_TEST (netio_telnet_gets_telnet_will_test) {
  char buf[256], *cmd, *res, telnet_opt;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, out_fd, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);

  out_fd = open_tmpfile();
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, out_fd, PR_NETIO_IO_WR);

  cmd = "Hello, World!\n";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;

  telnet_opt = 7;
  len = snprintf(pbuf->buf, pbuf->buflen-1, "Hello, %c%c%cWorld!\n", TELNET_IAC,
    TELNET_WILL, telnet_opt);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  pr_netio_close(in);

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strcmp(buf, cmd) == 0, "Expected string '%s', got '%s'", cmd,
    buf);

  /* Rewind the output stream fd. */
  lseek(out_fd, 0, SEEK_SET);
  len = read(out_fd, buf, sizeof(buf)-1);
  pr_netio_close(out);

  fail_unless(len == 3, "Expected to read 3 bytes from output stream, got %d",
    len);
  fail_unless(buf[0] == (char) TELNET_IAC, "Expected IAC at index 0, got %d",
    buf[0]);
  fail_unless(buf[1] == (char) TELNET_DONT, "Expected DONT at index 1, got %d",
    buf[1]);
  fail_unless(buf[2] == telnet_opt, "Expected opt '%c' at index 2, got %c",
    telnet_opt, buf[2]);

  test_cleanup();
}
END_TEST

START_TEST (netio_telnet_gets_telnet_bare_will_test) {
  char buf[256], *cmd, *res, telnet_opt;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_WR);

  cmd = "Hello, World!\n";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;

  telnet_opt = 7;
  len = snprintf(pbuf->buf, pbuf->buflen-1, "Hello, %c%cWorld!\n", TELNET_WILL,
    telnet_opt);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  pr_netio_close(in);
  pr_netio_close(out);

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strncmp(buf, cmd, 7) == 0, "Expected string '%*s', got '%*s'",
    7, cmd, 7, buf);
  fail_unless(buf[7] == (char) TELNET_WILL, "Expected WILL at index 7, got %d",
    buf[7]);
  fail_unless(buf[8] == telnet_opt, "Expected Telnet opt %c at index 8, got %d",
    telnet_opt, buf[8]);
  fail_unless(strcmp(buf + 9, cmd + 7) == 0, "Expected string '%s', got '%s'",
    cmd + 7, buf + 9);
}
END_TEST

START_TEST (netio_telnet_gets_telnet_will_multi_read_test) {
  char buf[256], *cmd, *res, telnet_opt;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, out_fd, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);

  out_fd = open_tmpfile();
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, out_fd, PR_NETIO_IO_WR);

  cmd = "Hello, World!\n";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;

  len = snprintf(pbuf->buf, pbuf->buflen-1, "Hello, %c%c", TELNET_IAC,
    TELNET_WILL);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strncmp(buf, cmd, 7) == 0, "Expected string '%*s', got '%*s'",
    7, cmd, 7, buf);

  /* Fill up the input stream's buffer with the rest of the Telnet WILL
   * sequence.
   */
  telnet_opt = 7;
  len = snprintf(pbuf->buf, pbuf->buflen-1, "%cWorld!\n", telnet_opt);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  /* Read again, to see if the state was preserved across multiple calls
   * to pr_netio_telnet_gets().
   */
  res = pr_netio_telnet_gets(buf + 7, sizeof(buf)-8, in, out);
  xerrno = errno;

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strcmp(buf, cmd) == 0, "Expected string '%s', got '%s'",
    cmd, buf);

  pr_netio_close(in);

  /* Rewind the output stream fd. */
  lseek(out_fd, 0, SEEK_SET);
  len = read(out_fd, buf, sizeof(buf)-1);
  pr_netio_close(out);

  fail_unless(len == 3, "Expected to read 3 bytes from output stream, got %d",
    len);
  fail_unless(buf[0] == (char) TELNET_IAC, "Expected IAC at index 0, got %d",
    buf[0]);
  fail_unless(buf[1] == (char) TELNET_DONT, "Expected DONT at index 1, got %d",
    buf[1]);
  fail_unless(buf[2] == telnet_opt, "Expected %c at index 2, got %d",
    telnet_opt, buf[2]);

  test_cleanup();
}
END_TEST

START_TEST (netio_telnet_gets_telnet_wont_test) {
  char buf[256], *cmd, *res, telnet_opt;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, out_fd, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);

  out_fd = open_tmpfile();
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, out_fd, PR_NETIO_IO_WR);

  cmd = "Hello, World!\n";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;

  telnet_opt = 7;
  len = snprintf(pbuf->buf, pbuf->buflen-1, "Hello, %c%c%cWorld!\n", TELNET_IAC,
    TELNET_WONT, telnet_opt);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  pr_netio_close(in);

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strcmp(buf, cmd) == 0, "Expected string '%s', got '%s'", cmd,
    buf);

  /* Rewind the output stream fd. */
  lseek(out_fd, 0, SEEK_SET);
  len = read(out_fd, buf, sizeof(buf)-1);
  pr_netio_close(out);

  fail_unless(len == 3, "Expected to read 3 bytes from output stream, got %d",
    len);
  fail_unless(buf[0] == (char) TELNET_IAC, "Expected IAC at index 0, got %d",
    buf[0]);
  fail_unless(buf[1] == (char) TELNET_DONT, "Expected DONT at index 1, got %d",
    buf[1]);
  fail_unless(buf[2] == telnet_opt, "Expected opt '%c' at index 2, got %c",
    telnet_opt, buf[2]);

  test_cleanup();
}
END_TEST

START_TEST (netio_telnet_gets_telnet_bare_wont_test) {
  char buf[256], *cmd, *res, telnet_opt;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_WR);

  cmd = "Hello, World!\n";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;

  telnet_opt = 7;
  len = snprintf(pbuf->buf, pbuf->buflen-1, "Hello, %c%cWorld!\n", TELNET_WONT,
    telnet_opt);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  pr_netio_close(in);
  pr_netio_close(out);

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strncmp(buf, cmd, 7) == 0, "Expected string '%*s', got '%*s'",
    7, cmd, 7, buf);
  fail_unless(buf[7] == (char) TELNET_WONT, "Expected WONT at index 7, got %d",
    buf[7]);
  fail_unless(buf[8] == telnet_opt, "Expected Telnet opt %c at index 8, got %d",
    telnet_opt, buf[8]);
  fail_unless(strcmp(buf + 9, cmd + 7) == 0, "Expected string '%s', got '%s'",
    cmd + 7, buf + 9);
}
END_TEST

START_TEST (netio_telnet_gets_telnet_do_test) {
  char buf[256], *cmd, *res, telnet_opt;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, out_fd, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);

  out_fd = open_tmpfile();
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, out_fd, PR_NETIO_IO_WR);

  cmd = "Hello, World!\n";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;

  telnet_opt = 7;
  len = snprintf(pbuf->buf, pbuf->buflen-1, "Hello, %c%c%cWorld!\n", TELNET_IAC,
    TELNET_DO, telnet_opt);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  pr_netio_close(in);

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strcmp(buf, cmd) == 0, "Expected string '%s', got '%s'", cmd,
    buf);

  /* Rewind the output stream fd. */
  lseek(out_fd, 0, SEEK_SET);
  len = read(out_fd, buf, sizeof(buf)-1);
  pr_netio_close(out);

  fail_unless(len == 3, "Expected to read 3 bytes from output stream, got %d",
    len);
  fail_unless(buf[0] == (char) TELNET_IAC, "Expected IAC at index 0, got %d",
    buf[0]);
  fail_unless(buf[1] == (char) TELNET_WONT, "Expected WONT at index 1, got %d",
    buf[1]);
  fail_unless(buf[2] == telnet_opt, "Expected opt '%c' at index 2, got %c",
    telnet_opt, buf[2]);

  test_cleanup();
}
END_TEST

START_TEST (netio_telnet_gets_telnet_bare_do_test) {
  char buf[256], *cmd, *res, telnet_opt;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_WR);

  cmd = "Hello, World!\n";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;

  telnet_opt = 7;
  len = snprintf(pbuf->buf, pbuf->buflen-1, "Hello, %c%cWorld!\n", TELNET_DO,
    telnet_opt);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  pr_netio_close(in);
  pr_netio_close(out);

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strncmp(buf, cmd, 7) == 0, "Expected string '%*s', got '%*s'",
    7, cmd, 7, buf);
  fail_unless(buf[7] == (char) TELNET_DO, "Expected DO at index 7, got %d",
    buf[7]);
  fail_unless(buf[8] == telnet_opt, "Expected Telnet opt %c at index 8, got %d",
    telnet_opt, buf[8]);
  fail_unless(strcmp(buf + 9, cmd + 7) == 0, "Expected string '%s', got '%s'",
    cmd + 7, buf + 9);
}
END_TEST

START_TEST (netio_telnet_gets_telnet_dont_test) {
  char buf[256], *cmd, *res, telnet_opt;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, out_fd, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);

  out_fd = open_tmpfile();
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, out_fd, PR_NETIO_IO_WR);

  cmd = "Hello, World!\n";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;

  telnet_opt = 7;
  len = snprintf(pbuf->buf, pbuf->buflen-1, "Hello, %c%c%cWorld!\n", TELNET_IAC,
    TELNET_DONT, telnet_opt);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  pr_netio_close(in);

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strcmp(buf, cmd) == 0, "Expected string '%s', got '%s'", cmd,
    buf);

  /* Rewind the output stream fd. */
  lseek(out_fd, 0, SEEK_SET);
  len = read(out_fd, buf, sizeof(buf)-1);
  pr_netio_close(out);

  fail_unless(len == 3, "Expected to read 3 bytes from output stream, got %d",
    len);
  fail_unless(buf[0] == (char) TELNET_IAC, "Expected IAC at index 0, got %d",
    buf[0]);
  fail_unless(buf[1] == (char) TELNET_WONT, "Expected WONT at index 1, got %d",
    buf[1]);
  fail_unless(buf[2] == telnet_opt, "Expected opt '%c' at index 2, got %c",
    telnet_opt, buf[2]);

  test_cleanup();
}
END_TEST

START_TEST (netio_telnet_gets_telnet_bare_dont_test) {
  char buf[256], *cmd, *res, telnet_opt;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_WR);

  cmd = "Hello, World!\n";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;

  telnet_opt = 7;
  len = snprintf(pbuf->buf, pbuf->buflen-1, "Hello, %c%cWorld!\n", TELNET_DONT,
    telnet_opt);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  pr_netio_close(in);
  pr_netio_close(out);

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strncmp(buf, cmd, 7) == 0, "Expected string '%*s', got '%*s'",
    7, cmd, 7, buf);
  fail_unless(buf[7] == (char) TELNET_DONT, "Expected DONT at index 7, got %d",
    buf[7]);
  fail_unless(buf[8] == telnet_opt, "Expected Telnet opt %c at index 8, got %d",
    telnet_opt, buf[8]);
  fail_unless(strcmp(buf + 9, cmd + 7) == 0, "Expected string '%s', got '%s'",
    cmd + 7, buf + 9);
}
END_TEST

START_TEST (netio_telnet_gets_telnet_ip_test) {
  char buf[256], *cmd, *res;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_WR);

  cmd = "Hello, World!\n";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;

  len = snprintf(pbuf->buf, pbuf->buflen-1, "Hello, %c%cWorld!\n", TELNET_IAC,
    TELNET_IP);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  pr_netio_close(in);
  pr_netio_close(out);

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strcmp(buf, cmd) == 0, "Expected string '%s', got '%s'", cmd,
    buf);
}
END_TEST

START_TEST (netio_telnet_gets_telnet_bare_ip_test) {
  char buf[256], *cmd, *res;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_WR);

  cmd = "Hello, World!\n";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;

  len = snprintf(pbuf->buf, pbuf->buflen-1, "Hello, %cWorld!\n", TELNET_IP);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  pr_netio_close(in);
  pr_netio_close(out);

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strncmp(buf, cmd, 7) == 0, "Expected string '%*s', got '%*s'",
    7, cmd, 7, buf);
  fail_unless(buf[7] == (char) TELNET_IP, "Expected IP at index 7, got %d",
    buf[7]);
  fail_unless(strcmp(buf + 8, cmd + 7) == 0, "Expected string '%s', got '%s'",
    cmd + 7, buf + 8);
}
END_TEST

START_TEST (netio_telnet_gets_telnet_dm_test) {
  char buf[256], *cmd, *res;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_WR);

  cmd = "Hello, World!\n";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;

  len = snprintf(pbuf->buf, pbuf->buflen-1, "Hello, %c%cWorld!\n", TELNET_IAC,
    TELNET_DM);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  pr_netio_close(in);
  pr_netio_close(out);

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strcmp(buf, cmd) == 0, "Expected string '%s', got '%s'", cmd,
    buf);
}
END_TEST

START_TEST (netio_telnet_gets_telnet_bare_dm_test) {
  char buf[256], *cmd, *res;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_WR);

  cmd = "Hello, World!\n";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;

  len = snprintf(pbuf->buf, pbuf->buflen-1, "Hello, %cWorld!\n", TELNET_DM);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  pr_netio_close(in);
  pr_netio_close(out);

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strncmp(buf, cmd, 7) == 0, "Expected string '%*s', got '%*s'",
    7, cmd, 7, buf);
  fail_unless(buf[7] == (char) TELNET_DM, "Expected DM at index 7, got %d",
    buf[7]);
  fail_unless(strcmp(buf + 8, cmd + 7) == 0, "Expected string '%s', got '%s'",
    cmd + 7, buf + 8);
}
END_TEST

START_TEST (netio_telnet_gets_telnet_single_iac_test) {
  char buf[256], *cmd, *res;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_WR);

  cmd = "Hello, World!\n";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;

  len = snprintf(pbuf->buf, pbuf->buflen-1, "Hello, %cWorld!\n", TELNET_IAC);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  pr_netio_close(in);
  pr_netio_close(out);

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strncmp(buf, cmd, 7) == 0, "Expected string '%*s', got '%*s'",
    7, cmd, 7, buf);
  fail_unless(buf[7] == (char) TELNET_IAC, "Expected IAC at index 7, got %d",
    buf[7]);
  fail_unless(strcmp(buf + 8, cmd + 7) == 0, "Expected string '%s', got '%s'",
    cmd + 7, buf + 8);
}
END_TEST

START_TEST (netio_telnet_gets_bug3521_test) {
  char buf[10], *cmd, *res, telnet_opt;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_WR);

  cmd = "Hello, World!\n";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;

  telnet_opt = 7;
  len = snprintf(pbuf->buf, pbuf->buflen-1, "Hello, %c%c%c%cWorld!\n",
    TELNET_IAC, TELNET_IAC, TELNET_WILL, telnet_opt);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  pr_netio_close(in);
  pr_netio_close(out);

  fail_unless(res == NULL, "Expected null");
  fail_unless(xerrno == E2BIG, "Failed to set errno to E2BIG, got %s (%d)",
    strerror(xerrno), xerrno);
}
END_TEST

START_TEST (netio_telnet_gets_bug3697_test) {
  char buf[256], *cmd, *res;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_WR);

  cmd = "Hello, World!\n";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;

  len = snprintf(pbuf->buf, pbuf->buflen-1, "Hello, %c%cWorld!\n", TELNET_IAC,
    TELNET_IAC);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  res = pr_netio_telnet_gets(buf, sizeof(buf)-1, in, out);
  xerrno = errno;

  pr_netio_close(in);
  pr_netio_close(out);

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strncmp(buf, cmd, 7) == 0, "Expected string '%*s', got '%*s'",
    7, cmd, 7, buf);
  fail_unless(buf[7] == (char) TELNET_IAC, "Expected IAC at index 7, got %d",
    buf[7]);
  fail_unless(strcmp(buf + 8, cmd + 7) == 0, "Expected string '%s', got '%s'",
    cmd + 7, buf + 8);
}
END_TEST

START_TEST (netio_telnet_gets_eof_test) {
  char buf[256], *cmd, *res;
  pr_netio_stream_t *in, *out;
  pr_buffer_t *pbuf;
  int len, xerrno;

  in = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_RD);
  out = pr_netio_open(p, PR_NETIO_STRM_CTRL, -1, PR_NETIO_IO_WR);

  cmd = "Hello, World!";

  pr_netio_buffer_alloc(in);
  pbuf = in->strm_buf;

  len = snprintf(pbuf->buf, pbuf->buflen-1, "%s", cmd);
  pbuf->remaining = pbuf->buflen - len;
  pbuf->current = pbuf->buf;

  buf[sizeof(buf)-1] = '\0';

  /* In this scenario, we have not supplied an LF, but the resulting buffer
   * is terminated with a NUL because of the end-of-stream (or error) checks
   * in pr_netio_telnet_gets(), when we read the input stream for more data
   * looking for that LF.
   */
  res = pr_netio_telnet_gets(buf, strlen(cmd) + 2, in, out);
  xerrno = errno;

  pr_netio_close(in);
  pr_netio_close(out);

  fail_unless(res != NULL, "Failed to get string from stream: (%d) %s",
    xerrno, strerror(xerrno));
  fail_unless(strcmp(buf, cmd) == 0, "Expected string '%s', got '%s'", cmd,
    buf);
}
END_TEST

Suite *tests_get_netio_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("netio");

  testcase = tcase_create("base");

  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, netio_open_test);
  tcase_add_test(testcase, netio_close_test);
  tcase_add_test(testcase, netio_buffer_alloc_test);
  tcase_add_test(testcase, netio_telnet_gets_args_test);
  tcase_add_test(testcase, netio_telnet_gets_single_line_test);
  tcase_add_test(testcase, netio_telnet_gets_multi_line_test);
  tcase_add_test(testcase, netio_telnet_gets_no_newline_test);
  tcase_add_test(testcase, netio_telnet_gets_telnet_will_test);
  tcase_add_test(testcase, netio_telnet_gets_telnet_bare_will_test);
  tcase_add_test(testcase, netio_telnet_gets_telnet_will_multi_read_test);
  tcase_add_test(testcase, netio_telnet_gets_telnet_wont_test);
  tcase_add_test(testcase, netio_telnet_gets_telnet_bare_wont_test);
  tcase_add_test(testcase, netio_telnet_gets_telnet_do_test);
  tcase_add_test(testcase, netio_telnet_gets_telnet_bare_do_test);
  tcase_add_test(testcase, netio_telnet_gets_telnet_dont_test);
  tcase_add_test(testcase, netio_telnet_gets_telnet_bare_dont_test);
  tcase_add_test(testcase, netio_telnet_gets_telnet_ip_test);
  tcase_add_test(testcase, netio_telnet_gets_telnet_bare_ip_test);
  tcase_add_test(testcase, netio_telnet_gets_telnet_dm_test);
  tcase_add_test(testcase, netio_telnet_gets_telnet_bare_dm_test);
  tcase_add_test(testcase, netio_telnet_gets_telnet_single_iac_test);
  tcase_add_test(testcase, netio_telnet_gets_bug3521_test);
  tcase_add_test(testcase, netio_telnet_gets_bug3697_test);
  tcase_add_test(testcase, netio_telnet_gets_eof_test);

  suite_add_tcase(suite, testcase);

  return suite;
}
