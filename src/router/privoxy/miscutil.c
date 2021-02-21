/*********************************************************************
 *
 * File        :  $Source: /cvsroot/ijbswa/current/miscutil.c,v $
 *
 * Purpose     :  zalloc, hash_string, strcmpic, strncmpic, and
 *                MinGW32 strdup functions.  These are each too small
 *                to deserve their own file but don't really fit in
 *                any other file.
 *
 * Copyright   :  Written by and Copyright (C) 2001-2020 the
 *                Privoxy team. https://www.privoxy.org/
 *
 *                Based on the Internet Junkbuster originally written
 *                by and Copyright (C) 1997 Anonymous Coders and
 *                Junkbusters Corporation.  http://www.junkbusters.com
 *
 *                The timegm replacement function was taken from GnuPG,
 *                Copyright (C) 2004 Free Software Foundation, Inc.
 *
 *                This program is free software; you can redistribute it
 *                and/or modify it under the terms of the GNU General
 *                Public License as published by the Free Software
 *                Foundation; either version 2 of the License, or (at
 *                your option) any later version.
 *
 *                This program is distributed in the hope that it will
 *                be useful, but WITHOUT ANY WARRANTY; without even the
 *                implied warranty of MERCHANTABILITY or FITNESS FOR A
 *                PARTICULAR PURPOSE.  See the GNU General Public
 *                License for more details.
 *
 *                The GNU General Public License should be included with
 *                this file.  If not, you can view it at
 *                http://www.gnu.org/copyleft/gpl.html
 *                or write to the Free Software Foundation, Inc., 59
 *                Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *********************************************************************/


#include "config.h"

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif /* #if !defined(_WIN32) */
#include <string.h>
#include <ctype.h>
#include <assert.h>

#if !defined(HAVE_TIMEGM) && defined(HAVE_TZSET) && defined(HAVE_PUTENV)
#include <time.h>
#endif /* !defined(HAVE_TIMEGM) && defined(HAVE_TZSET) && defined(HAVE_PUTENV) */

#include "project.h"
#include "miscutil.h"
#include "jcc.h"
#include "errlog.h"

/*********************************************************************
 *
 * Function    :  zalloc
 *
 * Description :  Returns allocated memory that is initialized
 *                with zeros.
 *
 * Parameters  :
 *          1  :  size = Size of memory chunk to return.
 *
 * Returns     :  Pointer to newly alloc'd memory chunk.
 *
 *********************************************************************/
void *zalloc(size_t size)
{
   void * ret;

#ifdef HAVE_CALLOC
   ret = calloc(1, size);
#else
#warning calloc appears to be unavailable. Your platform will become unsupported in the future
   if ((ret = (void *)malloc(size)) != NULL)
   {
      memset(ret, 0, size);
   }
#endif

   return(ret);

}


/*********************************************************************
 *
 * Function    :  zalloc_or_die
 *
 * Description :  zalloc wrapper that either succeeds or causes
 *                program termination.
 *
 *                Useful in situations were the string length is
 *                "small" and zalloc() failures couldn't be handled
 *                better anyway. In case of debug builds, failures
 *                trigger an assert().
 *
 * Parameters  :
 *          1  :  size = Size of memory chunk to return.
 *
 * Returns     :  Pointer to newly malloc'd memory chunk.
 *
 *********************************************************************/
void *zalloc_or_die(size_t size)
{
   void *buffer;

   buffer = zalloc(size);
   if (buffer == NULL)
   {
      assert(buffer != NULL);
      log_error(LOG_LEVEL_FATAL, "Out of memory in zalloc_or_die().");
      exit(1);
   }

   return(buffer);

}

/*********************************************************************
 *
 * Function    :  strdup_or_die
 *
 * Description :  strdup wrapper that either succeeds or causes
 *                program termination.
 *
 *                Useful in situations were the string length is
 *                "small" and strdup() failures couldn't be handled
 *                better anyway. In case of debug builds, failures
 *                trigger an assert().
 *
 * Parameters  :
 *          1  :  str = String to duplicate
 *
 * Returns     :  Pointer to newly strdup'd copy of the string.
 *
 *********************************************************************/
char *strdup_or_die(const char *str)
{
   char *new_str;

   new_str = strdup(str);

   if (new_str == NULL)
   {
      assert(new_str != NULL);
      log_error(LOG_LEVEL_FATAL, "Out of memory in strdup_or_die().");
      exit(1);
   }

   return(new_str);

}


