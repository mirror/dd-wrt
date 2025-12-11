/* Retrieve ELF / DWARF / source files from the debuginfod.
   Copyright (C) 2019-2024 Red Hat, Inc.
   Copyright (C) 2021, 2022 Mark J. Wielaard <mark@klomp.org>
   This file is part of elfutils.

   This file is free software; you can redistribute it and/or modify
   it under the terms of either

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at
       your option) any later version

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at
       your option) any later version

   or both in parallel, as here.

   elfutils is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see <http://www.gnu.org/licenses/>.  */


/* cargo-cult from libdwfl linux-kernel-modules.c */
/* In case we have a bad fts we include this before config.h because it
   can't handle _FILE_OFFSET_BITS.
   Everything we need here is fine if its declarations just come first.
   Also, include sys/types.h before fts. On some systems fts.h is not self
   contained. */
#ifdef BAD_FTS
  #include <sys/types.h>
  #include <fts.h>
#endif

#include "config.h"
#include "debuginfod.h"
#include "system.h"
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <gelf.h>

#ifdef ENABLE_IMA_VERIFICATION
#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/x509v3.h>
#include <arpa/inet.h>
#include <imaevm.h>
#endif
typedef enum {ignore, enforcing, undefined} ima_policy_t;


/* We might be building a bootstrap dummy library, which is really simple. */
#ifdef DUMMY_LIBDEBUGINFOD

debuginfod_client *debuginfod_begin (void) { errno = ENOSYS; return NULL; }
int debuginfod_find_debuginfo (debuginfod_client *c, const unsigned char *b,
                               int s, char **p) { return -ENOSYS; }
int debuginfod_find_executable (debuginfod_client *c, const unsigned char *b,
                                int s, char **p) { return -ENOSYS; }
int debuginfod_find_source (debuginfod_client *c, const unsigned char *b,
                            int s, const char *f, char **p)  { return -ENOSYS; }
int debuginfod_find_section (debuginfod_client *c, const unsigned char *b,
			     int s, const char *scn, char **p)
			      { return -ENOSYS; }
int debuginfod_find_metadata (debuginfod_client *c,
                              const char *k, const char *v, char **p) { return -ENOSYS; }
void debuginfod_set_progressfn(debuginfod_client *c,
			       debuginfod_progressfn_t fn) { }
void debuginfod_set_verbose_fd(debuginfod_client *c, int fd) { }
void debuginfod_set_user_data (debuginfod_client *c, void *d) { }
void* debuginfod_get_user_data (debuginfod_client *c) { return NULL; }
const char* debuginfod_get_url (debuginfod_client *c) { return NULL; }
int debuginfod_add_http_header (debuginfod_client *c,
				const char *h) { return -ENOSYS; }
const char* debuginfod_get_headers (debuginfod_client *c) { return NULL; }

void debuginfod_end (debuginfod_client *c) { }

#else /* DUMMY_LIBDEBUGINFOD */

#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <fts.h>
#include <regex.h>
#include <string.h>
#include <stdbool.h>
#include <linux/limits.h>
#include <time.h>
#include <utime.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <curl/curl.h>
#include <fnmatch.h>
#include <json-c/json.h>

/* If fts.h is included before config.h, its indirect inclusions may not
   give us the right LFS aliases of these functions, so map them manually.  */
#ifdef BAD_FTS
  #ifdef _FILE_OFFSET_BITS
    #define open open64
    #define fopen fopen64
  #endif
#else
  #include <sys/types.h>
  #include <fts.h>
#endif

/* Older curl.h don't define CURL_AT_LEAST_VERSION.  */
#ifndef CURL_AT_LEAST_VERSION
  #define CURL_VERSION_BITS(x,y,z) ((x)<<16|(y)<<8|(z))
  #define CURL_AT_LEAST_VERSION(x,y,z) \
    (LIBCURL_VERSION_NUM >= CURL_VERSION_BITS(x, y, z))
#endif

#include <pthread.h>

static pthread_once_t init_control = PTHREAD_ONCE_INIT;
static bool curl_has_https; // = false

static void
libcurl_init(void)
{
  curl_global_init(CURL_GLOBAL_DEFAULT);

  for (const char *const *protocol = curl_version_info(CURLVERSION_NOW)->protocols;
       *protocol != NULL; ++protocol)
    {
      if(strcmp("https", *protocol) == 0)
        curl_has_https = true;
    }
}


#ifdef ENABLE_IMA_VERIFICATION
struct public_key_entry
{
  struct public_key_entry *next; /* singly-linked list */
  uint32_t keyid; /* last 4 bytes of sha1 of public key */
  EVP_PKEY *key; /* openssl */
};
#endif


struct debuginfod_client
{
  /* Progress/interrupt callback function. */
  debuginfod_progressfn_t progressfn;

  /* Stores user data. */
  void* user_data;

  /* Stores current/last url, if any. */
  char* url;

  /* Accumulates outgoing http header names/values. */
  int user_agent_set_p; /* affects add_default_headers */
  struct curl_slist *headers;

  /* Flags the default_progressfn having printed something that
     debuginfod_end needs to terminate. */
  int default_progressfn_printed_p;

  /* Indicates whether the last query was cancelled by progressfn.  */
  bool progressfn_cancel;

  /* File descriptor to output any verbose messages if > 0.  */
  int verbose_fd;

  /* Maintain a long-lived curl multi-handle, which keeps a
     connection/tls/dns cache to recently seen servers. */
  CURLM *server_mhandle;
  
  /* Can contain all other context, like cache_path, server_urls,
     timeout or other info gotten from environment variables, the
     handle data, etc. So those don't have to be reparsed and
     recreated on each request.  */
  char * winning_headers;

#ifdef ENABLE_IMA_VERIFICATION
  /* IMA public keys */
  struct public_key_entry *ima_public_keys;
#endif
};


/* The cache_clean_interval_s file within the debuginfod cache specifies
   how frequently the cache should be cleaned. The file's st_mtime represents
   the time of last cleaning.  */
static const char *cache_clean_interval_filename = "cache_clean_interval_s";
static const long cache_clean_default_interval_s = 86400; /* 1 day */

/* The cache_miss_default_s within the debuginfod cache specifies how
   frequently the empty file should be released.*/
static const long cache_miss_default_s = 600; /* 10 min */
static const char *cache_miss_filename = "cache_miss_s";

/* The cache_max_unused_age_s file within the debuginfod cache specifies the
   the maximum time since last access that a file will remain in the cache.  */
static const char *cache_max_unused_age_filename = "max_unused_age_s";
static const long cache_default_max_unused_age_s = 604800; /* 1 week */

/* The metadata_retention_default_s file within the debuginfod cache
   specifies how long metadata query results should be cached. */
static const long metadata_retention_default_s = 3600; /* 1 hour */
static const char *metadata_retention_filename = "metadata_retention_s";

/* Location of the cache of files downloaded from debuginfods.
   The default parent directory is $HOME, or '/' if $HOME doesn't exist.  */
static const char *cache_default_name = ".debuginfod_client_cache";
static const char *cache_xdg_name = "debuginfod_client";

/* URLs of debuginfods, separated by url_delim. */
static const char *url_delim =  " ";

/* Timeout for debuginfods, in seconds (to get at least 100K). */
static const long default_timeout = 90;

/* Default retry count for download error. */
static const long default_retry_limit = 2;

/* Data associated with a particular CURL easy handle. Passed to
   the write callback.  */
struct handle_data
{
  /* Cache file to be written to in case query is successful.  */
  int fd;

  /* URL queried by this handle.  */
  char url[PATH_MAX];

  /* error buffer for this handle.  */
  char errbuf[CURL_ERROR_SIZE];

  /* This handle.  */
  CURL *handle;

  /* The client object whom we're serving. */
  debuginfod_client *client;

  /* Pointer to handle that should write to fd. Initially points to NULL,
     then points to the first handle that begins writing the target file
     to the cache. Used to ensure that a file is not downloaded from
     multiple servers unnecessarily.  */
  CURL **target_handle;

  /* Response http headers for this client handle, sent from the server */
  char *response_data;
  size_t response_data_size;

  /* Response metadata values for this client handle, sent from the server */
  char *metadata;
  size_t metadata_size;
};



#ifdef ENABLE_IMA_VERIFICATION
  static inline unsigned char hex2dec(char c)
  {
    if (c >= '0' && c <= '9') return (c - '0');
    if (c >= 'a' && c <= 'f') return (c - 'a') + 10;
    if (c >= 'A' && c <= 'F') return (c - 'A') + 10;
    return 0;
  }

  static inline ima_policy_t ima_policy_str2enum(const char* ima_pol)
  {
    if (NULL == ima_pol)                    return undefined;
    if (0 == strcmp(ima_pol, "ignore"))     return ignore;
    if (0 == strcmp(ima_pol, "enforcing"))  return enforcing;
    return undefined;
  }

  static inline const char* ima_policy_enum2str(ima_policy_t ima_pol)
  {
    switch (ima_pol)
    {
    case ignore:
      return "ignore";
    case enforcing:
      return "enforcing";
    case undefined:
      return "undefined";
    }
    return "";
  }


static uint32_t extract_skid_pk(EVP_PKEY *pkey) // compute keyid by public key hashing
{
  if (!pkey) return 0;
  uint32_t keyid = 0;
  X509_PUBKEY *pk = NULL;
  const unsigned char *public_key = NULL;                                                  
  int len;
  if (X509_PUBKEY_set(&pk, pkey) &&
      X509_PUBKEY_get0_param(NULL, &public_key, &len, NULL, pk))
    {
      uint8_t sha1[SHA_DIGEST_LENGTH];
      SHA1(public_key, len, sha1);
      memcpy(&keyid, sha1 + 16, 4);
    }
  X509_PUBKEY_free(pk);
  return ntohl(keyid);
}


static uint32_t extract_skid(X509* x509) // compute keyid from cert or its public key 
  {
    if (!x509) return 0;
    uint32_t keyid = 0;
    // Attempt to get the skid from the certificate
    const ASN1_OCTET_STRING *skid_asn1_str = X509_get0_subject_key_id(x509);
    if (skid_asn1_str)
      {
        int skid_len = ASN1_STRING_length(skid_asn1_str);
        memcpy(&keyid, ASN1_STRING_get0_data(skid_asn1_str) + skid_len - sizeof(keyid), sizeof(keyid));
      }
    else // compute keyid ourselves by hashing public key
      {
        EVP_PKEY *pkey = X509_get0_pubkey(x509);
        keyid = htonl(extract_skid_pk(pkey));
      }
    return ntohl(keyid);
  }


static void load_ima_public_keys (debuginfod_client *c)
{
  /* Iterate over the directories in DEBUGINFOD_IMA_CERT_PATH. */
  char *cert_paths = getenv(DEBUGINFOD_IMA_CERT_PATH_ENV_VAR);
  if (cert_paths == NULL || cert_paths[0] == '\0')
    return;
  cert_paths = strdup(cert_paths); // Modified during tokenization
  if (cert_paths == NULL)
    return;
  
  char* cert_dir_path;
  DIR *dp;
  struct dirent *entry;
  int vfd = c->verbose_fd;
  
  char *strtok_context = NULL;
  for(cert_dir_path = strtok_r(cert_paths, ":", &strtok_context);
      cert_dir_path != NULL;
      cert_dir_path = strtok_r(NULL, ":", &strtok_context))
    {
      dp = opendir(cert_dir_path);
      if(!dp) continue;
      while((entry = readdir(dp)))
        {
          // Only consider regular files with common x509 cert extensions
          if(entry->d_type != DT_REG || 0 != fnmatch("*.@(der|pem|crt|cer|cert)", entry->d_name, FNM_EXTMATCH)) continue;
          char certfile[PATH_MAX];
          strncpy(certfile, cert_dir_path, PATH_MAX - 1);
          if(certfile[strlen(certfile)-1] != '/') certfile[strlen(certfile)] = '/';
          strncat(certfile, entry->d_name, PATH_MAX - strlen(certfile) - 1);
          certfile[strlen(certfile)] = '\0';
          
          FILE *cert_fp = fopen(certfile, "r");
          if(!cert_fp) continue;

          X509 *x509 = NULL;
          EVP_PKEY *pkey = NULL;
          char *fmt = "";
          // Attempt to read the fp as DER
          if(d2i_X509_fp(cert_fp, &x509))
            fmt = "der ";
          // Attempt to read the fp as PEM and assuming the key matches that of the signature add this key to be used
          // Note we fseek since this is the second time we read from the fp
          else if(0 == fseek(cert_fp, 0, SEEK_SET) && PEM_read_X509(cert_fp, &x509, NULL, NULL))
            fmt = "pem "; // PEM with full certificate
          else if(0 == fseek(cert_fp, 0, SEEK_SET) && PEM_read_PUBKEY(cert_fp, &pkey, NULL, NULL)) 
            fmt = "pem "; // some PEM files have just a PUBLIC KEY in them
          fclose(cert_fp);

          if (x509)
            {
              struct public_key_entry *ne = calloc(1, sizeof(struct public_key_entry));
              if (ne)
                {
                  ne->key = X509_extract_key(x509);
                  ne->keyid = extract_skid(x509);
                  ne->next = c->ima_public_keys;
                  c->ima_public_keys = ne;
                  if (vfd >= 0)
                    dprintf(vfd, "Loaded %scertificate %s, keyid = %04x\n", fmt, certfile, ne->keyid);
                }
              X509_free (x509);
            }
          else if (pkey)
            {
              struct public_key_entry *ne = calloc(1, sizeof(struct public_key_entry));
              if (ne)
                {
                  ne->key = pkey; // preserve refcount
                  ne->keyid = extract_skid_pk(pkey);
                  ne->next = c->ima_public_keys;
                  c->ima_public_keys = ne;
                  if (vfd >= 0)
                    dprintf(vfd, "Loaded %spubkey %s, keyid %04x\n", fmt, certfile, ne->keyid);
                }
            }
          else
            {
              if (vfd >= 0)
                dprintf(vfd, "Cannot load certificate %s\n", certfile);
            }
        } /* for each file in directory */
      closedir(dp);
    } /* for each directory */
  
  free(cert_paths);
}


