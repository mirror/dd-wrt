/*
 * Pound - the reverse-proxy load-balancer
 * Copyright (C) 2023-2024 Sergey Poznyakoff
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

struct http_log_instr;

typedef void (*http_log_printer_fn) (struct stringbuf *sb,
				     struct http_log_instr *instr,
				     POUND_HTTP *phttp);

struct http_log_instr
{
  http_log_printer_fn prt;
  char *arg;
  void *data;
  SLIST_ENTRY (http_log_instr) link;
};

typedef SLIST_HEAD(,http_log_instr) HTTP_LOG_HEAD;

struct http_log_prog
{
  char *name;
  HTTP_LOG_HEAD head;
};

typedef struct http_log_prog *HTTP_LOG_PROG;

struct http_log_parser
{
  struct http_log_prog *prog;
  char const *fmt;
  char const *cur;
  void (*logfn) (void *, int, char const *, int);
  void *logdata;
};

static void
http_log_parser_error (struct http_log_parser *parser, char const *text,
		       char const *err)
{
  parser->logfn (parser->logdata, 1, text, err - parser->fmt);
}

static void
http_log_parser_warning (struct http_log_parser *parser, char const *text,
			 char const *err)
{
  parser->logfn (parser->logdata, 0, text, err - parser->fmt);
}

static void
add_instr (HTTP_LOG_PROG prog, http_log_printer_fn prt, void *data,
	   const char *fmt, size_t fmtsize)
{
    struct http_log_instr *p;
    if (fmt == NULL)
      fmtsize = 0;
    p = xzalloc (sizeof(*p) + (fmtsize ? (fmtsize + 1) : 0));
    p->prt = prt;
    p->data = data;
    if (fmtsize)
      {
	p->arg = (char*) (p + 1);
	memcpy (p->arg, fmt, fmtsize);
	p->arg[fmtsize] = 0;
      }
    else
      p->arg = NULL;
    SLIST_PUSH (&prog->head, p, link);
}

static void
print_str (struct stringbuf *sb, const char *arg)
{
  size_t len;
  if (!arg)
    {
      arg = "-";
      len = 1;
    }
  else
    len = strlen (arg);
  stringbuf_add (sb, arg, len);
}

static void
i_print (struct stringbuf *sb, struct http_log_instr *instr,
	 POUND_HTTP *phttp)
{
  print_str (sb, instr->arg);
}

static char *
anon_ip (char *buf)
{
  if (anonymise)
    {
      char *last;

      if ((last = strrchr (buf, '.')) != NULL
	  || (last = strrchr (buf, ':')) != NULL)
	strcpy (++last, "0");
    }
  return buf;
}

static char *
anon_addr2str (char *buf, size_t size, struct addrinfo const *from_host)
{
  if (from_host->ai_family == AF_UNIX)
    strncpy (buf, "socket", size);
  else
    anon_ip (addr2str (buf, size, from_host, 1));
  return buf;
}

static void
i_remote_ip (struct stringbuf *sb, struct http_log_instr *instr,
		    POUND_HTTP *phttp)
{
  char caddr[MAX_ADDR_BUFSIZE];
  print_str (sb, anon_addr2str (caddr, sizeof (caddr), &phttp->from_host));
}

static int
acl_match_ip (ACL *acl, char const *ipstr)
{
  struct addrinfo hints, *res, *ap;

  memset (&hints, 0, sizeof (hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_flags = AI_NUMERICHOST;

  if (getaddrinfo (ipstr, NULL, &hints, &res) == 0)
    {
      int rc = 0;
      for (ap = res; ap; ap = ap->ai_next)
	{
	  if ((rc = acl_match (acl, ap->ai_addr) == 0))
	    break;
	}
      freeaddrinfo (res);
      return rc;
    }
  return -1;
}

static char *
scan_forwarded_header (char const *hdr, ACL *acl)
{
  char const *end;
  char *buf = NULL;
  size_t bufsize = 0;

  if (!hdr || !acl)
    return NULL;

  end = hdr + strlen (hdr);
  while (end > hdr)
    {
      char const *p;
      size_t len, i, j;

      for (p = end - 1; p > hdr && *p != ','; p--)
	;
      len = end - p;
      if (len + 1 > bufsize)
	{
	  char *newbuf = realloc (buf, len + 1);
	  if (newbuf)
	    {
	      buf = newbuf;
	      bufsize = len + 1;
	    }
	  else
	    {
	      free (buf);
	      return NULL;
	    }
	}

      i = 0;
      j = 0;
      if (*p == ',')
	j++;
      while (j < len && isspace (p[j]))
	j++;
      while (j < len)
	{
	  if (isspace (p[j]))
	    break;
	  buf[i++] = p[j++];
	}
      buf[i] = 0;

      switch (acl_match_ip (acl, buf))
	{
	case 0:
	  return buf;

	case 1:
	  break;

	default:
	  goto end;
	}

      end = p;
    }
 end:
  free (buf);
  return NULL;
}

static char const *
get_forwarded_header_name (POUND_HTTP *phttp)
{
  if (phttp->svc->forwarded_header)
    return phttp->svc->forwarded_header;
  if (phttp->lstn->forwarded_header)
    return phttp->lstn->forwarded_header;
  if (forwarded_header)
    return forwarded_header;
  return DEFAULT_FORWARDED_HEADER;
}

static ACL *
get_trusted_ips (POUND_HTTP *phttp)
{
  if (phttp->svc->trusted_ips)
    return phttp->svc->trusted_ips;
  if (phttp->lstn->trusted_ips)
    return phttp->lstn->trusted_ips;
  if (trusted_ips)
    return trusted_ips;
  return NULL;
}

void
save_forwarded_header (POUND_HTTP *phttp)
{
  struct http_header *hdr;
  char const *hname;
  char const *val;

  free (phttp->orig_forwarded_header);
  phttp->orig_forwarded_header = NULL;
  hname = get_forwarded_header_name (phttp);
  if ((hdr = http_header_list_locate_name (&phttp->request.headers,
					   hname, strlen (hname))) != NULL)
    {
      if (is_combinable_header (hdr))
	{
	  if ((val = http_header_get_value (hdr)) != NULL)
	    phttp->orig_forwarded_header = xstrdup (val);
	}
      else
	{
	  struct stringbuf sb;

	  stringbuf_init_log (&sb);
	  for (;;)
	    {
	      size_t len = hdr->val_end - hdr->val_start;

	      stringbuf_add (&sb, hdr->header + hdr->val_start, len);
	      if ((hdr = http_header_list_next (hdr)) != NULL)
		{
		  if (len > 0)
		    stringbuf_add (&sb, ", ", 2);
		}
	      else
		break;
	    }
	  phttp->orig_forwarded_header = stringbuf_finish (&sb);
	  if (!phttp->orig_forwarded_header)
	    stringbuf_free (&sb);
	}
    }
}

static void
i_forwarded_ip (struct stringbuf *sb, struct http_log_instr *instr,
		POUND_HTTP *phttp)
{
  char *ip;
  char caddr[MAX_ADDR_BUFSIZE];

  if ((ip = scan_forwarded_header (phttp->orig_forwarded_header,
				    get_trusted_ips (phttp))) != NULL)
    {
      print_str (sb, ip);
      free (ip);
    }
  else
    print_str (sb, anon_addr2str (caddr, sizeof (caddr), &phttp->from_host));
}

static void
i_local_ip (struct stringbuf *sb, struct http_log_instr *instr,
		   POUND_HTTP *phttp)
{
  char caddr[MAX_ADDR_BUFSIZE];
  print_str (sb, addr2str (caddr, sizeof (caddr), &phttp->lstn->addr, 1));
}

static void
i_response_size (struct stringbuf *sb, struct http_log_instr *instr,
		 POUND_HTTP *phttp)
{
  stringbuf_printf (sb, "%"PRICLEN, phttp->res_bytes);
}

static void
i_response_size_clf (struct stringbuf *sb, struct http_log_instr *instr,
		     POUND_HTTP *phttp)
{
  if (phttp->res_bytes > 0)
    stringbuf_printf (sb, "%"PRICLEN, phttp->res_bytes);
  else
    stringbuf_add_char (sb, '-');
}

static void
i_protocol (struct stringbuf *sb, struct http_log_instr *instr,
	    POUND_HTTP *phttp)
{
  print_str (sb, phttp->ssl == NULL ? "http" : "https");
}

static void
i_method (struct stringbuf *sb, struct http_log_instr *instr,
	  POUND_HTTP *phttp)
{
  print_str (sb, method_name (phttp->request.method));
}

static void
i_query (struct stringbuf *sb, struct http_log_instr *instr,
	 POUND_HTTP *phttp)
{
  char const *query = NULL;
  http_request_get_query (&phttp->request, &query);
  if (query)
    {
      stringbuf_add_char (sb, '?');
      stringbuf_add_string (sb, query);
    }
}

static void
i_status (struct stringbuf *sb, struct http_log_instr *instr,
	  POUND_HTTP *phttp)
{
  if (instr->arg)
    stringbuf_add_string (sb, http_request_orig_line (&phttp->response));
  else
    stringbuf_printf (sb, "%3d", phttp->response_code);
}

enum
  {
    MILLI = 1000,
    MICRO = 1000000
  };

/*
 * If the format starts with begin: (default) the time is taken at the
 * beginning of the request processing. If it starts with end: it is the
 * time when the log entry gets written, close to the end of the request
 * processing. In addition to the formats supported by strftime(3), the
 * following format tokens are supported:
 *     sec		number of seconds since the Epoch
 *     msec		number of milliseconds since the Epoch
 *     usec		number of microseconds since the Epoch
 *     msec_frac	millisecond fraction
 *     usec_frac	microsecond fraction
 * These tokens can not be combined with each other or strftime(3) formatting
 * in the same format string. You can use multiple %{format}t tokens instead.
 */

