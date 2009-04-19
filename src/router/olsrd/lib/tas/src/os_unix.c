
/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * Copyright (c) 2004, Thomas Lopatic (thomas@olsr.org)
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

#if defined linux

#include "link.h"
#include "plugin.h"
#include "lib.h"
#include "os_unix.h"
#include "http.h"
#include "glua.h"
#include "glua_ext.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

static int mainSocket;

void
getRandomBytes(unsigned char *buff, int len)
{
  int file;
  int readLen;

  memset(buff, 0, len);

  file = open("/dev/random", O_RDONLY);

  if (file < 0) {
    fprintf(stderr, "warning: cannot open /dev/random\n");
    return;
  }

  while (len > 0) {
    readLen = read(file, buff, len);

    if (readLen < 0) {
      fprintf(stderr, "warning: cannot read from /dev/random\n");
      close(file);
      return;
    }

    buff += readLen;
    len -= readLen;
  }

  close(file);
}

int
addrLen(int family)
{
  return (family == AF_INET) ? sizeof(struct in_addr) : sizeof(struct in6_addr);
}

void
os_now(struct timeStamp *timeStamp)
{
  timeStamp->time = time(NULL);
}

int
timedOut(struct timeStamp *timeStamp, int sec)
{
  time_t now;

  time(&now);

  if ((time_t) (timeStamp->time + sec) > now)
    return -1;

  return 0;
}

unsigned int
getMicro(void)
{
  struct timeval timeVal;
  static struct timeval timeValPrev;
  static int firstTime = 1;

  gettimeofday(&timeVal, NULL);

  if (firstTime == 0 && timeValPrev.tv_sec == timeVal.tv_sec && timeValPrev.tv_usec >= timeVal.tv_usec)
    return timeValPrev.tv_sec * 1000000 + timeValPrev.tv_usec;

  firstTime = 0;

  timeValPrev.tv_sec = timeVal.tv_sec;
  timeValPrev.tv_usec = timeVal.tv_usec;

  return timeVal.tv_sec * 1000000 + timeVal.tv_usec;
}

void *
allocMem(int len)
{
  void *res;

  res = malloc(len);

  if (res == NULL) {
    fprintf(stderr, "cannot allocate %d bytes\n", len);
    exit(0);
  }

  memset(res, 0, len);

  return res;
}

void
freeMem(void *mem)
{
  free(mem);
}

int
writeFileOs(const struct fileId *fileId, const unsigned char *data, int len)
{
  int writeLen;

  if (len == 0)
    return 0;

  do
    writeLen = write(fileId->fileDesc, data, len);
  while (writeLen < 0 && errno == EINTR);

  if (writeLen < 0) {
    if (errno == EAGAIN)
      return 0;

    error("cannot write to file descriptor: %s\n", strerror(errno));
    return -1;
  }

  return writeLen;
}

int
readFileOs(const struct fileId *fileId, unsigned char *data, int len)
{
  int readLen;

  if (len == 0)
    return 0;

  do
    readLen = read(fileId->fileDesc, data, len);
  while (readLen < 0 && errno == EINTR);

  if (readLen < 0) {
    if (errno == EAGAIN)
      return 0;

    error("cannot read from file descriptor: %s\n", strerror(errno));
    return -1;
  }

  if (readLen == 0)
    return -1;

  return readLen;
}

int
checkAbsPath(const char *path)
{
  if (path[0] != '/')
    return -1;

  return 0;
}

char *
fullPath(const char *dir, const char *path)
{
  int dirLen = strlen(dir);
  int pathLen = strlen(path);
  char *buff = allocMem(dirLen + pathLen + 2);

  memcpy(buff, dir, dirLen);

  if (dirLen == 0 || buff[dirLen - 1] == '/')
    dirLen--;

  else
    buff[dirLen] = '/';

  memcpy(buff + dirLen + 1, path, pathLen + 1);

  return buff;
}

void
setExtension(char *res, const char *path, const char *ext)
{
  int i;
  int len = strlen(path);

  for (i = len - 1; i >= 0 && path[i] != '.' && path[i] != '/'; i--);

  if (path[i] == '.')
    len = i;

  memcpy(res, path, len);
  memcpy(res + len, ext, strlen(ext) + 1);
}

int
isDirectory(const char *rootDir, const char *path)
{
  char *full = fullPath(rootDir, path);
  struct stat statBuff;
  int res;

  res = stat(full, &statBuff);

  freeMem(full);

  if (res < 0)
    return -1;

  return S_ISDIR(statBuff.st_mode);
}

