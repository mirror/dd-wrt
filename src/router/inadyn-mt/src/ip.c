/*
Copyright (C) 2003-2004 Narcis Ilisei
Modifications by Bryan Hoover (bhoover@wecs.com)
Copyright (C) 2009 Bryan Hoover (bhoover@wecs.com)

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
	Dyn Dns update main implementation file 
	Author: narcis Ilisei
	Date: May 2003

	History:
      Sept. 2009
	  - Windows socket layer shutdown per startup
	  - Oct. 2009
	  - ip_initialize async
      Dec. 2010
	  - Added IPv6, IPv4 address parsing
*/
#define MODULE_TAG "IP: "

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <winuser.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "debug_if.h"
#include "ip.h"
#include "safe_mem.h"

#ifdef USE_THREADS

#include "threads_wrapper.h"

#ifdef HAVE_FUNC_GETHOSTBYNAME_R_NOT

#ifndef ASYNC_LOOKUP

static mutex_t	gethostname_mutex=MUTEX_T_INIT;

#endif

#endif

#endif

typedef enum
{
	/*begin parse*/

	NEW_LINE,
	NUMERIC,
	NEWLINE_COLON,

	/*"." after i'th digit group*/

	DOT_1,
	DOT_2,
	DOT_3,

	/*digit group number/state*/

	IPV4_NUMERIC_2,
	IPV4_NUMERIC_3,
	IPV4_NUMERIC_4,	

	/*check for double (consecutive) colon
	  and trans to DBL state if so (non dbl
	  state if not)*/

	COLON_DETECT_1,
	COLON_DETECT_2,
	COLON_DETECT_3,
	COLON_DETECT_4,
	COLON_DETECT_5,
	COLON_DETECT_6,
	COLON_DETECT_7,

	/*digit group number/state*/

	IPV6_NUMERIC_2,
	IPV6_NUMERIC_3,
	IPV6_NUMERIC_4,
	IPV6_NUMERIC_5,
	IPV6_NUMERIC_6,
	IPV6_NUMERIC_7,
	IPV6_NUMERIC_8,
	IPV6_FINAL,

	/*have had a double colon*/

	DBL_COLON_A,
	DBL_COLON_B,

	/*detect double colon as already in
	  DBL state -- it's an error*/

	DBL_COLON_DETECT_A,
	DBL_COLON_DETECT_B

} PARSER_STATE;

typedef struct
{
	FILE *p_file;
	PARSER_STATE state;
} OPTION_FILE_PARSER;

typedef struct
{
	int			fam_val;
	const char	*p_name;
} ADDR_FAMILY_NAME;

static const ADDR_FAMILY_NAME global_addrfamily_table[]  =
{
	{AF_INET,"AF_INET"},
	{AF_INET6,"AF_INET6"},

	{0,NULL}
};

static const char* unknown_addr_fam = "Unknown address family";

/*
	 basic resource allocations for the ip object
*/
RC_TYPE ip_construct(IP_SOCKET *p_self)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (p_self->is_constructed)
	{
		return RC_OK;
	}

	memset(p_self, 0, sizeof(IP_SOCKET));

	p_self->initialized = FALSE;
	p_self->socket = safe_malloc(sizeof(SOCKET));
	p_self->socket[0]=INVALID_SOCKET;
	p_self->addr_ar = safe_malloc(sizeof(struct addrinfo *));
	p_self->addr_ar[0]=NULL;
	p_self->server_socket_count=1;
	memset( &p_self->remote_addr, 0,sizeof(p_self->remote_addr));
	p_self->timeout = IP_DEFAULT_TIMEOUT;	

	p_self->is_constructed=TRUE;


	return RC_OK;
}

/*
	This is not a "complete" clone.  Take care to see what is/isn't coppied.
*/
RC_TYPE ip_clone(IP_SOCKET *p_self_dest,IP_SOCKET *p_self_src)
{

	if (!(p_self_dest))

		return RC_INVALID_POINTER;

	if (!(p_self_src))

		return RC_INVALID_POINTER;

	ip_construct(p_self_dest);

	p_self_dest->p_remote_host_name=p_self_src->p_remote_host_name;
	p_self_dest->port=p_self_src->port;
	p_self_dest->timeout=p_self_src->timeout;
	p_self_dest->initialized=0;


	return RC_OK;
}

