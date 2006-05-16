#ifndef __ARCHIVE_ZIP_DEFLATEDECODER_H
#define __ARCHIVE_ZIP_DEFLATEDECODER_H

#include "WindowOut.h"
#include "LSBFDecoder.h"
#include "InByte.h"
#include "HuffmanDecoder.h"
#include "Const.h"

namespace NDeflate{
namespace NDecoder{

typedef NStream::NLSBF::CDecoder<NStream::CInByte> CInBit;
typedef NCompression::NHuffman::CDecoder<kNumHuffmanBits> CHuffmanDecoder;

class CCoder
{
  NStream::NWindow::COut m_OutWindowStream;
  CInBit m_InBitStream;
  CHuffmanDecoder m_MainDecoder;
  CHuffmanDecoder m_DistDecoder;
  CHuffmanDecoder m_LevelDecoder; // table for decoding other tables;

  bool m_FinalBlock;
  bool m_StoredMode;
  UINT32 m_StoredBlockSize;

  void DeCodeLevelTable(BYTE *aNewLevels, int aNumLevels);
  void ReadTables();

  HRESULT CodeReal(ISequentialInStream *anInStream, ISequentialOutStream *anOutStream, const UINT64 *anInSize, const UINT64 *anOutSize);
  
public:
  CCoder();

  HRESULT Code(ISequentialInStream *anInStream, ISequentialOutStream *anOutStream, const UINT64 *anInSize, const UINT64 *anOutSize);
};

}}

#endif
