/* $Id$ */
/*
** file_decomp_SWF.c
**
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <zlib.h>
#include <string.h>
#ifdef LZMA
#include <lzma.h>
#endif

#include "file_decomp.h"
#ifdef FILE_DECOMP_SWF
#include "file_decomp_SWF.h"
#include "hi_eo_events.h"

#ifdef LZMA

#define LZMA_HEADER_LEN  (13)
#define LZMA_PRP_OFFSET  (0)
#define SWF_PRP_OFFSET   (9)
#define LZMA_UCL_OFFSET  (5)
#define SWF_UCL_OFFSET   (1)
#define SWF_HDR_LEN      (SWF_SIG_LEN + SWF_VER_LEN + SWF_UCL_LEN)

static fd_status_t File_Decomp_Process_LZMA_Header( fd_session_p_t SessionPtr )
{
    uint8_t LZMA_Header[LZMA_HEADER_LEN];
    uint8_t *SWF_Header = SessionPtr->Decomp_State.SWF.Header_Bytes;
    uint32_t SWF_Uncomp_Len;
    int idx;

    lzma_ret l_ret;
    lzma_stream *l_s = &(SessionPtr->Decomp_State.SWF.StreamLZMA);

    SWF_Uncomp_Len = 0;
    /* Read little-endian into value */
    for( idx=0; idx<4; idx++ )
        SWF_Uncomp_Len +=
            (uint32_t)((uint8_t)(*(SWF_Header + SWF_UCL_OFFSET + idx)) << (8*idx) );

    if( SWF_Uncomp_Len < SWF_HDR_LEN )
    {
        SessionPtr->Error_Event = HI_EO_SERVER_SWF_LZMA_FAILURE;
        return( File_Decomp_DecompError );
    }
    
    /* Set to -1 and let liblzma calculate the size automatically */
    *((uint64_t*)(LZMA_Header + LZMA_UCL_OFFSET)) = 0xFFFFFFFFFFFFFFFF;

    /* Move the LZMA Properties */
    for( idx=0; idx<SWF_LZMA_PRP_LEN; idx++ )
        LZMA_Header[LZMA_PRP_OFFSET + idx] = *(SWF_Header + SWF_PRP_OFFSET + idx);

    l_s->next_out = SessionPtr->Next_Out;
    l_s->avail_out = SessionPtr->Avail_Out;
    l_s->total_out = SessionPtr->Total_Out;

    l_s->next_in = LZMA_Header;
    l_s->avail_in = sizeof(LZMA_Header);

    l_ret = lzma_code(l_s, LZMA_RUN);

    SessionPtr->Next_Out = l_s->next_out;
    SessionPtr->Avail_Out = l_s->avail_out;
    SessionPtr->Total_Out = l_s->total_out;

    if( l_ret != LZMA_OK )
    {
        SessionPtr->Error_Event = HI_EO_SERVER_SWF_LZMA_FAILURE;
        return( File_Decomp_DecompError );
    }

    return( File_Decomp_OK );
}
#endif

static fd_status_t Decomp( fd_session_p_t SessionPtr )
{
    switch( SessionPtr->Decomp_Type )
    {
        case FILE_COMPRESSION_TYPE_ZLIB:
        {
            int z_ret;
            z_stream *z_s = &(SessionPtr->Decomp_State.SWF.StreamZLIB);

            SYNC_IN(z_s)
 
            z_ret = inflate(z_s, Z_SYNC_FLUSH);

            SYNC_OUT(z_s)

            if( z_ret == Z_STREAM_END )
            {
                return( File_Decomp_Complete );
            }

            if( z_ret != Z_OK )
            {
                SessionPtr->Error_Event = HI_EO_SERVER_SWF_ZLIB_FAILURE;
                return( File_Decomp_DecompError );
            }


            break;
        }
#ifdef LZMA
        case FILE_COMPRESSION_TYPE_LZMA:
        {
            lzma_ret l_ret;
            lzma_stream *l_s = &(SessionPtr->Decomp_State.SWF.StreamLZMA);

            SYNC_IN(l_s)
 
            l_ret = lzma_code(l_s, LZMA_RUN);

            SYNC_OUT(l_s)

            if( l_ret == LZMA_STREAM_END )
            {
                return( File_Decomp_Complete );
            }

            if( l_ret != LZMA_OK )
            {
                SessionPtr->Error_Event = HI_EO_SERVER_SWF_LZMA_FAILURE;
                return( File_Decomp_DecompError );
            }

            break;
        }
#endif
        default:
            return( File_Decomp_Error );
    }

    return( File_Decomp_OK );
}

fd_status_t File_Decomp_End_SWF( fd_session_p_t SessionPtr )
{
    if( SessionPtr == NULL )
        return( File_Decomp_Error );

    switch( SessionPtr->Decomp_Type )
    {
        case FILE_COMPRESSION_TYPE_ZLIB:
        {
            int z_ret;
            z_stream *z_s = &(SessionPtr->Decomp_State.SWF.StreamZLIB);

            z_ret = inflateEnd(z_s);

            if( z_ret != Z_OK )
            {
                SessionPtr->Error_Event = HI_EO_SERVER_SWF_ZLIB_FAILURE;
                return( File_Decomp_DecompError );
            }

            break;
        }
#ifdef LZMA
        case FILE_COMPRESSION_TYPE_LZMA:
        {
            lzma_stream *l_s = &(SessionPtr->Decomp_State.SWF.StreamLZMA);

            lzma_end(l_s);

            break;
        }
#endif
        default:
            return( File_Decomp_Error );
    }

    return( File_Decomp_OK );
}


