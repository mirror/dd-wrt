/* $Id$ */
/*
** file_decomp.c
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

//#include <zlib.h>
//#ifdef LZMA
//#include <lzma.h>
//#endif

#include "util.h"
#include "mempool.h"
#include "file_decomp.h"
#include "sf_types.h"
#include "detection_util.h"
#include "memory_stats.h"

#ifdef FILE_DECOMP_PDF
#include "file_decomp_PDF.h"
#endif
#ifdef FILE_DECOMP_SWF
#include "file_decomp_SWF.h"
#endif

#ifdef FILE_DECOMP_PDF
static const char PDF_Sig[5] = { '%', 'P', 'D', 'F', '-' };
#endif
#ifdef FILE_DECOMP_SWF
static const char SWF_ZLIB_Sig[3] = { 'C', 'W', 'S' };
#ifdef LZMA
static const char SWF_LZMA_Sig[3] = { 'Z', 'W', 'S' };
#endif
static const char SWF_Uncomp_Sig[3] = { 'F', 'W', 'S' };
#endif

/* Please assure that the following value correlates with the set of sig's */
#define MAX_SIG_LENGTH (5)

static struct sig_map_s
{
    const char *Sig;
    size_t Sig_Length;
    bool Enabled;
    file_type_t File_Type;
    file_compression_type_t File_Compression_Type;
} Signature_Map[] =
{

#ifdef FILE_DECOMP_PDF
    { PDF_Sig, sizeof(PDF_Sig), false, FILE_TYPE_PDF, FILE_COMPRESSION_TYPE_NONE },  // Compression type is embedded in PDF dictionaries
#endif /* FILE_DECOMP_PDF */
#ifdef FILE_DECOMP_SWF
    { SWF_ZLIB_Sig, sizeof(SWF_ZLIB_Sig), false, FILE_TYPE_SWF, FILE_COMPRESSION_TYPE_ZLIB },
#ifdef LZMA
    { SWF_LZMA_Sig, sizeof(SWF_LZMA_Sig), false, FILE_TYPE_SWF, FILE_COMPRESSION_TYPE_LZMA },
#endif /* LZMA */
#endif /* FILE_DECOMP_SWF */
    { NULL, 0, false, FILE_TYPE_NONE, FILE_COMPRESSION_TYPE_NONE }
};


static fd_config_p_t g_Config = NULL;

/* Define the elements of the Sig_State value (packed for storage efficiency */
#define SIG_MATCH_ACTIVE    (0x80)
#define SIG_SIG_INDEX_MASK  (0x70)
#define SIG_SIG_INDEX_SHIFT (4)
#define SIG_CHR_INDEX_MASK  (0x07)
#define SIG_CHR_INDEX_SHIFT (0)

static uint8_t File_Decomp_Buffer[DECODE_BLEN];

/* Look for possible sig at the current payload location.
   Do NOT beyond the current location (initial Next_In). */
