/*****************************************************************************
//
//  Copyright (c) 2004  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
******************************************************************************
//
//  Filename:       conn.c
//  Author:         Paul J.Y. Lahaie
//  Creation Date:  14/06/04
//
//  Description:
//      TODO
//
*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <sys/poll.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "conn.h"
#include "printers.h"
#include "http.h"
#include "array.h"

#define min(a,b) (a<b?a:b)

extern ARRAY *printers;

struct _connection_list *conn_list = NULL;

int http_header( Connection *conn );

void do_ipp( Connection *conn, int length );

Connection *add_conn( int fd )
{
  struct _connection_list *c_list;
  struct _connection_list *cl;

  if( ( c_list = calloc( 1, sizeof( struct _connection_list ) ) ) == NULL ) {
    // Out of memory -- bad
    return NULL;
  }

  c_list->conn.fd = fd;
  c_list->conn.state = CONN_BEGIN;
  c_list->conn.http = http_new();
  c_list->conn.used = 0;
  c_list->conn.buf_ptr = 0;
  c_list->next = NULL;

  if( conn_list == NULL ) {
    conn_list = c_list;
  } else {
    for( cl = conn_list; cl->next != NULL; cl = cl->next ) ;
    cl->next = c_list;
  }
  return (Connection *)c_list;
}

int remove_conn( Connection *conn )
{
  struct _connection_list *prev, *cl;

  if( conn_list == (struct _connection_list *)conn ) {
    prev = conn_list;
    conn_list = conn_list->next;
    if( prev->conn.ipp )
      ipp_free( prev->conn.ipp );
    if( prev->conn.http )
      http_free( prev->conn.http );
    free( prev );
  } else {

    for( prev = NULL, cl = conn_list; cl != NULL; prev = cl, cl = cl->next ) {
      if( cl == (struct _connection_list *)conn ) {
        if( prev ) {
          prev->next = cl->next;
        } else {
          conn_list = cl->next;
        }
        close( cl->conn.fd );
        if( cl->conn.ipp ) {
          ipp_free( cl->conn.ipp );
        }
        if( cl->conn.http )
          http_free( cl->conn.http );
        free( cl );
        break;
      }
    }
  }
  return 0;
}

void process_conn( struct _connection *conn )
{
  char *ct;
  int req_404;
  char *str_length;
  int length;
  int i;
  Printer *printer;
  Job *job;

  switch( conn->state ) {
    case CONN_BEGIN:
      // Find out what kind of request
      if( http_header( conn ) ) {
        printf("Reading HTTP header failed.\n" );
        //remove_conn( conn );
      }
      // Now we check to make it's a proper URL
      req_404 = 0;
      if( strncasecmp( "/printers/", conn->http->uri, 9 ) == 0 ) {
        // extract printer name
        if( get_printer( conn->http->uri+10 ) == NULL ) {
          req_404 = 1;
        }
      } else {
        req_404 = 1;
      }
      if( req_404 ) {
        conn->used = sprintf( conn->buffer, "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n" );
        conn->buf_ptr = 0;
        conn->state = CONN_OUTPUT;
        return;
      }
      if( ( ct = http_get_var( conn->http, "Content-Type" ) ) == NULL ) {
      	/* 2004/11/4 : windows 2000 use "Content-type" when sending request */
      	if( ( ct = http_get_var( conn->http, "Content-type" ) ) == NULL ) {
        // No content type, assume http?
        ct = "";
        }
      }
      
      if( strcmp( ct, "application/ipp" ) == 0 ) {
        if( ( str_length = http_get_var( conn->http, "Content-Length" ) ) == NULL ) {
          length = -1;
        } else {
          length = atoi( str_length );
        }
        do_ipp( conn, length );
        // conn->state = CONN_QUIET;
      } else {
        close( conn->fd );
      }
      break;
    case CONN_PRINTING_READ:
      for( i = 0; i < array_len( printers ); i++ ) {
        printer = array_get( printers, i );
        for( job = printer_list_job( printer, NULL ); job != NULL; job = printer_list_job( printer, job ) ) {
          if( job->fd == conn->fd ) {
            // This is the printer
            printer->used = read( conn->fd, printer->buffer, min(BUF_SIZE,conn->ipp->data_left) );
            printer->buf_ptr = 0;
            printer->state = PRINTER_PRINTING_WRITE;
            conn->state = CONN_PRINTING_WAIT;
            conn->ipp->data_left -= printer->used;
          }
        }
      }
      break;
    default:
      break;
  }
}

Connection *list_conn( void *iter )
{
  struct _connection_list *cl;

  if( iter == NULL ) {
    return (Connection *)conn_list;
  } else
    if( conn_list )
      for( cl = conn_list; cl != NULL; cl = cl->next )
        if( iter == cl )
          return (Connection *)cl->next;
  return NULL;
}

