/* random.c  -	random number generator
 * Copyright (C) 1998, 2000, 2001, 2002, 2003,
 *               2004, 2005  Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

/****************
 * This random number generator is modelled after the one described in
 * Peter Gutmann's paper: "Software Generation of Practically Strong
 * Random Numbers". See also chapter 6 in his book "Cryptographic
 * Security Architecture", New York, 2004, ISBN 0-387-95387-6.
 */


#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#ifdef	HAVE_GETHRTIME
#include <sys/times.h>
#endif
#ifdef HAVE_GETTIMEOFDAY
#include <sys/times.h>
#endif
#ifdef HAVE_GETRUSAGE
#include <sys/resource.h>
#endif
#ifdef __MINGW32__
#include <process.h>
#endif
#include "g10lib.h"
#include "rmd.h"
#include "random.h"
#include "rand-internal.h"
#include "cipher.h" /* only used for the rmd160_hash_buffer() prototype */
#include "ath.h"

#ifndef RAND_MAX   /* for SunOS */
#define RAND_MAX 32767
#endif


#if SIZEOF_UNSIGNED_LONG == 8
#define ADD_VALUE 0xa5a5a5a5a5a5a5a5
#elif SIZEOF_UNSIGNED_LONG == 4
#define ADD_VALUE 0xa5a5a5a5
#else
#error weird size for an unsigned long
#endif

#define BLOCKLEN  64   /* hash this amount of bytes */
#define DIGESTLEN 20   /* into a digest of this length (rmd160) */
/* poolblocks is the number of digests which make up the pool
 * and poolsize must be a multiple of the digest length
 * to make the AND operations faster, the size should also be
 * a multiple of ulong
 */
#define POOLBLOCKS 30
#define POOLSIZE (POOLBLOCKS*DIGESTLEN)
#if (POOLSIZE % SIZEOF_UNSIGNED_LONG)
#error Please make sure that poolsize is a multiple of ulong
#endif
#define POOLWORDS (POOLSIZE / SIZEOF_UNSIGNED_LONG)


static int is_initialized;
#define MASK_LEVEL(a) do { (a) &= 3; } while(0)
static unsigned char *rndpool;	/* allocated size is POOLSIZE+BLOCKLEN */
static unsigned char *keypool;	/* allocated size is POOLSIZE+BLOCKLEN */
static size_t pool_readpos;
static size_t pool_writepos;
static int pool_filled;
static int pool_balance;
static int just_mixed;
static int did_initial_extra_seeding;
static char *seed_file_name;
static int allow_seed_file_update;

static int secure_alloc;
static int quick_test;
static int faked_rng;

static ath_mutex_t pool_lock = ATH_MUTEX_INITIALIZER;
static int pool_is_locked; /* only used for assertion */

static ath_mutex_t nonce_buffer_lock = ATH_MUTEX_INITIALIZER;

static byte *get_random_bytes( size_t nbytes, int level, int secure );
static void read_pool( byte *buffer, size_t length, int level );
static void add_randomness( const void *buffer, size_t length, int source );
static void random_poll(void);
static void do_fast_random_poll (void);
static void read_random_source( int requester, size_t length, int level);
static int gather_faked( void (*add)(const void*, size_t, int), int requester,
						    size_t length, int level );

static struct {
    ulong mixrnd;
    ulong mixkey;
    ulong slowpolls;
    ulong fastpolls;
    ulong getbytes1;
    ulong ngetbytes1;
    ulong getbytes2;
    ulong ngetbytes2;
    ulong addbytes;
    ulong naddbytes;
} rndstats;

static void (*progress_cb) (void *,const char*,int,int, int );
static void *progress_cb_data;

/* Note, we assume that this function is used before any concurrent
   access happens. */
static void
initialize_basics(void)
{
  static int initialized;
  int err;

  if (!initialized)
    {
      initialized = 1;
      err = ath_mutex_init (&pool_lock);
      if (err)
        log_fatal ("failed to create the pool lock: %s\n", strerror (err) );
      
      err = ath_mutex_init (&nonce_buffer_lock);
      if (err)
        log_fatal ("failed to create the nonce buffer lock: %s\n",
                   strerror (err) );
    }
}