/*********************************************************************
 *
 * Function    :  malloc_or_die
 *
 * Description :  malloc wrapper that either succeeds or causes
 *                program termination.
 *
 *                Useful in situations were the buffer size is "small"
 *                and malloc() failures couldn't be handled better
 *                anyway. In case of debug builds, failures trigger
 *                an assert().
 *
 * Parameters  :
 *          1  :  buffer_size = Size of the space to allocate
 *
 * Returns     :  Pointer to newly malloc'd memory
 *
 *********************************************************************/
void *malloc_or_die(size_t buffer_size)
{
   char *new_buf;

   if (buffer_size == 0)
   {
      log_error(LOG_LEVEL_ERROR,
         "malloc_or_die() called with buffer size 0");
      assert(buffer_size != 0);
      buffer_size = 4096;
   }

   new_buf = malloc(buffer_size);

   if (new_buf == NULL)
   {
      assert(new_buf != NULL);
      log_error(LOG_LEVEL_FATAL, "Out of memory in malloc_or_die().");
      exit(1);
   }

   return(new_buf);

}


#if defined(unix)
/*********************************************************************
 *
 * Function    :  write_pid_file
 *
 * Description :  Writes a pid file with the pid of the main process.
 *                Exits if the file can't be opened
 *
 * Parameters  :
 *          1  :  pid_file = Path of the pid file that gets created.
 *
 * Returns     :  N/A
 *
 *********************************************************************/
void write_pid_file(const char *pid_file)
{
   FILE   *fp;

   if ((fp = fopen(pid_file, "w")) == NULL)
   {
      log_error(LOG_LEVEL_FATAL, "can't open pid file '%s': %E", pid_file);
   }
   else
   {
      fprintf(fp, "%u\n", (unsigned int) getpid());
      fclose (fp);
   }
   return;

}
#endif /* def unix */


/*********************************************************************
 *
 * Function    :  hash_string
 *
 * Description :  Take a string and compute a (hopefully) unique numeric
 *                integer value. This is useful to "switch" a string.
 *
 * Parameters  :
 *          1  :  s : string to be hashed.
 *
 * Returns     :  The string's hash
 *
 *********************************************************************/
unsigned int hash_string(const char* s)
{
   unsigned int h = 0;

   for (; *s; ++s)
   {
      h = 5 * h + (unsigned int)*s;
   }

   return (h);

}


/*********************************************************************
 *
 * Function    :  strcmpic
 *
 * Description :  Case insensitive string comparison
 *
 * Parameters  :
 *          1  :  s1 = string 1 to compare
 *          2  :  s2 = string 2 to compare
 *
 * Returns     :  0 if s1==s2, Negative if s1<s2, Positive if s1>s2
 *
 *********************************************************************/
int strcmpic(const char *s1, const char *s2)
{
   if (!s1) s1 = "";
   if (!s2) s2 = "";

   while (*s1 && *s2)
   {
      if ((*s1 != *s2) && (privoxy_tolower(*s1) != privoxy_tolower(*s2)))
      {
         break;
      }
      s1++, s2++;
   }
   return(privoxy_tolower(*s1) - privoxy_tolower(*s2));

}


/*********************************************************************
 *
 * Function    :  strncmpic
 *
 * Description :  Case insensitive string comparison (up to n characters)
 *
 * Parameters  :
 *          1  :  s1 = string 1 to compare
 *          2  :  s2 = string 2 to compare
 *          3  :  n = maximum characters to compare
 *
 * Returns     :  0 if s1==s2, Negative if s1<s2, Positive if s1>s2
 *
 *********************************************************************/
int strncmpic(const char *s1, const char *s2, size_t n)
{
   if (n <= (size_t)0) return(0);
   if (!s1) s1 = "";
   if (!s2) s2 = "";

   while (*s1 && *s2)
   {
      if ((*s1 != *s2) && (privoxy_tolower(*s1) != privoxy_tolower(*s2)))
      {
         break;
      }

      if (--n <= (size_t)0) break;

      s1++, s2++;
   }
   return(privoxy_tolower(*s1) - privoxy_tolower(*s2));

}


/*********************************************************************
 *
 * Function    :  chomp
 *
 * Description :  In-situ-eliminate all leading and trailing whitespace
 *                from a string.
 *
 * Parameters  :
 *          1  :  s : string to be chomped.
 *
 * Returns     :  chomped string
 *
 *********************************************************************/
