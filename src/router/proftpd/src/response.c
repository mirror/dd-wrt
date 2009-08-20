/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2001-2008 The ProFTPD Project team
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/* Command response routines
 * $Id: response.c,v 1.12 2008/09/26 20:57:29 castaglia Exp $
 */

#include "conf.h"

pr_response_t *resp_list = NULL, *resp_err_list = NULL;

static int resp_blocked = FALSE;

static pool *resp_pool = NULL;

static char resp_buf[PR_RESPONSE_BUFFER_SIZE] = {'\0'};
static char resp_ml_numeric[4] = {'\0'};

static char *(*resp_handler_cb)(pool *, const char *, ...) = NULL;

static const char *trace_channel = "response";

#define RESPONSE_WRITE_NUM_STR(strm, fmt, numeric, msg) \
  pr_trace_msg(trace_channel, 1, (fmt), (numeric), (msg)); \
  if (resp_handler_cb) \
    pr_netio_printf((strm), "%s", resp_handler_cb(resp_pool, (fmt), (numeric), \
      (msg))); \
  else \
    pr_netio_printf((strm), (fmt), (numeric), (msg));

#define RESPONSE_WRITE_STR(strm, fmt, msg) \
  pr_trace_msg(trace_channel, 1, (fmt), (msg)); \
  if (resp_handler_cb) \
    pr_netio_printf((strm), "%s", resp_handler_cb(resp_pool, (fmt), (msg))); \
  else \
    pr_netio_printf((strm), (fmt), (msg));

#define RESPONSE_WRITE_STR_ASYNC(strm, fmt, msg) \
  pr_trace_msg(trace_channel, 1, pstrcat(session.pool, "async: ", (fmt), NULL), \
    (msg)); \
  if (resp_handler_cb) \
    pr_netio_printf_async((strm), "%s", resp_handler_cb(resp_pool, (fmt), \
      (msg))); \
  else \
    pr_netio_printf_async((strm), (fmt), (msg));

pool *pr_response_get_pool(void) {
  return resp_pool;
}

void pr_response_set_pool(pool *p) {
  resp_pool = p;
}

void pr_response_register_handler(char *(*handler_cb)(pool *, const char *,
    ...)) {
  resp_handler_cb = handler_cb;
}

int pr_response_block(int bool) {
  if (bool == TRUE ||
      bool == FALSE) {
    resp_blocked = bool;
    return 0;
  }

  errno = EINVAL;
  return -1;
}

void pr_response_clear(pr_response_t **head) {
  *head = NULL;
}

void pr_response_flush(pr_response_t **head) {
  unsigned char ml = FALSE;
  char *last_numeric = NULL;
  pr_response_t *resp = NULL;

  if (resp_blocked)
    return;

  for (resp = *head; resp; resp = resp->next) {
    if (ml) {
      /* Look for end of multiline */
      if (!resp->next || (resp->num && strcmp(resp->num, last_numeric) != 0)) {
        RESPONSE_WRITE_NUM_STR(session.c->outstrm, "%s %s\r\n", last_numeric,
          resp->msg)
        ml = FALSE;

      } else {

        /* RFC2228's multiline responses are required for protected sessions. */
	if (MultilineRFC2228 || session.sp_flags) {
          RESPONSE_WRITE_NUM_STR(session.c->outstrm, "%s-%s\r\n", last_numeric,
            resp->msg)

	} else {
          RESPONSE_WRITE_STR(session.c->outstrm, " %s\r\n" , resp->msg)
        }
      }

    } else {
      /* Look for start of multiline */
      if (resp->next && (!resp->next->num ||
          strcmp(resp->num, resp->next->num) == 0)) {
        RESPONSE_WRITE_NUM_STR(session.c->outstrm, "%s-%s\r\n", resp->num,
          resp->msg)
        ml = TRUE;
        last_numeric = resp->num;

      } else {
        RESPONSE_WRITE_NUM_STR(session.c->outstrm, "%s %s\r\n", resp->num,
          resp->msg)
      }
    }
  }

  pr_response_clear(head);
}

void pr_response_add_err(const char *numeric, const char *fmt, ...) {
  pr_response_t *resp = NULL, **head = NULL;
  va_list msg;

  va_start(msg, fmt);
  vsnprintf(resp_buf, sizeof(resp_buf), fmt, msg);
  va_end(msg);
  
  resp_buf[sizeof(resp_buf) - 1] = '\0';
  
  resp = (pr_response_t *) pcalloc(resp_pool, sizeof(pr_response_t));
  resp->num = (numeric ? pstrdup(resp_pool, numeric) : NULL);
  resp->msg = pstrdup(resp_pool, resp_buf);

  pr_trace_msg(trace_channel, 7, "error response added to pending list: %s %s",
    resp->num ? resp->num : "(null)", resp->msg);

  for (head = &resp_err_list;
    *head &&
    (!numeric || !(*head)->num || strcmp((*head)->num, numeric) <= 0) &&
    !(numeric && !(*head)->num && head == &resp_list);
  head = &(*head)->next);

  resp->next = *head;
  *head = resp;
}