/*
	Resource free.
*/	
RC_TYPE ip_destruct(IP_SOCKET *p_self)
{
	if (p_self == NULL)
	{
		return RC_OK;
	}

	if (!(p_self->is_constructed))
	{
		return RC_OK;
	}

	ip_shutdown(p_self);


	free(p_self->socket);
	free(p_self->addr_ar);

	memset(p_self,0,sizeof(IP_SOCKET));


	return RC_OK;
}

RC_TYPE ip_initialize(IP_SOCKET *p_self)
{
	RC_TYPE			rc=RC_OK;
	struct addrinfo *result=NULL;
    struct addrinfo hints;
	int				resolv_ret=0;
	char			port_str[128];
	BOOL			is_ip_support_ok;

	
	if (p_self->initialized==TRUE) {

		return RC_OK;
	}

	do
	{
		if (!(is_ip_support_ok=(RC_OK==(rc=os_ip_support_startup()))))

			break;


		memset(&hints, 0,sizeof(struct addrinfo));

		if (p_self->is_ipv4) {

			hints.ai_family=AF_INET;
		} 
		else {

			hints.ai_family=AF_UNSPEC;
		}

		hints.ai_socktype=SOCK_STREAM;
		hints.ai_protocol=IPPROTO_TCP;
		hints.ai_flags=AI_CANONNAME;


		/*remote addres */
		if (p_self->p_remote_host_name==NULL) {

			rc=RC_INVALID_POINTER;
		}
		else {

#ifndef _WIN32
			snprintf(port_str,sizeof(port_str),"%d",p_self->port);
#else
			_snprintf(port_str,sizeof(port_str),"%d",p_self->port);
#endif

			if (!(resolv_ret=getaddrinfo(p_self->p_remote_host_name,port_str,&hints,&result))) {

				p_self->addr=result;
			} 
			else {
				
				DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "Error 0x%x resolving host name '%s'\n",
				            os_get_socket_error(),p_self->p_remote_host_name));

				rc=RC_IP_INVALID_REMOTE_ADDR;

				break;	
			}
		}
	}
	while(0);

	if (rc == RC_OK) {

		p_self->initialized=TRUE;		
	}
	else {

		if (is_ip_support_ok)

			os_ip_support_cleanup();

		ip_shutdown(p_self);
	}

	return rc;
}

/*
	Disconnect and some other clean up.
*/
RC_TYPE ip_shutdown(IP_SOCKET *p_self)
{
	int			i;


	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (!p_self->initialized)
	{
		return RC_OK;
	}

	/*
		close them, don't dealloc; socket are created
		with realloc so no mem leak, and must always
		have at least one allocated -- implemetation
		migrated to socket list from single static
	*/
	for (i=0;i<p_self->server_socket_count;i++) {

		if (!(p_self->socket[i]==INVALID_SOCKET)) {

			if (closesocket(p_self->socket[i])==0) {

				p_self->socket[i]=INVALID_SOCKET;
			}
			else {

				DBG_PRINTF((LOG_CRIT,"C:" MODULE_TAG "closesocket returned error 0x%x in ip_shutdown...\n", os_get_socket_error()));
			}
		}
	}

	p_self->server_socket_count=1;
	p_self->sock_index=0;

	if (p_self->addr) {
		
		freeaddrinfo(p_self->addr);

		p_self->addr=NULL;
	}

	os_ip_support_cleanup();

	p_self->initialized = FALSE;

	return RC_OK;
}

RC_TYPE ip_send(IP_SOCKET *p_self, const char *p_buf, int len)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (!p_self->initialized)
	{
		return RC_IP_OBJECT_NOT_INITIALIZED;
	}

	if ((p_self->socket[p_self->sock_index]==INVALID_SOCKET))
	{
		return RC_IP_INVALID_SOCKET;
	}

	if( send(p_self->socket[p_self->sock_index], (char*) p_buf, len, 0) == SOCKET_ERROR )
	{
		DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "Error 0x%x in send()\n", os_get_socket_error()));
		return RC_IP_SEND_ERROR;
	}
	return RC_OK;
}

