/*
 * Pound - the reverse-proxy load-balancer
 * Copyright (C) 2002-2010 Apsis GmbH
 * Copyright (C) 2018-2025 Sergey Poznyakoff
 *
 * Pound is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pound is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with pound.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pound.h"
#include "extern.h"

/*
 * Emit to BIO a response line with the given CODE, descriptive TEXT and
 * HEADERS (may be NULL).  TYPE and LEN supply values for Content-Type
 * and Content-Length, correspondingly.  PROTO gives the minor version
 * of the HTTP protocol version: 0 or 1.  If it is 1, the Connection:
 * close header will be added to response.
 */
static void
bio_http_reply_start (BIO *bio, int proto, int code, char const *text,
		      char const *headers,
		      char const *type, CONTENT_LENGTH len)
{
  BIO_printf (bio, "HTTP/1.%d %d %s\r\n",
	      proto,
	      code,
	      text);
  if (type)
    BIO_printf (bio, "Content-Type: %s\r\n", type);
  if (proto == 1)
    {
      BIO_printf (bio,
		  "Content-Length: %"PRICLEN"\r\n"
		  "Connection: close\r\n", len);
    }
  BIO_printf (bio, "%s\r\n", headers ? headers : "");
}

static int http_headers_send (BIO *be, HTTP_HEADER_LIST *head, int safe);

static void
bio_http_reply_start_list (BIO *bio, int proto, int code, char const *text,
			   HTTP_HEADER_LIST *head,
			   CONTENT_LENGTH len)
{
  BIO_printf (bio, "HTTP/1.%d %d %s\r\n", proto, code, text);
  if (proto == 1)
    {
      BIO_printf (bio,
		  "Content-Length: %"PRICLEN"\r\n"
		  "Connection: close\r\n", len);
    }
  http_headers_send (bio, head, 1);
  BIO_printf (bio, "\r\n");
}

/*
 * Emit to BIO response line with the given CODE and descriptive TEXT,
 * followed by HEADERS (may be NULL) and given CONTENT.  TYPE and
 * PROTO are same as for bio_http_reply_start.
 */
static void
bio_http_reply (BIO *bio, int proto, int code, char const *text,
		char const *headers,
		char const *type, char const *content)
{
  size_t len = strlen (content);
  bio_http_reply_start (bio, proto, code, text, headers, type, len);
  BIO_write (bio, content, len);
  BIO_flush (bio);
}

/*
 * HTTP error replies
 */
typedef struct
{
  int code;
  char const *reason;
  char const *text;
} HTTP_STATUS;

static HTTP_STATUS http_status[] = {
  [HTTP_STATUS_OK] = { 200, "OK", "Success" },
  [HTTP_STATUS_BAD_REQUEST] = {
    400,
    "Bad Request",
    "Your browser (or proxy) sent a request that"
    " this server could not understand."
  },
  [HTTP_STATUS_UNAUTHORIZED] = {
    401,
    "Unauthorized",
    "This server could not verify that you are authorized to access"
    " the document requested.  Either you supplied the wrong credentials"
    " (e.g., bad password), or your browser doesn't understand how to supply"
    " the credentials required."
  },
  [HTTP_STATUS_FORBIDDEN] = {
    403,
    "Forbidden",
    "You don't have permission to access this resource."
    " It is either read-protected or not readable by the server."
  },
  [HTTP_STATUS_NOT_FOUND] = {
    404,
    "Not Found",
    "The requested URL was not found on this server."
  },
  [HTTP_STATUS_METHOD_NOT_ALLOWED] = {
    405,
    "Method Not Allowed",
    "The request method is not supported for the requested resource."
  },
  [HTTP_STATUS_PAYLOAD_TOO_LARGE] = {
    413,
    "Payload Too Large",
    "The request content is larger than the proxy server is able to process."
  },
  [HTTP_STATUS_URI_TOO_LONG] = {
    414,
    "URI Too Long",
    "The length of the requested URL exceeds the capacity limit for"
    " this server."
  },
  [HTTP_STATUS_TOO_MANY_REQUESTS] = {
    429,
    "Too Many Requests",
    "Originator sent too many requests in a given amount of time."
  },
  [HTTP_STATUS_INTERNAL_SERVER_ERROR] = {
    500,
    "Internal Server Error",
    "The server encountered an internal error and was"
    " unable to complete your request."
  },
  [HTTP_STATUS_NOT_IMPLEMENTED] = {
    501,
    "Not Implemented",
    "The server does not support the action requested."
  },
  [HTTP_STATUS_SERVICE_UNAVAILABLE] = {
    503,
    "Service Unavailable",
    "The server is temporarily unable to service your"
    " request due to maintenance downtime or capacity"
    " problems. Please try again later."
  },
};

static char err_headers[] =
  "Expires: now\r\n"
  "Pragma: no-cache\r\n"
  "Cache-control: no-cache,no-store\r\n";

static char default_error_page[] = "\
<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\
<html><head>\
<title>%d %s</title>\
</head><body>\
<h1>%s</h1>\
<p>%s</p>\
</body></html>\
";

int
http_status_to_pound (int status)
{
  int i;

  for (i = 0; i < HTTP_STATUS_MAX; i++)
    if (http_status[i].code == status)
      return i;
  return -1;
}

int
pound_to_http_status (int err)
{
  if (!(err >= 0 && err < HTTP_STATUS_MAX))
    err = HTTP_STATUS_INTERNAL_SERVER_ERROR;
  return http_status[err].code;
}

char const *
http_status_reason (int code)
{
  int n = http_status_to_pound (code);
  if (n == -1)
    return NULL;
  return http_status[n].reason;
}

/*
 * Write to BIO an error HTTP 1.PROTO response.  ERR is one of
 * HTTP_STATUS_* constants.  TXT supplies the error page content.
 * If it is NULL, the default text from the http_status table is
 * used.
 */
