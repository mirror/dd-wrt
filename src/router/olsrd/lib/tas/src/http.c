
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

#include "link.h"
#include "plugin.h"
#include "lib.h"
#include "os_unix.h"
#include "http.h"
#include "glua.h"
#include "glua_ext.h"

#include <string.h>
#include <stdarg.h>

// #define TAS_BLOCK

#define DEF_CONFIG_ROOT_DIR "/etc/tas"
#define DEF_CONFIG_WORK_DIR "/var/run/tas"
#define DEF_CONFIG_PORT 1979
#define DEF_CONFIG_ADDR "127.0.0.1"
#define DEF_CONFIG_INDEX_FILE "index.html"
#define DEF_CONFIG_USER NULL
#define DEF_CONFIG_PASSWORD NULL
#define DEF_CONFIG_SESS_TIME 600
#define DEF_CONFIG_PUB_DIR "pub"
#define DEF_CONFIG_QUANTUM 30
#define DEF_CONFIG_MESS_TIME 60
#define DEF_CONFIG_MESS_LIMIT 100

static struct ipAddr confAddr;
static int confPort;
static const char *confRootDir;
static const char *confWorkDir;
static const char *confIndexFile;
static char *confUser;
static char *confPassword;
static int confSessTime;
static const char *confPubDir;
static int confQuantum;
static int confMessTime;
static int confMessLimit;

static struct {
  unsigned int sessId;
  unsigned char key[16];
} cookieStruct;

#define MAX_CONN 5

static int numConn;
static struct connInfo *conn[MAX_CONN];

struct sessInfo {
  unsigned int id;
  void *data;
  struct timeStamp time;
};

#define MAX_SESS 10

static int numSess;
static struct sessInfo *sess[MAX_SESS];

static struct extMap {
  const char *ext;
  const char *type;
  int state;
} extMap[] = {
  {
  ".png", "image/png", STATE_FILE}, {
  ".gif", "image/gif", STATE_FILE}, {
  ".jpg", "image/jpg", STATE_FILE}, {
  ".lsp", "text/html; charset=iso-8859-1", STATE_LSP}, {
  ".html", "text/html; charset=iso-8859-1", STATE_FILE}, {
  ".htm", "text/html; charset=iso-8859-1", STATE_FILE}, {
  NULL, NULL, 0}
};

struct tasMessage {
  struct tasMessage *next;

  struct timeStamp time;

  char *service;
  char *string;
  char *from;
};

static struct tasMessage *firstTasMsg, *lastTasMsg;
static int numTasMsg;

static void
rc4(unsigned char *buff, int len, unsigned char *key, int keyLen)
{
  int i, m, n;
  unsigned char state[256];
  unsigned char aux;

  for (i = 0; i < 256; i++)
    state[i] = (unsigned char)i;

  m = 0;
  n = 0;

  for (i = 0; i < 256; i++) {
    m = (m + key[n] + state[i]) & 255;

    aux = state[i];
    state[i] = state[m];
    state[m] = aux;

    n = (n + 1) % keyLen;
  }

  m = 0;
  n = 0;

  for (i = 0; i < len; i++) {
    n = (n + 1) & 255;
    m = (m + state[n]) & 255;

    aux = state[n];
    state[n] = state[m];
    state[m] = aux;

    buff[i] ^= state[(state[m] + state[n]) & 255];
  }
}

static int
mapHexDigit(int digit)
{
  if (digit >= 'A' && digit <= 'F')
    return digit + 10 - 'A';

  if (digit >= 'a' && digit <= 'f')
    return digit + 10 - 'a';

  if (digit >= '0' && digit <= '9')
    return digit - '0';

  return -1;
}

static int
addHexDigit(int *val, int digit)
{
  digit = mapHexDigit(digit);

  if (digit < 0)
    return -1;

  *val = (*val << 4) | digit;

  return 0;
}

static void
encHexString(char *hexString, unsigned char *hex, int len)
{
  static const char map[] = "0123456789ABCDEF";

  while (len-- > 0) {
    *hexString++ = map[*hex >> 4];
    *hexString++ = map[*hex++ & 15];
  }

  *hexString = 0;
}

static int
decHexString(unsigned char *hex, char *hexString, int len)
{
  int val;

  while (len-- > 0) {
    val = 0;

    if (addHexDigit(&val, *hexString++) < 0 || addHexDigit(&val, *hexString++) < 0)
      return -1;

    *hex++ = (unsigned char)val;
  }

  return 0;
}

static int
decBase64(unsigned char *out, char *in)
{
  static int map[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  };
  int state;
  unsigned int val;
  int digit;

  val = 0;
  state = 0;

  while (*in != 0 && *in != '=') {
    digit = map[(unsigned char)*in++];

    if (digit < 0)
      return -1;

    val = (val << 6) | digit;

    if (state == 1)
      *out++ = (unsigned char)(val >> 4);

    else if (state == 2)
      *out++ = (unsigned char)(val >> 2);

    else if (state == 3)
      *out++ = (unsigned char)val;

    state = (state + 1) & 3;
  }

  return 0;
}

static void
initInOutBuff(struct inOutBuff *buff)
{
  buff->off = 0;
  buff->len = 0;
  buff->cont = 0;

  buff->first = NULL;
  buff->last = NULL;
}

