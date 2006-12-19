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
//  Filename:       ipp.h
//  Author:         Paul J.Y. Lahaie
//  Creation Date:  14/06/04
//
//  Description:
//      TODO
//
*****************************************************************************/
#ifndef _IPP_H_
#define _IPP_H_

#include "array.h"

/* Operation codes */

#define IPP_PRINT_JOB        0x0002
#define IPP_PRINT_URI        0x0003
#define IPP_VALIDATE_JOB     0x0004
#define IPP_CREATE_JOB       0x0005
#define IPP_SEND_DOC         0x0006
#define IPP_SEND_URI         0x0007
#define IPP_CANCEL_JOB       0x0008
#define IPP_GET_JOB_ATTR     0x0009
#define IPP_GET_JOBS         0x000A
#define IPP_GET_PRINTER_ATTR 0x000B

/* Delimiter tags */

#define IPP_TAG_OPERATIONS   0x01
#define IPP_TAG_JOBS         0x02
#define IPP_TAG_END          0x03
#define IPP_TAG_PRINTER      0x04
#define IPP_TAG_UNSUPPORTED  0x05

/* Value tags */

/* Integer tags */

#define IPP_TAG_INTEGERS     0x20
#define IPP_TAG_INT          0x21
#define IPP_TAG_BOOL         0x22
#define IPP_TAG_ENUM         0x23
// #define IPP_TAG_RESERVED     0x24-0x2F

/* Octet-string tags */

#define IPP_TAG_OCTET_STRING 0x30
#define IPP_TAG_DATETIME     0x31
#define IPP_TAG_RESOLUTION   0x32
#define IPP_TAG_RANGE_INT    0x33
// #define IPP_TAG_RESERVED     0x34 for collection (in future)
#define IPP_TAG_TEXT_W_LANG  0x35
#define IPP_TAG_NAME_W_LANG  0x36

/* Character-string tags */

#define IPP_TAG_CHAR_STRING  0x40
#define IPP_TAG_TEXT_WO_LANG 0x41
#define IPP_TAG_NAME_WO_LANG 0x42
// #define IPP_TAG_RESERVED     0x43
#define IPP_TAG_KEYWORD      0x44
#define IPP_TAG_URI          0x45
#define IPP_TAG_URI_SCHEME   0x46
#define IPP_TAG_CHARSET      0x47
#define IPP_TAG_NATURAL_LANG 0x48
#define IPP_TAG_MIME_MEDIA   0x49

// #define IPP_TAG_RESERVED     0x4A - 0x4F for future character string types

// IPP response codes -- Success

#define IPP_SUCCESS          0x0000
#define IPP_SUCCESS_IGN      0x0001
#define IPP_SUCCESS_CONFLICT 0x0002

// IPP response codes - Client errors

#define IPP_ERR_BAD_REQUEST           0x0400
#define IPP_ERR_FORBIDDEN             0x0401
#define IPP_ERR_NOT_AUTHENTICATED     0x0402
#define IPP_ERR_NOT_AUTHORIZED        0x0403
#define IPP_ERR_NOT_POSSIBLE          0x0404
#define IPP_ERR_TIMEOUT               0x0405
#define IPP_ERR_NOT_FOUND             0x0406
#define IPP_ERR_GONE                  0x0407
#define IPP_ERR_REQUEST_TOO_LARGE     0x0408
#define IPP_ERR_VALUE_TOO_LONG        0x0409
#define IPP_ERR_FORMAT_NOT_SUPPORTED  0x040A
#define IPP_ERR_ATTR_NOT_SUPPORTED    0x040B
#define IPP_ERR_VAL_NOT_SUPPORTED     0x040B // Same as IPP_ERR_ATTR_NOT_SUPPORTED
#define IPP_ERR_URISCHEME_UNSUPPORTED 0x040C
#define IPP_ERR_CHARSET_NOT_SUPPORTED 0x040D
#define IPP_ERR_CONFLICTING_ATTR      0x040E

// IPP response codes - Server errors

#define IPP_ERR_INTERNAL_ERROR        0x0500
#define IPP_ERR_OPERATION_UNSUPPORTED 0x0501
#define IPP_ERR_SERVICE_UNAVAILABLE   0x0502
#define IPP_ERR_VERSION_UNSUPPORTED   0x0503
#define IPP_ERR_DEVICE_ERROR          0x0504
#define IPP_ERR_TEMP_ERROR            0x0505
#define IPP_ERR_NOT_ACCEPTING_JOBS    0x0506
#define IPP_ERR_BUSY                  0x0507
#define IPP_ERR_JOB_CANCELLED         0x0508

// Printer state enum
#define IPP_PRINTER_IDLE              0x03
#define IPP_PRINTER_PROCESSING        0x04
#define IPP_PRINTER_STOPPED           0x05

struct _ipp_attr_value { char type;
                         short len;
                         void *value; };

typedef struct _ipp_attr { char *name;
                           ARRAY *values; } IPP_ATTR;

struct _ipp_attr_seq { char seq;
                       ARRAY *attr; };
                       

typedef struct _ipp { int data_left;
                      short version;
                      union { short int operation;
                              short int response; };
                      int request_id;
                      ARRAY *seqs; } IPP;

IPP *ipp_new(void);

#endif
