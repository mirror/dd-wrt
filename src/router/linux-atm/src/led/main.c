/* main.c - Do what ever a LANE client does */

/*
 * Marko Kiiskila carnil@cs.tut.fi 
 * 
 * Copyright (c) 1996
 * Tampere University of Technology - Telecommunications Laboratory
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this
 * software and its documentation is hereby granted,
 * provided that both the copyright notice and this
 * permission notice appear in all copies of the software,
 * derivative works or modified versions, and any portions
 * thereof, that both notices appear in supporting
 * documentation, and that the use of this software is
 * acknowledged in any publications resulting from using
 * the software.
 * 
 * TUT ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION AND DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS
 * SOFTWARE.
 * 
 */

/* Copyright (C) 1999 Heikki Vatiainen hessu@cs.tut.fi */

#if HAVE_CONFIG_H
#include <config.h>
#endif

/* Global includes */
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>

#include <atm.h>
#include <atmd.h>

#include <linux/atmlec.h>

/* Local incs */
#include "join.h"
#include "lec.h"
#include "address.h"
#include "display.h"
#include "kernel.h"

#define COMPONENT "main.c"

static void main_loop(void);
static int reset = 0;

void sig_reset(int a)
{
        reset = 1;  

        return;
}

static void usage(const char *progname)
{
  printf("Usage: %s [-c LECS_address | -s LES_address] [-e esi] [-n VLAN_name]"
    " [-m mesg_mask] [-l listen_address | selector] [-i interface_number]"
    " [-I physical_interface_number]"
    " [-t 1516|1580|4544|9234|18190] [-1] [-2] [-p] [-F logfile]"
    " [-f Fore specific name]\n", progname);
}

/*
 * My First C function (TM), hessu@cs.tut.fi
 */
static int esi_convert(char *parsestring, unsigned char *mac_addr)
{
        const char *hexchars = "abcdefABCDEF0123456789";
        char hexnum [17+1], curr;
        int i = 0, j = -1, hexindex = 0, tmp;
        char *k, *string;
        
        if (strchr(parsestring,'.') ||     /* do we have separators like */
            strchr(parsestring,':')) {     /* 00:20:22:23:04:05 */
                k = parsestring;
                for (i = 0; i < strlen(parsestring); i++) {
                        curr = *k;
                        if (curr == ':' || curr == '.') {   /* separator ? */
                                if (i - j == 3) {  /* there were 2 hex characters */
                                        ;
                                }
                                else if (i - j == 2) {  /* there was only 1 hex char */
                                        hexnum [hexindex] = hexnum [hexindex-1];
                                        hexnum [hexindex-1] = '0';
                                        hexindex +=1;
                                }
                                else   /* too many hexchars in a byte */
                                        return -1;
                                j = i;  /* j is the location of the last separator */
                        }
                        else if (strchr(hexchars, curr) == NULL) /* not a hexchar ? */
                                return -1;
                        else {  /* we have a hex character */
                                hexnum [hexindex] = curr;
                                hexindex +=1;
                        }
                        k++;
                }
                hexnum [hexindex] = '\0';
                string = hexnum;
        } else {   /* no separators */
                k = parsestring;
                while (*k != '\0') {
                        if (strchr(hexchars, *k) == NULL)
                                return -1;
                        k++;
                }
                
                string = parsestring;
        }
        
        /* the esi now looks like 002022230405 */
        i = strlen(string);
        if (i != 12)
                return -1;
        for(i=0; i<6; i++) {
                sscanf(&string[i*2], "%2x", &tmp);
                mac_addr[i] = (unsigned char)tmp;
        }
        return 0;
}

/* Tells kernel what our LEC_ID is.
 * Returns < 0 for serisous error
 */
static int set_lec_id(uint16_t lec_id)
{
        struct atmlec_msg msg;

        memset(&msg, 0, sizeof(struct atmlec_msg));
        msg.type = l_set_lecid;
        msg.content.normal.flag = lec_id;
        if (msg_to_kernel(&msg, sizeof(struct atmlec_msg)) < 0) {
                diag(COMPONENT, DIAG_ERROR, "Could not tell kernel LEC_ID");
                return -1;
        }

        return 0;
}

