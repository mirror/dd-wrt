/*
    Mac-Telnet - Connect to RouterOS or mactelnetd devices via MAC address
    Copyright (C) 2010, Håkon Nessjøen <haakon.nessjoen@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define _BSD_SOURCE
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <endian.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#ifdef __linux__
#include <linux/if_ether.h>
#include <sys/mman.h>
#endif
#include "md5.h"
#include "protocol.h"
#include "console.h"
#include "interfaces.h"
#include "config.h"
#include "mactelnet.h"
#include "mndp.h"
#include "autologin.h"

#define PROGRAM_NAME "MAC-Telnet"

#define _(String) String

static int sockfd = 0;
static int insockfd;

static unsigned int outcounter = 0;
static unsigned int incounter = 0;
static int sessionkey = 0;
static int running = 1;

static unsigned char use_raw_socket = 0;
static unsigned char terminal_mode = 0;

static unsigned char srcmac[ETH_ALEN];
static unsigned char dstmac[ETH_ALEN];

static struct in_addr sourceip; 
static struct in_addr destip;
static int sourceport;

static int connect_timeout = CONNECT_TIMEOUT;
static char run_mndp = 0;
static int mndp_timeout = 0;

static int is_a_tty = 1;
static int quiet_mode = 0;
static int batch_mode = 0;
static int no_autologin = 0;

static char autologin_path[255];

static int keepalive_counter = 0;

static unsigned char encryptionkey[128];
static char username[255];
static char password[255];
static char nonpriv_username[255];

struct net_interface interfaces[MAX_INTERFACES];
struct net_interface *active_interface;

/* Protocol data direction */
unsigned char mt_direction_fromserver = 0;

static unsigned int send_socket;

static int handle_packet(unsigned char *data, int data_len);

static void print_version() {
	fprintf(stderr, PROGRAM_NAME " " PROGRAM_VERSION "\n");
}

void drop_privileges(char *username) {
	struct passwd *user = (struct passwd *) getpwnam(username);
	if (user == NULL) {
		fprintf(stderr, _("Failed dropping privileges. The user %s is not a valid username on local system.\n"), username);
		exit(1);
	}
	if (getuid() == 0) {
		/* process is running as root, drop privileges */
		if (setgid(user->pw_gid) != 0) {
			fprintf(stderr, _("setgid: Error dropping group privileges\n"));
			exit(1);
		}
		if (setuid(user->pw_uid) != 0) {
			fprintf(stderr, _("setuid: Error dropping user privileges\n"));
			exit(1);
		}
		/* Verify if the privileges were developed. */
		if (setuid(0) != -1) {
			fprintf(stderr, _("Failed to drop privileges\n"));
			exit(1);
		}
	}
}

static int send_udp(struct mt_packet *packet, int retransmit) {
	int sent_bytes;

	/* Clear keepalive counter */
	keepalive_counter = 0;

	if (!use_raw_socket) {
		/* Init SendTo struct */
		struct sockaddr_in socket_address;
		socket_address.sin_family = AF_INET;
		socket_address.sin_port = htons(MT_MACTELNET_PORT);
		socket_address.sin_addr.s_addr = htonl(INADDR_BROADCAST);

		sent_bytes = sendto(send_socket, packet->data, packet->size, 0, (struct sockaddr*)&socket_address, sizeof(socket_address));
	} else {
		sent_bytes = net_send_udp(sockfd, active_interface, srcmac, dstmac, &sourceip,  sourceport, &destip, MT_MACTELNET_PORT, packet->data, packet->size);
	}

	/* 
	 * Retransmit packet if no data is received within
	 * retransmit_intervals milliseconds.
	 */
	if (retransmit) {
		int i;

		for (i = 0; i < MAX_RETRANSMIT_INTERVALS; ++i) {
			fd_set read_fds;
			int reads;
			struct timeval timeout;
			int interval = retransmit_intervals[i] * 1000;

			/* Init select */
			FD_ZERO(&read_fds);
			FD_SET(insockfd, &read_fds);
			timeout.tv_sec = 0;
			timeout.tv_usec = interval;

			/* Wait for data or timeout */
			reads = select(insockfd + 1, &read_fds, NULL, NULL, &timeout);
			if (reads && FD_ISSET(insockfd, &read_fds)) {
				unsigned char buff[1500];
				int result;

				bzero(buff, 1500);
				result = recvfrom(insockfd, buff, 1500, 0, 0, 0);

				/* Handle incoming packets, waiting for an ack */
				if (result > 0 && handle_packet(buff, result) == MT_PTYPE_ACK) {
					return sent_bytes;
				}
			}

			/* Retransmit */
			send_udp(packet, 0);
		}

		if (is_a_tty && terminal_mode) {
			reset_term();
		}

		fprintf(stderr, _("\nConnection timed out\n"));
		exit(1);
	}
	return sent_bytes;
}

