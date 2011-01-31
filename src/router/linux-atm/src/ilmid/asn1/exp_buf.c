/* 
 * exp_buf.c - buffer routines for the buffer structure
 *
 *                
 *  ---------         ----------
 *  | ExpBuf |<------>| ExpBuf |<------> ...ExpBufs
 *  |        |---     |        |---
 *  ----------  |     ----------  |
 *              V                 V
 *              --------          --------
 *              | DATA |          | DATA |
 *              | BLK  |          | BLK  |
 *              --------          --------
 *                
 *                
 *     ExpBuf       
 *   --------------                           
 *   | readError  |
 *   | writeError |
 *   | dataStart  |-----------                
 *   | dataEnd    |--------  |                
 *   | curr       |------ |  |                
 *   | next       |     | |  |                
 *   | prev       |     | |  |            data
 *   | blkStart   |=====|=|==|==>--------------------------               
 *   | blkEnd     |---  | |  |   |                        | (each line    
 *   --------------  |  | |  |   |                        |  reps a byte  
 *                   |  | |  |-->| - - - - - - - - - - - -|  diff in addr)
 *                   |  | |      |       valid            |           
 *                   |  |-|----->|                        |      
 *                   |    |      |        data            |      
 *                   |    |      |                        |      
 *                   |    |      | - - - - - - - - - - - -|
 *                   |    |----->|(one byte after last valid data byte)
 *                   |           |                        |
 *                   |           --------------------------
 *                   |-----------> (one byte after last byte in data blk)
 *
 *
 *   readError - set to non-zero to indicate attempt to read past end of
 *               of data
 *   writeError- set to non-zero to indicate write error.
 *               Set if Alloc of new buf fails
 *   dataStart - pts to first VALID data byte ie  *dataStart is first byte
 *   dataEnd   - pts to byte AFTER last VALID byte *dataEnd is not in the data
 *               but *(dataEnd -1) is in the data
 *   curr      - used for current read ptr - points to next byte to be read
 *               so *curr is the next byte to be read.
 *   next      - pts to next BUF in list, NULL for last BUF in list
 *   prev      - pts to prev BUF in list, NULL for first BUF in list
 *   blkStart  - pts to start of the data blk. *blkStart is first byte
 *               in the buffer's data blk.
 *   blkEnd    - pts to byte AFTER last writable byte of the dataBlk. 
 *               *(blkEnd-1) is the last byte in the buffer's data blk.
 *
 * NOTES:
 *   - dataEnd is currently always the same as blkEnd 
 *   - at End Of Data (EOD) (no more data to be read)
 *     if (curr == dataEnd)
 *   - buffer has no valid data if (dataStart == dataEnd)
 *   - number of valid data bytes = (dataEnd - dataStart)
 *   - size of the data block = (blkEnd - blkStart)
 *
 *   - the write reverse routines modify dataStart 
 *   - the read routines modify the curr ptr. 
 *   - there are no 'forward' write routines at the moment
 *     (if there were they would adjust dataEnd)
 *
 *
 * Copyright (C) 1992 Michael Sample and the University of British Columbia
 *
 * This library is free software; you can redistribute it and/or
 * modify it provided that this copyright/license information is retained
 * in original form.
 *
 * If you modify this file, you must clearly indicate your changes.
 *
 * This source code is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */        

#if HAVE_CONFIG_H
#include <config.h>
#endif 

#include "asn_config.h"
#include "exp_buf.h"


/* default buffer data block size (used when allocating) */
unsigned long expBufDataBlkSizeG = 1024;


#ifdef DEBUG  /* otherwise macros */

/*
 * sets the size of the data block to attach to 
 * an ExpBuf when allocating a new one
 */
void
ExpBufInit PARAMS((dataBlkSize),
unsigned long dataBlkSize)
{
    expBufDataBlkSizeG = dataBlkSize;
}  /* InitBuffers */

/*
 * Allocates and returns an uninitialized ExpBuf with
 * no a data attached.
 */
ExpBuf*
ExpBufAllocBuf()
{
    return ((ExpBuf*)malloc(sizeof(ExpBuf)));
}

void
ExpBufFreeBuf PARAMS( (ptr),
ExpBuf* ptr)
{
    free(ptr);
}