static void *
phttp_field_ptr (struct http_log_instr *instr, POUND_HTTP *phttp)
{
  size_t off = (size_t) (intptr_t) instr->data;
  return (char*)phttp + off;
}

static void
i_timedfl (struct stringbuf *sb, struct http_log_instr *instr,
	   POUND_HTTP *phttp)
{
  struct timespec *ts = phttp_field_ptr (instr, phttp);
  struct tm tm;
  stringbuf_strftime (sb, "[%d/%b/%Y:%H:%M:%S %z]",
		      localtime_r (&ts->tv_sec, &tm));
}

static void
i_timefmt (struct stringbuf *sb, struct http_log_instr *instr,
	   POUND_HTTP *phttp)
{
  struct timespec *ts = phttp_field_ptr (instr, phttp);
  struct tm tm;
  stringbuf_strftime (sb, instr->arg, localtime_r (&ts->tv_sec, &tm));
}

static void
i_time_s (struct stringbuf *sb, struct http_log_instr *instr,
	  POUND_HTTP *phttp)
{
  struct timespec *ts = phttp_field_ptr (instr, phttp);
  stringbuf_printf (sb, "%ld", (long) ts->tv_sec);
}

static void
i_time_msec (struct stringbuf *sb, struct http_log_instr *instr,
	     POUND_HTTP *phttp)
{
  struct timespec *ts = phttp_field_ptr (instr, phttp);
  stringbuf_printf (sb, "%.0f",
		    (double) ts->tv_sec * MILLI + ts->tv_nsec / MICRO);
}

