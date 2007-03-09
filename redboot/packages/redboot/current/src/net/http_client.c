//==========================================================================
//
//      net/http_client.c
//
//      Stand-alone HTTP support for RedBoot
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002, 2003 Gary Thomas
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2002-05-22
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

// HTTP client support

#include <redboot.h>     // have_net
#include <net/net.h>
#include <net/http.h>

// So we remember which ports have been used
static int get_port = 7800;

static struct _stream{
    bool open;
    int  avail, actual_len, pos, filelen;
    char data[4096];
    char *bufp;
    tcp_socket_t sock;
} http_stream;

static __inline__ int
min(int a, int b)
{
    if (a < b) 
        return a;
    else
        return b;
}

int
http_stream_open(connection_info_t *info, int *err)
{
    int res;
    struct _stream *s = &http_stream;

    if (!info->server->sin_port)
        info->server->sin_port = 80;  // HTTP port
    if ((res = __tcp_open(&s->sock, info->server, get_port++, 5000, err)) < 0) {
        *err = HTTP_OPEN;
        return -1;
    }
    diag_sprintf(s->data, "GET %s HTTP/1.0\r\n\r\n", info->filename);
    __tcp_write_block(&s->sock, s->data, strlen(s->data));    
    s->avail = 0;
    s->open = true;
    s->pos = 0;
    return 0;
}

void
http_stream_close(int *err)
{    
    struct _stream *s = &http_stream;

    if (s->open) {
        __tcp_abort(&s->sock,1);
        s->open = false;    
    }
}

int
http_stream_read(char *buf,
                 int len,
                 int *err)
{    
    struct _stream *s = &http_stream;
    int total = 0;
    int cnt, code;

    if (!s->open) {
        return -1;  // Shouldn't happen, but...
    }
    while (len) {
        while (s->avail == 0) {
            // Need to wait for some data to arrive
            __tcp_poll();
            if (s->sock.state != _ESTABLISHED) {
                if (s->sock.state == _CLOSE_WAIT) {
                    // This connection is breaking
                    if (s->sock.data_bytes == 0 && s->sock.rxcnt == 0) {
                        __tcp_close(&s->sock);
                        return total;
                    }  
                } else if (s->sock.state == _CLOSED) {
                    	// The connection is gone
                    	s->open = false;
                    	return -1;
                } else {	
                    *err = HTTP_IO;
                    return -1;
		}
            }
            s->actual_len = __tcp_read(&s->sock, s->data, sizeof(s->data));
            if (s->actual_len > 0) {
                s->bufp = s->data;
                s->avail = s->actual_len;
                if (s->pos == 0) {
                    // First data - need to scan HTTP response header
                    if (strncmp(s->bufp, "HTTP/", 5) == 0) {
                        // Should look like "HTTP/1.1 200 OK"
                        s->bufp += 5;
                        s->avail -= 5;
                        // Find first space
                        while ((s->avail > 0) && (*s->bufp != ' ')) {
                            s->bufp++;  
                            s->avail--;
                        }
                        // Now the integer response
                        code = 0;
                        while ((s->avail > 0) && (*s->bufp == ' ')) {
                            s->bufp++;  
                            s->avail--;
                        }
                        while ((s->avail > 0) && isdigit(*s->bufp)) {
                            code = (code * 10) + (*s->bufp - '0');
                            s->bufp++;  
                            s->avail--;
                        }
                        // Make sure it says OK
                        while ((s->avail > 0) && (*s->bufp == ' ')) {
                            s->bufp++;  
                            s->avail--;
                        }
                        if (strncmp(s->bufp, "OK", 2)) {
                            switch (code) {
                            case 400:
                                *err = HTTP_BADREQ;
                                break;
                            case 404:
                                *err = HTTP_NOFILE;
                                break;
                            default:
                                *err = HTTP_BADHDR;
                                break;
                            }
                            return -1;
                        }
                        // Find \r\n\r\n - end of HTTP preamble
                        while (s->avail >= 4) {
                            // This could be done faster, but not simpler
                            if (strncmp(s->bufp, "\r\n\r\n", 4) == 0) {
                                s->bufp += 4;
                                s->avail -= 4;
#if 0 // DEBUG - show header
                                *(s->bufp-2) = '\0';
                                diag_printf(s->data);
#endif
                                break;
                            }
                            s->avail--;
                            s->bufp++;
                        }
                        s->pos++;
                    } else {
                        // Unrecognized response
                        *err = HTTP_BADHDR;
                        return -1;
                    }
                }
            } else if (s->actual_len < 0) {
                *err = HTTP_IO;
                return -1;
            }
        }
        cnt = min(len, s->avail);
        memcpy(buf, s->bufp, cnt);
        s->avail -= cnt;
        s->bufp += cnt;
        buf += cnt;
        total += cnt;
        len -= cnt;
    }
    return total;
}

char *
http_error(int err)
{
    char *errmsg = "Unknown error";

    switch (err) {
    case HTTP_NOERR:
        return "";
    case HTTP_BADHDR:
        return "Unrecognized HTTP response";
    case HTTP_BADREQ:
        return "Bad HTTP request (check file name)";
    case HTTP_NOFILE:
        return "No such file";
    case HTTP_OPEN:
        return "Can't connect to host";
    case HTTP_IO:
        return "I/O error";
    }
    return errmsg;
}

//
// RedBoot interface
//
GETC_IO_FUNCS(http_io, http_stream_open, http_stream_close,
              0, http_stream_read, http_error);
RedBoot_load(http, http_io, true, true, 0);
