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

#include "lock_file.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#if defined __ANDROID__
  #define DEFAULT_LOCKFILE_PREFIX "/data/local/olsrd"
#elif defined linux || defined __FreeBSD__ || defined __NetBSD__ || defined __OpenBSD__
  #define DEFAULT_LOCKFILE_PREFIX "/var/run/olsrd"
#elif defined _WIN32
  #define DEFAULT_LOCKFILE_PREFIX "C:\\olsrd"
#else /* defined _WIN32 */
  #define DEFAULT_LOCKFILE_PREFIX "olsrd"
#endif /* defined _WIN32 */

#ifdef _WIN32
  static HANDLE lck = INVALID_HANDLE_VALUE;
#else
  static int lock_fd = -1;
#endif

/**
 * @param cnf the olsrd configuration
 * @return a malloc-ed string for the default lock file name
 */
char * olsrd_get_default_lockfile(struct olsrd_config *cnf) {
  char buf[FILENAME_MAX];
  int ipv = (cnf->ip_version == AF_INET) ? 4 : 6;

#ifndef DEFAULT_LOCKFILE_PREFIX
  snprintf(buf, sizeof(buf), "%s-ipv%d.lock", cnf->configuration_file ? cnf->configuration_file : "olsrd", ipv);
#else
  snprintf(buf, sizeof(buf), "%s-ipv%d.lock", DEFAULT_LOCKFILE_PREFIX, ipv);
#endif /* DEFAULT_LOCKFILE_PREFIX */

  return strdup(buf);
}

/*
 * Creates a zero-length locking file and use fcntl to
 * place an exclusive lock over it. The lock will be
 * automatically erased when the olsrd process ends,
 * so it will even work well with a SIGKILL.
 *
 * Additionally the lock can be killed by removing the
 * locking file.
 */
static bool olsr_create_lock_file_internal(void) {
#ifdef _WIN32
  bool success;

  lck = CreateFile(olsr_cnf->lock_file,
      GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      NULL,
      OPEN_ALWAYS,
      FILE_ATTRIBUTE_NORMAL |
      FILE_FLAG_DELETE_ON_CLOSE,
      NULL);
  CreateEvent(NULL, TRUE, FALSE, olsr_cnf->lock_file);
  if ((INVALID_HANDLE_VALUE == lck) || (ERROR_ALREADY_EXISTS == GetLastError())) {
    olsr_remove_lock_file();
    return false;
  }

  success = LockFile( lck, 0, 0, 0, 0);

  if (!success) {
    olsr_remove_lock_file();
    return false;
  }

#else /* _WIN32 */
  struct flock lck;

  /* create file for lock */
  lock_fd = open(olsr_cnf->lock_file, O_WRONLY | O_CREAT, S_IRWXU);
  if (lock_fd < 0) {
    olsr_remove_lock_file();
    return false;
  }

  /* create exclusive lock for the whole file */
  lck.l_type = F_WRLCK;
  lck.l_whence = SEEK_SET;
  lck.l_start = 0;
  lck.l_len = 0;
  lck.l_pid = 0;

  if (fcntl(lock_fd, F_SETLK, &lck) == -1) {
    olsr_remove_lock_file();
    return false;
  }
#endif /* _WIN32 */

  return true;
}

bool olsr_create_lock_file(void) {
  bool created = false;
  int i;

  if (!olsr_cnf->host_emul) {
    return true;
  }

  for (i = 5; i >= 0; i--) {
    OLSR_PRINTF(3, "Trying to get olsrd lock...\n");
    if (olsr_create_lock_file_internal()) {
      /* lock successfully created */
      created = true;
      break;
    }

    sleep(1);
  }

  return created;
}

void olsr_remove_lock_file(void) {
#ifdef _WIN32
  if (INVALID_HANDLE_VALUE != lck) {
    CloseHandle(lck);
    lck = INVALID_HANDLE_VALUE;
  }
#else
  if (lock_fd >= 0) {
    close(lock_fd);
    lock_fd = -1;
  }
#endif

  if (!olsr_cnf->lock_file || !strlen(olsr_cnf->lock_file)) {
    return;
  }

  (void)remove(olsr_cnf->lock_file);
}
