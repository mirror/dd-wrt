/*
 * Pound - the reverse-proxy load-balancer
 * Copyright (C) 2002-2010 Apsis GmbH
 * Copyright (C) 2018-2022 Sergey Poznyakoff
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

char *progname;

void
logmsg (int prio, const char *fmt, ...)
{
  va_list ap;

  fprintf (stderr, "%s: ", progname);
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
  fputc ('\n', stderr);
}

char *conf_name = POUND_CONF;
char *socket_name;
int json_option;
int indent_option;

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
  static char kw_str[] = "control";
  static size_t kw_len = sizeof (kw_str) - 1;

  fp = fopen (conf_name, "r");
  if (!fp)
    {
      if (errno != ENOENT)
	logmsg (LOG_ERR, "can't open %s: %s", conf_name, strerror (errno));
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
	  logmsg (LOG_ERR, "%s:%d: line too long", conf_name, line);
	  break;
	}
      while (len > 0 && isspace (buf[len-1]))
	--len;
      buf[len] = 0;

      for (p = buf; *p && isspace (*p); p++)
	;

      if (*p == '#')
	continue;

      if (strncasecmp (p, kw_str, kw_len) == 0 &&
	  (p[kw_len] == ' ' || p[kw_len] == '\t'))
	{
	  for (p += kw_len; *p && isspace (*p); p++)
	    ;
	  if (*p == '"')
	    {
	      char *q;

	      q = name = ++p;
	      while (*p != '"')
		{
		  if (*p == 0)
		    {
		      logmsg (LOG_ERR, "%s:%d:%ld missing closing double-quote",
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
			  logmsg (LOG_ERR, "%s:%d:%ld unrecognized escape character",
				 conf_name, line, p - buf + 1);
			}
		    }
		  *q++ = *p++;
		}
	      *q = 0;
	      break;
	    }
	  else
	    {
	      logmsg (LOG_ERR, "%s:%d:%ld: expected quoted string",
		     conf_name, line, p - buf + 1);
	      break;
	    }
	}
    }
 end:
  if (ferror (fp))
    {
      logmsg (LOG_ERR, "%s: %s", conf_name, strerror (errno));
    }
  fclose (fp);

  if (name)
    socket_name = xstrdup (name);

  return name;
}

BIO *
open_socket (void)
{
  struct sockaddr_un ctrl;
  int fd;
  BIO *bio;

  if (strlen (socket_name) > sizeof (ctrl.sun_path))
    {
      logmsg (LOG_CRIT, "socket name too long");
      exit (1);
    }

  ctrl.sun_family = AF_UNIX;
  strncpy (ctrl.sun_path, socket_name, sizeof (ctrl.sun_path));
  if ((fd = socket (PF_UNIX, SOCK_STREAM, 0)) < 0)
    {
      logmsg (LOG_CRIT, "socket: %s", strerror (errno));
      exit (1);
    }
  if (connect (fd, (struct sockaddr *) &ctrl, sizeof (ctrl)) < 0)
    {
      logmsg (LOG_CRIT, "connect: %s", strerror (errno));
      exit (1);
    }

  if ((bio = BIO_new_fd (fd, BIO_CLOSE)) == NULL)
    {
      logmsg (LOG_CRIT, "BIO_new_fd failed");
      exit (1);
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
      logmsg (LOG_ERR, "bad uri: out of range near %s", *uri);
      exit (1);
    }
  if (!((idx < OI_BACKEND) ? (*p == 0 || *p == '/') : (*p == 0)))
    {
      logmsg (LOG_ERR, "bad uri: garbage near %s", p);
      exit (1);
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
      logmsg (LOG_ERR, "bad uri: should start with a /");
      exit (1);
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
      logmsg (LOG_ERR, "bad uri near %s", uri);
      exit (1);
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
	logmsg (LOG_ERR, "error reading response");
      else if (n == -2)
	logmsg (LOG_ERR, "BIO_gets not implemented; please, report");
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
read_response_line (BIO *bio, char **ret_status, int *ret_version)
{
  char buf[MAXBUF];
  int ver;
  char *p, *end;
  long code;

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
      logmsg (LOG_ERR, "unexpected response: %s", buf);
      exit (1);
    }

  p = end + strspn (end, " \n");
  *ret_status = p;
  *ret_version = ver;
  return code;
}

struct json_value *
read_response (BIO *bio)
{
  int code;
  char *status;
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

  if ((code = read_response_line (bio, &status, &ver)) != 200)
    {
      logmsg (LOG_ERR, "%s", status);
      exit (1);
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
	      logmsg (LOG_ERR, "bad header line: %s", buf);
	      exit (1);
	    }
	}
      else if (HDREQ (content_type, buf))
	{
	  p = HDRVAL (content_type, buf);
	  p += strspn (p, " \t");
	  if (strcmp (p, "application/json"))
	    {
	      logmsg (LOG_ERR, "unexpected content type: %s", p);
	      exit (1);
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
	  logmsg (LOG_CRIT, "read error");
	  exit (1);
	}
    }
  else if (conn_close || ver == 0)
    {
      struct stringbuf sb;

      stringbuf_init (&sb);
      while ((n = BIO_read (bio, buf, sizeof (buf))) > 0)
	stringbuf_add (&sb, buf, n);
      content_buf = stringbuf_finish (&sb);
    }
  else
    {
      logmsg (LOG_CRIT, "protocol error");
      exit (1);
    }

  //  printf ("resp: %s\n", content_buf);
  n = json_parse_string (content_buf, &json, &p);
  if (n != JSON_E_NOERR)
    {
      int len;
      logmsg (LOG_ERR, "error parsing JSON: %s", json_strerror (n));
      len = p - content_buf;
      logmsg (LOG_ERR, "JSON string: %*.*s HERE--> %s",
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

void
json_error (struct json_value *val, char const *fmt, ...)
{
  va_list ap;

  fprintf (stderr, "%s: ", progname);
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
  fprintf (stderr, " in: ");
  print_json (val, stderr);
  fputc ('\n', stderr);
}

static char *
json_object_get_string (struct json_value *obj, char const *name)
{
  struct json_value *val;
  if (json_object_get (obj, name, &val) == 0)
    {
      if (val->type == json_string)
	return val->v.s;
      else
	{
	  json_error (obj, "bad type of %s: expected string", name);
	  exit (1);
	}
    }
  else
    {
      json_error (obj, "no %s attribute", name);
      exit (1);
    }
}

static int
json_object_get_bool (struct json_value *obj, char const *name)
{
  struct json_value *val;
  if (json_object_get (obj, name, &val) == 0)
    {
      if (val->type == json_bool)
	return val->v.b;
      else
	{
	  json_error (obj, "bad type of %s: expected boolean", name);
	  exit (1);
	}
    }
  else
    {
      json_error (obj, "no %s attribute", name);
      exit (1);
    }
}

static long
json_object_get_integer (struct json_value *obj, char const *name)
{
  struct json_value *val;
  if (json_object_get (obj, name, &val) == 0)
    {
      if (val->type == json_integer || val->type == json_number)
	return val->v.n;
      else
	{
	  json_error (obj, "bad type of %s: expected integer", name);
	  exit (1);
	}
    }
  else
    {
      json_error (obj, "no %s attribute", name);
      exit (1);
    }
}

void
print_backend (struct json_value *obj, int n)
{
  char *type = json_object_get_string (obj, "type");
  printf ("    %3d. %s", n, type);
  if (strcmp (type, "backend") == 0)
    printf (" %s %s %ld %s",
	    json_object_get_string (obj, "protocol"),
	    json_object_get_string (obj, "address"),
	    json_object_get_integer (obj, "priority"),
	    json_object_get_bool (obj, "alive") ? "alive" : "dead");
  else if (strcmp (type, "redirect") == 0)
    printf (" %3ld %s%s",
	    json_object_get_integer (obj, "code"),
	    json_object_get_string (obj, "url"),
	    json_object_get_bool (obj, "redir_req") ?
	    " (redirect request)" : "");
  printf (" %s\n",
	  json_object_get_bool (obj, "enabled") ? "active" : "disabled");
}

void
print_sessions (struct json_value *obj)
{
  struct json_value *a;

  if (json_object_get (obj, "sessions", &a) == 0)
    {
      if (a->type == json_null)
	/* ok */;
      else if (a->type == json_array)
	{
	  size_t i, n = json_array_length (a);
	  for (i = 0; i < n; i++)
	    {
	      struct json_value *sess;
	      json_array_get (a, i, &sess);
	      printf ("    %3ld. Session %s %2ld %s\n", i,
		      json_object_get_string (sess, "key"),
		      json_object_get_integer (sess, "backend"),
		      json_object_get_string (sess, "expire"));
	    }
	}
      else
	{
	  json_error (obj, "bad type of %s: expected array", "sessions");
	  exit (1);
	}
    }
}

