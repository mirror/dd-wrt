/* Copyright (c) 1998-2011 John E. Davis
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static char *C_Char_Map_Name = "Keyword_Hash_Table";
static char *C_Hash_Function_Name = "keyword_hash";
static char *C_Hash_Table_Type = "Keyword_Table_Type";
static char *C_Hash_Table_Name = "Keyword_Table";
static char *C_Is_Keyword_Function_Name = "is_keyword";

typedef struct
{
   unsigned int hash;
   unsigned int len;
   unsigned int freq_statistic;
   char *name;
   char *type;
}
String_Table_Type;

static String_Table_Type String_Table [256];

static unsigned int Num_Strings;

static unsigned int Max_Table_Value;
static unsigned int Max_String_Len;
static unsigned int Min_String_Len;
static unsigned int Max_Hash_Value;
static unsigned int Tweak_Step_Size;
static int Use_Length = 1;

static int Char_Table_Map [256];

static void write_table (unsigned int min_hash, unsigned int max_hash)
{
   String_Table_Type *st, *st_max;
   unsigned int i;

   fprintf (stdout, "\n\
typedef SLCONST struct\n\
{\n\
   SLCONST char *name;\n\
   unsigned int type;\n\
}\n\
%s;\n\
",
	    C_Hash_Table_Type);

   fprintf (stdout, "\nstatic %s %s [/* %u */] =\n{\n",
	    C_Hash_Table_Type, C_Hash_Table_Name, (max_hash - min_hash) + 1);
   fprintf (stderr, "String Table Size: %u\n", (max_hash - min_hash) + 1);

   for (i = min_hash; i <= max_hash; i++)
     {
	st = String_Table;
	st_max = st + Num_Strings;
	while (st < st_max)
	  {
	     if ((unsigned int) st->hash == i)
	       break;
	     st++;
	  }

	if (st == st_max)
	  fprintf (stdout, "   {NULL,0},\n");
	else
	  fprintf (stdout, "   {\"%s\",\t%s},\n", st->name, st->type);
     }

   fputs ("};\n\n", stdout);

   fprintf (stdout, "\
static %s *%s (char *str, unsigned int len)\n\
{\n\
   unsigned int hash;\n\
   SLCONST char *name;\n\
   %s *kw;\n\
\n\
   if ((len < MIN_KEYWORD_LEN)\n\
       || (len > MAX_KEYWORD_LEN))\n\
     return NULL;\n\
\n\
   hash = %s (str, len);\n\
   if ((hash > MAX_HASH_VALUE) || (hash < MIN_HASH_VALUE))\n\
     return NULL;\n\
\n\
   kw = &%s[hash - MIN_HASH_VALUE];\n\
   if ((NULL != (name = kw->name))\n\
       && (*str == *name)\n\
       && (0 == strcmp (str, name)))\n\
     return kw;\n\
   return NULL;\n\
}\n\
",
	    C_Hash_Table_Type, C_Is_Keyword_Function_Name,
	    C_Hash_Table_Type, C_Hash_Function_Name, C_Hash_Table_Name);
}

static unsigned int hash_function (int *char_map,
				   unsigned char *s, unsigned int len)
{
   unsigned int sum;

   if (Use_Length) sum = len;
   else sum = 0;

   while (len)
     {
	len--;
	sum += (unsigned int) char_map [s[len]];
     }
   return sum;
}

static unsigned int Frequency_Table [256];

static void init_map (int *map)
{
   unsigned int i;

   for (i = 0; i < 256; i++) map [i] = -1;

   for (i = 0; i < Num_Strings; i++)
     {
	unsigned char *s;

	s = (unsigned char *) String_Table[i].name;

	while (*s != 0)
	  {
	     map [*s] = 0;
	     s++;
	  }
     }
}

static void write_map (int *map)
{
   unsigned int i;
   unsigned int min_hash, max_hash;
   char *type;

   min_hash = 0xFFFF;
   max_hash = 0;
   for (i = 0; i < Num_Strings; i++)
     {
	unsigned int h = String_Table[i].hash;
	if (h < min_hash) min_hash = h;
	if (h > max_hash) max_hash = h;
#if 0
	fprintf (stdout, "-->%s\t%u\n", String_Table[i].name, h);
#endif
     }

   fprintf (stdout, "#ifndef SLCONST\n#define SLCONST const\n#endif\n");
   fprintf (stdout, "#define MIN_HASH_VALUE\t%u\n", min_hash);
   fprintf (stdout, "#define MAX_HASH_VALUE\t%u\n", max_hash);
   fprintf (stdout, "#define MIN_KEYWORD_LEN %u\n", Min_String_Len);
   fprintf (stdout, "#define MAX_KEYWORD_LEN %u\n", Max_String_Len);

   for (i = 0; i < 256; i++)
     if (map[i] == -1) map[i] = (max_hash + 1);

   if (max_hash + 1 < 0xFF)
     type = "unsigned char";
   else if (max_hash + 1 < 0xFFFF)
     type = "unsigned short";
   else
     type = "unsigned long";

   fprintf (stderr, "Hash Table is of type %s with max hash %u\n",
	    type, max_hash);

   fprintf (stdout, "\nstatic SLCONST %s %s [256] =\n",
	    type, C_Char_Map_Name);

   fprintf (stdout, "{\n  ");

   for (i = 0; i < 255; i++)
     {
	fprintf (stdout, "%3d, ", map[i]);
	if ((i % 16) == 15)
	  fputs ("\n  ", stdout);
     }
   fprintf (stdout, "%3d\n};\n", map[255]);

   fputs ("\n", stdout);

   fprintf (stdout, "static %s %s (char *s, unsigned int len)\n",
	    type, C_Hash_Function_Name);

fprintf (stdout,"\
{\n\
   unsigned int sum;\n\
\n\
   sum = %s;\n\
   while (len)\n\
     {\n\
	len--;\n\
	sum += (unsigned int) %s [(unsigned char)s[len]];\n\
     }\n\
   return sum;\n\
}\n\
",
	 (Use_Length ? "len" : "0"),
	 C_Char_Map_Name);

   write_table (min_hash, max_hash);
}

