% These functions were taken from the jed editor

private variable Features;
if (0 == __is_initialized (&Features))
  Features = Assoc_Type [Int_Type,0];

private define pop_feature_namespace (nargs)
{
   variable f, ns = current_namespace ();
   if (nargs == 2)
     ns = ();
   f = ();
   if ((ns == NULL) or (ns == ""))
     ns = "Global";
   return strcat (ns, ".", f);
}

define _featurep ()
{
   variable f;
   f = pop_feature_namespace (_NARGS);
   return Features[f];
}

define provide ()
{
   variable f = pop_feature_namespace (_NARGS);
   Features[f] = 1;
}

define require ()
{
   variable feat, file;
   variable ns = current_namespace ();
   switch (_NARGS)
     {
      case 1:
	feat = ();
	file = feat;
     }
     {
      case 2:
	(feat, ns) = ();
	file = feat;
     }
     {
      case 3:
	(feat, ns, file) = ();
     }
     {
	usage ("require (feature [,namespace [,file]])");
     }

   if (_featurep (feat, ns))
     return;

   if (ns == NULL)
     () = evalfile (file);
   else
     () = evalfile (file, ns);

   if (feat == file)
     provide (file, ns);
   else if (_featurep (feat, ns))
     vmessage ("***Warning: feature %s not provided by %s", feat, file);
}

$1 = path_concat (path_dirname (__FILE__), "help/require.hlp");
if (NULL != stat_file ($1))
  add_doc_file ($1);