void
print_service (struct json_value *obj, int n)
{
  struct json_value *a;
  printf ("  %3d. Service \"%s\" %s (%ld)\n",
	  n,
	  json_object_get_string (obj, "name"),
	  json_object_get_bool (obj, "enabled") ? "active" : "disabled",
	  json_object_get_integer (obj, "tot_pri"));
  if (json_object_get (obj, "backends", &a) == 0)
    {
      if (a->type == json_null)
	/* ok */;
      else if (a->type == json_array)
	{
	  size_t i, n = json_array_length (a);
	  for (i = 0; i < n; i++)
	    {
	      struct json_value *be;
	      json_array_get (a, i, &be);
	      print_backend (be, i);
	    }
	}
      else
	{
	  json_error (obj, "bad type of %s: expected array", "backends");
	  exit (1);
	}
    }

  if (json_object_get (obj, "emergency", &a) == 0 && a->type == json_object)
    {
      printf ("    emergency backend %s %s %ld %s\n",
	      json_object_get_string (a, "protocol"),
	      json_object_get_string (a, "address"),
	      json_object_get_integer (a, "priority"),
	      json_object_get_bool (a, "alive") ? "alive" : "dead");
    }

  printf ("      Session type: %s\n",
	  json_object_get_string (obj, "session_type"));
  print_sessions (obj);
}

