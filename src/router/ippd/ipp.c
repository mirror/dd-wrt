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
//  Filename:       ipp.c
//  Author:         Paul J.Y. Lahaie
//  Creation Date:  14/06/04
//
//  Description:
//      
//
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "conn.h"
#include "ipp.h"
#include "printers.h"
#ifdef DEBUG_MEM
#include <dmalloc.h>
#endif

/***********************************
 *
 * Operation responses
 *
 ***********************************/

static IPP *ipp_print_job( Connection *conn, int len );
static IPP *ipp_printer_attrs( IPP *ipp );
static IPP *ipp_job_attrs( IPP *ipp );
static IPP *ipp_get_jobs( IPP *ipp );

/***********************************
 *
 * Stream manipulation functions
 *
 ***********************************/

int process_ipp_tags( Connection *conn );

/***********************************
 *
 * Write IPP buffer functions
 *
 ***********************************/

int  ipp_write( IPP *ipp, char *buffer, int len );
void ipp_write_int( char *buffer, int i, int *buf_ptr );
void ipp_write_short( char *buffer, short int s, int *buf_ptr );
void ipp_write_char( char *buffer, char c, int *buf_ptr );
void ipp_write_tag( char *buffer, IPP_ATTR *attr, int *buf_ptr );

/***********************************
 *
 * IPP object manipulation
 *
 ***********************************/

int ipp_add_tag( IPP *ipp, char tag_type, char *name,  void *value, short val_len, int set );
int ipp_copy_tag( IPP *dst, IPP *src, char seq, char *name );
struct _ipp_attr_seq *ipp_add_seq( IPP *ipp, char seq );

/***********************************
 *
 * Debugging functions
 *
 ***********************************/
void ipp_pretty_print( IPP *ipp );


/***********************************
 *
 * Code
 *
 ***********************************/

IPP *ipp_new(void)
{
  IPP *new_ipp;

  if( ( new_ipp = calloc( 1, sizeof( IPP ) ) ) == NULL ) {
    return NULL;
  } else {
    if( ( new_ipp->seqs = array_new() ) == NULL ) {
      free( new_ipp );
      return NULL;
    } else {
      return new_ipp;
    }
  }
}

void ipp_free( IPP *ipp )
{
  struct _ipp_attr_seq *s;
  IPP_ATTR *a;
  struct _ipp_attr_value *v;
  int i, e, g;

  for( i = 0; i < array_len( ipp->seqs ); i++ ) {
    s = array_get( ipp->seqs, i );
    for( e = 0; e < array_len( s->attr ); e++ ) {
      a = array_get( s->attr, e );
      for( g = 0; g < array_len( a->values ); g++ ) {
        v = array_get( a->values, g );
        free( v->value );
      }
      array_free( a->values );
      free( a->name );
    }
    array_free( s->attr );
  }
  array_free( ipp->seqs );
  free( ipp );
}

void do_ipp( Connection *conn, int length ) {
  IPP *reply;
  int len=0, pos;

  conn->ipp = ipp_new();
  conn->ipp->data_left = length;

  if( getshort_conn( conn, &conn->ipp->version ) ) {
    if( getshort_conn( conn, &conn->ipp->operation ) ) {
      if( getint_conn( conn, &conn->ipp->request_id ) ) {
        conn->ipp->data_left -= 8;
        process_ipp_tags( conn );
      }
    }
  }
  reply = NULL;
  switch( conn->ipp->operation ) {
    case IPP_GET_PRINTER_ATTR:
      reply = ipp_printer_attrs( conn->ipp );
      break;
    case IPP_GET_JOBS:
      reply = ipp_get_jobs( conn->ipp );
      break;
    case IPP_PRINT_JOB: // Special function
      printf( "Got print request.\n" );
      ipp_print_job( conn, length );
      break;
    case IPP_GET_JOB_ATTR:
      reply = ipp_job_attrs( conn->ipp );
      break;
    default:
      ipp_pretty_print( conn->ipp );
      reply = ipp_new();
      reply->response = IPP_ERR_NOT_FOUND; // No such uri
      reply->version = 256; //IPP 1.0
      reply->request_id = conn->ipp->request_id;
      ipp_add_tag( reply, IPP_TAG_OPERATIONS, NULL, NULL, 0, 0 );
      ipp_copy_tag( reply, conn->ipp, IPP_TAG_OPERATIONS, "attributes-charset" );
      ipp_copy_tag( reply, conn->ipp, IPP_TAG_OPERATIONS, "attributes-natural-language" );
  }
  if( reply ) {
#ifdef IPP_DEBUG
    printf( "Connection descriptor is: %d\n", conn->fd );
#endif
    len = ipp_write( reply, NULL, len );
    pos = sprintf( conn->buffer, "HTTP/1.1 200 OK\r\nContent-Type: application/ipp\r\nContent-Length: %d\r\n\r\n", len );
    ipp_write( reply, conn->buffer + pos, len );
    conn->used = len + pos;
    conn->buf_ptr = 0;
    conn->state = CONN_OUTPUT;
    ipp_free( reply );
  }
}

