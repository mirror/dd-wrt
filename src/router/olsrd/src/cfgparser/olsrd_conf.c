/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
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


#include "olsrd_conf.h"
#include "ipcalc.h"
#include "olsr_cfg.h"
#include "defs.h"
#include "net_olsr.h"
#include "olsr.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


extern FILE *yyin;
extern int yyparse(void);

static char copyright_string[] __attribute__((unused)) = "The olsr.org Optimized Link-State Routing daemon(olsrd) Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org) All rights reserved.";

int current_line;

/* Global stuff externed in defs.h */
FILE *debug_handle;             /* Where to send debug(defaults to stdout) */
struct olsrd_config *olsr_cnf;  /* The global configuration */

#ifdef MAKEBIN

/* Build as standalone binary */
int 
main(int argc, char *argv[])
{
  struct olsrd_config *cnf;

  if(argc == 1)
    {
      fprintf(stderr, "Usage: olsrd_cfgparser [filename] -print\n\n");
      exit(EXIT_FAILURE);
    }

  if((cnf = olsrd_parse_cnf(argv[1])) != NULL)
    {
      if((argc > 2) && (!strcmp(argv[2], "-print")))
	{
	  olsrd_print_cnf(cnf);  
	  olsrd_write_cnf(cnf, "./out.conf");
	}
      else
        printf("Use -print to view parsed values\n");
      printf("Configfile parsed OK\n");
    }
  else
    {
      printf("Failed parsing \"%s\"\n", argv[1]);
    }

  return 0;
}

#else

/* Build as part of olsrd */


#endif

struct olsrd_config *
olsrd_parse_cnf(const char *filename)
{
  struct olsr_if *in, *new_ifqueue;
  int rc;

  /* Initialize the global varibles - oparse.y needs it there */
  olsr_cnf = malloc(sizeof(*olsr_cnf));
  if (olsr_cnf == NULL) {
    fprintf(stderr, "Out of memory %s\n", __func__);
    return NULL;
  }

  set_default_cnf(olsr_cnf);

  printf("Parsing file: \"%s\"\n", filename);

  yyin = fopen(filename, "r");
  if (yyin == NULL) {
    fprintf(stderr, "Cannot open configuration file '%s': %s.\n",
            filename, strerror(errno));
    olsrd_free_cnf(olsr_cnf);
    olsr_cnf = NULL;
    return NULL;
  }

  current_line = 1;
  rc = yyparse();
  fclose(yyin);
  if (rc != 0) {
    olsrd_free_cnf(olsr_cnf);
    olsr_cnf = NULL;
    return NULL;
  }

  /* Reverse the queue (added by user request) */
  in = olsr_cnf->interfaces;
  new_ifqueue = NULL;

  while(in) {
    struct olsr_if *in_tmp = in; 
    in = in->next;

    in_tmp->next = new_ifqueue;
    new_ifqueue = in_tmp;
  }

  olsr_cnf->interfaces = new_ifqueue;

  for (in = olsr_cnf->interfaces; in != NULL; in = in->next) {
      /* set various stuff */
      in->configured = OLSR_FALSE;
      in->interf = NULL;
      in->host_emul = OLSR_FALSE;
  }
  return olsr_cnf;
}


