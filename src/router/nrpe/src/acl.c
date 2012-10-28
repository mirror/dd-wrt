/*-
 * acl.c - a small library for nrpe.c. It adds IPv4 subnets support to ACL in nrpe.
 * Copyright (c) 2011 Kaspersky Lab ZAO
 * Last Modified: 08-10-2011 by Konstantin Malov with Oleg Koreshkov's help 
 *
 * Description:
 * acl.c creates two linked lists. One is for IPv4 hosts and networks, another is for domain names.
 * All connecting hosts (if allowed_hosts is defined) are checked in these two lists.
 *
 * Some notes:
 * 1) IPv6 isn't supported in ACL.
 * 2) Only ANCII names are supported in ACL.
 *
 * License: GPL
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

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <syslog.h>
#include <stdarg.h>

#include "../include/acl.h"

/* This function checks if a char argumnet from valid char range.
 * Valid range is: ASCII only, a number or a letter, a space, a dot, a slash, a dash, a comma.
 *
 * Returns:
 *      0 - char isn't from valid group
 *  1 - char is a number
 *  2 - char is a letter
 *  3 - char is a space(' ')
 *  4 - char is a dot('.')
 *  5 - char is a slash('/')
 *  6 - char is a dash('-')
 *  7 - char is a comma(',')
 */

int isvalidchar(int c) {
        if (!isascii(c))
                return 0;

        if (isdigit(c))
                return 1;

        if (isalpha(c))
                return 2;

        if (isspace(c))
                return 3;

        switch (c) {
        case '.':
                return 4;
                break;
        case '/':
                return 5;
                break;
        case '-':
                return 6;
                break;
        case ',':
                return 7;
                break;
        default:
                return 0;
        }
}

/*
 * Get substring from allowed_hosts from s position to e position.
 */

char * acl_substring(char *string, int s, int e) {
    char *substring;
    int len = e - s;

        if (len < 0)
                return NULL;

    if ( (substring = malloc(len + 1)) == NULL)
        return NULL;

    memmove(substring, string + s, len + 1);
    return substring;
}

/*
 * Add IPv4 host or network to IP ACL. IPv4 format is X.X.X.X[/X].
 * Host will be added to ACL only if it has passed IPv4 format check.
 *
 * Returns:
 * 1 - on success
 * 0 - on failure
 *
 * States for IPv4 format check:
 *  0 - numbers(-> 1), dot(-> -1), slash(-> -1), other(-> -1)
 *  1 - numbers(-> 1), dot(-> 2),  slash(-> -1), other(-> -1)
 *  2 - numbers(-> 3), dot(-> -1), slash(-> -1), other(-> -1)
 *  3 - numbers(-> 3), dot(-> 4),  slash(-> -1), other(-> -1)
 *  4 - numbers(-> 5), dot(-> -1), slash(-> -1), other(-> -1)
 *  5 - numbers(-> 5), dot(-> 6),  slash(-> -1), other(-> -1)
 *  6 - numbers(-> 7), dot(-> -1), slash(-> -1), other(-> -1)
 *  7 - numbers(-> 7), dor(-> -1), slash(-> 8),  other(-> -1)
 *  8 - numbers(-> 9), dor(-> -1), slash(-> -1), other(-> -1)
 *  9 - numbers(-> 9), dot(-> -1), slash(-> -1), other(-> -1)
 *
 *  Good states are 7(IPv4 host) and 9(IPv4 network)
 */

