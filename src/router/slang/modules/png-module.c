/* -*- mode: C; mode: fold; -*- */
/*
Copyright (C) 2005-2011 John E. Davis

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

#include <png.h>

SLANG_MODULE(png);

static char *Module_Version_String = "0.1.1";
#define MODULE_VERSION_NUMBER  (0*10000 + 1*100 + 1)

/*{{{ Byte-swapping routines */

static int Is_Little_Endian;

static void byte_swap32 (unsigned char *p, unsigned char *t, unsigned int n)
{
   unsigned char *pmax, ch;

   /* t and p could point to the same buffer */
   pmax = p + 4 * n;
   while (p < pmax)
     {
	ch = *p;
	*t = *(p + 3);
	*(t + 3) = ch;

	ch = *(p + 1);
	*(t + 1) = *(p + 2);
	*(t + 2) = ch;
	p += 4;
	t += 4;
     }
}

static void byte_swap16 (unsigned char *p, unsigned char *t, unsigned int n)
{
   unsigned char *pmax, ch;

   pmax = p + 2 * n;
   while (p < pmax)
     {
	ch = *p;
	*t = *(p + 1);
	*(t + 1) = ch;
	p += 2;
	t += 2;
     }
}

/*}}}*/

/*{{{ Png_Type */

typedef struct
{
   FILE *fp;
   int mode;			       /* 'r' or 'w' */
   png_struct *png;
   png_info *info;
}
Png_Type;

static void free_png_type (Png_Type *p)
{
   if (p == NULL)
     return;
   if (p->png != NULL)
     {
	if (p->mode == 'r')
	  {
	     if (p->info != NULL)
	       png_destroy_read_struct (&p->png, &p->info, NULL);
	     else
	       png_destroy_read_struct (&p->png, NULL, NULL);
	  }
	else
	  {
	     if (p->info != NULL)
	       png_destroy_write_struct (&p->png, &p->info);
	     else
	       png_destroy_write_struct (&p->png, NULL);
	  }
     }

   if (p->fp != NULL)
     fclose (p->fp);

   SLfree ((char *) p);
}

static Png_Type *alloc_png_type (int mode)
{
   Png_Type *p;

   if (NULL != (p = (Png_Type *)SLmalloc (sizeof (Png_Type))))
     {
	memset ((char *) p, 0, sizeof (Png_Type));
	p->mode = mode;
     }
   return p;
}

/*}}}*/

static png_byte **allocate_image_pointers (png_uint_32 height, png_byte *data, png_uint_32 rowbytes, int flip)
{
   png_byte **image_pointers;
   png_uint_32 i;

   if (NULL == (image_pointers = (png_byte **) SLmalloc (height * sizeof (png_byte *))))
     return NULL;

   if (flip)
     {
	i = height;
	while (i != 0)
	  {
	     i--;
	     image_pointers[i] = data;
	     data += rowbytes;
	  }
	return image_pointers;
     }
   for (i = 0; i < height; i++)
     {
	image_pointers[i] = data;
	data += rowbytes;
     }
   return image_pointers;
}

static void free_image_pointers (png_byte **image_pointers)
{
   if (image_pointers == NULL)
     return;

   SLfree ((char *) image_pointers);
}

/*{{{ png read functions */

