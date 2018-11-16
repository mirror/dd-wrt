/* BEGIN CSTYLED */
/*
 * Copyright (c) 2016-present, Yann Collet, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */


/* ***************************************************************
*  Tuning parameters
*****************************************************************/
/*!
 * HEAPMODE :
 * Select how default decompression function ZSTD_decompress() allocates its context,
 * on stack (0), or into heap (1, default; requires malloc()).
 * Note that functions with explicit context such as ZSTD_decompressDCtx() are unaffected.
 */
#ifndef ZSTD_HEAPMODE
#  define ZSTD_HEAPMODE 1
#endif

/*!
*  LEGACY_SUPPORT :
*  if set to 1+, ZSTD_decompress() can decode older formats (v0.1+)
*/
#ifndef ZSTD_LEGACY_SUPPORT
#  define ZSTD_LEGACY_SUPPORT 0
#endif

/*!
 *  MAXWINDOWSIZE_DEFAULT :
 *  maximum window size accepted by DStream __by default__.
 *  Frames requiring more memory will be rejected.
 *  It's possible to set a different limit using ZSTD_DCtx_setMaxWindowSize().
 */
#ifndef ZSTD_MAXWINDOWSIZE_DEFAULT
#  define ZSTD_MAXWINDOWSIZE_DEFAULT (((U32)1 << ZSTD_WINDOWLOG_DEFAULTMAX) + 1)
#endif

/*!
 *  NO_FORWARD_PROGRESS_MAX :
 *  maximum allowed nb of calls to ZSTD_decompressStream() and ZSTD_decompress_generic()
 *  without any forward progress
 *  (defined as: no byte read from input, and no byte flushed to output)
 *  before triggering an error.
 */
#ifndef ZSTD_NO_FORWARD_PROGRESS_MAX
#  define ZSTD_NO_FORWARD_PROGRESS_MAX 16
#endif


/*-*******************************************************
*  Dependencies
*********************************************************/
#include <sys/zstd/mem.h>
#include <sys/zstd/compiler.h>    /* prefetch */
#include <sys/zstd/cpu.h>         /* bmi2 */
#include <sys/zstd/mem.h>         /* low level memory routines */
#define FSE_STATIC_LINKING_ONLY
#include <sys/zstd/fse.h>
#define HUF_STATIC_LINKING_ONLY
#include <sys/zstd/huf.h>
#include <sys/zstd/zstd_internal.h>  /* blockProperties_t */
#include <sys/zstd/zstd_decompress_internal.h>   /* ZSTD_DCtx */
#include <sys/zstd/zstd_ddict.h>  /* ZSTD_DDictDictContent */
#include <sys/zstd/zstd_decompress_block.h>   /* ZSTD_decompressBlock_internal */

#if defined(ZSTD_LEGACY_SUPPORT) && (ZSTD_LEGACY_SUPPORT>=1)
#  include <sys/zstd/zstd_legacy.h>
#endif



/*-*************************************************************
*   Context management
***************************************************************/

size_t ZSTD_estimateDCtxSize(void) { return sizeof(ZSTD_DCtx); }


static size_t ZSTD_startingInputLength(ZSTD_format_e format)
{
    size_t const startingInputLength = (format==ZSTD_f_zstd1_magicless) ?
                    ZSTD_frameHeaderSize_prefix - ZSTD_FRAMEIDSIZE :
                    ZSTD_frameHeaderSize_prefix;
    ZSTD_STATIC_ASSERT(ZSTD_FRAMEHEADERSIZE_PREFIX >= ZSTD_FRAMEIDSIZE);
    /* only supports formats ZSTD_f_zstd1 and ZSTD_f_zstd1_magicless */
    assert( (format == ZSTD_f_zstd1) || (format == ZSTD_f_zstd1_magicless) );
    return startingInputLength;
}

