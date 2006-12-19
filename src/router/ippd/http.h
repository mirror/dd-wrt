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
//  Filename:       http.h
//  Author:         Paul J.Y. Lahaie
//  Creation Date:  14/06/04
//
//  Description:
//      TODO
//
*****************************************************************************/
#ifndef _HTTP_H_
#define _HTTP_H_

typedef struct _http_var { char name[128];
                           char value[128]; } HTTP_VAR;

struct _http_var_list { HTTP_VAR var;
                        struct _http_var_list *next; };

typedef enum _HTTP_REQUEST { HTTP_GET,
                             HTTP_PUT,
                             HTTP_POST } HTTP_REQUEST;

typedef enum _HTTP_VERSION { HTTP_10,
                             HTTP_11 } HTTP_VERSION;

typedef struct _http { HTTP_REQUEST request;
                       HTTP_VERSION version;
                       char uri[128];
                       struct _http_var_list *variable; } HTTP;

#define http_new() (HTTP *)calloc( 1, sizeof( HTTP ) )
  
int http_add_var( HTTP *http, char *name, char *value );
char *http_get_var( HTTP *http, char *name );
int http_free( HTTP *http );
#endif
