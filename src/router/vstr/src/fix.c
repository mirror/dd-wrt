#define FIX_C
/*
 *  Copyright (C) 1999, 2000, 2001, 2002  James Antill
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  email: james@and.org
 */
#include "main.h"

#ifndef HAVE_MEMCHR
void *FIX_SYMBOL(memchr)(const void *source, int chr, size_t num)
{
 const char *tmp = source;

 while (((tmp - (char *)source) < num) && (*tmp != (char) chr))
   ++tmp;

 if (*tmp == chr)
   return (tmp);
 else
   return (NULL);
}
#endif

#ifndef HAVE_MEMRCHR
void *FIX_SYMBOL(memrchr)(const void *source, int chr, size_t num)
{
  const char *tmp = ((char *)source) + num;

  while (((tmp - (char *)source) > 0) && (*--tmp != (char) chr))
  { /* do nothing */ }

  if (*tmp == chr)
    return ((void *)tmp);
  else
    return (NULL);
}
#endif

#ifndef HAVE_MEMCPY
void *FIX_SYMBOL(memcpy)(void *dest, const void *src, size_t n)
{
 unsigned char *to = dest;
 unsigned const char *from = src;

 if (!n)
   return (dest);

 while (n--)
   *to++ = *from++;

 return (dest);
}
#endif

#ifndef HAVE_MEMPCPY
void *FIX_SYMBOL(mempcpy)(void *dest, const void *src, size_t n)
{
  memcpy(dest, src, n);

  return (((char *)dest) + n);
}
#endif

#ifndef HAVE_MEMCMP
int FIX_SYMBOL(memcmp)(const void *dest, const void *src, size_t n)
{
 unsigned const char *to = dest;
 unsigned const char *from = src;

 if (!n)
   return (0);

 while (n--)
 {
  if (*to - *from)
    return (*to - *from);

  ++to;
  ++from;
 }

 return (0);
}
#endif

#ifndef HAVE_MEMMEM
void *FIX_SYMBOL(memmem)(const void *src, size_t src_len,
                         const void *needle, size_t needle_len)
{
 char *tmp = (char *)src;

 if (needle_len > src_len)
   return (NULL);

 while (((size_t)(tmp - (const char *)src) <= (src_len - needle_len)) &&
        memcmp(src, needle, needle_len))
   ++tmp;

 if ((size_t)(tmp - (const char *)src) < (src_len - needle_len))
   return (NULL);

 return ((void *)src);
}
#endif

#ifndef HAVE_STRSPN
size_t FIX_SYMBOL(strspn)(const char *s, const char *str_accept)
{
 size_t num = 0;

 while (*s && strchr(str_accept, *s))
 {
  ++num;
  ++s;
 }

 return (num);
}
#endif

#ifndef HAVE_STRCSPN
size_t FIX_SYMBOL(strcspn)(const char *s, const char *str_reject)
{
 size_t num = 0;

 while (*s && !strchr(str_reject, *s))
 {
  ++num;
  ++s;
 }

 return (num);
}
#endif

#ifndef HAVE_STRNLEN
/* it is part of gnu's extentions */
size_t FIX_SYMBOL(strnlen)(const char *str, size_t count)
{
 const char *tmp = memchr(str, 0, count);

 if (!tmp)
   return (count);

 return (tmp - str);
}
#endif

#ifndef HAVE_STRNCMP
/* it is part of gnu's extentions */
size_t FIX_SYMBOL(strncmp)(const char *str1, const char *str2, size_t count)
{
 if (!count)
   return (0);

 while (!(*str1 - *str2) && *str1 && --count)
 {
  ++str1;
  ++str2;
 }

 return (*str1 - *str2);
}
#endif

#ifndef HAVE_STRNCASECMP
# warning "Assumes ASCI"
/* it is part of gnu's extentions */
size_t FIX_SYMBOL(strncasecmp)(const char *str1, const char *str2, size_t count)
{
 if (!count)
   return (0);

 while (!(tolower((unsigned char)*str1) - tolower((unsigned char)*str2)) &&
        *str1 && --count)
 {
  ++str1;
  ++str2;
 }

 return (tolower((unsigned char)*str1) - tolower((unsigned char)*str2));
}
#endif

#ifndef HAVE_STRCASECMP
# warning "Assumes ASCI"
int FIX_SYMBOL(strcasecmp)(const char *s1, const char *s2)
{
 int ret = tolower((unsigned char) *s1) - tolower((unsigned char) *s2);

 if (ret)
   return (ret);

 while (*s1)
 {
  ++s1;
  ++s2;
  ret = tolower((unsigned char) *s1) - tolower((unsigned char) *s2);
  if (ret)
    break;
 }

 return (ret);
}

