/* Misc Array Functions */
/*
Copyright (C) 2004-2011 John E. Davis

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

#include "slinclud.h"

#include "slang.h"
#include "_slang.h"

int SLang_get_array_element (SLang_Array_Type *at, SLindex_Type *indices, VOID_STAR data)
{
   int is_ptr;

   if ((at == NULL)
       || (indices == NULL)
       || (data == NULL))
     return -1;

   is_ptr = (at->flags & SLARR_DATA_VALUE_IS_POINTER);
   if (is_ptr) *(VOID_STAR *) data = NULL;
   return _pSLarray_aget_transfer_elem (at, indices, data, at->sizeof_type, is_ptr);
}

int SLang_set_array_element (SLang_Array_Type *at, SLindex_Type *indices, VOID_STAR data)
{
   if ((at == NULL)
       || (indices == NULL)
       || (data == NULL))
     return -1;

   return _pSLarray_aput_transfer_elem (at, indices, data, at->sizeof_type,
				       at->flags & SLARR_DATA_VALUE_IS_POINTER);
}

