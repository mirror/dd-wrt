/*!
 * @brief This process the HTTP handshake for SSTP
 *
 * @file sstp-http.c
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
#include "sstp-private.h"

/*!
 * @par SSTP HTTP Handshake
 *  The client will send the SSTP_DUPLEX_POST instead of the regular HTTP GET/POST. This
 *  request will ultimately start the processing of the SSTP session and the server will
 *  reply us back with a HTTP 200 if successful.
 *
 * @code
 *   SSTP_DUPLEX_POST /sra_{BA195980-CD49-458b-9E23-C84EE0ADCD75}/ HTTP/1.1
 *   Content-Length: 18446744073709551615
 *   Server: n3zz-dc1.sstp-test.net
 *   SSTPCORRELATIONID: {<UUID>}
 * @endcode
 *
 * @code
 *  HTTP/1.1 200
 *  Content-Length: 18446744073709551615
 *  Server: Microsoft-HTTPAPI/2.0
 *  Date: Sat, 19 Feb 2011 02:13:44 GMT
 * @endcode
 *
 * @par TODO:
 *  We need to improve the receive logic in this file to make sure we drain 
 *  the entire HTTP request and nothing more.
 */

/*!
 * @brief A HTTP context to perform handshake with server
 */
struct sstp_http
{
    /*! Server we are connecting to */
    const char *server;

    /*! The caller supplied argument */
    void *uarg;

    /*! The handshake complete callbac */
    sstp_http_done_fn done_cb;

    /*! The buffer used for send / recv */
    sstp_buff_st *buf;

    /*! The server / client mode */
    int mode;

    /*! The username */
    char *user;

    /*! The password */
    char *pass;

    /*! The uuid if set */
    char uuid[64];
};


/*!
 * @brief Called when receive of the HTTP PROXY data is complete
 */
static void sstp_recv_proxy_complete(sstp_stream_st *client, 
    sstp_buff_st *buf, void *ctx, status_t status);


#if 0
/*!
 * @brief Sent by the server in response to a client hello message
 */
static void sstp_http_send_response(void)
{
    return;
}


/*!
 * @brief This receives the response to the send hello
 */
static void sstp_http_recv_response(void)
{
    return;
}


/*! 
 * @brief Server's receive hello from the client
 */
static void sstp_http_recv_hello(sstp_http_st *http)
{
    return;
}
#endif

status_t sstp_http_create(sstp_http_st **http, const char *server, 
    sstp_http_done_fn done_cb, void *uarg, int mode)
{
    int ret = 0;

    /* Allocate the HTTP context */
    *http = calloc(1, sizeof(sstp_http_st));
    if (!*http)
    {
        return SSTP_FAIL;
    }

    /* Set the HTTP context */
    (*http)->uarg    = uarg;
    (*http)->done_cb = done_cb;
    (*http)->server  = server;
    (*http)->mode    = mode;

    /* Create the buffer */
    ret = sstp_buff_create(&(*http)->buf, 8192);
    if (SSTP_OKAY != ret)
    {
        free(*http);
        return SSTP_FAIL;
    }

    /* Set a random UUID */
    sstp_get_guid((*http)->uuid, sizeof((*http)->uuid));

    return SSTP_OKAY;
}


void sstp_http_free(sstp_http_st *http)
{
    /* Check the input argument */
    if (!http)
    {
        return;
    }

    /* Deallocate the buffer */
    if (http->buf)
    {
        sstp_buff_destroy(http->buf);
    }

    if (http->pass)
    {
        free(http->pass);
    }

    if (http->user)
    {
        free(http->user);
    }

    /* Free the HTTP request */
    free(http);   
}


/*!
 * @brief Receive the server hello, check status code
 */
