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

print ("Ok\n");

exit (0);
#endif
