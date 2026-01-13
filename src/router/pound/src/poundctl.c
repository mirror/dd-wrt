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
#include "json.h"
#include <assert.h>

typedef struct
{
  int tls;
  char *path;
  char *host;
  char *user;
  char *pass;
  socklen_t addrlen;
  union
  {
    struct sockaddr sa;
    struct sockaddr_in in4;
    struct sockaddr_in6 in6;
    struct sockaddr_un un;
  } addr;
} URL;

typedef struct server_defn
{
  char *name;
  char *url;
  char *ca_file;
  char *ca_path;
  char *cert;
  int verify;
  struct locus_range locus;
} SERVER;

char *conf_name = POUND_CONF;
int json_option;
int indent_option;
int verbose_option;
char *tmpl_path;
char *tmpl_file = "poundctl.tmpl";
char *tmpl_name = "default";

SERVER default_server_defn = {
  .verify = 1
};
SERVER *server = &default_server_defn;
URL *url;

#define HT_TYPE SERVER
#define HT_NO_HASH_FREE
#define HT_NO_DELETE
#define HT_NO_FOREACH
#define HT_NO_FOREACH_SAFE
#include "ht.h"

SERVER_HASH *server_hash;

static CFGPARSER_TABLE server_parsetab[] = {
  {
    .name = "End",
    .parser = cfg_parse_end
  },
  {
    .name = "URL",
    .parser = cfg_assign_string,
    .off = offsetof (struct server_defn, url)
  },
  {
    .name = "CAFile",
    .parser = cfg_assign_string,
    .off = offsetof (struct server_defn, ca_file)
  },
  {
    .name = "CAPath",
    .parser = cfg_assign_string,
    .off = offsetof (struct server_defn, ca_path)
  },
  {
    .name = "ClientCert",
    .parser = cfg_assign_string,
    .off = offsetof (struct server_defn, cert)
  },
  {
    .name = "Verify",
    .parser = cfg_assign_bool,
    .off = offsetof (struct server_defn, verify)
  },
  { NULL }
};

static int
parse_server (void *call_data, void *section_data)
{
  struct token *tok;
  SERVER *srv, *old;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;
  XZALLOC (srv);
  srv->name = xstrdup (tok->str);
  if ((old = SERVER_INSERT (server_hash, srv)) != NULL)
    {
      conf_error ("redefinition of %s", srv->name);
      conf_error_at_locus_range (&old->locus, "previously defined here");
      return CFGPARSER_FAIL;
    }
  return cfgparser_loop (server_parsetab, srv, NULL, DEPREC_OK, &srv->locus);
}

static CFGPARSER_TABLE top_level_parsetab[] = {
  {
    .type = KWT_TOPLEVEL,
  },
  {
    .name = "URL",
    .parser = cfg_assign_string,
    .data = &default_server_defn.url
  },
  {
    .name = "CAFile",
    .parser = cfg_assign_string,
    .data = &default_server_defn.ca_file
  },
  {
    .name = "CAPath",
    .parser = cfg_assign_string,
    .data = &default_server_defn.ca_path
  },
  {
    .name = "ClientCert",
    .parser = cfg_assign_string,
    .data = &default_server_defn.cert
  },
  {
    .name = "Verify",
    .parser = cfg_assign_bool,
    .data = &default_server_defn.verify
  },
  {
    .name = "TemplateFile",
    .parser = cfg_assign_string,
    .data = &tmpl_file,
  },
  {
    .name = "TemplatePath",
    .parser = cfg_assign_string,
    .data = &tmpl_path
  },
  {
    .name = "TemplateName",
    .parser = cfg_assign_string,
    .data = &tmpl_name
  },
  {
    .name = "Server",
    .parser = parse_server
  },
  { NULL }
};

#define DOT_POUNDCTL_NAME ".poundctl"

static void
read_config (void)
{
  char *poundctl_conf;
  char *homedir = getenv ("HOME");
  if (!homedir)
    {
      struct passwd *pwd = getpwuid (getuid ());
      if (!pwd)
	errormsg (1, errno, "can't get passwd entry for uid %ld",
		  (long) getuid ());
      homedir = pwd->pw_dir;
    }

  server_hash = SERVER_HASH_NEW ();

  if ((poundctl_conf = getenv ("POUNDCTL_CONF")) != NULL)
    {
      if (poundctl_conf[0])
	{
	  if (cfgparser_parse (poundctl_conf, homedir, top_level_parsetab, NULL,
			       DEPREC_OK, 0))
	    exit (1);
	}
    }
  else
    {
      char *buf = xmalloc (strlen (homedir) + 1 + sizeof (DOT_POUNDCTL_NAME));
      strcat (strcat (strcpy (buf, homedir), "/"), DOT_POUNDCTL_NAME);
      if (access (buf, F_OK) == 0)
	{
	  if (cfgparser_parse (".poundctl", homedir, top_level_parsetab, NULL,
			       DEPREC_OK, 0))
	    exit (1);
	}
      free (buf);
    }
}