static struct connInfo *
newConnInfo(struct fileId *sockId, struct ipAddr *addr)
{
  struct connInfo *info = allocMem(sizeof(struct connInfo));

  info->sockId = sockId;
  info->addr = addr;

  info->state = STATE_REQUEST;

  initInOutBuff(&info->read);
  initInOutBuff(&info->write[0]);
  initInOutBuff(&info->write[1]);
  initInOutBuff(&info->write[2]);

  info->which = 0;

  info->flags = FLAG_READ | FLAG_WRITE;

  info->buff = NULL;

  info->buffUsed = 0;
  info->buffTotal = 0;

  info->firstHead = NULL;
  info->lastHead = NULL;

  info->verb = NULL;
  info->host = NULL;
  info->path = NULL;
  info->para = NULL;
  info->proto = NULL;

  info->contType = "text/html; charset=iso-8859-1";
  info->contLen = -1;

  info->newSess = NULL;

  info->authUser = NULL;
  info->authPass = NULL;

  return info;
}

static void
freeInOutBuff(struct inOutBuff *buff)
{
  struct chunk *walker, *next;

  for (walker = buff->first; walker != NULL; walker = next) {
    next = walker->next;
    freeMem(walker);
  }
}

static void
freeWorkBuff(struct workBuff *buff)
{
  struct workBuff *next;

  while (buff != NULL) {
    next = buff->next;
    freeMem(buff);
    buff = next;
  }
}

static void
freeConnInfo(struct connInfo *info)
{
  freeMem(info->sockId);
  freeMem(info->addr);

  freeInOutBuff(&info->read);
  freeInOutBuff(&info->write[0]);
  freeInOutBuff(&info->write[1]);
  freeInOutBuff(&info->write[2]);

  freeWorkBuff(info->buff);

  freeMem(info);
}

static struct sessInfo *
newSessInfo(void)
{
  static unsigned int sessId = 0;
  struct sessInfo *info;

  info = allocMem(sizeof(struct sessInfo));

  info->id = sessId++;
  info->data = NULL;

  os_now(&info->time);

  debug(DEBUG_SESSION, "new session, id = %u\n", info->id);

  return info;
}

void *
allocBuff(struct connInfo *info, int len)
{
  struct workBuff *buff;
  unsigned char *res;

  debug(DEBUG_CONNECTION, "%d bytes of buffer space requested\n", len);

  if (info->buff != NULL)
    debug(DEBUG_CONNECTION, "existing buffer, size = %d bytes, used = %d bytes, remaining = %d bytes\n", info->buffTotal,
          info->buffUsed, info->buffTotal - info->buffUsed);

  else
    debug(DEBUG_CONNECTION, "no existing buffer\n");

  if (info->buff == NULL || len > info->buffTotal - info->buffUsed) {
    info->buffTotal = (len > BUFF_SIZE) ? len : BUFF_SIZE;
    info->buffUsed = 0;

    debug(DEBUG_CONNECTION, "new buffer of %d bytes\n", info->buffTotal);

    buff = allocMem(sizeof(struct workBuff) + info->buffTotal);

    buff->data = (unsigned char *)(buff + 1);

    buff->next = info->buff;
    info->buff = buff;
  }

  res = info->buff->data + info->buffUsed;

  info->buffUsed += len;

  debug(DEBUG_CONNECTION, "used = %d bytes, remaining = %d bytes\n", info->buffUsed, info->buffTotal - info->buffUsed);

  return res;
}

void
httpInit(void)
{
  parseIpAddr(&confAddr, DEF_CONFIG_ADDR);
  confPort = DEF_CONFIG_PORT;
  confRootDir = DEF_CONFIG_ROOT_DIR;
  confWorkDir = DEF_CONFIG_WORK_DIR;
  confIndexFile = DEF_CONFIG_INDEX_FILE;
  confUser = DEF_CONFIG_USER;
  confPassword = DEF_CONFIG_PASSWORD;
  confSessTime = DEF_CONFIG_SESS_TIME;
  confPubDir = DEF_CONFIG_PUB_DIR;
  confQuantum = DEF_CONFIG_QUANTUM;
  confMessTime = DEF_CONFIG_MESS_TIME;
  confMessLimit = DEF_CONFIG_MESS_LIMIT;

  getRandomBytes(cookieStruct.key, 16);
}

int
httpSetAddress(const char *addrStr, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  if (parseIpAddr(&confAddr, addrStr) < 0) {
    error("invalid IP address: %s\n", addrStr);
    return -1;
  }

  return 0;
}

int
httpSetPort(const char *portStr, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  unsigned int port;

  if (stringToInt(&port, portStr) < 0) {
    error("invalid port number: %s\n", portStr);
    return -1;
  }

  if (port > 65535) {
    error("invalid port number: %u\n", port);
    return -1;
  }

  confPort = port;

  return 0;
}

int
httpSetRootDir(const char *rootDir, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  if (checkAbsPath(rootDir) < 0) {
    error("root directory (%s) requires an absolute path\n", rootDir);
    return -1;
  }

  confRootDir = myStrdup(rootDir);
  return 0;
}

int
httpSetWorkDir(const char *workDir, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  if (checkAbsPath(workDir) < 0) {
    error("work directory (%s) requires an absolute path\n", workDir);
    return -1;
  }

  confWorkDir = myStrdup(workDir);
  return 0;
}

int
httpSetIndexFile(const char *indexFile, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon
                 __attribute__ ((unused)))
{
  confIndexFile = myStrdup(indexFile);
  return 0;
}

int
httpSetUser(const char *user, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  confUser = myStrdup(user);
  return 0;
}

int
httpSetPassword(const char *password, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon
                __attribute__ ((unused)))
{
  confPassword = myStrdup(password);
  return 0;
}