#endif

#ifndef HAVE_STPCPY
char *FIX_SYMBOL(stpcpy)(char *copy, const char *from)
{
  while (*from)
    *copy++ = *from++;

  *copy = 0;

  /* returns the end of the copied string */
  return (copy);
}
#endif

#ifndef HAVE_C9X_SNPRINTF_RET
int FIX_SYMBOL(vsnprintf)(char *str, size_t size, const char *fmt, va_list ap)
{
  static FILE *fp = NULL;

  ASSERT(!str && !size);
  if (!fp)
    fp = fopen("/dev/null", "ab");

  return (vfprintf(fp, fmt, ap));
}
#endif

#ifndef HAVE_ASPRINTF
int FIX_SYMBOL(asprintf)(char **ret, const char *fmt, ... )
{
 int sz = 0;
 char solaris_hack[2]; /* Solaris braindamage:
                        * http://bugs.opensolaris.org/bugdatabase/view_bug.do?bug_id=4894857 */
 va_list ap;

 va_start(ap, fmt);

 sz = vsnprintf(solaris_hack, sizeof(solaris_hack), fmt, ap);

 va_end(ap);

 if (sz == -1)
   return (-1);

 if (!(*ret = malloc(sz + 1)))
   return (-1);

 va_start(ap, fmt);

 vsprintf(*ret, fmt, ap);

 va_end(ap);

 return (sz);
}
#endif

#if !defined(HAVE_GETOPT_LONG) && defined(HAVE_POSIX_HOST)
/* Getopt for GNU.
   NOTE: getopt is now part of the C library, so if you don't know what
   "Keep this file name-space clean" means, talk to roland@gnu.ai.mit.edu
   before changing it!

   Copyright (C) 1987, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99
        Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#undef _
#define _(x) (x)

/* This version of `getopt' appears to the caller like standard Unix `getopt'
   but it behaves differently for the user, since it allows the user
   to intersperse the options with the other arguments.

   As `getopt' works, it permutes the elements of ARGV so that,
   when it is done, all the options precede everything else.  Thus
   all application programs are extended to handle flexible argument order.

   Setting the environment variable POSIXLY_CORRECT disables permutation.
   Then the behavior is completely standard.

   GNU application programs can use a third alternative mode in which
   they can distinguish the relative order of options and other arguments.  */

/* #include "getopt.h" */

/* For communication from `getopt' to the caller.
   When `getopt' finds an option that takes an argument,
   the argument value is returned here.
   Also, when `ordering' is RETURN_IN_ORDER,
   each non-option ARGV-element is returned here.  */

char *optarg;

/* Index in ARGV of the next element to be scanned.
   This is used for communication to and from the caller
   and for communication between successive calls to `getopt'.

   On entry to `getopt', zero means this is the first call; initialize.

   When `getopt' returns -1, this is the index of the first of the
   non-option elements that the caller should itself scan.

   Otherwise, `optind' communicates from one call to the next
   how much of ARGV has been scanned so far.  */

/* 1003.2 says this must be 1 before any call.  */
int optind = 1;

/* Formerly, initialization of getopt depended on optind==0, which
   causes problems with re-calling getopt as programs generally don't
   know that. */

int __getopt_initialized;

/* The next char to be scanned in the option-element
   in which the last option character we returned was found.
   This allows us to pick up the scan where we left off.

   If this is zero, or a null string, it means resume the scan
   by advancing to the next ARGV-element.  */

static char *nextchar;

/* Callers store zero here to inhibit the error message
   for unrecognized options.  */

int opterr = 1;

/* Set to an option character which was unrecognized.
   This must be initialized on some systems to avoid linking in the
   system's own getopt implementation.  */

int optopt = '?';

/* Describe how to deal with options that follow non-option ARGV-elements.

   If the caller did not specify anything,
   the default is REQUIRE_ORDER if the environment variable
   POSIXLY_CORRECT is defined, PERMUTE otherwise.

   REQUIRE_ORDER means don't recognize them as options;
   stop option processing when the first non-option is seen.
   This is what Unix does.
   This mode of operation is selected by either setting the environment
   variable POSIXLY_CORRECT, or using `+' as the first character
   of the list of option characters.

   PERMUTE is the default.  We permute the contents of ARGV as we scan,
   so that eventually all the non-options are at the end.  This allows options
   to be given in any order, even with programs that were not written to
   expect this.

   RETURN_IN_ORDER is an option available to programs that were written
   to expect options and other ARGV-elements in any order and that care about
   the ordering of the two.  We describe each non-option ARGV-element
   as if it were the argument of an option with character code 1.
   Using `-' as the first character of the list of option characters
   selects this mode of operation.

   The special argument `--' forces an end of option-scanning regardless
   of the value of `ordering'.  In the case of RETURN_IN_ORDER, only
   `--' can cause `getopt' to return -1 with `optind' != ARGC.  */

