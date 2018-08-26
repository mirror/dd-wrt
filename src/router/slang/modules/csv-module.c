#include <stdio.h>
#include <string.h>
#include <slang.h>

SLANG_MODULE(csv);

static int CSV_Type_Id = 0;

typedef struct _CSV_Type CSV_Type;
struct _CSV_Type
{
   char delimchar;
   char quotechar;
   SLang_Name_Type *read_callback;
   SLang_Any_Type *callback_data;
#define CSV_SKIP_BLANK_ROWS	0x01
#define CSV_STOP_ON_BLANK_ROWS	0x02
#define BLANK_ROW_BEHAVIOR	(CSV_SKIP_BLANK_ROWS|CSV_STOP_ON_BLANK_ROWS)
#define CSV_QUOTE_SOME		0x04
#define CSV_QUOTE_ALL		0x08
   int flags;
};

static int execute_read_callback (CSV_Type *csv, char **sptr)
{
   char *s;

   *sptr = NULL;

   if ((-1 == SLang_start_arg_list ())
       || (-1 == SLang_push_anytype (csv->callback_data))
       || (-1 == SLang_end_arg_list ())
       || (-1 == SLexecute_function (csv->read_callback)))
     return -1;

   if (SLang_peek_at_stack () == SLANG_NULL_TYPE)
     {
	(void) SLang_pop_null ();
	return 0;
     }

   if (-1 == SLang_pop_slstring (&s))
     return -1;

   *sptr = s;
   return 1;
}

typedef struct
{
   char **values;
   SLindex_Type num_allocated;
   SLindex_Type num;
}
Values_Array_Type;

static int push_values_array (Values_Array_Type *av, int allow_empty_array)
{
   SLang_Array_Type *at;
   char **new_values;

   if (av->num == 0)
     {
	if (allow_empty_array == 0)
	  return SLang_push_null ();
	SLfree ((char *) av->values);
	av->values = NULL;
     }
   else
     {
	if (NULL == (new_values = (char **)SLrealloc ((char *)av->values, av->num*sizeof(char *))))
	  return -1;
	av->values = new_values;
     }

   av->num_allocated = av->num;
   at = SLang_create_array (SLANG_STRING_TYPE, 0, av->values, &av->num, 1);

   if (at == NULL)
     return -1;

   av->num_allocated = 0;
   av->num = 0;
   av->values = NULL;

   return SLang_push_array (at, 1);
}

static int init_values_array_type (Values_Array_Type *av)
{
   memset ((char *)av, 0, sizeof(Values_Array_Type));
   return 0;
}

static void free_values_array (Values_Array_Type *av)
{
   SLindex_Type i, num;
   char **values;

   if (NULL == (values = av->values))
     return;
   num = av->num;
   for (i = 0; i < num; i++)
     SLang_free_slstring (values[i]);
   SLfree ((char *)values);
}

static int store_value (Values_Array_Type *va, char *value)
{
   SLindex_Type num_allocated;

   num_allocated = va->num_allocated;
   if (num_allocated == va->num)
     {
	char **values;
	num_allocated += 256;
	values = (char **)SLrealloc ((char *)va->values, num_allocated*sizeof(char *));
	if (values == NULL)
	  return -1;
	va->values = values;
     }
   if (NULL == (va->values[va->num] = SLang_create_slstring (value)))
     return -1;

   va->num++;
   return 0;
}

#define NEXT_CHAR(ch) \
   while (do_read \
	  || (0 == (ch = line[line_ofs++])) \
	  || (ch == '\r')) \
   { \
      if ((do_read == 0) && (ch == '\r') && (line[line_ofs] == '\n')) \
	{ \
	   line_ofs++; \
	   ch = '\n'; \
	   break; \
	} \
      SLang_free_slstring (line); \
      line = NULL; \
      status = execute_read_callback (csv, &line); \
      do_read = 0; \
      if (status == -1) \
	goto return_error; \
      line_ofs = 0; \
      if (status == 0) \
	{ \
	   ch = 0; \
	   break; \
	} \
   }


