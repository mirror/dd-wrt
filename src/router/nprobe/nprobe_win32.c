/*
 *  Copyright (C) 2002-04 Luca Deri <deri@ntop.org>
 *
 *  			  http://www.ntop.org/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
	http://www.winprog.org/tutorial/
	http://www.informit.com/articles/printerfriendly.asp?p=342886
	ftp://sources.redhat.com/pub/pthreads-win32/

*/

#include "nprobe.h"

/* ****************************************************** */

#ifndef __GNUC__
#define EPOCHFILETIME (116444736000000000i64)
#else
#define EPOCHFILETIME (116444736000000000LL)
#endif

struct timezone {
    int tz_minuteswest; /* minutes W of Greenwich */
    int tz_dsttime;     /* type of dst correction */
};

#if 0
int gettimeofday(struct timeval *tv, void *notUsed) {
  tv->tv_sec = time(NULL);
  tv->tv_usec = 0;
  return(0);
}
#endif

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    FILETIME        ft;
    LARGE_INTEGER   li;
    __int64         t;
    static int      tzflag;

    if (tv)
    {
        GetSystemTimeAsFileTime(&ft);
        li.LowPart  = ft.dwLowDateTime;
        li.HighPart = ft.dwHighDateTime;
        t  = li.QuadPart;       /* In 100-nanosecond intervals */
        t -= EPOCHFILETIME;     /* Offset to the Epoch time */
        t /= 10;                /* In microseconds */
        tv->tv_sec  = (long)(t / 1000000);
        tv->tv_usec = (long)(t % 1000000);
    }

    if (tz)
    {
        if (!tzflag)
        {
            _tzset();
            tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }

    return 0;
}


/* ****************************************************** */

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

int
inet_aton(const char *cp, struct in_addr *addr)
{
  addr->s_addr = inet_addr(cp);
  return (addr->s_addr == INADDR_NONE) ? 0 : 1;
}

/* ******************************************************* */

/* **************************************

WIN32 MULTITHREAD STUFF

************************************** */

int pthread_create(pthread_t *threadId, void* notUsed, void *(*__start_routine) (void *), char* userParm) {
  DWORD dwThreadId, dwThrdParam = 1;

  (*threadId) = CreateThread(NULL, /* no security attributes */
			     0,            /* use default stack size */
			     (LPTHREAD_START_ROUTINE)__start_routine, /* thread function */
			     userParm,     /* argument to thread function */
			     0,            /* use default creation flags */
			     &dwThreadId); /* returns the thread identifier */

  if(*threadId != NULL)
    return(1);
  else
    return(0);
}

/* ************************************ */

void pthread_detach(pthread_t *threadId) {
  CloseHandle((HANDLE)*threadId);
}

/* ************************************ */

int pthread_join (pthread_t threadId, void **_value_ptr) {
	LPDWORD value_ptr;
	DWORD my_value_ptr;

	if(_value_ptr != NULL) 
		value_ptr = (LPDWORD)_value_ptr;
	else
		value_ptr = (LPDWORD)&my_value_ptr;
	return(GetExitCodeThread((HANDLE)threadId, value_ptr));
}

/* ************************************ */

int pthread_mutex_init(pthread_mutex_t *mutex, char* notused) {
  (*mutex) = CreateMutex(NULL, FALSE, NULL);
  return(0);
}

/* ************************************ */

void pthread_mutex_destroy(pthread_mutex_t *mutex) {
  ReleaseMutex(*mutex);
  CloseHandle(*mutex);
}

/* ************************************ */

int pthread_mutex_lock(pthread_mutex_t *mutex) {

  if(*mutex == NULL) 
		printf("Error\n");
  WaitForSingleObject(*mutex, INFINITE);
  return(0);
}

/* ************************************ */

int pthread_mutex_trylock(pthread_mutex_t *mutex) {
  if(WaitForSingleObject(*mutex, 0) == WAIT_FAILED)
    return(1);
  else
    return(0);
}

/* ************************************ */

int pthread_mutex_unlock(pthread_mutex_t *mutex) {
   if(*mutex == NULL)
		printf("Error\n");
   return(!ReleaseMutex(*mutex));
}