char*
ExpBufAllocData()
{
    return((char*)malloc(expBufDataBlkSizeG));
}

void
ExpBufFreeData PARAMS( (ptr),
char* ptr)
{
    free( ptr);
}


ExpBuf*
ExpBufNext PARAMS( (b),
ExpBuf* b)
{
    return(b->next);
}

ExpBuf*
ExpBufPrev PARAMS( (b),
ExpBuf* b)
{
    return(b->prev);
}

/*
 * set curr ptr used in reads to the first byte
 * to be read
 */
void
ExpBufResetInReadMode PARAMS((b),
ExpBuf* b)
{
    b->curr = b->dataStart;
    b->readError = 0;
    b->writeError = 1; /* catch wrong mode errors */
}

/* 
 * sets dataStart to end of buffer
 * so following writes (backward)
 * over-write any existing data associated with
 * the buffer
 */
void
ExpBufResetInWriteRvsMode PARAMS((b),
ExpBuf* b)
{
    b->dataEnd = b->dataStart = b->blkEnd;
    b->writeError = 0;
    b->readError = 1;  /* catch wrong mode errors */
}


/*
 * returns true if no more data can be read from
 * the given buffer. only valid when buffer in read (fwd)
 * mode.
 */
int
ExpBufAtEod PARAMS((b),
ExpBuf* b)
{
    return(b->curr == b->dataEnd);
}


/*
 * returns true if no more reverse writes can be done
 * to the buffer.  Only valid when buffers in reverse
 * write mode
 */
int
ExpBufFull PARAMS((b),
ExpBuf* b)
{
    return((b)->dataStart == (b)->blkStart);
}


/*
 * returns true if the given buffer has no
 * valid data in it's data block 
 */
int
ExpBufHasNoData PARAMS((b),
ExpBuf* b)
{
    return(b->dataStart == b->dataEnd);
}


/*
 * returns the number of valid data bytes in the
 * given buffer's data block
 */
unsigned long
ExpBufDataSize PARAMS((b),
ExpBuf* b)
{
    return(b->dataEnd - b->dataStart);
}

/*
 * returns size of data block that is attached to
 * the given buffer.
 */
unsigned long
ExpBufDataBlkSize PARAMS((b),
ExpBuf* b)
{
    return(b->blkEnd - b->blkStart);
}

/*
 * returns a ptr the beginning of the valid data of
 * the given buffer.
 * returns NULL is there is no valid data.
 */
char*
ExpBufDataPtr PARAMS((b),
ExpBuf* b)
{
    if (ExpBufHasNoData(b))
        return(NULL);
    else
        return(b->dataStart);
}

#endif /* DEBUG */


/*
 * returns last ExpBuf in a list of bufs.
 * The given buf can be any buf in the list.
 */
ExpBuf*
ExpBufListLastBuf PARAMS((b),
ExpBuf* b)
{
    for (; b->next != NULL; b = b->next);
    return(b);
}

/*
 * returns first buf in a list of bufs .
 * The given buf can be any buf in the list
 */
ExpBuf*
ExpBufListFirstBuf PARAMS((b),
ExpBuf* b)
{
    for (; b->prev != NULL; b = b->prev);
    return(b);
}

/*
 *  Allocates a Buf and allocates an attaches a
 *  data block of expBufDataBlkSizeG to that buffer.
 *  sets up the blk for writing in that the data start
 *  and data end point to the byte after the data blk.
 */
ExpBuf*
ExpBufAllocBufAndData()
{
    ExpBuf* retVal;

    retVal = ExpBufAllocBuf();

    if (retVal == NULL)
        return(NULL);

    retVal->readError = 0;
    retVal->writeError = 0;
    retVal->blkStart = ExpBufAllocData();

    if (retVal->blkStart == NULL)
    {
        ExpBufFreeBuf(retVal);
        return(NULL);
    }

    retVal->next = NULL;
    retVal->prev = NULL;
    retVal->curr = retVal->blkEnd = retVal->dataStart = retVal->dataEnd =
        retVal->blkStart + expBufDataBlkSizeG;

    return(retVal);
} /* ExpBufAllocBufAndData */


/*
 * Frees ExpBuf's and associated data blocks after 
 * after (next ptr) and including the given buffer, b.
 */
