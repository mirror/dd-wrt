/*!
 * @brief Managing the interface with pppd
 *
 * @file sstp-pppd.c
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
#include <string.h>
#include <unistd.h>
#include <paths.h>

#include "sstp-private.h"



/*!
 * @brief Context for the PPPd operations
 */
struct sstp_pppd
{
    /*< Task structure */
    sstp_task_st *task;

    /*< A buffer we can receive data with */
    sstp_buff_st *rx_buf;

    /*< A buffer we can send data with */
    sstp_buff_st *tx_buf;

    /*< The SSL stream context */
    sstp_stream_st *stream;

    /*< Listener for retrieving data from pppd */
    event_st *ev_recv;

    /*< The event base */
    event_base_st *ev_base;

    /*< The chap structure */
    sstp_chap_st chap;

    /*< The notify function */
    sstp_pppd_fn notify;
    
    /*< The argument to pass to this function */
    void *arg;

    /*< The socket to pppd */
    int sock;

    /*< Should we notify client of ip_up */
    int ip_up;
    
    /*< Should we stil continue checking for CHAP structure */
    int auth_done;

    /*< Enable authentication check */
    int auth_check;

    /*< Delete the file */
    int del_file;

    /*< The temporary file name */
    char tmpfile[64];

    /*< The time pppd was terminated */
    unsigned long t_end;

    /*< The time pppd was started */
    unsigned long t_start;

    /*< The number of bytes sent */
    unsigned long long sent_bytes;

    /*< The number of bytes received */
    unsigned long long recv_bytes;
};


static status_t ppp_process_data(sstp_pppd_st *ctx);


/*!
 * @brief Record the number of bytes sent to host from server
 */
static void ppp_record_recv(sstp_pppd_st *ctx, unsigned int len)
{
    ctx->recv_bytes += len;
}


/*!
 * @brief Record the number of bytes sent to server from host
 */
static void ppp_record_sent(sstp_pppd_st *ctx, unsigned int len)
{
    ctx->sent_bytes += len;
}


/*!
 * @brief Throttle receive operation, previous send incomplete.
 *
 * @par Function:
 *  If the send operation was blocked, we'll receive a complete event.
 *  1) Continue sending the remainding data in rx-buffer to server
 *  2) If send() blocks again, we'll re-enter at this point
 *  3) When complete, re-add the sstp_pppd_recv event function here.
 */
static void ppp_send_complete(sstp_stream_st *stream, sstp_buff_st *buf,
    sstp_pppd_st *ctx, status_t status)
{
    if (SSTP_OKAY != status)
    {
        log_err("TODO: Handle shutdown here");
    }

    /* Continue processing input */
    status = ppp_process_data(ctx);
    switch (status)
    {
    case SSTP_INPROG:
        /* Will invoke this function again */
        break;

    case SSTP_OKAY:
        /* Record the number of bytes sent */
        ppp_record_sent(ctx, buf->len);

        /* We had to trottle the recevie operation, re-start */
        event_add(ctx->ev_recv, NULL);
        break;

    case SSTP_FAIL:
    default:
        log_err("TODO: Handle processing failure");
        break;
    }
}


/*!
 * @brief Delete the temporary file at our earliest convenience.
 */
static void sstp_pppd_deltmp(sstp_pppd_st *ctx)
{
    if (!ctx->del_file)
    {
        return;
    }

    if (0 > unlink(ctx->tmpfile))
    {
        log_warn("Could not remove temporary file, %s (%d)",
            strerror(errno), errno);
    }
    
    ctx->del_file = 0;
}


/*! 
 * @brief Retrieve the CHAP context
 */
sstp_chap_st *sstp_pppd_getchap(sstp_pppd_st *ctx)
{
    return (&ctx->chap);
}


void sstp_pppd_session_details(sstp_pppd_st *ctx, sstp_session_st *sess)
{
    unsigned long t_end = ((ctx->t_end == 0) 
        ? time(NULL) 
        : ctx->t_end);

    sess->established = t_end - ctx->t_start;
    sess->rx_bytes = ctx->recv_bytes;
    sess->tx_bytes = ctx->sent_bytes;
}

