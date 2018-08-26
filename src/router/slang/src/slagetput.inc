/* -*- mode: C -*- */
#ifdef AGET_FROM_INDEX_ARRAY_FUN
static int AGET_FROM_INDEX_ARRAY_FUN (GENERIC_TYPE *src_data, SLindex_Type num_elements,
				      SLang_Array_Type *ind_at, int is_range,
				      GENERIC_TYPE *dest_data)
{
   SLindex_Type *indices, *indices_max;

   if (is_range)
     {
	SLarray_Range_Array_Type *r = (SLarray_Range_Array_Type *)ind_at->data;
	SLindex_Type idx = r->first_index, delta = r->delta;
	SLindex_Type j, jmax = (SLindex_Type) ind_at->num_elements;

	if (jmax == 0)
	  return 0;

	if ((idx >= 0)
	    && ((idx + delta * (jmax-1)) >= 0)
	    && ((idx + delta * (jmax-1)) < num_elements))
	  {
	     for (j = 0; j < jmax; j++)
	       {
		  dest_data[j] = src_data[idx];
		  idx += delta;
	       }
	     return 0;
	  }

	for (j = 0; j < jmax; j++)
	  {
	     SLindex_Type i = idx;
	     if (i < 0)
	       {
		  i += num_elements;
		  if (i < 0)
		    i = num_elements;
	       }
	     if (i >= num_elements)
	       {
		  SLang_set_error (SL_Index_Error);
		  return -1;
	       }
	     dest_data[j] = src_data[i];
	     idx += delta;
	  }
	return 0;
     }

   /* Since the index array is linear, I can address it directly */
   indices = (SLindex_Type *) ind_at->data;
   indices_max = indices + ind_at->num_elements;

   while (indices < indices_max)
     {
	SLindex_Type i = *indices;

	if (i < 0)
	  {
	     i += num_elements;
	     if (i < 0)
	       i = num_elements;
	  }
	if (i >= num_elements)
	  {
	     SLang_set_error (SL_Index_Error);
	     return -1;
	  }
	*dest_data++ = src_data[i];
	indices++;
     }
   return 0;
}
# undef AGET_FROM_INDEX_ARRAY_FUN
#endif

#ifdef APUT_FROM_INDEX_ARRAY_FUN
static int APUT_FROM_INDEX_ARRAY_FUN (char *src_data, SLuindex_Type data_increment,
				      SLang_Array_Type *ind_at, int is_range,
				      GENERIC_TYPE *dest_data, SLindex_Type num_elements)
{
   SLindex_Type *indices, *indices_max;

   if (is_range)
     {
	SLarray_Range_Array_Type *r = (SLarray_Range_Array_Type *)ind_at->data;
	SLindex_Type idx = r->first_index, delta = r->delta;
	SLindex_Type j, jmax = (SLindex_Type) ind_at->num_elements;

	if (jmax == 0)
	  return 0;

	if ((idx >= 0)
	    && ((idx + delta * (jmax-1)) >= 0)
	    && ((idx + delta * (jmax-1)) < num_elements))
	  {
	     for (j = 0; j < jmax; j++)
	       {
		  dest_data[idx] = *(GENERIC_TYPE *)src_data;
		  idx += delta;
		  src_data += data_increment;
	       }
	     return 0;
	  }

	for (j = 0; j < jmax; j++)
	  {
	     SLindex_Type i = idx;
	     if (i < 0)
	       {
		  i += num_elements;
		  if (i < 0)
		    i = num_elements;
	       }
	     if (i >= num_elements)
	       {
		  SLang_set_error (SL_Index_Error);
		  return -1;
	       }
	     dest_data[i] = *(GENERIC_TYPE *)src_data;
	     idx += delta;
	     src_data += data_increment;
	  }
	return 0;
     }

   /* Since the index array is linear, I can address it directly */
   indices = (SLindex_Type *) ind_at->data;
   indices_max = indices + ind_at->num_elements;

   while (indices < indices_max)
     {
	SLindex_Type i = *indices;

	if (i < 0)
	  {
	     i += num_elements;
	     if (i < 0)
	       i = num_elements;
	  }
	if (i >= num_elements)
	  {
	     SLang_set_error (SL_Index_Error);
	     return -1;
	  }
	dest_data[i] = *(GENERIC_TYPE *)src_data;
	src_data += data_increment;
	indices++;
     }
   return 0;
}
# undef APUT_FROM_INDEX_ARRAY_FUN
#endif

#undef GENERIC_TYPE
