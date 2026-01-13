/*
 * DNS resolver for pound.
 * Copyright (C) 2024-2025 Sergey Poznyakoff
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
#include <adns.h>
#include <assert.h>
#include "resolver.h"

/* Global parameters. */
static struct resolver_config conf;

void
resolver_set_config (struct resolver_config *newcfg)
{
  conf = *newcfg;
}

void
get_negative_expire_time (struct timespec *ts, BACKEND *be)
{
  clock_gettime (CLOCK_REALTIME, ts);
  ts->tv_sec += be->v.mtx.retry_interval
		      ? be->v.mtx.retry_interval : conf.retry_interval;
}

static struct dns_response *
dns_response_alloc (enum dns_resp_type type, size_t count)
{
  struct dns_response *resp;

  if ((resp = calloc (1, sizeof (*resp))) != NULL)
    {
      if ((resp->addr = calloc (count, sizeof (resp->addr[0]))) != NULL)
	{
	  resp->type = type;
	  resp->count = count;
	}
      else
	{
	  free (resp);
	  resp = NULL;
	}
    }
  return resp;
}

void
dns_response_free (struct dns_response *resp)
{
  int i;

  switch (resp->type)
    {
    case dns_resp_addr:
      free (resp->addr);
      break;

    case dns_resp_srv:
      for (i = 0; i < resp->count; i++)
	free (resp->srv[i].host);
      free (resp->srv);
      break;

    case dns_resp_none:
      break;
    }
  free (resp);
}

#define DEFAULT_QFLAGS \
	(adns_qf_cname_loose | \
	 adns_qf_quoteok_query | \
	 adns_qf_quoteok_cname | \
	 adns_qf_quoteok_anshost)

static void
dns_log_cb (adns_state ads, void *logfndata, const char *fmt, va_list ap)
{
  struct stringbuf *sb = logfndata;
  int rc = stringbuf_vprintf (sb, fmt, ap);
  if (rc)
    stringbuf_reset (sb);
  else
    {
      char *p;
      while ((p = memchr (stringbuf_value (sb), '\n', stringbuf_len (sb))))
	{
	  *p++ = 0;
	  logmsg (LOG_ERR, "(%"PRItid") %s", POUND_TID (),
		  stringbuf_value (sb));
	  stringbuf_consume (sb, p - stringbuf_value (sb));
	}
    }
}

static pthread_key_t dns_state_key;
static pthread_once_t dns_state_key_once = PTHREAD_ONCE_INIT;

struct thread_dns_state
{
  adns_state state;
  struct stringbuf sb;
};

static void
dns_state_free (void *f)
{
  if (f)
    {
      struct thread_dns_state *s = f;
      adns_finish (s->state);
      stringbuf_free (&s->sb);
      free (f);
    }
}

static void
dns_state_key_create (void)
{
  pthread_key_create (&dns_state_key, dns_state_free);
}

static struct thread_dns_state *
dns_state_create (void)
{
  int flags = adns_if_nosigpipe;
  struct thread_dns_state *ds;

  ds = calloc (1, sizeof (*ds));
  if (!ds)
    lognomem ();
  else
    {
      int rc;

      if (conf.debug)
	flags |= adns_if_debug;
      stringbuf_init_log (&ds->sb);
      rc = adns_init_logfn (&ds->state, flags, conf.config_text,
			    dns_log_cb, &ds->sb);
      if (rc)
	{
	  logmsg (LOG_ERR, "can't initialize DNS state: %s", strerror (rc));
	  stringbuf_free (&ds->sb);
	  free (ds);
	  ds = NULL;
	}
    }
  return ds;
}

static adns_state *
dns_get_state (void)
{
  struct thread_dns_state *state;

  pthread_once (&dns_state_key_once, dns_state_key_create);
  state = pthread_getspecific (dns_state_key);
  if (!state)
    {
      state = dns_state_create ();
      if (!state)
	exit (1);
      pthread_setspecific (dns_state_key, state);
    }
  return &state->state;
}

/* Table of correspondence between ADNS status codes and dns status.
   Values are increased by 1 to be able to tell whether the entry is
   initialized or not. */
