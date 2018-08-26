_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("slprep");

public variable X = 0;

#ifdef FOO_MOO_TOO_KOO
failed ("ifdef");
#else
X = 1;
#endif
#if (X!=1)
failed ("X!=1");
#else
X=-1;
#endif

#if !eval(X==-1)
failed ("eval");
#else

#<ignore>
failed ("#<ignore>");
#</ignore>

define gbar ();
static define sbar ();
if ((0 == is_defined ("gbar")) || (is_defined ("sbar")))
{
   failed ("is_defined(bar)");
}
#ifnexists gbar
failed ("gbar does not exist");
#endif
#ifnexists sbar
failed ("sbar does not exist");
#endif
#if (NULL == __get_reference("gbar"))
failed ("No preprocessor reference to gbar");
#endif
if (NULL == __get_reference ("sbar"))
{
   failed ("no reference to sbar");
}

%Currently, sbar is effectively private and not accessable from the
%preprocessor since it has a different private namespace.  This may be
%be changed in a future version so that the prep shares the uses the
%file's private namespace.
%#if (NULL == __get_reference("sbar"))
%failed ("No preprocessor reference to sbar");
%#endif

implements ("foo");
define sfoo ();
if (is_defined ("sfoo"))
{
   failed ("is_defined(sfoo)");	       %  is_defined looks at global namespace
}
#ifnexists sfoo
failed ("sfoo does not exist");
#endif
#if (NULL == __get_reference("sfoo"))
failed ("No preprocessor reference to sfoo");
#endif

print ("Ok\n");

exit (0);
#endif