/* Tell kernel the parameters this ELAN has.
 * Returns < 0 for serious error
 */
static int config_kernel(void)
{
        struct atmlec_msg msg;

        memset(&msg, 0, sizeof(struct atmlec_msg));
        msg.type = l_config;
        msg.content.config.maximum_unknown_frame_count = lec_params.c10_max_unknown_frames;
        msg.content.config.max_unknown_frame_time = lec_params.c11_max_unknown_frame_time;
        msg.content.config.max_retry_count = lec_params.c13_max_retry_count;
        msg.content.config.aging_time = lec_params.c17_aging_time;
        msg.content.config.forward_delay_time = lec_params.c18_forward_delay_time;
        msg.content.config.arp_response_time = lec_params.c20_le_arp_response_time;
        msg.content.config.flush_timeout = lec_params.c21_flush_timeout;
        msg.content.config.path_switching_delay = lec_params.c22_path_switching_delay;
        msg.content.config.lane_version = (lec_params.c29_v2_capable) ? 2 : 1;
	msg.content.config.mtu = maxmtu2itfmtu(lec_params.c3_max_frame_size);
	msg.content.config.is_proxy = lec_params.c4_proxy_flag;

        if (msg_to_kernel(&msg, sizeof(struct atmlec_msg)) < 0) {
                diag(COMPONENT, DIAG_ERROR, "Could not tell kernel ELAN parameters");
                return -1;
        }

        return 0;
}