static fd_status_t Locate_Sig_Here( fd_session_p_t SessionPtr )
{
    unsigned int Sig_Index, Char_Index;

    /* If there's no new input, we don't change state */
    if( (SessionPtr->Avail_In == 0) ||
        (SessionPtr->Next_In == NULL) || (SessionPtr->Next_Out == NULL) )
        return( File_Decomp_Error );

    if( SessionPtr->Avail_Out < MAX_SIG_LENGTH )
        return( File_Decomp_BlockOut );

    /* Given that we are here, there is at least one input byte to process.
       And at least enough room in the output stream for the signature. */

   /* Have we started down a sig string? */
    if( (SessionPtr->Sig_State & SIG_MATCH_ACTIVE) != 0 )
    {
        /* Get the current index into the sig map table (indicating which sig) and
           the index into the sig itself.  */
        Sig_Index = (SessionPtr->Sig_State & SIG_SIG_INDEX_MASK) >> SIG_SIG_INDEX_SHIFT;
        /* Char_Index indicates the sig char that we are looking for now. */
        Char_Index = (SessionPtr->Sig_State & SIG_CHR_INDEX_MASK) >> SIG_CHR_INDEX_SHIFT;
    }
    else
    {
        Sig_Index = 0;
        Char_Index = 0;
    }

    /* There must be more in the input stream for us to look at, else
       we indicate that we didn't find the sig yet. */
    if( SessionPtr->Avail_In <= Char_Index )
        return( File_Decomp_BlockIn );

    /* NOTE:  The following code block makes the assumption that there are
              at least MAX_SIG_LENGTH bytes in the output buffer.  This assumption
              is valid for the current implementation where the signature only
              occurs at the beginning of the file.  For the generic case of the sig
              begin embedded with the file, the seach will need to modified.*/
    while( 1 )
    {
        /* if we get to the end of the sig table (or the table is empty),
           indicate that we didn't match a sig */
        if( Signature_Map[Sig_Index].Sig == NULL )
            return( File_Decomp_NoSig );

        /* Get next char and see if it matches next char in sig */
        if( (Signature_Map[Sig_Index].Enabled) &&
            (*(SessionPtr->Next_In+Char_Index) == *(Signature_Map[Sig_Index].Sig+Char_Index)) )
        {
            /* Check to see if we are at the end of the sig string. */
            if( Char_Index == (Signature_Map[Sig_Index].Sig_Length-1) )
            {
                uint8_t *Sig = (uint8_t *)Signature_Map[Sig_Index].Sig;
                uint16_t Len = (uint16_t)Signature_Map[Sig_Index].Sig_Length;

                SessionPtr->File_Type = Signature_Map[Sig_Index].File_Type;
                SessionPtr->Decomp_Type = Signature_Map[Sig_Index].File_Compression_Type;

                if( (SessionPtr->File_Type == FILE_TYPE_SWF) && ((SessionPtr->Modes & FILE_REVERT_BIT) != 0) )
                {
                    Sig = (uint8_t *)SWF_Uncomp_Sig;
                    Len = (uint16_t)sizeof( SWF_Uncomp_Sig );
                }
                /* The following is safe as we can only be here is there are
                   are least MAX_SIG_LENGTH bytes in the output buffer */
                (void)Put_N(SessionPtr, Sig, Len);
                /* Skip the Sig bytes in the input stream */
                SessionPtr->Next_In += Len;
                SessionPtr->Avail_In -= Len;
                SessionPtr->Total_In += Len;
                return( File_Decomp_OK );
            }

            /* check for more available input bytes */
            if( Char_Index < SessionPtr->Avail_In )
            {
                /* Set to the next char and keep checking this matching sig */
                Char_Index += 1;
                continue; /* goto top of while() loop */
            }
            else
            {
                /* Indicate that we are actively finding a sig, save the char index
                   and save the sig index.  We'll pickup where we left off when more
                   input is available. */
                SessionPtr->Sig_State = SIG_MATCH_ACTIVE |
                                       ((Sig_Index & SIG_SIG_INDEX_MASK) << SIG_SIG_INDEX_SHIFT) |
                                       ((Char_Index & SIG_CHR_INDEX_MASK) << SIG_CHR_INDEX_SHIFT);
                return( File_Decomp_BlockIn );
            }
        }
        else
        {
            /* Failed somewhere matching this sig, goto next sig and reset the
               Char_Index to the beginning */
            Sig_Index += 1;
            Char_Index = 0;
        }
    }
}

static fd_status_t Initialize_Decompression( fd_session_p_t SessionPtr )
{
    fd_status_t Ret_Code = File_Decomp_OK;

    switch( SessionPtr->File_Type )
    {
#ifdef FILE_DECOMP_SWF
        case( FILE_TYPE_SWF ):
        {
            Ret_Code = File_Decomp_Init_SWF( SessionPtr );
            break;
        }
#endif
#ifdef FILE_DECOMP_PDF
        case( FILE_TYPE_PDF ):
        {
            Ret_Code = File_Decomp_Init_PDF( SessionPtr );
            break;
        }
#endif
        default:
           return( File_Decomp_Error );
    }

    if( Ret_Code == File_Decomp_OK )
        SessionPtr->State = STATE_ACTIVE;

    return( Ret_Code );
}

static fd_status_t Process_Decompression( fd_session_p_t SessionPtr )
{
    fd_status_t Ret_Code = File_Decomp_OK;

    switch( SessionPtr->File_Type )
    {
#ifdef FILE_DECOMP_SWF
        case( FILE_TYPE_SWF ):
        {
            Ret_Code = File_Decomp_SWF( SessionPtr );
            break;
        }
#endif
#ifdef FILE_DECOMP_PDF
        case( FILE_TYPE_PDF ):
        {
            Ret_Code = File_Decomp_PDF( SessionPtr );
            break;
        }
#endif
        default:
           return( File_Decomp_Error );
    }

    if( Ret_Code == File_Decomp_Complete )
        SessionPtr->State = STATE_COMPLETE;

    return( Ret_Code );
}

/* File_Decomp_OneTimeInit() is called at the preprocessor init to clear internal state */
fd_status_t File_Decomp_OneTimeInit()
{
    g_Config = NULL;

    return( File_Decomp_OK );
}

