/* -*- mode: C; mode: fold; -*- */
/*
Copyright (C) 2008-2011 John E. Davis

This file is part of the S-Lang Library.

The S-Lang Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The S-Lang Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.
*/

#include "config.h"

#include <stdio.h>
#include <errno.h>
#include <slang.h>

#include <string.h>
#include <zlib.h>

SLANG_MODULE(zlib);

static char *Module_Version_String = "0.1.0";
#define MODULE_VERSION_NUMBER  (0*10000 + 1*100 + 0)

typedef struct
{
   int type;
#define DEFLATE_TYPE 1
#define INFLATE_TYPE 2

   int initialized;
   z_stream zs;

#define DEFAULT_START_BUFLEN 0x4000
   unsigned int start_buflen;
#define DEFAULT_BUFLEN_INC 0x4000
   unsigned int dbuflen;
   int windowbits;
}
ZLib_Type;

static int ZLib_Type_Id = -1;
int ZLib_Error = -1;

static int check_zerror (int e)
{
   switch (e)
     {
      case Z_ERRNO:
	e = errno;
	(void) SLerrno_set_errno (e);
	SLang_verror (ZLib_Error, "Z library errno error: %s", SLerrno_strerror(e));
	break;

      case Z_STREAM_ERROR:
	SLang_verror (ZLib_Error, "Z library stream error");
	break;

      case Z_DATA_ERROR:
	SLang_verror (ZLib_Error, "Z library data error");
	break;

      case Z_MEM_ERROR:
	SLang_verror (SL_Malloc_Error, "Z library memory allocation error");
	break;

      case Z_BUF_ERROR:
	SLang_verror (ZLib_Error, "Z library buffer error");
	break;

      case Z_VERSION_ERROR:
	SLang_verror (ZLib_Error, "Z library version mismatch error");
	break;

      case Z_NEED_DICT:		       /* not handled by this module */
	SLang_verror (ZLib_Error, "Z library dictionary error");
	break;

      default:
	if (e >= 0)
	  return 0;

	SLang_verror (ZLib_Error, "Unknown Error Code");
     }
   return -1;
}

static int check_deflate_object (ZLib_Type *zp)
{
   if (zp->type != DEFLATE_TYPE)
     {
	SLang_verror (SL_TypeMismatch_Error, "Expecting a Zlib_Type deflate object");
	return -1;
     }
   return 0;
}

static int check_inflate_object (ZLib_Type *zp)
{
   if (zp->type != INFLATE_TYPE)
     {
	SLang_verror (SL_TypeMismatch_Error, "Expecting a Zlib_Type inflate object");
	return -1;
     }
   return 0;
}

static int init_deflate_object (ZLib_Type *z,
				int level, int method, int windowbits,
				int memlevel, int strategy)
{
   z_stream *zs;
   int ret;

   memset ((char *) z, 0, sizeof(ZLib_Type));
   z->type = DEFLATE_TYPE;
   z->start_buflen = DEFAULT_START_BUFLEN;
   z->dbuflen = DEFAULT_BUFLEN_INC;

   zs = &z->zs;

   zs->zalloc = Z_NULL;
   zs->zfree = Z_NULL;
   zs->opaque = Z_NULL;

   ret = deflateInit2 (zs, level, method, windowbits, memlevel, strategy);
   if (ret == Z_STREAM_ERROR)
     {
	SLang_verror (ZLib_Error, "One of more deflate parameters are invalid.");
	deflateEnd (zs);
     }

   if (-1 == check_zerror (ret))
     {
	deflateEnd (zs);
	return -1;
     }

   z->initialized = 1;
   return 0;
}

static int run_deflate (ZLib_Type *z, int flush,
			unsigned char *str, unsigned int len,
			unsigned char **bufp, unsigned int *totalp)
{
   z_stream *zs;
   unsigned char *buf;
   unsigned int buflen;

   zs = &z->zs;

   buflen = z->start_buflen;
   if (NULL == (buf = (unsigned char *) SLmalloc (buflen+1)))
     {
	*bufp = NULL;
	*totalp = 0;
	return -1;
     }

   zs->next_in = str;
   zs->avail_in = len;
   zs->next_out = buf;
   zs->avail_out = buflen;

   while (1)
     {
	unsigned int total;
	int ret;

	ret = deflate (zs, flush);

	if (ret != Z_BUF_ERROR)
	  {
	     if (-1 == check_zerror (ret))
	       goto return_error;
	  }

	total = buflen - zs->avail_out;

	if (/* done -- flush == Z_FINISH */
	    (ret == Z_STREAM_END)
	    /* Done with current input */
	    || ((zs->avail_in == 0) && (zs->avail_out != 0))
	   )
	  {
	     if (total != buflen)
	       {
		  unsigned char *new_buf;

		  new_buf = (unsigned char *) SLrealloc ((char *)buf, total+1);
		  if (new_buf == NULL)
		    goto return_error;

		  buf = new_buf;
	       }
	     buf[total] = 0;	       /* ok, because of +1 in malloc calls */
	     *bufp = buf;
	     *totalp = total;
	     return 0;
	  }

	if (zs->avail_out == 0)
	  {
	     unsigned char *new_buf;
	     unsigned int dbuflen = z->dbuflen;
	     buflen += dbuflen;
	     new_buf = (unsigned char *)SLrealloc ((char *) buf, buflen+1);
	     if (new_buf == NULL)
	       goto return_error;

	     buf = new_buf;
	     zs->avail_out = dbuflen;
	     zs->next_out = buf + total;
	  }
     }

return_error:
   SLfree ((char *) buf);
   *bufp = NULL;
   *totalp = 0;
   return -1;
}

