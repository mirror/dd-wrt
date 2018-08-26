% The code in this file implements completion callbacks for the slang
% reaadline code.  It may also be used by other slang applications.
%
% Version 0.1.0
require ("glob");

private variable Word_Bytes = Char_Type[256];
Word_Bytes[[['0':'9'], ['A':'Z'], ['a':'z'], '_', [128:255]]] = 1;
Word_Bytes['.'] = 1;

private variable File_Bytes = @Word_Bytes;
File_Bytes[['-', '/', '.', '+', '#', '~']] = 1;

private define filename_completions (partial_word)
{
   variable completions = glob(partial_word+"*");
   if ((partial_word == "..") || (partial_word == "."))
     completions = ["..", completions];

   variable n = length (completions);
   _for (0, n-1, 1)
     {
	variable i = ();
	variable file = completions[i];
	variable st = stat_file (file);
	if ((st != NULL) && (stat_is ("dir", st.st_mode)))
	  completions[i] = path_concat (file, "");
     }
   return completions;
}

private define struct_completion (partial_word)
{
   variable
     words = strchop (partial_word, '.', 0),
     completion = words[0],
     completions = String_Type[0],
     s = __get_reference (completion),
     i, fields, word;

   if ((s == NULL) || (0 == __is_initialized(s)))
     return completions;

   s = @s;
   if (0 == is_struct_type (s))
     return completions;

   _for i (1, length(words)-2, 1)
     {
	fields = get_struct_field_names (s);
	word = words[i];
	ifnot (any (fields == word))
	  return completions;
	s = get_struct_field (s, word);
	if (0 == is_struct_type (s))
	  return completions;

	completion = strcat (completion, ".", word);
     }
   fields = get_struct_field_names (s);
   word = words[-1];
   i = where (0 == strnbytecmp(fields, word, strbytelen(word)));
   fields = fields[i];
   if (length (fields) == 0)
     return completions;
   return completion + "." + fields;
}

private define compute_completions (line, point, partial_wordp, posp)
{
   variable i = 0;
   variable instr = 0, inchar = 0;
   variable istart = NULL;
   variable word_bytes = Word_Bytes;

   if (line[0] == '!')
     {
	word_bytes = File_Bytes;
     }

   while (i < point)
     {
	variable ch = line[i]; i++;

	if (word_bytes[ch])
	  {
	     if (istart == NULL)
	       istart = i-1;
	     continue;
	  }

	if (ch == '"')
	  {
	     if (inchar)
	       continue;
	     istart = i;
	     instr = not instr;
	     continue;
	  }
	if (ch == '\'')
	  {
	     if (instr)
	       continue;
	     inchar = not (inchar);
	     continue;
	  }
	if (ch == '\\')
	  {
	     i++;
	     continue;
	  }
	if (inchar || instr)
	  continue;
	istart = NULL;
     }

   if (istart == NULL)
     istart = point;

   if (istart > point)
     return NULL;

   variable partial_word = line[[istart:point-1]];

   % It would be useful here to allow this to be customized via, e.g.,
   % return completion_hook (line, partial_word, istart, in_string)

   variable completions;
   if (instr || (line[0] == '!'))      %  !shell-escape
     completions = filename_completions (partial_word);
   else if (is_substr (partial_word, "."))
     completions = struct_completion (partial_word);
   else
     {
	variable c1 = _apropos ("Global", "^"+partial_word, 0xFF);
	variable c2 = _apropos ("", "^"+partial_word, 0xFF);
	if (c1 == NULL)
	  completions = c2;
	else if (c2 == NULL)
	  completions = c1;
	else
	  completions = [c1, c2];
     }

   @posp = istart;
   @partial_wordp = partial_word;

   return completions[array_sort(completions)];
}

private define completion_callback (line, point)
{
   variable completions, partial_word, pos;
   completions = compute_completions (line, point, &partial_word, &pos);

   if (completions == NULL)
     return String_Type[0], 0;

   return completions, pos;
}

private define list_completions (completions)
{
   variable max_len = 1 + max (strlen(completions));
   variable max_cols = rline_get_edit_width ();

   if (max_cols < 1)
     max_cols = 80;

   if (max_len > max_cols)
     max_len = max_cols;

   variable ncols = max_cols/max_len;
   variable fmt = sprintf ("%%-%ds", max_len);
   variable col = 0;
   () = fputs ("\r\n", stdout);
   foreach (completions)
     {
	variable word = ();
	() = fprintf (stdout, fmt, word);
	col++;
	if (col == ncols)
	  {
	     () = fputs ("\r\n", stdout);
	     col = 0;
	  }
     }
   if (col)
     () = fputs ("\r\n", stdout);
}

rline_set_list_completions_callback (&list_completions);
rline_set_completion_callback (&completion_callback);