void
ExpBufFreeBufAndDataList PARAMS( (b),
ExpBuf* b)
{
    ExpBuf* tmp;

    for(; b != NULL;)
    {
        tmp = b->next;
        ExpBufFreeBufAndData(b);
        b = tmp;
    }
}  /* ExpBufFreeBufAndDataList */


/*
 * puts the given data in a buffer and sets it up for reading
 * the data.  This results in a "full" buffer with a data
 * blk size of given data's len
 */
void
ExpBufInstallDataInBuf PARAMS( ( buf, data, len),
ExpBuf* buf _AND_
char* data _AND_
unsigned long int len)
{
    buf->readError = 0;
    buf->writeError = 0;
    buf->blkStart = buf->dataStart = buf->curr = data;
    buf->next = NULL;
    buf->prev = NULL;
    buf->blkEnd = buf->dataEnd = data + len;
}  /* ExpBufInstallDataInBuf */



/*  Buf reading and writing routines follow */

/* READ
 * returns the next byte to be read without
 * advancing the pointer. No check for end of
 * data - this is lame
 */
unsigned char
ExpBufPeekByte PARAMS( (b),
ExpBuf** b)
{
    if ((*b)->curr == (*b)->dataEnd)
        (*b)->readError = 1;
    return(*(*b)->curr);
}  /* ExpBufPeek */


/*  READ
 *  copy the next len chars in the buffer  to the given
 *  dst char string.  The curr ptr in the buffer is advanced
 *  appropriately
 */
int
ExpBufCopy PARAMS( (dst, b, len),
char* dst _AND_
ExpBuf** b _AND_
unsigned long int len)
{
    unsigned long int gotLen;
    int totalGotLen = 0;
    char* srcPtr;

    gotLen = len;
    while (1)         /* optimize std path - eg only one ExpBufGetSeg needed */
    {
        srcPtr = ExpBufGetSeg(b, &gotLen);
        memcpy(dst + totalGotLen, srcPtr, gotLen);
        
        totalGotLen += gotLen;

        if (totalGotLen >= len)
            return(totalGotLen);

        if (gotLen == 0)  /* eod */
        {
            (*b)->readError = 1;
            return(totalGotLen);
        }

        gotLen = len - totalGotLen;
    }

    /* not reached */

}  /* ExpBufCopy */


/*
 * advance the curr ptr in the given buffer over the next
 * len bytes
 */
void
ExpBufSkip PARAMS( (b, len),
ExpBuf** b _AND_
unsigned long int len)
{
    unsigned long int lenRemaining;

    lenRemaining = len;
    while ((len > 0) && ExpBufGetSeg(b, &lenRemaining))
    {
        len -= lenRemaining;

        if (lenRemaining == 0)
        {
            (*b)->readError = 1;
            return;
        }

        lenRemaining = len;
    }
}  /* ExpBufSkip */


/*   READ
 *   returns a ptr to the next "len" bytes (contiguous).  
 *   if "len" is greater than the available contiguous bytes
 *   len is set the the number of contig. bytes the returned
 *   ptr references. Subsequent call to ExpBufGetSeg or other ExpBufGet
 *   routines will return ptrs to the following bytes (ie curr is advanced).
 *   Changes *b to pt to the next buffer and sets curr for the
 *   that buffer to dataStart if the current one has been totally read.
 *
 *   if the value returned in the len param is zero or the 
 *   returned char* is NULL then at end of data (eod)
 *
 */

char*
ExpBufGetSeg PARAMS( (b, len),
ExpBuf** b _AND_
unsigned long int* len)
{
    int bytesLeft;
    char* retVal;

    if (ExpBufAtEod(*b))
    {
        *len = 0;
        return(NULL);
    }

    bytesLeft = (*b)->dataEnd - (*b)->curr;
    retVal = (*b)->curr;

    /* check for "buffer fault" */
    if ( bytesLeft <= *len)
    {
        *len = bytesLeft;

        if ((*b)->next != NULL)
        {
            *b = (*b)->next;

            /* get next buffer with valid data */
            while ( ((*b)->next != NULL) && ExpBufHasNoData(*b) )
                *b = (*b)->next;

            /* reset current pointer to beggining of data if nec */
            (*b)->curr = (*b)->dataStart;
        }
        else
            (*b)->curr += *len;
    }
    else
        (*b)->curr += *len;

    return(retVal);
 
}  /* ExpBufGetSeg */