static void deflate_intrin (ZLib_Type *zp, SLang_BString_Type *inbstr, int *flush)
{
   SLang_BString_Type *outbstr;
   unsigned char *inbuf, *outbuf;
   unsigned int inlen, outlen;

   if (-1 == check_deflate_object (zp))
     return;

   if (NULL == (inbuf = SLbstring_get_pointer (inbstr, &inlen)))
     return;

   if (zp->start_buflen < inlen)
     zp->start_buflen = inlen;

   if (-1 == run_deflate (zp, *flush, inbuf, inlen, &outbuf, &outlen))
     return;

   if (NULL == (outbstr = SLbstring_create_malloced (outbuf, outlen, 1)))
     return;

   (void) SLang_push_bstring (outbstr);
   SLbstring_free (outbstr);
}

static void deflate_reset_intrin (ZLib_Type *z)
{
   if (-1 == check_deflate_object (z))
     return;

   check_zerror (deflateReset (&z->zs));
}

static void deflate_flush_intrin (ZLib_Type *z, int *flush)
{
   unsigned char *buf;
   unsigned int len;
   SLang_BString_Type *bstr;

   if (-1 == check_deflate_object (z))
     return;

   if (-1 == run_deflate (z, *flush, (unsigned char *)"", 0, &buf, &len))
     return;

   if (NULL == (bstr = SLbstring_create_malloced (buf, len, 1)))
     return;

   (void) SLang_push_bstring (bstr);
   SLbstring_free (bstr);
}

static void free_deflate_object (ZLib_Type *z)
{
   if (z->initialized)
     deflateEnd (&z->zs);
   SLfree ((char *) z);
}

static void deflate_new_intrin (int *level, int *method, int *wbits,
				int *memlevel, int *strategy)
{
   /* According to the docs, the parameters are checked by deflateInit2 */
   ZLib_Type *z;
   SLang_MMT_Type *mmt;

   if (NULL == (z = (ZLib_Type *) SLmalloc (sizeof (ZLib_Type))))
     return;

   if (-1 == init_deflate_object (z, *level, *method, *wbits, *memlevel, *strategy))
     {
	SLfree ((char *) z);
	return;
     }

   if (NULL == (mmt = SLang_create_mmt (ZLib_Type_Id, (VOID_STAR) z)))
     {
	free_deflate_object (z);
	return;
     }

   if (0 == SLang_push_mmt (mmt))
     return;

   SLang_free_mmt (mmt);
}

static int run_inflate (ZLib_Type *z, int flush,
			unsigned char *str, unsigned int len,
			unsigned char **bufp, unsigned int *totalp)
{
   z_stream *zs;
   unsigned char *buf;
   unsigned int buflen;

   zs = &z->zs;
   zs->next_in = str;
   zs->avail_in = len;

   if (z->initialized == 0)
     {
	zs = &z->zs;
	zs->zalloc = Z_NULL;
	zs->zfree = Z_NULL;
	zs->opaque = Z_NULL;

	if (-1 == check_zerror (inflateInit2 (zs, z->windowbits)))
	  {
	     inflateEnd (zs);
	     return -1;
	  }
	z->initialized = 1;
     }

   buflen = z->start_buflen;
   if (NULL == (buf = (unsigned char *) SLmalloc (buflen+1)))
     {
	*bufp = NULL;
	*totalp = 0;
	return -1;
     }
   zs->next_out = buf;
   zs->avail_out = buflen;

   while (1)
     {
	unsigned int total;
	int ret;

	ret = inflate (zs, flush);

	if (ret != Z_BUF_ERROR)
	  {
	     if (-1 == check_zerror (ret))
	       goto return_error;
	  }

	total = buflen - zs->avail_out;

	if (/* done -- flush == Z_FINISH */
	    (ret == Z_STREAM_END)
	    /* Done with current input */
	    || ((zs->avail_in == 0) && (zs->avail_out != 0))
	   )
	  {
	     if (total != buflen)
	       {
		  unsigned char *new_buf;

		  new_buf = (unsigned char *) SLrealloc ((char *)buf, total+1);
		  if (new_buf == NULL)
		    goto return_error;

		  buf = new_buf;
	       }
	     buf[total] = 0;	       /* ok, because of +1 in malloc calls */
	     *bufp = buf;
	     *totalp = total;
	     return 0;
	  }

	if (zs->avail_out == 0)
	  {
	     unsigned char *new_buf;
	     unsigned int dbuflen = z->dbuflen;
	     buflen += dbuflen;
	     new_buf = (unsigned char *)SLrealloc ((char *) buf, buflen+1);
	     if (new_buf == NULL)
	       goto return_error;

	     buf = new_buf;
	     zs->avail_out = dbuflen;
	     zs->next_out = buf + total;
	  }
     }

return_error:
   SLfree ((char *) buf);
   *bufp = NULL;
   *totalp = 0;
   return -1;
}

