
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

#include "olsrd_conf.h"
#include "ipcalc.h"
#include "olsr_cfg.h"
#include "defs.h"
#include "net_olsr.h"
#include "olsr.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef linux
#include <linux/types.h>
#include <linux/rtnetlink.h>
#include <linux/version.h>
#endif

extern FILE *yyin;
extern int yyparse(void);

static char interface_defaults_name[] = "[InterfaceDefaults]";

const char *FIB_METRIC_TXT[] = {
  "flat",
  "correct",
  "approx",
};

const char *GW_UPLINK_TXT[] = {
  "none",
  "ipv4",
  "ipv6",
  "both"
};

const char *OLSR_IF_MODE[] = {
  "mesh",
  "ether"
};

static char copyright_string[] __attribute__ ((unused)) =
  "The olsr.org Optimized Link-State Routing daemon(olsrd) Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org) All rights reserved.";

int current_line;

/* Global stuff externed in defs.h */
FILE *debug_handle;                    /* Where to send debug(defaults to stdout) */
struct olsrd_config *olsr_cnf;         /* The global configuration */

#ifdef MAKEBIN

/* Build as standalone binary */
int
main(int argc, char *argv[])
{
  struct olsrd_config *cnf;

  if (argc == 1) {
    fprintf(stderr, "Usage: olsrd_cfgparser [filename] -print\n\n");
    exit(EXIT_FAILURE);
  }

  if ((cnf = olsrd_parse_cnf(argv[1])) != NULL) {
    if ((argc > 2) && (!strcmp(argv[2], "-print"))) {
      olsrd_print_cnf(cnf);
      olsrd_write_cnf(cnf, "./out.conf");
    } else
      printf("Use -print to view parsed values\n");
    printf("Configfile parsed OK\n");
  } else {
    printf("Failed parsing \"%s\"\n", argv[1]);
  }

  return 0;
}

#else

/* Build as part of olsrd */

#endif

int
olsrd_parse_cnf(const char *filename)
{
  struct olsr_if *in, *new_ifqueue;
  int rc;

  fprintf(stderr, "Parsing file: \"%s\"\n", filename);

  yyin = fopen(filename, "r");
  if (yyin == NULL) {
    fprintf(stderr, "Cannot open configuration file '%s': %s.\n", filename, strerror(errno));
    return -1;
  }

  current_line = 1;
  rc = yyparse();
  fclose(yyin);
  if (rc != 0) {
    return -1;
  }

  /* Reverse the queue (added by user request) */
  in = olsr_cnf->interfaces;
  new_ifqueue = NULL;

  while (in) {
    struct olsr_if *in_tmp = in;
    in = in->next;

    in_tmp->next = new_ifqueue;
    new_ifqueue = in_tmp;
  }

  olsr_cnf->interfaces = new_ifqueue;

  for (in = olsr_cnf->interfaces; in != NULL; in = in->next) {
    /* set various stuff */
    in->configured = false;
    in->interf = NULL;
    in->host_emul = false;
  }
  return 0;
}