/* File_Decomp_CleanExit() is called at the preprocessor 'clean exit' time to release resources */
fd_status_t File_Decomp_CleanExit()
{
    if( (g_Config != NULL) && (g_Config->fd_MemPool != NULL) && (mempool_destroy(g_Config->fd_MemPool) == 0))
    {
        SnortPreprocFree(g_Config->fd_MemPool, sizeof(MemPool), PP_HTTPINSPECT, 
             PP_MEM_CATEGORY_MEMPOOL);
        g_Config->fd_MemPool = NULL;
        g_Config = NULL;
        return( File_Decomp_OK );
    }
    else
    {
        return( File_Decomp_Error );
    }
}

/* The caller provides the memory size in the config struct. */
fd_status_t File_Decomp_Config( fd_config_p_t ConfigPtr )
{
    unsigned Max_Sessions;

    if( ConfigPtr == NULL )
        return( File_Decomp_Error );

    /* Our soul global, pointer to the config block */
    g_Config = ConfigPtr;

    Max_Sessions = ConfigPtr->Max_Memory / sizeof( fd_session_t );

    g_Config->fd_MemPool = (MemPool *)SnortPreprocAlloc(1, sizeof(MemPool),
                                           PP_HTTPINSPECT, PP_MEM_CATEGORY_MEMPOOL);
    if( g_Config->fd_MemPool == NULL )
        return( File_Decomp_Error );

    if( (mempool_init(g_Config->fd_MemPool, Max_Sessions, sizeof( fd_session_t ))) != 0 )
        return( File_Decomp_Error );

    return( File_Decomp_OK );
}

/* The caller provides Compr_Depth, Decompr_Depth and Modes in the session object.
   Based on the requested Modes, gear=up to initialize the potential decompressors. */
fd_status_t File_Decomp_Init( fd_session_p_t SessionPtr )
{
    int Sig;

    if( SessionPtr == NULL )
        return( File_Decomp_Error );

    SessionPtr->State = STATE_READY;
    SessionPtr->File_Type = FILE_TYPE_NONE;
    SessionPtr->Decomp_Type = FILE_COMPRESSION_TYPE_NONE;

    for( Sig=0; Signature_Map[Sig].Sig != NULL; Sig++ )
    {
#ifdef FILE_DECOMP_PDF
        if( (Signature_Map[Sig].File_Type == FILE_TYPE_PDF ) &&
            ((SessionPtr->Modes & FILE_PDF_ANY) != 0) )
            Signature_Map[Sig].Enabled = true;
#endif

#ifdef FILE_DECOMP_SWF
        if( (Signature_Map[Sig].File_Type == FILE_TYPE_SWF ) &&
            (Signature_Map[Sig].File_Compression_Type == FILE_COMPRESSION_TYPE_ZLIB) &&
            ((SessionPtr->Modes & FILE_SWF_ZLIB_BIT) != 0) )
            Signature_Map[Sig].Enabled = true;

#ifdef LZMA
        if( (Signature_Map[Sig].File_Type == FILE_TYPE_SWF ) &&
            (Signature_Map[Sig].File_Compression_Type == FILE_COMPRESSION_TYPE_LZMA) &&
            ((SessionPtr->Modes & FILE_SWF_LZMA_BIT) != 0) )
            Signature_Map[Sig].Enabled = true;
#endif
#endif

    }

    return( File_Decomp_OK );
}

/* Setup session to use internal decompression buffer. Set compr/decompr limits */
fd_status_t File_Decomp_SetBuf( fd_session_p_t SessionPtr )
{
    if( SessionPtr == NULL )
        return( File_Decomp_Error );

    SessionPtr->Buffer = File_Decomp_Buffer;
    SessionPtr->Buffer_Len = sizeof(File_Decomp_Buffer);

    SessionPtr->Next_Out = File_Decomp_Buffer;
    SessionPtr->Avail_Out = sizeof(File_Decomp_Buffer);

    /* If Compr/Decompr limits are set, then enforce then. */
    if( SessionPtr->Decompr_Depth > 0 )
    {
        uint32_t remainder;

        if( SessionPtr->Total_Out > SessionPtr->Decompr_Depth )
            return( File_Decomp_Error );

        /* Calc whats left in allowance */
        remainder = (SessionPtr->Total_Out - SessionPtr->Decompr_Depth);

        /* Use smaller of remainder or value provided */
        SessionPtr->Avail_Out = (remainder < SessionPtr->Avail_Out) ?
                                 remainder : SessionPtr->Avail_Out;
    }

    if( SessionPtr->Compr_Depth > 0 )
    {
        uint32_t remainder;

        if( SessionPtr->Total_In > SessionPtr->Compr_Depth )
            return( File_Decomp_Error );

        remainder = (SessionPtr->Total_In - SessionPtr->Compr_Depth);

        SessionPtr->Avail_In = (remainder < SessionPtr->Avail_In) ?
                                remainder : SessionPtr->Avail_In;
    }

    /* SessionPtr->Next_In is set by the caller to File_Decomp() */

    return( File_Decomp_OK );
}