void
print_services (struct json_value *obj, char *heading)
{
  struct json_value *a;

  if (json_object_get (obj, "services", &a) == 0)
    {
      if (a->type == json_null)
	/* ok */;
      else if (a->type == json_array && json_array_length (a) > 0)
	{
	  size_t i, n = json_array_length (a);
	  if (heading)
	    printf ("%s\n", heading);
	  for (i = 0; i < n; i++)
	    {
	      struct json_value *svc;
	      json_array_get (a, i, &svc);
	      print_service (svc, i);
	    }
	}
      else
	{
	  json_error (obj, "bad type of %s: expected array", "services");
	  exit (1);
	}
    }
}

void
print_listener (struct json_value *obj, int n)
{
  printf ("%3d. Listener %s %s %s\n",
	  n,
	  json_object_get_string (obj, "protocol"),
	  json_object_get_string (obj, "address"),
	  json_object_get_bool (obj, "active") ? "active" : "disabled");
  print_services (obj, NULL);
}

void
print_listener_array (struct json_value *val)
{
  struct json_value *a;

  if (json_object_get (val, "listeners", &a) == 0)
    {
      if (a->type == json_null)
	printf ("no listeners defined\n");
      else if (a->type == json_array)
	{
	  size_t n = json_array_length (a);
	  if (n == 0)
	    printf ("no listeners defined\n");
	  else
	    {
	      size_t i;
	      for (i = 0; i < n; i++)
		{
		  struct json_value *lst;
		  json_array_get (a, i, &lst);
		  print_listener (lst, i);
		}
	    }
	}
      else
	{
	  json_error (val, "bad type of %s: expected array", "listeners");
	  exit (1);
	}
    }
  else
    {
      json_error (val, "no \"listeners\" attribute");
      exit (1);
    }
}