static void ZSTD_initDCtx_internal(ZSTD_DCtx* dctx)
{
    dctx->format = ZSTD_f_zstd1;  /* ZSTD_decompressBegin() invokes ZSTD_startingInputLength() with argument dctx->format */
    dctx->staticSize  = 0;
    dctx->maxWindowSize = ZSTD_MAXWINDOWSIZE_DEFAULT;
    dctx->ddict       = NULL;
    dctx->ddictLocal  = NULL;
    dctx->dictEnd     = NULL;
    dctx->ddictIsCold = 0;
    dctx->inBuff      = NULL;
    dctx->inBuffSize  = 0;
    dctx->outBuffSize = 0;
    dctx->streamStage = zdss_init;
    dctx->legacyContext = NULL;
    dctx->previousLegacyVersion = 0;
    dctx->noForwardProgress = 0;
    dctx->bmi2 = ZSTD_cpuid_bmi2(ZSTD_cpuid());
}

ZSTD_DCtx* ZSTD_createDCtx_advanced(ZSTD_customMem customMem)
{
    if (!customMem.customAlloc ^ !customMem.customFree) return NULL;

    {   ZSTD_DCtx* const dctx = (ZSTD_DCtx*)ZSTD_malloc(sizeof(*dctx), customMem);
        if (!dctx) return NULL;
        dctx->customMem = customMem;
        ZSTD_initDCtx_internal(dctx);
        return dctx;
    }
}

size_t ZSTD_freeDCtx(ZSTD_DCtx* dctx)
{
    if (dctx==NULL) return 0;   /* support free on NULL */
    if (dctx->staticSize) return ERROR(memory_allocation);   /* not compatible with static DCtx */
    {   ZSTD_customMem const cMem = dctx->customMem;
        ZSTD_freeDDict(dctx->ddictLocal);
        dctx->ddictLocal = NULL;
        ZSTD_free(dctx->inBuff, cMem);
        dctx->inBuff = NULL;
#if defined(ZSTD_LEGACY_SUPPORT) && (ZSTD_LEGACY_SUPPORT >= 1)
        if (dctx->legacyContext)
            ZSTD_freeLegacyStreamContext(dctx->legacyContext, dctx->previousLegacyVersion);
#endif
        ZSTD_free(dctx, cMem);
        return 0;
    }
}

/*-*************************************************************
 *   Frame header decoding
 ***************************************************************/

/** ZSTD_frameHeaderSize_internal() :
 *  srcSize must be large enough to reach header size fields.
 *  note : only works for formats ZSTD_f_zstd1 and ZSTD_f_zstd1_magicless.
 * @return : size of the Frame Header
 *           or an error code, which can be tested with ZSTD_isError() */
static size_t ZSTD_frameHeaderSize_internal(const void* src, size_t srcSize, ZSTD_format_e format)
{
    size_t const minInputSize = ZSTD_startingInputLength(format);
    if (srcSize < minInputSize) return ERROR(srcSize_wrong);

    {   BYTE const fhd = ((const BYTE*)src)[minInputSize-1];
        U32 const dictID= fhd & 3;
        U32 const singleSegment = (fhd >> 5) & 1;
        U32 const fcsId = fhd >> 6;
        return minInputSize + !singleSegment
             + ZSTD_did_fieldSize[dictID] + ZSTD_fcs_fieldSize[fcsId]
             + (singleSegment && !fcsId);
    }
}

/** ZSTD_frameHeaderSize() :
 *  srcSize must be >= ZSTD_frameHeaderSize_prefix.
 * @return : size of the Frame Header,
 *           or an error code (if srcSize is too small) */
size_t ZSTD_frameHeaderSize(const void* src, size_t srcSize)
{
    return ZSTD_frameHeaderSize_internal(src, srcSize, ZSTD_f_zstd1);
}


/** ZSTD_getFrameHeader_advanced() :
 *  decode Frame Header, or require larger `srcSize`.
 *  note : only works for formats ZSTD_f_zstd1 and ZSTD_f_zstd1_magicless
 * @return : 0, `zfhPtr` is correctly filled,
 *          >0, `srcSize` is too small, value is wanted `srcSize` amount,
 *           or an error code, which can be tested using ZSTD_isError() */