static void
initialize(void)
{
  initialize_basics ();
  /* The data buffer is allocated somewhat larger, so that we can use
     this extra space (which is allocated in secure memory) as a
     temporary hash buffer */
  rndpool = secure_alloc ? gcry_xcalloc_secure(1,POOLSIZE+BLOCKLEN)
                         : gcry_xcalloc(1,POOLSIZE+BLOCKLEN);
  keypool = secure_alloc ? gcry_xcalloc_secure(1,POOLSIZE+BLOCKLEN)
                         : gcry_xcalloc(1,POOLSIZE+BLOCKLEN);
  is_initialized = 1;

}


/* Used to register a progress callback. */
void
_gcry_register_random_progress (void (*cb)(void *,const char*,int,int,int),
                                void *cb_data )
{
  progress_cb = cb;
  progress_cb_data = cb_data;
}


/* This progress function is currently used by the random modules to give hint
   on how much more entropy is required. */
void
_gcry_random_progress (const char *what, int printchar, int current, int total)
{
  if (progress_cb)
    progress_cb (progress_cb_data, what, printchar, current, total);
}


/* Initialize this random subsystem.  If FULL is false, this function
   merely calls the initialize and does not do anything more.  Doing
   this is not really required but when running in a threaded
   environment we might get a race condition otherwise. */
void
_gcry_random_initialize (int full)
{
  if (!full)
    initialize_basics ();
  else if (!is_initialized)
    initialize ();
}

void
_gcry_random_dump_stats()
{
    log_info (
	    "random usage: poolsize=%d mixed=%lu polls=%lu/%lu added=%lu/%lu\n"
	    "              outmix=%lu getlvl1=%lu/%lu getlvl2=%lu/%lu\n",
	POOLSIZE, rndstats.mixrnd, rndstats.slowpolls, rndstats.fastpolls,
		  rndstats.naddbytes, rndstats.addbytes,
	rndstats.mixkey, rndstats.ngetbytes1, rndstats.getbytes1,
		    rndstats.ngetbytes2, rndstats.getbytes2 );
}

void
_gcry_secure_random_alloc()
{
    secure_alloc = 1;
}


int
_gcry_quick_random_gen( int onoff )
{
  int last;

  /* No need to lock it here because we are only initializing.  A
     prerequisite of the entire code is that it has already been
     initialized before any possible concurrent access */
  read_random_source(0,0,0); /* init */
  last = quick_test;
  if( onoff != -1 )
    quick_test = onoff;
  return faked_rng? 1 : last;
}

int
_gcry_random_is_faked()
{
  if( !is_initialized )
    initialize();
  return (faked_rng || quick_test);
}

/*
 * Return a pointer to a randomized buffer of LEVEL and NBYTES length.
 * Caller must free the buffer. 
 */
static byte *
get_random_bytes ( size_t nbytes, int level, int secure)
{
  byte *buf, *p;
  int err;

  /* First a hack toavoid the strong random using our regression test suite. */
  if (quick_test && level > 1)
    level = 1;

  /* Make sure the requested level is in range. */
  MASK_LEVEL(level);

  /* Lock the pool. */
  err = ath_mutex_lock (&pool_lock);
  if (err)
    log_fatal ("failed to acquire the pool lock: %s\n", strerror (err));
  pool_is_locked = 1;

  /* Keep some statistics. */
  if (level >= 2)
    {
      rndstats.getbytes2 += nbytes;
      rndstats.ngetbytes2++;
    }
  else
    {
      rndstats.getbytes1 += nbytes;
      rndstats.ngetbytes1++;
    }

  /* Allocate the return buffer. */
  buf = secure && secure_alloc ? gcry_xmalloc_secure( nbytes )
                               : gcry_xmalloc( nbytes );

  /* Fill that buffer with random. */
  for (p = buf; nbytes > 0; )
    {
      size_t n;

      n = nbytes > POOLSIZE? POOLSIZE : nbytes;
      read_pool( p, n, level );
      nbytes -= n;
      p += n;
    }

  /* Release the pool lock. */
  pool_is_locked = 0;
  err = ath_mutex_unlock (&pool_lock);
  if (err)
    log_fatal ("failed to release the pool lock: %s\n", strerror (err));

  /* Return the buffer. */
  return buf;
}


