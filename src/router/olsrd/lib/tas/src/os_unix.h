
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

#include <netinet/in.h>

struct ipAddr {
  int domain;

  union {
    struct in_addr v4;
    struct in6_addr v6;
  } addr;
};

struct fileId {
  int fileDesc;
};

struct timeStamp {
  unsigned int time;
};

extern void getRandomBytes(unsigned char *buff, int len);

extern int addrLen(int family);

extern void os_now(struct timeStamp *timeStamp);
extern int timedOut(struct timeStamp *timeStamp, int sec);

extern unsigned int getMicro(void);

extern void *allocMem(int len);
extern void freeMem(void *mem);

extern int writeFileOs(const struct fileId *sockId, const unsigned char *data, int len);
extern int readFileOs(const struct fileId *sockId, unsigned char *data, int len);
extern int checkAbsPath(const char *path);
extern char *fullPath(const char *dir, const char *path);
extern void setExtension(char *res, const char *path, const char *ext);
extern int isDirectory(const char *rootDir, const char *path);
extern int openFile(struct fileId *fileId, const char *rootDir, const char *path);
extern void closeFile(const struct fileId *sockId);
extern int fileIsNewer(const char *fileName1, const char *fileName2);
extern int createAllDirs(char *path);

extern int parseIpAddr(struct ipAddr *addr, const char *addrStr);
extern char *ipAddrToString(struct ipAddr *addr);
extern char *rawIpAddrToString(void *rawAddr, int len);
extern int createMainSocket(const struct ipAddr *addr, int port);
extern int acceptConn(struct fileId **sockId, struct ipAddr **addr);
extern void closeMainSocket(void);
extern int waitForSockets(struct fileId *sockIds[], int *flags[], int num);

#endif

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
