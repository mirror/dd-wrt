require ("glob");

define slsh_get_doc_string (obj)
{
   variable help = get_doc_string_from_file (obj);
   if (help != NULL)
     return help;

   variable path = get_slang_load_path ();
   variable delim = path_get_delimiter ();
   foreach (strtok (path, char (delim)))
     {
	variable dir = ();
	dir = path_concat (dir, "help");
	foreach (glob (path_concat (dir, "*.hlp")))
	  {
	     variable file = ();
	     help = get_doc_string_from_file (file, obj);
	     if (help != NULL)
	       return help;
	  }
     }
   return NULL;
}