static void free_ima_public_keys (debuginfod_client *c)
{
  while (c->ima_public_keys)
    {
      EVP_PKEY_free (c->ima_public_keys->key);
      struct public_key_entry *oen = c->ima_public_keys->next;
      free (c->ima_public_keys);
      c->ima_public_keys = oen;
    }
}
#endif



static size_t
debuginfod_write_callback (char *ptr, size_t size, size_t nmemb, void *data)
{
  ssize_t count = size * nmemb;

  struct handle_data *d = (struct handle_data*)data;

  /* Indicate to other handles that they can abort their transfer.  */
  if (*d->target_handle == NULL)
    {
      *d->target_handle = d->handle;
      /* update the client object */
      const char *url = NULL;
      CURLcode curl_res = curl_easy_getinfo (d->handle,
                                             CURLINFO_EFFECTIVE_URL, &url);
      if (curl_res == CURLE_OK && url)
        {
          free (d->client->url);
          d->client->url = strdup(url); /* ok if fails */
        }
    }

  /* If this handle isn't the target handle, abort transfer.  */
  if (*d->target_handle != d->handle)
    return -1;

  return (size_t) write(d->fd, (void*)ptr, count);
}

/* handle config file read and write */
static int
debuginfod_config_cache(debuginfod_client *c, char *config_path,
			long cache_config_default_s,
			struct stat *st)
{
  int fd = open(config_path, O_CREAT | O_RDWR, DEFFILEMODE);
  if (fd < 0)
    return -errno;

  if (fstat (fd, st) < 0)
    {
      int ret = -errno;
      close (fd);
      return ret;
    }

  if (st->st_size == 0)
    {
      if (dprintf(fd, "%ld", cache_config_default_s) < 0)
	{
	  int ret = -errno;
	  close (fd);
	  return ret;
	}

      close (fd);
      return cache_config_default_s;
    }

  long cache_config;
  /* PR29696 - NB: When using fdopen, the file descriptor is NOT
     dup'ed and will be closed when the stream is closed. Manually
     closing fd after fclose is called will lead to a race condition
     where, if reused, the file descriptor will compete for its
     regular use before being incorrectly closed here.  */
  FILE *config_file = fdopen(fd, "r");
  if (config_file)
    {
      if (fscanf(config_file, "%ld", &cache_config) != 1)
	cache_config = cache_config_default_s;
      if (0 != fclose (config_file) && c->verbose_fd >= 0)
	dprintf (c->verbose_fd, "fclose failed with %s (err=%d)\n",
		 strerror (errno), errno);
    }
  else
    {
      cache_config = cache_config_default_s;
      if (0 != close (fd) && c->verbose_fd >= 0)
	dprintf (c->verbose_fd, "close failed with %s (err=%d)\n",
		 strerror (errno), errno);
    }
  return cache_config;
}

/* Delete any files that have been unmodied for a period
   longer than $DEBUGINFOD_CACHE_CLEAN_INTERVAL_S.  */
static int
debuginfod_clean_cache(debuginfod_client *c,
		       char *cache_path, char *interval_path,
		       char *max_unused_path)
{
  time_t clean_interval, max_unused_age;
  int rc = -1;
  struct stat st;

  /* Create new interval file.  */
  rc = debuginfod_config_cache(c, interval_path,
			       cache_clean_default_interval_s, &st);
  if (rc < 0)
    return rc;
  clean_interval = (time_t)rc;

  /* Check timestamp of interval file to see whether cleaning is necessary.  */
  if (time(NULL) - st.st_mtime < clean_interval)
    /* Interval has not passed, skip cleaning.  */
    return 0;

  /* Update timestamp representing when the cache was last cleaned.
     Do it at the start to reduce the number of threads trying to do a
     cleanup simultaneously.  */
  utime (interval_path, NULL);

  /* Read max unused age value from config file.  */
  rc = debuginfod_config_cache(c, max_unused_path,
			       cache_default_max_unused_age_s, &st);
  if (rc < 0)
    return rc;
  max_unused_age = (time_t)rc;

  char * const dirs[] = { cache_path, NULL, };

  FTS *fts = fts_open(dirs, 0, NULL);
  if (fts == NULL)
    return -errno;

  regex_t re;
  const char * pattern = ".*/(metadata.*|[a-f0-9]+(/hdr.*|/debuginfo|/executable|/source.*|))$"; /* include dirs */
  /* NB: also matches .../section/ subdirs, so extracted section files also get cleaned. */
  if (regcomp (&re, pattern, REG_EXTENDED | REG_NOSUB) != 0)
    return -ENOMEM;

  FTSENT *f;
  long files = 0;
  time_t now = time(NULL);
  while ((f = fts_read(fts)) != NULL)
    {
      /* ignore any files that do not match the pattern.  */
      if (regexec (&re, f->fts_path, 0, NULL, 0) != 0)
        continue;

      files++;
      if (c->progressfn) /* inform/check progress callback */
        if ((c->progressfn) (c, files, 0))
          break;

      switch (f->fts_info)
        {
        case FTS_F:
          /* delete file if max_unused_age has been met or exceeded w.r.t. atime.  */
          if (now - f->fts_statp->st_atime >= max_unused_age)
            (void) unlink (f->fts_path);
          break;

        case FTS_DP:
          /* Remove if old & empty.  Weaken race against concurrent creation by 
             checking mtime. */
          if (now - f->fts_statp->st_mtime >= max_unused_age)
            (void) rmdir (f->fts_path);
          break;

        default:
          ;
        }
    }
  fts_close (fts);
  regfree (&re);

  return 0;
}


#define MAX_BUILD_ID_BYTES 64


static void
add_default_headers(debuginfod_client *client)
{
  if (client->user_agent_set_p)
    return;

  /* Compute a User-Agent: string to send.  The more accurately this
     describes this host, the likelier that the debuginfod servers
     might be able to locate debuginfo for us. */

  char* utspart = NULL;
  struct utsname uts;
  int rc = 0;
  rc = uname (&uts);
  if (rc == 0)
    rc = asprintf(& utspart, "%s/%s", uts.sysname, uts.machine);
  if (rc < 0)
    utspart = NULL;

  FILE *f = fopen ("/etc/os-release", "r");
  if (f == NULL)
    f = fopen ("/usr/lib/os-release", "r");
  char *id = NULL;
  char *version = NULL;
  if (f != NULL)
    {
      while (id == NULL || version == NULL)
        {
          char buf[128];
          char *s = &buf[0];
          if (fgets (s, sizeof(buf), f) == NULL)
            break;

          int len = strlen (s);
          if (len < 3)
            continue;
          if (s[len - 1] == '\n')
            {
              s[len - 1] = '\0';
              len--;
            }

          char *v = strchr (s, '=');
          if (v == NULL || strlen (v) < 2)
            continue;

          /* Split var and value. */
          *v = '\0';
          v++;

          /* Remove optional quotes around value string. */
          if (*v == '"' || *v == '\'')
            {
              v++;
              s[len - 1] = '\0';
            }
          if (id == NULL && strcmp (s, "ID") == 0)
            id = strdup (v);
          if (version == NULL && strcmp (s, "VERSION_ID") == 0)
            version = strdup (v);
        }
      fclose (f);
    }

  char *ua = NULL;
  rc = asprintf(& ua, "User-Agent: %s/%s,%s,%s/%s",
                PACKAGE_NAME, PACKAGE_VERSION,
                utspart ?: "",
                id ?: "",
                version ?: "");
  if (rc < 0)
    ua = NULL;

  if (ua)
    (void) debuginfod_add_http_header (client, ua);

  free (ua);
  free (id);
  free (version);
  free (utspart);
}

/* Add HTTP headers found in the given file, one per line. Blank lines or invalid
 * headers are ignored.
 */
static void
add_headers_from_file(debuginfod_client *client, const char* filename)
{
  int vds = client->verbose_fd;
  FILE *f = fopen (filename, "r");
  if (f == NULL)
    {
      if (vds >= 0)
	dprintf(vds, "header file %s: %s\n", filename, strerror(errno));
      return;
    }

  while (1)
    {
      char buf[8192];
      char *s = &buf[0];
      if (feof(f))
        break;
      if (fgets (s, sizeof(buf), f) == NULL)
        break;
      for (char *c = s; *c != '\0'; ++c)
        if (!isspace(*c))
          goto nonempty;
      continue;
    nonempty:
      ;
      size_t last = strlen(s)-1;
      if (s[last] == '\n')
        s[last] = '\0';
      int rc = debuginfod_add_http_header(client, s);
      if (rc < 0 && vds >= 0)
        dprintf(vds, "skipping bad header: %s\n", strerror(-rc));
    }
  fclose (f);
}


#define xalloc_str(p, fmt, args...)        \
  do                                       \
    {                                      \
      if (asprintf (&p, fmt, args) < 0)    \
        {                                  \
          p = NULL;                        \
          rc = -ENOMEM;                    \
          goto out;                        \
        }                                  \
    } while (0)


/* Offer a basic form of progress tracing */
static int
default_progressfn (debuginfod_client *c, long a, long b)
{
  const char* url = debuginfod_get_url (c);
  int len = 0;

  /* We prefer to print the host part of the URL to keep the
     message short. */
  if (url != NULL)
    {
      const char* buildid = strstr(url, "buildid/");
      if (buildid != NULL)
        len = (buildid - url);
      else
        len = strlen(url);
    }

  if (b == 0 || url==NULL) /* early stage */
    dprintf(STDERR_FILENO,
            "\rDownloading %c", "-/|\\"[a % 4]);
  else if (b < 0) /* download in progress but unknown total length */
    dprintf(STDERR_FILENO,
            "\rDownloading from %.*s %ld",
            len, url, a);
  else /* download in progress, and known total length */
    dprintf(STDERR_FILENO,
            "\rDownloading from %.*s %ld/%ld",
            len, url, a, b);
  c->default_progressfn_printed_p = 1;

  return 0;
}

/* This is a callback function that receives http response headers in buffer for use
 * in this program. https://curl.se/libcurl/c/CURLOPT_HEADERFUNCTION.html is the
 * online documentation.
 */
static size_t
header_callback (char * buffer, size_t size, size_t numitems, void * userdata)
{
  struct handle_data *data = (struct handle_data *) userdata;
  if (size != 1)
    return 0;
  if (data->client
      && data->client->verbose_fd >= 0
      && numitems > 2)
    dprintf (data->client->verbose_fd, "header %.*s", (int)numitems, buffer);
  // Some basic checks to ensure the headers received are of the expected format
  if (strncasecmp(buffer, "X-DEBUGINFOD", 11)
      || buffer[numitems-2] != '\r'
      || buffer[numitems-1] != '\n'
      || (buffer == strstr(buffer, ":")) ){
    return numitems;
  }
  /* Temporary buffer for realloc */
  char *temp = NULL;
  temp = realloc(data->response_data, data->response_data_size + numitems);
  if (temp == NULL)
    return 0;

  memcpy(temp + data->response_data_size, buffer, numitems-1);
  data->response_data = temp;
  data->response_data_size += numitems-1;
  data->response_data[data->response_data_size-1] = '\n';
  data->response_data[data->response_data_size] = '\0';
  return numitems;
}