int
olsrd_sanity_check_cnf(struct olsrd_config *cnf)
{
  struct olsr_if           *in = cnf->interfaces;
  struct if_config_options *io;

  /* Debug level */
  if(cnf->debug_level < MIN_DEBUGLVL ||
     cnf->debug_level > MAX_DEBUGLVL)
    {
      fprintf(stderr, "Debuglevel %d is not allowed\n", cnf->debug_level);
      return -1;
    }

  /* IP version */
  if(cnf->ip_version != AF_INET &&
     cnf->ip_version != AF_INET6)
    {
      fprintf(stderr, "Ipversion %d not allowed!\n", cnf->ip_version);
      return -1;
    }

  /* TOS */
  if(//cnf->tos < MIN_TOS ||
     cnf->tos > MAX_TOS)
    {
      fprintf(stderr, "TOS %d is not allowed\n", cnf->tos);
      return -1;
    }

  if(cnf->willingness_auto == OLSR_FALSE &&
     (cnf->willingness > MAX_WILLINGNESS))
    {
      fprintf(stderr, "Willingness %d is not allowed\n", cnf->willingness);
      return -1;
    }

  /* Hysteresis */
  if(cnf->use_hysteresis == OLSR_TRUE)
    {
      if(cnf->hysteresis_param.scaling < MIN_HYST_PARAM ||
	 cnf->hysteresis_param.scaling > MAX_HYST_PARAM)
	{
	  fprintf(stderr, "Hyst scaling %0.2f is not allowed\n", cnf->hysteresis_param.scaling);
	  return -1;
	}

      if(cnf->hysteresis_param.thr_high <= cnf->hysteresis_param.thr_low)
	{
	  fprintf(stderr, "Hyst upper(%0.2f) thr must be bigger than lower(%0.2f) threshold!\n", cnf->hysteresis_param.thr_high, cnf->hysteresis_param.thr_low);
	  return -1;
	}

      if(cnf->hysteresis_param.thr_high < MIN_HYST_PARAM ||
	 cnf->hysteresis_param.thr_high > MAX_HYST_PARAM)
	{
	  fprintf(stderr, "Hyst upper thr %0.2f is not allowed\n", cnf->hysteresis_param.thr_high);
	  return -1;
	}

      if(cnf->hysteresis_param.thr_low < MIN_HYST_PARAM ||
	 cnf->hysteresis_param.thr_low > MAX_HYST_PARAM)
	{
	  fprintf(stderr, "Hyst lower thr %0.2f is not allowed\n", cnf->hysteresis_param.thr_low);
	  return -1;
	}
    }

  /* Check Link quality dijkstra limit */
  if (olsr_cnf->lq_dinter < cnf->pollrate && olsr_cnf->lq_dlimit != 255) {
  	fprintf(stderr, "Link quality dijkstra limit must be higher than pollrate\n");
  	return -1;
  }
  
  /* Pollrate */

  if(cnf->pollrate < MIN_POLLRATE ||
     cnf->pollrate > MAX_POLLRATE)
    {
      fprintf(stderr, "Pollrate %0.2f is not allowed\n", cnf->pollrate);
      return -1;
    }

  /* NIC Changes Pollrate */

  if(cnf->nic_chgs_pollrate < MIN_NICCHGPOLLRT ||
     cnf->nic_chgs_pollrate > MAX_NICCHGPOLLRT)
    {
      fprintf(stderr, "NIC Changes Pollrate %0.2f is not allowed\n", cnf->nic_chgs_pollrate);
      return -1;
    }

  /* TC redundancy */

  if(//cnf->tc_redundancy < MIN_TC_REDUNDANCY ||
     cnf->tc_redundancy > MAX_TC_REDUNDANCY)
    {
      fprintf(stderr, "TC redundancy %d is not allowed\n", cnf->tc_redundancy);
      return -1;
    }

  /* MPR coverage */
  if(cnf->mpr_coverage < MIN_MPR_COVERAGE ||
     cnf->mpr_coverage > MAX_MPR_COVERAGE)
    {
      fprintf(stderr, "MPR coverage %d is not allowed\n", cnf->mpr_coverage);
      return -1;
    }

  /* Link Q and hysteresis cannot be activated at the same time */
  if(cnf->use_hysteresis == OLSR_TRUE && cnf->lq_level)
    {
      fprintf(stderr, "Hysteresis and LinkQuality cannot both be active! Deactivate one of them.\n");
      return -1;
    }

  /* Link quality level */

  if(cnf->lq_level > MAX_LQ_LEVEL)
    {
      fprintf(stderr, "LQ level %d is not allowed\n", cnf->lq_level);
      return -1;
    }

  /* Link quality window size */
  if(cnf->lq_level && (cnf->lq_aging < MIN_LQ_AGING || cnf->lq_aging > MAX_LQ_AGING))
    {
      fprintf(stderr, "LQ aging factor %f is not allowed\n", cnf->lq_aging);
      return -1;
    }

  /* NAT threshold value */
  if(cnf->lq_level && (cnf->lq_nat_thresh < 0.1 || cnf->lq_nat_thresh > 1.0))
    {
      fprintf(stderr, "NAT threshold %f is not allowed\n", cnf->lq_nat_thresh);
      return -1;
    }

  if(in == NULL)
    {
      fprintf(stderr, "No interfaces configured!\n");
      return -1;
    }

  /* Interfaces */
  while(in)
    {
      io = in->cnf;

      if(in->name == NULL || !strlen(in->name))
	{
	  fprintf(stderr, "Interface has no name!\n");
	  return -1;
	}

      if(io == NULL)
	{
	  fprintf(stderr, "Interface %s has no configuration!\n", in->name);
	  return -1;
	}
	
      /* HELLO interval */

      if (io->hello_params.validity_time < 0.0)
      {
        if (cnf->lq_level == 0)
          io->hello_params.validity_time = NEIGHB_HOLD_TIME;

        else
          io->hello_params.validity_time = (int)(REFRESH_INTERVAL / cnf->lq_aging);
      }

      if(io->hello_params.emission_interval < cnf->pollrate ||
	 io->hello_params.emission_interval > io->hello_params.validity_time)
	{
	  fprintf(stderr, "Bad HELLO parameters! (em: %0.2f, vt: %0.2f)\n", io->hello_params.emission_interval, io->hello_params.validity_time);
	  return -1;
	}

      /* TC interval */
      if(io->tc_params.emission_interval < cnf->pollrate ||
	 io->tc_params.emission_interval > io->tc_params.validity_time)
	{
	  fprintf(stderr, "Bad TC parameters! (em: %0.2f, vt: %0.2f)\n", io->tc_params.emission_interval, io->tc_params.validity_time);
	  return -1;
	}

      /* MID interval */
      if(io->mid_params.emission_interval < cnf->pollrate ||
	 io->mid_params.emission_interval > io->mid_params.validity_time)
	{
	  fprintf(stderr, "Bad MID parameters! (em: %0.2f, vt: %0.2f)\n", io->mid_params.emission_interval, io->mid_params.validity_time);
	  return -1;
	}

      /* HNA interval */
      if(io->hna_params.emission_interval < cnf->pollrate ||
	 io->hna_params.emission_interval > io->hna_params.validity_time)
	{
	  fprintf(stderr, "Bad HNA parameters! (em: %0.2f, vt: %0.2f)\n", io->hna_params.emission_interval, io->hna_params.validity_time);
	  return -1;
	}

      in = in->next;
    }

  return 0;
}