char *chomp(char *string)
{
   char *p, *q, *r;

   /*
    * strip trailing whitespace
    */
   p = string + strlen(string);
   while (p > string && privoxy_isspace(*(p-1)))
   {
      p--;
   }
   *p = '\0';

   /*
    * find end of leading whitespace
    */
   q = r = string;
   while (*q && privoxy_isspace(*q))
   {
      q++;
   }

   /*
    * if there was any, move the rest forwards
    */
   if (q != string)
   {
      while (q <= p)
      {
         *r++ = *q++;
      }
   }

   return(string);

}


/*********************************************************************
 *
 * Function    :  string_append
 *
 * Description :  Reallocate target_string and append text to it.
 *                This makes it easier to append to malloc'd strings.
 *                This is similar to the (removed) strsav(), but
 *                running out of memory isn't catastrophic.
 *
 *                Programming style:
 *
 *                The following style provides sufficient error
 *                checking for this routine, with minimal clutter
 *                in the source code.  It is recommended if you
 *                have many calls to this function:
 *
 *                char * s = strdup(...); // don't check for error
 *                string_append(&s, ...);  // don't check for error
 *                string_append(&s, ...);  // don't check for error
 *                string_append(&s, ...);  // don't check for error
 *                if (NULL == s) { ... handle error ... }
 *
 *                OR, equivalently:
 *
 *                char * s = strdup(...); // don't check for error
 *                string_append(&s, ...);  // don't check for error
 *                string_append(&s, ...);  // don't check for error
 *                if (string_append(&s, ...)) {... handle error ...}
 *
 * Parameters  :
 *          1  :  target_string = Pointer to old text that is to be
 *                extended.  *target_string will be free()d by this
 *                routine.  target_string must be non-NULL.
 *                If *target_string is NULL, this routine will
 *                do nothing and return with an error - this allows
 *                you to make many calls to this routine and only
 *                check for errors after the last one.
 *          2  :  text_to_append = Text to be appended to old.
 *                Must not be NULL.
 *
 * Returns     :  JB_ERR_OK on success, and sets *target_string
 *                   to newly malloc'ed appended string.  Caller
 *                   must free(*target_string).
 *                JB_ERR_MEMORY on out-of-memory.  (And free()s
 *                   *target_string and sets it to NULL).
 *                JB_ERR_MEMORY if *target_string is NULL.
 *
 *********************************************************************/
jb_err string_append(char **target_string, const char *text_to_append)
{
   size_t old_len;
   char *new_string;
   size_t new_size;

   assert(target_string);
   assert(text_to_append);

   if (*target_string == NULL)
   {
      return JB_ERR_MEMORY;
   }

   if (*text_to_append == '\0')
   {
      return JB_ERR_OK;
   }

   old_len = strlen(*target_string);

   new_size = strlen(text_to_append) + old_len + 1;

   if (NULL == (new_string = realloc(*target_string, new_size)))
   {
      free(*target_string);

      *target_string = NULL;
      return JB_ERR_MEMORY;
   }

   strlcpy(new_string + old_len, text_to_append, new_size - old_len);

   *target_string = new_string;
   return JB_ERR_OK;
}


/*********************************************************************
 *
 * Function    :  string_join
 *
 * Description :  Join two strings together.  Frees BOTH the original
 *                strings.  If either or both input strings are NULL,
 *                fails as if it had run out of memory.
 *
 *                For comparison, string_append requires that the
 *                second string is non-NULL, and doesn't free it.
 *
 *                Rationale: Too often, we want to do
 *                string_append(s, html_encode(s2)).  That assert()s
 *                if s2 is NULL or if html_encode() runs out of memory.
 *                It also leaks memory.  Proper checking is cumbersome.
 *                The solution: string_join(s, html_encode(s2)) is safe,
 *                and will free the memory allocated by html_encode().
 *
 * Parameters  :
 *          1  :  target_string = Pointer to old text that is to be
 *                extended.  *target_string will be free()d by this
 *                routine.  target_string must be non-NULL.
 *          2  :  text_to_append = Text to be appended to old.
 *
 * Returns     :  JB_ERR_OK on success, and sets *target_string
 *                   to newly malloc'ed appended string.  Caller
 *                   must free(*target_string).
 *                JB_ERR_MEMORY on out-of-memory, or if
 *                   *target_string or text_to_append is NULL.  (In
 *                   this case, frees *target_string and text_to_append,
 *                   sets *target_string to NULL).
 *
 *********************************************************************/
