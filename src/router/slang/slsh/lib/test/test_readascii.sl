require("readascii");

private define make_xyz_data (xmin, xmax, n)
{
   variable x = [xmin:xmax:#n];
   variable y = cos (x);
   variable z = sin (x);

   return x, y, z;
}

private define make_lines_array (x, y, z, delim)
{
   variable n = length (x);
   variable fmt = strcat (strjoin (["%S", "%S", "%S"], delim), "\n");
   return array_map (String_Type, &sprintf, fmt, x, y, z);
}

private define make_lines_list (x, y, z, delim)
{
   variable list = {};
   foreach (make_lines_array (x, y, z, delim))
     {
	variable line = ();
	list_append (list, line);
     }
   return list;
}

private define concat_lines ()
{
   variable args = __pop_list (_NARGS);
   if (typeof (args[0]) == Array_Type)
     return [__push_list(args)];

   variable list = args[0];
   _for (1, _NARGS-1, 1)
     {
	variable i = ();
	variable item = args[i];
	if (typeof (item) != List_Type)
	  {
	     list_append (list, item);
	     continue;
	  }
	while (length (item))
	  list_append (list, list_pop (item));
     }
   return list;
}

private define run_test (make_lines_func)
{
   variable xmin = 1.0, xmax = 5.0, n = 20;

   variable x, y, z;
   (x, y, z) = make_xyz_data (xmin, xmax, n);
   variable delim = ",";

   variable lines = (@make_lines_func) (x, y, z, ",");
   variable xa, ya, za, na;
   variable lastline, lastlinenum;

   na = readascii (lines, &xa, &ya, &za; delim=",", lastlinenum=&lastlinenum);
   if (n != na)
     throw RunTimeError, "readascii returned $na, not $n as expected"$;
   if (lastlinenum != n)
     throw RunTimeError, "lastlinenum=$lastlinenum instead of $n"$;
   if (any (fneqs(x, xa) or fneqs(y, ya) or fneqs(z, za)))
     throw RunTimeError, "Arrays have unequal values";

   variable x1, y1, z1, n1 = 2;
   (x1, y1, z1) = make_xyz_data (xmax, 2*xmax, n1);
   lines = concat_lines (lines, "@\n", (@make_lines_func)(x1, y1, z1, delim));

   na = readascii (lines, &xa, &ya, &za; delim=",", lastlinenum=&lastlinenum);
   if (n+n1 != na)
     throw RunTimeError, "readascii returned $na, not $n1 + $n2 as expected"$;
   if (lastlinenum != length (lines))
     throw RunTimeError, "lastlinenum=$lastlinenum instead of $n"$;
   if (any (fneqs([x,x1], xa) or fneqs([y,y1], ya) or fneqs([z,z1], za)))
     throw RunTimeError, "Arrays have unequal values";

   na = readascii (lines, &xa, &ya, &za; delim=",", lastlinenum=&lastlinenum,
		   lastline=&lastline, stop_on_mismatch);
   if (n != na)
     throw RunTimeError, "readascii returned $na, not $n as expected"$;
   if (lastlinenum != n+1)
     throw RunTimeError, "lastlinenum=$lastlinenum instead of $n+1"$;
   if (any (fneqs(x, xa) or fneqs(y, ya) or fneqs(z, za)))
     throw RunTimeError, "Arrays have unequal values";
   if (lastline != "@\n")
     throw RunTimeError, "lastline=$lastline instead of @\n";

   lines = concat_lines (lines, "%%%%%%\n");

   na = readascii (lines, &xa, &ya, &za; delim=",",
		   lastlinenum=&lastlinenum, lastline=&lastline,
		   comment="@", stop_on_mismatch);
   if (n+n1 != na)
     throw RunTimeError, "readascii returned $na, not return $n+$n1 as expected"$;
   if (lastlinenum != length (lines))
     throw RunTimeError, "lastlinenum=$lastlinenum instead of $n"$;
   if (any (fneqs([x,x1], xa) or fneqs([y,y1], ya) or fneqs([z,z1], za)))
     throw RunTimeError, "Arrays have unequal values";
   if (lastline != lines[-1])
     throw RunTimeError, "lastline=$lastline instead of lines[-1]\n";

   na = readascii (lines, &za, &xa; delim=",", cols=[1,3],
		   lastlinenum=&lastlinenum, lastline=&lastline,
		   comment="@", stop_on_mismatch);
   if (n+n1 != na)
     throw RunTimeError, "readascii returned $na, not return $n+$n1 as expected"$;
   if (lastlinenum != length (lines))
     throw RunTimeError, "lastlinenum=$lastlinenum instead of $n"$;
   if (any (fneqs([x,x1], za) or fneqs([z,z1], xa)))
     throw RunTimeError, "Arrays have unequal values";
   if (lastline != lines[-1])
     throw RunTimeError, "lastline=$lastline instead of lines[-1]\n";

   na = readascii (lines, &za, &xa; delim=",", as_list, cols=[1,3],
		   lastlinenum=&lastlinenum, lastline=&lastline,
		   comment="@", stop_on_mismatch);
   if (n+n1 != na)
     throw RunTimeError, "readascii returned $na, not return $n+$n1 as expected"$;
   if (lastlinenum != length (lines))
     throw RunTimeError, "lastlinenum=$lastlinenum instead of $n"$;
   if ((typeof (za) != List_Type) || (typeof (xa) != List_Type))
     throw RunTimeError, "Used as_list qualifier but did not get a list, got $za,$xa"$;
   if (any (fneqs([x,x1], [__push_list(za)])
	    or fneqs([z,z1], [__push_list(xa)] )))
     throw RunTimeError, "Arrays have unequal values";
   if (lastline != lines[-1])
     throw RunTimeError, "lastline=$lastline instead of lines[-1]\n";
}

define slsh_main ()
{
   run_test (&make_lines_array);
   run_test (&make_lines_list);
}