static enum
{
  REQUIRE_ORDER, PERMUTE, RETURN_IN_ORDER
} ordering;
/* Value of POSIXLY_CORRECT environment variable.  */
static char *posixly_correct;

/* Avoid depending on library functions or files
   whose names are inconsistent.  */

static char *
my_index (str, chr)
     const char *str;
     int chr;
{
  while (*str)
    {
      if (*str == chr)
        return (char *) str;
      str++;
    }
  return 0;
}

/* Handle permutation of arguments.  */

/* Describe the part of ARGV that contains non-options that have
   been skipped.  `first_nonopt' is the index in ARGV of the first of them;
   `last_nonopt' is the index after the last of them.  */

static int first_nonopt;
static int last_nonopt;

# define SWAP_FLAGS(ch1, ch2)

/* Exchange two adjacent subsequences of ARGV.
   One subsequence is elements [first_nonopt,last_nonopt)
   which contains all the non-options that have been skipped so far.
   The other is elements [last_nonopt,optind), which contains all
   the options processed since those non-options were skipped.

   `first_nonopt' and `last_nonopt' are relocated so that they describe
   the new indices of the non-options in ARGV after they are moved.  */

static void
exchange (argv)
     char **argv;
{
  int bottom = first_nonopt;
  int middle = last_nonopt;
  int top = optind;
  char *tem;

  /* Exchange the shorter segment with the far end of the longer segment.
     That puts the shorter segment into the right place.
     It leaves the longer segment in the right place overall,
     but it consists of two parts that need to be swapped next.  */

  while (top > middle && middle > bottom)
    {
      if (top - middle > middle - bottom)
        {
          /* Bottom segment is the short one.  */
          int len = middle - bottom;
          register int i;

          /* Swap it with the top part of the top segment.  */
          for (i = 0; i < len; i++)
            {
              tem = argv[bottom + i];
              argv[bottom + i] = argv[top - (middle - bottom) + i];
              argv[top - (middle - bottom) + i] = tem;
              SWAP_FLAGS (bottom + i, top - (middle - bottom) + i);
            }
          /* Exclude the moved bottom segment from further swapping.  */
          top -= len;
        }
      else
        {
          /* Top segment is the short one.  */
          int len = top - middle;
          register int i;

          /* Swap it with the bottom part of the bottom segment.  */
          for (i = 0; i < len; i++)
            {
              tem = argv[bottom + i];
              argv[bottom + i] = argv[middle + i];
              argv[middle + i] = tem;
              SWAP_FLAGS (bottom + i, middle + i);
            }
          /* Exclude the moved top segment from further swapping.  */
          bottom += len;
        }
    }

  /* Update records for the slots the non-options now occupy.  */

  first_nonopt += (optind - last_nonopt);
  last_nonopt = optind;
}

/* Initialize the internal data when the first call is made.  */

static const char *_getopt_initialize (int argc, char *const *argv,
                                       const char *optstring)
{
  /* Start processing options with ARGV-element 1 (since ARGV-element 0
     is the program name); the sequence of previously skipped
     non-option ARGV-elements is empty.  */

  first_nonopt = last_nonopt = optind;

  nextchar = NULL;

  posixly_correct = getenv ("POSIXLY_CORRECT");

  /* Determine how to handle the ordering of options and nonoptions.  */

  if (optstring[0] == '-')
    {
      ordering = RETURN_IN_ORDER;
      ++optstring;
    }
  else if (optstring[0] == '+')
    {
      ordering = REQUIRE_ORDER;
      ++optstring;
    }
  else if (posixly_correct != NULL)
    ordering = REQUIRE_ORDER;
  else
    ordering = PERMUTE;

  return optstring;
}

