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
//  Filename:       conn.h
//  Author:         Paul J.Y. Lahaie
//  Creation Date:  14/06/04
//
//  Description:
//      TODO
//
*****************************************************************************/
#ifndef _CONN_H_
#define _CONN_H_

#include "http.h"
#include "ipp.h"

#define BUF_SIZE 4096

typedef enum { CONN_BEGIN,
               CONN_QUIET,
               CONN_PRINTING_READ,
               CONN_PRINTING_WAIT,
               CONN_PRINTING_QUEUED,
               CONN_OUTPUT } CONN_STATE;

typedef struct _connection {
  int fd;
  CONN_STATE state;
  short int buf_ptr;
  short int used;
  char buffer[BUF_SIZE];
  HTTP *http;
  IPP *ipp;
} Connection;

struct _connection_list {
  struct _connection conn;
  struct _connection_list *next;
};

Connection *add_conn( int fd );
int remove_conn( Connection *conn );
Connection *list_conn( void *iter );
int getc_conn(Connection *conn, char *c );
int getshort_conn(Connection *conn, short int *s );
int getint_conn(Connection *conn, int *i );
int read_conn(Connection *conn, char *buffer, int i );
void process_conn(Connection *conn );
Connection *get_conn( int fd );

#endif