static void
i_time_msec_frac (struct stringbuf *sb, struct http_log_instr *instr,
		  POUND_HTTP *phttp)
{
  struct timespec *ts = phttp_field_ptr (instr, phttp);
  stringbuf_printf (sb, "%03ld", ts->tv_nsec / MICRO);
}

static void
i_time_usec (struct stringbuf *sb, struct http_log_instr *instr,
	     POUND_HTTP *phttp)
{
  struct timespec *ts = phttp_field_ptr (instr, phttp);
  stringbuf_printf (sb, "%.0f",
		    (double) ts->tv_sec * MICRO + ts->tv_nsec / MILLI);
}

static void
i_time_usec_frac (struct stringbuf *sb, struct http_log_instr *instr,
		  POUND_HTTP *phttp)
{
  struct timespec *ts = phttp_field_ptr (instr, phttp);
  stringbuf_printf (sb, "%06ld", ts->tv_nsec / MILLI);
}

struct argprt
{
  char const *arg;
  int arglen;
  http_log_printer_fn prt;
};
#define ARG(s) s, sizeof(s)-1

static http_log_printer_fn
argprt_find(struct argprt const *ap, char const *arg, int arglen)
{
  for (; ap->arg; ap++)
    if (ap->arglen == arglen && memcmp(ap->arg, arg, arglen) == 0)
      return ap->prt;
  return NULL;
}