static size_t
metadata_callback (char * buffer, size_t size, size_t numitems, void * userdata)
{
  if (size != 1)
    return 0;
  /* Temporary buffer for realloc */
  char *temp = NULL;
  struct handle_data *data = (struct handle_data *) userdata;
  temp = realloc(data->metadata, data->metadata_size + numitems + 1);
  if (temp == NULL)
    return 0;
  
  memcpy(temp + data->metadata_size, buffer, numitems);
  data->metadata = temp;
  data->metadata_size += numitems;
  data->metadata[data->metadata_size] = '\0';
  return numitems;
}


/* This function takes a copy of DEBUGINFOD_URLS, server_urls, and
 * separates it into an array of urls to query, each with a
 * corresponding IMA policy. The url_subdir is either 'buildid' or
 * 'metadata', corresponding to the query type. Returns 0 on success
 * and -Posix error on failure.
 */
int
init_server_urls(char* url_subdir, const char* type,
                 char *server_urls, char ***server_url_list, ima_policy_t **url_ima_policies,
                 int *num_urls, int vfd)
{
  /* Initialize the memory to zero */
  char *strtok_saveptr;
  ima_policy_t verification_mode = ignore; // The default mode  
  char *server_url = strtok_r(server_urls, url_delim, &strtok_saveptr);
  /* Count number of URLs.  */
  int n = 0;

  while (server_url != NULL)
    {
      // When we encountered a (well-formed) token off the form
      // ima:foo, we update the policy under which results from that
      // server will be ima verified
      if (startswith(server_url, "ima:"))
        {
#ifdef ENABLE_IMA_VERIFICATION
          ima_policy_t m = ima_policy_str2enum(server_url + strlen("ima:"));
          if(m != undefined)
            verification_mode = m;
          else if (vfd >= 0)
            dprintf(vfd, "IMA mode not recognized, skipping %s\n", server_url);
#else
          if (vfd >= 0)
            dprintf(vfd, "IMA signature verification is not enabled, treating %s as ima:ignore\n", server_url);
#endif
          goto continue_next_url;
        }

      if (verification_mode==enforcing &&
          0==strcmp(url_subdir, "buildid") &&
          0==strcmp(type,"section")) // section queries are unsecurable
        {
          if (vfd >= 0)
            dprintf(vfd, "skipping server %s section query in IMA enforcing mode\n", server_url);
          goto continue_next_url;
        }

      // Construct actual URL for libcurl
      int r;
      char *tmp_url;
      if (strlen(server_url) > 1 && server_url[strlen(server_url)-1] == '/')
        r = asprintf(&tmp_url, "%s%s", server_url, url_subdir);
      else
        r = asprintf(&tmp_url, "%s/%s", server_url, url_subdir);

      if (r == -1)
        return -ENOMEM;
      
      /* PR 27983: If the url is duplicate, skip it */
      int url_index;
      for (url_index = 0; url_index < n; ++url_index)
        {
          if(strcmp(tmp_url, (*server_url_list)[url_index]) == 0)
            {
              url_index = -1;
              break;
            }
        }
      if (url_index == -1)
        {
          if (vfd >= 0)
            dprintf(vfd, "duplicate url: %s, skipping\n", tmp_url);
          free(tmp_url);
        }
      else
        {
          /* Have unique URL, save it, along with its IMA verification tag. */
          n ++;
          if (NULL == (*server_url_list = reallocarray(*server_url_list, n, sizeof(char*)))
              || NULL == (*url_ima_policies = reallocarray(*url_ima_policies, n, sizeof(ima_policy_t))))
            {
              free (tmp_url);
              return -ENOMEM;
            }
          (*server_url_list)[n-1] = tmp_url;
          if(NULL != url_ima_policies) (*url_ima_policies)[n-1] = verification_mode;
        }

    continue_next_url:
      server_url = strtok_r(NULL, url_delim, &strtok_saveptr);
    }
  *num_urls = n;
  return 0;
}

/* Some boilerplate for checking curl_easy_setopt.  */
#define curl_easy_setopt_ck(H,O,P) do {			\
      CURLcode curl_res = curl_easy_setopt (H,O,P);	\
      if (curl_res != CURLE_OK)				\
	    {						\
	      if (vfd >= 0)				\
	        dprintf (vfd,				\
                         "Bad curl_easy_setopt: %s\n",	\
                         curl_easy_strerror(curl_res));	\
	      return -EINVAL;				\
	    }						\
      } while (0)


/*
 * This function initializes a CURL handle. It takes optional callbacks for the write
 * function and the header function, which if defined will use userdata of type struct handle_data*.
 * Specifically the data[i] within an array of struct handle_data's.
 * Returns 0 on success and -Posix error on failure.
 */
int
init_handle(debuginfod_client *client,
  size_t (*w_callback)(char *buffer, size_t size, size_t nitems, void *userdata),
  size_t (*h_callback)(char *buffer, size_t size, size_t nitems, void *userdata),
  struct handle_data *data, int i, long timeout,
  int vfd)
{
  data->handle = curl_easy_init();
  if (data->handle == NULL)
    return -ENETUNREACH;

  if (vfd >= 0)
    dprintf (vfd, "url %d %s\n", i, data->url);

  /* Only allow http:// + https:// + file:// so we aren't being
    redirected to some unsupported protocol.
    libcurl will fail if we request a single protocol that is not
    available. https missing is the most likely issue  */
#if CURL_AT_LEAST_VERSION(7, 85, 0)
  curl_easy_setopt_ck(data->handle, CURLOPT_PROTOCOLS_STR,
                      curl_has_https ? "https,http,file" : "http,file");
#else
  curl_easy_setopt_ck(data->handle, CURLOPT_PROTOCOLS,
                      ((curl_has_https ? CURLPROTO_HTTPS : 0) | CURLPROTO_HTTP | CURLPROTO_FILE));
#endif
  curl_easy_setopt_ck(data->handle, CURLOPT_URL, data->url);
  if (vfd >= 0)
    curl_easy_setopt_ck(data->handle, CURLOPT_ERRORBUFFER,
      data->errbuf);
  if (w_callback)
    {
      curl_easy_setopt_ck(data->handle,
                          CURLOPT_WRITEFUNCTION, w_callback);
      curl_easy_setopt_ck(data->handle, CURLOPT_WRITEDATA, data);
    }
  if (timeout > 0)
    {
      /* Make sure there is at least some progress,
         try to get at least 100K per timeout seconds.  */
      curl_easy_setopt_ck (data->handle, CURLOPT_LOW_SPEED_TIME,
                           timeout);
      curl_easy_setopt_ck (data->handle, CURLOPT_LOW_SPEED_LIMIT,
                           100 * 1024L);
    }
  curl_easy_setopt_ck(data->handle, CURLOPT_FILETIME, (long) 1);
  curl_easy_setopt_ck(data->handle, CURLOPT_FOLLOWLOCATION, (long) 1);
  curl_easy_setopt_ck(data->handle, CURLOPT_FAILONERROR, (long) 1);
  curl_easy_setopt_ck(data->handle, CURLOPT_NOSIGNAL, (long) 1);
  if (h_callback)
    {
      curl_easy_setopt_ck(data->handle,
                          CURLOPT_HEADERFUNCTION, h_callback);
      curl_easy_setopt_ck(data->handle, CURLOPT_HEADERDATA, data);
    }
  #if LIBCURL_VERSION_NUM >= 0x072a00 /* 7.42.0 */
  curl_easy_setopt_ck(data->handle, CURLOPT_PATH_AS_IS, (long) 1);
  #else
  /* On old curl; no big deal, canonicalization here is almost the
      same, except perhaps for ? # type decorations at the tail. */
  #endif
  curl_easy_setopt_ck(data->handle, CURLOPT_AUTOREFERER, (long) 1);
  curl_easy_setopt_ck(data->handle, CURLOPT_ACCEPT_ENCODING, "");
  curl_easy_setopt_ck(data->handle, CURLOPT_HTTPHEADER, client->headers);

  return 0;
}


/*
 * This function busy-waits on one or more curl queries to complete. This can
 * be controlled via only_one, which, if true, will find the first winner and exit
 * once found. If positive maxtime and maxsize dictate the maximum allowed wait times
 * and download sizes respectively. Returns 0 on success and -Posix error on failure.
 */
int
perform_queries(CURLM *curlm, CURL **target_handle, struct handle_data *data, debuginfod_client *c,
                int num_urls, long maxtime, long maxsize, bool only_one, int vfd, int *committed_to)
{
  int still_running = -1;
  long loops = 0;
  *committed_to = -1;
  bool verbose_reported = false;
  struct timespec start_time, cur_time;
  if (c->winning_headers != NULL)
    {
      free (c->winning_headers);
      c->winning_headers = NULL;
    }
  if (maxtime > 0 && clock_gettime(CLOCK_MONOTONIC_RAW, &start_time) == -1)
    return -errno;
  long delta = 0;
  do
    {
      /* Check to see how long querying is taking. */
      if (maxtime > 0)
        {
          if (clock_gettime(CLOCK_MONOTONIC_RAW, &cur_time) == -1)
            return -errno;
          delta = cur_time.tv_sec - start_time.tv_sec;
          if ( delta >  maxtime)
            {
              dprintf(vfd, "Timeout with max time=%lds and transfer time=%lds\n", maxtime, delta );
              return -ETIME;
            }
        }
      /* Wait 1 second, the minimum DEBUGINFOD_TIMEOUT.  */
      curl_multi_wait(curlm, NULL, 0, 1000, NULL);
      CURLMcode curlm_res = curl_multi_perform(curlm, &still_running);
      
      if (only_one)
        {
          /* If the target file has been found, abort the other queries.  */
          if (target_handle && *target_handle != NULL)
            {
              for (int i = 0; i < num_urls; i++)
                if (data[i].handle != *target_handle)
                  curl_multi_remove_handle(curlm, data[i].handle);
                else
                  {
                    *committed_to = i;
                    if (c->winning_headers == NULL)
                      {
                        c->winning_headers = data[*committed_to].response_data;
                        if (vfd >= 0 && c->winning_headers != NULL)
                          dprintf(vfd, "\n%s", c->winning_headers);
                        data[*committed_to].response_data = NULL;
                        data[*committed_to].response_data_size = 0;
                      }
                  }
            }
          
          if (vfd >= 0 && !verbose_reported && *committed_to >= 0)
            {
              bool pnl = (c->default_progressfn_printed_p && vfd == STDERR_FILENO);
              dprintf (vfd, "%scommitted to url %d\n", pnl ? "\n" : "",
                       *committed_to);
              if (pnl)
                c->default_progressfn_printed_p = 0;
              verbose_reported = true;
            }
        }
      
      if (curlm_res != CURLM_OK)
        {
          switch (curlm_res)
            {
            case CURLM_CALL_MULTI_PERFORM: continue;
            case CURLM_OUT_OF_MEMORY: return -ENOMEM;
            default: return -ENETUNREACH;
            }
        }
      
      long dl_size = -1;
      if (target_handle && *target_handle && (c->progressfn || maxsize > 0))
        {
          /* Get size of file being downloaded. NB: If going through
             deflate-compressing proxies, this number is likely to be
             unavailable, so -1 may show. */
          CURLcode curl_res;
#if CURL_AT_LEAST_VERSION(7, 55, 0)
          curl_off_t cl;
          curl_res = curl_easy_getinfo(*target_handle,
                                       CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,
                                       &cl);
          if (curl_res == CURLE_OK && cl >= 0)
            dl_size = (cl > LONG_MAX ? LONG_MAX : (long)cl);
#else
          double cl;
          curl_res = curl_easy_getinfo(*target_handle,
                                       CURLINFO_CONTENT_LENGTH_DOWNLOAD,
                                       &cl);
          if (curl_res == CURLE_OK && cl >= 0)
            dl_size = (cl >= (double)(LONG_MAX+1UL) ? LONG_MAX : (long)cl);
#endif
          /* If Content-Length is -1, try to get the size from
             X-Debuginfod-Size */
          if (dl_size == -1 && c->winning_headers != NULL)
            {
              long xdl;
              char *hdr = strcasestr(c->winning_headers, "x-debuginfod-size");
              size_t off = strlen("x-debuginfod-size:");
              
              if (hdr != NULL && sscanf(hdr + off, "%ld", &xdl) == 1)
                dl_size = xdl;
            }
        }
      
      if (c->progressfn) /* inform/check progress callback */
        {
          loops ++;
          long pa = loops; /* default param for progress callback */
          if (target_handle && *target_handle) /* we've committed to a server; report its download progress */
            {
              /* PR30809: Check actual size of cached file.  This same
                 fd is shared by all the multi-curl handles (but only
                 one will end up writing to it).  Another way could be
                 to tabulate totals in debuginfod_write_callback(). */
              struct stat cached;
              int statrc = fstat(data[*committed_to].fd, &cached);
              if (statrc == 0)
                pa = (long) cached.st_size;
              else
                {
                  /* Otherwise, query libcurl for its tabulated total.
                     However, that counts http body length, not
                     decoded/decompressed content length, so does not
                     measure quite the same thing as dl. */
                  CURLcode curl_res;
#if CURL_AT_LEAST_VERSION(7, 55, 0)
                  curl_off_t dl;
                  curl_res = curl_easy_getinfo(target_handle,
                                               CURLINFO_SIZE_DOWNLOAD_T,
                                               &dl);
                  if (curl_res == 0 && dl >= 0)
                    pa = (dl > LONG_MAX ? LONG_MAX : (long)dl);
#else
                  double dl;
                  curl_res = curl_easy_getinfo(target_handle,
                                               CURLINFO_SIZE_DOWNLOAD,
                                               &dl);
                  if (curl_res == 0)
                    pa = (dl >= (double)(LONG_MAX+1UL) ? LONG_MAX : (long)dl);
#endif
                }
              
              if ((*c->progressfn) (c, pa, dl_size == -1 ? 0 : dl_size))
		{
		  c->progressfn_cancel = true;
		  break;
		}
            }
        }
      /* Check to see if we are downloading something which exceeds maxsize, if set.*/
      if (target_handle && *target_handle && dl_size > maxsize && maxsize > 0)
        {
          if (vfd >=0)
            dprintf(vfd, "Content-Length too large.\n");
          return -EFBIG;
        }
    } while (still_running);
  
  return 0;
}