static void sstp_pppd_ipup(sstp_pppd_st* ctx, sstp_buff_st *tx)
{
    uint8_t *buf = sstp_pkt_data(tx);
    uint16_t proto;

    proto = (ntohs(*(uint16_t *) buf));
    if (proto == SSTP_PPP_IPCP)
    {
        if (ctx->notify)
        {
            ctx->notify(ctx->arg, SSTP_PPP_UP);
        }

        ctx->ip_up = 1;
    }

    return;
}

/*!
 * @brief Intercept any CHAP / PAP authentication with the peer.
 */
static void sstp_pppd_check_auth(sstp_pppd_st* ctx, sstp_buff_st *tx)
{
    uint8_t *buf = sstp_pkt_data(tx);
    int ret = 0;

    /* Check if we have received the MS-CHAPv2(0xC223) credentials */
    switch (ntohs(*(uint16_t*)buf))
    {
    case SSTP_PPP_AUTH_CHAP:

        /* At this point, calculate the MPPE key ourselves */
        if (buf[2] == 0x02)
        {
            memcpy(&ctx->chap, &buf[7], sizeof(sstp_chap_st));

            if (ctx->notify)
            {
                ctx->notify(ctx->arg, SSTP_PPP_AUTH);
            }
 
            ctx->auth_done = 1;
            break;
        }

        if (buf[2] == 0x04)
        {
            log_info("Unsupported");
            break;
        }
        
        sstp_pppd_deltmp(ctx);
        break;

    case SSTP_PPP_AUTH_PAP:
        
        sstp_pppd_deltmp(ctx);

        /* No need to set the MPPE keys, they are all zero */
        //ret = sstp_state_accept(ctx->state);
        if (SSTP_FAIL == ret)
        {
            sstp_die("Negotiation with server failed", -1);
        }
        
    default:

        break;
    }
}


/*!
 * @brief Process any data in the input buffer and forward them to server
 */
static status_t ppp_process_data(sstp_pppd_st *ctx)
{
    sstp_buff_st *rx = ctx->rx_buf;
    sstp_buff_st *tx = ctx->tx_buf;
    status_t ret = SSTP_FAIL;

    /* Initialize TX-buffer */
    sstp_buff_reset(tx);

    /* Iterate over the frames received */
    while (rx->off < rx->len)
    {
        int max = 0;
        int off = 0;

        /* Initialize send buffer */
        ret = sstp_pkt_init(tx, SSTP_MSG_DATA);
        if (SSTP_OKAY != ret)
        {
            return SSTP_FAIL;
        }

        /* Copy a single frame to the tx-buffer */
        max = tx->max - tx->len;
        off = rx->len - rx->off;
        ret = sstp_frame_decode((unsigned char*) rx->data + rx->off, &off,
            (unsigned char*) tx->data + tx->len, &max);
        if (SSTP_OKAY != ret)
        {
            /* We needed to read more ... */
            if (SSTP_OVERFLOW == ret ||
               (rx->len == (rx->off + off))) // TODO: Why!?!
            {
                /* Move current packet to beginning of buffer */
                memmove(rx->data, rx->data + rx->off, rx->len - rx->off);
                rx->len = off;
                rx->off = 0;

                /* Need more data, re-add read event */
                return SSTP_OKAY;
            }

            /* Checksum Error!, drop this segment */
            rx->off += off;
            continue;
        }

        /* Update length */
        tx->len += max;
        rx->off += off;

        /* Update the final length of the packet */
        sstp_pkt_update(tx);

        /* If plugin is not enabled, then we need to check for auth */
        if (ctx->auth_check && !ctx->auth_done) 
        {
            sstp_pppd_check_auth(ctx, tx);
        }

        /* If plugin is not enabled, then we need to send ip-up */
        if (ctx->auth_check && !ctx->ip_up)
        {
            sstp_pppd_ipup(ctx, tx);
        }

        sstp_pkt_trace(tx);

        /* Send a PPP frame */
        ret = sstp_stream_send(ctx->stream, tx, (sstp_complete_fn) 
                ppp_send_complete, ctx, 1);
        if (SSTP_OKAY != ret)
        {
            return SSTP_INPROG;
        }

        /* Record the number of bytes sent */
        ppp_record_sent(ctx, tx->len);
    }
    
    /* Start over in an empty buffer */
    if (rx->off == rx->len)
    {
        sstp_buff_reset(rx);
    }

    return SSTP_OKAY;
}