void
print_list_response (struct json_value *val, char const *arg)
{
  ++arg;

  if (*arg == 0 || *arg == '-')
    {
      print_listener_array (val);
      print_services (val, "Global services");
    }
  else
    {
      long n = strtol (arg, NULL, 10);
      print_listener (val, n);
    }
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
      logmsg (LOG_CRIT, "too many arguments");
      exit (1);
    }
  BIO_printf (bio, "GET /listener%s HTTP/1.1\r\n"
		   "Host: localhost\r\n\r\n",
	      uri);
  val = read_response (bio);
  if (json_option)
    print_json (val, stdout);
  else
    print_list_response (val, uri);
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
      logmsg (LOG_CRIT, "required argument missing");
      return 1;
    }
  else if (argc > 1)
    {
      logmsg (LOG_CRIT, "too many arguments");
      return 1;
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
      logmsg (LOG_CRIT, "command failed");
      return 1;
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
      logmsg (LOG_CRIT, "required argument missing");
      return 1;
    }
  else if (argc > 2)
    {
      logmsg (LOG_CRIT, "too many arguments");
      return 1;
    }
  uri = argv[0];
  check_uri (uri, objid);
  if (objid[OI_LAST] < OI_SERVICE)
    {
      logmsg (LOG_CRIT, "bad uri: service not specified");
      return 1;
    }
  if (objid[OI_LAST] > OI_SERVICE)
    {
      logmsg (LOG_CRIT, "bad uri: spurious backend specification");
      return 1;
    }

  key = argv[1];
  BIO_printf (bio, "DELETE /session%s?key=%s HTTP/1.1\r\n"
		   "Host: localhost\r\n\r\n",
	      uri, key);
  val = read_response (bio);
  if (json_option)
    print_json (val, stdout);
  print_service (val, objid[OI_SERVICE]);
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
      logmsg (LOG_CRIT, "required argument missing");
      return 1;
    }
  else if (argc > 2)
    {
      logmsg (LOG_CRIT, "too many arguments");
      return 1;
    }
  uri = argv[0];
  check_uri (uri, objid);
  if (objid[OI_LAST] != OI_BACKEND)
    {
      logmsg (LOG_CRIT, "bad uri: backend not specified");
      return 1;
    }

  key = argv[1];
  BIO_printf (bio, "PUT /session%s?key=%s HTTP/1.1\r\n"
		   "Host: localhost\r\n\r\n",
	      uri, key);
  val = read_response (bio);
  if (json_option)
    print_json (val, stdout);
  print_service (val, objid[OI_SERVICE]);
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
  "   -c FILE           location of pound configuration file",
  "   -i N              indentation level for JSON output",
  "   -j                JSON output format",
  "   -s SOCKET         sets control socket pathname",
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
  exit (code);
}

int
main (int argc, char **argv)
{
  int c;
  BIO *bio;
  COMMAND command;

  if ((progname = strrchr (argv[0], '/')) == NULL)
    progname = argv[0];
  else
    progname++;

  while ((c = getopt (argc, argv, "c:i:jhs:")) != EOF)
    {
      switch (c)
	{
	case 'c':
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

	default:
	  exit (1);
	}
    }

  if (!socket_name && get_socket_name () == NULL)
    {
      logmsg (LOG_CRIT, "can't determine control socket name; use the -s option");
      exit (1);
    }

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

  bio = open_socket ();

  return command (bio, argc, argv);
}
