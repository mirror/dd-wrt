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
//  Filename:       array.c
//  Author:         Paul J.Y. Lahaie
//  Creation Date:  14/06/04
//
//  Description:
//      TODO
//
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "array.h"
#ifdef DEBUG_MEM
#include <dmalloc.h>
#endif

#define ARRAY_ALLOC_SIZE 5

ARRAY *array_new( void ) {
  ARRAY *n;

  if( ( n = calloc( 1, sizeof( ARRAY ) ) ) == NULL ) {
    return NULL;
  }

  if( ( n->data = calloc( ARRAY_ALLOC_SIZE, sizeof( void * ) ) ) == NULL ) {
    free( n );
    return NULL;
  }
  return n;
}

int array_add( ARRAY *array, void *data ) {
  void **n_array;
  int i;

  for( i = 0; i < array->len && array->data[i] != NULL; i++ ) ;

  if( i == array->len ) { // Fell through
    if( ( n_array = realloc( array->data, (array->len+ARRAY_ALLOC_SIZE)*sizeof( void * ) ) ) == NULL ) {
      return 0;
    } else {
      memset( n_array+array->len, 0, ARRAY_ALLOC_SIZE*sizeof( void * ) );
      array->data = n_array;
      array->len += ARRAY_ALLOC_SIZE;
    }
  }
  array->data[i] = data;
  return 1;
}

int array_len( ARRAY *array ) {
  int i;
  for( i = 0; i < array->len && array->data[i] != NULL; i++ ) ;
  return i;
}

int array_free( ARRAY *array ) {
  int i;

  for( i = 0; i < array->len && array->data[i] != NULL; i++ ) {
    free( array->data[i] );
  }
  free( array->data );
  free( array );
  return 0;
}

void *array_get( ARRAY *array, int idx ) {
  if( idx < array->len )
    return array->data[idx];
  else
    return NULL;
}
