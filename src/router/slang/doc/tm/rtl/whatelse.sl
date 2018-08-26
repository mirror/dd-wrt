#!/usr/bin/env slsh
% -*- slang -*-

% This file is used to determine what functions still need documenting.
% I think that it provides a good example of the use of associative arrays.

_debug_info = 1;

variable Src_Files = "../../../src/*.c";
variable TM_Files = "*.tm";
variable Unwanted_Files = "../../../src/calc.c";

define grep (pat, files)
{
   if (strlen (files) == 0)
     return String_Type[0];

   variable fp = popen ("rgrep '$pat' $files"$, "r");
   variable matches;

   matches = fgetslines (fp);
   () = pclose (fp);

   return matches;
}

static define prune_array (a, b)
{
   foreach (b) using ("keys")
     {
	variable k = ();
	if (0 == assoc_key_exists (a, k))
	  continue;
	assoc_delete_key (a, k);
	b[k] = 0;
     }
}

define get_with_pattern (a, pat, white)
{
   variable f;

   foreach (grep (pat, Src_Files))
     {
	f = ();

	f = strtok (f, white)[1];
	a [f] = 1;
     }

   if (Unwanted_Files != NULL) foreach (grep (pat, Unwanted_Files))
     {
	f = ();
	f = strtok (f, white)[1];
	assoc_delete_key (a, f);
     }
}

define get_src_intrinsics ()
{
   variable f;
   variable src = Assoc_Type[Int_Type];

   get_with_pattern (src, "^[ \t]+MAKE_INTRINSIC.*(\".*\"", "\"");
   get_with_pattern (src, "^[ \t]+MAKE_MATH_UNARY.*(\".*\"", "\"");
   get_with_pattern (src, "^[ \t]+MAKE_APP_UNARY.*(\".*\"", "\"");
   get_with_pattern (src, "^[ \t]+MAKE_ARITH_UNARY.*(\".*\"", "\"");
   get_with_pattern (src, "^[ \t]+MAKE_ARITH_BINARY.*(\".*\"", "\"");
   get_with_pattern (src, "^[ \t]+MAKE_VARIABLE.*(\".*\"", "\"");
   get_with_pattern (src, "^[ \t]+MAKE_DCONSTANT.*(\".*\"", "\"");
   get_with_pattern (src, "^[ \t]+MAKE_FCONSTANT.*(\".*\"", "\"");
   get_with_pattern (src, "^[ \t]+MAKE_ICONSTANT.*(\".*\"", "\"");
   get_with_pattern (src, "^[ \t]+MAKE_LLCONSTANT.*(\".*\"", "\"");

   get_with_pattern (src, "SLclass_allocate_class[ \t]+(\"", "\"");
   get_with_pattern (src, "SLadd_intrinsic[^(]+(\"", "\"");

   return src;
}

define get_doc_intrinsics ()
{
   variable funs;
   variable doc = Assoc_Type[Int_Type];

   funs = grep ("^\\\\function{", TM_Files);
   foreach (funs)
     {
	variable f;
	f = ();
	f = strtok (f, "{}")[1];
	doc [f] = 1;
     }
   funs = grep ("^\\\\variable{", TM_Files);
   foreach (funs)
     {
	f = ();
	f = strtok (f, "{}")[1];
	doc [f] = 1;
     }
   funs = grep ("^\\\\datatype{", TM_Files);
   foreach (funs)
     {
	f = ();
	f = strtok (f, "{}")[1];
	doc [f] = 1;
     }
   return doc;
}

define slsh_main ()
{
   variable k;
   variable src, doc;

   doc = get_doc_intrinsics ();
   src = get_src_intrinsics ();

   prune_array (src, doc);

   k = assoc_get_keys (src);
   k = k[array_sort(k)];

   foreach (k)
     {
	message ();
     }

   k = assoc_get_keys (doc);
   variable v = assoc_get_values (doc);
   variable i = where (v);
   if (length (i))
     {
	message ("\n***The following are documented but unknown***");
	foreach (k[i])
	  message ();
     }
}

