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
//  Filename:       array.h
//  Author:         Paul J.Y. Lahaie
//  Creation Date:  14/06/04
//
//  Description:
//      TODO
//
*****************************************************************************/
#ifndef _ARRAY_H_
#define _ARRAY_H_

typedef struct _array { int len;
                        void **data; } ARRAY;

ARRAY *array_new(void);
int array_len( ARRAY *array );
int array_free( ARRAY *array );
int array_add( ARRAY *array, void *data );
void *array_get( ARRAY *array, int idx );

#endif