static int
bio_err_reply (BIO *bio, int proto, int err, struct http_errmsg *msg,
	       int (*cbf) (HTTP_HEADER_LIST *, void *), void *data,
	       int *resp_code, CONTENT_LENGTH *resp_size, char **resp_line)
{
  char const *content;
  struct http_errmsg tmp = HTTP_ERRMSG_INITIALIZER (tmp);
  char *mem = NULL;
  HTTP_HEADER_LIST hlist = DLIST_HEAD_INITIALIZER (hlist);
  size_t len;
  int rc = HTTP_STATUS_OK;

  if (!(err >= 0 && err < HTTP_STATUS_MAX))
    {
      logmsg (LOG_NOTICE,
	      "INTERNAL ERROR: unsupported error code in call to bio_err_reply");
      err = HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

  if (!(msg && msg->text))
    {
      struct stringbuf sb;
      stringbuf_init_log (&sb);
      stringbuf_printf (&sb, default_error_page,
			http_status[err].code,
			http_status[err].reason,
			http_status[err].reason,
			http_status[err].text);
      if ((mem = stringbuf_finish (&sb)) != NULL)
	tmp.text = mem;
      else
	{
	  stringbuf_free (&sb);
	  tmp.text = (char*)http_status[err].text;
	}
      msg = &tmp;
    }

  content = msg->text;
  len = strlen (content);

  http_header_list_append_list (&hlist, &msg->hdr);
  http_header_list_append (&hlist, "Content-Type: text/html", H_KEEP);
  http_header_list_parse (&hlist, err_headers, H_KEEP, NULL);

  if (!cbf || (rc = cbf (&hlist, data)) == HTTP_STATUS_OK)
    {
      bio_http_reply_start_list (bio,
				 proto,
				 http_status[err].code,
				 http_status[err].reason,
				 &hlist,
				 (CONTENT_LENGTH) len);
      BIO_write (bio, content, len);
      BIO_flush (bio);
    }

  http_header_list_free (&hlist);
  free (mem);

  if (rc == HTTP_STATUS_OK)
    {
      if (resp_code)
	*resp_code = http_status[err].code;
      if (resp_size)
	*resp_size = len;
      if (resp_line)
	{
	  struct stringbuf sb;
	  stringbuf_init_log (&sb);
	  stringbuf_printf (&sb, "%d %s",
			    http_status[err].code,
			    http_status[err].reason);
	  if ((*resp_line = stringbuf_finish (&sb)) == NULL)
	    stringbuf_free (&sb);
	}
    }

  return rc;
}

static inline int
http_err_reply_cb (POUND_HTTP *phttp, int err, struct http_errmsg *msg,
		   int (*cbf) (HTTP_HEADER_LIST *, void *), void *data)
{
  return bio_err_reply (phttp->cl, phttp->request.version, err, msg, cbf, data,
			&phttp->response_code, &phttp->res_bytes,
			&phttp->response.request);
}

/*
 * Send error response to the client BIO.  ERR is one of HTTP_STATUS_*
 * constants.  Status code and reason phrase will be taken from the
 * http_status array.  If custom error page is defined in the listener,
 * it will be used, otherwise the default error page for the ERR code
 * will be generated
 */
static int
http_err_reply (POUND_HTTP *phttp, int err)
{
  int rc = http_err_reply_cb (phttp, err, phttp->lstn->http_err[err],
			      NULL, NULL);
  phttp->conn_closed = 1;
  return rc;
}

#define LOG_TIME_SIZE   32

static char *
log_duration (char *buf, size_t size, struct timespec const *start)
{
  struct timespec end, diff;
  clock_gettime (CLOCK_REALTIME, &end);
  diff = timespec_sub (&end, start);
  snprintf (buf, size, "%ld.%03ld", diff.tv_sec, diff.tv_nsec / 1000000);
  return buf;
}

/*
 * Log an error message.
 * Arguments:
 *   phttp   - pointer to a POUND_HTTP structure describing current connection;
 *   flags   - bitmask of the flags below;
 *   code    - HTTP status code; -1 means don't show status code;
 *   en      - system error number; if 0, system error is not included in the
 *             message; if -1, output SSL error instead
 *   fmt     - printf-style format;
 *   ...     - arguments to fmt
 */

#define max(a,b) ((a) > (b) ? (a) : (b))
#define max3(a,b,c) max(max((a), (b)), (c))

#define PHTTP_LOG_DFL        0
#define PHTTP_LOG_BACKEND    0x01  /* Include backend information. */
#define PHTTP_LOG_REVERSE    0x02  /* Indicate reverse direction. */
#define PHTTP_LOG_HANDSHAKE  0x04  /* Reporting handshake error: zero return
				      from ERR_get_error() indicates
				      ECONNRESET. */

static void ATTR_PRINTFLIKE(5,6)
phttp_log (POUND_HTTP *phttp, int flags, int status, int en,
	   char const *fmt, ...)
{
  va_list ap;
  struct stringbuf sb;
  char buf[max3(MAXBUF, MAX_ADDR_BUFSIZE, LOG_TIME_SIZE)];
  char *p;

  stringbuf_init_log (&sb);
  stringbuf_printf (&sb, "(%"PRItid")", POUND_TID ());
  if (status != -1)
    stringbuf_printf (&sb, " e%d", pound_to_http_status (status));
  stringbuf_printf (&sb, ": %s",
		    addr2str (buf, sizeof (buf), &phttp->from_host, 1));
  if (phttp->be && (flags & PHTTP_LOG_BACKEND))
    {
      if (flags & PHTTP_LOG_REVERSE)
	stringbuf_add (&sb, " <= ", 4);
      else
	stringbuf_add (&sb, " => ", 4);
      stringbuf_add_string (&sb, str_be (buf, sizeof (buf), phttp->backend));
    }

  if (phttp->request.request)
    stringbuf_printf (&sb, ": \"%s\"", phttp->request.request);

  stringbuf_add (&sb, ": ", 2);
  va_start (ap, fmt);
  stringbuf_vprintf (&sb, fmt, ap);
  va_end (ap);

  switch (en)
    {
    case 0:
      break;

    case -1:
      {
	unsigned long err = ERR_get_error ();
	if (err == 0)
	  {
	    if (flags & PHTTP_LOG_HANDSHAKE)
	      stringbuf_add_string (&sb, ": connection reset by peer");
	  }
	else
	  {
	    stringbuf_add (&sb, ": ", 2);
	    ERR_error_string_n (err, buf, sizeof (buf));
	    stringbuf_add_string (&sb, buf);
	  }
      }
      break;

    default:
      stringbuf_add (&sb, ": ", 2);
      stringbuf_add_string (&sb, strerror (en));
    }

  if (phttp->response.request)
    stringbuf_printf (&sb, " (%s sec)",
		      log_duration (buf, sizeof (buf), &phttp->start_req));

  if ((p = stringbuf_finish (&sb)) != NULL)
    logmsg (LOG_NOTICE, "%s", p);
  else
    logmsg (LOG_CRIT, "failed to log message");
  stringbuf_free (&sb);
}

static char xdig[] = "0123456789ABCDEF";

static inline int
xtod (int c)
{
  char *p = strchr (xdig, c_toupper (c));
  return p ? p - xdig : -1;
}

static char *
urlencode (char const *input, char const *exc)
{
  int c;
  struct stringbuf sb;
  stringbuf_init_log (&sb);
  while ((c = *input++) != 0)
    {
      if (c_is_class (c, CCTYPE_UNRSRV) || (exc && strchr (exc, c)))
	stringbuf_add_char (&sb, c);
      else
	{
	  stringbuf_add_char (&sb, '%');
	  stringbuf_add_char (&sb, xdig[c >> 4]);
	  stringbuf_add_char (&sb, xdig[c & 0xf]);
	}
    }
  stringbuf_add_char (&sb, 0);
  return stringbuf_finish (&sb);
}

static char *
urldecode (char const *input, char **end)
{
  int c;
  struct stringbuf sb;
  stringbuf_init_log (&sb);
  while ((c = *input++) != 0)
    {
      if (c == '%')
	{
	  int n;
	  c = xtod (*input++);
	  if (c == -1)
	    break;
	  n = c << 4;
	  c = xtod (*input++);
	  if (c == -1)
	    break;
	  n += c;
	  stringbuf_add_char (&sb, n);
	}
      else
	stringbuf_add_char (&sb, c);
    }
  if (c == -1)
    --input;
  if (end)
    *end = (char*)input;
  return stringbuf_finish (&sb);
}

static int
submatch_realloc (struct submatch *sm, GENPAT re)
{
  size_t n = genpat_nsub (re);
  if (n > sm->matchmax)
    {
      POUND_REGMATCH *p = realloc (sm->matchv, n * sizeof (p[0]));
      if (!p)
	return -1;
      sm->matchmax = n;
      sm->matchv = p;
    }
  sm->matchn = n;
  return 0;
}

static void
submatch_reset (struct submatch *sm)
{
  string_unref (sm->tag);
  sm->tag = NULL;
  free (sm->subject);
  sm->subject = NULL;
  sm->matchn = 0;
}

static void
submatch_free (struct submatch *sm)
{
  submatch_reset (sm);
  free (sm->matchv);
  submatch_init (sm);
}

void
submatch_queue_free (struct submatch_queue *smq)
{
  int i;

  for (i = 0; i < SMQ_SIZE; i++)
    {
      submatch_free (&smq->sm[i]);
    }
}

static struct submatch *
submatch_queue_get (struct submatch_queue const *smq, int n)
{
  return (struct submatch *)(smq->sm + (smq->cur + SMQ_SIZE - n) % SMQ_SIZE);
}

static struct submatch *
submatch_queue_push (struct submatch_queue *smq, STRING *tag,
		     struct submatch *src)
{
  struct submatch *sm;
  smq->cur = (smq->cur + 1) % SMQ_SIZE;
  sm = submatch_queue_get (smq, 0);
  submatch_reset (sm);
  *sm = *src;
  sm->tag = string_ref (tag);
  return sm;
}

static int
submatch_queue_find (struct submatch_queue *smq, char const *tag, size_t len)
{
  int i;
  for (i = 0; i < SMQ_SIZE; i++)
    {
      char const *s = string_ptr (submatch_queue_get (smq, i)->tag);
      if (s && strlen (s) == len && memcmp (s, tag, len) == 0)
	return i;
    }
  return -1;
}

static int
submatch_exec (GENPAT re, char const *subject, struct submatch *sm)
{
  int res;

  if (submatch_realloc (sm, re))
    {
      lognomem ();
      return 0;
    }
  res = genpat_match (re, subject, sm->matchn, sm->matchv) == 0;
  if (res)
    {
      if ((sm->subject = strdup (subject)) == NULL)
	return 0;
    }
  return res;
}

static int
submatch_exec_decode (GENPAT re, char const *subject, int decode,
		      struct submatch *sm)
{
  int res;
  char *decoded = NULL;
  if (decode)
    {
      if ((decoded = urldecode (subject, NULL)) == NULL)
	return -1;
      subject = decoded;
    }
  res = submatch_exec (re, subject, sm);
  free (decoded);
  return res;
}

static int http_request_get_query_param (struct http_request *,
					 char const *, size_t,
					 struct query_param **);

typedef int (*accessor_func) (POUND_HTTP *, char const *, int,
			      struct stringbuf *);

struct accessor
{
  char *name;
  accessor_func func;
  int arg;
};

static int
accessor_url (POUND_HTTP *phttp, char const *arg, int arglen,
	      struct stringbuf *sb)
{
  char const *val;
  int rc = http_request_get_url (&phttp->request, &val);
  if (rc == RETRIEVE_OK && val)
    stringbuf_add_string (sb, val);
  else
    rc = RETRIEVE_NOT_FOUND;
  return rc;
}

static int
accessor_path (POUND_HTTP *phttp, char const *arg, int arglen,
	       struct stringbuf *sb)
{
  char const *val;
  int rc = http_request_get_path (&phttp->request, &val);
  if (rc == RETRIEVE_OK && val)
    stringbuf_add_string (sb, val);
  else
    rc = RETRIEVE_NOT_FOUND;
  return rc;
}

static int
accessor_query (POUND_HTTP *phttp, char const *arg, int arglen,
		struct stringbuf *sb)
{
  char const *val;
  int rc = http_request_get_query (&phttp->request, &val);
  if (rc == RETRIEVE_OK && val)
    stringbuf_add_string (sb, val);
  else
    rc = RETRIEVE_NOT_FOUND;
  return rc;
}

static int
accessor_param (POUND_HTTP *phttp, char const *arg, int arglen,
		struct stringbuf *sb)
{
  struct query_param *qp;
  int rc;
  if ((rc = http_request_get_query_param (&phttp->request, arg,
					  arglen, &qp)) == RETRIEVE_OK &&
      qp->value)
    stringbuf_add_string (sb, qp->value);
  else
    rc = RETRIEVE_NOT_FOUND;
  return rc;
}

static int
accessor_get_header (POUND_HTTP *phttp, char const *arg, int arglen,
		     char const **ret_val, size_t *ret_len)
{
  struct http_header *hdr;
  char const *val;

  if ((hdr = http_header_list_locate_name (&phttp->request.headers,
					   arg, arglen)) == NULL)
    return RETRIEVE_NOT_FOUND;

  if ((val = http_header_get_value (hdr)) != NULL)
    {
      *ret_val = val;
      *ret_len = strlen (val);
    }
  else
    return RETRIEVE_NOT_FOUND;

  return RETRIEVE_OK;
}

static int
accessor_header (POUND_HTTP *phttp, char const *arg, int arglen,
		 struct stringbuf *sb)
{
  char const *val;
  size_t len;
  int rc;

  rc = accessor_get_header (phttp, arg, arglen, &val, &len);
  if (rc == RETRIEVE_OK)
    stringbuf_add (sb, val, len);
  return rc;
}

static int
accessor_host (POUND_HTTP *phttp, char const *arg, int arglen,
	       struct stringbuf *sb)
{
  char const *host;
  size_t len;
  int rc = accessor_get_header (phttp, "host", 4, &host, &len);
  if (rc == RETRIEVE_OK)
    {
      char *p;
      if ((p = memchr (host, ':', len)) != NULL)
	len = p - host;
      stringbuf_add (sb, host, len);
    }
  return rc;
}

static int
accessor_port (POUND_HTTP *phttp, char const *arg, int arglen,
	       struct stringbuf *sb)
{
  char const *host;
  size_t len;
  if (accessor_get_header (phttp, "host", 4, &host, &len) == RETRIEVE_OK)
    {
      char *p;
      if ((p = memchr (host, ':', len)) != NULL)
	{
	  stringbuf_add (sb, p, len - (p - host));
	  return RETRIEVE_OK;
	}
    }
  return RETRIEVE_NOT_FOUND;
}

int
accessor_remote_ip (POUND_HTTP *phttp, char const *arg, int arglen,
		    struct stringbuf *sb)
{
  stringbuf_store_ip (sb, phttp, !(arg[0] == '0' && arg[1] == 0));
  return RETRIEVE_OK;
}

static struct accessor accessors[] = {
  { "url",      accessor_url },
  { "path",     accessor_path },
  { "query",    accessor_query },
  { "param",    accessor_param, 1 },
  { "header",   accessor_header, 1 },
  { "host",     accessor_host },
  { "port",     accessor_port },
  { "remoteip", accessor_remote_ip, 1 },
  { NULL }
};

static accessor_func
find_accessor (char const *input, size_t len, char **ret_arg, size_t *ret_arglen)
{
  struct accessor *ap;
  char const *arg = NULL;
  size_t arglen = 0;
  size_t i;

  input = c_trimlws (input, &len);
  if (len == 0)
    return NULL;
  i = c_memcspn (input, CCTYPE_BLANK, len);

  for (ap = accessors; ; ap++)
    {
      if (ap->name == NULL)
	return NULL;
      if (strlen (ap->name) == i && memcmp (ap->name, input, i) == 0)
	break;
    }

  if (ap->arg)
    {
      if (!c_isblank (input[i]))
	return NULL;
      i += c_memspn (input + i, CCTYPE_BLANK, len - i);

      arg = input + i;
      arglen = len - i;

      arglen = c_trimrws (arg, arglen);
      if (arglen == 0)
	return NULL;
    }
  else
    {
      i += c_memspn (input + i, CCTYPE_BLANK, len - i);
      if (i < len)
	return NULL;
      arg = NULL;
      arglen = 0;
    }

  *ret_arg = (char*) arg;
  *ret_arglen = arglen;

  return ap->func;
}

/*
 * Expand backreferences and request accessors in STR, using the submatch
 * queue SMQ and HTTP request REQ.  If the latter is NULL, request accessors
 * are not handled.  Place the result in string buffer SB.  The string
 * WHAT gives the identifier for use in diagnostic messages.
 *
 * On success, returns number of expansions made.  If invalid references or
 * accessors are encountered, returns -1.  The failed references are copied
 * to the output buffer verbatim and error message is logged for each of
 * them.
 *
 * Eventual memory allocation failures are handled by SB.  The caller is
 * supposed to check its error status.
 */
static int
expand_string_to_buffer (struct stringbuf *sb, char const *str,
			 POUND_HTTP *phttp,
			 struct http_request *req, char *what)
{
  char *p;
  char const *start = str; /* Save the string for error reporting. */
  int result = 0; /* Number of expansions made. */

  while (*str)
    {
      int brace;
      size_t len = strcspn (str, "$%");
      stringbuf_add (sb, str, len);
      str += len;
      if (*str == 0)
	break;
      else if ((str[0] == '$' && (str[1] == '$' || str[1] == '%')) ||
	       str[1] == 0)
	{
	  stringbuf_add_char (sb, str[0]);
	  str += 2;
	}
      else if (str[0] == '%' && c_isxdigit (str[1]) && c_isxdigit (str[2]))
	{
	  stringbuf_add (sb, str, 3);
	  str += 3;
	}
      else if (req && str[0] == '%' && str[1] == '[')
	{
	  char *q;
	  size_t len;
	  accessor_func acc;
	  char *arg;
	  size_t arglen;

	  q = strchr (str + 2, ']');
	  if (q == NULL)
	    {
	      logmsg (LOG_WARNING, "%s \"%s\": unclosed %%[ at offset %d",
		      what, start, (int)(str - start));
	      stringbuf_add (sb, str, 2);
	      str += 2;
	      result = -1;
	      continue;
	    }

	  len = q - str;

	  if ((acc = find_accessor (str + 2, len - 2, &arg, &arglen)) == NULL)
	    {
	      stringbuf_add (sb, str, len + 1);
	    }
	  else if (acc (phttp, arg, arglen, sb) == RETRIEVE_OK)
	    {
	      if (result >= 0)
		result++;
	    }

	  str = q + 1;
	}
      else if ((brace = (str[1] == '{')) || c_isdigit (str[1]))
	{
	  long groupno;
	  errno = 0;
	  groupno = strtoul (str + 1 + brace, &p, 10);
	  if (errno)
	    {
	      stringbuf_add (sb, str, 2);
	      str += 2;
	    }
	  else
	    {
	      /*
	       * For compatibility with versions 4.4 - 4.5, %N is taken to
	       * mean $N(1).
	       */
	      int refno = str[0] == '$' ? 0 : 1;
	      struct submatch *sm;

	      if (str[0] == '$' && *p == '(')
		{
		  long n;

		  p++;
		  len = strcspn (p, "()");
		  if (p[len] != ')')
		    {
		      logmsg (LOG_WARNING,
			      "%s \"%s\": missing closing parenthesis in"
			      " reference started at offset %ld",
			      what, start, str - start + 1);
		      p += len;
		      stringbuf_add (sb, str, p - str);
		      str = p;
		      result = -1;
		      continue;
		    }

		  if (c_isdigit (*p))
		    {
		      errno = 0;
		      n = strtoul (p, &p, 10);
		      if (errno || *p != ')')
			{
			  logmsg (LOG_WARNING,
				  "%s \"%s\": invalid reference number "
				  "at offset %ld",
				  what, start, p - start + 1);
			  stringbuf_add (sb, str, p - start);
			  str = p;
			  result = -1;
			  continue;
			}
		    }
		  else
		    {
		      n = submatch_queue_find (&phttp->smq, p, len);
		      p += len;
		    }
		  if (n < 0 || n >= SMQ_SIZE)
		    {
		      int len = p - str + 1;
		      logmsg (LOG_WARNING,
			      "%s \"%s\" refers to non-existing group %*.*s",
			      what, start, len, len, str);
		      stringbuf_add (sb, str, p - str);
		      str = p;
		      result = -1;
		      continue;
		    }
		  refno = n;
		  p++;
		}

	      if (brace)
		{
		  if (*p != '}')
		    {
		      logmsg (LOG_WARNING,
			      "%s \"%s\": missing closing brace in reference"
			      " started at offset %ld",
			      what, start, str - start + 1);
		      stringbuf_add (sb, str, p - str);
		      str = p;
		      result = -1;
		      continue;
		    }
		  p++;
		}

	      sm = submatch_queue_get (&phttp->smq, refno);

	      if (sm->subject && groupno <= sm->matchn)
		{
		  stringbuf_add (sb, sm->subject + sm->matchv[groupno].rm_so,
				 sm->matchv[groupno].rm_eo - sm->matchv[groupno].rm_so);
		  if (result >= 0)
		    result++;
		}
	      else
		{
		  int n = p - str;
		  stringbuf_add (sb, str, n);
		  logmsg (LOG_WARNING,
			  "%s \"%s\" refers to non-existing group %*.*s",
			  what, start, n, n, str);
		  result = -1;
		}
	      str = p;
	    }
	}
      else
	{
	  int n = str - start + 1;
	  logmsg (LOG_WARNING,
		  "%s \"%s\": unescaped %% character at offset %d",
		  what, start, n);
	  stringbuf_add_char (sb, *str);
	  str++;
	  result = -1;
	}
    }
  return result;
}

char *
expand_string (char const *str, POUND_HTTP *phttp, struct http_request *req,
	       char *what)
{
  struct stringbuf sb;
  char *p = NULL;

  stringbuf_init_log (&sb);
  if (expand_string_to_buffer (&sb, str, phttp, req, what) == -1
      || (p = stringbuf_finish (&sb)) == NULL)
    stringbuf_free (&sb);
  return p;
}

char *
expand_string_encode (char const *str, int encode, char const *exc,
		      POUND_HTTP *phttp,
		      struct http_request *req,
		      char *what)
{
  char *s = expand_string (str, phttp, req, what);
  if (s && encode)
    {
      char *enc = urlencode (s, exc);
      free (s);
      s = enc;
    }
  return s;
}

static char *
expand_url (char const *url, POUND_HTTP *phttp, int has_uri)
{
  struct stringbuf sb;
  char *p;

  stringbuf_init_log (&sb);

  switch (expand_string_to_buffer (&sb, url, phttp, &phttp->request,
				   "Redirect expression"))
    {
    case -1:
      stringbuf_free (&sb);
      return NULL;

    case 0:
      break;

    default:
      has_uri = 1;
    }

  /* For compatibility with previous versions */
  if (!has_uri)
    stringbuf_add_string (&sb, phttp->request.url);

  if ((p = stringbuf_finish (&sb)) == NULL)
    stringbuf_free (&sb);
  return p;
}

typedef int (*PNDLUA_FUN) (POUND_HTTP *phttp,
			   struct pndlua_closure const *clos,
			   char **argv, void *data);

static int
pndlua_apply (PNDLUA_FUN plfun, POUND_HTTP *phttp, struct http_request *req,
	      struct pndlua_closure const *clos, void *data)
{
  int i;
  char **argv;
  int res = -1;

  argv = calloc (clos->argc, sizeof (argv[0]));
  if (!argv)
    {
      lognomem ();
      return -1;
    }

  for (i = 0; i < clos->argc; i++)
    {
      argv[i] = expand_string (clos->argv[i], phttp, req, "lua");
      if (argv[i] == 0)
	goto err;
    }

  res = plfun (phttp, clos, argv, data);

 err:
  for (i = 0; i < clos->argc && argv[i]; i++)
    free (argv[i]);
  free (argv);

  return res;
}

static int rewrite_apply (POUND_HTTP *phttp, struct http_request *request,
			  int what);

static int
control_response (POUND_HTTP *phttp)
{
  if (rewrite_apply (phttp, &phttp->request, REWRITE_REQUEST))
    return HTTP_STATUS_INTERNAL_SERVER_ERROR;
  return control_response_basic (phttp);
}
/*
 * Reply with a redirect
 */
static int
redirect_response (POUND_HTTP *phttp)
{
  struct be_redirect const *redirect = &phttp->backend->v.redirect;
  int code = redirect->status;
  char const *code_msg, *cont, *hdr;
  char *xurl, *url;
  struct stringbuf sb_url, sb_cont, sb_loc;
  int i;

  /*
   * Notice: the codes below must be in sync with the ones accepted
   * by the assign_redirect function in config.c
   */
  switch (code)
    {
    case 301:
      code_msg = "Moved Permanently";
      break;

    case 302:
      code_msg = "Found";
      break;

    case 303:
      code_msg = "See Other";
      break;

    case 307:
      code_msg = "Temporary Redirect";
      break;

    case 308:
      code_msg = "Permanent Redirect";
      break;

    default:
      logmsg (LOG_ERR,
	      "INTERNAL ERROR: unsupported status code %d passed to"
	      " redirect_response; please report", code);
      return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

  if (rewrite_apply (phttp, &phttp->request, REWRITE_REQUEST))
    return HTTP_STATUS_INTERNAL_SERVER_ERROR;

  xurl = expand_url (redirect->url, phttp, redirect->has_uri);
  if (!xurl)
    {
      return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

  /*
   * Make sure to return a safe version of the URL (otherwise CSRF
   * becomes a possibility)
   *
   * FIXME: 1. This should be optional.
   *        2. Use urlencode or http_request_split/http_request_rebuild to
   *           do that.
   */
  stringbuf_init_log (&sb_url);
  for (i = 0; xurl[i]; i++)
    {
      if (c_isalnum (xurl[i]) || xurl[i] == '_' || xurl[i] == '.'
	  || xurl[i] == ':' || xurl[i] == '/' || xurl[i] == '?'
	  || xurl[i] == '&' || xurl[i] == ';' || xurl[i] == '-'
	  || xurl[i] == '=' || xurl[i] == '+')
	stringbuf_add_char (&sb_url, xurl[i]);
      else
	stringbuf_printf (&sb_url, "%%%02x", xurl[i]);
    }
  url = stringbuf_finish (&sb_url);
  free (xurl);

  if (!url)
    {
      stringbuf_free (&sb_url);
      return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

  stringbuf_init_log (&sb_cont);
  stringbuf_printf (&sb_cont,
		    "<html><head><title>Redirect</title></head>"
		    "<body><h1>Redirect</h1>"
		    "<p>You should go to <a href=\"%s\">%s</a></p>"
		    "</body></html>",
		    url, url);

  if ((cont = stringbuf_finish (&sb_cont)) == NULL)
    {
      stringbuf_free (&sb_url);
      stringbuf_free (&sb_cont);
      return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

  stringbuf_init_log (&sb_loc);
  stringbuf_printf (&sb_loc, "Location: %s\r\n", url);

  if ((hdr = stringbuf_finish (&sb_loc)) == NULL)
    {
      stringbuf_free (&sb_url);
      stringbuf_free (&sb_cont);
      stringbuf_free (&sb_loc);
      return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

  bio_http_reply (phttp->cl, phttp->request.version, code, code_msg, hdr,
		  "text/html", cont);

  stringbuf_free (&sb_url);
  stringbuf_free (&sb_cont);
  stringbuf_free (&sb_loc);

  phttp->response_code = code;

  return HTTP_STATUS_OK;
}

enum
  {
    COPY_OK,
    COPY_EOF,
    COPY_READ_ERR,
    COPY_WRITE_ERR,
    COPY_BAD_DATA,
    COPY_TOO_LONG
  };

static char const *
copy_status_string (int rc)
{
  switch (rc)
    {
    case COPY_OK:
      return "no error";

    case COPY_EOF:
      return "unexpected end of input";

    case COPY_READ_ERR:
      return "read error";

    case COPY_WRITE_ERR:
      return "write error";

    case COPY_BAD_DATA:
      return "malformed data";

    case COPY_TOO_LONG:
      return "line too long";

    default:
      return "unknown error";
    }
}

/*
 * Read and write some binary data
 */
static int
copy_bin (BIO *cl, BIO *be, CONTENT_LENGTH cont, int no_write,
	  CONTENT_LENGTH *res_bytes)
{
  char buf[MAXBUF];
  int res;

  while (cont > 0)
    {
      if ((res = BIO_read (cl, buf,
			   cont > sizeof (buf) ? sizeof (buf) : cont)) < 0)
	{
	  if (BIO_should_retry (cl))
	    continue;
	  return COPY_READ_ERR;
	}
      else if (res == 0)
	return COPY_EOF;
      if (!no_write)
	{
	  int rc;

	  while ((rc = BIO_write (be, buf, res)) < 0)
	    {
	      if (!BIO_should_retry (be))
		return COPY_WRITE_ERR;
	    }
	}
      cont -= res;
      if (res_bytes)
	*res_bytes += res;
    }
  if (!no_write)
    if (BIO_flush (be) != 1)
      return COPY_WRITE_ERR;
  return COPY_OK;
}

static int
acme_response (POUND_HTTP *phttp)
{
  int fd;
  struct stat st;
  BIO *bin;
  char *file_name;
  int rc = HTTP_STATUS_OK;

  if (rewrite_apply (phttp, &phttp->request, REWRITE_REQUEST))
    return HTTP_STATUS_INTERNAL_SERVER_ERROR;

  file_name = expand_url ("$1", phttp, 1);

  if ((fd = openat (phttp->backend->v.acme.wd, file_name, O_RDONLY)) == -1)
    {
      if (errno == ENOENT)
	{
	  rc = HTTP_STATUS_NOT_FOUND;
	}
      else
	{
	  rc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
	  phttp_log (phttp, PHTTP_LOG_DFL,
		     rc, errno, "can't open %s", file_name);
	}
    }
  else if (fstat (fd, &st))
    {
      rc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
      phttp_log (phttp, PHTTP_LOG_DFL,
		 rc, errno, "can't stat %s", file_name);
      close (fd);
    }
  else
    {
      int ec;

      bin = BIO_new_fd (fd, BIO_CLOSE);

      phttp->response_code = 200;
      bio_http_reply_start (phttp->cl, phttp->request.version, 200, "OK", NULL,
			    "text/html", (CONTENT_LENGTH) st.st_size);
      ec = copy_bin (bin, phttp->cl, st.st_size, 0, NULL);
      switch (ec)
	{
	case COPY_OK:
	  break;

	case COPY_READ_ERR:
	case COPY_WRITE_ERR:
	  rc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
	  phttp_log (phttp, PHTTP_LOG_DFL,
		     rc, errno,
		     "%s while sending file %s",
		     copy_status_string (ec), file_name);
	  break;

	default:
	  rc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
	  phttp_log (phttp, PHTTP_LOG_DFL,
		     rc, 0,
		     "%s while sending file %s",
		     copy_status_string (ec), file_name);
	}

      BIO_free (bin);
      BIO_flush (phttp->cl);
    }
  free (file_name);
  return rc;
}

int
http_header_list_parse (HTTP_HEADER_LIST *head, char const *text,
			int replace, char **end)
{
  struct stringbuf sb;
  int res = 0;

  stringbuf_init_log (&sb);
  while (*text)
    {
      char *hdr;
      int len = strcspn (text, "\r\n");

      if (len == 0)
	break;

      stringbuf_reset (&sb);
      stringbuf_add (&sb, text, len);
      if ((hdr = stringbuf_finish (&sb)) == NULL ||
	  http_header_list_append (head, hdr, replace))
	{
	  res = -1;
	  break;
	}

      text += len;
      if (*text == '\r')
	text++;
      if (*text == '\n')
	text++;
    }
  stringbuf_free (&sb);
  if (end)
    *end = (char*) text;
  return res;
}

static int
cb_hdr_rewrite (HTTP_HEADER_LIST *hlist, void *data)
{
  POUND_HTTP *phttp = data;
  struct http_request req;
  int rc = 0;

  http_request_init (&req);
  req.headers = *hlist;
  if (rewrite_apply (phttp, &req, REWRITE_RESPONSE))
    rc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
  *hlist = req.headers;
  DLIST_INIT (&req.headers);
  http_request_free (&req);
  return rc;
}

static int
error_response (POUND_HTTP *phttp)
{
  int err = phttp->backend->v.error.status;
  struct http_errmsg *ep;

  if (rewrite_apply (phttp, &phttp->request, REWRITE_REQUEST))
    return HTTP_STATUS_INTERNAL_SERVER_ERROR;

  if (phttp->backend->v.error.msg.text)
    ep = &phttp->backend->v.error.msg;
  else if (phttp->lstn->http_err[err])
    ep = phttp->lstn->http_err[err];
  else
    ep = NULL;

  return http_err_reply_cb (phttp, err, ep, cb_hdr_rewrite, phttp);
}

static char file_headers[] = "Content-Type: text/plain\r\n";

static int
file_response (POUND_HTTP *phttp)
{
  char const *file_name;
  struct http_request req;
  BIO *bin;
  int rc, fd;
  struct stat st;

  if (rewrite_apply (phttp, &phttp->request, REWRITE_REQUEST))
    return HTTP_STATUS_INTERNAL_SERVER_ERROR;

  http_request_init (&req);
  if (http_header_list_parse (&req.headers, file_headers, H_REPLACE, NULL))
    return HTTP_STATUS_INTERNAL_SERVER_ERROR;

  if (rewrite_apply (phttp, &req, REWRITE_RESPONSE))
    return HTTP_STATUS_INTERNAL_SERVER_ERROR;

  if (http_request_get_path (&phttp->request, &file_name))
    {
      http_request_free (&req);
      return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }
  if (*file_name == '/')
    file_name++;
  if (!*file_name)
    rc = HTTP_STATUS_NOT_FOUND;

  else if ((fd = openat (phttp->backend->v.file.wd, file_name, O_RDONLY)) == -1)
    {
      if (errno == ENOENT)
	{
	  rc = HTTP_STATUS_NOT_FOUND;
	}
      else
	{
	  rc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
	  phttp_log (phttp, PHTTP_LOG_DFL,
		     rc, errno, "can't open %s", file_name);
	}
    }
  else if (fstat (fd, &st))
    {
      rc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
      phttp_log (phttp, PHTTP_LOG_DFL,
		 rc, errno, "can't stat %s", file_name);
      close (fd);
    }
  else
    {
      bin = BIO_new_fd (fd, BIO_CLOSE);
      bio_http_reply_start_list (phttp->cl,
				 phttp->request.version,
				 http_status[HTTP_STATUS_OK].code,
				 http_status[HTTP_STATUS_OK].reason,
				 &req.headers,
				 (CONTENT_LENGTH) st.st_size);
      rc = copy_bin (bin, phttp->cl, st.st_size, 0, NULL);
      switch (rc)
	{
	case COPY_OK:
	  break;

	case COPY_READ_ERR:
	case COPY_WRITE_ERR:
	  rc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
	  phttp_log (phttp, PHTTP_LOG_DFL,
		     rc, errno,
		     "%s while sending file %s",
		     copy_status_string (rc), file_name);
	  break;

	default:
	  rc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
	  phttp_log (phttp, PHTTP_LOG_DFL,
		     rc, 0,
		     "%s while sending file %s",
		     copy_status_string (rc), file_name);
	}

      BIO_free (bin);
      BIO_flush (phttp->cl);
      rc = HTTP_STATUS_OK;
    }

  http_request_free (&req);

  return rc;
}

static int
http_response_ok (POUND_HTTP *phttp)
{
  return phttp->response_code >= 100 && phttp->response_code <= 599;
}

static int
lua_response (POUND_HTTP *phttp)
{
  int res;

  if (rewrite_apply (phttp, &phttp->request, REWRITE_REQUEST))
    return HTTP_STATUS_INTERNAL_SERVER_ERROR;

  res = pndlua_apply (pndlua_backend, phttp, &phttp->request,
		      &phttp->backend->v.lua, NULL);
  if (res != -1)
    {
      /* Verify response. */
      if (!http_response_ok (phttp))
	{
	  phttp_log (phttp, PHTTP_LOG_BACKEND, -1, 0, "malformed response");
	  res = -1;
	}
      else
	{
	  int n;
	  struct stringbuf *body;
	  char const *reason = phttp->response.request;
	  if (reason && *reason)
	    {
	      if (c_isdigit (reason[0]) &&
		  c_isdigit (reason[1]) &&
		  c_isdigit (reason[2]) &&
		  c_isblank (reason[3]))
		{
		  reason = c_trimlws (reason + 4, NULL);
		}
	    }
	  else if ((n = http_status_to_pound (phttp->response_code)) != -1)
	    reason = http_status[n].reason;
	  else
	    reason = "";

	  body = phttp->response.body;
	  phttp->res_bytes = body ? stringbuf_len (body) : 0;

	  if (rewrite_apply (phttp, &phttp->response, REWRITE_RESPONSE))
	    return HTTP_STATUS_INTERNAL_SERVER_ERROR;

	  bio_http_reply_start_list (phttp->cl,
				     phttp->request.version,
				     phttp->response_code,
				     reason,
				     &phttp->response.headers,
				     (CONTENT_LENGTH) phttp->res_bytes);
	  if (body)
	    BIO_write (phttp->cl, stringbuf_value (body), phttp->res_bytes);
	  BIO_flush (phttp->cl);

	  res = HTTP_STATUS_OK;
	}
    }

  return res;

}

static void
drain_eol (BIO *in)
{
  char c;
  do
    {
      if (BIO_read (in, &c, 1) <= 0)
	{
	  if (BIO_should_retry (in))
	    continue;
	  break;
	}
    }
  while (c != '\n');
}

/*
 * Get a "line" from a BIO, strip the trailing newline, skip the input
 * stream if buffer too small.
 * The result buffer is 0-terminated.
 * Return 0 on success.
 */
static int
get_line (BIO *in, char *const buf, int bufsize)
{
  char tmp;
  int i, seen_cr;

  memset (buf, 0, bufsize);
  for (i = 0, seen_cr = 0; i < bufsize - 1; i++)
    switch (BIO_read (in, &tmp, 1))
      {
      case -2:
	/*
	 * BIO_gets not implemented
	 */
	return COPY_READ_ERR;
      case 0:
	if (BIO_should_retry (in))
	  continue;
	return i == 0 ? COPY_EOF : COPY_OK;
      case -1:
	return COPY_READ_ERR;
      default:
	if (seen_cr)
	  {
	    if (tmp != '\n')
	      {
		/*
		 * we have CR not followed by NL
		 */
		drain_eol (in);
		return COPY_BAD_DATA;
	      }
	    else
	      {
		buf[i - 1] = '\0';
		return 0;
	      }
	  }

	if (!c_iscntrl (tmp) || tmp == '\t')
	  {
	    buf[i] = tmp;
	    continue;
	  }

	if (tmp == '\r')
	  {
	    seen_cr = 1;
	    continue;
	  }

	if (tmp == '\n')
	  {
	    /*
	     * line ends in NL only (no CR)
	     */
	    buf[i] = 0;
	    return 0;
	  }

	/*
	 * all other control characters cause an error
	 */
	drain_eol (in);
	return COPY_BAD_DATA;
      }

  /*
   * line too long
   */
  drain_eol (in);
  return COPY_TOO_LONG;
}

/*
 * Strip trailing CRLF
 */
static int
strip_eol (char *lin)
{
  while (*lin)
    if (*lin == '\n' || (*lin == '\r' && *(lin + 1) == '\n'))
      {
	*lin = '\0';
	return 1;
      }
    else
      lin++;
  return 0;
}

/*
 * Convert string ARG into numeric value of type CONTENT_LENGTH.
 * Input:
 *   arg    -   string to convert
 *   base   -   conversion base: 10 or 16
 * Output:
 *   retval -  converted value
 *   endptr -  if not NULL, pointer to the character in arg at which the
 *             conversion stopped,
 * Return value:
 *   0      - success
 *  -1      - invalid input (including overflow)
 */
int
strtoclen (char const *arg, int base, CONTENT_LENGTH *retval, char **endptr)
{
  int c;
  int rc = 0;
  char const *s = arg;
  CONTENT_LENGTH val = 0;
  CONTENT_LENGTH cutoff = CONTENT_LENGTH_MAX / (CONTENT_LENGTH) base;
  CONTENT_LENGTH cutlim = CONTENT_LENGTH_MAX % (CONTENT_LENGTH) base;

  for (c = *s; c != '\0'; c = *++s)
    {
      if (c >= '0' && c <= '9')
	c -= '0';
      else if (base == 16)
	{
	  if (c >= 'a' && c <= 'f')
	    c -= 'a' - 10;
	  else if (c >= 'A' && c <= 'F')
	    c -= 'A' - 10;
	  else
	    break;
	}
      else
	break;
      if (val > cutoff || (val == cutoff && c > cutlim))
	{
	  /* Overflow. */
	  rc = -1;
	  break;
	}
      else
	{
	  val *= (CONTENT_LENGTH) base;
	  val += c;
	}
    }

  if (rc == 0)
    {
      if (s == arg)
	rc = -1;
      else if (val < 0)
	rc = -1;
    }
  *retval = val;
  if (endptr)
    *endptr = (char*)s;
  return rc;
}

enum
  {
    CL_HEADER,
    CL_CHUNK
  };

CONTENT_LENGTH
get_content_length (char const *arg, int mode)
{
  char *p;
  CONTENT_LENGTH n;

  if (mode == CL_HEADER)
    arg = c_trimlws (arg, NULL);

  if (strtoclen (arg, mode == CL_HEADER ? 10 : 16, &n, &p))
    return NO_CONTENT_LENGTH;
  p = (char*) c_trimlws (p, NULL);
  if (*p)
    {
      if (!(mode == CL_CHUNK && *p == ';'))
	return NO_CONTENT_LENGTH;
    }

  return n;
}

/*
 * Copy chunked
 */
static int
copy_chunks (BIO *cl, BIO *be, char *buf, unsigned bufsize,
	     int no_write,
	     CONTENT_LENGTH max_size, CONTENT_LENGTH *res_bytes)
{
  CONTENT_LENGTH cont, tot_size;
  int res;

  for (tot_size = 0;;)
    {
      if ((res = get_line (cl, buf, bufsize)) != COPY_OK)
	{
	  logmsg (LOG_NOTICE, "(%"PRItid") chunked read error: %s",
		  POUND_TID (),
		  copy_status_string (res));
	  return res == COPY_READ_ERR
	    ? HTTP_STATUS_INTERNAL_SERVER_ERROR
	    : HTTP_STATUS_BAD_REQUEST;
	}

      if ((cont = get_content_length (buf, CL_CHUNK)) == NO_CONTENT_LENGTH)
	{
	  /*
	   * not chunk header
	   */
	  logmsg (LOG_NOTICE, "(%"PRItid") bad chunk header <%s>: %s",
		  POUND_TID (), buf, strerror (errno));
	  return HTTP_STATUS_BAD_REQUEST;
	}
      if (!no_write)
	if (BIO_printf (be, "%s\r\n", buf) <= 0)
	  {
	    logmsg (LOG_NOTICE, "(%"PRItid") error write chunked: %s",
		    POUND_TID (), strerror (errno));
	    return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	  }

      tot_size += cont;
      if (max_size > 0 && tot_size > max_size)
	{
	  logmsg (LOG_WARNING, "(%"PRItid") chunk content too large",
		  POUND_TID ());
	  return HTTP_STATUS_PAYLOAD_TOO_LARGE;
	}

      if (cont > 0)
	{
	  int ec = copy_bin (cl, be, cont, no_write, res_bytes);
	  switch (ec)
	    {
	    case COPY_OK:
	      break;

	    case COPY_READ_ERR:
	    case COPY_WRITE_ERR:
	      if (errno)
		logmsg (LOG_NOTICE,
			"(%"PRItid") %s while copying chunk of length %"PRICLEN": %s",
			POUND_TID (), copy_status_string (ec), cont,
			strerror (errno));
	      else
		logmsg (LOG_NOTICE,
			"(%"PRItid") %s while copying chunk of length %"PRICLEN,
			POUND_TID (), copy_status_string (ec), cont);
	      return HTTP_STATUS_INTERNAL_SERVER_ERROR;

	    default:
	      logmsg (LOG_NOTICE,
		      "(%"PRItid") %s while copying chunk of length %"PRICLEN,
			  POUND_TID (), copy_status_string (ec), cont);
	      return HTTP_STATUS_BAD_REQUEST;
	    }
	}
      else
	break;
      /*
       * final CRLF
       */
      if ((res = get_line (cl, buf, bufsize)) != COPY_OK)
	{
	  logmsg (LOG_NOTICE, "(%"PRItid") error after chunk: %s",
		  POUND_TID (),
		  copy_status_string (res));
	  return res == COPY_READ_ERR
	    ? HTTP_STATUS_INTERNAL_SERVER_ERROR
	    : HTTP_STATUS_BAD_REQUEST;
	}

      if (buf[0])
	{
	  logmsg (LOG_NOTICE, "(%"PRItid") unexpected after chunk \"%s\"",
		  POUND_TID (), buf);
	  return HTTP_STATUS_BAD_REQUEST;
	}
      if (!no_write)
	if (BIO_printf (be, "%s\r\n", buf) <= 0)
	  {
	    logmsg (LOG_NOTICE, "(%"PRItid") error after chunk write: %s",
		    POUND_TID (), strerror (errno));
	    return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	  }
    }
  /*
   * possibly trailing headers
   */
  for (;;)
    {
      if ((res = get_line (cl, buf, bufsize)) != COPY_OK)
	{
	  logmsg (LOG_NOTICE, "(%"PRItid") error post-chunk: %s",
		  POUND_TID (),
		  copy_status_string (res));
	  return res == COPY_READ_ERR
	    ? HTTP_STATUS_INTERNAL_SERVER_ERROR
	    : HTTP_STATUS_BAD_REQUEST;
	}

      if (!no_write)
	if (BIO_printf (be, "%s\r\n", buf) <= 0)
	  {
	    logmsg (LOG_NOTICE, "(%"PRItid") error post-chunk write: %s",
		    POUND_TID (), strerror (errno));
	    return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	  }
      if (!buf[0])
	break;
    }
  if (!no_write)
    if (BIO_flush (be) != 1)
      {
	logmsg (LOG_NOTICE, "(%"PRItid") copy_chunks flush error: %s",
		POUND_TID (), strerror (errno));
	return HTTP_STATUS_INTERNAL_SERVER_ERROR;
      }
  return HTTP_STATUS_OK;
}

static int err_to = -1;

typedef struct
{
  int timeout;
  RENEG_STATE *reneg_state;
} BIO_ARG;

/*
 * Time-out for client read/gets
 * the SSL manual says not to do it, but it works well enough anyway...
 */
static long
bio_callback
#if OPENSSL_VERSION_MAJOR >= 3
(BIO *bio, int cmd, const char *argp, size_t len, int argi,
 long argl, int ret, size_t *processed)
#else
(BIO *bio, int cmd, const char *argp, int argi, long argl, long ret)
#endif
{
  BIO_ARG *bio_arg;
  struct pollfd p;
  int to, p_res, p_err;

  if ((bio_arg = (BIO_ARG *) BIO_get_callback_arg (bio)) == NULL)
    return ret;

  if (cmd == BIO_CB_FREE)
    {
      free (bio_arg);
      return ret;
    }

  if (cmd != BIO_CB_READ && cmd != BIO_CB_WRITE)
    return ret;

  /*
   * a time-out already occurred
   */
  if ((to = bio_arg->timeout * 1000) < 0)
    {
      errno = ETIMEDOUT;
      return -1;
    }

  /*
   * Renegotiations
   */
  /*
   * logmsg(LOG_NOTICE, "RENEG STATE %d",
   * bio_arg->reneg_state==NULL?-1:*bio_arg->reneg_state);
   */
  if (bio_arg->reneg_state != NULL && *bio_arg->reneg_state == RENEG_ABORT)
    {
      logmsg (LOG_NOTICE, "REJECTING renegotiated session");
      errno = ECONNABORTED;
      return -1;
    }

  if (to == 0)
    return ret;

  for (;;)
    {
      memset (&p, 0, sizeof (p));
      BIO_get_fd (bio, &p.fd);
      p.events = (cmd == BIO_CB_READ) ? (POLLIN | POLLPRI) : POLLOUT;
      p_res = poll (&p, 1, to);
      p_err = errno;
      switch (p_res)
	{
	case 1:
	  if (cmd == BIO_CB_READ)
	    {
	      if ((p.revents & POLLIN) || (p.revents & POLLPRI))
		/*
		 * there is readable data
		 */
		return ret;
	      else
		{
#ifdef  EBUG
		  logmsg (LOG_WARNING, "(%"PRItid") CALLBACK read 0x%04x poll: %s",
			  POUND_TID (), p.revents, strerror (p_err));
#endif
		  errno = EIO;
		}
	    }
	  else
	    {
	      if (p.revents & POLLOUT)
		/*
		 * data can be written
		 */
		return ret;
	      else
		{
#ifdef  EBUG
		  logmsg (LOG_WARNING, "(%"PRItid") CALLBACK write 0x%04x poll: %s",
			  POUND_TID (), p.revents, strerror (p_err));
#endif
		  errno = ECONNRESET;
		}
	    }
	  return -1;

	case 0:
	  /*
	   * timeout - mark the BIO as unusable for the future
	   */
	  bio_arg->timeout = err_to;
#ifdef  EBUG
	  logmsg (LOG_WARNING,
		  "(%"PRItid") CALLBACK timeout poll after %d secs: %s",
		  POUND_TID (), to / 1000, strerror (p_err));
#endif
	  errno = ETIMEDOUT;
	  return 0;

	default:
	  /*
	   * error
	   */
	  if (p_err != EINTR)
	    {
#ifdef  EBUG
	      logmsg (LOG_WARNING, "(%"PRItid") CALLBACK bad %d poll: %s",
		      POUND_TID (), p_res, strerror (p_err));
#endif
	      return -2;
#ifdef  EBUG
	    }
	  else
	    logmsg (LOG_WARNING, "(%"PRItid") CALLBACK interrupted %d poll: %s",
		    POUND_TID (), p_res, strerror (p_err));
#else
	    }
#endif
	}
    }
}

static void
set_callback (BIO *cl, int timeout, RENEG_STATE *state)
{
  BIO_ARG *arg = malloc (sizeof (*arg));
  if (!arg)
    {
      lognomem ();
      return;
    }

  arg->timeout = timeout;
  arg->reneg_state = state;

  BIO_set_callback_arg (cl, (char *) arg);
#if OPENSSL_VERSION_MAJOR >= 3
  BIO_set_callback_ex (cl, bio_callback);
#else
  BIO_set_callback (cl, bio_callback);
#endif
}

/*
 * Check if the file underlying a BIO is readable
 */
static int
is_readable (BIO *bio, int to_wait)
{
  struct pollfd p;

  if (BIO_pending (bio) > 0)
    return 1;
  memset (&p, 0, sizeof (p));
  BIO_get_fd (bio, &p.fd);
  p.events = POLLIN | POLLPRI;
  return (poll (&p, 1, to_wait * 1000) > 0);
}

struct method_def
{
  char const *name;
  size_t length;
  int meth;
  int group;
};

static struct method_def methods[] = {
#define S(s) s, sizeof(s)-1
  { S("GET"),          METH_GET,           0 },
  { S("POST"),         METH_POST,          0 },
  { S("HEAD"),         METH_HEAD,          0 },
  { S("PUT"),          METH_PUT,           1 },
  { S("PATCH"),        METH_PATCH,         1 },
  { S("DELETE"),       METH_DELETE,        1 },
  { S("LOCK"),         METH_LOCK,          2 },
  { S("UNLOCK"),       METH_UNLOCK,        2 },
  { S("PROPFIND"),     METH_PROPFIND,      2 },
  { S("PROPPATCH"),    METH_PROPPATCH,     2 },
  { S("SEARCH"),       METH_SEARCH,        2 },
  { S("MKCOL"),        METH_MKCOL,         2 },
  { S("MOVE"),         METH_MOVE,          2 },
  { S("COPY"),         METH_COPY,          2 },
  { S("OPTIONS"),      METH_OPTIONS,       2 },
  { S("TRACE"),        METH_TRACE,         2 },
  { S("MKACTIVITY"),   METH_MKACTIVITY,    2 },
  { S("CHECKOUT"),     METH_CHECKOUT,      2 },
  { S("MERGE"),        METH_MERGE,         2 },
  { S("REPORT"),       METH_REPORT,        2 },
  { S("SUBSCRIBE"),    METH_SUBSCRIBE,     3 },
  { S("UNSUBSCRIBE"),  METH_UNSUBSCRIBE,   3 },
  { S("BPROPPATCH"),   METH_BPROPPATCH,    3 },
  { S("POLL"),         METH_POLL,          3 },
  { S("BMOVE"),        METH_BMOVE,         3 },
  { S("BCOPY"),        METH_BCOPY,         3 },
  { S("BDELETE"),      METH_BDELETE,       3 },
  { S("BPROPFIND"),    METH_BPROPFIND,     3 },
  { S("NOTIFY"),       METH_NOTIFY,        3 },
  { S("CONNECT"),      METH_CONNECT,       3 },
#undef S
  { NULL }
};

static struct method_def *
find_method (const char *str, int len)
{
  struct method_def *m;

  for (m = methods; m->name; m++)
    {
      if (len == m->length && c_strncasecmp (m->name, str, m->length) == 0)
	return m;
    }
  return NULL;
}

char const *
method_name (int meth)
{
  if (meth < 0 || meth >= sizeof (methods) / sizeof (methods[0]))
    return "BAD";
  return methods[meth].name;
}

/*
 * Table of headers whose value is a comma-separated list, and which
 * therefore are allowed to occur multiple times in the same message.
 *
 * RFC 7230, sect. 3.2.2
 *  A sender MUST NOT generate multiple header fields with the same field
 *  name in a message unless either the entire field value for that
 *  header field is defined as a comma-separated list [i.e., #(values)]
 *  or the header field is a well-known exception (as noted below).
 *
 *  A recipient MAY combine multiple header fields with the same field
 *  name into one "field-name: field-value" pair, without changing the
 *  semantics of the message, by appending each subsequent field value to
 *  the combined field value in order, separated by a comma.
 *
 * Headers from that table are rebuilt as described above in order to
 * simplify header matching.
 */
typedef struct
{
  char *name;
  size_t len;
} HEADER_NAME;

/*
 * A modified version of strhash function from OpenSSL.
 */
unsigned long
strhash_ci (const char *c, size_t len)
{
  unsigned long ret = 0;
  long n;
  unsigned long v;
  int r;

  if ((c == NULL) || (*c == '\0'))
    return ret;

  n = 0x100;
  while (len--)
    {
      v = n | c_tolower (*c);
      n += 0x100;
      r = (int)((v >> 2) ^ v) & 0x0f;
      /* cast to uint64_t to avoid 32 bit shift of 32 bit value */
      ret = (ret << r) | (unsigned long)((uint64_t)ret >> (32 - r));
      ret &= 0xFFFFFFFFL;
      ret ^= v * v;
      c++;
    }
  return (ret >> 16) ^ ret;
}

static unsigned long
HEADER_NAME_hash (const HEADER_NAME *hname)
{
  return strhash_ci (hname->name, hname->len);
}

static int
HEADER_NAME_cmp (const HEADER_NAME *a, const HEADER_NAME *b)
{
  return a->len != b->len || c_strncasecmp (a->name, b->name, a->len);
}

#define HT_TYPE HEADER_NAME
#define HT_TYPE_HASH_FN_DEFINED 1
#define HT_TYPE_CMP_FN_DEFINED 1
#define HT_NO_FOREACH
#define HT_NO_HASH_FREE
#define HT_NO_DELETE
#include "ht.h"

static HEADER_NAME_HASH *combinable_headers;

/* Add header to the multi-value header table. */
void
combinable_header_add (char const *name)
{
  HEADER_NAME *hname;

  if (!combinable_headers)
    {
      if ((combinable_headers = HEADER_NAME_HASH_NEW ()) == NULL)
	xnomem ();
    }
  XZALLOC (hname);
  hname->name = xstrdup (name);
  hname->len = strlen (name);
  if (HEADER_NAME_INSERT (combinable_headers, hname))
    {
      free (hname->name);
      free (hname);
    }
}

/* Return true if the name given is a name of a combinble multi-value header. */
int
is_combinable_header (struct http_header *hdr)
{
  HEADER_NAME key;
  if (!combinable_headers)
    return 0;
  key.name = (char*) http_header_name_ptr (hdr);
  key.len = http_header_name_len (hdr);
  return HEADER_NAME_RETRIEVE (combinable_headers, &key) != NULL;
}

/*
 * Return length of the quoted string beginning at FSTR[0], scanning at
 * most FLEN bytes.  The returned length includes both initial and closing
 * double-quote (if such is present).
 */
size_t
field_string_len (char const *fstr, size_t flen)
{
  size_t i;
  for (i = 1; i < flen; i++)
    {
      if (fstr[i] == '"')
	{
	  i++;
	  break;
	}
      else if (fstr[i] == '\\')
	{
	  i++;
	  if (i == flen)
	    break;
	}
    }
  return i;
}

/*
 * Return length of the initial segment of FSTR ending with an unquoted ','.
 * Scan at most FLEN bytes.
 */
size_t
field_segment_len (char const *fstr, size_t flen)
{
  size_t i;
  for (i = 0; i < flen; )
    {
      if (fstr[i] == ',')
	break;
      else if (fstr[i] == '"')
	i += field_string_len (fstr, flen);
      else
	i++;
    }
  return i;
}

/*
 * Apply PRED to each non-empty element in comma-separated list of values HVAL.
 * Continue until HLEN bytes have been processed or PRED returned non-0,
 * whichever happens first.  Return the last value returned by PRED.
 * If ENDP is not NULL, store there the pointer to the last element processed
 * by PRED.
 *
 * The function is called as:
 *
 *       PRED(ELT, LEN, DATA)
 *
 * where ELT is a pointer to the list element with leading whitespace removed,
 * LEN is its length (not counting trailing whitespace), and DATA is the DATA
 * argument passed to field_list_filter.
 */
int
field_list_filter (char const *hval, size_t hlen,
		   int (*pred) (char const *, size_t, void *),
		   void *data,
		   char **endp)
{
  int retval = 0;
  char *last = NULL;

  do
    {
      size_t n = c_memspn (hval, CCTYPE_BLANK, hlen);
      hval += n;
      hlen -= n;
      if (hlen == 0)
	break;
      n = field_segment_len (hval, hlen);
      if (n > 0)
	{
	  size_t i = n;

	  last = c_trimws (hval, &i);

	  hval += n;
	  hlen -= n;

	  retval = pred (last, i, data);
	}
      if (hlen > 0)
	{
	  hval++;
	  hlen--;
	}
    }
  while (hlen > 0 && retval == 0);

  if (endp)
    *endp = last;

  return retval;
}

struct token_closure
{
  char const *token;
  size_t toklen;
};

static int
field_token_casecmp (char const *str, size_t len, void *data)
{
  struct token_closure *cp = data;
  return len == cp->toklen && c_strncasecmp (str, cp->token, len) == 0;
}

static char *
cs_locate_token (char const *subj, char const *tok, char **nextp)
{
  struct token_closure tc;
  char *found;
  size_t len = strlen(subj);

  tc.token = tok;
  tc.toklen = strlen (tok);
  if (field_list_filter (subj, len, field_token_casecmp,
			 &tc, &found))
    {
      if (nextp)
	{
	  char *p = strchr (found + tc.toklen, ',');
	  if (p)
	    {
	      p++;
	      p += c_memspn (p, CCTYPE_BLANK, strlen (p));
	    }
	  *nextp = p;
	}
      return found;
    }
  return NULL;
}

static int
qualify_header (struct http_header *hdr)
{
  POUND_REGMATCH matches[4];
  static struct
  {
    char const *header;
    int len;
    int val;
  } hd_types[] = {
#define S(s) s, sizeof (s) - 1
    { S ("Transfer-encoding"), HEADER_TRANSFER_ENCODING },
    { S ("Content-length"),    HEADER_CONTENT_LENGTH },
    { S ("Connection"),        HEADER_CONNECTION },
    { S ("Location"),          HEADER_LOCATION },
    { S ("Content-location"),  HEADER_CONTLOCATION },
    { S ("Host"),              HEADER_HOST },
    { S ("Referer"),           HEADER_REFERER },
    { S ("User-agent"),        HEADER_USER_AGENT },
    { S ("Destination"),       HEADER_DESTINATION },
    { S ("Expect"),            HEADER_EXPECT },
    { S ("Upgrade"),           HEADER_UPGRADE },
    { S ("Authorization"),     HEADER_AUTHORIZATION },
    { S (""),                  HEADER_OTHER },
#undef S
  };
  int i;

  if (genpat_match (HEADER, hdr->header, 4, matches) == 0)
    {
      hdr->name_start = matches[1].rm_so;
      hdr->name_end = matches[1].rm_eo;
      hdr->val_start = matches[2].rm_so;
      hdr->val_end = matches[2].rm_eo;
      for (i = 0; hd_types[i].len > 0; i++)
	if ((matches[1].rm_eo - matches[1].rm_so) == hd_types[i].len
	    && c_strncasecmp (hdr->header + matches[1].rm_so,
			      hd_types[i].header,
			      hd_types[i].len) == 0)
	  {
	    return hdr->code = hd_types[i].val;
	  }
      return hdr->code = HEADER_OTHER;
    }
  else
    return hdr->code = HEADER_ILLEGAL;
}

static struct http_header *
http_header_alloc (char const *text)
{
  struct http_header *hdr;

  if ((hdr = calloc (1, sizeof (*hdr))) == NULL)
    {
      lognomem ();
      return NULL;
    }
  if ((hdr->header = strdup (text)) == NULL)
    {
      lognomem ();
      free (hdr);
      return NULL;
    }

  qualify_header (hdr);

  return hdr;
}

static struct http_header *
http_header_dup (struct http_header *hdr)
{
  struct http_header *p = calloc (1, sizeof (*p));
  if (p)
    {
      *p = *hdr;
      if ((p->header = strdup (hdr->header)) != NULL)
	{
	  p->value = NULL;
	  return p;
	}
      else
	{
	  free (p);
	  p = NULL;
	}
    }
  return p;
}

static void
http_header_free (struct http_header *hdr)
{
  free (hdr->header);
  free (hdr->value);
  free (hdr);
}

static int
http_header_change (struct http_header *hdr, char const *text, int alloc)
{
  char *ctext;

  if (alloc)
    {
      if ((ctext = strdup (text)) == NULL)
	{
	  lognomem ();
	  return -1;
	}
    }
  else
    ctext = (char*)text;
  free (hdr->header);
  hdr->header = ctext;
  free (hdr->value);
  hdr->value = NULL;
  qualify_header (hdr);
  return 0;
}

static int
http_header_copy_value (struct http_header *hdr, char *buf, size_t len)
{
  size_t n;

  if (buf == NULL || len == 0)
    {
      errno = EINVAL;
      return -1;
    }
  len--;
  n = hdr->val_end - hdr->val_start;

  if (len < n)
    len = n;

  memcpy (buf, hdr->header + hdr->val_start, n);
  buf[n] = 0;
  return 0;
}

char *
http_header_get_value (struct http_header *hdr)
{
  if (!hdr->value)
    {
      size_t n = hdr->val_end - hdr->val_start + 1;
      if ((hdr->value = malloc (n)) == NULL)
	{
	  lognomem ();
	  return NULL;
	}
      http_header_copy_value (hdr, hdr->value, n);
    }
  return hdr->value;
}

static struct http_header *
http_header_list_locate (HTTP_HEADER_LIST *head, int code)
{
  struct http_header *hdr;
  DLIST_FOREACH (hdr, head, link)
    {
      if (hdr->code == code)
	return hdr;
    }
  return NULL;
}

struct http_header *
http_header_list_locate_name (HTTP_HEADER_LIST *head, char const *name,
			      size_t len)
{
  struct http_header *hdr;
  if (len == 0)
    len = strcspn (name, ":");
  DLIST_FOREACH (hdr, head, link)
    {
      if (http_header_name_len (hdr) == len &&
	  c_strncasecmp (http_header_name_ptr (hdr), name, len) == 0)
	return hdr;
    }
  return NULL;
}

/* Return next header with the same name as hdr. */
struct http_header *
http_header_list_next (struct http_header *hdr)
{
  size_t len = http_header_name_len (hdr);
  char const *name = http_header_name_ptr (hdr);
  while ((hdr = DLIST_NEXT (hdr, link)) != NULL)
    {
      if (http_header_name_len (hdr) == len &&
	  c_strncasecmp (http_header_name_ptr (hdr), name, len) == 0)
	return hdr;
    }
  return NULL;
}

/*
 * Append header TEXT to the list HEAD.  TEXT must be a correctly
 * formatted header line (NAME ":" VALUE).  Prior to appending, the
 * header list is scanned for a header with that name.  If found, the
 * REPLACE argument determines the further action: if it is 0, the list
 * is not modified, otherwise, the existing header is replaced with the
 * new value.
 */
int
http_header_list_append (HTTP_HEADER_LIST *head, char const *text, int replace)
{
  struct http_header *hdr;

  if ((hdr = http_header_list_locate_name (head, text, 0)) != NULL)
    {
      switch (replace)
	{
	case H_KEEP:
	  return 0;

	case H_REPLACE:
	  return http_header_change (hdr, text, 1);

	case H_APPEND:
	  {
	    char const *val = http_header_get_value (hdr);
	    if (*val)
	      {
		struct stringbuf sb;

		stringbuf_init_log (&sb);
		stringbuf_add (&sb, hdr->header,
			       hdr->val_end - hdr->name_start);
		stringbuf_add_char (&sb, ',');
		val = text + hdr->name_end + 1;
		if (!c_isspace (*val))
		  stringbuf_add_char (&sb, ' ');
		stringbuf_add_string (&sb, val);
		val = stringbuf_finish (&sb);
		if (!val)
		  return -1;
		return http_header_change (hdr, val, 0);
	      }
	    else
	      http_header_change (hdr, text, 1);
	  }
	}
    }
  else if ((hdr = http_header_alloc (text)) == NULL)
    return -1;
  else if (hdr->code == HEADER_ILLEGAL)
    {
      http_header_free (hdr);
      return 1;
    }
  else
    DLIST_INSERT_TAIL (head, hdr, link);
  return 0;
}

int
http_header_list_append_list (HTTP_HEADER_LIST *head, HTTP_HEADER_LIST *add)
{
  struct http_header *hdr;
  DLIST_FOREACH (hdr, add, link)
    {
      struct http_header *copy = http_header_dup (hdr);
      if (!copy)
	{
	  lognomem ();
	  return -1;
	}
      DLIST_INSERT_TAIL (head, copy, link);
    }
  return 0;
}

void
http_header_list_remove_field (HTTP_HEADER_LIST *head, char const *field)
{
  struct http_header *hdr, *tmp;
  size_t len = strlen (field);

  /* Remove existing headers with this field name. */
  DLIST_FOREACH_SAFE (hdr, tmp, head, link)
    {
      if (http_header_name_len (hdr) == len &&
	  c_strncasecmp (http_header_name_ptr (hdr), field, len) == 0)
	{
	  DLIST_REMOVE (head, hdr, link);
	  http_header_free (hdr);
	}
    }
}

void
http_header_list_free (HTTP_HEADER_LIST *head)
{
  while (!DLIST_EMPTY (head))
    {
      struct http_header *hdr = DLIST_FIRST (head);
      DLIST_REMOVE_HEAD (head, link);
      http_header_free (hdr);
    }
}

static void
http_header_list_remove (HTTP_HEADER_LIST *head, struct http_header *hdr)
{
  DLIST_REMOVE (head, hdr, link);
  http_header_free (hdr);
}

static void
http_header_list_filter (HTTP_HEADER_LIST *head, MATCHER *m)
{
  struct http_header *hdr, *tmp;

  DLIST_FOREACH_SAFE (hdr, tmp, head, link)
    {
      if (genpat_match (m->pat, hdr->header, 0, NULL) == 0)
	{
	  http_header_list_remove (head, hdr);
	}
    }
}

static void http_request_free_query (struct http_request *req);

static int
http_request_split (struct http_request *req)
{
  if (req->split)
    {
      size_t url_len = strlen (req->url);
      char *p;
      size_t path_len = url_len;
      int query_start = -1, query_len = 0;

      for (p = req->url + url_len; p > req->url; p--)
	{
	  if (*p == '?')
	    {
	      path_len = p - req->url;
	      query_start = path_len + 1;
	      query_len = url_len - query_start;
	      break;
	    }
	}

      free (req->path);
      if ((req->path = malloc (path_len + 1)) == NULL)
	{
	  lognomem ();
	  return -1;
	}
      memcpy (req->path, req->url, path_len);
      req->path[path_len] = 0;

      free (req->query);
      http_request_free_query (req);
      if (query_len > 0)
	{
	  if ((req->query = malloc (query_len + 1)) == NULL)
	    {
	      lognomem ();
	      return -1;
	    }
	  memcpy (req->query, req->url + query_start, query_len);
	  req->query[query_len] = 0;
	}
      else
	req->query = NULL;

      req->split = 0;
    }

  return 0;
}

static int
http_request_rebuild_line (struct http_request *req)
{
  struct stringbuf sb;
  char *str;

  stringbuf_init_log (&sb);
  stringbuf_add_string (&sb, methods[req->method].name);
  stringbuf_add_char (&sb, ' ');
  stringbuf_add_string (&sb, req->url);
  stringbuf_add_char (&sb, ' ');
  stringbuf_add_string (&sb, "HTTP/1.");
  stringbuf_add_char (&sb, req->version + '0');

  if ((str = stringbuf_finish (&sb)) == NULL)
    {
      stringbuf_free (&sb);
      return -1;
    }

  if (req->orig_request_line)
    free (req->request);
  else
    req->orig_request_line = req->request;
  req->request = str;

  return 0;
}

static int
http_request_rebuild_url (struct http_request *req)
{
  struct stringbuf sb;
  char *str;

  stringbuf_init_log (&sb);
  stringbuf_add_string (&sb, req->path);
  if (req->query)
    {
      stringbuf_add_char (&sb, '?');
      stringbuf_add_string (&sb, req->query);
    }
  if ((str = stringbuf_finish (&sb)) == NULL)
    {
      stringbuf_free (&sb);
      return -1;
    }
  free (req->url);
  req->url = str;

  return http_request_rebuild_line (req);
}

static int
http_request_rebuild_query (struct http_request *req)
{
  struct stringbuf sb;
  struct query_param *qp;
  char *p;
  int more = 0;

  stringbuf_init_log (&sb);
  DLIST_FOREACH (qp, &req->query_head, link)
    {
      if (more)
	stringbuf_add_char (&sb, '&');
      else
	more = 1;
      stringbuf_add_string (&sb, qp->name);
      if (qp->value)
	{
	  stringbuf_add_char (&sb, '=');
	  stringbuf_add_string (&sb, qp->value);
	}
    }
  if ((p = stringbuf_finish (&sb)) == NULL)
    {
      stringbuf_free (&sb);
      return -1;
    }
  free (req->query);
  req->query = p;

  return http_request_rebuild_url (req);
}

static int
http_request_get_request_line (struct http_request *req, char const **str)
{
  *str = req->request;
  return 0;
}

char const *
http_request_orig_line (struct http_request *req)
{
  if (!req)
    return "";
  return req->orig_request_line ? req->orig_request_line :
	  req->request ? req->request : "";
}

int
http_request_get_url (struct http_request *req, char const **retval)
{
  *retval = req->url;
  return 0;
}

int
http_request_set_url (struct http_request *req, char const *url)
{
  char *p;

  if ((p = strdup (url)) == NULL)
    {
      lognomem ();
      return -1;
    }
  free (req->url);
  req->url = p;
  req->split = 1;
  return http_request_rebuild_line (req);
}

int
http_request_get_path (struct http_request *req, char const **retval)
{
  if (http_request_split (req))
    return -1;
  *retval = req->path;
  return 0;
}

int
http_request_set_path (struct http_request *req, char const *path)
{
  char *val;
  char const *s;

  if (http_request_split (req))
      return -1;

  if (http_request_get_path (req, &s))
    return -1;
  if ((val = strdup (path)) == NULL)
    {
      lognomem ();
      return -1;
    }
  free (req->path);
  req->path = val;

  return http_request_rebuild_url (req);
}

int
http_request_get_query (struct http_request *req, char const **retval)
{
  if (http_request_split (req))
    return -1;
  *retval = req->query;
  return 0;
}

static void
query_param_free (struct query_param *qp)
{
  free (qp->name);
  free (qp->value);
  free (qp);
}

static void
http_request_free_query (struct http_request *req)
{
  while (!DLIST_EMPTY (&req->query_head))
    {
      struct query_param *qp = DLIST_FIRST (&req->query_head);
      DLIST_SHIFT (&req->query_head, link);
      query_param_free (qp);
    }
}

static int
http_request_parse_query (struct http_request *req)
{
  char const *query;

  if (http_request_get_query (req, &query))
    return -1;
  if (query)
    {
      while (*query)
	{
	  size_t pl = strcspn (query, "&");
	  size_t nl;
	  char *q;
	  char *val;
	  struct query_param *qp;

	  q = memchr (query, '=', pl);
	  if (q == NULL)
	    {
	      nl = pl;
	      val = NULL;
	    }
	  else
	    {
	      nl = q - query;
	      if ((val = malloc (pl - nl)) == NULL)
		{
		  lognomem ();
		  return -1;
		}
	      memcpy (val, query + nl + 1, pl - nl - 1);
	      val[pl - nl - 1] = 0;
	    }

	  if ((qp = malloc (sizeof (*qp))) == NULL)
	    {
	      lognomem ();
	      return -1;
	    }

	  if ((qp->name = malloc (nl + 1)) == NULL)
	    {
	      lognomem ();
	      free (qp);
	      return -1;
	    }
	  memcpy (qp->name, query, nl);
	  qp->name[nl] = 0;
	  qp->value = val;

	  DLIST_PUSH (&req->query_head, qp, link);

	  query += pl;
	  if (*query)
	    query++;
	}
    }
  return 0;
}

int
http_request_count_query_param (struct http_request *req)
{
  struct query_param *qp;
  int n = 0;

  if (http_request_split (req))
    return -1;
  DLIST_FOREACH (qp, &req->query_head, link)
    n++;
  return n;
}

static int
http_request_get_query_param (struct http_request *req,
			      char const *name, size_t namelen,
			      struct query_param **retval)
{
  struct query_param *qp;

  if (http_request_split (req))
    return RETRIEVE_ERROR;
  if (QUERY_EMPTY (&req->query_head) && http_request_parse_query (req))
    return RETRIEVE_ERROR;

  DLIST_FOREACH (qp, &req->query_head, link)
    {
      if (strlen (qp->name) == namelen && memcmp (qp->name, name, namelen) == 0)
	{
	  *retval = qp;
	  return RETRIEVE_OK;
	}
    }
  return RETRIEVE_NOT_FOUND;
}

int
http_request_get_query_param_value (struct http_request *req, char const *name,
				    char const **retval)
{
  struct query_param *qp;
  int rc = http_request_get_query_param (req, name, strlen (name), &qp);
  if (rc == RETRIEVE_OK)
    *retval = qp->value;
  return rc;
}

int
http_request_set_query (struct http_request *req, char const *rawquery)
{
  char *p;

  if (http_request_split (req))
      return -1;
  if (!rawquery)
    p = NULL;
  else if ((p = strdup (rawquery)) == NULL)
    {
      lognomem ();
      return -1;
    }
  free (req->query);
  req->query = p;
  http_request_free_query (req);
  return http_request_rebuild_url (req);
}

int
http_request_set_query_param (struct http_request *req, char const *name,
			      char const *raw_value)
{
  struct query_param *qp;
  int rc = http_request_get_query_param (req, name, strlen (name), &qp);
  char *value;

  switch (rc)
    {
    case RETRIEVE_ERROR:
      return RETRIEVE_ERROR;

    case RETRIEVE_NOT_FOUND:
      /* not found */
      if (raw_value == NULL)
	return RETRIEVE_OK;
      if ((value = strdup (raw_value)) == NULL)
	{
	  lognomem ();
	  return RETRIEVE_ERROR;
	}
      if ((qp = malloc (sizeof (*qp))) == NULL)
	{
	  lognomem ();
	  return RETRIEVE_ERROR;
	}
      if ((qp->name = strdup (name)) == NULL)
	{
	  lognomem ();
	  free (qp);
	  return RETRIEVE_ERROR;
	}
      qp->value = value;
      DLIST_PUSH (&req->query_head, qp, link);
      break;

    case 0:
      if (raw_value == NULL)
	{
	  DLIST_REMOVE (&req->query_head, qp, link);
	  query_param_free (qp);
	  if (DLIST_EMPTY (&req->query_head))
	    {
	      free (req->query);
	      req->query = NULL;
	      return http_request_rebuild_url (req);
	    }
	}
      else
	{
	  if ((value = strdup (raw_value)) == NULL)
	    {
	      lognomem ();
	      return -1;
	    }
	  free (qp->value);
	  qp->value = value;
	}
      break;
    }

  return http_request_rebuild_query (req);
}

static char const *
http_request_header_value (struct http_request *req, int code)
{
  struct http_header *hdr;
  if ((hdr = http_header_list_locate (&req->headers, code)) != NULL)
    return http_header_get_value (hdr);
  return NULL;
}

static char const *
http_request_host (struct http_request *req)
{
  return http_request_header_value (req, HEADER_HOST);
}

void
http_request_free (struct http_request *req)
{
  free (req->request);
  http_header_list_free (&req->headers);
  free (req->url);
  free (req->path);
  free (req->query);
  http_request_free_query (req);
  free (req->orig_request_line);
  free (req->eval_result);
  if (req->body)
    {
      stringbuf_free (req->body);
      free (req->body);
    }
  http_request_init (req);
}

typedef struct
{
  struct http_header *hdr;
  struct stringbuf sb;
  int count;
} COMPOSE_HEADER;

static unsigned long
COMPOSE_HEADER_hash (const COMPOSE_HEADER *cp)
{
  return strhash_ci (http_header_name_ptr (cp->hdr),
		     http_header_name_len (cp->hdr));
}

static int
COMPOSE_HEADER_cmp (const COMPOSE_HEADER *a, const COMPOSE_HEADER *b)
{
  size_t alen, blen;
  alen = http_header_name_len (a->hdr);
  blen = http_header_name_len (b->hdr);
  return alen != blen || c_strncasecmp (http_header_name_ptr (a->hdr),
					http_header_name_ptr (b->hdr), alen);
}

#define HT_TYPE COMPOSE_HEADER
#define HT_TYPE_HASH_FN_DEFINED 1
#define HT_TYPE_CMP_FN_DEFINED 1
#define HT_NO_DELETE 1
#include "ht.h"

static void
compose_header_free (COMPOSE_HEADER *hdr, void *data)
{
  stringbuf_free (&hdr->sb);
  free (hdr);
}

static void
compose_header_hash_free (COMPOSE_HEADER_HASH *chash)
{
  if (chash)
    {
      COMPOSE_HEADER_FOREACH (chash, compose_header_free, NULL);
      COMPOSE_HEADER_HASH_FREE (chash);
    }
}

static void
compose_header_finish (COMPOSE_HEADER *chdr, void *data)
{
  int *errp = data;

  if (*errp == 0)
    {
      char *val = stringbuf_finish (&chdr->sb);
      if (val)
	http_header_change (chdr->hdr, val, 0);
      else
	{
	  *errp = 1;
	  stringbuf_free (&chdr->sb);
	}
    }
  else
    stringbuf_free (&chdr->sb);
  free (chdr);
}

static int
compose_header_add (char const *str, size_t len, void *data)
{
  COMPOSE_HEADER *comp = data;
  if (comp->count > 0)
    stringbuf_add (&comp->sb, ", ", 2);
  stringbuf_add (&comp->sb, str, len);
  comp->count++;
  return 0;
}

static int
http_request_read (BIO *in, const LISTENER *lstn, char *buf,
		   struct http_request *req)
{
  int res;
  COMPOSE_HEADER_HASH *chash = NULL;

  if (combinable_headers)
    {
      chash = COMPOSE_HEADER_HASH_NEW ();
      if (!chash)
	{
	  lognomem ();
	  return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	}
    }

  http_request_init (req);

  /*
   * HTTP/1.1 allows leading CRLF
   */
  while ((res = get_line (in, buf, lstn->linebufsize)) == COPY_OK)
    if (buf[0])
      break;

  switch (res)
    {
    case COPY_OK:
      break;

    case COPY_EOF:
      return HTTP_STATUS_OK;

    case COPY_READ_ERR:
      logmsg (LOG_NOTICE, "(%"PRItid") %s",
	      POUND_TID (),
	      copy_status_string (res));
      return HTTP_STATUS_INTERNAL_SERVER_ERROR;

    case COPY_TOO_LONG:
      logmsg (LOG_NOTICE, "(%"PRItid") %s",
	      POUND_TID (),
	      copy_status_string (res));
      return HTTP_STATUS_URI_TOO_LONG;

    default:
      logmsg (LOG_NOTICE, "(%"PRItid") %s",
	      POUND_TID (),
	      copy_status_string (res));
      return HTTP_STATUS_BAD_REQUEST;
    }

  if ((req->request = strdup (buf)) == NULL)
    {
      lognomem ();
      compose_header_hash_free (chash);
      return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

  for (;;)
    {
      struct http_header *hdr;

      if ((res = get_line (in, buf, lstn->linebufsize)) != COPY_OK)
	{
	  http_request_free (req);
	  compose_header_hash_free (chash);
	  return res == COPY_TOO_LONG ?
	    HTTP_STATUS_PAYLOAD_TOO_LARGE : HTTP_STATUS_INTERNAL_SERVER_ERROR;
	}

      if (!buf[0])
	break;

      if ((hdr = http_header_alloc (buf)) == NULL)
	{
	  http_request_free (req);
	  compose_header_hash_free (chash);
	  return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	}
      else if (hdr->code == HEADER_ILLEGAL)
	http_header_free (hdr);
      else
	{
	  if (is_combinable_header (hdr))
	    {
	      COMPOSE_HEADER *comp, key;

	      key.hdr = hdr;
	      if ((comp = COMPOSE_HEADER_RETRIEVE (chash, &key)) != NULL)
		{
		  field_list_filter (hdr->header + hdr->val_start,
				     hdr->val_end - hdr->val_start,
				     compose_header_add,
				     comp, NULL);
		  http_header_free (hdr);
		  if (stringbuf_err (&comp->sb))
		    {
		      http_request_free (req);
		      compose_header_hash_free (chash);
		      return HTTP_STATUS_INTERNAL_SERVER_ERROR;
		    }
		  continue;
		}
	      else
		{
		  if ((comp = malloc (sizeof (*comp))) == NULL)
		    {
		      lognomem ();
		      http_request_free (req);
		      compose_header_hash_free (chash);
		      return HTTP_STATUS_INTERNAL_SERVER_ERROR;
		    }
		  comp->hdr = hdr;
		  comp->count = 0;
		  stringbuf_init_log (&comp->sb);
		  stringbuf_add (&comp->sb, http_header_name_ptr (hdr),
				 http_header_name_len (hdr));
		  stringbuf_add (&comp->sb, ": ", 2);
		  field_list_filter (hdr->header + hdr->val_start,
				     hdr->val_end - hdr->val_start,
				     compose_header_add,
				     comp, NULL);
		  if (stringbuf_err (&comp->sb))
		    {
		      http_request_free (req);
		      compose_header_hash_free (chash);
		      return HTTP_STATUS_INTERNAL_SERVER_ERROR;
		    }
		  COMPOSE_HEADER_INSERT (chash, comp);
		}
	    }
	  DLIST_INSERT_TAIL (&req->headers, hdr, link);
	}
    }

  /* Finalize multiple-value headers */
  if (chash)
    {
      res = 0;
      COMPOSE_HEADER_FOREACH (chash, compose_header_finish, &res);
      COMPOSE_HEADER_HASH_FREE (chash);
      if (res)
	{
	  http_request_free (req);
	  return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	}
    }

  return HTTP_STATUS_OK;
}

/*
 * Extract username and/or password from the Basic Authorization header.
 * Input:
 *   hdrval   - value of the Authorization header;
 * Output:
 *   u_name   - return pointer address for user name
 *   u_pass   - return pointer address for password (can be NULL)
 * Return value:
 *   0        - Success. Name returned in *u_name.
 *   1        - Not a Basic Authorization header.
 *  -1        - Other error.
 */
static int
get_basic_auth (char const *hdrval, char **u_name, char **u_pass)
{
  size_t len;
  BIO *bb, *b64;
  int inlen, u_len;
  char buf[MAXBUF], *q;
  int rc;

  if (c_strncasecmp (hdrval, "Basic", 5))
    return 1;

  hdrval += 5;
  hdrval += c_memspn (hdrval, CCTYPE_BLANK, strlen (hdrval));

  len = strlen (hdrval);
  if (*hdrval == '"')
    {
      hdrval++;
      len--;

      len = c_trimrws (hdrval, len);

      if (len == 0 || hdrval[len] != '"')
	return 1;
      len--;
    }

  if ((bb = BIO_new (BIO_s_mem ())) == NULL)
    {
      logmsg (LOG_WARNING, "(%"PRItid") can't alloc BIO_s_mem", POUND_TID ());
      return -1;
    }

  if ((b64 = BIO_new (BIO_f_base64 ())) == NULL)
    {
      logmsg (LOG_WARNING, "(%"PRItid") can't alloc BIO_f_base64",
	      POUND_TID ());
      BIO_free (bb);
      return -1;
    }

  b64 = BIO_push (b64, bb);
  BIO_write (bb, hdrval, len);
  BIO_write (bb, "\n", 1);
  inlen = BIO_read (b64, buf, sizeof (buf));
  BIO_free_all (b64);
  if (inlen <= 0)
    {
      logmsg (LOG_WARNING, "(%"PRItid") can't read BIO_f_base64",
	      POUND_TID ());
      return -1;
    }

  rc = -1;
  if ((q = memchr (buf, ':', inlen)) != NULL)
    {
      char *p, *pass;

      u_len = q - buf;
      if ((p = malloc (u_len + 1)) != NULL)
	{
	  memcpy (p, buf, u_len);
	  p[u_len] = 0;
	  *u_name = p;

	  inlen -= u_len;
	  if (u_pass == NULL)
	    rc = 0;
	  else if ((pass = malloc (inlen)) != NULL)
	    {
	      memcpy (pass, q + 1, inlen-1);
	      pass[inlen-1] = 0;
	      *u_pass = pass;
	      rc = 0;
	    }
	  else
	    free (p);
	}
    }
  memset (buf, 0, sizeof (buf));
  return rc;
}

/*
 * If the request bears the Authorization header, extract username and
 * password from it.  Return 0 on success, 1 if no data are available
 * (either Authorization header is missing, or it is not a Basic auth
 * header), and -1 on error.
 */
int
http_request_get_basic_auth (struct http_request *req,
			     char **u_name, char **u_pass)
{
  struct http_header *hdr;
  char const *val;

  if ((hdr = http_header_list_locate (&req->headers,
				      HEADER_AUTHORIZATION)) != NULL)
    {
      if ((val = http_header_get_value (hdr)) == NULL)
	return -1;
      return get_basic_auth (val, u_name, u_pass);
    }
  return 1;
}

static int
parse_http_request (struct http_request *req, int group)
{
  char *str;
  size_t len, ulen;
  struct method_def *md;
  char const *url;
  int http_ver;
  int status = HTTP_STATUS_OK;
  int rebuild = 0;

  str = req->request;
  len = strcspn (str, " ");
  if (len == 0 || str[len-1] == 0)
    return HTTP_STATUS_BAD_REQUEST;

  if ((md = find_method (str, len)) == NULL)
    return HTTP_STATUS_BAD_REQUEST;

  if (md->group > group)
    status = HTTP_STATUS_METHOD_NOT_ALLOWED;

  str += len;
  len = strspn (str, " ");
  if (len > 1)
    rebuild = 1;
  str += len;

  if (*str == 0)
    return HTTP_STATUS_BAD_REQUEST;

  url = str;
  len = strcspn (url, " ");
  ulen = len;

  str += len;
  len = strspn (str, " ");
  if (len > 1)
    rebuild = 1;
  str += len;

  if (!(strncmp (str, "HTTP/1.", 7) == 0 &&
	((http_ver = str[7]) == '0' || http_ver == '1') &&
	str[8] == 0))
    return HTTP_STATUS_BAD_REQUEST;

  if (status == HTTP_STATUS_OK)
    {
     req->method = md->meth;
      if ((req->url = xstrndup (url, ulen)) == NULL)
	{
	  lognomem ();
	  return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	}

      req->version = http_ver - '0';
      req->split = 1;

      if (rebuild)
	http_request_rebuild_line (req);
    }

  return status;
}

static int
match_headers (HTTP_HEADER_LIST *headers, GENPAT re,
	       struct submatch *sm)
{
  struct http_header *hdr;

  DLIST_FOREACH (hdr, headers, link)
    {
      if (hdr->header && submatch_exec (re, hdr->header, sm))
	return 1;
    }
  return 0;
}

/*
 * Match request (or response) REQ obtained from PHTTP against condition COND.
 * Return value:
 *
 *  0    request does not satisfy the condition;
 *  1    request satisfies the condition;
 * -1    an error occurred; diagnostic message has been issued;
 */
int
match_cond (SERVICE_COND *cond, POUND_HTTP *phttp,
	    struct http_request *req)
{
  int res = 1;
  int r;
  SERVICE_COND *subcond;
  char const *str;
  struct submatch sm = SUBMATCH_INITIALIZER;

  watcher_lock (cond->watcher);
  switch (cond->type)
    {
    case COND_ACL:
      {
	struct addrinfo *tmp;
	if ((r = acl_match (cond->acl.acl,
			    get_remote_ip (phttp,
					   cond->acl.forwarded,
					   &tmp)->ai_addr)) == -1)
	  res = -1;
	else
	  res = r == 0;
	if (tmp)
	  freeaddrinfo (tmp);
      }
      break;

    case COND_URL:
      if (http_request_get_url (req, &str) == -1)
	res = -1;
      else if ((res = submatch_exec (cond->re, str, &sm)) == 1)
	submatch_queue_push (&phttp->smq, cond->tag, &sm);
      break;

    case COND_PATH:
      if (http_request_get_path (req, &str) == -1)
	res = -1;
      else if ((res = submatch_exec_decode (cond->re, str, cond->decode, &sm)) == 1)
	submatch_queue_push (&phttp->smq, cond->tag, &sm);
      break;

    case COND_QUERY:
      if (http_request_get_query (req, &str) == -1)
	res = -1;
      else if ((res = submatch_exec (cond->re, str, &sm)) == 1)
	submatch_queue_push (&phttp->smq, cond->tag, &sm);
      break;

    case COND_QUERY_PARAM:
      switch (http_request_get_query_param_value (req,
						  string_ptr (cond->sm.string),
						  &str))
	{
	case RETRIEVE_ERROR:
	  res = -1;
	  break;

	case RETRIEVE_NOT_FOUND:
	  res = 0;
	  break;

	default:
	  if (str == NULL)
	    res = 0;
	  else if ((res = submatch_exec_decode (cond->sm.re, str, cond->decode,
						&sm)) == 1)
	    submatch_queue_push (&phttp->smq, cond->tag, &sm);
	}
      break;

    case COND_HDR:
      if ((res = match_headers (&req->headers, cond->re, &sm)) == 1)
	submatch_queue_push (&phttp->smq, cond->tag, &sm);
      break;

    case COND_HOST:
      if ((res = match_headers (&req->headers, cond->re, &sm)) == 1)
	submatch_queue_push (&phttp->smq, cond->tag, &sm);
      if (res)
	{
	  /*
	   * On match, adjust subgroup references and subject pointer
	   * to refer to the Host: header value.
	   */
	  struct submatch *sm = submatch_queue_get (&phttp->smq, 0);
	  int n, i;
	  char const *s = sm->subject;
	  POUND_REGMATCH *mv = sm->matchv;
	  int mc = sm->matchn;
	  char *p;

	  /* Skip initial whitespace and "host:" */
	  s += strspn (s, " \t\n") + 5;
	  /* Skip whitespace after header name. */
	  s += strspn (s, " \t\n");
	  /* Compute fix-up offset. */
	  n = s - sm->subject;
	  /* Adjust all subgroups. */
	  /* rm_so of the 0th group is always 0, so adjust only rm_eo: */
	  mv->rm_eo -= n;
	  for (i = 1; i < mc; i++)
	    {
	      ++mv;
	      mv->rm_so -= n;
	      mv->rm_eo -= n;
	    }
	  /* Adjust subject. */
	  if ((p = strdup (s)) != NULL)
	    {
	      free (sm->subject);
	      sm->subject = p;
	    }
	  else
	    {
	      lognomem ();
	    }
	}
      break;

    case COND_BASIC_AUTH:
      if ((r = basic_auth (&cond->pwfile, req)) == -1)
	res = -1;
      else
	res = r == 0;
      break;

    case COND_STRING_MATCH:
      {
	char *subj;

	subj = expand_string (string_ptr (cond->sm.string), phttp, req,
			      "string_match");
	if (subj)
	  {
	    if ((res = submatch_exec (cond->sm.re, subj, &sm)) == 1)
	      submatch_queue_push (&phttp->smq, cond->tag, &sm);
	    free (subj);
	  }
	else
	  res = -1;
      }
      break;

    case COND_DYN:
      if (SLIST_EMPTY (&cond->dyn.boolean.head))
	{
	  res = 0;
	  break;
	}
      /* fall through */
    case COND_BOOL:
      if (cond->boolean.op == BOOL_NOT)
	{
	  subcond = SLIST_FIRST (&cond->boolean.head);
	  if ((r = match_cond (subcond, phttp, req)) == -1)
	    res = -1;
	  else
	    res = ! r;
	}
      else
	{
	  SLIST_FOREACH (subcond, &cond->boolean.head, next)
	    {
	      res = match_cond (subcond, phttp, req);
	      if (res == -1)
		break;
	      if ((cond->boolean.op == BOOL_AND) ? (res == 0) : (res == 1))
		break;
	    }
	}
      break;

    case COND_CLIENT_CERT:
      res = (phttp->x509 != NULL &&
	     X509_cmp (phttp->x509, cond->x509) == 0 &&
	     SSL_get_verify_result (phttp->ssl) == X509_V_OK);
      break;

    case COND_LUA:
      res = pndlua_apply (pndlua_match, phttp, req, &cond->clua, NULL);
      break;

    case COND_TBF:
      {
	char *key;

	key = expand_string (string_ptr (cond->tbf.key), phttp, req, "tbf");
	if (key)
	  {
	    res = tbf_eval (cond->tbf.tbf, key);
	    free (key);
	  }
	else
	  res = -1;
      }
      break;

    case COND_REF:
      if ((res = http_request_eval_get (req, cond->ref)) == -1)
	{
	  /* Save away current submatch queue. */
	  struct submatch_queue smq = phttp->smq;
	  /* Reinitialize the queue. */
	  submatch_queue_init (&phttp->smq);
	  /* Evaluate the condition and cache the result. */
	  res = match_cond (detached_cond (cond->ref), phttp, req);
	  http_request_eval_cache (req, cond->ref, res);
	  /* Restore submatch queue. */
	  submatch_queue_free (&phttp->smq);
	  phttp->smq = smq;
	}
      break;
    }
  watcher_unlock (cond->watcher);

  return res;
}

static int
http_headers_send (BIO *be, HTTP_HEADER_LIST *head, int safe)
{
  struct http_header *hdr;
  DLIST_FOREACH (hdr, head, link)
    {
      if (safe &&
	  (hdr->code == HEADER_CONTENT_LENGTH ||
	   hdr->code == HEADER_CONNECTION))
	continue;
      if (BIO_printf (be, "%s\r\n", hdr->header) <= 0)
	return -1;
    }
  return 0;
}

static int
http_request_send (BIO *be, struct http_request *req)
{
  char const *s;

  if (http_request_get_request_line (req, &s))
    return -1;
  if (BIO_printf (be, "%s\r\n", s) <= 0)
    return -1;
  return http_headers_send (be, &req->headers, 0);
}

static int
add_forwarded_headers (POUND_HTTP *phttp)
{
  char caddr[MAX_ADDR_BUFSIZE];
  struct stringbuf sb;
  char *str;
  int port;

  stringbuf_init_log (&sb);

  stringbuf_printf (&sb, "X-Forwarded-For: %s",
		    addr2str (caddr, sizeof (caddr), &phttp->from_host, 1));
  if ((str = stringbuf_finish (&sb)) == NULL
      || http_header_list_append (&phttp->request.headers, str, H_APPEND))
    {
      stringbuf_free (&sb);
      return -1;
    }

  stringbuf_reset (&sb);
  stringbuf_printf (&sb, "X-Forwarded-Proto: %s",
		    phttp->ssl == NULL ? "http" : "https");
  if ((str = stringbuf_finish (&sb)) == NULL
      || http_header_list_append (&phttp->request.headers, str, H_REPLACE))
    {
      stringbuf_free (&sb);
      return -1;
    }

  switch (phttp->from_host.ai_family)
    {
    case AF_INET:
      port = ntohs (((struct sockaddr_in *)phttp->lstn->addr.ai_addr)->sin_port);
      break;

    case AF_INET6:
      port = ntohs (((struct sockaddr_in6 *)phttp->lstn->addr.ai_addr)->sin6_port);
      break;

    default:
      stringbuf_free (&sb);
      return 0;
    }

  stringbuf_reset (&sb);
  stringbuf_printf (&sb, "X-Forwarded-Port: %d", port);
  if ((str = stringbuf_finish (&sb)) == NULL
      || http_header_list_append (&phttp->request.headers, str, H_REPLACE))
    {
      stringbuf_free (&sb);
      return -1;
    }

  stringbuf_free (&sb);
  return 0;
}

static int
set_header_from_bio (BIO *bio, struct http_request *req,
		     char const *hdr, struct stringbuf *sb)
{
  char buf[MAXBUF];
  int rc;
  char *str;

  if ((rc = get_line (bio, buf, sizeof (buf))) == COPY_OK)
    {
      stringbuf_reset (sb);
      stringbuf_printf (sb, "%s: %s", hdr, c_trimlws (buf, NULL));
      if ((str = stringbuf_finish (sb)) == NULL
	  || http_header_list_append (&req->headers, str, H_REPLACE))
	{
	  return -1;
	}
    }
  else if (rc != COPY_EOF)
    logmsg (LOG_ERR, "(%"PRItid") error reading data: %s",
	    POUND_TID (), copy_status_string (rc));
  return 0;
}

static int
add_ssl_headers (POUND_HTTP *phttp)
{
  int res = 0;
  const SSL_CIPHER *cipher;
  struct stringbuf sb;
  char *str;
  char buf[MAXBUF];
  BIO *bio = NULL;

  stringbuf_init_log (&sb);
  if ((cipher = SSL_get_current_cipher (phttp->ssl)) != NULL)
    {
      SSL_CIPHER_description (cipher, buf, sizeof (buf));
      strip_eol (buf);
      stringbuf_printf (&sb, "X-SSL-cipher: %s/%s",
			SSL_get_version (phttp->ssl),
			buf);
      if ((str = stringbuf_finish (&sb)) == NULL
	  || http_header_list_append (&phttp->request.headers, str, H_REPLACE))
	{
	  res = -1;
	  goto end;
	}
      stringbuf_reset (&sb);
    }

  if (phttp->lstn->clnt_check > 0 && phttp->x509 != NULL
      && (bio = BIO_new (BIO_s_mem ())) != NULL)
    {
      int i;

      BIO_set_mem_eof_return (bio, 0);
      X509_NAME_print_ex (bio, X509_get_subject_name (phttp->x509), 8,
			  XN_FLAG_ONELINE & ~ASN1_STRFLGS_ESC_MSB);
      if (set_header_from_bio (bio, &phttp->request, "X-SSL-Subject", &sb))
	{
	  res = -1;
	  goto end;
	}

      X509_NAME_print_ex (bio, X509_get_issuer_name (phttp->x509), 8,
			  XN_FLAG_ONELINE & ~ASN1_STRFLGS_ESC_MSB);
      if (set_header_from_bio (bio, &phttp->request, "X-SSL-Issuer", &sb))
	{
	  res = -1;
	  goto end;
	}

      ASN1_TIME_print (bio, X509_get_notBefore (phttp->x509));
      if (set_header_from_bio (bio, &phttp->request, "X-SSL-notBefore", &sb))
	{
	  res = -1;
	  goto end;
	}

      ASN1_TIME_print (bio, X509_get_notAfter (phttp->x509));
      if (set_header_from_bio (bio, &phttp->request, "X-SSL-notAfter", &sb))
	{
	  res = -1;
	  goto end;
	}

      stringbuf_reset (&sb);
      stringbuf_printf (&sb, "X-SSL-serial: %ld",
			ASN1_INTEGER_get (X509_get_serialNumber (phttp->x509)));
      if ((str = stringbuf_finish (&sb)) == NULL
	  || http_header_list_append (&phttp->request.headers, str, H_REPLACE))
	{
	  res = -1;
	  goto end;
	}
      stringbuf_reset (&sb);

      PEM_write_bio_X509 (bio, phttp->x509);
      stringbuf_add_string (&sb, "X-SSL-certificate: ");
      i = 0;
      while (get_line (bio, buf, sizeof (buf)) == COPY_OK)
	{
	  if (i > 0)
	    stringbuf_add_string (&sb, "\r\n\t");
	  stringbuf_add_string (&sb, buf);
	  i++;
	}
      if ((str = stringbuf_finish (&sb)) == NULL
	  || http_header_list_append (&phttp->request.headers, str, H_REPLACE))
	{
	  res = -1;
	  goto end;
	}
    }

 end:
  if (bio)
    BIO_free_all (bio);
  stringbuf_free (&sb);

  return res;
}

static int rewrite_rule_check (REWRITE_RULE *rule,
			       struct http_request *request,
			       POUND_HTTP *phttp, int what);

static int
rewrite_op_apply (REWRITE_OP_HEAD *head, struct http_request *request,
		  POUND_HTTP *phttp, int what)
{
  int res = 0;
  REWRITE_OP *op;
  char *s;

  static struct
  {
    char *name;
    int (*setter) (struct http_request *, char const *);
    char *xenc;
  } rwtab[] = {
    [REWRITE_URL_SET]    = { "url", http_request_set_url },
    [REWRITE_PATH_SET]   = { "path", http_request_set_path, "/" },
    [REWRITE_QUERY_SET]  = { "query", http_request_set_query },
  };

  SLIST_FOREACH (op, head, next)
    {
      switch (op->type)
	{
	case REWRITE_REWRITE_RULE:
	  res = rewrite_rule_check (op->v.rule, request, phttp, what);
	  break;

	case REWRITE_HDR_DEL:
	  http_header_list_filter (&request->headers, op->v.hdrdel);
	  break;

	case REWRITE_HDR_SET:
	  if ((s = expand_string (op->v.str, phttp, request, "Header")) != NULL)
	    {
	      res = http_header_list_append (&request->headers, s, H_REPLACE);
	      free (s);
	    }
	  else
	    res = -1;
	  break;

	case REWRITE_QUERY_PARAM_SET:
	  if (op->v.qp.value == NULL)
	    res = http_request_set_query_param (request, op->v.qp.name, NULL);
	  else if ((s = expand_string_encode (op->v.qp.value, op->encode, NULL,
					      phttp, request,
					      "query parameter")) != NULL)
	    {
	      res = http_request_set_query_param (request, op->v.qp.name, s);
	      free (s);
	    }
	  else
	    res = -1;
	  break;

	case REWRITE_QUERY_DELETE:
	  res = http_request_set_query (request, NULL);
	  break;

	case REWRITE_LUA:
	  res = pndlua_apply (pndlua_modify, phttp, request,
			      &op->v.lua, &what);
	  break;

	default:
	  if ((s = expand_string_encode (op->v.str, op->encode,
					 rwtab[op->type].xenc,
					 phttp, request,
					 rwtab[op->type].name)) != NULL)
	    {
	      res = rwtab[op->type].setter (request, s);
	      free (s);
	    }
	  else
	    res = -1;
	  break;
	}
      if (res)
	break;
    }

  return res;
}

static int
rewrite_rule_check (REWRITE_RULE *rule, struct http_request *request,
		    POUND_HTTP *phttp, int what)
{
  int res = 0;

  do
    {
      if (match_cond (&rule->cond, phttp, request) == 1)
	{
	  res = rewrite_op_apply (&rule->ophead, request, phttp, what);
	  break;
	}
    }
  while ((rule = rule->iffalse) != NULL);
  return res;
}

static int
rewrite_apply_rules (POUND_HTTP *phttp, int what,
		     REWRITE_RULE_HEAD *rewrite_rules,
		     struct http_request *request)
{
  int res = 0;
  REWRITE_RULE *rule;

  SLIST_FOREACH (rule, rewrite_rules, next)
    {
      if ((res = rewrite_rule_check (rule, request, phttp, what)) != 0)
	break;
    }
  return res;
}

static int
rewrite_apply (POUND_HTTP *phttp, struct http_request *request, int what)
{
  REWRITE_RULE_HEAD *rwhead[2] = {
    &phttp->lstn->rewrite[what],
    &phttp->svc->rewrite[what]
  };
  if (what == REWRITE_RESPONSE)
    {
      REWRITE_RULE_HEAD *t = rwhead[0];
      rwhead[0] = rwhead[1];
      rwhead[1] = t;
    }

  return rewrite_apply_rules (phttp, what, rwhead[0], request) ||
    rewrite_apply_rules (phttp, what, rwhead[1], request);
}

/*
 * Cleanup code. This should really be in the pthread_cleanup_push, except
 * for bugs in some implementations
 */

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#  define clear_error(ssl)
#else /* OPENSSL_VERSION_NUMBER >= 0x10000000L */
#  define clear_error(ssl) \
	if(ssl != NULL) { ERR_clear_error(); ERR_remove_thread_state(NULL); }
#endif

static void
socket_setup (int sock)
{
  int n = 1;
  struct linger l;

  setsockopt (sock, SOL_SOCKET, SO_KEEPALIVE, (void *) &n, sizeof (n));
  l.l_onoff = 1;
  l.l_linger = 10;
  setsockopt (sock, SOL_SOCKET, SO_LINGER, (void *) &l, sizeof (l));
#ifdef  TCP_LINGER2
  n = 5;
  setsockopt (sock, SOL_TCP, TCP_LINGER2, (void *) &n, sizeof (n));
#endif
  n = 1;
  setsockopt (sock, SOL_TCP, TCP_NODELAY, (void *) &n, sizeof (n));
}

static int
force_http_10 (POUND_HTTP *phttp)
{
  /*
   * check the value if nohttps11:
   *  - if 0 ignore
   *  - if 1 and SSL force HTTP/1.0
   *  - if 2 and SSL and MSIE force HTTP/1.0
   */
  switch (phttp->lstn->noHTTPS11)
    {
    case 1:
      return (phttp->ssl != NULL);

    case 2:
      {
	char const *agent = http_request_header_value (&phttp->request,
						       HEADER_USER_AGENT);
	return (phttp->ssl != NULL && agent != NULL &&
		strstr (agent, "MSIE") != NULL);
      }

    default:
      break;
    }
  return 0;
}

static void
close_backend (POUND_HTTP *phttp)
{
  if (phttp->be)
    {
      BIO_reset (phttp->be);
      BIO_free_all (phttp->be);
      phttp->be = NULL;
    }
}

static int
http_response_validate (struct http_request *req)
{
  char const *str = req->request;
  int http_ver;

  if (!(strncmp (str, "HTTP/1.", 7) == 0 &&
	((http_ver = str[7]) == '0' || http_ver == '1') &&
	c_isblank (str[8])))
    return 0;
  req->version = http_ver - '0';

  str = c_trimlws (str + 8, NULL);

  switch (str[0])
    {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
      if (c_isdigit (str[1]) && c_isdigit (str[2]) && (str[3] == 0 || c_isblank (str[3])))
	return (int) strtol (str, NULL, 10);
    }

  return 0;
}

/*
 * get the response
 */
static int
backend_response (POUND_HTTP *phttp)
{
  enum { RESP_OK, RESP_SKIP, RESP_DRAIN } resp_mode = RESP_OK;
  int be_11 = 0;  /* Whether backend connection is using HTTP/1.1. */
  CONTENT_LENGTH content_length;
  char buf[MAXBUF];
  struct http_header *hdr;
  char *val;
  int res;

  phttp->res_bytes = 0;
  do
    {
      int chunked; /* True if request contains Transfer-Encoding: chunked */
      int code;

      /* Free previous response, if any */
      http_request_free (&phttp->response);
      res = http_request_read (phttp->be, phttp->lstn, phttp->buffer,
			       &phttp->response);
      if (res != HTTP_STATUS_OK)
	{
	  phttp_log (phttp, PHTTP_LOG_BACKEND | PHTTP_LOG_REVERSE,
		     res, errno, "response read error");
	  return res;
	}
      else if (phttp->response.request == NULL)
	{
	  res = HTTP_STATUS_SERVICE_UNAVAILABLE;
	  phttp_log (phttp, PHTTP_LOG_BACKEND | PHTTP_LOG_REVERSE,
		     res, 0, "eof from backend");
	  return res;
	}

      if ((code = http_response_validate (&phttp->response)) == 0)
	{
	  res = HTTP_STATUS_SERVICE_UNAVAILABLE;
	  phttp_log (phttp, PHTTP_LOG_BACKEND | PHTTP_LOG_REVERSE,
		     res, 0, "malformed response");
	  return res;
	}

      be_11 = (phttp->response.version == 1);
      phttp->response_code = code;

      switch (phttp->response_code)
	{
	case 100:
	  /*
	   * responses with code 100 are never passed back to the client
	   */
	  resp_mode = RESP_SKIP;
	  break;

	case 101:
	  phttp->ws_state |= WSS_RESP_101;
	  phttp->no_cont = 1;
	  break;

	case 204:
	case 304:
	case 305:
	case 306:
	  /* No content is expected */
	  phttp->no_cont = 1;
	  break;

	default:
	  switch (phttp->response_code / 100)
	    {
	    case 1:
	      phttp->no_cont = 1;
	      break;

	    case 4:
	    case 5:
	      if (phttp->svc->rewrite_errors != -1
		  ? phttp->svc->rewrite_errors
		  : phttp->lstn->rewrite_errors == 1)
		{
		  int err = http_status_to_pound (phttp->response_code);
		  if (err != -1 && phttp->lstn->http_err[err])
		    {
		      if (http_err_reply_cb (phttp, err,
					     phttp->lstn->http_err[err],
					     cb_hdr_rewrite, phttp)
			  != HTTP_STATUS_OK)
			return HTTP_STATUS_INTERNAL_SERVER_ERROR;
		      resp_mode = RESP_DRAIN;
		    }
		}
	    }
	}

      if (resp_mode == RESP_OK)
	{
	  if (rewrite_apply (phttp, &phttp->response, REWRITE_RESPONSE))
	    return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	}

      chunked = 0;
      content_length = NO_CONTENT_LENGTH;
      DLIST_FOREACH (hdr, &phttp->response.headers, link)
	{
	  switch (hdr->code)
	    {
	    case HEADER_CONNECTION:
	      if ((val = http_header_get_value (hdr)) == NULL)
		return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	      if (!c_strcasecmp ("close", val))
		phttp->conn_closed = 1;
	      /*
	       * Connection: upgrade
	       */
	      else if (genpat_match (CONN_UPGRD, val, 0, NULL) == 0)
		phttp->ws_state |= WSS_RESP_HEADER_CONNECTION_UPGRADE;
	      break;

	    case HEADER_UPGRADE:
	      if ((val = http_header_get_value (hdr)) == NULL)
		return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	      if (cs_locate_token (val, "websocket", NULL))
		phttp->ws_state |= WSS_RESP_HEADER_UPGRADE_WEBSOCKET;
	      break;

	    case HEADER_TRANSFER_ENCODING:
	      if ((val = http_header_get_value (hdr)) == NULL)
		return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	      if (cs_locate_token (val, "chunked", NULL) &&
			  phttp->request.method != METH_HEAD &&
			  phttp->response_code != 304)
		{
		  chunked = 1;
		  phttp->no_cont = 0;
		}
	      break;

	    case HEADER_CONTENT_LENGTH:
	      if ((val = http_header_get_value (hdr)) == NULL)
		return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	      if ((content_length = get_content_length (val, CL_HEADER)) == NO_CONTENT_LENGTH)
		{
		  phttp_log (phttp,
			     PHTTP_LOG_BACKEND | PHTTP_LOG_REVERSE,
			     -1, 0, "invalid content length: %s", val);
		  http_header_list_remove (&phttp->response.headers, hdr);
		}
	      break;

	    case HEADER_LOCATION:
	      if ((val = http_header_get_value (hdr)) == NULL)
		return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	      else if (phttp->lstn->rewr_loc)
		{
		  char const *v_host = http_request_host (&phttp->request);
		  char const *path;
		  if (v_host && v_host[0] &&
		      need_rewrite (val, v_host, phttp->lstn, phttp->backend,
				    &path))
		    {
		      struct stringbuf sb;
		      char *p;

		      stringbuf_init_log (&sb);
		      stringbuf_printf (&sb, "Location: %s://%s/%s",
					(phttp->ssl == NULL ? "http" : "https"),
					v_host,
					path);
		      if ((p = stringbuf_finish (&sb)) == NULL)
			{
			  stringbuf_free (&sb);
			  return HTTP_STATUS_INTERNAL_SERVER_ERROR;
			}
		      http_header_change (hdr, p, 0);
		    }
		}
	      break;

	    case HEADER_CONTLOCATION:
	      if ((val = http_header_get_value (hdr)) == NULL)
		return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	      else if (phttp->lstn->rewr_loc)
		{
		  char const *v_host = http_request_host (&phttp->request);
		  char const *path;
		  if (v_host && v_host[0] &&
		      need_rewrite (val, v_host, phttp->lstn, phttp->backend,
				    &path))
		    {
		      struct stringbuf sb;
		      char *p;

		      stringbuf_init_log (&sb);
		      stringbuf_printf (&sb,
					"Content-location: %s://%s/%s",
					(phttp->ssl == NULL ? "http" : "https"),
					v_host,
					path);
		      if ((p = stringbuf_finish (&sb)) == NULL)
			{
			  stringbuf_free (&sb);
			  return HTTP_STATUS_INTERNAL_SERVER_ERROR;
			}
		      http_header_change (hdr, p, 0);
		    }
		  break;
		}
	    }
	}

      /*
       * possibly record session information (for header-based sessions
       * only)
       */
      upd_session (phttp->svc, &phttp->response.headers, phttp->backend);

      /*
       * send the response
       */
      if (resp_mode == RESP_OK)
	{
	  if (http_request_send (phttp->cl, &phttp->response))
	    {
	      if (errno)
		phttp_log (phttp, PHTTP_LOG_BACKEND | PHTTP_LOG_REVERSE,
			   -1, errno, "error writing response");
	      return -1;
	    }
	  /* Final CRLF */
	  BIO_puts (phttp->cl, "\r\n");
	}

      if (BIO_flush (phttp->cl) != 1)
	{
	  if (errno)
	    phttp_log (phttp, PHTTP_LOG_BACKEND | PHTTP_LOG_REVERSE,
		       -1, errno, "error sending response headers");
	  return -1;
	}

      if (!phttp->no_cont)
	{
	  /*
	   * ignore this if request was HEAD or similar
	   */
	  if (be_11 && chunked)
	    {
	      /*
	       * had Transfer-encoding: chunked so read/write all
	       * the chunks (HTTP/1.1 only)
	       */
	      if (copy_chunks (phttp->be, phttp->cl,
			       phttp->buffer, phttp->lstn->linebufsize,
			       resp_mode != RESP_OK, 0,
			       &phttp->res_bytes) != HTTP_STATUS_OK)
		{
		  /*
		   * copy_chunks() has its own error messages
		   */
		  return -1;
		}
	    }
	  else if (content_length >= 0)
	    {
	      /*
	       * may have had Content-length, so do raw reads/writes
	       * for the length
	       */
	      int ec = copy_bin (phttp->be, phttp->cl, content_length,
				 resp_mode != RESP_OK, &phttp->res_bytes);
	      switch (ec)
		{
		case COPY_OK:
		  break;

		case COPY_READ_ERR:
		case COPY_WRITE_ERR:
		  phttp_log (phttp,
			     PHTTP_LOG_BACKEND | PHTTP_LOG_REVERSE,
			     -1, errno,
			     "%s while sending backend response",
			     copy_status_string (ec));
		  return -1;

		default:
		  phttp_log (phttp,
			     PHTTP_LOG_BACKEND | PHTTP_LOG_REVERSE,
			     -1, 0,
			     "%s while sending backend response",
			     copy_status_string (ec));
		  return -1;
		}
	    }
	  else if (resp_mode == RESP_OK)
	    {
	      if (is_readable (phttp->be, phttp->backend->v.reg.to))
		{
		  char one;
		  BIO *be_unbuf;

		  /*
		   * old-style response - content until EOF
		   * also implies the client may not use HTTP/1.1
		   */
		  be_11 = 0;
		  phttp->conn_closed = 1;

		  /*
		   * first read whatever is already in the input buffer
		   */
		  while (BIO_pending (phttp->be))
		    {
		      if (BIO_read (phttp->be, &one, 1) != 1)
			{
			  phttp_log (phttp,
				     PHTTP_LOG_BACKEND | PHTTP_LOG_REVERSE,
				     -1, errno,
				     "error reading response");
			  return -1;
			}
		      if (BIO_write (phttp->cl, &one, 1) != 1)
			{
			  if (errno)
			    phttp_log (phttp,
				       PHTTP_LOG_BACKEND | PHTTP_LOG_REVERSE,
				       -1, errno,
				       "error writing response");
			  return -1;
			}
		      phttp->res_bytes++;
		    }
		  BIO_flush (phttp->cl);

		  /*
		   * find the socket BIO in the chain
		   */
		  if ((be_unbuf =
			   BIO_find_type (phttp->be,
					  backend_is_https (phttp->backend)
					    ? BIO_TYPE_SSL
					    : BIO_TYPE_SOCKET)) == NULL)
		    {
		      logmsg (LOG_ERR,
			      "(%"PRItid") error getting unbuffered BIO: %s",
			      POUND_TID (), strerror (errno));
		      return -1;
		    }

		  /*
		   * copy till EOF
		   */
		  while ((res = BIO_read (be_unbuf, buf, sizeof (buf))) > 0)
		    {
		      if (BIO_write (phttp->cl, buf, res) != res)
			{
			  phttp_log (phttp,
				     PHTTP_LOG_BACKEND | PHTTP_LOG_REVERSE,
				     -1, errno,
				     "error copying response body");
			  return -1;
			}
		      else
			{
			  phttp->res_bytes += res;
			  BIO_flush (phttp->cl);
			}
		    }
		}
	    }
	  if (BIO_flush (phttp->cl) != 1)
	    {
	      if (errno)
		phttp_log (phttp,
			   PHTTP_LOG_BACKEND | PHTTP_LOG_REVERSE,
			   -1, errno,
			   "error flushing response");
	      return -1;
	    }
	}
      else if (phttp->ws_state == WSS_COMPLETE)
	{
	  /*
	   * special mode for Websockets - content until EOF
	   */
	  char one;
	  BIO *cl_unbuf;
	  BIO *be_unbuf;
	  struct pollfd p[2];

	  /* Force connection close on both sides when ws has finished */
	  be_11 = 0;
	  phttp->conn_closed = 1;

	  memset (p, 0, sizeof (p));
	  BIO_get_fd (phttp->cl, &p[0].fd);
	  p[0].events = POLLIN | POLLPRI;
	  BIO_get_fd (phttp->be, &p[1].fd);
	  p[1].events = POLLIN | POLLPRI;

	  while (BIO_pending (phttp->cl) || BIO_pending (phttp->be)
		 || poll (p, 2, phttp->backend->v.reg.ws_to * 1000) > 0)
	    {

	      /*
	       * first read whatever is already in the input buffer
	       */
	      while (BIO_pending (phttp->cl))
		{
		  if (BIO_read (phttp->cl, &one, 1) != 1)
		    {
		      phttp_log (phttp, PHTTP_LOG_BACKEND,
				 -1, -1,
				 "error reading ws request");
		      return -1;
		    }
		  if (BIO_write (phttp->be, &one, 1) != 1)
		    {
		      phttp_log (phttp, PHTTP_LOG_BACKEND,
				 -1, -1,
				 "error writing ws request");
		      return -1;
		    }
		}
	      BIO_flush (phttp->be);

	      while (BIO_pending (phttp->be))
		{
		  if (BIO_read (phttp->be, &one, 1) != 1)
		    {
		      phttp_log (phttp,
				 PHTTP_LOG_BACKEND | PHTTP_LOG_REVERSE,
				 -1, errno,
				 "error reading ws response");
		      return -1;
		    }
		  if (BIO_write (phttp->cl, &one, 1) != 1)
		    {
		      if (errno)
			phttp_log (phttp, PHTTP_LOG_BACKEND,
				   -1, -1,
				   "error writing ws response");
		      return -1;
		    }
		  phttp->res_bytes++;
		}
	      BIO_flush (phttp->cl);

	      /*
	       * find the socket BIO in the chain
	       */
	      if ((cl_unbuf =
		   BIO_find_type (phttp->cl,
				  SLIST_EMPTY (&phttp->lstn->ctx_head)
				  ? BIO_TYPE_SOCKET : BIO_TYPE_SSL)) == NULL)
		{
		  logmsg (LOG_ERR,
			  "(%"PRItid") error getting unbuffered BIO: %s",
			  POUND_TID (), strerror (errno));
		  return -1;
		}
	      if ((be_unbuf = BIO_find_type (phttp->be,
					     backend_is_https (phttp->backend)
					       ? BIO_TYPE_SSL
					       : BIO_TYPE_SOCKET)) == NULL)
		{
		  logmsg (LOG_ERR,
			  "(%"PRItid") error getting unbuffered BIO: %s",
			  POUND_TID (), strerror (errno));
		  return -1;
		}

	      /*
	       * copy till EOF
	       */
	      if (p[0].revents)
		{
		  res = BIO_read (cl_unbuf, buf, sizeof (buf));
		  if (res <= 0)
		    {
		      break;
		    }
		  if (BIO_write (phttp->be, buf, res) != res)
		    {
		      if (errno)
			phttp_log (phttp, PHTTP_LOG_BACKEND,
				   -1, -1,
				   "error copying ws request body");
		      return -1;
		    }
		  else
		    {
		      BIO_flush (phttp->be);
		    }
		  p[0].revents = 0;
		}
	      if (p[1].revents)
		{
		  res = BIO_read (be_unbuf, buf, sizeof (buf));
		  if (res <= 0)
		    {
		      break;
		    }
		  if (BIO_write (phttp->cl, buf, res) != res)
		    {
		      if (errno)
			phttp_log (phttp,
				   PHTTP_LOG_DFL | PHTTP_LOG_REVERSE,
				   -1, errno,
				   "error copying ws response body");
		      return -1;
		    }
		  else
		    {
		      phttp->res_bytes += res;
		      BIO_flush (phttp->cl);
		    }
		  p[1].revents = 0;
		}
	    }
	}
    }
  while (resp_mode == RESP_SKIP);

  if (!be_11)
    close_backend (phttp);

  return HTTP_STATUS_OK;
}

/*
 * Pass the request to the backend.  Return 0 on success and pound
 * http error number otherwise.
 */
static int
send_to_backend (POUND_HTTP *phttp, int chunked, CONTENT_LENGTH content_length)
{
  struct http_header *hdr;
  char const *val;

  /*
   * this is the earliest we can check for Destination - we
   * had no back-end before
   */
  if (phttp->lstn->rewr_dest &&
      (hdr = http_header_list_locate (&phttp->request.headers,
				      HEADER_DESTINATION)) != NULL)
    {
      POUND_REGMATCH matches[4];

      if ((val = http_header_get_value (hdr)) == NULL)
	return HTTP_STATUS_INTERNAL_SERVER_ERROR;

      if (genpat_match (LOCATION, val, 4, matches))
	{
	  logmsg (LOG_ERR, "(%"PRItid") can't parse Destination %s",
		  POUND_TID (), val);
	}
      else
	{
	  char caddr[MAX_ADDR_BUFSIZE];
	  struct stringbuf sb;
	  char *p;

	  stringbuf_init_log (&sb);
	  str_be (caddr, sizeof (caddr), phttp->backend);
	  stringbuf_printf (&sb,
			    "Destination: %s://%s%s",
			    backend_is_https (phttp->backend) ? "https" : "http",
			    caddr,
			    val + matches[3].rm_so);
	  if ((p = stringbuf_finish (&sb)) == NULL)
	    {
	      stringbuf_free (&sb);
	      return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	    }

	  http_header_change (hdr, p, 0);
	}
    }

  /*
   * Add "canned" headers.
   */
  if (phttp->lstn->header_options & HDROPT_FORWARDED_HEADERS)
    {
      if (add_forwarded_headers (phttp))
	lognomem ();
    }

  if (phttp->ssl != NULL && (phttp->lstn->header_options & HDROPT_SSL_HEADERS))
    {
      if (add_ssl_headers (phttp))
	lognomem ();
    }

  if (phttp->backend->v.reg.servername)
    {
      struct stringbuf sb;
      char *hf;
      int rc;

      stringbuf_init_log (&sb);
      stringbuf_printf (&sb, "Host: %s", phttp->backend->v.reg.servername);
      hf = stringbuf_finish (&sb);
      if (!hf)
	{
	  stringbuf_free (&sb);
	  return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	}
      rc = http_header_list_append (&phttp->request.headers, hf, H_REPLACE);
      stringbuf_free (&sb);
      if (rc)
	return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

  if (rewrite_apply (phttp, &phttp->request, REWRITE_REQUEST))
    return HTTP_STATUS_INTERNAL_SERVER_ERROR;

  /*
   * Send the request and its headers
   */
  if (http_request_send (phttp->be, &phttp->request))
    {
      if (errno)
	phttp_log (phttp, PHTTP_LOG_BACKEND,
		   HTTP_STATUS_INTERNAL_SERVER_ERROR, errno,
		   "error writing to backend");
      return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

  /*
   * final CRLF
   */
  BIO_puts (phttp->be, "\r\n");

  if (chunked)
    {
      /*
       * had Transfer-encoding: chunked so read/write all the chunks
       * (HTTP/1.1 only)
       */
      int rc = copy_chunks (phttp->cl, phttp->be,
			    phttp->buffer, phttp->lstn->linebufsize,
			    phttp->backend->be_type != BE_REGULAR,
			    phttp->lstn->max_req_size, NULL);
      if (rc != HTTP_STATUS_OK)
	{
	  phttp_log (phttp, PHTTP_LOG_BACKEND,
		     rc, 0, "error sending chunks");
	  return rc;
	}
    }
  else if (content_length > 0)
    {
      /*
       * had Content-length, so do raw reads/writes for the length
       */
      int rc = copy_bin (phttp->cl, phttp->be, content_length,
			 phttp->backend->be_type != BE_REGULAR, NULL);
      switch (rc)
	{
	case COPY_OK:
	  break;

	case COPY_READ_ERR:
	case COPY_WRITE_ERR:
	  phttp_log (phttp, PHTTP_LOG_BACKEND,
		     HTTP_STATUS_INTERNAL_SERVER_ERROR, errno,
		     "%s while sending client request",
		     copy_status_string (rc));
	  return HTTP_STATUS_INTERNAL_SERVER_ERROR;

	default:
	  phttp_log (phttp, PHTTP_LOG_BACKEND,
		     HTTP_STATUS_INTERNAL_SERVER_ERROR, 0,
		     "%s while sending client request",
		     copy_status_string (rc));
	  return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	}
    }

  /*
   * flush to the back-end
   */
  if (BIO_flush (phttp->be) != 1)
    {
      phttp_log (phttp, PHTTP_LOG_BACKEND,
		 HTTP_STATUS_INTERNAL_SERVER_ERROR, errno,
		 "error flushing data");
      return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }
  return 0;
}

static int
open_backend (POUND_HTTP *phttp, BACKEND *backend, int sock)
{
  BIO *bb;

  /* Create new BIO */
  if ((phttp->be = BIO_new_socket (sock, 1)) == NULL)
    {
      phttp_log (phttp, PHTTP_LOG_DFL,
		 HTTP_STATUS_SERVICE_UNAVAILABLE, -1,
		 "BIO_new_socket server failed");
      shutdown (sock, 2);
      close (sock);
      return HTTP_STATUS_SERVICE_UNAVAILABLE;
    }

  /* Configure it */
  BIO_set_close (phttp->be, BIO_CLOSE);
  if (backend->v.reg.to > 0)
    {
      set_callback (phttp->be, backend->v.reg.to, &phttp->reneg_state);
    }

  /*
   * Set up SSL, if requested.
   */
  if (backend_is_https (backend))
    {
      SSL *be_ssl;

      if ((be_ssl = SSL_new (backend->v.reg.ctx)) == NULL)
	{
	  phttp_log (phttp, PHTTP_LOG_DFL,
		     HTTP_STATUS_SERVICE_UNAVAILABLE, -1,
		     "SSL_new to backend failed");
	  return HTTP_STATUS_SERVICE_UNAVAILABLE;
	}
      if (backend->v.reg.servername)
	SSL_set_tlsext_host_name (be_ssl, backend->v.reg.servername);
      SSL_set_bio (be_ssl, phttp->be, phttp->be);
      if ((bb = BIO_new (BIO_f_ssl ())) == NULL)
	{
	  phttp_log (phttp, PHTTP_LOG_DFL,
		     HTTP_STATUS_SERVICE_UNAVAILABLE, -1,
		     "BIO_new(Bio_f_ssl()) failed");
	  return HTTP_STATUS_SERVICE_UNAVAILABLE;
	}

      BIO_set_ssl (bb, be_ssl, BIO_CLOSE);
      BIO_set_ssl_mode (bb, 1);
      phttp->be = bb;

      if (BIO_do_handshake (phttp->be) <= 0)
	{
	  phttp_log (phttp, PHTTP_LOG_HANDSHAKE,
		     HTTP_STATUS_SERVICE_UNAVAILABLE, -1,
		     "handshake with backend failed");
	  return HTTP_STATUS_SERVICE_UNAVAILABLE;
	}
    }

  if ((bb = BIO_new (BIO_f_buffer ())) == NULL)
    {
      phttp_log (phttp, PHTTP_LOG_DFL,
		 HTTP_STATUS_SERVICE_UNAVAILABLE, -1,
		 "BIO_new(buffer) server failed");
      return HTTP_STATUS_SERVICE_UNAVAILABLE;
    }

  BIO_set_buffer_size (bb, MAXBUF);
  BIO_set_close (bb, BIO_CLOSE);
  phttp->be = BIO_push (bb, phttp->be);

  return 0;
}

static int
select_backend (POUND_HTTP *phttp)
{
  BACKEND *backend;
  int sock;
  char caddr[MAX_ADDR_BUFSIZE];

  if ((backend = get_backend (phttp)) != NULL)
    {
      if (phttp->be != NULL)
	{
	  if (backend == phttp->backend)
	    {
	      /* Same backend as before: nothing to do */
	      backend_unref (backend);
	      return 0;
	    }
	  else
	    close_backend (phttp);
	}

      do
	{
	  if (backend->be_type == BE_REGULAR)
	    {
	      /*
	       * Try to open connection to this backend.
	       */
	      int sock_proto;

	      switch (backend->v.reg.addr.ai_family)
		{
		case AF_INET:
		  sock_proto = PF_INET;
		  break;

		case AF_INET6:
		  sock_proto = PF_INET6;
		  break;

		case AF_UNIX:
		  sock_proto = PF_UNIX;
		  break;

		default:
		  phttp_log (phttp, PHTTP_LOG_DFL,
			     HTTP_STATUS_SERVICE_UNAVAILABLE, 0,
			     "backend: unknown family %d",
			     backend->v.reg.addr.ai_family);
		  backend_unref (backend);
		  return HTTP_STATUS_SERVICE_UNAVAILABLE;
		}

	      if ((sock = socket (sock_proto, SOCK_STREAM, 0)) < 0)
		{
		  phttp_log (phttp, PHTTP_LOG_DFL,
			     HTTP_STATUS_SERVICE_UNAVAILABLE, errno,
			     "backend %s: socket create",
			     str_be (caddr, sizeof (caddr), backend));
		  backend_unref (backend);
		  return HTTP_STATUS_SERVICE_UNAVAILABLE;
		}

	      if (connect_nb (sock, &backend->v.reg.addr,
			      backend->v.reg.conn_to) == 0)
		{
		  int res;

		  /*
		   * Connected successfully.  Open the backend connection.
		   */
		  if (sock_proto == PF_INET || sock_proto == PF_INET6)
		    socket_setup (sock);
		  if ((res = open_backend (phttp, backend, sock)) != 0)
		    {
		      close_backend (phttp);
		      return res;
		    }
		  /* New backend selected. */
		  backend_unref (phttp->backend);
		  phttp->backend = backend;
		  return 0;
		}

	      phttp_log (phttp, PHTTP_LOG_DFL,
			 -1, errno, "can't connect to backend %s",
			 str_be (caddr, sizeof (caddr), backend));
	      shutdown (sock, 2);
	      close (sock);
	      /*
	       * The following will close all sessions associated with
	       * this backend, and mark the backend itself as dead, so
	       * that it won't be returned again by the call to get_backed
	       * below.
	       */
	      kill_be (phttp->svc, backend, BE_KILL);
	      /* Try next backend now. */
	      backend_unref (backend);
	    }
	  else
	    {
	      /* New backend selected. */
	      backend_unref (phttp->backend);
	      phttp->backend = backend;
	      return 0;
	    }
	}
      while ((backend = get_backend (phttp)) != NULL);
    }

  /*
   * No backend found or is available.
   */
  phttp_log (phttp, PHTTP_LOG_DFL,
	     HTTP_STATUS_SERVICE_UNAVAILABLE, 0, "no backend found");

  return HTTP_STATUS_SERVICE_UNAVAILABLE;
}

static void
backend_update_stats (BACKEND *be, struct timespec const *start,
		      struct timespec const *end)
{
  struct timespec diff;
  double t;

  pthread_mutex_lock (&be->mut);
  diff = timespec_sub (end, start);
  t = (double) diff.tv_sec * 1e9 + diff.tv_nsec;
  be->avgtime = (be->numreq * be->avgtime + t) / (be->numreq + 1);
  be->avgsqtime = (be->numreq * be->avgsqtime + t*t) / (be->numreq + 1);
  be->numreq++;
  pthread_mutex_unlock (&be->mut);
}

enum transfer_encoding
  {
    TRANSFER_ENCODING_NONE,
    TRANSFER_ENCODING_CHUNKED,
    TRANSFER_ENCODING_UNKNOWN
  };

enum { HTTP_ABORT = -2 };

static inline int
is_http10 (POUND_HTTP *phttp)
{
  return phttp->request.version == 0;
}

static int
http_process_request (POUND_HTTP *phttp)
{
  int res;  /* General-purpose result variable */
  CONTENT_LENGTH content_length;
  struct http_header *hdr, *hdrtemp;
  char *val;
  /* FIXME: this belongs to struct http_request, perhaps. */
  int transfer_encoding;

  /*
   * check for correct request
   */
  if ((res = parse_http_request (&phttp->request,
				 phttp->lstn->verb)) != HTTP_STATUS_OK)
    {
      phttp_log (phttp, PHTTP_LOG_DFL,
		 res, 0, "error parsing request");
      return res;
    }

  phttp->no_cont = phttp->request.method == METH_HEAD;
  if (phttp->request.method == METH_GET)
    phttp->ws_state |= WSS_REQ_GET;

  if (phttp->lstn->url_pat &&
      genpat_match (phttp->lstn->url_pat, phttp->request.url, 0, NULL))
    {
      phttp_log (phttp, PHTTP_LOG_DFL,
		 HTTP_STATUS_NOT_IMPLEMENTED, 0,
		 "bad URL \"%s\"", phttp->request.url);
      return HTTP_STATUS_NOT_IMPLEMENTED;
    }

  /*
   * check headers
   */
  transfer_encoding = TRANSFER_ENCODING_NONE;
  content_length = NO_CONTENT_LENGTH;
  DLIST_FOREACH_SAFE (hdr, hdrtemp, &phttp->request.headers, link)
    {
      switch (hdr->code)
	{
	case HEADER_CONNECTION:
	  if ((val = http_header_get_value (hdr)) == NULL)
	    return HTTP_ABORT;
	  if (cs_locate_token (val, "close", NULL))
	    phttp->conn_closed = 1;
	  /*
	   * Connection: upgrade
	   */
	  else if (genpat_match (CONN_UPGRD, val, 0, NULL) == 0)
	    phttp->ws_state |= WSS_REQ_HEADER_CONNECTION_UPGRADE;
	  break;

	case HEADER_UPGRADE:
	  if ((val = http_header_get_value (hdr)) == NULL)
	    return HTTP_ABORT;
	  if (cs_locate_token (val, "websocket", NULL))
	    phttp->ws_state |= WSS_REQ_HEADER_UPGRADE_WEBSOCKET;
	  break;

	case HEADER_TRANSFER_ENCODING:
	  if ((val = http_header_get_value (hdr)) == NULL)
	    return HTTP_ABORT;
	  else if (transfer_encoding == TRANSFER_ENCODING_CHUNKED)
	    {
	      phttp_log (phttp, PHTTP_LOG_DFL,
			 HTTP_STATUS_BAD_REQUEST, 0,
			 "multiple Transfer-Encoding headers");
	      return HTTP_STATUS_BAD_REQUEST;
	    }
	  else
	    {
	      char *next;
	      if (cs_locate_token (val, "chunked", &next))
		{
		  if (next)
		    {
		      /*
		       * When the "chunked" transfer-coding is used,
		       * it MUST be the last transfer-coding applied
		       * to the message-body.
		       */
		      phttp_log (phttp, PHTTP_LOG_DFL,
				 HTTP_STATUS_BAD_REQUEST, 0,
				 "multiple Transfer-Encoding headers");
		      return HTTP_STATUS_BAD_REQUEST;
		    }
		  transfer_encoding = TRANSFER_ENCODING_CHUNKED;
		}
	      else
		transfer_encoding = TRANSFER_ENCODING_UNKNOWN;
	    }
	  break;

	case HEADER_CONTENT_LENGTH:
	  if ((val = http_header_get_value (hdr)) == NULL)
	    return HTTP_ABORT;
	  if (content_length != NO_CONTENT_LENGTH || strchr (val, ','))
	    {
	      phttp_log (phttp, PHTTP_LOG_DFL,
			 HTTP_STATUS_BAD_REQUEST, 0,
			 "multiple Content-Length headers");
	      return HTTP_STATUS_BAD_REQUEST;
	    }
	  else if ((content_length = get_content_length (val, CL_HEADER)) == NO_CONTENT_LENGTH)
	    {
	      phttp_log (phttp, PHTTP_LOG_DFL,
			 HTTP_STATUS_BAD_REQUEST, 0,
			 "bad Content-Length value");
	      return HTTP_STATUS_BAD_REQUEST;
	    }

	  if (content_length == NO_CONTENT_LENGTH)
	    {
	      http_header_list_remove (&phttp->request.headers, hdr);
	    }
	  break;

	case HEADER_EXPECT:
	  /*
	   * We do NOT support the "Expect: 100-continue" headers;
	   * Supporting them may involve severe performance penalties
	   * (non-responding back-end, etc).
	   * As a stop-gap measure we just skip these headers.
	   */
	  if ((val = http_header_get_value (hdr)) == NULL)
	    return HTTP_ABORT;
	  if (!c_strcasecmp ("100-continue", val))
	    {
	      http_header_list_remove (&phttp->request.headers, hdr);
	    }
	  break;

	case HEADER_ILLEGAL:
	  /*
	   * FIXME: This should not happen.  See the handling of
	   * HEADER_ILLEGAL in http_header_list_append.
	   */
	  phttp_log (phttp, PHTTP_LOG_DFL,
		     -1, 0, "bad header: %s", hdr->header);
	  http_header_list_remove (&phttp->request.headers, hdr);
	  break;
	}
    }

  /*
   * check for possible request smuggling attempt
   */
  if (transfer_encoding != TRANSFER_ENCODING_NONE)
    {
      if (transfer_encoding == TRANSFER_ENCODING_UNKNOWN)
	{
	  phttp_log (phttp, PHTTP_LOG_DFL,
		     HTTP_STATUS_NOT_IMPLEMENTED, 0,
		     "unknown Transfer-Encoding");
	  return HTTP_STATUS_NOT_IMPLEMENTED;
	}
      else if (content_length != NO_CONTENT_LENGTH)
	{
	  phttp_log (phttp, PHTTP_LOG_DFL,
		     HTTP_STATUS_BAD_REQUEST, 0,
		     "both Transfer-Encoding and Content-Length given");
	  return HTTP_STATUS_BAD_REQUEST;
	}
    }

  if (phttp->lstn->max_uri_length > 0)
    {
      int rc;
      char const *url;
      rc = http_request_get_url (&phttp->request, &url);
      if (rc == RETRIEVE_OK)
	{
	  size_t urlen;
	  if ((urlen = strlen (url)) > phttp->lstn->max_uri_length)
	    {
	      phttp_log (phttp, PHTTP_LOG_DFL,
			 HTTP_STATUS_URI_TOO_LONG, 0,
			 "URI too long: %zu bytes",
			 urlen);
	      return HTTP_STATUS_URI_TOO_LONG;
	    }
	}
      else
	{
	  phttp_log (phttp, PHTTP_LOG_DFL,
		     HTTP_STATUS_INTERNAL_SERVER_ERROR, 0,
		     "can't get URL from the request (shouldn't happen)");
	  return HTTP_ABORT;
	}
    }

  /*
   * possibly limited request size
   */
  if (phttp->lstn->max_req_size > 0 && content_length > 0
      && content_length > phttp->lstn->max_req_size)
    {
      phttp_log (phttp, PHTTP_LOG_DFL,
		 HTTP_STATUS_PAYLOAD_TOO_LARGE, 0,
		 "request too large: %"PRICLEN" bytes",
		 content_length);
      return HTTP_STATUS_PAYLOAD_TOO_LARGE;
    }

  if (phttp->be != NULL)
    {
      if (is_readable (phttp->be, 0))
	{
	  /*
	   * The only way it's readable is if it's at EOF, so close
	   * it!
	   */
	  close_backend (phttp);
	}
    }

  /*
   * check that the requested URL still fits the old back-end (if
   * any)
   */
  if ((phttp->svc = get_service (phttp)) == NULL)
    {
      phttp_log (phttp, PHTTP_LOG_DFL,
		 HTTP_STATUS_SERVICE_UNAVAILABLE, 0,
		 "no suitable service found");
      return HTTP_STATUS_SERVICE_UNAVAILABLE;
    }

  if ((res = select_backend (phttp)) != 0)
    return res;

  /*
   * if we have anything but a regular backend we close the channel
   */
  if (phttp->be != NULL && phttp->backend->be_type != BE_REGULAR)
    close_backend (phttp);

  if (force_http_10 (phttp))
    phttp->conn_closed = 1;

  phttp->res_bytes = 0;
  http_request_free (&phttp->response);

  clock_gettime (CLOCK_REALTIME, &phttp->be_start);
  switch (phttp->backend->be_type)
    {
    case BE_REDIRECT:
      res = redirect_response (phttp);
      break;

    case BE_ACME:
      res = acme_response (phttp);
      break;

    case BE_CONTROL:
      res = control_response (phttp);
      break;

    case BE_ERROR:
      res = error_response (phttp);
      break;

    case BE_METRICS:
      res = metrics_response (phttp);
      break;

    case BE_REGULAR:
      /* Send the request. */
      res = send_to_backend (phttp,
			     !is_http10 (phttp) &&
			     (transfer_encoding == TRANSFER_ENCODING_CHUNKED),
			     content_length);
      if (res == 0)
	/* Process the response. */
	res = backend_response (phttp);
      break;

    case BE_FILE:
      res = file_response (phttp);
      break;

    case BE_LUA:
      res = lua_response (phttp);
      break;

    case BE_MATRIX:
    case BE_BACKEND_REF:
      /* shouldn't happen */
      abort ();
    }
  return res;
}

/*
 * handle an HTTP request
 */
void
do_http (POUND_HTTP *phttp)
{
  int res;  /* General-purpose result variable */
  BIO *bb;

  if (phttp->lstn->allow_client_reneg)
    phttp->reneg_state = RENEG_ALLOW;
  else
    phttp->reneg_state = RENEG_INIT;

  socket_setup (phttp->sock);

  if ((phttp->cl = BIO_new_socket (phttp->sock, 1)) == NULL)
    {
      phttp_log (phttp, PHTTP_LOG_DFL,
		 -1, -1, "BIO_new_socket failed");
      shutdown (phttp->sock, 2);
      close (phttp->sock);
      return;
    }
  set_callback (phttp->cl, phttp->lstn->to, &phttp->reneg_state);

  if (!SLIST_EMPTY (&phttp->lstn->ctx_head))
    {
      if ((phttp->ssl = SSL_new (SLIST_FIRST (&phttp->lstn->ctx_head)->ctx)) == NULL)
	{
	  phttp_log (phttp, PHTTP_LOG_DFL,
		     -1, -1, "SSL_new failed");
	  return;
	}
      SSL_set_app_data (phttp->ssl, &phttp->reneg_state);
      SSL_set_bio (phttp->ssl, phttp->cl, phttp->cl);
      if ((bb = BIO_new (BIO_f_ssl ())) == NULL)
	{
	  phttp_log (phttp, PHTTP_LOG_DFL,
		     -1, -1, "BIO_new(Bio_f_ssl()) failed");
	  return;
	}
      BIO_set_ssl (bb, phttp->ssl, BIO_CLOSE);
      BIO_set_ssl_mode (bb, 0);
      phttp->cl = bb;
      if (BIO_do_handshake (phttp->cl) <= 0)
	{
	  phttp_log (phttp, PHTTP_LOG_HANDSHAKE,
		     -1, -1, "handshake failed");
	  return;
	}
      else
	{
	  if ((phttp->x509 = SSL_get_peer_certificate (phttp->ssl)) != NULL
	      && phttp->lstn->clnt_check < 3
	      && SSL_get_verify_result (phttp->ssl) != X509_V_OK)
	    {
	      phttp_log (phttp, PHTTP_LOG_DFL,
			 -1, 0, "bad certificate");
	      return;
	    }
	}
    }
  else
    {
      X509_free (phttp->x509);
      phttp->x509 = NULL;
    }
  phttp->backend = NULL;

  if ((bb = BIO_new (BIO_f_buffer ())) == NULL)
    {
      phttp_log (phttp, PHTTP_LOG_DFL, -1, -1, "BIO_new failed");
      return;
    }
  BIO_set_close (phttp->cl, BIO_CLOSE);
  BIO_set_buffer_size (phttp->cl, MAXBUF);
  phttp->cl = BIO_push (bb, phttp->cl);

  for (;;)
    {
      http_request_free (&phttp->request);
      http_request_free (&phttp->response);
      submatch_queue_free (&phttp->smq);
      phttp_lua_stash_reset (phttp);

      phttp->ws_state = WSS_INIT;
      phttp->conn_closed = 0;
      clock_gettime (CLOCK_REALTIME, &phttp->start_req);
      res = http_request_read (phttp->cl, phttp->lstn, phttp->buffer,
			       &phttp->request);
      if (res != HTTP_STATUS_OK)
	{
	  if (errno)
	    {
	      phttp_log (phttp, PHTTP_LOG_DFL,
			 res, errno, "error reading request");
	    }
	  clock_gettime (CLOCK_REALTIME, &phttp->end_req);
	}
      else if (phttp->request.request == NULL)
	break;
      else
	{
	  clock_gettime (CLOCK_REALTIME, &phttp->start_req);
	  res = http_process_request (phttp);
	  clock_gettime (CLOCK_REALTIME, &phttp->end_req);
	  if (enable_backend_stats && phttp->backend)
	    backend_update_stats (phttp->backend, &phttp->be_start,
				  &phttp->end_req);
	}

      if (res < 0)
	http_err_reply (phttp, HTTP_STATUS_INTERNAL_SERVER_ERROR);
      else if (res != HTTP_STATUS_OK)
	http_err_reply (phttp, res);

      if (phttp->response_code == 0)
	phttp->response_code = http_status[res].code;
      http_log (phttp);

      if (res < 0)
	break;

      /*
       * Stop processing if:
       *  - client is not HTTP/1.1
       *      or
       *  - we had a "Connection: closed" header
       */
      if (is_http10 (phttp) || phttp->conn_closed)
	break;
    }
}

void *
thr_http (void *dummy)
{
  POUND_HTTP *phttp;

  while ((phttp = pound_http_dequeue ()) != NULL)
    {
      do_http (phttp);
      clear_error (phttp->ssl);
      pound_http_destroy (phttp);
      active_threads_decr ();
    }
  logmsg (LOG_NOTICE, "(%"PRItid") thread terminating on idle timeout",
	  POUND_TID ());
  return NULL;
}