/* Copy SRC to DEST, s,/,#,g */

static void
path_escape (const char *src, char *dest, size_t dest_len)
{
  /* PR32218: Reversibly-escaping src character-by-character, for
     large enough strings, risks ENAMETOOLONG errors.  For long names,
     a simple hash based generated name instead, while still
     attempting to preserve the as much of the content as possible.
     It's possible that absurd choices of incoming names will collide
     or still get truncated, but c'est la vie.
  */
  
  /* Compute a three-way min() for the actual output string generated. */
  assert (dest_len > 10); /* Space enough for degenerate case of
                             "HASHHASH-\0". NB: dest_len is not
                             user-controlled. */
  /* Use only NAME_MAX/2 characters in the output file name.
     ENAMETOOLONG has been observed even on 300-ish character names on
     some filesystems. */
  const size_t max_dest_len = NAME_MAX/2;
  dest_len = dest_len > max_dest_len ? max_dest_len : dest_len;
  /* Use only strlen(src)+10 bytes, if that's smaller.  Yes, we could
     just fit an entire escaped name in there in theory, without the
     hash+etc.  But then again the hashing protects against #-escape
     aliasing collisions: "foo[bar" "foo]bar" both escape to
     "foo#bar", thus aliasing, but have different "HASH-foo#bar".
  */
  const size_t hash_prefix_destlen = strlen(src)+10; /* DEADBEEF-src\0 */
  dest_len = dest_len > hash_prefix_destlen ? hash_prefix_destlen : dest_len;

  char *dest_write = dest + dest_len - 1;
  (*dest_write--) = '\0'; /* Ensure a \0 there. */

  /* Copy from back toward front, preferring to keep the .extension. */
  for (int fi=strlen(src)-1; fi >= 0 && dest_write >= dest; fi--)
    {
      char src_char = src[fi];
      switch (src_char)
        {
          /* Pass through ordinary identifier chars. */
        case '.': case '-': case '_':
        case 'a'...'z':
        case 'A'...'Z':
        case '0'...'9':
          *dest_write-- = src_char;
          break;
          
          /* Replace everything else, esp. security-sensitive /. */
        default:
          *dest_write-- = '#';
          break;
        }
    }

  /* djb2 hash algorithm: DJBX33A */
  unsigned long hash = 5381;
  const char *c = src;
  while (*c)
    hash = ((hash << 5) + hash) + *c++;
  char name_hash_str [9];
  /* Truncate to 4 bytes; along with the remaining hundredish bytes of text,
     should be ample against accidental collisions. */
  snprintf (name_hash_str, sizeof(name_hash_str), "%08x", (unsigned int) hash);
  memcpy (&dest[0], name_hash_str, 8); /* Overwrite the first few characters */
  dest[8] = '-'; /* Add a bit of punctuation to make hash stand out */
}

/* Attempt to update the atime */
static void
update_atime (int fd)
{
  struct timespec tvs[2];

  tvs[0].tv_sec = tvs[1].tv_sec = 0;
  tvs[0].tv_nsec = UTIME_NOW;
  tvs[1].tv_nsec = UTIME_OMIT;

  (void) futimens (fd, tvs);  /* best effort */
}

/* Attempt to read an ELF/DWARF section with name SECTION from FD and write
   it to a separate file in the debuginfod cache.  If successful the absolute
   path of the separate file containing SECTION will be stored in USR_PATH.
   FD_PATH is the absolute path for FD.

   If the section cannot be extracted, then return a negative error code.
   -ENOENT indicates that the parent file was able to be read but the
   section name was not found.  -EEXIST indicates that the section was
   found but had type SHT_NOBITS.  */

static int
extract_section (int fd, const char *section, char *fd_path, char **usr_path)
{
  elf_version (EV_CURRENT);

  Elf *elf = elf_begin (fd, ELF_C_READ_MMAP_PRIVATE, NULL);
  if (elf == NULL)
    return -EIO;

  size_t shstrndx;
  int rc = elf_getshdrstrndx (elf, &shstrndx);
  if (rc < 0)
    {
      rc = -EIO;
      goto out;
    }

  int sec_fd = -1;
  char *sec_path_tmp = NULL;
  Elf_Scn *scn = NULL;

  /* Try to find the target section and copy the contents into a
     separate file.  */
  while (true)
    {
      scn = elf_nextscn (elf, scn);
      if (scn == NULL)
	{
	  rc = -ENOENT;
	  goto out;
	}
      GElf_Shdr shdr_storage;
      GElf_Shdr *shdr = gelf_getshdr (scn, &shdr_storage);
      if (shdr == NULL)
	{
	  rc = -EIO;
	  goto out;
	}

      const char *scn_name = elf_strptr (elf, shstrndx, shdr->sh_name);
      if (scn_name == NULL)
	{
	  rc = -EIO;
	  goto out;
	}
      if (strcmp (scn_name, section) == 0)
	{
	  /* We found the desired section.  */
	  if (shdr->sh_type == SHT_NOBITS)
	    {
	      rc = -EEXIST;
	      goto out;
	    }

	  Elf_Data *data = NULL;
	  data = elf_rawdata (scn, NULL);
	  if (data == NULL)
	    {
	      rc = -EIO;
	      goto out;
	    }

	  if (data->d_buf == NULL)
	    {
	      rc = -EIO;
	      goto out;
	    }

	  /* Compute the absolute filename we'll write the section to.
	     Replace the last component of FD_PATH with the path-escaped
	     section filename.  */
	  int i = strlen (fd_path);
          while (i >= 0)
	    {
	      if (fd_path[i] == '/')
		{
		  fd_path[i] = '\0';
		  break;
		}
	      --i;
	    }

          char suffix[NAME_MAX];
	  path_escape (section, suffix, sizeof(suffix));
	  rc = asprintf (&sec_path_tmp, "%s/section-%s.XXXXXX",
			 fd_path, suffix);
	  if (rc == -1)
	    {
	      rc = -ENOMEM;
	      goto out1;
	    }

	  sec_fd = mkstemp (sec_path_tmp);
	  if (sec_fd < 0)
	    {
	      rc = -EIO;
	      goto out2;
	    }

	  ssize_t res = write_retry (sec_fd, data->d_buf, data->d_size);
	  if (res < 0 || (size_t) res != data->d_size)
	    {
	      rc = -EIO;
	      goto out3;
	    }

	  /* Success.  Rename tmp file and update USR_PATH.  */
	  char *sec_path;
	  if (asprintf (&sec_path, "%s/section-%s", fd_path, section) == -1)
	    {
	      rc = -ENOMEM;
	      goto out3;
	    }

	  rc = rename (sec_path_tmp, sec_path);
	  if (rc < 0)
	    {
	      free (sec_path);
	      rc = -EIO;
	      goto out3;
	    }

	  if (usr_path != NULL)
	    *usr_path = sec_path;
	  else
	    free (sec_path);
	  update_atime(fd);
	  rc = sec_fd;
	  goto out2;
	}
    }

out3:
  close (sec_fd);
  unlink (sec_path_tmp);

out2:
  free (sec_path_tmp);

out1:
out:
  elf_end (elf);
  return rc;
}

/* Search TARGET_CACHE_DIR for a debuginfo or executable file containing
   an ELF/DWARF section with name SCN_NAME.  If found, extract the section
   to a separate file in TARGET_CACHE_DIR and return a file descriptor
   for the section file. The path for this file will be stored in USR_PATH.
   Return a negative errno if unsuccessful.  -ENOENT indicates that SCN_NAME
   is confirmed to not exist.  */

static int
cache_find_section (const char *scn_name, const char *target_cache_dir,
		    char **usr_path)
{
  int debug_fd;
  int rc = -EEXIST;
  char parent_path[PATH_MAX];

  /* Check the debuginfo first.  */
  snprintf (parent_path, PATH_MAX, "%s/debuginfo", target_cache_dir);
  debug_fd = open (parent_path, O_RDONLY);
  if (debug_fd >= 0)
    {
      rc = extract_section (debug_fd, scn_name, parent_path, usr_path);
      close (debug_fd);
    }

  /* If the debuginfo file couldn't be found or the section type was
     SHT_NOBITS, check the executable.  */
  if (rc == -EEXIST)
    {
      snprintf (parent_path, PATH_MAX, "%s/executable", target_cache_dir);
      int exec_fd = open (parent_path, O_RDONLY);

      if (exec_fd >= 0)
	{
	  rc = extract_section (exec_fd, scn_name, parent_path, usr_path);
	  close (exec_fd);

	  /* Don't return -ENOENT if the debuginfo wasn't opened.  The
	     section may exist in the debuginfo but not the executable.  */
	  if (debug_fd < 0 && rc == -ENOENT)
	    rc = -EREMOTE;
	}
    }

  return rc;
}


#ifdef ENABLE_IMA_VERIFICATION
/* Extract the hash algorithm name from the signature header, of which
   there are several types.  The name will be used for openssl hashing
   of the file content.  The header doesn't need to be super carefully
   parsed, because if any part of it is wrong, be it the hash
   algorithm number or hash value or whatever, it will fail
   computation or verification.  Return NULL in case of error.  */
static const char*
get_signature_params(debuginfod_client *c, unsigned char *bin_sig)
{
  int hashalgo = 0;
  
  switch (bin_sig[0])
    {
    case EVM_IMA_XATTR_DIGSIG:
#ifdef IMA_VERITY_DIGSIG /* missing on debian-i386 trybot */
    case IMA_VERITY_DIGSIG:
#endif
      break;
    default:
      if (c->verbose_fd >= 0)
        dprintf (c->verbose_fd, "Unknown ima digsig %d\n", (int)bin_sig[0]);
      return NULL;
    }

  switch (bin_sig[1])
    {
    case DIGSIG_VERSION_2:
      {
        struct signature_v2_hdr hdr_v2;
        memcpy(& hdr_v2, & bin_sig[1], sizeof(struct signature_v2_hdr));
        hashalgo = hdr_v2.hash_algo;
        break;
      }
    default:
      if (c->verbose_fd >= 0)
        dprintf (c->verbose_fd, "Unknown ima signature version %d\n", (int)bin_sig[1]);
      return NULL;
    }

  switch (hashalgo)
    {
    case PKEY_HASH_SHA1: return "sha1";
    case PKEY_HASH_SHA256: return "sha256";
      // (could add many others from enum pkey_hash_algo)
    default:
      if (c->verbose_fd >= 0)
        dprintf (c->verbose_fd, "Unknown ima pkey hash %d\n", hashalgo);
      return NULL;
    }
}


/* Verify given hash against given signature blob.  Return 0 on ok, -errno otherwise. */
static int
debuginfod_verify_hash(debuginfod_client *c, const unsigned char *hash, int size,
                       const char *hash_algo, unsigned char *sig, int siglen)
{
  int ret = -EBADMSG;
  struct public_key_entry *pkey;
  struct signature_v2_hdr hdr;
  EVP_PKEY_CTX *ctx;
  const EVP_MD *md;

  memcpy(&hdr, sig, sizeof(struct signature_v2_hdr)); /* avoid just aliasing */

  if (c->verbose_fd >= 0)
    dprintf (c->verbose_fd, "Searching for ima keyid %04x\n", ntohl(hdr.keyid));
        
  /* Find the matching public key. */
  for (pkey = c->ima_public_keys; pkey != NULL; pkey = pkey->next)
    if (pkey->keyid == ntohl(hdr.keyid)) break;
  if (!pkey)
    return -ENOKEY;

  if (!(ctx = EVP_PKEY_CTX_new(pkey->key, NULL)))
    goto err;
  if (!EVP_PKEY_verify_init(ctx))
    goto err;
  if (!(md = EVP_get_digestbyname(hash_algo)))
    goto err;
  if (!EVP_PKEY_CTX_set_signature_md(ctx, md))
    goto err;
  ret = EVP_PKEY_verify(ctx, sig + sizeof(hdr),
                        siglen - sizeof(hdr), hash, size);
  if (ret == 1)
    ret = 0; // success!
  else if (ret == 0)
    ret = -EBADMSG;
 err:
  EVP_PKEY_CTX_free(ctx);
  return ret;
}