/*
	Receive data into user's buffer.
	return 
		if the max len has been received 
		if a timeout occures
	In p_recv_len the total number of bytes are returned.
	Note:
		if the recv_len is bigger than 0, no error is returned.
*/
RC_TYPE ip_recv(IP_SOCKET *p_self, char *p_buf, int max_recv_len, int *p_recv_len)
{
	RC_TYPE	rc = RC_OK;
	int		remaining_buf_len = max_recv_len;
	int		total_recv_len = 0;
	int		recv_len = 0;

	if (p_self == NULL || p_buf == NULL || p_recv_len == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (!p_self->initialized)
	{
		return RC_IP_OBJECT_NOT_INITIALIZED;
	}

	if ((p_self->socket[p_self->sock_index]==INVALID_SOCKET))
	{
		return RC_IP_INVALID_SOCKET;
	}

	while (remaining_buf_len > 0)
	{
		int chunk_size = remaining_buf_len > IP_DEFAULT_READ_CHUNK_SIZE ?
		                 IP_DEFAULT_READ_CHUNK_SIZE : remaining_buf_len;

		recv_len = recv(p_self->socket[p_self->sock_index], p_buf + total_recv_len, chunk_size, 0);

		if (recv_len < 0)
		{

			{
				DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "Error 0x%x in recv()\n", os_get_socket_error()));
				rc = RC_IP_RECV_ERROR;
			}
			break;
		}

		if (recv_len == 0)
		{
			if (total_recv_len == 0)
			{
				rc = RC_IP_RECV_ERROR;
			}
			break;
		}

		total_recv_len += recv_len;
		remaining_buf_len = max_recv_len - total_recv_len;
	}


	*p_recv_len = total_recv_len;
	return rc;
}


/*Accessors */

RC_TYPE ip_set_port(IP_SOCKET *p_self, int p)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (p < 0 || p > IP_SOCKET_MAX_PORT)
	{
		return RC_IP_BAD_PARAMETER;
	}
	p_self->port = p;
	return RC_OK;
}

RC_TYPE ip_set_remote_name(IP_SOCKET *p_self, const char* p)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}
	p_self->p_remote_host_name = p;
	return RC_OK;
}

RC_TYPE ip_set_remote_timeout(IP_SOCKET *p_self, int t)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}
	p_self->timeout = t;
	return RC_OK;
}

RC_TYPE ip_set_is_ipv4(IP_SOCKET *p_self, BOOL is_ipv4)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	p_self->is_ipv4=is_ipv4;

	return RC_OK;
}

RC_TYPE ip_get_port(IP_SOCKET *p_self, int *p_port)
{
	if (p_self == NULL || p_port == NULL)
	{
		return RC_INVALID_POINTER;
	}
	*p_port = p_self->port;
	return RC_OK;
}

RC_TYPE ip_get_remote_name(IP_SOCKET *p_self, const char* *p)
{
	if (p_self == NULL || p == NULL)
	{
		return RC_INVALID_POINTER;
	}
	*p = p_self->p_remote_host_name;
	return RC_OK;
}

RC_TYPE ip_get_remote_timeout(IP_SOCKET *p_self, int *p)
{
	if (p_self == NULL || p == NULL)
	{
		return RC_INVALID_POINTER;
	}
	*p = p_self->timeout;
	return RC_OK;
}

RC_TYPE ip_get_is_ipv4(IP_SOCKET *p_self, BOOL *is_ipv4)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	*is_ipv4=p_self->is_ipv4;

	return RC_OK;
}

const char* addr_family_get_name(int in_val)
{
	const ADDR_FAMILY_NAME *it = global_addrfamily_table;

	while (it->p_name != NULL)
	{
		if (it->fam_val == in_val)
		{
			return it->p_name;
		}
		++it;
	}

	return unknown_addr_fam;
}

static RC_TYPE push_in_buffer(char* p_src, int src_len, char *p_buffer, int* p_act_len, int max_len)
{
	if (*p_act_len + src_len > max_len)
	{
		return RC_FILE_IO_OUT_OF_BUFFER;
	}
	memcpy(p_buffer + *p_act_len,p_src, src_len);
	*p_act_len += src_len;
	return RC_OK;
}

static RC_TYPE parser_init(OPTION_FILE_PARSER *p_cfg, FILE *p_file)
{
	memset(p_cfg, 0, sizeof(*p_cfg));

	p_cfg->state = NEW_LINE;
	p_cfg->p_file = p_file;

	return RC_OK;
}

