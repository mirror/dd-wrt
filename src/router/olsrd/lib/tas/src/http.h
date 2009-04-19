
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

#include "olsrd_plugin.h"       /* union set_plugin_parameter_addon */

#define CHUNK_SIZE 1024

struct chunk {
  struct chunk *next;
  unsigned char data[CHUNK_SIZE];
};

struct inOutBuff {
  int off, len;
  int cont;
  struct chunk *first, *last;
};

#define BUFF_SIZE 1024

struct workBuff {
  struct workBuff *next;
  unsigned char *data;
};

struct httpHeader {
  struct httpHeader *next;
  char *name;
  char *value;
};

#define STATE_REQUEST 0
#define STATE_HEADERS 1
#define STATE_RESPONSE 2
#define STATE_FILE 3
#define STATE_LSP 4
#define STATE_DRAIN 5

struct connInfo {
  struct fileId *sockId;
  struct ipAddr *addr;

  int state;

  struct inOutBuff read;
  struct inOutBuff write[3];

  int which;

  int flags;

  struct workBuff *buff;

  int buffUsed;
  int buffTotal;

  struct httpHeader *firstHead, *lastHead;

  char *verb;
  char *host;
  char *path;
  char *para;
  char *proto;

  const char *contType;
  int contLen;

  struct sessInfo *newSess;

  struct fileId fileId;

  char *authUser;
  char *authPass;
};

extern void httpInit(void);

extern int httpSetAddress(const char *addrStr, void *data, set_plugin_parameter_addon addon);
extern int httpSetPort(const char *portStr, void *data, set_plugin_parameter_addon addon);
extern int httpSetRootDir(const char *rootDir, void *data, set_plugin_parameter_addon addon);
extern int httpSetWorkDir(const char *workDir, void *data, set_plugin_parameter_addon addon);
extern int httpSetIndexFile(const char *indexFile, void *data, set_plugin_parameter_addon addon);
extern int httpSetUser(const char *user, void *data, set_plugin_parameter_addon addon);
extern int httpSetPassword(const char *password, void *data, set_plugin_parameter_addon addon);
extern int httpSetSessTime(const char *timeStr, void *data, set_plugin_parameter_addon addon);
extern int httpSetPubDir(const char *pref, void *data, set_plugin_parameter_addon addon);
extern int httpSetQuantum(const char *quantumStr, void *data, set_plugin_parameter_addon addon);
extern int httpSetMessTime(const char *timeStr, void *data, set_plugin_parameter_addon addon);
extern int httpSetMessLimit(const char *limitStr, void *data, set_plugin_parameter_addon addon);

extern int httpSetup(void);
extern int httpService(int freq);
extern void httpShutdown(void);
extern void httpAddTasMessage(const char *service, const char *string, const char *from);
extern int httpGetTasMessage(const char *service, char **string, char **from);

extern void writeBuff(struct inOutBuff *write, const unsigned char *data, int dataLen);
extern void *allocBuff(struct connInfo *info, int len);

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