/* Validate an IMA file signature.
 * Returns 0 on signature validity, -EINVAL on signature invalidity, -ENOSYS on undefined imaevm machinery,
 * -ENOKEY on key issues, or other -errno.
 */

static int
debuginfod_validate_imasig (debuginfod_client *c, int fd)
{
  int rc = ENOSYS;

    char* sig_buf = NULL;
    EVP_MD_CTX *ctx = NULL;
    if (!c || !c->winning_headers)
    {
      rc = -ENODATA;
      goto exit_validate;
    }
    // Extract the HEX IMA-signature from the header
    char* hdr_ima_sig = strcasestr(c->winning_headers, "x-debuginfod-imasignature");
    if (!hdr_ima_sig || 1 != sscanf(hdr_ima_sig + strlen("x-debuginfod-imasignature:"), "%ms", &sig_buf))
    {
      rc = -ENODATA;
      goto exit_validate;
    }
    if (strlen(sig_buf) > MAX_SIGNATURE_SIZE) // reject if too long
    {
      rc = -EBADMSG;
      goto exit_validate;
    }
    // Convert the hex signature to bin
    size_t bin_sig_len = strlen(sig_buf)/2;
    unsigned char bin_sig[MAX_SIGNATURE_SIZE/2];
    for (size_t b = 0; b < bin_sig_len; b++)
      bin_sig[b] = (hex2dec(sig_buf[2*b]) << 4) | hex2dec(sig_buf[2*b+1]);

    // Compute the binary digest of the cached file (with file descriptor fd)
    ctx = EVP_MD_CTX_new();
    const char* sighash_name = get_signature_params(c, bin_sig) ?: "";
    const EVP_MD *md = EVP_get_digestbyname(sighash_name);
    if (!ctx || !md || !EVP_DigestInit(ctx, md))
    {
      rc = -EBADMSG;
      goto exit_validate;
    }

    long data_len;
    char* hdr_data_len = strcasestr(c->winning_headers, "x-debuginfod-size");
    if (!hdr_data_len || 1 != sscanf(hdr_data_len + strlen("x-debuginfod-size:") , "%ld", &data_len))
    {
      rc = -ENODATA;
      goto exit_validate;
    }

    char file_data[DATA_SIZE]; // imaevm.h data chunk hash size 
    ssize_t n;
    for(off_t k = 0; k < data_len; k += n)
      {
        if (-1 == (n = pread(fd, file_data, DATA_SIZE, k)))
          {
            rc = -errno;
            goto exit_validate;
          }
        
        if (!EVP_DigestUpdate(ctx, file_data, n))
          {
            rc = -EBADMSG;
            goto exit_validate;
          }
      }
    
    uint8_t bin_dig[MAX_DIGEST_SIZE];
    unsigned int bin_dig_len;
    if (!EVP_DigestFinal(ctx, bin_dig, &bin_dig_len))
    {
      rc = -EBADMSG;
      goto exit_validate;
    }

    // XXX: in case of DIGSIG_VERSION_3, need to hash the file hash, yo dawg
    
    int res = debuginfod_verify_hash(c,
                                     bin_dig, bin_dig_len,
                                     sighash_name,
                                     & bin_sig[1], bin_sig_len-1); // skip over first byte of signature
    if (c->verbose_fd >= 0)
      dprintf (c->verbose_fd, "Computed ima signature verification res=%d\n", res);
    rc = res;

 exit_validate:
    free (sig_buf);
    EVP_MD_CTX_free(ctx);
    return rc;
}
#endif /* ENABLE_IMA_VERIFICATION */




/* Helper function to create client cache directory.
   $XDG_CACHE_HOME takes priority over $HOME/.cache.
   $DEBUGINFOD_CACHE_PATH takes priority over $HOME/.cache and $XDG_CACHE_HOME.

   Return resulting path name or NULL on error.  Caller must free resulting string.
 */
static char *
make_cache_path(void)
{
  char* cache_path = NULL;
  int rc = 0;
  /* Determine location of the cache. The path specified by the debuginfod
     cache environment variable takes priority.  */
  char *cache_var = getenv(DEBUGINFOD_CACHE_PATH_ENV_VAR);
  if (cache_var != NULL && strlen (cache_var) > 0)
    xalloc_str (cache_path, "%s", cache_var);
  else
    {
      /* If a cache already exists in $HOME ('/' if $HOME isn't set), then use
         that. Otherwise use the XDG cache directory naming format.  */
      xalloc_str (cache_path, "%s/%s", getenv ("HOME") ?: "/", cache_default_name);

      struct stat st;
      if (stat (cache_path, &st) < 0)
        {
          char cachedir[PATH_MAX];
          char *xdg = getenv ("XDG_CACHE_HOME");

          if (xdg != NULL && strlen (xdg) > 0)
            snprintf (cachedir, PATH_MAX, "%s", xdg);
          else
            snprintf (cachedir, PATH_MAX, "%s/.cache", getenv ("HOME") ?: "/");

          /* Create XDG cache directory if it doesn't exist.  */
          if (stat (cachedir, &st) == 0)
            {
              if (! S_ISDIR (st.st_mode))
                {
                  rc = -EEXIST;
                  goto out1;
                }
            }
          else
            {
              rc = mkdir (cachedir, 0700);

              /* Also check for EEXIST and S_ISDIR in case another client just
                 happened to create the cache.  */
              if (rc < 0
                  && (errno != EEXIST
                      || stat (cachedir, &st) != 0
                      || ! S_ISDIR (st.st_mode)))
                {
                  rc = -errno;
                  goto out1;
                }
            }

          free (cache_path);
          xalloc_str (cache_path, "%s/%s", cachedir, cache_xdg_name);
        }
    }

  goto out;
  
 out1:
  (void) rc;
  free (cache_path);
  cache_path = NULL;

 out:
  if (cache_path != NULL)
    (void) mkdir (cache_path, 0700); // failures with this mkdir would be caught later too
  return cache_path;
}


