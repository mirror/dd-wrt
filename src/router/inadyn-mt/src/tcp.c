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
	  - Windows socket layer shutdown per startup
*/
#define MODULE_TAG      "TCP: "  

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#include <stdlib.h>
#include <string.h>

#include "tcp.h"
#include "safe_mem.h"
#include "debug_if.h"

#define super_construct(p)					ip_construct(p)
#define super_clone(p_dest,p_src)			ip_clone(p_dest,p_src)
#define super_destruct(p)					ip_destruct(p)
#define super_initialize(p)					ip_initialize(p)
#define super_shutdown(p)					ip_shutdown(p)
#define super_send(p,p_buf,len)				ip_send(p,p_buf,len)
#define super_recv(p,p_buf,len,p_len)		ip_recv(p,p_buf,len,p_len)
#define super_set_port(p_self, p)			ip_set_port(p_self, p)
#define super_get_port(p_self, p)			ip_get_port(p_self, p)
#define super_set_remote_name(p_self, p)	ip_set_remote_name(p_self, p)
#define super_get_remote_name(p_self, p)	ip_get_remote_name(p_self, p)
#define super_set_remote_timeout(p_self, p)	ip_set_remote_timeout(p_self, p)
#define super_get_remote_timeout(p_self, p)	ip_get_remote_timeout(p_self, p)
#define super_get_is_ipv4(p_self,is_ipv4)	ip_get_is_ipv4(p_self,is_ipv4)
#define super_set_is_ipv4(p_self,is_ipv4)	ip_set_is_ipv4(p_self,is_ipv4)

#ifdef USE_THREADS

/*mutex storage*/
static mutex_t		connect_in_progress_mutex=MUTEX_T_INIT;
static mutex_t		timer_loop_mutex=MUTEX_T_INIT;
static mutex_t		test_timer_loop_mutex=MUTEX_T_INIT;

static volatile int			is_connect_in_progress=0;
static volatile RC_TYPE		global_is_online=RC_OK;

#endif

/*
	 basic resource allocations for the tcp object
*/
RC_TYPE tcp_construct(TCP_SOCKET *p_self)
{
	RC_TYPE rc;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (p_self->is_constructed) 
	{
		return RC_OK;
	}

	rc = super_construct(&p_self->super);

	if (rc != RC_OK)
	{
		return rc;
	}

	/*reset its part of the struct (skip IP part)*/
	memset( ((char*)p_self + sizeof(p_self->super)) , 0, sizeof(*p_self) - sizeof(p_self->super));
	p_self->initialized = FALSE;
	p_self->is_constructed=TRUE;
	
	return RC_OK;
}

