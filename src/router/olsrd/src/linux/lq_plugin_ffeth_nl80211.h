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

#ifdef __linux__
#ifdef LINUX_NL80211 /* Optional - not supported on all platforms */

#ifndef LQ_ETX_FFETH_NL80211_
#define LQ_ETX_FFETH_NL80211_

#include "olsr_types.h"
#include "lq_plugin.h"

#ifdef LINUX_NL80211
#include <net/ethernet.h>
#include "nl80211_link_info.h"
#endif

#define LQ_ALGORITHM_ETX_FFETH_NL80211_NAME "etx_ffeth_nl80211"

#define LQ_FFETH_WINDOW 32
#define LQ_FFETH_QUICKSTART_INIT 4

struct lq_ffeth {
  uint8_t valueLq;
  uint8_t valueNlq;
#ifdef LINUX_NL80211
  uint8_t valueBandwidth;
  uint8_t valueRSSI;
#endif
};

struct lq_ffeth_hello {
  struct lq_ffeth smoothed_lq;
  struct lq_ffeth lq;
  uint8_t windowSize, activePtr;
  uint16_t last_seq_nr;
  uint16_t missed_hellos;
  bool perfect_eth;
  uint16_t received[LQ_FFETH_WINDOW], total[LQ_FFETH_WINDOW];
};

extern struct lq_handler lq_etx_ffeth_nl80211_handler;

#endif /* LQ_ETX_FFETH_NL80211_ */

#endif /* LINUX_NL80211 */
#endif /* __linux__ */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