/* Query each of the server URLs found in $DEBUGINFOD_URLS for the file
   with the specified build-id and type (debuginfo, executable, source or
   section).  If type is source, then type_arg should be a filename.  If
   type is section, then type_arg should be the name of an ELF/DWARF
   section.  Otherwise type_arg may be NULL.  Return a file descriptor
   for the target if successful, otherwise return an error code.
*/
static int
debuginfod_query_server_by_buildid (debuginfod_client *c,
			 const unsigned char *build_id,
                         int build_id_len,
                         const char *type,
                         const char *type_arg,
                         char **path)
{
  char *server_urls;
  char *urls_envvar;
  const char *section = NULL;
  const char *filename = NULL;
  char *cache_path = NULL;
  char *maxage_path = NULL;
  char *interval_path = NULL;
  char *cache_miss_path = NULL;
  char *target_cache_dir = NULL;
  char *target_cache_path = NULL;
  char *target_cachehdr_path = NULL;
  char *target_cache_tmppath = NULL;
  char *target_cachehdr_tmppath = NULL;
  char suffix[NAME_MAX];
  char build_id_bytes[MAX_BUILD_ID_BYTES * 2 + 1];
  int vfd = c->verbose_fd;
  int rc, r;

  c->progressfn_cancel = false;

  if (strcmp (type, "source") == 0)
    filename = type_arg;
  else if (strcmp (type, "section") == 0)
    {
      section = type_arg;
      if (section == NULL)
	return -EINVAL;
    }

  if (vfd >= 0)
    {
      dprintf (vfd, "debuginfod_find_%s ", type);
      if (build_id_len == 0) /* expect clean hexadecimal */
	dprintf (vfd, "%s", (const char *) build_id);
      else
	for (int i = 0; i < build_id_len; i++)
	  dprintf (vfd, "%02x", build_id[i]);
      if (filename != NULL)
	dprintf (vfd, " %s\n", filename);
      dprintf (vfd, "\n");
    }

  /* Is there any server we can query?  If not, don't do any work,
     just return with ENOSYS.  Don't even access the cache.  */
  urls_envvar = getenv(DEBUGINFOD_URLS_ENV_VAR);
  if (vfd >= 0)
    dprintf (vfd, "server urls \"%s\"\n",
	     urls_envvar != NULL ? urls_envvar : "");
  if (urls_envvar == NULL || urls_envvar[0] == '\0')
    {
      rc = -ENOSYS;
      goto out;
    }

  /* Clear the obsolete data from a previous _find operation. */
  free (c->url);
  c->url = NULL;
  free (c->winning_headers);
  c->winning_headers = NULL;

  /* PR 27982: Add max size if DEBUGINFOD_MAXSIZE is set. */
  long maxsize = 0;
  const char *maxsize_envvar;
  maxsize_envvar = getenv(DEBUGINFOD_MAXSIZE_ENV_VAR);
  if (maxsize_envvar != NULL)
    maxsize = atol (maxsize_envvar);

  /* PR 27982: Add max time if DEBUGINFOD_MAXTIME is set. */
  long maxtime = 0;
  const char *maxtime_envvar;
  maxtime_envvar = getenv(DEBUGINFOD_MAXTIME_ENV_VAR);
  if (maxtime_envvar != NULL)
    maxtime = atol (maxtime_envvar);
  if (maxtime && vfd >= 0)
    dprintf(vfd, "using max time %lds\n", maxtime);

  const char *headers_file_envvar;
  headers_file_envvar = getenv(DEBUGINFOD_HEADERS_FILE_ENV_VAR);
  if (headers_file_envvar != NULL)
    add_headers_from_file(c, headers_file_envvar);

  /* Maxsize is valid*/
  if (maxsize > 0)
    {
      if (vfd)
        dprintf (vfd, "using max size %ldB\n", maxsize);
      char *size_header = NULL;
      rc = asprintf (&size_header, "X-DEBUGINFOD-MAXSIZE: %ld", maxsize);
      if (rc < 0)
        {
          rc = -ENOMEM;
          goto out;
        }
      rc = debuginfod_add_http_header(c, size_header);
      free(size_header);
      if (rc < 0)
        goto out;
    }
  add_default_headers(c);

  /* Copy lowercase hex representation of build_id into buf.  */
  if (vfd >= 0)
    dprintf (vfd, "checking build-id\n");
  if ((build_id_len >= MAX_BUILD_ID_BYTES) ||
      (build_id_len == 0 &&
       strlen ((const char *) build_id) > MAX_BUILD_ID_BYTES*2))
    {
      rc = -EINVAL;
      goto out;
    }

  if (build_id_len == 0) /* expect clean hexadecimal */
    strcpy (build_id_bytes, (const char *) build_id);
  else
    for (int i = 0; i < build_id_len; i++)
      sprintf(build_id_bytes + (i * 2), "%02x", build_id[i]);

  if (filename != NULL)
    {
      if (vfd >= 0)
	dprintf (vfd, "checking filename\n");
      if (filename[0] != '/') // must start with /
	{
	  rc = -EINVAL;
	  goto out;
	}

      path_escape (filename, suffix, sizeof(suffix));
      /* If the DWARF filenames are super long, this could exceed
         PATH_MAX and truncate/collide.  Oh well, that'll teach
         them! */
    }
  else if (section != NULL)
    path_escape (section, suffix, sizeof(suffix));
  else
    suffix[0] = '\0';

  if (suffix[0] != '\0' && vfd >= 0)
    dprintf (vfd, "suffix %s\n", suffix);

  /* set paths needed to perform the query
     example format:
     cache_path:        $HOME/.cache
     target_cache_dir:  $HOME/.cache/0123abcd
     target_cache_path: $HOME/.cache/0123abcd/debuginfo
     target_cache_path: $HOME/.cache/0123abcd/executable-.debug_info
     target_cache_path: $HOME/.cache/0123abcd/source-HASH-#PATH#TO#SOURCE
     target_cachehdr_path: $HOME/.cache/0123abcd/hdr-debuginfo
     target_cachehdr_path: $HOME/.cache/0123abcd/hdr-executable...
  */

  cache_path = make_cache_path();
  if (!cache_path)
    {
      rc = -ENOMEM;
      goto out;
    }
  xalloc_str (target_cache_dir, "%s/%s", cache_path, build_id_bytes);
  (void) mkdir (target_cache_dir, 0700); // failures with this mkdir would be caught later too

  if (suffix[0] != '\0') { /* section, source queries */ 
    xalloc_str (target_cache_path, "%s/%s-%s", target_cache_dir, type, suffix);
    xalloc_str (target_cachehdr_path, "%s/hdr-%s-%s", target_cache_dir, type, suffix);
  } else {
    xalloc_str (target_cache_path, "%s/%s", target_cache_dir, type);
    xalloc_str (target_cachehdr_path, "%s/hdr-%s", target_cache_dir, type);
  }
  xalloc_str (target_cache_tmppath, "%s.XXXXXX", target_cache_path);
  xalloc_str (target_cachehdr_tmppath, "%s.XXXXXX", target_cachehdr_path);  

  /* XXX combine these */
  xalloc_str (interval_path, "%s/%s", cache_path, cache_clean_interval_filename);
  xalloc_str (cache_miss_path, "%s/%s", cache_path, cache_miss_filename);
  xalloc_str (maxage_path, "%s/%s", cache_path, cache_max_unused_age_filename);

  if (vfd >= 0)
    dprintf (vfd, "checking cache dir %s\n", cache_path);

  /* Make sure cache dir exists. debuginfo_clean_cache will then make
     sure the interval, cache_miss and maxage files exist.  */
  if (mkdir (cache_path, ACCESSPERMS) != 0
      && errno != EEXIST)
    {
      rc = -errno;
      goto out;
    }

  rc = debuginfod_clean_cache(c, cache_path, interval_path, maxage_path);
  if (rc != 0)
    goto out;

  /* Check if the target is already in the cache. */
  int fd = open(target_cache_path, O_RDONLY);
  if (fd >= 0)
    {
      struct stat st;
      if (fstat(fd, &st) != 0)
        {
          rc = -errno;
          close (fd);
          goto out;
        }

      /* If the file is non-empty, then we are done. */
      if (st.st_size > 0)
        {
          if (path != NULL)
            {
              *path = strdup(target_cache_path);
              if (*path == NULL)
                {
                  rc = -errno;
                  close (fd);
                  goto out;
                }
            }
          /* Success!!!! */
          update_atime(fd);
          rc = fd;

          /* Attempt to transcribe saved headers. */
          int fdh = open (target_cachehdr_path, O_RDONLY);
          if (fdh >= 0)
            {
              if (fstat (fdh, &st) == 0 && st.st_size > 0)
                {
                  c->winning_headers = malloc(st.st_size);
                  if (NULL != c->winning_headers)
                    {
                      ssize_t bytes_read = pread_retry(fdh, c->winning_headers, st.st_size, 0);
                      if (bytes_read <= 0)
                        {
                          free (c->winning_headers);
                          c->winning_headers = NULL;
                          (void) unlink (target_cachehdr_path);
                        }
                      if (vfd >= 0)
                        dprintf (vfd, "found %s (bytes=%ld)\n", target_cachehdr_path, (long)bytes_read);
                    }
                }

              update_atime (fdh);
              close (fdh);
            }
 
          goto out;
        }
      else
        {
          /* The file is empty. Attempt to download only if enough time
             has passed since the last attempt. */
          time_t cache_miss;
          time_t target_mtime = st.st_mtime;

          close(fd); /* no need to hold onto the negative-hit file descriptor */
          
          rc = debuginfod_config_cache(c, cache_miss_path,
                                       cache_miss_default_s, &st);
          if (rc < 0)
            goto out;

          cache_miss = (time_t)rc;
          if (time(NULL) - target_mtime <= cache_miss)
            {
              rc = -ENOENT;
              goto out;
            }
          else
            /* TOCTOU non-problem: if another task races, puts a working
               download or an empty file in its place, unlinking here just
               means WE will try to download again as uncached. */
            (void) unlink(target_cache_path);
        }
    }
  else if (errno == EACCES)
    /* Ensure old 000-permission files are not lingering in the cache. */
    (void) unlink(target_cache_path);

  if (section != NULL)
    {
      /* Try to extract the section from a cached file before querying
	 any servers.  */
      rc = cache_find_section (section, target_cache_dir, path);

      /* If the section was found or confirmed to not exist, then we
	 are done.  */
      if (rc >= 0 || rc == -ENOENT)
	goto out;
    }

  long timeout = default_timeout;
  const char* timeout_envvar = getenv(DEBUGINFOD_TIMEOUT_ENV_VAR);
  if (timeout_envvar != NULL)
    timeout = atoi (timeout_envvar);

  if (vfd >= 0)
    dprintf (vfd, "using timeout %ld\n", timeout);

  /* make a copy of the envvar so it can be safely modified.  */
  server_urls = strdup(urls_envvar);
  if (server_urls == NULL)
    {
      rc = -ENOMEM;
      goto out;
    }
  /* thereafter, goto out0 on error*/

  /* Because of a race with cache cleanup / rmdir, try to mkdir/mkstemp up to twice. */
  for(int i=0; i<2; i++)
    {
      /* (re)create target directory in cache */
      (void) mkdir(target_cache_dir, 0700); /* files will be 0400 later */
      
      /* NB: write to a temporary file first, to avoid race condition of
         multiple clients checking the cache, while a partially-written or empty
         file is in there, being written from libcurl. */
      fd = mkstemp (target_cache_tmppath);
      if (fd >= 0) break;
    }
  if (fd < 0) /* Still failed after two iterations. */
    {
      rc = -errno;
      goto out0;
    }

  char **server_url_list = NULL;
  ima_policy_t* url_ima_policies = NULL;
  char *server_url;
  int num_urls;
  r = init_server_urls("buildid", type, server_urls, &server_url_list, &url_ima_policies, &num_urls, vfd);
  if (0 != r)
    {
      rc = r;
      goto out1;
    }

  /* No URLs survived parsing / filtering?  Abort abort abort. */
  if (num_urls == 0)
    {
      rc = -ENOSYS;
      goto out1;
    }
  
  int retry_limit = default_retry_limit;
  const char* retry_limit_envvar = getenv(DEBUGINFOD_RETRY_LIMIT_ENV_VAR);
  if (retry_limit_envvar != NULL)
    retry_limit = atoi (retry_limit_envvar);

  CURLM *curlm = c->server_mhandle;

  /* Tracks which handle should write to fd. Set to the first
     handle that is ready to write the target file to the cache.  */
  CURL *target_handle = NULL;
  struct handle_data *data = malloc(sizeof(struct handle_data) * num_urls);
  if (data == NULL)
    {
      rc = -ENOMEM;
      goto out1;
    }

  /* thereafter, goto out2 on error.  */

 /*The beginning of goto block query_in_parallel.*/
 query_in_parallel:
  rc = -ENOENT; /* Reset rc to default.*/

  /* Initialize handle_data with default values. */
  for (int i = 0; i < num_urls; i++)
    {
      data[i].handle = NULL;
      data[i].fd = -1;
      data[i].errbuf[0] = '\0';
      data[i].response_data = NULL;
      data[i].response_data_size = 0;
    }

  char *escaped_string = NULL;
  size_t escaped_strlen = 0;
  if (filename)
    {
      escaped_string = curl_easy_escape(&target_handle, filename+1, 0);
      if (!escaped_string)
        {
          rc = -ENOMEM;
          goto out2;
        }
      char *loc = escaped_string;
      escaped_strlen = strlen(escaped_string);
      while ((loc = strstr(loc, "%2F")))
        {
          loc[0] = '/';
          //pull the string back after replacement
          // loc-escaped_string finds the distance from the origin to the new location
          // - 2 accounts for the 2F which remain and don't need to be measured.
          // The two above subtracted from escaped_strlen yields the remaining characters
          // in the string which we want to pull back
          memmove(loc+1, loc+3,escaped_strlen - (loc-escaped_string) - 2);
          //Because the 2F was overwritten in the memmove (as desired) escaped_strlen is
          // now two shorter.
          escaped_strlen -= 2;
        }
    }
  /* Initialize each handle.  */
  for (int i = 0; i < num_urls; i++)
    {
      if ((server_url = server_url_list[i]) == NULL)
        break;
      if (vfd >= 0)
#ifdef ENABLE_IMA_VERIFICATION
        dprintf (vfd, "init server %d %s [IMA verification policy: %s]\n", i, server_url, ima_policy_enum2str(url_ima_policies[i]));
#else
        dprintf (vfd, "init server %d %s\n", i, server_url);
#endif

      data[i].fd = fd;
      data[i].target_handle = &target_handle;
      data[i].client = c;

      if (filename) /* must start with / */
        {
          /* PR28034 escape characters in completed url to %hh format. */
          snprintf(data[i].url, PATH_MAX, "%s/%s/%s/%s", server_url,
                   build_id_bytes, type, escaped_string);
        }
      else if (section)
	snprintf(data[i].url, PATH_MAX, "%s/%s/%s/%s", server_url,
		 build_id_bytes, type, section);
      else
        snprintf(data[i].url, PATH_MAX, "%s/%s/%s", server_url, build_id_bytes, type);

      r = init_handle(c, debuginfod_write_callback, header_callback, &data[i], i, timeout, vfd);
      if (0 != r)
        {
          rc = r;
          if (filename) curl_free (escaped_string);
          goto out2;
        }

      curl_multi_add_handle(curlm, data[i].handle);
    }

  if (filename) curl_free(escaped_string);

  /* Query servers in parallel.  */
  if (vfd >= 0)
    dprintf (vfd, "query %d urls in parallel\n", num_urls);
  int committed_to;
  r = perform_queries(curlm, &target_handle, data, c, num_urls, maxtime, maxsize, true,  vfd, &committed_to);
  if (0 != r)
    {
      rc = r;
      goto out2;
    }

  /* Check whether a query was successful. If so, assign its handle
     to verified_handle.  */
  int num_msg;
  rc = -ENOENT;
  CURL *verified_handle = NULL;
  do
    {
      CURLMsg *msg;

      msg = curl_multi_info_read(curlm, &num_msg);
      if (msg != NULL && msg->msg == CURLMSG_DONE)
        {
	  if (vfd >= 0)
	    {
	      bool pnl = (c->default_progressfn_printed_p
			  && vfd == STDERR_FILENO);
	      dprintf (vfd, "%sserver response %s\n", pnl ? "\n" : "",
		       curl_easy_strerror (msg->data.result));
	      if (pnl)
		c->default_progressfn_printed_p = 0;
	      for (int i = 0; i < num_urls; i++)
		if (msg->easy_handle == data[i].handle)
		  {
		    if (strlen (data[i].errbuf) > 0)
		      dprintf (vfd, "url %d %s\n", i, data[i].errbuf);
		    break;
		  }
	    }

          if (msg->data.result != CURLE_OK)
            {
              long resp_code;
              CURLcode ok0;
              /* Unsuccessful query, determine error code.  */
              switch (msg->data.result)
                {
                case CURLE_COULDNT_RESOLVE_HOST: rc = -EHOSTUNREACH; break; // no NXDOMAIN
                case CURLE_URL_MALFORMAT: rc = -EINVAL; break;
                case CURLE_COULDNT_CONNECT: rc = -ECONNREFUSED; break;
                case CURLE_PEER_FAILED_VERIFICATION: rc = -ECONNREFUSED; break;
                case CURLE_REMOTE_ACCESS_DENIED: rc = -EACCES; break;
                case CURLE_WRITE_ERROR: rc = -EIO; break;
                case CURLE_OUT_OF_MEMORY: rc = -ENOMEM; break;
                case CURLE_TOO_MANY_REDIRECTS: rc = -EMLINK; break;
                case CURLE_SEND_ERROR: rc = -ECONNRESET; break;
                case CURLE_RECV_ERROR: rc = -ECONNRESET; break;
                case CURLE_OPERATION_TIMEDOUT: rc = -ETIME; break;
                case CURLE_HTTP_RETURNED_ERROR:
                  ok0 = curl_easy_getinfo (msg->easy_handle,
                                          CURLINFO_RESPONSE_CODE,
				          &resp_code);
                  /* 406 signals that the requested file was too large */
                  if ( ok0 == CURLE_OK && resp_code == 406)
                    rc = -EFBIG;
		  else if (section != NULL && resp_code == 503)
		    rc = -EINVAL;
                  else
                    rc = -ENOENT;
                  break;
                default: rc = -ENOENT; break;
                }
            }
          else
            {
              /* Query completed without an error. Confirm that the
                 response code is 200 when using HTTP/HTTPS and 0 when
                 using file:// and set verified_handle.  */

              if (msg->easy_handle != NULL)
                {
                  char *effective_url = NULL;
                  long resp_code = 500;
                  CURLcode ok1 = curl_easy_getinfo (target_handle,
						    CURLINFO_EFFECTIVE_URL,
						    &effective_url);
                  CURLcode ok2 = curl_easy_getinfo (target_handle,
						    CURLINFO_RESPONSE_CODE,
						    &resp_code);
                  if(ok1 == CURLE_OK && ok2 == CURLE_OK && effective_url)
                    {
                      if (strncasecmp (effective_url, "HTTP", 4) == 0)
                        if (resp_code == 200)
                          {
                            verified_handle = msg->easy_handle;
                            break;
                          }
                      if (strncasecmp (effective_url, "FILE", 4) == 0)
                        if (resp_code == 0)
                          {
                            verified_handle = msg->easy_handle;
                            break;
                          }
                    }
                  /* - libcurl since 7.52.0 version start to support
                       CURLINFO_SCHEME;
                     - before 7.61.0, effective_url would give us a
                       url with upper case SCHEME added in the front;
                     - effective_url between 7.61 and 7.69 can be lack
                       of scheme if the original url doesn't include one;
                     - since version 7.69 effective_url will be provide
                       a scheme in lower case.  */
                  #if LIBCURL_VERSION_NUM >= 0x073d00 /* 7.61.0 */
                  #if LIBCURL_VERSION_NUM <= 0x074500 /* 7.69.0 */
                  char *scheme = NULL;
                  CURLcode ok3 = curl_easy_getinfo (target_handle,
                                                    CURLINFO_SCHEME,
                                                    &scheme);
                  if(ok3 == CURLE_OK && scheme)
                    {
                      if (startswith (scheme, "HTTP"))
                        if (resp_code == 200)
                          {
                            verified_handle = msg->easy_handle;
                            break;
                          }
                    }
                  #endif
                  #endif
                }
            }
        }
    } while (num_msg > 0);

  /* Create an empty file in the cache if the query fails with ENOENT and
     it wasn't cancelled early.  */
  if (rc == -ENOENT && !c->progressfn_cancel)
    {
      int efd = open (target_cache_path, O_CREAT|O_EXCL, DEFFILEMODE);
      if (efd >= 0)
        close(efd);
    }
  else if (rc == -EFBIG)
    goto out2;

  /* If the verified_handle is NULL and rc != -ENOENT, the query fails with
   * an error code other than 404, then do several retry within the retry_limit.
   * Clean up all old handles and jump back to the beginning of query_in_parallel,
   * reinitialize handles and query again.*/
  if (verified_handle == NULL)
    {
      if (rc != -ENOENT && retry_limit-- > 0)
        {
	  if (vfd >= 0)
            dprintf (vfd, "Retry failed query, %d attempt(s) remaining\n", retry_limit);
	  /* remove all handles from multi */
          for (int i = 0; i < num_urls; i++)
            {
              curl_multi_remove_handle(curlm, data[i].handle); /* ok to repeat */
              curl_easy_cleanup (data[i].handle);
              free(data[i].response_data);
              data[i].response_data = NULL;
            }
            free(c->winning_headers);
            c->winning_headers = NULL;
	    goto query_in_parallel;
	}
      else
	goto out2;
    }

  if (vfd >= 0)
    {
      bool pnl = c->default_progressfn_printed_p && vfd == STDERR_FILENO;
      dprintf (vfd, "%sgot file from server\n", pnl ? "\n" : "");
      if (pnl)
	c->default_progressfn_printed_p = 0;
    }

  /* we've got one!!!! */
  time_t mtime;
#if defined(_TIME_BITS) && _TIME_BITS == 64
  CURLcode curl_res = curl_easy_getinfo(verified_handle, CURLINFO_FILETIME_T, (void*) &mtime);
#else
  CURLcode curl_res = curl_easy_getinfo(verified_handle, CURLINFO_FILETIME, (void*) &mtime);
#endif
  if (curl_res == CURLE_OK)
    {
      struct timespec tvs[2];
      tvs[0].tv_sec = 0;
      tvs[0].tv_nsec = UTIME_OMIT;
      tvs[1].tv_sec = mtime;
      tvs[1].tv_nsec = 0;
      (void) futimens (fd, tvs);  /* best effort */
    }

  /* PR27571: make cache files casually unwriteable; dirs are already 0700 */
  (void) fchmod(fd, 0400);
  /* PR31248: lseek back to beginning */
  (void) lseek(fd, 0, SEEK_SET);
                
  if(NULL != url_ima_policies && ignore != url_ima_policies[committed_to])
    {
#ifdef ENABLE_IMA_VERIFICATION
      int result = debuginfod_validate_imasig(c, fd);
#else
      int result = -ENOSYS;
#endif
      if(0 == result)
        {
          if (vfd >= 0) dprintf (vfd, "valid signature\n");
        }
      else if (enforcing == url_ima_policies[committed_to])
        {
          // All invalid signatures are rejected.
          // Additionally in enforcing mode any non-valid signature is rejected, so by reaching
          // this case we do so since we know it is not valid. Note - this not just invalid signatures
          // but also signatures that cannot be validated
          if (vfd >= 0) dprintf (vfd, "error: invalid or missing signature (%d)\n", result);
          rc = result;
          goto out2;
        }
    }

  /* rename tmp->real */
  rc = rename (target_cache_tmppath, target_cache_path);
  if (rc < 0)
    {
      rc = -errno;
      goto out2;
      /* Perhaps we need not give up right away; could retry or something ... */
    }

  /* write out the headers, best effort basis */
  if (c->winning_headers) {
    int fdh = mkstemp (target_cachehdr_tmppath);
    if (fdh >= 0) {
      size_t bytes_to_write = strlen(c->winning_headers)+1; // include \0
      size_t bytes = pwrite_retry (fdh, c->winning_headers, bytes_to_write, 0);
      (void) close (fdh);
      if (bytes == bytes_to_write)
        (void) rename (target_cachehdr_tmppath, target_cachehdr_path);
      else
        (void) unlink (target_cachehdr_tmppath);
      if (vfd >= 0)
        dprintf (vfd, "saved %ld bytes of headers to %s\n", (long)bytes, target_cachehdr_path);
    }
  }
  
  /* remove all handles from multi */
  for (int i = 0; i < num_urls; i++)
    {
      curl_multi_remove_handle(curlm, data[i].handle); /* ok to repeat */
      curl_easy_cleanup (data[i].handle);
      free (data[i].response_data);
    }

  for (int i = 0; i < num_urls; ++i)
    free(server_url_list[i]);
  free(server_url_list);
  free(url_ima_policies);
  free (data);
  free (server_urls);

  /* don't close fd - we're returning it */
  /* don't unlink the tmppath; it's already been renamed. */
  if (path != NULL)
   *path = strdup(target_cache_path);

  rc = fd;
  goto out;

/* error exits */
 out2:
  /* remove all handles from multi */
  for (int i = 0; i < num_urls; i++)
    {
      if (data[i].handle != NULL)
	{
	  curl_multi_remove_handle(curlm, data[i].handle); /* ok to repeat */
	  curl_easy_cleanup (data[i].handle);
	  free (data[i].response_data);
	}
    }

  unlink (target_cache_tmppath);
  close (fd); /* before the rmdir, otherwise it'll fail */
  (void) rmdir (target_cache_dir); /* nop if not empty */
  free(data);

 out1:
  for (int i = 0; i < num_urls; ++i)
    free(server_url_list[i]);
  free(server_url_list);
  free(url_ima_policies);

 out0:
  free (server_urls);

/* general purpose exit */
 out:
  /* Reset sent headers */
  curl_slist_free_all (c->headers);
  c->headers = NULL;
  c->user_agent_set_p = 0;
  
  /* Conclude the last \r status line */
  /* Another possibility is to use the ANSI CSI n K EL "Erase in Line"
     code.  That way, the previously printed messages would be erased,
     and without a newline. */
  if (c->default_progressfn_printed_p)
    dprintf(STDERR_FILENO, "\n");

  if (vfd >= 0)
    {
      if (rc < 0)
	dprintf (vfd, "not found %s (err=%d)\n", strerror (-rc), rc);
      else
	dprintf (vfd, "found %s (fd=%d)\n", target_cache_path, rc);
    }

  free (cache_path);
  free (maxage_path);
  free (interval_path);
  free (cache_miss_path);
  free (target_cache_dir);
  free (target_cache_path);
  if (rc < 0 && target_cache_tmppath != NULL)
    (void)unlink (target_cache_tmppath);
  free (target_cache_tmppath);
  if (rc < 0 && target_cachehdr_tmppath != NULL)
    (void)unlink (target_cachehdr_tmppath);
  free (target_cachehdr_tmppath);
  free (target_cachehdr_path);
    
  return rc;
}