int wait_conn( Connection *conn, int msec ) {
  struct pollfd wait_poll; // Only need one

  wait_poll.fd = conn->fd;
  wait_poll.events = POLLIN;

  return poll( &wait_poll, 1, msec );
}

int http_gets( Connection *conn, char *line, int lsize ) {
  int done = 0, total_read = 0;
  // Read up to \r\n, take the \r\n and return string
  while( ! done ) {
    if( conn->buf_ptr == conn->used ) {
      if( ! wait_conn( conn, 1000 ) ) { // Wait for data
        return total_read;
      } else {
        conn->buf_ptr = 0;
        if( ( conn->used = read( conn->fd, conn->buffer, BUF_SIZE ) ) == 0 ) {
          conn->buf_ptr = 0;
          return total_read;
        }
      }
    }
    for(; conn->buf_ptr < conn->used; conn->buf_ptr++ ) {
      if ( total_read == lsize-1 ) {
        line[total_read] = '\0';
        done = 1;
      } else {
        switch( conn->buffer[conn->buf_ptr] ) {
          case '\r':
            if ( conn->buf_ptr+1 < conn->used && conn->buffer[conn->buf_ptr+1] == '\n' ) {
              line[total_read] = '\0';
              done = 1;
              conn->buf_ptr+=2;
            } else {
              line[total_read++] = '\r';
            }
            break;
          case '\n':
            line[total_read] = '\0';
            done = 1;
            conn->buf_ptr++;
            break;
          default:
            line[total_read++] = conn->buffer[conn->buf_ptr];
        }
      }
      if( done )
        break;
    }
  }
  return total_read;
}

int http_header( Connection *conn ) {
  char *request, *uri, *version, line[1024];

  if( http_gets( conn, line, 1024 ) ) {
    request = strtok( line, " " );
    uri = strtok( NULL, " " );
    version = strtok( NULL, " " );
    if( request == NULL || uri == NULL || version == NULL ) {
      // Bad request
      return 1;
    } else {
      if( strcmp( request, "GET" ) == 0 ) {
        conn->http->request = HTTP_GET;
      } else if ( strcmp( request, "PUT" ) == 0 ) {
        conn->http->request = HTTP_PUT;
      } else if ( strcmp( request, "POST" ) == 0 ) {
        conn->http->request = HTTP_POST;
      } else {
        return 1; // Bad request
      }
      strcpy( conn->http->uri, uri );
      if( strcmp( version, "HTTP/1.0" ) == 0 ) {
        conn->http->version = HTTP_10;
      } else if ( strcmp( version, "HTTP/1.1" ) == 0 ) {
        conn->http->version = HTTP_11;
      }
    }
  }
  while( http_gets( conn, line, 1024 ) ) {
    int i;

    for( i = 0; line[i] != '\0'; i++ ) {
      if( line[i] == ':' ) {
        line[i++] = '\0';
        break;
      }
    }
    while( isspace( line[i] ) )
      i++;
    http_add_var( conn->http, line, line+i );
  }
  return 0;
}

int getc_conn( Connection *conn, char *c ) {
  if( conn->used - conn->buf_ptr ) {
    *c = conn->buffer[conn->buf_ptr++];
  } else {
    if( wait_conn( conn, 1000 ) ) {
      if( ( conn->used = read( conn->fd, conn->buffer, BUF_SIZE ) ) > 0 ) {
        conn->buf_ptr = 1;
        *c = conn->buffer[0];
      } else {
        return 0;
      }
    } else {
      return 0;
    }
  }
  return 1;
}

int getshort_conn( Connection *conn, short int *s )
{
  char c1, c2;

  if( getc_conn( conn, &c1 ) ) {
    if( getc_conn( conn, &c2 ) ) {
      *s = c1 << 8 | c2;
      return 1;
    }
  }
  return 0;
}

int getint_conn( Connection *conn, int *i )
{
  short int s1, s2;

  if( getshort_conn( conn, &s1 ) ) {
    if( getshort_conn( conn, &s2 ) ) {
      *i = s1 << 16 | s2;
      return 1;
    }
  }
  return 0;
}

int read_conn( Connection *conn, char *buffer, int i )
{
  if( conn->used - conn->buf_ptr < i ) {
    if( wait_conn( conn, 1000 ) ) {
      memmove( conn->buffer, conn->buffer+conn->buf_ptr, conn->used - conn->buf_ptr );
      conn->used -= conn->buf_ptr;
      conn->buf_ptr = 0;
      conn->used += read( conn->fd, conn->buffer+conn->used, BUF_SIZE-conn->used );
      if( conn->used < i )
        return 0;
    } else {
      return 0;
    }
  }
  memcpy( buffer, conn->buffer + conn->buf_ptr, i );
  conn->buf_ptr += i;
  return 1;
}

Connection *get_conn( int fd ) {
  Connection *conn;

  for( conn = list_conn( NULL ); conn != NULL; conn = list_conn( conn ) )
    if( conn->fd == fd )
      return conn;
  return NULL;
}
