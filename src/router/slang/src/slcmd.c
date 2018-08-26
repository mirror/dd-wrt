/* cmd line facility for slang */
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

#if SLANG_HAS_FLOAT
# include <math.h>
#endif

#include "slang.h"
#include "_slang.h"

#ifndef HAVE_STDLIB_H
/* Oh dear.  Where is the prototype for atof?  If not in stdlib, then
 * I do not know where.  Not in math.h onsome systems either.
 */
extern double atof ();
#endif

static SLcmd_Cmd_Type *SLcmd_find_command (SLFUTURE_CONST char *s, SLcmd_Cmd_Type *cmd)
{
   SLCONST char *cmdstr;
   char chs = *s++, ch;

   while ((cmd->cmdfun != NULL)
	  && (NULL != (cmdstr = cmd->cmd))
	  && (0 != (ch = *cmdstr++)))
     {
	if ((ch == chs) && !strcmp (s, cmdstr)) return cmd;
	cmd++;
     }
   return NULL;
}

static int extract_token (SLFUTURE_CONST char **strptr, char *buf)
{
   SLFUTURE_CONST char *s;
   char *b;
   char ch, quote;

   *buf = 0;

   s = *strptr;
   while (((ch = *s) != 0)
	  && ((ch == ' ') || (ch == '\t') || (ch == '\n')))
     s++;

   *strptr = s;

   if (ch == 0) return 0;
   if (ch == '%') return 0;

   b = buf;

   *b++ = ch;
   s++;

   if ((ch == '\'') || (ch == '"'))
     {
	quote = ch;
	while ((ch = *s) != 0)
	  {
	     s++;
	     *b++ = ch;
	     if (ch == quote)
	       break;

	     if (ch == '\\')
	       {
		  if (0 == (ch = *s))
		    break;
		  *b++ = ch;
		  s++;
	       }
	  }
	*strptr = s;
	*b = 0;
	return 1;
     }

   while (((ch = *s) != 0)
	  && (ch != ' ')
	  && (ch != '\t')
	  && (ch != '\n')
	  && (ch != '%'))
     *b++ = *s++;

   *strptr = s;
   *b = 0;
   return 1;
}

static int allocate_arg_space (SLcmd_Cmd_Table_Type *table, int argc, unsigned int *space_ptr)
{
   unsigned int space = *space_ptr;
   char *p;

   if (argc + 1 < (int) space)
     return 0;

   if (space > 128)
     {
	if (space > 1024) space += 1024;
	else space += 128;
     }
   else space += 32;

   if (NULL == (p = SLrealloc ((char *)table->string_args, space * sizeof (char *))))
     return -1;
   table->string_args = (SLFUTURE_CONST char **)p;
   table->string_args [argc] = NULL;

   if (NULL == (p = SLrealloc ((char *)table->int_args, space * sizeof (int))))
     return -1;
   table->int_args = (int *)p;

   if (NULL == (p = SLrealloc ((char *)table->double_args, space * sizeof (double))))
     return -1;
   table->double_args = (double *)p;

   if (NULL == (p = SLrealloc ((char *)table->arg_type, space * sizeof (SLtype))))
     return -1;
   table->arg_type = (SLtype *)p;

   *space_ptr = space;
   return 0;
}