/*!
 * @brief Receive the data from the pppd daemon, forwarding it to the
 *  sstp-server.
 */
static void sstp_pppd_recv(int fd, short event, sstp_pppd_st *ctx)
{
    sstp_buff_st *rx = ctx->rx_buf;
    status_t ret = SSTP_FAIL;

    /* Receive a chunk */
    rx->len += read(fd, rx->data + rx->len, rx->max - rx->len);
    if (rx->len <= 0)
    {
        if (ctx->notify)
        {
            ctx->notify(ctx->arg, SSTP_PPP_DOWN);
        }
        goto done;
    }

    /* Process the input */
    ret = ppp_process_data(ctx);
    switch (ret)
    {
    case SSTP_INPROG:
        /* Let the ppp_send_complete finish it */
        break;

    case SSTP_OKAY:
        /* Re-add the event to receive more */
        event_add(ctx->ev_recv, NULL);
        break;

    case SSTP_FAIL:
    default:
        log_err("TODO: Handle failure of processing");
        break;
    }

done:

    return;
}


/*!
 * @brief Send data received from the sstp peer back through pppd/pppX
 */
status_t sstp_pppd_send(sstp_pppd_st *ctx, const char *buf, int len)
{
    status_t status = SSTP_FAIL;
    unsigned char *frame = NULL;
    int flen = 0;
    int ret  = 0;

    /* Get the maximum size of the frame */
    flen = (len << 1) + 4;

    /* Allocate some stack space (do not free!) */
    frame = alloca(flen);
    if (!frame)
    {
        goto done;
    }

    /* Perform the HDLC encoding of the frame */
    ret = sstp_frame_encode((const unsigned char*) buf, len, frame, &flen);
    if (SSTP_OKAY != ret)
    {
        log_err("Could not encode frame");
        goto done;
    }

    /* Record the number of bytes received */
    ppp_record_recv(ctx, len);

    /* Write the data back to the pppd */
    ret = write(ctx->sock, frame, flen);
    if (ret != flen)
    {
        log_err("Could not complete write of frame");
        goto done;
    }

    /* Success */
    status = SSTP_OKAY;

done:
    
    return status;
}


status_t sstp_pppd_start(sstp_pppd_st *ctx, sstp_option_st *opts, 
        const char *sockname)
{
    status_t status  = SSTP_FAIL;
    status_t ret     = SSTP_FAIL;

    /* Launch PPPd, unless PPPd launched us */
    if (!(SSTP_OPT_NOLAUNCH & opts->enable))
    {
        const char *args[20];
        int i = 0;
        int j = 0;
 
        /* Create the task */
        ret = sstp_task_new(&ctx->task, SSTP_TASK_USEPTY);
        if (SSTP_OKAY != ret)
        {
            log_err("Could not create a new task for pppd");
            goto done;
        }

        /* Configure the command line */
        args[i++] = "/usr/sbin/pppd";
        args[i++] = sstp_task_ttydev(ctx->task);
        args[i++] = "38400";

        /* Write user to file */
        if (opts->user)
        {
            args[i++] = "user";
            args[i++] = opts->user;
        }

        /* Write the password to file */
        if (opts->password)
        {
            int fd = 0;
            char buff[255];

            sprintf(ctx->tmpfile, "%s/sstp-pppd.XXXXXX", SSTP_TMP_PATH);

            /* Create a file to keep the options */
            fd = mkstemp(ctx->tmpfile);
            if (fd <= 0)
            {
                log_err("Could not create pppd script");
                goto done;
            }

            /* Dump password to temporary file */
            j = snprintf(buff, sizeof(buff), "password \"%s\"\n", opts->password);
            if (write(fd, buff, j) != j)
            {
                log_warn("Could not write password to file");
            }

            /* Close file, and enable check for auth */
            ctx->auth_check = 1;
            close(fd);

            /* Append the file argument to pppd */
            args[i++] = "file";
            args[i++] = ctx->tmpfile;

            /* Remember to delete the file */
            ctx->del_file = 1;
        }

        /* In case we are using plugin */
        if (!(opts->enable & SSTP_OPT_NOPLUGIN))
        {
            args[i++] = "plugin";
            args[i++] = "sstp-pppd-plugin.so";
            args[i++] = "sstp-sock";
            args[i++] = sockname;
        }

        /* Copy all the arguments to pppd */
        for (j = 0; j < opts->pppdargc; j++)
        {
            args[i++] = opts->pppdargv[j];
        }

        /* Terminate the argument vector */
        args[i++] = NULL;

        /* Start the task */
        ret = sstp_task_start(ctx->task, args);
        if (SSTP_OKAY != ret)
        {
            goto done;
        }

        /* Get the socket to listen on */
        ctx->sock    = sstp_task_stdout(ctx->task);
    }
    else
    {
        /* pppd is our parent, we communciate over a pty terminal */
        ctx->sock = STDIN_FILENO;
    }

    /* Need to record approximate time */
    ctx->t_start = time(NULL);

    /* Add the event context */
    ctx->ev_recv = event_new(ctx->ev_base, ctx->sock, EV_READ, (event_fn) 
            sstp_pppd_recv, ctx);

    /* Add the receive event */
    event_add(ctx->ev_recv, NULL);

    /* Success! */
    status = SSTP_OKAY;

done:

    return (status);
}


