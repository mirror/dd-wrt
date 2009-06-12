/*
  LzmaDecodeSize.c
  LZMA Decoder (optimized for Size version)
  
  LZMA SDK 4.16 Copyright (c) 1999-2005 Igor Pavlov (2005-03-18)
  http://www.7-zip.org/

  LZMA SDK is licensed under two licenses:
  1) GNU Lesser General Public License (GNU LGPL)
  2) Common Public License (CPL)
  It means that you can select one of these two licenses and 
  follow rules of that license.

  SPECIAL EXCEPTION:
  Igor Pavlov, as the author of this code, expressly permits you to 
  statically or dynamically link your code (or bind by name) to the 
  interfaces of this file without subjecting your linked code to the 
  terms of the CPL or GNU LGPL. Any modifications or additions 
  to this file, however, are subject to the LGPL or CPL terms.
*/
#include <lzma.h>
#include "LzmaDecode.h"

#define  NO_MALLOC

#ifndef Byte
#define Byte unsigned char
#endif

#define kNumTopBits 24
#define kTopValue ((UInt32)1 << kNumTopBits)

#define kNumBitModelTotalBits 11
#define kBitModelTotal (1 << kNumBitModelTotalBits)
#define kNumMoveBits 5


typedef struct _CRangeDecoder
{
  UInt32 Range;
  UInt32 Code;
} CRangeDecoder;

static unsigned int offset=0;
static unsigned int vall;
static unsigned char *buffer;
static unsigned char *BufferLim;
static  int ExtraBytes;

static Byte GetByte(void)
{
if (((unsigned int)offset % 4) == 0) {
	vall = *(unsigned int *)buffer;
	buffer += 4;
}
    if (buffer>=BufferLim)
	{
	ExtraBytes=1;
	return 0xff;
	}
  return *(((unsigned char *)&vall) + (offset++ & 3));
}

/* #define ReadByte (*rd->Buffer++) */
#define ReadByte (GetByte())

void RangeDecoderInit(CRangeDecoder *rd,
    Byte *stream, UInt32 bufferSize)
{
  int i;
  BufferLim = stream + bufferSize;
  ExtraBytes = 0;
  rd->Code = 0;
  rd->Range = (0xFFFFFFFF);
  for(i = 0; i < 5; i++)
    rd->Code = (rd->Code << 8) | ReadByte;
}

#define RC_INIT_VAR UInt32 range = rd->Range; UInt32 code = rd->Code;        
#define RC_FLUSH_VAR rd->Range = range; rd->Code = code;
#define RC_NORMALIZE if (range < kTopValue) { range <<= 8; code = (code << 8) | ReadByte; }

UInt32 RangeDecoderDecodeDirectBits(CRangeDecoder *rd, int numTotalBits)
{
  RC_INIT_VAR
  UInt32 result = 0;
  int i;
  for (i = numTotalBits; i != 0; i--)
  {
    /* UInt32 t; */
    range >>= 1;

    result <<= 1;
    if (code >= range)
    {
      code -= range;
      result |= 1;
    }
    /*
    t = (code - range) >> 31;
    t &= 1;
    code -= range & (t - 1);
    result = (result + result) | (1 - t);
    */
    RC_NORMALIZE
  }
  RC_FLUSH_VAR
  return result;
}

int RangeDecoderBitDecode(CProb *prob, CRangeDecoder *rd)
{
  UInt32 bound = (rd->Range >> kNumBitModelTotalBits) * *prob;
  if (rd->Code < bound)
  {
    rd->Range = bound;
    *prob += (kBitModelTotal - *prob) >> kNumMoveBits;
    if (rd->Range < kTopValue)
    {
      rd->Code = (rd->Code << 8) | ReadByte;
      rd->Range <<= 8;
    }
    return 0;
  }
  else
  {
    rd->Range -= bound;
    rd->Code -= bound;
    *prob -= (*prob) >> kNumMoveBits;
    if (rd->Range < kTopValue)
    {
      rd->Code = (rd->Code << 8) | ReadByte;
      rd->Range <<= 8;
    }
    return 1;
  }
}

#define RC_GET_BIT2(prob, mi, A0, A1) \
  UInt32 bound = (range >> kNumBitModelTotalBits) * *prob; \
  if (code < bound) \
    { A0; range = bound; *prob += (kBitModelTotal - *prob) >> kNumMoveBits; mi <<= 1; } \
  else \
    { A1; range -= bound; code -= bound; *prob -= (*prob) >> kNumMoveBits; mi = (mi + mi) + 1; } \
  RC_NORMALIZE