void
olsrd_free_cnf(struct olsrd_config *cnf)
{
  struct ip_prefix_list   *hd,   *h = cnf->hna_entries;
  struct olsr_if           *ind, *in = cnf->interfaces;
  struct plugin_entry      *ped, *pe = cnf->plugins;
  struct olsr_lq_mult      *mult, *next_mult;
  
  while(h)
    {
      hd = h;
      h = h->next;
      free(hd);
    }

  while(in)
    {
      for (mult = in->cnf->lq_mult; mult != NULL; mult = next_mult)
      {
        next_mult = mult->next;
        free(mult);
      }

      free(in->cnf);
      ind = in;
      in = in->next;
      free(ind->name);
      free(ind->config);
      free(ind);
    }

  while(pe)
    {
      ped = pe;
      pe = pe->next;
      free(ped->name);
      free(ped);
    }

  return;
}



struct olsrd_config *
olsrd_get_default_cnf(void)
{
  struct olsrd_config *c = malloc(sizeof(struct olsrd_config));
  if (c == NULL) {
    fprintf(stderr, "Out of memory %s\n", __func__);
    return NULL;
  }

  set_default_cnf(c);
  return c;
}




void
set_default_cnf(struct olsrd_config *cnf)
{
    memset(cnf, 0, sizeof(*cnf));
    
    cnf->debug_level = DEF_DEBUGLVL;
    cnf->no_fork = OLSR_FALSE;
    cnf->host_emul = OLSR_FALSE;
    cnf->ip_version  = AF_INET;
    cnf->ipsize = sizeof(struct in_addr);
    cnf->maxplen = 32;
    cnf->allow_no_interfaces = DEF_ALLOW_NO_INTS;
    cnf->tos = DEF_TOS;
    cnf->rttable = 254;
    cnf->rttable_default = 0;
    cnf->willingness_auto = DEF_WILL_AUTO;
    cnf->ipc_connections = DEF_IPC_CONNECTIONS;
    cnf->fib_metric = DEF_FIB_METRIC;

    cnf->use_hysteresis = DEF_USE_HYST;
    cnf->hysteresis_param.scaling = HYST_SCALING;
    cnf->hysteresis_param.thr_high = HYST_THRESHOLD_HIGH;
    cnf->hysteresis_param.thr_low = HYST_THRESHOLD_LOW;

    cnf->pollrate = DEF_POLLRATE;
    cnf->nic_chgs_pollrate = DEF_NICCHGPOLLRT;

    cnf->tc_redundancy = TC_REDUNDANCY;
    cnf->mpr_coverage = MPR_COVERAGE;
    cnf->lq_level = DEF_LQ_LEVEL;
    cnf->lq_fish = DEF_LQ_FISH;
    cnf->lq_dlimit = DEF_LQ_DIJK_LIMIT;
    cnf->lq_dinter = DEF_LQ_DIJK_INTER;
    cnf->lq_aging = DEF_LQ_AGING;
    cnf->lq_algorithm = NULL;
    cnf->lq_nat_thresh = DEF_LQ_NAT_THRESH;
    cnf->clear_screen = DEF_CLEAR_SCREEN;

    cnf->del_gws = OLSR_FALSE;
    cnf->will_int = 10 * HELLO_INTERVAL;
    cnf->max_jitter = 0.0;
    cnf->exit_value = EXIT_SUCCESS;
    cnf->max_tc_vtime = 0.0;
    cnf->ioctl_s = 0;
#if LINUX_POLICY_ROUTING
    cnf->rtnl_s = 0;
#endif

#if defined __FreeBSD__ || defined __MacOSX__ || defined __NetBSD__ || defined __OpenBSD__
    cnf->rts = 0;
#endif
}




