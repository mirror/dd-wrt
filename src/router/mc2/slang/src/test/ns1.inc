#ifnexists Testing_Line_Token
implements (This_Namespace);
private variable NS = This_Namespace;
#else
private variable NS = current_namespace ();
if (path_basename (__FILE__) != "ns1.inc")
  failed ("__FILE__ is %s and not %s", __FILE__, "ns1.inc");
#endif

define func ()
{
   return NS;
}