jb_err string_join(char **target_string, char *text_to_append)
{
   jb_err err;

   assert(target_string);

   if (text_to_append == NULL)
   {
      freez(*target_string);
      return JB_ERR_MEMORY;
   }

   err = string_append(target_string, text_to_append);

   freez(text_to_append);

   return err;
}


/*********************************************************************
 *
 * Function    :  string_toupper
 *
 * Description :  Produce a copy of string with all convertible
 *                characters converted to uppercase.
 *
 * Parameters  :
 *          1  :  string = string to convert
 *
 * Returns     :  Uppercase copy of string if possible,
 *                NULL on out-of-memory or if string was NULL.
 *
 *********************************************************************/
char *string_toupper(const char *string)
{
   char *result, *p;
   const char *q;

   if (!string || ((result = (char *) zalloc(strlen(string) + 1)) == NULL))
   {
      return NULL;
   }

   q = string;
   p = result;

   while (*q != '\0')
   {
      *p++ = (char)toupper((int) *q++);
   }

   return result;

}


/*********************************************************************
 *
 * Function    :  string_tolower
 *
 * Description :  Produce a copy of string with all convertible
 *                characters converted to lowercase.
 *
 * Parameters  :
 *          1  :  string = string to convert
 *
 * Returns     :  Lowercase copy of string if possible,
 *                NULL on out-of-memory or if string was NULL.
 *
 *********************************************************************/
char *string_tolower(const char *string)
{
   char *result, *p;
   const char *q;

   if (!string || ((result = (char *)zalloc(strlen(string) + 1)) == NULL))
   {
      return NULL;
   }

   q = string;
   p = result;

   while (*q != '\0')
   {
      *p++ = (char)privoxy_tolower(*q++);
   }

   return result;

}


/*********************************************************************
 *
 * Function    :  string_move
 *
 * Description :  memmove wrapper to move the last part of a string
 *                towards the beginning, overwriting the part in
 *                the middle. strlcpy() can't be used here as the
 *                strings overlap.
 *
 * Parameters  :
 *          1  :  dst = Destination to overwrite
 *          2  :  src = Source to move.
 *
 * Returns     :  N/A
 *
 *********************************************************************/
void string_move(char *dst, char *src)
{
   assert(dst < src);

   /* +1 to copy the terminating nul as well. */
   memmove(dst, src, strlen(src)+1);
}


/*********************************************************************
 *
 * Function    :  bindup
 *
 * Description :  Duplicate the first n characters of a string that may
 *                contain '\0' characters.
 *
 * Parameters  :
 *          1  :  string = string to be duplicated
 *          2  :  len = number of bytes to duplicate
 *
 * Returns     :  pointer to copy, or NULL if failure
 *
 *********************************************************************/
char *bindup(const char *string, size_t len)
{
   char *duplicate;

   duplicate = (char *)malloc(len);
   if (NULL != duplicate)
   {
      memcpy(duplicate, string, len);
   }

   return duplicate;

}


/*********************************************************************
 *
 * Function    :  make_path
 *
 * Description :  Takes a directory name and a file name, returns
 *                the complete path.  Handles windows/unix differences.
 *                If the file name is already an absolute path, or if
 *                the directory name is NULL or empty, it returns
 *                the filename.
 *
 * Parameters  :
 *          1  :  dir: Name of directory or NULL for none.
 *          2  :  file: Name of file.  Should not be NULL or empty.
 *
 * Returns     :  "dir/file" (Or on windows, "dir\file").
 *                It allocates the string on the heap.  Caller frees.
 *                Returns NULL in error (i.e. NULL file or out of
 *                memory)
 *
 *********************************************************************/
