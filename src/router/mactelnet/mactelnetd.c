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
#define _XOPEN_SOURCE 600
#define _BSD_SOURCE
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <endian.h>
#include <time.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
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
#else
#include <sys/time.h>
#endif
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <pwd.h>
#include <utmp.h>
#include <syslog.h>
#include <sys/utsname.h>
#include "md5.h"
#include "protocol.h"
#include "console.h"
#include "interfaces.h"
#include "users.h"
#include "config.h"

#define PROGRAM_NAME "MAC-Telnet Daemon"

#define MAX_INSOCKETS 100

#define MT_INTERFACE_LEN 128

/* Max ~5 pings per second */
#define MT_MAXPPS MT_MNDP_BROADCAST_INTERVAL * 5

#define _(String) String
#define gettext_noop(String) String

static int sockfd;
static int insockfd;
static int mndpsockfd;

static int pings = 0;

struct net_interface interfaces[MAX_INTERFACES];

static int use_raw_socket = 0;

static struct in_addr sourceip; 
static struct in_addr destip;
static int sourceport;

static time_t last_mndp_time = 0;

/* Protocol data direction */
unsigned char mt_direction_fromserver = 1;

/* Anti-timeout is every 10 seconds. Give up after 15. */
#define MT_CONNECTION_TIMEOUT 15

/* Connection states */
enum mt_connection_state {
	STATE_AUTH,
	STATE_CLOSED,
	STATE_ACTIVE
};

/** Connection struct */
struct mt_connection {
	struct net_interface *interface;
	char interface_name[256];

	unsigned short seskey;
	unsigned int incounter;
	unsigned int outcounter;
	time_t lastdata;

	int terminal_mode;
	enum mt_connection_state state;
	int ptsfd;
	int slavefd;
	int pid;
	int wait_for_ack;
	int have_enckey;

	char username[30];
	unsigned char trypassword[17];
	unsigned char srcip[IPV4_ALEN];
	unsigned char srcmac[ETH_ALEN];
	unsigned short srcport;
	unsigned char dstmac[ETH_ALEN];
	unsigned char enckey[16];
	unsigned short terminal_width;
	unsigned short terminal_height;
	char terminal_type[30];

	struct mt_connection *next;
};

static void uwtmp_login(struct mt_connection *);
static void uwtmp_logout(struct mt_connection *);

static struct mt_connection *connections_head = NULL;

static void list_add_connection(struct mt_connection *conn) {
	struct mt_connection *p;
	struct mt_connection *last;
	if (connections_head == NULL) {
		connections_head = conn;
		connections_head->next = NULL;
		return;
	}
	for (p = connections_head; p != NULL; p = p->next) {last = p;}
	last->next = conn;
	conn->next = NULL;
}

static void list_remove_connection(struct mt_connection *conn) {
	struct mt_connection *p;
	struct mt_connection *last;
	if (connections_head == NULL) {
		return;
	}

	if (conn->state == STATE_ACTIVE && conn->ptsfd > 0) {
		close(conn->ptsfd);
	}
	if (conn->state == STATE_ACTIVE && conn->slavefd > 0) {
		close(conn->slavefd);
	}

	uwtmp_logout(conn);

	if (connections_head == conn) {
		connections_head = conn->next;
		free(conn);
		return;
	}

	for (p = connections_head; p != NULL; p = p->next) {
		if (p == conn) {
			last->next = p->next;
			free(p);
			return;
		}
		last = p;
	}
}

static struct mt_connection *list_find_connection(unsigned short seskey, unsigned char *srcmac) {
	struct mt_connection *p;

	if (connections_head == NULL) {
		return NULL;
	}

	for (p = connections_head; p != NULL; p = p->next) {
		if (p->seskey == seskey && memcmp(srcmac, p->srcmac, ETH_ALEN) == 0) {
			return p;
		}
	}

	return NULL;
}

static int find_socket(unsigned char *mac) {
	int i;

	for (i = 0; i < MAX_INTERFACES; ++i) {
		if (interfaces[i].in_use && memcmp(mac, interfaces[i].mac_addr, ETH_ALEN) == 0) {
			return i;
		}

		if (!interfaces[i].in_use) {
			break;
		}
	}
	return -1;
}