#define RC_GET_BIT(prob, mi) RC_GET_BIT2(prob, mi, ; , ;)               

int RangeDecoderBitTreeDecode(CProb *probs, int numLevels, CRangeDecoder *rd)
{
  int mi = 1;
  int i;
  #ifdef _LZMA_LOC_OPT
  RC_INIT_VAR
  #endif
  for(i = numLevels; i != 0; i--)
  {
    #ifdef _LZMA_LOC_OPT
    CProb *prob = probs + mi;
    RC_GET_BIT(prob, mi)
    #else
    mi = (mi + mi) + RangeDecoderBitDecode(probs + mi, rd);
    #endif
  }
  #ifdef _LZMA_LOC_OPT
  RC_FLUSH_VAR
  #endif
  return mi - (1 << numLevels);
}

int RangeDecoderReverseBitTreeDecode(CProb *probs, int numLevels, CRangeDecoder *rd)
{
  int mi = 1;
  int i;
  int symbol = 0;
  #ifdef _LZMA_LOC_OPT
  RC_INIT_VAR
  #endif
  for(i = 0; i < numLevels; i++)
  {
    #ifdef _LZMA_LOC_OPT
    CProb *prob = probs + mi;
    RC_GET_BIT2(prob, mi, ; , symbol |= (1 << i))
    #else
    int bit = RangeDecoderBitDecode(probs + mi, rd);
    mi = mi + mi + bit;
    symbol |= (bit << i);
    #endif
  }
  #ifdef _LZMA_LOC_OPT
  RC_FLUSH_VAR
  #endif
  return symbol;
}

Byte LzmaLiteralDecode(CProb *probs, CRangeDecoder *rd)
{ 
  int symbol = 1;
  #ifdef _LZMA_LOC_OPT
  RC_INIT_VAR
  #endif
  do
  {
    #ifdef _LZMA_LOC_OPT
    CProb *prob = probs + symbol;
    RC_GET_BIT(prob, symbol)
    #else
    symbol = (symbol + symbol) | RangeDecoderBitDecode(probs + symbol, rd);
    #endif
  }
  while (symbol < 0x100);
  #ifdef _LZMA_LOC_OPT
  RC_FLUSH_VAR
  #endif
  return symbol;
}

Byte LzmaLiteralDecodeMatch(CProb *probs, CRangeDecoder *rd, Byte matchByte)
{ 
  int symbol = 1;
  #ifdef _LZMA_LOC_OPT
  RC_INIT_VAR
  #endif
  do
  {
    int bit;
    int matchBit = (matchByte >> 7) & 1;
    matchByte <<= 1;
    #ifdef _LZMA_LOC_OPT
    {
      CProb *prob = probs + 0x100 + (matchBit << 8) + symbol;
      RC_GET_BIT2(prob, symbol, bit = 0, bit = 1)
    }
    #else
    bit = RangeDecoderBitDecode(probs + 0x100 + (matchBit << 8) + symbol, rd);
    symbol = (symbol << 1) | bit;
    #endif
    if (matchBit != bit)
    {
      while (symbol < 0x100)
      {
        #ifdef _LZMA_LOC_OPT
        CProb *prob = probs + symbol;
        RC_GET_BIT(prob, symbol)
        #else
        symbol = (symbol + symbol) | RangeDecoderBitDecode(probs + symbol, rd);
        #endif
      }
      break;
    }
  }
  while (symbol < 0x100);
  #ifdef _LZMA_LOC_OPT
  RC_FLUSH_VAR
  #endif
  return symbol;
}

#define kNumPosBitsMax 4
#define kNumPosStatesMax (1 << kNumPosBitsMax)

#define kLenNumLowBits 3
#define kLenNumLowSymbols (1 << kLenNumLowBits)
#define kLenNumMidBits 3
#define kLenNumMidSymbols (1 << kLenNumMidBits)
#define kLenNumHighBits 8
#define kLenNumHighSymbols (1 << kLenNumHighBits)

#define LenChoice 0
#define LenChoice2 (LenChoice + 1)
#define LenLow (LenChoice2 + 1)
#define LenMid (LenLow + (kNumPosStatesMax << kLenNumLowBits))
#define LenHigh (LenMid + (kNumPosStatesMax << kLenNumMidBits))
#define kNumLenProbs (LenHigh + kLenNumHighSymbols) 