/* Reentrant string tokenizer.  Generic version.

Slightly modified from: glibc 2.1.3

Copyright (C) 1991, 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
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

char *strtok_r(char *s, const char *delim, char **save_ptr) {
  char *token;

  if (s == NULL)
    s = *save_ptr;

  /* Scan leading delimiters.  */
  s += strspn (s, delim);
  if (*s == '\0')
    return NULL;

  /* Find the end of the token.  */
  token = s;
  s = strpbrk (token, delim);
  if (s == NULL)
    /* This token finishes the string.  */
    *save_ptr = "";
  else {
    /* Terminate the token and make *SAVE_PTR point past it.  */
    *s = '\0';
    *save_ptr = s + 1;
  }

  return token;
}

/* ******************************** */


void revertSlash(char *str, int mode) {
  int i;

  for(i=0; str[i] != '\0'; i++)
    switch(mode) {
    case 0:
      if(str[i] == '/') str[i] = '\\';
      //else if(str[i] == ' ') str[i] = '_';
      break;
    case 1:
      if(str[i] == '\\') str[i] = '/';
      break;
    }
}

char* printAvailableInterfaces(int index) {
  char ebuf[PCAP_ERRBUF_SIZE];
  char *tmpDev = pcap_lookupdev(ebuf), *ifName;
  int ifIdx=0, defaultIdx = -1, numInterfaces = 0;
  u_int i;
  char intNames[32][256];

  if(tmpDev == NULL) {
    traceEvent(TRACE_INFO, "Unable to locate default interface (%s)", ebuf);
    exit(-1);
  }

  ifName = tmpDev;

  if(index == -1) printf("\n\nAvailable interfaces:\n");

  if(!isWinNT()) {
    for(i=0;; i++) {
      if(tmpDev[i] == 0) {
	if(ifName[0] == '\0')
	  break;
	else {
	  if(index == -1) {
	    numInterfaces++;
	    printf("\t[index=%d] '%s'\n", ifIdx, ifName);
	  }

	  if(ifIdx < 32) {
	    strcpy(intNames[ifIdx], ifName);
	    if(defaultIdx == -1) {
	      if(strncmp(intNames[ifIdx], "PPP", 3) /* Avoid to use the PPP interface */
		 && strncmp(intNames[ifIdx], "ICSHARE", 6)) {
		/* Avoid to use the internet sharing interface */
		defaultIdx = ifIdx;
	      }
	    }
	  }
	  ifIdx++;
	  ifName = &tmpDev[i+1];
	}
      }
    }

    tmpDev = intNames[defaultIdx];
  } else {
    /* WinNT/2K */
    static char tmpString[128];
    int i, j,ifDescrPos = 0;
    unsigned short *ifName; /* UNICODE */
    char *ifDescr;

    ifName = (unsigned short *)tmpDev;

    while(*(ifName+ifDescrPos) || *(ifName+ifDescrPos-1))
      ifDescrPos++;
    ifDescrPos++;	/* Step over the extra '\0' */
    ifDescr = (char*)(ifName + ifDescrPos); /* cast *after* addition */

    while(tmpDev[0] != '\0') {
		u_char skipInterface;

      for(j=0, i=0; !((tmpDev[i] == 0) && (tmpDev[i+1] == 0)); i++) {
	if(tmpDev[i] != 0)
	  tmpString[j++] = tmpDev[i];
      }

      tmpString[j++] = 0;

  	  if(strstr(ifDescr, "NdisWan"))
		skipInterface = 1;
	  else
		  skipInterface = 0;

      if(index == -1) {
		  if(!skipInterface) {
			printf("\t[index=%d] '%s'\n", ifIdx, ifDescr);
	     	numInterfaces++;
	     }
      }

		ifDescr += strlen(ifDescr)+1;

      tmpDev = &tmpDev[i+3];
       if(!skipInterface) {
			strcpy(intNames[ifIdx++], tmpString);
			defaultIdx = 0;
	   }
    }

    if(defaultIdx != -1)
      tmpDev = intNames[defaultIdx]; /* Default */
  }

  if(index == -1) {
    if(numInterfaces == 0) {
      traceEvent(TRACE_WARNING, "no interfaces available! This application cannot");
      traceEvent(TRACE_WARNING, "work make sure that winpcap is installed properly");
      traceEvent(TRACE_WARNING, "and that you have network interfaces installed.");
    }
    return(NULL);
  } else if((index < 0) || (index >= ifIdx)) {
    traceEvent(TRACE_ERROR, "Interface index %d out of range\n", index);
    exit(-1);
  } else
    return(strdup(intNames[index]));
}