size_t ZSTD_getFrameHeader_advanced(ZSTD_frameHeader* zfhPtr, const void* src, size_t srcSize, ZSTD_format_e format)
{
    const BYTE* ip = (const BYTE*)src;
    size_t const minInputSize = ZSTD_startingInputLength(format);

    memset(zfhPtr, 0, sizeof(*zfhPtr));   /* not strictly necessary, but static analyzer do not understand that zfhPtr is only going to be read only if return value is zero, since they are 2 different signals */
    if (srcSize < minInputSize) return minInputSize;
    if (src==NULL) return ERROR(GENERIC);   /* invalid parameter */

    if ( (format != ZSTD_f_zstd1_magicless)
      && (MEM_readLE32(src) != ZSTD_MAGICNUMBER) ) {
        if ((MEM_readLE32(src) & 0xFFFFFFF0U) == ZSTD_MAGIC_SKIPPABLE_START) {
            /* skippable frame */
            if (srcSize < ZSTD_skippableHeaderSize)
                return ZSTD_skippableHeaderSize; /* magic number + frame length */
            memset(zfhPtr, 0, sizeof(*zfhPtr));
            zfhPtr->frameContentSize = MEM_readLE32((const char *)src + ZSTD_FRAMEIDSIZE);
            zfhPtr->frameType = ZSTD_skippableFrame;
            return 0;
        }
        return ERROR(prefix_unknown);
    }

    /* ensure there is enough `srcSize` to fully read/decode frame header */
    {   size_t const fhsize = ZSTD_frameHeaderSize_internal(src, srcSize, format);
        if (srcSize < fhsize) return fhsize;
        zfhPtr->headerSize = (U32)fhsize;
    }

    {   BYTE const fhdByte = ip[minInputSize-1];
        size_t pos = minInputSize;
        U32 const dictIDSizeCode = fhdByte&3;
        U32 const checksumFlag = (fhdByte>>2)&1;
        U32 const singleSegment = (fhdByte>>5)&1;
        U32 const fcsID = fhdByte>>6;
        U64 windowSize = 0;
        U32 dictID = 0;
        U64 frameContentSize = ZSTD_CONTENTSIZE_UNKNOWN;
        if ((fhdByte & 0x08) != 0)
            return ERROR(frameParameter_unsupported); /* reserved bits, must be zero */

        if (!singleSegment) {
            BYTE const wlByte = ip[pos++];
            U32 const windowLog = (wlByte >> 3) + ZSTD_WINDOWLOG_ABSOLUTEMIN;
            if (windowLog > ZSTD_WINDOWLOG_MAX)
                return ERROR(frameParameter_windowTooLarge);
            windowSize = (1ULL << windowLog);
            windowSize += (windowSize >> 3) * (wlByte&7);
        }
        switch(dictIDSizeCode)
        {
            default: assert(0);  /* impossible */
            case 0 : break;
            case 1 : dictID = ip[pos]; pos++; break;
            case 2 : dictID = MEM_readLE16(ip+pos); pos+=2; break;
            case 3 : dictID = MEM_readLE32(ip+pos); pos+=4; break;
        }
        switch(fcsID)
        {
            default: assert(0);  /* impossible */
            case 0 : if (singleSegment) frameContentSize = ip[pos]; break;
            case 1 : frameContentSize = MEM_readLE16(ip+pos)+256; break;
            case 2 : frameContentSize = MEM_readLE32(ip+pos); break;
            case 3 : frameContentSize = MEM_readLE64(ip+pos); break;
        }
        if (singleSegment) windowSize = frameContentSize;

        zfhPtr->frameType = ZSTD_frame;
        zfhPtr->frameContentSize = frameContentSize;
        zfhPtr->windowSize = windowSize;
        zfhPtr->blockSizeMax = (unsigned) MIN(windowSize, ZSTD_BLOCKSIZE_MAX);
        zfhPtr->dictID = dictID;
        zfhPtr->checksumFlag = checksumFlag;
    }
    return 0;
}