int LzmaLenDecode(CProb *p, CRangeDecoder *rd, int posState)
{
  if(RangeDecoderBitDecode(p + LenChoice, rd) == 0)
    return RangeDecoderBitTreeDecode(p + LenLow +
        (posState << kLenNumLowBits), kLenNumLowBits, rd);
  if(RangeDecoderBitDecode(p + LenChoice2, rd) == 0)
    return kLenNumLowSymbols + RangeDecoderBitTreeDecode(p + LenMid +
        (posState << kLenNumMidBits), kLenNumMidBits, rd);
  return kLenNumLowSymbols + kLenNumMidSymbols + 
      RangeDecoderBitTreeDecode(p + LenHigh, kLenNumHighBits, rd);
}

#define kNumStates 12
#define kNumLitStates 7

#define kStartPosModelIndex 4
#define kEndPosModelIndex 14
#define kNumFullDistances (1 << (kEndPosModelIndex >> 1))

#define kNumPosSlotBits 6
#define kNumLenToPosStates 4

#define kNumAlignBits 4
#define kAlignTableSize (1 << kNumAlignBits)

#define kMatchMinLen 2

#define IsMatch 0
#define IsRep (IsMatch + (kNumStates << kNumPosBitsMax))
#define IsRepG0 (IsRep + kNumStates)
#define IsRepG1 (IsRepG0 + kNumStates)
#define IsRepG2 (IsRepG1 + kNumStates)
#define IsRep0Long (IsRepG2 + kNumStates)
#define PosSlot (IsRep0Long + (kNumStates << kNumPosBitsMax))
#define SpecPos (PosSlot + (kNumLenToPosStates << kNumPosSlotBits))
#define Align (SpecPos + kNumFullDistances - kEndPosModelIndex)
#define LenCoder (Align + kAlignTableSize)
#define RepLenCoder (LenCoder + kNumLenProbs)
#define Literal (RepLenCoder + kNumLenProbs)

#if Literal != LZMA_BASE_SIZE
StopCompilingDueBUG
#endif

int LzmaDecode(
    Byte *buffer, UInt32 bufferSize,
    int lc, int lp, int pb,
    unsigned char *inStream, UInt32 inSize,
    unsigned char *outStream, UInt32 outSize,
    UInt32 *outSizeProcessed)
{
  UInt32 numProbs = Literal + ((UInt32)LZMA_LIT_SIZE << (lc + lp));
  CProb *p = (CProb *)buffer;
  CRangeDecoder rd;
  UInt32 i;
  int state = 0;
  Byte previousByte = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  UInt32 nowPos = 0;
  UInt32 posStateMask = (1 << pb) - 1;
  UInt32 literalPosMask = (1 << lp) - 1;
  int len = 0;
  if (bufferSize < numProbs * sizeof(CProb))
    return LZMA_RESULT_NOT_ENOUGH_MEM;
  for (i = 0; i < numProbs; i++)
    p[i] = kBitModelTotal >> 1; 
  RangeDecoderInit(&rd,inStream, inSize);
  *outSizeProcessed = 0;
  while(nowPos < outSize)
  {
    int posState = (int)(nowPos & posStateMask);
    if (ExtraBytes != 0)
      return LZMA_RESULT_DATA_ERROR;
    if (RangeDecoderBitDecode(p + IsMatch + (state << kNumPosBitsMax) + posState, &rd) == 0)
    {
      CProb *probs = p + Literal + (LZMA_LIT_SIZE * 
        (((
        (nowPos 
        )
        & literalPosMask) << lc) + (previousByte >> (8 - lc))));

      if (state >= kNumLitStates)
      {
        Byte matchByte;
        matchByte = outStream[nowPos - rep0];
        previousByte = LzmaLiteralDecodeMatch(probs, &rd, matchByte);
      } else {
        previousByte = LzmaLiteralDecode(probs, &rd);

      }
      outStream[nowPos++] = previousByte;
      if (state < 4) state = 0;
      else if (state < 10) state -= 3;
      else state -= 6;
    }
    else             
    {
      if (RangeDecoderBitDecode(p + IsRep + state, &rd) == 1)
      {
        if (RangeDecoderBitDecode(p + IsRepG0 + state, &rd) == 0)
        {
          if (RangeDecoderBitDecode(p + IsRep0Long + (state << kNumPosBitsMax) + posState, &rd) == 0)
          {
            if (
               (nowPos 
               )
               == 0)
              return LZMA_RESULT_DATA_ERROR;
            state = state < 7 ? 9 : 11;
            previousByte = outStream[nowPos - rep0];
            outStream[nowPos++] = previousByte;
            continue;
          }
        }
        else
        {
          UInt32 distance;
          if(RangeDecoderBitDecode(p + IsRepG1 + state, &rd) == 0)
            distance = rep1;
          else 
          {
            if(RangeDecoderBitDecode(p + IsRepG2 + state, &rd) == 0)
              distance = rep2;
            else
            {
              distance = rep3;
              rep3 = rep2;
            }
            rep2 = rep1;
          }
          rep1 = rep0;
          rep0 = distance;
        }
        len = LzmaLenDecode(p + RepLenCoder, &rd, posState);
        state = state < 7 ? 8 : 11;
      }
      else
      {
        int posSlot;
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        state = state < 7 ? 7 : 10;
        len = LzmaLenDecode(p + LenCoder, &rd, posState);
        posSlot = RangeDecoderBitTreeDecode(p + PosSlot +
            ((len < kNumLenToPosStates ? len : kNumLenToPosStates - 1) << 
            kNumPosSlotBits), kNumPosSlotBits, &rd);
        if (posSlot >= kStartPosModelIndex)
        {
          int numDirectBits = ((posSlot >> 1) - 1);
          rep0 = ((2 | ((UInt32)posSlot & 1)) << numDirectBits);
          if (posSlot < kEndPosModelIndex)
          {
            rep0 += RangeDecoderReverseBitTreeDecode(
                p + SpecPos + rep0 - posSlot - 1, numDirectBits, &rd);
          }
          else
          {
            rep0 += RangeDecoderDecodeDirectBits(&rd, 
                numDirectBits - kNumAlignBits) << kNumAlignBits;
            rep0 += RangeDecoderReverseBitTreeDecode(p + Align, kNumAlignBits, &rd);
          }
        }
        else
          rep0 = posSlot;
        rep0++;
      }
      if (rep0 == (UInt32)(0))
      {
        /* it's for stream version */
        len = -1;
        break;
      }
      if (rep0 > nowPos 
        )
      {
        return LZMA_RESULT_DATA_ERROR;
      }
      len += kMatchMinLen;
      do
      {
        previousByte = outStream[nowPos - rep0];
        outStream[nowPos++] = previousByte;
        len--;
      }
      while(len != 0 && nowPos < outSize);
    }
  }

  *outSizeProcessed = nowPos;
  return LZMA_RESULT_OK;
}