static void send_auth(char *username, char *password) {
	struct mt_packet data;
	unsigned short width = 0;
	unsigned short height = 0;
	char *terminal = getenv("TERM");
	char md5data[100];
	unsigned char md5sum[17];
	int plen;
	md5_state_t state;

#if defined(__linux__) && defined(_POSIX_MEMLOCK_RANGE)
	mlock(md5data, sizeof(md5data));
	mlock(md5sum, sizeof(md5data));
#endif

	/* Concat string of 0 + password + encryptionkey */
	md5data[0] = 0;
	strncpy(md5data + 1, password, 82);
	md5data[83] = '\0';
	memcpy(md5data + 1 + strlen(password), encryptionkey, 16);

	/* Generate md5 sum of md5data with a leading 0 */
	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)md5data, strlen(password) + 17);
	md5_finish(&state, (md5_byte_t *)md5sum + 1);
	md5sum[0] = 0;

	/* Send combined packet to server */
	init_packet(&data, MT_PTYPE_DATA, srcmac, dstmac, sessionkey, outcounter);
	plen = add_control_packet(&data, MT_CPTYPE_PASSWORD, md5sum, 17);
	plen += add_control_packet(&data, MT_CPTYPE_USERNAME, username, strlen(username));
	plen += add_control_packet(&data, MT_CPTYPE_TERM_TYPE, terminal, strlen(terminal));
	
	if (is_a_tty && get_terminal_size(&width, &height) != -1) {
		width = htole16(width);
		height = htole16(height);
		plen += add_control_packet(&data, MT_CPTYPE_TERM_WIDTH, &width, 2);
		plen += add_control_packet(&data, MT_CPTYPE_TERM_HEIGHT, &height, 2);
	}

	outcounter += plen;

	/* TODO: handle result */
	send_udp(&data, 1);
}

static void sig_winch(int sig) {
	unsigned short width,height;
	struct mt_packet data;
	int plen;

	/* terminal height/width has changed, inform server */
	if (get_terminal_size(&width, &height) != -1) {
		init_packet(&data, MT_PTYPE_DATA, srcmac, dstmac, sessionkey, outcounter);
		width = htole16(width);
		height = htole16(height);
		plen = add_control_packet(&data, MT_CPTYPE_TERM_WIDTH, &width, 2);
		plen += add_control_packet(&data, MT_CPTYPE_TERM_HEIGHT, &height, 2);
		outcounter += plen;

		send_udp(&data, 1);
	}

	/* reinstate signal handler */
	signal(SIGWINCH, sig_winch);
}

static int handle_packet(unsigned char *data, int data_len) {
	struct mt_mactelnet_hdr pkthdr;
	parse_packet(data, &pkthdr);

	/* We only care about packets with correct sessionkey */
	if (pkthdr.seskey != sessionkey) {
		return -1;
	}

	/* Handle data packets */
	if (pkthdr.ptype == MT_PTYPE_DATA) {
		struct mt_packet odata;
		struct mt_mactelnet_control_hdr cpkt;
		int success = 0;

		/* Always transmit ACKNOWLEDGE packets in response to DATA packets */
		init_packet(&odata, MT_PTYPE_ACK, srcmac, dstmac, sessionkey, pkthdr.counter + (data_len - MT_HEADER_LEN));
		send_udp(&odata, 0);

		/* Accept first packet, and all packets greater than incounter, and if counter has
		wrapped around. */
		if (incounter == 0 || pkthdr.counter > incounter || (incounter - pkthdr.counter) > 65535) {
			incounter = pkthdr.counter;
		} else {
			/* Ignore double or old packets */
			return -1;
		}

		/* Parse controlpacket data */
		success = parse_control_packet(data + MT_HEADER_LEN, data_len - MT_HEADER_LEN, &cpkt);

		while (success) {

			/* If we receive encryptionkey, transmit auth data back */
			if (cpkt.cptype == MT_CPTYPE_ENCRYPTIONKEY) {
				memcpy(encryptionkey, cpkt.data, cpkt.length);
				send_auth(username, password);
			}

			/* If the (remaining) data did not have a control-packet magic byte sequence,
			   the data is raw terminal data to be outputted to the terminal. */
			else if (cpkt.cptype == MT_CPTYPE_PLAINDATA) {
				cpkt.data[cpkt.length] = 0;
				fputs((const char *)cpkt.data, stdout);
			}

			/* END_AUTH means that the user/password negotiation is done, and after this point
			   terminal data may arrive, so we set up the terminal to raw mode. */
			else if (cpkt.cptype == MT_CPTYPE_END_AUTH) {

				/* we have entered "terminal mode" */
				terminal_mode = 1;

				if (is_a_tty) {
					/* stop input buffering at all levels. Give full control of terminal to RouterOS */
					raw_term();

					setvbuf(stdin,  (char*)NULL, _IONBF, 0);

					/* Add resize signal handler */
					signal(SIGWINCH, sig_winch);
				}
			}

			/* Parse next controlpacket */
			success = parse_control_packet(NULL, 0, &cpkt);
		}
	}
	else if (pkthdr.ptype == MT_PTYPE_ACK) {
		/* Handled elsewhere */
	}

	/* The server wants to terminate the connection, we have to oblige */
	else if (pkthdr.ptype == MT_PTYPE_END) {
		struct mt_packet odata;

		/* Acknowledge the disconnection by sending a END packet in return */
		init_packet(&odata, MT_PTYPE_END, srcmac, dstmac, pkthdr.seskey, 0);
		send_udp(&odata, 0);

		if (!quiet_mode) {
			fprintf(stderr, _("Connection closed.\n"));
		}

		/* exit */
		running = 0;
	} else {
		fprintf(stderr, _("Unhandeled packet type: %d received from server %s\n"), pkthdr.ptype, ether_ntoa((struct ether_addr *)dstmac));
		return -1;
	}

	return pkthdr.ptype;
}

