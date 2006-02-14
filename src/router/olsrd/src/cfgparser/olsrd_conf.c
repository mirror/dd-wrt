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
 * $Id: olsrd_conf.c,v 1.46 2005/11/17 04:25:44 tlopatic Exp $
 */


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "olsrd_conf.h"


extern FILE *yyin;
extern int yyparse(void);

static char copyright_string[] = "The olsr.org Optimized Link-State Routing daemon(olsrd) Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org) All rights reserved.";

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
  struct olsr_if *in, *new_ifqueue, *in_tmp;

  /* Stop the compiler from complaining */
  strlen(copyright_string);

  cnf = malloc(sizeof(struct olsrd_config));
  if (cnf == NULL)
    {
      fprintf(stderr, "Out of memory %s\n", __func__);
      return NULL;
  }

  set_default_cnf(cnf);

  printf("Parsing file: \"%s\"\n", filename);

  yyin = fopen(filename, "r");
  
  if (yyin == NULL)
    {
      fprintf(stderr, "Cannot open configuration file '%s': %s.\n",
	      filename, strerror(errno));
      free(cnf);
      return NULL;
  }

  current_line = 1;

  if (yyparse() != 0)
    {
      fclose(yyin);
      olsrd_free_cnf(cnf);
      return NULL;
    }
  
  fclose(yyin);

  /* Reverse the queue (added by user request) */
  in = cnf->interfaces;
  new_ifqueue = NULL;

  while(in)
    {
      in_tmp = in; 
      in = in->next;

      in_tmp->next = new_ifqueue;
      new_ifqueue = in_tmp;
    }

  cnf->interfaces = new_ifqueue;

  in = cnf->interfaces;

  while(in)
    {
      /* set various stuff */
      in->index = cnf->ifcnt++;
      in->configured = OLSR_FALSE;
      in->interf = NULL;
      in->host_emul = OLSR_FALSE;
      in = in->next;
    }


  return cnf;
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

  /* Pollrate */

  if(cnf->pollrate < MIN_POLLRATE ||
     cnf->pollrate > MAX_POLLRATE)
    {
      fprintf(stderr, "Pollrate %0.2f is not allowed\n", cnf->pollrate);
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
  if(cnf->lq_level && (cnf->lq_wsize < MIN_LQ_WSIZE || cnf->lq_wsize > MAX_LQ_WSIZE))
    {
      fprintf(stderr, "LQ window size %d is not allowed\n", cnf->lq_wsize);
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
          io->hello_params.validity_time = cnf->lq_wsize * io->hello_params.emission_interval;
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
  struct hna4_entry        *h4d, *h4 = cnf->hna4_entries;
  struct hna6_entry        *h6d, *h6 = cnf->hna6_entries;
  struct olsr_if           *ind, *in = cnf->interfaces;
  struct plugin_entry      *ped, *pe = cnf->plugins;
  struct olsr_lq_mult      *mult, *next_mult;
  
  while(h4)
    {
      h4d = h4;
      h4 = h4->next;
      free(h4d);
    }

  while(h6)
    {
      h6d = h6;
      h6 = h6->next;
      free(h6d);
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
olsrd_get_default_cnf()
{
  cnf = malloc(sizeof(struct olsrd_config));
  if (cnf == NULL)
    {
      fprintf(stderr, "Out of memory %s\n", __func__);
      return NULL;
  }

  set_default_cnf(cnf);

  return cnf;
}




void
set_default_cnf(struct olsrd_config *cnf)
{
    memset(cnf, 0, sizeof(struct olsrd_config));
    
    cnf->debug_level = DEF_DEBUGLVL;
    cnf->no_fork = OLSR_FALSE;
    cnf->host_emul = OLSR_FALSE;
    cnf->ip_version  = AF_INET;
    cnf->allow_no_interfaces = DEF_ALLOW_NO_INTS;
    cnf->tos = DEF_TOS;
    cnf->willingness_auto = DEF_WILL_AUTO;
    cnf->ipc_connections = DEF_IPC_CONNECTIONS;
    cnf->open_ipc = cnf->ipc_connections ? OLSR_TRUE : OLSR_FALSE;

    cnf->use_hysteresis = DEF_USE_HYST;
    cnf->hysteresis_param.scaling = HYST_SCALING;
    cnf->hysteresis_param.thr_high = HYST_THRESHOLD_HIGH;
    cnf->hysteresis_param.thr_low = HYST_THRESHOLD_LOW;

    cnf->pollrate = DEF_POLLRATE;

    cnf->tc_redundancy = TC_REDUNDANCY;
    cnf->mpr_coverage = MPR_COVERAGE;
    cnf->lq_level = DEF_LQ_LEVEL;
    cnf->lq_fish = DEF_LQ_FISH;
    cnf->lq_dlimit = DEF_LQ_DIJK_LIMIT;
    cnf->lq_dinter = DEF_LQ_DIJK_INTER;
    cnf->lq_wsize = DEF_LQ_WSIZE;
    cnf->clear_screen = DEF_CLEAR_SCREEN;
}




struct if_config_options *
get_default_if_config()
{
  struct if_config_options *io = malloc(sizeof(struct if_config_options));
  struct in6_addr in6;
 
  memset(io, 0, sizeof(struct if_config_options));

  io->ipv6_addrtype = 1; /* XXX - FixMe */

  if(inet_pton(AF_INET6, OLSR_IPV6_MCAST_SITE_LOCAL, &in6) < 0)
    {
      fprintf(stderr, "Failed converting IP address %s\n", OLSR_IPV6_MCAST_SITE_LOCAL);
      return NULL;
    }
  memcpy(&io->ipv6_multi_site.v6, &in6, sizeof(struct in6_addr));

  if(inet_pton(AF_INET6, OLSR_IPV6_MCAST_GLOBAL, &in6) < 0)
    {
      fprintf(stderr, "Failed converting IP address %s\n", OLSR_IPV6_MCAST_GLOBAL);
      return NULL;
    }
  memcpy(&io->ipv6_multi_glbl.v6, &in6, sizeof(struct in6_addr));

  io->lq_mult = NULL;

  io->weight.fixed = OLSR_FALSE;
  io->weight.value = 0;

  io->ipv6_addrtype = 0; /* global */

  io->hello_params.emission_interval = HELLO_INTERVAL;
  io->hello_params.validity_time = -1.0;
  io->tc_params.emission_interval = TC_INTERVAL;
  io->tc_params.validity_time = TOP_HOLD_TIME;
  io->mid_params.emission_interval = MID_INTERVAL;
  io->mid_params.validity_time = MID_HOLD_TIME;
  io->hna_params.emission_interval = HNA_INTERVAL;
  io->hna_params.validity_time = HNA_HOLD_TIME;

  return io;

}



void
olsrd_print_cnf(struct olsrd_config *cnf)
{
  struct hna4_entry        *h4 = cnf->hna4_entries;
  struct hna6_entry        *h6 = cnf->hna6_entries;
  struct olsr_if           *in = cnf->interfaces;
  struct plugin_entry      *pe = cnf->plugins;
  struct ipc_host          *ih = cnf->ipc_hosts;
  struct ipc_net           *ie = cnf->ipc_nets;
  struct olsr_lq_mult      *mult;
  char ipv6_buf[100];             /* buffer for IPv6 inet_htop */
  struct in_addr in4;

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
  if(cnf->willingness_auto)
    printf("Willingness      : AUTO\n");
  else
    printf("Willingness      : %d\n", cnf->willingness);

  printf("IPC connections  : %d\n", cnf->ipc_connections);

  while(ih)
    {
      in4.s_addr = ih->host.v4;
      printf("\tHost %s\n", inet_ntoa(in4));
      ih = ih->next;
    }
  
  while(ie)
    {
      in4.s_addr = ie->net.v4;
      printf("\tNet %s/", inet_ntoa(in4));
      in4.s_addr = ie->mask.v4;
      printf("%s\n", inet_ntoa(in4));
      ie = ie->next;
    }


  printf("Pollrate         : %0.2f\n", cnf->pollrate);

  printf("TC redundancy    : %d\n", cnf->tc_redundancy);

  printf("MPR coverage     : %d\n", cnf->mpr_coverage);
   
  printf("LQ level         : %d\n", cnf->lq_level);

  printf("LQ fish eye      : %d\n", cnf->lq_fish);

  printf("LQ Dijkstra limit: %d, %0.2f\n", cnf->lq_dlimit, cnf->lq_dinter);

  printf("LQ window size   : %d\n", cnf->lq_wsize);

  printf("Clear screen     : %s\n", cnf->clear_screen ? "yes" : "no");

  /* Interfaces */
  if(in)
    {
      printf("Interfaces:\n");
      while(in)
	{
	  printf(" dev: \"%s\"\n", in->name);
	  
	  if(in->cnf->ipv4_broadcast.v4)
	    {
	      in4.s_addr = in->cnf->ipv4_broadcast.v4;
	      printf("\tIPv4 broadcast           : %s\n", inet_ntoa(in4));
	    }
	  else
	    {
	      printf("\tIPv4 broadcast           : AUTO\n");
	    }
	  
	  printf("\tIPv6 addrtype            : %s\n", in->cnf->ipv6_addrtype ? "site-local" : "global");
	  
	  //union olsr_ip_addr       ipv6_multi_site;
	  //union olsr_ip_addr       ipv6_multi_glbl;
	  printf("\tIPv6 multicast site/glbl : %s", (char *)inet_ntop(AF_INET6, &in->cnf->ipv6_multi_site.v6, ipv6_buf, sizeof(ipv6_buf)));
	  printf("/%s\n", (char *)inet_ntop(AF_INET6, &in->cnf->ipv6_multi_glbl.v6, ipv6_buf, sizeof(ipv6_buf)));
	  
	  printf("\tHELLO emission/validity  : %0.2f/%0.2f\n", in->cnf->hello_params.emission_interval, in->cnf->hello_params.validity_time);
	  printf("\tTC emission/validity     : %0.2f/%0.2f\n", in->cnf->tc_params.emission_interval, in->cnf->tc_params.validity_time);
	  printf("\tMID emission/validity    : %0.2f/%0.2f\n", in->cnf->mid_params.emission_interval, in->cnf->mid_params.validity_time);
	  printf("\tHNA emission/validity    : %0.2f/%0.2f\n", in->cnf->hna_params.emission_interval, in->cnf->hna_params.validity_time);
	  
          for (mult = in->cnf->lq_mult; mult != NULL; mult = mult->next)
          {
            inet_ntop(cnf->ip_version, &mult->addr, ipv6_buf,
                      sizeof (ipv6_buf));

            printf("\tLinkQualityMult          : %s %0.2f\n",
                   ipv6_buf, mult->val);
          }
	  
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
  if(cnf->use_hysteresis)
    {
      printf("Using hysteresis:\n");
      printf("\tScaling      : %0.2f\n", cnf->hysteresis_param.scaling);
      printf("\tThr high/low : %0.2f/%0.2f\n", cnf->hysteresis_param.thr_high, cnf->hysteresis_param.thr_low);
    }
  else
    printf("Not using hysteresis\n");

  /* HNA IPv4 */
  if(h4)
    {

      printf("HNA4 entries:\n");
      while(h4)
	{
	  in4.s_addr = h4->net.v4;
	  printf("\t%s/", inet_ntoa(in4));
	  in4.s_addr = h4->netmask.v4;
	  printf("%s\n", inet_ntoa(in4));

	  h4 = h4->next;
	}
    }

  /* HNA IPv6 */
  if(h6)
    {
      printf("HNA6 entries:\n");
      while(h6)
	{
	  printf("\t%s/%d\n", (char *)inet_ntop(AF_INET6, &h6->net.v6, ipv6_buf, sizeof(ipv6_buf)), h6->prefix_len);
	  h6 = h6->next;
	}
    }
}

void *olsrd_cnf_malloc(unsigned int len)
{
  return malloc(len);
}

void olsrd_cnf_free(void *addr)
{
  free(addr);
}

#if defined WIN32_STDIO_HACK
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
#else
void win32_stdio_hack(unsigned int handle) {}
#endif
