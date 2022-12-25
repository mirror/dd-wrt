/*
 * Pound - the reverse-proxy load-balancer
 * Copyright (C) 2002-2010 Apsis GmbH
 *
 * This file is part of Pound.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact information:
 * Apsis GmbH
 * P.O.Box
 * 8707 Uetikon am See
 * Switzerland
 * EMail: roseg@apsis.ch
 */

#include "pound.h"
#include "extern.h"

/*
 * HTTP error replies
 */
typedef struct
{
  int code;
  char const *text;
} HTTP_STATUS;

static HTTP_STATUS http_status[] = {
  [HTTP_STATUS_OK] = { 200, "OK" },
  [HTTP_STATUS_BAD_REQUEST] = { 400, "Bad Request" },
  [HTTP_STATUS_NOT_FOUND] = { 404, "Not Found" },
  [HTTP_STATUS_PAYLOAD_TOO_LARGE] = { 413, "Payload Too Large" },
  [HTTP_STATUS_URI_TOO_LONG] = { 414, "URI Too Long" },
  [HTTP_STATUS_INTERNAL_SERVER_ERROR] = { 500, "Internal Server Error" },
  [HTTP_STATUS_NOT_IMPLEMENTED] = { 501, "Not Implemented" },
  [HTTP_STATUS_SERVICE_UNAVAILABLE] = { 503, "Service Unavailable" },
};

static char *err_response =
	"HTTP/1.0 %d %s\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: %d\r\n"
	"Expires: now\r\n"
	"Pragma: no-cache\r\n"
	"Cache-control: no-cache,no-store\r\n"
	"\r\n"
	"%s";

/*
 * Reply with an error
 */
static void
err_reply (BIO *c, int err, char const *txt)
{
  if (!(err >= 0 && err < HTTP_STATUS_MAX))
    {
      err = HTTP_STATUS_INTERNAL_SERVER_ERROR;
      txt = "Bad error code returned";
    }
  if (!txt)
    txt = http_status[err].text;
  BIO_printf (c, err_response, http_status[err].code, http_status[err].text,
	      strlen (txt), txt);
  BIO_flush (c);
  return;
}

static void
listener_err_reply (BIO *c, int err, LISTENER const *lstn)
{
  err_reply (c, err, lstn->http_err[err]);
}

static char *
expand_url (char const *url, char const *orig_url, struct submatch *sm, int redir_req)
{
  struct stringbuf sb;
  char *p;

  stringbuf_init (&sb);
  while (*url)
    {
      size_t len = strcspn (url, "$");
      stringbuf_add (&sb, url, len);
      url += len;
      if (*url == 0)
	break;
      else if (url[1] == '$' || url[1] == 0)
	{
	  stringbuf_add_char (&sb, url[0]);
	  url += 2;
	}
      else if (isdigit (url[1]))
	{
	  long n;
	  errno = 0;
	  n = strtoul (url + 1, &p, 10);
	  if (errno)
	    {
	      stringbuf_add_char (&sb, url[0]);
	      url++;
	    }
	  else
	    {
	      if (n < sm->matchn)
		{
		  stringbuf_add (&sb, orig_url + sm->matchv[n].rm_so,
				 sm->matchv[n].rm_eo - sm->matchv[n].rm_so);
		}
	      else
		{
		  stringbuf_add (&sb, url, p - url);
		}
	      redir_req = 1;
	      url = p;
	    }
	}
    }

  /* For compatibility with previous versions */
  if (!redir_req)
    stringbuf_add_string (&sb, orig_url);

  return stringbuf_finish (&sb);
}

/*
 * Reply with a redirect
 */
static int
redirect_reply (BIO *c, const char *url, BACKEND *be, struct submatch *sm)
{
  int code = be->redir_code;
  char const *code_msg, *cont;
  char *xurl;
  struct stringbuf cont_buf, url_buf;
  int i;

  switch (code)
    {
    case 301:
      code_msg = "Moved Permanently";
      break;

    case 307:
      code_msg = "Temporary Redirect";
      break;

    default:
      code_msg = "Found";
      break;
    }

  xurl = expand_url (be->url, url, sm, be->redir_req);

  /*
   * Make sure to return a safe version of the URL (otherwise CSRF
   * becomes a possibility)
   */
  stringbuf_init (&url_buf);
  for (i = 0; xurl[i]; i++)
    {
      if (isalnum (xurl[i]) || xurl[i] == '_' || xurl[i] == '.'
	  || xurl[i] == ':' || xurl[i] == '/' || xurl[i] == '?' || xurl[i] == '&'
	  || xurl[i] == ';' || xurl[i] == '-' || xurl[i] == '=')
	stringbuf_add_char (&url_buf, xurl[i]);
      else
	stringbuf_printf (&url_buf, "%%%02x", xurl[i]);
    }
  url = stringbuf_finish (&url_buf);
  free (xurl);

  stringbuf_init (&cont_buf);
  stringbuf_printf (&cont_buf,
		    "<html><head><title>Redirect</title></head>"
		    "<body><h1>Redirect</h1>"
		    "<p>You should go to <a href=\"%s\">%s</a></p>"
		    "</body></html>",
		    url, url);
  cont = stringbuf_finish (&cont_buf);

  BIO_printf (c,
	      "HTTP/1.0 %d %s\r\n"
	      "Location: %s\r\n"
	      "Content-Type: text/html\r\n"
	      "Content-Length: %"PRILONG"\r\n\r\n"
	      "%s",
	      code, code_msg, url, (LONG)strlen (cont), cont);

  BIO_flush (c);

  stringbuf_free (&cont_buf);
  stringbuf_free (&url_buf);

  return HTTP_STATUS_OK;
}

/*
 * Read and write some binary data
 */
static int
copy_bin (BIO *cl, BIO *be, LONG cont, LONG *res_bytes, int no_write)
{
  char buf[MAXBUF];
  int res;

  while (cont > L0)
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
acme_reply (BIO *c, const char *url, BACKEND *be, struct submatch *sm)
{
  int fd;
  struct stat st;
  BIO *bin;
  char *file_name;
  int rc = HTTP_STATUS_OK;

  file_name = expand_url (be->url, url, sm, 1);

  if ((fd = open (file_name, O_RDONLY)) == -1)
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

      BIO_printf (c,
		  "HTTP/1.0 %d %s\r\n"
		  "Content-Type: text/plain\r\n"
		  "Content-Length: %"PRILONG"\r\n\r\n",
		  200, "OK", (LONG) st.st_size);

      if (copy_bin (bin, c, st.st_size, NULL, 0))
	{
	  if (errno)
	    logmsg (LOG_NOTICE, "(%"PRItid") error copying file %s: %s",
		    POUND_TID (), file_name, strerror (errno));
	}

      BIO_free (bin);
      BIO_flush (c);
    }
  free (file_name);
  return rc;
}

/*
 * Get a "line" from a BIO, strip the trailing newline, skip the input
 * stream if buffer too small
 * The result buffer is NULL terminated
 * Return 0 on success
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
 * Copy chunked
 */
