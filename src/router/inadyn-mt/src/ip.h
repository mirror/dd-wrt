/*
Copyright (C) 2003-2004 Narcis Ilisei

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/*
	History:

	Dec. 2010
		added pointer to addrinfo structure to IP_SOCKET
		changed IP_SOCKET SOCKET field to pointer to SOCKET
		added IPv6 function headers
		bhoover@wecs.com
*/

/* interface for tcp functions */

#ifndef _IP_H_INCLUDED
#define _IP_H_INCLUDED

#include "os.h"
#include "errorcode.h"

/* SOME DEFAULT CONFIGURATIONS */
#define IP_DEFAULT_TIMEOUT	20000 /*ms*/
#define IP_SOCKET_MAX_PORT 65535
#define IP_DEFAULT_READ_CHUNK_SIZE	100

/*socket shutdown types*/
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02

/* typedefs */

#ifndef _WIN32

typedef struct linger LINGER;

#else

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#endif

typedef enum
{
    TYPE_TCP,
    TYPE_UDP
} IP_TYPES;

typedef struct
{
	BOOL				initialized;
	BOOL				is_constructed;
	BOOL				is_ipv4;
	int					type;
	SOCKET				*socket;
	int					sock_index;
	struct sockaddr_in	remote_addr;
	const char			*p_remote_host_name;

	unsigned short		port;
	int					timeout;

	/*IPv6 trans*/
	struct				addrinfo *addr;
	struct				addrinfo **addr_ar;
	int					server_socket_count;
} IP_SOCKET;

/*public functions*/

/*
	 basic resource allocations for the  object
*/
RC_TYPE ip_construct(IP_SOCKET *p_self);

/*
	CAUTION:  only the pre-init stuff is copy (see the function) in present implementation.
*/
RC_TYPE ip_clone(IP_SOCKET *p_self_dest,IP_SOCKET *p_self_src);

/*
	Resource free .
*/	
RC_TYPE ip_destruct(IP_SOCKET *p_self);

/*
	Sets up the object.

	- ...
*/
RC_TYPE ip_initialize(IP_SOCKET *p_self);

/*
	Disconnect and some other clean up.
*/
RC_TYPE ip_shutdown(IP_SOCKET *p_self);

/* send data*/
RC_TYPE ip_send(IP_SOCKET *p_self, const char *p_buf, int len);

/* receive data*/
RC_TYPE ip_recv(IP_SOCKET *p_self, char *p_buf, int max_recv_len, int *p_recv_len);


/*Accessors */
RC_TYPE ip_set_port(IP_SOCKET *p_self, int p);
RC_TYPE ip_set_remote_name(IP_SOCKET *p_self, const char* p);
RC_TYPE ip_set_remote_timeout(IP_SOCKET *p_self, int t);
RC_TYPE ip_set_is_ipv4(IP_SOCKET *p_self, BOOL is_ipv4);

RC_TYPE ip_get_port(IP_SOCKET *p_self, int *p_port);
RC_TYPE ip_get_remote_name(IP_SOCKET *p_self, const char* *p);
RC_TYPE ip_get_remote_timeout(IP_SOCKET *p_self, int *p);
RC_TYPE ip_get_is_ipv4(IP_SOCKET *p_self, BOOL *is_ipv4);

/*extract string representation ipv4 or ipv6 ip from string*/
RC_TYPE parse_ip_address(char **str_ip,char *in_str);

const char* addr_family_get_name(int in_val);

#endif /*_IP_H_INCLUDED*/