/* Returns a new session object from the MemPool */
fd_session_p_t File_Decomp_New(void* scbPtr)
{
    fd_session_p_t New_Session;
    MemBucket *bkt;

    if( (g_Config == NULL) || (g_Config->fd_MemPool) == NULL )
        return( NULL );

    bkt = mempool_alloc(g_Config->fd_MemPool);

    if( bkt == NULL )
    {
        mempool_free( g_Config->fd_MemPool, bkt );

        return( (fd_session_p_t)NULL );
    }
    else
    {
        bkt->scbPtr = scbPtr;
        New_Session = bkt->data;
        New_Session->bkt = bkt;
        New_Session->State = STATE_NEW;
        New_Session->Sig_State = 0;
        New_Session->Total_In = 0;
        New_Session->Total_Out = 0;
        New_Session->Avail_In = 0;
        New_Session->Next_In = NULL;
        New_Session->Avail_Out = 0;
        New_Session->Next_Out = NULL;

        return( New_Session );
    }
}

/* Process Decompression.  The session Next_In, Avail_In, Next_Out, Avail_Out MUST have been
   set by caller.
*/
fd_status_t File_Decomp( fd_session_p_t SessionPtr )
{
    fd_status_t Return_Code;

    if( (g_Config == NULL) || (g_Config->fd_MemPool == NULL) || (SessionPtr->State == STATE_NEW) ||
        (SessionPtr->Next_In == NULL) || (SessionPtr->Next_Out == NULL) )
        return( File_Decomp_Error );

    /* STATE_NEW: Look for one of the configured file signatures. */
    if( SessionPtr->State == STATE_READY )
    {
        /* Look for the signature at the beginning of the payload stream. */
        if( (Return_Code = Locate_Sig_Here( SessionPtr )) == File_Decomp_OK )
        {
            /* We now know the file type and decompression type.  Setup appropriate state. */
            if( (Return_Code = Initialize_Decompression( SessionPtr )) == File_Decomp_OK )
            {
                return( Process_Decompression( SessionPtr ) );
            }
            else
                return( Return_Code );
        }
        else
            /* Locate_Sig_Here() might return BlockIn, BlockOut, Error, or NoSig */
            return( Return_Code );
    }
    else if( SessionPtr->State == STATE_ACTIVE )
    {
        return( Process_Decompression( SessionPtr ) );
    }
    else
        return( File_Decomp_Error );
}


fd_status_t File_Decomp_End( fd_session_p_t SessionPtr )
{
    if( SessionPtr == NULL )
        return( File_Decomp_Error );

    switch( SessionPtr->File_Type )
    {
#ifdef FILE_DECOMP_SWF
        case( FILE_TYPE_SWF ):
        {
            return( File_Decomp_End_SWF( SessionPtr ) );
        }
#endif
#ifdef FILE_DECOMP_PDF
        case( FILE_TYPE_PDF ):
        {
            return( File_Decomp_End_PDF( SessionPtr ) );
        }
#endif
        default:
           return( File_Decomp_Error );
    }

    return( File_Decomp_OK );
}

fd_status_t File_Decomp_Reset( fd_session_p_t SessionPtr )
{
    fd_status_t Ret_Code;

    if( SessionPtr == NULL )
        return( File_Decomp_Error );

    Ret_Code = File_Decomp_End( SessionPtr );

    SessionPtr->State = STATE_READY;

    return( Ret_Code );
}

fd_status_t File_Decomp_StopFree( fd_session_p_t SessionPtr )
{
    if( SessionPtr == NULL )
        return( File_Decomp_Error );

    File_Decomp_End( SessionPtr );
    File_Decomp_Free( SessionPtr );

    return( File_Decomp_OK );
}


void File_Decomp_Free( fd_session_p_t SessionPtr )
{
    mempool_free(g_Config->fd_MemPool, SessionPtr->bkt );
}

void File_Decomp_Alert( fd_session_p_t SessionPtr, int Event )
{
    if( (SessionPtr != NULL) && (SessionPtr->Alert_Callback != NULL) && (SessionPtr->Alert_Context) )
        (SessionPtr->Alert_Callback)(SessionPtr->Alert_Context, Event);
}