static struct argprt timeprt[] = {
  { ARG("s"), i_time_s },
  { ARG("msec"), i_time_msec },
  { ARG("msec_frac"), i_time_msec_frac },
  { ARG("usec"), i_time_usec },
  { ARG("usec_frac"), i_time_usec_frac },
  { NULL }
};

static int
p_time (struct http_log_parser *parser, char const *arg, int len)
{
  static char beg_pfx[] = "begin:";
  static int beg_pfx_len = sizeof (beg_pfx) - 1;
  static char end_pfx[] = "end:";
  static int end_pfx_len = sizeof (end_pfx) - 1;

  size_t ts_off = offsetof (POUND_HTTP, start_req);
  http_log_printer_fn prt;

  if (arg == NULL)
    {
      prt = i_timedfl;
    }
  else
    {
      if (beg_pfx_len <= len && memcmp (arg, beg_pfx, beg_pfx_len) == 0)
	{
	  arg += beg_pfx_len;
	  len -= beg_pfx_len;
	}
      else if (end_pfx_len <= len && memcmp (arg, end_pfx, end_pfx_len) == 0)
	{
	  ts_off = offsetof (POUND_HTTP, end_req);
	  arg += end_pfx_len;
	  len -= end_pfx_len;
	}

      if (len == 0)
	{
	  prt = i_timedfl;
	  arg = NULL;
	  len = 0;
	}
      else if ((prt = argprt_find (timeprt, arg, len)) == NULL)
	prt = i_timefmt;
      else
	{
	  arg = NULL;
	  len = 0;
	}
    }
  add_instr (parser->prog, prt, (void*) (intptr_t) ts_off, arg, len);
  return 0;
}

static void
i_process_time_ms (struct stringbuf *sb, struct http_log_instr *instr,
		   POUND_HTTP *phttp)
{
  struct timespec diff = timespec_sub (&phttp->end_req, &phttp->start_req);
  stringbuf_printf (sb, "%ld",
		    (unsigned long) diff.tv_sec * MILLI + diff.tv_nsec / MICRO);
}

static void
i_process_time_us (struct stringbuf *sb, struct http_log_instr *instr,
		   POUND_HTTP *phttp)
{
  struct timespec diff = timespec_sub (&phttp->end_req, &phttp->start_req);
  stringbuf_printf (sb, "%ld",
		    (unsigned long) diff.tv_sec * MICRO + diff.tv_nsec / MILLI);
}

static void
i_process_time_s (struct stringbuf *sb, struct http_log_instr *instr,
		  POUND_HTTP *phttp)
{
  struct timespec diff = timespec_sub (&phttp->end_req, &phttp->start_req);
  stringbuf_printf (sb, "%ld", diff.tv_sec);
}

static void
i_process_time_f (struct stringbuf *sb, struct http_log_instr *instr,
		  POUND_HTTP *phttp)
{
  struct timespec diff = timespec_sub (&phttp->end_req, &phttp->start_req);
  stringbuf_printf (sb, "%ld.%03ld", diff.tv_sec, diff.tv_nsec / MICRO);
}

static struct argprt proctimeprt[] = {
  { ARG("s"), i_process_time_s },
  { ARG("f"), i_process_time_f },
  { ARG("ms"), i_process_time_ms },
  { ARG("us"), i_process_time_us },
  { NULL }
};

static int
p_process_time (struct http_log_parser *parser, char const *arg, int len)
{
  http_log_printer_fn prt;

  if (arg == NULL)
    prt = i_process_time_s;
  else if ((prt = argprt_find (proctimeprt, arg, len)) == NULL)
    {
      http_log_parser_error (parser, "bad process time format", arg);
      return -1;
    }
  add_instr (parser->prog, prt, NULL, NULL, 0);
  return 0;
}