int SLcmd_execute_string (SLFUTURE_CONST char *str, SLcmd_Cmd_Table_Type *table)
{
   SLFUTURE_CONST char *s, *arg_type, *last_str, *cmd_name;
   SLcmd_Cmd_Type *cmd;
   char *buf;
   int token_present;
   int i;
   int status;
   unsigned int len;
   int argc;
   unsigned int space;

   table->argc = 0;
   table->string_args = NULL;
   table->int_args = NULL;
   table->double_args = NULL;
   table->arg_type = NULL;

   buf = SLmake_string (str);
   if (buf == NULL)
     return -1;

   status = extract_token (&str, buf);
   if (status <= 0)
     {
	SLfree (buf);
	return status;
     }

   if (((len = strlen (buf)) >= 32)
       || (NULL == (cmd = SLcmd_find_command (buf, table->table))))
     {
	_pSLang_verror (SL_UNDEFINED_NAME,"%s: invalid command", buf);
	SLfree (buf);
	return -1;
     }

   if (NULL == (cmd_name = SLmake_string (buf)))
     {
	SLfree (buf);
	return -1;
     }

   space = 0;
   argc = 0;
   if (-1 == allocate_arg_space (table, argc, &space))
     {
	SLfree (buf);
	return -1;
     }
   table->arg_type[argc] = SLANG_STRING_TYPE;
   table->string_args[argc++] = cmd_name;

   arg_type = cmd->arg_type;
   status = -1;
   while (*arg_type)
     {
	int guess_type = 0;

	last_str = str;

	if (-1 == allocate_arg_space (table, argc, &space))
	  goto error;

	if (-1 == (token_present = extract_token (&str, buf)))
	  goto error;

	table->string_args[argc] = NULL;

	if (token_present)
	  {
	     char *b = buf;
	     len = strlen (b);

	     if ((*b == '"') && (len > 1))
	       {
		  b++;
		  len -= 2;
		  b[len] = 0;
		  guess_type = SLANG_STRING_TYPE;
		  SLexpand_escaped_string (buf, b, b + len, -1);
		  len = strlen (buf);
	       }
	     else if ((*b == '\'') && (len > 1))
	       {
		  SLwchar_Type ch;
		  b++;
		  len -= 2;
		  b[len] = 0;
		  guess_type = SLANG_INT_TYPE;
		  ch = *b;
		  if (ch == '\\')
		    {
		       if (NULL == _pSLexpand_escaped_char (b, b+len, &ch, NULL))
			 goto error;
		    }
		  sprintf (buf, "%lu", (unsigned long)ch);
		  len = strlen (buf);
	       }
	     else guess_type = SLang_guess_type (buf);
	  }

	switch (*arg_type++)
	  {
	     /* variable argument number */
	   case 'v':
	     if (token_present == 0) break;
	   case 'V':
	     if (token_present == 0)
	       {
		  _pSLang_verror (SL_INVALID_PARM, "%s: Expecting argument", cmd_name);
		  goto error;
	       }

	     while (*last_str == ' ') last_str++;
	     len = strlen (last_str);
	     str = last_str + len;

	     s = SLmake_nstring (last_str, len);
	     if (s == NULL) goto error;

	     table->arg_type[argc] = SLANG_STRING_TYPE;
	     table->string_args[argc++] = s;
	     break;

	   case 's':
	     if (token_present == 0) break;
	   case 'S':
	     if (token_present == 0)
	       {
		  _pSLang_verror (SL_TYPE_MISMATCH, "%s: Expecting string argument", cmd_name);
		  goto error;
	       }

	     s = SLmake_nstring (buf, len);
	     if (s == NULL) goto error;
	     table->arg_type[argc] = SLANG_STRING_TYPE;
	     table->string_args[argc++] = s;
	     break;

	     /* integer argument */
	   case 'i':
	     if (token_present == 0) break;
	   case 'I':
	     if ((token_present == 0) || (SLANG_INT_TYPE != guess_type))
	       {
		  _pSLang_verror (SL_TYPE_MISMATCH, "%s: Expecting integer argument", cmd_name);
		  goto error;
	       }

	     table->arg_type[argc] = SLANG_INT_TYPE;
	     table->int_args[argc++] = SLatoi((unsigned char *) buf);
	     break;

	     /* floating point arg */
#if SLANG_HAS_FLOAT
	   case 'f':
	     if (token_present == 0) break;
	   case 'F':
	     if ((token_present == 0) || (SLANG_STRING_TYPE == guess_type))
	       {
		  _pSLang_verror (SL_TYPE_MISMATCH, "%s: Expecting double argument", cmd_name);
		  goto error;
	       }
	     table->arg_type[argc] = SLANG_DOUBLE_TYPE;
	     table->double_args[argc++] = atof(buf);
	     break;
#endif
	     /* Generic type */
	   case 'g':
	     if (token_present == 0) break;
	   case 'G':
	     if (token_present == 0)
	       {
		  _pSLang_verror (SL_TYPE_MISMATCH, "%s: Expecting argument", cmd_name);
		  goto error;
	       }

	     switch (guess_type)
	       {
		case SLANG_INT_TYPE:
		  table->arg_type[argc] = SLANG_INT_TYPE;
		  table->int_args[argc++] = SLatoi((unsigned char *) buf);
		  break;

		case SLANG_STRING_TYPE:
		  s = SLmake_nstring (buf, len);
		  if (s == NULL) goto error;

		  table->arg_type[argc] = SLANG_STRING_TYPE;
		  table->string_args[argc++] = s;
		  break;
#if SLANG_HAS_FLOAT
		case SLANG_DOUBLE_TYPE:
		  table->arg_type[argc] = SLANG_DOUBLE_TYPE;
		  table->double_args[argc++] = atof(buf);
#endif
	       }
	     break;
	  }
     }

   /*                 call function */
   status = (*cmd->cmdfun)(argc, table);

   error:
   if (table->string_args != NULL) for (i = 0; i < argc; i++)
     {
	if (NULL != table->string_args[i])
	  {
	     SLfree ((char *) table->string_args[i]);
	     table->string_args[i] = NULL;
	  }
     }
   SLfree ((char *)table->string_args); table->string_args = NULL;
   SLfree ((char *)table->double_args); table->double_args = NULL;
   SLfree ((char *)table->int_args); table->int_args = NULL;
   SLfree ((char *)table->arg_type); table->arg_type = NULL;

   SLfree (buf);
   return status;
}