int process_ipp_tag( Connection *conn, char tag_type ) {
  static char prev_name[1025];
  char name[1025], value[1025];
  int set;
  short int len;

  memset( name, 0, sizeof( name ) );
  memset( value, 0, sizeof( value ) );

  if( getshort_conn( conn, &len ) )
    conn->ipp->data_left-=2;
    if( len < 1024 ) {
      if( len == 0 ) {
        strcpy( name, prev_name );
        set = 1;
      } else {
        if( !read_conn( conn, name, len ) ) {
          return 0;
        }
        conn->ipp->data_left -= len;
        set = 0;
      }
      if( getshort_conn( conn, &len ) )
        conn->ipp->data_left -= 2;
        if( len < 1024 )
          if( read_conn( conn, value, len ) ) {
            conn->ipp->data_left -= len;
            ipp_add_tag( conn->ipp, tag_type, name, value, len, set );
            strcpy( prev_name, name );
            return 1;
          }
    }
  return 0;
}

int process_ipp_tags( Connection *conn ) {
  char current_group = '\0';
  char tag_type;

  while( 1 ) {
    if( !getc_conn( conn, &tag_type ) )
      break;
    conn->ipp->data_left--;
    if( tag_type == IPP_TAG_END ) {
      break;
    }
    if( current_group == '\0' && (tag_type&0xF0)) {
      return 0;
    }
    if( !(tag_type&0xF0) ) {
      ipp_add_tag( conn->ipp, tag_type, NULL, NULL, 0, 0 );
      current_group = tag_type;
    } else {
      if( !process_ipp_tag( conn, tag_type ) ) {
        return 0;
      }
    }
  }
  return 1;;
}

IPP_ATTR *ipp_get_tag( IPP *ipp, char seq, char *name, int nr ) {
  int i, e;
  struct _ipp_attr_seq *s;
  IPP_ATTR *a;

  for( i = 0; i < array_len( ipp->seqs); i++ ) {
    s = array_get( ipp->seqs, i );
    if( s->seq == seq && !--nr) {
      for( e = 0; e < array_len( s->attr ); e++ ) {
        a = array_get( s->attr, e );
        if( strcmp( a->name, name ) == 0 ) {
          return a;
        }
      }
    }
  }
  return NULL;
}

struct _ipp_attr_seq *ipp_add_seq( IPP *ipp, char seq ) {
  struct _ipp_attr_seq *n_seq;

  if( ( n_seq = calloc( 1, sizeof(struct _ipp_attr_seq ) ) ) == NULL ) {
    return NULL;
  }

  n_seq->seq = seq;

  n_seq->attr = array_new();

  if( !array_add( ipp->seqs, n_seq ) ) { // Couldn't add to array
    free( n_seq );
    return NULL;
  } else {
    return n_seq;
  }
}