/*
	This is not a "complete" clone.  Take care to see what is/isn't coppied.
*/
RC_TYPE tcp_clone(TCP_SOCKET **p_self_dest,TCP_SOCKET *p_self_src)
{

	if (p_self_src == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (p_self_dest == NULL)
	{
		return RC_INVALID_POINTER;
	}

	*p_self_dest=safe_malloc(sizeof(TCP_SOCKET));

	tcp_construct(*p_self_dest);

	super_clone(&(*p_self_dest)->super,&p_self_src->super);


	return RC_OK;
}

/*
	Resource free.
*/	
RC_TYPE tcp_destruct(TCP_SOCKET *p_self)
{
	RC_TYPE	rc;


	if (p_self == NULL)
	{
		return RC_OK;
	}

	super_destruct(&p_self->super);

	if (!(p_self->is_constructed))
	{
		return RC_OK;
	}

	rc=tcp_shutdown(p_self);

	memset(p_self,0,sizeof(TCP_SOCKET));
		
	return rc;
}

static RC_TYPE local_set_params(TCP_SOCKET *p_self)
{
	int timeout;
	/*set default TCP specififc params*/
	tcp_get_remote_timeout(p_self, &timeout);

	if (timeout == 0)
	{
		tcp_set_remote_timeout(p_self, TCP_DEFAULT_TIMEOUT);
	}
	return RC_OK;
}

static RC_TYPE do_tcp_create_socket(TCP_SOCKET *p_self,LINGER so_linger,int timeout)
{
	RC_TYPE	rc=RC_IP_SOCKET_CREATE_ERROR;


	do {

		local_set_params(p_self);

		/*call the super*/
		rc = super_initialize(&p_self->super);

		if (rc != RC_OK)
		{
			break;
		}

		if (!(p_self->super.type == TYPE_TCP))
		{
			rc = RC_IP_BAD_PARAMETER;
		}
		else {			

			struct addrinfo *addr=p_self->super.addr;
			int				socket_index=0;


			do {

				p_self->super.socket[socket_index]=
					socket(addr->ai_family,addr->ai_socktype,addr->ai_protocol);

				p_self->super.addr_ar[socket_index]=addr;

				addr=addr->ai_next;

				/*only error if no sockets at all created*/
				if (!(p_self->super.socket[socket_index] == INVALID_SOCKET)) {

					rc=RC_OK;

					/* set timeouts */
					setsockopt(p_self->super.socket[socket_index],SOL_SOCKET,SO_RCVTIMEO,
						(char*) &timeout,sizeof(timeout));

					setsockopt(p_self->super.socket[socket_index],SOL_SOCKET,SO_SNDTIMEO,
						(char*) &timeout,sizeof(timeout));

					setsockopt(p_self->super.socket[socket_index],SOL_SOCKET,SO_LINGER,
						(char *) &so_linger,sizeof(LINGER));									

					if (!(addr)) {

						break;
					}
					else {

						/*socket already 1 from ip construct*/

						p_self->super.server_socket_count++;

						p_self->super.socket=safe_realloc(p_self->super.socket,sizeof(SOCKET)*
							p_self->super.server_socket_count);

						p_self->super.addr_ar=safe_realloc(p_self->super.addr_ar,sizeof(struct addrinfo *)*
							p_self->super.server_socket_count);

						socket_index++;

						p_self->super.socket[socket_index]=INVALID_SOCKET;
						p_self->super.addr_ar[socket_index]=NULL;
					}
				}				

			} while(addr);
		}
	} 
	while(0);

	return rc;
}

RC_TYPE tcp_create_socket(TCP_SOCKET *p_self)
{
	LINGER	so_linger;

	memset(&so_linger,0,sizeof(LINGER));

	/*document*/
	so_linger.l_linger=0;
	so_linger.l_onoff=1;


	return do_tcp_create_socket(p_self,so_linger,p_self->super.timeout);
}

/* 
	Sets up the object.
	- ...
*/
RC_TYPE tcp_do_initialize(TCP_SOCKET *p_self,int is_connect_all)
{
	RC_TYPE			rc;
	int				i=0;


	do
	{
		if (!(RC_OK==(rc=tcp_create_socket(p_self))))

			break;

		rc=RC_IP_CONNECT_FAILED;


		for (i=0;i<p_self->super.server_socket_count;i++) {

			/*connect*/
			if (0 == connect(p_self->super.socket[i],p_self->super.addr_ar[i]->ai_addr,
				p_self->super.addr_ar[i]->ai_addrlen)) {

				rc=RC_OK;

				if (is_connect_all) {

					p_self->super.sock_index=0;
				}
				else {

					p_self->super.sock_index=i;

					break;
				}
			}
		}
	}
	while(0);

	if (rc != RC_OK)
	{
		tcp_shutdown(p_self);		
	}
	else
	{
		p_self->initialized = TRUE;
	}
			
	return rc;
}

RC_TYPE tcp_initialize_all(TCP_SOCKET *p_self)
{

	return tcp_do_initialize(p_self,1);
}

RC_TYPE tcp_initialize(TCP_SOCKET *p_self)
{

	return tcp_do_initialize(p_self,0);
}

/* 
	Disconnect and some other clean up.
*/
RC_TYPE tcp_shutdown(TCP_SOCKET *p_self)
{
	RC_TYPE	rc;


	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	rc=super_shutdown(&p_self->super);

	p_self->initialized=FALSE;


	return rc;
}

/* send data*/
RC_TYPE tcp_send(TCP_SOCKET *p_self, const char *p_buf, int len)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (!p_self->initialized)
	{
		return RC_TCP_OBJECT_NOT_INITIALIZED;
	}
	return super_send(&p_self->super, p_buf, len);
}

/* receive data*/
RC_TYPE tcp_recv(TCP_SOCKET *p_self,char *p_buf, int max_recv_len, int *p_recv_len)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (!p_self->initialized)
	{
		return RC_TCP_OBJECT_NOT_INITIALIZED;
	}
	return super_recv(&p_self->super, p_buf, max_recv_len, p_recv_len);
}