int
httpSetSessTime(const char *timeStr, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  unsigned int time;

  if (stringToInt(&time, timeStr) < 0) {
    error("invalid timeout: %s\n", timeStr);
    return -1;
  }

  if (time > 86400) {
    error("invalid timeout: %u\n", time);
    return -1;
  }

  confSessTime = time;

  return 0;
}

int
httpSetPubDir(const char *pubDir, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  confPubDir = myStrdup(pubDir);
  return 0;
}

int
httpSetQuantum(const char *quantumStr, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon
               __attribute__ ((unused)))
{
  unsigned int quantum;

  if (stringToInt(&quantum, quantumStr) < 0) {
    error("invalid quantum: %s\n", quantumStr);
    return -1;
  }

  if (quantum > 100) {
    error("invalid quantum: %u\n", quantum);
    return -1;
  }

  confQuantum = quantum;

  return 0;
}

int
httpSetMessTime(const char *timeStr, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  unsigned int time;

  if (stringToInt(&time, timeStr) < 0) {
    error("invalid timeout: %s\n", timeStr);
    return -1;
  }

  if (time > 365 * 86400) {
    error("invalid timeout: %u\n", time);
    return -1;
  }

  confMessTime = time;

  return 0;
}

int
httpSetMessLimit(const char *limitStr, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon
                 __attribute__ ((unused)))
{
  unsigned int limit;

  if (stringToInt(&limit, limitStr) < 0) {
    error("invalid limit: %s\n", limitStr);
    return -1;
  }

  if (limit > 1000000) {
    error("invalid limit: %u\n", limit);
    return -1;
  }

  confMessLimit = limit;

  return 0;
}

int
httpSetup(void)
{
  int i;

  if (createMainSocket(&confAddr, confPort) < 0) {
    error("cannot create main socket\n");
    return -1;
  }

  numConn = 0;

  for (i = 0; i < MAX_CONN; i++)
    conn[i] = NULL;

  numSess = 0;

  for (i = 0; i < MAX_SESS; i++)
    sess[i] = NULL;

  firstTasMsg = NULL;
  lastTasMsg = NULL;

  numTasMsg = 0;

  return 0;
}

static int
readConn(struct connInfo *info)
{
  struct inOutBuff *read = &info->read;
  int readLen;
  struct chunk *chunk;

  for (;;) {
    if (read->last == NULL || read->len == CHUNK_SIZE) {
      chunk = allocMem(sizeof(struct chunk));

      chunk->next = NULL;

      if (read->last != NULL)
        read->last->next = chunk;

      read->last = chunk;

      if (read->first == NULL)
        read->first = chunk;

      read->len = 0;
    }

    readLen = readFileOs(info->sockId, &read->last->data[read->len], CHUNK_SIZE - read->len);

    if (readLen < 0) {
      error("cannot read from network connection\n");
      return -1;
    }

    debug(DEBUG_CONNECTION, "read %d bytes from connection\n", readLen);

    if (readLen == 0)
      return 0;

    read->len += readLen;
    read->cont += readLen;
  }
}

void
writeBuff(struct inOutBuff *write, const unsigned char *data, int dataLen)
{
  int writeLen;
  struct chunk *chunk;

  while (dataLen > 0) {
    if (write->last == NULL || write->len == CHUNK_SIZE) {
      chunk = allocMem(sizeof(struct chunk));

      chunk->next = NULL;

      if (write->last != NULL)
        write->last->next = chunk;

      write->last = chunk;

      if (write->first == NULL)
        write->first = chunk;

      write->len = 0;
    }

    writeLen = CHUNK_SIZE - write->len;

    if (dataLen < writeLen)
      writeLen = dataLen;

    memcpy(&write->last->data[write->len], data, writeLen);

    write->len += writeLen;
    write->cont += writeLen;

    dataLen -= writeLen;
    data += writeLen;
  }
}

static int
lineLength(const struct inOutBuff *read)
{
  struct chunk *chunk;
  int idx, len, off;
  int count;

  count = 0;

  for (chunk = read->first; chunk != NULL; chunk = chunk->next) {
    len = (chunk == read->last) ? read->len : CHUNK_SIZE;
    off = (chunk == read->first) ? read->off : 0;

    for (idx = off; idx < len; idx++) {
      count++;

      if (chunk->data[idx] == 10)
        return count;
    }
  }

  return -1;
}

static int
readBuff(struct inOutBuff *read, unsigned char *data, int dataLen)
{
  int readLen;
  struct chunk *chunk;
  int len;

  while (dataLen > 0) {
    if (read->first == NULL)
      return -1;

    len = (read->first == read->last) ? read->len : CHUNK_SIZE;

    readLen = len - read->off;

    if (dataLen < readLen)
      readLen = dataLen;

    memcpy(data, &read->first->data[read->off], readLen);

    read->off += readLen;
    read->cont -= readLen;

    dataLen -= readLen;
    data += readLen;

    if (read->off == len) {
      chunk = read->first;

      read->first = chunk->next;

      if (read->first == NULL)
        read->last = NULL;

      freeMem(chunk);

      read->off = 0;
    }
  }

  return 0;
}

static int
writeConn(struct connInfo *info)
{
  struct inOutBuff *write = &info->write[info->which];
  int writeLen;
  struct chunk *chunk;
  int len;

  for (;;) {
    if (write->first == NULL)
      return 0;

    len = (write->first == write->last) ? write->len : CHUNK_SIZE;

    writeLen = writeFileOs(info->sockId, &write->first->data[write->off], len - write->off);

    if (writeLen < 0) {
      error("cannot write to network connection\n");
      return -1;
    }

    debug(DEBUG_CONNECTION, "wrote %d bytes to connection\n", writeLen);

    if (writeLen == 0)
      return 0;

    write->off += writeLen;
    write->cont -= writeLen;

    if (write->off == len) {
      chunk = write->first;

      write->first = chunk->next;

      if (write->first == NULL)
        write->last = NULL;

      freeMem(chunk);

      write->off = 0;
    }
  }
}