int ipp_add_tag( IPP *ipp, char tag_type, char *name,  void *value, short val_len, int set )
{
  struct _ipp_attr_seq *s;
  struct _ipp_attr_value *attr_value;
  IPP_ATTR *attr;
  ARRAY *vals;
  int i;

  if( tag_type&0xF0 ) { // Value
    s = array_get( ipp->seqs, array_len( ipp->seqs ) - 1 );

    for( i = 0; i < array_len( s->attr ); i++ ) {
      attr = array_get( s->attr, i );
      if( strcmp( attr->name, name ) == 0 ) {
        if( set ) {
          vals = attr->values;
        } else {
          array_free( attr->values );
          vals = attr->values = array_new();
        }
        break;
      }
    }

    if( i == array_len( s->attr ) ) {
      attr = calloc( 1, sizeof( IPP_ATTR ) );
      attr->name = strdup( name );
      vals = attr->values = array_new();
      array_add( s->attr, attr );
    }

    attr_value = malloc( sizeof( struct _ipp_attr_value ) );

    attr_value->type = tag_type;

    if( tag_type & (IPP_TAG_INTEGERS|IPP_TAG_OCTET_STRING) ) {  // Binary
      attr_value->value = malloc( val_len );
      memcpy( attr_value->value, value, val_len );
    } else if ( tag_type & IPP_TAG_CHAR_STRING ) { // CHARACTER-STRING
      attr_value->value = malloc( val_len + 1 ); // Terminating NUL
      memcpy( attr_value->value, value, val_len );
      ((char *)attr_value->value)[val_len] = '\0';
    }
    attr_value->len = val_len;
    array_add( vals, attr_value );
  } else {
    ipp_add_seq( ipp, tag_type );
  }
  return 1;
}

int ipp_copy_tag( IPP *dst, IPP *src, char seq, char *name )
{
  struct _ipp_attr_value *av;
  int i;
  IPP_ATTR *attr = ipp_get_tag( src, seq, name, 1 );

  if( attr ) {
    for( i = 0; i < array_len( attr->values ); i++ ) {
      av = array_get( attr->values, i );
      ipp_add_tag( dst, av->type, name, av->value, av->len, 1 );
    }
    return 1;
  } else {
    return 0;
  }

}

int ipp_write( IPP *ipp, char *buffer, int len )
{
  int i = 0, e, x;
  struct _ipp_attr_seq *s;
  IPP_ATTR *a;

  ipp_write_short( buffer, ipp->version, &i );
  ipp_write_short( buffer, ipp->response, &i );
  ipp_write_int( buffer, ipp->request_id, &i );
  for( e = 0; e < array_len( ipp->seqs ); e++ ) {
    s = array_get( ipp->seqs, e );
    ipp_write_char( buffer, s->seq, &i );
    for( x = 0; x < array_len( s->attr ); x++ ) {
      a = array_get( s->attr, x );
      ipp_write_tag( buffer, a, &i );
    }
  }
  ipp_write_char( buffer, IPP_TAG_END, &i );
  return i;
}

void ipp_write_short( char *buffer, short int s, int *buf_ptr ) {
  ipp_write_char( buffer, s>>8, buf_ptr );
  ipp_write_char( buffer, s&255, buf_ptr );
}

void ipp_write_int( char *buffer, int i, int *buf_ptr ) {
  ipp_write_char( buffer, i>>24, buf_ptr );
  ipp_write_char( buffer, i>>16, buf_ptr );
  ipp_write_char( buffer, i>>8, buf_ptr );
  ipp_write_char( buffer, i&255, buf_ptr );
}

void ipp_write_tag( char *buffer, IPP_ATTR *attr, int *buf_ptr )
{
  int i;
  struct _ipp_attr_value *av;

  for( i = 0; i < array_len( attr->values ); i++ ) {
    av = array_get( attr->values, i );
    ipp_write_char( buffer, av->type, buf_ptr );
    if( i == 0 ) { // For first attribute, write name
      ipp_write_short( buffer, strlen( attr->name ), buf_ptr );
      if( buffer )
        memcpy( buffer+*buf_ptr, attr->name, strlen( attr->name ) );
      (*buf_ptr) += strlen( attr->name );
    } else {
      ipp_write_short( buffer, 0, buf_ptr ); // Name is NUL for set
    }
    ipp_write_short( buffer, av->len, buf_ptr );
    if( buffer )
      memcpy( buffer+*buf_ptr, av->value, av->len );
    (*buf_ptr) += av->len;
  }
}