static int decode_csv_row (CSV_Type *csv, int flags)
{
   char *line;
   size_t line_ofs;
   char *value;
   size_t value_size, value_ofs;
   char delimchar, quotechar;
   int return_status;
   Values_Array_Type av;
   int do_read, in_quote;
   int blank_line_seen;
   int is_quoted;

   if (NULL == csv->read_callback)
     {
	SLang_verror (SL_InvalidParm_Error, "CSV decoder object has no read callback function");
	return -1;
     }
	
   if (-1 == init_values_array_type (&av))
     return -1;

   delimchar = csv->delimchar;
   quotechar = csv->quotechar;
   value_ofs = line_ofs = 0;
   value_size = 0;
   value = NULL;
   line = NULL;
   do_read = 1;

   in_quote = 0;
   return_status = -1;
   blank_line_seen = 0;
   is_quoted = 0;
   while (1)
     {
	int status;
	char ch;

	if (value_ofs == value_size)
	  {
	     char *new_value;

	     if (value_size < 64)
	       value_size += 32;
	     else if (value_size < 8192)
	       value_size *= 2;
	     else value_size += 8192;

	     new_value = SLrealloc (value, value_size);
	     if (new_value == NULL)
	       goto return_error;
	     value = new_value;
	  }

	NEXT_CHAR(ch)

	if ((ch == quotechar) && quotechar)
	  {
	     if (in_quote)
	       {
		  NEXT_CHAR(ch)
		  if (ch == quotechar)
		    {
		       value[value_ofs++] = ch;
		       continue;
		    }

		  if ((ch != ',') && (ch != 0) && (ch != '\n'))
		    {
		       SLang_verror (SL_Data_Error, "Expecting a delimiter after an end-quote character");
		       goto return_error;
		    }
		  in_quote = 0;
		  /* drop */
	       }
	     else if (value_ofs != 0)
	       {
		  SLang_verror (SL_Data_Error, "Misplaced quote character inside a csv field");
		  goto return_error;
	       }
	     else 
	       {
		  in_quote = 1;
		  is_quoted = 1;
		  continue;
	       }
	  }

	if (ch == delimchar)
	  {
	     if (in_quote)
	       {
		  value[value_ofs++] = ch;
		  continue;
	       }
	     value[value_ofs] = 0;
	     if (-1 == store_value (&av, value))
	       goto return_error;
	     value_ofs = 0;
	     continue;
	  }
	if ((ch == 0) || (ch == '\n'))
	  {
	     if (in_quote)
	       {
		  if (ch == '\n')
		    {
		       value[value_ofs++] = ch;
		       do_read = 1;
		       continue;
		    }
		  SLang_verror (SL_Data_Error, "No closing quote seen parsing CSV data");
		  goto return_error;
	       }

	     if ((ch == '\n') || (av.num != 0) || (value_ofs > 0))
	       {
		  if ((is_quoted == 0)
		      && (ch == '\n') && (av.num == 0) && (value_ofs == 0))
		    {
		       /* blank line */
		       int blank_line_behavior = (flags & BLANK_ROW_BEHAVIOR);
		       if (blank_line_behavior == CSV_SKIP_BLANK_ROWS)
			 {
			    do_read = 1;
			    continue;
			 }
		       if (blank_line_behavior == CSV_STOP_ON_BLANK_ROWS)
			 {
			    blank_line_seen = 1;
			    break;
			 }
		    }
		  value[value_ofs] = 0;
		  if (-1 == store_value (&av, value))
		    goto return_error;
	       }
	     break;		       /* done */
	  }

	value[value_ofs++] = ch;
     }

   /* Get here if at end of line or file */
   return_status = push_values_array (&av, blank_line_seen);
   /* drop */

return_error:
   SLfree (value);
   free_values_array(&av);
   if (line != NULL)
     SLang_free_slstring (line);
   return return_status;
}

static void free_csv_type (CSV_Type *csv)
{
   if (csv == NULL)
     return;
   if (csv->callback_data != NULL) SLang_free_anytype (csv->callback_data);
   if (csv->read_callback != NULL) SLang_free_function (csv->read_callback);
   SLfree ((char *)csv);
}

static CSV_Type *pop_csv_type (SLang_MMT_Type **mmtp)
{
   SLang_MMT_Type *mmt;

   if (NULL == (mmt = SLang_pop_mmt (CSV_Type_Id)))
     {
	*mmtp = NULL;
	return NULL;
     }
   *mmtp = mmt;
   return (CSV_Type *)SLang_object_from_mmt (mmt);
}

