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
#if HAVE_CRYPT_H
# include <crypt.h>
#endif

static pthread_mutex_t crypt_mutex = PTHREAD_MUTEX_INITIALIZER;

static int
auth_plain (const char *pass, const char *hash)
{
  return strcmp (pass, hash);
}

static int
auth_crypt (const char *pass, const char *hash)
{
  int res = 1;
  char *cp;

  pthread_mutex_lock (&crypt_mutex);
  cp = crypt (pass, hash);
  if (cp)
    res = strcmp (cp, hash);
  pthread_mutex_unlock (&crypt_mutex);
  return res;
}

#ifndef OPENSSL_NO_MD5

static char *
to64 (char *s, unsigned long v, int n)
{
  static unsigned char itoa64[] =         /* 0 ... 63 => ASCII - 64 */
	    "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  while (--n >= 0)
    {
      *s++ = itoa64[v&0x3f];
      v >>= 6;
    }
  return s;
}

#define APR_MD5_DIGESTSIZE 16
#define APR1_ID_STR "$apr1$"
#define APR1_ID_LEN (sizeof (APR1_ID_STR)-1)

/*
 * Hashed password size is:
 *   6     bytes for the initial APR1_ID_STR
 *   8     bytes (at most) of salt
 *   1     byte for $
 *  22     bytes of hash
 *   1     byte for trailing \0
 *  --
 *  38     bytes total
 */
#define APR1_PW_SIZE 38

char *
apr_md5_encode (const char *pw, const char *salt, char *result, size_t nbytes)
{
   char passwd[APR1_PW_SIZE];
   char *p;
   unsigned char final[EVP_MAX_MD_SIZE];
   unsigned int sz;
   ssize_t slen, plen, i;
   EVP_MD_CTX *ctx, *ctx1;

   if (!strncmp (salt, APR1_ID_STR, APR1_ID_LEN))
     salt += APR1_ID_LEN;

   if ((p = memchr (salt, '$', 8)) != NULL)
     slen = p - salt;
   else
     slen = 8;

   plen = strlen (pw);

   ctx = EVP_MD_CTX_create ();
   EVP_MD_CTX_init (ctx);
   EVP_DigestInit (ctx, EVP_md5 ());

   EVP_DigestUpdate (ctx, pw, plen);
   EVP_DigestUpdate (ctx, APR1_ID_STR, APR1_ID_LEN);
   EVP_DigestUpdate (ctx, salt, slen);

   ctx1 = EVP_MD_CTX_create ();
   EVP_MD_CTX_init (ctx1);
   EVP_DigestInit (ctx1, EVP_md5 ());

   EVP_DigestUpdate (ctx1, pw, plen);
   EVP_DigestUpdate (ctx1, salt, slen);
   EVP_DigestUpdate (ctx1, pw, plen);
   EVP_DigestFinal_ex (ctx1, final, &sz);
   if (sz != APR_MD5_DIGESTSIZE)
     {
       EVP_MD_CTX_destroy (ctx);
       EVP_MD_CTX_destroy (ctx1);
       return NULL;
     }

   for (i = plen; i > 0; i -= APR_MD5_DIGESTSIZE)
     EVP_DigestUpdate (ctx, final,
		 (i > APR_MD5_DIGESTSIZE) ? APR_MD5_DIGESTSIZE : i);

   memset (final, 0, sizeof (final));

   for (i = plen; i != 0; i >>= 1)
     EVP_DigestUpdate (ctx, (i & 1) ? (char*)final : pw, 1);

   strcpy (passwd, APR1_ID_STR);
   strncat (passwd, salt, slen);
   strcat (passwd, "$");

   EVP_DigestFinal_ex (ctx, final, &sz);
   if (sz != APR_MD5_DIGESTSIZE)
     {
       EVP_MD_CTX_destroy (ctx);
       EVP_MD_CTX_destroy (ctx1);
       return NULL;
     }

   for (i = 0; i < 1000; i++)
     {
#if OPENSSL_VERSION_NUMBER > 0x10100000L
       EVP_MD_CTX_reset (ctx1);
#else
       EVP_MD_CTX_cleanup (ctx1);
#endif
       EVP_DigestInit (ctx1, EVP_md5 ());
       if (i & 1)
	 EVP_DigestUpdate (ctx1, pw, plen);
       else
	 EVP_DigestUpdate (ctx1, final, APR_MD5_DIGESTSIZE);

       if (i % 3)
	 EVP_DigestUpdate (ctx1, salt, slen);

       if (i % 7)
	 EVP_DigestUpdate (ctx1, pw, plen);

       if (i & 1)
	 EVP_DigestUpdate (ctx1, final, APR_MD5_DIGESTSIZE);
       else
	 EVP_DigestUpdate (ctx1, pw, plen);
       EVP_DigestFinal_ex (ctx1, final, NULL);
     }

   EVP_MD_CTX_destroy (ctx);
   EVP_MD_CTX_destroy (ctx1);

   p = passwd + strlen (passwd);

   p = to64 (p, (final[0]<<16) | (final[6]<<8) | final[12], 4);
   p = to64 (p, (final[1]<<16) | (final[7]<<8) | final[13], 4);
   p = to64 (p, (final[2]<<16) | (final[8]<<8) | final[14], 4);
   p = to64 (p, (final[3]<<16) | (final[9]<<8) | final[15], 4);
   p = to64 (p, (final[4]<<16) | (final[10]<<8) | final[5], 4);
   p = to64 (p, final[11], 2);
   *p = '\0';

   memset (final, 0, sizeof (final));

   i = strlen (passwd);
   if (i >= nbytes)
     i = nbytes - 1;
   memcpy (result, passwd, i);
   result[i] = 0;

   return result;
}

