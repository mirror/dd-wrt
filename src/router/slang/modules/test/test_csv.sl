require ("csv");

private variable Table = struct {author = {}, title = {}, sample = {}};

private define add_entry (author, title, sample)
{
   list_append (Table.author, author);
   list_append (Table.title, title);
   list_append (Table.sample, sample);
}

add_entry ("Poe, Edgar Allan", "The Raven", "\
Once upon a midnight dreary, while I pondered weak and weary,\n\
Over many a quaint and curious volume of forgotten lore,\n\
While I nodded, nearly napping, suddenly there came a tapping,\n\
As of some one gently rapping, rapping at my chamber door.\n\
\"Tis some visitor,\" I muttered, tapping at my chamber door -\n\
Only this, and nothing more.");

add_entry ("Frost, Robert", "Mending Wall", "\
He moves in darkness as it seems to me,\n\
Not of woods only and the shade of trees.\n\
He will not go behind his father's saying,\n\
And he likes having thought of it so well\n\
He says again, \"Good fences make good neighbors.\"");

define slsh_main ()
{
   variable file = sprintf ("/tmp/testcsv-%ld.csv", _time() mod getpid());
   csv_writecol (file, Table);

   variable names = get_struct_field_names (Table);

   variable table = csv_readcol (file;has_header);

   if (any(names != get_struct_field_names (table)))
     {
	() = fprintf (stderr, "csv_read/write failed to produce a table with the expected column names\n");
	exit (1);
     }

   foreach (names)
     {
	variable name = ();
	ifnot (_eqs(get_struct_field (table, name), get_struct_field (table, name)))
	  {
	     fprintf (stderr, "column %S entries are not equal\n", name);
	     exit (1);
	  }
     }

   table = csv_readcol (file, 1, 3; has_header);
   if (any(names[[1,3]-1] != get_struct_field_names (table)))
     {
	() = fprintf (stderr, "csv_read/write failed to produce a table with the expected column names\n");
	exit (1);
     }
   foreach (get_struct_field_names (table))
     {
	name = ();
	ifnot (_eqs(get_struct_field (table, name), get_struct_field (table, name)))
	  {
	     fprintf (stderr, "column %S entries are not equal\n", name);
	     exit (1);
	  }
     }
   
   () = remove (file);
}

