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
//  Filename:       http.c
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

#include "http.h"
#ifdef DEBUG_MEM
#include <dmalloc.h>
#endif

int http_add_var( HTTP *http, char *name, char *value ) {
  struct _http_var_list *http_var, *new_var;

  if( !(new_var = calloc( 1, sizeof( struct _http_var_list ) ) ) ) {
    return 1;
  } else {
    strcpy( new_var->var.name, name );
    strcpy( new_var->var.value, value );
    if( !http->variable )  {
      http->variable = new_var;
    } else {
      for( http_var = http->variable; http_var->next != NULL; http_var = http_var->next )
        ;
      http_var->next = new_var;
    }
  }
      
  return 0;
}

char *http_get_var( HTTP *http, char *name ) {
  struct _http_var_list *http_var;

  for( http_var = http->variable; http_var != NULL; http_var = http_var->next ) {
    if( strcmp( http_var->var.name, name ) == 0 ) {
      return http_var->var.value;
    }
  }
  return NULL;
}

int http_free( HTTP *http ) {
  struct _http_var_list *http_var, *prev;
  if( http->variable ) {
    for( prev = NULL, http_var = http->variable; http_var != NULL; prev = http_var, http_var = http_var->next ) {
      if( prev )
        free(prev);
    }
    if( prev )
      free( prev );
  }
  free( http );
  return 0;
}
