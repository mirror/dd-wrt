
/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 * Copyright (c) 2004, Thomas Lopatic (thomas@lopatic.de)
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

/*
 * Copyright (c) 1996,1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#include <unistd.h>
#include <sys/time.h>
#include <sys/times.h>
#include <ctype.h>
#include <dlfcn.h>
#include <io.h>
#include <arpa/inet.h>

#include "defs.h"

void PError(char *Str);
void WinSockPError(char *Str);

void
sleep(unsigned int Sec)
{
  Sleep(Sec * 1000);
}

static unsigned int RandState;

void
srandom(unsigned int Seed)
{
  RandState = Seed;
}

unsigned int
random(void)
{
  RandState = RandState * 1103515245 + 12345;

  return (RandState ^ (RandState >> 16)) & RAND_MAX;
}

int
getpid(void)
{
  HANDLE h = GetCurrentThread();
  return (int)h;
}

int
nanosleep(struct timespec *Req, struct timespec *Rem)
{
  Sleep(Req->tv_sec * 1000 + Req->tv_nsec / 1000000);

  Rem->tv_sec = 0;
  Rem->tv_nsec = 0;

  return 0;
}

int
gettimeofday(struct timeval *TVal, void *TZone __attribute__ ((unused)))
{
  SYSTEMTIME SysTime;
  FILETIME FileTime;
  unsigned __int64 Ticks;

  GetSystemTime(&SysTime);
  SystemTimeToFileTime(&SysTime, &FileTime);

  Ticks = ((__int64) FileTime.dwHighDateTime << 32) | (__int64) FileTime.dwLowDateTime;

  Ticks -= 116444736000000000LL;

  TVal->tv_sec = (unsigned int)(Ticks / 10000000);
  TVal->tv_usec = (unsigned int)(Ticks % 10000000) / 10;
  return 0;
}

long
times(struct tms *Dummy __attribute__ ((unused)))
{
  return (long)GetTickCount();
}

int
inet_aton(const char *AddrStr, struct in_addr *Addr)
{
  Addr->s_addr = inet_addr(AddrStr);

  return 1;
}

char *
StrError(unsigned int ErrNo)
{
  static char Msg[1000];

#if !defined WINCE
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, ErrNo, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), Msg, sizeof(Msg), NULL);
#else
  short WideMsg[1000];

  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, ErrNo, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), WideMsg, sizeof(WideMsg) / 2,
                NULL);

  if (WideCharToMultiByte(CP_ACP, 0, WideMsg, -1, Msg, sizeof(Msg), NULL, NULL) == 0)
    strscpy(Msg, "[cannot convert string]", sizeof(Msg));
#endif

  return Msg;
}

void
PError(char *Str)
{
  fprintf(stderr, "ERROR - %s: %s", Str, StrError(GetLastError()));
}

void
WinSockPError(char *Str)
{
  fprintf(stderr, "ERROR - %s: %s", Str, StrError(WSAGetLastError()));
}

// XXX - not thread-safe, which is okay for our purposes

void *
dlopen(char *Name, int Flags __attribute__ ((unused)))
{
#if !defined WINCE
  return (void *)LoadLibrary(Name);
#else
  short WideName[1000];

  MultiByteToWideChar(CP_ACP, 0, Name, -1, WideName, sizeof(WideName));
  return (void *)LoadLibrary(WideName);
#endif
}

int
dlclose(void *Handle)
{
  FreeLibrary((HMODULE) Handle);
  return 0;
}

void *
dlsym(void *Handle, const char *Name)
{
#if !defined WINCE
  return GetProcAddress((HMODULE) Handle, Name);
#else
  short WideName[1000];

  MultiByteToWideChar(CP_ACP, 0, Name, -1, WideName, sizeof(WideName));
  return GetProcAddress((HMODULE) Handle, WideName);
#endif
}

char *
dlerror(void)
{
  return StrError(GetLastError());
}

#define NS_INADDRSZ 4
#define NS_IN6ADDRSZ 16
#define NS_INT16SZ 2

static int
inet_pton4(const char *src, unsigned char *dst)
{
  int saw_digit, octets, ch;
  u_char tmp[NS_INADDRSZ], *tp;

  saw_digit = 0;
  octets = 0;
  *(tp = tmp) = 0;

  while ((ch = *src++) != '\0') {
    if (ch >= '0' && ch <= '9') {
      unsigned int new = *tp * 10 + (ch - '0');

      if (new > 255)
        return (0);

      *tp = new;

      if (!saw_digit) {
        if (++octets > 4)
          return (0);

        saw_digit = 1;
      }
    }

    else if (ch == '.' && saw_digit) {
      if (octets == 4)
        return (0);

      *++tp = 0;

      saw_digit = 0;
    }

    else
      return (0);
  }

  if (octets < 4)
    return (0);

  memcpy(dst, tmp, NS_INADDRSZ);
  return (1);
}

static int
inet_pton6(const char *src, unsigned char *dst)
{
  static const char xdigits[] = "0123456789abcdef";
  u_char tmp[NS_IN6ADDRSZ], *tp, *endp, *colonp;
  const char *curtok;
  int ch, saw_xdigit;
  u_int val;

  tp = memset(tmp, '\0', NS_IN6ADDRSZ);
  endp = tp + NS_IN6ADDRSZ;
  colonp = NULL;

  if (*src == ':')
    if (*++src != ':')
      return (0);

  curtok = src;
  saw_xdigit = 0;
  val = 0;

  while ((ch = tolower(*src++)) != '\0') {
    const char *pch;

    pch = strchr(xdigits, ch);

    if (pch != NULL) {
      val <<= 4;
      val |= (pch - xdigits);

      if (val > 0xffff)
        return (0);

      saw_xdigit = 1;
      continue;
    }

    if (ch == ':') {
      curtok = src;

      if (!saw_xdigit) {
        if (colonp)
          return (0);

        colonp = tp;
        continue;
      }

      else if (*src == '\0') {
        return (0);
      }

      if (tp + NS_INT16SZ > endp)
        return (0);

      *tp++ = (u_char) (val >> 8) & 0xff;
      *tp++ = (u_char) val & 0xff;
      saw_xdigit = 0;
      val = 0;
      continue;
    }

    if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) && inet_pton4(curtok, tp) > 0) {
      tp += NS_INADDRSZ;
      saw_xdigit = 0;
      break;
    }

    return (0);
  }

  if (saw_xdigit) {
    if (tp + NS_INT16SZ > endp)
      return (0);

    *tp++ = (u_char) (val >> 8) & 0xff;
    *tp++ = (u_char) val & 0xff;
  }

  if (colonp != NULL) {
    const int n = tp - colonp;
    int i;

    if (tp == endp)
      return (0);

    for (i = 1; i <= n; i++) {
      endp[-i] = colonp[n - i];
      colonp[n - i] = 0;
    }

    tp = endp;
  }

  if (tp != endp)
    return (0);

  memcpy(dst, tmp, NS_IN6ADDRSZ);
  return (1);
}

int
inet_pton(int af, const char *src, void *dst)
{
  switch (af) {
  case AF_INET:
    return (inet_pton4(src, dst));

  case AF_INET6:
    return (inet_pton6(src, dst));

  default:
    return -1;
  }
}

static char *
inet_ntop4(const unsigned char *src, char *dst, int size)
{
  static const char fmt[] = "%u.%u.%u.%u";
  char tmp[sizeof "255.255.255.255"];

  if (sprintf(tmp, fmt, src[0], src[1], src[2], src[3]) > size)
    return (NULL);

  return strscpy(dst, tmp, size);
}

static char *
inet_ntop6(const unsigned char *src, char *dst, int size)
{
  char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"], *tp;
  struct {
    int base, len;
  } best, cur;
  u_int words[NS_IN6ADDRSZ / NS_INT16SZ];
  int i;

  memset(words, '\0', sizeof words);

  for (i = 0; i < NS_IN6ADDRSZ; i += 2)
    words[i / 2] = (src[i] << 8) | src[i + 1];

  best.base = -1;
  cur.base = -1;

  for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
    if (words[i] == 0) {
      if (cur.base == -1)
        cur.base = i, cur.len = 1;

      else
        cur.len++;
    }

    else {
      if (cur.base != -1) {
        if (best.base == -1 || cur.len > best.len)
          best = cur;

        cur.base = -1;
      }
    }
  }

  if (cur.base != -1) {
    if (best.base == -1 || cur.len > best.len)
      best = cur;
  }

  if (best.base != -1 && best.len < 2)
    best.base = -1;

  tp = tmp;

  for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
    if (best.base != -1 && i >= best.base && i < (best.base + best.len)) {
      if (i == best.base)
        *tp++ = ':';

      continue;
    }

    if (i != 0)
      *tp++ = ':';

    if (i == 6 && best.base == 0 && (best.len == 6 || (best.len == 5 && words[5] == 0xffff))) {
      if (!inet_ntop4(src + 12, tp, sizeof tmp - (tp - tmp)))
        return (NULL);

      tp += strlen(tp);

      break;
    }

    tp += sprintf(tp, "%x", words[i]);
  }

  if (best.base != -1 && (best.base + best.len) == (NS_IN6ADDRSZ / NS_INT16SZ))
    *tp++ = ':';

  *tp++ = '\0';

  if ((tp - tmp) > size)
    return (NULL);

  return strscpy(dst, tmp, size);
}

char *
inet_ntop(int af, const void *src, char *dst, int size)
{
  switch (af) {
  case AF_INET:
    return (inet_ntop4(src, dst, size));

  case AF_INET6:
    return (inet_ntop6(src, dst, size));

  default:
    return (NULL);
  }
}

int
isatty(int fd)
{
#if !defined WINCE
  HANDLE Hand;
  CONSOLE_SCREEN_BUFFER_INFO Info;
  unsigned long Events;

  if (fd == 0) {
    Hand = GetStdHandle(STD_INPUT_HANDLE);
    return GetNumberOfConsoleInputEvents(Hand, &Events);
  }

  else if (fd == 1) {
    Hand = GetStdHandle(STD_OUTPUT_HANDLE);
    return GetConsoleScreenBufferInfo(Hand, &Info);
  }

  else if (fd == 2) {
    Hand = GetStdHandle(STD_ERROR_HANDLE);
    return GetConsoleScreenBufferInfo(Hand, &Info);
  }

  return -1;
#else
  return 0;
#endif
}

#define CHUNK_SIZE 512

/* and we emulate a real write(2) syscall using send() */
int
write(int fd, const void *buf, unsigned int count)
{
  size_t written = 0;
  while (written < count) {
    ssize_t rc = send(fd, (const char *)buf + written,
                      min(count - written, CHUNK_SIZE), 0);
    if (rc <= 0) {
      break;
    }
    written += rc;
  }
  return written;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