struct if_config_options *
get_default_if_config(void)
{
  struct in6_addr in6;
  struct if_config_options *io = malloc(sizeof(*io));

  if(io == NULL) {
    fprintf(stderr, "Out of memory %s\n", __func__);
    return NULL;
  }

  memset(io, 0, sizeof(*io));

  io->ipv6_addrtype = 1; /* XXX - FixMe */

  inet_pton(AF_INET6, OLSR_IPV6_MCAST_SITE_LOCAL, &in6);
  io->ipv6_multi_site.v6 = in6;

  inet_pton(AF_INET6, OLSR_IPV6_MCAST_GLOBAL, &in6);
  io->ipv6_multi_glbl.v6 = in6;

  io->lq_mult = NULL;

  io->weight.fixed = OLSR_FALSE;
  io->weight.value = 0;

  io->ipv6_addrtype = 0; /* global */

  io->hello_params.emission_interval = HELLO_INTERVAL;
  io->hello_params.validity_time = NEIGHB_HOLD_TIME;
  io->tc_params.emission_interval = TC_INTERVAL;
  io->tc_params.validity_time = TOP_HOLD_TIME;
  io->mid_params.emission_interval = MID_INTERVAL;
  io->mid_params.validity_time = MID_HOLD_TIME;
  io->hna_params.emission_interval = HNA_INTERVAL;
  io->hna_params.validity_time = HNA_HOLD_TIME;
  io->autodetect_chg = OLSR_TRUE;

  return io;

}