/* Accessors*/
RC_TYPE tcp_set_port(TCP_SOCKET *p_self, int p)
{
	return super_set_port(&p_self->super, p);
}

RC_TYPE tcp_set_remote_name(TCP_SOCKET *p_self, const char* p)
{
	return super_set_remote_name(&p_self->super, p);
}

RC_TYPE tcp_set_remote_timeout(TCP_SOCKET *p_self, int p)
{
	return super_set_remote_timeout(&p_self->super, p);
}

RC_TYPE tcp_set_is_ipv4(TCP_SOCKET *p_self, BOOL is_ipv4)
{
	return super_set_is_ipv4(&p_self->super,is_ipv4);
}

RC_TYPE tcp_get_port(TCP_SOCKET *p_self, int *p)
{
	return super_get_port(&p_self->super, p);
}

RC_TYPE tcp_get_remote_name(TCP_SOCKET *p_self, const char* *p)
{
	return super_get_remote_name(&p_self->super, p);
}

RC_TYPE tcp_get_remote_timeout(TCP_SOCKET *p_self, int *p)
{
	return super_get_remote_timeout(&p_self->super, p);
}

RC_TYPE tcp_get_is_ipv4(TCP_SOCKET *p_self, BOOL *is_ipv4)
{
	return super_get_is_ipv4(&p_self->super,is_ipv4);
}

/*
	Some fun with experimental stuff -- presently used to test whether online -- launches a thread that 
	attempts socket create, connect and sets global online status based on success or fail.  thread can 
	be orphaned	if timeout.  successive async calls will be effectively queued waiting for success/fail 
	setting of any previous caller.  Interface is tcp_test_connect.  bhoover@wecs.com
*/
#ifdef USE_THREADS

/*set opts no linger, and quick timeout*/
static RC_TYPE tcp_create_quick_socket(TCP_SOCKET *p_self)
{
	LINGER	so_linger;

	memset(&so_linger,0,sizeof(LINGER));

	/*document*/
	so_linger.l_linger=0;
	so_linger.l_onoff=1;
	
	return do_tcp_create_socket(p_self,so_linger,5000);	
}

static RC_TYPE do_connect(TCP_SOCKET *p_self)
{

	RC_TYPE			rc=RC_IP_CONNECT_FAILED;
	struct addrinfo	*addr;
	int				i;


	for (i=0;i<p_self->super.server_socket_count;i++) {

		addr=p_self->super.addr_ar[i];

		/*connect*/
		if (0 == connect(p_self->super.socket[i],addr->ai_addr,addr->ai_addrlen)) {

			rc=RC_OK;

			break;
		}
	}

	return rc;
}

static void destroy_connect_data(CONNECT_THREAD_DATA **p_data)
{

	tcp_destruct((*p_data)->p_self);
		
	free((*p_data)->p_self);

	destroy_mutex(&(*p_data)->t_data_mutex);
	destroy_semaphore(&(*p_data)->t_data_sem);
	
	free(*p_data);
	*p_data=NULL;
}

static CONNECT_THREAD_DATA *create_connect_data(void *p_data)
{
	CONNECT_THREAD_DATA	*p_thread_data=NULL;


	p_thread_data=safe_malloc(sizeof(CONNECT_THREAD_DATA));


	if (!(p_thread_data)) {

		DBG_PRINTF((LOG_CRIT,"C:" MODULE_TAG "failed allocating connect thread data in create_connect_data...\n"));

		return NULL;
	}
	else {

		memset(p_thread_data,0,sizeof(CONNECT_THREAD_DATA));
		p_thread_data->p_self=p_data;
		p_thread_data->rc=RC_IP_CONNECT_FAILED;

		create_mutex(&p_thread_data->t_data_mutex);
		create_semaphore(&p_thread_data->t_data_sem);

		return p_thread_data;
	}
}