/*
 *   WRITE
 *   Copies len bytes from the data pointer into the given buffer 
 *
 *   FILLS EXP_BUFFERS BACKWARDS! from the end of the data to the beginning
 *   LINKS BUFFERS BACKWARDS! if a buf is full it allocs another an
 *                            puts it at the HEAD of the buffer list
 *
 *   changes *b to pt to the new "prev" buffer if the current one
 *   has been totally filled
 *   Rvs is for REVERSE!
 *
 *   modifies the dataStart pointer to reflect the new data
 */

void
ExpBufPutSegRvs PARAMS( (b, data, len),
ExpBuf** b _AND_
char* data _AND_
unsigned long int len)
{
    int bytesLeft;
    ExpBuf* buf;
    char* dataPtr;

    buf = *b;

    if (buf->writeError)
        return;

    bytesLeft = buf->dataStart - buf->blkStart;
    dataPtr = data + len;  /* pts to end of data to be written */

    /* optimize fast path */
    
    do
    {
        if (bytesLeft > len) /* enough room in this buffer for write */
        {
            buf->dataStart -= len;
            memcpy(buf->dataStart, data, len);
            break; /* this is the normal exit from this loop */
        }
        else   
        {
            /*
             * going to fill this buffer completely,
             * so alloc other one (only if one is not
             * already linked in)
             */
            dataPtr  = dataPtr - bytesLeft;
            buf->dataStart = buf->blkStart;
            memcpy(buf->dataStart, dataPtr, bytesLeft);
            
            len -= bytesLeft;
            
            if (buf->prev == NULL)
            {
                /* alloc & insert new buf at head of buffer list */
                buf = ExpBufAllocBufAndData();

                if (buf == NULL)
                {
                    (*b)->writeError = 1;
                    return;
                }

                buf->next = *b;
                (*b)->prev = buf;
            }
            else 
                buf = buf->prev;
            
            *b = buf; /* update head of list */

            bytesLeft = buf->dataStart - buf->blkStart;
        }
    }
    while (1);

    /* not reached */

}  /* ExpBufPutSegRvs */



/*
 *   returns the next byte and advances the curr ptr by one.
 *   sets the readError flag if there is no byte to read
 *   (ie at end of data)
 */
unsigned char
ExpBufGetByte PARAMS( (b),
ExpBuf** b)
{
    unsigned char retVal;
 

    if (ExpBufAtEod(*b))
    {
        (*b)->readError = 1;
        return((unsigned char)0);
    }

    retVal = *(*b)->curr++;

    /* "buffer fault" - if end of this buf, go on to next, if any */
    if ( ExpBufAtEod(*b) && ((*b)->next != NULL))
    {
        *b = (*b)->next;

        /* get next buffer with valid data */
        while ( ((*b)->next != NULL) && ExpBufHasNoData(*b) )
            *b = (*b)->next;

        /* reset current pointer to beggining of data if nec */
        (*b)->curr = (*b)->dataStart;
    }

    return(retVal);
    
}  /* ExpBufGetByte */


/* WRITE
 * Puts a single octet into the buffer
 * writes in reverse. 
 * allocates new buffers as nec - may change
 * (*b) to new buffer since writing backwards
 */
void
ExpBufPutByteRvs PARAMS( (b, byte),
ExpBuf** b _AND_
unsigned char byte)
{
    ExpBuf* new;

    if ((*b)->writeError)
        return;

    *(--(*b)->dataStart) = byte;

    /*
     * check if buffer is full and alloc new one if nec
     * and insert it before this one since writing backwards
     */
    if (ExpBufFull(*b))
    {
        if ((*b)->prev == NULL)
        {
            /*
             * no prev buf so alloc & insert
             * new buf as head of buffer list
             */
            new = ExpBufAllocBufAndData();
            if (new == NULL)
            {
                (*b)->writeError = 1;
                return;
            }
                
            new->next = *b;
            (*b)->prev = new;
            *b = new;
        }
        else 
        {
            (*b) = (*b)->prev;
            ExpBufResetInWriteRvsMode(*b);
        }

    }
} /* ExpBufPutByteRvs */