void
olsrd_print_cnf(struct olsrd_config *cnf)
{
  struct ip_prefix_list   *h  = cnf->hna_entries;
  struct olsr_if           *in = cnf->interfaces;
  struct plugin_entry      *pe = cnf->plugins;
  struct ip_prefix_list    *ie = cnf->ipc_nets;
  struct olsr_lq_mult      *mult;
  char ipv6_buf[100];             /* buffer for IPv6 inet_htop */

  printf(" *** olsrd configuration ***\n");

  printf("Debug Level      : %d\n", cnf->debug_level);
  if(cnf->ip_version == AF_INET6)
    printf("IpVersion        : 6\n");
  else
    printf("IpVersion        : 4\n");
  if(cnf->allow_no_interfaces)
    printf("No interfaces    : ALLOWED\n");
  else
    printf("No interfaces    : NOT ALLOWED\n");
  printf("TOS              : 0x%02x\n", cnf->tos);
  printf("RtTable          : 0x%02x\n", cnf->rttable);
  printf("RtTableDefault   : 0x%02x\n", cnf->rttable_default);
  if(cnf->willingness_auto)
    printf("Willingness      : AUTO\n");
  else
    printf("Willingness      : %d\n", cnf->willingness);

  printf("IPC connections  : %d\n", cnf->ipc_connections);
  while(ie)
    {
      struct ipaddr_str strbuf;
      if (ie->net.prefix_len == olsr_cnf->maxplen) {
          printf("\tHost %s\n", olsr_ip_to_string(&strbuf, &ie->net.prefix));
      } else {
          printf("\tNet %s/%d\n", olsr_ip_to_string(&strbuf, &ie->net.prefix), ie->net.prefix_len);
      }
      ie = ie->next;
    }


  printf("Pollrate         : %0.2f\n", cnf->pollrate);

  printf("NIC ChangPollrate: %0.2f\n", cnf->nic_chgs_pollrate);

  printf("TC redundancy    : %d\n", cnf->tc_redundancy);

  printf("MPR coverage     : %d\n", cnf->mpr_coverage);
   
  printf("LQ level         : %d\n", cnf->lq_level);

  printf("LQ fish eye      : %d\n", cnf->lq_fish);

  printf("LQ Dijkstra limit: %d, %0.2f\n", cnf->lq_dlimit, cnf->lq_dinter);

  printf("LQ aging factor  : %f\n", cnf->lq_aging);

  printf("LQ algorithm name: %s\n", cnf->lq_algorithm ? cnf->lq_algorithm : "default");
  
  printf("NAT threshold    : %f\n", cnf->lq_nat_thresh);

  printf("Clear screen     : %s\n", cnf->clear_screen ? "yes" : "no");

  /* Interfaces */
  if(in)
    {
      printf("Interfaces:\n");
      while(in)
	{
	  printf(" dev: \"%s\"\n", in->name);
	  
	  if(in->cnf->ipv4_broadcast.v4.s_addr)
	    {
	      printf("\tIPv4 broadcast           : %s\n", inet_ntoa(in->cnf->ipv4_broadcast.v4));
	    }
	  else
	    {
	      printf("\tIPv4 broadcast           : AUTO\n");
	    }
	  
	  printf("\tIPv6 addrtype            : %s\n", in->cnf->ipv6_addrtype ? "site-local" : "global");
	  
	  //union olsr_ip_addr       ipv6_multi_site;
	  //union olsr_ip_addr       ipv6_multi_glbl;
	  printf("\tIPv6 multicast site/glbl : %s", inet_ntop(AF_INET6, &in->cnf->ipv6_multi_site.v6, ipv6_buf, sizeof(ipv6_buf)));
	  printf("/%s\n", inet_ntop(AF_INET6, &in->cnf->ipv6_multi_glbl.v6, ipv6_buf, sizeof(ipv6_buf)));
	  
	  printf("\tHELLO emission/validity  : %0.2f/%0.2f\n", in->cnf->hello_params.emission_interval, in->cnf->hello_params.validity_time);
	  printf("\tTC emission/validity     : %0.2f/%0.2f\n", in->cnf->tc_params.emission_interval, in->cnf->tc_params.validity_time);
	  printf("\tMID emission/validity    : %0.2f/%0.2f\n", in->cnf->mid_params.emission_interval, in->cnf->mid_params.validity_time);
	  printf("\tHNA emission/validity    : %0.2f/%0.2f\n", in->cnf->hna_params.emission_interval, in->cnf->hna_params.validity_time);
	  
          for (mult = in->cnf->lq_mult; mult != NULL; mult = mult->next)
          {
            printf("\tLinkQualityMult          : %s %0.2f\n", inet_ntop(cnf->ip_version, &mult->addr, ipv6_buf, sizeof (ipv6_buf)), (float)(mult->value)/65536.0);
          }

          printf("\tAutodetetc changes       : %s\n", in->cnf->autodetect_chg ? "yes" : "no");

	  in = in->next;
	}
    }




  /* Plugins */
  if(pe)
    {
      printf("Plugins:\n");

      while(pe)
	{
	  printf("\tName: \"%s\"\n", pe->name);
	  pe = pe->next;
	}
    }

  /* Hysteresis */
  if(cnf->use_hysteresis) {
    printf("Using hysteresis:\n");
    printf("\tScaling      : %0.2f\n", cnf->hysteresis_param.scaling);
    printf("\tThr high/low : %0.2f/%0.2f\n", cnf->hysteresis_param.thr_high, cnf->hysteresis_param.thr_low);
  } else {
    printf("Not using hysteresis\n");
  }

  /* HNA IPv4 and IPv6 */
  if(h) {
    printf("HNA%d entries:\n", cnf->ip_version == AF_INET ? 4 : 6);
    while(h) {
      struct ipaddr_str buf;
      printf("\t%s/", olsr_ip_to_string(&buf, &h->net.prefix));
      if (cnf->ip_version == AF_INET) {
        union olsr_ip_addr ip;
        olsr_prefix_to_netmask(&ip, h->net.prefix_len);
        printf("%s\n", olsr_ip_to_string(&buf, &ip));
      } else {
        printf("%d\n", h->net.prefix_len);
      }
      h = h->next;
    }
  }
}

