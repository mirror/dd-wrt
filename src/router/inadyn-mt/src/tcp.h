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
	  - Non-blocking socket create, and connect
*/


/* interface for tcp functions */

#ifndef _TCP_H_INCLUDED
#define _TCP_H_INCLUDED

#ifdef USE_THREADS

#include "threads_wrapper.h"

#endif

#include "os.h"
#include "errorcode.h"
#include "ip.h"

/* SOME DEFAULT CONFIGURATIONS */
#define TCP_DEFAULT_TIMEOUT	20000 /*ms*/


typedef struct 
{
	IP_SOCKET super;
	BOOL is_constructed;
	BOOL initialized;
} TCP_SOCKET;

typedef struct
{
	TCP_SOCKET super;
} TCP_CLIENT_SOCKET;

#ifdef USE_THREADS

/*	test whether can connect while doing timeout 
	tests, and client exit test callbacks
*/
typedef int (*CB_EXIT_COND)(void*);

typedef struct CONNECT_THREAD_DATA {

	mutex_t				t_data_mutex;
	semaphore_t			t_data_sem;

	TCP_SOCKET			*p_self;
	volatile BOOL		is_thread_exit;
	volatile BOOL		is_parent_exit;

	volatile RC_TYPE	rc;

} CONNECT_THREAD_DATA;

#endif

/*public functions*/

/*
	 basic resource allocations for the tcp object
*/
RC_TYPE tcp_construct(TCP_SOCKET *p_self);

/*
	CAUTION:  see the function and super to see just what is "cloned" in present implementation.
*/
RC_TYPE tcp_clone(TCP_SOCKET **p_self_dest,TCP_SOCKET *p_self_src);

/*
	Resource free.
*/	
RC_TYPE tcp_destruct(TCP_SOCKET *p_self);

/* 
	Sets up the object.

	- ...
*/
RC_TYPE tcp_initialize(TCP_SOCKET *p_self);
/*return connections to all can from getaddrinfo*/
RC_TYPE tcp_initialize_all(TCP_SOCKET *p_self);
RC_TYPE tcp_do_initialize(TCP_SOCKET *p_self,int is_connect_all);

/* 
	Disconnect and some other clean up.
*/
RC_TYPE tcp_shutdown(TCP_SOCKET *p_self);


/* send data*/
RC_TYPE tcp_send(TCP_SOCKET *p_self, const char *p_buf, int len);

/* receive data*/
RC_TYPE tcp_recv(TCP_SOCKET *p_self, char *p_buf, int max_recv_len, int *p_recv_len);

#ifdef USE_THREADS

/* test if success connect -- with timeout, and client exit callbacks*/
RC_TYPE tcp_test_connect(TCP_SOCKET *p_self,CB_EXIT_COND p_exit_func,void *p_cb_data);

/*initialize/connect -- with timeout, and client exit callbacks*/
RC_TYPE tcp_initialize_async(TCP_SOCKET *p_self,CB_EXIT_COND p_exit_func,void *p_cb_data);

#endif

/* Accessors */

RC_TYPE tcp_set_port(TCP_SOCKET *p_self, int p);
RC_TYPE tcp_set_remote_name(TCP_SOCKET *p_self, const char* p);
RC_TYPE tcp_set_remote_timeout(TCP_SOCKET *p_self, int t);
RC_TYPE tcp_set_is_ipv4(TCP_SOCKET *p_self, BOOL is_ipv4);

RC_TYPE tcp_get_port(TCP_SOCKET *p_self, int *p_port);
RC_TYPE tcp_get_remote_name(TCP_SOCKET *p_self, const char* *p);
RC_TYPE tcp_get_remote_timeout(TCP_SOCKET *p_self, int *p);
RC_TYPE tcp_get_is_ipv4(TCP_SOCKET *p_self, BOOL *is_ipv4);


#endif /*_TCP_H_INCLUDED*/
