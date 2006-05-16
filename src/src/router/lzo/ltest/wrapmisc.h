/* wrapmisc.h -- misc wrapper functions for the test driver

   This file is part of the LZO data compression library.

   Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
 */


/*************************************************************************
// compression levels of zlib
**************************************************************************/

#if defined(ALG_ZLIB)

#if (ULONG_MAX > 0xffffffffL)       /* 64 bits or more */
#define ZLIB_MEM_COMPRESS       600000L
#define ZLIB_MEM_DECOMPRESS     120000L
#else
#define ZLIB_MEM_COMPRESS       300000L
#define ZLIB_MEM_DECOMPRESS      60000L
#endif


#ifndef USE_MALLOC
static m_bytep zlib_heap_ptr = NULL;
static m_uint32 zlib_heap_used = 0;
static m_uint32 zlib_heap_size = 0;

static
voidpf zlib_zalloc ( voidpf opaque, unsigned items, unsigned size )
{
    m_uint32 bytes = (m_uint32) items * size;
    voidpf ptr = (voidpf) zlib_heap_ptr;

    bytes = (bytes + 15u) & ~15u;
    if (zlib_heap_used + bytes > zlib_heap_size)
        return 0;

    zlib_heap_ptr  += bytes;
    zlib_heap_used += bytes;
    opaque = 0;
    return ptr;
}

static
void zlib_zfree ( voidpf opaque, voidpf ptr )
{
    opaque = 0;
    ptr = 0;
}
#endif

static
void zlib_alloc_init ( z_stream *strm, m_voidp wrkmem, m_uint32 s )
{
#ifndef USE_MALLOC
    zlib_heap_ptr  = (m_bytep) wrkmem;
    zlib_heap_size = s;
    zlib_heap_used = 0;

    strm->zalloc = (alloc_func) zlib_zalloc;
    /*strm->zfree = (free_func) zlib_zfree;*/
    strm->zfree = zlib_zfree;
#else
    strm->zalloc = (alloc_func) 0;
    strm->zfree = (free_func) 0;
#endif
}


int zlib_compress       ( const m_bytep src, m_uint  src_len,
                                m_bytep dst, m_uintp dst_len,
                                m_voidp wrkmem,
                                int c, int level )
{
    /* use the undocumented feature to suppress the zlib header */
    z_stream stream;
    int err = Z_OK;
    int flush = Z_FINISH;
    int windowBits = opt_dict ? MAX_WBITS : -(MAX_WBITS);

#if 0
    stream.next_in = (Bytef *) src;         /* UNCONST */
#else
    {
        union { const m_bytep cp; m_bytep p; } u;
        u.cp = src;
        stream.next_in = (Bytef *) u.p;     /* UNCONST */
    }
#endif
    stream.avail_in = src_len;
    stream.next_out = (Bytef *) dst;
    stream.avail_out = *dst_len;
    *dst_len = 0;

    zlib_alloc_init(&stream,wrkmem,ZLIB_MEM_COMPRESS);

#if 0
    err = deflateInit(&stream, level);
#else
    err = deflateInit2(&stream, level, c, windowBits,
                       MAX_MEM_LEVEL > 8 ? 8 : MAX_MEM_LEVEL,
                       Z_DEFAULT_STRATEGY);
#endif
    if (err == Z_OK && opt_dict && dict)
        err = deflateSetDictionary(&stream,dict,dict_len);
    if (err == Z_OK)
    {

        err = deflate(&stream, flush);
        if (err != Z_STREAM_END)
        {
            deflateEnd(&stream);
            err = (err == Z_OK) ? Z_BUF_ERROR : err;
        }
        else
        {
            *dst_len = (m_uint) stream.total_out;
            err = deflateEnd(&stream);
        }
    }
    windowBits = windowBits;
    return err;
}


M_PRIVATE(int)
zlib_decompress         ( const m_bytep src, m_uint  src_len,
                                m_bytep dst, m_uintp dst_len,
                                m_voidp wrkmem )
{
    /* use the undocumented feature to suppress the zlib header */
    z_stream stream;
    int err = Z_OK;
    int flush = Z_FINISH;
    int windowBits = opt_dict ? MAX_WBITS : -(MAX_WBITS);

#if 0
    stream.next_in = (Bytef *) src;         /* UNCONST */
#else
    {
        union { const m_bytep cp; m_bytep p; } u;
        u.cp = src;
        stream.next_in = (Bytef *) u.p;     /* UNCONST */
    }
#endif
    stream.avail_in = src_len;
    stream.next_out = (Bytef *) dst;
    stream.avail_out = *dst_len;
    *dst_len = 0;

    zlib_alloc_init(&stream,wrkmem,ZLIB_MEM_DECOMPRESS);

#if 0
    err = inflateInit(&stream);
#else
    if (windowBits < 0)
        stream.avail_in++;  /* inflate requires an extra "dummy" byte */
    err = inflateInit2(&stream, windowBits);
#endif
    while (err == Z_OK)
    {
        err = inflate(&stream, flush);
        if (flush == Z_FINISH && err == Z_OK)
            err = Z_BUF_ERROR;
        if (err == Z_STREAM_END)
        {
            *dst_len = (m_uint) stream.total_out;
            err = inflateEnd(&stream);
            break;
        }
        else if (err == Z_NEED_DICT && opt_dict && dict)
            err = inflateSetDictionary(&stream,dict,dict_len);
        else if (err != Z_OK)
        {
            (void) inflateEnd(&stream);
            break;
        }
    }
    windowBits = windowBits;
    return err;
}