/* prints an interface (and checks it againt the default config) and the inverted config */
static void
olsrd_print_interface_cnf(struct if_config_options *cnf, struct if_config_options *cnfi, bool defcnf)
{
  struct olsr_lq_mult *mult;
  int lq_mult_cnt = 0;
  char ipv6_buf[INET6_ADDRSTRLEN];                  /* buffer for IPv6 inet_htop */

  if (cnf->ipv4_multicast.v4.s_addr) {
    printf("\tIPv4 broadcast/multicast : %s%s\n", inet_ntoa(cnf->ipv4_multicast.v4),DEFAULT_STR(ipv4_multicast.v4.s_addr));
  } else {
    printf("\tIPv4 broadcast/multicast : AUTO%s\n",DEFAULT_STR(ipv4_multicast.v4.s_addr));
  }

  if (cnf->mode==IF_MODE_ETHER){
    printf("\tMode           : ether%s\n",DEFAULT_STR(mode));
  } else {
    printf("\tMode           : mesh%s\n",DEFAULT_STR(mode));
  }

  printf("\tIPv6 multicast           : %s%s\n", inet_ntop(AF_INET6, &cnf->ipv6_multicast.v6, ipv6_buf, sizeof(ipv6_buf)),DEFAULT_STR(ipv6_multicast.v6));

  printf("\tHELLO emission/validity  : %0.2f%s/%0.2f%s\n", cnf->hello_params.emission_interval, DEFAULT_STR(hello_params.emission_interval),
         cnf->hello_params.validity_time,DEFAULT_STR(hello_params.validity_time));
  printf("\tTC emission/validity     : %0.2f%s/%0.2f%s\n", cnf->tc_params.emission_interval, DEFAULT_STR(tc_params.emission_interval),
         cnf->tc_params.validity_time,DEFAULT_STR(tc_params.validity_time));
  printf("\tMID emission/validity    : %0.2f%s/%0.2f%s\n", cnf->mid_params.emission_interval, DEFAULT_STR(mid_params.emission_interval),
         cnf->mid_params.validity_time,DEFAULT_STR(mid_params.validity_time));
  printf("\tHNA emission/validity    : %0.2f%s/%0.2f%s\n", cnf->hna_params.emission_interval, DEFAULT_STR(hna_params.emission_interval),
         cnf->hna_params.validity_time,DEFAULT_STR(hna_params.validity_time));

  for (mult = cnf->lq_mult; mult != NULL; mult = mult->next) {
    lq_mult_cnt++;
    printf("\tLinkQualityMult          : %s %0.2f %s\n", inet_ntop(olsr_cnf->ip_version, &mult->addr, ipv6_buf, sizeof(ipv6_buf)),
      (float)(mult->value) / 65536.0, ((lq_mult_cnt > cnf->orig_lq_mult_cnt)?" (d)":""));
  }

  printf("\tAutodetect changes       : %s%s\n", cnf->autodetect_chg ? "yes" : "no",DEFAULT_STR(autodetect_chg));
}