void ipp_write_char( char *buffer, char c, int *buf_ptr )
{
  if( buffer )
    buffer[*buf_ptr] = c;
  (*buf_ptr)++;
}

void ipp_print_val( char type, void *value, int len ) {
  if( type & 0x20 ) {
    // Integer type
    switch( len ) {
      case 1: printf( "%d\n", *(char *)value ); break;
      case 2: printf( "%d\n", ntohs(*(short *)value )); break;
      case 4: printf( "%d\n", ntohl(*(int *)value )); break;
      default:
        printf( "unknown int format\n" );
    }
  } else if ( type & 0x30 ) {
    switch( type ) {
      case IPP_TAG_OCTET_STRING:
        printf( "%s\n", (char *)value );
      default:
        printf( "Tag type not supported.\n" );
    }
  } else {
    printf( "%s\n", (char *)value );
  }
}

void ipp_pretty_print( IPP *ipp ) {
  int i, e, g;
  struct _ipp_attr_seq *s;
  IPP_ATTR *a;
  struct _ipp_attr_value *av;
  char val[1024];

  printf( "IPP version: %d.%d\n", ipp->version>>8, ipp->version&255 );
  printf( "    Op/resp: %d\n", ipp->operation );
  printf( "    Request: %d\n", ipp->request_id );
  if( array_len( ipp->seqs ) ) {
    printf( "    Attributes:\n" );
    for( i = 0; i < array_len( ipp->seqs ); i++ ) {
      s = array_get( ipp->seqs, i );
      switch( s->seq ) {
        case IPP_TAG_OPERATIONS:
          printf( "Operations sequence\n" );
          break;
        case IPP_TAG_JOBS:
          printf( "Jobs sequence\n" );
          break;
        case IPP_TAG_PRINTER:
          printf( "Printer sequence\n" );
          break;
        case IPP_TAG_UNSUPPORTED:
          printf( "Unsupported sequence\n" );
          break;
        default:
          printf( "Unknown sequence\n" );
          break;
      }
      for( e = 0; e < array_len( s->attr ); e++ ) {
        a = array_get( s->attr, e );
        printf( "%s:", a->name );
        if( array_len( a->values ) > 1 ) {
          printf("\n");
          for( g = 0; g < array_len( a->values ); g++ ) {
            av = array_get( a->values, g );
            memcpy( val, av->value, av->len );
            val[av->len] = '\0'; 
            printf( "\t(%x) ", av->type );
            ipp_print_val( av->type, val, av->len );
          }
        } else {
          av = array_get( a->values, 0 );
          memcpy( val, av->value, av->len );
          val[av->len] = '\0';
          printf( " (%x) ", av->type );
          ipp_print_val( av->type, val, av->len );
        }
      }
    }
  }
}

Printer *ipp_get_printer( IPP *ipp )
{
  char *printer_uri;
  IPP_ATTR *attr;
  struct _ipp_attr_value *av;

  if( ( attr = ipp_get_tag( ipp, IPP_TAG_OPERATIONS, "printer-uri", 1 ) ) == NULL ) {
    return NULL; // Bad URI
  }
  av = array_get( attr->values, 0 );
  printer_uri = av->value;
  // Parse printer-uri
  while( *printer_uri++ != ':' ) ; // Skip protocol
  while( *printer_uri++ == '/' ) ; // Skip //
  while( *printer_uri++ != '/' ) ; // Skip hostname
  if( strncasecmp( "printers/", printer_uri, 8 ) == 0 )
    return get_printer( printer_uri+9 );
  else
    return NULL;
}

