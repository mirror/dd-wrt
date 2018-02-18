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

#include "rebuild_packet.h"
#include "ipcalc.h"
#include "defs.h"
#include "olsr.h"
#include "mid_set.h"
#include "mantissa.h"
#include "net_olsr.h"

/**
 *Process/rebuild MID message. Converts the OLSR
 *packet to the internal mid_message format.
 *@param mmsg the mid_message struct in wich infomation
 *is to be put.
 *@param m the entire OLSR message revieved.
 *@return negative on error
 */

void
mid_chgestruct(struct mid_message *mmsg, const union olsr_message *m)
{
  int i;
  struct mid_alias *alias, *alias_tmp;
  int no_aliases;

  /* Checking if everything is ok */
  if ((!m) || (m->v4.olsr_msgtype != MID_MESSAGE))
    return;

  alias = NULL;

  if (olsr_cnf->ip_version == AF_INET) {
    /* IPv4 */
    const struct midaddr *maddr = m->v4.message.mid.mid_addr;
    /*
     * How many aliases?
     * nextmsg contains size of
     * the addresses + 12 bytes(nextmessage, from address and the header)
     */
    no_aliases = ((ntohs(m->v4.olsr_msgsize) - 12) / 4);

    /*printf("Aliases: %d\n", no_aliases); */
    mmsg->mid_origaddr.v4.s_addr = m->v4.originator;
    mmsg->addr.v4.s_addr = m->v4.originator;
    /*seq number */
    mmsg->mid_seqno = ntohs(m->v4.seqno);
    mmsg->mid_addr = NULL;

    /* Get vtime */
    mmsg->vtime = me_to_reltime(m->v4.olsr_vtime);

    /*printf("Sequencenuber of MID from %s is %d\n", ip_to_string(&mmsg->addr), mmsg->mid_seqno); */

    for (i = 0; i < no_aliases; i++) {
      alias = olsr_malloc(sizeof(struct mid_alias), "MID chgestruct");

      alias->alias_addr.v4.s_addr = maddr->addr;
      alias->next = mmsg->mid_addr;
      mmsg->mid_addr = alias;
      maddr++;
    }

    if (olsr_cnf->debug_level > 1) {
      struct ipaddr_str buf;
      OLSR_PRINTF(3, "Alias list for %s: ", olsr_ip_to_string(&buf, &mmsg->mid_origaddr));
      OLSR_PRINTF(3, "%s", olsr_ip_to_string(&buf, &mmsg->addr));
      alias_tmp = mmsg->mid_addr;
      while (alias_tmp) {
        OLSR_PRINTF(3, " - %s", olsr_ip_to_string(&buf, &alias_tmp->alias_addr));
        alias_tmp = alias_tmp->next;
      }
      OLSR_PRINTF(3, "\n");
    }
  } else {
    /* IPv6 */
    const struct midaddr6 *maddr6 = m->v6.message.mid.mid_addr;
    /*
     * How many aliases?
     * nextmsg contains size of
     * the addresses + 12 bytes(nextmessage, from address and the header)
     */
    no_aliases = ((ntohs(m->v6.olsr_msgsize) - 12) / 16);       /* NB 16 */

    /*printf("Aliases: %d\n", no_aliases); */
    mmsg->mid_origaddr.v6 = m->v6.originator;
    mmsg->addr.v6 = m->v6.originator;
    /*seq number */
    mmsg->mid_seqno = ntohs(m->v6.seqno);
    mmsg->mid_addr = NULL;

    /* Get vtime */
    mmsg->vtime = me_to_reltime(m->v6.olsr_vtime);

    /*printf("Sequencenuber of MID from %s is %d\n", ip_to_string(&mmsg->addr), mmsg->mid_seqno); */

    for (i = 0; i < no_aliases; i++) {
      alias = olsr_malloc(sizeof(struct mid_alias), "MID chgestruct 2");

      /*printf("Adding alias: %s\n", olsr_ip_to_string(&buf, (union olsr_ip_addr *)&maddr6->addr)); */
      alias->alias_addr.v6 = maddr6->addr;
      alias->next = mmsg->mid_addr;
      mmsg->mid_addr = alias;

      maddr6++;
    }

    if (olsr_cnf->debug_level > 1) {
      struct ipaddr_str buf;
      OLSR_PRINTF(3, "Alias list for %s", ip6_to_string(&buf, &mmsg->mid_origaddr.v6));
      OLSR_PRINTF(3, "%s", ip6_to_string(&buf, &mmsg->addr.v6));

      alias_tmp = mmsg->mid_addr;
      while (alias_tmp) {
        OLSR_PRINTF(3, " - %s", ip6_to_string(&buf, &alias_tmp->alias_addr.v6));
        alias_tmp = alias_tmp->next;
      }
      OLSR_PRINTF(3, "\n");
    }
  }

}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
