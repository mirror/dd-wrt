/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2014
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  Functions to asynchronously convert name to IP address

  */

#include "config.h"
#include "sysincl.h"

#include "nameserv_async.h"
#include "logging.h"
#include "memory.h"
#include "privops.h"
#include "sched.h"
#include "util.h"

#ifdef USE_PTHREAD_ASYNCDNS
#include <pthread.h>

/* ================================================== */

struct DNS_Async_Instance {
  const char *name;
  DNS_Status status;
  IPAddr addresses[DNS_MAX_ADDRESSES];
  DNS_NameResolveHandler handler;
  void *arg;

  pthread_t thread;
  int pipe[2];
};

static int resolving_threads = 0;

/* ================================================== */

static void *
start_resolving(void *anything)
{
  struct DNS_Async_Instance *inst = (struct DNS_Async_Instance *)anything;

  inst->status = PRV_Name2IPAddress(inst->name, inst->addresses, DNS_MAX_ADDRESSES);

  /* Notify the main thread that the result is ready */
  if (write(inst->pipe[1], "", 1) < 0)
    ;

  return NULL;
}

/* ================================================== */

static void
end_resolving(int fd, int event, void *anything)
{
  struct DNS_Async_Instance *inst = (struct DNS_Async_Instance *)anything;
  int i;

  if (pthread_join(inst->thread, NULL)) {
    LOG_FATAL("pthread_join() failed");
  }

  resolving_threads--;

  SCH_RemoveFileHandler(inst->pipe[0]);
  close(inst->pipe[0]);
  close(inst->pipe[1]);

  for (i = 0; inst->status == DNS_Success && i < DNS_MAX_ADDRESSES &&
              inst->addresses[i].family != IPADDR_UNSPEC; i++)
    ;

  (inst->handler)(inst->status, i, inst->addresses, inst->arg);

  Free(inst);
}

/* ================================================== */

void
DNS_Name2IPAddressAsync(const char *name, DNS_NameResolveHandler handler, void *anything)
{
  struct DNS_Async_Instance *inst;

  inst = MallocNew(struct DNS_Async_Instance);
  inst->name = name;
  inst->handler = handler;
  inst->arg = anything;
  inst->status = DNS_Failure;

  if (pipe(inst->pipe)) {
    LOG_FATAL("pipe() failed");
  }

  UTI_FdSetCloexec(inst->pipe[0]);
  UTI_FdSetCloexec(inst->pipe[1]);

  resolving_threads++;
  assert(resolving_threads <= 1);

  if (pthread_create(&inst->thread, NULL, start_resolving, inst)) {
    LOG_FATAL("pthread_create() failed");
  }

  SCH_AddFileHandler(inst->pipe[0], SCH_FILE_INPUT, end_resolving, inst);
}

/* ================================================== */

#else
#error
#endif