char * make_path(const char * dir, const char * file)
{
   if ((file == NULL) || (*file == '\0'))
   {
      return NULL; /* Error */
   }

   if ((dir == NULL) || (*dir == '\0') /* No directory specified */
#if defined(_WIN32)
      || (*file == '\\') || (file[1] == ':') /* Absolute path (DOS) */
#else /* ifndef _WIN32 */
      || (*file == '/') /* Absolute path (U*ix) */
#endif /* ifndef _WIN32 */
      )
   {
      return strdup(file);
   }
   else
   {
      char * path;
      size_t path_size = strlen(dir) + strlen(file) + 2; /* +2 for trailing (back)slash and \0 */

#if defined(unix)
      if (*dir != '/' && basedir && *basedir)
      {
         /*
          * Relative path, so start with the base directory.
          */
         path_size += strlen(basedir) + 1; /* +1 for the slash */
         path = malloc(path_size);
         if (!path) log_error(LOG_LEVEL_FATAL, "malloc failed!");
         strlcpy(path, basedir, path_size);
         strlcat(path, "/", path_size);
         strlcat(path, dir, path_size);
      }
      else
#endif /* defined unix */
      {
         path = malloc(path_size);
         if (!path) log_error(LOG_LEVEL_FATAL, "malloc failed!");
         strlcpy(path, dir, path_size);
      }

      assert(NULL != path);
#if defined(_WIN32)
      if (path[strlen(path)-1] != '\\')
      {
         strlcat(path, "\\", path_size);
      }
#else /* ifndef _WIN32 */
      if (path[strlen(path)-1] != '/')
      {
         strlcat(path, "/", path_size);
      }
#endif /* ifndef _WIN32 */
      strlcat(path, file, path_size);

      return path;
   }
}


/*********************************************************************
 *
 * Function    :  pick_from_range
 *
 * Description :  Pick a positive number out of a given range.
 *                Should only be used if randomness would be nice,
 *                but isn't really necessary.
 *
 * Parameters  :
 *          1  :  range: Highest possible number to pick.
 *
 * Returns     :  Picked number.
 *
 *********************************************************************/
long int pick_from_range(long int range)
{
   long int number;
#ifdef _WIN32
   static unsigned long seed = 0;
#endif /* def _WIN32 */

   assert(range != 0);
   assert(range > 0);

   if (range <= 0) return 0;

#ifdef HAVE_ARC4RANDOM
   number = arc4random() % range + 1;
#elif defined(HAVE_RANDOM)
   number = random() % range + 1;
#elif defined(MUTEX_LOCKS_AVAILABLE)
   privoxy_mutex_lock(&rand_mutex);
#ifdef _WIN32
   if (!seed)
   {
      seed = (unsigned long)(GetCurrentThreadId()+GetTickCount());
   }
   srand(seed);
   seed = (unsigned long)((rand() << 16) + rand());
#endif /* def _WIN32 */
   number = (unsigned long)((rand() << 16) + (rand())) % (unsigned long)(range + 1);
   privoxy_mutex_unlock(&rand_mutex);
#else
   /*
    * XXX: Which platforms reach this and are there
    * better options than just using rand() and hoping
    * that it's safe?
    */
   log_error(LOG_LEVEL_INFO, "No thread-safe PRNG available? Header time randomization "
      "might cause crashes, predictable results or even combine these fine options.");
   number = rand() % (long int)(range + 1);

#endif /* (def HAVE_ARC4RANDOM) */

   return number;
}


#ifdef USE_PRIVOXY_STRLCPY
/*********************************************************************
 *
 * Function    :  privoxy_strlcpy
 *
 * Description :  strlcpy(3) look-alike for those without decent libc.
 *
 * Parameters  :
 *          1  :  destination: buffer to copy into.
 *          2  :  source: String to copy.
 *          3  :  size: Size of destination buffer.
 *
 * Returns     :  The length of the string that privoxy_strlcpy() tried to create.
 *
 *********************************************************************/
size_t privoxy_strlcpy(char *destination, const char *source, const size_t size)
{
   if (0 < size)
   {
      snprintf(destination, size, "%s", source);
      /*
       * Platforms that lack strlcpy() also tend to have
       * a broken snprintf implementation that doesn't
       * guarantee nul termination.
       *
       * XXX: the configure script should detect and reject those.
       */
      destination[size-1] = '\0';
   }
   return strlen(source);
}
#endif /* def USE_PRIVOXY_STRLCPY */


#ifndef HAVE_STRLCAT
/*********************************************************************
 *
 * Function    :  privoxy_strlcat
 *
 * Description :  strlcat(3) look-alike for those without decent libc.
 *
 * Parameters  :
 *          1  :  destination: C string.
 *          2  :  source: String to copy.
 *          3  :  size: Size of destination buffer.
 *
 * Returns     :  The length of the string that privoxy_strlcat() tried to create.
 *
 *********************************************************************/
size_t privoxy_strlcat(char *destination, const char *source, const size_t size)
{
   const size_t old_length = strlen(destination);
   return old_length + strlcpy(destination + old_length, source, size - old_length);
}
#endif /* ndef HAVE_STRLCAT */


