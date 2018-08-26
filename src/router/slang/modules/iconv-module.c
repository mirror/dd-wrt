/* This code was written by Dino Leonardo Sangoi <SANGOID@lloydadriatico.it> */

#include "config.h"

#include <stdio.h>
#include <slang.h>
#include <string.h>
#include <iconv.h>
#include <errno.h>

SLANG_MODULE(iconv);

#ifndef ICONV_CONST
# define ICONV_CONST
#endif

static int ICONV_Type_Id = 0;

typedef struct
{
   iconv_t cd;
}
ICONV_Type;

static void free_iconv_type (ICONV_Type *it)
{
   SLfree ((char *) it);
}

static SLang_MMT_Type *allocate_iconv_type (iconv_t cd)
{
   ICONV_Type *it;
   SLang_MMT_Type *mmt;

   it = (ICONV_Type *) SLmalloc (sizeof (ICONV_Type));
   if (it == NULL)
     return NULL;

   it->cd = cd;

   if (NULL == (mmt = SLang_create_mmt (ICONV_Type_Id, (VOID_STAR) it)))
     {
	free_iconv_type (it);
	return NULL;
     }
   return mmt;
}

static void _iconv_open(char *tocode, char *fromcode)
{
   iconv_t cd;
   SLang_MMT_Type *mmt;

   cd = iconv_open(tocode, fromcode);
   if (cd == (iconv_t)(-1))
     {
	SLang_verror (SL_INTRINSIC_ERROR, "Error preparing iconv to convert from '%s' to '%s'.", fromcode, tocode);
	return;
     }

   if (NULL == (mmt = allocate_iconv_type (cd)))
     {
	iconv_close(cd);
	return;
     }

   if (-1 == SLang_push_mmt (mmt))
     {
	SLang_free_mmt (mmt);
	return;
     }
   return;
}

static void _iconv_close(ICONV_Type *it)
{
   if (it->cd != (iconv_t)-1)
     {
	iconv_close(it->cd);
	it->cd = (iconv_t)-1;
     }
}

static void destroy_iconv (SLtype type, VOID_STAR f)
{
   ICONV_Type *it;
   (void) type;

   it = (ICONV_Type *) f;
   iconv_close(it->cd);
   free_iconv_type (it);
}

static void _iconv_reset(ICONV_Type *it)
{
   iconv(it->cd, NULL, NULL, NULL, NULL);
}

#define SHIFT_BUF_LEN 64U
static void _iconv_reset_shift(ICONV_Type *it)
{
   size_t n = SHIFT_BUF_LEN;
   char buf[SHIFT_BUF_LEN], *p = buf;
   SLang_BString_Type *bstr;
   size_t rc;

   rc = iconv(it->cd, NULL, NULL, &p, &n);
   if ((rc == (size_t)(-1)) || (n > rc))
     {
	SLang_verror (SL_Internal_Error, "Internal error: shift buffer too small in iconv_reset_shift!");
	return;
     }
   buf[SHIFT_BUF_LEN-n] = '\0';
   bstr = SLbstring_create((unsigned char *)buf, SHIFT_BUF_LEN-n);
   if (bstr == NULL)
     return;

   (void) SLang_push_bstring(bstr);
   SLbstring_free (bstr);
}

static void _iconv(ICONV_Type *it, SLang_BString_Type *bstr)
{
   char *buf = NULL;
   size_t rc;
   char ICONV_CONST *instr;
   char *outstr;
   size_t inn, outn, bufn;
   size_t fail = (size_t)-1;
   unsigned int bstrlen;

   if (NULL == (instr = (char ICONV_CONST *)SLbstring_get_pointer(bstr, &bstrlen)))
     return;
   inn = (size_t) bstrlen;

   bufn = outn = 2*inn + 2;
   if (NULL == (buf = SLmalloc(bufn)))
     return;

   outstr = buf;

   while (1)
     {
	errno = 0;
	rc = iconv(it->cd, &instr, &inn, &outstr, &outn);
	if (rc != (size_t)-1)
	  break; /* ok! */

	if (fail == inn)
	  {
	     SLang_verror (SL_Unknown_Error, "Unknown error in iconv");
	     goto error;
	  }

	fail = inn;
	switch (errno)
	  {
	   case EILSEQ:
	     SLang_verror (SL_InvalidUTF8_Error, "Invalid multibyte sequence or unable to convert to the target encoding");
	     goto error;
	   case EINVAL:
	     SLang_verror (SL_InvalidUTF8_Error, "Incomplete multibyte sequence");
	     goto error;
	   case 0:
	     /* drop */
	     /* grrrr
	      * At least on windows, libiconv returns with errno = 0
	      * (or unmodified?) when there's no more roon on outstr
	      * if so, fallback
	      */
	   case E2BIG:
	       {
		  char *p;
		  int outdelta;

		  outdelta = outstr - buf;
		  outn += bufn;
		  bufn += bufn;
		  p = SLrealloc(buf, bufn);
		  if (p == NULL)
		    goto error;
		  buf = p;
		  outstr = buf + outdelta;
	       }
	     break;
	   default:
	     SLang_verror (SL_Unknown_Error, "Unknown iconv error");
	     goto error;
	  }
     }

   bstr = SLbstring_create((unsigned char *) buf, outstr - buf);
   if (bstr != NULL)
     (void) SLang_push_bstring(bstr);
   SLbstring_free (bstr);
   /* drop */
error:
   SLfree(buf);
}

#define DUMMY_ICONV_TYPE 255
#define P DUMMY_ICONV_TYPE
#define V SLANG_VOID_TYPE
#define S SLANG_STRING_TYPE
#define B SLANG_BSTRING_TYPE
static SLang_Intrin_Fun_Type ICONV_Intrinsics [] =
{
   MAKE_INTRINSIC_2("iconv_open", _iconv_open, V, S, S),
   MAKE_INTRINSIC_1("iconv_close", _iconv_close, V, P),
   MAKE_INTRINSIC_1("iconv_reset", _iconv_reset, V, P),
   MAKE_INTRINSIC_1("iconv_reset_shift", _iconv_reset_shift, V, P),
   MAKE_INTRINSIC_2("iconv", _iconv, V, P, B),
   SLANG_END_INTRIN_FUN_TABLE
};

#undef B
#undef S
#undef V
#undef P

static int register_iconv_type (void)
{
   SLang_Class_Type *cl;

   if (ICONV_Type_Id != 0)
     return 0;

   if (NULL == (cl = SLclass_allocate_class ("ICONV_Type")))
     return -1;

   if (-1 == SLclass_set_destroy_function (cl, destroy_iconv))
     return -1;

   /* By registering as SLANG_VOID_TYPE, slang will dynamically allocate a
    * type.
    */
   if (-1 == SLclass_register_class (cl, SLANG_VOID_TYPE, sizeof (ICONV_Type), SLANG_CLASS_TYPE_MMT))
     return -1;

   ICONV_Type_Id = SLclass_get_class_id (cl);
   if (-1 == SLclass_patch_intrin_fun_table1 (ICONV_Intrinsics, DUMMY_ICONV_TYPE, ICONV_Type_Id))
     return -1;

   return 0;
}

int init_iconv_module_ns (char *ns_name)
{
   SLang_NameSpace_Type *ns = SLns_create_namespace (ns_name);
   if (ns == NULL)
     return -1;

   if (-1 == register_iconv_type ())
     return -1;

   if (-1 == SLns_add_intrin_fun_table (ns, ICONV_Intrinsics, "__ICONV__"))
     return -1;

   return 0;
}

/* This function is optional */
void deinit_iconv_module (void)
{
}
