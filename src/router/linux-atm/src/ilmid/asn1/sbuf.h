/*
 * sbuf_buf.h  - a buffer consisting of one contiguous block
 *               that checks for read and write range errors.
 * MS 92
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
 */

#ifndef _asn_buf_h_
#define _asn_buf_h_

typedef struct SBuf
{
    char* dataStart;  /* byte last written (or end) */
    char* dataEnd;    /* ptr to first byte after last valid data byte */  
    char* blkStart;   /* ptr to first byte of the buffer */
    char* blkEnd;     /* ptr to first byte past end of the buffer */
    char* readLoc;    /* next byte to read (or end) */     
    int writeError;   /* whether write error occurred */
    int readError;    /* whether read error occurred */
} SBuf;

/* initializes a buffer into an 'empty' state */
#define SBufInit(b, data, dataLen)\
{ (b)->readError = (b)->writeError = 1;\
  (b)->blkStart = data;\
  (b)->blkEnd = data + dataLen;\
  (b)->dataStart = (b)->dataEnd = (b)->readLoc = (b)->blkEnd;\
}
                       
#define SBufResetInReadMode(b)\
{ (b)->readLoc = (b)->dataStart;\
  (b)->readError = 0;\
  (b)->writeError = 1;\
}

#define SBufResetInWriteRvsMode(b)\
{ (b)->dataStart = (b)->dataEnd = (b)->blkEnd;\
  (b)->writeError = 0;\
  (b)->readError = 1;\
}

/* installs given block of data into a buffer and sets it up for reading */
#define SBufInstallData(b, data, dataLen)\
   SBufInit(b, data, dataLen);\
   (b)->dataStart = (b)->blkStart;\
   SBufResetInReadMode(b);

/* returns the number of bytes in the data portion */
#define SBufDataLen(b)\
  ((b)->dataEnd - (b)->dataStart)

/* returns the pointer to the first data byte */
#define SBufDataPtr(b)\
  ((b)->dataStart)

/* returns the size of block, the maximum size for data */
#define SBufBlkLen(b)\
  ((b)->blkEnd - (b)->blkStart)

/* returns a pointer to the first byte of the block */
#define SBufBlkPtr(b)\
  ((b)->blkStart)

/* returns true if there is no more data to be read in the SBuf */
#define SBufEod(b)\
  ((b)->readLoc >= (b)->dataEnd)

/* returns true if you attempted to read past the end of data */
#define SBufReadError(b)\
  ((b)->readError)

/* 
 * returns true if you attempted to write past the end of the block
 * (remember SBufs do not expand like ExpBufs)
 */
#define SBufWriteError(b)\
  ((b)->writeError)

/* Skips the next skipLen bytes for reading */
#define SBufSkip(b, skipLen)\
{ if ( ((b)->readLoc + skipLen) > (b)->dataEnd)\
  {\
      (b)->readLoc = (b)->dataEnd;\
      (b)->readError = 1;\
  }\
  else\
      (b)->readLoc += skipLen;\
}


/* 
 * copies copyLen bytes from buffer b into char* dst.
 * assumes dst is pre-allocated and is large enough.
 * Will set the read error flag is you attempt to copy
 * more than the number of unread bytes available.
 */
#define SBufCopy(dst, b, copyLen)\
{ if (((b)->readLoc + copyLen) > (b)->dataEnd)\
  {\
      memcpy(dst, (b)->readLoc, (b)->dataEnd - (b)->readLoc);\
      (b)->readLoc = (b)->dataEnd;\
      (b)->readError = 1;\
  }\
  else\
  {\
      memcpy(dst, (b)->readLoc, copyLen);\
      (b)->readLoc += copyLen;\
  }\
}

/*
 * returns the next byte from the buffer without advancing the 
 * current read location.
 */
#define SBufPeekByte(b)\
    ((SBufEod(b))? ((b)->readError = 1):(unsigned char) *((b)->readLoc))

/*
 * WARNING: this is a fragile macro. be careful where you use it.
 * return a pointer into the buffer for the next bytes to be read
 * if *lenPtr uread bytes are not available, *lenPtr will be set
 * to the number of byte that are available.  The current read location
 * is advance by the number of bytes returned in *lenPtr.  The read error
 * flag will NOT set, ever, by this routine.
 */
#define SBufGetSeg( b, lenPtr)\
    ((b)->readLoc);\
    if (((b)->readLoc + *lenPtr) > (b)->dataEnd)\
    {\
         *lenPtr = (b)->dataEnd - (b)->readLoc;\
         (b)->readLoc = (b)->dataEnd;\
    }\
    else\
        (b)->readLoc += *lenPtr;

/* 
 * Write in reverse the char* seg of segLen bytes to the buffer b.
 * A reverse write of segement really just prepends the given seg
 * (in original order) to the buffers existing data
 */
#define SBufPutSegRvs(b, seg, segLen)\
{ if (((b)->dataStart - segLen) < (b)->blkStart)\
      (b)->writeError = 1;\
  else\
  {\
     (b)->dataStart -= segLen;\
     memcpy((b)->dataStart, seg, segLen);\
  }\
}

/*
 * returns the next byte from buffer b's data and advances the
 * current read location by one byte.  This will set the read error
 * flag if you attempt to read past the end of the SBuf
 */
#define SBufGetByte(b)\
   (unsigned char)((SBufEod(b))? ((b)->readError = 1):*((b)->readLoc++))

/*
 * writes (prepends) the given byte to buffer b's data
 */
#define SBufPutByteRvs(b, byte)\
{ if ((b)->dataStart <= (b)->blkStart)\
      (b)->writeError = 1;\
  else\
      *(--(b)->dataStart) = byte;\
}
                          

#endif /* conditional include */