/* Add BUFLEN bytes from BUF to the internal random pool.  QUALITY
   should be in the range of 0..100 to indicate the goodness of the
   entropy added, or -1 for goodness not known. 

   Note, that this function currently does nothing.
*/
gcry_error_t
gcry_random_add_bytes (const void * buf, size_t buflen, int quality)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;

  if (!buf || quality < -1 || quality > 100)
    err = GPG_ERR_INV_ARG;
  if (!buflen)
    return 0; /* Shortcut this dummy case. */
#if 0
  /* Before we actuall enable this code, we need to lock the pool,
     have a look at the quality and find a way to add them without
     disturbing the real entropy (we have estimated). */
  /*add_randomness( buf, buflen, 1 );*/
#endif
  return err;
}   
    
/* The public function to return random data of the quality LEVEL. */
void *
gcry_random_bytes( size_t nbytes, enum gcry_random_level level )
{
  if (!is_initialized)
    initialize();
  return get_random_bytes( nbytes, level, 0 );
}

/* The public function to return random data of the quality LEVEL;
   this version of the function retrun the random a buffer allocated
   in secure memory. */
void *
gcry_random_bytes_secure( size_t nbytes, enum gcry_random_level level )
{
  if (!is_initialized)
    initialize();
  return get_random_bytes( nbytes, level, 1 );
}


/* Public function to fill the buffer with LENGTH bytes of
   cryptographically strong random bytes. level 0 is not very strong,
   1 is strong enough for most usage, 2 is good for key generation
   stuff but may be very slow.  */
void
gcry_randomize (void *buffer, size_t length, enum gcry_random_level level)
{
  byte *p;
  int err;

  /* Make sure we are initialized. */
  if (!is_initialized)
    initialize ();

  /* Handle our hack used for regression tests of Libgcrypt. */
  if( quick_test && level > 1 )
    level = 1;

  /* Make sure the level is okay. */
  MASK_LEVEL(level);

  /* Acquire the pool lock. */
  err = ath_mutex_lock (&pool_lock);
  if (err)
    log_fatal ("failed to acquire the pool lock: %s\n", strerror (err));
  pool_is_locked = 1;

  /* Update the statistics. */
  if (level >= 2)
    {
      rndstats.getbytes2 += length;
      rndstats.ngetbytes2++;
    }
  else
    {
      rndstats.getbytes1 += length;
      rndstats.ngetbytes1++;
    }

  /* Read the random into the provided buffer. */
  for (p = buffer; length > 0;)
    {
      size_t n;

      n = length > POOLSIZE? POOLSIZE : length;
      read_pool (p, n, level);
      length -= n;
      p += n;
    }

  /* Release the pool lock. */
  pool_is_locked = 0;
  err = ath_mutex_unlock (&pool_lock);
  if (err)
    log_fatal ("failed to release the pool lock: %s\n", strerror (err));

}




/*
   Mix the pool:

   |........blocks*20byte........|20byte|..44byte..|
   <..44byte..>           <20byte> 
        |                    |
        |                    +------+
        +---------------------------|----------+
                                    v          v
   |........blocks*20byte........|20byte|..44byte..|
                                 <.....64bytes.....>   
                                         |
      +----------------------------------+
     Hash
      v
   |.............................|20byte|..44byte..|
   <20byte><20byte><..44byte..>
      |                |
      |                +---------------------+
      +-----------------------------+        |
                                    v        v
   |.............................|20byte|..44byte..|
                                 <.....64byte......>
                                        |
              +-------------------------+
             Hash
              v
   |.............................|20byte|..44byte..|
   <20byte><20byte><..44byte..>

   and so on until we did this for all blocks. 

   To better protect against implementation errors in this code, we
   xor a digest of the entire pool into the pool before mixing.

   Note, that this function muts only be called with a locked pool.
 */