int main(int argc, char **argv)
{
        char mac_addr[ETH_ALEN];
        char elan_name[32 + 1];
        char preferred_les[ATM_ESA_LEN]; /* LANE2 */
        char foreId[255]; /* Max size for a TLV */
        char atm2textbuff[100];
        char esibuff[20];
        int esi_set = 0;
        int listen_addr_set = 0;
        int atm_set=0;
        int proxy_flag = 0;
        int lane_version = 0;              /* LANE2 */
        int max_frame_size = MTU_UNSPEC;
        int lecs_method = LECS_WELLKNOWN;
        int poll_ret = 0, itf = 0, phys_itf = 0, selector = 0;
        int daemon_flag = 0;
        pid_t pid;
        struct sockaddr_atmsvc manual_atm_addr;
        struct sockaddr_atmsvc listen_addr;
        char pidbuf[PATH_MAX + 1];
        int fd;
	int retval;
        
        memset(elan_name, '\0', sizeof(elan_name));
        memset(foreId, '\0', sizeof(foreId));
        memset(preferred_les, 0, ATM_ESA_LEN);
        memset(&manual_atm_addr, 0, sizeof(struct sockaddr_atmsvc));
        memset(&listen_addr, 0, sizeof(struct sockaddr_atmsvc));
        listen_addr.sas_family = AF_ATMSVC;

        set_application("zeppelin"); /* for debug msgs */

        while(poll_ret != -1) {
                poll_ret = getopt(argc, argv, "bc:e:n:s:m:l:i:I:q:12pf:t:F:");
                switch(poll_ret) {
                case 'b':
                        daemon_flag = 1;
                        break;
                case 'c':
                        if (atm_set) {
                                usage(argv[0]);
                                exit(-1);
                        }
                        if (text2atm(optarg, (struct sockaddr *)&manual_atm_addr,
                                     sizeof(struct sockaddr_atmsvc), T2A_NAME) < 0) {
                                diag(COMPONENT, DIAG_ERROR, "Invalid LECS address");
                                usage(argv[0]);
                                exit(-1);
                        }
                        atm2text(atm2textbuff, sizeof(atm2textbuff),
                                 (struct sockaddr *)&manual_atm_addr, 0);
                        diag(COMPONENT, DIAG_INFO, "LECS address: %s", atm2textbuff);
                        lecs_method = LECS_MANUAL;
                        atm_set=1;
                        break;
                case 'e':
                        if(esi_convert(optarg, mac_addr)<0) {
                                diag(COMPONENT, DIAG_ERROR, "Invalid ESI format");
                                usage(argv[0]);
                                exit(-1);
                        }
                        mac2text(esibuff, mac_addr);
                        diag(COMPONENT, DIAG_DEBUG, "LEC ESI:%s", esibuff);
                        esi_set=1;
                        break;
                case 'n':
                        if (strlen(optarg) > 32) {
                                diag(COMPONENT, DIAG_ERROR, "ELAN name too long");
                                exit(-1);
                        }
                        strcpy(elan_name, optarg);
                        diag(COMPONENT, DIAG_INFO, "Vlan name :'%s'", elan_name);
                        break;
                case 's':
                        if (atm_set) {
                                usage(argv[0]);
                                exit(-1);
                        }
                        if (text2atm(optarg, (struct sockaddr *)&manual_atm_addr,
                                     sizeof(struct sockaddr_atmsvc), T2A_NAME) < 0) {
                                diag(COMPONENT, DIAG_ERROR, "Invalid LES address");
                                usage(argv[0]);
                                exit(-1);
                        }
                        atm2text(atm2textbuff, sizeof(atm2textbuff),
                                 (struct sockaddr *)&manual_atm_addr, 0);
                        diag(COMPONENT, DIAG_INFO, "LES address: %s", atm2textbuff);
                        lecs_method = LECS_NONE;
                        atm_set=1;
                        break;
                case 'm':
                        set_verbosity(NULL, DIAG_DEBUG);
                        break;
                case 'l':
			if (isdigit(optarg[0]) && strlen(optarg) < 4 &&
			  sscanf(optarg, "%d", &selector) &&
			  selector >=0 && selector <= 0xff) {
				listen_addr.sas_addr.prv[ATM_ESA_LEN - 1] =
				  (char) selector;
				diag(COMPONENT, DIAG_INFO, "Selector byte set "
				  "to %d", selector);
			} else {
                                if (text2atm(optarg, (struct sockaddr *)&listen_addr,
                                             sizeof(struct sockaddr_atmsvc), T2A_NAME) < 0) {
                                        diag(COMPONENT, DIAG_ERROR, "Invalid ATM listen address");
                                        usage(argv[0]);
                                        exit(-1);
                                }
                                listen_addr_set = 1;
                        }
                        break;
                case 'i':
                        if (sscanf(optarg, "%d", &itf) <= 0 || itf >= MAX_LEC_ITF) {
                                diag(COMPONENT, DIAG_ERROR, "Invalid interface number");
                                usage(argv[0]);
                                exit(-1);
                        }
                        diag(COMPONENT, DIAG_INFO, "Interface number set to %d", itf);
                        break;
                case 'I':
                        if (sscanf(optarg, "%d", &phys_itf) <= 0 || phys_itf < 0) {
                                diag(COMPONENT, DIAG_ERROR, "Invalid physical interface number");
                                usage(argv[0]);
                                exit(-1);
                        }
                        diag(COMPONENT, DIAG_INFO, "Physical interface number set to %d", phys_itf);
                        break;
                case 'q':
#if 0
                        if (text2qos(optarg,NULL,0) < 0) {
                                diag(COMPONENT, DIAG_ERROR, "Invalid QOS specification");
                                usage(argv[0]);
                                exit(-1);
                        }
                        qos_spec = optarg;
#endif
                        diag(COMPONENT, DIAG_INFO, "-q is deprecated, ignoring it");
                        break;
                case '1':
                        lane_version = 1;
                        break;
                case '2':
                        lane_version = 2;
                        break;
                case 'p':
                        proxy_flag = 1;
                        break;
                case 'f':
                        if (strlen(optarg) > 255) {
                                diag(COMPONENT, DIAG_ERROR, "foreId too long");
                                exit(-1);
                        }
                        memcpy (foreId, optarg, strlen(optarg));
                        foreId[strlen(optarg)] = '\0';
                        diag(COMPONENT, DIAG_INFO, "foreId :'%s'", foreId);
                        break;
                case 't':	/* ERIC */
                        if( !strncmp( optarg, "1516", 4 )) max_frame_size = MTU_1516;
                        else if( !strncmp( optarg, "1580", 4 )) max_frame_size = MTU_1580;
                        else if( !strncmp( optarg, "4544", 4 )) max_frame_size = MTU_4544;
                        else if( !strncmp( optarg, "9234", 4 )) max_frame_size = MTU_9234;
                        else if( !strncmp( optarg, "18190", 5 )) max_frame_size = MTU_18190;
                        break;
                case 'F':
                        set_logfile(optarg);
                        diag(COMPONENT, DIAG_DEBUG, "logfile set to %s", optarg);
                        break;
                case -1:
                        break;
                default:
                        usage(argv[0]);
                        exit(-1);
                }
        }
        if (argc != optind) {
                usage(argv[0]);
                exit(1);
        }
        if (lane_version == 1 && max_frame_size == MTU_1580) {
                diag(COMPONENT, DIAG_ERROR, "MTU 1580 not defined with LANEv1");
                exit(-1);
        }

        /* Reserve signals */
        signal(SIGHUP, sig_reset);
        signal(SIGPIPE, SIG_IGN);

	if (!esi_set) {
		if(addr_getesi(mac_addr, phys_itf) < 0) {
			diag(COMPONENT, DIAG_ERROR, "Can't get ESI from kernel!");
			return -1;
		}
		mac2text(esibuff, mac_addr);
		diag(COMPONENT, DIAG_DEBUG, "LEC ESI:%s", esibuff);

		if (itf != 0)
			mac_addr[0] = 0x2 | ((itf - 1) << 2);
	}
                
	if ((itf = kernel_init(mac_addr, itf)) < 0 ) {
		diag(COMPONENT, DIAG_FATAL, "Kernel interface creation failed, exiting...");
		return -1;
	} 

	if (daemon_flag == 1) {
		daemon_flag = 0;
		pid = fork();
		if (pid < 0) {
			diag(COMPONENT, DIAG_FATAL, "fork failed, exiting...");
			return -1;
		}
		if (pid) {
			/* parent */
			return 0;
		} else {
			/* child */
			if (setsid() < 0) {
				diag(COMPONENT, DIAG_FATAL, "setsid failed, exiting...");
				return -1;
			}
		}
	}

	sprintf(pidbuf, "/var/run/lec%d.pid", itf);
	fd = open(pidbuf, O_CREAT | O_WRONLY, 0600);
	if (fd < 0) {
		diag(COMPONENT, DIAG_FATAL, "open(%s, ..) failed, %s", pidbuf, strerror(errno));
		return -1;
	}
	sprintf(pidbuf, "%d\n", getpid());
	write(fd, pidbuf, strlen(pidbuf));
	close(fd);

        /* Loop here until the Sun gets cold */
        while (1) {
                if (!listen_addr_set) {
                        char sel = listen_addr.sas_addr.prv[ATM_ESA_LEN - 1];
                        if (get_listenaddr(listen_addr.sas_addr.prv, phys_itf) < 0) {
                                diag(COMPONENT, DIAG_FATAL, "Could not figure out my ATM address");
                                exit(-1);
                        }
                        listen_addr.sas_addr.prv[ATM_ESA_LEN - 1] = sel;
                }

                atm2text(atm2textbuff, sizeof(atm2textbuff),
                         (struct sockaddr *)&listen_addr, A2T_NAME | A2T_PRETTY | A2T_LOCAL);
                diag(COMPONENT, DIAG_INFO, "Our ATM address: %s", atm2textbuff);

                diag(COMPONENT, DIAG_DEBUG, "initializing lec parameters");
                init_lec_params(mac_addr, elan_name, listen_addr.sas_addr.prv,
                                itf, foreId, max_frame_size, proxy_flag, lane_version);

		if (lecs_method != LECS_MANUAL && lecs_method != LECS_NONE) {
			diag(COMPONENT, DIAG_DEBUG, "trying to get LECS address from ILMI");
			/* Not sure why this memset is necessary */
			memset(&manual_atm_addr, 0, sizeof(struct sockaddr_atmsvc));
			retval = get_lecsaddr(phys_itf, &manual_atm_addr);
			if (retval <= 0) {
				diag(COMPONENT, DIAG_DEBUG,
				     "get_lecsaddr failed; not enough "
				     "memory allocated for all addresses "
				     "or no LECS address registered");
			} else {
				diag(COMPONENT, DIAG_DEBUG, "obtained LECS address from ILMI");
				lecs_method = LECS_FROM_ILMI;
			}
		}

                diag(COMPONENT, DIAG_DEBUG, "About to connect LECS");
                if (lec_configure(lecs_method, &manual_atm_addr, &listen_addr) < 0) {
                        close_connections();
                        random_delay();
                        continue;
                }
                diag(COMPONENT, DIAG_DEBUG, "About to connect LES");
                if (les_connect(lecs_method, &manual_atm_addr, &listen_addr) < 0) {
                        close_connections();
                        random_delay();
                        continue;
                }
                diag(COMPONENT, DIAG_DEBUG, "About to connect BUS");
                if (bus_connect() < 0) {
                        close_connections();
                        random_delay();
                        continue;
                }
                diag(COMPONENT, DIAG_DEBUG, "About to create data direct listen socket");
                if (create_data_listen() < 0) {
                        close_connections();
                        random_delay();
                        continue;
                }
                diag(COMPONENT, DIAG_DEBUG, "About to tell kernel our LEC_ID %d", lec_params.c14_lec_id);
                if (set_lec_id(lec_params.c14_lec_id) < 0) {
                        close_connections();
                        continue;
                }
                diag(COMPONENT, DIAG_DEBUG, "About to tell kernel LEC parameters");
                if (config_kernel() < 0) {
                        close_connections();
                        continue;
                }

                diag(COMPONENT, DIAG_DEBUG, "Joined ELAN '%s' successfully", lec_params.c5_elan_name);

                main_loop();
                diag(COMPONENT, DIAG_INFO, "Resetting...");
                close_connections();
                random_delay();

                reset = 0;
        }

        return 0; /* not reached */
}