static IPP *ipp_printer_attrs( IPP *ipp )
{
  IPP *response;
  char *printer_uri;
  IPP_ATTR *attr;
  struct _ipp_attr_value *av;
  Printer *printer;
  Job *job;
  short val;
  int int_val;

  response = ipp_new();

  response->version = 256; // IPP 1.0
  response->request_id = ipp->request_id;
  ipp_add_tag( response, IPP_TAG_OPERATIONS, NULL, NULL, 0, 0 );
  ipp_copy_tag( response, ipp, IPP_TAG_OPERATIONS, "attributes-charset" );
  ipp_copy_tag( response, ipp, IPP_TAG_OPERATIONS, "attributes-natural-language" );
  if( ( printer = ipp_get_printer( ipp ) ) == NULL ) {
    response->response = 0x0406; // Bad URI
  } else {
    response->response = 0x0000;
    attr = ipp_get_tag( ipp, IPP_TAG_OPERATIONS, "printer-uri", 1 );
    av = array_get( attr->values, 0 );
    printer_uri = av->value;
    ipp_add_tag( response, IPP_TAG_PRINTER, NULL, NULL, 0, 0 );
    ipp_add_tag( response, IPP_TAG_URI, "printer-uri-supported", printer_uri, strlen( printer_uri ), 0 );
    ipp_add_tag( response, IPP_TAG_TEXT_WO_LANG, "printer-name", printer->name, strlen( printer->name ), 0 );
    ipp_add_tag( response, IPP_TAG_TEXT_WO_LANG, "printer-make-and-model", printer->make, strlen( printer->make ), 0 );
    ipp_add_tag( response, IPP_TAG_URI, "device-uri", printer->device, strlen( printer->device ), 0 );

    // List of operations we support

    int_val = htonl(IPP_PRINT_JOB);
    ipp_add_tag( response, IPP_TAG_ENUM, "operations-supported", &int_val, 4, 0 );
    int_val = htonl(IPP_GET_JOB_ATTR);
    ipp_add_tag( response, IPP_TAG_ENUM, "operations-supported", &int_val, 4, 1 );
    int_val = htonl(IPP_GET_JOBS);
    ipp_add_tag( response, IPP_TAG_ENUM, "operations-supported", &int_val, 4, 1 );
    int_val = htonl(IPP_GET_PRINTER_ATTR);
    ipp_add_tag( response, IPP_TAG_ENUM, "operations-supported", &int_val, 4, 1 );

    // Printer state
    if( printer->state == PRINTER_OPEN ) {
      int_val = htonl( IPP_PRINTER_IDLE );
    } else if( printer->state == PRINTER_CLOSED ) {
      // First we try to open the printer
#ifdef IPP_DEBUG
      fprintf( stderr, "Trying to open printer.\n" );
#endif
      if( ( printer->fd = open( printer->device, O_RDWR ) ) >= 0 ) {
        if( printer->jobs ) { // We have jobs
          Connection *conn;

          if( ( conn = get_conn( printer->jobs->job.fd ) ) ) {
            if( conn->buf_ptr < conn->used ) {
              memcpy( printer->buffer, conn->buffer + conn->buf_ptr, conn->used - conn->buf_ptr );
              printer->used = conn->used - conn->buf_ptr;
              printer->buf_ptr = 0;
              printer->state = PRINTER_PRINTING_WRITE;
              conn->state = CONN_PRINTING_WAIT;
              conn->ipp->data_left -= conn->used - conn->buf_ptr;
            } else {
              if( conn->ipp->data_left ) {
                printer->state = PRINTER_PRINTING_WAIT;
                conn->state = CONN_PRINTING_READ;
              }
            }
          }
          int_val = htonl( IPP_PRINTER_PROCESSING );
        } else {
          printer->state = PRINTER_OPEN;
          val = htonl( IPP_PRINTER_IDLE );
        }
      } else {
        int_val = htonl( IPP_PRINTER_STOPPED );
        ipp_add_tag( response, IPP_TAG_KEYWORD, "printer-state-reasons", "shutdown", sizeof( "shutdown" ), 0 );
      }
      /* 2004/11/5 : put back into closed state so that kernel can remove lp0 when printer unplugged */
      if (printer->state == PRINTER_OPEN) {
        close(printer->fd);
        printer->fd = 0;
        printer->state = PRINTER_CLOSED;
      }
    } else {
      int_val = htonl( IPP_PRINTER_PROCESSING );
    }

    ipp_add_tag( response, IPP_TAG_ENUM, "printer-state", &int_val, 4, 0 );

    // We only do UTF-8.  It's string.h friendly.

    ipp_add_tag( response, IPP_TAG_CHARSET, "charset-configured", "utf8", sizeof( "utf8" ), 0 );
    ipp_add_tag( response, IPP_TAG_CHARSET, "charset-supported", "utf8", sizeof( "utf8" ), 0 );

    //  Windows only sends application/octet-stream -- we only support that

    ipp_add_tag( response, IPP_TAG_MIME_MEDIA, "document-format-default", "application/octet-stream", strlen( "application/octet-stream" ), 0 );
    ipp_add_tag( response, IPP_TAG_MIME_MEDIA, "document-format-supported", "application/octet-stream", strlen( "application/octet-stream" ), 0 );

    //  We don't override any of the values inside the printer steam
    ipp_add_tag( response, IPP_TAG_KEYWORD, "pdl-override-supported", "not-attempted", strlen( "not-attempted" ), 0 );

    // Count how many jobs on this printer

    for( int_val = 0, job = printer_list_job(printer, NULL); job != NULL; job = printer_list_job( printer, job ) ) ;
    ipp_add_tag( response, IPP_TAG_INT, "queued-job-count", &int_val, 4, 0 );

    if( ( int_val = (time(NULL) - printer->create_time) ) == 0 ) { // Amount of seconds printer has been up
      int_val = 1; // Must start at 1;
    }
    int_val = htonl( int_val );
    ipp_add_tag( response, IPP_TAG_INT, "printer-up-time", &int_val, 4, 0 );
  }

  return response;
}