static void
mix_pool (unsigned char *pool)
{
  static unsigned char failsafe_digest[DIGESTLEN];
  static int failsafe_digest_valid;

  unsigned char *hashbuf = pool + POOLSIZE;
  unsigned char *p, *pend;
  int i, n;
  RMD160_CONTEXT md;

#if DIGESTLEN != 20
#error must have a digest length of 20 for ripe-md-160
#endif

  assert (pool_is_locked);
  _gcry_rmd160_init( &md );

  /* loop over the pool */
  pend = pool + POOLSIZE;
  memcpy(hashbuf, pend - DIGESTLEN, DIGESTLEN );
  memcpy(hashbuf+DIGESTLEN, pool, BLOCKLEN-DIGESTLEN);
  _gcry_rmd160_mixblock( &md, (char*)hashbuf);
  memcpy(pool, hashbuf, 20 );

  if (failsafe_digest_valid && pool == rndpool)
    {
      for (i=0; i < 20; i++)
        pool[i] ^= failsafe_digest[i];
    }
  
  p = pool;
  for (n=1; n < POOLBLOCKS; n++)
    {
      memcpy (hashbuf, p, DIGESTLEN);

      p += DIGESTLEN;
      if (p+DIGESTLEN+BLOCKLEN < pend)
        memcpy (hashbuf+DIGESTLEN, p+DIGESTLEN, BLOCKLEN-DIGESTLEN);
      else 
        {
          unsigned char *pp = p + DIGESTLEN;
          
          for (i=DIGESTLEN; i < BLOCKLEN; i++ )
            {
              if ( pp >= pend )
                pp = pool;
              hashbuf[i] = *pp++;
	    }
	}
      
      _gcry_rmd160_mixblock( &md, (char*)hashbuf);
      memcpy(p, hashbuf, 20 );
    }

    /* Our hash implementation does only leave small parts (64 bytes)
       of the pool on the stack, so it is okay not to require secure
       memory here.  Before we use this pool, it will be copied to the
       help buffer anyway. */
    if ( pool == rndpool)
      {
        _gcry_rmd160_hash_buffer ((char*)failsafe_digest,
                                  (char*)pool, POOLSIZE);
        failsafe_digest_valid = 1;
      }

    _gcry_burn_stack (384); /* for the rmd160_mixblock(), rmd160_hash_buffer */
}


void
_gcry_set_random_seed_file( const char *name )
{
  if (seed_file_name)
    BUG ();
  seed_file_name = gcry_xstrdup (name);
}


/*
  Read in a seed form the random_seed file
  and return true if this was successful.
 */
static int
read_seed_file (void)
{
  int fd;
  struct stat sb;
  unsigned char buffer[POOLSIZE];
  int n;

  assert (pool_is_locked);

  if (!seed_file_name)
    return 0;
  
#ifdef HAVE_DOSISH_SYSTEM
  fd = open( seed_file_name, O_RDONLY | O_BINARY );
#else
  fd = open( seed_file_name, O_RDONLY );
#endif
  if( fd == -1 && errno == ENOENT)
    {
      allow_seed_file_update = 1;
      return 0;
    }

  if (fd == -1 )
    {
      log_info(_("can't open `%s': %s\n"), seed_file_name, strerror(errno) );
      return 0;
    }
  if (fstat( fd, &sb ) )
    {
      log_info(_("can't stat `%s': %s\n"), seed_file_name, strerror(errno) );
      close(fd);
      return 0;
    }
  if (!S_ISREG(sb.st_mode) )
    {
      log_info(_("`%s' is not a regular file - ignored\n"), seed_file_name );
      close(fd);
      return 0;
    }
  if (!sb.st_size )
    {
      log_info(_("note: random_seed file is empty\n") );
      close(fd);
      allow_seed_file_update = 1;
      return 0;
    }
  if (sb.st_size != POOLSIZE ) 
    {
      log_info(_("warning: invalid size of random_seed file - not used\n") );
      close(fd);
      return 0;
    }

  do
    {
      n = read( fd, buffer, POOLSIZE );
    } 
  while (n == -1 && errno == EINTR );

  if (n != POOLSIZE)
    {
      log_fatal(_("can't read `%s': %s\n"), seed_file_name,strerror(errno) );
      close(fd);/*NOTREACHED*/
      return 0;
    }
  
  close(fd);

  add_randomness( buffer, POOLSIZE, 0 );
  /* add some minor entropy to the pool now (this will also force a mixing) */
  {	
    pid_t x = getpid();
    add_randomness( &x, sizeof(x), 0 );
  }
  {
    time_t x = time(NULL);
    add_randomness( &x, sizeof(x), 0 );
  }
  {	
    clock_t x = clock();
    add_randomness( &x, sizeof(x), 0 );
  }

  /* And read a few bytes from our entropy source.  By using a level
   * of 0 this will not block and might not return anything with some
   * entropy drivers, however the rndlinux driver will use
   * /dev/urandom and return some stuff - Do not read to much as we
   * want to be friendly to the scare system entropy resource. */
  read_random_source( 0, 16, 0 );

  allow_seed_file_update = 1;
  return 1;
}