M_PRIVATE(int)
zlib_8_1_compress       ( const m_bytep src, m_uint  src_len,
                                m_bytep dst, m_uintp dst_len,
                                m_voidp wrkmem )
{ return zlib_compress(src,src_len,dst,dst_len,wrkmem,Z_DEFLATED,1); }

M_PRIVATE(int)
zlib_8_2_compress       ( const m_bytep src, m_uint  src_len,
                                m_bytep dst, m_uintp dst_len,
                                m_voidp wrkmem )
{ return zlib_compress(src,src_len,dst,dst_len,wrkmem,Z_DEFLATED,2); }

M_PRIVATE(int)
zlib_8_3_compress       ( const m_bytep src, m_uint  src_len,
                                m_bytep dst, m_uintp dst_len,
                                m_voidp wrkmem )
{ return zlib_compress(src,src_len,dst,dst_len,wrkmem,Z_DEFLATED,3); }

M_PRIVATE(int)
zlib_8_4_compress       ( const m_bytep src, m_uint  src_len,
                                m_bytep dst, m_uintp dst_len,
                                m_voidp wrkmem )
{ return zlib_compress(src,src_len,dst,dst_len,wrkmem,Z_DEFLATED,4); }

M_PRIVATE(int)
zlib_8_5_compress       ( const m_bytep src, m_uint  src_len,
                                m_bytep dst, m_uintp dst_len,
                                m_voidp wrkmem )
{ return zlib_compress(src,src_len,dst,dst_len,wrkmem,Z_DEFLATED,5); }

M_PRIVATE(int)
zlib_8_6_compress       ( const m_bytep src, m_uint  src_len,
                                m_bytep dst, m_uintp dst_len,
                                m_voidp wrkmem )
{ return zlib_compress(src,src_len,dst,dst_len,wrkmem,Z_DEFLATED,6); }

M_PRIVATE(int)
zlib_8_7_compress       ( const m_bytep src, m_uint  src_len,
                                m_bytep dst, m_uintp dst_len,
                                m_voidp wrkmem )
{ return zlib_compress(src,src_len,dst,dst_len,wrkmem,Z_DEFLATED,7); }

M_PRIVATE(int)
zlib_8_8_compress       ( const m_bytep src, m_uint  src_len,
                                m_bytep dst, m_uintp dst_len,
                                m_voidp wrkmem )
{ return zlib_compress(src,src_len,dst,dst_len,wrkmem,Z_DEFLATED,8); }

M_PRIVATE(int)
zlib_8_9_compress       ( const m_bytep src, m_uint  src_len,
                                m_bytep dst, m_uintp dst_len,
                                m_voidp wrkmem )
{ return zlib_compress(src,src_len,dst,dst_len,wrkmem,Z_DEFLATED,9); }


#endif /* ALG_ZLIB */


/*************************************************************************
// compression levels of bzip2
**************************************************************************/

#if defined(ALG_BZIP2)

#endif /* ALG_BZIP2 */


/*************************************************************************
// other wrappers (pseudo compressors)
**************************************************************************/

#if defined(ALG_ZLIB)

M_PRIVATE(int)
zlib_adler32_x_compress ( const m_bytep src, m_uint  src_len,
                                m_bytep dst, m_uintp dst_len,
                                m_voidp wrkmem )
{
    uLong adler;
    adler = adler32(0L, Z_NULL, 0);
    adler = adler32(adler, dst, src_len);
    *dst_len = src_len;
    if (src) src = 0;       /* avoid warning */
    if (wrkmem) wrkmem = 0; /* avoid warning */
    return 0;
}


M_PRIVATE(int)
zlib_crc32_x_compress   ( const m_bytep src, m_uint  src_len,
                                m_bytep dst, m_uintp dst_len,
                                m_voidp wrkmem )
{
    uLong crc;
    crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, dst, src_len);
    *dst_len = src_len;
    if (src) src = 0;       /* avoid warning */
    if (wrkmem) wrkmem = 0; /* avoid warning */
    return 0;
}

#endif /* ALG_ZLIB */


/*
vi:ts=4:et
*/

