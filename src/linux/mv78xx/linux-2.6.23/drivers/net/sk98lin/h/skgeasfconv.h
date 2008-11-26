
/******************************************************************************
 *
 * Name:    skgeasfconv.h
 * Project: asf/ipmi
 * Version: $Revision: 1.1.2.1 $
 * Date:    $Date: 2006/08/28 09:06:28 $
 * Purpose: asf/ipmi interface in windows driver
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  (C)Copyright 1998-2002 SysKonnect.
 *  (C)Copyright 2002-2003 Marvell.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  The information in this file is provided "AS IS" without warranty.
 *
 ******************************************************************************/

#ifndef _ASFWMI_H
#define _ASFWMI_H

#ifdef __cplusplus
extern "C" {
#endif

// lengths for string conversion
#define ASF_IPADDRGRPSIZE           (4)                                 // 4 groups in ip address string (111.222.333.444)
#define ASF_MACADDRGRPSIZE          (6)                                 // 6 groups in mac address string (11-22-33-44-55-66)
#define ASF_GUIDGRPSIZE             (16)                                // 16 groups in a GUID string
#define ASF_COMMUNITYSTRLEN         (64)                                // length of community string
#define ASF_IPADDRSTRLEN            (3*ASF_IPADDRGRPSIZE+3)             // length of xxx.xxx.xxx.xxx
#define ASF_MACADDRSTRLEN           (2*ASF_MACADDRGRPSIZE+5)            // length of xx-xx-xx-xx-xx-xx
#define ASF_GUIDSTRLEN              (2*ASF_GUIDGRPSIZE)                 // length of GUID string

// module sizes
#define ASF_MAX_STRINGLEN           (ASF_COMMUNITYSTRLEN+1)             // length of a ascii string (with string end marker 0x00)
#define ASF_MAX_UNICODESTRINGLEN    (ASF_COMMUNITYSTRLEN)               // length of a unicode string (without length information)


// tags in strings
#define ASF_IPSEPARATOR             ('.')                               // separator in ip string
#define ASF_MACSEPARATOR            ('-')                               // separator in mac address


// modes for AsfWmiInternal2External() and AsfWmiExternal2Internal()
#define ASF_MODE_IPADDR             (10)        // input is a IP address (IPv4 format)
#define ASF_MODE_MACADDR            (11)        // input is a MAC address
#define ASF_MODE_COMMUNITY          (12)        // input is a community string
#define ASF_MODE_GUID               (13)        // input is a number
#define ASF_MODE_SYSID              (14)        // input is a number
#define ASF_MODE_MANUID             (15)        // input is a number

// modes for  AsfWmiHexVal2Str()
#define ASF_MODE_IPSTRDECIMAL       (15)        // get string with ip in decimal
#define ASF_MODE_MACADDRHEX         (16)        // get string in hex

// returncodes
#define ASF_RETVAL_FAIL             (-1)
#define ASF_RETVAL_UNDEFINED        (0)
#define ASF_RETVAL_SUCCESS          (1)

// Unicode String structure
typedef struct _STR_ASF_UNISTRING
{
    SK_U16  len;
    SK_U16  buf[ASF_MAX_UNICODESTRINGLEN];

} STR_ASF_UNISTRING;


// function prototypes
SK_I8 AsfMac2Asci( SK_U8 *buf, SK_U32 *len, SK_U8 *mac );
SK_I8 AsfIp2Asci( SK_U8 *buf, SK_U32 *len, SK_U8 *ip );
SK_I8 AsfAsci2Mac( SK_U8 *buf, SK_U32 len, SK_U8 *mac );
SK_I8 AsfAsci2Ip( SK_U8 *buf, SK_U32 len, SK_U8 *ip );
SK_I8 AsfHex2Array( SK_U8 *buf, SK_U32 len, SK_U8 *array );
SK_I8 AsfArray2Hex( SK_U8 *buf, SK_U32 len, SK_U8 *array );
SK_I8 AsfHex2U8( SK_U8 *buf, SK_U8 *val );
SK_I8 AsfInt2Hex( SK_U8 *buf, SK_U8 size, SK_U32 val );
SK_I8 AsfDec2Int( SK_U8 *buf, SK_U8 size, SK_U32 *val );

#ifdef __cplusplus
}
#endif  // cpp

#endif  // asfwmi.h