void
_gcry_update_random_seed_file()
{
  unsigned long *sp, *dp;
  int fd, i;
  int err;
  
  if ( !seed_file_name || !is_initialized || !pool_filled )
    return;
  if ( !allow_seed_file_update )
    {
      log_info(_("note: random_seed file not updated\n"));
      return;
    }

  err = ath_mutex_lock (&pool_lock);
  if (err)
    log_fatal ("failed to acquire the pool lock: %s\n", strerror (err));
  pool_is_locked = 1;

  /* copy the entropy pool to a scratch pool and mix both of them */
  for (i=0,dp=(ulong*)keypool, sp=(ulong*)rndpool;
       i < POOLWORDS; i++, dp++, sp++ ) 
    {
      *dp = *sp + ADD_VALUE;
    }
  mix_pool(rndpool); rndstats.mixrnd++;
  mix_pool(keypool); rndstats.mixkey++;

#ifdef HAVE_DOSISH_SYSTEM
  fd = open (seed_file_name, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,
             S_IRUSR|S_IWUSR );
#else
  fd = open (seed_file_name, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR );
#endif

  if (fd == -1 )
    log_info (_("can't create `%s': %s\n"), seed_file_name, strerror(errno) );
  else 
    {
      do
        {
          i = write (fd, keypool, POOLSIZE );
        } 
      while( i == -1 && errno == EINTR );
      if (i != POOLSIZE) 
        log_info (_("can't write `%s': %s\n"),
                  seed_file_name, strerror(errno) );
      if (close(fd))
        log_info(_("can't close `%s': %s\n"),
                 seed_file_name, strerror(errno) );
    }
  
  pool_is_locked = 0;
  err = ath_mutex_unlock (&pool_lock);
  if (err)
    log_fatal ("failed to release the pool lock: %s\n", strerror (err));

}


/* Read random out of the pool. This function is the core of the
   public random functions.  Note that Level 0 is special and in fact
   an alias for level 1. */
