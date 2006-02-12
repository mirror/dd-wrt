/* dhcpd.c
 *
 * udhcp Server
 * Copyright (C) 1999 Matthew Ramsay <matthewr@moreton.com.au>
 *			Chris Trew <ctrew@moreton.com.au>
 *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>

#include "debug.h"
#include "dhcpd.h"
#include "arpping.h"
#include "socket.h"
#include "options.h"
#include "files.h"
#include "leases.h"
#include "packet.h"
#include "serverpacket.h"
#include "pidfile.h"
#include "get_time.h"


/* globals */
struct dhcpOfferedAddr *leases;
struct server_config_t server_config;
static int signal_pipe[2];

/* Exit and cleanup */
static void exit_server(int retval)
{
	pidfile_delete(server_config.pidfile);
	CLOSE_LOG();
	exit(retval);
}


/* Signal handler */
static void signal_handler(int sig)
{
	if (send(signal_pipe[1], &sig, sizeof(sig), MSG_DONTWAIT) < 0) {
		LOG(LOG_ERR, "Could not send signal: %s", 
			strerror(errno));
	}
}


#ifdef COMBINED_BINARY	
int udhcpd_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{	
	fd_set rfds;
	struct timeval tv;
	int server_socket = -1;
	int bytes, retval;
	struct dhcpMessage packet;
	unsigned char *state;
	unsigned char *server_id, *requested, *hostname;
	u_int32_t server_id_align, requested_align;
	unsigned long timeout_end;
	struct option_set *option;
	struct dhcpOfferedAddr *lease;
	int pid_fd;
	int max_sock;
	int sig;

	/* DD-WRT (belanger) : ignore signals until we're ready */
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	OPEN_LOG("udhcpd");
	LOG(LOG_INFO, "udhcp server (v%s) started", VERSION);

	memset(&server_config, 0, sizeof(struct server_config_t));

	if (argc < 2)
		read_config(DHCPD_CONF_FILE);
	else 
		read_config(argv[1]);

	pid_fd = pidfile_acquire(server_config.pidfile);
	pidfile_write_release(pid_fd);

	if ((option = find_option(server_config.options, DHCP_LEASE_TIME))) {
		memcpy(&server_config.lease, option->data + 2, 4);
		server_config.lease = ntohl(server_config.lease);
	}
	else server_config.lease = LEASE_TIME;

	leases = malloc(sizeof(struct dhcpOfferedAddr) * server_config.max_leases);
	memset(leases, 0, sizeof(struct dhcpOfferedAddr) * server_config.max_leases);
	read_leases(server_config.lease_file);
	read_statics(server_config.statics_file);

	/* DD-WRT (belanger) : write leases now */
	write_leases();

	if (read_interface(server_config.interface, &server_config.ifindex,
			   &server_config.server, server_config.arp) < 0)
		exit_server(1);

#ifndef DEBUGGING
	pid_fd = pidfile_acquire(server_config.pidfile); /* hold lock during fork. */
	if (daemon(0, 0) == -1) {
		perror("fork");
		exit_server(1);
	}
	pidfile_write_release(pid_fd);
#endif


	socketpair(AF_UNIX, SOCK_STREAM, 0, signal_pipe);
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGHUP, signal_handler);

	timeout_end = get_time(0) + server_config.auto_time;
	while(1) { /* loop until universe collapses */

		if (server_socket < 0)
			if ((server_socket = listen_socket(INADDR_ANY, SERVER_PORT, server_config.interface)) < 0) {
				LOG(LOG_ERR, "FATAL: couldn't create server socket, %s", strerror(errno));
				exit_server(0);
			}			

		FD_ZERO(&rfds);
		FD_SET(server_socket, &rfds);
		FD_SET(signal_pipe[0], &rfds);
		if (server_config.auto_time) {
			tv.tv_sec = timeout_end - get_time(0);
			tv.tv_usec = 0;
		}
		if (!server_config.auto_time || tv.tv_sec > 0) {
			max_sock = server_socket > signal_pipe[0] ? server_socket : signal_pipe[0];
			retval = select(max_sock + 1, &rfds, NULL, NULL, 
					server_config.auto_time ? &tv : NULL);
		} 
		else 
			retval = 0; /* If we already timed out, fall through */

		if (retval == 0) {
			write_leases();
			timeout_end = get_time(0) + server_config.auto_time;
			continue;
		} 
		else if (retval < 0 && errno != EINTR) {
			DEBUG(LOG_INFO, "error on select");
			continue;
		}
		
		if (FD_ISSET(signal_pipe[0], &rfds)) {
			if (read(signal_pipe[0], &sig, sizeof(sig)) < 0)
				continue; /* probably just EINTR */
			switch (sig) {
			case SIGUSR1:
				LOG(LOG_INFO, "Received a SIGUSR1");
				write_leases();
				/* why not just reset the timeout, eh */
				timeout_end = get_time(0) + server_config.auto_time;
				continue;
			case SIGHUP:
				LOG(LOG_INFO, "Received a SIGHUP");
				read_leases(server_config.lease_file);
				read_statics(server_config.statics_file);
				continue;
			case SIGTERM:
				LOG(LOG_INFO, "Received a SIGTERM");
				exit_server(0);
			}
		}

		if ((bytes = get_packet(&packet, server_socket)) < 0) { /* this waits for a packet - idle */
			if (bytes == -1 && errno != EINTR) {
				DEBUG(LOG_INFO, "error on read, %s, reopening socket", strerror(errno));
				close(server_socket);
				server_socket = -1;
			}
			continue;
		}

		if ((state = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) {
			DEBUG(LOG_ERR, "couldn't get option from packet, ignoring");
			continue;
		}

		hostname = get_option(&packet, DHCP_HOST_NAME);

		/* ADDME: look for a static lease */
		    
		/* If a hostname is supplied, and that hostname is a static lease, and that
		   static lease has an FF:FF:FF:FF:FF:FF MAC address, then use that entry. */
		if ( NULL == hostname ||
		     NULL == (lease = find_lease_by_hostname(hostname)) || 
		     (lease->expires != EXPIRES_NEVER) ||
		     0 != memcmp(lease->chaddr, MAC_BCAST_ADDR, strlen(MAC_BCAST_ADDR))) {
		  
		  /* Otherwise, look up the table using the supplied MAC address. */
		  lease = find_lease_by_chaddr(packet.chaddr);

		}

		switch (state[0]) {
		case DHCPDISCOVER:
			LOG(LOG_INFO,"received DISCOVER from %02x:%02x:%02x:%02x:%02x:%02x",
			    packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], 
			    packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]);	// modify by honor
			
			if (sendOffer(&packet, lease) < 0) {
				LOG(LOG_ERR, "send OFFER failed");
			}
			break;			
 		case DHCPREQUEST:
			requested = get_option(&packet, DHCP_REQUESTED_IP);
			server_id = get_option(&packet, DHCP_SERVER_ID);

			if (requested) memcpy(&requested_align, requested, 4);
			if (server_id) memcpy(&server_id_align, server_id, 4);
		
			if (requested) {
			  struct in_addr addr;
			  addr.s_addr = requested_align;
			  LOG(LOG_INFO, "received REQUEST for %s from %02x:%02x:%02x:%02x:%02x:%02x",
			      inet_ntoa(addr),
			      packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], 
			      packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]);
			}
			else {
			  LOG(LOG_INFO, "received REQUEST from %02x:%02x:%02x:%02x:%02x:%02x",
			      packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], 
			      packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]);
			}

			if (lease) { /*ADDME: or static lease */
				if (server_id) {
					/* SELECTING State */
					DEBUG(LOG_INFO, "server_id = %08x", ntohl(server_id_align));
					if (server_id_align == server_config.server && requested && 
					    requested_align == lease->yiaddr) {
						sendACK(&packet, lease->yiaddr);
					}
					else
						sendNAK(&packet); //Sveasoft - shouldn't we let them know we don't like the request?
				} 
				else {
					if (requested) {
						/* INIT-REBOOT State */
						if (lease->yiaddr == requested_align)
							sendACK(&packet, lease->yiaddr);
						else 
							sendNAK(&packet);
					} else {
						/* RENEWING or REBINDING State */
						if (lease->yiaddr == packet.ciaddr)
							sendACK(&packet, lease->yiaddr);
//						else if (!packet.ciaddr)
							/* Accept an invalid request in RENEWING state,
							   where the ciaddr should be set, but is not. */
							/* e.g. Linksys Print Server */
//							sendACK(&packet, lease->yiaddr); //note: let's not support broken stuff - Sveasoft 2005-01-19
						else {
							/* don't know what to do!!!! */
							sendNAK(&packet);
						}
					}						
				}
				if (lease->expires != EXPIRES_NEVER) {
					/* Don't change hostname of static leases */
					if (hostname) {
						bytes = hostname[-1];
						if (bytes >= (int) sizeof(lease->hostname))
							bytes = sizeof(lease->hostname) - 1;
						strncpy(lease->hostname, hostname, bytes);
							lease->hostname[bytes] = '\0';
					} else
						lease->hostname[0] = '\0';
				}
			
			/* what to do if we have no record of the client */
			} 
			else if (server_id) {
				/* SELECTING State */
				sendNAK(&packet);       // by honor

			} 
			else if (requested) {
				/* INIT-REBOOT State */
				if ((lease = find_lease_by_yiaddr(requested_align))) {
					if (lease_expired(lease)) {
						/* probably best if we drop this lease */
						memset(lease->chaddr, 0, 16);
					/* make some contention for this address */
					} 
					else 
						sendNAK(&packet);
				} 
				else if (requested_align < server_config.start || 
					 requested_align > server_config.end) {
					sendNAK(&packet);
				} 
				else {
					sendNAK(&packet);
				}

			} 
			else if (packet.ciaddr) {

				/* RENEWING or REBINDING State */
				sendNAK(&packet);
			}
			break;
		case DHCPDECLINE:
			LOG(LOG_INFO,"received DECLINE from %02x:%02x:%02x:%02x:%02x:%02x",
			    packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], 
			    packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]);	// modify by honor
			if (lease && lease->expires != EXPIRES_NEVER) {
				memset(lease->chaddr, 0, 16);
				lease->expires = get_time(0) + server_config.decline_time;
			}			
			break;
		case DHCPRELEASE:
			LOG(LOG_INFO,"received RELEASE from %02x:%02x:%02x:%02x:%02x:%02x",
			    packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], 
			    packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]);	// modify by honor
			if (lease && lease->expires != EXPIRES_NEVER) 
				lease->expires = get_time(0);
			break;
		case DHCPINFORM:
			LOG(LOG_INFO,"received INFORM from %02x:%02x:%02x:%02x:%02x:%02x",
			    packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], 
			    packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]);	// modify by honor
			send_inform(&packet);
			break;	
		default:
			LOG(LOG_WARNING, "unsupported DHCP message (%02x) -- ignoring", state[0]);
		}
	}

	return 0;
}