static IPP *ipp_job_attrs( IPP *ipp )
{
  IPP *response;
  Job *job;
  Printer *printer;
  IPP_ATTR *temp_attr;
  struct _ipp_attr_value *av;
  int job_id;

  response = ipp_new();
                                                                                
  response->version = 256; // IPP 1.0
  response->request_id = ipp->request_id;
  ipp_add_tag( response, IPP_TAG_OPERATIONS, NULL, NULL, 0, 0 );
  ipp_copy_tag( response, ipp, IPP_TAG_OPERATIONS, "attributes-charset" );
  ipp_copy_tag( response, ipp, IPP_TAG_OPERATIONS, "attributes-natural-language" );

  // Now we try to find the job...

  // Check to see if we got printer-uri and job-id

  if( ( printer = ipp_get_printer( ipp ) ) == NULL ) {
    if( ( temp_attr = ipp_get_tag( ipp, IPP_TAG_OPERATIONS, "job-uri", 1 ) ) == NULL ) {
      response->response = 0x0406;
      return response;
    } else {
    }
  } else {
      if( ( temp_attr = ipp_get_tag( ipp, IPP_TAG_OPERATIONS, "job-id", 1 ) ) == NULL ) {
        response->response = 0x0406;
        return response;
      } else {
        av = array_get( temp_attr->values, 0 ); // Not a set
        job_id = ntohl(*(int *)av->value);
        for( job = printer_list_job( printer, NULL ); job != NULL; job++ ) {
          if( job->id == job_id ) {
            ipp_add_tag( response, IPP_TAG_JOBS, NULL, NULL, 0, 0 );
            // Job description
            response->response = 0x0000;
            return response;
          }
        }
      }
    }
  response->response = 0x406;
  return response;
} 

static IPP *ipp_get_jobs( IPP *ipp )
{
  IPP *response;
  Printer *printer;
  Job *job;

  response = ipp_new(); // Allocate reply

  response->version = 256; //IPP 1.0
  response->request_id = ipp->request_id; // Reply for this IPP req

  ipp_add_tag( response, IPP_TAG_OPERATIONS, NULL, NULL, 0, 0 );
  ipp_copy_tag( response, ipp, IPP_TAG_OPERATIONS, "attributes-charset" );
  ipp_copy_tag( response, ipp, IPP_TAG_OPERATIONS, "attributes-natural-language" );
  if( ( printer = ipp_get_printer( ipp ) ) == NULL ) { // Get printer from printer-uri
    response->response = 0x0406; // No such uri
  } else {
    response->response = 0x0000;
    ipp_add_tag( response, IPP_TAG_TEXT_WO_LANG, "status-message", "successful-ok", strlen( "successful-ok" ), 0 );
    for( job = printer_list_job( printer, NULL ); job != NULL; job = printer_list_job( printer, job ) ) {
        ipp_add_tag( response, IPP_TAG_JOBS, NULL, NULL, 0, 0 );
    }
  }
  return response;
}