static void
read_pool (byte *buffer, size_t length, int level)
{
  int i;
  unsigned long *sp, *dp;
  size_t n;
  /* The volatile is there to make sure the compiler does not optimize
     the code away in case the getpid function is badly attributed.
     Note that we keep a pid in a static variable as well as in a
     stack based one; the latter is to detect ill behaving thread
     libraries, ignoring the pool mutexes. */
  static volatile pid_t my_pid = (pid_t)(-1); 
  volatile pid_t my_pid2;

 retry:
  /* Get our own pid, so that we can detect a fork. */
  my_pid2 = getpid ();
  if (my_pid == (pid_t)(-1))                                
    my_pid = my_pid2;
  if ( my_pid != my_pid2 )
    {
      /* We detected a plain fork; i.e. we are now the child.  Update
         the static pid and add some randomness. */
      pid_t x;

      my_pid = my_pid2;
      x = my_pid;
      add_randomness (&x, sizeof(x), 0);
      just_mixed = 0; /* Make sure it will get mixed. */
    }

  assert (pool_is_locked);

  /* Our code does not allow to extract more than POOLSIZE.  Better
     check it here. */
  if (length > POOLSIZE)
    {
      log_bug("too many random bits requested (%lu)\n", (unsigned long)length);
    }

  if (!pool_filled)
    {
      if (read_seed_file() )
        pool_filled = 1;
    }

  /* For level 2 quality (key generation) we always make sure that the
     pool has been seeded enough initially. */
  if (level == 2 && !did_initial_extra_seeding)
    {
      size_t needed;

      pool_balance = 0;
      needed = length - pool_balance;
      if (needed < POOLSIZE/2)
        needed = POOLSIZE/2;
      else if( needed > POOLSIZE )
        BUG ();
      read_random_source (3, needed, 2);
      pool_balance += needed;
      did_initial_extra_seeding = 1;
    }

  /* For level 2 make sure that there is enough random in the pool. */
  if (level == 2 && pool_balance < length)
    {
      size_t needed;
      
      if (pool_balance < 0)
        pool_balance = 0;
      needed = length - pool_balance;
      if (needed > POOLSIZE)
        BUG ();
      read_random_source( 3, needed, 2 );
      pool_balance += needed;
    }

  /* make sure the pool is filled */
  while (!pool_filled)
    random_poll();

  /* Always do a fast random poll (we have to use the unlocked version). */
  do_fast_random_poll();
  
  /* Mix the pid in so that we for sure won't deliver the same random
     after a fork. */
  {
    pid_t apid = my_pid;
    add_randomness (&apid, sizeof (apid), 0);
  }

  /* Mix the pool (if add_randomness() didn't it). */
  if (!just_mixed)
    {
      mix_pool(rndpool);
      rndstats.mixrnd++;
    }

  /* Create a new pool. */
  for(i=0,dp=(ulong*)keypool, sp=(ulong*)rndpool;
      i < POOLWORDS; i++, dp++, sp++ )
    *dp = *sp + ADD_VALUE;

  /* Mix both pools. */
  mix_pool(rndpool); rndstats.mixrnd++;
  mix_pool(keypool); rndstats.mixkey++;

  /* Read the required data.  We use a readpointer to read from a
     different position each time */
  for (n=0; n < length; n++)
    {
      *buffer++ = keypool[pool_readpos++];
      if (pool_readpos >= POOLSIZE)
        pool_readpos = 0;
      pool_balance--;
    }
 
  if (pool_balance < 0)
    pool_balance = 0;

  /* Clear the keypool. */
  memset (keypool, 0, POOLSIZE);

  /* We need to detect whether a fork has happened.  A fork might have
     an identical pool and thus the child and the parent could emit
     the very same random number.  This test here is to detect forks
     in a multi-threaded process. */
  if ( getpid () != my_pid2 )
    {
      pid_t x = getpid();
      add_randomness (&x, sizeof(x), 0);
      just_mixed = 0; /* Make sure it will get mixed. */
      my_pid = x;     /* Also update the static pid. */
      goto retry;
    }
}


/*
 * Add LENGTH bytes of randomness from buffer to the pool.
 * source may be used to specify the randomness source.
 * Source is:
 *	0 - used ony for initialization
 *	1 - fast random poll function
 *	2 - normal poll function
 *	3 - used when level 2 random quality has been requested
 *	    to do an extra pool seed.
 */
static void
add_randomness( const void *buffer, size_t length, int source )
{
  const byte *p = buffer;

  assert (pool_is_locked);
  if (!is_initialized)
    initialize ();
  rndstats.addbytes += length;
  rndstats.naddbytes++;
  while (length-- )
    {
      rndpool[pool_writepos++] ^= *p++;
      if (pool_writepos >= POOLSIZE )
        {
          if (source > 1)
            pool_filled = 1;
          pool_writepos = 0;
          mix_pool(rndpool); rndstats.mixrnd++;
          just_mixed = !length;
	}
    }
}



static void
random_poll()
{
  rndstats.slowpolls++;
  read_random_source (2, POOLSIZE/5, 1);
}


static int (*
getfnc_gather_random (void))(void (*)(const void*, size_t, int), int,
			     size_t, int)
{
  static int (*fnc)(void (*)(const void*, size_t, int), int, size_t, int);
  
  if (fnc)
    return fnc;

#if USE_RNDLINUX
  if ( !access (NAME_OF_DEV_RANDOM, R_OK)
       && !access (NAME_OF_DEV_URANDOM, R_OK))
    {
      fnc = _gcry_rndlinux_gather_random;
      return fnc;
    }
#endif

#if USE_RNDEGD
  if ( _gcry_rndegd_connect_socket (1) != -1 )
    {
      fnc = _gcry_rndegd_gather_random;
      return fnc;
    }
#endif

#if USE_RNDUNIX
  fnc = _gcry_rndunix_gather_random;
  return fnc;
#endif

#if USE_RNDW32
  fnc = _gcry_rndw32_gather_random;
  return fnc;
#endif

  log_fatal (_("no entropy gathering module detected\n"));

  return NULL; /*NOTREACHED*/
}

