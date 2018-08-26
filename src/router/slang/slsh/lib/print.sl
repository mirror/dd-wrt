private variable Pager_Rows = 22;
private variable Pager = getenv ("PAGER");
if (Pager == NULL)
  Pager = "more";

% Print Methods
private variable Print_Device_Type = struct
{
   fp,
   printf,
   puts,
   close,
   clientdata
};

% Print to file-pointer method
private define fp_puts_method (p, str)
{
   variable n = fputs (str, p.fp);
   if (n != strbytelen (str))
     return -1;
   return n;
}

private define fp_printf_method ()
{
   variable args = __pop_args (_NARGS-1);
   variable p = ();
   return fp_puts_method (p, sprintf (__push_args(args)));
}
private define fp_close_method (p)
{
   return fclose (p.fp);
}
private define new_fp_print (fp)
{
   variable p = @Print_Device_Type;
   p.fp = fp;
   p.puts = &fp_puts_method;
   p.printf = &fp_printf_method;
   return p;
}

% Print to a pager

#ifexists SIGPIPE
private variable Sigpipe_Handler;
#endif

private define close_pager (fp)
{
   if (fp != NULL)
     () = pclose (fp);
#ifexists SIGPIPE
   signal (SIGPIPE, Sigpipe_Handler);
#endif
}
private define pager_close_method (p)
{
   close_pager (p.fp);
   return 0;
}

private define new_pager_print (cmd)
{
#ifnexists popen
   return NULL;
#else
# ifexists SIGPIPE
   signal (SIGPIPE, SIG_IGN, &Sigpipe_Handler);
# endif
   variable fp = popen (cmd, "w");

   try
     {
	if (fp == NULL)
	  throw OpenError, "Unable to open the pager ($cmd)"$;

# ifexists setvbuf
	() = setvbuf (fp, _IONBF, 0);
# endif
	variable p = new_fp_print (fp);
	p.close = &pager_close_method;
	return p;
     }
   catch AnyError:
     {
	close_pager (fp);
	throw;
     }
#endif
}

% Print to a filename
private define new_file_print (filename)
{
   variable fp = fopen (filename, "w");
   if (fp == NULL)
     throw OpenError, "Unable to open $filename for writing."$;

   variable p = new_fp_print (fp);
   p.close = &fp_close_method;
   p.clientdata = filename;
   return p;
}