static int
copy_chunks (BIO *cl, BIO *be, LONG * res_bytes, int no_write, LONG max_size)
{
  char buf[MAXBUF];
  LONG cont, tot_size;
  regmatch_t matches[2];
  int res;

  for (tot_size = 0L;;)
    {
      if ((res = get_line (cl, buf, sizeof (buf))) < 0)
	{
	  logmsg (LOG_NOTICE, "(%"PRItid" chunked read error: %s",
		  POUND_TID (),
		  strerror (errno));
	  return -1;
	}
      else if (res > 0)
	/*
	 * EOF
	 */
	return 0;
      if (!regexec (&CHUNK_HEAD, buf, 2, matches, 0))
	cont = STRTOL (buf, NULL, 16);
      else
	{
	  /*
	   * not chunk header
	   */
	  logmsg (LOG_NOTICE, "(%"PRItid") bad chunk header <%s>: %s",
		  POUND_TID (), buf, strerror (errno));
	  return -2;
	}
      if (!no_write)
	if (BIO_printf (be, "%s\r\n", buf) <= 0)
	  {
	    logmsg (LOG_NOTICE, "(%"PRItid") error write chunked: %s",
		    POUND_TID (), strerror (errno));
	    return -3;
	  }

      tot_size += cont;
      if (max_size > L0 && tot_size > max_size)
	{
	  logmsg (LOG_WARNING, "(%"PRItid") chunk content too large",
		  POUND_TID ());
	  return -4;
	}

      if (cont > L0)
	{
	  if (copy_bin (cl, be, cont, res_bytes, no_write))
	    {
	      if (errno)
		logmsg (LOG_NOTICE, "(%"PRItid") error copy chunk cont: %s",
			POUND_TID (), strerror (errno));
	      return -4;
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
	  return -5;
	}
      else if (res > 0)
	{
	  logmsg (LOG_NOTICE, "(%"PRItid") unexpected EOF after chunk",
		  POUND_TID ());
	  return -5;
	}
      if (buf[0])
	logmsg (LOG_NOTICE, "(%"PRItid") unexpected after chunk \"%s\"",
		POUND_TID (), buf);
      if (!no_write)
	if (BIO_printf (be, "%s\r\n", buf) <= 0)
	  {
	    logmsg (LOG_NOTICE, "(%"PRItid") error after chunk write: %s",
		    POUND_TID (), strerror (errno));
	    return -6;
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
	  return -7;
	}
      else if (res > 0)
	break;
      if (!no_write)
	if (BIO_printf (be, "%s\r\n", buf) <= 0)
	  {
	    logmsg (LOG_NOTICE, "(%"PRItid") error post-chunk write: %s",
		    POUND_TID (), strerror (errno));
	    return -8;
	  }
      if (!buf[0])
	break;
    }
  if (!no_write)
    if (BIO_flush (be) != 1)
      {
	logmsg (LOG_NOTICE, "(%"PRItid") copy_chunks flush error: %s",
		POUND_TID (), strerror (errno));
	return -4;
      }
  return 0;
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

  if (cmd != BIO_CB_READ && cmd != BIO_CB_WRITE)
    return ret;

  /*
   * a time-out already occured
   */
  if ((bio_arg = (BIO_ARG *) BIO_get_callback_arg (bio)) == NULL)
    return ret;
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
set_callback (BIO *cl, BIO_ARG *arg)
{
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

static void
free_headers (char **headers)
{
  int i;

  for (i = 0; i < MAXHEADERS; i++)
    if (headers[i])
      free (headers[i]);
  free (headers);
  return;
}

static char **
get_headers (BIO *in, BIO *cl, const LISTENER *lstn)
{
  char **headers, buf[MAXBUF];
  int res, n;

  /*
   * HTTP/1.1 allows leading CRLF
   */
  memset (buf, 0, sizeof (buf));
  while ((res = get_line (in, buf, sizeof (buf))) == 0)
    if (buf[0])
      break;

  if (res < 0)
    {
      /*
       * this is expected to occur only on client reads
       */
      /*
       * logmsg(LOG_NOTICE, "headers: bad starting read");
       */
      return NULL;
    }

  if ((headers = calloc (MAXHEADERS, sizeof (char *))) == NULL)
    {
      logmsg (LOG_WARNING, "(%"PRItid") e500 headers: out of memory",
	      POUND_TID ());
      listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
      return NULL;
    }
  if ((headers[0] = malloc (MAXBUF)) == NULL)
    {
      free_headers (headers);
      logmsg (LOG_WARNING, "(%"PRItid") e500 header: out of memory",
	      POUND_TID ());
      listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
      return NULL;
    }
  memset (headers[0], 0, MAXBUF);
  strncpy (headers[0], buf, MAXBUF - 1);

  for (n = 1; n < MAXHEADERS; n++)
    {
      if (get_line (in, buf, sizeof (buf)))
	{
	  free_headers (headers);
	  /*
	   * this is not necessarily an error, EOF/timeout are possible
	   * logmsg(LOG_WARNING, "(%lx) e500 can't read header",
	   * pthread_self()); err_reply(cl, h500, lstn->err500);
	   */
	  return NULL;
	}
      if (!buf[0])
	return headers;
      if ((headers[n] = malloc (MAXBUF)) == NULL)
	{
	  free_headers (headers);
	  logmsg (LOG_WARNING, "(%"PRItid") e500 header: out of memory",
		  POUND_TID ());
	  listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
	  return NULL;
	}
      memset (headers[n], 0, MAXBUF);
      strncpy (headers[n], buf, MAXBUF - 1);
    }

  free_headers (headers);
  logmsg (LOG_NOTICE, "(%"PRItid") e500 too many headers", POUND_TID ());
  listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
  return NULL;
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
  { S("RPC_IN_DATA"),  METH_RPC_IN_DATA,   4 },
  { S("RPC_OUT_DATA"), METH_RPC_OUT_DATA,  4 },
#undef S
  { NULL }
};

static struct method_def *
find_method (const char *str, int group)
{
  struct method_def *m;

  for (m = methods; m->name; m++)
    {
      if (strncasecmp (m->name, str, m->length) == 0)
	return m;
    }
  return NULL;
}

static int
parse_http_request (const char *req, int group,
		    int *ret_meth, char *ret_url, /* FIXME: size_t url_size, */
		    int *ret_http_ver)
{
  size_t len;
  struct method_def *md;
  char const *url;
  int http_ver;
  int n;

  len = strcspn (req, " ");
  if (len == 0 || req[len-1] == 0)
    return -1;

  if ((md = find_method (req, len)) == NULL)
    return -1;

  if (md->group > group)
    return -1;

  req += len;
  req += strspn (req, " ");

  if (*req == 0)
    return -1;

  url = req;
  len = strcspn (url, " ");

  req += len;
  req += strspn (req, " ");
  if (!(strncmp (req, "HTTP/1.", 7) == 0 &&
	((http_ver = req[7]) == '0' || http_ver == '1') &&
	req[8] == 0))
    return -1;

  *ret_meth = md->meth;
  n = cpURL (ret_url, (char*) url, len);
  if (n != strlen (ret_url))
    /*
     * the URL probably contained a %00 aka NULL - which we don't
     * allow
     */
    return -1;

  *ret_http_ver = http_ver;

  return 0;
}

/*
 * HTTP Logging
 *
 * Function signatures are overly complex.  They will get simplified when
 * do_http gets revised.
 */

static char *
anon_addr2str (char *buf, size_t size, struct addrinfo const *from_host)
{
  if (from_host->ai_family == AF_UNIX)
    {
      strncpy (buf, "socket", size);
    }
  else
    {
      addr2str (buf, size, from_host, 1);
      if (anonymise)
	{
	  char *last;

	  if ((last = strrchr (buf, '.')) != NULL
	      || (last = strrchr (buf, ':')) != NULL)
	    strcpy (++last, "0");
	}
    }
  return buf;
}

#define LOG_TIME_SIZE   32

/*
 * Apache log-file-style time format
 */
static char *
log_time_str (char *res, size_t size, struct timespec const *ts)
{
  struct tm tm;
  strftime (res, size, "%d/%b/%Y:%H:%M:%S %z", localtime_r (&ts->tv_sec, &tm));
  return res;
}

#define LOG_BYTES_SIZE  32

/*
 * Apache log-file-style number format
 */
static char *
log_bytes (char *res, size_t size, LONG cnt)
{
  if (cnt > L0)
    snprintf (res, size, "%"PRILONG, cnt);
  else
    strcpy (res, "-");
  return res;
}

static char *
log_duration (char *buf, size_t size, struct timespec const *start)
{
  struct timespec end, diff;
  clock_gettime (CLOCK_REALTIME, &end);
  diff = timespec_sub (&end, start);
  snprintf (buf, size, "%ld.%03ld", diff.tv_sec, diff.tv_nsec / 1000000);
  return buf;
}

static void
http_log_0 (struct addrinfo const *from_host, struct timespec *ts,
	    char *request, char *v_host,
	    char *u_agent, char *referer, char *u_name,
	    LISTENER *lstn, BACKEND *be,
	    int code, char *response, LONG bytes)
{
  /* nothing */
}

static void
http_log_1 (struct addrinfo const *from_host, struct timespec *ts,
	    char *request, char *v_host,
	    char *u_agent, char *referer, char *u_name,
	    LISTENER *lstn, BACKEND *be,
	    int code, char *response, LONG bytes)
{
  char buf[MAX_ADDR_BUFSIZE];
  logmsg (LOG_INFO, "%s %s - %s",
	  anon_addr2str (buf, sizeof (buf), from_host), request, response);
}

static char *
be_service_name (BACKEND *be)
{
  switch (be->be_type)
    {
    case BE_BACKEND:
      if (be->service->name[0])
	return be->service->name;
      break;
    case BE_REDIRECT:
      return "(redirect)";
    case BE_ACME:
      return "(acme)";
    case BE_CONTROL:
      return "(control)";
    }
  return "-";
}

static void
http_log_2 (struct addrinfo const *from_host, struct timespec *ts,
	    char *request, char *v_host,
	    char *u_name,  char *u_agent, char *referer,
	    LISTENER *lstn, BACKEND *be,
	    int code, char *response, LONG bytes)
{
  char caddr[MAX_ADDR_BUFSIZE];
  char baddr[MAX_ADDR_BUFSIZE];
  char timebuf[LOG_TIME_SIZE];

  if (v_host[0])
    logmsg (LOG_INFO,
	    "%s %s - %s (%s/%s -> %s) %s sec",
	    anon_addr2str (caddr, sizeof (caddr), from_host),
	    request, response, v_host,
	    be_service_name (be),
	    str_be (baddr, sizeof (baddr), be),
	    log_duration (timebuf, sizeof (timebuf), ts));
  else
    logmsg (LOG_INFO,
	    "%s %s - %s (%s -> %s) %s sec",
	    anon_addr2str (caddr, sizeof (caddr), from_host),
	    request, response,
	    be_service_name (be),
	    str_be (baddr, sizeof (baddr), be),
	    log_duration (timebuf, sizeof (timebuf), ts));
}

static void
http_log_3 (struct addrinfo const *from_host, struct timespec *ts,
	    char *request, char *v_host,
	    char *u_name, char *u_agent, char *referer,
	    LISTENER *lstn, BACKEND *be,
	    int code, char *response, LONG bytes)
{
  char caddr[MAX_ADDR_BUFSIZE];
  char timebuf[LOG_TIME_SIZE];
  char bytebuf[LOG_BYTES_SIZE];

  logmsg (LOG_INFO,
	  "%s %s - %s [%s] \"%s\" %03d %s \"%s\" \"%s\"",
	  v_host[0] ? v_host : "-",
	  anon_addr2str (caddr, sizeof (caddr), from_host),
	  u_name[0] ? u_name : "-",
	  log_time_str (timebuf, sizeof (timebuf), ts),
	  request,
	  code,
	  log_bytes (bytebuf, sizeof (bytebuf), bytes),
	  referer, u_agent);
}

static void
http_log_4 (struct addrinfo const *from_host, struct timespec *ts,
	    char *request, char *v_host,
	    char *u_agent, char *referer, char *u_name,
	    LISTENER *lstn, BACKEND *be,
	    int code, char *response, LONG bytes)
{
  char caddr[MAX_ADDR_BUFSIZE];
  char timebuf[LOG_TIME_SIZE];
  char bytebuf[LOG_BYTES_SIZE];

  logmsg (LOG_INFO,
	  "%s - %s [%s] \"%s\" %03d %s \"%s\" \"%s\"",
	  anon_addr2str (caddr, sizeof (caddr), from_host),
	  u_name,
	  log_time_str (timebuf, sizeof (timebuf), ts),
	  request,
	  code,
	  log_bytes (bytebuf, sizeof (bytebuf), bytes),
	  referer, u_agent);
}

static void
http_log_5 (struct addrinfo const *from_host, struct timespec *ts,
	    char *request, char *v_host,
	    char *u_agent, char *referer, char *u_name,
	    LISTENER *lstn, BACKEND *be,
	    int code, char *response, LONG bytes)
{
  char caddr[MAX_ADDR_BUFSIZE];
  char baddr[MAX_ADDR_BUFSIZE];
  char timebuf[LOG_TIME_SIZE];
  char dbuf[LOG_TIME_SIZE];
  char bytebuf[LOG_BYTES_SIZE];

  logmsg (LOG_INFO,
	  "%s %s - %s [%s] \"%s\" %03d %s \"%s\" \"%s\" (%s -> %s) %s sec",
	  v_host[0] ? v_host : "-",
	  anon_addr2str (caddr, sizeof (caddr), from_host),
	  u_name,
	  log_time_str (timebuf, sizeof (timebuf), ts),
	  request,
	  code,
	  log_bytes (bytebuf, sizeof (bytebuf), bytes),
	  referer, u_agent,
	  be_service_name (be), str_be (baddr, sizeof (baddr), be),
	  log_duration (dbuf, sizeof (dbuf), ts));
}

static void (*http_logger[]) (struct addrinfo const *, struct timespec *,
			      char *, char *,
			      char *, char *, char *, LISTENER *, BACKEND *,
			      int, char *, LONG) = {
  http_log_0,
  http_log_1,
  http_log_2,
  http_log_3,
  http_log_4,
  http_log_5
};

static void
http_log (struct addrinfo const *from_host, struct timespec *ts,
	  char *request, char *v_host,
	  char *u_agent, char *referer, char *u_name,
	  LISTENER *lstn, BACKEND *be,
	  int code, char *response, LONG bytes)
{
  http_logger[lstn->log_level] (from_host, ts,
				request, v_host,
				u_agent, referer, u_name, lstn, be,
				code, response, bytes);
}

/*
 * Cleanup code. This should really be in the pthread_cleanup_push, except
 * for bugs in some implementations
 */

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#  define clear_error()
#else /* OPENSSL_VERSION_NUMBER >= 0x10000000L */
#  define clear_error() \
	if(ssl != NULL) { ERR_clear_error(); ERR_remove_thread_state(NULL); }
#endif

#define clean_all()                                                     \
  do                                                                    \
    {                                                                   \
      if (ssl != NULL)                                                  \
	BIO_ssl_shutdown (cl);                                          \
      if (be != NULL)                                                   \
	{                                                               \
	  BIO_flush (be);                                               \
	  BIO_reset(be);                                                \
	  BIO_free_all(be);                                             \
	  be = NULL;                                                    \
	}                                                               \
      if (cl != NULL)                                                   \
	{                                                               \
	  BIO_flush (cl);                                               \
	  BIO_reset (cl);                                               \
	  BIO_free_all (cl);                                            \
	  cl = NULL;                                                    \
	}                                                               \
      if (x509 != NULL)                                                 \
	{                                                               \
	  X509_free (x509);                                             \
	  x509 = NULL;                                                  \
	}                                                               \
      clear_error ();                                                   \
      submatch_free (&sm);						\
    }                                                                   \
  while (0)

/*
 * handle an HTTP request
 */
void
do_http (THR_ARG *arg)
{
  int cl_11, be_11, res, chunked, n, sock, no_cont, skip, conn_closed,
    force_10, sock_proto, is_rpc, is_ws;
  int method;
  LISTENER *lstn;
  SERVICE *svc;
  BACKEND *backend, *cur_backend;
  struct addrinfo from_host;
  struct sockaddr_storage from_host_addr;
  BIO *cl, *be, *bb, *b64;
  X509 *x509;
  char request[MAXBUF],
    response[MAXBUF],
    buf[MAXBUF],
    url[MAXBUF],
    loc_path[MAXBUF],
    **headers,
    headers_ok[MAXHEADERS],
    v_host[MAXBUF], referer[MAXBUF], u_agent[MAXBUF], u_name[MAXBUF],
    caddr[MAX_ADDR_BUFSIZE], *mh;
  char duration_buf[LOG_TIME_SIZE];
  SSL *ssl, *be_ssl;
  LONG cont, res_bytes = 0;
  regmatch_t matches[4];
  struct linger l;
  struct timespec start_req;
  RENEG_STATE reneg_state;
  BIO_ARG ba1, ba2;
  enum
  {
    WSS_REQ_GET = 0x01,
    WSS_REQ_HEADER_CONNECTION_UPGRADE = 0x02,
    WSS_REQ_HEADER_UPGRADE_WEBSOCKET = 0x04,

    WSS_RESP_101 = 0x08,
    WSS_RESP_HEADER_CONNECTION_UPGRADE = 0x10,
    WSS_RESP_HEADER_UPGRADE_WEBSOCKET = 0x20,
    WSS_COMPLETE = WSS_REQ_GET
      | WSS_REQ_HEADER_CONNECTION_UPGRADE
      | WSS_REQ_HEADER_UPGRADE_WEBSOCKET | WSS_RESP_101 |
      WSS_RESP_HEADER_CONNECTION_UPGRADE | WSS_RESP_HEADER_UPGRADE_WEBSOCKET
  };
  struct submatch sm = SUBMATCH_INITIALIZER;

  reneg_state = RENEG_INIT;
  ba1.reneg_state = &reneg_state;
  ba2.reneg_state = &reneg_state;
  ba1.timeout = 0;
  ba2.timeout = 0;
  from_host = arg->from_host;
  memcpy (&from_host_addr, from_host.ai_addr, from_host.ai_addrlen);
  from_host.ai_addr = (struct sockaddr *) &from_host_addr;
  lstn = arg->lstn;
  sock = arg->sock;
  free (arg->from_host.ai_addr);
  free (arg);

  if (lstn->allow_client_reneg)
    reneg_state = RENEG_ALLOW;

  n = 1;
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

  cl = NULL;
  be = NULL;
  ssl = NULL;
  x509 = NULL;

  if ((cl = BIO_new_socket (sock, 1)) == NULL)
    {
      logmsg (LOG_WARNING, "(%"PRItid") BIO_new_socket failed", POUND_TID ());
      shutdown (sock, 2);
      close (sock);
      return;
    }
  ba1.timeout = lstn->to;
  set_callback (cl, &ba1);

  if (!SLIST_EMPTY (&lstn->ctx_head))
    {
      if ((ssl = SSL_new (SLIST_FIRST (&lstn->ctx_head)->ctx)) == NULL)
	{
	  logmsg (LOG_WARNING, "(%"PRItid") SSL_new: failed", POUND_TID ());
	  BIO_reset (cl);
	  BIO_free_all (cl);
	  return;
	}
      SSL_set_app_data (ssl, &reneg_state);
      SSL_set_bio (ssl, cl, cl);
      if ((bb = BIO_new (BIO_f_ssl ())) == NULL)
	{
	  logmsg (LOG_WARNING, "(%"PRItid") BIO_new(Bio_f_ssl()) failed",
		  POUND_TID ());
	  BIO_reset (cl);
	  BIO_free_all (cl);
	  return;
	}
      BIO_set_ssl (bb, ssl, BIO_CLOSE);
      BIO_set_ssl_mode (bb, 0);
      cl = bb;
      if (BIO_do_handshake (cl) <= 0)
	{
	  /*
	   * no need to log every client without a certificate...
	   * addr2str(caddr, sizeof (caddr), &from_host, 1);
	   * logmsg(LOG_NOTICE, "BIO_do_handshake with %s failed: %s",
	   * caddr, ERR_error_string(ERR_get_error(), NULL)); x509 =
	   * NULL;
	   */
	  BIO_reset (cl);
	  BIO_free_all (cl);
	  return;
	}
      else
	{
	  if ((x509 = SSL_get_peer_certificate (ssl)) != NULL
	      && lstn->clnt_check < 3
	      && SSL_get_verify_result (ssl) != X509_V_OK)
	    {
	      logmsg (LOG_NOTICE, "Bad certificate from %s",
		      addr2str (caddr, sizeof (caddr), &from_host, 1));
	      X509_free (x509);
	      BIO_reset (cl);
	      BIO_free_all (cl);
	      return;
	    }
	}
    }
  else
    {
      x509 = NULL;
    }
  cur_backend = NULL;

  if ((bb = BIO_new (BIO_f_buffer ())) == NULL)
    {
      logmsg (LOG_WARNING, "(%"PRItid") BIO_new(buffer) failed", POUND_TID ());
      if (x509 != NULL)
	X509_free (x509);
      BIO_reset (cl);
      BIO_free_all (cl);
      return;
    }
  BIO_set_close (cl, BIO_CLOSE);
  BIO_set_buffer_size (cl, MAXBUF);
  cl = BIO_push (bb, cl);

  for (cl_11 = be_11 = 0;;)
    {
      res_bytes = L0;
      is_rpc = -1;
      is_ws = 0;
      v_host[0] = referer[0] = u_agent[0] = u_name[0] = '\0';
      conn_closed = 0;
      for (n = 0; n < MAXHEADERS; n++)
	headers_ok[n] = 1;
      if ((headers = get_headers (cl, cl, lstn)) == NULL)
	{
	  if (!cl_11)
	    {
	      if (errno)
		{
		  logmsg (LOG_NOTICE, "(%"PRItid") error read from %s: %s",
			  POUND_TID (),
			  addr2str (caddr, sizeof (caddr), &from_host, 1),
			  strerror (errno));
		  /*
		   * err_reply(cl, h500, lstn->err500);
		   */
		}
	    }
	  clean_all ();
	  return;
	}

      clock_gettime (CLOCK_REALTIME, &start_req);

      /*
       * check for correct request
       */
      strncpy (request, headers[0], MAXBUF);
      if (parse_http_request (request, lstn->verb, &method, url, &cl_11))
	{
	  logmsg (LOG_WARNING, "(%"PRItid") e501 bad request \"%s\" from %s",
		  POUND_TID (), request,
		  addr2str (caddr, sizeof (caddr), &from_host, 1));
	  listener_err_reply (cl, HTTP_STATUS_NOT_IMPLEMENTED, lstn);
	  free_headers (headers);
	  clean_all ();
	  return;
	}

      no_cont = method == METH_HEAD;
      switch (method)
	{
	case METH_RPC_IN_DATA:
	  is_rpc = 1;
	  break;

	case METH_RPC_OUT_DATA:
	  is_rpc = 0;
	  break;

	case METH_GET:
	  is_ws |= WSS_REQ_GET;
	}

      if (lstn->has_pat && regexec (&lstn->url_pat, url, 0, NULL, 0))
	{
	  logmsg (LOG_NOTICE, "(%"PRItid") e501 bad URL \"%s\" from %s",
		  POUND_TID (), url,
		  addr2str (caddr, sizeof (caddr), &from_host, 1));
	  listener_err_reply (cl, HTTP_STATUS_NOT_IMPLEMENTED, lstn);
	  free_headers (headers);
	  clean_all ();
	  return;
	}

      /*
       * check other headers
       */
      for (chunked = 0, cont = L_1, n = 1; n < MAXHEADERS && headers[n]; n++)
	{
	  /*
	   * no overflow - see check_header for details
	   */
	  switch (check_header (headers[n], buf))
	    {
	    case HEADER_HOST:
	      strcpy (v_host, buf);
	      break;

	    case HEADER_REFERER:
	      strcpy (referer, buf);
	      break;

	    case HEADER_USER_AGENT:
	      strcpy (u_agent, buf);
	      break;

	    case HEADER_CONNECTION:
	      if (!strcasecmp ("close", buf))
		conn_closed = 1;
	      /*
	       * Connection: upgrade
	       */
	      else if (!regexec (&CONN_UPGRD, buf, 0, NULL, 0))
		is_ws |= WSS_REQ_HEADER_CONNECTION_UPGRADE;
	      break;

	    case HEADER_UPGRADE:
	      if (!strcasecmp ("websocket", buf))
		is_ws |= WSS_REQ_HEADER_UPGRADE_WEBSOCKET;
	      break;

	    case HEADER_TRANSFER_ENCODING:
	      if (!strcasecmp ("chunked", buf))
		chunked = 1;
	      else
		{
		  logmsg (LOG_NOTICE,
			  "(%"PRItid") e400 multiple Transfer-encoding \"%s\" from %s",
			  POUND_TID (), url,
			  addr2str (caddr, sizeof (caddr), &from_host, 1));
		  err_reply (cl, HTTP_STATUS_BAD_REQUEST,
			     "Bad request: multiple Transfer-encoding values");
		  free_headers (headers);
		  clean_all ();
		  return;
		}
	      break;

	    case HEADER_CONTENT_LENGTH:
	      if (cont != L_1 || strchr (buf, ','))
		{
		  logmsg (LOG_NOTICE,
			  "(%"PRItid") e400 multiple Content-length \"%s\" from %s",
			  POUND_TID (), url,
			  addr2str (caddr, sizeof (caddr), &from_host, 1));
		  err_reply (cl, HTTP_STATUS_BAD_REQUEST,
			     "Bad request: multiple Content-length values");
		  free_headers (headers);
		  clean_all ();
		  return;
		}
	      for (mh = buf; *mh; mh++)
		if (!isdigit (*mh))
		  {
		    logmsg (LOG_NOTICE,
			    "(%"PRItid") e400 Content-length bad value \"%s\" from %s",
			    POUND_TID (), url,
			    addr2str (caddr, sizeof (caddr), &from_host, 1));
		    err_reply (cl, HTTP_STATUS_BAD_REQUEST,
			       "Bad request: Content-length bad value");
		    free_headers (headers);
		    clean_all ();
		    return;
		  }
	      if ((cont = ATOL (buf)) < 0L)
		headers_ok[n] = 0;
	      if (is_rpc == 1 && (cont < 0x20000L || cont > 0x80000000L))
		is_rpc = -1;
	      break;

	    case HEADER_EXPECT:
	      /*
	       * we do NOT support the "Expect: 100-continue" headers
	       * support may involve severe performance penalties (non-responding back-end, etc)
	       * as a stop-gap measure we just skip these headers
	       */
	      if (!strcasecmp ("100-continue", buf))
		headers_ok[n] = 0;
	      break;

	    case HEADER_ILLEGAL:
	      if (lstn->log_level > 0)
		{
		  logmsg (LOG_NOTICE, "(%"PRItid") bad header from %s (%s)",
			  POUND_TID (),
			  addr2str (caddr, sizeof (caddr), &from_host, 1),
			  headers[n]);
		}
	      headers_ok[n] = 0;
	      break;
	    }

	  if (headers_ok[n] && !SLIST_EMPTY (&lstn->head_off))
	    {
	      /*
	       * maybe header to be removed
	       */
	      MATCHER *m;

	      SLIST_FOREACH (m, &lstn->head_off, next)
		if (!(headers_ok[n] = regexec (&m->pat, headers[n], 0, NULL, 0)))
		  break;
	    }
	  /*
	   * get User name
	   */
	  if (!regexec (&AUTHORIZATION, headers[n], 2, matches, 0))
	    {
	      int inlen;

	      if ((bb = BIO_new (BIO_s_mem ())) == NULL)
		{
		  logmsg (LOG_WARNING, "(%"PRItid") Can't alloc BIO_s_mem",
			  POUND_TID ());
		  continue;
		}
	      if ((b64 = BIO_new (BIO_f_base64 ())) == NULL)
		{
		  logmsg (LOG_WARNING, "(%"PRItid") Can't alloc BIO_f_base64",
			  POUND_TID ());
		  BIO_free (bb);
		  continue;
		}
	      b64 = BIO_push (b64, bb);
	      BIO_write (bb, headers[n] + matches[1].rm_so,
			 matches[1].rm_eo - matches[1].rm_so);
	      BIO_write (bb, "\n", 1);
	      if ((inlen = BIO_read (b64, buf, sizeof (buf))) <= 0)
		{
		  logmsg (LOG_WARNING, "(%"PRItid") Can't read BIO_f_base64",
			  POUND_TID ());
		  BIO_free_all (b64);
		  continue;
		}
	      BIO_free_all (b64);
	      if ((mh = memchr (buf, ':', inlen)) == NULL)
		{
		  logmsg (LOG_WARNING, "(%"PRItid") Unknown authentication",
			  POUND_TID ());
		  continue;
		}
	      *mh = '\0';
	      strcpy (u_name, buf);
	    }
	}

      /*
       * check for possible request smuggling attempt
       */
      if (chunked != 0 && cont != L_1)
	{
	  logmsg (LOG_NOTICE,
		  "(%"PRItid") e501 Transfer-encoding and Content-length \"%s\" from %s",
		  POUND_TID (), url,
		  addr2str (caddr, sizeof (caddr), &from_host, 1));
	  err_reply (cl, HTTP_STATUS_BAD_REQUEST,
		     "Bad request: Transfer-encoding and Content-length headers present");
	  free_headers (headers);
	  clean_all ();
	  return;
	}

      /*
       * possibly limited request size
       */
      if (lstn->max_req > L0 && cont > L0 && cont > lstn->max_req
	  && is_rpc != 1)
	{
	  logmsg (LOG_NOTICE, "(%"PRItid") e413 request too large (%"PRILONG") from %s",
		  POUND_TID (), cont,
		  addr2str (caddr, sizeof (caddr), &from_host, 1));
	  listener_err_reply (cl, HTTP_STATUS_PAYLOAD_TOO_LARGE, lstn);
	  free_headers (headers);
	  clean_all ();
	  return;
	}

      if (be != NULL)
	{
	  if (is_readable (be, 0))
	    {
	      /*
	       * The only way it's readable is if it's at EOF, so close
	       * it!
	       */
	      BIO_reset (be);
	      BIO_free_all (be);
	      be = NULL;
	    }
	}

      /*
       * check that the requested URL still fits the old back-end (if
       * any)
       */
      if ((svc = get_service (lstn, from_host.ai_addr, url, &headers[1], &sm)) == NULL)
	{
	  logmsg (LOG_NOTICE, "(%"PRItid") e503 no service \"%s\" from %s %s",
		  POUND_TID (), request,
		  addr2str (caddr, sizeof (caddr), &from_host, 1),
		  v_host[0] ? v_host : "-");
	  listener_err_reply (cl, HTTP_STATUS_SERVICE_UNAVAILABLE, lstn);
	  free_headers (headers);
	  clean_all ();
	  return;
	}
      if ((backend = get_backend (svc, &from_host, url, &headers[1])) == NULL)
	{
	  logmsg (LOG_NOTICE, "(%"PRItid") e503 no back-end \"%s\" from %s %s",
		  POUND_TID (), request,
		  addr2str (caddr, sizeof (caddr), &from_host, 1),
		  v_host[0] ? v_host : "-");
	  listener_err_reply (cl, HTTP_STATUS_SERVICE_UNAVAILABLE, lstn);
	  free_headers (headers);
	  clean_all ();
	  return;
	}

      if (be != NULL && backend != cur_backend)
	{
	  BIO_reset (be);
	  BIO_free_all (be);
	  be = NULL;
	}
      while (be == NULL && backend->be_type == BE_BACKEND)
	{
	  switch (backend->addr.ai_family)
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
	      logmsg (LOG_WARNING, "(%"PRItid") e503 backend: unknown family %d",
		      POUND_TID (), backend->addr.ai_family);
	      listener_err_reply (cl, HTTP_STATUS_SERVICE_UNAVAILABLE, lstn);
	      free_headers (headers);
	      clean_all ();
	      return;
	    }

	  if ((sock = socket (sock_proto, SOCK_STREAM, 0)) < 0)
	    {
	      str_be (buf, sizeof (buf), backend);
	      logmsg (LOG_WARNING, "(%"PRItid") e503 backend %s socket create: %s",
		      POUND_TID (), buf, strerror (errno));
	      listener_err_reply (cl, HTTP_STATUS_SERVICE_UNAVAILABLE, lstn);
	      free_headers (headers);
	      clean_all ();
	      return;
	    }
	  if (connect_nb (sock, &backend->addr, backend->conn_to) < 0)
	    {
	      str_be (buf, sizeof (buf), backend);
	      logmsg (LOG_WARNING, "(%"PRItid") backend %s connect: %s",
		      POUND_TID (), buf, strerror (errno));
	      shutdown (sock, 2);
	      close (sock);
	      kill_be (svc, backend, BE_KILL);
	      if ((backend = get_backend (svc, &from_host, url, &headers[1])) == NULL)
		{
		  logmsg (LOG_NOTICE, "(%"PRItid") e503 no back-end \"%s\" from %s",
			  POUND_TID (), request,
			  addr2str (caddr, sizeof (caddr), &from_host, 1));
		  listener_err_reply (cl, HTTP_STATUS_SERVICE_UNAVAILABLE, lstn);
		  free_headers (headers);
		  clean_all ();
		  return;
		}
	      continue;
	    }

	  if (sock_proto == PF_INET || sock_proto == PF_INET6)
	    {
	      n = 1;
	      setsockopt (sock, SOL_SOCKET, SO_KEEPALIVE, (void *) &n,
			  sizeof (n));
	      l.l_onoff = 1;
	      l.l_linger = 10;
	      setsockopt (sock, SOL_SOCKET, SO_LINGER, (void *) &l,
			  sizeof (l));
#ifdef  TCP_LINGER2
	      n = 5;
	      setsockopt (sock, SOL_TCP, TCP_LINGER2, (void *) &n,
			  sizeof (n));
#endif
	      n = 1;
	      setsockopt (sock, SOL_TCP, TCP_NODELAY, (void *) &n,
			  sizeof (n));
	    }
	  if ((be = BIO_new_socket (sock, 1)) == NULL)
	    {
	      logmsg (LOG_WARNING, "(%"PRItid") e503 BIO_new_socket server failed",
		      POUND_TID ());
	      shutdown (sock, 2);
	      close (sock);
	      listener_err_reply (cl, HTTP_STATUS_SERVICE_UNAVAILABLE, lstn);
	      free_headers (headers);
	      clean_all ();
	      return;
	    }
	  BIO_set_close (be, BIO_CLOSE);
	  if (backend->to > 0)
	    {
	      ba2.timeout = backend->to;
	      set_callback (be, &ba2);
	    }
	  if (backend->ctx != NULL)
	    {
	      if ((be_ssl = SSL_new (backend->ctx)) == NULL)
		{
		  logmsg (LOG_WARNING, "(%"PRItid") be SSL_new: failed",
			  POUND_TID ());
		  listener_err_reply (cl, HTTP_STATUS_SERVICE_UNAVAILABLE, lstn);
		  free_headers (headers);
		  clean_all ();
		  return;
		}
	      SSL_set_bio (be_ssl, be, be);
	      if ((bb = BIO_new (BIO_f_ssl ())) == NULL)
		{
		  logmsg (LOG_WARNING, "(%"PRItid") BIO_new(Bio_f_ssl()) failed",
			  POUND_TID ());
		  listener_err_reply (cl, HTTP_STATUS_SERVICE_UNAVAILABLE, lstn);
		  free_headers (headers);
		  clean_all ();
		  return;
		}
	      BIO_set_ssl (bb, be_ssl, BIO_CLOSE);
	      BIO_set_ssl_mode (bb, 1);
	      be = bb;
	      if (BIO_do_handshake (be) <= 0)
		{
		  str_be (buf, sizeof (buf), backend);
		  logmsg (LOG_NOTICE, "BIO_do_handshake with %s failed: %s",
			  buf, ERR_error_string (ERR_get_error (), NULL));
		  listener_err_reply (cl, HTTP_STATUS_SERVICE_UNAVAILABLE, lstn);
		  free_headers (headers);
		  clean_all ();
		  return;
		}
	    }
	  if ((bb = BIO_new (BIO_f_buffer ())) == NULL)
	    {
	      logmsg (LOG_WARNING, "(%"PRItid") e503 BIO_new(buffer) server failed",
		      POUND_TID ());
	      listener_err_reply (cl, HTTP_STATUS_SERVICE_UNAVAILABLE, lstn);
	      free_headers (headers);
	      clean_all ();
	      return;
	    }
	  BIO_set_buffer_size (bb, MAXBUF);
	  BIO_set_close (bb, BIO_CLOSE);
	  be = BIO_push (bb, be);
	}
      cur_backend = backend;

      /*
       * if we have anything but a BACK_END we close the channel
       */
      if (be != NULL && cur_backend->be_type != BE_BACKEND)
	{
	  BIO_reset (be);
	  BIO_free_all (be);
	  be = NULL;
	}

      /*
       * send the request
       */
      if (cur_backend->be_type == BE_BACKEND)
	{
	  for (n = 0; n < MAXHEADERS && headers[n]; n++)
	    {
	      if (!headers_ok[n])
		continue;
	      /*
	       * this is the earliest we can check for Destination - we
	       * had no back-end before
	       */
	      if (lstn->rewr_dest
		  && check_header (headers[n], buf) == HEADER_DESTINATION)
		{
		  if (regexec (&LOCATION, buf, 4, matches, 0))
		    {
		      logmsg (LOG_NOTICE, "(%"PRItid") Can't parse Destination %s",
			      POUND_TID (), buf);
		      break;
		    }
		  str_be (caddr, sizeof (caddr), cur_backend);
		  strcpy (loc_path, buf + matches[3].rm_so);
		  snprintf (buf, sizeof (buf),
			    "Destination: http://%s%s", caddr,
			    loc_path);
		  free (headers[n]);
		  if ((headers[n] = strdup (buf)) == NULL)
		    {
		      logmsg (LOG_WARNING,
			      "(%"PRItid") rewrite Destination - out of memory: %s",
			      POUND_TID (), strerror (errno));
		      free_headers (headers);
		      clean_all ();
		      return;
		    }
		}
	      if (BIO_printf (be, "%s\r\n", headers[n]) <= 0)
		{
		  str_be (buf, sizeof (buf), cur_backend);
		  logmsg (LOG_WARNING,
			  "(%"PRItid") e500 error write to %s/%s: %s (%s sec)",
			  POUND_TID (), buf, request, strerror (errno),
			  log_duration (duration_buf, sizeof (duration_buf), &start_req));
		  listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
		  free_headers (headers);
		  clean_all ();
		  return;
		}
	    }
	  /*
	   * add header if required
	   */
	  if (lstn->add_head != NULL)
	    if (BIO_printf (be, "%s\r\n", lstn->add_head) <= 0)
	      {
		str_be (buf, sizeof (buf), cur_backend);
		logmsg (LOG_WARNING,
			"(%"PRItid") e500 error write AddHeader to %s: %s (%s sec)",
			POUND_TID (), buf, strerror (errno),
			log_duration (duration_buf, sizeof (duration_buf), &start_req));
		listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
		free_headers (headers);
		clean_all ();
		return;
	      }
	}
      free_headers (headers);

      /*
       * if SSL put additional headers for client certificate
       */
      if (cur_backend->be_type == BE_BACKEND && ssl != NULL)
	{
	  const SSL_CIPHER *cipher;

	  if ((cipher = SSL_get_current_cipher (ssl)) != NULL)
	    {
	      SSL_CIPHER_description (cipher, buf, sizeof (buf));
	      strip_eol (buf);
	      if (BIO_printf (be, "X-SSL-cipher: %s/%s\r\n",
			      SSL_get_version (ssl),
			      buf) <= 0)
		{
		  str_be (buf, sizeof (buf), cur_backend);
		  logmsg (LOG_WARNING,
			  "(%"PRItid") e500 error write X-SSL-cipher to %s: %s (%s sec)",
			  POUND_TID (), buf, strerror (errno),
			  log_duration (duration_buf, sizeof (duration_buf), &start_req));
		  listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
		  clean_all ();
		  return;
		}
	    }

	  if (lstn->clnt_check > 0 && x509 != NULL
	      && (bb = BIO_new (BIO_s_mem ())) != NULL)
	    {
	      X509_NAME_print_ex (bb, X509_get_subject_name (x509), 8,
				  XN_FLAG_ONELINE & ~ASN1_STRFLGS_ESC_MSB);
	      get_line (bb, buf, sizeof (buf));
	      if (BIO_printf (be, "X-SSL-Subject: %s\r\n", buf) <= 0)
		{
		  str_be (buf, sizeof (buf), cur_backend);
		  logmsg (LOG_WARNING,
			  "(%"PRItid") e500 error write X-SSL-Subject to %s: %s (%s sec)",
			  POUND_TID (), buf, strerror (errno),
			  log_duration (duration_buf, sizeof (duration_buf), &start_req));
		  listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
		  BIO_free_all (bb);
		  clean_all ();
		  return;
		}

	      X509_NAME_print_ex (bb, X509_get_issuer_name (x509), 8,
				  XN_FLAG_ONELINE & ~ASN1_STRFLGS_ESC_MSB);
	      get_line (bb, buf, sizeof (buf));
	      if (BIO_printf (be, "X-SSL-Issuer: %s\r\n", buf) <= 0)
		{
		  str_be (buf, sizeof (buf), cur_backend);
		  logmsg (LOG_WARNING,
			  "(%"PRItid") e500 error write X-SSL-Issuer to %s: %s (%s sec)",
			  POUND_TID (), buf, strerror (errno),
			  log_duration (duration_buf, sizeof (duration_buf), &start_req));
		  listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
		  BIO_free_all (bb);
		  clean_all ();
		  return;
		}

	      ASN1_TIME_print (bb, X509_get_notBefore (x509));
	      get_line (bb, buf, sizeof (buf));
	      if (BIO_printf (be, "X-SSL-notBefore: %s\r\n", buf) <= 0)
		{
		  str_be (buf, sizeof (buf), cur_backend);
		  logmsg (LOG_WARNING,
			  "(%"PRItid") e500 error write X-SSL-notBefore to %s: %s (%s sec)",
			  POUND_TID (), buf, strerror (errno),
			  log_duration (duration_buf, sizeof (duration_buf), &start_req));
		  listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
		  BIO_free_all (bb);
		  clean_all ();
		  return;
		}

	      ASN1_TIME_print (bb, X509_get_notAfter (x509));
	      get_line (bb, buf, sizeof (buf));
	      if (BIO_printf (be, "X-SSL-notAfter: %s\r\n", buf) <= 0)
		{
		  str_be (buf, sizeof (buf), cur_backend);
		  logmsg (LOG_WARNING,
			  "(%"PRItid") e500 error write X-SSL-notAfter to %s: %s (%s sec)",
			  POUND_TID (), buf, strerror (errno),
			  log_duration (duration_buf, sizeof (duration_buf), &start_req));
		  listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
		  BIO_free_all (bb);
		  clean_all ();
		  return;
		}
	      if (BIO_printf (be, "X-SSL-serial: %ld\r\n",
			      ASN1_INTEGER_get (X509_get_serialNumber (x509))) <= 0)
		{
		  str_be (buf, sizeof (buf), cur_backend);
		  logmsg (LOG_WARNING,
			  "(%"PRItid") e500 error write X-SSL-serial to %s: %s (%s sec)",
			  POUND_TID (), buf, strerror (errno),
			  log_duration (duration_buf, sizeof (duration_buf), &start_req));
		  listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
		  BIO_free_all (bb);
		  clean_all ();
		  return;
		}
	      PEM_write_bio_X509 (bb, x509);
	      get_line (bb, buf, sizeof (buf));
	      if (BIO_printf (be, "X-SSL-certificate: %s", buf) <= 0)
		{
		  str_be (buf, sizeof (buf), cur_backend);
		  logmsg (LOG_WARNING,
			  "(%"PRItid") e500 error write X-SSL-certificate to %s: %s (%s sec)",
			  POUND_TID (), buf, strerror (errno),
			  log_duration (duration_buf, sizeof (duration_buf), &start_req));
		  listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
		  BIO_free_all (bb);
		  clean_all ();
		  return;
		}
	      while (get_line (bb, buf, sizeof (buf)) == 0)
		{
		  if (BIO_printf (be, "%s", buf) <= 0)
		    {
		      str_be (buf, sizeof (buf), cur_backend);
		      logmsg (LOG_WARNING,
			      "(%"PRItid") e500 error write X-SSL-certificate to %s: %s (%s sec)",
			      POUND_TID (), buf, strerror (errno),
			      log_duration (duration_buf, sizeof (duration_buf), &start_req));
		      listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
		      BIO_free_all (bb);
		      clean_all ();
		      return;
		    }
		}
	      if (BIO_printf (be, "\r\n") <= 0)
		{
		  str_be (buf, sizeof (buf), cur_backend);
		  logmsg (LOG_WARNING,
			  "(%"PRItid") e500 error write X-SSL-certificate to %s: %s (%s sec)",
			  POUND_TID (), buf, strerror (errno),
			  log_duration (duration_buf, sizeof (duration_buf), &start_req));
		  listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
		  BIO_free_all (bb);
		  clean_all ();
		  return;
		}
	      BIO_free_all (bb);
	    }
	}
      /*
       * put additional client IP header
       */
      if (cur_backend->be_type == BE_BACKEND)
	{
	  addr2str (caddr, sizeof (caddr), &from_host, 1);
	  BIO_printf (be, "X-Forwarded-For: %s\r\n", caddr);

	  /*
	   * final CRLF
	   */
	  BIO_puts (be, "\r\n");
	}

      if (cl_11 && chunked)
	{
	  /*
	   * had Transfer-encoding: chunked so read/write all the chunks
	   * (HTTP/1.1 only)
	   */
	  if (copy_chunks (cl, be, NULL, cur_backend->be_type != BE_BACKEND,
			   lstn->max_req))
	    {
	      str_be (buf, sizeof (buf), cur_backend);
	      logmsg (LOG_NOTICE,
		      "(%"PRItid") e500 for %s copy_chunks to %s/%s (%s sec)",
		      POUND_TID (),
		      addr2str (caddr, sizeof (caddr), &from_host, 1),
		      buf, request,
		      log_duration (duration_buf, sizeof (duration_buf), &start_req));
	      listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
	      clean_all ();
	      return;
	    }
	}
      else if (cont > L0 && is_rpc != 1)
	{
	  /*
	   * had Content-length, so do raw reads/writes for the length
	   */
	  if (copy_bin (cl, be, cont, NULL, cur_backend->be_type != BE_BACKEND))
	    {
	      str_be (buf, sizeof (buf), cur_backend);
	      logmsg (LOG_NOTICE,
		      "(%"PRItid") e500 for %s error copy client cont to %s/%s: %s (%s sec)",
		      POUND_TID (),
		      addr2str (caddr, sizeof (caddr), &from_host, 1),
		      buf, request, strerror (errno),
		      log_duration (duration_buf, sizeof (duration_buf), &start_req));
	      listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
	      clean_all ();
	      return;
	    }
	}
      else if (cont > 0L && is_readable (cl, lstn->to))
	{
	  char one;
	  BIO *cl_unbuf;
	  /*
	   * special mode for RPC_IN_DATA - content until EOF
	   * force HTTP/1.0 - client closes connection when done.
	   */
	  cl_11 = be_11 = 0;

	  /*
	   * first read whatever is already in the input buffer
	   */
	  while (BIO_pending (cl))
	    {
	      if (BIO_read (cl, &one, 1) != 1)
		{
		  logmsg (LOG_NOTICE, "(%"PRItid") error read request pending: %s",
			  POUND_TID (), strerror (errno));
		  clean_all ();
		  pthread_exit (NULL);
		}
	      if (++res_bytes > cont)
		{
		  logmsg (LOG_NOTICE,
			  "(%"PRItid") error read request pending: max. RPC length exceeded",
			  POUND_TID ());
		  clean_all ();
		  pthread_exit (NULL);
		}
	      if (BIO_write (be, &one, 1) != 1)
		{
		  if (errno)
		    logmsg (LOG_NOTICE,
			    "(%"PRItid") error write request pending: %s",
			    POUND_TID (), strerror (errno));
		  clean_all ();
		  pthread_exit (NULL);
		}
	    }
	  BIO_flush (be);

	  /*
	   * find the socket BIO in the chain
	   */
	  if ((cl_unbuf = BIO_find_type (cl,
					 SLIST_EMPTY (&lstn->ctx_head)
					   ? BIO_TYPE_SOCKET : BIO_TYPE_SSL)) == NULL)
	    {
	      logmsg (LOG_WARNING, "(%"PRItid") error get unbuffered: %s",
		      POUND_TID (), strerror (errno));
	      clean_all ();
	      pthread_exit (NULL);
	    }

	  /*
	   * copy till EOF
	   */
	  while ((res = BIO_read (cl_unbuf, buf, sizeof (buf))) > 0)
	    {
	      if ((res_bytes += res) > cont)
		{
		  logmsg (LOG_NOTICE,
			  "(%"PRItid") error copy request body: max. RPC length exceeded",
			  POUND_TID ());
		  clean_all ();
		  pthread_exit (NULL);
		}
	      if (BIO_write (be, buf, res) != res)
		{
		  if (errno)
		    logmsg (LOG_NOTICE, "(%"PRItid") error copy request body: %s",
			    POUND_TID (), strerror (errno));
		  clean_all ();
		  pthread_exit (NULL);
		}
	      else
		{
		  BIO_flush (be);
		}
	    }
	}

      /*
       * flush to the back-end
       */
      if (cur_backend->be_type == BE_BACKEND && BIO_flush (be) != 1)
	{
	  str_be (buf, sizeof (buf), cur_backend);
	  logmsg (LOG_NOTICE,
		  "(%"PRItid") e500 for %s error flush to %s/%s: %s (%s sec)",
		  POUND_TID (),
		  addr2str (caddr, sizeof (caddr), &from_host, 1),
		  buf, request, strerror (errno),
		  log_duration (duration_buf, sizeof (duration_buf), &start_req));
	  listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
	  clean_all ();
	  return;
	}

      /*
       * check on no_https_11:
       *  - if 0 ignore
       *  - if 1 and SSL force HTTP/1.0
       *  - if 2 and SSL and MSIE force HTTP/1.0
       */
      switch (lstn->noHTTPS11)
	{
	case 1:
	  force_10 = (ssl != NULL);
	  break;

	case 2:
	  force_10 = (ssl != NULL && strstr (u_agent, "MSIE") != NULL);
	  break;

	default:
	  force_10 = 0;
	  break;
	}

      if (cur_backend->be_type != BE_BACKEND)
	{
	  int code;

	  switch (cur_backend->be_type)
	    {
	    case BE_REDIRECT:
	      code = redirect_reply (cl, url, cur_backend, &sm);
	      break;

	    case BE_ACME:
	      code = acme_reply (cl, url, cur_backend, &sm);
	      break;

	    case BE_CONTROL:
	      code = control_reply (cl, method, url, cur_backend);
	    }

	  if (code != HTTP_STATUS_OK)
	    listener_err_reply (cl, code, lstn);

	  http_log (&from_host, &start_req,
		    request, v_host,
		    u_agent, referer, u_name,
		    lstn, cur_backend,
		    code, response, 0); //FIXME: number of bytes

	  if (!cl_11 || conn_closed || force_10)
	    break;
	  continue;
	}
      else if (is_rpc == 1)
	{
	  http_log (&from_host, &start_req,
		    request, v_host,
		    u_agent, referer, u_name, lstn, cur_backend,
		    0, response, res_bytes); //FIXME: response code
	  /*
	   * no response expected - bail out
	   */
	  break;
	}

      /*
       * get the response
       */
      for (skip = 1; skip;)
	{
	  if ((headers = get_headers (be, cl, lstn)) == NULL)
	    {
	      str_be (buf, sizeof (buf), cur_backend);
	      logmsg (LOG_NOTICE,
		      "(%"PRItid") e500 for %s response error read from %s/%s: %s (%s secs)",
		      POUND_TID (),
		      addr2str (caddr, sizeof (caddr), &from_host, 1),
		      buf, request, strerror (errno),
		      log_duration (duration_buf, sizeof (duration_buf), &start_req));
	      listener_err_reply (cl, HTTP_STATUS_INTERNAL_SERVER_ERROR, lstn);
	      clean_all ();
	      return;
	    }

	  strncpy (response, headers[0], sizeof (response));
	  be_11 = (response[7] == '1');
	  /*
	   * responses with code 100 are never passed back to the client
	   */
	  skip = !regexec (&RESP_SKIP, response, 0, NULL, 0);
	  /*
	   * some response codes (1xx, 204, 304) have no content
	   */
	  if (!no_cont && !regexec (&RESP_IGN, response, 0, NULL, 0))
	    no_cont = 1;
	  if (!strncasecmp ("101", response + 9, 3))
	    is_ws |= WSS_RESP_101;

	  for (chunked = 0, cont = -1L, n = 1; n < MAXHEADERS && headers[n];
	       n++)
	    {
	      switch (check_header (headers[n], buf))
		{
		case HEADER_CONNECTION:
		  if (!strcasecmp ("close", buf))
		    conn_closed = 1;
		  /*
		   * Connection: upgrade
		   */
		  else if (!regexec (&CONN_UPGRD, buf, 0, NULL, 0))
		    is_ws |= WSS_RESP_HEADER_CONNECTION_UPGRADE;
		  break;

		case HEADER_UPGRADE:
		  if (!strcasecmp ("websocket", buf))
		    is_ws |= WSS_RESP_HEADER_UPGRADE_WEBSOCKET;
		  break;

		case HEADER_TRANSFER_ENCODING:
		  if (!strcasecmp ("chunked", buf))
		    {
		      chunked = 1;
		      no_cont = 0;
		    }
		  break;

		case HEADER_CONTENT_LENGTH:
		  cont = ATOL (buf);
		  /*
		   * treat RPC_OUT_DATA like reply without
		   * content-length
		   */
		  if (is_rpc == 0)
		    {
		      if (cont >= 0x20000L && cont <= 0x80000000L)
			cont = -1L;
		      else
			is_rpc = -1;
		    }
		  break;

		case HEADER_LOCATION:
		  if (v_host[0]
		      && need_rewrite (lstn->rewr_loc, buf, loc_path, v_host,
				       lstn, cur_backend))
		    {
		      snprintf (buf, sizeof (buf), "Location: %s://%s/%s",
				(ssl == NULL ? "http" : "https"), v_host,
				loc_path);
		      free (headers[n]);
		      if ((headers[n] = strdup (buf)) == NULL)
			{
			  logmsg (LOG_WARNING,
				  "(%"PRItid") rewrite Location - out of memory: %s",
				  POUND_TID (), strerror (errno));
			  free_headers (headers);
			  clean_all ();
			  return;
			}
		    }
		  break;

		case HEADER_CONTLOCATION:
		  if (v_host[0]
		      && need_rewrite (lstn->rewr_loc, buf, loc_path, v_host,
				       lstn, cur_backend))
		    {
		      snprintf (buf, sizeof (buf),
				"Content-location: %s://%s/%s",
				(ssl == NULL ? "http" : "https"), v_host,
				loc_path);
		      free (headers[n]);
		      if ((headers[n] = strdup (buf)) == NULL)
			{
			  logmsg (LOG_WARNING,
				  "(%"PRItid") rewrite Content-location - out of memory: %s",
				  POUND_TID (), strerror (errno));
			  free_headers (headers);
			  clean_all ();
			  return;
			}
		    }
		  break;
		}
	    }

	  /*
	   * possibly record session information (only for
	   * cookies/header)
	   */
	  upd_session (svc, &headers[1], cur_backend);

	  /*
	   * send the response
	   */
	  if (!skip)
	    for (n = 0; n < MAXHEADERS && headers[n]; n++)
	      {
		if (BIO_printf (cl, "%s\r\n", headers[n]) <= 0)
		  {
		    if (errno)
		      {
			logmsg (LOG_NOTICE, "(%"PRItid") error write to %s: %s",
				POUND_TID (),
				addr2str (caddr, sizeof (caddr), &from_host, 1),
				strerror (errno));
		      }
		    free_headers (headers);
		    clean_all ();
		    return;
		  }
	      }
	  free_headers (headers);

	  /*
	   * final CRLF
	   */
	  if (!skip)
	    BIO_puts (cl, "\r\n");
	  if (BIO_flush (cl) != 1)
	    {
	      if (errno)
		{
		  logmsg (LOG_NOTICE, "(%"PRItid") error flush headers to %s: %s",
			  POUND_TID (),
			  addr2str (caddr, sizeof (caddr), &from_host, 1),
			  strerror (errno));
		}
	      clean_all ();
	      return;
	    }

	  if (!no_cont)
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
		  if (copy_chunks (be, cl, &res_bytes, skip, L0))
		    {
		      /*
		       * copy_chunks() has its own error messages
		       */
		      clean_all ();
		      return;
		    }
		}
	      else if (cont >= L0)
		{
		  /*
		   * may have had Content-length, so do raw reads/writes
		   * for the length
		   */
		  if (copy_bin (be, cl, cont, &res_bytes, skip))
		    {
		      if (errno)
			logmsg (LOG_NOTICE,
				"(%"PRItid") error copy server cont: %s",
				POUND_TID (), strerror (errno));
		      clean_all ();
		      return;
		    }
		}
	      else if (!skip)
		{
		  if (is_readable (be, cur_backend->to))
		    {
		      char one;
		      BIO *be_unbuf;
		      /*
		       * old-style response - content until EOF
		       * also implies the client may not use HTTP/1.1
		       */
		      cl_11 = be_11 = 0;

		      /*
		       * first read whatever is already in the input buffer
		       */
		      while (BIO_pending (be))
			{
			  if (BIO_read (be, &one, 1) != 1)
			    {
			      logmsg (LOG_NOTICE,
				      "(%"PRItid") error read response pending: %s",
				      POUND_TID (), strerror (errno));
			      clean_all ();
			      return;
			    }
			  if (BIO_write (cl, &one, 1) != 1)
			    {
			      if (errno)
				logmsg (LOG_NOTICE,
					"(%"PRItid") error write response pending: %s",
					POUND_TID (), strerror (errno));
			      clean_all ();
			      return;
			    }
			  res_bytes++;
			}
		      BIO_flush (cl);

		      /*
		       * find the socket BIO in the chain
		       */
		      if ((be_unbuf =
			   BIO_find_type (be, cur_backend->ctx ? BIO_TYPE_SSL : BIO_TYPE_SOCKET)) == NULL)
			{
			  logmsg (LOG_WARNING,
				  "(%"PRItid") error get unbuffered: %s",
				  POUND_TID (), strerror (errno));
			  clean_all ();
			  return;
			}

		      /*
		       * copy till EOF
		       */
		      while ((res = BIO_read (be_unbuf, buf, sizeof (buf))) > 0)
			{
			  if (BIO_write (cl, buf, res) != res)
			    {
			      if (errno)
				logmsg (LOG_NOTICE,
					"(%"PRItid") error copy response body: %s",
					POUND_TID (), strerror (errno));
			      clean_all ();
			      return;
			    }
			  else
			    {
			      res_bytes += res;
			      BIO_flush (cl);
			    }
			}
		    }
		}
	      if (BIO_flush (cl) != 1)
		{
		  /*
		   * client closes RPC_OUT_DATA connection - no error
		   */
		  if (is_rpc == 0 && res_bytes > 0L)
		    break;
		  if (errno)
		    {
		      logmsg (LOG_NOTICE, "(%"PRItid") error final flush to %s: %s",
			      POUND_TID (),
			      addr2str (caddr, sizeof (caddr), &from_host, 1),
			      strerror (errno));
		    }
		  clean_all ();
		  return;
		}
	    }
	  else if (is_ws == WSS_COMPLETE)
	    {
	      /*
	       * special mode for Websockets - content until EOF
	       */
	      char one;
	      BIO *cl_unbuf;
	      BIO *be_unbuf;
	      struct pollfd p[2];

	      cl_11 = be_11 = 0;

	      memset (p, 0, sizeof (p));
	      BIO_get_fd (cl, &p[0].fd);
	      p[0].events = POLLIN | POLLPRI;
	      BIO_get_fd (be, &p[1].fd);
	      p[1].events = POLLIN | POLLPRI;

	      while (BIO_pending (cl) || BIO_pending (be)
		     || poll (p, 2, cur_backend->ws_to * 1000) > 0)
		{

		  /*
		   * first read whatever is already in the input buffer
		   */
		  while (BIO_pending (cl))
		    {
		      if (BIO_read (cl, &one, 1) != 1)
			{
			  logmsg (LOG_NOTICE,
				  "(%"PRItid") error read ws request pending: %s",
				  POUND_TID (), strerror (errno));
			  clean_all ();
			  return;
			}
		      if (BIO_write (be, &one, 1) != 1)
			{
			  if (errno)
			    logmsg (LOG_NOTICE,
				    "(%"PRItid") error write ws request pending: %s",
				    POUND_TID (), strerror (errno));
			  clean_all ();
			  return;
			}
		    }
		  BIO_flush (be);

		  while (BIO_pending (be))
		    {
		      if (BIO_read (be, &one, 1) != 1)
			{
			  logmsg (LOG_NOTICE,
				  "(%"PRItid") error read ws response pending: %s",
				  POUND_TID (), strerror (errno));
			  clean_all ();
			  return;
			}
		      if (BIO_write (cl, &one, 1) != 1)
			{
			  if (errno)
			    logmsg (LOG_NOTICE,
				    "(%"PRItid") error write ws response pending: %s",
				    POUND_TID (), strerror (errno));
			  clean_all ();
			  return;
			}
		      res_bytes++;
		    }
		  BIO_flush (cl);

		  /*
		   * find the socket BIO in the chain
		   */
		  if ((cl_unbuf =
		       BIO_find_type (cl,
				      SLIST_EMPTY (&lstn->ctx_head)
				       ? BIO_TYPE_SOCKET : BIO_TYPE_SSL)) == NULL)
		    {
		      logmsg (LOG_WARNING, "(%"PRItid") error get unbuffered: %s",
			      POUND_TID (), strerror (errno));
		      clean_all ();
		      return;
		    }
		  if ((be_unbuf = BIO_find_type (be, cur_backend->ctx ? BIO_TYPE_SSL : BIO_TYPE_SOCKET)) == NULL)
		    {
		      logmsg (LOG_WARNING, "(%"PRItid") error get unbuffered: %s",
			      POUND_TID (), strerror (errno));
		      clean_all ();
		      return;
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
		      if (BIO_write (be, buf, res) != res)
			{
			  if (errno)
			    logmsg (LOG_NOTICE,
				    "(%"PRItid") error copy ws request body: %s",
				    POUND_TID (), strerror (errno));
			  clean_all ();
			  return;
			}
		      else
			{
			  BIO_flush (be);
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
		      if (BIO_write (cl, buf, res) != res)
			{
			  if (errno)
			    logmsg (LOG_NOTICE,
				    "(%"PRItid") error copy ws response body: %s",
				    POUND_TID (), strerror (errno));
			  clean_all ();
			  return;
			}
		      else
			{
			  res_bytes += res;
			  BIO_flush (cl);
			}
		      p[1].revents = 0;
		    }
		}
	    }
	}

      http_log (&from_host, &start_req,
		request, v_host,
		u_agent, referer, u_name, lstn, cur_backend,
		strtol (response+9, NULL, 10), response, res_bytes);

      if (!be_11)
	{
	  BIO_reset (be);
	  BIO_free_all (be);
	  be = NULL;
	}

      /*
       * Stop processing if:
       *  - client is not HTTP/1.1
       *      or
       *  - we had a "Connection: closed" header
       *      or
       *  - this is an SSL connection and we had a NoHTTPS11 directive
       */
      if (!cl_11 || conn_closed || force_10)
	break;
    }

  /*
   * This may help with some versions of IE with a broken channel shutdown
   */
  if (ssl != NULL)
    SSL_set_shutdown (ssl, SSL_SENT_SHUTDOWN | SSL_RECEIVED_SHUTDOWN);

  clean_all ();
  return;
}

void *
thr_http (void *dummy)
{
  THR_ARG *arg;

  while ((arg = get_thr_arg ()) != NULL)
    {
      do_http (arg);
      active_threads_decr ();
    }
  logmsg (LOG_NOTICE, "thread %"PRItid" terminating on idle timeout",
	  POUND_TID ());
  return NULL;
}
