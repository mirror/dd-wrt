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
   ifnot (len) return x;

   % allow n to be negative and large
   n = len + n mod len;
   return x[[n:n+len-1] mod len];
}

$1 = path_concat (path_dirname (__FILE__), "help/arrayfuns.hlp");
if (NULL != stat_file ($1))
  add_doc_file ($1);

provide ("arrayfuns");