int add_ipv4_to_acl(char *ipv4) {
        int state = 0;
        int octet = 0;
        int index = 0;  /* position in data array */
        int data[5];    /* array to store ip octets and mask */
        int len = strlen(ipv4);
        int i, c;
        unsigned long ip, mask;
        struct ip_acl *ip_acl_curr;

        /* Check for min and max IPv4 valid length */
        if (len < 7 || len > 18)
                return 0;

        /* default mask for ipv4 */
        data[4] = 32;

        /* Basic IPv4 format check */
        for (i = 0; i < len; i++) {
                /* Return 0 on error state */
                if (state == -1)
                        return 0;

                c = ipv4[i];

                switch (c) {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                        octet = octet * 10 + CHAR_TO_NUMBER(c);
                        switch (state) {
                        case 0: case 2: case 4: case 6: case 8:
                                state++;
                                break;
                        }
                        break;
                case '.':
                        switch (state) {
                        case 1: case 3: case 5:
                                data[index++] = octet;
                                octet = 0;
                                state++;
                                break;
                        default:
                                state = -1;
                        }
                        break;
                case '/':
                        switch (state) {
                        case 7:
                                data[index++] = octet;
                                octet = 0;
                                state++;
                                break;
                        default:
                                state = -1;
                        }
                        break;
                default:
                        state = -1;
                }
        }

        /* Exit state handling */
        switch (state) {
        case 7: case 9:
                data[index] = octet;
                break;
        default:
                /* Bad states */
                return 0;
        }

        /*
         * Final IPv4 format check.
         */
        for (i=0; i < 4; i++) {
                if (data[i] < 0 || data[i] > 255) {
                        syslog(LOG_ERR,"Invalid IPv4 address/network format(%s) in allowed_hosts option\n",ipv4);
                        return 0;
                }
        }

        if (data[4] < 0 || data[4] > 32) {
                syslog(LOG_ERR,"Invalid IPv4 network mask format(%s) in allowed_hosts option\n",ipv4);
                return 0;
        }

        /* Conver ip and mask to unsigned long */
        ip = htonl((data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3]);
        mask =  htonl(-1 << (32 - data[4]));

        /* Wrong network address */
        if ( (ip & mask) != ip) {
                syslog(LOG_ERR,"IP address and mask do not match in %s\n",ipv4);
                return 0;
        }

        /* Add addr to ip_acl list */
        if ( (ip_acl_curr = malloc(sizeof(*ip_acl_curr))) == NULL) {
                syslog(LOG_ERR,"Can't allocate memory for ACL, malloc error\n");
                return 0;
        }

        /* Save result in ACL ip list */
        ip_acl_curr->addr.s_addr = ip;
        ip_acl_curr->mask.s_addr = mask;
        ip_acl_curr->next = NULL;

        if (ip_acl_head == NULL) {
                ip_acl_head = ip_acl_curr;
        } else {
                ip_acl_prev->next = ip_acl_curr;
        }
        ip_acl_prev = ip_acl_curr;
        return 1;
}

/*
 * Add domain to DNS ACL list
 * Domain will be added only if it has passed domain name check.
 *
 * In this case domain valid format is:
 * 1) Domain names must use only alphanumeric characters and dashes (-).
 * 2) Domain names mustn't begin or end with dashes (-).
 * 3) Domain names mustn't have more than 63 characters.
 *
 * Return:
 * 1 - for success
 * 0 - for failure
 *
 * 0 - alpha(-> 1), number(-> 1), dot(-> -1), dash(-> -1), all other(-> -1)
 * 1 - alpha(-> 1), number(-> 1), dot(-> 2),  dash(-> 6),  all other(-> -1)
 * 2 - alpha(-> 3), number(-> 1), dot(-> -1), dash(-> -1), all other(-> -1)
 * 3 - alpha(-> 4), number(-> 1), dot(-> 2),  dash(-> 6),  all other(-> -1)
 * 4 - alpha(-> 5), number(-> 1), dot(-> 2),  dash(-> 6),  all other(-> -1)
 * 5 - alpha(-> 1), number(-> 1), dot(-> 2),  dash(-> 6),  all other(-> -1)
 * 6 - alpha(-> 1), number(-> 1), dot(-> 2),  dash(-> 6),  all other(-> -1)

 * For real FQDN only 4 and 5 states are good for exit.
 * I don't check if top domain exists (com, ru and etc.)
 * But in real life NRPE could work in LAN,
 * with local domain zones like .local or with names like 'mars' added to /etc/hosts.
 * So 1 is good state too. And maybe this check is not necessary at all...
 */

int add_domain_to_acl(char *domain) {
        int state = 0;
        int len = strlen(domain);
        int i, c;

        struct dns_acl *dns_acl_curr;

        if (len > 63)
                return 0;

        for (i = 0; i < len; i++) {
                c = domain[i];
                switch (isvalidchar(c)) {
                case 1:
                        state = 1;
                        break;
                case 2:
                        switch (state) {
                        case 0: case 1: case 5: case 6:
                                state = 1;
                                break;
                        case 2: case 3: case 4:
                                state++;
                                break;
                        }
                        break;

                case 4:
                        switch (state) {
                        case 0: case 2:
                                state = -1;
                                break;
                        default:
                                state = 2;
                        }
                        break;
                case 6:
                        switch (state) {
                        case 0: case 2:
                                state = -1;
                                break;
                        default:
                                state = 6;
                        }
                        break;
                default:
                        /* Not valid chars */
                        return 0;
                }
        }

        /* Check exit code */
        switch (state) {
        case 1: case 4: case 5:
                /* Add name to domain ACL list */
                if ( (dns_acl_curr = malloc(sizeof(*dns_acl_curr))) == NULL) {
                        syslog(LOG_ERR,"Can't allocate memory for ACL, malloc error\n");
                        return 0;
                }
                strcpy(dns_acl_curr->domain, domain);
                dns_acl_curr->next = NULL;

                if (dns_acl_head == NULL)
                        dns_acl_head = dns_acl_curr;
                else
                        dns_acl_prev->next = dns_acl_curr;

                dns_acl_prev = dns_acl_curr;
                return 1;
        default:
                return 0;
        }
}