static void sstp_recv_hello_complete(sstp_stream_st *client, 
    sstp_buff_st *buf, void *ctx, status_t status)
{
    sstp_http_st *http = (sstp_http_st*) ctx;
    http_header_st array[7];
    http_header_st *entry;
    int attr  = 7;
    int code  = 0;
    int ret   = 0;

    /* Handle timeout, error, etc */
    if (SSTP_OKAY != status)
    {
        goto done;
    }

    /* Adjust our response until we are successful */
    status = SSTP_FAIL;

    /* Get the HTTP headers */
    ret = sstp_http_get(buf, &code, &attr, array);
    if (SSTP_OKAY != ret)
    {
        log_err("Could not parse the HTTP headers");
        goto done;
    }

    /* HTTP status code must be 200 */
    if (code != 200)
    {
        log_err("Error: Expected HTTP code 200");
        goto done;
    }

    /* Get the Content-Length if specified */
    entry = sstp_http_get_header("Content-Length", attr, array);
    if (entry != NULL)
    {
        unsigned long long length = strtoull(entry->value, NULL, 10);
        if (length != -1ULL)
        {
            log_err("Error: Received invalid content length");
            goto done;
        }
    }

    status = SSTP_OKAY;

done:
 
    http->done_cb(http->uarg, status);
}


/*! 
 * @brief Called once a send operation is complete.
 */
static void sstp_http_send_complete(sstp_stream_st *stream, sstp_buff_st *buf, 
        sstp_http_st *http, status_t result)
{
    /* Check the result */
    if (SSTP_OKAY != result)
    {
        http->done_cb(http->uarg, SSTP_FAIL);
    }

    /* Setup a receiver for HTTP messages */
    sstp_stream_setrecv(stream, sstp_stream_recv, http->buf,
            (sstp_complete_fn) sstp_recv_hello_complete, http, 60);
}


static void sstp_http_send_proxy_complete(sstp_stream_st *stream, sstp_buff_st *buf, 
        sstp_http_st *http, status_t result)
{
    /* Check the result */
    if (SSTP_OKAY != result)
    {
        http->done_cb(http->uarg, SSTP_FAIL);
    }

    /* Setup a receiver for HTTP messages */
    sstp_stream_setrecv(stream, sstp_stream_recv_plain, http->buf,
            (sstp_complete_fn) sstp_recv_proxy_complete, http, 60);
}

/*! 
 * @brief Send the client hello to the server
 *
 * @par Note: 
 *   Response is expected in 60 seconds
 */
static status_t sstp_http_send_hello(sstp_http_st *http, 
        sstp_stream_st *stream)
{
    int ret = 0;

    sstp_buff_reset(http->buf);

    /* Add the HTTP header */
    ret = sstp_buff_print(http->buf, "SSTP_DUPLEX_POST %s HTTP/1.1\r\n",
            SSTP_HTTP_DFLT_PATH);
    if (SSTP_OKAY != ret)
    {
        return ret;
    }

    /* Add the Host attribute */
    ret = sstp_buff_print(http->buf, "Host: %s\r\n", http->server);
    if (SSTP_OKAY != ret)
    {
        return ret;
    }

    /* Add the Content-Length attribute */
    ret = sstp_buff_print(http->buf, "Content-Length: %llu\r\n", -1ULL);
    if (SSTP_OKAY != ret)
    {
        return ret;
    }

    /* Add the UUID attribute */
    ret = sstp_buff_print(http->buf, "SSTPCORRELATIONID: %s\r\n\r\n", 
            http->uuid, strlen(http->uuid));
    if (SSTP_OKAY != ret)
    {
        return ret;
    }

    /* Send the buffer */
    return sstp_stream_send(stream, http->buf, (sstp_complete_fn)
            sstp_http_send_complete, http, 10);
}


status_t sstp_http_handshake(sstp_http_st *http, sstp_stream_st *stream)
{
    int ret = SSTP_FAIL;

    /* Send the HELLO to the server */
    switch (http->mode)
    {
    case SSTP_MODE_CLIENT:

        /* Send the sstp hello to the server */
        ret = sstp_http_send_hello(http, stream);
        if (SSTP_OKAY != ret)
        {
            break;
        }

        /* Setup a receiver for HTTP messages */
        sstp_stream_setrecv(stream, sstp_stream_recv, http->buf,
                (sstp_complete_fn) sstp_recv_hello_complete, http, 60);
        break;

    case SSTP_MODE_SERVER:
    default:
        ret = SSTP_NOTIMPL;
        break;
    }

    return ret;
}


/*!
 * @brief Perform Basic authentication for now. Support digest in the 
 *  future.
 */