/*	
	IP string parse, sans regex.

    Not fully error checked (for instance, could miss hex digits in IPv4 format, and there is no 
	range checking), but shouldn't be a problem with reasonably sane input (I don't imagine ip servers 
	are trying to trick clients :-)).   
*/
static RC_TYPE parser_read_ip(OPTION_FILE_PARSER *p_cfg,char *p_buffer,int maxlen,char *ch,int *count,int *parse_end)
{

	#define HEX_DIGITS		"0123456789aAbBcCdDeEfF"
	#define DEC_DIGITS		"0123456789"

	RC_TYPE rc = RC_OK;


	switch (p_cfg->state)
	{
	case NEW_LINE:

		if (!(strcmp(ch,":")))
		{
			p_cfg->state=NEWLINE_COLON;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (strpbrk(ch, HEX_DIGITS))
		{
			p_cfg->state=NUMERIC;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		return rc;

	case NEWLINE_COLON:

		if (!(strcmp(ch,":")))
		{
			p_cfg->state=DBL_COLON_DETECT_A;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		return RC_IP_PARSER_INVALID_IP;

	case NUMERIC:

		if (strpbrk(ch, HEX_DIGITS))
		{
			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);
		}

		if (!(strcmp(ch,".")))
		{
			p_cfg->state=DOT_1;
				
			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);
		}

		if (!(strcmp(ch,":")))
		{
			p_cfg->state=COLON_DETECT_1;
				
			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);
		}

		return RC_IP_PARSER_INVALID_IP;

	case DOT_1:

		if (strpbrk(ch, DEC_DIGITS))
		{
			p_cfg->state=IPV4_NUMERIC_2;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		return RC_IP_PARSER_INVALID_IP;

	case DOT_2:

		if (strpbrk(ch, DEC_DIGITS))
		{
			p_cfg->state=IPV4_NUMERIC_3;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		return RC_IP_PARSER_INVALID_IP;

	case DOT_3:

		if (strpbrk(ch, DEC_DIGITS))
		{
			p_cfg->state=IPV4_NUMERIC_4;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		return RC_IP_PARSER_INVALID_IP;

	case IPV4_NUMERIC_2:

		if (strpbrk(ch, DEC_DIGITS))
		{
			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,".")))
		{
			p_cfg->state=DOT_2;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);
		}

		return RC_IP_PARSER_INVALID_IP;

	case IPV4_NUMERIC_3:

		if (strpbrk(ch, DEC_DIGITS))
		{
			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,".")))
		{
			p_cfg->state=DOT_3;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);
		}

		return RC_IP_PARSER_INVALID_IP;

	case IPV4_NUMERIC_4:

		if (strpbrk(ch, DEC_DIGITS))
		{
			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		*parse_end=1;

		return rc;

	case COLON_DETECT_1:

		if (strpbrk(ch, HEX_DIGITS))
		{
			p_cfg->state=IPV6_NUMERIC_2;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,":")))
		{
			p_cfg->state=DBL_COLON_DETECT_B;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		return RC_IP_PARSER_INVALID_IP;

	case COLON_DETECT_2:

		if (strpbrk(ch, HEX_DIGITS))
		{
			p_cfg->state=IPV6_NUMERIC_3;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,":")))
		{
			p_cfg->state=DBL_COLON_DETECT_B;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		return RC_IP_PARSER_INVALID_IP;

	case COLON_DETECT_3:

		if (strpbrk(ch, HEX_DIGITS))
		{
			p_cfg->state=IPV6_NUMERIC_4;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,":")))
		{
			p_cfg->state=DBL_COLON_DETECT_B;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		return RC_IP_PARSER_INVALID_IP;

	case COLON_DETECT_4:

		if (strpbrk(ch, HEX_DIGITS))
		{
			p_cfg->state=IPV6_NUMERIC_5;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,":")))
		{
			p_cfg->state=DBL_COLON_DETECT_B;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		return RC_IP_PARSER_INVALID_IP;

	case COLON_DETECT_5:

		if (strpbrk(ch, HEX_DIGITS))
		{
			p_cfg->state=IPV6_NUMERIC_6;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,":")))
		{
			p_cfg->state=DBL_COLON_DETECT_B;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		return RC_IP_PARSER_INVALID_IP;

	case COLON_DETECT_6:

		if (strpbrk(ch, HEX_DIGITS))
		{
			p_cfg->state=IPV6_NUMERIC_7;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,":")))
		{
			p_cfg->state=DBL_COLON_DETECT_B;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		return RC_IP_PARSER_INVALID_IP;

	case COLON_DETECT_7:

		if (strpbrk(ch, HEX_DIGITS))
		{
			p_cfg->state=IPV6_NUMERIC_8;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,":")))
		{
			*parse_end=1;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);
		}

		return RC_IP_PARSER_INVALID_IP;

	case IPV6_NUMERIC_2:

		if (strpbrk(ch, HEX_DIGITS))
		{
			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,":")))
		{
			p_cfg->state=COLON_DETECT_2;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		return RC_IP_PARSER_INVALID_IP;

	case IPV6_NUMERIC_3:

		if (strpbrk(ch, HEX_DIGITS))
		{
			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,":")))
		{
			p_cfg->state=COLON_DETECT_3;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		return RC_IP_PARSER_INVALID_IP;

	case IPV6_NUMERIC_4:

		if (strpbrk(ch, HEX_DIGITS))
		{
			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,":")))
		{
			p_cfg->state=COLON_DETECT_4;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		return RC_IP_PARSER_INVALID_IP;

	case IPV6_NUMERIC_5:

		if (strpbrk(ch, HEX_DIGITS))
		{
			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,":")))
		{
			p_cfg->state=COLON_DETECT_5;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		return RC_IP_PARSER_INVALID_IP;

	case IPV6_NUMERIC_6:

		if (strpbrk(ch, HEX_DIGITS))
		{
			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,":")))
		{
			p_cfg->state=COLON_DETECT_6;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);
		}

		return RC_IP_PARSER_INVALID_IP;

	case IPV6_NUMERIC_7:

		if (strpbrk(ch, HEX_DIGITS))
		{
			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,":")))
		{
			p_cfg->state=COLON_DETECT_7;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,".")))
		{
			p_cfg->state=DOT_1;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		return RC_IP_PARSER_INVALID_IP;

	case IPV6_NUMERIC_8:

		if (strpbrk(ch, HEX_DIGITS))
		{
			p_cfg->state=IPV6_FINAL;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		return RC_IP_PARSER_INVALID_IP;

	case IPV6_FINAL:

		if (strpbrk(ch, HEX_DIGITS))
		{
			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		*parse_end=1;

		return rc;

	case DBL_COLON_A:

		if (strpbrk(ch, HEX_DIGITS))
		{
			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,":")))
		{
			p_cfg->state=DBL_COLON_DETECT_A;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,".")))
		{
			p_cfg->state=DOT_1;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);
		}

		*parse_end=1;

		return rc;

	case DBL_COLON_B:

		if (strpbrk(ch, HEX_DIGITS))
		{
			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,":")))
		{
			p_cfg->state=DBL_COLON_DETECT_B;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,".")))
		{
			return RC_IP_PARSER_INVALID_IP;
		}

		*parse_end=1;

		return rc;

	case DBL_COLON_DETECT_A:

		if (strpbrk(ch, HEX_DIGITS))
		{
			p_cfg->state=DBL_COLON_A;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,":")))
		{
			return RC_IP_PARSER_INVALID_IP;
		}

		*parse_end=1;

		return rc;

	case DBL_COLON_DETECT_B:

		if (strpbrk(ch, HEX_DIGITS))
		{
			p_cfg->state=DBL_COLON_B;

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);			
		}

		if (!(strcmp(ch,":")))
		{
			return RC_IP_PARSER_INVALID_IP;
		}

		*parse_end=1;

		return rc;

	default:

		return rc;
	}
}