static void inflate_intrin (ZLib_Type *zp, SLang_BString_Type *inbstr, int *flush)
{
   SLang_BString_Type *outbstr;
   unsigned char *inbuf, *outbuf;
   unsigned int inlen, outlen;

   if (-1 == check_inflate_object (zp))
     return;

   if (NULL == (inbuf = SLbstring_get_pointer (inbstr, &inlen)))
     return;

   if (zp->start_buflen < inlen)
     zp->start_buflen = inlen;

   if (-1 == run_inflate (zp, *flush, inbuf, inlen, &outbuf, &outlen))
     return;

   if (NULL == (outbstr = SLbstring_create_malloced (outbuf, outlen, 1)))
     return;

   (void) SLang_push_bstring (outbstr);
   SLbstring_free (outbstr);
}

static int init_inflate_object (ZLib_Type *z, int windowbits)
{
   memset ((char *) z, 0, sizeof(ZLib_Type));
   z->type = INFLATE_TYPE;
   z->windowbits = windowbits;
   z->initialized = 0;
   z->start_buflen = DEFAULT_START_BUFLEN;
   z->dbuflen = DEFAULT_BUFLEN_INC;
   return 0;
}

static void inflate_new_intrin (int *windowbits)
{
   ZLib_Type *z;
   SLang_MMT_Type *mmt;

   if (NULL == (z = (ZLib_Type *) SLmalloc (sizeof (ZLib_Type))))
     return;

   if (-1 == init_inflate_object (z, *windowbits))
     {
	SLfree ((char *) z);
	return;
     }

   if (NULL == (mmt = SLang_create_mmt (ZLib_Type_Id, (VOID_STAR) z)))
     {
	free_deflate_object (z);
	return;
     }

   if (0 == SLang_push_mmt (mmt))
     return;

   SLang_free_mmt (mmt);
}

static void inflate_reset_intrin (ZLib_Type *z)
{
   if (-1 == check_inflate_object (z))
     return;

   if (z->initialized)
     check_zerror (deflateReset (&z->zs));
}

static void inflate_flush_intrin (ZLib_Type *z, int *flush)
{
   unsigned char *buf;
   unsigned int len;
   SLang_BString_Type *bstr;

   if (-1 == check_inflate_object (z))
     return;

   if (-1 == run_inflate (z, *flush, (unsigned char *)"", 0, &buf, &len))
     return;

   if (NULL == (bstr = SLbstring_create_malloced (buf, len, 1)))
     return;

   (void) SLang_push_bstring (bstr);
   SLbstring_free (bstr);
}

static void free_inflate_object (ZLib_Type *z)
{
   if (z->initialized)
     inflateEnd (&z->zs);
   SLfree ((char *) z);
}

static void destroy_zlib_type (SLtype type, VOID_STAR f)
{
   ZLib_Type *z;
   (void) type;

   z = (ZLib_Type *) f;
   if (z->type == DEFLATE_TYPE)
     free_deflate_object (z);
   else
     free_inflate_object (z);
}

static const char *zlib_version_intrin (void)
{
   return zlibVersion ();
}