static char *
getToken(char **point)
{
  char *localPoint = *point;
  char *start;

  while (*localPoint == 9 || *localPoint == 32)
    localPoint++;

  start = localPoint;

  while (*localPoint != 9 && *localPoint != 32 && *localPoint != 0)
    localPoint++;

  if (localPoint == start)
    return NULL;

  if (*localPoint != 0)
    *localPoint++ = 0;

  *point = localPoint;

  return start;
}

static void
writeBuffString(struct inOutBuff *write, const char *string)
{
  writeBuff(write, (const unsigned char *)string, strlen(string));
}

static int
cookieToSession(unsigned int *sessId, char *cookie)
{
  unsigned char mac1[16];
  unsigned char mac2[16];

  debug(DEBUG_SESSION, "cookie = %s\n", cookie);

  if (decHexString((unsigned char *)&cookieStruct.sessId, cookie, 4) < 0) {
    debug(DEBUG_SESSION, "cannot decode session id\n");
    return -1;
  }

  if (decHexString(mac1, cookie + 8, 16) < 0) {
    debug(DEBUG_SESSION, "cannot decode authenticator\n");
    return -1;
  }

  memset(mac2, 0, 16);
  rc4(mac2, 16, (unsigned char *)&cookieStruct, sizeof(cookieStruct));

  if (memcmp(mac1, mac2, 16) != 0) {
    debug(DEBUG_SESSION, "invalid authenticator\n");
    return -1;
  }

  *sessId = cookieStruct.sessId;

  debug(DEBUG_SESSION, "session id = %u\n", *sessId);

  return 0;
}

static char *
sessionToCookie(unsigned int sessId)
{
  unsigned char mac[16];
  static char buff[41];

  debug(DEBUG_SESSION, "session id = %u\n", sessId);

  cookieStruct.sessId = sessId;

  memset(mac, 0, 16);
  rc4(mac, 16, (unsigned char *)&cookieStruct, sizeof(cookieStruct));

  encHexString(buff, (unsigned char *)&cookieStruct.sessId, 4);
  encHexString(buff + 8, mac, 16);

  debug(DEBUG_SESSION, "cookie = %s\n", buff);

  return buff;
}

static void
writeBuffInt(struct inOutBuff *write, unsigned int val)
{
  char buff[10];

  writeBuffString(write, intToString(buff, val));
}

static void
printBuff(struct inOutBuff *buff, const char *form, ...)
{
  int i = 0;
  int start = 0;
  char *strVal;
  int intVal;

  va_list args;

  va_start(args, form);

  for (;;) {
    start = i;

    while (form[i] != '%' && form[i] != 0)
      i++;

    if (i > start)
      writeBuff(buff, (const unsigned char *)(form + start), i - start);

    if (form[i] == 0)
      break;

    if (form[i + 1] == '%')
      writeBuff(buff, (const unsigned char *)"%", 1);

    else if (form[i + 1] == 's') {
      strVal = va_arg(args, char *);
      writeBuffString(buff, strVal);
    }

    else if (form[i + 1] == 'd') {
      intVal = va_arg(args, int);
      writeBuffInt(buff, intVal);
    }

    i += 2;
  }

  va_end(args);
}

static const char *
errNoToErrStr(int errNo)
{
  switch (errNo) {
  case 200:
    return "OK";

  case 400:
    return "Bad Request";

  case 401:
    return "Unauthorized";

  case 404:
    return "Not Found";

  case 500:
    return "Internal Server Error";

  case 501:
    return "Not Implemented";

  case 505:
    return "HTTP Version Not Supported";

  default:
    return "For No Reason";
  }
}

static int
writeHeaders(struct connInfo *info, int errNo)
{
  printBuff(&info->write[0], "HTTP/1.1 %d %s\r\n", errNo, errNoToErrStr(errNo));

  printBuff(&info->write[0], "Server: TAS/0.1\r\n");

  if (info->contType != NULL)
    printBuff(&info->write[0], "Content-Type: %s\r\n", info->contType);

  if (info->contLen >= 0)
    printBuff(&info->write[0], "Content-Length: %d\r\n", info->contLen);

  if (info->newSess != NULL)
    printBuff(&info->write[0], "Set-Cookie: %s\r\n", sessionToCookie(info->newSess->id));

  if (errNo == 401)
    printBuff(&info->write[0], "WWW-Authenticate: Basic realm=\"TAS\"\r\n");

  printBuff(&info->write[0], "Accept-Ranges: none\r\n");
  printBuff(&info->write[0], "Connection: close\r\n");

  printBuff(&info->write[0], "Expires: Thu, 01 Jan 1970 00:00:00 GMT\r\n");
  printBuff(&info->write[0], "Cache-Control: no-cache\r\n");
  printBuff(&info->write[0], "Pragma: No-cache\r\n");

  printBuff(&info->write[1], "\r\n");

  return 0;
}

static void
writeErrorMsg(struct connInfo *info, int errNo, char *errMsg)
{
  if (info->verb == NULL || strcmp(info->verb, "HEAD") != 0) {
    printBuff(&info->write[2], "<html>\r\n");
    printBuff(&info->write[2], "<head><title>Error %d: %s</title></head>\r\n", errNo, errNoToErrStr(errNo));
    printBuff(&info->write[2], "<body>Error %d: %s (%s)</body>\r\n", errNo, errNoToErrStr(errNo),
              (errMsg == NULL) ? "Unknown Reason" : errMsg);
    printBuff(&info->write[2], "</html>\r\n");

    info->contLen = info->write[2].cont;
  }

  writeHeaders(info, errNo);

  info->state = STATE_DRAIN;
}