/* Scan elements of ARGV (whose length is ARGC) for option characters
   given in OPTSTRING.

   If an element of ARGV starts with '-', and is not exactly "-" or "--",
   then it is an option element.  The characters of this element
   (aside from the initial '-') are option characters.  If `getopt'
   is called repeatedly, it returns successively each of the option characters
   from each of the option elements.

   If `getopt' finds another option character, it returns that character,
   updating `optind' and `nextchar' so that the next call to `getopt' can
   resume the scan with the following option character or ARGV-element.

   If there are no more option characters, `getopt' returns -1.
   Then `optind' is the index in ARGV of the first ARGV-element
   that is not an option.  (The ARGV-elements have been permuted
   so that those that are not options now come last.)

   OPTSTRING is a string containing the legitimate option characters.
   If an option character is seen that is not listed in OPTSTRING,
   return '?' after printing an error message.  If you set `opterr' to
   zero, the error message is suppressed but we still return '?'.

   If a char in OPTSTRING is followed by a colon, that means it wants an arg,
   so the following text in the same ARGV-element, or the text of the following
   ARGV-element, is returned in `optarg'.  Two colons mean an option that
   wants an optional arg; if there is text in the current ARGV-element,
   it is returned in `optarg', otherwise `optarg' is set to zero.

   If OPTSTRING starts with `-' or `+', it requests different methods of
   handling the non-option ARGV-elements.
   See the comments about RETURN_IN_ORDER and REQUIRE_ORDER, above.

   Long-named options begin with `--' instead of `-'.
   Their names may be abbreviated as long as the abbreviation is unique
   or is an exact match for some defined option.  If they have an
   argument, it follows the option name in the same ARGV-element, separated
   from the option name by a `=', or else the in next ARGV-element.
   When `getopt' finds a long-named option, it returns 0 if that option's
   `flag' field is nonzero, the value of the option's `val' field
   if the `flag' field is zero.
   The elements of ARGV aren't really const, because we permute them.
   But we pretend they're const in the prototype to be compatible
   with other systems.

   LONGOPTS is a vector of `struct option' terminated by an
   element containing a name which is zero.

   LONGIND returns the index in LONGOPT of the long-named option found.
   It is only valid when a long-named option has been found by the most
   recent call.

   If LONG_ONLY is nonzero, '-' as well as '--' can introduce
   long-named options.  */