#ifdef linux
static int olsrd_sanity_check_rtpolicy(struct olsrd_config *cnf) {
  int prio;

  /* calculate rt_policy defaults if necessary */
  if (!cnf->smart_gw_active) {
    /* default is "no policy rules" and "everything into the main table" */
    if (cnf->rt_table == DEF_RT_AUTO) {
      cnf->rt_table = 254;
    }
    if (cnf->rt_table_default == DEF_RT_AUTO) {
      cnf->rt_table_default = cnf->rt_table;
    }
    if (cnf->rt_table_tunnel != DEF_RT_AUTO) {
      fprintf(stderr, "Warning, setting a table for tunnels without SmartGW does not make sense.\n");
    }
    cnf->rt_table_tunnel = cnf->rt_table_default;

    /* priority rules default is "none" */
    if (cnf->rt_table_pri == DEF_RT_AUTO) {
      cnf->rt_table_pri = DEF_RT_NONE;
    }
    if (cnf->rt_table_defaultolsr_pri == DEF_RT_AUTO) {
      cnf->rt_table_defaultolsr_pri = DEF_RT_NONE;
    }
    if (cnf->rt_table_tunnel_pri == DEF_RT_AUTO) {
      cnf->rt_table_tunnel_pri = DEF_RT_NONE;
    }
    if (cnf->rt_table_default_pri == DEF_RT_AUTO) {
      cnf->rt_table_default_pri = DEF_RT_NONE;
    }
  }
  else {
    /* default is "policy rules" and "everything into separate tables (254, 223, 224)" */
    if (cnf->rt_table == DEF_RT_AUTO) {
      cnf->rt_table = 254;
    }
    if (cnf->rt_table_default == DEF_RT_AUTO) {
      cnf->rt_table_default = 223;
    }
    if (cnf->rt_table_tunnel == DEF_RT_AUTO) {
      cnf->rt_table_tunnel = 224;
    }

    /* default for "rt_table_pri" is none (main table already has a policy rule */
    prio = 32766;
    if (cnf->rt_table_pri > 0) {
      prio = cnf->rt_table_pri;
    }
    else if (cnf->rt_table_pri == DEF_RT_AUTO) {
      /* choose default */
      olsr_cnf->rt_table_pri = 0;
      fprintf(stderr, "No policy rule for rt_table_pri\n");
    }

    /* default for "rt_table_defaultolsr_pri" is +10 */
    prio += 10;
    if (cnf->rt_table_defaultolsr_pri > 0) {
      prio = cnf->rt_table_defaultolsr_pri;
    }
    else if (cnf->rt_table_defaultolsr_pri == DEF_RT_AUTO) {
      olsr_cnf->rt_table_defaultolsr_pri = prio;
      fprintf(stderr, "Choose priority %u for rt_table_defaultolsr_pri\n", prio);
    }

    prio += 10;
    if (cnf->rt_table_tunnel_pri > 0) {
      prio = cnf->rt_table_tunnel_pri;
    }
    else if (cnf->rt_table_tunnel_pri == DEF_RT_AUTO) {
      olsr_cnf->rt_table_tunnel_pri = prio;
      fprintf(stderr, "Choose priority %u for rt_table_tunnel_pri\n", prio);
    }

    prio += 10;
    if (cnf->rt_table_default_pri == DEF_RT_AUTO) {
      olsr_cnf->rt_table_default_pri = prio;
      fprintf(stderr, "Choose priority %u for rt_table_default_pri\n", prio);
    }
  }

  /* check rule priorities */
  if (cnf->rt_table_pri > 0) {
    if (cnf->rt_table >= 253) {
      fprintf(stderr, "rttable %d does not need policy rules from OLSRd\n", cnf->rt_table);
      return -1;
    }
  }

  prio = cnf->rt_table_pri;
  if (cnf->rt_table_defaultolsr_pri > 0) {
    if (prio >= cnf->rt_table_defaultolsr_pri) {
      fprintf(stderr, "rttable_defaultolsr priority must be greater than %d\n", prio);
      return -1;
    }
    if (cnf->rt_table_default >= 253) {
      fprintf(stderr, "rttable %d does not need policy rules from OLSRd\n", cnf->rt_table_default);
      return -1;
    }
    prio = olsr_cnf->rt_table_defaultolsr_pri;
  }

  if (cnf->rt_table_tunnel_pri > 0 ) {
    if (prio >= cnf->rt_table_tunnel_pri) {
      fprintf(stderr, "rttable_tunnel priority must be greater than %d\n", prio);
      return -1;
    }
    if (cnf->rt_table_tunnel >= 253) {
      fprintf(stderr, "rttable %d does not need policy rules from OLSRd\n", cnf->rt_table_tunnel);
      return -1;
    }
    prio = cnf->rt_table_tunnel_pri;
  }

  if (cnf->rt_table_default_pri > 0) {
    if (prio >= cnf->rt_table_default_pri) {
      fprintf(stderr, "rttable_default priority must be greater than %d\n", prio);
      return -1;
    }
    if (cnf->rt_table_default >= 253) {
      fprintf(stderr, "rttable %d does not need policy rules from OLSRd\n", cnf->rt_table_default);
      return -1;
    }
  }

  /* filter rt_proto entry */
  if (cnf->rt_proto == 1) {
    /* protocol 1 is reserved, so better use 0 */
    cnf->rt_proto = 0;
  }
  else if (cnf->rt_proto == 0) {
    cnf->rt_proto = RTPROT_BOOT;
  }
  return 0;
}

#endif


static 
int olsrd_sanity_check_interface_cnf(struct if_config_options * io, struct olsrd_config * cnf, char* name) {
  struct olsr_lq_mult *mult;

  /* HELLO interval */

  if (io->hello_params.validity_time < 0.0) {
    if (cnf->lq_level == 0)
      io->hello_params.validity_time = NEIGHB_HOLD_TIME;

    else
      io->hello_params.validity_time = (int)(REFRESH_INTERVAL / cnf->lq_aging);
  }

  if (io->hello_params.emission_interval < cnf->pollrate || io->hello_params.emission_interval > io->hello_params.validity_time) {
    fprintf(stderr, "Bad HELLO parameters! (em: %0.2f, vt: %0.2f) for dev %s\n", io->hello_params.emission_interval,
            io->hello_params.validity_time, name);
    return -1;
  }

  /* TC interval */
  if (io->tc_params.emission_interval < cnf->pollrate || io->tc_params.emission_interval > io->tc_params.validity_time) {
    fprintf(stderr, "Bad TC parameters! (em: %0.2f, vt: %0.2f) for dev %s\n", io->tc_params.emission_interval,
        io->tc_params.validity_time, name);
    return -1;
  }

  if (cnf->min_tc_vtime > 0.0 && (io->tc_params.validity_time / io->tc_params.emission_interval) < 128) {
    fprintf(stderr, "Please use a tc vtime at least 128 times the emission interval while using the min_tc_vtime hack.\n");
    return -1;
  }
  /* MID interval */
  if (io->mid_params.emission_interval < cnf->pollrate || io->mid_params.emission_interval > io->mid_params.validity_time) {
    fprintf(stderr, "Bad MID parameters! (em: %0.2f, vt: %0.2f) for dev %s\n", io->mid_params.emission_interval,
            io->mid_params.validity_time, name);
    return -1;
  }

  /* HNA interval */
  if (io->hna_params.emission_interval < cnf->pollrate || io->hna_params.emission_interval > io->hna_params.validity_time) {
    fprintf(stderr, "Bad HNA parameters! (em: %0.2f, vt: %0.2f) for dev %s\n", io->hna_params.emission_interval,
            io->hna_params.validity_time, name);
    return -1;
  }

  for (mult = io->lq_mult; mult; mult=mult->next) {
    if (mult->value > LINK_LOSS_MULTIPLIER) {
      struct ipaddr_str buf;

      fprintf(stderr, "Bad Linkquality multiplier ('%s' on IP %s: %0.2f)\n",
          name, olsr_ip_to_string(&buf, &mult->addr), (float)mult->value / (float)LINK_LOSS_MULTIPLIER);
      return -1;
    }
  }
  return 0;
}