static void (*
getfnc_fast_random_poll (void))( void (*)(const void*, size_t, int), int)
{
#if USE_RNDW32
  return _gcry_rndw32_gather_random_fast;
#endif
  return NULL;
}


static void
do_fast_random_poll (void)
{
  static void (*fnc)( void (*)(const void*, size_t, int), int) = NULL;
  static int initialized = 0;

  assert (pool_is_locked);

  rndstats.fastpolls++;

  if (!initialized )
    {
      if (!is_initialized )
        initialize();
      initialized = 1;
      fnc = getfnc_fast_random_poll ();
    }

  if (fnc)
    (*fnc)( add_randomness, 1 );

  /* Continue with the generic functions. */
#if HAVE_GETHRTIME
  {	
    hrtime_t tv;
    tv = gethrtime();
    add_randomness( &tv, sizeof(tv), 1 );
  }
#elif HAVE_GETTIMEOFDAY
  {	
    struct timeval tv;
    if( gettimeofday( &tv, NULL ) )
      BUG();
    add_randomness( &tv.tv_sec, sizeof(tv.tv_sec), 1 );
    add_randomness( &tv.tv_usec, sizeof(tv.tv_usec), 1 );
  }
#elif HAVE_CLOCK_GETTIME
  {	struct timespec tv;
  if( clock_gettime( CLOCK_REALTIME, &tv ) == -1 )
    BUG();
  add_randomness( &tv.tv_sec, sizeof(tv.tv_sec), 1 );
  add_randomness( &tv.tv_nsec, sizeof(tv.tv_nsec), 1 );
  }
#else /* use times */
# ifndef HAVE_DOSISH_SYSTEM
  {	struct tms buf;
  times( &buf );
  add_randomness( &buf, sizeof buf, 1 );
  }
# endif
#endif

#ifdef HAVE_GETRUSAGE
# ifdef RUSAGE_SELF
  {	
    struct rusage buf;
    /* QNX/Neutrino does return ENOSYS - so we just ignore it and
     * add whatever is in buf.  In a chroot environment it might not
     * work at all (i.e. because /proc/ is not accessible), so we better 
     * ugnore all error codes and hope for the best
     */
    getrusage (RUSAGE_SELF, &buf );
    add_randomness( &buf, sizeof buf, 1 );
    memset( &buf, 0, sizeof buf );
  }
# else /*!RUSAGE_SELF*/
#  ifdef __GCC__
#   warning There is no RUSAGE_SELF on this system
#  endif
# endif /*!RUSAGE_SELF*/
#endif /*HAVE_GETRUSAGE*/

  /* time and clock are availabe on all systems - so we better do it
     just in case one of the above functions didn't work */
  {
    time_t x = time(NULL);
    add_randomness( &x, sizeof(x), 1 );
  }
  {	
    clock_t x = clock();
    add_randomness( &x, sizeof(x), 1 );
  }
}


/* The fast random pool function as called at some places in
   libgcrypt.  This is merely a wrapper to make sure that this module
   is initalized and to look the pool.  Note, that this function is a
   NOP unless a random function has been used or _gcry_initialize (1)
   has been used.  We use this hack so that the internal use of this
   function in cipher_open and md_open won't start filling up the
   radnom pool, even if no random will be required by the process. */
void
_gcry_fast_random_poll (void)
{
  int err;

  if (!is_initialized)
    return;

  err = ath_mutex_lock (&pool_lock);
  if (err)
    log_fatal ("failed to acquire the pool lock: %s\n", strerror (err));
  pool_is_locked = 1;

  do_fast_random_poll ();

  pool_is_locked = 0;
  err = ath_mutex_unlock (&pool_lock);
  if (err)
    log_fatal ("failed to acquire the pool lock: %s\n", strerror (err));

}



static void
read_random_source( int requester, size_t length, int level )
{
  static int (*fnc)(void (*)(const void*, size_t, int), int,
                             size_t, int) = NULL;
  if (!fnc ) 
    {
      if (!is_initialized )
        initialize();

      fnc = getfnc_gather_random ();

      if (!fnc)
        {
          faked_rng = 1;
          fnc = gather_faked;
	}
      if (!requester && !length && !level)
        return; /* Just the init was requested. */
    }

  if ((*fnc)( add_randomness, requester, length, level ) < 0)
    log_fatal ("No way to gather entropy for the RNG\n");
}


