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
#include "json.h"
#include <assert.h>

char *conf_name = POUND_CONF;
char *socket_name;
int json_option;
int indent_option;
int verbose_option;
char *tmpl_path;
char *tmpl_file = "poundctl.tmpl";
char *tmpl_name = "default";

struct keyword
{
  char const *name;
  size_t len;
  int flag;
};

#define S(a) a, sizeof (a) - 1
static struct keyword inline_keyword[] = {
  { S("control"), 1 },
  { NULL }
};
static struct keyword block_keyword[] = {
  { S("socket"), 1 },
  { S("end"), 0 },
  { NULL }
};

static struct keyword *keywords[] = {
  inline_keyword,
  block_keyword
};

static int
find_keyword (int kwtab, char const *arg, size_t len)
{
  struct keyword *kw;
  for (kw = keywords[kwtab]; kw->name; kw++)
    if (strncasecmp (kw->name, arg, len) == 0)
      return kw->flag;
  return -1;
}

/*
 * A temporary solution to obtain socket name from the pound.cfg file.
 * FIXME: Use the parser from config.c for that.
 */
char *
get_socket_name (void)
{
  FILE *fp;
  char  buf[MAXBUF], *name = NULL;
  int line = 0;
  int kwtab = 0;

  if (verbose_option)
    errormsg (0, 0, "scanning file %s", conf_name);
  fp = fopen (conf_name, "r");
  if (!fp)
    {
      if (errno != ENOENT || verbose_option)
	errormsg (0, errno, "can't open %s", conf_name);
      return NULL;
    }
  while (fgets (buf, sizeof (buf), fp))
    {
      size_t len;
      char *p;

      ++line;
      len = strlen (buf);
      if (len == 0)
	continue;
      if (buf[len-1] != '\n')
	{
	  errormsg (0, 0, "%s:%d: line too long", conf_name, line);
	  break;
	}
      while (len > 0 && isspace (buf[len-1]))
	--len;
      buf[len] = 0;

      for (p = buf; *p && isspace (*p); p++)
	;

      if (*p == 0 || *p == '#')
	continue;

      len = strcspn (p, " \t");
      switch (find_keyword (kwtab, p, len))
	{
	case 0:
	  kwtab = 0;
	  break;

	case 1:
	  for (p += len; *p && isspace (*p); p++)
	    ;
	  if (*p == '"')
	    {
	      char *q;

	      q = name = ++p;
	      while (*p != '"')
		{
		  if (*p == 0)
		    {
		      errormsg (0, 0, "%s:%d:%ld missing closing double-quote",
				conf_name, line, p - buf + 1);
		      name = NULL;
		      goto end;
		    }

		  if (*p == '\\')
		    {
		      if (p[1] == '\\' || p[1] == '\"')
			{
			  p++;
			}
		      else
			{
			  errormsg (0, 0, "%s:%d:%ld unrecognized escape character",
				    conf_name, line, p - buf + 1);
			}
		    }
		  *q++ = *p++;
		}
	      *q = 0;
	    }
	  else if (*p == 0 && kwtab == 0)
	    {
	      kwtab = 1;
	      continue;
	    }
	  else
	    {
	      errormsg (0, 0, "%s:%d:%ld: expected quoted string",
			conf_name, line, p - buf + 1);
	    }
	  goto end;

	default:
	  continue;
	}
    }
 end:
  if (ferror (fp))
    {
      errormsg (0, 0, "%s: %s", conf_name, strerror (errno));
    }
  fclose (fp);

  if (name)
    socket_name = xstrdup (name);

  return socket_name;
}

BIO *
open_socket (void)
{
  struct sockaddr_un ctrl;
  int fd;
  BIO *bio;

  if (verbose_option)
    errormsg (0, 0, "connecting to %s", socket_name);

  if (strlen (socket_name) > sizeof (ctrl.sun_path))
    {
      errormsg (1, 0, "socket name too long");
    }

  ctrl.sun_family = AF_UNIX;
  strncpy (ctrl.sun_path, socket_name, sizeof (ctrl.sun_path));
  if ((fd = socket (PF_UNIX, SOCK_STREAM, 0)) < 0)
    {
      errormsg (1, errno, "socket");
    }
  if (connect (fd, (struct sockaddr *) &ctrl, sizeof (ctrl)) < 0)
    {
      errormsg (1, errno, "connect");
      exit (1);
    }

  if ((bio = BIO_new_fd (fd, BIO_CLOSE)) == NULL)
    {
      errormsg (1, 0, "BIO_new_fd failed");
    }

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
  if (!isspace (*p))
    goto err;

  while (isspace (*p))
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
  (strncasecmp (s, h_##n, l_##n) == 0 && s[l_##n] == ':')
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
	  conn_close = strcasecmp (p, "close") == 0;
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

int
command_list (BIO *bio, int argc, char **argv)
{
  char *uri = "/";
  struct json_value *val;

  if (argc == 1)
    uri = argv[0];
  else if (argc > 1)
    {
      errormsg (1, 0, "too many arguments");
    }
  BIO_printf (bio, "GET /listener%s HTTP/1.1\r\n"
		   "Host: localhost\r\n\r\n",
	      uri);
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
  BIO_printf (bio, "%s /listener%s HTTP/1.1\r\n"
		   "Host: localhost\r\n\r\n",
	      verb, uri);
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
  BIO_printf (bio, "DELETE /session%s?key=%s HTTP/1.1\r\n"
		   "Host: localhost\r\n\r\n",
	      uri, key);
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
  BIO_printf (bio, "PUT /session%s?key=%s HTTP/1.1\r\n"
		   "Host: localhost\r\n\r\n",
	      uri, key);
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
  "   -f FILE           location of pound configuration file",
  "   -i N              indentation level for JSON output",
  "   -j                JSON output format",
  "   -s SOCKET         sets control socket pathname",
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
  { "Configuration file",  STRING_CONSTANT, { .s_const = POUND_CONF } },
  { "Template search path",STRING_CONSTANT, { .s_const = POUND_TMPL_PATH } },
  { NULL }
};

int
main (int argc, char **argv)
{
  int c;
  BIO *bio;
  COMMAND command;

  set_progname (argv[0]);
  json_memabrt = xnomem;

  while ((c = getopt (argc, argv, "f:i:jhs:T:t:vV")) != EOF)
    {
      switch (c)
	{
	case 'f':
	  conf_name = optarg;
	  break;

	case 's':
	  socket_name = optarg;
	  break;

	case 'h':
	  usage (0);

	case 'i':
	  indent_option = atoi (optarg);
	  break;

	case 'j':
	  json_option = 1;
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

  if (!socket_name && get_socket_name () == NULL)
    {
      errormsg (1, 0, "can't determine control socket name; use the -s option");
    }
  if (verbose_option)
    errormsg (0, 0, "info: using socket %s", socket_name);

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
  bio = open_socket ();

  return command (bio, argc, argv);
}
