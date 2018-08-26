import("pcre");

define pcre_matches ()
{
   variable nargs = _NARGS;
   if (nargs < 2)
     usage ("\
strings = pcre_matches (regexp, str [,pcre_exec_options])\n\
Qualifiers:\n\
  options=0       pcre_compile options if regexp is a string\n\
  offset=0        pcre_exec start matching offset in bytes\n\
";
	   );
   variable re, str, options;
   if (nargs == 2)
     0;
   (re, str, options) = ();
   variable pos = qualifier ("offset", 0);
   if (typeof (re) != PCRE_Type)
     {
	variable compile_options = qualifier ("options", 0);
	re = pcre_compile (re, options);
     }

   variable n = pcre_exec (re, str, pos, options);
   if (n == 0)
     return NULL;

   variable matches = String_Type[n];
   _for (0, n-1, 1)
     {
	variable i = ();
	matches[i] = pcre_nth_substr (re, str, i);
     }
   return matches;
}

$1 = path_concat (path_dirname (__FILE__), "help/pcrefuns.hlp");
if (NULL != stat_file ($1))
  add_doc_file ($1);

provide("pcre");