static int
gather_faked( void (*add)(const void*, size_t, int), int requester,
	      size_t length, int level )
{
    static int initialized=0;
    size_t n;
    char *buffer, *p;

    if( !initialized ) {
	log_info(_("WARNING: using insecure random number generator!!\n"));
	/* we can't use tty_printf here - do we need this function at
	  all - does it really make sense or canit be viewed as a potential
	  security problem ? wk 17.11.99 */
#if 0
	tty_printf(_("The random number generator is only a kludge to let\n"
		   "it run - it is in no way a strong RNG!\n\n"
		   "DON'T USE ANY DATA GENERATED BY THIS PROGRAM!!\n\n"));
#endif
	initialized=1;
#ifdef HAVE_RAND
	srand( time(NULL)*getpid());
#else
	srandom( time(NULL)*getpid());
#endif
    }

    p = buffer = gcry_xmalloc( length );
    n = length;
#ifdef HAVE_RAND
    while( n-- )
	*p++ = ((unsigned)(1 + (int) (256.0*rand()/(RAND_MAX+1.0)))-1);
#else
    while( n-- )
	*p++ = ((unsigned)(1 + (int) (256.0*random()/(RAND_MAX+1.0)))-1);
#endif
    add_randomness( buffer, length, requester );
    gcry_free(buffer);
    return 0; /* okay */
}


/* Create an unpredicable nonce of LENGTH bytes in BUFFER. */
void
gcry_create_nonce (void *buffer, size_t length)
{
  static unsigned char nonce_buffer[20+8];
  static int nonce_buffer_initialized = 0;
  static volatile pid_t my_pid; /* The volatile is there to make sure the
                                   compiler does not optimize the code away
                                   in case the getpid function is badly
                                   attributed. */
  volatile pid_t apid;
  unsigned char *p;
  size_t n;
  int err;

  /* Make sure we are initialized. */
  if (!is_initialized)
    initialize ();

  /* Acquire the nonce buffer lock. */
  err = ath_mutex_lock (&nonce_buffer_lock);
  if (err)
    log_fatal ("failed to acquire the nonce buffer lock: %s\n",
               strerror (err));

  apid = getpid ();
  /* The first time intialize our buffer. */
  if (!nonce_buffer_initialized)
    {
      time_t atime = time (NULL);
      pid_t xpid = apid;

      my_pid = apid;

      if ((sizeof apid + sizeof atime) > sizeof nonce_buffer)
        BUG ();

      /* Initialize the first 20 bytes with a reasonable value so that
         a failure of gcry_randomize won't affect us too much.  Don't
         care about the uninitialized remaining bytes. */
      p = nonce_buffer;
      memcpy (p, &xpid, sizeof xpid);
      p += sizeof xpid;
      memcpy (p, &atime, sizeof atime); 

      /* Initialize the never changing private part of 64 bits. */
      gcry_randomize (nonce_buffer+20, 8, GCRY_WEAK_RANDOM);

      nonce_buffer_initialized = 1;
    }
  else if ( my_pid != apid )
    {
      /* We forked. Need to reseed the buffer - doing this for the
         private part should be sufficient. */
      gcry_randomize (nonce_buffer+20, 8, GCRY_WEAK_RANDOM);
      /* Update the pid so that we won't run into here again and
         again. */
      my_pid = apid;
    }

  /* Create the nonce by hashing the entire buffer, returning the hash
     and updating the first 20 bytes of the buffer with this hash. */
  for (p = buffer; length > 0; length -= n, p += n)
    {
      _gcry_sha1_hash_buffer ((char*)nonce_buffer,
                              (char*)nonce_buffer, sizeof nonce_buffer);
      n = length > 20? 20 : length;
      memcpy (p, nonce_buffer, n);
    }


  /* Release the nonce buffer lock. */
  err = ath_mutex_unlock (&nonce_buffer_lock);
  if (err)
    log_fatal ("failed to release the nonce buffer lock: %s\n",
               strerror (err));

}
