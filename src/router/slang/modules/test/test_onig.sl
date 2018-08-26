require("./onig.sl");

private define exec (syntax, pattern, astr)
{
   variable reg = onig_new (pattern, ONIG_OPTION_DEFAULT, "ascii", syntax);
   variable r = onig_search (reg, astr);
   if (r)
     {
	variable i;
	for (i = 0; i < r; i++)
	  {
	     variable nth = onig_nth_match (reg, i);
	     () = fprintf(stderr, "%d: (%d-%d)\n", i, nth[0], nth[1]);
	  }
     }
   else
     {
	() = fprintf(stderr, "search fail\n");
     }
}

define slsh_main ()
{

   exec("perl",
	"\\p{XDigit}\\P{XDigit}\\p{^XDigit}\\P{^XDigit}\\p{XDigit}",
	"bgh3a");

   exec("java",
	"\\p{XDigit}\\P{XDigit}[a-c&&b-g]", "xbgc");

   exec("asis",
	"abc def* e+ g?ddd[a-rvvv] (vv){3,7}hv\\dvv(?:aczui ss)\\W\\w$",
	"abc def* e+ g?ddd[a-rvvv] (vv){3,7}hv\\dvv(?:aczui ss)\\W\\w$");

   message ("Supported syntaxes:");
   array_map (Void_Type, &message, onig_get_syntaxes());
   message ("Supported encodings:");
   %array_map (Void_Type, &message, onig_get_encodings());
}