void pr_response_add(const char *numeric, const char *fmt, ...) {
  pr_response_t *resp = NULL, **head = NULL;
  va_list msg;

  va_start(msg, fmt);
  vsnprintf(resp_buf, sizeof(resp_buf), fmt, msg);
  va_end(msg);

  resp_buf[sizeof(resp_buf) - 1] = '\0';
  
  resp = (pr_response_t *) pcalloc(resp_pool, sizeof(pr_response_t));
  resp->num = (numeric ? pstrdup(resp_pool, numeric) : NULL);
  resp->msg = pstrdup(resp_pool, resp_buf);
  
  pr_trace_msg(trace_channel, 7, "response added to pending list: %s %s",
    resp->num ? resp->num : "(null)", resp->msg);

  for (head = &resp_list;
    *head &&
    (!numeric || !(*head)->num || strcmp((*head)->num, numeric) <= 0) &&
    !(numeric && !(*head)->num && head == &resp_list);
  head = &(*head)->next);

  resp->next = *head;
  *head = resp;
}

void pr_response_send_async(const char *resp_numeric, const char *fmt, ...) {
  char buf[PR_TUNABLE_BUFFER_SIZE] = {'\0'};
  va_list msg;
  int maxlen;

  if (resp_blocked)
    return;

  sstrncpy(buf, resp_numeric, sizeof(buf));
  sstrcat(buf, " ", sizeof(buf));
  
  maxlen = sizeof(buf) - strlen(buf) - 1;
  
  va_start(msg, fmt);
  vsnprintf(buf + strlen(buf), maxlen, fmt, msg);
  va_end(msg);
  
  buf[sizeof(buf) - 1] = '\0';
  sstrcat(buf, "\r\n", sizeof(buf));

  RESPONSE_WRITE_STR_ASYNC(session.c->outstrm, "%s", buf)
}

void pr_response_send(const char *resp_numeric, const char *fmt, ...) {
  va_list msg;

  if (resp_blocked)
    return;

  va_start(msg, fmt);
  vsnprintf(resp_buf, sizeof(resp_buf), fmt, msg);
  va_end(msg);
  
  resp_buf[sizeof(resp_buf) - 1] = '\0';

  RESPONSE_WRITE_NUM_STR(session.c->outstrm, "%s %s\r\n", resp_numeric,
    resp_buf)
}

void pr_response_send_ml_start(const char *resp_numeric, const char *fmt, ...) {
  va_list msg;

  if (resp_blocked)
    return;

  va_start(msg, fmt);
  vsnprintf(resp_buf, sizeof(resp_buf), fmt, msg);
  va_end(msg);

  resp_buf[sizeof(resp_buf) - 1] = '\0';
  sstrncpy(resp_ml_numeric, resp_numeric, sizeof(resp_ml_numeric));

  RESPONSE_WRITE_NUM_STR(session.c->outstrm, "%s-%s\r\n", resp_ml_numeric,
    resp_buf)
}

void pr_response_send_ml(const char *fmt, ...) {
  va_list msg;

  if (resp_blocked)
    return;

  va_start(msg, fmt);
  vsnprintf(resp_buf, sizeof(resp_buf), fmt, msg);
  va_end(msg);

  resp_buf[sizeof(resp_buf) - 1] = '\0';

  RESPONSE_WRITE_STR(session.c->outstrm, " %s\r\n", resp_buf)
}

void pr_response_send_ml_end(const char *fmt, ...) {
  va_list msg;

  if (resp_blocked)
    return;
 
  va_start(msg, fmt);
  vsnprintf(resp_buf, sizeof(resp_buf), fmt, msg);
  va_end(msg);

  resp_buf[sizeof(resp_buf) - 1] = '\0';

  RESPONSE_WRITE_NUM_STR(session.c->outstrm, "%s %s\r\n", resp_ml_numeric,
    resp_buf)
}

void pr_response_send_raw(const char *fmt, ...) {
  va_list msg;

  if (resp_blocked)
    return;

  va_start(msg, fmt);
  vsnprintf(resp_buf, sizeof(resp_buf), fmt, msg);
  va_end(msg);

  resp_buf[sizeof(resp_buf) - 1] = '\0';

  RESPONSE_WRITE_STR(session.c->outstrm, "%s\r\n", resp_buf)
}