static void
i_user_name (struct stringbuf *sb, struct http_log_instr *instr,
	     POUND_HTTP *phttp)
{
  char *user;
  if (http_request_get_basic_auth (&phttp->request, &user, NULL))
    user = NULL;
  print_str (sb, user);
  free (user);
}

static void
i_url (struct stringbuf *sb, struct http_log_instr *instr,
       POUND_HTTP *phttp)
{
  char const *val = NULL;
  http_request_get_path (&phttp->request, &val);
  print_str (sb, val);
}

static void
i_listener_name (struct stringbuf *sb, struct http_log_instr *instr,
		 POUND_HTTP *phttp)
{
  print_str (sb, phttp->lstn->name);
}

static void
i_tid (struct stringbuf *sb, struct http_log_instr *instr,
       POUND_HTTP *phttp)
{
  stringbuf_printf (sb, "%"PRItid, POUND_TID ());
}

static void
i_request (struct stringbuf *sb, struct http_log_instr *instr,
	   POUND_HTTP *phttp)
{
  print_str (sb, http_request_orig_line (&phttp->request));
}

static void
i_backend (struct stringbuf *sb, struct http_log_instr *instr,
	   POUND_HTTP *phttp)
{
  char caddr[MAX_ADDR_BUFSIZE];
  print_str (sb, str_be (caddr, sizeof (caddr), phttp->backend));
}

static char *
be_service_name (BACKEND *be)
{
  switch (be->be_type)
    {
    case BE_BACKEND:
      if (be->service->name)
       return be->service->name;
      break;
    case BE_REDIRECT:
      return "(redirect)";
    case BE_ACME:
      return "(acme)";
    case BE_CONTROL:
      return "(control)";
    case BE_ERROR:
      return "(error)";
    case BE_METRICS:
      return "(metrics)";
    case BE_BACKEND_REF:
      /* shouldn't happen */
      break;
    }
  return "-";
}

static void
i_service (struct stringbuf *sb, struct http_log_instr *instr,
	   POUND_HTTP *phttp)
{
  print_str (sb, be_service_name (phttp->backend));
}

static void
i_listener (struct stringbuf *sb, struct http_log_instr *instr,
	    POUND_HTTP *phttp)
{
  print_str (sb, phttp->lstn->name);
}

static struct argprt nameprt[] = {
  { ARG("backend"), i_backend },
  { ARG("service"), i_service },
  { ARG("listener"), i_listener },
  { NULL }
};

static int
p_objname (struct http_log_parser *parser, char const *arg, int len)
{
  http_log_printer_fn prt;

  if ((prt = argprt_find (nameprt, arg, len)) == NULL)
    {
      http_log_parser_error (parser, "unrecognized Pound object name", arg);
      return -1;
    }
  add_instr (parser->prog, prt, NULL, NULL, 0);
  return 0;
}

static void
i_backend_locus (struct stringbuf *sb, struct http_log_instr *instr,
		 POUND_HTTP *phttp)
{
  print_str (sb, phttp->backend->locus);
}

static void
i_service_locus (struct stringbuf *sb, struct http_log_instr *instr,
		 POUND_HTTP *phttp)
{
  print_str (sb, phttp->svc->locus);
}

static void
i_listener_locus (struct stringbuf *sb, struct http_log_instr *instr,
		 POUND_HTTP *phttp)
{
  print_str (sb, phttp->lstn->locus);
}

static struct argprt locprt[] = {
  { ARG("backend"), i_backend_locus },
  { ARG("service"), i_service_locus },
  { ARG("listener"), i_listener_locus },
  { NULL }
};

static int
p_objloc (struct http_log_parser *parser, char const *arg, int len)
{
  http_log_printer_fn prt;

  if ((prt = argprt_find (locprt, arg, len)) == NULL)
    {
      http_log_parser_error (parser, "unrecognized Pound object name", arg);
      return -1;
    }
  add_instr (parser->prog, prt, NULL, NULL, 0);
  return 0;
}