int adns_to_dns_tab[] = {
#define STAT(s) ((s)+1)
  [adns_s_ok]                  = STAT (dns_success),

  [adns_s_nomemory]            = STAT (dns_failure),
  [adns_s_unknownrrtype]       = STAT (dns_failure),
  [adns_s_systemfail]          = STAT (dns_failure),

  /* remotely induced errors, detected locally */
  [adns_s_timeout]             = STAT (dns_temp_failure),
  [adns_s_allservfail]         = STAT (dns_temp_failure),
  [adns_s_norecurse]           = STAT (dns_temp_failure),
  [adns_s_invalidresponse]     = STAT (dns_failure),
  [adns_s_unknownformat]       = STAT (dns_failure),

  /* remotely induced errors), reported by remote server to us */
  [adns_s_rcodeservfail]       = STAT (dns_not_found),
  [adns_s_rcodeformaterror]    = STAT (dns_not_found),
  [adns_s_rcodenotimplemented] = STAT (dns_not_found),
  [adns_s_rcoderefused]        = STAT (dns_not_found),
  [adns_s_rcodeunknown]        = STAT (dns_not_found),

  /* remote configuration errors */
  [adns_s_inconsistent]        = STAT (dns_not_found),
  [adns_s_prohibitedcname]     = STAT (dns_not_found),
  [adns_s_answerdomaininvalid] = STAT (dns_not_found),
  [adns_s_answerdomaintoolong] = STAT (dns_not_found),
  [adns_s_invaliddata]         = STAT (dns_not_found),

  /* permanent problems with the query */
  [adns_s_querydomainwrong]    = STAT (dns_failure),
  [adns_s_querydomaininvalid]  = STAT (dns_failure),
  [adns_s_querydomaintoolong]  = STAT (dns_failure),

  /* permanent errors */
  [adns_s_nxdomain]            = STAT (dns_not_found),
  [adns_s_nodata]              = STAT (dns_not_found),
#undef STAT
};

/* Convert ADNS status code E to DNS status. */
static int
adns_to_dns_status (int e)
{
  /* If it is negative, fail right away */
  if (e < 0)
    return dns_failure;
  /* Look up in table. */
  if (e < sizeof (adns_to_dns_tab) / sizeof (adns_to_dns_tab[0]))
    {
      int r;

      if ((r = adns_to_dns_tab[e]) > 0)
		return r - 1;
    }
  /*
   * If not found in table, use adns_s_max_ constants to decide the
   * error class.
   */
  if (e < adns_s_max_localfail)
    return dns_failure;
  if (e < adns_s_max_remotefail)
    return dns_not_found;
  if (e < adns_s_max_tempfail)
    return dns_temp_failure;
  if (e < adns_s_max_misconfig)
    return dns_not_found;
  if (e < adns_s_max_misquery)
    return dns_not_found;
  return dns_not_found;
}

static int
errno_to_dns_status (int e)
{
  switch (e)
    {
    case 0:
      return dns_success;
    case EAGAIN:
#ifdef EINPROGRESS
    case EINPROGRESS:
#endif
#ifdef ETIMEDOUT
    case ETIMEDOUT:
#endif
      return dns_temp_failure;
    default:
      break;
    }
  return dns_failure;
}

typedef struct
{
  char name[1];
} CNAME_REF;

static unsigned long
CNAME_REF_hash (const CNAME_REF *cp)
{
  return strhash_ci (cp->name, strlen (cp->name));
}

static int
CNAME_REF_cmp (const CNAME_REF *a, const CNAME_REF *b)
{
  return c_strcasecmp (a->name, b->name);
}

#define HT_TYPE CNAME_REF
#define HT_TYPE_HASH_FN_DEFINED 1
#define HT_TYPE_CMP_FN_DEFINED 1
#define HT_NO_DELETE
#include "ht.h"

static int
cname_install (CNAME_REF_HASH *hash, unsigned *n, char const *name,
	       char const **new_name)
{
  CNAME_REF *rec, *old;

  rec = malloc (sizeof (*rec) + strlen (name));
  if (rec == NULL)
    return errno;
  strcpy (rec->name, name);
  if ((old = CNAME_REF_INSERT (hash, rec)) != NULL)
    {
      free (rec);
      return EEXIST;
    }
  ++*n;
  *new_name = rec->name;
  return 0;
}

/*
 * dns_query - look up a label NAME of RR type TYPE in the DNS.  Follow
 * CNAME chains of up to dns_max_cname_chain elements.  In other respects
 * the behavior is the same as that of adns_synchronous.
 *
 * FIXME: in the presence of a CNAME chain, this function does two
 * extra lookups, compared with the hypothetical libresolv implementation.
 * This is due to the specifics of libadns.
 */