fd_status_t File_Decomp_Init_SWF( fd_session_p_t SessionPtr )
{
    if( SessionPtr == NULL )
        return( File_Decomp_Error );

    /* Indicate the we need to look for the remainder of the
       uncompressed header. */
    SessionPtr->Decomp_State.SWF.State = SWF_STATE_GET_HEADER;
    SessionPtr->Decomp_State.SWF.Header_Cnt = 0;

    switch( SessionPtr->Decomp_Type )
    {
        case FILE_COMPRESSION_TYPE_ZLIB:
        {
            int z_ret;
            z_stream *z_s;

            SessionPtr->Decomp_State.SWF.Header_Len = 
                SWF_VER_LEN + SWF_UCL_LEN;

            z_s = &(SessionPtr->Decomp_State.SWF.StreamZLIB);

            memset( (char *)z_s, 0, sizeof(z_stream));

            z_s->zalloc = (alloc_func)NULL;
            z_s->zfree = (free_func)NULL;
            SYNC_IN(z_s)
 
            z_ret = inflateInit(z_s);

            if( z_ret != Z_OK )
            {
                SessionPtr->Error_Event = HI_EO_SERVER_SWF_ZLIB_FAILURE;
                return( File_Decomp_DecompError );
            }

            break;
        }
#ifdef LZMA
        case FILE_COMPRESSION_TYPE_LZMA:
        {
            lzma_ret l_ret;
            lzma_stream *l_s;

            SessionPtr->Decomp_State.SWF.Header_Len = 
                SWF_VER_LEN + SWF_UCL_LEN + SWF_LZMA_CML_LEN + SWF_LZMA_PRP_LEN;

            l_s = &(SessionPtr->Decomp_State.SWF.StreamLZMA);

            memset( (char *)l_s, 0, sizeof(lzma_stream));

            SYNC_IN(l_s)
 
            l_ret = lzma_alone_decoder(l_s, UINT64_MAX);

            if( l_ret != LZMA_OK )
            {
                SessionPtr->Error_Event = HI_EO_SERVER_SWF_LZMA_FAILURE;
                return( File_Decomp_DecompError );
            }


            break;
        }
#endif
        default:
            return( File_Decomp_Error );
    }

    return( File_Decomp_OK );
}

fd_status_t File_Decomp_SWF( fd_session_p_t SessionPtr )
{
    fd_status_t Ret_Code;

    if( (SessionPtr == NULL) || (SessionPtr->File_Type != FILE_TYPE_SWF) )
        return( File_Decomp_Error );

    /* Are we still looking for the balance of the uncompressed header? */
    switch( SessionPtr->Decomp_State.SWF.State )
    {
        case( SWF_STATE_GET_HEADER ):
        {
            uint8_t *Cnt_Ptr = &(SessionPtr->Decomp_State.SWF.Header_Cnt);  // For convenience
            uint8_t *Len_Ptr = &(SessionPtr->Decomp_State.SWF.Header_Len);  // For convenience

            while( *Len_Ptr > *Cnt_Ptr )
            {
                if( SessionPtr->Avail_In == 0 )
                    return( File_Decomp_BlockIn );

                if( SessionPtr->Avail_Out == 0 )
                    return( File_Decomp_BlockOut );

                SessionPtr->Decomp_State.SWF.Header_Bytes[*Cnt_Ptr] =
                    *(SessionPtr->Next_In);

                (void)Move_1(SessionPtr);
                *Cnt_Ptr += 1;
            }

            SessionPtr->Decomp_State.SWF.State = SWF_STATE_PROC_HEADER;
            /* INTENTIONAL FALL-THROUGH INTO SWF_STATE_PROC_HEADER CASE. */
        }
        case( SWF_STATE_PROC_HEADER ):
        {
#ifdef LZMA
            if( SessionPtr->Decomp_Type == FILE_COMPRESSION_TYPE_LZMA )
            {
                Ret_Code = File_Decomp_Process_LZMA_Header( SessionPtr );
                if( Ret_Code != File_Decomp_OK )
                    return( Ret_Code );
            }
#endif

            SessionPtr->Decomp_State.SWF.State = SWF_STATE_DATA;
            /* INTENTIONAL FALL-THROUGH INTO SWF_STATE_DATA CASE. */
        }
        case( SWF_STATE_DATA ):
        {
            Ret_Code = Decomp( SessionPtr );
            if( Ret_Code != File_Decomp_Complete )
                return( Ret_Code );

            Ret_Code = File_Decomp_End_SWF( SessionPtr );
            if( Ret_Code != File_Decomp_OK )
                return( Ret_Code );

            return( File_Decomp_Complete );
        }
        default:
            return( File_Decomp_Error );
    }

    return( File_Decomp_OK );
}

#endif
