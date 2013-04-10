
/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org)
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
 * System logging interface for GNU/Linux systems
 */

#include "../olsr_cfg.h"
#include "../log.h"
#include <syslog.h>
#include <stdarg.h>

#ifdef __ANDROID__
#include <android/log.h>
#endif /* __ANDROID__ */

void
olsr_openlog(const char *ident __attribute__((unused)))
{
#ifndef __ANDROID__
  openlog(ident, LOG_PID | LOG_ODELAY, LOG_DAEMON);
  setlogmask(LOG_UPTO(LOG_INFO));
#endif /* __ANDROID__ */

  return;
}

#if defined SYSLOG_NUMBERING && SYSLOG_NUMBERING

unsigned int olsr_syslog_ctr = 0;

void 
olsr_syslog_real(int level, const char *format, ...)
{

#else /* defined SYSLOG_NUMBERING && SYSLOG_NUMBERING */

void 
olsr_syslog(int level, const char *format, ...)
{

#endif /* defined SYSLOG_NUMBERING && SYSLOG_NUMBERING */

  int linux_level;
  va_list arglist;

  switch (level) {
  case (OLSR_LOG_INFO):
#ifdef __ANDROID__
    linux_level = ANDROID_LOG_INFO;
#else /* __ANDROID__ */
    linux_level = LOG_INFO;
#endif /* __ANDROID__ */
    break;
  case (OLSR_LOG_ERR):
#ifdef __ANDROID__
    linux_level = ANDROID_LOG_ERROR;
#else /* __ANDROID__ */
    linux_level = LOG_ERR;
#endif /* __ANDROID__ */
    break;
  default:
    return;
  }

  va_start(arglist, format);
#ifdef __ANDROID__
  __android_log_vprint(linux_level, "olsrd", format, arglist);
#else /* __ANDROID__ */
  vsyslog(linux_level, format, arglist);
#endif /* __ANDROID__ */
  va_end(arglist);

  return;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