static int skip_section (void *call_data, void *section_data);
static int skip_acl_def (void *call_data, void *section_data);
static int skip_acl_ref (void *call_data, void *section_data);
static int skip_directive (void *call_data, void *section_data);


static CFGPARSER_TABLE section_parsetab[] = {
  {
    .name = "End",
    .parser = cfg_parse_end
  },
  {
    .name = "Service",
    .parser = skip_section
  },
  {
    .name = "Backend",
    .parser = skip_section
  },
  {
    .name = "Emergency",
    .parser = skip_section
  },
  {
    .name = "Session",
    .parser = skip_section
  },
  {
    .name = "Match",
    .parser = skip_section
  },
  {
    .name = "Rewrite",
    .parser = skip_section
  },
  {
    .name = "ACL",
    .parser = skip_acl_ref,
  },
  {
    .name = "TrustedIP",
    .parser = skip_acl_ref
  },
  {
    .name = "",
    .type = KWT_WILDCARD,
    .parser = skip_directive
  },
  { NULL }
};

static int
skip_section (void *call_data, void *section_data)
{
  if (skip_directive (call_data, section_data) == CFGPARSER_FAIL)
    return CFGPARSER_FAIL;
  return cfgparser_loop (section_parsetab, call_data, section_data,
			 DEPREC_OK, NULL);
}

static int
skip_to_end (void)
{
  struct token *tok;
  while ((tok = gettkn_any ()) != NULL)
    {
      if (tok->type == T_IDENT && c_strcasecmp (tok->str, "end") == 0)
	return CFGPARSER_OK;
      while ((tok = gettkn_any ()) != NULL && tok->type != '\n')
	;
    }
  conf_error ("%s", "unexpected end of file");
  return CFGPARSER_FAIL;
}

static int
skip_acl_def (void *call_data, void *section_data)
{
  if (skip_directive (call_data, section_data) == CFGPARSER_FAIL)
    return CFGPARSER_FAIL;
  return skip_to_end ();
}

static int
skip_acl_ref (void *call_data, void *section_data)
{
  struct token *tok;
  if ((tok = gettkn_any ()) == NULL)
    return CFGPARSER_FAIL;
  if (tok->type == '\n')
    return skip_to_end ();
  return skip_directive (call_data, section_data);
}

static int
skip_stringlist (void *call_data, void *section_data)
{
  return skip_to_end ();
}

static int
skip_directive (void *call_data, void *section_data)
{
  struct token *tok;
  while ((tok = gettkn_any ()) != NULL)
    {
      if (tok->type == '\n')
	return CFGPARSER_OK_NONL;
    }
  return CFGPARSER_FAIL;
}

static CFGPARSER_TABLE resolver_parsetab[] = {
  {
    .name = "End",
    .parser = cfg_parse_end
  },
  {
    .name = "ConfigText",
    .parser = skip_stringlist
  },
  {
    .name = "",
    .type = KWT_WILDCARD,
    .parser = skip_directive
  },
  { NULL }
};

static int
skip_resolver (void *call_data, void *section_data)
{
  return cfgparser_loop (resolver_parsetab, call_data, section_data,
			 DEPREC_OK, NULL);
}

static CFGPARSER_TABLE control_parsetab[] = {
  {
    .name = "End",
    .parser = cfg_parse_end
  },
  {
    .name = "Socket",
    .parser = cfg_assign_string
  },
  {
    .name = "",
    .type = KWT_WILDCARD,
    .parser = skip_directive
  },
  { NULL }
};

static int
parse_control (void *call_data, void *section_data)
{
  struct token *tok;
  if ((tok = gettkn_any ()) == NULL)
    return CFGPARSER_FAIL;
  switch (tok->type)
    {
    case '\n':
      return cfgparser_loop (control_parsetab, call_data, section_data,
			     DEPREC_OK, NULL);
    case T_STRING:
      *(char **) call_data = xstrdup (tok->str);
      break;
    default:
      conf_error ("expected string or newline, but found %s",
		  token_type_str (tok->type));
      return CFGPARSER_FAIL;
    }
  return CFGPARSER_OK;
}

static CFGPARSER_TABLE pound_top_level_parsetab[] = {
  {
    .type = KWT_TOPLEVEL
  },
  {
    .name = "IncludeDir",
    .parser = cfg_parse_includedir
  },
  {
    .name = "Control",
    .parser = parse_control,
  },
  {
    .name = "ListenHTTP",
    .parser = skip_section
  },
  {
    .name = "ListenHTTPS",
    .parser = skip_section
  },
  {
    .name = "Service",
    .parser = skip_section
  },
  {
    .name = "ACL",
    .parser = skip_acl_def,
  },
  {
    .name = "TrustedIP",
    .parser = skip_acl_def,
  },
  {
    .name = "CombineHeaders",
    .parser = skip_stringlist
  },
  {
    .name = "Resolver",
    .parser = skip_resolver
  },
  {
    .name = "",
    .type = KWT_TABREF,
    .ref = cfg_global_parsetab
  },
  {
    .name = "",
    .type = KWT_WILDCARD,
    .parser = skip_directive
  },
  { NULL }
};