static int
dns_query (const char *name, adns_rrtype type, adns_answer **ans_ret)
{
  adns_state *state = dns_get_state();
  adns_answer *ans = NULL, *cnans = NULL;
  int rc;

  /*
   * First, look up the requested RR type.  If the actual record is
   * a CNAME pointing to the requested RR, this will be handled by
   * adns due to adns_qf_cname_loose flag in DEFAULT_QFLAGS.
   *
   * If it is a CNAME pointing to a CNAME, this will result in the
   * first extra lookup (see FIXME above).
   */
  rc = adns_synchronous (*state, name, type, DEFAULT_QFLAGS, &ans);
  if (rc == 0 && ans->status == adns_s_prohibitedcname
      && conf.max_cname_chain > 1)
    {
      CNAME_REF_HASH *hash = CNAME_REF_HASH_NEW ();
      unsigned cname_count = 0;

      /* Record the queried name, first. */
      if ((rc = cname_install (hash, &cname_count, name, &name)) == 0)
	{
	  /* Follow the CNAME chain. */
	  while (cname_count - 1 <= conf.max_cname_chain)
	    {
	      if ((rc = adns_synchronous (*state, name, adns_r_cname,
					  DEFAULT_QFLAGS, &cnans)))
		break;
	      if (cnans->status == adns_s_ok)
		{
		  /*
		   * CNAME found. Record it and continue.
		   */
		  rc = cname_install (hash, &cname_count, cnans->rrs.str[0],
				      &name);
		  free (cnans);
		  if (rc)
		    {
		      if (rc == EEXIST)
			/*
			 * Loop detected.  The returned ans retains the
			 * adns_s_prohibitedcname status.
			 */
			rc = 0;
		      break;
		    }
		}
	      else if (cnans->status == adns_s_nodata)
		{
		  /*
		   * RR found, but has a different type.
		   * Look up the requested type using the last
		   * recorded name.  This accounts for second
		   * extra lookup.
		   */
		  free (cnans);
		  rc = adns_synchronous (*state, name, type, DEFAULT_QFLAGS, &ans);
		  break;
		}
	      else
		{
		  /*
		   * Another error.  Replace original answer with
		   * the last one.
		   */
		  free (ans);
		  ans = cnans;
		  break;
		}
	    }
	  CNAME_REF_HASH_FREE (hash);
	}
    }
  if (rc == 0)
    *ans_ret = ans;
  else
    free (ans);
  return adns_to_dns_status (rc);
}

static int
dns_generic_lookup (char const *name, adns_rrtype rrtype, char const *rrname,
		    int (*conv) (int, struct dns_response *, adns_answer *),
		    struct dns_response **presp)
{
  adns_answer *ans = NULL;
  int err, rc;
  struct dns_response *resp;
  enum dns_resp_type resp_type;

  switch (rrtype)
    {
    case adns_r_a:
    case adns_r_aaaa:
      resp_type = dns_resp_addr;
      break;

    case adns_r_srv_raw:
      resp_type = dns_resp_srv;
      break;

    default:
      abort ();
    }

  err = dns_query (name, rrtype, &ans);
  if (err != 0)
    {
      rc = errno_to_dns_status (err);
      if (rc != dns_not_found)
	logmsg (LOG_ERR, "Querying for %s records of %s: %s",
		rrname,
		name,
		strerror (err));
      return rc;
    }
  if (ans->status != adns_s_ok)
    {
      rc = adns_to_dns_status (ans->status);
      if (rc != dns_not_found)
	logmsg (LOG_ERR, "Querying for %s records of %s: %s",
		rrname,
		name,
		adns_strerror (ans->status));

      free (ans);
      return rc;
    }
  if (ans->nrrs == 0)
    {
      free (ans);
      return dns_not_found;
    }

  rc = dns_success;
  resp = dns_response_alloc (resp_type, ans->nrrs);
  if (!resp)
    {
      lognomem ();
      rc = dns_failure;
    }
  else
    {
      int i;

      resp->expires = ans->expires;
      for (i = 0; i < ans->nrrs; i++)
	{
	  if (conv (i, resp, ans))
	    {
	      resp->count = i;
	      dns_response_free (resp);
	      resp = NULL;
	      rc = dns_failure;
	      break;
	    }
	}
    }
  free (ans);
  *presp = resp;
  return rc;
}