static int find_interface() {
	fd_set read_fds;
	struct mt_packet data;
	struct sockaddr_in myip;
	unsigned char emptymac[ETH_ALEN];
	int i, testsocket;
	struct timeval timeout;
	int optval = 1;

	/* TODO: reread interfaces on HUP */
	bzero(&interfaces, sizeof(struct net_interface) * MAX_INTERFACES);

	bzero(emptymac, ETH_ALEN);

	if (net_get_interfaces(interfaces, MAX_INTERFACES) <= 0) {
		fprintf(stderr, _("Error: No suitable devices found\n"));
		exit(1);
	}

	for (i = 0; i < MAX_INTERFACES; ++i) {
		if (!interfaces[i].in_use) {
			break;
		}

		/* Skip loopback interfaces */
		if (memcmp("lo", interfaces[i].name, 2) == 0) {
			continue;
		}

		/* Initialize receiving socket on the device chosen */
		myip.sin_family = AF_INET;
		memcpy((void *)&myip.sin_addr, interfaces[i].ipv4_addr, IPV4_ALEN);
		myip.sin_port = htons(sourceport);

		/* Initialize socket and bind to udp port */
		if ((testsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			continue;
		}

		setsockopt(testsocket, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
		setsockopt(testsocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

		if (bind(testsocket, (struct sockaddr *)&myip, sizeof(struct sockaddr_in)) == -1) {
			close(testsocket);
			continue;
		}

		/* Ensure that we have mac-address for this interface  */
		if (!interfaces[i].has_mac) {
			close(testsocket);
			continue;
		}

		/* Set the global socket handle and source mac address for send_udp() */
		send_socket = testsocket;
		memcpy(srcmac, interfaces[i].mac_addr, ETH_ALEN);
		active_interface = &interfaces[i];

		/* Send a SESSIONSTART message with the current device */
		init_packet(&data, MT_PTYPE_SESSIONSTART, srcmac, dstmac, sessionkey, 0);
		send_udp(&data, 0);

		timeout.tv_sec = connect_timeout;
		timeout.tv_usec = 0;

		FD_ZERO(&read_fds);
		FD_SET(insockfd, &read_fds);
		select(insockfd + 1, &read_fds, NULL, NULL, &timeout);
		if (FD_ISSET(insockfd, &read_fds)) {
			/* We got a response, this is the correct device to use */
			return 1;
		}

		close(testsocket);
	}
	return 0;
}

/*
 * TODO: Rewrite main() when all sub-functionality is tested
 */
int main (int argc, char **argv) {
	int result;
	struct mt_packet data;
	struct sockaddr_in si_me;
	struct autologin_profile *login_profile;
	unsigned char buff[1500];
	unsigned char print_help = 0, have_username = 0, have_password = 0;
	unsigned char drop_priv = 0;
	int c;
	int optval = 1;

	strncpy(autologin_path, AUTOLOGIN_PATH, 254);

	setlocale(LC_ALL, "");

	while (1) {
		c = getopt(argc, argv, "lnqt:u:p:U:vh?BAa:");

		if (c == -1) {
			break;
		}

		switch (c) {

			case 'n':
				use_raw_socket = 1;
				break;

			case 'u':
				/* Save username */
				strncpy(username, optarg, sizeof(username) - 1);
				username[sizeof(username) - 1] = '\0';
				have_username = 1;
				break;

			case 'p':
				/* Save password */
#if defined(__linux__) && defined(_POSIX_MEMLOCK_RANGE)
				mlock(password, sizeof(password));
#endif
				strncpy(password, optarg, sizeof(password) - 1);
				password[sizeof(password) - 1] = '\0';
				have_password = 1;
				break;

			case 'U':
				/* Save nonpriv_username */
				strncpy(nonpriv_username, optarg, sizeof(nonpriv_username) - 1);
				nonpriv_username[sizeof(nonpriv_username) - 1] = '\0';
				drop_priv = 1;
				break;

			case 't':
				connect_timeout = atoi(optarg);
				mndp_timeout = connect_timeout;
				break;

			case 'v':
				print_version();
				exit(0);
				break;

			case 'q':
				quiet_mode = 1;
				break;

			case 'l':
				run_mndp = 1;
				break;

			case 'B':
				batch_mode = 1;
				break;

			case 'A':
				no_autologin = 1;
				break;

			case 'a':
				strncpy(autologin_path, optarg, 254);
				break;

			case 'h':
			case '?':
				print_help = 1;
				break;

		}
	}
	if (run_mndp) {
		return mndp(mndp_timeout, batch_mode);
	}
	if (argc - optind < 1 || print_help) {
		print_version();
		fprintf(stderr, _("Usage: %s <MAC|identity> [-h] [-n] [-a <path>] [-A] [-t <timeout>] [-u <user>] [-p <password>] [-U <user>] | -l [-B] [-t <timeout>]\n"), argv[0]);

		if (print_help) {
			fprintf(stderr, _("\nParameters:\n"
			"  MAC            MAC-Address of the RouterOS/mactelnetd device. Use mndp to\n"
			"                 discover it.\n"
			"  identity       The identity/name of your destination device. Uses\n"
			"                 MNDP protocol to find it.\n"
			"  -l             List/Search for routers nearby (MNDP). You may use -t to set timeout.\n"
			"  -B             Batch mode. Use computer readable output (CSV), for use with -l.\n"
			"  -n             Do not use broadcast packets. Less insecure but requires\n"
			"                 root privileges.\n"
			"  -a <path>      Use specified path instead of the default: " AUTOLOGIN_PATH " for autologin config file.\n"
			"  -A             Disable autologin feature.\n"
			"  -t <timeout>   Amount of seconds to wait for a response on each interface.\n"
			"  -u <user>      Specify username on command line.\n"
			"  -p <password>  Specify password on command line.\n"
			"  -U <user>      Drop privileges to this user. Used in conjunction with -n\n"
			"                 for security.\n"
			"  -q             Quiet mode.\n"
			"  -h             This help.\n"
			"\n"));
		}
		return 1;
	}

	is_a_tty = isatty(fileno(stdout)) && isatty(fileno(stdin));
	if (!is_a_tty) {
		quiet_mode = 1;
	}

	if (!no_autologin) {
		autologin_readfile(autologin_path);
		login_profile = autologin_find_profile(argv[optind]);

		if (!quiet_mode && login_profile != NULL && (login_profile->hasUsername || login_profile->hasPassword)) {
			fprintf(stderr, _("Using autologin credentials from %s\n"), autologin_path);
		}
		if (!have_username) {
			if (login_profile != NULL && login_profile->hasUsername) {
				have_username = 1;
				strncpy(username, login_profile->username, sizeof(username) - 1);
				username[sizeof(username) - 1] = '\0';
			}
		}

		if (!have_password) {
			if (login_profile != NULL && login_profile->hasPassword) {
				have_password = 1;
				strncpy(password, login_profile->password, sizeof(password) - 1);
				password[sizeof(password) - 1] = '\0';
			}
		}
	}

	/* Seed randomizer */
	srand(time(NULL));

	if (use_raw_socket) {
		if (geteuid() != 0) {
			fprintf(stderr, _("You need to have root privileges to use the -n parameter.\n"));
			return 1;
		}

		sockfd = net_init_raw_socket();

		if (drop_priv) {
			drop_privileges(nonpriv_username);
		}
	} else if (drop_priv) {
		fprintf(stderr, _("The -U option must be used in conjunction with the -n parameter.\n"));
		return 1;
	}

	/* Receive regular udp packets with this socket */
	insockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (insockfd < 0) {
		perror("insockfd");
		return 1;
	}

	if (!use_raw_socket) {
		if (setsockopt(insockfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof (optval))==-1) {
			perror("SO_BROADCAST");
			return 1;
		}
	}

	/* Need to use, to be able to autodetect which interface to use */
	setsockopt(insockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval));

	/* Get mac-address from string, or check for hostname via mndp */
	if (!query_mndp_or_mac(argv[optind], dstmac, !quiet_mode)) {
		/* No valid mac address found, abort */
		return 1;
	}

	if (!have_username) {
		if (!quiet_mode) {
			printf(_("Login: "));
		}
		scanf("%254s", username);
	}

	if (!have_password) {
		char *tmp;
		tmp = getpass(quiet_mode ? "" : _("Password: "));
#if defined(__linux__) && defined(_POSIX_MEMLOCK_RANGE)
		mlock(password, sizeof(password));
#endif
		strncpy(password, tmp, sizeof(password) - 1);
		password[sizeof(password) - 1] = '\0';
		/* security */
		memset(tmp, 0, strlen(tmp));
#ifdef __linux__
		free(tmp);
#endif
	}


	/* Set random source port */
	sourceport = 1024 + (rand() % 1024);

	/* Set up global info about the connection */
	inet_pton(AF_INET, (char *)"255.255.255.255", &destip);
	memcpy(&sourceip, &(si_me.sin_addr), IPV4_ALEN);

	/* Session key */
	sessionkey = rand() % 65535;

	/* stop output buffering */
	setvbuf(stdout, (char*)NULL, _IONBF, 0);

	if (!quiet_mode) {
		printf(_("Connecting to %s..."), ether_ntoa((struct ether_addr *)dstmac));
	}

	/* Initialize receiving socket on the device chosen */
	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(sourceport);

	/* Bind to udp port */
	if (bind(insockfd, (struct sockaddr *)&si_me, sizeof(si_me)) == -1) {
		fprintf(stderr, _("Error binding to %s:%d, %s\n"), inet_ntoa(si_me.sin_addr), sourceport, strerror(errno));
		return 1;
	}

	if (!find_interface() || (result = recvfrom(insockfd, buff, 1400, 0, 0, 0)) < 1) {
		fprintf(stderr, _("Connection failed.\n"));
		return 1;
	}
	if (!quiet_mode) {
		printf(_("done\n"));
	}

	/* Handle first received packet */
	handle_packet(buff, result);

	init_packet(&data, MT_PTYPE_DATA, srcmac, dstmac, sessionkey, 0);
	outcounter +=  add_control_packet(&data, MT_CPTYPE_BEGINAUTH, NULL, 0);

	/* TODO: handle result of send_udp */
	result = send_udp(&data, 1);

	while (running) {
		fd_set read_fds;
		int reads;
		static int terminal_gone = 0;
		struct timeval timeout;

		/* Init select */
		FD_ZERO(&read_fds);
		if (!terminal_gone) {
			FD_SET(0, &read_fds);
		}
		FD_SET(insockfd, &read_fds);

		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		/* Wait for data or timeout */
		reads = select(insockfd+1, &read_fds, NULL, NULL, &timeout);
		if (reads > 0) {
			/* Handle data from server */
			if (FD_ISSET(insockfd, &read_fds)) {
				bzero(buff, 1500);
				result = recvfrom(insockfd, buff, 1500, 0, 0, 0);
				handle_packet(buff, result);
			}
			/* Handle data from keyboard/local terminal */
			if (FD_ISSET(0, &read_fds) && terminal_mode) {
				unsigned char keydata[512];
				int datalen;

				datalen = read(STDIN_FILENO, &keydata, 512);

				if (datalen > 0) {
					/* Data received, transmit to server */
					init_packet(&data, MT_PTYPE_DATA, srcmac, dstmac, sessionkey, outcounter);
					add_control_packet(&data, MT_CPTYPE_PLAINDATA, &keydata, datalen);
					outcounter += datalen;
					send_udp(&data, 1);
				} else {
					terminal_gone = 1;
				}
			}
		/* Handle select() timeout */
		} else {
			/* handle keepalive counter, transmit keepalive packet every 10 seconds
			   of inactivity  */
			if (keepalive_counter++ == 10) {
				struct mt_packet odata;
				init_packet(&odata, MT_PTYPE_ACK, srcmac, dstmac, sessionkey, outcounter);
				send_udp(&odata, 0);
			}
		}
	}

	if (is_a_tty && terminal_mode) {
		/* Reset terminal back to old settings */
		reset_term();
	}

	close(sockfd);
	close(insockfd);

	return 0;
}