#define LZMA_MAX_SIZE 16000
char lzmaInternalData[LZMA_MAX_SIZE];
int lzma_decode(unsigned char *inBuffer,unsigned char *outBuffer,
		unsigned int inLength,unsigned int *outLength ) {


  unsigned int compressedSize, outSize, outSizeProcessed, lzmaInternalSize;
  int i,res;
  int lc, lp, pb;
  unsigned char properties[5];
  unsigned char prop0;
  buffer = inBuffer;
  offset=0;
  BufferLim = inBuffer + inLength;
  for (i=0;i<5;i++) 
  {
     properties[i] = GetByte();
  }

  outSize=0;
  for (i= 0 ; i < 4; i++) 
  {
     outSize += (unsigned int)(GetByte()) << (i* 8);
  }
  if (outSize == 0xFFFFFFFF) 
  {
    diag_printf("stream version is not supported.\n");
    return 1;
  }
  for (i = 0; i < 4; i++)
  {
    if (GetByte() != 0)
    {
      diag_printf("too long file.\n");
      return 1; 
    }
  }
  prop0 = properties[0];
  if (prop0 >= (9*5*5))
  {
    diag_printf("Properties error.\n");
    return 1;
  }
  for (pb = 0; prop0 >= (9 * 5); 
    pb++, prop0 -= (9 * 5));
  for (lp = 0; prop0 >= 9; 
    lp++, prop0 -= 9);
  lc = prop0;

  lzmaInternalSize = 
    (LZMA_BASE_SIZE + (LZMA_LIT_SIZE << (lc + lp)))* sizeof(CProb);
  
  //diag_printf("lc=%d lp=%d basesize=%d lzmaInternalSize=%d\n",lc,lp,LZMA_BASE_SIZE,lzmaInternalSize);
//  if (lzmaInternalSize > LZMA_MAX_SIZE) {
//     diag_printf("lzmaInternalSize=%d is greater LZMA_MAX_SIZE=%d.\n",lzmaInternalSize,LZMA_MAX_SIZE);
//     return 1;
//  }
  //diag_printf("Calling LzmaDecode with lzmaInternalData=%p\n",lzmaInternalData);
  compressedSize = inLength - 13;
  res = LzmaDecode((unsigned char *)lzmaInternalData, lzmaInternalSize,
		   lc, lp, pb,
		   buffer, compressedSize,
		   outBuffer, outSize, &outSizeProcessed);
  *outLength = outSizeProcessed;
  return res;


}