int
olsrd_sanity_check_cnf(struct olsrd_config *cnf)
{
  struct olsr_if *in = cnf->interfaces;
  struct if_config_options *io;

  /* Debug level */
  if (cnf->debug_level < MIN_DEBUGLVL || cnf->debug_level > MAX_DEBUGLVL) {
    fprintf(stderr, "Debuglevel %d is not allowed\n", cnf->debug_level);
    return -1;
  }

  /* IP version */
  if (cnf->ip_version != AF_INET && cnf->ip_version != AF_INET6) {
    fprintf(stderr, "Ipversion %d not allowed!\n", cnf->ip_version);
    return -1;
  }

  /* TOS */
  if (cnf->tos > MAX_TOS) {
    fprintf(stderr, "TOS %d is not allowed\n", cnf->tos);
    return -1;
  }

  if (cnf->willingness_auto == false && (cnf->willingness > MAX_WILLINGNESS)) {
    fprintf(stderr, "Willingness %d is not allowed\n", cnf->willingness);
    return -1;
  }

  /* Hysteresis */
  if (cnf->use_hysteresis == true) {
    if (cnf->hysteresis_param.scaling < MIN_HYST_PARAM || cnf->hysteresis_param.scaling > MAX_HYST_PARAM) {
      fprintf(stderr, "Hyst scaling %0.2f is not allowed\n", cnf->hysteresis_param.scaling);
      return -1;
    }

    if (cnf->hysteresis_param.thr_high <= cnf->hysteresis_param.thr_low) {
      fprintf(stderr, "Hyst upper(%0.2f) thr must be bigger than lower(%0.2f) threshold!\n", cnf->hysteresis_param.thr_high,
              cnf->hysteresis_param.thr_low);
      return -1;
    }

    if (cnf->hysteresis_param.thr_high < MIN_HYST_PARAM || cnf->hysteresis_param.thr_high > MAX_HYST_PARAM) {
      fprintf(stderr, "Hyst upper thr %0.2f is not allowed\n", cnf->hysteresis_param.thr_high);
      return -1;
    }

    if (cnf->hysteresis_param.thr_low < MIN_HYST_PARAM || cnf->hysteresis_param.thr_low > MAX_HYST_PARAM) {
      fprintf(stderr, "Hyst lower thr %0.2f is not allowed\n", cnf->hysteresis_param.thr_low);
      return -1;
    }
  }

  /* Pollrate */
  if (cnf->pollrate < MIN_POLLRATE || cnf->pollrate > MAX_POLLRATE) {
    fprintf(stderr, "Pollrate %0.2f is not allowed\n", cnf->pollrate);
    return -1;
  }

  /* NIC Changes Pollrate */

  if (cnf->nic_chgs_pollrate < MIN_NICCHGPOLLRT || cnf->nic_chgs_pollrate > MAX_NICCHGPOLLRT) {
    fprintf(stderr, "NIC Changes Pollrate %0.2f is not allowed\n", cnf->nic_chgs_pollrate);
    return -1;
  }

  /* TC redundancy */
  if (cnf->tc_redundancy != 2) {
    fprintf(stderr, "Sorry, tc-redundancy 0/1 are not working on 0.5.6. "
        "It was discovered late in the stable tree development and cannot "
        "be solved without a difficult change in the dijkstra code. "
        "Feel free to contact the olsr-user mailinglist "
        "(http://www.olsr.org/?q=mailing-lists) to learn more "
        "about the problem. The next version of OLSR will have working "
        "tc-redundancy again.\n");
    return -1;
  }

  /* MPR coverage */
  if (cnf->mpr_coverage < MIN_MPR_COVERAGE || cnf->mpr_coverage > MAX_MPR_COVERAGE) {
    fprintf(stderr, "MPR coverage %d is not allowed\n", cnf->mpr_coverage);
    return -1;
  }

  /* Link Q and hysteresis cannot be activated at the same time */
  if (cnf->use_hysteresis == true && cnf->lq_level) {
    fprintf(stderr, "Hysteresis and LinkQuality cannot both be active! Deactivate one of them.\n");
    return -1;
  }

  /* Link quality level */
  if (cnf->lq_level != 0 && cnf->lq_level != 2) {
    fprintf(stderr, "LQ level %d is not allowed\n", cnf->lq_level);
    return -1;
  }

  /* Link quality window size */
  if (cnf->lq_level && (cnf->lq_aging < MIN_LQ_AGING || cnf->lq_aging > MAX_LQ_AGING)) {
    fprintf(stderr, "LQ aging factor %f is not allowed\n", cnf->lq_aging);
    return -1;
  }

  /* NAT threshold value */
  if (cnf->lq_level && (cnf->lq_nat_thresh < 0.1 || cnf->lq_nat_thresh > 1.0)) {
    fprintf(stderr, "NAT threshold %f is not allowed\n", cnf->lq_nat_thresh);
    return -1;
  }

#if defined linux
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
  if (cnf->ip_version == AF_INET6 && cnf->smart_gw_active) {
    fprintf(stderr, "Smart gateways are not supported for linux kernel 2.4 and ipv6\n");
    return -1;
  }
#endif

  /* this rtpolicy settings are also currently only used in Linux */
  if (olsrd_sanity_check_rtpolicy(cnf)) {
    return -1;
  }

#endif

  if (in == NULL) {
    fprintf(stderr, "No interfaces configured!\n");
    return -1;
  }

  if (cnf->min_tc_vtime < 0.0) {
    fprintf(stderr, "Error, negative minimal tc time not allowed.\n");
    return -1;
  }
  if (cnf->min_tc_vtime > 0.0) {
	  fprintf(stderr, "Warning, you are using the min_tc_vtime hack. We hope you know what you are doing... contact olsr.org otherwise.\n");
  }

  if (cnf->smart_gw_type >= GW_UPLINK_CNT) {
    fprintf(stderr, "Error, illegal gateway uplink type: %d\n", cnf->smart_gw_type);
    return -1;
  }
  if (cnf->smart_gw_downlink < MIN_SMARTGW_SPEED || cnf->smart_gw_downlink > MAX_SMARTGW_SPEED) {
    fprintf(stderr, "Error, bad gateway downlink speed: %d kbit/s (should be %d-%d)\n",
        cnf->smart_gw_downlink, MIN_SMARTGW_SPEED, MAX_SMARTGW_SPEED);
    return -1;
  }
  if (cnf->smart_gw_uplink < MIN_SMARTGW_SPEED || cnf->smart_gw_uplink > MAX_SMARTGW_SPEED) {
    fprintf(stderr, "Error, bad gateway uplink speed: %d kbit/s (should be %d-%d)\n",
        cnf->smart_gw_uplink, MIN_SMARTGW_SPEED, MAX_SMARTGW_SPEED);
    return -1;
  }

  if (cnf->interface_defaults == NULL) {
    /* get a default configuration if the user did not specify one */
    cnf->interface_defaults = get_default_if_config();
  } else {
    olsrd_print_interface_cnf(cnf->interface_defaults, cnf->interface_defaults, false);
    olsrd_sanity_check_interface_cnf(cnf->interface_defaults, cnf, interface_defaults_name);
  }

  /* Interfaces */
  while (in) {
    struct olsr_lq_mult *mult, *mult_orig;

    io = in->cnf;

    olsrd_print_interface_cnf(in->cnf, in->cnfi, false);

    /*apply defaults*/
    {
      size_t pos;
      struct olsr_lq_mult *mult_temp, *mult_orig_walk;
      uint8_t *cnfptr = (uint8_t*)in->cnf;
      uint8_t *cnfiptr = (uint8_t*)in->cnfi;
      uint8_t *defptr = (uint8_t*)cnf->interface_defaults;

      /*save interface specific lqmults, as they are merged togehter later*/
      mult_orig = io->lq_mult;

      assert(in->cnf);
      assert(in->cnfi);
      for (pos = 0; pos < sizeof(*in->cnf); pos++) {
        if (cnfptr[pos] != cnfiptr[pos]) {
          cnfptr[pos] = defptr[pos]; cnfiptr[pos]=0x00;
        }
        else cnfiptr[pos]=0xFF;
      }

      io->lq_mult=NULL;
      /*copy default lqmults into this interface*/
      for (mult = cnf->interface_defaults->lq_mult; mult; mult=mult->next) {
        /*search same lqmult in orig_list*/
        for (mult_orig_walk = mult_orig; mult_orig_walk; mult_orig_walk=mult_orig_walk->next) {
          if (ipequal(&mult_orig_walk->addr,&mult->addr)) {
            break;
          }
        }
        if (mult_orig_walk == NULL) {
          mult_temp=malloc(sizeof(struct olsr_lq_mult));
          memcpy(mult_temp,mult,sizeof(struct olsr_lq_mult));
          mult_temp->next=io->lq_mult;
          io->lq_mult=mult_temp;
        }
      }
    }

    if (in->name == NULL || !strlen(in->name)) {
      fprintf(stderr, "Interface has no name!\n");
      return -1;
    }

    if (io == NULL) {
      fprintf(stderr, "Interface %s has no configuration!\n", in->name);
      return -1;
    }

    /*merge lqmults*/
    if (mult_orig!=NULL) {
      io->orig_lq_mult_cnt=1;
      /*search last of interface specific lqmults*/
      mult = mult_orig;
      while (mult->next!=NULL) {
        mult=mult->next;
      }
      /*append default lqmults ath the end of the interface specific (to ensure they can overwrite them)*/
      mult->next=io->lq_mult;
      io->lq_mult=mult_orig;
    }

    if (olsrd_sanity_check_interface_cnf(io, cnf, in->name)) return -1;

    in = in->next;
  }

  return 0;
}

