/*
 * Pound - the reverse-proxy load-balancer
 * Copyright (C) 2002-2010 Apsis GmbH
 * Copyright (C) 2018-2024 Sergey Poznyakoff
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
 * Emit to BIO response line with the given CODE, descriptive TEXT and
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
  BIO_printf (bio, "HTTP/1.%d %d %s\r\n"
	      "Content-Type: %s\r\n",
	      proto,
	      code,
	      text,
	      type);
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

/*
 * Write to BIO an error HTTP 1.PROTO response.  ERR is one of
 * HTTP_STATUS_* constants.  TXT supplies the error page content.
 * If it is NULL, the default text from the http_status table is
 * used.
 */
static void
bio_err_reply (BIO *bio, int proto, int err, char const *content)
{
  struct stringbuf sb;

  if (!(err >= 0 && err < HTTP_STATUS_MAX))
    {
      logmsg (LOG_NOTICE, "INTERNAL ERROR: unsupported error code in call to bio_err_reply");
      err = HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

  stringbuf_init_log (&sb);
  if (!content)
    {
      stringbuf_printf (&sb, default_error_page,
			http_status[err].code,
			http_status[err].reason,
			http_status[err].reason,
			http_status[err].text);
      if ((content = stringbuf_finish (&sb)) == NULL)
	content = http_status[err].text;
    }
  bio_http_reply (bio, proto, http_status[err].code, http_status[err].reason,
		  err_headers, "text/html", content);
  stringbuf_free (&sb);
}

/*
 * Send error response to the client BIO.  ERR is one of HTTP_STATUS_*
 * constants.  Status code and reason phrase will be taken from the
 * http_status array.  If custom error page is defined in the listener,
 * it will be used, otherwise the default error page for the ERR code
 * will be generated
 */
static void
http_err_reply (POUND_HTTP *phttp, int err)
{
  bio_err_reply (phttp->cl, phttp->request.version, err,
		 phttp->lstn->http_err[err]);
  phttp->conn_closed = 1;
}

static int
submatch_realloc (struct submatch *sm, regex_t const *re)
{
  size_t n = re->re_nsub + 1;
  if (n > sm->matchmax)
    {
      regmatch_t *p = realloc (sm->matchv, n * sizeof (p[0]));
      if (!p)
	return -1;
      sm->matchmax = n;
      sm->matchv = p;
    }
  sm->matchn = n;
  return 0;
}

static void
submatch_free (struct submatch *sm)
{
  free (sm->matchv);
  free (sm->subject);
  submatch_init (sm);
}

static void
submatch_reset (struct submatch *sm)
{
  free (sm->subject);
  sm->subject = NULL;
  sm->matchn = 0;
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
submatch_queue_push (struct submatch_queue *smq)
{
  struct submatch *sm;
  smq->cur = (smq->cur + 1) % SMQ_SIZE;
  sm = submatch_queue_get (smq, 0);
  submatch_reset (sm);
  return sm;
}

static int
submatch_exec (regex_t const *re, char const *subject, struct submatch *sm)
{
  int res;

  submatch_free (sm);
  if (submatch_realloc (sm, re))
    {
      lognomem ();
      return 0;
    }
  res = regexec (re, subject, sm->matchn, sm->matchv, 0) == 0;
  if (res)
    {
      if ((sm->subject = strdup (subject)) == NULL)
	return 0;
    }
  return res;
}

static int http_request_get_query_param (struct http_request *,
					 char const *, size_t,
					 struct query_param **);

typedef int (*accessor_func) (struct http_request *, char const *, int,
			      char const **, size_t *);

struct accessor
{
  char *name;
  accessor_func func;
  int arg;
};

static int
accessor_url (struct http_request *req, char const *arg, int arglen,
	      char const **ret_val, size_t *ret_len)
{
  char const *val;
  int rc = http_request_get_url (req, &val);
  if (rc == RETRIEVE_OK && val)
    {
      *ret_len = strlen (val);
      *ret_val = val;
    }
  return rc;
}

static int
accessor_path (struct http_request *req, char const *arg, int arglen,
	       char const **ret_val, size_t *ret_len)
{
  char const *val;
  int rc = http_request_get_path (req, &val);
  if (rc == RETRIEVE_OK && val)
    {
      *ret_len = strlen (val);
      *ret_val = val;
    }
  return rc;
}

static int
accessor_query (struct http_request *req, char const *arg, int arglen,
		char const **ret_val, size_t *ret_len)
{
  char const *val;
  int rc = http_request_get_query (req, &val);
  if (rc == RETRIEVE_OK && val)
    {
      *ret_len = strlen (val);
      *ret_val = val;
    }
  return rc;
}

static int
accessor_param (struct http_request *req, char const *arg, int arglen,
		char const **ret_val, size_t *ret_len)
{
  struct query_param *qp;
  int rc;
  if ((rc = http_request_get_query_param (req, arg, arglen, &qp)) == RETRIEVE_OK)
    {
      *ret_val = qp->value;
      if (qp->value)
	*ret_len = strlen (qp->value);
    }
  return rc;
}

static int
accessor_header (struct http_request *req, char const *arg, int arglen,
		 char const **ret_val, size_t *ret_len)
{
  struct http_header *hdr;

  if ((hdr = http_header_list_locate_name (&req->headers, arg, arglen)) == NULL)
    return RETRIEVE_NOT_FOUND;

  if ((*ret_val = http_header_get_value (hdr)) != NULL)
    *ret_len = strlen (*ret_val);
  return RETRIEVE_OK;
}

static int
accessor_host (struct http_request *req, char const *arg, int arglen,
	       char const **ret_val, size_t *ret_len)
{
  char const *host;
  size_t len;
  int rc = accessor_header (req, "host", 4, &host, &len);
  if (rc == RETRIEVE_OK && host)
    {
      char *p;
      *ret_val = host;
      if ((p = memchr (host, ':', len)) != NULL)
	*ret_len = p - host;
      else
	*ret_len = len;
    }
  return rc;
}

static int
accessor_port (struct http_request *req, char const *arg, int arglen,
	       char const **ret_val, size_t *ret_len)
{
  char const *host;
  size_t len;
  int rc = accessor_header (req, "host", 4, &host, &len);
  if (rc == RETRIEVE_OK && host)
    {
      char *p;
      if ((p = memchr (host, ':', len)) != NULL)
	{
	  *ret_val = p;
	  *ret_len = len - (p - host);
	}
      else
	{
	  *ret_val = host + len;
	  *ret_len = 0;
	}
    }
  return rc;
}

static struct accessor accessors[] = {
  { "url",      accessor_url },
  { "path",     accessor_path },
  { "query",    accessor_query },
  { "param",    accessor_param, 1 },
  { "header",   accessor_header, 1 },
  { "host",     accessor_host },
  { "port",     accessor_port },
  { NULL }
};

static accessor_func
find_accessor (char const *input, size_t len, char **ret_arg, size_t *ret_arglen)
{
  struct accessor *ap;
  char const *arg = NULL;
  size_t arglen = 0;
  size_t i;

  while (len && (*input == ' ' || *input == '\t'))
    {
      input++;
      len--;
    }
  if (len == 0)
    return NULL;

  for (i = 0; i < len; i++)
    if (input[i] == ' ' || input[i] == '\t')
      break;

  for (ap = accessors; ; ap++)
    {
      if (ap->name == NULL)
	return NULL;
      if (strlen (ap->name) == i && memcmp (ap->name, input, i) == 0)
	break;
    }

  if (ap->arg)
    {
      if (!isspace (input[i]))
	return NULL;

      do
	{
	  i++;
	  if (i == len)
	    return NULL;
	}
      while (isspace (input[i]));

      arg = input + i;
      arglen = len - i;

      do
	{
	  if (arglen == 0)
	    return NULL;
	}
      while (isspace (arg[arglen-1]));
    }
  else
    {
      while (i < len && isspace(input[i]))
	i++;
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
      else if (str[0] == '%' && isxdigit (str[1]) && isxdigit (str[2]))
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
	  char const *val;

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
	  else if (acc (&phttp->request, arg, arglen, &val, &len) == 0 && val)
	    {
	      stringbuf_add (sb, val, len);
	      if (result >= 0)
		result++;
	    }

	  str = q + 1;
	}
      else if ((brace = (str[1] == '{')) || isdigit (str[1]))
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
		  errno = 0;
		  n = strtoul (p + 1, &p, 10);
		  if (errno || *p != ')')
		    {
		      int len = str - start + 1;
		      logmsg (LOG_WARNING,
			      "%s \"%s\": missing closing parenthesis in"
			      " reference started in position %d ",
			      what, start, len);
		      stringbuf_add (sb, str, p - str);
		      str = p;
		      result = -1;
		      continue;
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
		      int n = str - start + 1;
		      logmsg (LOG_WARNING,
			      "%s \"%s\": missing closing brace in reference"
			      " started in position %d", what, start, n);
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
		  "%s \"%s\": unescaped %% character in position %d",
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

static int rewrite_apply (REWRITE_RULE_HEAD *rewrite_rules,
			  struct http_request *request, POUND_HTTP *phttp);

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
      logmsg (LOG_NOTICE,
	      "INTERNAL ERROR: unsupported status code %d passed to"
	      " redirect_response; please report", code);
      return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

  if (rewrite_apply (&phttp->lstn->rewrite[REWRITE_REQUEST], &phttp->request,
		     phttp) ||
      rewrite_apply (&phttp->svc->rewrite[REWRITE_REQUEST], &phttp->request,
		     phttp))
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
      if (isalnum (xurl[i]) || xurl[i] == '_' || xurl[i] == '.'
	  || xurl[i] == ':' || xurl[i] == '/' || xurl[i] == '?'
	  || xurl[i] == '&' || xurl[i] == ';' || xurl[i] == '-'
	  || xurl[i] == '=')
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

/*
 * Read and write some binary data
 */
static int
copy_bin (BIO *cl, BIO *be, CONTENT_LENGTH cont, CONTENT_LENGTH *res_bytes,
	  int no_write)
{
  char buf[MAXBUF];
  int res;

  while (cont > 0)
    {
      if ((res = BIO_read (cl, buf, cont > sizeof (buf) ? sizeof (buf) : cont)) < 0)
	return -1;
      else if (res == 0)
	return -2;
      if (!no_write)
	if (BIO_write (be, buf, res) != res)
	  return -3;
      cont -= res;
      if (res_bytes)
	*res_bytes += res;
    }
  if (!no_write)
    if (BIO_flush (be) != 1)
      return -4;
  return 0;
}

static int
acme_response (POUND_HTTP *phttp)
{
  int fd;
  struct stat st;
  BIO *bin;
  char *file_name;
  int rc = HTTP_STATUS_OK;

  if (rewrite_apply (&phttp->lstn->rewrite[REWRITE_REQUEST], &phttp->request,
		     phttp) ||
      rewrite_apply (&phttp->svc->rewrite[REWRITE_REQUEST], &phttp->request,
		     phttp))
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
	  logmsg (LOG_ERR, "can't open %s: %s", file_name, strerror (errno));
	  rc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
	}
    }
  else if (fstat (fd, &st))
    {
      logmsg (LOG_ERR, "can't stat %s: %s", file_name, strerror (errno));
      close (fd);
      rc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }
  else
    {
      bin = BIO_new_fd (fd, BIO_CLOSE);

      phttp->response_code = 200;
      bio_http_reply_start (phttp->cl, phttp->request.version, 200, "OK", NULL,
			    "text/html", (CONTENT_LENGTH) st.st_size);

      if (copy_bin (bin, phttp->cl, st.st_size, NULL, 0))
	{
	  if (errno)
	    logmsg (LOG_NOTICE, "(%"PRItid") error copying file %s: %s",
		    POUND_TID (), file_name, strerror (errno));
	}

      BIO_free (bin);
      BIO_flush (phttp->cl);
    }
  free (file_name);
  return rc;
}

int
parse_header_text (HTTP_HEADER_LIST *head, char const *text)
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
	  http_header_list_append (head, hdr, H_REPLACE))
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
  return res;
}

