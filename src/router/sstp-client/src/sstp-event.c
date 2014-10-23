/*!
 * @brief Event API for sstp-client
 *
 * @file sstp-event.c
 *
 * @author Copyright (C) 2011 Eivind Naess, 
 *      All Rights Reserved
 *
 * @par License:
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <config.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <paths.h>
#include <sys/un.h>
#include <unistd.h>
#include <netdb.h>
#include <sstp-api.h>

#include "sstp-private.h"
#include "sstp-client.h"


/*!
 * @brief The event notification context structure
 */
struct sstp_event 
{
    /*! The Unix Domain Socket */
    int sock;

    /*! The unix socket path+name */
    char sockname[SSTP_DFLT_BUFSZ+1];

    /*! Callback function to notfify of the result */
    sstp_event_fn event_cb;

    /*! The argument to pass event_cb */
    void *arg;

    /*! The receive key */
    sstp_api_attr_st *rkey;

    /*! The send key */
    sstp_api_attr_st *skey;

    /*! Event listener */
    event_st *ev_event;
};


static int sstp_event_auth(sstp_event_st *ctx, int sock, 
        sstp_api_msg_st *msg)
{
    char *buff = NULL;
    int cnt    = (SSTP_API_ATTR_MAX+1);
    int ret    = SSTP_OKAY;
    int retval = SSTP_FAIL;
    sstp_api_attr_st *list[SSTP_API_ATTR_MAX+1];
    
    /* Allocate buffer on stack */
    buff = alloca(msg->msg_len);
    if (!buff)
    {
        log_err("Could not read the length");
        goto done;
    }

    /* Read the remainder of the payload */
    ret = read(sock, buff, msg->msg_len);
    if (ret < 0 || ret != msg->msg_len)
    {
        log_err("Could not read the payload");
        goto done;
    }

    /* Parse the Attribute */
    ret = sstp_api_attr_parse(buff, msg->msg_len, list, cnt);
    if (ret != 0)
    {
        log_err("Could not parse attributes");
        goto done;
    }

    /* Check for SEND KEY */
    ctx->skey = list[SSTP_API_ATTR_MPPE_SEND];
    if (!ctx->skey)
    {
        log_err("Missing attribute MPPE SEND");
        goto done;
    }

    /* Check for RECV KEY */
    ctx->rkey = list[SSTP_API_ATTR_MPPE_RECV];
    if (!ctx->rkey)
    {
        log_err("Missing attribute MPPE RECV");
        goto done;
    }

    /* Success */
    ctx->event_cb(ctx->arg, SSTP_OKAY);

    /* Prepare the ACK */
    sstp_api_msg_new((unsigned char*)msg, SSTP_API_MSG_ACK);

    /* ACK the message */
    ret = send(sock, msg, sizeof(*msg), 0);
    if (ret < 0 || ret != sizeof(*msg))
    {
        log_warn("Could not reply message back to pppd");
    }

    /* Success */
    retval = SSTP_OKAY;

done:

    return retval;
}


static int sstp_event_addr(sstp_event_st *ctx, int sock, 
        sstp_api_msg_st *msg)
{
    unsigned char buff[255];
    int ret    = SSTP_OKAY;
    int retval = SSTP_FAIL;
    sstp_client_st *client = (sstp_client_st*) ctx->arg;
    
    /* Prepare the ACK */
    msg = sstp_api_msg_new(buff, SSTP_API_MSG_ACK);

    /* Append the gateway address */
    sstp_api_attr_add(msg, SSTP_API_ATTR_GATEWAY, 
            strlen(client->host.name),
            client->host.name);
           
    /* Append the IP Address */
    sstp_api_attr_add(msg, SSTP_API_ATTR_ADDR,
            client->host.alen,
            &client->host.addr);

    /* ACK the message */
    ret = send(sock, msg, msg->msg_len + sizeof(*msg), 0);
    if (ret <= 0 || ret != (msg->msg_len + sizeof(*msg)))
    {
        log_warn("Could not reply message back to pppd");
    }

    /* Success */
    retval = SSTP_OKAY;
    return retval;
}