#define DUMMY_ZLIB_TYPE ((SLtype)-1)
#define I SLANG_INT_TYPE
#define B SLANG_BSTRING_TYPE
static SLang_Intrin_Fun_Type Module_Intrinsics [] =
{
   MAKE_INTRINSIC_0("zlib_version", zlib_version_intrin, SLANG_STRING_TYPE),
   MAKE_INTRINSIC_5("_zlib_deflate_new", deflate_new_intrin, SLANG_VOID_TYPE, I, I, I, I, I),
   MAKE_INTRINSIC_3("_zlib_deflate", deflate_intrin, SLANG_VOID_TYPE, DUMMY_ZLIB_TYPE, B, I),
   MAKE_INTRINSIC_2("_zlib_deflate_flush", deflate_flush_intrin, SLANG_VOID_TYPE, DUMMY_ZLIB_TYPE, I),
   MAKE_INTRINSIC_1("_zlib_deflate_reset", deflate_reset_intrin, SLANG_VOID_TYPE, DUMMY_ZLIB_TYPE),

   MAKE_INTRINSIC_1("_zlib_inflate_new", inflate_new_intrin, SLANG_VOID_TYPE, I),
   MAKE_INTRINSIC_3("_zlib_inflate", inflate_intrin, SLANG_VOID_TYPE, DUMMY_ZLIB_TYPE, B, I),
   MAKE_INTRINSIC_2("_zlib_inflate_flush", inflate_flush_intrin, SLANG_VOID_TYPE, DUMMY_ZLIB_TYPE, I),
   MAKE_INTRINSIC_1("_zlib_inflate_reset", inflate_reset_intrin, SLANG_VOID_TYPE, DUMMY_ZLIB_TYPE),
   SLANG_END_INTRIN_FUN_TABLE
};
#undef I

static SLang_Intrin_Var_Type Module_Variables [] =
{
   MAKE_VARIABLE("_zlib_module_version_string", &Module_Version_String, SLANG_STRING_TYPE, 1),
   SLANG_END_INTRIN_VAR_TABLE
};

static SLang_IConstant_Type Module_IConstants [] =
{
   MAKE_ICONSTANT("_zlib_module_version", MODULE_VERSION_NUMBER),

   MAKE_ICONSTANT("ZLIB_NO_COMPRESSION", Z_NO_COMPRESSION),
   MAKE_ICONSTANT("ZLIB_BEST_SPEED", Z_BEST_SPEED),
   MAKE_ICONSTANT("ZLIB_BEST_COMPRESSION", Z_BEST_COMPRESSION),
   MAKE_ICONSTANT("ZLIB_DEFAULT_COMPRESSION", Z_DEFAULT_COMPRESSION),

   MAKE_ICONSTANT("ZLIB_NO_FLUSH", Z_NO_FLUSH),
   MAKE_ICONSTANT("ZLIB_SYNC_FLUSH", Z_SYNC_FLUSH),
   MAKE_ICONSTANT("ZLIB_FULL_FLUSH", Z_FULL_FLUSH),
   MAKE_ICONSTANT("ZLIB_FINISH", Z_FINISH),

   MAKE_ICONSTANT("ZLIB_FILTERED", Z_FILTERED),
   MAKE_ICONSTANT("ZLIB_HUFFMAN_ONLY", Z_HUFFMAN_ONLY),
#ifdef Z_RLE
   MAKE_ICONSTANT("ZLIB_RLE", Z_RLE),
#endif
#ifdef Z_FIXED
   MAKE_ICONSTANT("ZLIB_FIXED", Z_FIXED),
#endif
   MAKE_ICONSTANT("ZLIB_DEFAULT_STRATEGY", Z_DEFAULT_STRATEGY),

   MAKE_ICONSTANT("ZLIB_DEFLATED", Z_DEFLATED),
   SLANG_END_ICONST_TABLE
};

static int register_classes (void)
{
   SLang_Class_Type *cl;

   if (ZLib_Type_Id != -1)
     return 0;

   if (NULL == (cl = SLclass_allocate_class ("ZLib_Type")))
     return -1;

   (void) SLclass_set_destroy_function (cl, destroy_zlib_type);

   if (-1 == SLclass_register_class (cl, SLANG_VOID_TYPE,
				     sizeof (ZLib_Type),
				     SLANG_CLASS_TYPE_MMT))
     return -1;

   ZLib_Type_Id = SLclass_get_class_id (cl);
   if (-1 == SLclass_patch_intrin_fun_table1 (Module_Intrinsics, DUMMY_ZLIB_TYPE, ZLib_Type_Id))
     return -1;

   return 0;
}

int init_zlib_module_ns (char *ns_name)
{
   SLang_NameSpace_Type *ns = SLns_create_namespace (ns_name);
   if (ns == NULL)
     return -1;

   if (-1 == register_classes ())
     return -1;

   if ((-1 == ZLib_Error)
       && (-1 == (ZLib_Error = SLerr_new_exception (SL_RunTime_Error, "ZLibError", "ZLib Error"))))
     return -1;

   if (
       (-1 == SLns_add_intrin_var_table (ns, Module_Variables, NULL))
       || (-1 == SLns_add_intrin_fun_table (ns, Module_Intrinsics, NULL))
       || (-1 == SLns_add_iconstant_table (ns, Module_IConstants, NULL))
       )
     return -1;

   return 0;
}

/* This function is optional */
void deinit_zlib_module (void)
{
}