static int
error_response (POUND_HTTP *phttp)
{
  int err = phttp->backend->v.error.status;
  const char *text = phttp->backend->v.error.text
			? phttp->backend->v.error.text
			: phttp->lstn->http_err[err]
			    ? phttp->lstn->http_err[err]
			    : http_status[err].text;
  size_t len = strlen (text);
  struct http_request req;
  BIO *bin;

  http_request_init (&req);
  if (parse_header_text (&req.headers, err_headers))
    return HTTP_STATUS_INTERNAL_SERVER_ERROR;

  if (rewrite_apply (&phttp->lstn->rewrite[REWRITE_RESPONSE], &req, phttp) ||
      rewrite_apply (&phttp->svc->rewrite[REWRITE_RESPONSE], &req, phttp))
    return HTTP_STATUS_INTERNAL_SERVER_ERROR;

  bin = BIO_new_mem_buf (text, len);

  bio_http_reply_start_list (phttp->cl,
			     phttp->request.version,
			     http_status[err].code,
			     http_status[err].reason,
			     &req.headers,
			     (CONTENT_LENGTH) len);
  if (copy_bin (bin, phttp->cl, len, NULL, 0))
    {
      if (errno)
	logmsg (LOG_NOTICE, "(%"PRItid") error sending response %d: %s",
		POUND_TID (), http_status[err].code, strerror (errno));
    }

  BIO_free (bin);
  BIO_flush (phttp->cl);
  http_request_free
    (&req);
  phttp->response_code = http_status[err].code;
  return 0;
}