static void
i_header (struct stringbuf *sb, struct http_log_instr *instr,
	  POUND_HTTP *phttp)
{
  struct http_header *hdr;
  char const *val = NULL;

  if (instr->arg &&
      (hdr = http_header_list_locate_name (&phttp->request.headers, instr->arg,
					   strlen (instr->arg))) != NULL)
    val = http_header_get_value (hdr);
  if (val)
    stringbuf_add_string (sb, val);
}

static void
i_header_clf (struct stringbuf *sb, struct http_log_instr *instr,
	      POUND_HTTP *phttp)
{
  struct http_header *hdr;
  char const *val = NULL;

  if (instr->arg &&
      (hdr = http_header_list_locate_name (&phttp->request.headers,
					   instr->arg,
					   strlen (instr->arg))) != NULL)
    val = http_header_get_value (hdr);
  print_str (sb, val);
}


enum
  {
    SPEC_NO_ARG,
    SPEC_REQ_ARG,
    SPEC_OPT_ARG,
  };

struct http_log_spec
{
  int ch;                   /* Specifier character. */
  http_log_printer_fn prt;  /* Printer function. */
  int arg;                  /* Argument requirement (see constants above). */
  int (*parser) (struct http_log_parser *, char const *, int);
};

static struct http_log_spec http_log_spec[] = {
    /* The percent sign */
    { '%', i_print },
    /* Remote IP-address */
    { 'a', i_forwarded_ip },
    /* Local IP-address */
    { 'A', i_local_ip },
    /* Size of response in bytes. */
    { 'B', i_response_size },
    /* Size of response in bytes in CLF format, i.e. a '-' rather
       than a 0 when no bytes are sent. */
    { 'b', i_response_size_clf },
    /* The time taken to serve the request, in microseconds. */
    { 'D', i_process_time_ms },
    /* Remote hostname - same as %a */
    { 'h', i_remote_ip },
    /* The request protocol. */
    { 'H', i_protocol },
    /* The contents of VARNAME: header line(s) in the request sent to the
       server. */
    { 'i', i_header, SPEC_REQ_ARG },
    /* Same as %i, but in CLF format. */
    { 'I', i_header_clf, SPEC_REQ_ARG },
    /* Object locus & name:
       %{backend}N
       %{service}N
       %{listener}N */
    { 'L', NULL, SPEC_REQ_ARG, p_objloc },
    { 'N', NULL, SPEC_REQ_ARG, p_objname },
    /* The request method. */
    { 'm', i_method },
    /* The canonical port of the server serving the request. */
    // { 'p', i_canon_port },
    /* Thread ID */
    { 'P', i_tid },
    /* The query string (prepended with a ? if a query string exists,
       otherwise an empty string). */
    { 'q', i_query },
    /* First line of request. */
    { 'r', i_request },
    /* Status */
    { 's', i_status, SPEC_OPT_ARG },
    /* %t          Time the request was received (standard english format)
       %{format}t  The time, in the form given by format, which should be in
		   strftime(3) format. (potentially localized) */
    { 't', NULL, SPEC_OPT_ARG, p_time },
    /* %T          The time taken to serve the request, in seconds.
       %{UNIT}T    The time taken to serve the request, in a time unit given
		   by UNIT. Valid units are ms for milliseconds, us for
		   microseconds, s for seconds, and f for seconds with
		   fractional part. Using s gives the same result as %T
		   without any format; using us gives the same result as %D. */
    { 'T', NULL, SPEC_OPT_ARG, p_process_time },
    /* Remote user. */
    { 'u', i_user_name },
    /* The URL path requested, not including any query string. */
    { 'U', i_url },
    /* Listener name */
    { 'v', i_listener_name },
    { 0 }
};

static struct http_log_spec *
find_spec (int c)
{
  struct http_log_spec *p;

  for (p = http_log_spec; p->ch; p++)
    if (p->ch == c)
      return p;
  return NULL;
}

static struct http_log_prog http_log_tab[MAX_HTTP_LOG_FORMATS];
static int http_log_next;

static int
find_log_prog (char const *name, int *alloc)
{
  int i;

  if (alloc)
    *alloc = 0;
  for (i = 0; i < http_log_next; i++)
    if (strcmp (http_log_tab[i].name, name) == 0)
      return i;
  if (!alloc)
    return -1;
  if (http_log_next == MAX_HTTP_LOG_FORMATS)
    return -1;
  *alloc = 1;
  i = http_log_next++;
  memset (&http_log_tab[i], 0, sizeof (http_log_tab[i]));
  http_log_tab[i].name = xstrdup (name);
  return i;
}

