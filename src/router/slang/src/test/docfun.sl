_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("doc functions");

% Tests go here....

define test_doc_funs ()
{
   variable old = get_doc_files ();
   variable num_old = length (get_doc_files ());
   variable new_file = "/new/doc/file";
   add_doc_file (new_file);
   variable new = get_doc_files ();
   if (length (new) != num_old + 1)
     failed ("add_doc_file after get_doc_files");
   if (new[-1] != new_file)
     failed ("add_doc_file after get_doc_files: new file not found");

   old = get_doc_files ();

   variable list = String_Type[0];
   foreach (["1", "2", "3"])
     {
	variable next_item = ();
	set_doc_files (list);
	!if (_eqs (list, get_doc_files ()))
	  failed ("set_doc_files with %d files", length (list));
	list = [list, next_item];
     }
   variable real_file = "../../doc/text/slangfun.txt";
   if (NULL == get_doc_string_from_file (real_file, "strcat"))
     failed ("expected to find doc for strcat in $real_file"$);
   add_doc_file (real_file);
   if (NULL == get_doc_string_from_file ("strcat"))
     failed ("expected to find doc for strcat in internal list");
}

test_doc_funs ();

print ("Ok\n");

exit (0);

