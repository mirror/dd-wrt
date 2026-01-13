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
#include "resolver.h"
#include <openssl/x509v3.h>
#include <assert.h>
#include <dirent.h>
#include <sys/stat.h>

static void
regcomp_error_at_locus_range (struct locus_range const *loc, GENPAT rx,
			      char const *expr)
{
  size_t off;
  char const *errmsg = genpat_error (rx, &off);

  if (off)
    conf_error_at_locus_range (loc, "%s at byte %zu", errmsg, off);
  else
    conf_error_at_locus_range (loc, "%s", errmsg);
  if (expr)
    conf_error_at_locus_range (loc, "regular expression: %s", expr);
}

static void
openssl_error_at_locus_range (struct locus_range const *loc,
			      char const *filename, char const *msg)
{
  unsigned long n = ERR_get_error ();
  if (filename)
    conf_error_at_locus_range (loc, "%s: %s: %s", filename, msg,
			       ERR_error_string (n, NULL));
  else
    conf_error_at_locus_range (loc, "%s: %s", msg, ERR_error_string (n, NULL));

  if ((n = ERR_get_error ()) != 0)
    {
      do
	{
	  conf_error_at_locus_range (loc, "%s", ERR_error_string (n, NULL));
	}
      while ((n = ERR_get_error ()) != 0);
    }
}

#define conf_regcomp_error(rc, rx, expr)				\
  regcomp_error_at_locus_range (last_token_locus_range (), rx, expr)

#define conf_openssl_error(file, msg)				\
  openssl_error_at_locus_range (last_token_locus_range (), file, msg)

char *
locus_point_str (struct locus_point const *loc)
{
  struct stringbuf sb;
  stringbuf_init_log (&sb);
  stringbuf_format_locus_point (&sb, loc);
  return stringbuf_value (&sb);
}

char *
locus_range_str (struct locus_range const *loc)
{
  struct stringbuf sb;
  stringbuf_init_log (&sb);
  stringbuf_format_locus_range (&sb, loc);
  return stringbuf_value (&sb);
}

struct config_option
{
  char *name;
  int code;
  int has_arg;
};

static char const *
config_option_name (struct config_option const *optab, int code)
{
  for (; optab->name; optab++)
    if (optab->code == code)
      return optab->name;
  return NULL;
}

static struct config_option const *
config_option_find (struct config_option const *optab, char const *name)
{
  for (; optab->name; optab++)
    {
      if (strcmp (optab->name, name) == 0)
	return optab;
    }
  return NULL;
}

static int
gettok_option (struct config_option const *optab, int *opt, char const **arg)
{
  struct token *tok;

  if ((tok = gettkn_any ()) == NULL)
    {
      *opt = -1;
    }
  else if (tok->type != T_LITERAL || tok->str[0] != '-')
    {
      *opt = -1;
      putback_tkn (tok);
      return CFGPARSER_OK;
    }

  if ((optab = config_option_find (optab, tok->str + 1)) == NULL)
    {
      conf_error ("unexpected token: %s", tok->str);
      return CFGPARSER_FAIL;
    }

  *opt = optab->code;
  if (optab->has_arg)
    {
      if ((tok = gettkn_expect (T_STRING)) == NULL)
	return CFGPARSER_FAIL;
      *arg = tok->str;
    }
  else
    *arg = NULL;
  return CFGPARSER_OK;
}

BACKEND *
backend_create (BACKEND_TYPE type, int prio, struct locus_range const *loc)
{
  BACKEND *be = calloc (1, sizeof (*be));
  if (be)
    {
      be->be_type = type;
      be->priority = prio;
      pthread_mutex_init (&be->mut, &mutex_attr_recursive);
      locus_range_init (&be->locus);
      if (loc)
	locus_range_copy (&be->locus, loc);
      backend_refcount_init (be);
    }
  return be;
}

static BACKEND *
xbackend_create (BACKEND_TYPE type, int prio, struct locus_range const *loc)
{
  BACKEND *be = backend_create (type, prio, loc);
  if (!be)
    xnomem ();
  return be;
}

/*
 * Named backends
 */
typedef struct named_backend
{
  char *name;
  struct locus_range locus;
  int priority;
  int disabled;
  struct be_matrix bemtx;
  SLIST_ENTRY (named_backend) link;
} NAMED_BACKEND;

#define HT_TYPE NAMED_BACKEND
#include "ht.h"

typedef struct named_backend_table
{
  NAMED_BACKEND_HASH *hash;
  SLIST_HEAD(,named_backend) head;
} NAMED_BACKEND_TABLE;

static void
named_backend_table_init (NAMED_BACKEND_TABLE *tab)
{
  tab->hash = NAMED_BACKEND_HASH_NEW ();
  SLIST_INIT (&tab->head);
}

static void
named_backend_table_free (NAMED_BACKEND_TABLE *tab)
{
  NAMED_BACKEND_HASH_FREE (tab->hash);
  while (!SLIST_EMPTY (&tab->head))
    {
      NAMED_BACKEND *ent = SLIST_FIRST (&tab->head);
      SLIST_SHIFT (&tab->head, link);
      free (ent);
    }
}

static NAMED_BACKEND *
named_backend_insert (NAMED_BACKEND_TABLE *tab, char const *name, BACKEND *be)
{
  NAMED_BACKEND *bp, *old;

  bp = xmalloc (sizeof (*bp) + strlen (name) + 1);
  bp->name = (char*) (bp + 1);
  strcpy (bp->name, name);
  locus_range_init (&bp->locus);
  locus_range_copy (&bp->locus, &be->locus);
  bp->priority = be->priority;
  bp->disabled = be->disabled;
  bp->bemtx = be->v.mtx;
  if ((old = NAMED_BACKEND_INSERT (tab->hash, bp)) != NULL)
    {
      free (bp);
      return old;
    }
  SLIST_PUSH (&tab->head, bp, link);
  return NULL;
}

static NAMED_BACKEND *
named_backend_retrieve (NAMED_BACKEND_TABLE *tab, char const *name)
{
  NAMED_BACKEND key;

  key.name = (char*) name;
  return NAMED_BACKEND_RETRIEVE (tab->hash, &key);
}

typedef struct
{
  int log_level;
  int facility;
  unsigned clnt_to;
  unsigned be_to;
  unsigned ws_to;
  unsigned be_connto;
  unsigned ignore_case;
  int re_type;
  int header_options;
  BALANCER_ALGO balancer_algo;
  NAMED_BACKEND_TABLE named_backend_table;
  struct resolver_config resolver;
  unsigned linebufsize;
} POUND_DEFAULTS;

static int
assign_bufsize (void *call_data, void *section_data)
{
  return cfg_assign_unsigned_min (call_data, MAXBUF, 0);
}

static int
assign_address_string (void *call_data, void *section_data)
{
  struct token *tok = gettkn_expect_mask (T_BIT (T_IDENT) |
					  T_BIT (T_STRING) |
					  T_BIT (T_LITERAL));
  if (!tok)
    return CFGPARSER_FAIL;
  *(char**)call_data = xstrdup (tok->str);
  return CFGPARSER_OK;
}

static int
assign_port_string (void *call_data, void *section_data)
{
  struct token *tok = gettkn_expect_mask (T_BIT (T_IDENT) |
					  T_BIT (T_STRING) |
					  T_BIT (T_LITERAL) |
					  T_BIT (T_NUMBER));
  if (!tok)
    return CFGPARSER_FAIL;
  *(char**)call_data = xstrdup (tok->str);
  return CFGPARSER_OK;
}

static int
assign_address_family (void *call_data, void *section_data)
{
  static struct kwtab kwtab[] = {
    { "any",  AF_UNSPEC },
    { "unix", AF_UNIX },
    { "inet", AF_INET },
    { "inet6", AF_INET6 },
    { NULL }
  };
  return cfg_assign_int_enum (call_data, gettkn_expect (T_IDENT), kwtab,
			      "address family name");
}

static int
assign_port_generic (struct token *tok, int family, int *port)
{
  struct addrinfo hints, *res;
  int rc;

  if (!tok)
    return CFGPARSER_FAIL;

  if (tok->type != T_IDENT && tok->type != T_NUMBER)
    {
      conf_error_at_locus_range (&tok->locus,
				 "expected port number or service name, but found %s",
				 token_type_str (tok->type));
      return CFGPARSER_FAIL;
    }

  memset (&hints, 0, sizeof(hints));
  hints.ai_flags = 0;
  hints.ai_family = family;
  hints.ai_socktype = SOCK_STREAM;
  rc = getaddrinfo (NULL, tok->str, &hints, &res);
  if (rc != 0)
    {
      conf_error_at_locus_range (&tok->locus,
				 "bad port number: %s", gai_strerror (rc));
      return CFGPARSER_FAIL;
    }

  switch (res->ai_family)
    {
    case AF_INET:
      *port = ((struct sockaddr_in *)res->ai_addr)->sin_port;
      break;

    case AF_INET6:
      *port = ((struct sockaddr_in6 *)res->ai_addr)->sin6_port;
      break;

    default:
      conf_error_at_locus_range (&tok->locus, "%s",
				 "Port is supported only for INET/INET6 back-ends");
      return CFGPARSER_FAIL;
    }
  freeaddrinfo (res);
  return CFGPARSER_OK;
}

static int
assign_port_int (void *call_data, void *section_data)
{
  return assign_port_generic (gettkn_any (), AF_UNSPEC, call_data);
}

static int
assign_CONTENT_LENGTH (void *call_data, void *section_data)
{
  CONTENT_LENGTH n;
  char *p;
  struct token *tok = gettkn_expect (T_NUMBER);

  if (!tok)
    return CFGPARSER_FAIL;

  if (strtoclen (tok->str, 10, &n, &p) || *p)
    {
      conf_error ("%s", "bad long number");
      return CFGPARSER_FAIL;
    }
  *(CONTENT_LENGTH *)call_data = n;
  return 0;
}

static int
assign_cert (void *call_data, void *section_data)
{
  X509 **x509_ptr = call_data, *cert;
  struct token *tok;
  FILE *fp;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;

  if ((fp = fopen_include (tok->str)) == NULL)
    {
      fopen_error (LOG_ERR, errno, include_wd, tok->str, &tok->locus);
      return CFGPARSER_FAIL;
    }
  cert = PEM_read_X509 (fp, NULL, NULL, NULL);
  fclose (fp);
  if (cert == NULL)
    {
      conf_openssl_error (tok->str, "can't load certificate");
      return CFGPARSER_FAIL;
    }
  *x509_ptr = cert;
  return CFGPARSER_OK;
}

/*
 * ACL support
 */

/* Max. number of bytes in an inet address (suitable for both v4 and v6) */
#define MAX_INADDR_BYTES 16

typedef struct cidr
{
  int family;                           /* Address family */
  int len;                              /* Address length */
  unsigned char addr[MAX_INADDR_BYTES]; /* Network address */
  unsigned char mask[MAX_INADDR_BYTES]; /* Address mask */
  SLIST_ENTRY (cidr) next;              /* Link to next CIDR */
} CIDR;

/* Create a new ACL. */
static ACL *
new_acl (char const *name)
{
  ACL *acl;

  XZALLOC (acl);
  if (name)
    acl->name = xstrdup (name);
  else
    acl->name = NULL;
  SLIST_INIT (&acl->head);

  return acl;
}

/* Match cidr against inet address ap/len.  Return 0 on match, 1 otherwise. */
static int
cidr_match (CIDR *cidr, unsigned char *ap, size_t len)
{
  size_t i;

  if (cidr->len == len)
    {
      for (i = 0; i < len; i++)
	{
	  if (cidr->addr[i] != (ap[i] & cidr->mask[i]))
	    return 1;
	}
    }
  return 0;
}

/*
 * Split the inet address of SA to address pointer and length, suitable
 * for use with the above functions.  Store pointer in RET_PTR.  Return
 * address length in bytes, or -1 if SA has invalid address family.
 */
int
sockaddr_bytes (struct sockaddr *sa, unsigned char **ret_ptr)
{
  switch (sa->sa_family)
    {
    case AF_INET:
      *ret_ptr = (unsigned char *) &(((struct sockaddr_in*)sa)->sin_addr.s_addr);
      return 4;

    case AF_INET6:
      *ret_ptr = (unsigned char *) &(((struct sockaddr_in6*)sa)->sin6_addr);
      return 16;

    default:
      break;
    }
  return -1;
}

static int
dynacl_read (void *obj, char const *filename, WORKDIR *wd)
{
  return config_parse_acl_file (obj, filename, wd);
}

static void
dynacl_clear (void *obj)
{
  acl_clear (obj);
}

static int
dynacl_register (ACL *acl, char const *filename, struct locus_range const *loc)
{
  acl->watcher = watcher_register (acl, filename, loc,
				   dynacl_read, dynacl_clear);
  return acl->watcher == NULL;
}

/*
 * Match sockaddr SA against ACL.  Return 0 if it matches, 1 if it does not
 * and -1 on error (invalid address family).
 */
int
acl_match (ACL *acl, struct sockaddr *sa)
{
  CIDR *cidr;
  unsigned char *ap;
  size_t len;
  int rc = 1;

  if ((len = sockaddr_bytes (sa, &ap)) == -1)
    return -1;

  acl_lock (acl);
  SLIST_FOREACH (cidr, &acl->head, next)
    {
      if (cidr->family == sa->sa_family && cidr_match (cidr, ap, len) == 0)
	{
	  rc = 0;
	  break;
	}
    }
  acl_unlock (acl);

  return rc;
}

void
acl_clear (ACL *acl)
{
  while (!SLIST_EMPTY (&acl->head))
    {
      struct cidr *cp = SLIST_FIRST (&acl->head);
      SLIST_SHIFT (&acl->head, next);
      free (cp);
    }
}

static void
masklen_to_netmask (unsigned char *buf, size_t len, size_t masklen)
{
  int i, cnt;

  cnt = masklen / 8;
  for (i = 0; i < cnt; i++)
    buf[i] = 0xff;
  if (i == MAX_INADDR_BYTES)
    return;
  cnt = 8 - masklen % 8;
  buf[i++] = (0xff >> cnt) << cnt;
  for (; i < MAX_INADDR_BYTES; i++)
    buf[i] = 0;
}