char *
scan_pound_cfg (void)
{
  char *control_socket = NULL;
  if (access (conf_name, F_OK) == 0)
    {
      if (verbose_option)
	errormsg (0, 0, "scanning file %s", conf_name);
      if (cfgparser_parse (conf_name, NULL, pound_top_level_parsetab,
			   &control_socket, DEPREC_OK, 0))
	return NULL;
    }
  return control_socket;
}

static void
openssl_errormsg (int code, char const *fmt, ...)
{
  va_list ap;
  unsigned long n = ERR_get_error ();

  fprintf (stderr, "%s: ", progname);
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  fprintf (stderr, ": %s", ERR_error_string (n, NULL));

  if ((n = ERR_get_error ()) != 0)
    {
      do
	{
	  fprintf (stderr, ": %s", ERR_error_string (n, NULL));
	}
      while ((n = ERR_get_error ()) != 0);
    }
  fputc ('\n', stderr);
  if (code)
    exit (code);
}

static void
url_parse_host (char *str, URL *url)
{
  struct addrinfo *addr;
  struct addrinfo hints;
  int n = strcspn (str, "/");
  char *host = xstrndup (str, n);
  char *p;
  int rc, len;

  memset (&hints, 0, sizeof (hints));
  hints.ai_socktype = SOCK_STREAM;

  if (host[0] == '[' && host[n-1] == ']')
    hints.ai_family = AF_INET6;
  else
    hints.ai_family = AF_UNSPEC;

  if ((p = strchr (host, ':')) != NULL)
    *p++ = 0;
  else if (url->tls)
    p = PORT_HTTPS_STR;
  else
    p = PORT_HTTP_STR;

  if ((rc = getaddrinfo (host, p, &hints, &addr)) == 0)
    {
      if (addr->ai_family == AF_INET || addr->ai_family == AF_INET6)
	{
	  url->addrlen = addr->ai_addrlen;
	  memcpy (&url->addr, addr->ai_addr, addr->ai_addrlen);
	}
      else
	errormsg (1, 0, "unexpected address family");
      freeaddrinfo (addr);
    }
  else
    errormsg (1, 0, "can't resolve %s: %s", host, gai_strerror (rc));

  url->host = host;
  str += n;
  len = strlen (str);
  if (!(len > 0 && str[len-1] == '/'))
    {
      url->path = xmalloc (len + 2);
      strcpy (url->path, str);
      url->path[len] = '/';
      url->path[len+1] = 0;
    }
  else
    url->path = xstrdup (str);
}

static void
url_parse_creds (char *str, URL *url)
{
  int n, j;
  char **dst = &url->user;

  switch (str[n = strcspn (str, ":@")])
    {
    case ':':
      j = strcspn (str + n + 1, "@");
      if (str[n + 1 + j] != '@')
	break;
      url->user = xstrndup (str, n);
      str += n + 1;
      n = j;
      dst = &url->pass;
      /* fall through */
    case '@':
      *dst = xstrndup (str, n);
      str += n + 1;
      /* fall through */
    }
  url_parse_host (str, url);
}

static void
url_parse_scheme (char *str, URL *url)
{
  char *p;
  int len;

  if ((p = strchr (str, ':')) != NULL && p[1] == '/' && p[2] == '/')
    {
      if ((len = (p - str)) == 4 && memcmp (str, "http", len) == 0)
	url->tls = 0;
      else if (len == 5 && memcmp (str, "https", len) == 0)
	url->tls = 1;
      else
	errormsg (1, 0, "unsupported URL scheme: %s", str);
      str = p + 3;

      url_parse_creds (str, url);
    }
  else
    {
      url->tls = 0;
      url->path = xstrdup ("/");
      url->host = xstrdup ("localhost");
      url->user = NULL;
      url->pass = NULL;
      url->addrlen = sizeof (struct sockaddr_un);
      url->addr.un.sun_family = AF_UNIX;
      if (strlen (str) > sizeof (url->addr.un.sun_path))
	errormsg (1, 0, "socket name too long");
      strncpy (url->addr.un.sun_path, str, sizeof (url->addr.un.sun_path));
    }
}

static URL *
url_parse (char *str)
{
  URL *url = xzalloc (sizeof (*url));
  url_parse_scheme (str, url);
  return url;
}