static unsigned int compute_hash (unsigned int i)
{
   String_Table_Type *s;
   unsigned int hash;

   s = String_Table + i;
   hash = hash_function (Char_Table_Map, (unsigned char *) s->name, s->len);
   return s->hash = hash;
}

static int tweak_character (unsigned char ch, unsigned int bad)
{
   unsigned int val;
   unsigned int val_save;
   unsigned int i, j;
   unsigned int nvalues;

   val_save = (unsigned int) Char_Table_Map [ch];
   nvalues = Max_Table_Value;

   val = 0;
   while (nvalues)
     {
	val += Tweak_Step_Size;
	val = val % Max_Table_Value;

	Char_Table_Map[ch] = (int) val;

	for (i = 0; i <= bad; i++)
	  {
	     unsigned int hash = compute_hash (i);
	     for (j = 0; j < i; j++)
	       {
		  if (hash == String_Table[j].hash)
		    break;
	       }
	     if (j != i)
	       break;
	  }

	if (i > bad) return 0;
	nvalues--;
     }

   Char_Table_Map [ch] = (int) val_save;
#if 0
   /* reset hashes */
   for (i = 0; i <= bad; i++)
    (void) compute_hash (i);
#endif
   return -1;
}

static void sort_according_to_frequency (unsigned char *s, unsigned int len)
{
   /* since len is small (I hope), I will be lazy */
   unsigned char si, sj;
   unsigned int fi;
   unsigned int i, j, imax;

   imax = len - 1;
   for (i = 0; i < imax; i++)
     {
	si = s[i];
	fi = Frequency_Table [si];

	for (j = i + 1; j < len; j++)
	  {
	     sj = s[j];
	     if (Frequency_Table[sj] < fi)
	       {
		  s[i] = sj;
		  s[j] = si;
		  break;
	       }
	  }
     }
}

static void create_frequency_table (unsigned int num)
{
   unsigned int i;

   memset ((char *) Frequency_Table, 0, sizeof(Frequency_Table));

   for (i = 0; i < num; i++)
     {
	unsigned char *s, ch;

	s = (unsigned char *)String_Table[i].name;
	while (0 != (ch = *s))
	  {
	     Frequency_Table [ch] += 1;
	     s++;
	  }
     }
}

static int tweak_hash_function (unsigned int bad, unsigned int good)
{
   unsigned char unique_chars [256];
   unsigned char bad_chars [256], good_chars[256];
   unsigned char *s;
   unsigned int i, len;

   memset ((char *)unique_chars, 0, sizeof (unique_chars));
   memset ((char *)bad_chars, 0, sizeof (bad_chars));
   memset ((char *)good_chars, 0, sizeof (good_chars));

   s = (unsigned char *) String_Table[bad].name;
   while (*s != 0)
     {
	bad_chars [*s] = 1;
	s++;
     }

   s = (unsigned char *) String_Table[good].name;
   while (*s != 0)
     {
	good_chars [*s] = 1;
	s++;
     }

   /* Find out the characters that are in good or bad, and not both.  That
    * way we are free to manipulate those to avoid the collision.
    */
   len = 0;
   for (i = 0; i < 256; i++)
     {
	if (bad_chars[i])
	  {
	     if (good_chars [i] == 0)
	       unique_chars [len++] = i;
	  }
	else if (good_chars [i])
	  unique_chars [len++] = i;
     }

   /* Unfortunately, the unique_chars may already be part of the words
    * that have already been hashed.  So, sort them according to how often
    * they occur and deal with those that occur the least often first.
    */
#if 1
   create_frequency_table (bad);
#endif
   sort_according_to_frequency (unique_chars, len);

   for (i = 0; i < len; i++)
     {
	unsigned char ch = unique_chars [i];
	if (0 == tweak_character (ch, bad))
	  return 0;
     }

   return -1;
}