static Png_Type *open_png_file (char *file)
{
   png_byte header[8];
   Png_Type *p;

   if (NULL == (p = alloc_png_type ('r')))
     return NULL;

   if ((NULL == (p->fp = fopen (file, "rb")))
       || (8 != fread (header, 1, 8, p->fp))
       || (0 != png_sig_cmp(header, 0, 8)))
     {
	SLang_verror (SL_Open_Error, "Unable to open %s as a png file", file);
	free_png_type (p);
	return NULL;
     }

   if (NULL == (p->png = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
     {
	SLang_verror (SL_Open_Error, "Unable to read png structure from %s", file);
	free_png_type (p);
	return NULL;
     }

   if (NULL == (p->info = png_create_info_struct (p->png)))
     {
	SLang_verror (SL_Read_Error, "Unable to create info struct for %s", file);
	free_png_type (p);
	return NULL;
     }

   return p;
}

static void fixup_array_rgb (SLang_Array_Type *at)
{
   SLindex_Type num_rows, num_cols, row;
   unsigned char *data;

   num_rows = at->dims[0];
   num_cols = at->dims[1];
   data = (unsigned char *) at->data;

   /* Convert RGBRGBRGB....  to 0RGB0RGB0RGB ... */
   for (row = 0; row < num_rows; row++)
     {
	unsigned char *p = data + 3*num_cols;
	unsigned char *q = p + num_cols;
	while (p != data)
	  {
	     *(--q) = *(--p);
	     *(--q) = *(--p);
	     *(--q) = *(--p);
	     *(--q) = 0;	       /* or 0xFF */
	  }
	data += 4*num_cols;
     }

   if (Is_Little_Endian)
     byte_swap32 ((unsigned char *)at->data, (unsigned char *)at->data, at->num_elements);
}

static void fixup_array_rgba (SLang_Array_Type *at)
{
   unsigned char *data, *data_max;

   data = (unsigned char *) at->data;
   data_max = data + at->num_elements;

   /* RGBARGBA -> ARGBARGB */
   while (data < data_max)
     {
	unsigned char a = data[3];
	data[3] = data[2];
	data[2] = data[1];
	data[1] = data[0];
	data[0] = a;
	data += 4;
     }
   if (Is_Little_Endian)
     byte_swap32 ((unsigned char *)at->data, (unsigned char *)at->data, at->num_elements);
}

static void fixup_array_ga (SLang_Array_Type *at)
{
   if (Is_Little_Endian)
     byte_swap16 ((unsigned char *)at->data, (unsigned char *) at->data, at->num_elements);
}

/* For little endian systems, ARGB is equivalent to the int32 BGRA.
 * So, to read the image as RGB
 */
static SLang_Array_Type *read_image_internal (char *file, int flip, int *color_typep)
{
   Png_Type *p;
   png_uint_32 width, height, rowbytes;
   png_struct *png;
   png_info *info;
   int bit_depth;
   int interlace_type;
   int color_type;
   unsigned int sizeof_type;
   SLindex_Type dims[2];
   SLtype data_type;
   png_byte **image_pointers = NULL;
   png_byte *data = NULL;
   SLang_Array_Type *at;
   void (*fixup_array_fun) (SLang_Array_Type *);

   if (NULL == (p = open_png_file (file)))
     return NULL;

   png = p->png;
   if (setjmp (png_jmpbuf (png)))
     {
	free_png_type (p);
	if (data != NULL) SLfree ((char *) data);
	free_image_pointers (image_pointers);
	SLang_verror (SL_Read_Error, "Error encountered during I/O to %s", file);
	return NULL;
     }

   png_init_io (png, p->fp);
   png_set_sig_bytes (png, 8);
   info = p->info;
   png_read_info(png, info);

   width = png_get_image_width (png, info);
   height = png_get_image_height (png, info);
   interlace_type = png_get_interlace_type (png, info);
   bit_depth = png_get_bit_depth (png, info);

   if (bit_depth == 16)
     png_set_strip_16 (png);

   switch (png_get_color_type (png, info))
     {
      case PNG_COLOR_TYPE_GRAY:
#if defined(PNG_LIBPNG_VER) && (PNG_LIBPNG_VER >= 10209)
	if (bit_depth < 8) png_set_expand_gray_1_2_4_to_8 (png);
#else				       /* deprecated */
	if (bit_depth < 8) png_set_gray_1_2_4_to_8 (png);
#endif
	break;
      case PNG_COLOR_TYPE_GRAY_ALPHA:
	/* png_set_gray_to_rgb (png); */
	break;

      case PNG_COLOR_TYPE_PALETTE:
	png_set_palette_to_rgb (png);
	break;
     }

   if (png_get_valid(png, info, PNG_INFO_tRNS))
     png_set_tRNS_to_alpha(png);

   png_read_update_info (png, info);

   color_type = png_get_color_type (png, info);
   switch (color_type)
     {
      case PNG_COLOR_TYPE_RGBA:
	sizeof_type = 4;
	fixup_array_fun = fixup_array_rgba;
	data_type = SLang_get_int_type (32);
	break;

      case PNG_COLOR_TYPE_RGB:
	sizeof_type = 4;
	fixup_array_fun = fixup_array_rgb;
	data_type = SLang_get_int_type (32);
	break;

      case PNG_COLOR_TYPE_GRAY_ALPHA:
	sizeof_type = 2;
	fixup_array_fun = fixup_array_ga;
	data_type = SLang_get_int_type (16);
	break;

      case PNG_COLOR_TYPE_GRAY:
	sizeof_type = 1;
	fixup_array_fun = NULL;
	data_type = SLANG_UCHAR_TYPE;
	break;

      default:
	SLang_verror (SL_Read_Error, "Unsupported PNG color-type");
	free_png_type (p);
	return NULL;
     }
   *color_typep = color_type;

   /* Use the high-level interface */
   rowbytes = png_get_rowbytes (png, info);
   if (rowbytes > width * sizeof_type)
     {
	SLang_verror (SL_INTERNAL_ERROR, "Unexpected value returned from png_get_rowbytes");
	free_png_type (p);
	return NULL;
     }

   if (NULL == (data = (png_byte *) SLmalloc (height * width * sizeof_type)))
     {
	free_png_type (p);
	return NULL;
     }

   if (NULL == (image_pointers = allocate_image_pointers (height, data, width * sizeof_type, flip)))
     {
	SLfree ((char *) data);
	free_png_type (p);
	return NULL;
     }
   png_read_image(png, image_pointers);

   dims[0] = height;
   dims[1] = width;

   if (NULL == (at = SLang_create_array (data_type, 0, (VOID_STAR) data, dims, 2)))
     {
	SLfree ((char *) data);
	free_image_pointers (image_pointers);
	free_png_type (p);
	return NULL;
     }
   free_png_type (p);
   free_image_pointers (image_pointers);
   if (fixup_array_fun != NULL)
     (*fixup_array_fun) (at);
   return at;
}

static void read_image (int flipped)
{
   int color_type;
   char *file;
   SLang_Ref_Type *ref = NULL;
   SLang_Array_Type *at;

   if ((SLang_Num_Function_Args == 2)
       && (-1 == SLang_pop_ref (&ref)))
     return;

   if (-1 == SLang_pop_slstring (&file))
     {
	file = NULL;
	goto free_return;
     }

   if (NULL == (at = read_image_internal (file, flipped, &color_type)))
     goto free_return;

   if ((ref != NULL)
       && (-1 == SLang_assign_to_ref (ref, SLANG_INT_TYPE, &color_type)))
     {
	SLang_free_array (at);
	goto free_return;
     }

   (void) SLang_push_array (at, 1);

   free_return:
   SLang_free_slstring (file);
   if (ref != NULL)
     SLang_free_ref (ref);
}

/*}}}*/

static void write_gray_to_gray (png_struct *png, png_byte *data, SLindex_Type num_cols, png_byte *tmpbuf)
{
   (void) num_cols;
   (void) tmpbuf;
   png_write_row (png, data);
}

static void write_gray_to_gray_alpha (png_struct *png, png_byte *data, SLindex_Type num_cols, png_byte *tmpbuf)
{
   SLindex_Type i, j;

   j = 0;
   for (i = 0; i < num_cols; i++)
     {
	tmpbuf[j] = data[i];
	tmpbuf[j+1] = 0xFF;
	j += 2;
     }
   png_write_row (png, tmpbuf);
}

static void write_gray_alpha_to_gray (png_struct *png, png_byte *data, SLindex_Type num_cols, png_byte *tmpbuf)
{
   SLindex_Type i;

   if (Is_Little_Endian == 0)
     data++; /* AGAGAG... -> GAGAGA... */

   for (i = 0; i < num_cols; i++)
     {
	tmpbuf[i] = *data;
	data += 2;
     }
   png_write_row (png, tmpbuf);
}

static void write_gray_alpha_to_gray_alpha (png_struct *png, png_byte *data, SLindex_Type num_cols, png_byte *tmpbuf)
{
   if (Is_Little_Endian == 0)
     {
	png_write_row (png, data);
	return;
     }

   byte_swap16 ((unsigned char *) data, (unsigned char *) tmpbuf, num_cols);
   png_write_row (png, tmpbuf);
}

static void write_rgb_alpha_to_rgb_alpha (png_struct *png, png_byte *data, SLindex_Type num_cols, png_byte *tmpbuf)
{
   unsigned char *data_max;
   unsigned char *p;

   if (Is_Little_Endian)
     {
	byte_swap32 ((unsigned char *) data, (unsigned char *) tmpbuf, num_cols);
	data = tmpbuf;
     }
   data_max = data + 4 * num_cols;
   p = tmpbuf;
   /* Change ARGBARGB... to RGBARGBA... */
   while (data < data_max)
     {
	unsigned char a = data[0];
	p[0] = data[1];
	p[1] = data[2];
	p[2] = data[3];
	p[3] = a;
	data += 4;
	p += 4;
     }
   png_write_row (png, tmpbuf);
}

static void write_rgb_to_rgb (png_struct *png, png_byte *data, SLindex_Type num_cols, png_byte *tmpbuf)
{
   SLindex_Type i;
   png_byte *p, *q;

   if (Is_Little_Endian)
     {
	byte_swap32 ((unsigned char *) data, (unsigned char *) tmpbuf, num_cols);
	p = tmpbuf;
     }
   else p = data;

   /* ARGBARGB... -> RGBRGBRGB */
   q = tmpbuf;
   for (i = 0; i < num_cols; i++)
     {
	p++;
	*q++ = *p++;
	*q++ = *p++;
	*q++ = *p++;
     }
   png_write_row (png, tmpbuf);
}

static int write_array (png_struct *png, png_byte **image_pointers, SLindex_Type num_rows, SLindex_Type num_cols,
			void (*write_row_func) (png_struct *, png_byte *, SLindex_Type, png_byte *),
			png_byte *tmpbuf)
{
   int num_pass;
   SLindex_Type i;

   num_pass = png_set_interlace_handling(png);
   while (num_pass > 0)
     {
	num_pass--;
	for (i = 0; i < num_rows; i++)
	  (*write_row_func) (png, image_pointers[i], num_cols, tmpbuf);
     }
   return 0;
}

static int write_image_internal (char *file, SLang_Array_Type *at,
				 int color_type,
				 void (*write_fun)(png_struct *, png_byte *p, SLindex_Type, png_byte *),
				 int flip)
{
   FILE *fp;
   Png_Type *p = NULL;
   png_struct *png;
   png_info *info;
   SLindex_Type width, height;
   png_byte **image_pointers;
   int bit_depth;
   int status = -1;
   png_byte *tmpbuf;

   bit_depth = 8;

   height = at->dims[0];
   width = at->dims[1];

   if (NULL == (image_pointers = allocate_image_pointers (height, (png_byte *)at->data, width * at->sizeof_type, flip)))
     return -1;

   if (NULL == (tmpbuf = (png_byte *)SLmalloc (4*width)))
     {
	free_image_pointers (image_pointers);
	return -1;
     }

   if (NULL == (fp = fopen (file, "wb")))
     {
	(void)SLerrno_set_errno (errno);
	SLang_verror (SL_Open_Error, "Unable to open %s", file);
	goto return_error;
     }

   if (NULL == (p = alloc_png_type ('w')))
     goto return_error;

   p->fp = fp;

   if (NULL == (p->png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
     {
	SLang_verror (SL_Open_Error, "png_create_write_struct failed");
	goto return_error;
     }
   png = p->png;
   if (NULL == (p->info = png_create_info_struct (png)))
     {
	SLang_verror (SL_Open_Error, "png_create_info_struct failed");
	goto return_error;
     }
   info = p->info;
   if (setjmp(png_jmpbuf(png)))
     {
	SLang_verror (SL_Write_Error, "PNG I/O error");
	goto return_error;
     }
   png_init_io(png, fp);

   png_set_IHDR (png, info, width, height,
		 bit_depth, color_type, PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
   png_write_info(png, info);

   if (-1 == write_array (png, image_pointers, height, width, write_fun, tmpbuf))
     goto return_error;

   png_write_end(png, NULL);
   if (EOF == fclose (p->fp))
     {
	SLang_verror (SL_Write_Error, "Error closing %s", file);
	SLerrno_set_errno (errno);
     }
   else status = 0;

   p->fp = NULL;
   /* drop */
   return_error:
   if (tmpbuf != NULL)
     SLfree ((char *) tmpbuf);
   free_image_pointers (image_pointers);
   if (p != NULL)
     free_png_type (p);

   return status;
}

static void write_image (int flip)
{
   char *file;
   SLang_Array_Type *at;
   int with_alpha = 0;
   int has_with_alpha = 0;
   int color_type;
   void (*write_fun) (png_struct *, png_byte *, SLindex_Type, png_byte *);

   if (SLang_Num_Function_Args == 3)
     {
	if (-1 == SLang_pop_int (&with_alpha))
	  return;
	has_with_alpha = 1;
     }

   if (-1 == SLang_pop_array (&at, 0))
     return;

   if (at->num_dims != 2)
     {
	SLang_verror (SL_InvalidParm_Error, "Expecting a 2-d array");
	SLang_free_array (at);
	return;
     }

   switch (SLang_get_int_size (at->data_type))
     {
      case -8:
      case 8:
	if (with_alpha)
	  {
	     write_fun = write_gray_to_gray_alpha;
	     color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
	  }
	else
	  {
	     write_fun = write_gray_to_gray;
	     color_type = PNG_COLOR_TYPE_GRAY;
	  }
	break;
      case -16:
      case 16:
	if (has_with_alpha && (with_alpha == 0))
	  {
	     write_fun = write_gray_alpha_to_gray;
	     color_type = PNG_COLOR_TYPE_GRAY;
	  }
	else
	  {
	     write_fun = write_gray_alpha_to_gray_alpha;
	     color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
	  }
	break;
      case -32:
      case 32:
	if (with_alpha)
	  {
	     write_fun = write_rgb_alpha_to_rgb_alpha;
	     color_type = PNG_COLOR_TYPE_RGBA;
	  }
	else
	  {
	     write_fun = write_rgb_to_rgb;
	     color_type = PNG_COLOR_TYPE_RGB;
	  }
	break;
      default:
	SLang_verror (SL_InvalidParm_Error, "Expecting an 8, 16, or 32 bit integer array");
	SLang_free_array (at);
	return;
     }

   if (-1 == SLang_pop_slstring (&file))
     {
	SLang_free_array (at);
	return;
     }
   (void) write_image_internal (file, at, color_type, write_fun, flip);
   SLang_free_slstring (file);
   SLang_free_array (at);
}

static void read_intrin (void)
{
   read_image (0);
}
static void read_flipped_intrin (void)
{
   read_image (1);
}

static void write_intrin (void)
{
   write_image (0);
}
static void write_flipped_intrin (void)
{
   write_image (1);
}

static SLang_Intrin_Fun_Type Module_Intrinsics [] =
{
   MAKE_INTRINSIC_0("png_read", read_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("png_read_flipped", read_flipped_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("png_write", write_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("png_write_flipped", write_flipped_intrin, SLANG_VOID_TYPE),
   SLANG_END_INTRIN_FUN_TABLE
};

static SLang_Intrin_Var_Type Module_Variables [] =
{
   MAKE_VARIABLE("_png_module_version_string", &Module_Version_String, SLANG_STRING_TYPE, 1),
   SLANG_END_INTRIN_VAR_TABLE
};

static SLang_IConstant_Type Module_IConstants [] =
{
   MAKE_ICONSTANT("PNG_COLOR_TYPE_GRAY", PNG_COLOR_TYPE_GRAY),
   MAKE_ICONSTANT("PNG_COLOR_TYPE_GRAY_ALPHA", PNG_COLOR_TYPE_GRAY_ALPHA),
   MAKE_ICONSTANT("PNG_COLOR_TYPE_RGB", PNG_COLOR_TYPE_RGB),
   MAKE_ICONSTANT("PNG_COLOR_TYPE_RGBA", PNG_COLOR_TYPE_RGBA),

   MAKE_ICONSTANT("_png_module_version", MODULE_VERSION_NUMBER),
   SLANG_END_ICONST_TABLE
};

int init_png_module_ns (char *ns_name)
{
   unsigned short x;

   SLang_NameSpace_Type *ns = SLns_create_namespace (ns_name);
   if (ns == NULL)
     return -1;

   x = 0xFF;
   Is_Little_Endian = (*(unsigned char *)&x == 0xFF);

   if (
       (-1 == SLns_add_intrin_var_table (ns, Module_Variables, NULL))
       || (-1 == SLns_add_intrin_fun_table (ns, Module_Intrinsics, NULL))
       || (-1 == SLns_add_iconstant_table (ns, Module_IConstants, NULL))
       )
     return -1;

   return 0;
}

/* This function is optional */
void deinit_png_module (void)
{
}