#ifndef _WIN32
static void *connect_thread(void *p_data)
#else
static void connect_thread(void *p_data)
#endif
{
	CONNECT_THREAD_DATA	*p_ct_data=(CONNECT_THREAD_DATA *) p_data;
	TCP_SOCKET			*p_self;


	if (!(p_ct_data)) {

		DBG_PRINTF((LOG_CRIT,"C:" MODULE_TAG "p_ct_data null in connect_thread...\n"));
	}
	else {

		p_self=p_ct_data->p_self;

		if (!(p_self)) {

			DBG_PRINTF((LOG_CRIT,"C:" MODULE_TAG "p_ct_data->p_self null in connect_thread...\n"));

			p_ct_data->rc=RC_INVALID_POINTER;
		}
		else {

			DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "attempting to create socket in connect_thread...\n"));
		
			if (!(tcp_create_quick_socket(p_self)==RC_OK)) {

				global_is_online=RC_IP_CONNECT_FAILED;

				DBG_PRINTF((LOG_CRIT,"C:" MODULE_TAG "failed socket create in connect_thread...\n"));
			}
			else {

				DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "calling do_connect from connect_thread\n"));

				if (RC_OK==(p_ct_data->rc=do_connect(p_self))) {

					p_self->initialized=TRUE;
				}

				global_is_online=p_ct_data->rc;

				DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "connect thread returned from connect, connected: %d\n",(p_ct_data->rc==RC_OK)));
			}
		}

		p_ct_data->is_thread_exit=1;

		get_mutex(&p_ct_data->t_data_mutex);
		signal_sem(&p_ct_data->t_data_sem);
		release_mutex(&p_ct_data->t_data_mutex);

		if (p_ct_data->is_parent_exit) {

			/*we're orphaned, and in case program still running, deallocate what can*/

			DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "connect thread destroying parent data...\n"));

			destroy_connect_data(&p_ct_data);
		}

		DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "connect thread exiting...\n"));
	}

	get_mutex(&connect_in_progress_mutex);
	is_connect_in_progress-=1;
	release_mutex(&connect_in_progress_mutex);

#ifdef _WIN32
	_endthread();
#else
	pthread_exit(p_ct_data);

	/*compiler complaints (pthread_exit does not return)*/
	return p_ct_data;
#endif

}

#endif

#ifdef USE_THREADS

#ifndef _WIN32
static void exit_thread(pthread_t thread,void **p_thread_data)
#else
static void exit_thread(unsigned long thread,void **p_thread_data)
#endif
{

#ifndef _WIN32	

	if (thread)

		pthread_join(thread,p_thread_data);		

#else

	if (thread)
		
		WaitForSingleObject((HANDLE) thread,INFINITE);
#endif

}

#ifndef _WIN32
static void create_connect_thread(pthread_t *thread,CONNECT_THREAD_DATA *p_thread_data)
#else
static void create_connect_thread(unsigned long *thread,CONNECT_THREAD_DATA *p_thread_data)
#endif
{

#ifdef _WIN32
	*thread=_beginthread(connect_thread,0,(void *) p_thread_data);
#else
	pthread_create(thread,NULL,connect_thread,(void *) p_thread_data);
#endif

}

#ifndef _WIN32
static int kill_thread(pthread_t thread)
#else
static int kill_thread(unsigned long thread)
#endif
{

#ifndef _WIN32
	return pthread_detach(thread);
/*	return pthread_cancel(thread);*/
#else
	return 0;
#endif

}