int
openFile(struct fileId *fileId, const char *rootDir, const char *path)
{
  int fileDesc;
  char *full = fullPath(rootDir, path);

  fileDesc = open(full, O_RDONLY | O_NONBLOCK);

  if (fileDesc < 0) {
    error("cannot open file %s: %s\n", full, strerror(errno));
    freeMem(full);
    return -1;
  }

  fileId->fileDesc = fileDesc;

  freeMem(full);
  return 0;
}

void
closeFile(const struct fileId *fileId)
{
  close(fileId->fileDesc);
}

int
fileIsNewer(const char *fileName1, const char *fileName2)
{
  struct stat stat1, stat2;

  if (stat(fileName1, &stat1) < 0) {
    error("cannot stat %s: %s\n", fileName1, strerror(errno));
    return -1;
  }

  if (stat(fileName2, &stat2) < 0) {
    if (errno != ENOENT)
      error("cannot stat %s: %s\n", fileName2, strerror(errno));

    return -1;
  }

  return stat1.st_mtime > stat2.st_mtime;
}

int
createAllDirs(char *path)
{
  int i;
  int fail;

  for (i = 0; path[i] != 0; i++) {
    if (path[i] == '/' && i > 0) {
      path[i] = 0;

      fail = (mkdir(path, 0755) < 0 && errno != EEXIST);

      path[i] = '/';

      if (fail)
        return -1;
    }
  }

  return 0;
}

int
parseIpAddr(struct ipAddr *addr, const char *addrStr)
{
  memset(addr, 0, sizeof(struct ipAddr));

  if (inet_pton(AF_INET, addrStr, &addr->addr.v4) > 0) {
    addr->domain = PF_INET;
    return 0;
  }

  if (inet_pton(AF_INET6, addrStr, &addr->addr.v6) > 0) {
    addr->domain = PF_INET6;
    return 0;
  }

  fprintf(stderr, "cannot parse IP address\n");
  return -1;
}

char *
ipAddrToString(struct ipAddr *addr)
{
  static char buff[8][40];
  static int i = 0;
  char *res;

  res = buff[i];

  if (addr->domain == PF_INET)
    inet_ntop(AF_INET, &addr->addr.v4, res, 40);

  else
    inet_ntop(AF_INET6, &addr->addr.v6, res, 40);

  i = (i + 1) & 7;

  return res;
}

char *
rawIpAddrToString(void *rawAddr, int len)
{
  struct ipAddr addr;

  if (len == 4) {
    memcpy(&addr.addr.v4, rawAddr, 4);
    addr.domain = PF_INET;
  }

  else {
    memcpy(&addr.addr.v6, rawAddr, 16);
    addr.domain = PF_INET6;
  }

  return ipAddrToString(&addr);
}

static int
createSockAddr(struct sockaddr *sockAddr, const struct ipAddr *addr, int port)
{
  struct sockaddr_in *sockAddr4;
  struct sockaddr_in6 *sockAddr6;

  memset(sockAddr, 0, sizeof(struct sockaddr));

  if (addr->domain == PF_INET) {
    sockAddr4 = (struct sockaddr_in *)sockAddr;

    sockAddr4->sin_family = AF_INET;
    sockAddr4->sin_port = htons((short)port);
    sockAddr4->sin_addr.s_addr = addr->addr.v4.s_addr;

    return 0;
  }

  if (addr->domain == PF_INET6) {
    sockAddr6 = (struct sockaddr_in6 *)sockAddr;

    sockAddr6->sin6_family = AF_INET6;
    sockAddr6->sin6_port = htons((short)port);
    memcpy(&sockAddr6->sin6_addr, &addr->addr.v6, sizeof(struct in6_addr));

    return 0;
  }

  fprintf(stderr, "invalid protocol family: %d\n", addr->domain);
  return -1;
}

static int
addrFromSockAddr(struct ipAddr *addr, const struct sockaddr *sockAddr)
{
  const struct sockaddr_in *sockAddr4 = (const struct sockaddr_in *)sockAddr;
  const struct sockaddr_in6 *sockAddr6 = (const struct sockaddr_in6 *)sockAddr;

  memset(addr, 0, sizeof(struct ipAddr));

  if (sockAddr4->sin_family == AF_INET) {
    addr->domain = PF_INET;
    addr->addr.v4.s_addr = sockAddr4->sin_addr.s_addr;
    return 0;
  }

  if (sockAddr6->sin6_family == AF_INET6) {
    addr->domain = PF_INET6;
    memcpy(&addr->addr.v6, &sockAddr6->sin6_addr, sizeof(struct in6_addr));
    return 0;
  }

  fprintf(stderr, "invalid address family: %d\n", sockAddr4->sin_family);
  return -1;
}