/** ZSTD_decodeFrameHeader() :
 * `headerSize` must be the size provided by ZSTD_frameHeaderSize().
 * @return : 0 if success, or an error code, which can be tested using ZSTD_isError() */
static size_t ZSTD_decodeFrameHeader(ZSTD_DCtx* dctx, const void* src, size_t headerSize)
{
    size_t const result = ZSTD_getFrameHeader_advanced(&(dctx->fParams), src, headerSize, dctx->format);
    if (ZSTD_isError(result)) return result;    /* invalid header */
    if (result>0) return ERROR(srcSize_wrong);  /* headerSize too small */
    if (dctx->fParams.dictID && (dctx->dictID != dctx->fParams.dictID))
        return ERROR(dictionary_wrong);
    if (dctx->fParams.checksumFlag) XXH64_reset(&dctx->xxhState, 0);
    return 0;
}

/*-*************************************************************
 *   Frame decoding
 ***************************************************************/


void ZSTD_checkContinuity(ZSTD_DCtx* dctx, const void* dst)
{
    if (dst != dctx->previousDstEnd) {   /* not contiguous */
        dctx->dictEnd = dctx->previousDstEnd;
        dctx->virtualStart = (const char*)dst - ((const char*)(dctx->previousDstEnd) - (const char*)(dctx->prefixStart));
        dctx->prefixStart = dst;
        dctx->previousDstEnd = dst;
    }
}

static size_t ZSTD_copyRawBlock(void* dst, size_t dstCapacity,
                          const void* src, size_t srcSize)
{
    DEBUGLOG(5, "ZSTD_copyRawBlock");
    if (dst == NULL) {
        if (srcSize == 0) return 0;
        return ERROR(dstBuffer_null);
    }
    if (srcSize > dstCapacity) return ERROR(dstSize_tooSmall);
    memcpy(dst, src, srcSize);
    return srcSize;
}

static size_t ZSTD_setRleBlock(void* dst, size_t dstCapacity,
                               BYTE b,
                               size_t regenSize)
{
    if (dst == NULL) {
        if (regenSize == 0) return 0;
        return ERROR(dstBuffer_null);
    }
    if (regenSize > dstCapacity) return ERROR(dstSize_tooSmall);
    memset(dst, b, regenSize);
    return regenSize;
}


/*! ZSTD_decompressFrame() :
 * @dctx must be properly initialized
 *  will update *srcPtr and *srcSizePtr,
 *  to make *srcPtr progress by one frame. */