/* Usage: obj = cvs_decoder_new (&read_callback, callback_data, delim, quote, flags) */
static void new_csv_decoder_intrin (void)
{
   CSV_Type *csv;
   SLang_MMT_Type *mmt;

   if (NULL == (csv = (CSV_Type *)SLmalloc(sizeof(CSV_Type))))
     return;
   memset ((char *)csv, 0, sizeof(CSV_Type));

   if ((-1 == SLang_pop_int (&csv->flags))
       ||(-1 == SLang_pop_char (&csv->quotechar))
       || (-1 == SLang_pop_char (&csv->delimchar))
       || (-1 == SLang_pop_anytype (&csv->callback_data))
       || (NULL == (csv->read_callback = SLang_pop_function ()))
       || (NULL == (mmt = SLang_create_mmt (CSV_Type_Id, (VOID_STAR)csv))))
     {
	free_csv_type (csv);
	return;
     }

   if (-1 == SLang_push_mmt (mmt))
     SLang_free_mmt (mmt);
}

static void decode_csv_row_intrin (void)
{
   CSV_Type *csv;
   SLang_MMT_Type *mmt;
   int flags = 0;
   int has_flags = 0;

   if (SLang_Num_Function_Args == 2)
     {
	if (-1 == SLang_pop_int (&flags))
	  return;

	has_flags = 1;
     }
   if (NULL == (csv = pop_csv_type (&mmt)))
     return;

   if (has_flags == 0)
     flags = csv->flags;

   (void) decode_csv_row (csv, flags);
   SLang_free_mmt (mmt);
}

/* returns a malloced string */
static char *csv_encode (CSV_Type *csv,
			 char **fields, SLuindex_Type nfields,
			 int flags)
{
   char *encoded_str, *s;
   size_t size;
   SLuindex_Type i;
   char delimchar, quotechar;
   int quote_some, quote_all;
   char *fieldflags;

   delimchar = csv->delimchar;
   quotechar = csv->quotechar;
   quote_some = flags & (CSV_QUOTE_SOME|CSV_QUOTE_ALL);
   quote_all = flags & CSV_QUOTE_ALL;

   size = 0;
   if (nfields > 1)
     size += nfields-1;		       /* for delimiters */
   size += 3;			       /* for CRLF\0 */

   fieldflags = SLmalloc(nfields+1);
   if (fieldflags == NULL)
     return NULL;

   for (i = 0; i < nfields; i++)
     {
	char ch, *f, *field = fields[i];
	int needs_quote = 0;

	fieldflags[i] = 0;
	if ((field == NULL) || (*field == 0))
	  {
	     if (quote_some)
	       {
		  fieldflags[i] = 1;
		  size += 2;
	       }
	     continue;
	  }
	f = field;
	while ((ch = *f++) != 0)
	  {
	     size++;
	     if (ch == quotechar)
	       {
		  needs_quote=1;
		  size++;
		  continue;
	       }
	     if (ch == delimchar)
	       {
		  needs_quote = 1;
		  continue;
	       }

	     if ((unsigned char)ch > ' ')
	       continue;
		  
	     if (ch == '\n')
	       {
		  size++;	       /* for \r */
		  needs_quote = 1;
		  continue;
	       }
	     if (quote_some)
	       needs_quote = 1;
	  }

	if (needs_quote || quote_all)
	  {
	     fieldflags[i] = 1;
	     size += 2;
	  }
     }

   if (NULL == (encoded_str = SLmalloc (size)))
     {
	SLfree (fieldflags);
	return NULL;
     }
   s = encoded_str;

   i = 0;
   while (i < nfields)
     {
	char ch, *f, *field;
	int needs_quote;

	needs_quote = fieldflags[i];
	field = fields[i];
	i++;

	if ((i > 1) && (i <= nfields))
	  *s++ = delimchar;

	if (needs_quote) *s++ = quotechar;

	if ((field == NULL) || (*field == 0))
	  {
	     if (needs_quote)
	       *s++ = quotechar;
	     continue;
	  }

	f = field;
	while ((ch = *f++) != 0)
	  {
	     if (ch == quotechar)
	       {
		  *s++ = ch;
		  *s++ = ch;
		  continue;
	       }

	     if (ch == '\n')
	       {
		  *s++ = '\r';
		  *s++ = ch;
		  continue;
	       }

	     *s++ = ch;
	  }
	if (needs_quote)
	  *s++ = quotechar;
     }

   *s++ = '\r';
   *s++ = '\n';
   *s = 0;

   SLfree (fieldflags);
   return encoded_str;
}

