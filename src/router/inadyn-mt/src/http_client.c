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

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#include <string.h>
#include "http_client.h"
#include "errorcode.h"


#define super_construct(p)					tcp_construct(p)
#define super_destruct(p)					tcp_destruct(p)
#define super_initialize(p,is_init_all)		tcp_do_initialize(p,is_init_all)
#define super_init_async(p)					tcp_initialize_async(p,p_exit_func,p_cb_data)
#define super_test_connect(p)				tcp_test_connect(p,p_exit_func,p_cb_data)
#define super_shutdown(p)					tcp_shutdown(p)
#define super_set_port(p_self,p)			tcp_set_port(p_self,p)
#define super_get_port(p_self,p)			tcp_get_port(p_self,p)
#define super_set_remote_name(p_self,p)		tcp_set_remote_name(p_self,p)
#define super_get_remote_name(p_self,p)		tcp_get_remote_name(p_self,p)
#define super_set_remote_timeout(p_self,p)	tcp_set_remote_timeout(p_self,p)
#define super_get_remote_timeout(p_self,p)	tcp_get_remote_timeout(p_self,p)
#define super_get_is_ipv4(p_self,is_ipv4)	tcp_get_is_ipv4(p_self,is_ipv4)
#define super_set_is_ipv4(p_self,is_ipv4)	tcp_set_is_ipv4(p_self,is_ipv4)

/*public functions*/

/*
	 basic resource allocations for the tcp object
*/
RC_TYPE http_client_construct(HTTP_CLIENT *p_self)
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

	/*init*/
	memset( (char*)p_self + sizeof(p_self->super), 0 , sizeof(*p_self) - sizeof(p_self->super));
	p_self->initialized = FALSE;
	p_self->is_constructed=TRUE;

	return RC_OK;
}

/*
	Resource free.
*/	
RC_TYPE http_client_destruct(HTTP_CLIENT *p_self)
{
	RC_TYPE	rc;


	if (p_self==NULL)
	{
		return RC_OK;
	}

	rc=super_destruct(&p_self->super);

	if (!(p_self->is_constructed))
	{
		return RC_OK;
	}

	http_client_shutdown(p_self);

	memset(p_self,0,sizeof(HTTP_CLIENT));
	

	/*free*/
	return rc;
}


static RC_TYPE local_set_params(HTTP_CLIENT *p_self)
{
	{
		int timeout;
		/*set default TCP specififc params*/
		http_client_get_remote_timeout(p_self, &timeout);

		if (timeout == 0)
		{
			http_client_set_remote_timeout(p_self, HTTP_DEFAULT_TIMEOUT);
		}
	}

	{
		int port;
		http_client_get_port(p_self, &port);
		if ( port == 0)
		{
			http_client_set_port(p_self, HTTP_DEFAULT_PORT);
		}
	}

	return RC_OK;
}

RC_TYPE http_client_do_init(HTTP_CLIENT *p_self,int is_init_all)
{
	RC_TYPE rc;

	do
	{
		rc = local_set_params(p_self);

		if (rc != RC_OK)
		{
			break;
		}


		rc = super_initialize(&p_self->super,is_init_all);

		if (rc != RC_OK)
		{
			break;
		}

		/*local init*/

	}
	while(0);

	if (rc != RC_OK)
	{
		http_client_shutdown(p_self);
	}
	else
	{
		p_self->initialized = TRUE;
	}

	return rc;
}

RC_TYPE http_client_init(HTTP_CLIENT *p_self)
{

	return http_client_do_init(p_self,0);
}

RC_TYPE http_client_init_all(HTTP_CLIENT *p_self)
{

	return http_client_do_init(p_self,1);
}

/*
	Disconnect and some other clean up.
*/
RC_TYPE http_client_shutdown(HTTP_CLIENT *p_self)
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

/* Send req and get response */
RC_TYPE http_client_transaction(HTTP_CLIENT *p_self, HTTP_TRANSACTION *p_tr )
{
	RC_TYPE rc;
	if (p_self == NULL || p_tr == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (!p_self->initialized)
	{
		return RC_HTTP_OBJECT_NOT_INITIALIZED;
	}

	do
	{
		rc = tcp_send(&p_self->super, p_tr->p_req, p_tr->req_len);
		if (rc != RC_OK)
		{
			break;
		}

		rc = tcp_recv(&p_self->super, p_tr->p_rsp, p_tr->max_rsp_len, &p_tr->rsp_len);
	}
	while(0);

	return rc;
}

/* Accessors */

RC_TYPE http_client_set_port(HTTP_CLIENT *p_self, int p)
{
	return super_set_port(&p_self->super, p);
}

RC_TYPE http_client_set_remote_name(HTTP_CLIENT *p_self, const char* p)
{
	return super_set_remote_name(&p_self->super, p);
}

RC_TYPE http_client_set_remote_timeout(HTTP_CLIENT *p_self, int p)
{
	return super_set_remote_timeout(&p_self->super, p);
}

RC_TYPE http_client_set_is_ipv4(HTTP_CLIENT *p_self, BOOL is_ipv4)
{
	return super_set_is_ipv4(&p_self->super,is_ipv4);
}

RC_TYPE http_client_get_port(HTTP_CLIENT *p_self, int *p)
{
	return super_get_port(&p_self->super, p);
}
RC_TYPE http_client_get_remote_name(HTTP_CLIENT *p_self, const char* *p)
{
	return super_get_remote_name(&p_self->super, p);
}
RC_TYPE http_client_get_remote_timeout(HTTP_CLIENT *p_self, int *p)
{
	return super_get_remote_timeout(&p_self->super, p);
}

RC_TYPE http_client_get_is_ipv4(HTTP_CLIENT *p_self, BOOL *is_ipv4)
{
	return super_get_is_ipv4(&p_self->super,is_ipv4);
}

#ifdef USE_THREADS

RC_TYPE http_client_init_async(HTTP_CLIENT *p_self,CB_EXIT_COND p_exit_func,void *p_cb_data)
{
	RC_TYPE rc;
	do
	{
		/*set local params*/
		rc = local_set_params(p_self);
		if (rc != RC_OK)
		{
			break;
		}


		/*call super*/
		rc = super_init_async(&p_self->super);
		if (rc != RC_OK)
		{
			break;
		}

		/*local init*/

	}
	while(0);

	if (rc != RC_OK)
	{
		http_client_shutdown(p_self);
	}
	else
	{
		p_self->initialized = TRUE;
	}

	return rc;
}

RC_TYPE http_client_test_connect(HTTP_CLIENT *p_self,CB_EXIT_COND p_exit_func,void *p_cb_data)
{
	RC_TYPE	rc;


	if (!((rc=local_set_params(p_self))==RC_OK))
		
		return rc;

	return super_test_connect(&p_self->super);
}

#endif /*USE_THREADS*/
