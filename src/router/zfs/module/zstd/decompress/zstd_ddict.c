/*
 * Copyright (c) 2016-present, Yann Collet, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */

/* zstd_ddict.c :
 * concentrates all logic that needs to know the internals of ZSTD_DDict object */

/*-*******************************************************
*  Dependencies
*********************************************************/
#include <sys/zstd/cpu.h>         /* bmi2 */
#include <sys/zstd/mem.h>         /* low level memory routines */
#define FSE_STATIC_LINKING_ONLY
#include <sys/zstd/fse.h>
#define HUF_STATIC_LINKING_ONLY
#include <sys/zstd/huf.h>
#include <sys/zstd/zstd_decompress_internal.h>
#include <sys/zstd/zstd_ddict.h>

#if defined(ZSTD_LEGACY_SUPPORT) && (ZSTD_LEGACY_SUPPORT>=1)
#  include <<sys/zstd/zstd_legacy.h>
#endif



/*-*******************************************************
*  Types
*********************************************************/
struct ZSTD_DDict_s {
    void* dictBuffer;
    const void* dictContent;
    size_t dictSize;
    ZSTD_entropyDTables_t entropy;
    U32 dictID;
    U32 entropyPresent;
    ZSTD_customMem cMem;
};  /* typedef'd to ZSTD_DDict within "zstd.h" */

const void* ZSTD_DDict_dictContent(const ZSTD_DDict* ddict)
{
    assert(ddict != NULL);
    return ddict->dictContent;
}

size_t ZSTD_DDict_dictSize(const ZSTD_DDict* ddict)
{
    assert(ddict != NULL);
    return ddict->dictSize;
}

void ZSTD_copyDDictParameters(ZSTD_DCtx* dctx, const ZSTD_DDict* ddict)
{
    DEBUGLOG(4, "ZSTD_copyDDictParameters");
    assert(dctx != NULL);
    assert(ddict != NULL);
    dctx->dictID = ddict->dictID;
    dctx->prefixStart = ddict->dictContent;
    dctx->virtualStart = ddict->dictContent;
    dctx->dictEnd = (const BYTE*)ddict->dictContent + ddict->dictSize;
    dctx->previousDstEnd = dctx->dictEnd;
    if (ddict->entropyPresent) {
        dctx->litEntropy = 1;
        dctx->fseEntropy = 1;
        dctx->LLTptr = ddict->entropy.LLTable;
        dctx->MLTptr = ddict->entropy.MLTable;
        dctx->OFTptr = ddict->entropy.OFTable;
        dctx->HUFptr = ddict->entropy.hufTable;
        dctx->entropy.rep[0] = ddict->entropy.rep[0];
        dctx->entropy.rep[1] = ddict->entropy.rep[1];
        dctx->entropy.rep[2] = ddict->entropy.rep[2];
    } else {
        dctx->litEntropy = 0;
        dctx->fseEntropy = 0;
    }
}

size_t ZSTD_freeDDict(ZSTD_DDict* ddict)
{
    if (ddict==NULL) return 0;   /* support free on NULL */
    {   ZSTD_customMem const cMem = ddict->cMem;
        ZSTD_free(ddict->dictBuffer, cMem);
        ZSTD_free(ddict, cMem);
        return 0;
    }
}