/* zeppelin loops here when it is in operational state.  The check
 * against reset variable is probably not needed since select() will
 * return < 0 when a signal interrupts it.
 */
static void main_loop(void)
{
        fd_set rfds, cfds;
        int retval, ret1, ret2, ret3;

        while(!reset) {
                retval = ret1 = ret2 = ret3 = 0;
                FD_ZERO(&rfds);
                conn_get_fds(&rfds);
                FD_ZERO(&cfds);
                conn_get_connecting_fds(&cfds);

                retval = select(FD_SETSIZE, &rfds, &cfds, NULL, NULL);
                diag(COMPONENT, DIAG_DEBUG, "main_loop: select returned %d", retval);
                if (retval < 0) {
                        diag(COMPONENT, DIAG_ERROR, "main_loop: select: %s", strerror(errno));
                        break; /* leave main_loop */
                }
                if (retval == 0) {
                        /* Timeout, funny, since we have no timers */
                        continue;
                }
                if (FD_ISSET(lec_params.kernel->fd, &rfds)) {
                        ret1 = msg_from_kernel();
                        FD_CLR(lec_params.kernel->fd, &rfds);
                }
                ret2 = complete_connections(&cfds);
                ret3 = check_connections(&rfds);
                
                if (ret1 < 0 || ret2 < 0 || ret3 < 0)
                        break; /* leave main_loop */
        }
        
        diag(COMPONENT, DIAG_DEBUG, "exiting main_loop");

        return;
}
