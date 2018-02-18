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

#include "cli.h"

#include "builddata.h"

#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

#ifdef _WIN32
#include <winbase.h>
void ListInterfaces(void);
#endif /* _WIN32 */

void print_version(void) {
  printf("\n *** %s ***\n Build date: %s on %s\n http://www.olsr.org\n\n",
      olsrd_version, build_date, build_host);
}

/**
 * Print the command line usage
 */
void print_usage(bool error) {
  fprintf(
      stderr,
        "%s"
        "usage: olsrd [-f <configfile>] [ -i interface1 interface2 ... ]\n"
        "  [-d <debug_level>] [-ipv6] [-multi <IPv6 multicast address>]\n"
        "  [-lql <LQ level>] [-lqw <LQ winsize>] [-lqnt <nat threshold>]\n"
        "  [-bcast <broadcastaddr>] [-ipc] [-delgw]\n"
        "  [-hint <hello interval (secs)>] [-tcint <tc interval (secs)>]\n"
        "  [-midint <mid interval (secs)>] [-hnaint <hna interval (secs)>]\n"
        "  [-T <Polling Rate (secs)>] [-nofork] [-hemu <ip_address>]\n"
        "  [-lql <LQ level>] [-lqa <LQ aging factor>]\n"
        "  [-pidfile <pid file>]\n",
        error ? "Error in command line parameters!\n" : "");
}

#define NEXT_ARG do { argv++;argc--; } while (0)
#define CHECK_ARGC do { if(!argc) { \
     argv--; \
     snprintf(buf, sizeof(buf), "%s: You must provide a parameter when using the %s switch", __func__, *argv); \
     *error = buf; return EXIT_FAILURE; \
     } } while (0)

static char buf[FILENAME_MAX + 1024];

/**
 * Process command line arguments passed to olsrd
 *
 */