static int
auth_apr (const char *pass, const char *hash)
{
  char buf[120];
  char *cp = apr_md5_encode (pass, hash, buf, sizeof (buf));
  return cp ? strcmp (cp, hash) : 1;
}
#endif /* OPENSSL_NO_MD5 */

static int
auth_sha1 (const char *pass, const char *hash)
{
  int len;
  BIO *bb, *b64;
  char hashbuf[SHA_DIGEST_LENGTH], resbuf[EVP_MAX_MD_SIZE];
  unsigned int sz;

  if ((bb = BIO_new (BIO_s_mem ())) == NULL)
    {
      logmsg (LOG_WARNING, "(%"PRItid") Can't alloc BIO_s_mem", POUND_TID ());
      return 1;
    }

  if ((b64 = BIO_new (BIO_f_base64 ())) == NULL)
    {
      logmsg (LOG_WARNING, "(%"PRItid") Can't alloc BIO_f_base64",
	      POUND_TID ());
      BIO_free (bb);
      return 1;
    }

  b64 = BIO_push (b64, bb);
  hash += 5; /* Skip past {SHA} */
  BIO_write (bb, hash, strlen (hash));
  BIO_write (bb, "\n", 1);
  len = BIO_read (b64, hashbuf, sizeof (hashbuf));
  if (len <= 0)
    {
      logmsg (LOG_WARNING, "(%"PRItid") Can't read BIO_f_base64",
	      POUND_TID ());
      BIO_free_all (b64);
      return 1;
    }

  if (!BIO_eof (b64))
    {
      logmsg (LOG_WARNING, "(%"PRItid") excess data in SHA1 hash",
	      POUND_TID ());
      BIO_free_all (b64);
      return 1;
    }

  BIO_free_all (b64);

  sz = sizeof (resbuf);
  EVP_Digest (pass, strlen (pass), (unsigned char *)resbuf,
	      &sz, EVP_sha1 (), NULL);

  return !(sz == SHA_DIGEST_LENGTH && memcmp (resbuf, hashbuf, sz) == 0);
}

struct auth_matcher
{
  char *auth_pfx;
  size_t auth_len;
  int (*auth_match) (const char *, const char *);
};