void
olsrd_free_cnf(struct olsrd_config *cnf)
{
  struct ip_prefix_list *hd, *h = cnf->hna_entries;
  struct olsr_if *ind, *in = cnf->interfaces;
  struct plugin_entry *ped, *pe = cnf->plugins;
  struct olsr_lq_mult *mult, *next_mult;

  while (h) {
    hd = h;
    h = h->next;
    free(hd);
  }

  while (in) {
    for (mult = in->cnf->lq_mult; mult != NULL; mult = next_mult) {
      next_mult = mult->next;
      free(mult);
    }

    free(in->cnf);
    free(in->cnfi);

    ind = in;
    in = in->next;

    free(ind);
  }

  while (pe) {
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
  cnf->no_fork = false;
  cnf->host_emul = false;
  cnf->ip_version = AF_INET;
  cnf->ipsize = sizeof(struct in_addr);
  cnf->maxplen = 32;
  cnf->allow_no_interfaces = DEF_ALLOW_NO_INTS;
  cnf->tos = DEF_TOS;
  cnf->olsrport = DEF_OLSRPORT;
  cnf->rt_proto = DEF_RTPROTO;
  cnf->rt_table = DEF_RT_AUTO;
  cnf->rt_table_default = DEF_RT_AUTO;
  cnf->rt_table_tunnel = DEF_RT_AUTO;
  cnf->rt_table_pri = DEF_RT_AUTO;
  cnf->rt_table_default_pri = DEF_RT_AUTO;
  cnf->rt_table_defaultolsr_pri = DEF_RT_AUTO;
  cnf->rt_table_tunnel_pri = DEF_RT_AUTO;

  cnf->willingness_auto = DEF_WILL_AUTO;
  cnf->willingness = DEF_WILLINGNESS;
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
  cnf->lq_aging = DEF_LQ_AGING;
  cnf->lq_algorithm = NULL;
  cnf->lq_nat_thresh = DEF_LQ_NAT_THRESH;
  cnf->clear_screen = DEF_CLEAR_SCREEN;

  cnf->del_gws = false;
  cnf->will_int = 10 * HELLO_INTERVAL;
  cnf->max_jitter = 0.0;
  cnf->exit_value = EXIT_SUCCESS;
  cnf->max_tc_vtime = 0.0;
  cnf->ioctl_s = 0;
  cnf->use_niit = DEF_USE_NIIT;
  cnf->niit4to6_if_index = 0;
  cnf->niit6to4_if_index = 0;

  cnf->smart_gw_active = DEF_SMART_GW;
  cnf->smart_gw_allow_nat = DEF_GW_ALLOW_NAT;
  cnf->smart_gw_type = DEF_GW_TYPE;
  cnf->smart_gw_uplink = DEF_UPLINK_SPEED;
  cnf->smart_gw_uplink_nat = DEF_GW_UPLINK_NAT;
  cnf->smart_gw_downlink = DEF_DOWNLINK_SPEED;

  cnf->use_src_ip_routes = DEF_USE_SRCIP_ROUTES;

#ifdef LINUX_NETLINK_ROUTING
  cnf->rtnl_s = 0;
#endif

#if defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __MacOSX__ || defined __NetBSD__ || defined __OpenBSD__
  cnf->rts = 0;
#endif
}

struct if_config_options *
get_default_if_config(void)
{
  struct if_config_options *io = malloc(sizeof(*io));

  if (io == NULL) {
    fprintf(stderr, "Out of memory %s\n", __func__);
    return NULL;
  }

  memset(io, 0, sizeof(*io));

  io->mode = DEF_IF_MODE;

  io->ipv6_multicast = ipv6_def_multicast;

  io->lq_mult = NULL;

  io->weight.fixed = false;
  io->weight.value = 0;

  io->hello_params.emission_interval = HELLO_INTERVAL;
  io->hello_params.validity_time = NEIGHB_HOLD_TIME;
  io->tc_params.emission_interval = TC_INTERVAL;
  io->tc_params.validity_time = TOP_HOLD_TIME;
  io->mid_params.emission_interval = MID_INTERVAL;
  io->mid_params.validity_time = MID_HOLD_TIME;
  io->hna_params.emission_interval = HNA_INTERVAL;
  io->hna_params.validity_time = HNA_HOLD_TIME;
  io->autodetect_chg = true;

  return io;

}

void
olsrd_print_cnf(struct olsrd_config *cnf)
{
  struct ip_prefix_list *h = cnf->hna_entries;
  struct olsr_if *in = cnf->interfaces;
  struct plugin_entry *pe = cnf->plugins;
  struct ip_prefix_list *ie = cnf->ipc_nets;

  printf(" *** olsrd configuration ***\n");

  printf("Debug Level      : %d\n", cnf->debug_level);
  if (cnf->ip_version == AF_INET6)
    printf("IpVersion        : 6\n");
  else
    printf("IpVersion        : 4\n");
  if (cnf->allow_no_interfaces)
    printf("No interfaces    : ALLOWED\n");
  else
    printf("No interfaces    : NOT ALLOWED\n");
  printf("TOS              : 0x%02x\n", cnf->tos);
  printf("OlsrPort          : 0x%03x\n", cnf->olsrport);
  printf("RtTable          : %u\n", cnf->rt_table);
  printf("RtTableDefault   : %u\n", cnf->rt_table_default);
  printf("RtTableTunnel    : %u\n", cnf->rt_table_tunnel);
  if (cnf->willingness_auto)
    printf("Willingness      : AUTO\n");
  else
    printf("Willingness      : %d\n", cnf->willingness);

  printf("IPC connections  : %d\n", cnf->ipc_connections);
  while (ie) {
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

  printf("LQ aging factor  : %f\n", cnf->lq_aging);

  printf("LQ algorithm name: %s\n", cnf->lq_algorithm ? cnf->lq_algorithm : "default");

  printf("NAT threshold    : %f\n", cnf->lq_nat_thresh);

  printf("Clear screen     : %s\n", cnf->clear_screen ? "yes" : "no");

  printf("Use niit         : %s\n", cnf->use_niit ? "yes" : "no");

  printf("Smart Gateway    : %s\n", cnf->smart_gw_active ? "yes" : "no");

  printf("SmGw. Allow NAT  : %s\n", cnf->smart_gw_allow_nat ? "yes" : "no");

  printf("Smart Gw. Uplink : %s\n", GW_UPLINK_TXT[cnf->smart_gw_type]);

  printf("SmGw. Uplink NAT : %s\n", cnf->smart_gw_uplink_nat ? "yes" : "no");

  printf("Smart Gw. speed  : %d kbit/s up, %d kbit/s down\n",
      cnf->smart_gw_uplink, cnf->smart_gw_downlink);

  if (olsr_cnf->smart_gw_prefix.prefix_len == 0) {
    printf("# Smart Gw. prefix : ::/0\n");
  }
  else {
    printf("Smart Gw. prefix : %s\n", olsr_ip_prefix_to_string(&cnf->smart_gw_prefix));
  }

  /* Interfaces */
  if (in) {
    /*print interface default config*/
    printf(" InterfaceDefaults: \n");
    olsrd_print_interface_cnf(cnf->interface_defaults, cnf->interface_defaults, true);

    while (in)
    { 
      if (cnf->interface_defaults!=in->cnf)
      {
        printf(" dev: \"%s\"\n", in->name);

        olsrd_print_interface_cnf(in->cnf, in->cnfi, false);
      }
      in = in->next;
    }
  }

  /* Plugins */
  if (pe) {
    printf("Plugins:\n");

    while (pe) {
      printf("\tName: \"%s\"\n", pe->name);
      pe = pe->next;
    }
  }

  /* Hysteresis */
  if (cnf->use_hysteresis) {
    printf("Using hysteresis:\n");
    printf("\tScaling      : %0.2f\n", cnf->hysteresis_param.scaling);
    printf("\tThr high/low : %0.2f/%0.2f\n", cnf->hysteresis_param.thr_high, cnf->hysteresis_param.thr_low);
  } else {
    printf("Not using hysteresis\n");
  }

  /* HNA IPv4 and IPv6 */
  if (h) {
    printf("HNA%d entries:\n", cnf->ip_version == AF_INET ? 4 : 6);
    while (h) {
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
struct ioinfo {
  unsigned int handle;
  unsigned char attr;
  char buff;
  int flag;
  CRITICAL_SECTION lock;
};

void
win32_stdio_hack(unsigned int handle)
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

void *
win32_olsrd_malloc(size_t size)
{
  return malloc(size);
}

void
win32_olsrd_free(void *ptr)
{
  free(ptr);
}
#endif

static void update_has_gateway_fields(void) {
  struct ip_prefix_list *h;

  olsr_cnf->has_ipv4_gateway = false;
  olsr_cnf->has_ipv6_gateway = false;

  for (h = olsr_cnf->hna_entries; h != NULL; h = h->next) {
    olsr_cnf->has_ipv4_gateway |= ip_prefix_is_v4_inetgw(&h->net) || ip_prefix_is_mappedv4_inetgw(&h->net);
    olsr_cnf->has_ipv6_gateway |= ip_prefix_is_v6_inetgw(&h->net);
  }
}

void
ip_prefix_list_add(struct ip_prefix_list **list, const union olsr_ip_addr *net, uint8_t prefix_len)
{
  struct ip_prefix_list *new_entry = malloc(sizeof(*new_entry));

  new_entry->net.prefix = *net;
  new_entry->net.prefix_len = prefix_len;

  /* Queue */
  new_entry->next = *list;
  *list = new_entry;

  /* update gateway flags */
  update_has_gateway_fields();
}

int
ip_prefix_list_remove(struct ip_prefix_list **list, const union olsr_ip_addr *net, uint8_t prefix_len)
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

      /* update gateway flags */
      update_has_gateway_fields();
      return 1;
    }
    prev = h;
    h = h->next;
  }
  return 0;
}

struct ip_prefix_list *
ip_prefix_list_find(struct ip_prefix_list *list, const union olsr_ip_addr *net, uint8_t prefix_len)
{
  struct ip_prefix_list *h;
  for (h = list; h != NULL; h = h->next) {
    if (prefix_len == h->net.prefix_len && ipequal(net, &h->net.prefix)) {
      return h;
    }
  }
  return NULL;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
