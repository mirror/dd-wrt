/*
 * Token bucket filter support for pound
 * Copyright (C) 2025 Sergey Poznyakoff
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

typedef struct token_bucket
{
  char const *name;    /* Entry name. */
  struct timespec ts;  /* Microseconds since Epoch. */
  struct timespec expiry;  /* Expiration time. */
  uint64_t tokens;     /* Tokens available. */
  DLIST_ENTRY (token_bucket) link;
} TOKBKT;

#define HT_TYPE TOKBKT
#include "ht.h"

static inline uint64_t
timediff (struct timespec const *a, struct timespec const *b)
{
  return (a->tv_sec - b->tv_sec) * NANOSECOND + (a->tv_nsec - b->tv_nsec);
}

static inline void
timespec_add_nsec (struct timespec *a, uint64_t nsec)
{
  a->tv_sec += nsec / NANOSECOND;
  a->tv_nsec += nsec % NANOSECOND;
  if (a->tv_nsec >= NANOSECOND)
    {
      a->tv_sec++;
      a->tv_nsec %= NANOSECOND;
    }
}

static TOKBKT *
tbf_lookup (TOKBKT_HASH *tbh,  char const *keyid)
{
  TOKBKT *ent, key;
  key.name = keyid;
  if ((ent = TOKBKT_RETRIEVE (tbh, &key)) == NULL)
    {
      if ((ent = malloc (sizeof (*ent) + strlen (keyid) + 1)) == NULL)
	{
	  lognomem ();
	}
      else
	{
	  memset (ent, 0, sizeof (*ent));
	  ent->name = (char*) (ent + 1);
	  strcpy ((char*)ent->name, keyid);
	  TOKBKT_INSERT (tbh, ent);
	}
    }
  return ent;
}

struct tbf
{
  uint64_t rate;
  unsigned maxtok;
  TOKBKT_HASH *tbh;
  pthread_mutex_t mut;
  DLIST_HEAD (,token_bucket) head;
};

static void tbf_expire_job (enum job_ctl, void *, const struct timespec *);

unsigned tbf_cleanup_interval;

static inline void
tbf_arm_expiration (TBF *tbf)
{
  TOKBKT *bkt = DLIST_FIRST (&tbf->head);
  if (bkt)
    job_enqueue (&bkt->expiry, tbf_expire_job, tbf);
}

static void
tbf_expire_job (enum job_ctl ctl, void *data, const struct timespec *tsnow)
{
  TBF *tbf = data;
  TOKBKT *bkt;
  struct timespec now;

  if (ctl != job_ctl_run)
    return;

  pthread_mutex_lock (&tbf->mut);
  clock_gettime (CLOCK_REALTIME, &now);

  while ((bkt = DLIST_FIRST (&tbf->head)) != NULL &&
	 timespec_cmp (&bkt->expiry, &now) <= 0)
    {
      TOKBKT_DELETE (tbf->tbh, bkt);
      DLIST_REMOVE (&tbf->head, bkt, link);
      free (bkt);
    }

  if (!DLIST_EMPTY (&tbf->head))
    tbf_arm_expiration (tbf);

  pthread_mutex_unlock (&tbf->mut);
}

static int
tbf_check (TBF *tbf, char const *keyid)
{
  TOKBKT *ent;
  struct timespec now;
  int rearm;

  if (tbf->maxtok == 0 || tbf->rate == 0)
    {
      errno = EINVAL;
      return -1;
    }

  if ((ent = tbf_lookup (tbf->tbh, keyid)) == NULL)
    return -1;

  clock_gettime (CLOCK_REALTIME, &now);

  if (timespec_cmp (&now, &ent->expiry) > 0)
    {
      ent->ts = now;
      ent->tokens = tbf->maxtok;
    }
  else
    {
      uint64_t elapsed = timediff (&now, &ent->ts);
      uint64_t tokens = elapsed / tbf->rate;

      timespec_add_nsec (&ent->ts, tokens * tbf->rate);

      if (tokens > tbf->maxtok || ent->tokens >= tbf->maxtok - tokens)
	ent->tokens = tbf->maxtok;
      else
	ent->tokens += tokens;
    }

  ent->expiry = now;
  timespec_add_nsec (&ent->expiry,
		     (1 + tbf->maxtok - ent->tokens) * tbf->rate);

  rearm = DLIST_EMPTY (&tbf->head);
  DLIST_REMOVE (&tbf->head, ent, link);
  DLIST_INSERT_TAIL (&tbf->head, ent, link);
  if (rearm)
    tbf_arm_expiration (tbf);

  if (ent->tokens != 0)
    {
      ent->tokens--;
      return 1;
    }
  else
    return 0;
}

TBF *
tbf_alloc (uint64_t rate, unsigned maxtok)
{
  struct tbf *tbf;
  XZALLOC (tbf);
  tbf->tbh = TOKBKT_HASH_NEW ();
  tbf->rate = rate;
  tbf->maxtok = maxtok;
  pthread_mutex_init (&tbf->mut, NULL);
  DLIST_INIT (&tbf->head);
  return tbf;
}

int
tbf_eval (TBF *tbf, char const *keyid)
{
  int res;
  pthread_mutex_lock (&tbf->mut);
  res = tbf_check (tbf, keyid);
  pthread_mutex_unlock (&tbf->mut);
  return res;
}
