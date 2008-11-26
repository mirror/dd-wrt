/******************************************************************************
 *
 * Name:    skgeasfconv.c
 * Project: asf/ipmi
 * Version: $Revision: 1.1.2.1 $
 * Date:    $Date: 2006/08/28 09:06:52 $
 * Purpose: asf/ipmi interface in windows driver
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  (C)Copyright 1998-2002 SysKonnect GmbH.
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

#if (defined(DEBUG) || ((!defined(LINT)) && (!defined(SK_SLIM))))
static const char SysKonnectFileId[] =
"$Header: /data/cvs/sweprojects/yukon2/lindrv/asf_linux/Attic/skgeasfconv.c,v 1.1.2.1 2006/08/28 09:06:52 mlindner Exp $" ;
#endif

#define __SKASF_C

#ifdef __cplusplus
extern "C" {
#endif  /* cplusplus */


#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"
#include "h/sktypes.h"
// #include <stdlib.h>     // for string operations


SK_I8 AsfMac2Asci( SK_U8 *buf, SK_U32 *len, SK_U8 *mac )  {
	SK_I8 	RetCode;
//	SK_U8	i, x;
	SK_U8	i;

	RetCode = 1;
	*len = 0;
	for( i=0; i<6; i++ )  {
		AsfInt2Hex( &buf[*len], 2, (SK_U32) mac[i] );
		(*len)+=2;
		if( i < 5 )
			buf[(*len)++] = '-';
	}
	return( RetCode );
}

SK_I8 AsfAsci2Mac( SK_U8 *buf, SK_U32 len, SK_U8 *mac )  {
	SK_I8 	RetCode;
	SK_U8	i, ind;

	RetCode = 1;

	ind = 0;
	for( i=0; i<6; i++ )  {
		AsfHex2U8( &buf[ind], &mac[i] );
		ind+=2;
		if( (buf[ind] != '-') && (buf[ind] != ':') && ( i < 5) )  {
			RetCode = 0;
			break;
		}
		ind++;
	}

	return( RetCode );
}

SK_I8 AsfIp2Asci( SK_U8 *buf, SK_U32 *len, SK_U8 *ip )  {
	SK_I8 	RetCode;
	SK_U8	i, x;

	RetCode = 1;
	*len = 0;
	for( i=0; i<4; i++ )  {
		x = ip[i] / 100;  /*  H  */
		if( x > 0 )
			buf[(*len)++] = '0' + x;
		x = (ip[i] % 100) / 10;  /*  Z */
		if( (x > 0) || (ip[i] > 99))
			buf[(*len)++] = '0' + x;
		x = ip[i] % 10;  /* E */
		buf[(*len)++] = '0' + x;
		if( i < 3 )
			buf[(*len)++] = '.';
	}
	return( RetCode );
}

SK_I8 AsfAsci2Ip( SK_U8 *buf, SK_U32 len, SK_U8 *ip )  {
	SK_I8 	RetCode;
	SK_U8	TmpBuf [3];
	SK_U8	i,j, ind, bind;

	RetCode = 1;

	bind = 0;
	ind = 0;
	for ( i=0; i<len; i++ ) {
		if( buf[i]  != '.' )  {
			if( bind < 3 )
				TmpBuf[bind++] = buf[i];
			else  {
				RetCode = -1;
				break;
			}
		}  //  	if( buf[i]  != ':' ) 
		if( (buf[i]  == '.') || (i==(len-1)) ) {
			ip[ind] = 0;
			j = 0;
			if( bind == 3 )  {
				bind--;
				ip[ind] += ( (TmpBuf[j++]-0x30) * 100 );
			}
			if( bind == 2 )  {
				bind--;
				ip[ind] += ( (TmpBuf[j++]-0x30) * 10 );
			}
			if( bind == 1)  {
				bind--;
				ip[ind] += (TmpBuf[j++]-0x30);
			}
			ind++;
			bind = 0;
		}  //  else  if( buf[i]  != ':' )  {
	}  //	for ( i=0; i<ASF_MAC_STRLEN; i++ ) {

	return( RetCode );
}

SK_I8 AsfHex2Array( SK_U8 *buf, SK_U32 len, SK_U8 *array )  {
	SK_I8 	RetCode;
	SK_U8	i;

	RetCode = 1;

	for( i=0; i<len; i+=2 )  {
		AsfHex2U8( &buf[i], &array[i/2] );
	}
	return( RetCode );
}

SK_I8 AsfArray2Hex( SK_U8 *buf, SK_U32 len, SK_U8 *array )  {
	SK_I8 	RetCode;
	SK_U8	i;

	RetCode = 1;

	for( i=0; i<len; i+=2 )  {
		AsfInt2Hex( &buf[i], 2, (SK_U32) array[i/2] );
	}
	return( RetCode );
}

SK_I8 AsfHex2U8( SK_U8 *buf, SK_U8 *val )  {
	SK_U8 i, size;
	
	size = 2;
	*val = 0;
	for( i=0; i<size; i++ )  {
		if( (buf[size-1-i] >= '0') && (buf[size-1-i] <= '9') )
			*val |= ((buf[size-1-i]-'0') << (i*4));

		if( (buf[size-1-i] >= 'A') && (buf[size-1-i] <= 'F') )
			*val |= ((buf[size-1-i]-'A'+10) << (i*4));

		if( (buf[size-1-i] >= 'a') && (buf[size-1-i] <= 'f') )
			*val |= ((buf[size-1-i]-'a'+10) << (i*4));
	}
	return( 1 );
}

SK_I8 AsfInt2Hex( SK_U8 *buf, SK_U8 size, SK_U32 val )  {
	SK_U8 i;
	SK_U8 x;

	if( size > 4 )
		return( 0 );

	for( i=0; i<size; i++ )  {
		x = (SK_U8) (val>>(i*4))&0x000f;
		if( (x >= 0) && (x <= 9) )
			buf[size-1-i] = x + '0';
		if( (x >= 0xa) && (x <= 0xf) )
			buf[size-1-i] = x - 0xa + 'A';
	}
	return( 1 );
}

SK_I8 AsfDec2Int( SK_U8 *buf, SK_U8 size, SK_U32 *val )  {
	SK_U8 i;
	
	*val = 0;

	if( size > 4 )
		return( 0 );

	for( i=0; i<size; i++ )  {
		if( (buf[size-1-i] >= '0') && (buf[size-1-i] <= '9') )
			*val |= ((buf[size-1-i]-'0') << (i+8));
	}
	return( 1 );
}


#ifdef __cplusplus
}
#endif  /* __cplusplus */