/*
	-create a thread to create and connect a socket
	-orphan the thread if does not return within timeout period
	-return socket connection success/fail
	-limit number of connection threads orphaned at any 
	 given time so that if one is orphaned, return connection 
	 fail

	Algorithm assumption is that if orphaned thread(s) then network problems,
	and probably due to not online.  A more general solution would take into
	account that different callers may/probably would have different dests
	and any problems could be on the dest end
*/
RC_TYPE tcp_initialize_async(TCP_SOCKET *p_self,CB_EXIT_COND p_exit_func,void *p_cb_data)
{
#ifndef _WIN32
	pthread_t			c_thread=0;
#else
	unsigned long		c_thread=0;
#endif
	CONNECT_THREAD_DATA		*p_thread_data;
	CONNECT_THREAD_DATA		*p_thread_return=NULL;
	int						sleep_time=0;
	RC_TYPE					ret_rc;
	TCP_SOCKET				*p_this_self;
	int						is_exit_request=0;
	int						is_t_data_mutex_released=0;
	int						is_got_test_timer_mutex;


	DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "Entered tcp_initialize_async...\n"));		

	if (!(p_self)) {

		DBG_PRINTF((LOG_CRIT,"C:" MODULE_TAG "p_self null in tcp_initialize_async...\n"));		

		return RC_INVALID_POINTER;
	}


	get_mutex(&connect_in_progress_mutex);

	if (!(is_connect_in_progress)) {

		/*no orphaned thread, not in timer loop*/

		get_mutex(&timer_loop_mutex);
	}
	else {

		/*orphaned thread, or still in timer loop -- don't let still in loop prevent us an attempt*/

		if ((get_mutex_try(&timer_loop_mutex)==0)) {

			/*we've orphaned a connection thread because of timeout -- just return connection failure*/

			DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "connection in progress in tcp_initialize_async...\n"));

			release_mutex(&timer_loop_mutex);
			release_mutex(&connect_in_progress_mutex);

			return RC_IP_CONNECT_FAILED;
		}
	}

	is_connect_in_progress+=1;
	release_mutex(&connect_in_progress_mutex);

	/*if client requests exit we may abandon connect thread
	  so rallocate client's p_self data -- we'll deallocate
	  this if connect thread exits normally -- so our
	  dangling thread does not have p_self data pulled out
	  from under it when/if our client quits*/

	/*see the function to see just what is/isn't cloned*/
	tcp_clone(&p_this_self,p_self);
	p_self=p_this_self;

	is_got_test_timer_mutex=(get_mutex_try(&test_timer_loop_mutex)==0);
	p_thread_data=create_connect_data(p_self);

	get_mutex(&p_thread_data->t_data_mutex);
	create_connect_thread(&c_thread,p_thread_data);
		
	/*loop/sleep 'til thread returns or prog exit requested*/

	while(!(p_thread_data->is_thread_exit) && !((is_exit_request=p_exit_func(p_cb_data)))) {		

		sleep_time+=sleep_lightly_ms(1000,p_exit_func,p_cb_data);

		if (!(sleep_time<p_self->super.timeout)) {
			
			DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "tcp_initialize_async timed out...\n"));		

			break;
		}
	}

	if (p_thread_data->is_thread_exit) {

		release_mutex(&p_thread_data->t_data_mutex);
		is_t_data_mutex_released=1;

	} 
	else {

		DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "connect thread has not exited...attempting to shutdown socket in tcp_initialize_async...\n"));

		if ((kill_thread(c_thread)==0)) {
			
			c_thread=0;

			DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "killed connect thread in tcp_initialize_async...\n"));
		}

	}

	if (c_thread) {

		DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "tcp_initialize_async joinng connect thread...\n"));

		exit_thread(c_thread,(void **) &p_thread_return);
	}

	DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "tcp_initialize_async returning...\n"));

	if (is_exit_request)

		ret_rc=global_is_online;

	else {

		global_is_online=p_thread_data->rc;
		ret_rc=global_is_online;
	}

	if (is_got_test_timer_mutex)
		release_mutex(&test_timer_loop_mutex);

	release_mutex(&timer_loop_mutex);

	/*if program not quiting, and thread exited, free assocated data*/
	if (!(p_thread_data->is_thread_exit)) {

		p_thread_data->is_parent_exit=1;

		release_mutex(&p_thread_data->t_data_mutex);
	} 
	else {

		if (!(is_t_data_mutex_released))
			release_mutex(&p_thread_data->t_data_mutex);

		wait_sem(&p_thread_data->t_data_sem);

		DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "destroying connect data in tcp_initialize_async...\n"));

		destroy_connect_data(&p_thread_data);
	}

	return ret_rc;
}

RC_TYPE tcp_test_connect(TCP_SOCKET *p_self,CB_EXIT_COND p_exit_func,void *p_cb_data)
{

	RC_TYPE	rc_is_online=RC_OK;


	DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "entered tcp_test_connect...\n"));

	if (get_mutex_try(&test_timer_loop_mutex)==0) {

		DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "timer loop mutex is free in tcp_test_connect...\n"));

		rc_is_online=tcp_initialize_async(p_self,p_exit_func,p_cb_data);

		release_mutex(&test_timer_loop_mutex);

		DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "tcp_initialize_async returned %d in tcp_test_connect...\n",(rc_is_online==RC_OK)));

	}
	else {

		/*init async presently in timeout loop -- wait for it*/

		DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "timer loop mutex is not free in tcp_test_connect...\n"));

		get_mutex(&test_timer_loop_mutex);
		release_mutex(&test_timer_loop_mutex);

		rc_is_online=global_is_online;

		DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "got timer loop mutex in tcp_test_connect, return to be %d...\n",(rc_is_online==RC_OK)));
	}

	return rc_is_online;
}

#endif /*USE_THREADS*/