static void
writeError(struct connInfo *info, int errNo)
{
  writeErrorMsg(info, errNo, NULL);
}

static void
toLower(char *string)
{
  while (*string != 0) {
    if (*string >= 'A' && *string <= 'Z')
      *string += 32;

    string++;
  }
}

static void
unescape(char *string)
{
  int i, k, val;

  debug(DEBUG_REQUEST | DEBUG_LUA, "unescaped string = %s\n", string);

  k = 0;

  for (i = 0; string[i] != 0; i++) {
    if (string[i] == '%' && string[i + 1] != 0 && string[i + 2] != 0) {
      val = 0;

      if (addHexDigit(&val, string[i + 1]) >= 0 && addHexDigit(&val, string[i + 2]) >= 0) {
        string[k++] = (char)val;
        i += 2;
        continue;
      }
    }

    string[k++] = string[i];
  }

  string[k] = 0;

  debug(DEBUG_REQUEST | DEBUG_LUA, "escaped string = %s\n", string);
}

static int
serviceConn(struct connInfo *info)
{
  int i, k, len, len2;
  char *line, *tmp, *tmp2;
  struct httpHeader *head;
  unsigned char fileBuff[8192];
  char **argList;
  char *errMsg;
  unsigned int sessId;
  struct sessInfo *currSess;
  int pub;

  switch (info->state) {
  case STATE_REQUEST:
    debug(DEBUG_CONNECTION, "connection state is STATE_REQUEST\n");

    len = lineLength(&info->read);

    if (len <= 0)
      return 0;

    line = allocBuff(info, len);

    readBuff(&info->read, (unsigned char *)line, len);
    chomp(line, len);

    debug(DEBUG_REQUEST, "request line is '%s'\n", line);

    info->verb = getToken(&line);
    tmp = getToken(&line);
    info->proto = getToken(&line);

    debug(DEBUG_REQUEST, "verb = %s, uri = %s, protocol = %s\n", (info->verb == NULL) ? "none" : info->verb,
          (tmp == NULL) ? "none" : tmp, (info->proto == NULL) ? "none" : info->proto);

    if (info->verb == NULL || tmp == NULL || info->proto == NULL) {
      error("request without verb (%s), URI (%s), or protocol (%s)\n", (info->verb == NULL) ? "none" : info->verb,
            (tmp == NULL) ? "none" : tmp, (info->proto == NULL) ? "none" : info->proto);
      writeError(info, 400);
      return 0;
    }

    if (strcmp(info->verb, "GET") != 0 && strcmp(info->verb, "HEAD") != 0) {
      error("unsupported verb: %s\n", info->verb);
      writeError(info, 501);
      return 0;
    }

    if (strcmp(info->proto, "HTTP/1.1") != 0) {
      error("unsupported protocol version: %s\n", info->proto);
      writeError(info, 505);
      return 0;
    }

    if (strncmp(tmp, "http://", 7) == 0) {
      tmp += 7;

      info->host = tmp;

      while (*tmp != ':' && *tmp != '/' && *tmp != 0)
        tmp++;

      if (*tmp == 0) {
        error("URI host part does not end in ':' or '/'\n");
        writeError(info, 400);
        return 0;
      }

      if (*tmp == ':') {
        *tmp++ = 0;

        while (*tmp != '/' && *tmp != 0)
          tmp++;

        if (*tmp == 0) {
          error("URI port part does not end in '/'\n");
          writeError(info, 400);
          return 0;
        }

        tmp++;
      }

      else
        *tmp++ = 0;

      debug(DEBUG_REQUEST, "host = %s\n", info->host);

      info->path = tmp;
    }

    else if (tmp[0] == '/')
      info->path = ++tmp;

    else {
      error("URI path part is not an absolute path\n");
      writeError(info, 400);
      return 0;
    }

    while (*tmp != '?' && *tmp != 0) {
      if (tmp[0] == '.' && tmp[1] == '.') {
        error("URI path part contains '..'\n");
        writeError(info, 400);
        return 0;
      }

      tmp++;
    }

    if (*tmp == '?') {
      *tmp++ = 0;
      info->para = tmp;
    }

    debug(DEBUG_REQUEST, "path = %s, parameters = %s\n", info->path, (info->para == NULL) ? "none" : info->para);

    info->state = STATE_HEADERS;
    return 0;

  case STATE_HEADERS:
    debug(DEBUG_CONNECTION, "connection state is STATE_HEADERS\n");

    len = lineLength(&info->read);

    if (len <= 0)
      return 0;

    line = allocBuff(info, len);

    readBuff(&info->read, (unsigned char *)line, len);
    chomp(line, len);

    debug(DEBUG_REQUEST, "header line is '%s'\n", line);

    if (*line == 0) {
      if (info->host == NULL)
        for (head = info->firstHead; head != NULL; head = head->next)
          if (strcmp(head->name, "host") == 0)
            info->host = head->value;

      debug(DEBUG_REQUEST, "last header line, host = %s\n", (info->host == NULL) ? "none" : info->host);

      info->state = STATE_RESPONSE;
      return 0;
    }

    if (*line == 9 || *line == 32) {
      debug(DEBUG_REQUEST, "continued header line\n");

      if (info->lastHead == NULL) {
        error("no previous header to continue\n");
        writeError(info, 400);
        return 0;
      }

      len2 = strlen(info->lastHead->value);

      tmp = allocBuff(info, len2 + len);

      memcpy(tmp, info->lastHead->value, len2);
      memcpy(tmp + len2, line, len);

      info->lastHead->value = tmp;

      debug(DEBUG_REQUEST, "updated header, name = %s, value = '%s'\n", info->lastHead->name, info->lastHead->value);
    }

    else {
      tmp = getToken(&line);

      if (tmp == NULL) {
        error("header without name\n");
        writeError(info, 400);
        return 0;
      }

      for (i = 0; tmp[i] != ':' && tmp[i] != 0; i++);

      if (tmp[i] != ':' || tmp[i + 1] != 0) {
        error("header name does not end in ':'\n");
        writeError(info, 400);
        return 0;
      }

      tmp[i] = 0;

      toLower(tmp);

      head = allocBuff(info, sizeof(struct httpHeader));

      head->next = NULL;
      head->name = tmp;
      head->value = line;

      if (info->lastHead == NULL)
        info->firstHead = head;

      else
        info->lastHead->next = head;

      info->lastHead = head;

      debug(DEBUG_REQUEST, "new header, name = %s, value = '%s'\n", info->lastHead->name, info->lastHead->value);
    }

    return 0;

  case STATE_RESPONSE:
    debug(DEBUG_CONNECTION, "connection state is STATE_RESPONSE\n");

    unescape(info->path);

    len = strlen(confPubDir);

    pub = (len > 0 && strncmp(info->path, confPubDir, len) == 0 && (info->path[len] == 0 || info->path[len] == '/'));

    debug(DEBUG_REQUEST, "%s path\n", (pub == 0) ? "protected" : "public");

    if (pub == 0 && (confUser != NULL || confPassword != NULL)) {
      debug(DEBUG_REQUEST, "authentication required\n");

      for (head = info->firstHead; head != NULL; head = head->next)
        if (memcmp(head->name, "authorization", 14) == 0)
          break;

      if (head == NULL) {
        debug(DEBUG_REQUEST, "no authorization header present\n");

        writeError(info, 401);
        return 0;
      }

      tmp = getToken(&head->value);

      if (tmp == NULL || strcmp(tmp, "Basic") != 0) {
        error("\"Basic\" authorization info expected\n");
        writeError(info, 401);
        return 0;
      }

      tmp = getToken(&head->value);

      if (tmp == NULL) {
        error("authorization info lacks base-64 encoded data\n");
        writeError(info, 401);
        return 0;
      }

      tmp2 = allocBuff(info, strlen(tmp) * 3 / 4 + 1);

      if (decBase64((unsigned char *)tmp2, tmp) < 0) {
        error("base-64 decode failed\n");
        writeError(info, 401);
        return 0;
      }

      for (i = 0; tmp2[i] != ':' && tmp2[i] != 0; i++);

      if (tmp2[i] == 0) {
        error("authorization info lacks ':'\n");
        writeError(info, 401);
        return 0;
      }

      tmp2[i++] = 0;

      debug(DEBUG_REQUEST, "user = %s, password = %s\n", tmp2, tmp2 + i);

      if ((confUser != NULL && strcmp(confUser, tmp2) != 0) || (confPassword != NULL && strcmp(confPassword, tmp2 + i) != 0)) {
        error("user authentication failed\n");
        writeError(info, 401);
        return 0;
      }
    }

    if (isDirectory(confRootDir, info->path) > 0) {
      debug(DEBUG_REQUEST, "path is a directory\n");

      tmp = fullPath(info->path, confIndexFile);

      debug(DEBUG_REQUEST, "updated path = %s\n", tmp);

      info->path = allocBuff(info, strlen(tmp) + 1);
      strcpy(info->path, tmp);

      freeMem(tmp);
    }

    if (openFile(&info->fileId, confRootDir, info->path) < 0) {
      error("cannot find resource %s in root directory %s\n", info->path, confRootDir);
      writeError(info, 404);
      return 0;
    }

    for (i = 0; extMap[i].ext != NULL; i++) {
      len = strlen(extMap[i].ext);
      len2 = strlen(info->path);

      if (len2 >= len && memcmp(info->path + len2 - len, extMap[i].ext, len) == 0)
        break;
    }

    if (extMap[i].ext != NULL) {
      info->state = extMap[i].state;
      info->contType = extMap[i].type;

      debug(DEBUG_REQUEST, "extension recognized, next state = %d, content type = %s\n", info->state, info->contType);
    }

    else
      info->state = STATE_FILE;

    if (strcmp(info->verb, "HEAD") == 0) {
      closeFile(&info->fileId);

      writeHeaders(info, 200);

      info->state = STATE_DRAIN;
    }

    return 0;

  case STATE_FILE:
    debug(DEBUG_CONNECTION, "connection state is STATE_FILE\n");

    for (;;) {
      len = readFileOs(&info->fileId, fileBuff, sizeof(fileBuff));

      debug(DEBUG_CONNECTION, "read %d bytes from file\n", len);

      if (len <= 0) {
        if (len < 0) {
          closeFile(&info->fileId);

          info->contLen = info->write[2].cont;
          writeHeaders(info, 200);

          info->state = STATE_DRAIN;
        }

        break;
      }

      writeBuff(&info->write[2], fileBuff, len);
    }

    return 0;

  case STATE_LSP:
    debug(DEBUG_CONNECTION, "connection state is STATE_LSP\n");

    tmp = allocBuff(info, strlen(info->path) + 4 + 1);
    setExtension(tmp, info->path, ".lua");

    debug(DEBUG_LUA, "lua file name = %s\n", tmp);

    if (lspToLua(confRootDir, info->path, confWorkDir, tmp) < 0) {
      error("cannot transform %s into %s\n", info->path, tmp);
      writeError(info, 500);
      return 0;
    }

    tmp2 = allocBuff(info, strlen(info->path) + 4 + 1);
    setExtension(tmp2, info->path, ".lex");

    debug(DEBUG_LUA, "lex file name = %s\n", tmp2);

    if (luaToLex(&errMsg, confWorkDir, tmp, tmp2) < 0) {
      error("cannot transform %s into %s\n", tmp, tmp2);
      writeErrorMsg(info, 500, errMsg);

      if (errMsg != NULL)
        freeMem(errMsg);

      return 0;
    }

    tmp = info->para;

    if (tmp != NULL) {
      len = 3 * sizeof(char *);

      for (i = 0; tmp[i] != 0; i++) {
        if (tmp[i] == '+')
          tmp[i] = ' ';

        if (tmp[i] == '=' || tmp[i] == '&')
          len += sizeof(char *);
      }

      argList = allocBuff(info, len);

      i = 0;
      k = 0;

      while (tmp[i] != 0) {
        argList[k++] = tmp + i;

        while (tmp[i] != 0 && tmp[i] != '=')
          i++;

        if (tmp[i] == 0) {
          error("end of parameters while looking for '='\n");
          writeError(info, 400);
          return 0;
        }

        tmp[i++] = 0;

        debug(DEBUG_LUA, "parameter name = '%s'\n", argList[k - 1]);

        argList[k++] = tmp + i;

        while (tmp[i] != 0 && tmp[i] != '&')
          i++;

        if (tmp[i] != 0)
          tmp[i++] = 0;

        debug(DEBUG_LUA, "parameter value = '%s'\n", argList[k - 1]);
      }

      for (i = 0; i < k; i++)
        unescape(argList[i]);

      argList[k++] = NULL;
      argList[k++] = NULL;
    }

    else
      argList = NULL;

    currSess = NULL;

    for (head = info->firstHead; head != NULL; head = head->next)
      if (memcmp(head->name, "cookie", 7) == 0 && cookieToSession(&sessId, head->value) >= 0)
        break;

    if (head != NULL) {
      debug(DEBUG_SESSION, "looking for existing session\n");

      for (i = 0; i < numSess && sess[i]->id != sessId; i++);

      if (i < numSess) {
        debug(DEBUG_SESSION, "existing session found\n");

        currSess = sess[i];

        os_now(&currSess->time);
      }
    }

    if (currSess == NULL) {
      debug(DEBUG_SESSION, "no existing session\n");

      info->newSess = newSessInfo();
      currSess = info->newSess;
    }

    if (runLua(&errMsg, info, confWorkDir, tmp2, argList, &currSess->data) < 0) {
      error("cannot run %s\n", tmp2);

      if (info->newSess != NULL) {
        debug(DEBUG_SESSION, "cleaning up newly created session\n");

        if (info->newSess->data != NULL) {
          debug(DEBUG_SESSION, "freeing lua context\n");

          freeLuaSession(info->newSess->data);
        }

        freeMem(info->newSess);
        info->newSess = NULL;
      }

      debug(DEBUG_SESSION, "purging io buffer\n");

      freeInOutBuff(&info->write[1]);
      freeInOutBuff(&info->write[2]);

      initInOutBuff(&info->write[1]);
      initInOutBuff(&info->write[2]);

      writeErrorMsg(info, 500, errMsg);

      if (errMsg != NULL)
        freeMem(errMsg);

      return 0;
    }

    debug(DEBUG_SESSION, "lua code successfully executed\n");

    if (info->newSess != NULL) {
      if (info->newSess->data == NULL) {
        debug(DEBUG_SESSION, "no session required\n");

        freeMem(info->newSess);
        info->newSess = NULL;
      }

      else {
        if (numSess == MAX_SESS) {
          error("session limit reached, deleting least recently used session %d\n", sess[0]->id);
          freeLuaSession(sess[0]->data);
          freeMem(sess[0]);

          for (i = 0; i < MAX_SESS - 1; i++)
            sess[i] = sess[i + 1];

          numSess--;
        }

        sess[numSess++] = info->newSess;

        debug(DEBUG_SESSION, "session added\n");
      }
    }

    else {
      debug(DEBUG_SESSION, "aging sessions\n");

      for (i = 0; sess[i]->id != currSess->id; i++);

      while (i < numSess - 1) {
        sess[i] = sess[i + 1];
        i++;
      }

      if (currSess->data == NULL) {
        debug(DEBUG_SESSION, "session not required any longer\n");

        sess[i] = NULL;
        freeMem(currSess);

        numSess--;
      }

      else {
        debug(DEBUG_SESSION, "session stored\n");

        sess[i] = currSess;
      }
    }

    info->contLen = info->write[2].cont;
    writeHeaders(info, 200);

    info->state = STATE_DRAIN;
    return 0;

  case STATE_DRAIN:
    debug(DEBUG_CONNECTION, "connection state is STATE_DRAIN\n");
    debug(DEBUG_CONNECTION, "which = %d\n", info->which);

    if (info->write[info->which].first == NULL)
      info->which++;

    if (info->which == 3)
      return -1;

    return 0;
  }

  return 0;
}