/* Checks connectiong host in ACL
 *
 * Returns:
 * 1 - on success
 * 0 - on failure
 */

int is_an_allowed_host(struct in_addr host) {
        struct ip_acl *ip_acl_curr = ip_acl_head;
        struct dns_acl *dns_acl_curr = dns_acl_head;
        struct in_addr addr;
        struct hostent *he;

        while (ip_acl_curr != NULL) {
                if ( (host.s_addr & ip_acl_curr->mask.s_addr) == ip_acl_curr->addr.s_addr)
                        return 1;

                ip_acl_curr = ip_acl_curr->next;
        }

        while(dns_acl_curr != NULL) {
        he = gethostbyname(dns_acl_curr->domain);
        if (he == NULL)
                        return 0;

                while (*he->h_addr_list) {
                        memmove((char *)&addr,*he->h_addr_list++, sizeof(addr));
                        if (addr.s_addr == host.s_addr)
                                return 1;
                }
                dns_acl_curr = dns_acl_curr->next;
        }
        return 0;
}

/* The trim() function takes a source string and copies it to the destination string,
 * stripped of leading and training whitespace. The destination string must be 
 * allocated at least as large as the source string.
 */

void trim( char *src, char *dest) {
	char *sptr, *dptr;

	for( sptr = src; isblank( *sptr) && *sptr; sptr++); /* Jump past leading spaces */
	for( dptr = dest; !isblank( *sptr) && *sptr; ) {
		*dptr = *sptr;
		sptr++;
		dptr++;
	}
	*dptr = '\0';
	return;
}

/* This function splits allowed_hosts to substrings with comma(,) as a delimeter.
 * It doesn't check validness of ACL record (add_ipv4_to_acl() and add_domain_to_acl() do),
 * just trims spaces from ACL records.
 * After this it sends ACL records to add_ipv4_to_acl() or add_domain_to_acl().
 */

void parse_allowed_hosts(char *allowed_hosts) {
	char *hosts = strdup( allowed_hosts);	/* Copy since strtok* modifes original */
	char *saveptr;
	char *tok;
	const char *delim = ",";
	char *trimmed_tok;

	tok = strtok_r( hosts, delim, &saveptr);
	while( tok) {
		trimmed_tok = malloc( sizeof( char) * ( strlen( tok) + 1));
		trim( tok, trimmed_tok);
		if( strlen( trimmed_tok) > 0) {
			if (!add_ipv4_to_acl(trimmed_tok) && !add_domain_to_acl(trimmed_tok)) {
				syslog(LOG_ERR,"Can't add to ACL this record (%s). Check allowed_hosts option!\n",trimmed_tok);
			}
		}
		free( trimmed_tok);
		tok = strtok_r(( char *)0, delim, &saveptr);
	}

	free( hosts);
}

/*
 * Converts mask in unsigned long format to two digit prefix
 */

unsigned int prefix_from_mask(struct in_addr mask) {
        int prefix = 0;
        unsigned long bit = 1;
        int i;

        for (i = 0; i < 32; i++) {
                if (mask.s_addr & bit)
                        prefix++;

                bit = bit << 1;
        }
        return (prefix);
}

/*
 * It shows all hosts in ACL lists
 */

void show_acl_lists(void) {
        struct ip_acl *ip_acl_curr = ip_acl_head;
        struct dns_acl *dns_acl_curr = dns_acl_head;

        while (ip_acl_curr != NULL) {
                printf(" IP ACL: %s/%u %u\n", inet_ntoa(ip_acl_curr->addr), prefix_from_mask(ip_acl_curr->mask), ip_acl_curr->addr.s_addr);
                ip_acl_curr = ip_acl_curr->next;
        }

        while (dns_acl_curr != NULL) {
                printf("DNS ACL: %s\n", dns_acl_curr->domain);
                dns_acl_curr = dns_acl_curr->next;
        }
}
