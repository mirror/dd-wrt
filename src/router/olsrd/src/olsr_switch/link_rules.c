/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2005, Andreas Tønnesen(andreto@olsr.org)
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
 * $Id: link_rules.c,v 1.6 2005/06/03 06:12:23 kattemat Exp $
 */

#include "link_rules.h"
#include "olsr_host_switch.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int
ohs_check_link(struct ohs_connection *oc, union olsr_ip_addr *dst)
{
  struct ohs_ip_link *links = oc->links;

  while(links)
    {
      if(COMP_IP(&links->dst, dst))
	{
          int r;

          if(links->quality == 0)
            {
              if(logbits & LOG_LINK)
                printf("%s -> %s Q: %d\n", 
                   olsr_ip_to_string(&oc->ip_addr),
                   olsr_ip_to_string(dst),
                   links->quality);

              return 0;
            }

          r = 1 + (int)((100.0*rand())/(RAND_MAX + 1.0));

          if(logbits & LOG_LINK)
            printf("%s -> %s Q: %d R: %d\n", 
                   olsr_ip_to_string(&oc->ip_addr),
                   olsr_ip_to_string(dst),
                   links->quality, r);

          /* Random - based on quality */
          if(links->quality > r)
            return 0;
          else
            return 1;
	}

      links = links->next;
    }

  return 1;
}

int
ohs_delete_all_related_links(struct ohs_connection *oc)
{
  struct ohs_ip_link *links = oc->links;
  int cnt = 0;

  /* Delete links from this node*/
  while(links)
    {
      struct ohs_ip_link *tmp_link = links;
      links = links->next;
      free(tmp_link);
      cnt++;
    }

  /* Delete links to this node */

  // XXX - ToDo

  return cnt;
}

struct ohs_ip_link *
add_link(struct ohs_connection *src, struct ohs_connection *dst)
{
  struct ohs_ip_link *link;
  
  /* Create new link */
  link = malloc(sizeof(struct ohs_ip_link));
  if(!link)
    OHS_OUT_OF_MEMORY("New link");
  /* Queue */
  link->next = src->links;
  src->links = link;
  COPY_IP(&link->dst, &dst->ip_addr);
  src->linkcnt++;

  return link;
}

int
remove_link(struct ohs_connection *oc, struct ohs_ip_link *lnk)
{
  struct ohs_ip_link *links = oc->links;
  struct ohs_ip_link *prev_link = NULL;

  while(links)
    {
      if(links == lnk)
        {
          /* Remove */
          if(prev_link)
            prev_link->next = links->next;
          else
            oc->links = links->next;

          free(lnk);
          oc->linkcnt--;
          return 1;
        }
      prev_link = links;
      links = links->next;
    }
  return 0;
}

struct ohs_ip_link *
get_link(struct ohs_connection *oc, union olsr_ip_addr *dst)
{
  struct ohs_ip_link *links = oc->links;

  while(links)
    {
      if(COMP_IP(&links->dst, dst))
	{
	  return links;
	}

      links = links->next;
    }

  return NULL;
}