/* See debuginfod.h  */
debuginfod_client  *
debuginfod_begin (void)
{
  /* Initialize libcurl lazily, but only once.  */
  pthread_once (&init_control, libcurl_init);

  debuginfod_client *client;
  size_t size = sizeof (struct debuginfod_client);
  client = calloc (1, size);

  if (client != NULL)
    {
      if (getenv(DEBUGINFOD_PROGRESS_ENV_VAR))
	client->progressfn = default_progressfn;
      if (getenv(DEBUGINFOD_VERBOSE_ENV_VAR))
	client->verbose_fd = STDERR_FILENO;
      else
	client->verbose_fd = -1;

      // allocate 1 curl multi handle
      client->server_mhandle = curl_multi_init ();
      if (client->server_mhandle == NULL)
	goto out1;
    }

#ifdef ENABLE_IMA_VERIFICATION
  load_ima_public_keys (client);
#endif

  // extra future initialization
  
  goto out;

 out1:
  free (client);
  client = NULL;

 out:  
  return client;
}

void
debuginfod_set_user_data(debuginfod_client *client,
                         void *data)
{
  client->user_data = data;
}

void *
debuginfod_get_user_data(debuginfod_client *client)
{
  return client->user_data;
}

const char *
debuginfod_get_url(debuginfod_client *client)
{
  return client->url;
}

const char *
debuginfod_get_headers(debuginfod_client *client)
{
  return client->winning_headers;
}

void
debuginfod_end (debuginfod_client *client)
{
  if (client == NULL)
    return;

  curl_multi_cleanup (client->server_mhandle);
  curl_slist_free_all (client->headers);
  free (client->winning_headers);
  free (client->url);
#ifdef ENABLE_IMA_VERIFICATION
  free_ima_public_keys (client);
#endif
  free (client);
}

int
debuginfod_find_debuginfo (debuginfod_client *client,
			   const unsigned char *build_id, int build_id_len,
                           char **path)
{
  return debuginfod_query_server_by_buildid(client, build_id, build_id_len,
                                 "debuginfo", NULL, path);
}


/* See debuginfod.h  */
int
debuginfod_find_executable(debuginfod_client *client,
			   const unsigned char *build_id, int build_id_len,
                           char **path)
{
  return debuginfod_query_server_by_buildid(client, build_id, build_id_len,
                                 "executable", NULL, path);
}

/* See debuginfod.h  */
int debuginfod_find_source(debuginfod_client *client,
			   const unsigned char *build_id, int build_id_len,
                           const char *filename, char **path)
{
  return debuginfod_query_server_by_buildid(client, build_id, build_id_len,
                                 "source", filename, path);
}

int
debuginfod_find_section (debuginfod_client *client,
			 const unsigned char *build_id, int build_id_len,
			 const char *section, char **path)
{
  int rc = debuginfod_query_server_by_buildid(client, build_id, build_id_len,
                                              "section", section, path);
  if (rc != -EINVAL && rc != -ENOSYS)
    return rc;
  /* NB: we fall through in case of ima:enforcing-filtered DEBUGINFOD_URLS servers,
     so we can download the entire file, verify it locally, then slice it. */
  
  /* The servers may have lacked support for section queries.  Attempt to
     download the debuginfo or executable containing the section in order
     to extract it.  */
  rc = -EEXIST;
  int fd = -1;
  char *tmp_path = NULL;

  fd = debuginfod_find_debuginfo (client, build_id, build_id_len, &tmp_path);
  if (client->progressfn_cancel)
    {
      if (fd >= 0)
	{
	  /* This shouldn't happen, but we'll check this condition
	     just in case.  */
	  close (fd);
	  free (tmp_path);
	}
      return -ENOENT;
    }
  if (fd >= 0)
    {
      rc = extract_section (fd, section, tmp_path, path);
      close (fd);
    }

  if (rc == -EEXIST)
    {
      /* Either the debuginfo couldn't be found or the section should
	 be in the executable.  */
      fd = debuginfod_find_executable (client, build_id,
				       build_id_len, &tmp_path);
      if (fd >= 0)
	{
	  rc = extract_section (fd, section, tmp_path, path);
	  close (fd);
	}
      else
	/* Update rc so that we return the most recent error code.  */
	rc = fd;
    }

  free (tmp_path);
  return rc;
}