int
FIX_SYMBOL(_getopt_internal) (argc, argv, optstring, longopts, longind, long_only)
     int argc;
     char *const *argv;
     const char *optstring;
     const struct option *longopts;
     int *longind;
     int long_only;
{
  optarg = NULL;

  if (argc < 1)
    return -1;

  if (optind == 0 || !__getopt_initialized)
    {
      if (optind == 0)
        optind = 1;     /* Don't scan ARGV[0], the program name.  */
      optstring = _getopt_initialize (argc, argv, optstring);
      __getopt_initialized = 1;
    }

  /* Test whether ARGV[optind] points to a non-option argument.
     Either it does not have option syntax, or there is an environment flag
     from the shell indicating it is not an option.  The later information
     is only used when the used in the GNU libc.  */
# define NONOPTION_P (argv[optind][0] != '-' || argv[optind][1] == '\0')

  if (nextchar == NULL || *nextchar == '\0')
    {
      /* Advance to the next ARGV-element.  */

      /* Give FIRST_NONOPT & LAST_NONOPT rational values if OPTIND has been
         moved back by the user (who may also have changed the arguments).  */
      if (last_nonopt > optind)
        last_nonopt = optind;
      if (first_nonopt > optind)
        first_nonopt = optind;

      if (ordering == PERMUTE)
        {
          /* If we have just processed some options following some non-options,
             exchange them so that the options come first.  */

          if (first_nonopt != last_nonopt && last_nonopt != optind)
            exchange ((char **) argv);
          else if (last_nonopt != optind)
            first_nonopt = optind;

          /* Skip any additional non-options
             and extend the range of non-options previously skipped.  */

          while (optind < argc && NONOPTION_P)
            optind++;
          last_nonopt = optind;
        }

      /* The special ARGV-element `--' means premature end of options.
         Skip it like a null option,
         then exchange with previous non-options as if it were an option,
         then skip everything else like a non-option.  */

      if (optind != argc && !strcmp (argv[optind], "--"))
        {
          optind++;

          if (first_nonopt != last_nonopt && last_nonopt != optind)
            exchange ((char **) argv);
          else if (first_nonopt == last_nonopt)
            first_nonopt = optind;
          last_nonopt = argc;

          optind = argc;
        }

      /* If we have done all the ARGV-elements, stop the scan
         and back over any non-options that we skipped and permuted.  */

      if (optind == argc)
        {
          /* Set the next-arg-index to point at the non-options
             that we previously skipped, so the caller will digest them.  */
          if (first_nonopt != last_nonopt)
            optind = first_nonopt;
          return -1;
        }

      /* If we have come to a non-option and did not permute it,
         either stop the scan or describe it to the caller and pass it by.  */

      if (NONOPTION_P)
        {
          if (ordering == REQUIRE_ORDER)
            return -1;
          optarg = argv[optind++];
          return 1;
        }

      /* We have found another option-ARGV-element.
         Skip the initial punctuation.  */

      nextchar = (argv[optind] + 1
                  + (longopts != NULL && argv[optind][1] == '-'));
    }

  /* Decode the current option-ARGV-element.  */

  /* Check whether the ARGV-element is a long option.

     If long_only and the ARGV-element has the form "-f", where f is
     a valid short option, don't consider it an abbreviated form of
     a long option that starts with f.  Otherwise there would be no
     way to give the -f short option.

     On the other hand, if there's a long option "fubar" and
     the ARGV-element is "-fu", do consider that an abbreviation of
     the long option, just like "--fu", and not "-f" with arg "u".

     This distinction seems to be the most useful approach.  */

  if (longopts != NULL
      && (argv[optind][1] == '-'
          || (long_only && (argv[optind][2] || !my_index (optstring, argv[optind][1])))))
    {
      char *nameend;
      const struct option *p;
      const struct option *pfound = NULL;
      int exact = 0;
      int ambig = 0;
      int indfound = -1;
      int option_index;

      for (nameend = nextchar; *nameend && *nameend != '='; nameend++)
        /* Do nothing.  */ ;

      /* Test all long options for either exact match
         or abbreviated matches.  */
      for (p = longopts, option_index = 0; p->name; p++, option_index++)
        if (!strncmp (p->name, nextchar, nameend - nextchar))
          {
            if ((unsigned int) (nameend - nextchar)
                == (unsigned int) strlen (p->name))
              {
                /* Exact match found.  */
                pfound = p;
                indfound = option_index;
                exact = 1;
                break;
              }
            else if (pfound == NULL)
              {
                /* First nonexact match found.  */
                pfound = p;
                indfound = option_index;
              }
            else
              /* Second or later nonexact match found.  */
              ambig = 1;
          }

      if (ambig && !exact)
        {
          if (opterr)
            fprintf (stderr, _("%s: option `%s' is ambiguous\n"),
                     argv[0], argv[optind]);
          nextchar += strlen (nextchar);
          optind++;
          optopt = 0;
          return '?';
        }

      if (pfound != NULL)
        {
          option_index = indfound;
          optind++;
          if (*nameend)
            {
              /* Don't test has_arg with >, because some C compilers don't
                 allow it to be used on enums.  */
              if (pfound->has_arg)
                optarg = nameend + 1;
              else
                {
                  if (opterr)
                    {
                      if (argv[optind - 1][1] == '-')
                        /* --option */
                        fprintf (stderr,
                                 _("%s: option `--%s' doesn't allow an argument\n"),
                                 argv[0], pfound->name);
                      else
                        /* +option or -option */
                        fprintf (stderr,
                                 _("%s: option `%c%s' doesn't allow an argument\n"),
                                 argv[0], argv[optind - 1][0], pfound->name);
                    }

                  nextchar += strlen (nextchar);

                  optopt = pfound->val;
                  return '?';
                }
            }
          else if (pfound->has_arg == 1)
            {
              if (optind < argc)
                optarg = argv[optind++];
              else
                {
                  if (opterr)
                    fprintf (stderr,
                           _("%s: option `%s' requires an argument\n"),
                           argv[0], argv[optind - 1]);
                  nextchar += strlen (nextchar);
                  optopt = pfound->val;
                  return optstring[0] == ':' ? ':' : '?';
                }
            }
          nextchar += strlen (nextchar);
          if (longind != NULL)
            *longind = option_index;
          if (pfound->flag)
            {
              *(pfound->flag) = pfound->val;
              return 0;
            }
          return pfound->val;
        }

      /* Can't find it as a long option.  If this is not getopt_long_only,
         or the option starts with '--' or is not a valid short
         option, then it's an error.
         Otherwise interpret it as a short option.  */
      if (!long_only || argv[optind][1] == '-'
          || my_index (optstring, *nextchar) == NULL)
        {
          if (opterr)
            {
              if (argv[optind][1] == '-')
                /* --option */
                fprintf (stderr, _("%s: unrecognized option `--%s'\n"),
                         argv[0], nextchar);
              else
                /* +option or -option */
                fprintf (stderr, _("%s: unrecognized option `%c%s'\n"),
                         argv[0], argv[optind][0], nextchar);
            }
          nextchar = (char *) "";
          optind++;
          optopt = 0;
          return '?';
        }
    }

  /* Look at and handle the next short option-character.  */

  {
    char c = *nextchar++;
    char *temp = my_index (optstring, c);

    /* Increment `optind' when we start to process its last character.  */
    if (*nextchar == '\0')
      ++optind;

    if (temp == NULL || c == ':')
      {
        if (opterr)
          {
            if (posixly_correct)
              /* 1003.2 specifies the format of this message.  */
              fprintf (stderr, _("%s: illegal option -- %c\n"),
                       argv[0], c);
            else
              fprintf (stderr, _("%s: invalid option -- %c\n"),
                       argv[0], c);
          }
        optopt = c;
        return '?';
      }
    /* Convenience. Treat POSIX -W foo same as long option --foo */
    if (temp[0] == 'W' && temp[1] == ';')
      {
        char *nameend;
        const struct option *p;
        const struct option *pfound = NULL;
        int exact = 0;
        int ambig = 0;
        int indfound = 0;
        int option_index;

        /* This is an option that requires an argument.  */
        if (*nextchar != '\0')
          {
            optarg = nextchar;
            /* If we end this ARGV-element by taking the rest as an arg,
               we must advance to the next element now.  */
            optind++;
          }
        else if (optind == argc)
          {
            if (opterr)
              {
                /* 1003.2 specifies the format of this message.  */
                fprintf (stderr, _("%s: option requires an argument -- %c\n"),
                         argv[0], c);
              }
            optopt = c;
            if (optstring[0] == ':')
              c = ':';
            else
              c = '?';
            return c;
          }
        else
          /* We already incremented `optind' once;
             increment it again when taking next ARGV-elt as argument.  */
          optarg = argv[optind++];

        /* optarg is now the argument, see if it's in the
           table of longopts.  */

        for (nextchar = nameend = optarg; *nameend && *nameend != '='; nameend++)
          /* Do nothing.  */ ;

        /* Test all long options for either exact match
           or abbreviated matches.  */
        for (p = longopts, option_index = 0; p->name; p++, option_index++)
          if (!strncmp (p->name, nextchar, nameend - nextchar))
            {
              if ((unsigned int) (nameend - nextchar) == strlen (p->name))
                {
                  /* Exact match found.  */
                  pfound = p;
                  indfound = option_index;
                  exact = 1;
                  break;
                }
              else if (pfound == NULL)
                {
                  /* First nonexact match found.  */
                  pfound = p;
                  indfound = option_index;
                }
              else
                /* Second or later nonexact match found.  */
                ambig = 1;
            }
        if (ambig && !exact)
          {
            if (opterr)
              fprintf (stderr, _("%s: option `-W %s' is ambiguous\n"),
                       argv[0], argv[optind]);
            nextchar += strlen (nextchar);
            optind++;
            return '?';
          }
        if (pfound != NULL)
          {
            option_index = indfound;
            if (*nameend)
              {
                /* Don't test has_arg with >, because some C compilers don't
                   allow it to be used on enums.  */
                if (pfound->has_arg)
                  optarg = nameend + 1;
                else
                  {
                    if (opterr)
                      fprintf (stderr, _("\
%s: option `-W %s' doesn't allow an argument\n"),
                               argv[0], pfound->name);

                    nextchar += strlen (nextchar);
                    return '?';
                  }
              }
            else if (pfound->has_arg == 1)
              {
                if (optind < argc)
                  optarg = argv[optind++];
                else
                  {
                    if (opterr)
                      fprintf (stderr,
                               _("%s: option `%s' requires an argument\n"),
                               argv[0], argv[optind - 1]);
                    nextchar += strlen (nextchar);
                    return optstring[0] == ':' ? ':' : '?';
                  }
              }
            nextchar += strlen (nextchar);
            if (longind != NULL)
              *longind = option_index;
            if (pfound->flag)
              {
                *(pfound->flag) = pfound->val;
                return 0;
              }
            return pfound->val;
          }
          nextchar = NULL;
          return 'W';   /* Let the application handle it.   */
      }
    if (temp[1] == ':')
      {
        if (temp[2] == ':')
          {
            /* This is an option that accepts an argument optionally.  */
            if (*nextchar != '\0')
              {
                optarg = nextchar;
                optind++;
              }
            else
              optarg = NULL;
            nextchar = NULL;
          }
        else
          {
            /* This is an option that requires an argument.  */
            if (*nextchar != '\0')
              {
                optarg = nextchar;
                /* If we end this ARGV-element by taking the rest as an arg,
                   we must advance to the next element now.  */
                optind++;
              }
            else if (optind == argc)
              {
                if (opterr)
                  {
                    /* 1003.2 specifies the format of this message.  */
                    fprintf (stderr,
                           _("%s: option requires an argument -- %c\n"),
                           argv[0], c);
                  }
                optopt = c;
                if (optstring[0] == ':')
                  c = ':';
                else
                  c = '?';
              }
            else
              /* We already incremented `optind' once;
                 increment it again when taking next ARGV-elt as argument.  */
              optarg = argv[optind++];
            nextchar = NULL;
          }
      }
    return c;
  }
}