int
httpService(int freq)
{
  struct fileId *sockId;
  struct ipAddr *addr;
  int i, k;
#ifdef TAS_BLOCK
  struct fileId *waitIds[MAX_CONN];
  int *waitFlags[MAX_CONN];
#endif
  unsigned int micro, microLimit;
  struct tasMessage *tasMsg;

  micro = getMicro();

#ifdef TAS_BLOCK
  for (i = 0; i < numConn; i++) {
    waitIds[i] = conn[i]->sockId;
    waitFlags[i] = &conn[i]->flags;

    conn[i]->flags = FLAG_READ;

    if (conn[i]->firstWrite != NULL)
      conn[i]->flags |= FLAG_WRITE;
  }

  if (waitForSockets(waitIds, waitFlags, numConn) < 0)
    return 0;
#endif

  while (numConn < MAX_CONN) {
    if (acceptConn(&sockId, &addr) < 0)
      break;

    conn[numConn++] = newConnInfo(sockId, addr);
  }

  i = 0;

  while (i < numConn) {
    if (((conn[i]->flags & FLAG_READ) != 0 && readConn(conn[i]) < 0)
        || ((conn[i]->flags & FLAG_WRITE) != 0 && writeConn(conn[i]) < 0) || serviceConn(conn[i]) < 0) {
      closeFile(conn[i]->sockId);

      freeConnInfo(conn[i]);

      for (k = i; k < numConn - 1; k++)
        conn[k] = conn[k + 1];

      conn[k] = NULL;

      numConn--;
    }

    else
      i++;
  }

  while (numSess > 0 && confSessTime > 0 && timedOut(&sess[0]->time, confSessTime) >= 0) {
    error("session %d timed out\n", sess[0]->id);

    freeLuaSession(sess[0]->data);

    freeMem(sess[0]);

    for (i = 0; i < numSess - 1; i++)
      sess[i] = sess[i + 1];

    numSess--;

    debug(DEBUG_SESSION, "%d sessions left\n", numSess);
  }

  while (numTasMsg > 0 && confMessTime > 0 && timedOut(&firstTasMsg->time, confMessTime) >= 0) {
    tasMsg = firstTasMsg;

    debug(DEBUG_MESSAGE, "message timed out, service ='%s', string = '%s', from = %s\n", tasMsg->service, tasMsg->string,
          tasMsg->from);

    firstTasMsg = firstTasMsg->next;

    if (lastTasMsg == tasMsg)
      lastTasMsg = NULL;

    freeMem(tasMsg->service);
    freeMem(tasMsg->string);
    freeMem(tasMsg->from);
    freeMem(tasMsg);

    numTasMsg--;

    debug(DEBUG_MESSAGE, "%d messages left\n", numTasMsg);
  }

  micro = getMicro() - micro;
  microLimit = (10000 * confQuantum) / freq;

  debug(DEBUG_QUANTUM, "service time = %u us, limit = %u us\n", micro, microLimit);

  if (microLimit > 0 && micro > microLimit)
    error("service took longer than expected (%u us, limit is %u us)\n", micro, microLimit);

  return 0;
}

