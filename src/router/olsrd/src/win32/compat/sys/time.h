/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#ifdef _WIN32

#if !defined(MINGW_VERSION) || MINGW_VERSION < 40600

#if !defined TL_SYS_TIME_H_INCLUDED

#define TL_SYS_TIME_H_INCLUDED

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#undef interface

#define timeradd(x, y, z)                       \
  do                                            \
  {                                             \
    (z)->tv_sec = (x)->tv_sec + (y)->tv_sec;    \
                                                \
    (z)->tv_usec = (x)->tv_usec + (y)->tv_usec; \
                                                \
    if ((z)->tv_usec >= 1000000)                \
    {                                           \
      (z)->tv_sec++;                            \
      (z)->tv_usec -= 1000000;                  \
    }                                           \
  }                                             \
  while (0)

#define timersub(x, y, z)                       \
  do                                            \
  {                                             \
    (z)->tv_sec = (x)->tv_sec - (y)->tv_sec;    \
                                                \
    (z)->tv_usec = (x)->tv_usec - (y)->tv_usec; \
                                                \
    if ((z)->tv_usec < 0)                       \
    {                                           \
      (z)->tv_sec--;                            \
      (z)->tv_usec += 1000000;                  \
    }                                           \
  }                                             \
  while (0)

#if !defined WINCE

#ifndef _TIMESPEC_DEFINED
#define _TIMESPEC_DEFINED
struct timespec {
  unsigned int tv_sec;
  unsigned int tv_nsec;
};
#endif /* _TIMESPEC_DEFINED */
#else /* !defined WINCE */
#include <time.h>
#endif /* !defined WINCE */

int nanosleep(struct timespec *Req, struct timespec *Rem);

int gettimeofday(struct timeval *TVal, void *TZone);

#endif /* !defined TL_SYS_TIME_H_INCLUDED */

#endif /* !defined(MINGW_VERSION) || MINGW_VERSION < 40600 */

#endif /* _WIN32 */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