static void sstp_event_accept(int fd, short event, sstp_event_st *ctx)
{
    int sock = (-1);
    int len  = ( 0);

    sstp_api_msg_st msg;
    sstp_api_msg_t  type;

    /* Accept the incoming socket */
    sock = accept(fd, NULL, NULL);
    if (sock < 0)
    {
        log_err("Unable to accept connection on socket, %s (%d)", 
            strerror(errno), errno);
        goto done;
    }

    log_info("Received callback from sstp-plugin");

    /* Read the header from the plugin */
    len = read(sock, &msg, sizeof(msg));
    if (len < 0 || len != sizeof(msg))
    {
        log_err("Could not read the header of the message: %d, %s (%d)", 
            len, strerror(errno), errno);
        goto done;
    }

    /* Validate message header */
    if (sstp_api_msg_type(&msg, &type))
    {
        log_err("Invalid Message");
        goto done;
    }

    switch (type)
    {
    /* Receive the MPPE keys */
    case SSTP_API_MSG_AUTH:
        sstp_event_auth(ctx, sock, &msg);
        break;

    /* Return the IP Address and Gateway */
    case SSTP_API_MSG_ADDR:
        sstp_event_addr(ctx, sock, &msg);
        break;

    default:
        break;
    }

    
done:
 
    /* Close the client socket */
    if (sock > 0)
    {
        close(sock);
    }

    event_add(ctx->ev_event, NULL);
}


status_t sstp_event_mppe_result(sstp_event_st *ctx, uint8_t **skey, 
        size_t *slen, uint8_t **rkey, size_t *rlen)
{
    sstp_api_attr_st *key = NULL;
    
    key = ctx->skey;
    *skey = key->attr_data;
    *slen = key->attr_len;

    key = ctx->rkey;
    *rkey = key->attr_data;
    *rlen = key->attr_len;

    return SSTP_OKAY;
}


const char *sstp_event_sockname(sstp_event_st *ctx)
{
    return ctx->sockname;
}


status_t sstp_event_create(sstp_event_st **ctx, sstp_option_st *opts, 
    event_base_st *base, sstp_event_fn event_cb, void *arg)
{
    struct sockaddr_un addr;
    status_t status = SSTP_FAIL;
    int sock = (-1);
    int ret  = (-1);
    int alen = (sizeof(addr));
    sstp_event_st *obj = NULL;

    /* Create the Unix domain socket */
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
    {
        log_err("Could not create unix socket, %s (%d)", 
            strerror(errno), errno);
        goto done;
    }

    /* Initialize the address */
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/sstpc-%s", SSTP_RUNTIME_DIR, 
        (opts->ipparam) ? opts->ipparam : SSTP_SOCK_NAME);

    /* Make sure we remove any existing file first */
    unlink(addr.sun_path);

    /* Bind the socket */
    ret = bind(sock, (struct sockaddr*) &addr, alen);
    if (ret < 0)
    {
        log_err("Could not bind ipc socket, %s (%d)", 
            strerror(errno), errno);
        goto done;
    }

    /* Listen to the socket */
    ret = listen(sock, 1);
    if (ret < 0)
    {
        log_err("Could not listen on ipc socket");
        goto done;
    }

    /* If we are running as root, let's change the permissions */
    if (getuid() == 0)
    {
        ret = chown(addr.sun_path, sstp_get_uid(opts->priv_user), 
                sstp_get_gid(opts->priv_group));
        if (ret != 0)
        {
            log_warn("Could not change ownership of socket");
        }
    }

    /* Create new context */
    obj = calloc(1, sizeof(sstp_event_st));
    if (obj == NULL)
    {
        log_err("Could not allocate memory for event context");
        goto done;
    }

    /* Notify in the logs */
    log_info("Waiting for sstp-plugin to connect on: %s",
            addr.sun_path);

    /* Configure our local context */
    obj->sock     = sock;
    obj->arg      = arg;
    obj->event_cb = event_cb;
    strncpy(obj->sockname, addr.sun_path, sizeof(obj->sockname));

    /* Configure a event object for accept socket */
    obj->ev_event = event_new(base, sock, EV_READ, (event_fn) 
            sstp_event_accept, obj);

    /* Add a read event for accept */
    event_add(obj->ev_event, NULL);

    /* Save the return value */
    *ctx = obj;
    status = SSTP_OKAY;

done:
    
    return (status);
}


void sstp_event_free(sstp_event_st *ctx)
{
    /* Remove the IPC socket */
    if (ctx->sockname[0])
    {
        const char *name = ctx->sockname;
        
        /* In case we are running in a sandbox */
        if (getuid() != 0)
        {
            name = rindex(ctx->sockname, '/')+1;
        }

        /* Unlink the file */
        if (0 > unlink(name))
        {
            log_warn("Could not remove socket, %s (%d)",
                strerror(errno), errno);
        }
    }
    
    /* Close the socket */
    if (ctx->sock > 0)
    {
        close(ctx->sock);
        ctx->sock = -1;
    }

    /* Remove event listener */
    event_del(ctx->ev_event);
    event_free(ctx->ev_event);

    /* Free the context */
    free(ctx);
}