static size_t ZSTD_decompressFrame(ZSTD_DCtx* dctx,
                                   void* dst, size_t dstCapacity,
                             const void** srcPtr, size_t *srcSizePtr)
{
    const BYTE* ip = (const BYTE*)(*srcPtr);
    BYTE* const ostart = (BYTE* const)dst;
    BYTE* const oend = ostart + dstCapacity;
    BYTE* op = ostart;
    size_t remainingSrcSize = *srcSizePtr;

    DEBUGLOG(4, "ZSTD_decompressFrame (srcSize:%i)", (int)*srcSizePtr);

    /* check */
    if (remainingSrcSize < ZSTD_frameHeaderSize_min+ZSTD_blockHeaderSize)
        return ERROR(srcSize_wrong);

    /* Frame Header */
    {   size_t const frameHeaderSize = ZSTD_frameHeaderSize(ip, ZSTD_frameHeaderSize_prefix);
        if (ZSTD_isError(frameHeaderSize)) return frameHeaderSize;
        if (remainingSrcSize < frameHeaderSize+ZSTD_blockHeaderSize)
            return ERROR(srcSize_wrong);
        CHECK_F( ZSTD_decodeFrameHeader(dctx, ip, frameHeaderSize) );
        ip += frameHeaderSize; remainingSrcSize -= frameHeaderSize;
    }

    /* Loop on each block */
    while (1) {
        size_t decodedSize;
        blockProperties_t blockProperties;
        size_t const cBlockSize = ZSTD_getcBlockSize(ip, remainingSrcSize, &blockProperties);
        if (ZSTD_isError(cBlockSize)) return cBlockSize;

        ip += ZSTD_blockHeaderSize;
        remainingSrcSize -= ZSTD_blockHeaderSize;
        if (cBlockSize > remainingSrcSize) return ERROR(srcSize_wrong);

        switch(blockProperties.blockType)
        {
        case bt_compressed:
            decodedSize = ZSTD_decompressBlock_internal(dctx, op, oend-op, ip, cBlockSize, /* frame */ 1);
            break;
        case bt_raw :
            decodedSize = ZSTD_copyRawBlock(op, oend-op, ip, cBlockSize);
            break;
        case bt_rle :
            decodedSize = ZSTD_setRleBlock(op, oend-op, *ip, blockProperties.origSize);
            break;
        case bt_reserved :
        default:
            return ERROR(corruption_detected);
        }

        if (ZSTD_isError(decodedSize)) return decodedSize;
        if (dctx->fParams.checksumFlag)
            XXH64_update(&dctx->xxhState, op, decodedSize);
        op += decodedSize;
        ip += cBlockSize;
        remainingSrcSize -= cBlockSize;
        if (blockProperties.lastBlock) break;
    }

    if (dctx->fParams.frameContentSize != ZSTD_CONTENTSIZE_UNKNOWN) {
        if ((U64)(op-ostart) != dctx->fParams.frameContentSize) {
            return ERROR(corruption_detected);
    }   }
    if (dctx->fParams.checksumFlag) { /* Frame content checksum verification */
        U32 const checkCalc = (U32)XXH64_digest(&dctx->xxhState);
        U32 checkRead;
        if (remainingSrcSize<4) return ERROR(checksum_wrong);
        checkRead = MEM_readLE32(ip);
        if (checkRead != checkCalc) return ERROR(checksum_wrong);
        ip += 4;
        remainingSrcSize -= 4;
    }

    /* Allow caller to get size read */
    *srcPtr = ip;
    *srcSizePtr = remainingSrcSize;
    return op-ostart;
}