static struct auth_matcher auth_match_tab[] = {
#define S(s) #s, sizeof(#s)-1
#ifndef OPENSSL_NO_MD5
  { S($apr1$), auth_apr },
#endif
  { S({SHA}), auth_sha1 },
  { "", 0, auth_crypt },
  { "", 0, auth_plain },
  { NULL }
};

static int
auth_match (const char *pass, const char *hash)
{
  struct auth_matcher *p;
  size_t plen = strlen (hash);

  for (p = auth_match_tab; p->auth_match; p++)
    {
      if (p->auth_len < plen &&
	  memcmp (p->auth_pfx, hash, p->auth_len) == 0)
	{
	  if (p->auth_match (pass, hash) == 0)
	    return 0;
	  if (p->auth_len > 0)
	    break;
	}
    }
  return 1;
}

static void
user_pass_free (USER_PASS_HEAD *head)
{
  while (!SLIST_EMPTY (head))
    {
      struct user_pass *up = SLIST_FIRST (head);
      SLIST_SHIFT (head, link);
      free (up);
    }
}

static int
pass_file_changed (struct pass_file *pwf, struct stat const *st)
{
#if HAVE_STRUCT_STAT_ST_MTIM
  if (timespec_cmp (&pwf->mtim, &st->st_mtim))
    {
      pwf->mtim = st->st_mtim;
      return 1;
    }
#else
  if (pwf->mtim.tv_sec != st->st_mtime)
    {
      pwf->mtim.tv_sec = st->st_mtime;
      return 1;
    }
#endif
  return 0;
}

static void
pass_file_fill (struct pass_file *pwf)
{
  FILE *fp;
  char buf[MAXBUF];
  struct stat st;

  if ((fp = fopen_wd (pwf->wd, pwf->filename)) == NULL)
    {
      int ec = errno;
      fopen_error (LOG_WARNING, ec, pwf->wd, pwf->filename, &pwf->locus);
      if (ec == ENOENT)
	{
	  user_pass_free (&pwf->head);
	  memset (&pwf->mtim, 0, sizeof (pwf->mtim));
	}
      return;
    }

  if (fstat (fileno (fp), &st))
    {
      logmsg (LOG_WARNING, "fstat(%s) failed: %s", pwf->filename,
	      strerror (errno));
      fclose (fp);
      return;
    }

  if (pass_file_changed (pwf, &st))
    {
      /* Free existing entries. */
      user_pass_free (&pwf->head);

      /* Rescan the file. */
      while (fgets (buf, sizeof (buf), fp))
	{
	  struct user_pass *up;
	  char *p, *q;
	  int ulen;

	  for (p = buf; *p && (*p == ' ' || *p == '\t'); p++);
	  if (*p == '#')
	    continue;
	  q = p + strlen (p);
	  if (q == p)
	    continue;
	  if (q[-1] == '\n')
	    *--q = 0;
	  if (!*p)
	    continue;
	  if ((q = strchr (p, ':')) == NULL)
	    continue;
	  ulen = q - p;

	  if ((up = malloc (sizeof (up[0]) + strlen (p))) == NULL)
	    {
	      lognomem ();
	      break;
	    }

	  strcpy (up->user, p);
	  up->user[ulen] = 0;
	  up->pass = up->user + ulen + 1;

	  SLIST_PUSH (&pwf->head, up, link);
	}
    }
  fclose (fp);
  return;
}

static int
basic_auth_internal (struct pass_file *pwf, char const *user, char const *pass)
{
  struct user_pass *up;

  pass_file_fill (pwf);
  SLIST_FOREACH (up, &pwf->head, link)
    {
      if (strcmp (user, up->user) == 0)
	return auth_match (pass, up->pass);
    }

  return 1;
}

int
basic_auth (struct pass_file *pwf, struct http_request *req)
{
  char *user;
  char *pass;
  int rc;

  if ((rc = http_request_get_basic_auth (req, &user, &pass)) == 0)
    {
      rc = basic_auth_internal (pwf, user, pass);
      memset (pass, 0, strlen (pass));
      free (pass);
      free (user);
    }
  return rc;
}
