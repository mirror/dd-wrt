% Struct functions

private define make_indices (num_dims, d, i)
{
   _for (0, num_dims-1, 1)
     {
	variable j = ();
	if (j == d)
	  i;
	else
	  [:];
     }
}

define struct_filter (s, i)
{
   variable dim = qualifier ("dim");
   variable copy = qualifier_exists ("copy");
   if (copy)
     s = @s;

   variable field;
   foreach field (get_struct_field_names (s))
     {
	variable value = get_struct_field (s, field);
	if (typeof (value) != Array_Type)
	  continue;
	if (dim == NULL)
	  {
	     set_struct_field (s, field, value[i]);
	     continue;
	  }
	variable dims = array_shape (value);
	variable num_dims = length (dims);
	variable d = dim;
	if (d < 0)
	  d += num_dims;

	if ((d < 0) || (d >= num_dims))
	  continue;

	set_struct_field (s, field, value[make_indices(num_dims, d, i)]);
     }
   if (copy)
     return s;
}

define struct_combine ()
{
   variable args = __pop_list (_NARGS);
   variable fields = String_Type[0];
   variable arg;
   foreach arg (args)
     {
	if (arg == NULL)
	  continue;
	if (is_struct_type (arg))
	  arg = get_struct_field_names (arg);
	fields = [fields, arg];
     }

   % Get just the unique names
   variable i, a = Assoc_Type[Int_Type];
   ifnot (length (fields))
     return NULL;
   _for i (0, length (fields)-1, 1)
     a[fields[i]] = i;
   i = assoc_get_values (a);
   fields = fields[i[array_sort (i)]];

   variable s = @Struct_Type (fields);
   foreach arg (args)
     {
	if (0 == is_struct_type (arg))
	  continue;
	foreach (get_struct_field_names (arg))
	  {
	     variable field = ();
	     set_struct_field (s, field, get_struct_field (arg, field));
	  }
     }
   return s;
}

define struct_field_exists (s, field)
{
   return length (where (field == get_struct_field_names (s)));
}

$1 = path_concat (path_dirname (__FILE__), "help/structfuns.hlp");
if (NULL != stat_file ($1))
  add_doc_file ($1);

provide ("structfuns");