/*********************************************************************
 *
 * Function    :  privoxy_millisleep
 *
 * Description :  Sleep a number of milliseconds
 *
 * Parameters  :
 *          1  :  delay: Number of milliseconds to sleep
 *
 * Returns     :  -1 on error, 0 otherwise
 *
 *********************************************************************/
int privoxy_millisleep(unsigned milliseconds)
{
#ifdef HAVE_NANOSLEEP
   struct timespec rqtp = {0};
   struct timespec rmtp = {0};

   rqtp.tv_sec = milliseconds / 1000;
   rqtp.tv_nsec = (milliseconds % 1000) * 1000 * 1000;

   return nanosleep(&rqtp, &rmtp);
#elif defined (_WIN32)
   Sleep(milliseconds);

   return 0;
#else
#warning Missing privoxy_milisleep() implementation. delay-response{} will not work.

   return -1;
#endif /* def HAVE_NANOSLEEP */

}


/*********************************************************************
 *
 * Function    :  privoxy_gmtime_r
 *
 * Description :  Behave like gmtime_r() and convert a
 *                time_t to a struct tm.
 *
 * Parameters  :
 *          1  :  time_spec: The time to convert
 *          2  :  result: The struct tm to use as storage
 *
 * Returns     :  Pointer to the result or NULL on error.
 *
 *********************************************************************/
struct tm *privoxy_gmtime_r(const time_t *time_spec, struct tm *result)
{
   struct tm *timeptr;

#ifdef HAVE_GMTIME_R
   timeptr = gmtime_r(time_spec, result);
#elif defined(MUTEX_LOCKS_AVAILABLE)
   privoxy_mutex_lock(&gmtime_mutex);
   timeptr = gmtime(time_spec);
#else
#warning Using unlocked gmtime()
   timeptr = gmtime(time_spec);
#endif

   if (timeptr == NULL)
   {
#if !defined(HAVE_GMTIME_R) && defined(MUTEX_LOCKS_AVAILABLE)
      privoxy_mutex_unlock(&gmtime_mutex);
#endif
      return NULL;
   }

#if !defined(HAVE_GMTIME_R)
   *result = *timeptr;
   timeptr = result;
#ifdef MUTEX_LOCKS_AVAILABLE
   privoxy_mutex_unlock(&gmtime_mutex);
#endif
#endif

   return timeptr;
}

#if !defined(HAVE_TIMEGM) && defined(HAVE_TZSET) && defined(HAVE_PUTENV)
/*********************************************************************
 *
 * Function    :  timegm
 *
 * Description :  libc replacement function for the inverse of gmtime().
 *                Copyright (C) 2004 Free Software Foundation, Inc.
 *
 *                Code originally copied from GnuPG, modifications done
 *                for Privoxy: style changed, #ifdefs for _WIN32 added
 *                to have it work on mingw32.
 *
 *                XXX: It's very unlikely to happen, but if the malloc()
 *                call fails the time zone will be permanently set to UTC.
 *
 * Parameters  :
 *          1  :  tm: Broken-down time struct.
 *
 * Returns     :  tm converted into time_t seconds.
 *
 *********************************************************************/
time_t timegm(struct tm *tm)
{
   time_t answer;
   char *zone;

   zone = getenv("TZ");
   putenv("TZ=UTC");
   tzset();
   answer = mktime(tm);
   if (zone)
   {
      char *old_zone;

      old_zone = malloc(3 + strlen(zone) + 1);
      if (old_zone)
      {
         strcpy(old_zone, "TZ=");
         strcat(old_zone, zone);
         putenv(old_zone);
#ifdef _WIN32
         /* http://man7.org/linux/man-pages/man3/putenv.3.html
          *   int putenv(char *string);
          *     The string pointed to by string becomes part of the environment, so altering the
          *     string changes the environment.
          * In other words, the memory pointed to by *string is used until
          *   a) another call to putenv() with the same e-var name
          *   b) the program exits
          *
          * Windows e-vars don't work that way, so let's not leak memory.
          */
         free(old_zone);
#endif /* def _WIN32 */
      }
   }
   else
   {
#ifdef HAVE_UNSETENV
      unsetenv("TZ");
#elif defined(_WIN32)
      putenv("TZ=");
#else
      putenv("TZ");
#endif
   }
   tzset();

   return answer;
}
#endif /* !defined(HAVE_TIMEGM) && defined(HAVE_TZSET) && defined(HAVE_PUTENV) */


/*
  Local Variables:
  tab-width: 3
  end:
*/