static const char *sstp_proxy_basicauth(const char *user, 
        const char *pass, char *buf, int size)
{
    EVP_ENCODE_CTX ctx;
    int tot = 0;
    int len = 0;
    unsigned char out[255];

    EVP_EncodeInit  (&ctx);
    EVP_EncodeUpdate(&ctx, out + tot, &len, 
		(unsigned char*) user, strlen(user));
    tot += len;
    EVP_EncodeUpdate(&ctx, out + tot, &len, 
		(unsigned char*) ":", 1);
    tot += len;
    EVP_EncodeUpdate(&ctx, out + tot, &len, 
		(unsigned char*) pass, strlen(pass));
    tot += len;
    EVP_EncodeFinal (&ctx, out + tot, &len);
    tot += len;

    snprintf(buf, size, "Basic %s", out);
    return (buf);
}


/*!
 * @brief Update with the credentials
 */
void sstp_http_setcreds(sstp_http_st *http, const char *user,
        const char *pass)
{
    http->user = strdup(user);
    http->pass = strdup(pass);
}


/*!
 * @brief Set the UUID of the connection
 */
void sstp_http_setuuid(sstp_http_st *http, const char *uuid)
{
    strncpy(http->uuid, uuid, sizeof(http->uuid));
}


/*!
 * @brief Called when receive is complete from the proxy.
 */
static void sstp_recv_proxy_complete(sstp_stream_st *client, 
    sstp_buff_st *buf, void *ctx, status_t status)
{
    sstp_http_st *http = (sstp_http_st*) ctx;
    http_header_st array[15];
    http_header_st *entry;
    int attr  = 15;
    int code  = 0;
    int ret   = 0;

    /* TODO: Handle timeout, error, etc */
    if (SSTP_OKAY != status)
    {
        goto done;
    }

    /* Get the HTTP headers */
    ret = sstp_http_get(buf, &code, &attr, array);
    if (SSTP_OKAY != ret)
    {
        log_err("Could not parse the HTTP headers");
        goto done;
    }
    
    switch (code)
    {
    case 200:
        status = SSTP_OKAY;
        break;

    case 407:
        
        /* Get the Content-Length if specified */
        entry = sstp_http_get_header("Proxy-Authenticate", attr, array);
        if (strncasecmp(entry->value, "Basic", 5))
        {
            log_err("Received unsupported authentication: %s", 
                    entry->value);
            status = SSTP_FAIL;
            break;
        }

        /* Let the layer above provide user/pass */
        status = SSTP_AUTHENTICATE;
        break;

    default:
        status = SSTP_FAIL;
        break;
    }

done:

    http->done_cb(http->uarg, status);
}


/*!
 * @brief Called when we want to initiate a connection to the proxy
 */
status_t sstp_http_proxy(sstp_http_st *http, sstp_stream_st *stream)
{
    status_t ret = SSTP_FAIL;

    /* Reset buffer before use */
    sstp_buff_reset(http->buf);

    /* Format the buffer */
    ret = sstp_buff_print(http->buf, SSTP_HTTP_PROXY_CONNECT_FMT,
            http->server, PACKAGE);
    if (SSTP_OKAY != ret)
    {
        goto done;
    }

    /* Username and password */
    if (http->user && http->pass)
    {
        char auth[255];

        /* Get the basic auth */
        sstp_proxy_basicauth(http->user, http->pass, auth, sizeof(auth));

        /* Format the buffer */
        ret = sstp_buff_print(http->buf, SSTP_HTTP_PROXY_AUTH_FMT, auth);
        if (SSTP_OKAY != ret)
        {
            goto done;
        }
    }

    /* Add the termination to the HTTP header */
    ret = sstp_buff_print(http->buf, "\r\n");
    if (SSTP_OKAY != ret)
    {
        goto done;
    }

    /* Send the HTTP header */
    ret = sstp_stream_send_plain(stream, http->buf, (sstp_complete_fn)
                sstp_http_send_proxy_complete, http, 10);
    if (SSTP_OKAY != ret)
    {
        goto done;
    }

    /* Configure the receiver */
    sstp_stream_setrecv(stream, sstp_stream_recv_plain, http->buf,
            (sstp_complete_fn) sstp_recv_proxy_complete, http, 60);
done:
    
    return ret;
}