static size_t ZSTD_decompressMultiFrame(ZSTD_DCtx* dctx,
                                        void* dst, size_t dstCapacity,
                                  const void* src, size_t srcSize,
                                  const void* dict, size_t dictSize,
                                  const ZSTD_DDict* ddict)
{
    void* const dststart = dst;
    int moreThan1Frame = 0;

    DEBUGLOG(5, "ZSTD_decompressMultiFrame");
    assert(dict==NULL || ddict==NULL);  /* either dict or ddict set, not both */

    if (ddict) {
        dict = ZSTD_DDict_dictContent(ddict);
        dictSize = ZSTD_DDict_dictSize(ddict);
    }

    while (srcSize >= ZSTD_frameHeaderSize_prefix) {

#if defined(ZSTD_LEGACY_SUPPORT) && (ZSTD_LEGACY_SUPPORT >= 1)
        if (ZSTD_isLegacy(src, srcSize)) {
            size_t decodedSize;
            size_t const frameSize = ZSTD_findFrameCompressedSizeLegacy(src, srcSize);
            if (ZSTD_isError(frameSize)) return frameSize;
            /* legacy support is not compatible with static dctx */
            if (dctx->staticSize) return ERROR(memory_allocation);

            decodedSize = ZSTD_decompressLegacy(dst, dstCapacity, src, frameSize, dict, dictSize);
            if (ZSTD_isError(decodedSize)) return decodedSize;

            assert(decodedSize <=- dstCapacity);
            dst = (BYTE*)dst + decodedSize;
            dstCapacity -= decodedSize;

            src = (const BYTE*)src + frameSize;
            srcSize -= frameSize;

            continue;
        }
#endif

        {   U32 const magicNumber = MEM_readLE32(src);
            DEBUGLOG(4, "reading magic number %08X (expecting %08X)",
                        (U32)magicNumber, (U32)ZSTD_MAGICNUMBER);
            if ((magicNumber & 0xFFFFFFF0U) == ZSTD_MAGIC_SKIPPABLE_START) {
                size_t skippableSize;
                if (srcSize < ZSTD_skippableHeaderSize)
                    return ERROR(srcSize_wrong);
                skippableSize = MEM_readLE32((const BYTE*)src + ZSTD_FRAMEIDSIZE)
                              + ZSTD_skippableHeaderSize;
                if (srcSize < skippableSize) return ERROR(srcSize_wrong);

                src = (const BYTE *)src + skippableSize;
                srcSize -= skippableSize;
                continue;
        }   }

        if (ddict) {
            /* we were called from ZSTD_decompress_usingDDict */
            CHECK_F(ZSTD_decompressBegin_usingDDict(dctx, ddict));
        } else {
            /* this will initialize correctly with no dict if dict == NULL, so
             * use this in all cases but ddict */
            CHECK_F(ZSTD_decompressBegin_usingDict(dctx, dict, dictSize));
        }
        ZSTD_checkContinuity(dctx, dst);

        {   const size_t res = ZSTD_decompressFrame(dctx, dst, dstCapacity,
                                                    &src, &srcSize);
            if (ZSTD_isError(res)) return res;
            assert(res <= dstCapacity);
            dst = (BYTE*)dst + res;
            dstCapacity -= res;
        }
        moreThan1Frame = 1;
    }  /* while (srcSize >= ZSTD_frameHeaderSize_prefix) */

    if (srcSize) return ERROR(srcSize_wrong); /* input not entirely consumed */

    return (BYTE*)dst - (BYTE*)dststart;
}

size_t ZSTD_decompress_usingDict(ZSTD_DCtx* dctx,
                                 void* dst, size_t dstCapacity,
                           const void* src, size_t srcSize,
                           const void* dict, size_t dictSize)
{
    return ZSTD_decompressMultiFrame(dctx, dst, dstCapacity, src, srcSize, dict, dictSize, NULL);
}


size_t ZSTD_decompressDCtx(ZSTD_DCtx* dctx, void* dst, size_t dstCapacity, const void* src, size_t srcSize)
{
    return ZSTD_decompress_usingDict(dctx, dst, dstCapacity, src, srcSize, NULL, 0);
}

/*-**************************************
*   Advanced Streaming Decompression API
*   Bufferless and synchronous
****************************************/

static size_t ZSTD_refDictContent(ZSTD_DCtx* dctx, const void* dict, size_t dictSize)
{
    dctx->dictEnd = dctx->previousDstEnd;
    dctx->virtualStart = (const char*)dict - ((const char*)(dctx->previousDstEnd) - (const char*)(dctx->prefixStart));
    dctx->prefixStart = dict;
    dctx->previousDstEnd = (const char*)dict + dictSize;
    return 0;
}

/*! ZSTD_loadDEntropy() :
 *  dict : must point at beginning of a valid zstd dictionary.
 * @return : size of entropy tables read */