int
FIX_SYMBOL(getopt) (argc, argv, optstring)
     int argc;
     char *const *argv;
     const char *optstring;
{
  return _getopt_internal (argc, argv, optstring, NULL, NULL, 0);
}

int
FIX_SYMBOL(getopt_long) (argc, argv, options, long_options, opt_index)
     int argc;
     char *const *argv;
     const char *options;
     const struct option *long_options;
     int *opt_index;
{
  return _getopt_internal (argc, argv, options, long_options, opt_index, 0);
}

/* Like getopt_long, but '-' as well as '--' can indicate a long option.
   If an option that starts with '-' (not '--') doesn't match a long option,
   but does match a short option, it is parsed as a short option
   instead.  */

int
FIX_SYMBOL(getopt_long_only) (argc, argv, options, long_options, opt_index)
     int argc;
     char *const *argv;
     const char *options;
     const struct option *long_options;
     int *opt_index;
{
  return _getopt_internal (argc, argv, options, long_options, opt_index, 1);
}

#endif

#if !defined(HAVE_POLL) && defined(HAVE_POSIX_HOST)
/* hacked slightly, so that invalid fd's work ...
 * glibc-2.1 fixes this itself */
/* Copyright (C) 1994, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* Poll the file descriptors described by the NFDS structures starting at
   FDS.  If TIMEOUT is nonzero and not -1, allow TIMEOUT milliseconds for
   an event to occur; if TIMEOUT is -1, block until an event occurs.
   Returns the number of file descriptors with events, zero if timed out,
   or -1 for errors.  */

