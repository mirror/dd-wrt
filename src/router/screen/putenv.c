/* Copyright (c) 2008, 2009
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 *      Micah Cowan (micah@cowan.name)
 *      Sadrul Habib Chowdhury (sadrul@users.sourceforge.net)
 * Copyright (c) 1993-2002, 2003, 2005, 2006, 2007
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 * Copyright (c) 1987 Oliver Laumann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, see
 * https://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 */

/*
 *  putenv  --  put value into environment
 *
 *  Usage:  i = putenv (string)
 *    int i;
 *    char  *string;
 *
 *  where string is of the form <name>=<value>.
 *  If "value" is 0, then "name" will be deleted from the environment.
 *  Putenv returns 0 normally, -1 on error (not enough core for malloc).
 *
 *  Putenv may need to add a new name into the environment, or to
 *  associate a value longer than the current value with a particular
 *  name.  So, to make life simpler, putenv() copies your entire
 *  environment into the heap (i.e. malloc()) from the stack
 *  (i.e. where it resides when your process is initiated) the first
 *  time you call it.
 *
 *  HISTORY
 *  3-Sep-91 Michael Schroeder (mlschroe). Modified to behave as
 *    as putenv.
 * 16-Aug-91 Tim MacKenzie (tym) at Monash University. Modified for
 *    use in screen (iScreen) (ignores final int parameter)
 * 14-Oct-85 Michael Mauldin (mlm) at Carnegie-Mellon University
 *      Ripped out of CMU lib for Rob-O-Matic portability
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *    Created for VAX.  Too bad Bell Labs didn't provide this.  It's
 *    unfortunate that you have to copy the whole environment onto the
 *    heap, but the bookkeeping-and-not-so-much-copying approach turns
 *    out to be much hairier.  So, I decided to do the simple thing,
 *    copying the entire environment onto the heap the first time you
 *    call putenv(), then doing realloc() uniformly later on.
 */

#include "config.h"

#ifdef NEEDPUTENV

#if defined(__STDC__)
# define __P(a) a
#else
# define __P(a) ()
#endif

char  *malloc __P((int));
char  *realloc __P((char *, int));
void   free __P((char *));
int    sprintf __P((char *, char *, ...));

#define EXTRASIZE 5		/* increment to add to env. size */

static int  envsize = -1;	/* current size of environment */
extern char **environ;		/* the global which is your env. */

static int  findenv __P((char *));  /* look for a name in the env. */
static int  newenv __P((void));     /* copy env. from stack to heap */
static int  moreenv __P((void));    /* incr. size of env. */

int
unsetenv(name)
char *name;
{
  register int i;
  
  if (envsize < 0)
    {				/* first time putenv called */
      if (newenv() < 0)		/* copy env. to heap */
	return -1;
    }
  i = findenv(name);
  if (i < 0)
    return 0;			/* Already here */
  
  free(environ[i]);
  if (envsize > 0)
    envsize--;
  for (; environ[i]; i++)
    environ[i] = environ[i+1];
  return 0;			/* Already here */
}

int
putenv(string)
char *string;
{ 
  register int  i;
  register char *p;
  
  if (envsize < 0)
    {				/* first time putenv called */
      if (newenv() < 0)		/* copy env. to heap */
	return -1;
    }
  
  i = findenv(string);		/* look for name in environment */

  if (i < 0)
    {			/* name must be added */
      for (i = 0; environ[i]; i++);
      if (i >= (envsize - 1))
	{			/* need new slot */
	  if (moreenv() < 0)
	    return -1;
	}
      p = malloc(strlen(string) + 1);
      if (p == 0)		/* not enough core */
	return -1;
      environ[i + 1] = 0;	/* new end of env. */
    }
  else
    {			/* name already in env. */
      p = realloc(environ[i], strlen(string) + 1);
      if (p == 0)
	return -1;
    }
  sprintf(p, "%s", string); /* copy into env. */
  environ[i] = p;
  
  return 0;
}

static int
findenv(name)
char *name;
{
  register char *namechar, *envchar;
  register int  i, found;
  
  found = 0;
  for (i = 0; environ[i] && !found; i++)
    { 
      envchar = environ[i];
      namechar = name;
      while (*namechar && *namechar != '=' && (*namechar == *envchar))
        { 
	  namechar++;
	  envchar++;
        }
      found = ((*namechar == '\0' || *namechar == '=') && *envchar == '=');
    }
  return found ? i - 1 : -1;
}

static int
newenv()
{ 
  register char **env, *elem;
  register int i, esize;

  for (i = 0; environ[i]; i++)
    ;
  esize = i + EXTRASIZE + 1;
  env = (char **)malloc(esize * sizeof (elem));
  if (env == 0)
    return -1;

  for (i = 0; environ[i]; i++)
    { 
      elem = malloc(strlen(environ[i]) + 1);
      if (elem == 0)
	return -1;
      env[i] = elem;
      strcpy(elem, environ[i]);
    }
   
  env[i] = 0;
  environ = env;
  envsize = esize;
  return 0;
}

static int
moreenv()
{ 
  register int  esize;
  register char **env;
  
  esize = envsize + EXTRASIZE;
  env = (char **)realloc((char *)environ, esize * sizeof (*env));
  if (env == 0)
    return -1;
  environ = env;
  envsize = esize;
  return 0;
}

#endif /* NEEDPUTENV */