status_t sstp_pppd_stop(sstp_pppd_st *ctx)
{
    /* Cleanup the task */
    if (ctx->task)
    {
        /* Check if task is still running, then kill it */
        if (sstp_task_alive(ctx->task))
        {
            sstp_task_stop(ctx->task);
        }

        /* Free the task */
        sstp_task_destroy(ctx->task);
        ctx->task  = NULL;
        ctx->t_end = time(NULL);
    }

    return SSTP_FAIL;
}


status_t sstp_pppd_create(sstp_pppd_st **ctx, event_base_st *base,
        sstp_stream_st *stream, sstp_pppd_fn notify_cb, void *arg)
{
    status_t ret    = SSTP_FAIL;
    status_t status = SSTP_FAIL;

    *ctx = calloc(1, sizeof(sstp_pppd_st));
    if (!*ctx)
    {
        goto done;
    }

    ret = sstp_buff_create(&(*ctx)->tx_buf, 16384);
    if (SSTP_OKAY != ret)
    {
        goto done;
    }

    ret = sstp_buff_create(&(*ctx)->rx_buf, 16384);
    if (SSTP_OKAY != ret)
    {
        goto done;
    }

    /* Save a reference to the stream handle */
    (*ctx)->stream = stream;
    (*ctx)->notify = notify_cb;
    (*ctx)->arg    = arg;
    (*ctx)->ev_base= base;

    /* Success */
    status = SSTP_OKAY;

done:
    
    if (SSTP_OKAY != status)
    {
        sstp_pppd_free(*ctx);
    }

    return status;
}


void sstp_pppd_free(sstp_pppd_st *ctx)
{
    if (!ctx)
    {
        return;
    }

    sstp_pppd_deltmp(ctx);

    /* Cleanup the task */
    if (ctx->task)
    {
        /* Check if task is still running, then kill it */
        if (sstp_task_alive(ctx->task))
        {
            sstp_task_stop(ctx->task);
        }

        /* Free resources */
        sstp_task_destroy(ctx->task);
        ctx->task  = NULL;
        ctx->t_end = time(NULL);
    }

    /* Dispose send buffers */
    if (ctx->tx_buf)
    {
        sstp_buff_destroy(ctx->tx_buf);
        ctx->tx_buf = NULL;
    }

    /* Dispose receive buffers */
    if (ctx->rx_buf)
    {
        sstp_buff_destroy(ctx->rx_buf);
        ctx->rx_buf = NULL;
    }

    /* Dispose of receive event */
    if (ctx->ev_recv)
    {
        event_del(ctx->ev_recv);
        event_free(ctx->ev_recv);
    }

    /* Free pppd context */
    free(ctx);
}