#if defined WIN32
struct ioinfo
{
	unsigned int handle;
	unsigned char attr;
	char buff;
	int flag;
	CRITICAL_SECTION lock;
};

void win32_stdio_hack(unsigned int handle)
{
  HMODULE lib;
  struct ioinfo **info;

  lib = LoadLibrary("msvcrt.dll");

  info = (struct ioinfo **)GetProcAddress(lib, "__pioinfo");

  // (*info)[1].handle = handle;
  // (*info)[1].attr = 0x89; // FOPEN | FTEXT | FPIPE;

  (*info)[2].handle = handle;
  (*info)[2].attr = 0x89;

  // stdout->_file = 1;
  stderr->_file = 2;

  // setbuf(stdout, NULL);
  setbuf(stderr, NULL);
}

void*
win32_olsrd_malloc(size_t size)
{
	return malloc(size);
}

void
win32_olsrd_free(void* ptr)
{
	free(ptr);
}
#endif

void ip_prefix_list_add(struct ip_prefix_list **list,
                        const union olsr_ip_addr *net,
                        olsr_u8_t prefix_len)
{
  struct ip_prefix_list *new_entry = malloc(sizeof(*new_entry));
  
  new_entry->net.prefix = *net;
  new_entry->net.prefix_len = prefix_len;

  /* Queue */
  new_entry->next = *list;
  *list = new_entry;
}

int ip_prefix_list_remove(struct ip_prefix_list **list,
                          const union olsr_ip_addr *net,
                          olsr_u8_t prefix_len)
{
  struct ip_prefix_list *h = *list, *prev = NULL;

  while (h != NULL) {
    if (ipequal(net, &h->net.prefix) && h->net.prefix_len == prefix_len) {
      /* Dequeue */
      if (prev == NULL) {
        *list = h->next;
      } else {
        prev->next = h->next;
      }
      free(h);
      return 1;
    }
    prev = h;
    h = h->next;
  }
  return 0;
}

struct ip_prefix_list *ip_prefix_list_find(struct ip_prefix_list *list,
                                           const union olsr_ip_addr *net,
                                           olsr_u8_t prefix_len)
{
  struct ip_prefix_list *h;
  for (h = list; h != NULL; h = h->next) {
    if (prefix_len == h->net.prefix_len && ipequal(net, &h->net.prefix)) {
      return h;
    }
  }
  return NULL;
}