int
http_log_format_find (char const *name)
{
  return find_log_prog (name, NULL);
}

int
http_log_format_check (int n)
{
  if (n < 0 || n >= http_log_next)
    return -1;
  return 0;
}

int
http_log_format_compile (char const *name, char const *fmt,
			 void (*logfn) (void *, int, char const *, int),
			 void *logdata)
{
  struct http_log_parser parser;
  char *p;
  size_t len;
  int i;
  int alloc;

  i = find_log_prog (name, &alloc);
  if (i == -1)
    {
      logfn (logdata, 1, "format table full", -1);
      return -1;
    }
  else if (!alloc)
    {
      logfn (logdata, 1, "format already defined", -1);
      return -1;
    }

  parser.prog = http_log_tab + i;
  parser.fmt = fmt;
  parser.cur = fmt;
  parser.logfn = logfn;
  parser.logdata = logdata;

  SLIST_INIT (&parser.prog->head);
  while ((p = strchr (parser.cur, '%')))
    {
      char *arg = NULL;
      size_t arglen;
      struct http_log_spec *tptr;

      len = p - parser.cur;
      if (len)
	add_instr (parser.prog, i_print, NULL, parser.cur, len);
      p++;
      if (*p == '>' && p[1] == 's')
	{
	  arg = "1";
	  arglen = 1;
	  p++;
	}
      else if (*p == '{')
	{
	  char *q = strchr (p + 1, '}');

	  if (!q)
	    {
	      http_log_parser_error (&parser, "missing terminating `}'", p);
	      add_instr (parser.prog, i_print, NULL, p - 1, 2);
	      parser.cur = p + 1;
	      continue;
	    }
	  arglen = q - p - 1;
	  arg = p + 1;
	  p = q + 1;
	}

      tptr = find_spec (*p);
      if (!tptr)
	{
	  http_log_parser_error (&parser, "unknown format char", p);
	  add_instr (parser.prog, i_print, NULL, parser.cur, p - parser.cur + 1);
	}
      else
	{
	  switch (tptr->arg)
	    {
	    case SPEC_NO_ARG:
	      if (arg)
		{
		  http_log_parser_warning (&parser,
					   "format specifier does not "
					   "take arguments",
					   p);
		  arg = NULL;
		}
	      break;

	    case SPEC_OPT_ARG:
	      break;

	    case SPEC_REQ_ARG:
	      if (!arg)
		{
		  http_log_parser_error (&parser,
					 "format specifier requires argument",
					 p);
		  return -1;
		}
	    }
	  if (tptr->ch == '%')
	    {
	      /* Special case */
	      arg = "%";
	      arglen = 1;
	    }
	  if (tptr->parser)
	    {
	      if (tptr->parser (&parser, arg, arglen))
		return -1;
	    }
	  else
	    add_instr (parser.prog, tptr->prt, NULL, arg, arglen);
	}
      parser.cur = p + 1;
    }
  len = strlen (parser.cur);
  if (len)
    add_instr (parser.prog, i_print, NULL, parser.cur, len);
  return i;
}

void
http_log (POUND_HTTP *phttp)
{
  struct stringbuf sb;
  struct http_log_prog *prog;
  struct http_log_instr *ip;
  char *msg;

  if (http_log_format_check (phttp->lstn->log_level))
    return;
  if (STATUS_MASK (phttp->response_code) & phttp->svc->log_suppress_mask)
    return;
  prog = http_log_tab + phttp->lstn->log_level;
  if (SLIST_EMPTY (&prog->head))
    return;

  stringbuf_init_log (&sb);
  SLIST_FOREACH (ip, &prog->head, link)
    {
      ip->prt (&sb, ip, phttp);
    }
  if ((msg = stringbuf_finish (&sb)) == NULL)
    {
      logmsg (LOG_ERR, "error formatting log message");
    }
  else
    {
      logmsg (LOG_INFO, "%s", msg);
    }
  stringbuf_free (&sb);
}