size_t
ZSTD_loadDEntropy(ZSTD_entropyDTables_t* entropy,
                  const void* const dict, size_t const dictSize)
{
    const BYTE* dictPtr = (const BYTE*)dict;
    const BYTE* const dictEnd = dictPtr + dictSize;

    if (dictSize <= 8) return ERROR(dictionary_corrupted);
    assert(MEM_readLE32(dict) == ZSTD_MAGIC_DICTIONARY);   /* dict must be valid */
    dictPtr += 8;   /* skip header = magic + dictID */

    ZSTD_STATIC_ASSERT(offsetof(ZSTD_entropyDTables_t, OFTable) == offsetof(ZSTD_entropyDTables_t, LLTable) + sizeof(entropy->LLTable));
    ZSTD_STATIC_ASSERT(offsetof(ZSTD_entropyDTables_t, MLTable) == offsetof(ZSTD_entropyDTables_t, OFTable) + sizeof(entropy->OFTable));
    ZSTD_STATIC_ASSERT(sizeof(entropy->LLTable) + sizeof(entropy->OFTable) + sizeof(entropy->MLTable) >= HUF_DECOMPRESS_WORKSPACE_SIZE);
    {   void* const workspace = &entropy->LLTable;   /* use fse tables as temporary workspace; implies fse tables are grouped together */
        size_t const workspaceSize = sizeof(entropy->LLTable) + sizeof(entropy->OFTable) + sizeof(entropy->MLTable);
        size_t const hSize = HUF_readDTableX2_wksp(entropy->hufTable,
                                                dictPtr, dictEnd - dictPtr,
                                                workspace, workspaceSize);
        if (HUF_isError(hSize)) return ERROR(dictionary_corrupted);
        dictPtr += hSize;
    }

    {   short offcodeNCount[MaxOff+1];
        U32 offcodeMaxValue = MaxOff, offcodeLog;
        size_t const offcodeHeaderSize = FSE_readNCount(offcodeNCount, &offcodeMaxValue, &offcodeLog, dictPtr, dictEnd-dictPtr);
        if (FSE_isError(offcodeHeaderSize)) return ERROR(dictionary_corrupted);
        if (offcodeMaxValue > MaxOff) return ERROR(dictionary_corrupted);
        if (offcodeLog > OffFSELog) return ERROR(dictionary_corrupted);
        ZSTD_buildFSETable( entropy->OFTable,
                            offcodeNCount, offcodeMaxValue,
                            OF_base, OF_bits,
                            offcodeLog);
        dictPtr += offcodeHeaderSize;
    }

    {   short matchlengthNCount[MaxML+1];
        unsigned matchlengthMaxValue = MaxML, matchlengthLog;
        size_t const matchlengthHeaderSize = FSE_readNCount(matchlengthNCount, &matchlengthMaxValue, &matchlengthLog, dictPtr, dictEnd-dictPtr);
        if (FSE_isError(matchlengthHeaderSize)) return ERROR(dictionary_corrupted);
        if (matchlengthMaxValue > MaxML) return ERROR(dictionary_corrupted);
        if (matchlengthLog > MLFSELog) return ERROR(dictionary_corrupted);
        ZSTD_buildFSETable( entropy->MLTable,
                            matchlengthNCount, matchlengthMaxValue,
                            ML_base, ML_bits,
                            matchlengthLog);
        dictPtr += matchlengthHeaderSize;
    }

    {   short litlengthNCount[MaxLL+1];
        unsigned litlengthMaxValue = MaxLL, litlengthLog;
        size_t const litlengthHeaderSize = FSE_readNCount(litlengthNCount, &litlengthMaxValue, &litlengthLog, dictPtr, dictEnd-dictPtr);
        if (FSE_isError(litlengthHeaderSize)) return ERROR(dictionary_corrupted);
        if (litlengthMaxValue > MaxLL) return ERROR(dictionary_corrupted);
        if (litlengthLog > LLFSELog) return ERROR(dictionary_corrupted);
        ZSTD_buildFSETable( entropy->LLTable,
                            litlengthNCount, litlengthMaxValue,
                            LL_base, LL_bits,
                            litlengthLog);
        dictPtr += litlengthHeaderSize;
    }

    if (dictPtr+12 > dictEnd) return ERROR(dictionary_corrupted);
    {   int i;
        size_t const dictContentSize = (size_t)(dictEnd - (dictPtr+12));
        for (i=0; i<3; i++) {
            U32 const rep = MEM_readLE32(dictPtr); dictPtr += 4;
            if (rep==0 || rep >= dictContentSize) return ERROR(dictionary_corrupted);
            entropy->rep[i] = rep;
    }   }

    return dictPtr - (const BYTE*)dict;
}

