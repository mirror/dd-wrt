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
//  Filename:       printers.c
//  Author:         Paul J.Y. Lahaie
//  Creation Date:  14/06/04
//
//  Description:
//      TODO
//
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "printers.h"
#include "array.h"
#ifdef DEBUG_MEM
#include <dmalloc.h>
#endif

unsigned int cur_job = 1;

ARRAY *printers;

int init_printers(char *filename) {
  Printer *new_printer;
  char *pr_name = NULL, *make = NULL, *device = NULL;
  char *var, *val;
  char line[1024];
  FILE *fp;

  if( ( fp = fopen( filename, "r" ) ) == NULL ) {
    return 0;
  }

  printers = array_new();

  while( fgets( line, sizeof( line ), fp ) ) {
    switch( line[0] ) {
      case '\n':
      case '#':
      case ';':
      case '[': // New printer
        if( pr_name && make && device ) {
          // New printer def
          new_printer = calloc( 1, sizeof( Printer ) );
          strcpy( new_printer->name, pr_name );
          strcpy( new_printer->make, make );
          strcpy( new_printer->device, device );
          new_printer->create_time = time(NULL);
	  new_printer->state = PRINTER_CLOSED;
          fprintf( stderr, "Adding printer: %s %s %s %d\n", pr_name, make, device, new_printer->fd );
          array_add( printers, new_printer );
        }
        if( pr_name )
          free( pr_name );
        if( make )
          free( make );
        if( device )
          free( device );
        device = make = NULL;
        val = strtok( line+1, "]\n" );
        pr_name = strdup( val );
      default:
        var = strtok( line, "=\n" );
        val = strtok( NULL, "\n" );
        if( var && val ) {
          if( strcmp( var, "make" ) == 0 ) {
            make = strdup( val );
          } else if ( strcmp( var, "device" ) == 0 ) {
            device = strdup( val );
          }
        }
    }
  }
  if( pr_name && make && device ) {
    // New printer def
    new_printer = calloc( 1, sizeof( Printer ) );
    strcpy( new_printer->name, pr_name );
    strcpy( new_printer->make, make );
    strcpy( new_printer->device, device );
    new_printer->create_time = time( NULL );
    if( ( new_printer->fd = open( device, O_RDWR ) ) < 0 ) {
      new_printer->state = PRINTER_CLOSED;
    } else {
      new_printer->state = PRINTER_OPEN;
    }
    array_add( printers, new_printer );
  }
  if( pr_name )
    free( pr_name );
  if( make )
    free( make );
  if( device )
    free( device );
  return 1;
}

Printer *get_printer( char *name ) {
  Printer *p;
  int i;

  for( i = 0; i < array_len( printers ); i++ ) {
    p = array_get( printers, i );
    if( strcmp( p->name, name ) == 0 )
      return p;
  }
  return NULL;
}

Job *printer_add_job( Printer *printer, int fd, char *name, int size )
{
  Jobs *j_l, *jobs = calloc( 1, sizeof( Jobs ) );
                                                                                
  jobs->job.fd = fd;
  jobs->job.name = strdup( name );
  jobs->job.size = size;
  jobs->job.id = cur_job++;
                                                                                
  if( printer->jobs ) {
    for( j_l = printer->jobs; j_l->next != NULL; j_l = j_l->next ) ;
    j_l->next = jobs;
  } else {
    printer->jobs = jobs;
  }
  return (Job *)jobs;
}

Job *printer_list_job( Printer *printer, Job *prev ) {
  Jobs *j_l;
                                                                                
  if( prev == NULL )
    return (Job *)printer->jobs;
  if( printer->jobs ) {
    for( j_l = printer->jobs; j_l != NULL; j_l = j_l->next ) {
      if( prev == (Job *)j_l)
        return (Job *)j_l->next;
    }
    return (Job *)printer->jobs;
  }
}

Job *get_job( char *job ) {
  return NULL;
}
