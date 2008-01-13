#ifndef __DEFLATE_ENCODER_H
#define __DEFLATE_ENCODER_H

#include "BinTree3Z.h"
#include "LSBFEncoder.h"
#include "HuffmanEncoder.h"
#include "Const.h"

namespace NDeflate {
namespace NEncoder {

struct CCodeValue
{
  BYTE Flag;
  union
  {
    BYTE Imm;
    BYTE Len;
  };
  UINT16 Pos;
};

class COnePosMatches
{
public:
  UINT16 *MatchDistances;
  UINT16 LongestMatchLength;    
  UINT16 LongestMatchDistance;
  void Init(UINT16 *aMatchDistances)
  {
    MatchDistances = aMatchDistances;
  };
};

struct COptimal
{
  UINT32 Price;
  UINT16 PosPrev;
  UINT16 BackPrev;
};

const int kNumOpts = 0x1000;

class CCoder
{
  UINT32 m_FinderPos;
  
  COptimal m_Optimum[kNumOpts];
  
  NBT3Z::CInTree m_MatchFinder;

  NStream::NLSBF::CEncoder m_OutStream;
  NStream::NLSBF::CReverseEncoder m_ReverseOutStream;
  
  NCompression::NHuffman::CEncoder m_MainCoder;
  NCompression::NHuffman::CEncoder m_DistCoder;
  NCompression::NHuffman::CEncoder m_LevelCoder;

  BYTE m_LastLevels[kMaxTableSize];

  UINT32 m_ValueIndex;
  CCodeValue *m_Values;

  UINT32 m_OptimumEndIndex;
  UINT32 m_OptimumCurrentIndex;
  UINT32 m_AdditionalOffset;

  UINT32 m_LongestMatchLength;    
  UINT32 m_LongestMatchDistance;
  UINT16 *m_MatchDistances;

  UINT32 m_NumFastBytes;
  UINT32 m_MatchLengthEdge;

  BYTE  m_LiteralPrices[256];
  
  BYTE  m_LenPrices[kNumLenCombinations];
  BYTE  m_PosPrices[kDistTableSize];

  UINT32 m_CurrentBlockUncompressedSize;

  COnePosMatches *m_OnePosMatchesArray;
  UINT16 *m_OnePosMatchesMemory;

  UINT64 m_BlockStartPostion;
  int m_NumPasses;

  bool m_Created;

  HRESULT Create();
  void Free();

  void GetBacks(UINT32 aPos);

  void ReadGoodBacks();
  void MovePos(UINT32 aNum);
  UINT32 Backward(UINT32 &aBackRes, UINT32 aCur);
  UINT32 GetOptimal(UINT32 &aBackRes);

  void InitStructures();
  void CodeLevelTable(BYTE *aNewLevels, int aNumLevels, bool aCodeMode);
  int WriteTables(bool aWriteMode, bool anFinalBlock);
  void CopyBackBlockOp(UINT32 aDistance, UINT32 aLength);
  void WriteBlockData(bool aWriteMode, bool anFinalBlock);

  HRESULT CodeReal(ISequentialInStream *anInStream, ISequentialOutStream *anOutStream, const UINT64 *anInSize);

public:
  CCoder();
  ~CCoder();

  HRESULT SetEncoderNumPasses(UINT32 A);
  HRESULT SetEncoderNumFastBytes(UINT32 A);
  HRESULT Code(ISequentialInStream *anInStream, ISequentialOutStream *anOutStream, const UINT64 *anInSize);
};

}}

#endif