/*
 * Get a "line" from a BIO, strip the trailing newline, skip the input
 * stream if buffer too small.
 * The result buffer is NULL terminated.
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
	return -1;
      case 0:
      case -1:
	return 1;
      default:
	if (seen_cr)
	  {
	    if (tmp != '\n')
	      {
		/*
		 * we have CR not followed by NL
		 */
		do
		  {
		    if (BIO_read (in, &tmp, 1) <= 0)
		      return 1;
		  }
		while (tmp != '\n');
		return 1;
	      }
	    else
	      {
		buf[i - 1] = '\0';
		return 0;
	      }
	  }

	if (!iscntrl (tmp) || tmp == '\t')
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
	do
	  {
	    if (BIO_read (in, &tmp, 1) <= 0)
	      return 1;
	  }
	while (tmp != '\n');
	return 1;
      }

  /*
   * line too long
   */
  do
    {
      if (BIO_read (in, &tmp, 1) <= 0)
	return 1;
    }
  while (tmp != '\n');
  return 1;
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
    {
      while (*arg == ' ' || *arg == '\t')
	arg++;
    }

  if (strtoclen (arg, mode == CL_HEADER ? 10 : 16, &n, &p))
    return NO_CONTENT_LENGTH;
  while (*p == ' ' || *p == '\t')
    p++;
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
copy_chunks (BIO *cl, BIO *be, CONTENT_LENGTH *res_bytes, int no_write,
	     CONTENT_LENGTH max_size)
{
  char buf[MAXBUF];
  CONTENT_LENGTH cont, tot_size;
  int res;

  for (tot_size = 0;;)
    {
      if ((res = get_line (cl, buf, sizeof (buf))) < 0)
	{
	  logmsg (LOG_NOTICE, "(%"PRItid" chunked read error: %s",
		  POUND_TID (),
		  strerror (errno));
	  return res < 0
	         ? HTTP_STATUS_INTERNAL_SERVER_ERROR
	         : HTTP_STATUS_BAD_REQUEST;
	}
      else if (res > 0)
	/*
	 * EOF
	 */
	return HTTP_STATUS_OK;

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
	  if (copy_bin (cl, be, cont, res_bytes, no_write))
	    {
	      if (errno)
		logmsg (LOG_NOTICE, "(%"PRItid") error copyinh chunk of length %"PRICLEN": %s",
			POUND_TID (), cont, strerror (errno));
	      return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	    }
	}
      else
	break;
      /*
       * final CRLF
       */
      if ((res = get_line (cl, buf, sizeof (buf))) < 0)
	{
	  logmsg (LOG_NOTICE, "(%"PRItid") error after chunk: %s",
		  POUND_TID (),
		  strerror (errno));
	  return res < 0
	         ? HTTP_STATUS_INTERNAL_SERVER_ERROR
	         : HTTP_STATUS_BAD_REQUEST;
	}
      else if (res > 0)
	{
	  logmsg (LOG_NOTICE, "(%"PRItid") unexpected EOF after chunk",
		  POUND_TID ());
	  return HTTP_STATUS_BAD_REQUEST;
	}
      if (buf[0])
	logmsg (LOG_NOTICE, "(%"PRItid") unexpected after chunk \"%s\"",
		POUND_TID (), buf);
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
      if ((res = get_line (cl, buf, sizeof (buf))) < 0)
	{
	  logmsg (LOG_NOTICE, "(%"PRItid") error post-chunk: %s",
		  POUND_TID (),
		  strerror (errno));
	  return res < 0
	         ? HTTP_STATUS_INTERNAL_SERVER_ERROR
	         : HTTP_STATUS_BAD_REQUEST;
	}
      else if (res > 0)
	break;
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
(BIO *bio, int cmd, const char *argp, int argi,
	      long argl, long ret)
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
		  logmsg (LOG_WARNING, "(%lx) CALLBACK read 0x%04x poll: %s",
			  pthread_self (), p.revents, strerror (p_err));
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
		  logmsg (LOG_WARNING, "(%lx) CALLBACK write 0x%04x poll: %s",
			  pthread_self (), p.revents, strerror (p_err));
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
		  "(%lx) CALLBACK timeout poll after %d secs: %s",
		  pthread_self (), to / 1000, strerror (p_err));
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
	      logmsg (LOG_WARNING, "(%lx) CALLBACK bad %d poll: %s",
		      pthread_self (), p_res, strerror (p_err));
#endif
	      return -2;
#ifdef  EBUG
	    }
	  else
	    logmsg (LOG_WARNING, "(%lx) CALLBACK interrupted %d poll: %s",
		    pthread_self (), p_res, strerror (p_err));
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
      if (len == m->length && strncasecmp (m->name, str, m->length) == 0)
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
static unsigned long
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
      v = n | tolower (*c);
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
  return a->len != b->len || strncasecmp (a->name, b->name, a->len);
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
 * Find first occurrence of TOK in a comma-separated list of values SUBJ.
 * Use case-insensitive comparison unless CI is 0.  Ignore whitespace.
 *
 * Return pointer to the first occurrence of TOK, if found or NULL
 * otherwise.
 * Unless NEXTP is NULL, initialize it with the pointer to the next item in
 * SUBJ.
 */
static char *
cs_locate_token (char const *subj, char const *tok, int ci, char **nextp)
{
  size_t toklen = strlen (tok);
  char const *next = NULL;

  while (*subj)
    {
      size_t i, len;

      while (*subj && isspace (*subj))
	subj++;
      if (!*subj)
	return NULL;
      len = strcspn (subj, ",");
      for (i = len; i > 0; i--)
	if (!isspace (subj[i-1]))
	  break;
      next = subj + len;
      if (*next)
	next++;
      if (i == toklen && (ci ? strncasecmp : strncmp) (subj, tok, i) == 0)
	break;
      subj = next;
    }

  if (nextp)
    *nextp = (char*) next;

  return *subj != 0 ? (char*) subj : NULL;
}