int debuginfod_find_metadata (debuginfod_client *client,
                              const char* key, const char* value, char **path)
{
  char *server_urls = NULL;
  char *urls_envvar = NULL;
  char *cache_path = NULL;
  char *target_cache_dir = NULL;
  char *target_cache_path = NULL;
  char *target_cache_tmppath = NULL;
  char *target_file_name = NULL;
  char *key_and_value = NULL;
  int rc = 0, r;
  int vfd = client->verbose_fd;
  struct handle_data *data = NULL;

  client->progressfn_cancel = false;
  
  json_object *json_metadata = json_object_new_object();
  json_bool json_metadata_complete = true;
  json_object *json_metadata_arr = json_object_new_array();
  if (NULL == json_metadata)
    {
      rc = -ENOMEM;
      goto out;
    }
  json_object_object_add(json_metadata, "results",
                         json_metadata_arr ?: json_object_new_array() /* Empty array */);

  if (NULL == value || NULL == key)
    {
      rc = -EINVAL;
      goto out;
    }

  if (vfd >= 0)
    dprintf (vfd, "debuginfod_find_metadata %s %s\n", key, value);

  /* Without query-able URL, we can stop here*/
  urls_envvar = getenv(DEBUGINFOD_URLS_ENV_VAR);
  if (vfd >= 0)
    dprintf (vfd, "server urls \"%s\"\n",
      urls_envvar != NULL ? urls_envvar : "");
  if (urls_envvar == NULL || urls_envvar[0] == '\0')
  {
    rc = -ENOSYS;
    goto out;
  }

  /* set paths needed to perform the query
     example format:
     cache_path:        $HOME/.cache
     target_cache_dir:  $HOME/.cache/metadata
     target_cache_path: $HOME/.cache/metadata/KEYENCODED_VALUEENCODED
     target_cache_path: $HOME/.cache/metadata/KEYENCODED_VALUEENCODED.XXXXXX
  */

  // libcurl > 7.62ish has curl_url_set()/etc. to construct these things more properly.
  // curl_easy_escape() is older
  {
    CURL *c = curl_easy_init();
    if (!c)
      {
        rc = -ENOMEM;
        goto out;
      }
    char *key_escaped = curl_easy_escape(c, key, 0);
    char *value_escaped = curl_easy_escape(c, value, 0);
    
    // fallback to unescaped values in unlikely case of error
    xalloc_str (key_and_value, "key=%s&value=%s", key_escaped ?: key, value_escaped ?: value);
    xalloc_str (target_file_name, "%s_%s", key_escaped ?: key, value_escaped ?: value);
    curl_free(value_escaped);
    curl_free(key_escaped);
    curl_easy_cleanup(c);
  }

  /* Check if we have a recent result already in the cache. */
  cache_path = make_cache_path();
  if (! cache_path)
    {
      rc = -ENOMEM;
      goto out;
    }
  xalloc_str (target_cache_dir, "%s/metadata", cache_path);
  (void) mkdir (target_cache_dir, 0700);
  xalloc_str (target_cache_path, "%s/%s", target_cache_dir, target_file_name);
  xalloc_str (target_cache_tmppath, "%s/%s.XXXXXX", target_cache_dir, target_file_name);

  int fd = open(target_cache_path, O_RDONLY);
  if (fd >= 0)
    {
      struct stat st;
      int metadata_retention = 0;
      time_t now = time(NULL);
      char *metadata_retention_path = 0;

      xalloc_str (metadata_retention_path, "%s/%s", cache_path, metadata_retention_filename);
      if (metadata_retention_path)
        {
          rc = debuginfod_config_cache(client, metadata_retention_path,
                                       metadata_retention_default_s, &st);
          free (metadata_retention_path);
          if (rc < 0)
            rc = 0;
        }
      else
        rc = 0;
      metadata_retention = rc;

      if (fstat(fd, &st) != 0)
        {
          rc = -errno;
          close (fd);
          goto out;
        }

      if (metadata_retention > 0 && (now - st.st_mtime <= metadata_retention))
        {
          if (client && client->verbose_fd >= 0)
            dprintf (client->verbose_fd, "cached metadata %s", target_file_name);

          if (path != NULL)
            {
              *path = target_cache_path; // pass over the pointer
              target_cache_path = NULL; // prevent free() in our own cleanup
            }

          /* Success!!!! */
          rc = fd;
          goto out;
        }

      /* We don't have to clear the likely-expired cached object here
         by unlinking.  We will shortly make a new request and save
         results right on top.  Erasing here could trigger a TOCTOU
         race with another thread just finishing a query and passing
         its results back.
      */
      // (void) unlink (target_cache_path);

      close (fd);
    }

  /* No valid cached metadata found: time to make the queries. */

  free (client->url);
  client->url = NULL;

  long maxtime = 0;
  const char *maxtime_envvar;
  maxtime_envvar = getenv(DEBUGINFOD_MAXTIME_ENV_VAR);
  if (maxtime_envvar != NULL)
    maxtime = atol (maxtime_envvar);
  if (maxtime && vfd >= 0)
    dprintf(vfd, "using max time %lds\n", maxtime);

  long timeout = default_timeout;
  const char* timeout_envvar = getenv(DEBUGINFOD_TIMEOUT_ENV_VAR);
  if (timeout_envvar != NULL)
    timeout = atoi (timeout_envvar);
  if (vfd >= 0)
    dprintf (vfd, "using timeout %ld\n", timeout);

  add_default_headers(client);

  /* Make a copy of the envvar so it can be safely modified.  */
  server_urls = strdup(urls_envvar);
  if (server_urls == NULL)
  {
    rc = -ENOMEM;
    goto out;
  }

  /* Thereafter, goto out1 on error*/

  char **server_url_list = NULL;
  ima_policy_t* url_ima_policies = NULL;
  char *server_url;
  int num_urls = 0;
  r = init_server_urls("metadata", NULL, server_urls, &server_url_list, &url_ima_policies, &num_urls, vfd);
  if (0 != r)
    {
      rc = r;
      goto out1;
    }

  CURLM *curlm = client->server_mhandle;

  CURL *target_handle = NULL;
  data = malloc(sizeof(struct handle_data) * num_urls);
  if (data == NULL)
    {
      rc = -ENOMEM;
      goto out1;
    }

  /* thereafter, goto out2 on error.  */

  /* Initialize handle_data  */
  for (int i = 0; i < num_urls; i++)
    {
      if ((server_url = server_url_list[i]) == NULL)
        break;
      if (vfd >= 0)
        dprintf (vfd, "init server %d %s\n", i, server_url);
      
      data[i].errbuf[0] = '\0';
      data[i].target_handle = &target_handle;
      data[i].client = client;
      data[i].metadata = NULL;
      data[i].metadata_size = 0;
      data[i].response_data = NULL;
      data[i].response_data_size = 0;
      
      snprintf(data[i].url, PATH_MAX, "%s?%s", server_url, key_and_value);
      
      r = init_handle(client, metadata_callback, header_callback, &data[i], i, timeout, vfd);
      if (0 != r)
        {
          rc = r;
          goto out2;
        }
      curl_multi_add_handle(curlm, data[i].handle);
    }

  /* Query servers */
  if (vfd >= 0)
    dprintf (vfd, "Starting %d queries\n",num_urls);
  int committed_to;
  r = perform_queries(curlm, NULL, data, client, num_urls, maxtime, 0, false, vfd, &committed_to);
  if (0 != r)
    {
      rc = r;
      goto out2;
    }

  /* NOTE: We don't check the return codes of the curl messages since
     a metadata query failing silently is just fine. We want to know what's
     available from servers which can be connected with no issues.
     If running with additional verbosity, the failure will be noted in stderr */

  /* Building the new json array from all the upstream data and
     cleanup while at it.
   */
  for (int i = 0; i < num_urls; i++)
    {
      curl_multi_remove_handle(curlm, data[i].handle); /* ok to repeat */
      curl_easy_cleanup (data[i].handle);
      free (data[i].response_data);
      
      if (NULL == data[i].metadata)
        {
          if (vfd >= 0)
            dprintf (vfd, "Query to %s failed with error message:\n\t\"%s\"\n",
                     data[i].url, data[i].errbuf);
          json_metadata_complete = false;
          continue;
        }

      json_object *upstream_metadata = json_tokener_parse(data[i].metadata);
      json_object *upstream_complete;
      json_object *upstream_metadata_arr;
      if (NULL == upstream_metadata ||
          !json_object_object_get_ex(upstream_metadata, "results", &upstream_metadata_arr) ||
          !json_object_object_get_ex(upstream_metadata, "complete", &upstream_complete))
        continue;
      json_metadata_complete &= json_object_get_boolean(upstream_complete);
      // Combine the upstream metadata into the json array
      for (int j = 0, n = json_object_array_length(upstream_metadata_arr); j < n; j++)
        {
          json_object *entry = json_object_array_get_idx(upstream_metadata_arr, j);
          json_object_get(entry); // increment reference count
          json_object_array_add(json_metadata_arr, entry);
        }
      json_object_put(upstream_metadata);

      free (data[i].metadata);
    }

  /* Because of race with cache cleanup / rmdir, try to mkdir/mkstemp up to twice. */
  for (int i=0; i<2; i++)
    {
      /* (re)create target directory in cache */
      (void) mkdir(target_cache_dir, 0700); /* files will be 0400 later */

      /* NB: write to a temporary file first, to avoid race condition of
         multiple clients checking the cache, while a partially-written or empty
         file is in there, being written from libcurl. */
      fd = mkstemp (target_cache_tmppath);
      if (fd >= 0) break;
    }
  if (fd < 0) /* Still failed after two iterations. */
    {
      rc = -errno;
      goto out1;
    }
    
  /* Plop the complete json_metadata object into the cache. */
  json_object_object_add(json_metadata, "complete", json_object_new_boolean(json_metadata_complete));
  const char* json_string = json_object_to_json_string_ext(json_metadata, JSON_C_TO_STRING_PRETTY);
  if (json_string == NULL)
    {
      rc = -ENOMEM;
      goto out1;
    }
  ssize_t res = write_retry (fd, json_string, strlen(json_string));
  (void) lseek(fd, 0, SEEK_SET); // rewind file so client can read it from the top
  
  /* NB: json_string is auto deleted when json_metadata object is nuked */
  if (res < 0 || (size_t) res != strlen(json_string))
    {
      rc = -EIO;
      goto out1;
    }
  /* PR27571: make cache files casually unwriteable; dirs are already 0700 */
  (void) fchmod(fd, 0400);

  /* rename tmp->real */
  rc = rename (target_cache_tmppath, target_cache_path);
  if (rc < 0)
    {
      rc = -errno;
      goto out1;
      /* Perhaps we need not give up right away; could retry or something ... */
    }
  
  /* don't close fd - we're returning it */
  /* don't unlink the tmppath; it's already been renamed. */
  if (path != NULL)
   *path = strdup(target_cache_path);

  rc = fd;
  goto out1;

/* error exits */
out2:
  /* remove all handles from multi */
  for (int i = 0; i < num_urls; i++)
  {
    if (data[i].handle != NULL)
    {
      curl_multi_remove_handle(curlm, data[i].handle); /* ok to repeat */
      curl_easy_cleanup (data[i].handle);
      free (data[i].response_data);
      free (data[i].metadata);
    }
  }

out1:
  free(data);
                              
  for (int i = 0; i < num_urls; ++i)
    free(server_url_list[i]);
  free(server_url_list);
  free(url_ima_policies);

out:
  free (server_urls);
  json_object_put(json_metadata);
  /* Reset sent headers */
  curl_slist_free_all (client->headers);
  client->headers = NULL;
  client->user_agent_set_p = 0;

  free (target_cache_dir);
  free (target_cache_path);
  free (target_cache_tmppath);
  free (key_and_value);
  free (target_file_name);
  free (cache_path);
    
  return rc;
}


/* Add an outgoing HTTP header.  */
int debuginfod_add_http_header (debuginfod_client *client, const char* header)
{
  /* Sanity check header value is of the form Header: Value.
     It should contain at least one colon that isn't the first or
     last character.  */
  char *colon = strchr (header, ':'); /* first colon */
  if (colon == NULL /* present */
      || colon == header /* not at beginning - i.e., have a header name */
      || *(colon + 1) == '\0') /* not at end - i.e., have a value */
    /* NB: but it's okay for a value to contain other colons! */
    return -EINVAL;

  struct curl_slist *temp = curl_slist_append (client->headers, header);
  if (temp == NULL)
    return -ENOMEM;

  /* Track if User-Agent: is being set.  If so, signal not to add the
     default one. */
  if (startswith (header, "User-Agent:"))
    client->user_agent_set_p = 1;

  client->headers = temp;
  return 0;
}


void
debuginfod_set_progressfn(debuginfod_client *client,
			  debuginfod_progressfn_t fn)
{
  client->progressfn = fn;
}

void
debuginfod_set_verbose_fd(debuginfod_client *client, int fd)
{
  client->verbose_fd = fd;
}

#endif /* DUMMY_LIBDEBUGINFOD */