% Print to a reference
private define ref_printf_method ()
{
   variable args = __pop_args (_NARGS-1);
   variable p = ();
   p.fp = strcat (p.fp, sprintf (__push_args(args)));
   return 1;
}
private define ref_puts_method (p, str)
{
   p.fp = strcat (p.fp, str);
   return 1;
}
private define ref_close_method (p)
{
   @p.clientdata = p.fp;
   return 0;
}
private define new_ref_print (ref)
{
   variable p = @Print_Device_Type;
   p.fp = "";
   p.printf = &ref_printf_method;
   p.puts = &ref_puts_method;
   p.close = &ref_close_method;
   p.clientdata = ref;
   return p;
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

private define generic_to_string (x)
{
   variable t = typeof (x);

   if ((t == String_Type) or (t == BString_Type))
     return make_printable_string (x);

   return string (x);
}

private define struct_to_string (s, single_line)
{
   if (s == NULL)
     return "NULL";

   variable names = get_struct_field_names (s);
   variable comma = "";
   variable str = "{";
   variable comma_str = ", ";
   if (single_line == 0)
     comma_str = ",\n ";
   foreach (names)
     {
	variable name = ();
	str = strcat (str, comma, name, "=", generic_to_string(get_struct_field (s, name)));
	comma = comma_str;
     }
   return strcat (str, "}");
}

private define struct_to_single_line_string (s)
{
   return struct_to_string (s, 1);
}

private define print_list (a, device)
{
   if (-1 != device.puts ("{\n"))
     {
	variable s;
	foreach s (a)
	  {
	     if (-1 == device.printf ("%s\n", generic_to_string (s)))
	       break;
	  }
	then
	  () = device.puts ("}\n");
     }
}

private define write_2d_array (device, a, to_str)
{
   variable dims = array_shape (a);
   variable nrows = dims[0];
   variable ncols = dims[1];

   _for (0, nrows-1, 1)
     {
	variable i = ();
	_for (0, ncols-1, 1)
	  {
	     variable j = ();
	     if (-1 == device.printf ("%s ", (@to_str)(a[i,j])))
	       return -1;
	  }
	if (-1 == device.puts ("\n"))
	  return -1;
     }
   return 0;
}

private define print_array (a, device)
{
   variable dims, ndims;

   (dims, ndims, ) = array_info (a);
   variable nrows = dims[0];

   try
     {
	variable i, j;
	variable to_str;
	if (_is_struct_type (a))
	  to_str = &struct_to_single_line_string;
	else if (__is_numeric (a))
	  to_str = &string;
	else
	  to_str = &generic_to_string;

	if (ndims == 1)
	  {
	     _for i (0, nrows-1, 1)
	       {
		  if (-1 == device.printf ("%s\n", (@to_str)(a[i])))
		    return;
	       }
	     return;
	  }

	if (ndims == 2)
	  {
	     () = write_2d_array (device, a, to_str);
	     return;
	  }

	nrows = nint(prod(dims[[0:ndims-3]]));
	variable new_dims = [nrows, dims[ndims-2], dims[ndims-1]];
	reshape (a, new_dims);
	_for i (0, nrows-1, 1)
	  {
	     if ((-1 == write_2d_array (device, a[i,*,*], to_str))
		 || (-1 == device.puts ("\n")))
	       return;
	  }
     }
   finally
     {
	reshape (a, dims);
     }
}

define print ()
{
   variable usage_string
     =  ("print (OBJ [,&str|File_Type|Filename]);\n"
	 + "Qualifiers: pager[=pgm], nopager\n");

   if (_NARGS == 0)
     usage (usage_string);

   variable pager_pgm = Pager;
   variable use_pager = -1;	       %  auto

   if (qualifier_exists("nopager"))
     use_pager = 0;
   else if (qualifier_exists ("pager"))
     {
	use_pager = 1;
	pager_pgm = qualifier ("pager");
	if (pager_pgm == NULL)
	  pager_pgm = Pager;
     }

   variable device = NULL;
   if (_NARGS == 2)
     {
	device = ();
	switch (typeof (device))
	  {
	   case File_Type:
	     device = new_fp_print (device);
	  }
	  {
	   case String_Type:
	     device = new_file_print (device);
	  }
	  {
	   case Ref_Type:
	     device = new_ref_print (device);
	  }
	  {
	     usage (usage_string);
	  }
	use_pager = 0;
     }

   variable x = ();
   variable t = typeof (x);
   variable str_x = NULL;

   if (use_pager == -1)
     {
	switch (t)
	  {
	   case Array_Type:
	     variable dims = array_shape (x);
	     use_pager = ((dims[0] > Pager_Rows)
			  || (prod(dims) > 10*Pager_Rows));
	  }
	  {
	   case List_Type:
	     use_pager = length (x) > Pager_Rows;
	  }
	  {
	   case String_Type:
	     use_pager = count_byte_occurrences (x, '\n') > Pager_Rows;
	  }
	  {
	     if (is_struct_type (x))
	       str_x = struct_to_string (x, 0);
	     else
	       str_x = generic_to_string (x);

	     use_pager = (count_byte_occurrences (str_x, '\n') > Pager_Rows);
	  }
     }

   if (use_pager)
     device = new_pager_print (pager_pgm);

   if (device == NULL)
     device = new_fp_print (stdout);

   try
     {
	if (t == Array_Type)
	  return print_array (x, device);

	if (t == List_Type)
	  return print_list (x, device);

	if ((t == String_Type) && use_pager)
	  return device.puts (x);

	if (str_x != NULL)
	  x = str_x;
	else if (is_struct_type (x))
	  x = struct_to_string (x, 0);
	else
	  x = generic_to_string (x);

	if (-1 != device.puts (x))
	  {
	     () = device.puts ("\n");
	  }
     }
   finally
     {
	if (device.close != NULL)
	  () = device.close ();
     }
}

define print_set_pager (pager)
{
   Pager = pager;
}

define print_set_pager_lines (n)
{
   Pager_Rows = n;
}