int
FIX_SYMBOL(poll) (fds, nfds, timeout)
     struct pollfd *fds;
     unsigned long int nfds;
     int timeout;
{
 struct timeval tv;
 fd_set rset, wset, xset;
 struct pollfd *f;
 int ready;
 int maxfd = 0;

 FD_ZERO (&rset);
 FD_ZERO (&wset);
 FD_ZERO (&xset);

 for (f = fds; f < &fds[nfds]; ++f)
   if (f->fd != -1)
   {
    f->revents = 0;

    if (f->events & POLLIN)
      FD_SET (f->fd, &rset);
    if (f->events & POLLOUT)
      FD_SET (f->fd, &wset);
    if (f->events & POLLPRI)
      FD_SET (f->fd, &xset);
    if (f->fd > maxfd && (f->events & (POLLIN|POLLOUT|POLLPRI)))
      maxfd = f->fd;
   }

 tv.tv_sec = timeout / 1000;
 tv.tv_usec = (timeout % 1000) * 1000;

 ready = select (maxfd + 1, &rset, &wset, &xset,
                 timeout == -1 ? NULL : &tv);
 if ((ready == -1) && (errno == EBADF))
 {
  ready = 0;

  FD_ZERO (&rset);
  FD_ZERO (&wset);
  FD_ZERO (&xset);

  maxfd = -1;

  for (f = fds; f < &fds[nfds]; ++f)
    if (f->fd != -1)
    {
     fd_set sngl_rset, sngl_wset, sngl_xset;

     FD_ZERO (&sngl_rset);
     FD_ZERO (&sngl_wset);
     FD_ZERO (&sngl_xset);

     if (f->events & POLLIN)
       FD_SET (f->fd, &sngl_rset);
     if (f->events & POLLOUT)
       FD_SET (f->fd, &sngl_wset);
     if (f->events & POLLPRI)
       FD_SET (f->fd, &sngl_xset);

     if (f->events & (POLLIN|POLLOUT|POLLPRI))
     {
      struct timeval sngl_tv;

      sngl_tv.tv_sec = 0;
      sngl_tv.tv_usec = 0;

      if (select(f->fd + 1,
                 &sngl_rset, &sngl_wset, &sngl_xset, &sngl_tv) != -1)
      {
       if (f->events & POLLIN)
         FD_SET (f->fd, &rset);
       if (f->events & POLLOUT)
         FD_SET (f->fd, &wset);
       if (f->events & POLLPRI)
         FD_SET (f->fd, &xset);

       if (f->fd > maxfd && (f->events & (POLLIN|POLLOUT|POLLPRI)))
         maxfd = f->fd;
       ++ready;
      }
      else if (errno == EBADF)
        f->revents = POLLNVAL;
      else
        return (-1);
     }
    }

  if (ready)
  { /* Linux alters the tv struct... but it shouldn't matter here ...
     * as we're going to be a little bit out anyway as we've just eaten
     * more than a couple of cpu cycles above */
   ready = select (maxfd + 1, &rset, &wset, &xset,
                   timeout == -1 ? NULL : &tv);
  } /* what to do here ?? */
 }

 if (ready > 0)
   for (f = fds; f < &fds[nfds]; ++f)
     if (f->fd != -1)
     {
      if (FD_ISSET (f->fd, &rset))
        f->revents |= POLLIN;
      if (FD_ISSET (f->fd, &wset))
        f->revents |= POLLOUT;
      if (FD_ISSET (f->fd, &xset))
        f->revents |= POLLPRI;
     }

 return ready;
}
#endif