static int
parse_cidr_str (ACL *acl, char const *str, struct locus_range const *loc)
{
  char *mask;
  struct addrinfo hints, *res;
  unsigned long masklen;
  int rc;

  if ((mask = strchr (str, '/')) != NULL)
    {
      char *p;

      *mask++ = 0;

      errno = 0;
      masklen = strtoul (mask, &p, 10);
      if (errno || *p)
	{
	  conf_error_at_locus_range (loc, "%s", "invalid netmask");
	  return CFGPARSER_FAIL;
	}
    }

  memset (&hints, 0, sizeof (hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_NUMERICHOST;

  if ((rc = getaddrinfo (str, NULL, &hints, &res)) == 0)
    {
      CIDR *cidr;
      int len, i;
      unsigned char *p;

      if ((len = sockaddr_bytes (res->ai_addr, &p)) == -1)
	{
	  conf_error_at_locus_range (loc, "%s", "unsupported address family");
	  return CFGPARSER_FAIL;
	}
      XZALLOC (cidr);
      cidr->family = res->ai_family;
      cidr->len = len;
      memcpy (cidr->addr, p, len);
      if (!mask)
	masklen = len * 8;
      masklen_to_netmask (cidr->mask, cidr->len, masklen);
      /* Fix-up network address, just in case */
      for (i = 0; i < len; i++)
	cidr->addr[i] &= cidr->mask[i];
      SLIST_PUSH (&acl->head, cidr, next);
      freeaddrinfo (res);
    }
  else
    {
      conf_error_at_locus_range (loc, "invalid IP address: %s",
				 gai_strerror (rc));
      return CFGPARSER_FAIL;
    }
  return CFGPARSER_OK;
}

/* Parse CIDR at the current point of the input. */
static int
parse_cidr (ACL *acl)
{
  struct token *tok;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;
  return parse_cidr_str (acl, tok->str, &tok->locus);
}

/*
 * List of named ACLs.
 * There shouldn't be many of them, so it's perhaps no use in implementing
 * more sophisticated data structures than a mere singly-linked list.
 */
static ACL_HEAD acl_list = SLIST_HEAD_INITIALIZER (acl_list);

/*
 * Return a pointer to the named ACL, or NULL if no ACL with such name is
 * found.
 */
static ACL *
acl_by_name (char const *name)
{
  ACL *acl;
  SLIST_FOREACH (acl, &acl_list, next)
    {
      if (strcmp (acl->name, name) == 0)
	break;
    }
  return acl;
}

int
config_parse_acl_file (ACL *acl, char const *filename, WORKDIR *wd)
{
  FILE *fp;
  char buf[MAXBUF];
  struct locus_range loc;
  int rc;
  char *p;

  fp = fopen_wd (wd, filename);
  if (fp == NULL)
    return -1;

  locus_point_init (&loc.beg, filename, wd->name);
  locus_point_init (&loc.end, NULL, NULL);

  rc = 0;
  loc.beg.line--;
  while ((p = fgets (buf, sizeof buf, fp)) != NULL)
    {
      char *line;
      size_t len = strlen (p);

      loc.beg.line++;
      if (len == 0)
	continue;
      if (p[len-1] == '\n')
	len--;
      line = c_trimws (p, &len);
      if (len == 0 || *p == '#')
	continue;
      line[len] = 0;

      if (line[0] == '"' && line[len-1] == '"')
	{
	  line[--len] = 0;
	  line++;
	}

      if (parse_cidr_str (acl, line, &loc))
	rc++;
    }
  fclose (fp);
  locus_range_unref (&loc);
  return rc;
}

static int
parse_acl_file (ACL *acl, char const *filename)
{
  WORKDIR *wd;
  char const *basename;
  int rc;

  if ((basename = filename_split_wd (filename, &wd)) == NULL)
    return CFGPARSER_FAIL;
  rc = config_parse_acl_file (acl, basename, wd);
  workdir_unref (wd);
  if (rc == -1)
    {
      if (errno == ENOENT)
	conf_error ("file %s does not exist", filename);
      else
	conf_error ("can't open %s: %s", filename, strerror (errno));
      return CFGPARSER_FAIL;
    }
  else if (rc > 0)
    {
      conf_error ("errors reading %s", filename);
      return CFGPARSER_FAIL;
    }
  return CFGPARSER_OK;
}

/*
 * Parse ACL definition.
 * On entry, input must be positioned on the next token after ACL ["name"].
 */
static int
parse_acl (ACL *acl)
{
  struct token *tok;

  if ((tok = gettkn_any ()) == NULL)
    return CFGPARSER_FAIL;

  if (tok->type != '\n')
    {
      conf_error ("expected newline, but found %s", token_type_str (tok->type));
      return CFGPARSER_FAIL;
    }

  for (;;)
    {
      int rc;
      if ((tok = gettkn_any ()) == NULL)
	return CFGPARSER_FAIL;
      if (tok->type == '\n')
	continue;
      if (tok->type == T_IDENT)
	{
	  if (c_strcasecmp (tok->str, "end") == 0)
	    break;
	  if (c_strcasecmp (tok->str, "include") == 0)
	    {
	      if ((rc = cfg_parse_include (NULL, NULL)) == CFGPARSER_FAIL)
		return rc;
	      continue;
	    }
	  conf_error ("expected CIDR, \"Include\", or \"End\", but found %s",
		      token_type_str (tok->type));
	  return CFGPARSER_FAIL;
	}
      putback_tkn (tok);
      if ((rc = parse_cidr (acl)) != CFGPARSER_OK)
	return rc;
    }
  return CFGPARSER_OK;
}

#define OPT_FILE  0x1
#define OPT_WATCH 0x2
#define OPT_FWD   0x4

static int
get_file_option (int *opt, char const **filename, int fwd)
{
  static struct config_option optab[] = {
    { "forwarded", OPT_FWD },
    { "file", OPT_FILE, 1 },
    { "filewatch", OPT_FILE | OPT_WATCH, 1 },
    { NULL }
  };
  struct config_option *opptr = fwd ? optab : optab + 1;

  *opt = 0;
  *filename = NULL;

  for (;;)
    {
      int n;

      if (gettok_option (opptr, &n, filename) == CFGPARSER_FAIL)
	return CFGPARSER_FAIL;
      if (n == -1)
	break;
      *opt |= n;
    }
  return CFGPARSER_OK;
}

/*
 * Parse a named ACL.
 * Input is positioned after the "ACL" keyword.
 */
static int
parse_named_acl (void *call_data, void *section_data)
{
  ACL *acl;
  struct token *tok;
  int opt;
  char const *filename;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;

  if (acl_by_name (tok->str))
    {
      conf_error ("%s", "ACL with that name already defined");
      return CFGPARSER_FAIL;
    }

  acl = new_acl (tok->str);
  SLIST_PUSH (&acl_list, acl, next);

  if (get_file_option (&opt, &filename, 0) == CFGPARSER_FAIL)
    return CFGPARSER_FAIL;

  if (filename)
    {
      if (opt & OPT_WATCH)
	{
	  if (dynacl_register (acl, filename, &tok->locus))
	    return CFGPARSER_FAIL;
	}
      else if (parse_acl_file (acl, filename))
	return CFGPARSER_FAIL;
      return CFGPARSER_OK;
    }

  return parse_acl (acl);
}

/*
 * Parse ACL reference.  Three forms are accepted:
 * ACL "name"
 *   References a named ACL.
 * ACL -file "name"
 * ACL -filewatch "name"
 *   Read ACL from file.
 * ACL "\n" ... End
 *   Creates and references an unnamed ACL.
 */
static int
parse_acl_ref (ACL **ret_acl, int *pforwarded)
{
  struct token *tok;
  ACL *acl;
  int opt;
  char const *filename;

  if (get_file_option (&opt, &filename, pforwarded != NULL) == CFGPARSER_FAIL)
    return CFGPARSER_FAIL;
  if (pforwarded)
    *pforwarded = !!(opt & OPT_FWD);

  if (filename)
    {
      acl = new_acl (NULL);
      *ret_acl = acl;

      if (opt & OPT_WATCH)
	{
	  if (dynacl_register (acl, filename, last_token_locus_range ()))
	    return CFGPARSER_FAIL;
	}
      else if (parse_acl_file (acl, filename))
	return CFGPARSER_FAIL;
      return CFGPARSER_OK;
    }

  if ((tok = gettkn_any ()) == NULL)
    return CFGPARSER_FAIL;

  if (tok->type == '\n')
    {
      putback_tkn (tok);
      acl = new_acl (NULL);
      *ret_acl = acl;
      return parse_acl (acl);
    }
  else if (tok->type == T_STRING)
    {
      if ((acl = acl_by_name (tok->str)) == NULL)
	{
	  conf_error ("no such ACL: %s", tok->str);
	  return CFGPARSER_FAIL;
	}
      *ret_acl = acl;
    }
  else
    {
      conf_error ("expected ACL name or definition, but found %s",
		  token_type_str (tok->type));
      return CFGPARSER_FAIL;
    }
  return CFGPARSER_OK;
}

static int
assign_acl (void *call_data, void *section_data)
{
  return parse_acl_ref (call_data, NULL);
}

static int
parse_ECDHCurve (void *call_data, void *section_data)
{
  struct token *tok = gettkn_expect (T_STRING);
  if (!tok)
    return CFGPARSER_FAIL;
#if SET_DH_AUTO == 0 && !defined OPENSSL_NO_ECDH
  if (set_ECDHCurve (tok->str) == 0)
    {
      conf_error ("%s", "ECDHCurve: invalid curve name");
      return CFGPARSER_FAIL;
    }
#else
  conf_error ("%s", "statement ignored");
#endif
  return CFGPARSER_OK;
}

static int
parse_SSLEngine (void *call_data, void *section_data)
{
  struct token *tok = gettkn_expect (T_STRING);
  if (!tok)
    return CFGPARSER_FAIL;
#if HAVE_OPENSSL_ENGINE_H && OPENSSL_VERSION_MAJOR < 3
  ENGINE *e;

  if (!(e = ENGINE_by_id (tok->str)))
    {
      conf_error ("%s", "unrecognized engine");
      return CFGPARSER_FAIL;
    }

  if (!ENGINE_init (e))
    {
      ENGINE_free (e);
      conf_error ("%s", "could not init engine");
      return CFGPARSER_FAIL;
    }

  if (!ENGINE_set_default (e, ENGINE_METHOD_ALL))
    {
      ENGINE_free (e);
      conf_error ("%s", "could not set all defaults");
    }

  ENGINE_finish (e);
  ENGINE_free (e);
#else
  conf_error ("%s", "statement ignored");
#endif

  return CFGPARSER_OK;
}

static int
backend_parse_https (void *call_data, void *section_data)
{
  BACKEND *be = call_data;
  struct stringbuf sb;

  if ((be->v.mtx.ctx = SSL_CTX_new (SSLv23_client_method ())) == NULL)
    {
      conf_openssl_error (NULL, "SSL_CTX_new");
      return CFGPARSER_FAIL;
    }

  SSL_CTX_set_app_data (be->v.mtx.ctx, be);
  SSL_CTX_set_verify (be->v.mtx.ctx, SSL_VERIFY_NONE, NULL);
  SSL_CTX_set_mode (be->v.mtx.ctx, SSL_MODE_AUTO_RETRY);
#ifdef SSL_MODE_SEND_FALLBACK_SCSV
  SSL_CTX_set_mode (be->v.mtx.ctx, SSL_MODE_SEND_FALLBACK_SCSV);
#endif
  SSL_CTX_set_options (be->v.mtx.ctx, SSL_OP_ALL);
#ifdef  SSL_OP_NO_COMPRESSION
  SSL_CTX_set_options (be->v.mtx.ctx, SSL_OP_NO_COMPRESSION);
#endif
  SSL_CTX_clear_options (be->v.mtx.ctx,
			 SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION);
  SSL_CTX_clear_options (be->v.mtx.ctx, SSL_OP_LEGACY_SERVER_CONNECT);

  xstringbuf_init (&sb);
  stringbuf_printf (&sb, "%d-Pound-%ld", getpid (), random ());
  SSL_CTX_set_session_id_context (be->v.mtx.ctx,
				  (unsigned char *) stringbuf_value (&sb),
				  stringbuf_len (&sb));
  stringbuf_free (&sb);

  POUND_SSL_CTX_init (be->v.mtx.ctx);

  return CFGPARSER_OK;
}

static int
backend_parse_cert (void *call_data, void *section_data)
{
  BACKEND *be = call_data;
  struct token *tok;
  char *filename;

  if (be->v.mtx.ctx == NULL)
    {
      conf_error ("%s", "HTTPS must be used before this statement");
      return CFGPARSER_FAIL;
    }

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;

  if ((filename = filename_resolve (tok->str)) == NULL)
    return CFGPARSER_FAIL;

  if (SSL_CTX_use_certificate_chain_file (be->v.mtx.ctx, filename) != 1)
    {
      conf_openssl_error (filename, "SSL_CTX_use_certificate_chain_file");
      return CFGPARSER_FAIL;
    }

  if (SSL_CTX_use_PrivateKey_file (be->v.mtx.ctx, filename, SSL_FILETYPE_PEM) != 1)
    {
      conf_openssl_error (filename, "SSL_CTX_use_PrivateKey_file");
      return CFGPARSER_FAIL;
    }

  if (SSL_CTX_check_private_key (be->v.mtx.ctx) != 1)
    {
      conf_openssl_error (filename, "SSL_CTX_check_private_key failed");
      return CFGPARSER_FAIL;
    }
  free (filename);

  return CFGPARSER_OK;
}

static int
backend_assign_ciphers (void *call_data, void *section_data)
{
  BACKEND *be = call_data;
  struct token *tok;

  if (be->v.mtx.ctx == NULL)
    {
      conf_error ("%s", "HTTPS must be used before this statement");
      return CFGPARSER_FAIL;
    }

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;

  SSL_CTX_set_cipher_list (be->v.mtx.ctx, tok->str);
  return CFGPARSER_OK;
}

static int
backend_assign_priority (void *call_data, void *section_data)
{
  return cfg_assign_int_range (call_data, 1, -1);
}

static int
set_proto_opt (int *opt)
{
  int n;

  static struct kwtab kwtab[] = {
    { "SSLv2", SSL_OP_NO_SSLv2 },
    { "SSLv3", SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 },
#ifdef SSL_OP_NO_TLSv1
    { "TLSv1", SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 },
#endif
#ifdef SSL_OP_NO_TLSv1_1
    { "TLSv1_1", SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
		 SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 },
#endif
#ifdef SSL_OP_NO_TLSv1_2
    { "TLSv1_2", SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
		 SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 |
		 SSL_OP_NO_TLSv1_2 },
#endif
    { NULL }
  };
  int res = cfg_assign_int_enum (&n, gettkn_expect (T_IDENT), kwtab,
				 "protocol name");
  if (res == CFGPARSER_OK)
    *opt |= n;

  return res;
}

static int
disable_proto (void *call_data, void *section_data)
{
  SSL_CTX *ctx = *(SSL_CTX**) call_data;
  int n = 0;

  if (ctx == NULL)
    {
      conf_error ("%s", "HTTPS must be used before this statement");
      return CFGPARSER_FAIL;
    }

  if (set_proto_opt (&n) != CFGPARSER_OK)
    return CFGPARSER_FAIL;

  SSL_CTX_set_options (ctx, n);

  return CFGPARSER_OK;
}

static struct kwtab resolve_mode_kwtab[] = {
  { "immediate", bres_immediate },
  { "first", bres_first },
  { "all", bres_all },
  { "srv", bres_srv },
  { NULL }
};

char const *
resolve_mode_str (int mode)
{
  char const *ret = kw_to_str (resolve_mode_kwtab, mode);
  return ret ? ret : "UNKNOWN";
}

static int
assign_resolve_mode (void *call_data, void *section_data)
{
  int res = cfg_assign_int_enum (call_data, gettkn_expect (T_IDENT),
				 resolve_mode_kwtab,
				 "backend resolve mode");
#ifndef ENABLE_DYNAMIC_BACKENDS
  if (res != bres_immediate)
    {
      conf_error ("%s", "value not supported: pound compiled without support for dynamic backends");
      res = CFGPARSER_FAIL;
    }
#endif
  return res;
}

static CFGPARSER_TABLE backend_parsetab[] = {
  {
    .name = "End",
    .parser = cfg_parse_end
  },
  {
    .name = "Address",
    .parser = assign_address_string,
    .off = offsetof (BACKEND, v.mtx.hostname)
  },
  {
    .name = "Port",
    .parser = assign_port_int,
    .off = offsetof (BACKEND, v.mtx.port)
  },
  {
    .name = "Family",
    .parser = assign_address_family,
    .off = offsetof (BACKEND, v.mtx.family)
  },
  {
    .name = "Resolve",
    .parser = assign_resolve_mode,
    .off = offsetof (BACKEND, v.mtx.resolve_mode)
  },
  {
    .name = "IgnoreSRVWeight",
    .parser = cfg_assign_bool,
    .off = offsetof (BACKEND, v.mtx.ignore_srv_weight)
  },
  {
    .name = "OverrideTTL",
    .parser = cfg_assign_timeout,
    .off = offsetof (BACKEND, v.mtx.override_ttl)
  },
  {
    .name = "RetryInterval",
    .parser = cfg_assign_timeout,
    .off = offsetof (BACKEND, v.mtx.retry_interval)
  },
  {
    .name = "Priority",
    .parser = backend_assign_priority,
    .off = offsetof (BACKEND, priority)
  },
  {
    .name = "TimeOut",
    .parser = cfg_assign_timeout,
    .off = offsetof (BACKEND, v.mtx.to)
  },
  {
    .name = "WSTimeOut",
    .parser = cfg_assign_timeout,
    .off = offsetof (BACKEND, v.mtx.ws_to)
  },
  {
    .name = "ConnTO",
    .parser = cfg_assign_timeout,
    .off = offsetof (BACKEND, v.mtx.conn_to)
  },
  {
    .name = "HTTPS",
    .parser = backend_parse_https
  },
  {
    .name = "Cert",
    .parser = backend_parse_cert
  },
  {
    .name = "Ciphers",
    .parser = backend_assign_ciphers
  },
  {
    .name = "Disable",
    .parser = disable_proto,
    .off = offsetof (BACKEND, v.mtx.ctx)
  },
  {
    .name = "Disabled",
    .parser = cfg_assign_bool,
    .off = offsetof (BACKEND, disabled)
  },
  {
    .name = "ServerName",
    .parser = cfg_assign_string,
    .off = offsetof (BACKEND, v.mtx.servername)
  },
  { NULL }
};

static CFGPARSER_TABLE use_backend_parsetab[] = {
  {
    .name = "End",
    .parser = cfg_parse_end
  },
  {
    .name = "Priority",
    .parser = backend_assign_priority,
    .off = offsetof (BACKEND, priority)
  },
  {
    .name = "Disabled",
    .parser = cfg_assign_bool,
    .off = offsetof (BACKEND, disabled)
  },
  { NULL }
};

static BACKEND *
parse_backend_internal (CFGPARSER_TABLE *table, POUND_DEFAULTS *dfl)
{
  BACKEND *be;
  struct locus_range range = LOCUS_RANGE_INITIALIZER;

  be = xbackend_create (BE_MATRIX, 5, NULL);
  be->v.mtx.to = dfl->be_to;
  be->v.mtx.conn_to = dfl->be_connto;
  be->v.mtx.ws_to = dfl->ws_to;

  if (parser_loop (table, be, dfl, &range))
    return NULL;

  be->locus = range;

  return be;
}

static int
parse_backend (void *call_data, void *section_data)
{
  BALANCER_LIST *bml = call_data;
  BACKEND *be;
  struct token *tok;
  struct locus_point beg = LOCUS_POINT_INITIALIZER;

  locus_point_copy (&beg, &last_token_locus_range ()->beg);

  if ((tok = gettkn_any ()) == NULL)
    {
      locus_point_unref (&beg);
      return CFGPARSER_FAIL;
    }

  if (tok->type == T_STRING)
    {
      struct locus_range range = LOCUS_RANGE_INITIALIZER;
      be = xbackend_create (BE_BACKEND_REF, -1, NULL);
      be->v.be_name = xstrdup (tok->str);
      be->disabled = -1;

      if (parser_loop (use_backend_parsetab, be, section_data, &range))
	{
	  locus_point_unref (&beg);
	  return CFGPARSER_FAIL;
	}
      be->locus = range;
    }
  else
    {
      putback_tkn (tok);
      be = parse_backend_internal (backend_parsetab, section_data);
      if (!be)
	{
	  locus_point_unref (&beg);
	  return CFGPARSER_FAIL;
	}
    }

  locus_point_copy (&be->locus.beg, &beg);
  locus_point_unref (&beg);

  balancer_add_backend (balancer_list_get_normal (bml), be);

  return CFGPARSER_OK;
}

static int
parse_use_backend (void *call_data, void *section_data)
{
  BALANCER_LIST *bml = call_data;
  BACKEND *be;
  struct token *tok;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;

  be = xbackend_create (BE_BACKEND_REF, 5, &tok->locus);
  be->v.be_name = xstrdup (tok->str);
  locus_range_copy (&be->locus, &tok->locus);

  balancer_add_backend (balancer_list_get_normal (bml), be);

  return CFGPARSER_OK;
}

static int
parse_emergency (void *call_data, void *section_data)
{
  BALANCER_LIST *bml = call_data;
  BACKEND *be;
  POUND_DEFAULTS dfl = *(POUND_DEFAULTS*)section_data;

  dfl.be_to = 120;
  dfl.be_connto = 120;
  dfl.ws_to = 120;

  be = parse_backend_internal (backend_parsetab, &dfl);
  if (!be)
    return CFGPARSER_FAIL;

  balancer_add_backend (balancer_list_get_emerg (bml), be);

  return CFGPARSER_OK;
}

static int
parse_control_backend (void *call_data, void *section_data)
{
  BALANCER_LIST *bml = call_data;
  BACKEND *be = xbackend_create (BE_CONTROL, 1, last_token_locus_range ());
  balancer_add_backend (balancer_list_get_normal (bml), be);
  return CFGPARSER_OK;
}

static int
parse_metrics (void *call_data, void *section_data)
{
  BALANCER_LIST *bml = call_data;
  BACKEND *be = xbackend_create (BE_METRICS, 1, last_token_locus_range ());
  balancer_add_backend (balancer_list_get_normal (bml), be);
  return CFGPARSER_OK;
}

static SERVICE_COND *
service_cond_alloc (int type)
{
  SERVICE_COND *sc;
  XZALLOC (sc);
  service_cond_init (sc, type);
  return sc;
}

static void
service_cond_free (SERVICE_COND *sc)
{
  switch (sc->type)
    {
    case COND_QUERY_PARAM:
    case COND_STRING_MATCH:
      string_unref (sc->sm.string);
      genpat_free (sc->sm.re);
      break;

    case COND_URL:
    case COND_PATH:
    case COND_QUERY:
    case COND_HDR:
    case COND_HOST:
      genpat_free (sc->re);
      break;

    default:
      /* FIXME: so far this function is only used by dyncond_read which
	 operates on one of cond types handled above. */
      abort ();
    }
  string_unref (sc->tag);
  free (sc);
}

static SERVICE_COND *
service_cond_append (SERVICE_COND *cond, int type)
{
  SERVICE_COND *sc;

  assert (cond->type == COND_BOOL || cond->type == COND_DYN);
  sc = service_cond_alloc (type);
  SLIST_PUSH (&cond->boolean.head, sc, next);

  return sc;
}

static void
stringbuf_escape_regex (struct stringbuf *sb, char const *p)
{
  while (*p)
    {
      size_t len = strcspn (p, "\\[]{}().*+?");
      if (len > 0)
	stringbuf_add (sb, p, len);
      p += len;
      if (*p)
	{
	  stringbuf_add_char (sb, '\\');
	  stringbuf_add_char (sb, *p);
	  p++;
	}
    }
}

struct match_param
{
  char *from_file;
  int watch;
  STRING *tag;
  int decode;
};

#define MATCH_PARAM_INITIALIZER { NULL, 0, NULL, -1 }

static int
parse_match_mode (int dfl_re_type, int *gp_type, int *sp_flags,
		  struct match_param *param)
{
  enum
  {
    MATCH_RE,
    MATCH_EXACT,
    MATCH_BEG,
    MATCH_END,
    MATCH_CONTAIN,
    MATCH_ICASE,
    MATCH_CASE,
    MATCH_FILE,
    MATCH_FILEWATCH,
    MATCH_POSIX,
    MATCH_PCRE,
    MATCH_TAG,
    MATCH_DECODE
  };

  static struct config_option optab[] = {
    { "file",      MATCH_FILE, 1 },
    { "filewatch", MATCH_FILEWATCH, 1 },
    { "re",        MATCH_RE },
    { "exact",     MATCH_EXACT },
    { "beg",       MATCH_BEG },
    { "end",       MATCH_END },
    { "contain",   MATCH_CONTAIN },
    { "icase",     MATCH_ICASE },
    { "case",      MATCH_CASE },
    { "posix",     MATCH_POSIX },
    { "pcre",      MATCH_PCRE },
    { "perl",      MATCH_PCRE },
    { "tag",       MATCH_TAG, 1 },
    { "decode",    MATCH_DECODE },
    { NULL }
  };

  int disabled = 0;
#define MATCH_OPTION_MASK(n) (1<<(n))
#define MATCH_OPTION_DISABLED(n) (disabled & MATCH_OPTION_MASK(n))

  if (param == NULL)
    disabled |= MATCH_OPTION_MASK (MATCH_FILE)
      | MATCH_OPTION_MASK (MATCH_FILEWATCH)
      | MATCH_OPTION_MASK (MATCH_TAG)
      | MATCH_OPTION_MASK (MATCH_DECODE);
  else if (param->decode == -1)
    disabled |= MATCH_OPTION_MASK (MATCH_DECODE);

  for (;;)
    {
      int n;
      char const *arg;

      if (gettok_option (optab, &n, &arg) == CFGPARSER_FAIL)
	return CFGPARSER_FAIL;

      if (n == -1)
	break;

      if (MATCH_OPTION_DISABLED (n))
	{
	  conf_error ("unexpected token: %s", config_option_name (optab, n));
	  return CFGPARSER_FAIL;
	}

      switch (n)
	{
	case MATCH_CASE:
	  *sp_flags &= ~GENPAT_ICASE;
	  break;

	case MATCH_ICASE:
	  *sp_flags |= GENPAT_ICASE;
	  break;

	case MATCH_FILE:
	  param->from_file = xstrdup (arg);
	  param->watch = 0;
	  break;

	case MATCH_FILEWATCH:
	  param->from_file = xstrdup (arg);
	  param->watch = 1;
	  break;

	case MATCH_RE:
	  *gp_type = dfl_re_type;
	  break;

	case MATCH_POSIX:
	  *gp_type = GENPAT_POSIX;
	  break;

	case MATCH_EXACT:
	  *gp_type = GENPAT_EXACT;
	  break;

	case MATCH_BEG:
	  *gp_type = GENPAT_PREFIX;
	  break;

	case MATCH_END:
	  *gp_type = GENPAT_SUFFIX;
	  break;

	case MATCH_CONTAIN:
	  *gp_type = GENPAT_CONTAIN;
	  break;

	case MATCH_PCRE:
#ifdef HAVE_LIBPCRE
	  *gp_type = GENPAT_PCRE;
#else
	  conf_error ("%s", "pound compiled without PCRE");
	  return CFGPARSER_FAIL;
#endif
	  break;

	case MATCH_TAG:
	  param->tag = string_init (arg);
	  break;

	case MATCH_DECODE:
	  param->decode = 1;
	}
    }
  return CFGPARSER_OK;
}

static char *
header_prefix_regex (struct stringbuf *sb, int *gp_type,
		     char const *hdr, char const *expr)
{
  stringbuf_add_char (sb, '^');
  stringbuf_add_string (sb, hdr);
  stringbuf_add_char (sb, ':');
  switch (*gp_type)
    {
    case GENPAT_POSIX:
      stringbuf_add_string (sb, "[[:space:]]*");
      if (expr[0] == '^')
	expr++;
      stringbuf_add_string (sb, expr);
      break;

    case GENPAT_PCRE:
      stringbuf_add_string (sb, "\\s*");
      if (expr[0] == '^')
	expr++;
      stringbuf_add_string (sb, expr);
      break;

    case GENPAT_EXACT:
    case GENPAT_PREFIX:
      stringbuf_add_string (sb, "[[:space:]]*");
      stringbuf_escape_regex (sb, expr);
      *gp_type = GENPAT_POSIX;
      break;

    case GENPAT_SUFFIX:
      stringbuf_add_string (sb, "[[:space:]]*");
      stringbuf_add_string (sb, ".*");
      stringbuf_escape_regex (sb, expr);
      stringbuf_add_char (sb, '$');
      *gp_type = GENPAT_POSIX;
      break;

    case GENPAT_CONTAIN:
      stringbuf_add_string (sb, "[[:space:]]*");
      stringbuf_add_string (sb, ".*");
      stringbuf_escape_regex (sb, expr);
      *gp_type = GENPAT_POSIX;
      break;

    default:
      abort ();
    }
  return stringbuf_finish (sb);
}

static inline char *
host_prefix_regex (struct stringbuf *sb, int *gp_type, char const *expr)
{
  return header_prefix_regex (sb, gp_type, "Host", expr);
}

static int
parse_regex_compat (GENPAT *regex, int dfl_re_type, int gp_type, int flags)
{
  struct token *tok;
  int rc;

  if (parse_match_mode (dfl_re_type, &gp_type, &flags, NULL))
    return CFGPARSER_FAIL;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;

  rc = genpat_compile (regex, gp_type, tok->str, flags);
  if (rc)
    {
      conf_regcomp_error (rc, *regex, NULL);
      genpat_free (*regex);
      return CFGPARSER_FAIL;
    }

  return CFGPARSER_OK;
}

static int
dyncond_read_internal (SERVICE_COND *cond, char const *filename, WORKDIR *wd,
		       STRING *ref, enum service_cond_type cond_type,
		       int pat_type, int flags)
{
  FILE *fp;
  struct locus_range loc;
  char *p;
  char buf[MAXBUF];
  int rc;
  struct stringbuf sb;

  fp = fopen_wd (wd, filename);
  if (fp == NULL)
    return -1;

  locus_point_init (&loc.beg, filename, wd->name);
  locus_point_init (&loc.end, NULL, NULL);

  rc = 0;

  xstringbuf_init (&sb);
  loc.beg.line--;
  while ((p = fgets (buf, sizeof buf, fp)) != NULL)
    {
      int rc;
      size_t len = strlen (p);
      SERVICE_COND *hc;
      char *expr;
      int gpt = pat_type;

      loc.beg.line++;

      if (len == 0)
	continue;
      if (p[len-1] == '\n')
	len--;
      p = c_trimws (p, &len);
      if (len == 0 || *p == '#')
	continue;
      p[len] = 0;

      switch (cond_type)
	{
	case COND_HOST:
	  stringbuf_reset (&sb);
	  expr = host_prefix_regex (&sb, &gpt, p);
	  break;

	case COND_HDR:
	  if (ref)
	    {
	      stringbuf_reset (&sb);
	      expr = header_prefix_regex (&sb, &gpt, string_ptr (ref), p);
	      break;
	    }
	  /* fall through */
	default:
	  expr = p;
	}

      hc = service_cond_alloc (cond_type);
      hc->tag = string_ref (cond->tag);
      rc = genpat_compile (&hc->re, gpt, expr, flags);
      if (rc)
	{
	  regcomp_error_at_locus_range (&loc, hc->re, NULL);
	  service_cond_free (hc);
	  rc++;
	}
      else
	{
	  switch (cond_type)
	    {
	    case COND_QUERY_PARAM:
	    case COND_STRING_MATCH:
	      memmove (&hc->sm.re, &hc->re, sizeof (hc->sm.re));
	      hc->sm.string = string_ref (ref);
	      break;

	    default:
	      break;
	    }

	  SLIST_PUSH (&cond->boolean.head, hc, next);
	}
    }
  stringbuf_free (&sb);
  locus_range_unref (&loc);
  fclose (fp);
  return rc;
}

static int
dyncond_read (void *obj, char const *filename, WORKDIR *wd)
{
  SERVICE_COND *cond = obj;
  return dyncond_read_internal (cond, filename, wd,
				cond->dyn.string,
				cond->dyn.cond_type,
				cond->dyn.pat_type,
				cond->dyn.flags);
}

static int
dyncond_read_immediate (SERVICE_COND *cond, char *filename,
			STRING *ref, enum service_cond_type cond_type,
			int pat_type, int flags)
{
  WORKDIR *wd;
  char const *basename;
  int rc;

  if ((basename = filename_split_wd (filename, &wd)) == NULL)
    return CFGPARSER_FAIL;
  rc = dyncond_read_internal (cond, basename, wd, ref, cond_type, pat_type,
			      flags);
  workdir_unref (wd);
  if (rc == -1)
    {
      if (errno == ENOENT)
	conf_error ("file %s does not exist", filename);
      else
	conf_error ("can't open %s: %s", filename, strerror (errno));
      return CFGPARSER_FAIL;
    }
  else if (rc > 0)
    {
      conf_error ("errors reading %s", filename);
      return CFGPARSER_FAIL;
    }
  return CFGPARSER_OK;
}

static void
dyncond_clear (void *obj)
{
  SERVICE_COND *cond = obj;
  assert (cond->type == COND_BOOL || cond->type == COND_DYN);
  while (!SLIST_EMPTY (&cond->boolean.head))
    {
      SERVICE_COND *sc = SLIST_FIRST (&cond->boolean.head);
      SLIST_SHIFT (&cond->boolean.head, next);
      service_cond_free (sc);
    }
}

static int
dyncond_register (SERVICE_COND *cond, char const *filename,
		  struct locus_range const *loc)
{
  cond->watcher = watcher_register (cond, filename, loc,
				    dyncond_read, dyncond_clear);
  return cond->watcher == NULL;
}

static int
parse_cond_matcher (SERVICE_COND *top_cond,
		    enum service_cond_type type,
		    int dfl_re_type,
		    int gp_type, int flags, char const *string)
{
  struct token *tok;
  int rc;
  struct stringbuf sb;
  SERVICE_COND *cond;
  STRING *ref;
  struct match_param match_param = MATCH_PARAM_INITIALIZER;

  if (type == COND_PATH || type == COND_QUERY_PARAM)
    match_param.decode = 0;

  if (parse_match_mode (dfl_re_type, &gp_type, &flags, &match_param))
    return CFGPARSER_FAIL;

  if (match_param.from_file)
    {
      switch (type)
	{
	case COND_HDR:
	case COND_QUERY_PARAM:
	case COND_STRING_MATCH:
	  ref = string_init (string);
	  break;
	default:
	  ref = NULL;
	  break;
	}

      if (match_param.watch)
	{
	  cond = service_cond_append (top_cond, COND_DYN);
	  cond->tag = match_param.tag;
	  cond->decode = match_param.decode;
	  cond->dyn.boolean.op = BOOL_OR;
	  cond->dyn.string = ref;
	  cond->dyn.cond_type = type;
	  cond->dyn.pat_type = gp_type;
	  cond->dyn.flags = flags;
	  if (dyncond_register (cond, match_param.from_file,
				last_token_locus_range ()))
	    return CFGPARSER_FAIL;
	}
      else
	{
	  cond = service_cond_append (top_cond, COND_BOOL);
	  cond->tag = match_param.tag;
	  cond->decode = match_param.decode;
	  cond->boolean.op = BOOL_OR;
	  rc = dyncond_read_immediate (cond, match_param.from_file,
				       ref, type, gp_type, flags);
	  string_unref (ref);
	  if (rc)
	    return rc;
	}
      free (match_param.from_file);
    }
  else
    {
      char *expr;

      if ((tok = gettkn_expect (T_STRING)) == NULL)
	return CFGPARSER_FAIL;

      xstringbuf_init (&sb);
      cond = service_cond_append (top_cond, type);
      cond->tag = match_param.tag;
      cond->decode = match_param.decode;
      switch (type)
	{
	case COND_HOST:
	  expr = host_prefix_regex (&sb, &gp_type, tok->str);
	  break;

	case COND_HDR:
	  if (string)
	    {
	      expr = header_prefix_regex (&sb, &gp_type, string, tok->str);
	      break;
	    }
	  /* fall through */
	default:
	  expr = tok->str;
	}
      rc = genpat_compile (&cond->re, gp_type, expr, flags);
      if (rc)
	{
	  conf_regcomp_error (rc, cond->re, NULL);
	  // FIXME: genpat_free (cond->re);
	  return CFGPARSER_FAIL;
	}
      switch (type)
	{
	case COND_QUERY_PARAM:
	case COND_STRING_MATCH:
	  memmove (&cond->sm.re, &cond->re, sizeof (cond->sm.re));
	  cond->sm.string = string_init (string);
	  break;

	default:
	  break;
	}
      stringbuf_free (&sb);
    }

  return CFGPARSER_OK;
}

static int
parse_cond_acl (void *call_data, void *section_data)
{
  SERVICE_COND *cond = service_cond_append (call_data, COND_ACL);
  return parse_acl_ref (&cond->acl.acl, &cond->acl.forwarded);
}

static int
parse_cond_url_matcher (void *call_data, void *section_data)
{
  POUND_DEFAULTS *dfl = section_data;
  return parse_cond_matcher (call_data, COND_URL, dfl->re_type,
			     dfl->re_type,
			     (dfl->ignore_case ? GENPAT_ICASE : 0),
			     NULL);
}

static int
parse_cond_path_matcher (void *call_data, void *section_data)
{
  POUND_DEFAULTS *dfl = section_data;
  return parse_cond_matcher (call_data, COND_PATH, dfl->re_type,
			     dfl->re_type,
			     (dfl->ignore_case ? GENPAT_ICASE : 0),
			     NULL);
}

static int
parse_cond_query_matcher (void *call_data, void *section_data)
{
  POUND_DEFAULTS *dfl = section_data;
  return parse_cond_matcher (call_data, COND_QUERY, dfl->re_type,
			     dfl->re_type,
			     (dfl->ignore_case ? GENPAT_ICASE : 0),
			     NULL);
}

static int
parse_cond_query_param_matcher (void *call_data, void *section_data)
{
  SERVICE_COND *top_cond = call_data;
  POUND_DEFAULTS *dfl = section_data;
  int flags = (dfl->ignore_case ? GENPAT_ICASE : 0);
  struct token *tok;
  char *string;
  int rc;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;
  string = xstrdup (tok->str);
  rc = parse_cond_matcher (top_cond,
			   COND_QUERY_PARAM, dfl->re_type,
			   dfl->re_type, flags, string);
  free (string);
  return rc;
}

static int
parse_cond_string_matcher (void *call_data, void *section_data)
{
  SERVICE_COND *top_cond = call_data;
  POUND_DEFAULTS *dfl = section_data;
  int flags = (dfl->ignore_case ? GENPAT_ICASE : 0);
  struct token *tok;
  char *string;
  int rc;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;
  string = xstrdup (tok->str);
  rc = parse_cond_matcher (top_cond,
			   COND_STRING_MATCH, dfl->re_type, dfl->re_type,
			   flags,
			   string);
  free (string);
  return rc;
}

static int
parse_cond_hdr_matcher (void *call_data, void *section_data)
{
  POUND_DEFAULTS *dfl = section_data;
  char *string = NULL;
  struct token *tok;
  int rc;

  if ((tok = gettkn_any ()) == NULL)
    return CFGPARSER_FAIL;
  if (tok->type == T_STRING)
    {
      struct locus_range loc = LOCUS_RANGE_INITIALIZER;
      string = xstrdup (tok->str);
      locus_range_copy (&loc, &tok->locus);

      if ((tok = gettkn_any ()) == NULL)
	{
	  locus_range_unref (&loc);
	  free (string);
	  return CFGPARSER_FAIL;
	}
      putback_tkn (tok);
      if (tok->type == '\n')
	{
	  putback_synth (T_STRING, string, &loc);
	  free (string);
	  string = NULL;
	}
      locus_range_unref (&loc);
    }
  else
    putback_tkn (tok);
  rc = parse_cond_matcher (call_data, COND_HDR, dfl->re_type, dfl->re_type,
			   GENPAT_MULTILINE | GENPAT_ICASE,
			   string);
  free (string);
  return rc;
}

static int
parse_cond_head_deny_matcher (void *call_data, void *section_data)
{
  POUND_DEFAULTS *dfl = section_data;
  SERVICE_COND *cond = service_cond_append (call_data, COND_BOOL);
  cond->boolean.op = BOOL_NOT;
  return parse_cond_matcher (cond, COND_HDR, dfl->re_type, dfl->re_type,
			     GENPAT_MULTILINE | GENPAT_ICASE,
			     NULL);
}

static int
parse_cond_host (void *call_data, void *section_data)
{
  POUND_DEFAULTS *dfl = section_data;
  return parse_cond_matcher (call_data, COND_HOST, dfl->re_type,
			     GENPAT_EXACT, GENPAT_ICASE, NULL);
}

static int
parse_cond_basic_auth (void *call_data, void *section_data)
{
  SERVICE_COND *cond = service_cond_append (call_data, COND_BASIC_AUTH);
  struct locus_range loc = LOCUS_RANGE_INITIALIZER;
  struct token *tok;
  int opt;
  char const *filename;
  void *wt;

  if (get_file_option (&opt, &filename, 0) == CFGPARSER_FAIL)
    return CFGPARSER_FAIL;

  if (filename)
    {
      locus_range_copy (&loc, last_token_locus_range ());
      if (! (opt & OPT_WATCH))
	{
	  char const *basename;
	  WORKDIR *wd;
	  int rc;

	  if ((basename = filename_split_wd (filename, &wd)) == NULL)
	    return CFGPARSER_FAIL;
	  rc = basic_auth_read (cond, basename, wd);
	  workdir_unref (wd);
	  if (rc == -1)
	    {
	      if (errno == ENOENT)
		conf_error ("file %s does not exist", filename);
	      else
		conf_error ("can't open %s: %s", filename, strerror (errno));
	      return CFGPARSER_FAIL;
	    }
	  locus_range_unref (&loc);
	  return CFGPARSER_OK;
	}
    }
  else if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;
  else
    {
      filename = tok->str;
      locus_range_copy (&loc, &tok->locus);
    }

  wt = watcher_register (cond, filename, &loc,
			 basic_auth_read, basic_auth_clear);
  locus_range_unref (&loc);
  if (wt == NULL)
    return CFGPARSER_FAIL;
  return CFGPARSER_OK;
}

static int
parse_cond_client_cert (void *call_data, void *section_data)
{
  SERVICE_COND *cond = service_cond_append (call_data, COND_CLIENT_CERT);
  return assign_cert (&cond->x509, NULL);
}

#ifndef UINT64_MAX
# define UINT64_MAX ((uint64_t)-1)
#endif

/*
 * Compute (v * d) / n with range checking.  On success, store the result
 * in retval and return 0.
 */
static int
mulf (unsigned long v, unsigned n, unsigned long d, uint64_t *retval)
{
  if (UINT64_MAX / n < v)
    {
      uint64_t e = (v - UINT64_MAX / n) * n;
      if (UINT64_MAX / n < (v - e))
	return -1;
      *retval = ((v - e) * n) / d + e / d;
    }
  else
    *retval = (v * n) / d;
  return 0;
}

static int
parse_rate (uint64_t *ret_rate)
{
  struct token *tok;
  unsigned long rate;
  unsigned long n = 1;
  char *p;
  enum { I_SEC, I_MSEC, I_USEC };
  int i = I_SEC;
  static struct kwtab intervals[] = {
    { "s",  I_SEC },
    { "ms", I_MSEC },
    { "us", I_USEC },
    { NULL }
  };
  static unsigned mul[] = {
    NANOSECOND,
    1000000,
    1000,
  };
  if ((tok = gettkn_any ()) == NULL)
    return CFGPARSER_FAIL;
  errno = 0;
  rate = strtoul (tok->str, &p, 10);
  if (errno || rate == 0)
    {
      conf_error ("%s", "bad unsigned number");
      return CFGPARSER_FAIL;
    }
  else if (*p == '/')
    {
      ++p;

      if (c_isdigit (p[0]))
	{
	  n = strtoul (p, &p, 10);
	  if (n == 0 || errno)
	    {
	      conf_error ("%s", "bad interval specifier");
	      return CFGPARSER_FAIL;
	    }
	}

      if (kw_to_tok (intervals, p, 1, &i))
	{
	  struct locus_range r = *last_token_locus_range ();
	  r.beg.col += p - tok->str;
	  conf_error_at_locus_range (&r, "%s", "bad interval specifier");
	  return CFGPARSER_FAIL;
	}
    }
  else if (*p != 0)
    {
      conf_error ("%s", "invalid rate");
      return CFGPARSER_FAIL;
    }

  if (mulf (n, mul[i], rate, ret_rate))
    {
      conf_error ("%s", "effective rate is out of range");
      return CFGPARSER_FAIL;
    }

  if (*ret_rate == 0)
    {
      conf_error ("%s", "effective rate is 0");
      return CFGPARSER_FAIL;
    }

  return CFGPARSER_OK;
}

static int
parse_cond_tbf (void *call_data, void *section_data)
{
  SERVICE_COND *cond = service_cond_append (call_data, COND_TBF);
  struct token *tok;
  uint64_t rate;
  unsigned maxtok;
  STRING *key;
  int rc;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;
  key = string_init (tok->str);

  if ((rc = parse_rate (&rate)) != CFGPARSER_OK)
    return rc;
  if ((rc = cfg_assign_unsigned (&maxtok, section_data)) != CFGPARSER_OK)
    return rc;

  cond->tbf.key = key;
  cond->tbf.tbf = tbf_alloc (rate, maxtok);
  return CFGPARSER_OK;
}

static int
parse_lua_match (void *call_data, void *section_data)
{
  SERVICE_COND *cond = service_cond_append (call_data, COND_LUA);
  return pndlua_parse_closure (&cond->clua);
}

/* Named and detached conditions. */

/*
 * Named condition represents a detached condition during configuration
 * file processing.  At the end of parsing phase, all references to named
 * conditions are resolved, the conditions (SERVICE_COND pointers) they
 * refer to are moved to the detcond_index array and assigned the condition
 * number.  Finally, the memory allocated to named conditions is freed.
 */
typedef struct named_cond
{
  char *name;
  SERVICE_COND *cond;
  struct locus_range locus;
  int idx;
} NAMED_COND;

#define HT_TYPE NAMED_COND
#define HT_NO_DELETE
#define HT_NO_FOREACH_SAFE
#include "ht.h"

static NAMED_COND_HASH *named_cond_hash;

/* Array of detached conditions. */
SERVICE_COND **detcond_index;
static int detcond_count;

/* Return a pointer to the detached condition with the given number. */
SERVICE_COND *
detached_cond (int n)
{
  assert (n >= 0 && n < detcond_count);
  return detcond_index[n];
}

/*
 * eval_result array
 *
 * An array of detcond_count char elements is associated with each HTTP
 * request.  Each element keeps the result of evaluation of the corresponding
 * detached condition, increased by 1.  Thus, eval_result[n] == 0 means that
 * condition n has not yet been evaluated.
 */

/*
 * Get result of the latest evaluation of detached condition n.
 * Returns -1 if the condition has not been evaluated yet.
 */
int
http_request_eval_get (struct http_request *http, int n)
{
  assert (n >= 0 && n < detcond_count);
  if (!http->eval_result)
    return -1;
  return http->eval_result[n] - 1;
}

/* Store result res of evaluating the detached condition n. */
int
http_request_eval_cache (struct http_request *http, int n, int res)
{
  assert (n >= 0 && n < detcond_count);
  if (!http->eval_result)
    {
      http->eval_result = calloc (detcond_count,
				  sizeof (http->eval_result[0]));
      if (!http->eval_result)
	{
	  lognomem ();
	  return -1;
	}
    }
  return http->eval_result[n] = !!res + 1;
}

/*
 * Named condition hash management.
 */

/*
 * Free a named condition entry.
 * The service condition pointer is not freed.
 */
static void
named_cond_free (NAMED_COND *nc)
{
  free (nc->name);
  locus_range_unref (&nc->locus);
}

/*
 * Store the condition in the detcond_index array and free the named condition.
 */
static void
named_cond_store (NAMED_COND *nc, void *data)
{
  detcond_index[nc->idx] = nc->cond;
  named_cond_free (nc);
}

/* Finalize detached condition processing. */
static void
named_cond_finish (void)
{
  if (detcond_count)
    {
      detcond_index = xcalloc (detcond_count, sizeof (detcond_index[0]));
      NAMED_COND_FOREACH (named_cond_hash, named_cond_store, NULL);
      NAMED_COND_HASH_FREE (named_cond_hash);
    }
}

/* Find named condition by its name. */
static NAMED_COND *
named_cond_lookup (char const *name)
{
  NAMED_COND *cond = NULL;
  if (named_cond_hash)
    {
      NAMED_COND key;
      key.name = (char*)name;
      cond = NAMED_COND_RETRIEVE (named_cond_hash, &key);
    }
  return cond;
}

/* Allocate new named condition with boolean operation op. */
static NAMED_COND *
named_cond_new (char *name, int op)
{
  NAMED_COND *nc, *old;

  if (!named_cond_hash)
    named_cond_hash = NAMED_COND_HASH_NEW ();
  XZALLOC (nc);
  nc->name = name;
  nc->cond = service_cond_alloc (COND_BOOL);
  nc->cond->boolean.op = op;
  locus_range_init (&nc->locus);
  locus_range_copy (&nc->locus, last_token_locus_range ());

  old = NAMED_COND_INSERT (named_cond_hash, nc);
  if (old != NULL)
    {
      conf_error ("%s redefined", name);
      conf_error_at_locus_range (&old->locus, "originally defined here");
      //FIXME named_cond_free (nc);
      return NULL;
    }
  nc->idx = detcond_count++;
  return nc;
}

static int parse_bool_cond (SERVICE_COND *cond, void *section_data);

/* Parse a standalone Condition statement. */
static int
parse_named_cond (void *call_data, void *section_data)
{
  struct token *tok;
  char *name;
  int op = BOOL_AND;
  NAMED_COND *nc;
  int result;
  struct locus_range const *r;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;
  name = xstrdup (tok->str);
  if ((tok = gettkn_any ()) == NULL)
    return CFGPARSER_FAIL;
  if (tok->type == T_IDENT)
    {
      if (c_strcasecmp (tok->str, "and") == 0)
	op = BOOL_AND;
      else if (c_strcasecmp (tok->str, "or") == 0)
	op = BOOL_OR;
      else
	{
	  conf_error ("expected AND or OR, but found %s", tok->str);
	  return CFGPARSER_FAIL;
	}
    }
  else
    {
      op = BOOL_AND;
      putback_tkn (tok);
    }

  nc = named_cond_new (name, op);

  if (nc == NULL)
    return CFGPARSER_FAIL;

  result = parse_bool_cond (nc->cond, section_data);

  if ((r = last_token_locus_range ()) != NULL)
    locus_point_copy (&nc->locus.end, &r->end);
  return result;
}

static int
parse_cond_eval (void *call_data, void *section_data)
{
  struct token *tok;
  SERVICE_COND *cond;
  NAMED_COND *nc;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;

  if ((nc = named_cond_lookup (tok->str)) == NULL)
    {
      conf_error ("%s: no such condition", tok->str);
      return CFGPARSER_FAIL;
    }

  cond = service_cond_append (call_data, COND_REF);
  cond->ref = nc->idx;

  return CFGPARSER_OK;
}

static int
parse_redirect_backend (void *call_data, void *section_data)
{
  BALANCER_LIST *bml = call_data;
  struct token *tok;
  int code = 302;
  BACKEND *be;
  POUND_REGMATCH matches[5];
  struct locus_range range;

  range.beg = last_token_locus_range ()->beg;

  if ((tok = gettkn_any ()) == NULL)
    return CFGPARSER_FAIL;

  if (tok->type == T_NUMBER)
    {
      int n = atoi (tok->str);
      switch (n)
	{
	case 301:
	case 302:
	case 303:
	case 307:
	case 308:
	  code = n;
	  break;

	default:
	  conf_error ("%s", "invalid status code");
	  return CFGPARSER_FAIL;
	}

      if ((tok = gettkn_any ()) == NULL)
	return CFGPARSER_FAIL;
    }

  range.end = last_token_locus_range ()->end;

  if (tok->type != T_STRING)
    {
      conf_error ("expected %s, but found %s", token_type_str (T_STRING), token_type_str (tok->type));
      return CFGPARSER_FAIL;
    }

  be = xbackend_create (BE_REDIRECT, 1, &range);
  be->v.redirect.status = code;
  be->v.redirect.url = xstrdup (tok->str);

  if (genpat_match (LOCATION, be->v.redirect.url, 4, matches))
    {
      conf_error ("%s", "Redirect bad URL");
      return CFGPARSER_FAIL;
    }

  if ((be->v.redirect.has_uri = matches[3].rm_eo - matches[3].rm_so) == 1)
    /* the path is a single '/', so remove it */
    be->v.redirect.url[matches[3].rm_so] = '\0';

  balancer_add_backend (balancer_list_get_normal (bml), be);

  return CFGPARSER_OK;
}

static int
parse_http_errmsg (struct http_errmsg *errmsg)
{
  int rc;
  char *p;

  rc = cfg_assign_string_from_file (&errmsg->text, NULL);
  if (rc == CFGPARSER_FAIL)
    return rc;

  DLIST_INIT (&errmsg->hdr);
  if (http_header_list_parse (&errmsg->hdr, errmsg->text, H_REPLACE, &p) == 0
      && p != errmsg->text)
    {
      p++;
      memmove (errmsg->text, p, strlen (p) + 1);
    }
  else
    http_header_list_free (&errmsg->hdr);
  return CFGPARSER_OK;
}

static int
parse_error_backend (void *call_data, void *section_data)
{
  BALANCER_LIST *bml = call_data;
  struct token *tok;
  int n, status;
  BACKEND *be;
  int rc;
  struct locus_range range;
  struct http_errmsg errmsg = HTTP_ERRMSG_INITIALIZER(errmsg);

  range.beg = last_token_locus_range ()->beg;

  if ((tok = gettkn_expect (T_NUMBER)) == NULL)
    return CFGPARSER_FAIL;

  n = atoi (tok->str);
  if ((status = http_status_to_pound (n)) == -1)
    {
      conf_error ("%s", "unsupported status code");
      return CFGPARSER_FAIL;
    }

  if ((tok = gettkn_any ()) == NULL)
    return CFGPARSER_FAIL;

  if (tok->type == T_STRING)
    {
      putback_tkn (tok);
      if ((rc = parse_http_errmsg (&errmsg)) == CFGPARSER_FAIL)
	return rc;
    }
  else if (tok->type == '\n')
    rc = CFGPARSER_OK_NONL;
  else
    {
      conf_error ("%s", "string or newline expected");
      return CFGPARSER_FAIL;
    }

  range.end = last_token_locus_range ()->end;

  be = xbackend_create (BE_ERROR, 1, &range);
  be->v.error.status = status;
  be->v.error.msg = errmsg;

  balancer_add_backend (balancer_list_get_normal (bml), be);

  return rc;
}

static int
parse_errorfile (void *call_data, void *section_data)
{
  struct token *tok;
  int status;
  struct http_errmsg **http_err = call_data;

  if ((tok = gettkn_expect (T_NUMBER)) == NULL)
    return CFGPARSER_FAIL;

  if ((status = http_status_to_pound (atoi (tok->str))) == -1)
    {
      conf_error ("%s", "unsupported status code");
      return CFGPARSER_FAIL;
    }

  if (!http_err[status])
    XZALLOC (http_err[status]);
  return parse_http_errmsg (http_err[status]);
}

static int
parse_errN (void *call_data, void *section_data)
{
  struct http_errmsg **http_err = call_data;

  if (!*http_err)
    XZALLOC (*http_err);
  return parse_http_errmsg (*http_err);
}

static int
parse_sendfile_backend (void *call_data, void *section_data)
{
  BALANCER_LIST *bml = call_data;
  struct token *tok;
  BACKEND *be;
  int fd;
  struct locus_range range;
  WORKDIR *wd = get_include_wd ();

  range.beg = last_token_locus_range ()->beg;
  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;
  range.end = last_token_locus_range ()->end;

  fd = openat (wd->fd, tok->str, O_DIRECTORY | O_RDONLY | O_NDELAY);
  if (fd == -1)
    {
      conf_error ("can't open %s: %s", tok->str, strerror (errno));
      return CFGPARSER_FAIL;
    }

  be = xbackend_create (BE_FILE, 1, &range);
  be->v.file.wd = fd;

  balancer_add_backend (balancer_list_get_normal (bml), be);

  return CFGPARSER_OK;
}

static int
parse_lua_backend (void *call_data, void *section_data)
{
  BALANCER_LIST *bml = call_data;
  BACKEND *be;
  int rc;

  be = xbackend_create (BE_LUA, 1, last_token_locus_range ());
  if ((rc = pndlua_parse_closure (&be->v.lua)) == CFGPARSER_OK ||
      rc == CFGPARSER_OK_NONL)
    balancer_add_backend (balancer_list_get_normal (bml), be);
  return rc;
}

struct service_session
{
  int type;
  char *id;
  unsigned ttl;
};

static struct kwtab sess_type_tab[] = {
  { "IP", SESS_IP },
  { "COOKIE", SESS_COOKIE },
  { "URL", SESS_URL },
  { "PARM", SESS_PARM },
  { "BASIC", SESS_BASIC },
  { "HEADER", SESS_HEADER },
  { NULL }
};

char const *
sess_type_to_str (int type)
{
  if (type == SESS_NONE)
    return "NONE";
  return kw_to_str (sess_type_tab, type);
}

static int
session_type_parser (void *call_data, void *section_data)
{
  SERVICE *svc = call_data;
  struct token *tok;
  int n;

  if ((tok = gettkn_expect (T_IDENT)) == NULL)
    return CFGPARSER_FAIL;

  if (kw_to_tok (sess_type_tab, tok->str, 1, &n))
    {
      conf_error ("%s", "Unknown Session type");
      return CFGPARSER_FAIL;
    }
  svc->sess_type = n;

  return CFGPARSER_OK;
}

static CFGPARSER_TABLE session_parsetab[] = {
  {
    .name = "End",
    .parser = cfg_parse_end
  },
  {
    .name = "Type",
    .parser = session_type_parser
  },
  {
    .name = "TTL",
    .parser = cfg_assign_timeout,
    .off = offsetof (SERVICE, sess_ttl)
  },
  {
    .name = "ID",
    .parser = cfg_assign_string,
    .off = offsetof (SERVICE, sess_id)
  },
  { NULL }
};

static int
parse_session (void *call_data, void *section_data)
{
  SERVICE *svc = call_data;
  struct locus_range range = LOCUS_RANGE_INITIALIZER;

  if (parser_loop (session_parsetab, svc, section_data, &range))
    return CFGPARSER_FAIL;

  if (svc->sess_type == SESS_NONE)
    {
      conf_error_at_locus_range (&range, "Session type not defined");
      locus_range_unref (&range);
      return CFGPARSER_FAIL;
    }

  if (svc->sess_ttl == 0)
    {
      conf_error_at_locus_range (&range, "Session TTL not defined");
      locus_range_unref (&range);
      return CFGPARSER_FAIL;
    }

  switch (svc->sess_type)
    {
    case SESS_COOKIE:
    case SESS_URL:
    case SESS_HEADER:
      if (svc->sess_id == NULL)
	{
	  conf_error ("%s", "Session ID not defined");
	  locus_range_unref (&range);
	  return CFGPARSER_FAIL;
	}
      break;

    default:
      break;
    }

  locus_range_unref (&range);
  return CFGPARSER_OK;
}

static int
assign_dfl_ignore_case (void *call_data, void *section_data)
{
  POUND_DEFAULTS *dfl = section_data;
  return cfg_assign_bool (&dfl->ignore_case, NULL);
}

static int parse_cond (int op, SERVICE_COND *cond, void *section_data);

static int
parse_match (void *call_data, void *section_data)
{
  struct token *tok;
  int op = BOOL_AND;

  if ((tok = gettkn_any ()) == NULL)
    return CFGPARSER_FAIL;
  if (tok->type == T_IDENT)
    {
      if (c_strcasecmp (tok->str, "and") == 0)
	op = BOOL_AND;
      else if (c_strcasecmp (tok->str, "or") == 0)
	op = BOOL_OR;
      else
	{
	  conf_error ("expected AND or OR, but found %s", tok->str);
	  return CFGPARSER_FAIL;
	}
    }
  else
    putback_tkn (tok);

  return parse_cond (op, call_data, section_data);
}

static int parse_not_cond (void *call_data, void *section_data);

static CFGPARSER_TABLE match_conditions[] = {
  {
    .name = "ACL",
    .parser = parse_cond_acl
  },
  {
    .name = "URL",
    .parser = parse_cond_url_matcher
  },
  {
    .name = "Path",
    .parser = parse_cond_path_matcher
  },
  {
    .name = "Query",
    .parser = parse_cond_query_matcher
  },
  {
    .name = "QueryParam",
    .parser = parse_cond_query_param_matcher
  },
  {
    .name = "Header",
    .parser = parse_cond_hdr_matcher
  },
  {
    .name = "HeadRequire",
    .type = KWT_ALIAS,
    .deprecated = 1
  },
  {
    .name = "HeadDeny",
    .parser = parse_cond_head_deny_matcher,
    .deprecated = 1,
    .message = "use \"Not Header\" instead"
  },
  {
    .name = "Host",
    .parser = parse_cond_host
  },
  {
    .name = "BasicAuth",
    .parser = parse_cond_basic_auth
  },
  {
    .name = "TBF",
    .parser = parse_cond_tbf
  },
  {
    .name = "StringMatch",
    .parser = parse_cond_string_matcher
  },
  {
    .name = "LuaMatch",
    .parser = parse_lua_match
  },
  {
    .name = "Match",
    .parser = parse_match
  },
  {
    .name = "NOT",
    .parser = parse_not_cond
  },
  {
    .name = "ClientCert",
    .parser = parse_cond_client_cert
  },
  {
    .name = "Eval",
    .parser = parse_cond_eval
  },
  { NULL }
};

static CFGPARSER_TABLE negate_parsetab[] = {
  {
    .name = "",
    .type = KWT_SOFTREF,
    .ref = match_conditions
  },
  { NULL }
};

static int
parse_not_cond (void *call_data, void *section_data)
{
  SERVICE_COND *cond = service_cond_append (call_data, COND_BOOL);
  cond->boolean.op = BOOL_NOT;
  return cfgparser (negate_parsetab, cond, section_data,
		    1,
		    feature_is_set (FEATURE_WARN_DEPRECATED)
			   ? DEPREC_WARN : DEPREC_OK,
		    NULL);
}

static CFGPARSER_TABLE logcon_parsetab[] = {
  {
    .name = "End",
    .parser = cfg_parse_end
  },
  {
    .name = "",
    .type = KWT_SOFTREF,
    .ref = match_conditions
  },
  { NULL }
};

static int
parse_bool_cond (SERVICE_COND *cond, void *section_data)
{
  return parser_loop (logcon_parsetab, cond, section_data, NULL);
}

static int
parse_cond (int op, SERVICE_COND *cond, void *section_data)
{
  SERVICE_COND *subcond = service_cond_append (cond, COND_BOOL);
  subcond->boolean.op = op;
  return parse_bool_cond (subcond, section_data);
}

static int parse_else (void *call_data, void *section_data);
static int parse_rewrite (void *call_data, void *section_data);
static int parse_set_header (void *call_data, void *section_data);
static int parse_delete_header (void *call_data, void *section_data);
static int parse_set_url (void *call_data, void *section_data);
static int parse_set_path (void *call_data, void *section_data);
static int parse_set_query (void *call_data, void *section_data);
static int parse_delete_query (void *call_data, void *section_data);
static int parse_set_query_param (void *call_data, void *section_data);
static int parse_sub_rewrite (void *call_data, void *section_data);
static int parse_lua_modify (void *call_data, void *section_data);

static CFGPARSER_TABLE rewrite_ops[] = {
  {
    .name = "SetHeader",
    .parser = parse_set_header
  },
  {
    .name = "DeleteHeader",
    .parser = parse_delete_header
  },
  {
    .name = "SetURL",
    .parser = parse_set_url },
  {
    .name = "SetPath",
    .parser = parse_set_path
  },
  {
    .name = "SetQuery",
    .parser = parse_set_query
  },
  {
    .name = "DeleteQuery",
    .parser = parse_delete_query
  },
  {
    .name = "SetQueryParam",
    .parser = parse_set_query_param
  },
  {
    .name = "LuaModify",
    .parser = parse_lua_modify
  },
  { NULL }
};

static CFGPARSER_TABLE rewrite_rule_parsetab[] = {
  {
    .name = "End",
    .parser = cfg_parse_end
  },
  {
    .name = "Rewrite",
    .parser = parse_sub_rewrite,
    .off = offsetof (REWRITE_RULE, ophead)
  },
  {
    .name = "Else",
    .parser = parse_else,
    .off = offsetof (REWRITE_RULE, iffalse)
  },
  {
    .name = "",
    .off = offsetof (REWRITE_RULE, cond),
    .type = KWT_SOFTREF,
    .ref = match_conditions
  },
  {
    .name = "",
    .off = offsetof (REWRITE_RULE, ophead),
    .type = KWT_SOFTREF,
    .ref = rewrite_ops
  },
  { NULL }
};

static int
parse_end_else (void *call_data, void *section_data)
{
  struct token nl = { '\n' };
  putback_tkn (NULL);
  putback_tkn (&nl);
  return CFGPARSER_END;
}

static CFGPARSER_TABLE else_rule_parsetab[] = {
  {
    .name = "End",
    .parser = parse_end_else
  },
  {
    .name = "Rewrite",
    .parser = parse_sub_rewrite,
    .off = offsetof (REWRITE_RULE, ophead)
  },
  {
    .name = "Else",
    .parser = parse_else,
    .off = offsetof (REWRITE_RULE, iffalse)
  },
  {
    .name = "",
    .off = offsetof (REWRITE_RULE, cond),
    .type = KWT_SOFTREF,
    .ref = match_conditions
  },
  {
    .name = "",
    .off = offsetof (REWRITE_RULE, ophead),
    .type = KWT_SOFTREF,
    .ref = rewrite_ops
  },
  { NULL }
};

static REWRITE_OP *
rewrite_op_alloc (REWRITE_OP_HEAD *head, enum rewrite_type type)
{
  REWRITE_OP *op;

  XZALLOC (op);
  op->type = type;
  SLIST_PUSH (head, op, next);

  return op;
}

static int
parse_encode_option (int *p)
{
  static struct config_option optab[] = {
    { "encode", 1, 0 },
    { NULL }
  };
  if (*p == -1)
    {
      char const *a;
      if (gettok_option (optab, p, &a) == CFGPARSER_FAIL)
	return CFGPARSER_FAIL;
    }
  return CFGPARSER_OK;
}

static int
parse_rewrite_op (REWRITE_OP_HEAD *head, enum rewrite_type type, int encode)
{
  REWRITE_OP *op = rewrite_op_alloc (head, type);
  struct token *tok;

  if (encode)
    {
      op->encode = -1;
      if (parse_encode_option (&op->encode) == CFGPARSER_FAIL)
	return CFGPARSER_FAIL;
      op->encode = op->encode == 1;
    }
  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;

  op->v.str = xstrdup (tok->str);
  return CFGPARSER_OK;
}

static int
parse_delete_header (void *call_data, void *section_data)
{
  REWRITE_OP *op = rewrite_op_alloc (call_data, REWRITE_HDR_DEL);
  POUND_DEFAULTS *dfl = section_data;

  XZALLOC (op->v.hdrdel);
  return parse_regex_compat (&op->v.hdrdel->pat, dfl->re_type, dfl->re_type,
			     (dfl->ignore_case ? GENPAT_ICASE : 0));
}

static int
parse_set_header (void *call_data, void *section_data)
{
  return parse_rewrite_op (call_data, REWRITE_HDR_SET, 0);
}

static int
parse_set_url (void *call_data, void *section_data)
{
  return parse_rewrite_op (call_data, REWRITE_URL_SET, 0);
}

static int
parse_set_path (void *call_data, void *section_data)
{
  return parse_rewrite_op (call_data, REWRITE_PATH_SET, 1);
}

static int
parse_set_query (void *call_data, void *section_data)
{
  return parse_rewrite_op (call_data, REWRITE_QUERY_SET, 0);
}

static int
parse_delete_query (void *call_data, void *section_data)
{
  rewrite_op_alloc (call_data, REWRITE_QUERY_DELETE);
  return CFGPARSER_OK;
}

static int
parse_set_query_param (void *call_data, void *section_data)
{
  REWRITE_OP *op = rewrite_op_alloc (call_data, REWRITE_QUERY_PARAM_SET);
  struct token *tok;

  op->encode = -1;
  if (parse_encode_option (&op->encode) == CFGPARSER_FAIL)
    return CFGPARSER_FAIL;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;
  op->v.qp.name = xstrdup (tok->str);

  /* -encode option is allowed both before and after parameter name, */
  if (parse_encode_option (&op->encode) == CFGPARSER_FAIL)
    return CFGPARSER_FAIL;
  op->encode = op->encode == 1;

  if ((tok = gettkn_any ()) == NULL)
    return CFGPARSER_FAIL;
  if (tok->type == '\n')
    {
      op->v.qp.value = NULL;
      return CFGPARSER_OK_NONL;
    }
  else if (tok->type != T_STRING)
    {
      conf_error ("expected %s, but found %s", "string",
		  token_type_str (tok->type));
      return CFGPARSER_FAIL;
    }

  op->v.qp.value = xstrdup (tok->str);

  return CFGPARSER_OK;

}

static int
parse_lua_modify (void *call_data, void *section_data)
{
  REWRITE_OP *op = rewrite_op_alloc (call_data, REWRITE_LUA);
  return pndlua_parse_closure (&op->v.lua);
}

static REWRITE_RULE *
rewrite_rule_alloc (REWRITE_RULE_HEAD *head)
{
  REWRITE_RULE *rule;

  XZALLOC (rule);
  service_cond_init (&rule->cond, COND_BOOL);
  SLIST_INIT (&rule->ophead);

  if (head)
    SLIST_PUSH (head, rule, next);

  return rule;
}

static int
parse_else (void *call_data, void *section_data)
{
  REWRITE_RULE *rule = rewrite_rule_alloc (NULL);
  *(REWRITE_RULE**)call_data = rule;
  return parser_loop (else_rule_parsetab, rule, section_data, NULL);
}

static int
parse_sub_rewrite (void *call_data, void *section_data)
{
  REWRITE_OP *op = rewrite_op_alloc (call_data, REWRITE_REWRITE_RULE);
  op->v.rule = rewrite_rule_alloc (NULL);
  return parser_loop (rewrite_rule_parsetab, op->v.rule, section_data, NULL);
}

static CFGPARSER_TABLE match_response_conditions[] = {
  {
    .name = "Header",
    .parser = parse_cond_hdr_matcher
  },
  {
    .name = "StringMatch",
    .parser = parse_cond_string_matcher
  },
  {
    .name = "LuaMatch",
    .parser = parse_lua_match
  },
  {
    .name = "Match",
    .parser = parse_match
  },
  {
    .name = "NOT",
    .parser = parse_not_cond
  },
  { NULL }
};

static CFGPARSER_TABLE rewrite_response_ops[] = {
  {
    .name = "SetHeader",
    .parser = parse_set_header
  },
  {
    .name = "DeleteHeader",
    .parser = parse_delete_header
  },
  {
    .name = "LuaModify",
    .parser = parse_lua_modify
  },
  { NULL },
};

static int parse_response_else (void *call_data, void *section_data);
static int parse_response_sub_rewrite (void *call_data, void *section_data);

static CFGPARSER_TABLE response_rewrite_rule_parsetab[] = {
  {
    .name = "End",
    .parser = cfg_parse_end
  },
  {
    .name = "Rewrite",
    .parser = parse_response_sub_rewrite,
    .off = offsetof (REWRITE_RULE, ophead)
  },
  {
    .name = "Else",
    .parser = parse_response_else,
    .off = offsetof (REWRITE_RULE, iffalse)
  },
  {
    .name = "",
    .off = offsetof (REWRITE_RULE, cond),
    .type = KWT_SOFTREF,
    .ref = match_response_conditions
  },
  {
    .name = "",
    .off = offsetof (REWRITE_RULE, ophead),
    .type = KWT_SOFTREF,
    .ref = rewrite_response_ops
  },
  { NULL }
};

static CFGPARSER_TABLE response_else_rule_parsetab[] = {
  {
    .name = "End",
    .parser = parse_end_else
  },
  {
    .name = "Rewrite",
    .parser = parse_response_sub_rewrite,
    .off = offsetof (REWRITE_RULE, ophead)
  },
  {
    .name = "Else",
    .parser = parse_else,
    .off = offsetof (REWRITE_RULE, iffalse)
  },
  {
    .name = "",
    .off = offsetof (REWRITE_RULE, cond),
    .type = KWT_SOFTREF,
    .ref = match_response_conditions
  },
  {
    .name = "",
    .off = offsetof (REWRITE_RULE, ophead),
    .type = KWT_SOFTREF,
    .ref = rewrite_response_ops
  },
  { NULL }
};

static int
parse_response_else (void *call_data, void *section_data)
{
  REWRITE_RULE *rule = rewrite_rule_alloc (NULL);
  *(REWRITE_RULE**)call_data = rule;
  return parser_loop (response_else_rule_parsetab, rule, section_data, NULL);
}

static int
parse_response_sub_rewrite (void *call_data, void *section_data)
{
  REWRITE_OP *op = rewrite_op_alloc (call_data, REWRITE_REWRITE_RULE);
  op->v.rule = rewrite_rule_alloc (NULL);
  return parser_loop (response_rewrite_rule_parsetab, op->v.rule, section_data, NULL);
}

static int
parse_rewrite (void *call_data, void *section_data)
{
  struct token *tok;
  CFGPARSER_TABLE *table;
  REWRITE_RULE_HEAD *rw = call_data, *head;

  if ((tok = gettkn_any ()) == NULL)
    return CFGPARSER_FAIL;
  if (tok->type == T_IDENT)
    {
      if (c_strcasecmp (tok->str, "response") == 0)
	{
	  table = response_rewrite_rule_parsetab;
	  head = &rw[REWRITE_RESPONSE];
	}
      else if (c_strcasecmp (tok->str, "request") == 0)
	{
	  table = rewrite_rule_parsetab;
	  head = &rw[REWRITE_REQUEST];
	}
      else
	{
	  conf_error ("expected response, request, or newline, but found %s",
		      token_type_str (tok->type));
	  return CFGPARSER_FAIL;
	}
    }
  else
    {
      putback_tkn (tok);
      table = rewrite_rule_parsetab;
      head = &rw[REWRITE_REQUEST];
    }
  return parser_loop (table, rewrite_rule_alloc (head), section_data, NULL);
}

static REWRITE_RULE *
rewrite_rule_last_uncond (REWRITE_RULE_HEAD *head)
{
  if (!SLIST_EMPTY (head))
    {
      REWRITE_RULE *rw = SLIST_LAST (head);
      if (rw->cond.type == COND_BOOL && SLIST_EMPTY (&rw->cond.boolean.head))
	return rw;
    }

  return rewrite_rule_alloc (head);
}

#define __cat2__(a,b) a ## b
#define SETFN_NAME(part)			\
  __cat2__(parse_,part)
#define SETFN_SVC_NAME(part)			\
  __cat2__(parse_svc_,part)
#define SETFN_SVC_DECL(part)					     \
  static int							     \
  SETFN_SVC_NAME(part) (void *call_data, void *section_data)	     \
  {								     \
    REWRITE_RULE *rule = rewrite_rule_last_uncond (call_data);	     \
    return SETFN_NAME(part) (&rule->ophead, section_data);	     \
  }

SETFN_SVC_DECL (set_url)
SETFN_SVC_DECL (set_path)
SETFN_SVC_DECL (set_query)
SETFN_SVC_DECL (delete_query)
SETFN_SVC_DECL (set_query_param)
SETFN_SVC_DECL (set_header)
SETFN_SVC_DECL (delete_header)
SETFN_SVC_DECL (lua_modify)

/*
 * Support for backward-compatible HeaderRemove and HeadRemove directives.
 */
static int
parse_header_remove (void *call_data, void *section_data)
{
  POUND_DEFAULTS *dfl = section_data;
  REWRITE_RULE *rule = rewrite_rule_last_uncond (call_data);
  REWRITE_OP *op = rewrite_op_alloc (&rule->ophead, REWRITE_HDR_DEL);
  XZALLOC (op->v.hdrdel);
  return parse_regex_compat (&op->v.hdrdel->pat, dfl->re_type, dfl->re_type,
			     GENPAT_ICASE | GENPAT_MULTILINE);
}

static int
parse_balancer (void *call_data, void *section_data)
{
  BALANCER_ALGO *t = call_data;
  struct token *tok;

  if ((tok = gettkn_expect_mask (T_UNQ)) == NULL)
    return CFGPARSER_FAIL;
  if (c_strcasecmp (tok->str, "random") == 0)
    *t = BALANCER_ALGO_RANDOM;
  else if (c_strcasecmp (tok->str, "iwrr") == 0)
    *t = BALANCER_ALGO_IWRR;
  else
    {
      conf_error ("unsupported balancing strategy: %s", tok->str);
      return CFGPARSER_FAIL;
    }
  return CFGPARSER_OK;
}

static int
parse_log_suppress (void *call_data, void *section_data)
{
  int *result_ptr = call_data;
  struct token *tok;
  int n;
  int result = 0;
  static struct kwtab status_table[] = {
    { "all",      STATUS_MASK (100) | STATUS_MASK (200) |
		  STATUS_MASK (300) | STATUS_MASK (400) | STATUS_MASK (500) },
    { "info",     STATUS_MASK (100) },
    { "success",  STATUS_MASK (200) },
    { "redirect", STATUS_MASK (300) },
    { "clterr",   STATUS_MASK (400) },
    { "srverr",   STATUS_MASK (500) },
    { NULL }
  };

  if ((tok = gettkn_expect_mask (T_UNQ)) == NULL)
    return CFGPARSER_FAIL;

  do
    {
      if (strlen (tok->str) == 1 && c_isdigit (tok->str[0]))
	{
	  n = tok->str[0] - '0';
	  if (n <= 0 || n >= sizeof (status_table) / sizeof (status_table[0]))
	    {
	      conf_error ("%s", "unsupported status mask");
	      return CFGPARSER_FAIL;
	    }
	  n = STATUS_MASK (n * 100);
	}
      else if (kw_to_tok (status_table, tok->str, 1, &n) != 0)
	{
	  conf_error ("%s", "unsupported status mask");
	  return CFGPARSER_FAIL;
	}
      result |= n;
    }
  while ((tok = gettkn_any ()) != NULL && tok->type != T_ERROR &&
	 T_MASK_ISSET (T_UNQ, tok->type));

  if (tok == NULL)
    {
      conf_error ("%s", "unexpected end of file");
      return CFGPARSER_FAIL;
    }
  if (tok->type == T_ERROR)
    return CFGPARSER_FAIL;

  putback_tkn (tok);

  *result_ptr = result;

  return CFGPARSER_OK;
}

static CFGPARSER_TABLE service_parsetab[] = {
  {
    .name = "End",
    .parser = cfg_parse_end
  },
  {
    .name = "",
    .off = offsetof (SERVICE, cond),
    .type = KWT_SOFTREF,
    .ref = match_conditions
  },
  {
    .name = "Rewrite",
    .parser = parse_rewrite,
    .off = offsetof (SERVICE, rewrite)
  },
  {
    .name = "RewriteErrors",
    .parser =  cfg_assign_bool,
    .off = offsetof (SERVICE, rewrite_errors)
  },
  {
    .name = "SetHeader",
    .parser = SETFN_SVC_NAME (set_header),
    .off = offsetof (SERVICE, rewrite)
  },
  {
    .name = "DeleteHeader",
    .parser = SETFN_SVC_NAME (delete_header),
    .off = offsetof (SERVICE, rewrite)
  },
  {
    .name = "LuaModify",
    .parser = SETFN_SVC_NAME (lua_modify),
    .off = offsetof (SERVICE, rewrite)
  },
  {
    .name = "SetURL",
    .parser = SETFN_SVC_NAME (set_url),
    .off = offsetof (SERVICE, rewrite)
  },
  {
    .name = "SetPath",
    .parser = SETFN_SVC_NAME (set_path),
    .off = offsetof (SERVICE, rewrite)
  },
  {
    .name = "SetQuery",
    .parser = SETFN_SVC_NAME (set_query),
    .off = offsetof (SERVICE, rewrite)
  },
  {
    .name = "DeleteQuery",
    .parser = SETFN_SVC_NAME (delete_query),
    .off = offsetof (SERVICE, rewrite)
  },
  {
    .name = "SetQueryParam",
    .parser = SETFN_SVC_NAME (set_query_param),
    .off = offsetof (SERVICE, rewrite)
  },

  {
    .name = "Disabled",
    .parser = cfg_assign_bool,
    .off = offsetof (SERVICE, disabled)
  },
  {
    .name = "Redirect",
    .parser = parse_redirect_backend,
    .off = offsetof (SERVICE, balancers)
  },
  {
    .name = "Error",
    .parser = parse_error_backend,
    .off = offsetof (SERVICE, balancers)
  },
  {
    .name = "SendFile",
    .parser = parse_sendfile_backend,
    .off = offsetof (SERVICE, balancers)
  },
  {
    .name = "Backend",
    .parser = parse_backend,
    .off = offsetof (SERVICE, balancers)
  },
  {
    .name = "UseBackend",
    .parser = parse_use_backend,
    .off = offsetof (SERVICE, balancers)
  },
  {
    .name = "Emergency",
    .parser = parse_emergency,
    .off = offsetof (SERVICE, balancers)
  },
  {
    .name = "Metrics",
    .parser = parse_metrics,
    .off = offsetof (SERVICE, balancers)
  },
  {
    .name = "Control",
    .parser = parse_control_backend,
    .off = offsetof (SERVICE, balancers)
  },
  {
    .name = "LuaBackend",
    .parser = parse_lua_backend,
    .off = offsetof (SERVICE, balancers)
  },
  {
    .name = "Session",
    .parser = parse_session
  },
  {
    .name = "Balancer",
    .parser = parse_balancer,
    .off = offsetof (SERVICE, balancer_algo)
  },
  {
    .name = "ForwardedHeader",
    .parser = cfg_assign_string,
    .off = offsetof (SERVICE, forwarded_header)
  },
  {
    .name = "TrustedIP",
    .parser = assign_acl,
    .off = offsetof (SERVICE, trusted_ips)
  },
  {
    .name = "LogSuppress",
    .parser = parse_log_suppress,
    .off = offsetof (SERVICE, log_suppress_mask)
  },
  /* Backward compatibility */
  {
    .name = "IgnoreCase",
    .parser = assign_dfl_ignore_case,
    .deprecated = 1,
    .message = "use the -icase matching directive flag to request case-insensitive comparison"
  },

  { NULL }
};

static int
find_service_ident (SERVICE_HEAD *svc_head, char const *name)
{
  SERVICE *svc;
  SLIST_FOREACH (svc, svc_head, next)
    {
      if (svc->name && strcmp (svc->name, name) == 0)
	return 1;
    }
  return 0;
}

static SERVICE *
new_service (BALANCER_ALGO algo)
{
  SERVICE *svc;

  XZALLOC (svc);

  service_cond_init (&svc->cond, COND_BOOL);
  DLIST_INIT (&svc->balancers);

  svc->sess_type = SESS_NONE;
  pthread_mutex_init (&svc->mut, &mutex_attr_recursive);
  svc->balancer_algo = algo;
  svc->rewrite_errors = -1;
  locus_range_init (&svc->locus);

  DLIST_INIT (&svc->be_rem_head);
  pthread_cond_init (&svc->be_rem_cond, NULL);

  return svc;
}

static int
parse_service (void *call_data, void *section_data)
{
  SERVICE_HEAD *head = call_data;
  POUND_DEFAULTS *dfl = (POUND_DEFAULTS*) section_data;
  struct token *tok;
  SERVICE *svc;

  svc = new_service (dfl->balancer_algo);

  tok = gettkn_any ();

  if (!tok)
    return CFGPARSER_FAIL;

  if (tok->type == T_STRING)
    {
      if (find_service_ident (head, tok->str))
	{
	  conf_error ("%s", "service name is not unique");
	  return CFGPARSER_FAIL;
	}
      svc->name = xstrdup (tok->str);
    }
  else
    putback_tkn (tok);

  if ((svc->sessions = session_table_new ()) == NULL)
    {
      conf_error ("%s", "session_table_new failed");
      return CFGPARSER_FAIL;
    }

  if (parser_loop (service_parsetab, svc, dfl, &svc->locus))
    return CFGPARSER_FAIL;

  SLIST_PUSH (head, svc, next);

  return CFGPARSER_OK;
}

static int
parse_acme (void *call_data, void *section_data)
{
  SERVICE_HEAD *head = call_data;
  SERVICE *svc;
  BACKEND *be;
  SERVICE_COND *cond;
  struct token *tok;
  struct stat st;
  int rc;
  static char sp_acme[] = "^/\\.well-known/acme-challenge/(.+)";
  int fd;
  struct locus_range range = LOCUS_RANGE_INITIALIZER;

  locus_point_copy (&range.beg, &last_token_locus_range ()->beg);

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    {
      locus_range_unref (&range);
      return CFGPARSER_FAIL;
    }

  if (stat (tok->str, &st))
    {
      conf_error ("can't stat %s: %s", tok->str, strerror (errno));
      locus_range_unref (&range);
      return CFGPARSER_FAIL;
    }
  if (!S_ISDIR (st.st_mode))
    {
      conf_error ("%s is not a directory: %s", tok->str, strerror (errno));
      locus_range_unref (&range);
      return CFGPARSER_FAIL;
    }
  if ((fd = open (tok->str, O_RDONLY | O_NONBLOCK | O_DIRECTORY)) == -1)
    {
      conf_error ("can't open directory %s: %s", tok->str, strerror (errno));
      locus_range_unref (&range);
      return CFGPARSER_FAIL;
    }

  /* Create service; there'll be only one backend so the balancing algorithm
     doesn't really matter. */
  svc = new_service (BALANCER_ALGO_RANDOM);

  /* Create a URL matcher */
  cond = service_cond_append (&svc->cond, COND_URL);
  rc = genpat_compile (&cond->re, GENPAT_POSIX, sp_acme, 0);
  if (rc)
    {
      conf_regcomp_error (rc, cond->re, NULL);
      return CFGPARSER_FAIL;
    }

  locus_point_copy (&range.end, &last_token_locus_range ()->end);
  svc->locus = range;

  /* Create ACME backend */
  be = xbackend_create (BE_ACME, 1, &range);
  be->service = svc;
  be->priority = 1;
  be->v.acme.wd = fd;

  /* Register backend in service */
  balancer_add_backend (balancer_list_get_normal (&svc->balancers), be);
  service_recompute_pri_unlocked (svc, NULL, NULL);

  /* Register service in the listener */
  SLIST_PUSH (head, svc, next);

  return CFGPARSER_OK;
}


static int
listener_parse_xhttp (void *call_data, void *section_data)
{
  return cfg_assign_int_range (call_data, 0, 3);
}

static int
listener_parse_checkurl (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  POUND_DEFAULTS *dfl = section_data;

  if (lst->url_pat)
    {
      conf_error ("%s", "CheckURL multiple pattern");
      return CFGPARSER_FAIL;
    }

  return parse_regex_compat (&lst->url_pat, dfl->re_type, dfl->re_type,
			     (dfl->ignore_case ? GENPAT_ICASE : 0));
}

static int
listener_parse_socket_from (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  struct token *tok;
  struct sockaddr_un *sun;
  size_t len;

  if (lst->addr_str || lst->port_str)
    {
      conf_error ("%s", "Duplicate Address or SocketFrom statement");
      return CFGPARSER_FAIL;
    }

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;

  len = strlen (tok->str);
  if (len > UNIX_PATH_MAX)
    {
      conf_error ("%s", "UNIX path name too long");
      return CFGPARSER_FAIL;
    }

  len += offsetof (struct sockaddr_un, sun_path) + 1;
  sun = xmalloc (len);
  sun->sun_family = AF_UNIX;
  strcpy (sun->sun_path, tok->str);

  lst->addr.ai_socktype = SOCK_STREAM;
  lst->addr.ai_family = AF_UNIX;
  lst->addr.ai_protocol = 0;
  lst->addr.ai_addr = (struct sockaddr *) sun;
  lst->addr.ai_addrlen = len;

  lst->socket_from = 1;
  return CFGPARSER_OK;
}

static int
parse_rewritelocation (void *call_data, void *section_data)
{
  return cfg_assign_int_range (call_data, 0, 2);
}

struct canned_log_format
{
  char *name;
  char *fmt;
};

static struct canned_log_format canned_log_format[] = {
  /* 0 - not used */
  { "null", "" },
  /* 1 - regular logging */
  { "regular", "%a %r - %>s" },
  /* 2 - extended logging (show chosen backend server as well) */
  { "extended", "%a %r - %>s (%{Host}i/%{service}N -> %{backend}N) %{f}T sec" },
  /* 3 - Apache-like format (Combined Log Format with Virtual Host) */
  { "vhost_combined", "%v:%p %a %l %u %t \"%r\" %s %b \"%{Referer}i\" \"%{User-Agent}i\"" },
  /* 4 - same as 3 but without the virtual host information */
  { "combined", "%a %l %u %t \"%r\" %s %b \"%{Referer}i\" \"%{User-Agent}i\"" },
  /* 5 - same as 3 but with information about the Service and Backend used */
  { "detailed", "%v:%p %a %l %u %t \"%r\" %s %b \"%{Referer}i\" \"%{User-Agent}i\" (%{service}N -> %{backend}N) %{f}T sec" },
};
static int max_canned_log_format =
  sizeof (canned_log_format) / sizeof (canned_log_format[0]);

struct log_format_data
{
  struct locus_range *locus;
  int fn;
  int fatal;
};

void
log_format_diag (void *data, int fatal, char const *msg, int off)
{
  struct log_format_data *ld = data;
  if (ld->fn == -1)
    {
      struct locus_range loc = *ld->locus;
      loc.beg.col += off;
      loc.end = loc.beg;
      conf_error_at_locus_range (&loc, "%s", msg);
    }
  else
    {
      conf_error_at_locus_range (ld->locus, "INTERNAL ERROR: error compiling built-in format %d", ld->fn);
      conf_error_at_locus_range (ld->locus, "%s: near %s", msg,
				 canned_log_format[ld->fn].fmt + off);
      conf_error_at_locus_range (ld->locus, "please report");
    }
  ld->fatal = fatal;
}

static void
compile_canned_formats (void)
{
  struct log_format_data ld;
  int i;

  ld.locus = NULL;
  ld.fatal = 0;

  for (i = 0; i < max_canned_log_format; i++)
    {
      ld.fn = i;
      if (http_log_format_compile (canned_log_format[i].name,
				   canned_log_format[i].fmt,
				   log_format_diag, &ld) == -1 || ld.fatal)
	exit (1);
    }
}

static int
parse_log_level (void *call_data, void *section_data)
{
  int log_level;
  int *log_level_ptr = call_data;
  struct token *tok = gettkn_expect_mask (T_BIT (T_STRING) | T_BIT (T_NUMBER));
  if (!tok)
    return CFGPARSER_FAIL;

  if (tok->type == T_STRING)
    {
      log_level = http_log_format_find (tok->str);
      if (log_level == -1)
	{
	  conf_error ("undefined format: %s", tok->str);
	  return CFGPARSER_FAIL;
	}
    }
  else
    {
      char *p;
      long n;

      errno = 0;
      n = strtol (tok->str, &p, 10);
      if (errno || *p || n < 0 || n > INT_MAX)
	{
	  conf_error ("%s", "unsupported log level number");
	  return CFGPARSER_FAIL;
	}
      if (http_log_format_check (n))
	{
	  conf_error ("%s", "undefined log level");
	  return CFGPARSER_FAIL;
	}
      log_level = n;
    }
  *log_level_ptr = log_level;
  return CFGPARSER_OK;
}

static int
parse_log_format (void *call_data, void *section_data)
{
  struct token *tok;
  char *name;
  struct log_format_data ld;
  int rc;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;
  name = strdup (tok->str);
  if ((tok = gettkn_expect (T_STRING)) == NULL)
    {
      free (name);
      return CFGPARSER_FAIL;
    }

  ld.locus = &tok->locus;
  ld.fn = -1;
  ld.fatal = 0;

  if (http_log_format_compile (name, tok->str, log_format_diag, &ld) == -1 ||
      ld.fatal)
    rc = CFGPARSER_FAIL;
  else
    rc = CFGPARSER_OK;
  free (name);
  return rc;
}

static int
parse_header_options (void *call_data, void *section_data)
{
  int *opt = call_data;
  int n;
  struct token *tok;
  static struct kwtab options[] = {
    { "forwarded", HDROPT_FORWARDED_HEADERS },
    { "ssl",       HDROPT_SSL_HEADERS },
    { "all",       HDROPT_FORWARDED_HEADERS|HDROPT_SSL_HEADERS },
    { NULL }
  };

  for (;;)
    {
      char *name;
      int neg;

      if ((tok = gettkn_any ()) == NULL)
	return CFGPARSER_FAIL;
      if (tok->type == '\n')
	break;
      if (!(tok->type == T_IDENT || tok->type == T_LITERAL))
	{
	  conf_error ("unexpected %s", token_type_str (tok->type));
	  return CFGPARSER_FAIL;
	}

      name = tok->str;
      if (c_strcasecmp (name, "none") == 0)
	*opt = 0;
      else
	{
	  if (c_strncasecmp (name, "no-", 3) == 0)
	    {
	      neg = 1;
	      name += 3;
	    }
	  else
	    neg = 0;

	  if (kw_to_tok (options, name, 1, &n))
	    {
	      conf_error ("%s", "unknown option");
	      return CFGPARSER_FAIL;
	    }

	  if (neg)
	    *opt &= ~n;
	  else
	    *opt |= n;
	}
    }

  return CFGPARSER_OK_NONL;
}

static CFGPARSER_TABLE http_common[] = {
  {
    .name = "Address",
    .parser = assign_address_string,
    .off = offsetof (LISTENER, addr_str)
  },
  {
    .name = "Port",
    .parser = assign_port_string,
    .off = offsetof (LISTENER, port_str)
  },
  {
    .name = "SocketFrom",
    .parser = listener_parse_socket_from
  },
  {
    .name = "xHTTP",
    .parser = listener_parse_xhttp,
    .off = offsetof (LISTENER, verb)
  },
  {
    .name = "Client",
    .parser = cfg_assign_timeout,
    .off = offsetof (LISTENER, to)
  },
  {
    .name = "CheckURL",
    .parser = listener_parse_checkurl
  },
  {
    .name = "ErrorFile",
    .parser = parse_errorfile,
    .off = offsetof (LISTENER, http_err)
  },
  {
    .name = "MaxRequest",
    .parser = assign_CONTENT_LENGTH,
    .off = offsetof (LISTENER, max_req_size)
  },
  {
    .name = "MaxURI",
    .parser = cfg_assign_unsigned,
    .off = offsetof (LISTENER, max_uri_length)
  },

  {
    .name = "Rewrite",
    .parser = parse_rewrite,
    .off = offsetof (LISTENER, rewrite)
  },

  {
    .name = "RewriteErrors",
    .parser =  cfg_assign_bool,
    .off = offsetof (LISTENER, rewrite_errors)
  },
  {
    .name = "SetHeader",
    .parser = SETFN_SVC_NAME (set_header),
    .off = offsetof (LISTENER, rewrite)
  },
  {
    .name = "HeaderAdd",
    .type = KWT_ALIAS,
    .deprecated = 1
  },
  {
    .name = "AddHeader",
    .type = KWT_ALIAS,
    .deprecated = 1
  },
  {
    .name = "DeleteHeader",
    .parser = SETFN_SVC_NAME (delete_header),
    .off = offsetof (LISTENER, rewrite)
  },
  {
    .name = "HeaderRemove",
    .parser = parse_header_remove,
    .off = offsetof (LISTENER, rewrite),
    .deprecated = 1,
    .message = "use \"DeleteHeader\" instead"
  },
  {
    .name = "HeadRemove",
    .type = KWT_ALIAS,
    .deprecated = 1,
    .message = "use \"DeleteHeader\" instead"
  },
  {
    .name = "SetURL",
    .parser = SETFN_SVC_NAME (set_url),
    .off = offsetof (LISTENER, rewrite)
  },
  {
    .name = "SetPath",
    .parser = SETFN_SVC_NAME (set_path),
    .off = offsetof (LISTENER, rewrite)
  },
  {
    .name = "DeleteQuery",
    .parser = SETFN_SVC_NAME (delete_query),
    .off = offsetof (LISTENER, rewrite)
  },
  {
    .name = "SetQuery",
    .parser = SETFN_SVC_NAME (set_query),
    .off = offsetof (LISTENER, rewrite)
  },
  {
    .name = "SetQueryParam",
    .parser = SETFN_SVC_NAME (set_query_param),
    .off = offsetof (LISTENER, rewrite)
  },
  {
    .name = "LuaModify",
    .parser = SETFN_SVC_NAME (lua_modify),
    .off = offsetof (LISTENER, rewrite)
  },
  {
    .name = "HeaderOption",
    .parser = parse_header_options,
    .off = offsetof (LISTENER, header_options)
  },

  {
    .name = "RewriteLocation",
    .parser = parse_rewritelocation,
    .off = offsetof (LISTENER, rewr_loc)
  },
  {
    .name = "RewriteDestination",
    .parser = cfg_assign_bool,
    .off = offsetof (LISTENER, rewr_dest)
  },
  {
    .name = "LogLevel",
    .parser = parse_log_level,
    .off = offsetof (LISTENER, log_level)
  },
  {
    .name = "ForwardedHeader",
    .parser = cfg_assign_string,
    .off = offsetof (LISTENER, forwarded_header)
  },
  {
    .name = "TrustedIP",
    .parser = assign_acl,
    .off = offsetof (LISTENER, trusted_ips)
  },
  {
    .name = "Service",
    .parser = parse_service,
    .off = offsetof (LISTENER, services)
  },
  {
    .name = "LineBufferSize",
    .parser = assign_bufsize,
    .off = offsetof (LISTENER, linebufsize),
  },
  { NULL }
};

static CFGPARSER_TABLE http_deprecated[] = {
  /* Backward compatibility */
  {
    .name = "Err400",
    .parser = parse_errN,
    .off = offsetof (LISTENER, http_err[HTTP_STATUS_BAD_REQUEST]),
    .deprecated = 1,
    .message = "use \"ErrorFile 400\" instead"
  },
  {
    .name = "Err401",
    .parser = parse_errN,
    .off = offsetof (LISTENER, http_err[HTTP_STATUS_UNAUTHORIZED]),
    .deprecated = 1,
    .message = "use \"ErrorFile 401\" instead"
  },
  {
    .name = "Err403",
    .parser = parse_errN,
    .off = offsetof (LISTENER, http_err[HTTP_STATUS_FORBIDDEN]),
    .deprecated = 1,
    .message = "use \"ErrorFile 403\" instead"
  },
  {
    .name = "Err404",
    .parser = parse_errN,
    .off = offsetof (LISTENER, http_err[HTTP_STATUS_NOT_FOUND]),
    .deprecated = 1,
    .message = "use \"ErrorFile 404\" instead"
  },
  {
    .name = "Err413",
    .parser = parse_errN,
    .off = offsetof (LISTENER, http_err[HTTP_STATUS_PAYLOAD_TOO_LARGE]),
    .deprecated = 1,
    .message = "use \"ErrorFile 413\" instead"
  },
  {
    .name = "Err414",
    .parser = parse_errN,
    .off = offsetof (LISTENER, http_err[HTTP_STATUS_URI_TOO_LONG]),
    .deprecated = 1,
    .message = "use \"ErrorFile 414\" instead"
  },
  {
    .name = "Err500",
    .parser = parse_errN,
    .off = offsetof (LISTENER, http_err[HTTP_STATUS_INTERNAL_SERVER_ERROR]),
    .deprecated = 1,
    .message = "use \"ErrorFile 500\" instead"
  },
  {
    .name = "Err501",
    .parser = parse_errN,
    .off = offsetof (LISTENER, http_err[HTTP_STATUS_NOT_IMPLEMENTED]),
    .deprecated = 1,
    .message = "use \"ErrorFile 501\" instead"
  },
  {
    .name = "Err503",
    .parser = parse_errN,
    .off = offsetof (LISTENER, http_err[HTTP_STATUS_SERVICE_UNAVAILABLE]),
    .deprecated = 1,
    .message = "use \"ErrorFile 503\" instead"
  },

  { NULL }
};

static CFGPARSER_TABLE http_parsetab[] = {
  {
    .name = "End",
    .parser = cfg_parse_end
  },
  {
    .name = "",
    .type = KWT_TABREF,
    .ref = http_common
  },
  {
    .name = "",
    .type = KWT_TABREF,
    .ref = http_deprecated
  },
  {
    .name = "ACME",
    .parser = parse_acme,
    .off = offsetof (LISTENER, services)
  },

  { NULL }
};

static LISTENER *
listener_alloc (POUND_DEFAULTS *dfl)
{
  LISTENER *lst;

  XZALLOC (lst);

  lst->mode = 0600;
  lst->sock = -1;
  lst->to = dfl->clnt_to;
  lst->rewr_loc = 1;
  lst->log_level = dfl->log_level;
  lst->rewrite_errors = -1;
  lst->verb = 0;
  lst->header_options = dfl->header_options;
  lst->clnt_check = -1;
  lst->linebufsize = dfl->linebufsize;
  locus_range_init (&lst->locus);
  SLIST_INIT (&lst->rewrite[REWRITE_REQUEST]);
  SLIST_INIT (&lst->rewrite[REWRITE_RESPONSE]);
  SLIST_INIT (&lst->services);
  SLIST_INIT (&lst->ctx_head);
  return lst;
}

static int
find_listener_ident (LISTENER_HEAD *list_head, char const *name)
{
  LISTENER *lstn;
  SLIST_FOREACH (lstn, list_head, next)
    {
      if (lstn->name && strcmp (lstn->name, name) == 0)
	return 1;
    }
  return 0;
}

static int
foreach_client_cert (SERVICE_COND *cond, int (*cb) (X509 *, void *), void *data)
{
  int rc = 0;

  switch (cond->type)
    {
    case COND_CLIENT_CERT:
      if (cb)
	rc = cb (cond->x509, data);
      else
	rc = 1;
      break;

    case COND_BOOL:
      {
	SERVICE_COND *subcond;
	SLIST_FOREACH (subcond, &cond->boolean.head, next)
	  {
	    if ((rc = foreach_client_cert (subcond, cb, data)) != 0)
	      break;
	  }
      }
      break;

    default:
      break;
    }
  return rc;
}

static int
forbid_ssl_usage (SERVICE_HEAD *s_head, char const *msg)
{
  SERVICE *svc;
  SLIST_FOREACH (svc, s_head, next)
    {
      if (foreach_client_cert (&svc->cond, NULL, NULL))
	{
	  conf_error_at_locus_range (&svc->locus, "%s", msg);
	  return 1;
	}
    }
  return 0;
}

static int
resolve_listener_address (LISTENER *lst, char *defsrv)
{
  if (lst->addr.ai_addr == NULL)
    {
      struct addrinfo hints, *res, *ptr;
      char *service;
      int rc;

      memset (&hints, 0, sizeof (hints));
      hints.ai_family = AF_UNSPEC;
      hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;
      hints.ai_socktype = SOCK_STREAM;
      service = lst->port_str ? lst->port_str : defsrv;
      rc = getaddrinfo (lst->addr_str, service, &hints, &res);
      if (rc != 0)
	{
	  conf_error_at_locus_range (&lst->locus, "bad listener address: %s",
				     gai_strerror (rc));
	  return CFGPARSER_FAIL;
	}

      ptr = res;
      /* Prefer in6addr_any over INADDR_ANY. */
      if (ptr->ai_family == AF_INET &&
	  ((struct sockaddr_in*)ptr->ai_addr)->sin_addr.s_addr == INADDR_ANY &&
	  ptr->ai_next != NULL &&
	  ptr->ai_next->ai_family == AF_INET6 &&
	  memcmp (&((struct sockaddr_in6*)ptr->ai_next->ai_addr)->sin6_addr,
		  &in6addr_any, sizeof (in6addr_any)) == 0)
	ptr = ptr->ai_next;

      lst->addr = *ptr;
      lst->addr.ai_next = NULL;
      lst->addr.ai_addr = xmalloc (ptr->ai_addrlen);
      memcpy (lst->addr.ai_addr, ptr->ai_addr, ptr->ai_addrlen);
      freeaddrinfo (res);
    }
  return CFGPARSER_OK;
}

static int
parse_listen_http (void *call_data, void *section_data)
{
  LISTENER *lst;
  LISTENER_HEAD *list_head = call_data;
  POUND_DEFAULTS *dfl = section_data;
  struct token *tok;

  if ((lst = listener_alloc (dfl)) == NULL)
    return CFGPARSER_FAIL;

  if ((tok = gettkn_any ()) == NULL)
    return CFGPARSER_FAIL;
  else if (tok->type == T_STRING)
    {
      if (find_listener_ident (list_head, tok->str))
	{
	  conf_error ("%s", "listener name is not unique");
	  return CFGPARSER_FAIL;
	}
      lst->name = xstrdup (tok->str);
    }
  else
    putback_tkn (tok);

  if (parser_loop (http_parsetab, lst, section_data, &lst->locus))
    return CFGPARSER_FAIL;

  if (resolve_listener_address (lst, PORT_HTTP_STR) != CFGPARSER_OK)
    return CFGPARSER_FAIL;

  if (forbid_ssl_usage (&lst->services,
			"use of SSL features in ListenHTTP sections"
			" is forbidden"))
    return CFGPARSER_FAIL;
  if (lst->max_uri_length > lst->linebufsize)
    lst->max_uri_length = lst->linebufsize;

  SLIST_PUSH (list_head, lst, next);
  return CFGPARSER_OK;
}

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
# define general_name_string(n) \
	xstrndup ((char*)ASN1_STRING_get0_data (n->d.dNSName),	\
		 ASN1_STRING_length (n->d.dNSName) + 1)
#else
# define general_name_string(n) \
	xstrndup ((char*)ASN1_STRING_data(n->d.dNSName),	\
		 ASN1_STRING_length (n->d.dNSName) + 1)
#endif

static void
get_subjectaltnames (X509 *x509, POUND_CTX *pc, size_t san_max)
{
  STACK_OF (GENERAL_NAME) * san_stack =
    (STACK_OF (GENERAL_NAME) *) X509_get_ext_d2i (x509, NID_subject_alt_name,
						  NULL, NULL);
  char **result;

  if (san_stack == NULL)
    return;
  while (sk_GENERAL_NAME_num (san_stack) > 0)
    {
      GENERAL_NAME *name = sk_GENERAL_NAME_pop (san_stack);
      switch (name->type)
	{
	case GEN_DNS:
	  if (pc->subjectAltNameCount == san_max)
	    pc->subjectAltNames = x2nrealloc (pc->subjectAltNames,
					      &san_max,
					      sizeof (pc->subjectAltNames[0]));
	  pc->subjectAltNames[pc->subjectAltNameCount++] = general_name_string (name);
	  break;

	default:
	  logmsg (LOG_INFO, "unsupported subjectAltName type encountered: %i",
		  name->type);
	}
      GENERAL_NAME_free (name);
    }

  sk_GENERAL_NAME_pop_free (san_stack, GENERAL_NAME_free);
  if (pc->subjectAltNameCount
      && (result = realloc (pc->subjectAltNames,
			    pc->subjectAltNameCount * sizeof (pc->subjectAltNames[0]))) != NULL)
    pc->subjectAltNames = result;
}

static int
load_cert (char const *filename, LISTENER *lst)
{
  POUND_CTX *pc;

  XZALLOC (pc);

  if ((pc->ctx = SSL_CTX_new (SSLv23_server_method ())) == NULL)
    {
      conf_openssl_error (NULL, "SSL_CTX_new");
      return CFGPARSER_FAIL;
    }

  if (SSL_CTX_use_certificate_chain_file (pc->ctx, filename) != 1)
    {
      conf_openssl_error (filename, "SSL_CTX_use_certificate_chain_file");
      return CFGPARSER_FAIL;
    }
  if (SSL_CTX_use_PrivateKey_file (pc->ctx, filename, SSL_FILETYPE_PEM) != 1)
    {
      conf_openssl_error (filename, "SSL_CTX_use_PrivateKey_file");
      return CFGPARSER_FAIL;
    }

  if (SSL_CTX_check_private_key (pc->ctx) != 1)
    {
      conf_openssl_error (filename, "SSL_CTX_check_private_key");
      return CFGPARSER_FAIL;
    }

#ifdef SSL_CTRL_SET_TLSEXT_SERVERNAME_CB
  {
    /* we have support for SNI */
    FILE *fcert;
    X509 *x509;
    X509_NAME *xname = NULL;
    int i;
    size_t san_max;

    if ((fcert = fopen (filename, "r")) == NULL)
      {
	conf_error ("%s: could not open certificate file: %s", filename,
		    strerror (errno));
	return CFGPARSER_FAIL;
      }

    x509 = PEM_read_X509 (fcert, NULL, NULL, NULL);
    fclose (fcert);

    if (!x509)
      {
	conf_error ("%s: could not get certificate subject", filename);
	return CFGPARSER_FAIL;
      }

    pc->subjectAltNameCount = 0;
    pc->subjectAltNames = NULL;
    san_max = 0;

    /* Extract server name */
    xname = X509_get_subject_name (x509);
    for (i = -1;
	 (i = X509_NAME_get_index_by_NID (xname, NID_commonName, i)) != -1;)
      {
	X509_NAME_ENTRY *entry = X509_NAME_get_entry (xname, i);
	ASN1_STRING *value;
	char *str = NULL;
	value = X509_NAME_ENTRY_get_data (entry);
	if (ASN1_STRING_to_UTF8 ((unsigned char **)&str, value) >= 0)
	  {
	    if (pc->server_name == NULL)
	      pc->server_name = str;
	    else
	      {
		if (pc->subjectAltNameCount == san_max)
		  pc->subjectAltNames = x2nrealloc (pc->subjectAltNames,
						    &san_max,
						    sizeof (pc->subjectAltNames[0]));
		pc->subjectAltNames[pc->subjectAltNameCount++] = str;
	      }
	  }
      }

    get_subjectaltnames (x509, pc, san_max);
    X509_free (x509);

    if (pc->server_name == NULL)
      {
	conf_error ("%s: no CN in certificate subject name", filename);
	return CFGPARSER_FAIL;
      }
  }
#else
  if (res->ctx)
    conf_error ("%s: multiple certificates not supported", filename);
#endif
  SLIST_PUSH (&lst->ctx_head, pc, next);

  return CFGPARSER_OK;
}

static int
https_parse_cert (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  struct token *tok;
  struct stat st;
  char *certname;
  int rc = CFGPARSER_OK;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;

  if ((certname = filename_resolve (tok->str)) == NULL)
    return CFGPARSER_FAIL;

  if (stat (certname, &st))
    {
      conf_error ("%s: stat error: %s", certname, strerror (errno));
      return CFGPARSER_FAIL;
    }

  if (S_ISREG (st.st_mode))
    {
      rc = load_cert (certname, lst);
    }
  else if (S_ISDIR (st.st_mode))
    {
      DIR *dp;
      struct dirent *ent;
      struct stringbuf namebuf;
      size_t dirlen;

      dirlen = strlen (certname);
      while (dirlen > 0 && certname[dirlen-1] == '/')
	dirlen--;

      xstringbuf_init (&namebuf);
      stringbuf_add (&namebuf, certname, dirlen);
      stringbuf_add_char (&namebuf, '/');
      dirlen++;

      dp = opendir (certname);
      if (dp == NULL)
	{
	  conf_error ("%s: error opening directory: %s", certname,
		      strerror (errno));
	  stringbuf_free (&namebuf);
	  return CFGPARSER_FAIL;
	}

      while ((ent = readdir (dp)) != NULL)
	{
	  char *filename;

	  if (strcmp (ent->d_name, ".") == 0 || strcmp (ent->d_name, "..") == 0)
	    continue;

	  stringbuf_add_string (&namebuf, ent->d_name);
	  filename = stringbuf_finish (&namebuf);
	  if (stat (filename, &st))
	    {
	      conf_error ("%s: stat error: %s", filename, strerror (errno));
	    }
	  else if (S_ISREG (st.st_mode))
	    {
	      if ((rc = load_cert (filename, lst)) != CFGPARSER_OK)
		break;
	    }
	  else
	    conf_error ("warning: ignoring %s: not a regular file", filename);
	  stringbuf_truncate (&namebuf, dirlen);
	}
      closedir (dp);
      stringbuf_free (&namebuf);
    }
  else
    conf_error ("%s: not a regular file or directory", certname);
  free (certname);
  return rc;
}

static int
verify_OK (int pre_ok, X509_STORE_CTX *ctx)
{
  return 1;
}

#ifdef SSL_CTRL_SET_TLSEXT_SERVERNAME_CB
static int
SNI_server_name (SSL *ssl, int *dummy, POUND_CTX_HEAD *ctx_head)
{
  const char *server_name;
  POUND_CTX *pc;

  if ((server_name = SSL_get_servername (ssl, TLSEXT_NAMETYPE_host_name)) == NULL)
    return SSL_TLSEXT_ERR_NOACK;

  /* logmsg(LOG_DEBUG, "Received SSL SNI Header for servername %s", servername); */

  SSL_set_SSL_CTX (ssl, NULL);
  SLIST_FOREACH (pc, ctx_head, next)
    {
      if (fnmatch (pc->server_name, server_name, 0) == 0)
	{
	  /* logmsg(LOG_DEBUG, "Found cert for %s", servername); */
	  SSL_set_SSL_CTX (ssl, pc->ctx);
	  return SSL_TLSEXT_ERR_OK;
	}
      else if (pc->subjectAltNameCount > 0 && pc->subjectAltNames != NULL)
	{
	  int i;

	  for (i = 0; i < pc->subjectAltNameCount; i++)
	    {
	      if (fnmatch ((char *) pc->subjectAltNames[i], server_name, 0) ==
		  0)
		{
		  SSL_set_SSL_CTX (ssl, pc->ctx);
		  return SSL_TLSEXT_ERR_OK;
		}
	    }
	}
    }

  /* logmsg(LOG_DEBUG, "No match for %s, default used", server_name); */
  SSL_set_SSL_CTX (ssl, SLIST_FIRST (ctx_head)->ctx);
  return SSL_TLSEXT_ERR_OK;
}
#endif

static int
https_parse_client_cert (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  int depth;
  POUND_CTX *pc;

  if (SLIST_EMPTY (&lst->ctx_head))
    {
      conf_error ("%s", "ClientCert may only be used after Cert");
      return CFGPARSER_FAIL;
    }

  if (cfg_assign_int_range (&lst->clnt_check, 0, 3) != CFGPARSER_OK)
    return CFGPARSER_FAIL;

  if (lst->clnt_check > 0 && cfg_assign_int (&depth, NULL) != CFGPARSER_OK)
    return CFGPARSER_FAIL;

  switch (lst->clnt_check)
    {
    case 0:
      /* don't ask */
      SLIST_FOREACH (pc, &lst->ctx_head, next)
	SSL_CTX_set_verify (pc->ctx, SSL_VERIFY_NONE, NULL);
      break;

    case 1:
      /* ask but OK if no client certificate */
      SLIST_FOREACH (pc, &lst->ctx_head, next)
	{
	  SSL_CTX_set_verify (pc->ctx,
			      SSL_VERIFY_PEER |
			      SSL_VERIFY_CLIENT_ONCE, NULL);
	  SSL_CTX_set_verify_depth (pc->ctx, depth);
	}
      break;

    case 2:
      /* ask and fail if no client certificate */
      SLIST_FOREACH (pc, &lst->ctx_head, next)
	{
	  SSL_CTX_set_verify (pc->ctx,
			      SSL_VERIFY_PEER |
			      SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
	  SSL_CTX_set_verify_depth (pc->ctx, depth);
	}
      break;

    case 3:
      /* ask but do not verify client certificate */
      SLIST_FOREACH (pc, &lst->ctx_head, next)
	{
	  SSL_CTX_set_verify (pc->ctx,
			      SSL_VERIFY_PEER |
			      SSL_VERIFY_CLIENT_ONCE, verify_OK);
	  SSL_CTX_set_verify_depth (pc->ctx, depth);
	}
      break;
    }
  return CFGPARSER_OK;
}

static int
https_parse_disable (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  return set_proto_opt (&lst->ssl_op_enable);
}

static int
https_parse_ciphers (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  struct token *tok;
  POUND_CTX *pc;

  if (SLIST_EMPTY (&lst->ctx_head))
    {
      conf_error ("%s", "Ciphers may only be used after Cert");
      return CFGPARSER_FAIL;
    }

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;

  SLIST_FOREACH (pc, &lst->ctx_head, next)
    SSL_CTX_set_cipher_list (pc->ctx, tok->str);

  return CFGPARSER_OK;
}

static int
https_parse_honor_cipher_order (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  int bv;

  if (cfg_assign_bool (&bv, NULL) != CFGPARSER_OK)
    return CFGPARSER_FAIL;

  if (bv)
    {
      lst->ssl_op_enable |= SSL_OP_CIPHER_SERVER_PREFERENCE;
      lst->ssl_op_disable &= ~SSL_OP_CIPHER_SERVER_PREFERENCE;
    }
  else
    {
      lst->ssl_op_disable |= SSL_OP_CIPHER_SERVER_PREFERENCE;
      lst->ssl_op_enable &= ~SSL_OP_CIPHER_SERVER_PREFERENCE;
    }

  return CFGPARSER_OK;
}

static int
https_parse_allow_client_renegotiation (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;

  if (cfg_assign_int_range (&lst->allow_client_reneg, 0, 2) != CFGPARSER_OK)
    return CFGPARSER_FAIL;

  if (lst->allow_client_reneg == 2)
    {
      lst->ssl_op_enable |= SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION;
      lst->ssl_op_disable &= ~SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION;
    }
  else
    {
      lst->ssl_op_disable |= SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION;
      lst->ssl_op_enable &= ~SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION;
    }

  return CFGPARSER_OK;
}

static int
https_parse_calist (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  STACK_OF (X509_NAME) *cert_names;
  struct token *tok;
  POUND_CTX *pc;

  if (SLIST_EMPTY (&lst->ctx_head))
    {
      conf_error ("%s", "CAList may only be used after Cert");
      return CFGPARSER_FAIL;
    }

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;

  if ((cert_names = SSL_load_client_CA_file (tok->str)) == NULL)
    {
      conf_openssl_error (NULL, "SSL_load_client_CA_file");
      return CFGPARSER_FAIL;
    }

  SLIST_FOREACH (pc, &lst->ctx_head, next)
    SSL_CTX_set_client_CA_list (pc->ctx, cert_names);

  return CFGPARSER_OK;
}

static int
https_parse_verifylist (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  struct token *tok;
  POUND_CTX *pc;

  if (SLIST_EMPTY (&lst->ctx_head))
    {
      conf_error ("%s", "VerifyList may only be used after Cert");
      return CFGPARSER_FAIL;
    }

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;

  SLIST_FOREACH (pc, &lst->ctx_head, next)
    if (SSL_CTX_load_verify_locations (pc->ctx, tok->str, NULL) != 1)
      {
	conf_openssl_error (tok->str, "SSL_CTX_load_verify_locations");
	return CFGPARSER_FAIL;
      }

  return CFGPARSER_OK;
}

static int
https_parse_crlist (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  struct token *tok;
  X509_STORE *store;
  X509_LOOKUP *lookup;
  POUND_CTX *pc;

  if (SLIST_EMPTY (&lst->ctx_head))
    {
      conf_error ("%s", "CRlist may only be used after Cert");
      return CFGPARSER_FAIL;
    }

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;

  SLIST_FOREACH (pc, &lst->ctx_head, next)
    {
      store = SSL_CTX_get_cert_store (pc->ctx);
      if ((lookup = X509_STORE_add_lookup (store, X509_LOOKUP_file ())) == NULL)
	{
	  conf_openssl_error (NULL, "X509_STORE_add_lookup");
	  return CFGPARSER_FAIL;
	}

      if (X509_load_crl_file (lookup, tok->str, X509_FILETYPE_PEM) != 1)
	{
	  conf_openssl_error (tok->str, "X509_load_crl_file failed");
	  return CFGPARSER_FAIL;
	}

      X509_STORE_set_flags (store, X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
    }

  return CFGPARSER_OK;
}

static int
https_parse_nohttps11 (void *call_data, void *section_data)
{
  LISTENER *lst = call_data;
  return cfg_assign_int_range (&lst->noHTTPS11, 0, 2);
}

static CFGPARSER_TABLE https_parsetab[] = {
  {
    .name = "End",
    .parser = cfg_parse_end
  },

  {
    .name = "",
    .type = KWT_TABREF,
    .ref = http_common
  },
  {
    .name = "",
    .type = KWT_TABREF,
    .ref = http_deprecated
  },

  {
    .name = "Cert",
    .parser = https_parse_cert
  },
  {
    .name = "ClientCert",
    .parser = https_parse_client_cert
  },
  {
    .name = "Disable",
    .parser = https_parse_disable
  },
  {
    .name = "Ciphers",
    .parser = https_parse_ciphers
  },
  {
    .name = "SSLHonorCipherOrder",
    .parser = https_parse_honor_cipher_order
  },
  {
    .name = "SSLAllowClientRenegotiation",
    .parser = https_parse_allow_client_renegotiation
  },
  {
    .name = "CAlist",
    .parser = https_parse_calist
  },
  {
    .name = "VerifyList",
    .parser = https_parse_verifylist
  },
  {
    .name = "CRLlist",
    .parser = https_parse_crlist
  },
  {
    .name = "NoHTTPS11",
    .parser = https_parse_nohttps11
  },

  { NULL }
};

static int
client_cert_cb (X509 *x509, void *data)
{
  LISTENER *lst = data;
  POUND_CTX *pc;

  SLIST_FOREACH (pc, &lst->ctx_head, next)
    {
      X509_STORE *store = SSL_CTX_get_cert_store (pc->ctx);
      if (X509_STORE_add_cert (store, x509) != 1)
	{
	  openssl_error_at_locus_range (NULL, NULL, "X509_STORE_add_cert");
	  return -1;
	}
    }
  lst->verify = 1;
  return 0;
}


static int
flush_service_client_cert (LISTENER *lst)
{
  SERVICE *svc;
  SLIST_FOREACH (svc, &lst->services, next)
    {
      if (foreach_client_cert (&svc->cond, client_cert_cb, lst))
	return -1;
    }
  if (lst->verify == 1)
    {
      if (lst->clnt_check != -1)
	{
	  conf_error_at_locus_range (&lst->locus,
				     "ClientCert in ListenHTTPS"
				     " conflicts with that in Service");
	  return -1;
	}
      else
	{
	  POUND_CTX *pc;
	  SLIST_FOREACH (pc, &lst->ctx_head, next)
	    {
	      SSL_CTX_set_verify (pc->ctx,
				  SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE,
				  verify_OK);
	    }
	  lst->clnt_check = 3;
	}
    }
  return CFGPARSER_OK;
}

static int
parse_listen_https (void *call_data, void *section_data)
{
  LISTENER *lst;
  LISTENER_HEAD *list_head = call_data;
  POUND_DEFAULTS *dfl = section_data;
  POUND_CTX *pc;
  struct stringbuf sb;
  struct token *tok;

  if ((lst = listener_alloc (dfl)) == NULL)
    return CFGPARSER_FAIL;

  if ((tok = gettkn_any ()) == NULL)
    return CFGPARSER_FAIL;
  else if (tok->type == T_STRING)
    {
      if (find_listener_ident (list_head, tok->str))
	{
	  conf_error ("%s", "listener name is not unique");
	  return CFGPARSER_FAIL;
	}
      lst->name = xstrdup (tok->str);
    }
  else
    putback_tkn (tok);

  lst->ssl_op_enable = SSL_OP_ALL;
#ifdef  SSL_OP_NO_COMPRESSION
  lst->ssl_op_enable |= SSL_OP_NO_COMPRESSION;
#endif
  lst->ssl_op_disable =
    SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION | SSL_OP_LEGACY_SERVER_CONNECT |
    SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS;

  if (parser_loop (https_parsetab, lst, section_data, &lst->locus))
    return CFGPARSER_FAIL;

  if (resolve_listener_address (lst, PORT_HTTPS_STR) != CFGPARSER_OK)
    return CFGPARSER_FAIL;

  if (lst->max_uri_length > lst->linebufsize)
    lst->max_uri_length = lst->linebufsize;

  if (SLIST_EMPTY (&lst->ctx_head))
    {
      conf_error_at_locus_range (&lst->locus, "Cert statement is missing");
      return CFGPARSER_FAIL;
    }

  if (flush_service_client_cert (lst) != CFGPARSER_OK)
    {
      return CFGPARSER_FAIL;
    }

#ifdef SSL_CTRL_SET_TLSEXT_SERVERNAME_CB
  if (!SLIST_EMPTY (&lst->ctx_head))
    {
      SSL_CTX *ctx = SLIST_FIRST (&lst->ctx_head)->ctx;
      if (!SSL_CTX_set_tlsext_servername_callback (ctx, SNI_server_name)
	  || !SSL_CTX_set_tlsext_servername_arg (ctx, &lst->ctx_head))
	{
	  conf_openssl_error (NULL, "can't set SNI callback");
	  return CFGPARSER_FAIL;
	}
    }
#endif

  xstringbuf_init (&sb);
  SLIST_FOREACH (pc, &lst->ctx_head, next)
    {
      SSL_CTX_set_app_data (pc->ctx, lst);
      SSL_CTX_set_mode (pc->ctx, SSL_MODE_AUTO_RETRY);
      SSL_CTX_set_options (pc->ctx, lst->ssl_op_enable);
      SSL_CTX_clear_options (pc->ctx, lst->ssl_op_disable);
      stringbuf_reset (&sb);
      stringbuf_printf (&sb, "%d-Pound-%ld", getpid (), random ());
      SSL_CTX_set_session_id_context (pc->ctx, (unsigned char *) sb.base,
				      sb.len);
      POUND_SSL_CTX_init (pc->ctx);
      SSL_CTX_set_info_callback (pc->ctx, SSLINFO_callback);
    }
  stringbuf_free (&sb);

  SLIST_PUSH (list_head, lst, next);
  return CFGPARSER_OK;
}

static int
parse_threads_compat (void *call_data, void *section_data)
{
  int rc;
  unsigned n;

  if ((rc = cfg_assign_unsigned (&n, section_data)) != CFGPARSER_OK)
    return rc;

  worker_min_count = worker_max_count = n;

  return CFGPARSER_OK;
}

static int
parse_control_socket (void *call_data, void *section_data)
{
  struct addrinfo *addr = call_data;
  struct token *tok;
  struct sockaddr_un *sun;
  size_t len;

  /* Get socket address */
  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;

  len = strlen (tok->str);
  if (len > UNIX_PATH_MAX)
    {
      conf_error_at_locus_range (&tok->locus,
				 "%s", "UNIX path name too long");
      return CFGPARSER_FAIL;
    }

  len += offsetof (struct sockaddr_un, sun_path) + 1;
  sun = xmalloc (len);
  sun->sun_family = AF_UNIX;
  strcpy (sun->sun_path, tok->str);
  unlink_at_exit (sun->sun_path);

  addr->ai_socktype = SOCK_STREAM;
  addr->ai_family = AF_UNIX;
  addr->ai_protocol = 0;
  addr->ai_addr = (struct sockaddr *) sun;
  addr->ai_addrlen = len;

  return CFGPARSER_OK;
}

static CFGPARSER_TABLE control_parsetab[] = {
  {
    .name = "End",
    .parser = cfg_parse_end
  },
  {
    .name = "Socket",
    .parser = parse_control_socket,
    .off = offsetof (LISTENER, addr)
  },
  {
    .name = "ChangeOwner",
    .parser = cfg_assign_bool,
    .off = offsetof (LISTENER, chowner)
  },
  {
    .name = "Mode",
    .parser = cfg_assign_mode,
    .off = offsetof (LISTENER, mode)
  },
  { NULL }
};

static int
parse_control_listener (void *call_data, void *section_data)
{
  struct token *tok;
  LISTENER *lst;
  SERVICE *svc;
  BACKEND *be;
  int rc;

  if ((tok = gettkn_any ()) == NULL)
    return CFGPARSER_FAIL;
  lst = listener_alloc (section_data);
  switch (tok->type)
    {
    case '\n':
      rc = parser_loop (control_parsetab, lst, section_data, &lst->locus);
      if (rc == CFGPARSER_OK)
	{
	  if (lst->addr.ai_addrlen == 0)
	    {
	      conf_error_at_locus_range (&lst->locus, "%s",
					 "Socket statement is missing");
	      rc = CFGPARSER_FAIL;
	    }
	}
      break;

    case T_STRING:
      locus_point_copy (&lst->locus.beg, &last_token_locus_range ()->beg);
      putback_tkn (tok);
      rc = parse_control_socket (&lst->addr, section_data);
      locus_point_copy (&lst->locus.end, &last_token_locus_range ()->end);
      break;

    default:
      conf_error ("expected string or newline, but found %s",
		  token_type_str (tok->type));
      rc = CFGPARSER_FAIL;
    }

  if (rc != CFGPARSER_OK)
    return CFGPARSER_FAIL;

  lst->verb = 1; /* Need PUT and DELETE methods */
  /* Register listener in the global listener list */
  SLIST_PUSH (&listeners, lst, next);

  /* Create service; there'll be only one backend so the balancing algorithm
     doesn't really matter. */
  svc = new_service (BALANCER_ALGO_RANDOM);
  locus_range_copy (&svc->locus, &lst->locus);

  /* Register service in the listener */
  SLIST_PUSH (&lst->services, svc, next);

  /* Create backend */
  be = xbackend_create (BE_CONTROL, 1, &lst->locus);
  be->service = svc;
  /* Register backend in service */
  balancer_add_backend (balancer_list_get_normal (&svc->balancers), be);
  service_recompute_pri_unlocked (svc, NULL, NULL);

  return CFGPARSER_OK;
}

static int
parse_named_backend (void *call_data, void *section_data)
{
  NAMED_BACKEND_TABLE *tab = call_data;
  struct token *tok;
  BACKEND *be;
  struct locus_range range = LOCUS_RANGE_INITIALIZER;
  NAMED_BACKEND *olddef;
  char *name;

  locus_point_copy (&range.beg, &last_token_locus_range ()->beg);

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;

  name = xstrdup (tok->str);

  be = parse_backend_internal (backend_parsetab, section_data);
  if (!be)
    return CFGPARSER_FAIL;
  locus_point_copy (&range.end, &last_token_locus_range ()->end);
  locus_range_copy (&be->locus, &range);

  olddef = named_backend_insert (tab, name, be);
  free (name);
  pthread_mutex_destroy (&be->mut);
  locus_range_unref (&be->locus);
  free (be);

  if (olddef)
    {
      conf_error_at_locus_range (&range, "redefinition of named backend %s",
				 olddef->name);
      conf_error_at_locus_range (&olddef->locus,
				 "original definition was here");
      return CFGPARSER_FAIL;
    }

  locus_range_unref (&range);
  return CFGPARSER_OK;
}

static int
parse_combine_headers (void *call_data, void *section_data)
{
  struct token *tok;

  if ((tok = gettkn_any ()) == NULL)
    return CFGPARSER_FAIL;

  if (tok->type != '\n')
    {
      conf_error ("expected newline, but found %s", token_type_str (tok->type));
      return CFGPARSER_FAIL;
    }

  for (;;)
    {
      int rc;
      if ((tok = gettkn_any ()) == NULL)
	return CFGPARSER_FAIL;
      if (tok->type == '\n')
	continue;
      if (tok->type == T_IDENT)
	{
	  if (c_strcasecmp (tok->str, "end") == 0)
	    break;
	  if (c_strcasecmp (tok->str, "include") == 0)
	    {
	      if ((rc = cfg_parse_include (NULL, NULL)) == CFGPARSER_FAIL)
		return rc;
	      continue;
	    }
	  conf_error ("expected quoted string, \"Include\", or \"End\", but found %s",
		      token_type_str (tok->type));
	  return CFGPARSER_FAIL;
	}
      if (tok->type == T_STRING)
	combinable_header_add (tok->str);
      else
	{
	  conf_error ("expected quoted string, \"Include\", or \"End\", but found %s",
		      token_type_str (tok->type));
	  return CFGPARSER_FAIL;
	}
    }
  return CFGPARSER_OK;
}

static struct kwtab regex_type_table[] = {
  { "posix", GENPAT_POSIX },
#ifdef HAVE_LIBPCRE
  { "pcre",  GENPAT_PCRE },
  { "perl",  GENPAT_PCRE },
#endif
  { NULL }
};

static int
assign_regex_type (void *call_data, void *section_data)
{
  return cfg_assign_int_enum (call_data, gettkn_expect (T_IDENT),
			      regex_type_table,
			      "regex type");
}

static int
read_resolv_conf (void *call_data, void *section_data)
{
  char **pstr = call_data;

  if (*pstr)
    {
      conf_error ("%s", "ConfigFile statement overrides prior ConfigText");
      free (*pstr);
      *pstr = NULL;
    }
  return cfg_assign_string_from_file (pstr, section_data);
}

static int
read_resolv_text (void *call_data, void *section_data)
{
  char **pstr = call_data;
  char *str;
  struct token *tok;

  if (*pstr)
    {
      conf_error ("%s", "ConfigText statement overrides prior ConfigFile");
      free (*pstr);
      *pstr = NULL;
    }

  if ((tok = gettkn_any ()) == NULL)
    {
      conf_error ("%s", "unexpected end of file");
      return CFGPARSER_FAIL;
    }
  if (tok->type != '\n')
    {
      conf_error ("expected newline, but found %s",
		  token_type_str (tok->type));
      return CFGPARSER_FAIL;
    }

  if (cfg_read_to_end (cur_input, &str) == EOF)
    return CFGPARSER_FAIL;
  *pstr = xstrdup (str);
  return CFGPARSER_OK_NONL;
}

static CFGPARSER_TABLE resolver_parsetab[] = {
  {
    .name = "End",
    .parser = cfg_parse_end
  },
  {
    .name = "ConfigFile",
    .parser = read_resolv_conf,
  },
  {
    .name = "ConfigText",
    .parser = read_resolv_text,
  },
  {
    .name = "Debug",
    .parser = cfg_assign_bool,
    .off = offsetof (struct resolver_config, debug)
  },
  {
    .name = "CNAMEChain",
    .parser = cfg_assign_unsigned,
    .off = offsetof (struct resolver_config, max_cname_chain)
  },
  {
    .name = "RetryInterval",
    .parser = cfg_assign_timeout,
    .off = offsetof (struct resolver_config, retry_interval)
  },
  { NULL }
};

static int
parse_resolver (void *call_data, void *section_data)
{
  POUND_DEFAULTS *dfl = section_data;
  struct locus_range range = LOCUS_RANGE_INITIALIZER;
  int rc = parser_loop (resolver_parsetab, &dfl->resolver, dfl, &range);
#ifndef ENABLE_DYNAMIC_BACKENDS
  if (rc == CFGPARSER_OK)
    conf_error_at_locus_range (&range, "%s",
			       "section ignored: "
			       "pound compiled without support "
			       "for dynamic backends");
#endif
  locus_range_unref (&range);
  return rc;
}

static CFGPARSER_TABLE top_level_parsetab[] = {
  {
    .type = KWT_TOPLEVEL
  },
  {
    .name = "IncludeDir",
    .parser = cfg_parse_includedir
  },
  {
    .name = "User",
    .parser = cfg_assign_string,
    .data = &user
  },
  {
    .name = "Group",
    .parser = cfg_assign_string,
    .data = &group
  },
  {
    .name = "RootJail",
    .parser = cfg_assign_string,
    .data = &root_jail
  },
  {
    .name = "Daemon",
    .parser = cfg_assign_bool,
    .data = &daemonize
  },
  {
    .name = "Supervisor",
    .parser = cfg_assign_bool,
    .data = &enable_supervisor
  },
  {
    .name = "WorkerMinCount",
    .parser = cfg_assign_unsigned,
    .data = &worker_min_count
  },
  {
    .name = "WorkerMaxCount",
    .parser = cfg_assign_unsigned,
    .data = &worker_max_count
  },
  {
    .name = "Threads",
    .parser = parse_threads_compat
  },
  {
    .name = "WorkerIdleTimeout",
    .parser = cfg_assign_timeout,
    .data = &worker_idle_timeout
  },
  {
    .name = "Grace",
    .parser = cfg_assign_timeout,
    .data = &grace
  },
  {
    .name = "LogFacility",
    .parser = cfg_assign_log_facility,
    .off = offsetof (POUND_DEFAULTS, facility)
  },
  {
    .name = "LogLevel",
    .parser = parse_log_level,
    .off = offsetof (POUND_DEFAULTS, log_level)
  },
  {
    .name = "LogFormat",
    .parser = parse_log_format
  },
  {
    .name = "LogTag",
    .parser = cfg_assign_string,
    .data = &syslog_tag
  },
  {
    .name = "Alive",
    .parser = cfg_assign_timeout,
    .data = &alive_to
  },
  {
    .name = "Client",
    .parser = cfg_assign_timeout,
    .off = offsetof (POUND_DEFAULTS, clnt_to)
  },
  {
    .name = "TimeOut",
    .parser = cfg_assign_timeout,
    .off = offsetof (POUND_DEFAULTS, be_to)
  },
  {
    .name = "WSTimeOut",
    .parser = cfg_assign_timeout,
    .off = offsetof (POUND_DEFAULTS, ws_to)
  },
  {
    .name = "ConnTO",
    .parser = cfg_assign_timeout,
    .off = offsetof (POUND_DEFAULTS, be_connto)
  },
  {
    .name = "Balancer",
    .parser = parse_balancer,
    .off = offsetof (POUND_DEFAULTS, balancer_algo)
  },
  {
    .name = "HeaderOption",
    .parser = parse_header_options,
    .off = offsetof (POUND_DEFAULTS, header_options)
  },
  {
    .name = "ECDHCurve",
    .parser = parse_ECDHCurve
  },
  {
    .name = "SSLEngine",
    .parser = parse_SSLEngine
  },
  {
    .name = "Control",
    .parser = parse_control_listener
  },
  {
    .name = "Anonymise",
    .parser = cfg_int_set_one,
    .data = &anonymise
  },
  {
    .name = "Anonymize",
    .type = KWT_ALIAS
  },
  {
    .name = "Service",
    .parser = parse_service,
    .data = &services
  },
  {
    .name = "Backend",
    .parser = parse_named_backend,
    .off = offsetof (POUND_DEFAULTS, named_backend_table)
  },
  {
    .name = "Condition",
    .parser = parse_named_cond
  },
  {
    .name = "ListenHTTP",
    .parser = parse_listen_http,
    .data = &listeners
  },
  {
    .name = "ListenHTTPS",
    .parser = parse_listen_https,
    .data = &listeners
  },
  {
    .name = "ACL",
    .parser = parse_named_acl
  },
  {
    .name = "PidFile",
    .parser = cfg_assign_string,
    .data = &pid_name
  },
  {
    .name = "BackendStats",
    .parser = cfg_assign_bool,
    .data = &enable_backend_stats
  },
  {
    .name = "ForwardedHeader",
    .parser = cfg_assign_string,
    .data = &forwarded_header
  },
  {
    .name = "TrustedIP",
    .parser = assign_acl,
    .data = &trusted_ips
  },
  {
    .name = "CombineHeaders",
    .parser = parse_combine_headers
  },
  {
    .name = "RegexType",
    .parser = assign_regex_type,
    .off = offsetof (POUND_DEFAULTS, re_type),
  },
  {
    .name = "Resolver",
    .parser = parse_resolver
  },
  {
    .name = "Lua",
    .parser = pndlua_parse_config
  },
  {
    .name = "WatcherTTL",
    .parser = cfg_assign_unsigned,
    .data = &watcher_ttl
  },
  {
    .name = "LineBufferSize",
    .parser = assign_bufsize,
    .off = offsetof (POUND_DEFAULTS, linebufsize),
  },
  /* Backward compatibility. */
  {
    .name = "IgnoreCase",
    .parser = cfg_assign_bool,
    .off = offsetof (POUND_DEFAULTS, ignore_case),
    .deprecated = 1,
    .message = "use the -icase matching directive flag to request case-insensitive comparison"
  },

  { NULL }
};

static int
str_is_ipv4 (const char *addr)
{
  int c;
  int dot_count;
  int digit_count;

  dot_count = 0;
  digit_count = 0;
  for (; (c = *addr) != 0; addr++)
    {
      if (c == '.')
	{
	  if (++dot_count > 4)
	    return 0;
	  digit_count = 0;
	}
      else if (!(c_isdigit (c) && ++digit_count <= 3))
	return 0;
    }

  return dot_count == 3;
}

static int
str_is_ipv6 (const char *addr)
{
  int c;
  int col_count = 0; /* Number of colons */
  int dcol = 0;      /* Did we encounter a double-colon? */
  int dig_count = 0; /* Number of digits in the last group */

  for (; (c = *addr) != 0; addr++)
    {
      if (!c_isascii (c))
	return 0;
      else if (c_isxdigit (c))
	{
	  if (++dig_count > 4)
	    return 0;
	}
      else if (c == ':')
	{
	  if (col_count && dig_count == 0 && ++dcol > 1)
	    return 0;
	  if (++col_count > 7)
	    return 0;
	  dig_count = 0;
	}
      else
	return 0;
    }
  return col_count == 7 || dcol;
}

static int
str_is_ip (const char *addr)
{
  int c;
  int dot = 0;
  for (; (c = *addr) != 0 && c_isascii (c); addr++)
    {
      if (!c_isascii (c))
	break;
      else if (c_isxdigit (c))
	return str_is_ipv6 (addr);
      else if (c == dot)
	return str_is_ipv4 (addr);
      else if (c_isdigit (c))
	dot = '.';
      else
	break;
    }
  return 0;
}

void
backend_matrix_to_regular (struct be_matrix *mtx, struct addrinfo *addr,
			   struct be_regular *reg)
{
  memset (reg, 0, sizeof (*reg));
  reg->addr = *addr;

  switch (reg->addr.ai_family)
    {
    case AF_INET:
      ((struct sockaddr_in *)reg->addr.ai_addr)->sin_port = mtx->port;
      break;

    case AF_INET6:
      ((struct sockaddr_in6 *)reg->addr.ai_addr)->sin6_port = mtx->port;
      break;
    }

  reg->alive = 1;
  reg->to = mtx->to;
  reg->conn_to = mtx->conn_to;
  reg->ws_to = mtx->ws_to;
  reg->ctx = mtx->ctx;
  reg->servername = mtx->servername;
}

static int
backend_resolve (BACKEND *be)
{
  struct addrinfo addr;
  struct be_regular reg;
  char *hostname = be->v.mtx.hostname;

  if (get_host (hostname, &addr, be->v.mtx.family))
    {
      /* if we can't resolve it, assume this is a UNIX domain socket */
      struct sockaddr_un *sun;
      size_t len = strlen (hostname);
      if (len > UNIX_PATH_MAX)
	{
	  conf_error_at_locus_range (&be->locus, "%s",
				     "UNIX path name too long");
	  return CFGPARSER_FAIL;
	}

      len += offsetof (struct sockaddr_un, sun_path) + 1;
      sun = xmalloc (len);
      sun->sun_family = AF_UNIX;
      strcpy (sun->sun_path, hostname);

      addr.ai_socktype = SOCK_STREAM;
      addr.ai_family = AF_UNIX;
      addr.ai_protocol = 0;
      addr.ai_addr = (struct sockaddr *) sun;
      addr.ai_addrlen = len;
    }

  backend_matrix_to_regular (&be->v.mtx, &addr, &reg);
  free (hostname);
  be->v.reg = reg;
  be->be_type = BE_REGULAR;
  backend_refcount_init (be);
  return 0;
}

struct be_setup_closure
{
  /* Input */
  NAMED_BACKEND_TABLE *be_tab;
  SERVICE *svc;        /* Service. */
  BALANCER *bal;       /* Balancer. */

  /* Output */
  int be_count;        /* Number of backends processed. */
  int be_class;        /* Backend class mask. */
  int err;             /* Errors encountered or not. */
};

static void
be_setup_closure_init (struct be_setup_closure *cp,
		       NAMED_BACKEND_TABLE *tab,
		       SERVICE *svc, BALANCER *bal)
{
  memset (cp, 0, sizeof *cp);
  cp->be_tab = tab;
  cp->svc = svc;
  cp->bal = bal;
}

static int
backend_finalize (BACKEND *be, NAMED_BACKEND_TABLE *tab)
{
  if (be->be_type == BE_BACKEND_REF)
    {
      NAMED_BACKEND *nb;

      nb = named_backend_retrieve (tab, be->v.be_name);
      if (!nb)
	{
	  conf_error_at_locus_range (&be->locus,
				     "named backend %s is not declared",
				     be->v.be_name);
	  return -1;
	}
      free (be->v.be_name);
      be->be_type = BE_MATRIX;
      be->v.mtx = nb->bemtx;
      /* Hostname will be freed after resolving backend to be_regular.
	 FIXME: use STRING? */
      be->v.mtx.hostname = xstrdup (be->v.mtx.hostname);
      if (be->priority == -1)
	be->priority = nb->priority;
      if (be->disabled == -1)
	be->disabled = nb->disabled;
    }

  if (be->be_type == BE_MATRIX)
    {
      if (!be->v.mtx.hostname)
	{
	  conf_error_at_locus_range (&be->locus, "%s",
				     "Backend missing Address declaration");
	  return -1;
	}

      if (be->v.mtx.hostname[0] == '/' || str_is_ip (be->v.mtx.hostname))
	be->v.mtx.resolve_mode = bres_immediate;

      if (be->v.mtx.port == 0)
	{
	  be->v.mtx.port = htons (be->v.mtx.ctx == NULL ? PORT_HTTP : PORT_HTTPS);
	}
      else if (be->v.mtx.hostname[0] == '/')
	{
	  conf_error_at_locus_range (&be->locus,
				     "Port is not applicable to this address family");
	  return -1;
	}

      if (be->v.mtx.resolve_mode == bres_immediate)
	{
	  if (backend_resolve (be))
	    return -1;
	}
      else
	{
#ifdef ENABLE_DYNAMIC_BACKENDS
	  if (feature_is_set (FEATURE_DNS))
	    {
	      backend_matrix_init (be);
	    }
	  else
	    {
	      conf_error_at_locus_range (&be->locus,
					 "Dynamic backend creation is not "
					 "available: disabled by -Wno-dns");
	      return 1;
	    }
#else
	  conf_error_at_locus_range (&be->locus,
				     "Dynamic backend creation is not "
				     "available: pound compiled without "
				     "support for dynamic backends");
	  return 1;

#endif
	}
    }
  return 0;
}

#define BE_MASK(n) (1<<(n))
#define BX_(x)  ((x) - (((x)>>1)&0x77777777)			\
		 - (((x)>>2)&0x33333333)			\
		 - (((x)>>3)&0x11111111))
#define BITCOUNT(x)     (((BX_(x)+(BX_(x)>>4)) & 0x0F0F0F0F) % 255)

static void
cb_be_setup (BACKEND *be, void *data)
{
  struct be_setup_closure *clos = data;

  if (backend_finalize (be, clos->be_tab))
    {
      be->disabled = 1;
      clos->err = 1;
      return;
    }

  clos->be_count++;
  clos->be_class |= BE_MASK (be->be_type);
  be->service = clos->svc;
  if (be->priority > PRI_MAX)
    {
      conf_error_at_locus_range (&be->locus,
				 "backend priority out of allowed"
				 " range; reset to max. %d",
				 PRI_MAX);
      be->priority = PRI_MAX;
    }
}

static int
service_finalize (SERVICE *svc, void *data)
{
  BALANCER *bal;
  unsigned be_count = 0;
  NAMED_BACKEND_TABLE *tab = data;

  DLIST_FOREACH (bal, &svc->balancers, link)
    {
      struct be_setup_closure be_setup;
      bal->algo = svc->balancer_algo;
      be_setup_closure_init (&be_setup, tab, svc, bal);
      balancer_recompute_pri_unlocked (bal, cb_be_setup, &be_setup);
      if (be_setup.err)
	return -1;

      if (be_setup.be_count > 1)
	{
	  if (be_setup.be_class & ~(BE_MASK (BE_REGULAR) |
				    BE_MASK (BE_MATRIX) |
				    BE_MASK (BE_REDIRECT)))
	    {
	      conf_error_at_locus_range (&svc->locus,
			  "%s",
			  BITCOUNT (be_setup.be_class) == 1
			    ? "multiple backends of this type are not allowed"
			    : "service mixes backends of different types");
	      return -1;
	    }

	  if (be_setup.be_class & BE_MASK (BE_REDIRECT))
	    {
	      conf_error_at_locus_range (&svc->locus,
			  "warning: %s",
			  (be_setup.be_class & (BE_MASK (BE_REGULAR) |
						BE_MASK (BE_MATRIX)))
			     ? "service mixes regular and redirect backends"
			     : "service uses multiple redirect backends");
	      conf_error_at_locus_range (&svc->locus,
			  "%s",
			  "see section \"DEPRECATED FEATURES\" in pound(8)");
	    }
	}

      be_count += be_setup.be_count;
    }

  if (be_count == 0)
    {
      conf_error_at_locus_range (&svc->locus, "%s",
				 "warning: no backends defined");
    }

  service_lb_init (svc);

  return 0;
}

int
parse_config_file (char const *file, int nosyslog)
{
  int res = -1;
  POUND_DEFAULTS pound_defaults = {
    .log_level = 1,
    .facility = LOG_DAEMON,
    .clnt_to = 10,
    .be_to = 15,
    .ws_to = 600,
    .be_connto = 15,
    .ignore_case = 0,
    .re_type = GENPAT_POSIX,
    .header_options = HDROPT_FORWARDED_HEADERS | HDROPT_SSL_HEADERS,
    .balancer_algo = BALANCER_ALGO_RANDOM,
    .resolver = RESOLVER_CONFIG_INITIALIZER,
    .linebufsize = MAXBUF
  };

  named_backend_table_init (&pound_defaults.named_backend_table);
  compile_canned_formats ();

  if (cfgparser_open (file, NULL))
    return -1;

  res = parser_loop (top_level_parsetab, &pound_defaults, &pound_defaults,
		     NULL);
  if (res == 0)
    {
      if (cur_input)
	return -1;

      if (forbid_ssl_usage (&services,
			    "use of SSL features in top-level sections"
			    " is forbidden"))
	return -1;
#ifdef ENABLE_DYNAMIC_BACKENDS
      resolver_set_config (&pound_defaults.resolver);
#endif
      if (foreach_service (service_finalize,
			   &pound_defaults.named_backend_table))
	return -1;

      named_cond_finish ();

      if (pndlua_init ())
	return -1;

      if (worker_min_count > worker_max_count)
	abend (NULL, "WorkerMinCount is greater than WorkerMaxCount");
      if (!nosyslog)
	log_facility = pound_defaults.facility;
    }
  named_backend_table_free (&pound_defaults.named_backend_table);
  cfgparser_finish (root_jail || daemonize);
  return res;
}

enum
  {
    F_OFF,
    F_ON,
    F_DFL
  };

struct pound_feature
{
  char *name;
  char *descr;
  int enabled;
  void (*setfn) (int, char const *);
};

static void
set_include_dir (int enabled, char const *val)
{
  if (enabled)
    {
      struct stat st;
      if (val && (*val == 0 || strcmp (val, ".") == 0))
	val = NULL;
      else if (stat (val, &st))
	{
	  logmsg (LOG_ERR, "include-dir: can't stat %s: %s", val, strerror (errno));
	  exit (1);
	}
      else if (!S_ISDIR (st.st_mode))
	{
	  logmsg (LOG_ERR, "include-dir: %s is not a directory", val);
	  exit (1);
	}
      include_dir = val;
    }
  else
    include_dir = NULL;
}

static struct pound_feature feature[] = {
  [FEATURE_DNS] = {
    .name = "dns",
    .descr = "resolve host names found in configuration file (default)",
    .enabled = F_ON
  },
  [FEATURE_INCLUDE_DIR] = {
    .name = "include-dir",
    .descr = "include file directory",
    .enabled = F_DFL,
    .setfn = set_include_dir
  },
  [FEATURE_WARN_DEPRECATED] = {
    .name = "warn-deprecated",
    .descr = "warn if deprecated configuration statements are used (default)",
    .enabled = F_DFL,
  },
  { NULL }
};

int
feature_is_set (int f)
{
  return feature[f].enabled;
}

static int
feature_set (char const *name)
{
  int i, enabled = F_ON;
  size_t len;
  char *val;

  if ((val = strchr (name, '=')) != NULL)
    {
      len = val - name;
      val++;
    }
  else
    len = strlen (name);

  if (val == NULL && strncmp (name, "no-", 3) == 0)
    {
      name += 3;
      len -= 3;
      enabled = F_OFF;
    }

  if (*name)
    {
      for (i = 0; feature[i].name; i++)
	{
	  if (strlen (feature[i].name) == len &&
	      memcmp (feature[i].name, name, len) == 0)
	    {
	      if (feature[i].setfn)
		feature[i].setfn (enabled, val);
	      else if (val)
		break;
	      feature[i].enabled = enabled;
	      return 0;
	    }
	}
    }
  return -1;
}

struct string_value pound_settings[] = {
  { "Configuration file",  STRING_CONSTANT, { .s_const = POUND_CONF } },
  { "Include directory",   STRING_CONSTANT, { .s_const = SYSCONFDIR } },
  { "PID file",   STRING_CONSTANT,  { .s_const = POUND_PID } },
  { "Buffer size",STRING_INT, { .s_int = MAXBUF } },
  { "Regex types", STRING_CONSTANT, { .s_const = "POSIX"
#if HAVE_LIBPCRE == 1
				       ", PCRE"
#elif HAVE_LIBPCRE == 2
				       ", PCRE2"
#endif
    }
  },
  { "Dynamic backends", STRING_CONSTANT, { .s_const =
#if ENABLE_DYNAMIC_BACKENDS
					  "enabled"
#else
					  "disabled"
#endif
    }
  },
  { "FS event monitoring", STRING_CONSTANT, { .s_const =
#if WITH_INOTIFY
				 "inotify"
#elif WITH_KQUEUE
				 "kqueue"
#else
				 "periodic"
#endif
    }
  },
  { "Lua support", STRING_CONSTANT, { .s_const =
#if ENABLE_LUA
				     "enabled"
#else
				     "disabled"
#endif
    }
  },
#if ! SET_DH_AUTO
  { "DH bits",         STRING_INT, { .s_int = DH_LEN } },
  { "RSA regeneration interval", STRING_INT, { .s_int = T_RSA_KEYS } },
#endif
  { NULL }
};

void
print_help (void)
{
  int i;

  printf ("usage: %s [-FVcehv] [-W [no-]FEATURE] [-f FILE] [-p FILE]\n", progname);
  printf ("HTTP/HTTPS reverse-proxy and load-balancer\n");
  printf ("\nOptions are:\n\n");
  printf ("   -c               check configuration file syntax and exit\n");
  printf ("   -e               print errors on stderr (implies -F)\n");
  printf ("   -F               remain in foreground after startup\n");
  printf ("   -f FILE          read configuration from FILE\n");
  printf ("                    (default: %s)\n", POUND_CONF);
  printf ("   -p FILE          write PID to FILE\n");
  printf ("                    (default: %s)\n", POUND_PID);
  printf ("   -V               print program version, compilation settings, and exit\n");
  printf ("   -v               print log messages to stdout/stderr during startup\n");
  printf ("   -W [no-]FEATURE  enable or disable optional feature\n");
  printf ("\n");
  printf ("FEATUREs are:\n");
  for (i = 0; feature[i].name; i++)
    printf ("   %-16s %s\n", feature[i].name, feature[i].descr);
  printf ("\n");
  printf ("Report bugs and suggestions to <%s>\n", PACKAGE_BUGREPORT);
#ifdef PACKAGE_URL
  printf ("%s home page: <%s>\n", PACKAGE_NAME, PACKAGE_URL);
#endif
}

void
config_parse (int argc, char **argv)
{
  int c;
  int check_only = 0;
  char *conf_name = POUND_CONF;
  char *pid_file_option = NULL;
  int foreground_option = 0;
  int stderr_option = 0;

  set_progname (argv[0]);

  while ((c = getopt (argc, argv, "ceFf:hp:VvW:")) > 0)
    switch (c)
      {
      case 'c':
	check_only = 1;
	break;

      case 'e':
	stderr_option = foreground_option = 1;
	setlinebuf (stderr);
	setlinebuf (stdout);
	break;

      case 'F':
	foreground_option = 1;
	break;

      case 'f':
	conf_name = optarg;
	break;

      case 'h':
	print_help ();
	exit (0);

      case 'p':
	pid_file_option = optarg;
	break;

      case 'V':
	print_version (pound_settings);
	exit (0);

      case 'v':
	print_log = 1;
	break;

      case 'W':
	if (feature_set (optarg))
	  {
	    logmsg (LOG_ERR, "invalid feature name: %s", optarg);
	    exit (1);
	  }
	break;

      default:
	exit (1);
      }

  if (optind < argc)
    {
      logmsg (LOG_ERR, "unknown extra arguments (%s...)", argv[optind]);
      exit (1);
    }

  if (parse_config_file (conf_name, stderr_option))
    exit (1);

  if (check_only)
    {
      logmsg (LOG_INFO, "Config file %s is OK", conf_name);
      exit (0);
    }

  if (SLIST_EMPTY (&listeners))
    abend (NULL, "no listeners defined");

  if (pid_file_option)
    pid_name = pid_file_option;
  if (strcmp (pid_name, "-") == 0)
    pid_name = NULL;

  if (foreground_option)
    daemonize = 0;

  if (daemonize)
    {
      if (log_facility == -1)
	log_facility = LOG_DAEMON;
    }
}
