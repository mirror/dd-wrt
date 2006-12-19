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
//  Filename:       testconn.c
//  Author:         Paul J.Y. Lahaie
//  Creation Date:  14/06/04
//
//  Description:
//      TODO
//
*****************************************************************************/
#include <stdio.h>
#include "conn.h"

int main( int argc, char **argv) {
  struct _connection_list *cl;
  Connection *conn;

  add_conn( 1 );
  for( conn = list_conn( NULL ); conn != NULL; conn = list_conn( conn ) ) {
    printf( "%d\n", conn->fd );
  }
  add_conn( 2 );
  for( conn = list_conn( NULL ); conn != NULL; conn = list_conn( conn ) ) {
    cl = (struct _connection_list *)conn;
    printf( "%d\n", conn->fd );
  }
}