IPP *ipp_print_job( Connection *conn, int len )
{
  IPP *response, *ipp = conn->ipp;
  Printer *printer;

  if( ( printer = ipp_get_printer( ipp ) ) == NULL ) { // Get printer from printer-uri
    response = ipp_new();
    response->response = 0x0406; // No such uri
    response->version = 256; //IPP 1.0
    response->request_id = ipp->request_id;
    ipp_add_tag( response, IPP_TAG_OPERATIONS, NULL, NULL, 0, 0 );
    ipp_copy_tag( response, ipp, IPP_TAG_OPERATIONS, "attributes-charset" );
    ipp_copy_tag( response, ipp, IPP_TAG_OPERATIONS, "attributes-natural-language" );
    return response;
  } else {
    Job *job;

    job = printer_add_job( printer, conn->fd, "blah", len );
    if( job == (Job *)printer->jobs ) {
      if( conn->buf_ptr < conn->used ) {
      	/* 2004/11/8 : open device now */
      	if (printer->state == PRINTER_CLOSED) {
      		if( ( printer->fd = open( printer->device, O_RDWR ) ) >= 0 )
      			printer->state = PRINTER_CLOSED;
      		else
      			printf("open printer failed\n");
      	}
        memcpy( printer->buffer, conn->buffer + conn->buf_ptr, conn->used - conn->buf_ptr );
        printer->used = conn->used - conn->buf_ptr;
        printer->buf_ptr = 0;
        printer->state = PRINTER_PRINTING_WRITE;
        conn->state = CONN_PRINTING_WAIT;
        conn->ipp->data_left -= conn->used - conn->buf_ptr;
      } else {
        if( conn->ipp->data_left ) {
          printer->state = PRINTER_PRINTING_WAIT;
          conn->state = CONN_PRINTING_READ;;
        } else {
          int len, pos, val;

          // Print job is done...  Give response
          response = ipp_new();
          response->response = 0x0000; // Success
          response->request_id = conn->ipp->request_id;
          response->version = 256;
          ipp_add_tag( response, IPP_TAG_OPERATIONS, NULL, NULL, 0, 0 );
          ipp_copy_tag( response, conn->ipp, IPP_TAG_OPERATIONS, "attributes-charset" );
          ipp_copy_tag( response, conn->ipp, IPP_TAG_OPERATIONS, "attributes-natural-language" );
          /* 2004/11/8 : Added to make XP printer utility report success status*/
          ipp_add_tag( response, IPP_TAG_TEXT_WO_LANG, "status-message", "successful-ok", strlen( "successful-ok" ), 0 );
    	  ipp_add_tag( response, IPP_TAG_JOBS, NULL, NULL, 0, 0 );
    	  val = job->id;
    	  ipp_add_tag( response, IPP_TAG_INTEGERS, "job-id", &val, 4, 0 );
    	  val = 9;
    	  ipp_add_tag( response, IPP_TAG_ENUM, "job-state", &val, 4, 0 ); /* 9 = COMPLETED */
          // Done
          len = ipp_write( response, NULL, len );
          pos = sprintf( conn->buffer, "HTTP/1.1 200 OK\r\nContent-Type: application/ipp\r\nContent-Length: %d\r\n\r\n", len );
          ipp_write( response, conn->buffer + pos, len );
          conn->used = len + pos;
          conn->buf_ptr = 0;
          conn->state = CONN_OUTPUT;
          /* 2004/11/8: added to free response */
          ipp_free( response );
        }
      }
    } else {
      conn->state = CONN_PRINTING_QUEUED;
    }
    return NULL;
  }
}
