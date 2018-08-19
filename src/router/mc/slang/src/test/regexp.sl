_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("regexp");

% Tests go here....

static define test_regexp_match (pat, str, val)
{
   variable val1 = string_match (str, pat, 1);
   if (val != val1)
     failed ("string_match(%S,%S) ==> %S, not %S", str, pat, val1, val);

   variable ofs = 1+strbytelen (str);
   str = strcat (str, str);

   val1 = string_match (str, pat, ofs);
   if (val1)
     val += ofs-1;

   if (val1 != val)
     failed ("string_match($str,$pat,$ofs) ==> $val1, not $val"$);
}

test_regexp_match ("B.*[1-5]", "0xAB123G", 4);
test_regexp_match ("\([1-5]+\)G\1"R, "0xAB123G12F", 0);
test_regexp_match ("\([1-5]+\)G\1"R, "0xAB123G123F", 5);
test_regexp_match ("[1-5]", "0xAB123G", 5);
test_regexp_match ("G", "0xAB123G", 8);

static define test_globbing (glob, re)
{
   variable pat = glob_to_regexp (glob);
   if (re != pat)
     failed ("glob_to_regexp (%s) produced %s, expected %s", 
	     glob, pat, re);
}

#iffalse
test_globbing ("*.c", "^[^.].*\\.c$");
#else
test_globbing ("*.c", "^.*\\.c$");
#endif
test_globbing ("[*].c", "^[*]\\.c$");
test_globbing ("x+??$.$", "^x\\+..\\$\\.\\$$");
test_globbing ("x+[file$", "^x\\+\\[file\\$$");
test_globbing ("x+[^$", "^x\\+\\[^\\$$");
test_globbing ("x+[^]$", "^x\\+\\[^]\\$$");
test_globbing ("x+[^]]$", "^x\\+[^]]\\$$");
test_globbing ("[", "^\\[$");
test_globbing ("x[", "^x\\[$");
test_globbing ("x[]", "^x\\[]$");
test_globbing ("x[]]", "^x[]]$");
test_globbing ("x\\[]]", "^x\\\\[]]$");

print ("Ok\n");

exit (0);