int olsr_process_arguments(int argc, char *argv[], struct olsrd_config *cnf, struct if_config_options *ifcnf, char ** error) {
  *error = NULL;
  memset(buf, 0, sizeof(buf));

  while (argc > 1) {
    NEXT_ARG
    ;
#ifdef _WIN32
    /*
     *Interface list
     */
    if (strcmp(*argv, "-int") == 0) {
      ListInterfaces();
      return 1;
    }
#endif /* _WIN32 */

    /*
     *Configfilename
     */
    if (strcmp(*argv, "-f") == 0) {
      snprintf(buf, sizeof(buf), "%s: Configfilename must ALWAYS be first argument", __func__);
      *error = buf;
      return EXIT_FAILURE;
    }

    /*
     *Use IP version 6
     */
    if (strcmp(*argv, "-ipv6") == 0) {
      cnf->ip_version = AF_INET6;
      continue;
    }

    /*
     *Broadcast address
     */
    if (strcmp(*argv, "-bcast") == 0) {
      struct in_addr in;
      NEXT_ARG
      ;
      CHECK_ARGC;

      if (inet_pton(AF_INET, *argv, &in) == 0) {
        printf("Invalid broadcast address! %s\nSkipping it!\n", *argv);
        continue;
      }
      memcpy(&ifcnf->ipv4_multicast.v4, &in.s_addr, sizeof(ifcnf->ipv4_multicast.v4));
      continue;
    }

    /*
     * Set LQ level
     */
    if (strcmp(*argv, "-lql") == 0) {
      int tmp_lq_level;
      NEXT_ARG
      ;
      CHECK_ARGC;

      /* Sanity checking is done later */
      sscanf(*argv, "%d", &tmp_lq_level);
      olsr_cnf->lq_level = tmp_lq_level;
      continue;
    }

    /*
     * Set LQ winsize
     */
    if (strcmp(*argv, "-lqa") == 0) {
      float tmp_lq_aging;
      NEXT_ARG
      ;
      CHECK_ARGC;

      sscanf(*argv, "%f", &tmp_lq_aging);

      if (tmp_lq_aging < (float) MIN_LQ_AGING || tmp_lq_aging > (float) MAX_LQ_AGING) {
        snprintf(buf, sizeof(buf), "%s: LQ aging factor %f not allowed. Range [%f-%f]", __func__, (double) tmp_lq_aging, (double) MIN_LQ_AGING,
            (double) MAX_LQ_AGING);
        *error = buf;
        return EXIT_FAILURE;
      }
      olsr_cnf->lq_aging = tmp_lq_aging;
      continue;
    }

    /*
     * Set NAT threshold
     */
    if (strcmp(*argv, "-lqnt") == 0) {
      float tmp_lq_nat_thresh;
      NEXT_ARG
      ;
      CHECK_ARGC;

      sscanf(*argv, "%f", &tmp_lq_nat_thresh);

      if (tmp_lq_nat_thresh < 0.1f || tmp_lq_nat_thresh > 1.0f) {
        snprintf(buf, sizeof(buf), "%s: NAT threshold %f not allowed. Range [%f-%f]", __func__, (double) tmp_lq_nat_thresh, (double) 0.1, (double) 1.0);
        *error = buf;
        return EXIT_FAILURE;
      }
      olsr_cnf->lq_nat_thresh = tmp_lq_nat_thresh;
      continue;
    }

    /*
     * Enable additional debugging information to be logged.
     */
    if (strcmp(*argv, "-d") == 0) {
      NEXT_ARG
      ;
      CHECK_ARGC;

      sscanf(*argv, "%d", &cnf->debug_level);
      continue;
    }

    /*
     * Interfaces to be used by olsrd.
     */
    if (strcmp(*argv, "-i") == 0) {
      NEXT_ARG
      ;
      CHECK_ARGC;

      if (*argv[0] == '-') {
        snprintf(buf, sizeof(buf), "%s: You must provide an interface label", __func__);
        *error = buf;
        return EXIT_FAILURE;
      }
      printf("Queuing if %s\n", *argv);
      olsr_create_olsrif(*argv, false);

      while ((argc - 1) && (argv[1][0] != '-')) {
        NEXT_ARG
        ;
        printf("Queuing if %s\n", *argv);
        olsr_create_olsrif(*argv, false);
      }

      continue;
    }
    /*
     * Set the hello interval to be used by olsrd.
     *
     */
    if (strcmp(*argv, "-hint") == 0) {
      NEXT_ARG
      ;
      CHECK_ARGC;
      sscanf(*argv, "%f", &ifcnf->hello_params.emission_interval);
      ifcnf->hello_params.validity_time = ifcnf->hello_params.emission_interval * 3;
      continue;
    }

    /*
     * Set the HNA interval to be used by olsrd.
     *
     */
    if (strcmp(*argv, "-hnaint") == 0) {
      NEXT_ARG
      ;
      CHECK_ARGC;
      sscanf(*argv, "%f", &ifcnf->hna_params.emission_interval);
      ifcnf->hna_params.validity_time = ifcnf->hna_params.emission_interval * 3;
      continue;
    }

    /*
     * Set the MID interval to be used by olsrd.
     *
     */
    if (strcmp(*argv, "-midint") == 0) {
      NEXT_ARG
      ;
      CHECK_ARGC;
      sscanf(*argv, "%f", &ifcnf->mid_params.emission_interval);
      ifcnf->mid_params.validity_time = ifcnf->mid_params.emission_interval * 3;
      continue;
    }

    /*
     * Set the tc interval to be used by olsrd.
     *
     */
    if (strcmp(*argv, "-tcint") == 0) {
      NEXT_ARG
      ;
      CHECK_ARGC;
      sscanf(*argv, "%f", &ifcnf->tc_params.emission_interval);
      ifcnf->tc_params.validity_time = ifcnf->tc_params.emission_interval * 3;
      continue;
    }

    /*
     * Set the polling interval to be used by olsrd.
     */
    if (strcmp(*argv, "-T") == 0) {
      NEXT_ARG
      ;
      CHECK_ARGC;
      sscanf(*argv, "%f", &cnf->pollrate);
      continue;
    }

    /*
     * Should we set up and send on a IPC socket for the front-end?
     */
    if (strcmp(*argv, "-ipc") == 0) {
      cnf->ipc_connections = 1;
      continue;
    }

    /*
     * IPv6 multicast addr
     */
    if (strcmp(*argv, "-multi") == 0) {
      struct in6_addr in6;
      NEXT_ARG
      ;
      CHECK_ARGC;
      if (inet_pton(AF_INET6, *argv, &in6) <= 0) {
        snprintf(buf, sizeof(buf), "Failed converting IP address %s", *argv);
        *error = buf;
        return EXIT_FAILURE;
      }

      memcpy(&ifcnf->ipv6_multicast, &in6, sizeof(struct in6_addr));

      continue;
    }

    /*
     * Host emulation
     */
    if (strcmp(*argv, "-hemu") == 0) {
      struct in_addr in;
      struct olsr_if *ifa;

      NEXT_ARG
      ;
      CHECK_ARGC;
      if (inet_pton(AF_INET, *argv, &in) <= 0) {
        snprintf(buf, sizeof(buf), "Failed converting IP address %s", *argv);
        *error = buf;
        return EXIT_FAILURE;
      }
      /* Add hemu interface */

      ifa = olsr_create_olsrif("hcif01", true);

      if (!ifa)
        continue;

      ifa->cnf = get_default_if_config();
      ifa->host_emul = true;
      memset(&ifa->hemu_ip, 0, sizeof(ifa->hemu_ip));
      memcpy(&ifa->hemu_ip, &in, sizeof(in));
      cnf->host_emul = true;

      continue;
    }

    /*
     * Delete possible default GWs
     */
    if (strcmp(*argv, "-delgw") == 0) {
      olsr_cnf->del_gws = true;
      continue;
    }

    if (strcmp(*argv, "-nofork") == 0) {
      cnf->no_fork = true;
      continue;
    }

    if (strcmp(*argv, "-pidfile") == 0) {
      NEXT_ARG
      ;
      CHECK_ARGC;

      cnf->pidfile = *argv;
      continue;
    }

    print_usage(true);
    return EXIT_FAILURE;
  }

  return 0;
}