static size_t ZSTD_decompress_insertDictionary(ZSTD_DCtx* dctx, const void* dict, size_t dictSize)
{
    if (dictSize < 8) return ZSTD_refDictContent(dctx, dict, dictSize);
    {   U32 const magic = MEM_readLE32(dict);
        if (magic != ZSTD_MAGIC_DICTIONARY) {
            return ZSTD_refDictContent(dctx, dict, dictSize);   /* pure content mode */
    }   }
    dctx->dictID = MEM_readLE32((const char*)dict + ZSTD_FRAMEIDSIZE);

    /* load entropy tables */
    {   size_t const eSize = ZSTD_loadDEntropy(&dctx->entropy, dict, dictSize);
        if (ZSTD_isError(eSize)) return ERROR(dictionary_corrupted);
        dict = (const char*)dict + eSize;
        dictSize -= eSize;
    }
    dctx->litEntropy = dctx->fseEntropy = 1;

    /* reference dictionary content */
    return ZSTD_refDictContent(dctx, dict, dictSize);
}

size_t ZSTD_decompressBegin(ZSTD_DCtx* dctx)
{
    assert(dctx != NULL);
    dctx->expected = ZSTD_startingInputLength(dctx->format);  /* dctx->format must be properly set */
    dctx->stage = ZSTDds_getFrameHeaderSize;
    dctx->decodedSize = 0;
    dctx->previousDstEnd = NULL;
    dctx->prefixStart = NULL;
    dctx->virtualStart = NULL;
    dctx->dictEnd = NULL;
    dctx->entropy.hufTable[0] = (HUF_DTable)((HufLog)*0x1000001);  /* cover both little and big endian */
    dctx->litEntropy = dctx->fseEntropy = 0;
    dctx->dictID = 0;
    ZSTD_STATIC_ASSERT(sizeof(dctx->entropy.rep) == sizeof(repStartValue));
    memcpy(dctx->entropy.rep, repStartValue, sizeof(repStartValue));  /* initial repcodes */
    dctx->LLTptr = dctx->entropy.LLTable;
    dctx->MLTptr = dctx->entropy.MLTable;
    dctx->OFTptr = dctx->entropy.OFTable;
    dctx->HUFptr = dctx->entropy.hufTable;
    return 0;
}

size_t ZSTD_decompressBegin_usingDict(ZSTD_DCtx* dctx, const void* dict, size_t dictSize)
{
    CHECK_F( ZSTD_decompressBegin(dctx) );
    if (dict && dictSize)
        CHECK_E(ZSTD_decompress_insertDictionary(dctx, dict, dictSize), dictionary_corrupted);
    return 0;
}


/* ======   ZSTD_DDict   ====== */

size_t ZSTD_decompressBegin_usingDDict(ZSTD_DCtx* dctx, const ZSTD_DDict* ddict)
{
    DEBUGLOG(4, "ZSTD_decompressBegin_usingDDict");
    assert(dctx != NULL);
    if (ddict) {
        const char* const dictStart = (const char*)ZSTD_DDict_dictContent(ddict);
        size_t const dictSize = ZSTD_DDict_dictSize(ddict);
        const void* const dictEnd = dictStart + dictSize;
        dctx->ddictIsCold = (dctx->dictEnd != dictEnd);
        DEBUGLOG(4, "DDict is %s",
                    dctx->ddictIsCold ? "~cold~" : "hot!");
    }
    CHECK_F( ZSTD_decompressBegin(dctx) );
    if (ddict) {   /* NULL ddict is equivalent to no dictionary */
        ZSTD_copyDDictParameters(dctx, ddict);
    }
    return 0;
}

/* END CSTYLED */
