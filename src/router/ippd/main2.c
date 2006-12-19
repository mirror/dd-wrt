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
//  Filename:       main2.c
//  Author:         Paul J.Y. Lahaie
//  Creation Date:  14/06/04
//
//  Description:
//      TODO
//
*****************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "conn.h"
#include "ipp.h"

int main( int argc, char **argv ) {
  Connection conn;

  
  conn.fd = open( argv[1], O_RDONLY );
  conn.used = 0;
  conn.buf_ptr = 0;
  conn.state = CONN_BEGIN;
  conn.ipp = ipp_new();
  conn.http = calloc(1, sizeof( HTTP ) );
  process_conn( &conn );
}