static void encode_csv_row_intrin (void)
{
   SLang_Array_Type *at;
   CSV_Type *csv;
   SLang_MMT_Type *mmt;
   int flags;
   int has_flags;
   char *str;

   if (SLang_Num_Function_Args == 3)
     {
	if (-1 == SLang_pop_int (&flags))
	  return;
	has_flags = 1;
     }
   else has_flags = 0;

   if (-1 == SLang_pop_array_of_type (&at, SLANG_STRING_TYPE))
     return;

   if (NULL == (csv = pop_csv_type (&mmt)))
     {
	SLang_free_array (at);
	return;
     }

   if (0 == has_flags)
     flags = csv->flags;

   str = csv_encode (csv, (char **)at->data, at->num_elements, flags);
   SLang_free_mmt (mmt);
   SLang_free_array (at);
   (void) SLang_push_malloced_string (str);
}

static void new_csv_encoder_intrin (void)
{
   CSV_Type *csv;
   SLang_MMT_Type *mmt;

   if (NULL == (csv = (CSV_Type *)SLmalloc(sizeof(CSV_Type))))
     return;
   memset ((char *)csv, 0, sizeof(CSV_Type));

   if ((-1 == SLang_pop_int (&csv->flags))
       ||(-1 == SLang_pop_char (&csv->quotechar))
       || (-1 == SLang_pop_char (&csv->delimchar))
       || (NULL == (mmt = SLang_create_mmt (CSV_Type_Id, (VOID_STAR)csv))))
     {
	free_csv_type (csv);
	return;
     }

   if (-1 == SLang_push_mmt (mmt))
     SLang_free_mmt (mmt);
}

#define DUMMY_CSV_TYPE ((SLtype)-1)
static SLang_Intrin_Fun_Type Module_Intrinsics [] =
{
   MAKE_INTRINSIC_0("_csv_decoder_new", new_csv_decoder_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_csv_decode_row", decode_csv_row_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_csv_encoder_new", new_csv_encoder_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_csv_encode_row", encode_csv_row_intrin, SLANG_VOID_TYPE),
   SLANG_END_INTRIN_FUN_TABLE
};

static SLang_IConstant_Type Module_Constants [] =
{
   MAKE_ICONSTANT("CSV_SKIP_BLANK_ROWS", CSV_SKIP_BLANK_ROWS),
   MAKE_ICONSTANT("CSV_STOP_BLANK_ROWS", CSV_STOP_ON_BLANK_ROWS),
   MAKE_ICONSTANT("CSV_QUOTE_SOME", CSV_QUOTE_SOME),
   MAKE_ICONSTANT("CSV_QUOTE_ALL", CSV_QUOTE_ALL),
   SLANG_END_ICONST_TABLE
};

static void destroy_csv (SLtype type, VOID_STAR f)
{
   (void) type;
   free_csv_type ((CSV_Type *)f);
}

static int register_csv_type (void)
{
   SLang_Class_Type *cl;

   if (CSV_Type_Id != 0)
     return 0;

   if (NULL == (cl = SLclass_allocate_class ("CSV_Type")))
     return -1;

   if (-1 == SLclass_set_destroy_function (cl, destroy_csv))
     return -1;

   /* By registering as SLANG_VOID_TYPE, slang will dynamically allocate a
    * type.
    */
   if (-1 == SLclass_register_class (cl, SLANG_VOID_TYPE, sizeof (CSV_Type), SLANG_CLASS_TYPE_MMT))
     return -1;

   CSV_Type_Id = SLclass_get_class_id (cl);
   if (-1 == SLclass_patch_intrin_fun_table1 (Module_Intrinsics, DUMMY_CSV_TYPE, CSV_Type_Id))
     return -1;

   return 0;
}

   
int init_csv_module_ns (char *ns_name)
{
   SLang_NameSpace_Type *ns = SLns_create_namespace (ns_name);
   if (ns == NULL)
     return -1;

   if (-1 == register_csv_type ())
     return -1;

   if ((-1 == SLns_add_intrin_fun_table (ns, Module_Intrinsics, NULL))
       || (-1 == SLns_add_iconstant_table (ns, Module_Constants, NULL)))
     return -1;

   return 0;
}

/* This function is optional */
void deinit_csv_module (void)
{
}