BIO *
open_socket (URL *url)
{
  int fd;
  BIO *bio, *bb;

  if ((fd = socket (url->addr.sa.sa_family, SOCK_STREAM, 0)) < 0)
    {
      errormsg (1, errno, "socket");
    }
  if (connect (fd, &url->addr.sa, url->addrlen) < 0)
    {
      errormsg (1, errno, "connect");
      exit (1);
    }

  if ((bio = BIO_new_fd (fd, BIO_CLOSE)) == NULL)
    {
      errormsg (1, 0, "BIO_new_fd failed");
    }
  BIO_set_close (bio,  BIO_CLOSE);

  if (url->tls)
    {
      SSL_CTX *ctx;
      SSL *ssl;
      X509 *x509;
      int verify_result;

      if ((ctx = SSL_CTX_new (SSLv23_client_method ())) == NULL)
	errormsg (1, 0, "SSL_CTX_new");
      SSL_CTX_set_verify (ctx, SSL_VERIFY_NONE, NULL);
      SSL_CTX_set_mode (ctx, SSL_MODE_AUTO_RETRY);

      if (server->verify)
	{
	  if (!SSL_CTX_load_verify_locations (ctx, server->ca_file,
					      server->ca_path))
	    openssl_errormsg (1, "SSL_CTX_load_verify_locations");
	}

      if (server->cert)
	{
	  if (SSL_CTX_use_certificate_chain_file (ctx, server->cert) != 1)
	    openssl_errormsg (1, "SSL_CTX_use_certificate_chain_file");
	  if (SSL_CTX_use_PrivateKey_file (ctx, server->cert, SSL_FILETYPE_PEM) != 1)
	    openssl_errormsg (1, "SSL_CTX_use_PrivateKey_file");
	  if (SSL_CTX_check_private_key (ctx) != 1)
	    openssl_errormsg (1, "SSL_CTX_check_private_key failed");
	}

      if ((ssl = SSL_new (ctx)) == NULL)
	errormsg (1, 0, "SSL_new");
      SSL_set_tlsext_host_name (ssl, url->host);
      SSL_set_bio (ssl, bio, bio);

      if ((bb = BIO_new (BIO_f_ssl ())) == NULL)
	errormsg (1, 0, "BIO_new");
      BIO_set_ssl (bb, ssl, BIO_CLOSE);
      BIO_set_ssl_mode (bb, 1);

      if (BIO_do_handshake (bb) <= 0)
	errormsg (1, 0, "BIO_do_handshake failed: %s",
		  ERR_error_string (ERR_get_error (), NULL));

      if (server->verify &&
	  (x509 = SSL_get_peer_certificate (ssl)) != NULL &&
	  (verify_result = SSL_get_verify_result (ssl)) != X509_V_OK)
	{
	  errormsg (1, 0, "certificate verification failed: %s",
		    X509_verify_cert_error_string (verify_result));
	}

      bio = bb;
    }

  if ((bb = BIO_new (BIO_f_buffer ())) == NULL)
    errormsg (1, 0, "BIO_f_buffer failed");
  BIO_set_buffer_size (bb, MAXBUF);
  BIO_set_close (bb, BIO_CLOSE);
  bio = BIO_push (bb, bio);

  return bio;
}

typedef int OBJID[4];

enum
{
  OI_LAST,
  OI_LISTENER,
  OI_SERVICE,
  OI_BACKEND,
};

static void
oi_getn (char const **uri, int *objid, int idx)
{
  long n;
  char *p;
  errno = 0;
  n = strtol (*uri, &p, 10);
  if (errno || n < 0 || n > INT_MAX)
    {
      errormsg (1, 0, "bad uri: out of range near %s", *uri);
    }
  if (!((idx < OI_BACKEND) ? (*p == 0 || *p == '/') : (*p == 0)))
    {
      errormsg (1, 0, "bad uri: garbage near %s", p);
    }
  objid[idx] = n;
  *uri = p;
}

static void
check_uri (char const *uri, int *objid)
{
  objid[OI_LAST] = -1;

  if (*uri != '/')
    {
      errormsg (1, 0, "bad uri: should start with a /");
    }
  ++uri;

  objid[OI_LAST] = OI_LISTENER;
  if (*uri == '-')
    {
      objid[OI_LISTENER] = -1;
      uri++;
    }
  else
    oi_getn (&uri, objid, OI_LISTENER);

  if (*uri == 0)
    return;
  if (*uri != '/')
    {
      errormsg (1, 0, "bad uri near %s", uri);
    }

  ++uri;

  objid[OI_LAST] = OI_SERVICE;
  oi_getn (&uri, objid, OI_SERVICE);

  if (*uri == 0)
    return;

  ++uri;

  objid[OI_LAST] = OI_BACKEND;
  oi_getn (&uri, objid, OI_BACKEND);
}

static int
xgets (BIO *bio, char *buf, int len)
{
  int n;

  if ((n = BIO_gets (bio, buf, len)) < 0)
    {
      if (n == -1)
	errormsg (0, 0, "error reading response");
      else if (n == -2)
	errormsg (0, 0, "BIO_gets not implemented; please, report");
      exit (1);
    }
  if (n > 0 && buf[n-1] == '\n')
    buf[--n] = 0;
  if (n > 0 && buf[n-1] == '\r')
    buf[--n] = 0;
#if 0
  if (transcript_option)
    printf ("> %s\n", buf);
#endif
  return n;
}