void
httpShutdown(void)
{
  closeMainSocket();
}

void
httpAddTasMessage(const char *service, const char *string, const char *from)
{
  struct tasMessage *msg;

  debug(DEBUG_MESSAGE, "adding message, service = %s, string = %s, from = %s\n", service, string, from);

  msg = allocMem(sizeof(struct tasMessage));

  msg->next = NULL;

  os_now(&msg->time);

  msg->service = myStrdup(service);
  msg->string = myStrdup(string);
  msg->from = myStrdup(from);

  if (lastTasMsg != NULL)
    lastTasMsg->next = msg;

  else
    firstTasMsg = msg;

  lastTasMsg = msg;

  numTasMsg++;

  debug(DEBUG_MESSAGE, "new number of messages: %d\n", numTasMsg);
  debug(DEBUG_MESSAGE, "limiting message queue length\n");

  while (confMessLimit > 0 && numTasMsg > confMessLimit) {
    msg = firstTasMsg;

    debug(DEBUG_MESSAGE, "message removed, service ='%s', string = '%s', from = %s\n", msg->service, msg->string, msg->from);

    firstTasMsg = firstTasMsg->next;

    if (lastTasMsg == msg)
      lastTasMsg = NULL;

    freeMem(msg->service);
    freeMem(msg->string);
    freeMem(msg->from);
    freeMem(msg);

    numTasMsg--;
  }

  debug(DEBUG_MESSAGE, "%d messages left\n", numTasMsg);
}