int
createMainSocket(const struct ipAddr *addr, int port)
{
  struct sockaddr sockAddr;
  static int truePara = 1;
  int flags;

  if (createSockAddr(&sockAddr, addr, port) < 0) {
    fprintf(stderr, "cannot create socket address\n");
    return -1;
  }

  mainSocket = socket(addr->domain, SOCK_STREAM, IPPROTO_TCP);

  if (mainSocket < 0) {
    error("cannot create main socket: %s\n", strerror(errno));
    return -1;
  }

  if (setsockopt(mainSocket, SOL_SOCKET, SO_REUSEADDR, &truePara, sizeof(truePara)) < 0) {
    error("cannot set SO_REUSEADDR socket option: %s\n", strerror(errno));
    close(mainSocket);
    return -1;
  }

  flags = fcntl(mainSocket, F_GETFL);

  if (flags < 0) {
    error("cannot get flags : %s\n", strerror(errno));
    close(mainSocket);
    return -1;
  }

  if (fcntl(mainSocket, F_SETFL, flags | O_NONBLOCK) < 0) {
    error("cannot set flags: %s\n", strerror(errno));
    close(mainSocket);
    return -1;
  }

  if (bind(mainSocket, &sockAddr, sizeof(struct sockaddr)) < 0) {
    error("cannot bind main socket: %s\n", strerror(errno));
    close(mainSocket);
    return -1;
  }

  if (listen(mainSocket, 10) < 0) {
    error("cannot listen on main socket: %s\n", strerror(errno));
    close(mainSocket);
    return -1;
  }

  return 0;
}

int
acceptConn(struct fileId **sockId, struct ipAddr **addr)
{
  struct sockaddr sockAddr;
  socklen_t len;
  int sock;
  int flags;

  do {
    len = sizeof(struct sockaddr);

    sock = accept(mainSocket, &sockAddr, &len);
  }
  while (sock < 0 && errno == EINTR);

  if (sock < 0) {
    if (errno != EAGAIN)
      error("accept failed: %s\n", strerror(errno));

    return -1;
  }

  flags = fcntl(sock, F_GETFL);

  if (flags < 0) {
    error("cannot get flags : %s\n", strerror(errno));
    close(sock);
    return -1;
  }

  if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
    error("cannot set flags: %s\n", strerror(errno));
    close(sock);
    return -1;
  }

  *addr = allocMem(sizeof(struct ipAddr));

  if (addrFromSockAddr(*addr, &sockAddr) < 0) {
    error("cannot convert socket address\n");
    freeMem(addr);
    close(sock);
    return -1;
  }

  *sockId = allocMem(sizeof(struct fileId));

  (*sockId)->fileDesc = sock;

  return 0;
}

void
closeMainSocket(void)
{
  close(mainSocket);
}

int
waitForSockets(struct fileId *sockIds[], int *flags[], int num)
{
  fd_set readSet, writeSet;
  int i;
  int fileDesc;
  int max;
  int res;

  FD_ZERO(&readSet);
  FD_ZERO(&writeSet);

  FD_SET(mainSocket, &readSet);

  max = mainSocket;

  for (i = 0; i < num; i++) {
    fileDesc = sockIds[i]->fileDesc;

    if (fileDesc > max)
      max = fileDesc;

    if ((*flags[i] & FLAG_READ) != 0)
      FD_SET(fileDesc, &readSet);

    if ((*flags[i] & FLAG_WRITE) != 0)
      FD_SET(fileDesc, &writeSet);
  }

  do
    res = select(max + 1, &readSet, &writeSet, NULL, NULL);
  while (res < 0 && errno == EINTR);

  if (res < 0) {
    error("cannot select: %s\n", strerror(errno));
    return -1;
  }

  for (i = 0; i < num; i++) {
    *flags[i] = 0;

    fileDesc = sockIds[i]->fileDesc;

    if (FD_ISSET(fileDesc, &readSet))
      *flags[i] |= FLAG_READ;

    if (FD_ISSET(fileDesc, &writeSet))
      *flags[i] |= FLAG_WRITE;
  }

  return 0;
}

#endif

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