int
read_response_line (BIO *bio, char *ret_status, size_t size, int *ret_version)
{
  char buf[MAXBUF];
  int ver;
  char *p, *end;
  long code;
  size_t len;

  xgets (bio, buf, sizeof (buf));
  if (strncmp (buf, "HTTP/1.", 7))
    goto err;
  ver = buf[7] - '0';
  if (ver != 0 && ver != 1)
    goto err;
  p = buf + 8;
  if (!c_isspace (*p))
    goto err;

  while (c_isspace (*p))
    {
      if (!*p)
	goto err;
      p++;
    }

  code = strtol (p, &end, 10);
  if (code <= 0 || end - p != 3)
    {
  err:
      errormsg (1, 0, "unexpected response: %s", buf);
    }

  p = end + strspn (end, " \n");
  len = strlen (p);
  if (len >= size)
    len = size-1;
  memcpy (ret_status, p, len);
  ret_status[len] = 0;
  *ret_version = ver;
  return code;
}

struct json_value *
read_response (BIO *bio)
{
  int code;
  long content_length = -1;
  char buf[MAXBUF];
  int ver;
  int conn_close = 0;
  char *content_buf;
  int n;
  struct json_value *json;
  char *p;

#define DEFHDR(n, v)					\
  static char h_##n[] = v;				\
  static size_t l_##n = sizeof (h_##n) - 1
#define HDREQ(n, s)				\
  (c_strncasecmp (s, h_##n, l_##n) == 0 && s[l_##n] == ':')
#define HDRVAL(n, s)				\
  (s + l_##n + 1)

  DEFHDR (content_type, "Content-Type");
  DEFHDR (content_length, "Content-Length");
  DEFHDR (connection, "Connection");

  if ((code = read_response_line (bio, buf, sizeof (buf), &ver)) != 200)
    {
      errormsg (1, 0, "%s", buf);
    }

  for (;;)
    {
      char *p;

      n = xgets (bio, buf, sizeof (buf));
      if (n == 0)
	break;
      if (HDREQ (content_length, buf))
	{
	  errno = 0;
	  content_length = strtol (HDRVAL (content_length, buf), &p, 10);
	  if (errno || *p)
	    {
	      errormsg (1, 0, "bad header line: %s", buf);
	    }
	}
      else if (HDREQ (content_type, buf))
	{
	  p = HDRVAL (content_type, buf);
	  p += strspn (p, " \t");
	  if (strcmp (p, "application/json"))
	    {
	      errormsg (1, 0, "unexpected content type: %s", p);
	    }
	}
      else if (HDREQ (connection, buf))
	{
	  p = HDRVAL (connection, buf);
	  p += strspn (p, " \t");
	  conn_close = c_strcasecmp (p, "close") == 0;
	}
    }

  if (content_length != -1)
    {
      content_buf = xmalloc (content_length + 1);
      content_buf[content_length] = 0;
      n = BIO_read (bio, content_buf, content_length);
      if (n < 0)
	{
	  errormsg (1, 0, "read error");
	}
    }
  else if (conn_close || ver == 0)
    {
      struct stringbuf sb;

      xstringbuf_init (&sb);
      while ((n = BIO_read (bio, buf, sizeof (buf))) > 0)
	stringbuf_add (&sb, buf, n);
      content_buf = stringbuf_finish (&sb);
    }
  else
    {
      errormsg (1, 0, "protocol error");
    }

  //  printf ("resp: %s\n", content_buf);
  n = json_parse_string (content_buf, &json, &p);
  if (n != JSON_E_NOERR)
    {
      int len;
      errormsg (0, 0, "error parsing JSON: %s", json_strerror (n));
      len = p - content_buf;
      errormsg (0, 0, "JSON string: %*.*s HERE--> %s",
		len, len, content_buf, p);
      exit (1);
    }

  return json;
}

static void
write_string (void *data, char const *str, size_t len)
{
  fwrite (str, len, 1, (FILE*)data);
}

void
print_json (struct json_value *val, FILE *fp)
{
  struct json_format format = {
    .indent = indent_option,
    .precision = 0,
    .write = write_string,
    .data = fp
  };
  json_value_format (val, &format, 0);
}

static void
send_request (BIO *bio, char const *method, char const *fmt, ...)
{
  va_list ap;

  BIO_printf (bio, "%s %s", method, url->path);
  va_start (ap, fmt);
  BIO_vprintf (bio, fmt, ap);
  va_end (ap);
  BIO_printf (bio, " HTTP/1.1\r\n"
		   "Host: %s\r\n",
	      url->host);
  if (url->pass)
    {
      size_t len = strlen (url->user) + strlen (url->pass) + 1;
      char *buf = xmalloc (len + 1);
      char iobuf[MAXBUF];
      int inlen;
      BIO *bb, *b64;

      strcat (strcat (strcpy (buf, url->user), ":"), url->pass);

      if ((b64 = BIO_new (BIO_f_base64 ())) == NULL)
	errormsg (1, errno, "BIO_f_base64");

      if ((bb = BIO_new (BIO_s_mem ())) == NULL)
	errormsg (1, errno, "BIO_s_mem");

      b64 = BIO_push (b64, bb);
      BIO_write (b64, buf, len);
      BIO_flush (b64);
      inlen = BIO_read (bb, iobuf, sizeof (iobuf));
      if (inlen == -1)
	errormsg (1, errno, "failed to encode credentials");
      BIO_free_all (b64);

      BIO_printf (bio, "Authorization: Basic ");
      BIO_write (bio, iobuf, inlen-1);
      BIO_printf (bio, "\r\n");
    }
  BIO_printf (bio, "\r\n");
  BIO_flush (bio);
}

int
command_gen (char const *path, BIO *bio, int argc, char **argv)
{
  char *uri = "/";
  struct json_value *val;

  if (argc == 1)
    uri = argv[0];
  else if (argc > 1)
    {
      errormsg (1, 0, "too many arguments");
    }
  send_request (bio, "GET", "%s%s", path, uri);
  val = read_response (bio);
  if (json_option)
    print_json (val, stdout);
  else
    {
      TEMPLATE tmpl;

      tmpl = template_lookup (tmpl_name);
      if (!tmpl)
	{
	  errormsg (1, 0, "template %s not defined", tmpl_name);
	}
      template_run (tmpl, val, stdout);
    }
  json_value_free (val);
  return 0;
}

int
command_core (BIO *bio, int argc, char **argv)
{
  return command_gen ("core", bio, argc, argv);
}

int
command_list (BIO *bio, int argc, char **argv)
{
  return command_gen ("listener", bio, argc, argv);
}

int
command_on_off (BIO *bio, int argc, char **argv, char const *verb)
{
  char *uri;
  struct json_value *val;

  if (argc == 0)
    {
      errormsg (1, 0, "required argument missing");
    }
  else if (argc > 1)
    {
      errormsg (1, 0, "too many arguments");
    }
  uri = argv[0];
  send_request (bio, verb, "listener%s", uri);
  val = read_response (bio);
  if (json_option)
    print_json (val, stdout);
  if (val->type != json_bool)
    {
      json_error (val, "unexpected object type");
      return 1;
    }
  if (val->v.b == 0)
    {
      errormsg (1, 0, "command failed");
    }
  json_value_free (val);
  return 0;
}

int
command_disable (BIO *bio, int argc, char **argv)
{
  return command_on_off (bio, argc, argv, "DELETE");
}

int
command_enable (BIO *bio, int argc, char **argv)
{
  return command_on_off (bio, argc, argv, "PUT");
}

int
command_delete_session (BIO *bio, int argc, char **argv)
{
  char *uri, *key;
  struct json_value *val;
  OBJID objid;

  if (argc < 2)
    {
      errormsg (0, 0, "required argument missing");
      return 1;
    }
  else if (argc > 2)
    {
      errormsg (0, 0, "too many arguments");
      return 1;
    }
  uri = argv[0];
  check_uri (uri, objid);
  if (objid[OI_LAST] < OI_SERVICE)
    {
      errormsg (0, 0, "bad uri: service not specified");
      return 1;
    }
  if (objid[OI_LAST] > OI_SERVICE)
    {
      errormsg (0, 0, "bad uri: spurious backend specification");
      return 1;
    }

  key = argv[1];
  send_request (bio, "DELETE", "session%s?key=%s", uri, key);
  val = read_response (bio);
  if (json_option)
    print_json (val, stdout);
  else
    {
      TEMPLATE tmpl;

      tmpl = template_lookup (tmpl_name);
      if (!tmpl)
	{
	  errormsg (1, 0, "template %s not defined", tmpl_name);
	}
      template_run (tmpl, val, stdout);
    }
  json_value_free (val);
  return 0;
}

int
command_add_session (BIO *bio, int argc, char **argv)
{
  char *uri, *key;
  struct json_value *val;
  OBJID objid;

  if (argc < 2)
    {
      errormsg (0, 0, "required argument missing");
      return 1;
    }
  else if (argc > 2)
    {
      errormsg (0, 0, "too many arguments");
      return 1;
    }
  uri = argv[0];
  check_uri (uri, objid);
  if (objid[OI_LAST] != OI_BACKEND)
    {
      errormsg (0, 0, "bad uri: backend not specified");
      return 1;
    }

  key = argv[1];
  send_request (bio, "PUT", "session%s?key=%s", uri, key);
  val = read_response (bio);
  if (json_option)
    print_json (val, stdout);
  else
    {
      TEMPLATE tmpl;

      tmpl = template_lookup (tmpl_name);
      if (!tmpl)
	{
	  errormsg (1, 0, "template %s not defined", tmpl_name);
	}
      template_run (tmpl, val, stdout);
    }
  json_value_free (val);
  return 0;
}


typedef int (*COMMAND) (BIO *, int, char **);

struct dispatch_table
{
  char const *name;
  COMMAND command;
};

static struct dispatch_table dispatch[] = {
  { "list", command_list },
  { "core", command_core },
  { "disable", command_disable },
  { "off", command_disable },
  { "enable", command_enable },
  { "on", command_enable },
  { "delete", command_delete_session },
  { "del", command_delete_session },
  { "add", command_add_session },
  { NULL }
};

COMMAND
find_command (char const *name)
{
  struct dispatch_table *p;

  for (p = dispatch; p->name; p++)
    if (strcmp (p->name, name) == 0)
      return p->command;
  return NULL;
}

static char *usage_text[] = {
  "where L, S, and B stand for the numbers of listener, service and",
  "backend, correspondingly.  A dash in place of L stands for global",
  "scope.  Depending on COMMAND, either B or both S and B may be omitted.",
  "",
  "COMMANDs are:"
  "",
  "   core              show core status",
  "   list [/L/S/B]     list pound status; without argument, shows all",
  "                     listeners and underlying objects.",
  "   enable /L/S/B     enable listener, service, or backend.",
  "   disable /L/S/B    disable listener, service, or backend.",
  "   delete /L/S KEY   delete session with given key.",
  "   add /L/S/B KEY    add session with given key.",
  "",
  "Shortcuts:",
  "   on                same as enable",
  "   off               same as disable",
  "   del               same as delete",
  "",
  "OPTIONS:",
  "   -C FILE           load CA certificates from FILE (or directory)",
  "   -f FILE           location of pound configuration file",
  "   -i N              indentation level for JSON output",
  "   -j                JSON output format",
  "   -K FILE           load client certificate and key from FILE",
  "   -k                disable peer verification",
  "   -S SERVER         connect to SERVER defined in ~/.poundctl file",
  "   -s SOCKET         sets control socket pathname or URL",
  "   -t FILE           read templates from this file",
  "   -T NAME           name of the default template",
  "   -v                verbose output",
  "   -V                print program version, compilation settings, and exit",
  "   -h                show this help output and exit",
  NULL
};

void
usage (int code)
{
  FILE *fp = code ? stderr : stdout;
  int i;

  fprintf (fp, "usage: %s [OPTIONS] COMMAND [/L/S/B] [ARG]\n", progname);
  for (i = 0; usage_text[i]; i++)
    fprintf (fp, "%s\n", usage_text[i]);
  printf ("\n");
  printf ("Report bugs and suggestions to <%s>\n", PACKAGE_BUGREPORT);
#ifdef PACKAGE_URL
  printf ("%s home page: <%s>\n", PACKAGE_NAME, PACKAGE_URL);
#endif
  exit (code);
}

void
xnomem (void)
{
  errormsg (1, 0, "out of memory");
}

static FILE *
find_template_file (char const *name, char **ret_name)
{
  FILE *fp = NULL;
  char *file_name;
  struct stringbuf sb;

  xstringbuf_init (&sb);
  if (name[0] == '/' || strncmp (name, "./", 2) == 0 ||
      strncmp (name, "../", 3) == 0)
    {
      file_name = (char*) name;
      fp = fopen (file_name, "r");
      if (!fp)
	errormsg (1, errno, "can't open template file %s", file_name);
    }
  else
    {
      char *p;

      if (verbose_option)
	errormsg (0, 0, "info: looking for %s in %s", name, tmpl_path);

      p = tmpl_path;
      while (*p)
	{
	  size_t len = strcspn (p, ":");
	  int fd;

	  if (len > 0)
	    {
	      stringbuf_reset (&sb);
	      if (*p == '~')
		{
		  char *home = getenv ("HOME");
		  if (!home)
		    {
		      struct passwd *pw = getpwuid (getuid ());
		      assert (pw != NULL);
		      home = pw->pw_dir;
		    }
		  assert (home != NULL);
		  stringbuf_add_string (&sb, home);
		  if (*++p != '/')
		    stringbuf_add_char (&sb, '/');
		  len--;
		}
	      stringbuf_add (&sb, p, len);

	      file_name = stringbuf_finish (&sb);

	      fd = open (file_name, O_RDONLY);
	      if (fd == -1)
		{
		  if (errno != ENOENT)
		    errormsg (0, errno, "opening %s", file_name);
		}
	      else
		{
		  struct stat st;
		  if (fstat (fd, &st))
		    {
		      errormsg (0, errno, "stat %s", file_name);
		    }
		  else if (S_ISDIR (st.st_mode))
		    {
		      int dirfd = fd;
		      fd = openat (dirfd, name, O_RDONLY);
		      if (fd == -1)
			{
			  if (errno != ENOENT)
			    errormsg (0, errno, "opening %s/%s", file_name, name);
			}
		      else
			{
			  fp = fdopen (fd, "r");
			  if (fp)
			    {
			      sb.len--; /* a hack to remove terminating \0 */
			      stringbuf_add_char (&sb, '/');
			      stringbuf_add_string (&sb, name);
			      file_name = stringbuf_finish (&sb);
			      break;
			    }
			  errormsg (0, errno, "fdopen");
			}
		      close (dirfd);
		    }
		  else if (S_ISREG (st.st_mode))
		    {
		      fp = fdopen (fd, "r");
		      if (fp)
			break;
		      errormsg (0, errno, "fdopen");
		    }
		  close (fd);
		}
	    }

	  p += len;
	  if (*p == ':')
	    p++;
	}

      if (!fp)
	{
	  errormsg (0, 0, "%s file not found in %s", name, tmpl_path);
	  return NULL;
	}
    }

  *ret_name = xstrdup (file_name);

  stringbuf_free (&sb);
  return fp;
}

static size_t
line_start (char *text, size_t n)
{
  while (n > 0 && text[n-1] != '\n')
    n--;
  return n;
}

static size_t
line_no (char *text, size_t n)
{
  size_t i;

  for (i = 1;;i++)
    {
      n = line_start (text, n);
      if (n == 0)
	break;
      n--;
    }
  return i;
}

void
read_template (void)
{
  char *file_name;
  FILE *fp = find_template_file (tmpl_file, &file_name);

  if (fp)
    {
      char *buf;
      size_t size;
      ssize_t n;
      struct stat st;
      TEMPLATE tmpl;
      int rc;

      if (verbose_option)
	errormsg (0, 0, "info: reading template file %s", file_name);

      if (fstat (fileno (fp), &st))
	errormsg (1, errno, "can't stat %s", file_name);

      size = st.st_size;
      buf = xmalloc (size + 1);
      buf[size] = 0;
      n = fread (buf, size, 1, fp);
      if (n == -1)
	errormsg (1, errno, "error reading from %s", file_name);
      else if (n == 0)
	errormsg (1, 0, "short read from %s", file_name);

      rc = template_parse (buf, &tmpl, &size);
      if (rc != TMPL_ERR_OK)
	{
	  size_t ls = line_start (buf, size);
	  size_t ln = line_no (buf, size);

	  errormsg (1, 0, "%s:%zu:%zu: %s", file_name, ln, size - ls,
		    template_strerror (rc));
	}

      free (buf);
      /*
       * Ignore anything except definitions.
       * FIXME: Any warnings?
       */
      template_free (tmpl);
    }
}

static struct string_value poundctl_settings[] = {
  { "Configuration file", STRING_CONSTANT,
    { .s_const = "~/" DOT_POUNDCTL_NAME } },
  { "Pound configuration file",  STRING_CONSTANT, { .s_const = POUND_CONF } },
  { "Template search path",STRING_CONSTANT, { .s_const = POUND_TMPL_PATH } },
  { NULL }
};

int
main (int argc, char **argv)
{
  int c;
  BIO *bio;
  COMMAND command;
  struct stat sb;

  set_progname (argv[0]);
  json_memabrt = xnomem;

  read_config ();
  while ((c = getopt (argc, argv, "C:f:i:jK:khS:s:T:t:vV")) != EOF)
    {
      switch (c)
	{
	case 'C':
	  if (stat (optarg, &sb))
	    errormsg (1, errno, "can't stat %s", optarg);
	  else if (S_ISDIR (sb.st_mode))
	    server->ca_path = optarg;
	  else
	    server->ca_file = optarg;
	  break;

	case 'f':
	  conf_name = optarg;
	  break;

	case 'S':
	  if (server_hash)
	    {
	      SERVER key = { .name = optarg };
	      server = SERVER_RETRIEVE (server_hash, &key);
	      if (!server)
		errormsg (1, 0, "%s: no such server defined in configuration",
			  optarg);
	    }
	  break;

	case 's':
	  server->url = optarg;
	  break;

	case 'h':
	  usage (0);

	case 'i':
	  indent_option = atoi (optarg);
	  break;

	case 'j':
	  json_option = 1;
	  break;

	case 'K':
	  server->cert = optarg;
	  break;

	case 'k':
	  server->verify = 0;
	  break;

	case 'T':
	  tmpl_name = optarg;
	  break;

	case 't':
	  tmpl_file = optarg;
	  break;

	case 'V':
	  print_version (poundctl_settings);
	  exit (0);

	case 'v':
	  verbose_option++;
	  break;

	default:
	  exit (1);
	}
    }

  if ((tmpl_path = getenv ("POUND_TMPL_PATH")) == NULL)
    tmpl_path = POUND_TMPL_PATH;

  if (!server->url && (server->url = scan_pound_cfg ()) == NULL)
    {
      errormsg (1, 0, "can't determine control socket name; use the -s option");
    }
  if (verbose_option)
    errormsg (0, 0, "info: using socket %s", server->url);

  argc -= optind;
  argv += optind;

  if (argc == 0)
    command = command_list;
  else if ((command = find_command (argv[0])) == NULL)
    usage (1);
  else
    {
      argc--;
      argv++;
    }

  read_template ();

  url = url_parse (server->url);

  if (verbose_option)
    errormsg (0, 0, "connecting to %s", server->url);

  bio = open_socket (url);

  return command (bio, argc, argv);
}