static int perfect_hash (void)
{
   unsigned int i, j;
   unsigned int hash;
   int has_collisions = 1;

   for (i = 0; i < Num_Strings; i++)
     {
	hash = compute_hash (i);
	for (j = 0; j < i; j++)
	  {
	     if (hash != String_Table[j].hash)
	       continue;

	     /* Oops.  We have a collision.  tweak_hash_function will
	      * adjust the hash table array to resolve this collision
	      * and ensure that previous ones remain resolved.
	      */
	     if (-1 == tweak_hash_function (i, j))
	       {
		  has_collisions = 1;
		  /* return -1; */
	       }
	     break;
	  }
     }

   if (has_collisions == 0)
     return 0;

   has_collisions = 0;
   /* Now check for collisions */
   for (i = 0; i < Num_Strings; i++)
     {
	char *s = String_Table[i].name;
	hash = compute_hash (i);

	for (j = 0; j < i; j++)
	  {
	     if (hash != String_Table[j].hash)
	       continue;

	     has_collisions++;
	     fprintf (stderr, "Collision: %s, %s\n", s, String_Table [j].name);
	  }
     }

   if (has_collisions)
     return -1;

   return 0;
}

static int sort_function (String_Table_Type *a, String_Table_Type *b)
{
   return b->freq_statistic - a->freq_statistic;
}

static void sort_strings (void)
{
   unsigned int i;
   void (*qsort_fun) (String_Table_Type *, unsigned int,
		      unsigned int, int (*)(String_Table_Type *, String_Table_Type *));

   for (i = 0; i < Num_Strings; i++)
     {
	int f;
	unsigned char *s;

	f = 0;
	s = (unsigned char *) String_Table[i].name;
	while (*s != 0) f += Frequency_Table [*s++];
	String_Table [i].freq_statistic = f;
     }

   qsort_fun = (void (*) (String_Table_Type *, unsigned int,
			  unsigned int,
			  int (*)(String_Table_Type *, String_Table_Type *)))
     qsort;

   qsort_fun (String_Table, Num_Strings, sizeof (String_Table_Type),
	      sort_function);
}

int main (int argc, char **argv)
{
   char line[256];
   unsigned int count;
   unsigned int min_char;
   unsigned int max_char;
   unsigned int min_len;
   unsigned int max_len;
   unsigned int i;

   Tweak_Step_Size = 5;

   if (isatty (0) || (argc > 2))
     {
	fprintf (stderr, "Usage: %s [step-size] < keywords > hash.c\n", argv[0]);
	fprintf (stderr, "  Default step-size is %u\n", Tweak_Step_Size);
	return 1;
     }

   if (argc > 1)
     {
	if (1 != sscanf (argv[1], "%u", &Tweak_Step_Size))
	  Tweak_Step_Size = 5;

	if ((Tweak_Step_Size % 2) == 0) Tweak_Step_Size++;
     }

   count = 0;
   min_char = 255;
   max_char = 0;
   max_len = 0;
   min_len = 0xFFFF;

   while (NULL != fgets (line, sizeof (line), stdin))
     {
	char *s;
	unsigned int len;
	char *type;

	if (*line == '%') continue;

	if (count == 256)
	  {
	     fprintf (stderr, "Only 256 keywords permitted.\n");
	     return 1;
	  }

	len = strlen (line);
	if (len && (line [len - 1] == '\n'))
	  {
	     len--;
	     line [len] = 0;
	  }

	if (len == 0)
	  continue;

	s = malloc (len + 1);
	if (s == NULL)
	  {
	     fprintf (stderr, "Malloc error.\n");
	     return 1;
	  }
	strcpy (s, line);
	String_Table[count].name = s;

	type = s;
	while (*type && (*type != ' ') && (*type != '\t'))
	  type++;

	len = (unsigned int) (type - s);
	String_Table [count].len = len;
	if (len > max_len) max_len = len;
	if (len < min_len) min_len = len;

	if (*type != 0)
	  {
	     *type++ = 0;
	     while ((*type == ' ') || (*type == '\t'))
	       type++;
	  }

	String_Table [count].type = type;

	for (i = 0; i < len; i++)
	  {
	     unsigned char ch;

	     ch = (unsigned char) s[i];

	     if (ch < min_char)
	       min_char = ch;
	     if (ch > max_char)
	       max_char = ch;
	  }

	count++;
     }

   Max_String_Len = max_len;
   Min_String_Len = min_len;

   Num_Strings = count;
   Max_Table_Value = 1;
   while (Max_Table_Value < Num_Strings)
     Max_Table_Value = Max_Table_Value << 1;

   Max_Hash_Value = Max_Table_Value * Max_String_Len;

   fprintf (stderr, "Theoretical Max_Table_Value: %u\n", Max_Table_Value);
   fprintf (stderr, "Theoretical Max_Hash_Value: %u\n", Max_Hash_Value);

   create_frequency_table (Num_Strings);
   sort_strings ();

   init_map (Char_Table_Map);
   if (-1 == perfect_hash ())
     {
	fprintf (stderr, "Hash failed.\n");
	return -1;
     }

   fprintf (stderr, "Success.\n");

   fprintf (stdout, "/* Perfect hash generated by command line:\n *");
   for (i = 0; i < (unsigned int)argc; i++)
     fprintf (stdout, " %s", argv[i]);
   fputs ("\n */\n", stdout);

   write_map (Char_Table_Map);
   return 0;
}

