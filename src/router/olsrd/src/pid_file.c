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

#include "pid_file.h"
#include "olsr_cfg.h"

#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#ifdef __WIN32
#include <process.h>
#endif

/**
 * Write the current PID to the configured PID file (if one is configured)
 */
bool writePidFile(void) {
  if (olsr_cnf->pidfile) {
    char buf[PATH_MAX + 256];

    /* create / open the PID file */
#ifdef __WIN32
    mode_t mode = S_IRUSR | S_IWUSR;
#else
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
#endif
    int fd = open(olsr_cnf->pidfile, O_CREAT | O_WRONLY, mode);
    if (fd < 0) {
      snprintf(buf, sizeof(buf), "Could not create the PID file %s", olsr_cnf->pidfile);
      perror(buf);
      return false;
    }

    /* write the PID */
    {
#ifdef __WIN32
      pid_t pid = _getpid();
#else
      pid_t pid = getpid();
#endif
      int chars = snprintf(buf, sizeof(buf), "%d", (int) pid);
      ssize_t chars_written = write(fd, buf, chars);
      if (chars_written != chars) {
        close(fd);
        snprintf(buf, sizeof(buf), "Could not write PID %d to the PID file %s", (int) pid, olsr_cnf->pidfile);
        perror(buf);
        removePidFile();
        return false;
      }
    }

    if (close(fd) < 0) {
      snprintf(buf, sizeof(buf), "Could not close PID file %s", olsr_cnf->pidfile);
      perror(buf);
      removePidFile();
      return false;
    }
  }

  return true;
}

void removePidFile(void) {
  if (!olsr_cnf || !olsr_cnf->pidfile) {
    return;
  }

  if (remove(olsr_cnf->pidfile) < 0) {
    char buf[PATH_MAX + 256];
    snprintf(buf, sizeof(buf), "Could not remove the PID file %s", olsr_cnf->pidfile);
    perror(buf);
  }
}