static void setup_sockets() {
	int i;

	for (i = 0; i < MAX_INTERFACES; ++i) {
		int optval = 1;
		struct sockaddr_in si_me;
		struct ether_addr *mac = (struct ether_addr *)&(interfaces[i].mac_addr);

		if (interfaces[i].in_use == 0 || !interfaces[i].has_mac) {
			continue;
		}

		if (!use_raw_socket) {
			interfaces[i].socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (interfaces[i].socketfd < 0) {
				continue;
			}

			if (setsockopt(interfaces[i].socketfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof (optval))==-1) {
				perror("SO_BROADCAST");
				continue;
			}

			setsockopt(interfaces[i].socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

			/* Initialize receiving socket on the device chosen */
			si_me.sin_family = AF_INET;
			si_me.sin_port = htons(MT_MACTELNET_PORT);
			memcpy(&(si_me.sin_addr.s_addr), interfaces[i].ipv4_addr, IPV4_ALEN);

			if (bind(interfaces[i].socketfd, (struct sockaddr *)&si_me, sizeof(si_me))==-1) {
				fprintf(stderr, _("Error binding to %s:%d, %s\n"), inet_ntoa(si_me.sin_addr), sourceport, strerror(errno));
				continue;
			}
		}

		syslog(LOG_NOTICE, _("Listening on %s for %s\n"), interfaces[i].name, ether_ntoa(mac));

	}
}

static int send_udp(const struct mt_connection *conn, const struct mt_packet *packet) {
	if (use_raw_socket) {
		return net_send_udp(sockfd, conn->interface, conn->dstmac, conn->srcmac, &sourceip, sourceport, &destip, conn->srcport, packet->data, packet->size);
	} else {
		/* Init SendTo struct */
		struct sockaddr_in socket_address;
		socket_address.sin_family = AF_INET;
		socket_address.sin_port = htons(conn->srcport);
		socket_address.sin_addr.s_addr = htonl(INADDR_BROADCAST);

		return sendto(conn->interface->socketfd, packet->data, packet->size, 0, (struct sockaddr*)&socket_address, sizeof(socket_address));
	}
}

static int send_special_udp(struct net_interface *interface, unsigned short port, const struct mt_packet *packet) {
	unsigned char dstmac[ETH_ALEN];

	if (use_raw_socket) {
		memset(dstmac, 0xff, ETH_ALEN);
		return net_send_udp(sockfd, interface, interface->mac_addr, dstmac, (const struct in_addr *)&interface->ipv4_addr, port, &destip, port, packet->data, packet->size);
	} else {
		/* Init SendTo struct */
		struct sockaddr_in socket_address;
		socket_address.sin_family = AF_INET;
		socket_address.sin_port = htons(port);
		socket_address.sin_addr.s_addr = htonl(INADDR_BROADCAST);

		return sendto(interface->socketfd, packet->data, packet->size, 0, (struct sockaddr*)&socket_address, sizeof(socket_address));
	}
}


static void display_motd() {
	FILE *fp;
	int c;

	if ((fp = fopen("/etc/motd", "r"))) {
		while ((c = getc(fp)) != EOF) {
			putchar(c);
		}
		fclose(fp);
	}
}

static void display_nologin() {
	FILE *fp;
	int c;

	if ((fp = fopen(_PATH_NOLOGIN, "r"))) {
		while ((c = getc(fp)) != EOF) {
			putchar(c);
		}
		fclose(fp);
	}	
}

static void uwtmp_login(struct mt_connection *conn) {
	struct utmp utent;
	pid_t pid;

	pid = getpid();
	
	char *line = ttyname(conn->slavefd);
	if (strncmp(line, "/dev/", 5) == 0) {
		line += 5;
	}

	/* Setup utmp struct */
	memset((void *) &utent, 0, sizeof(utent));
	utent.ut_type = USER_PROCESS;
	utent.ut_pid = pid;
	strncpy(utent.ut_user, conn->username, sizeof(utent.ut_user));
	strncpy(utent.ut_line, line, sizeof(utent.ut_line));
	strncpy(utent.ut_id, utent.ut_line + 3, sizeof(utent.ut_id));
	strncpy(utent.ut_host, ether_ntoa((const struct ether_addr *)conn->srcmac), sizeof(utent.ut_host));
	time((time_t *)&(utent.ut_time));
	
	/* Update utmp and/or wtmp */
	setutent();
	pututline(&utent);
	endutent();
	updwtmp(_PATH_WTMP, &utent);
}

static void uwtmp_logout(struct mt_connection *conn) {
	if (conn->pid > 0) {
		struct utmp *utentp;
		struct utmp utent;
		setutent();

		while ((utentp = getutent()) != NULL) {
			if (utentp->ut_pid == conn->pid && utentp->ut_id) {
				break;
			}
		}

		if (utentp) {
			utent = *utentp;

			utent.ut_type = DEAD_PROCESS;
			utent.ut_tv.tv_sec = time(NULL);

			pututline(&utent);
			endutent();
			updwtmp(_PATH_WTMP, &utent);
		}
	}
}

static void abort_connection(struct mt_connection *curconn, struct mt_mactelnet_hdr *pkthdr, char *message) {
	struct mt_packet pdata;
	
	init_packet(&pdata, MT_PTYPE_DATA, pkthdr->dstaddr, pkthdr->srcaddr, pkthdr->seskey, curconn->outcounter);
	add_control_packet(&pdata, MT_CPTYPE_PLAINDATA, message, strlen(message));
	send_udp(curconn, &pdata);

	/* Make connection time out; lets the previous message get acked before disconnecting */
	curconn->state = STATE_CLOSED;
	init_packet(&pdata, MT_PTYPE_END, pkthdr->dstaddr, pkthdr->srcaddr, pkthdr->seskey, curconn->outcounter);
	send_udp(curconn, &pdata);
}

static void user_login(struct mt_connection *curconn, struct mt_mactelnet_hdr *pkthdr) {
	struct mt_packet pdata;
	unsigned char md5sum[17];
	char md5data[100];
	struct mt_credentials *user;
	char *slavename;

	/* Reparse user file before each login */
	read_userfile();

	if ((user = find_user(curconn->username)) != NULL) {
		md5_state_t state;
#if defined(__linux__) && defined(_POSIX_MEMLOCK_RANGE)
		mlock(md5data, sizeof(md5data));
		mlock(md5sum, sizeof(md5sum));
		if (user->password != NULL) {
			mlock(user->password, strlen(user->password));
		}
#endif

		/* Concat string of 0 + password + encryptionkey */
		md5data[0] = 0;
		strncpy(md5data + 1, user->password, 82);
		memcpy(md5data + 1 + strlen(user->password), curconn->enckey, 16);

		/* Generate md5 sum of md5data with a leading 0 */
		md5_init(&state);
		md5_append(&state, (const md5_byte_t *)md5data, strlen(user->password) + 17);
		md5_finish(&state, (md5_byte_t *)md5sum + 1);
		md5sum[0] = 0;

		init_packet(&pdata, MT_PTYPE_DATA, pkthdr->dstaddr, pkthdr->srcaddr, pkthdr->seskey, curconn->outcounter);
		curconn->outcounter += add_control_packet(&pdata, MT_CPTYPE_END_AUTH, NULL, 0);
		send_udp(curconn, &pdata);

		if (curconn->state == STATE_ACTIVE) {
			return;
		}
	}

	if (user == NULL || memcmp(md5sum, curconn->trypassword, 17) != 0) {
		syslog(LOG_NOTICE, _("(%d) Invalid login by %s."), curconn->seskey, curconn->username);

		/*_ Please include both \r and \n in translation, this is needed for the terminal emulator. */
		abort_connection(curconn, pkthdr, _("Login failed, incorrect username or password\r\n"));

		/* TODO: should wait some time (not with sleep) before returning, to minimalize brute force attacks */
		return;
	}

	/* User is logged in */
	curconn->state = STATE_ACTIVE;

	/* Enter terminal mode */
	curconn->terminal_mode = 1;

	/* Open pts handle */
	curconn->ptsfd = posix_openpt(O_RDWR);
	if (curconn->ptsfd == -1 || grantpt(curconn->ptsfd) == -1 || unlockpt(curconn->ptsfd) == -1) {
			syslog(LOG_ERR, "posix_openpt: %s", strerror(errno));
			/*_ Please include both \r and \n in translation, this is needed for the terminal emulator. */
			abort_connection(curconn, pkthdr, _("Terminal error\r\n"));
			return;
	}

	/* Get file path for our pts */
	slavename = ptsname(curconn->ptsfd);
	if (slavename != NULL) {
		pid_t pid;
		struct stat sb;
		struct passwd *user = (struct passwd *)getpwnam(curconn->username);
		if (user == NULL) {
			syslog(LOG_WARNING, _("(%d) Login ok, but local user not accessible (%s)."), curconn->seskey, curconn->username);
			/*_ Please include both \r and \n in translation, this is needed for the terminal emulator. */
			abort_connection(curconn, pkthdr, _("Local user not accessible\r\n"));
			return;
		}

		/* Change the owner of the slave pts */
		chown(slavename, user->pw_uid, user->pw_gid);

		curconn->slavefd = open(slavename, O_RDWR);
		if (curconn->slavefd == -1) {
			syslog(LOG_ERR, _("Error opening %s: %s"), slavename, strerror(errno));
			/*_ Please include both \r and \n in translation, this is needed for the terminal emulator. */
			abort_connection(curconn, pkthdr, _("Error opening terminal\r\n"));
			list_remove_connection(curconn);
			return;
		}

		if ((pid = fork()) == 0) {
			int i;

			/* Add login information to utmp/wtmp */
			uwtmp_login(curconn);

			syslog(LOG_INFO, _("(%d) User %s logged in."), curconn->seskey, curconn->username);

			/* Initialize terminal environment */
			setenv("USER", user->pw_name, 1);
			setenv("HOME", user->pw_dir, 1);
			setenv("SHELL", user->pw_shell, 1);
			setenv("TERM", curconn->terminal_type, 1);
			close(sockfd);
			close(insockfd);
			for (i = 0; i < MAX_INTERFACES; ++i) {
				if (interfaces[i].in_use && interfaces[i].socketfd > 0) {
					close(interfaces[i].socketfd);
				}
				if (!interfaces[i].in_use) {
					break;
				}
				
			}
			setsid();

			/* Don't let shell process inherit slavefd */
			fcntl (curconn->slavefd, F_SETFD, FD_CLOEXEC);
			close(curconn->ptsfd);
			
			/* Redirect STDIN/STDIO/STDERR */
			close(0);
			dup(curconn->slavefd);
			close(1);
			dup(curconn->slavefd);
			close(2);
			dup(curconn->slavefd);

			/* Set controlling terminal */
			ioctl(0, TIOCSCTTY, 1);
			tcsetpgrp(0, getpid());

			/* Set user id/group id */
			if ((setgid(user->pw_gid) != 0) || (setuid(user->pw_uid) != 0)) {
				syslog(LOG_ERR, _("(%d) Could not log in %s (%d:%d): setuid/setgid: %s"), curconn->seskey, curconn->username, user->pw_uid, user->pw_gid, strerror(errno));
				/*_ Please include both \r and \n in translation, this is needed for the terminal emulator. */
				abort_connection(curconn, pkthdr, _("Internal error\r\n"));
				exit(0);
			}

			/* Abort login if /etc/nologin exists */
			if (stat(_PATH_NOLOGIN, &sb) == 0 && getuid() != 0) {
				syslog(LOG_NOTICE, _("(%d) User %s disconnected with " _PATH_NOLOGIN " message."), curconn->seskey, curconn->username);
				display_nologin();
				curconn->state = STATE_CLOSED;
				init_packet(&pdata, MT_PTYPE_END, pkthdr->dstaddr, pkthdr->srcaddr, pkthdr->seskey, curconn->outcounter);
				send_udp(curconn, &pdata);
				exit(0);
			}

			/* Display MOTD */
			display_motd();

			chdir(user->pw_dir);

			/* Spawn shell */
			/* TODO: Maybe use "login -f USER" instead? renders motd and executes shell correctly for system */
			execl(user->pw_shell, user->pw_shell, "-", (char *) 0);
			exit(0); // just to be sure.
		}
		close(curconn->slavefd);
		curconn->pid = pid;
		set_terminal_size(curconn->ptsfd, curconn->terminal_width, curconn->terminal_height);
	}

}

static void handle_data_packet(struct mt_connection *curconn, struct mt_mactelnet_hdr *pkthdr, int data_len) {
	struct mt_mactelnet_control_hdr cpkt;
	struct mt_packet pdata;
	unsigned char *data = pkthdr->data;
	int got_user_packet = 0;
	int got_pass_packet = 0;
	int got_width_packet = 0;
	int got_height_packet = 0;
	int success;

	/* Parse first control packet */
	success = parse_control_packet(data, data_len - MT_HEADER_LEN, &cpkt);

	while (success) {
		if (cpkt.cptype == MT_CPTYPE_BEGINAUTH) {
			int plen,i;
			if (!curconn->have_enckey) {
				for (i = 0; i < 16; ++i) {
					curconn->enckey[i] = rand() % 256;
				}
				curconn->have_enckey = 1;

				memset(curconn->trypassword, 0, sizeof(curconn->trypassword));
			}
			init_packet(&pdata, MT_PTYPE_DATA, pkthdr->dstaddr, pkthdr->srcaddr, pkthdr->seskey, curconn->outcounter);
			plen = add_control_packet(&pdata, MT_CPTYPE_ENCRYPTIONKEY, (curconn->enckey), 16);
			curconn->outcounter += plen;

			send_udp(curconn, &pdata);

		} else if (cpkt.cptype == MT_CPTYPE_USERNAME) {

			memcpy(curconn->username, cpkt.data, cpkt.length > 29 ? 29 : cpkt.length);
			curconn->username[cpkt.length > 29 ? 29 : cpkt.length] = 0;
			got_user_packet = 1;

		} else if (cpkt.cptype == MT_CPTYPE_TERM_WIDTH) {
			unsigned short width;
			
			memcpy(&width, cpkt.data, 2);
			curconn->terminal_width = le16toh(width);
			got_width_packet = 1;

		} else if (cpkt.cptype == MT_CPTYPE_TERM_HEIGHT) {
			unsigned short height;

			memcpy(&height, cpkt.data, 2);
			curconn->terminal_height = le16toh(height);
			got_height_packet = 1;

		} else if (cpkt.cptype == MT_CPTYPE_TERM_TYPE) {

			memcpy(curconn->terminal_type, cpkt.data, cpkt.length > 29 ? 29 : cpkt.length);
			curconn->terminal_type[cpkt.length > 29 ? 29 : cpkt.length] = 0;

		} else if (cpkt.cptype == MT_CPTYPE_PASSWORD) {

#if defined(__linux__) && defined(_POSIX_MEMLOCK_RANGE)
			mlock(curconn->trypassword, 17);
#endif
			memcpy(curconn->trypassword, cpkt.data, 17);
			got_pass_packet = 1;

		} else if (cpkt.cptype == MT_CPTYPE_PLAINDATA) {

			/* relay data from client to shell */
			if (curconn->state == STATE_ACTIVE && curconn->ptsfd != -1) {
				write(curconn->ptsfd, cpkt.data, cpkt.length);
			}

		} else {
			syslog(LOG_WARNING, _("(%d) Unhandeled control packet type: %d"), curconn->seskey, cpkt.cptype);
		}

		/* Parse next control packet */
		success = parse_control_packet(NULL, 0, &cpkt);
	}
	
	if (got_user_packet && got_pass_packet) {
		user_login(curconn, pkthdr);
	}
	
	if (curconn->state == STATE_ACTIVE && (got_width_packet || got_height_packet)) {
		set_terminal_size(curconn->ptsfd, curconn->terminal_width, curconn->terminal_height);

	}
}

static void handle_packet(unsigned char *data, int data_len, const struct sockaddr_in *address) {
	struct mt_mactelnet_hdr pkthdr;
	struct mt_connection *curconn = NULL;
	struct mt_packet pdata;
	int interface_index;

	parse_packet(data, &pkthdr);

	/* Drop packets not belonging to us */
	if ((interface_index = find_socket(pkthdr.dstaddr)) < 0) {
		return;
	}

	switch (pkthdr.ptype) {

		case MT_PTYPE_PING:
			if (pings++ > MT_MAXPPS) {
				break;
			}
			init_pongpacket(&pdata, (unsigned char *)&(pkthdr.dstaddr), (unsigned char *)&(pkthdr.srcaddr));
			add_packetdata(&pdata, pkthdr.data - 4, data_len - (MT_HEADER_LEN - 4));
			{
				if (index >= 0) {
					send_special_udp(&interfaces[interface_index], MT_MACTELNET_PORT, &pdata);
				}
			}
			break;

		case MT_PTYPE_SESSIONSTART:
			syslog(LOG_DEBUG, _("(%d) New connection from %s."), pkthdr.seskey, ether_ntoa((struct ether_addr*)&(pkthdr.srcaddr)));
			curconn = calloc(1, sizeof(struct mt_connection));
			curconn->seskey = pkthdr.seskey;
			curconn->lastdata = time(NULL);
			curconn->state = STATE_AUTH;
			curconn->interface = &interfaces[interface_index];
			strncpy(curconn->interface_name, interfaces[interface_index].name, 254);
			curconn->interface_name[255] = '\0';
			memcpy(curconn->srcmac, pkthdr.srcaddr, ETH_ALEN);
			memcpy(curconn->srcip, &(address->sin_addr), IPV4_ALEN);
			curconn->srcport = htons(address->sin_port);
			memcpy(curconn->dstmac, pkthdr.dstaddr, ETH_ALEN);

			list_add_connection(curconn);

			init_packet(&pdata, MT_PTYPE_ACK, pkthdr.dstaddr, pkthdr.srcaddr, pkthdr.seskey, pkthdr.counter);
			send_udp(curconn, &pdata);
			break;

		case MT_PTYPE_END:
			curconn = list_find_connection(pkthdr.seskey, (unsigned char *)&(pkthdr.srcaddr));
			if (curconn == NULL) {
				break;
			}
			if (curconn->state != STATE_CLOSED) {
				init_packet(&pdata, MT_PTYPE_END, pkthdr.dstaddr, pkthdr.srcaddr, pkthdr.seskey, pkthdr.counter);
				send_udp(curconn, &pdata);
			}
			syslog(LOG_DEBUG, _("(%d) Connection closed."), curconn->seskey);
			list_remove_connection(curconn);
			return;

		case MT_PTYPE_ACK:
			curconn = list_find_connection(pkthdr.seskey, (unsigned char *)&(pkthdr.srcaddr));
			if (curconn == NULL) {
				break;
			}

			if (pkthdr.counter <= curconn->outcounter) {
				curconn->wait_for_ack = 0;
			}

			if (time(0) - curconn->lastdata > 9) {
				// Answer to anti-timeout packet
				init_packet(&pdata, MT_PTYPE_ACK, pkthdr.dstaddr, pkthdr.srcaddr, pkthdr.seskey, pkthdr.counter);
				send_udp(curconn, &pdata);
			}
			curconn->lastdata = time(NULL);
			return;

		case MT_PTYPE_DATA:
			curconn = list_find_connection(pkthdr.seskey, (unsigned char *)&(pkthdr.srcaddr));
			if (curconn == NULL) {
				break;
			}
			curconn->lastdata = time(NULL);

			/* ack the data packet */
			init_packet(&pdata, MT_PTYPE_ACK, pkthdr.dstaddr, pkthdr.srcaddr, pkthdr.seskey, pkthdr.counter + (data_len - MT_HEADER_LEN));
			send_udp(curconn, &pdata);

			/* Accept first packet, and all packets greater than incounter, and if counter has
			wrapped around. */
			if (curconn->incounter == 0 || pkthdr.counter > curconn->incounter || (curconn->incounter - pkthdr.counter) > 16777216) {
				curconn->incounter = pkthdr.counter;
			} else {
				/* Ignore double or old packets */
				return;
			}

			handle_data_packet(curconn, &pkthdr, data_len);
			break;
		default:
			if (curconn) {
				syslog(LOG_WARNING, _("(%d) Unhandeled packet type: %d"), curconn->seskey, pkthdr.ptype);
				init_packet(&pdata, MT_PTYPE_ACK, pkthdr.dstaddr, pkthdr.srcaddr, pkthdr.seskey, pkthdr.counter);
				send_udp(curconn, &pdata);
			}
		}
	if (0 && curconn != NULL) {
		printf("Packet, incounter %d, outcounter %d\n", curconn->incounter, curconn->outcounter);
	}
}

static void daemonize() {
	int pid,fd;

	pid = fork();

	/* Error? */
	if (pid < 0) {
		exit(1);
	}

	/* Parent exit */
	if (pid > 0) {
		exit(0);
	}

	setsid();
	close(0);
	close(1);
	close(2);
	
	fd = open("/dev/null",O_RDWR);
	dup(fd);
	dup(fd);
}

static void print_version() {
	fprintf(stderr, PROGRAM_NAME " " PROGRAM_VERSION "\n");
}

void mndp_broadcast() {
	struct mt_packet pdata;
	struct utsname s_uname;
	int i;
	unsigned int uptime;
#ifdef __linux__
	struct sysinfo s_sysinfo;

	if (sysinfo(&s_sysinfo) != 0) {
		return;
	}

	/* Seems like ping uptime is transmitted as little endian? */
	uptime = htole32(s_sysinfo.uptime);
#else
	struct timespec ts;

	if (clock_gettime(CLOCK_UPTIME, &ts) != -1) {
		uptime = htole32(((unsigned int)ts.tv_sec));
	}
#endif

	if (uname(&s_uname) != 0) {
		return;
	}

	for (i = 0; i < MAX_INTERFACES; ++i) {
		struct net_interface *interface = &interfaces[i];
		struct mt_mndp_hdr *header = (struct mt_mndp_hdr *)&(pdata.data);

		if (interfaces[i].has_mac == 0) {
			continue;
		}
		if (interfaces[i].in_use == 0) {
			break;
		}

		mndp_init_packet(&pdata, 0, 1);
		mndp_add_attribute(&pdata, MT_MNDPTYPE_ADDRESS, interface->mac_addr, ETH_ALEN);
		mndp_add_attribute(&pdata, MT_MNDPTYPE_IDENTITY, s_uname.nodename, strlen(s_uname.nodename));
		mndp_add_attribute(&pdata, MT_MNDPTYPE_VERSION, s_uname.release, strlen(s_uname.release));
		mndp_add_attribute(&pdata, MT_MNDPTYPE_PLATFORM, PLATFORM_NAME, strlen(PLATFORM_NAME));
		mndp_add_attribute(&pdata, MT_MNDPTYPE_HARDWARE, s_uname.machine, strlen(s_uname.machine));
		mndp_add_attribute(&pdata, MT_MNDPTYPE_TIMESTAMP, &uptime, 4);
		mndp_add_attribute(&pdata, MT_MNDPTYPE_SOFTID, MT_SOFTID_MACTELNET, strlen(MT_SOFTID_MACTELNET));
		mndp_add_attribute(&pdata, MT_MNDPTYPE_IFNAME, interface->name, strlen(interface->name));

		header->cksum = in_cksum((unsigned short *)&(pdata.data), pdata.size);
		send_special_udp(interface, MT_MNDP_PORT, &pdata);
	}
}

void sigterm_handler() {
	struct mt_connection *p;
	struct mt_packet pdata;
	/*_ Please include both \r and \n in translation, this is needed for the terminal emulator. */
	char message[] = gettext_noop("\r\n\r\nDaemon shutting down.\r\n");

	syslog(LOG_NOTICE, _("Daemon shutting down"));

	for (p = connections_head; p != NULL; p = p->next) {
		if (p->state == STATE_ACTIVE) {
			init_packet(&pdata, MT_PTYPE_DATA, p->interface->mac_addr, p->srcmac, p->seskey, p->outcounter);
			add_control_packet(&pdata, MT_CPTYPE_PLAINDATA, _(message), strlen(_(message)));
			send_udp(p, &pdata);

			init_packet(&pdata, MT_PTYPE_END, p->interface->mac_addr, p->srcmac, p->seskey, p->outcounter);
			send_udp(p, &pdata);
		}
	}

	/* Doesn't hurt to tidy up */
	close(sockfd);
	close(insockfd);
	if (!use_raw_socket) {
		int i;
		for (i = 0; i < MAX_INTERFACES; ++i) {
			if (interfaces[i].in_use && interfaces[i].socketfd > 0)
				close(interfaces[i].socketfd);
		}
	}
	closelog();
	exit(0);
}

void sighup_handler() {
	int i;
	struct mt_connection *p;

	syslog(LOG_NOTICE, _("SIGHUP: Reloading interfaces"));

	if (!use_raw_socket) {
		for (i = 0; i < MAX_INTERFACES; ++i) {
			close(interfaces[i].socketfd);
		}
	}

	bzero(interfaces, sizeof(interfaces));

	if (net_get_interfaces(interfaces, MAX_INTERFACES) <= 0) {
		syslog(LOG_ERR, _("No devices found! Exiting.\n"));
		exit(1);
	}

	setup_sockets();

	/* Reassign outgoing interfaces to connections again, since they may have changed */
	for (p = connections_head; p != NULL; p = p->next) {
		if (p->interface_name != NULL) {
			struct net_interface *interface = net_get_interface_ptr(interfaces, MAX_INTERFACES, p->interface_name, 0);
			if (interface != NULL) {
				p->interface = interface;
			} else {
				struct mt_connection tmp;
				syslog(LOG_NOTICE, _("(%d) Connection closed because interface %s is gone."), p->seskey, p->interface_name);
				tmp.next = p->next;
				list_remove_connection(p);
				p = &tmp;
			}
		}
	}
}

/*
 * TODO: Rewrite main() when all sub-functionality is tested
 */
int main (int argc, char **argv) {
	int result,i;
	struct sockaddr_in si_me;
	struct sockaddr_in si_me_mndp;
	struct timeval timeout;
	struct mt_packet pdata;
	fd_set read_fds;
	int c,optval = 1;
	int print_help = 0;
	int foreground = 0;
	int interface_count = 0;

	setlocale(LC_ALL, "");

	while ((c = getopt(argc, argv, "fnvh?")) != -1) {
		switch (c) {
			case 'f':
				foreground = 1;
				break;

			case 'n':
				use_raw_socket = 1;
				break;

			case 'v':
				print_version();
				exit(0);
				break;

			case 'h':
			case '?':
				print_help = 1;
				break;

		}
	}

	if (print_help) {
		print_version();
		fprintf(stderr, _("Usage: %s [-f|-n|-h]\n"), argv[0]);

		if (print_help) {
			fprintf(stderr, _("\nParameters:\n"
			"  -f        Run process in foreground.\n"
			"  -n        Do not use broadcast packets. Just a tad less insecure.\n"
			"  -h        This help.\n"
			"\n"));
		}
		return 1;
	}

	if (geteuid() != 0) {
		fprintf(stderr, _("You need to have root privileges to use %s.\n"), argv[0]);
		return 1;
	}

	/* Try to read user file */
	read_userfile();

	/* Seed randomizer */
	srand(time(NULL));

	if (use_raw_socket) {
		/* Transmit raw packets with this socket */
		sockfd = net_init_raw_socket();
	}

	/* Receive regular udp packets with this socket */
	insockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (insockfd < 0) {
		perror("insockfd");
		return 1;
	}

	/* Set source port */
	sourceport = MT_MACTELNET_PORT;

	/* Listen address*/
	inet_pton(AF_INET, (char *)"0.0.0.0", &sourceip);

	/* Set up global info about the connection */
	inet_pton(AF_INET, (char *)"255.255.255.255", &destip);

	/* Initialize receiving socket on the device chosen */
	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(sourceport);
	memcpy(&(si_me.sin_addr), &sourceip, IPV4_ALEN);

	setsockopt(insockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval));

	/* Bind to udp port */
	if (bind(insockfd, (struct sockaddr *)&si_me, sizeof(si_me))==-1) {
		fprintf(stderr, _("Error binding to %s:%d, %s\n"), inet_ntoa(si_me.sin_addr), sourceport, strerror(errno));
		return 1;
	}

	/* TODO: Move socket initialization out of main() */

	/* Receive mndp udp packets with this socket */
	mndpsockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (mndpsockfd < 0) {
		perror("mndpsockfd");
		return 1;
	}

	memset((char *)&si_me_mndp, 0, sizeof(si_me_mndp));
	si_me_mndp.sin_family = AF_INET;
	si_me_mndp.sin_port = htons(MT_MNDP_PORT);
	memcpy(&(si_me_mndp.sin_addr), &sourceip, IPV4_ALEN);

	setsockopt(mndpsockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval));

	/* Bind to udp port */
	if (bind(mndpsockfd, (struct sockaddr *)&si_me_mndp, sizeof(si_me_mndp))==-1) {
		fprintf(stderr, _("MNDP: Error binding to %s:%d, %s\n"), inet_ntoa(si_me_mndp.sin_addr), MT_MNDP_PORT, strerror(errno));
	}

	openlog("mactelnetd", LOG_PID, LOG_DAEMON);
	syslog(LOG_NOTICE, _("Bound to %s:%d"), inet_ntoa(si_me.sin_addr), sourceport);

	/* Enumerate available interfaces */
	net_get_interfaces(interfaces, MAX_INTERFACES);

	setup_sockets();

	if (!foreground) {
		daemonize();
	}

	/* Handle zombies etc */
	signal(SIGCHLD,SIG_IGN);
	signal(SIGTSTP,SIG_IGN);
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGHUP, sighup_handler);
	signal(SIGTERM, sigterm_handler);

	for (i = 0; i < MAX_INTERFACES; ++i) {
		if (interfaces[i].in_use && interfaces[i].has_mac) {
			interface_count++;
		}
	}
	
	if (interface_count == 0) {
		syslog(LOG_ERR, _("Unable to find any valid network interfaces\n"));
		exit(1);
	}

	while (1) {
		int reads;
		struct mt_connection *p;
		int maxfd=0;
		time_t now;

		/* Init select */
		FD_ZERO(&read_fds);
		FD_SET(insockfd, &read_fds);
		FD_SET(mndpsockfd, &read_fds);
		maxfd = insockfd > mndpsockfd ? insockfd : mndpsockfd;

		/* Add active connections to select queue */
		for (p = connections_head; p != NULL; p = p->next) {
			if (p->state == STATE_ACTIVE && p->wait_for_ack == 0 && p->ptsfd > 0) {
				FD_SET(p->ptsfd, &read_fds);
				if (p->ptsfd > maxfd) {
					maxfd = p->ptsfd;
				}
			}
		}

		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		/* Wait for data or timeout */
		reads = select(maxfd+1, &read_fds, NULL, NULL, &timeout);
		if (reads > 0) {
			/* Handle data from clients
			 TODO: Enable broadcast support (without raw sockets)
			 */
			if (FD_ISSET(insockfd, &read_fds)) {
				unsigned char buff[1500];
				struct sockaddr_in saddress;
				unsigned int slen = sizeof(saddress);
				result = recvfrom(insockfd, buff, 1500, 0, (struct sockaddr *)&saddress, &slen);
				handle_packet(buff, result, &saddress);
			}
			if (FD_ISSET(mndpsockfd, &read_fds)) {
				unsigned char buff[1500];
				struct sockaddr_in saddress;
				unsigned int slen = sizeof(saddress);
				result = recvfrom(mndpsockfd, buff, 1500, 0, (struct sockaddr *)&saddress, &slen);

				/* Handle MNDP broadcast request, max 1 rps */
				if (result == 4 && time(NULL) - last_mndp_time > 0) {
					mndp_broadcast();
					time(&last_mndp_time);
				}
			}
			/* Handle data from terminal sessions */
			for (p = connections_head; p != NULL; p = p->next) {
				/* Check if we have data ready in the pty buffer for the active session */
				if (p->state == STATE_ACTIVE && p->ptsfd > 0 && p->wait_for_ack == 0 && FD_ISSET(p->ptsfd, &read_fds)) {
					unsigned char keydata[1024];
					int datalen,plen;

					/* Read it */
					datalen = read(p->ptsfd, &keydata, 1024);
					if (datalen > 0) {
						/* Send it */
						init_packet(&pdata, MT_PTYPE_DATA, p->dstmac, p->srcmac, p->seskey, p->outcounter);
						plen = add_control_packet(&pdata, MT_CPTYPE_PLAINDATA, &keydata, datalen);
						p->outcounter += plen;
						p->wait_for_ack = 1;
						result = send_udp(p, &pdata);
					} else {
						/* Shell exited */
						struct mt_connection tmp;
						init_packet(&pdata, MT_PTYPE_END, p->dstmac, p->srcmac, p->seskey, p->outcounter);
						send_udp(p, &pdata);
						if (p->username != NULL) {
							syslog(LOG_INFO, _("(%d) Connection to user %s closed."), p->seskey, p->username);
						} else {
							syslog(LOG_INFO, _("(%d) Connection closed."), p->seskey);
						}
						tmp.next = p->next;
						list_remove_connection(p);
						p = &tmp;
					}
				}
				else if (p->state == STATE_ACTIVE && p->ptsfd > 0 && p->wait_for_ack == 1 && FD_ISSET(p->ptsfd, &read_fds)) {
					printf(_("(%d) Waiting for ack\n"), p->seskey);
				}
			}
		/* Handle select() timeout */
		}
		time(&now);
		
		if (now - last_mndp_time > MT_MNDP_BROADCAST_INTERVAL) {
			pings = 0;
			mndp_broadcast();
			last_mndp_time = now;
		}
		if (connections_head != NULL) {
			struct mt_connection *p,tmp;
			for (p = connections_head; p != NULL; p = p->next) {
				if (now - p->lastdata >= MT_CONNECTION_TIMEOUT) {
					syslog(LOG_INFO, _("(%d) Session timed out"), p->seskey);
					init_packet(&pdata, MT_PTYPE_DATA, p->dstmac, p->srcmac, p->seskey, p->outcounter);
					/*_ Please include both \r and \n in translation, this is needed for the terminal emulator. */
					add_control_packet(&pdata, MT_CPTYPE_PLAINDATA, _("Timeout\r\n"), 9);
					send_udp(p, &pdata);
					init_packet(&pdata, MT_PTYPE_END, p->dstmac, p->srcmac, p->seskey, p->outcounter);
					send_udp(p, &pdata);

					tmp.next = p->next;
					list_remove_connection(p);
					p = &tmp;
				}
			}
		}
	}

	/* Never reached */
	return 0;
}
