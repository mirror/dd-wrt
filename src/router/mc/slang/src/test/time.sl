_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("time functions");

#ifexists mktime
static variable t = _time ();
static variable tm = localtime (t);
if (t != mktime (tm))
  failed ("mktime");
tm.tm_sec -= 1;
if (t-1 != mktime (tm))
  failed ("mktime 1 sec earlier");
#endif

static define test_strftime ()
{
   variable tm = localtime (_time ());
   variable fmts = ["%a", "%A", "%b", "%B", "%c", "%C", "%d", "%D", "%e",
		    "%E", "%F", "%G", "%g", "%h", "%H", "%I", "%j", "%k",
		    "%l", "%m", "%M", "%n", "%O", "%p", "%P", "%r", "%R",
		    "%s", "%S", "%t", "%T", "%u", "%U", "%V", "%w", "%W",
		    "%x", "%X", "%y", "%Y", "%z", "%+", "%%"];
   
   % Test only a subset, since many systems do not support all the above
   fmts = ["%a", "%A", "%b", "%B", "%c", "%d",
	   "%H", "%I", "%j", 
	   "%m", "%M", "%p", 
	   "%S", "%U", "%w", "%W",
	   "%x", "%X", "%y", "%Y", "%z", "%%"];

   variable fmt1 = strjoin (fmts, " ");
   variable ans1 = strftime (fmt1, tm);
   variable ans0 = strjoin (array_map (String_Type, &strftime, fmts, tm), " ");
   if (ans0 != ans1)
     {
	failed ("strftime: %s != %s", ans0, ans1);
     }
   ans0 = strftime ("%d", tm);
   ans1 = sprintf ("%02d", tm.tm_mday);
   if (ans1 != ans0)
     failed ("strftime: failed %%d format: %s != %s", ans0, ans1);
}

test_strftime ();
print ("Ok\n");

exit (0);