static int
qualify_header (struct http_header *hdr)
{
  regmatch_t matches[4];
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

  if (regexec (&HEADER, hdr->header, 4, matches, 0) == 0)
    {
      hdr->name_start = matches[1].rm_so;
      hdr->name_end = matches[1].rm_eo;
      hdr->val_start = matches[2].rm_so;
      hdr->val_end = matches[2].rm_eo;
      for (i = 0; hd_types[i].len > 0; i++)
	if ((matches[1].rm_eo - matches[1].rm_so) == hd_types[i].len
	    && strncasecmp (hdr->header + matches[1].rm_so, hd_types[i].header,
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
http_header_alloc (char *text)
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
	  strncasecmp (http_header_name_ptr (hdr), name, len) == 0)
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
	  strncasecmp (http_header_name_ptr (hdr), name, len) == 0)
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
http_header_list_append (HTTP_HEADER_LIST *head, char *text, int replace)
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
	    char *val = http_header_get_value (hdr);
	    if (*val)
	      {
		struct stringbuf sb;

		stringbuf_init_log (&sb);
		stringbuf_add (&sb, hdr->header,
			       hdr->val_end - hdr->name_start);
		stringbuf_add_char (&sb, ',');
		val = text + hdr->name_end + 1;
		if (!isspace (*val))
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
http_header_list_append_list (HTTP_HEADER_LIST *head, HTTP_HEADER_LIST *add,
			      int replace)
{
  struct http_header *hdr;
  DLIST_FOREACH (hdr, add, link)
    {
      if (http_header_list_append (head, hdr->header, replace))
	return -1;
    }
  return 0;
}

static void
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
      if (regexec (&m->pat, hdr->header, 0, NULL, 0) == 0)
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

static int
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

static int
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

static int
http_request_set_query (struct http_request *req, char const *rawquery)
{
  char *p;

  if (http_request_split (req))
      return -1;
  if ((p = strdup (rawquery)) == NULL)
    {
      lognomem ();
      return -1;
    }
  free (req->query);
  req->query = p;
  http_request_free_query (req);
  return http_request_rebuild_url (req);
}

static int
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
      if (raw_value == 0)
	{
	  DLIST_REMOVE (&req->query_head, qp, link);
	  query_param_free (qp);
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
  http_request_init (req);
}

typedef struct
{
  struct http_header *hdr;
  struct stringbuf sb;
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
  return alen != blen || strncasecmp (http_header_name_ptr (a->hdr),
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
http_request_read (BIO *in, const LISTENER *lstn, struct http_request *req)
{
  char buf[MAXBUF];
  int res;
  COMPOSE_HEADER_HASH *chash = NULL;

  if (combinable_headers)
    {
      chash = COMPOSE_HEADER_HASH_NEW ();
      if (!chash)
	{
	  lognomem ();
	  return -1;
	}
    }

  http_request_init (req);

  /*
   * HTTP/1.1 allows leading CRLF
   */
  while ((res = get_line (in, buf, sizeof (buf))) == 0)
    if (buf[0])
      break;

  if (res < 0)
    {
      /*
       * this is expected to occur only on client reads
       */
      return -1;
    }

  if ((req->request = strdup (buf)) == NULL)
    {
      lognomem ();
      compose_header_hash_free (chash);
      return -1;
    }

  for (;;)
    {
      struct http_header *hdr;

      if (get_line (in, buf, sizeof (buf)))
	{
	  http_request_free (req);
	  compose_header_hash_free (chash);
	  /*
	   * this is not necessarily an error, EOF/timeout are possible
	   */
	  return -1;
	}

      if (!buf[0])
	break;

      if ((hdr = http_header_alloc (buf)) == NULL)
	{
	  http_request_free (req);
	  compose_header_hash_free (chash);
	  return -1;
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
		  stringbuf_add (&comp->sb, ", ", 2);
		  stringbuf_add (&comp->sb, hdr->header + hdr->val_start,
				 hdr->val_end - hdr->val_start);
		  http_header_free (hdr);
		  if (stringbuf_err (&comp->sb))
		    {
		      http_request_free (req);
		      compose_header_hash_free (chash);
		      return -1;
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
		      return -1;
		    }
		  comp->hdr = hdr;
		  stringbuf_init_log (&comp->sb);
		  stringbuf_add (&comp->sb, http_header_name_ptr (hdr),
				 http_header_name_len (hdr));
		  stringbuf_add (&comp->sb, ": ", 2);
		  stringbuf_add (&comp->sb, hdr->header + hdr->val_start,
				 hdr->val_end - hdr->val_start);
		  if (stringbuf_err (&comp->sb))
		    {
		      http_request_free (req);
		      compose_header_hash_free (chash);
		      return -1;
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
	  return -1;
	}
    }

  return 0;
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

  if (strncasecmp (hdrval, "Basic", 5))
    return 1;

  hdrval += 5;
  while (*hdrval && isspace (*hdrval))
    hdrval++;

  len = strlen (hdrval);
  if (*hdrval == '"')
    {
      hdrval++;
      len--;

      while (len > 0 && isspace (hdrval[len-1]))
	len--;

      if (len == 0 || hdrval[len] != '"')
	return 1;
      len--;
    }

  if ((bb = BIO_new (BIO_s_mem ())) == NULL)
    {
      logmsg (LOG_WARNING, "(%"PRItid") Can't alloc BIO_s_mem", POUND_TID ());
      return -1;
    }

  if ((b64 = BIO_new (BIO_f_base64 ())) == NULL)
    {
      logmsg (LOG_WARNING, "(%"PRItid") Can't alloc BIO_f_base64",
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
      logmsg (LOG_WARNING, "(%"PRItid") Can't read BIO_f_base64",
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

int
http_request_get_basic_auth (struct http_request *req,
			     char **u_name, char **u_pass)
{
  struct http_header *hdr;
  char const *val;

  if ((hdr = http_header_list_locate (&req->headers,
				      HEADER_AUTHORIZATION)) != NULL &&
      (val = http_header_get_value (hdr)) != NULL)
    return get_basic_auth (val, u_name, u_pass);
  return -1;
}

static int
parse_http_request (struct http_request *req, int group)
{
  char *str;
  size_t len;
  struct method_def *md;
  char const *url;
  int http_ver;

  str = req->request;
  len = strcspn (str, " ");
  if (len == 0 || str[len-1] == 0)
    return -1;

  if ((md = find_method (str, len)) == NULL)
    return -1;

  if (md->group > group)
    return -1;

  str += len;
  str += strspn (str, " ");

  if (*str == 0)
    return -1;

  url = str;
  len = strcspn (url, " ");

  str += len;
  str += strspn (str, " ");
  if (!(strncmp (str, "HTTP/1.", 7) == 0 &&
	((http_ver = str[7]) == '0' || http_ver == '1') &&
	str[8] == 0))
    return -1;

  req->method = md->meth;
  if ((req->url = malloc (len + 1)) == NULL)
    {
      lognomem ();
      return -1;
    }
  memcpy (req->url, url, len);
  req->url[len] = 0;

  req->version = http_ver - '0';
  req->split = 1;

  return 0;
}

static int
match_headers (HTTP_HEADER_LIST *headers, regex_t const *re,
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

int
match_cond (SERVICE_COND *cond, POUND_HTTP *phttp,
	    struct http_request *req)
{
  int res = 1;
  SERVICE_COND *subcond;
  char const *str;

  switch (cond->type)
    {
    case COND_ACL:
      res = acl_match (cond->acl, phttp->from_host.ai_addr) == 0;
      break;

    case COND_URL:
      if (http_request_get_url (req, &str) == -1)
	res = -1;
      else
	res = submatch_exec (&cond->re, str, submatch_queue_push (&phttp->smq));
      break;

    case COND_PATH:
      if (http_request_get_path (req, &str) == -1)
	res = -1;
      else
	res = submatch_exec (&cond->re, str, submatch_queue_push (&phttp->smq));
      break;

    case COND_QUERY:
      if (http_request_get_query (req, &str) == -1)
	res = -1;
      else
	res = submatch_exec (&cond->re, str, submatch_queue_push (&phttp->smq));
      break;

    case COND_QUERY_PARAM:
      switch (http_request_get_query_param_value (req, cond->sm.string->value,
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
	  else
	    res = submatch_exec (&cond->sm.re, str,
				 submatch_queue_push (&phttp->smq));
	}
      break;

    case COND_HDR:
      res = match_headers (&req->headers, &cond->re,
			   submatch_queue_push (&phttp->smq));
      break;

    case COND_HOST:
      res = match_headers (&req->headers, &cond->re,
			   submatch_queue_push (&phttp->smq));
      if (res)
	{
	  /*
	   * On match, adjust subgroup references and subject pointer
	   * to refer to the Host: header value.
	   */
	  struct submatch *sm = submatch_queue_get (&phttp->smq, 0);
	  int n, i;
	  char const *s = sm->subject;
	  regmatch_t *mv = sm->matchv;
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
      res = basic_auth (&cond->pwfile, req) == 0;
      break;

    case COND_STRING_MATCH:
      {
	char *subj;

	subj = expand_string (cond->sm.string->value, phttp, req,
			      "string_match");
	if (subj)
	  {
	    res = submatch_exec (&cond->sm.re, subj,
				 submatch_queue_push (&phttp->smq));
	    free (subj);
	  }
	else
	  res = -1;
      }
      break;

    case COND_BOOL:
      if (cond->bool.op == BOOL_NOT)
	{
	  subcond = SLIST_FIRST (&cond->bool.head);
	  res = ! match_cond (subcond, phttp, req);
	}
      else
	{
	  SLIST_FOREACH (subcond, &cond->bool.head, next)
	    {
	      res = match_cond (subcond, phttp, req);
	      if ((cond->bool.op == BOOL_AND) ? (res == 0) : (res == 1))
		break;
	    }
	}
      break;
    }

  return res;
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
      X509_NAME_print_ex (bio, X509_get_subject_name (phttp->x509), 8,
			  XN_FLAG_ONELINE & ~ASN1_STRFLGS_ESC_MSB);
      get_line (bio, buf, sizeof (buf));
      stringbuf_printf (&sb, "X-SSL-Subject: %s", buf);
      if ((str = stringbuf_finish (&sb)) == NULL
	  || http_header_list_append (&phttp->request.headers, str, H_REPLACE))
	{
	  res = -1;
	  goto end;
	}
      stringbuf_reset (&sb);

      X509_NAME_print_ex (bio, X509_get_issuer_name (phttp->x509), 8,
			  XN_FLAG_ONELINE & ~ASN1_STRFLGS_ESC_MSB);
      get_line (bio, buf, sizeof (buf));
      stringbuf_printf (&sb, "X-SSL-Issuer: %s", buf);
      if ((str = stringbuf_finish (&sb)) == NULL
	  || http_header_list_append (&phttp->request.headers, str, H_REPLACE))
	{
	  res = -1;
	  goto end;
	}
      stringbuf_reset (&sb);

      ASN1_TIME_print (bio, X509_get_notBefore (phttp->x509));
      get_line (bio, buf, sizeof (buf));
      stringbuf_printf (&sb, "X-SSL-notBefore: %s", buf);
      if ((str = stringbuf_finish (&sb)) == NULL
	  || http_header_list_append (&phttp->request.headers, str, H_REPLACE))
	{
	  res = -1;
	  goto end;
	}
      stringbuf_reset (&sb);

      ASN1_TIME_print (bio, X509_get_notAfter (phttp->x509));
      get_line (bio, buf, sizeof (buf));
      stringbuf_printf (&sb, "X-SSL-notAfter: %s", buf);
      if ((str = stringbuf_finish (&sb)) == NULL
	  || http_header_list_append (&phttp->request.headers, str, H_REPLACE))
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
      while (get_line (bio, buf, sizeof (buf)) == 0)
	{
	  stringbuf_add_string (&sb, buf);
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
			       POUND_HTTP *phttp);

static int
rewrite_op_apply (REWRITE_OP_HEAD *head, struct http_request *request,
		  POUND_HTTP *phttp)
{
  int res = 0;
  REWRITE_OP *op;
  char *s;

  static struct
  {
    char *name;
    int (*setter) (struct http_request *, char const *);
  } rwtab[] = {
    [REWRITE_URL_SET]    = { "url", http_request_set_url },
    [REWRITE_PATH_SET]   = { "path", http_request_set_path },
    [REWRITE_QUERY_SET]  = { "query", http_request_set_query },
  };

  SLIST_FOREACH (op, head, next)
    {
      switch (op->type)
	{
	case REWRITE_REWRITE_RULE:
	  res = rewrite_rule_check (op->v.rule, request, phttp);
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
	  if ((s = expand_string (op->v.qp.value, phttp, request,
				  "query parameter")) != NULL)
	    {
	      res = http_request_set_query_param (request, op->v.qp.name, s);
	      free (s);
	    }
	  else
	    res = -1;
	  break;

	default:
	  if ((s = expand_string (op->v.str, phttp, request,
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
		    POUND_HTTP *phttp)
{
  int res = 0;

  do
    {
      if (match_cond (&rule->cond, phttp, request))
	{
	  res = rewrite_op_apply (&rule->ophead, request, phttp);
	  break;
	}
    }
  while ((rule = rule->iffalse) != NULL);
  return res;
}

static int
rewrite_apply (REWRITE_RULE_HEAD *rewrite_rules,
	       struct http_request *request, POUND_HTTP *phttp)
{
  int res = 0;
  REWRITE_RULE *rule;

  SLIST_FOREACH (rule, rewrite_rules, next)
    {
      if ((res = rewrite_rule_check (rule, request, phttp)) != 0)
	break;
    }
  return res;
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

/*
 * get the response
 */
static int
backend_response (POUND_HTTP *phttp)
{
  int skip = 0;
  int be_11 = 0;  /* Whether backend connection is using HTTP/1.1. */
  CONTENT_LENGTH content_length;
  char caddr[MAX_ADDR_BUFSIZE], caddr2[MAX_ADDR_BUFSIZE];
  char duration_buf[LOG_TIME_SIZE];
  char buf[MAXBUF];
  struct http_header *hdr;
  char *val;
  int res;

  phttp->res_bytes = 0;
  do
    {
      int chunked; /* True if request contains Transfer-Encoding: chunked */

      /* Free previous response, if any */
      http_request_free (&phttp->response);

      if (http_request_read (phttp->be, phttp->lstn, &phttp->response))
	{
	  logmsg (LOG_NOTICE,
		  "(%"PRItid") e500 for %s response error read from %s/%s: %s (%s secs)",
		  POUND_TID (),
		  addr2str (caddr, sizeof (caddr), &phttp->from_host, 1),
		  str_be (caddr2, sizeof (caddr2), phttp->backend),
		  phttp->request.request, strerror (errno),
		  log_duration (duration_buf, sizeof (duration_buf),
				&phttp->start_req));
	  return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	}

      if (rewrite_apply (&phttp->lstn->rewrite[REWRITE_RESPONSE],
			 &phttp->response,
			 phttp) ||
	  rewrite_apply (&phttp->svc->rewrite[REWRITE_RESPONSE],
			 &phttp->response,
			 phttp))
	return HTTP_STATUS_INTERNAL_SERVER_ERROR;

      be_11 = (phttp->response.request[7] == '1');
      phttp->response_code = strtol (phttp->response.request+9, NULL, 10);

      switch (phttp->response_code)
	{
	case 100:
	  /*
	   * responses with code 100 are never passed back to the client
	   */
	  skip = 1;
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
	  if (phttp->response_code / 100 == 1)
	    phttp->no_cont = 1;
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
	      if (!strcasecmp ("close", val))
		phttp->conn_closed = 1;
	      /*
	       * Connection: upgrade
	       */
	      else if (!regexec (&CONN_UPGRD, val, 0, NULL, 0))
		phttp->ws_state |= WSS_RESP_HEADER_CONNECTION_UPGRADE;
	      break;

	    case HEADER_UPGRADE:
	      if ((val = http_header_get_value (hdr)) == NULL)
		return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	      if (cs_locate_token (val, "websocket", 1, NULL))
		phttp->ws_state |= WSS_RESP_HEADER_UPGRADE_WEBSOCKET;
	      break;

	    case HEADER_TRANSFER_ENCODING:
	      if ((val = http_header_get_value (hdr)) == NULL)
		return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	      if (cs_locate_token (val, "chunked", 1, NULL))
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
		  logmsg (LOG_WARNING, "(%"PRItid") invalid content length: %s",
			  POUND_TID (), val);
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
			  logmsg (LOG_WARNING,
				  "(%"PRItid") rewrite Location - out of memory: %s",
				  POUND_TID (), strerror (errno));
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
			  logmsg (LOG_WARNING,
				  "(%"PRItid") rewrite Content-location - "
				  "out of memory: %s",
				  POUND_TID (), strerror (errno));
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
      if (!skip)
	{
	  if (http_request_send (phttp->cl, &phttp->response))
	    {
	      if (errno)
		{
		  logmsg (LOG_NOTICE, "(%"PRItid") error write to %s: %s",
			  POUND_TID (),
			  addr2str (caddr, sizeof (caddr), &phttp->from_host, 1),
			  strerror (errno));
		}
	      return -1;
	    }
	  /* Final CRLF */
	  BIO_puts (phttp->cl, "\r\n");
	}

      if (BIO_flush (phttp->cl) != 1)
	{
	  if (errno)
	    {
	      logmsg (LOG_NOTICE, "(%"PRItid") error flush headers to %s: %s",
		      POUND_TID (),
		      addr2str (caddr, sizeof (caddr), &phttp->from_host, 1),
		      strerror (errno));
	    }
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
	      if (copy_chunks (phttp->be, phttp->cl, &phttp->res_bytes,
			       skip, 0) != HTTP_STATUS_OK)
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
	      if (copy_bin (phttp->be, phttp->cl, content_length,
			    &phttp->res_bytes, skip))
		{
		  if (errno)
		    logmsg (LOG_NOTICE,
			    "(%"PRItid") error copy server cont: %s",
			    POUND_TID (), strerror (errno));
		  return -1;
		}
	    }
	  else if (!skip)
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
			  logmsg (LOG_NOTICE,
				  "(%"PRItid") error read response pending: %s",
				  POUND_TID (), strerror (errno));
			  return -1;
			}
		      if (BIO_write (phttp->cl, &one, 1) != 1)
			{
			  if (errno)
			    logmsg (LOG_NOTICE,
				    "(%"PRItid") error write response pending: %s",
				    POUND_TID (), strerror (errno));
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
		      logmsg (LOG_WARNING,
			      "(%"PRItid") error get unbuffered: %s",
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
			  if (errno)
			    logmsg (LOG_NOTICE,
				    "(%"PRItid") error copy response body: %s",
				    POUND_TID (), strerror (errno));
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
		{
		  logmsg (LOG_NOTICE, "(%"PRItid") error final flush to %s: %s",
			  POUND_TID (),
			  addr2str (caddr, sizeof (caddr), &phttp->from_host, 1),
			  strerror (errno));
		}
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
		      logmsg (LOG_NOTICE,
			      "(%"PRItid") error read ws request pending: %s",
			      POUND_TID (), strerror (errno));
		      return -1;
		    }
		  if (BIO_write (phttp->be, &one, 1) != 1)
		    {
		      if (errno)
			logmsg (LOG_NOTICE,
				"(%"PRItid") error write ws request pending: %s",
				POUND_TID (), strerror (errno));
		      return -1;
		    }
		}
	      BIO_flush (phttp->be);

	      while (BIO_pending (phttp->be))
		{
		  if (BIO_read (phttp->be, &one, 1) != 1)
		    {
		      logmsg (LOG_NOTICE,
			      "(%"PRItid") error read ws response pending: %s",
			      POUND_TID (), strerror (errno));
		      return -1;
		    }
		  if (BIO_write (phttp->cl, &one, 1) != 1)
		    {
		      if (errno)
			logmsg (LOG_NOTICE,
				"(%"PRItid") error write ws response pending: %s",
				POUND_TID (), strerror (errno));
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
		  logmsg (LOG_WARNING, "(%"PRItid") error get unbuffered: %s",
			  POUND_TID (), strerror (errno));
		  return -1;
		}
	      if ((be_unbuf = BIO_find_type (phttp->be,
					     backend_is_https (phttp->backend)
					       ? BIO_TYPE_SSL
					       : BIO_TYPE_SOCKET)) == NULL)
		{
		  logmsg (LOG_WARNING, "(%"PRItid") error get unbuffered: %s",
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
			logmsg (LOG_NOTICE,
				"(%"PRItid") error copy ws request body: %s",
				POUND_TID (), strerror (errno));
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
			logmsg (LOG_NOTICE,
				"(%"PRItid") error copy ws response body: %s",
				POUND_TID (), strerror (errno));
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
  while (skip);

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
  char caddr[MAX_ADDR_BUFSIZE], caddr2[MAX_ADDR_BUFSIZE];
  char duration_buf[LOG_TIME_SIZE];

  /*
   * this is the earliest we can check for Destination - we
   * had no back-end before
   */
  if (phttp->lstn->rewr_dest &&
      (hdr = http_header_list_locate (&phttp->request.headers,
				      HEADER_DESTINATION)) != NULL)
    {
      regmatch_t matches[4];

      if ((val = http_header_get_value (hdr)) == NULL)
	return HTTP_STATUS_INTERNAL_SERVER_ERROR;

      if (regexec (&LOCATION, val, 4, matches, 0))
	{
	  logmsg (LOG_NOTICE, "(%"PRItid") Can't parse Destination %s",
		  POUND_TID (), val);
	}
      else
	{
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
	      logmsg (LOG_WARNING,
		      "(%"PRItid") rewrite Destination - out of memory: %s",
		      POUND_TID (), strerror (errno));
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

  if (rewrite_apply (&phttp->lstn->rewrite[REWRITE_REQUEST], &phttp->request,
		     phttp)
      || rewrite_apply (&phttp->svc->rewrite[REWRITE_REQUEST], &phttp->request,
			phttp))
    return HTTP_STATUS_INTERNAL_SERVER_ERROR;

  /*
   * Send the request and its headers
   */
  if (http_request_send (phttp->be, &phttp->request))
    {
      if (errno)
	{
	  logmsg (LOG_WARNING,
		  "(%"PRItid") e500 error write to %s/%s: %s (%s sec)",
		  POUND_TID (),
		  str_be (caddr, sizeof (caddr), phttp->backend),
		  phttp->request.request, strerror (errno),
		  log_duration (duration_buf, sizeof (duration_buf), &phttp->start_req));
	}
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
      int rc = copy_chunks (phttp->cl, phttp->be, NULL,
			    phttp->backend->be_type != BE_BACKEND,
			    phttp->lstn->max_req);
      if (rc != HTTP_STATUS_OK)
	{
	  logmsg (LOG_NOTICE,
		  "(%"PRItid") e%d for %s copy_chunks to %s/%s (%s sec)",
		  POUND_TID (),
		  pound_to_http_status(rc),
		  addr2str (caddr, sizeof (caddr), &phttp->from_host, 1),
		  str_be (caddr2, sizeof (caddr2), phttp->backend),
		  phttp->request.request,
		  log_duration (duration_buf, sizeof (duration_buf),
				&phttp->start_req));
	  return rc;
	}
    }
  else if (content_length > 0)
    {
      /*
       * had Content-length, so do raw reads/writes for the length
       */
      if (copy_bin (phttp->cl, phttp->be, content_length, NULL,
		    phttp->backend->be_type != BE_BACKEND))
	{
	  logmsg (LOG_NOTICE,
		  "(%"PRItid") e500 for %s error copy client cont to %s/%s: %s (%s sec)",
		  POUND_TID (),
		  addr2str (caddr, sizeof (caddr), &phttp->from_host, 1),
		  str_be (caddr2, sizeof (caddr2), phttp->backend),
		  phttp->request.request, strerror (errno),
		  log_duration (duration_buf, sizeof (duration_buf), &phttp->start_req));
	  return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	}
    }

  /*
   * flush to the back-end
   */
  if (BIO_flush (phttp->be) != 1)
    {
      logmsg (LOG_NOTICE,
	      "(%"PRItid") e500 for %s error flush to %s/%s: %s (%s sec)",
	      POUND_TID (),
	      addr2str (caddr, sizeof (caddr), &phttp->from_host, 1),
	      str_be (caddr2, sizeof (caddr2), phttp->backend),
	      phttp->request.request, strerror (errno),
	      log_duration (duration_buf, sizeof (duration_buf),
			    &phttp->start_req));
      return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }
  return 0;
}

static int
open_backend (POUND_HTTP *phttp, BACKEND *backend, int sock)
{
  char caddr[MAX_ADDR_BUFSIZE];
  BIO *bb;

  /* Create new BIO */
  if ((phttp->be = BIO_new_socket (sock, 1)) == NULL)
    {
      logmsg (LOG_WARNING, "(%"PRItid") e503 BIO_new_socket server failed",
	      POUND_TID ());
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
	  logmsg (LOG_WARNING, "(%"PRItid") be SSL_new: failed", POUND_TID ());
	  return HTTP_STATUS_SERVICE_UNAVAILABLE;
	}
      if (backend->v.reg.servername)
	SSL_set_tlsext_host_name (be_ssl, backend->v.reg.servername);
      SSL_set_bio (be_ssl, phttp->be, phttp->be);
      if ((bb = BIO_new (BIO_f_ssl ())) == NULL)
	{
	  logmsg (LOG_WARNING, "(%"PRItid") BIO_new(Bio_f_ssl()) failed",
		  POUND_TID ());
	  return HTTP_STATUS_SERVICE_UNAVAILABLE;
	}

      BIO_set_ssl (bb, be_ssl, BIO_CLOSE);
      BIO_set_ssl_mode (bb, 1);
      phttp->be = bb;

      if (BIO_do_handshake (phttp->be) <= 0)
	{
	  logmsg (LOG_NOTICE, "BIO_do_handshake with %s failed: %s",
		  str_be (caddr, sizeof (caddr), backend),
		  ERR_error_string (ERR_get_error (), NULL));
	  return HTTP_STATUS_SERVICE_UNAVAILABLE;
	}
    }

  if ((bb = BIO_new (BIO_f_buffer ())) == NULL)
    {
      logmsg (LOG_WARNING, "(%"PRItid") e503 BIO_new(buffer) server failed",
	      POUND_TID ());
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
  char const *v_host;
  char caddr[MAX_ADDR_BUFSIZE];

  if ((backend = get_backend (phttp)) != NULL)
    {
      if (phttp->be != NULL)
	{
	  if (backend == phttp->backend)
	    /* Same backend as before: nothing to do */
	    return 0;
	  else
	    close_backend (phttp);
	}

      do
	{
	  if (backend->be_type == BE_BACKEND)
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
		  logmsg (LOG_WARNING,
			  "(%"PRItid") e503 backend: unknown family %d",
			  POUND_TID (), backend->v.reg.addr.ai_family);
		  return HTTP_STATUS_SERVICE_UNAVAILABLE;
		}

	      if ((sock = socket (sock_proto, SOCK_STREAM, 0)) < 0)
		{
		  logmsg (LOG_WARNING,
			  "(%"PRItid") e503 backend %s socket create: %s",
			  POUND_TID (),
			  str_be (caddr, sizeof (caddr), backend),
			  strerror (errno));
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
		  phttp->backend = backend;
		  return 0;
		}

	      logmsg (LOG_WARNING, "(%"PRItid") backend %s connect: %s",
		      POUND_TID (),
		      str_be (caddr, sizeof (caddr), backend),
		      strerror (errno));
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
	    }
	  else
	    {
	      /* New backend selected. */
	      phttp->backend = backend;
	      return 0;
	    }
	}
      while ((backend = get_backend (phttp)) != NULL);
    }

  /*
   * No backend found or is available.
   */
  v_host = http_request_host (&phttp->request);
  logmsg (LOG_NOTICE, "(%"PRItid") e503 no back-end \"%s\" from %s %s",
	  POUND_TID (), phttp->request.request,
	  addr2str (caddr, sizeof (caddr), &phttp->from_host, 1),
	  (v_host && v_host[0]) ? v_host : "-");

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

/*
 * handle an HTTP request
 */
void
do_http (POUND_HTTP *phttp)
{
  int cl_11;  /* Whether client connection is using HTTP/1.1 */
  int res;  /* General-purpose result variable */
  int chunked; /* True if request contains Transfer-Encoding: chunked
		* FIXME: this belongs to struct http_request, perhaps.
		*/
  BIO *bb;
  char caddr[MAX_ADDR_BUFSIZE];
  CONTENT_LENGTH content_length;
  struct http_header *hdr, *hdrtemp;
  char *val;
  struct timespec be_start;

  if (phttp->lstn->allow_client_reneg)
    phttp->reneg_state = RENEG_ALLOW;
  else
    phttp->reneg_state = RENEG_INIT;

  socket_setup (phttp->sock);

  if ((phttp->cl = BIO_new_socket (phttp->sock, 1)) == NULL)
    {
      logmsg (LOG_ERR, "(%"PRItid") BIO_new_socket failed", POUND_TID ());
      shutdown (phttp->sock, 2);
      close (phttp->sock);
      return;
    }
  set_callback (phttp->cl, phttp->lstn->to, &phttp->reneg_state);

  if (!SLIST_EMPTY (&phttp->lstn->ctx_head))
    {
      if ((phttp->ssl = SSL_new (SLIST_FIRST (&phttp->lstn->ctx_head)->ctx)) == NULL)
	{
	  logmsg (LOG_ERR, "(%"PRItid") SSL_new: failed", POUND_TID ());
	  return;
	}
      SSL_set_app_data (phttp->ssl, &phttp->reneg_state);
      SSL_set_bio (phttp->ssl, phttp->cl, phttp->cl);
      if ((bb = BIO_new (BIO_f_ssl ())) == NULL)
	{
	  logmsg (LOG_ERR, "(%"PRItid") BIO_new(Bio_f_ssl()) failed",
		  POUND_TID ());
	  return;
	}
      BIO_set_ssl (bb, phttp->ssl, BIO_CLOSE);
      BIO_set_ssl_mode (bb, 0);
      phttp->cl = bb;
      if (BIO_do_handshake (phttp->cl) <= 0)
	{
	  return;
	}
      else
	{
	  if ((phttp->x509 = SSL_get_peer_certificate (phttp->ssl)) != NULL
	      && phttp->lstn->clnt_check < 3
	      && SSL_get_verify_result (phttp->ssl) != X509_V_OK)
	    {
	      logmsg (LOG_NOTICE, "Bad certificate from %s",
		      addr2str (caddr, sizeof (caddr), &phttp->from_host, 1));
	      return;
	    }
	}
    }
  else
    {
      phttp->x509 = NULL;
    }
  phttp->backend = NULL;

  if ((bb = BIO_new (BIO_f_buffer ())) == NULL)
    {
      logmsg (LOG_ERR, "(%"PRItid") BIO_new(buffer) failed", POUND_TID ());
      return;
    }
  BIO_set_close (phttp->cl, BIO_CLOSE);
  BIO_set_buffer_size (phttp->cl, MAXBUF);
  phttp->cl = BIO_push (bb, phttp->cl);

  cl_11 = 0;
  for (;;)
    {
      http_request_free (&phttp->request);
      http_request_free (&phttp->response);

      phttp->ws_state = WSS_INIT;
      phttp->conn_closed = 0;
      if (http_request_read (phttp->cl, phttp->lstn, &phttp->request))
	{
	  if (!cl_11)
	    {
	      if (errno)
		{
		  logmsg (LOG_NOTICE, "(%"PRItid") error read from %s: %s",
			  POUND_TID (),
			  addr2str (caddr, sizeof (caddr), &phttp->from_host, 1),
			  strerror (errno));
		}
	    }
	  return;
	}

      clock_gettime (CLOCK_REALTIME, &phttp->start_req);

      /*
       * check for correct request
       */
      if (parse_http_request (&phttp->request, phttp->lstn->verb))
	{
	  logmsg (LOG_WARNING, "(%"PRItid") e501 bad request \"%s\" from %s",
		  POUND_TID (), phttp->request.request,
		  addr2str (caddr, sizeof (caddr), &phttp->from_host, 1));
	  http_err_reply (phttp, HTTP_STATUS_NOT_IMPLEMENTED);
	  return;
	}
      cl_11 = phttp->request.version;

      phttp->no_cont = phttp->request.method == METH_HEAD;
      if (phttp->request.method == METH_GET)
	phttp->ws_state |= WSS_REQ_GET;

      if (phttp->lstn->has_pat &&
	  regexec (&phttp->lstn->url_pat, phttp->request.url, 0, NULL, 0))
	{
	  logmsg (LOG_NOTICE, "(%"PRItid") e501 bad URL \"%s\" from %s",
		  POUND_TID (), phttp->request.url,
		  addr2str (caddr, sizeof (caddr), &phttp->from_host, 1));
	  http_err_reply (phttp, HTTP_STATUS_NOT_IMPLEMENTED);
	  return;
	}

      /*
       * check headers
       */
      chunked = 0;
      content_length = NO_CONTENT_LENGTH;
      DLIST_FOREACH_SAFE (hdr, hdrtemp, &phttp->request.headers, link)
	{
	  switch (hdr->code)
	    {
	    case HEADER_CONNECTION:
	      if ((val = http_header_get_value (hdr)) == NULL)
		goto err;
	      if (cs_locate_token (val, "close", 1, NULL))
		phttp->conn_closed = 1;
	      /*
	       * Connection: upgrade
	       */
	      else if (!regexec (&CONN_UPGRD, val, 0, NULL, 0))
		phttp->ws_state |= WSS_REQ_HEADER_CONNECTION_UPGRADE;
	      break;

	    case HEADER_UPGRADE:
	      if ((val = http_header_get_value (hdr)) == NULL)
		goto err;
	      if (cs_locate_token (val, "websocket", 1, NULL))
		phttp->ws_state |= WSS_REQ_HEADER_UPGRADE_WEBSOCKET;
	      break;

	    case HEADER_TRANSFER_ENCODING:
	      if ((val = http_header_get_value (hdr)) == NULL)
		goto err;
	      else if (chunked)
		{
		  logmsg (LOG_NOTICE,
			  "(%"PRItid") e400 multiple Transfer-encoding: chunked on \"%s\" from %s",
			  POUND_TID (), phttp->request.url,
			  addr2str (caddr, sizeof (caddr), &phttp->from_host, 1));
		  http_err_reply (phttp, HTTP_STATUS_BAD_REQUEST);
		  return;
		}
	      else
		{
		  char *next;
		  if (cs_locate_token (val, "chunked", 1, &next))
		    {
		      if (*next)
			{
			  /*
			   * When the "chunked" transfer-coding is used,
			   * it MUST be the last transfer-coding applied
			   * to the message-body.
			   */
			  logmsg (LOG_NOTICE,
				  "(%"PRItid") e400 multiple Transfer-encoding on \"%s\" from %s",
				  POUND_TID (), phttp->request.url,
				  addr2str (caddr, sizeof (caddr), &phttp->from_host, 1));
			  http_err_reply (phttp, HTTP_STATUS_BAD_REQUEST);
			  return;
			}
		      chunked = 1;
		    }
		}
	      break;

	    case HEADER_CONTENT_LENGTH:
	      if ((val = http_header_get_value (hdr)) == NULL)
		goto err;
	      if (content_length != NO_CONTENT_LENGTH || strchr (val, ','))
		{
		  logmsg (LOG_NOTICE,
			  "(%"PRItid") e400 multiple Content-length \"%s\" from %s",
			  POUND_TID (), phttp->request.url,
			  addr2str (caddr, sizeof (caddr), &phttp->from_host, 1));
		  http_err_reply (phttp, HTTP_STATUS_BAD_REQUEST);
		  return;
		}
	      else if ((content_length = get_content_length (val, CL_HEADER)) == NO_CONTENT_LENGTH)
		{
		  logmsg (LOG_NOTICE,
			  "(%"PRItid") e400 Content-length bad value \"%s\" from %s",
			  POUND_TID (), phttp->request.url,
			  addr2str (caddr, sizeof (caddr), &phttp->from_host, 1));
		  http_err_reply (phttp, HTTP_STATUS_BAD_REQUEST);
		  return;
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
		goto err;
	      if (!strcasecmp ("100-continue", val))
		{
		  http_header_list_remove (&phttp->request.headers, hdr);
		}
	      break;

	    case HEADER_ILLEGAL:
	      /*
	       * FIXME: This should not happen.  See the handling of
	       * HEADER_ILLEGAL in http_header_list_append.
	       */
	      logmsg (LOG_NOTICE, "(%"PRItid") bad header from %s (%s)",
		      POUND_TID (),
		      addr2str (caddr, sizeof (caddr), &phttp->from_host, 1),
		      hdr->header);

	      http_header_list_remove (&phttp->request.headers, hdr);
	      break;
	    }
	}

      /*
       * check for possible request smuggling attempt
       */
      if (chunked != 0 && content_length != NO_CONTENT_LENGTH)
	{
	  logmsg (LOG_NOTICE,
		  "(%"PRItid") e501 Transfer-encoding and Content-length \"%s\" from %s",
		  POUND_TID (), phttp->request.url,
		  addr2str (caddr, sizeof (caddr), &phttp->from_host, 1));
	  http_err_reply (phttp, HTTP_STATUS_BAD_REQUEST);
	  return;
	}

      /*
       * possibly limited request size
       */
      if (phttp->lstn->max_req > 0 && content_length > 0
	  && content_length > phttp->lstn->max_req)
	{
	  logmsg (LOG_NOTICE,
		  "(%"PRItid") e413 request too large (%"PRICLEN") from %s",
		  POUND_TID (), content_length,
		  addr2str (caddr, sizeof (caddr), &phttp->from_host, 1));
	  http_err_reply (phttp, HTTP_STATUS_PAYLOAD_TOO_LARGE);
	  return;
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
	  char const *v_host = http_request_host (&phttp->request);
	  logmsg (LOG_NOTICE, "(%"PRItid") e503 no service \"%s\" from %s %s",
		  POUND_TID (), phttp->request.request,
		  addr2str (caddr, sizeof (caddr), &phttp->from_host, 1),
		  (v_host && v_host[0]) ? v_host : "-");
	  http_err_reply (phttp, HTTP_STATUS_SERVICE_UNAVAILABLE);
	  return;
	}

      if ((res = select_backend (phttp)) != 0)
	{
	  http_err_reply (phttp, res);
	  return;
	}

      /*
       * if we have anything but a BACK_END we close the channel
       */
      if (phttp->be != NULL && phttp->backend->be_type != BE_BACKEND)
	close_backend (phttp);

      if (force_http_10 (phttp))
	phttp->conn_closed = 1;

      phttp->res_bytes = 0;
      http_request_free (&phttp->response);

      save_forwarded_header (phttp);

      clock_gettime (CLOCK_REALTIME, &be_start);
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

	case BE_BACKEND:
	  /* Send the request. */
	  res = send_to_backend (phttp, cl_11 && chunked, content_length);
	  if (res == 0)
	    /* Process the response. */
	    res = backend_response (phttp);
	  break;

	case BE_BACKEND_REF:
	  /* shouldn't happen */
	  abort ();
	}

      clock_gettime (CLOCK_REALTIME, &phttp->end_req);
      if (enable_backend_stats)
	backend_update_stats (phttp->backend, &be_start, &phttp->end_req);

      if (res == -1)
	break;
      else
	{
	  if (res != HTTP_STATUS_OK)
	    http_err_reply (phttp, res);
	  if (phttp->response_code == 0)
	    phttp->response_code = http_status[res].code;
	  http_log (phttp);
	}

      /*
       * Stop processing if:
       *  - client is not HTTP/1.1
       *      or
       *  - we had a "Connection: closed" header
       */
      if (!cl_11 || phttp->conn_closed)
	break;
    }

  return;

 err:
  http_err_reply (phttp, HTTP_STATUS_INTERNAL_SERVER_ERROR);
  return;
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