#if !defined(HAVE_INET_NTOP) && defined(HAVE_POSIX_HOST)
const char *FIX_SYMBOL(inet_ntop)(int af, const void *cp, char *buf, size_t len)
{
#ifdef HAVE_INET_NTOA
  if (af == AF_INET)
  {
    const char *ptr = inet_ntoa(*(struct in_addr *)cp);
    size_t l_len = strlen(ptr) + 1;
    if (l_len > len)
      l_len = len;
    memcpy(buf, ptr, l_len);
    buf[len - 1] = 0; /* assume this is correct */
    return (buf);
  }
#else
# error "inet_ntop doesn't work"
#endif
  return (NULL);
}
#endif

#if !defined(HAVE_INET_PTON) && defined(HAVE_POSIX_HOST)
int FIX_SYMBOL(inet_pton)(int af, const char *cp, void *buf)
{
#ifdef HAVE_INET_ATON
  if (af == AF_INET)
  {
    return (inet_aton(cp, buf));
  }
#else
# ifdef INET_ADDR
  if (af == AF_INET)
  {
    long addr = 0;
    addr = inet_addr(cp);
    if (addr == -1)
      return (0);

    /* will people assume it's an int or a long ? */
    memcpy(buf, &addr, sizeof(long));

    return (1);
  }
# else
# error "inet_pton doesn't work"
# endif
#endif
  return (0);
}
#endif

#if !defined(HAVE_SENDFILE) && defined(HAVE_POSIX_HOST)
ssize_t FIX_SYMBOL(sendfile)(int out_fd, int in_fd,
                             off_t *offset, size_t nbytes)
{
#ifdef HAVE_FREEBSD_SENDFILE
 ssize_t ret = 0;

 /* don't get caught by the #define sendfile ... butcher all the arguments */
 ret = (sendfile)(in_fd, out_fd, offset ? *offset : 0, nbytes, NULL,
                  offset, 0);
 return (ret);
#else
 errno = EINVAL; /* pretend the in_fd isn't in the page cache in Linux
                  * as they have to fall back to read()/write() in that case
                  * anyway */
 return (-1);
#endif
}
#endif

#if !defined(HAVE_WCSNRTOMBS) && USE_WIDE_CHAR_T
#include <wchar.h>
size_t FIX_SYMBOL(wcsnrtombs)(char *dest, const wchar_t **src, size_t nwc,
                              size_t len, mbstate_t *ps)
{
  static mbstate_t loc_state;
  size_t ret = 0;
  int do_write = TRUE;

  if (!ps)
    ps = &loc_state;

  if (!nwc)
    return (0);

  while (TRUE)
  {
    static mbstate_t tmp_state;
    size_t tmp_len = 0;

    tmp_state = *ps;
    tmp_len = wcrtomb(NULL, **src, ps);
    *ps = tmp_state;

    if (tmp_len == (size_t)-1)
      return (tmp_len);

    if (tmp_len > len)
      do_write = FALSE;

    if (do_write)
    {
      size_t tmp = wcrtomb(dest, **src, ps);

      assert(tmp == tmp_len);
      len -= tmp;
      dest += tmp;
    }

    if (!**src)
    {
      *src = NULL;
      break;
    }

    ret += tmp_len;

    if (!--nwc)
      break;

    ++*src;
  }

  return (ret);
}
#endif
