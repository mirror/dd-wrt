define reverse (a)
{
#ifexists array_reverse
   a = @a;
   array_reverse (a);
   return a;
#else
   variable i = length (a);
   if (i <= 1)
     return a;
   
   i--;
   __tmp(a)[[i:0:-1]];
#endif
}

define shift (x, n)
{
   variable len = length(x);
   variable i = [0:len-1];
   
   % allow n to be negative and large
   n = len + n mod len;
   return x[(i + n)mod len];
}

$1 = path_concat (path_dirname (__FILE__), "help/arrayfuns.hlp");
if (NULL != stat_file ($1))
  add_doc_file ($1);

provide ("arrayfuns");