int
httpGetTasMessage(const char *service, char **string, char **from)
{
  struct tasMessage *msg, *prevMsg;

  debug(DEBUG_MESSAGE, "getting message, service = %s\n", service);

  prevMsg = NULL;

  debug(DEBUG_MESSAGE, "walking through message queue\n");

  for (msg = firstTasMsg; msg != NULL; msg = msg->next) {
    debug(DEBUG_MESSAGE, "  service = %s, string = %s\n", msg->service, msg->string);

    if (strcmp(msg->service, service) == 0)
      break;

    prevMsg = msg;
  }

  debug(DEBUG_MESSAGE, "walk finished\n");

  if (msg == NULL) {
    debug(DEBUG_MESSAGE, "no message found\n");

    return -1;
  }

  if (msg == firstTasMsg)
    firstTasMsg = msg->next;

  else
    prevMsg->next = msg->next;

  if (msg == lastTasMsg)
    lastTasMsg = prevMsg;

  *string = msg->string;
  *from = msg->from;

  freeMem(msg->service);
  freeMem(msg);

  numTasMsg--;

  debug(DEBUG_MESSAGE, "%d messages left\n", numTasMsg);
  debug(DEBUG_MESSAGE, "returning '%s' received from %s\n", *string, *from);

  return 0;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
