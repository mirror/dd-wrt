/*
 *	BIRD Library -- Generic Shell-Like Pattern Matching (currently only '?' and '*')
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 */

#include "nest/bird.h"
#include "lib/string.h"

#ifndef MATCH_FUNC_NAME
#define MATCH_FUNC_NAME patmatch
#endif

#ifndef Convert
#define Convert(x) x
#endif

int
MATCH_FUNC_NAME(byte *p, byte *s)
{
  while (*p)
    {
      if (*p == '?' && *s)
	p++, s++;
      else if (*p == '*')
	{
	  int z = p[1];

	  if (!z)
	    return 1;
	  if (z == '\\' && p[2])
	    z = p[2];
	  z = Convert(z);
	  for(;;)
	    {
	      while (*s && Convert(*s) != z)
		s++;
	      if (!*s)
		return 0;
	      if (MATCH_FUNC_NAME(p+1, s))
		return 1;
	      s++;
	    }
	}
      else
	{
	  if (*p == '\\' && p[1])
	    p++;
	  if (Convert(*p++) != Convert(*s++))
	    return 0;
	}
    }
  return !*s;
}

#if 0
/**
 * patmatch - match shell-like patterns
 * @p: pattern
 * @s: string
 *
 * patmatch() returns whether given string @s matches the given shell-like
 * pattern @p. The patterns consist of characters (which are matched literally),
 * question marks which match any single character, asterisks which match any
 * (possibly empty) string of characters and backslashes which are used to
 * escape any special characters and force them to be treated literally.
 *
 * The matching process is not optimized with respect to time, so please
 * avoid using this function for complex patterns.
 */
int
patmatch(byte *p, byte *s)
{ DUMMY; }
#endif