static int
rr_a_conv (int i, struct dns_response *resp, adns_answer *ans)
{
  resp->addr[i].s_in.sin_family = AF_INET;
  resp->addr[i].s_in.sin_port = 0;
  resp->addr[i].s_in.sin_addr = ans->rrs.inaddr[i];
  return 0;
}

static int
rr_aaaa_conv (int i, struct dns_response *resp, adns_answer *ans)
{
  resp->addr[i].s_in6.sin6_family = AF_INET6;
  resp->addr[i].s_in6.sin6_port = 0;
  resp->addr[i].s_in6.sin6_addr = ans->rrs.in6addr[i];
  return 0;
}

static int
dns_generic_addr_lookup (char const *name, int family,
			 struct dns_response **presp)
{
  int rr_type;
  char const *rr_name;
  int (*rr_conv) (int, struct dns_response *, adns_answer *);

  switch (family)
    {
    case AF_INET:
      rr_type = adns_r_a;
      rr_name = "A";
      rr_conv = rr_a_conv;
      break;

    case AF_INET6:
      rr_type = adns_r_aaaa;
      rr_name = "AAAA";
      rr_conv = rr_aaaa_conv;
      break;

    default:
      abort ();
    }

  return dns_generic_lookup (name, rr_type, rr_name, rr_conv, presp);
}

int
dns_addr_lookup (char const *name, int family, struct dns_response **presp)
{
  int rc;
  struct dns_response *r4 = NULL, *r6 = NULL;

  switch (family)
    {
    case AF_INET:
    case AF_INET6:
      return dns_generic_addr_lookup (name, family, presp);

    case AF_UNSPEC:
      break;

    default:
      abort();
    }

  rc = dns_generic_addr_lookup (name, AF_INET, &r4);
  switch (dns_generic_addr_lookup (name, AF_INET6, &r6))
    {
    case dns_success:
      rc = 0;
      break;

    case dns_not_found:
      if (rc != dns_success)
	return dns_not_found;
      break;

    case dns_temp_failure:
      if (rc != dns_success)
	return rc;

    case dns_failure:
      if (rc != dns_success)
	return dns_failure;
    }

  if (r4 == NULL)
    *presp = r6;
  else if (r6 == NULL)
    *presp = r4;
  else
    {
      struct dns_response *resp = dns_response_alloc (dns_resp_addr,
						      r4->count + r6->count);
      if (resp)
	{
	  int i, j;
	  resp->expires = r4->expires < r6->expires ? r4->expires : r6->expires;

	  for (i = j = 0; j < r4->count; i++, j++)
	    resp->addr[i] = r4->addr[j];
	  for (j = 0; j < r6->count; i++, j++)
	    resp->addr[i] = r6->addr[j];

	  *presp = resp;
	}
      else
	rc = dns_failure;

      dns_response_free (r4);
      dns_response_free (r6);
    }
  return rc;
}

static int
rr_srv_conv (int i, struct dns_response *resp, adns_answer *ans)
{
  resp->srv[i].priority = ans->rrs.srvraw[i].priority;
  resp->srv[i].weight = ans->rrs.srvraw[i].weight;
  resp->srv[i].port = ans->rrs.srvraw[i].port;
  if ((resp->srv[i].host = xstrdup (ans->rrs.srvraw[i].host)) == NULL)
    {
      lognomem ();
      return 1;
    }
  return 0;
}

static int
srv_cmp (void const *a, void const *b)
{
  struct dns_srv const *asrv = a;
  struct dns_srv const *bsrv = b;
  int rc = asrv->priority - bsrv->priority;
  if (rc == 0)
    {
      rc = bsrv->weight - asrv->weight;
      if (rc == 0)
	rc = c_strcasecmp (asrv->host, bsrv->host);
    }
  return rc;
}

int
dns_srv_lookup (char const *name, struct dns_response **presp)
{
  struct dns_response *resp;
  int rc = dns_generic_lookup (name, adns_r_srv_raw, "SRV", rr_srv_conv, &resp);
  if (rc == dns_success)
    {
      qsort (resp->srv, resp->count, sizeof (resp->srv[0]), srv_cmp);
      *presp = resp;
    }
  return rc;
}