/*	assume ip is in str_ip, and that any errors are because have yet
	to parse passed preliminary characters.
*/
RC_TYPE parse_ip_address(char **str_ip,char *in_str)
{
	RC_TYPE				rc = RC_OK;
	int					parse_end = 0;
	int					count = 0;
	char				ch[2];
	OPTION_FILE_PARSER	p_cfg;
	char				*p_buffer;
	int					maxlen;

	/*	TODO:	range check IPv4 digit groups
	*/

	if (!(in_str))

		return RC_INVALID_POINTER;

	*str_ip=NULL;

	maxlen=strlen(in_str);

	p_buffer=safe_malloc(maxlen+1);

	*p_buffer='\0';

	parser_init(&p_cfg,NULL);

	
	while(!(parse_end) && *in_str) {

		memset(ch,0,2);

		strncpy(ch,in_str++,1);

		if (!((rc=parser_read_ip(&p_cfg,p_buffer,maxlen,ch,&count,&parse_end))==RC_OK)) {

			memset(p_buffer,0,maxlen+1);

			count=0;

			parser_init(&p_cfg,NULL);
		}
	}

	if (rc==RC_OK) {

		push_in_buffer("\0",1,p_buffer,&count,maxlen);

		*str_ip=safe_malloc(strlen(p_buffer)+1);

		strcpy(*str_ip,p_buffer);	

		if (!(strcmp(*str_ip,"")))

			rc=RC_IP_PARSER_INVALID_IP;
	}

	free(p_buffer);

	return rc;
}

