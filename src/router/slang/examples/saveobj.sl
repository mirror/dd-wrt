% This example shows how one can save the values of slang variables to a file
% and then load those values back in another instance of the program.
%
% The following code defines two public functions:
%
%    save_object (FILE, obj, ...);
%    (obj,...) = load_object (FILE);
%
% For example,
%     a = [1:20];
%     b = 2.4;
%     c = struct { d, e }; c.d = 2.7; c.e = "foobar";
%     save_object ("foo.save", a, b, c);
%
% saves the values of the variables a, b, c to a file called "foo.save".
% These values may be retrieved later, e.g., by another program instance
% via:
%     (a,b,c) = load_object ("foo.save");
%
% Caveats:
%
% 1. Not all object types are supported.  The ones supported include:
%
%     All integer types (Int_Type, Char_Type, Long_Type, ...)
%     Float_Type, Double_Type
%     String_Type, BString_Type
%     Null_Type
%
%    as well as the container classes of the above objects:
%     Struct_Type, Array_Type
%
% 2. The algorithm for saving Struct_Type is recursive.  This allows one to
%    save a linked-list of Struct_Type objects.  However, due to the recursive
%    nature of the algorithm and the interpreter's finite stack size, such
%    linked-lists cannot be arbitrarily long.
%
% 3. Objects are saved in the native representation.  As such, the files are
%    not portable across machine architectures.
%
% File Format:
%
% Each slang object is written to the file with the following format
%   Data_Type            (integer)
%   Length of Data Bytes (unsigned integer)
%   Data Bytes
%
% Here, Data Bytes may specify other objects if the parent is a container
% object.

%_debug_info = 1;

private variable Type_Map = Assoc_Type[Integer_Type, -1];
private variable Write_Object_Funs = Assoc_Type[Ref_Type];
private variable Read_Object_Funs = Assoc_Type[Ref_Type];

!if (is_defined ("_Save_Object_Cache_Type"))
typedef struct
{
   index
}
_Save_Object_Cache_Type;

private variable Object_Cache;
private variable Num_Cached;

private define delete_cache ()
{
   Object_Cache = NULL;
   Num_Cached = 0;
}

private define create_cache ()
{
   delete_cache ();
}

% If the object does not need cached, return the object.
% If the object needs cached but does not exist in the cache, cache it and
%   return it.
% Otherwise, the object is in the cache, to return a _Save_Object_Cache_Type
% representing the object.
private define cache_object (obj)
{
   variable t = typeof (obj);

   if ((t != Array_Type)
       and (0 == is_struct_type (obj))
       and (t != BString_Type))
     {
	%vmessage ("not caching %S (type %S)", obj, typeof (obj));
	return obj;
     }

   variable n = Num_Cached;
   variable c = Object_Cache;
   while (n)
     {
	if (__is_same (c.obj, obj))
	  {
	     obj = @_Save_Object_Cache_Type;
	     obj.index = n;
	     return obj;
	  }

	c = c.next;
	n--;
     }

   c = struct {obj, next};
   c.obj = obj;
   c.next = Object_Cache;
   Object_Cache = c;
   Num_Cached++;
   %vmessage ("%S (type %S) added to cache", c.obj, typeof (c.obj));

   return obj;
}

private define get_object_from_cache (index)
{
   variable depth = Num_Cached - index;
   variable c = Object_Cache;
   while (depth)
     {
	c = c.next;
	depth--;
     }
   return c.obj;
}

private define get_type_id (type)
{
   variable id;
   id = Type_Map[string (type)];
   if (id == -1)
     verror ("Object %S is not supported", type);
   return id;
}

private define write_not_implemented (fp, object)
{
   () = fprintf (stderr, "write for object %S not implemented\n", typeof (object));
   return 0;
}

private define do_fwrite (a, fp)
{
   %vmessage ("Writing %S", a);
   variable n = fwrite (a, fp);
   if (n == -1)
     verror ("fwrite failed: %s", errno_string (errno));
   return n;
}

private define do_fread (t, n, fp)
{
   variable b;
   if (n != fread (&b, t, n, fp))
     verror ("fread failed: %s", errno_string (errno));
   %vmessage ("Read %S", b);
   return b;
}

private define do_ftell (fp)
{
   variable pos = ftell (fp);
   if (-1 == pos)
     verror ("ftell failed: %s", errno_string (errno));
   return pos;
}

private define do_fseek (fp, ofs, whence)
{
   if (-1 == fseek (fp, ofs, whence))
     verror ("fseek failed: %s", errno_string (errno));
}

private define sizeof (t)
{
   variable size;

   switch (t)
     { case Char_Type or case UChar_Type: size = 1; }
     { case Int16_Type or case UInt16_Type: size = 2; }
     { case Int32_Type or case UInt32_Type: size = 4; }
     { case Float_Type: size = 4; }
     { case Double_Type: size = 8; }
     {
	verror ("sizeof (%S) not implemented", t);
     }

   return size;
}

private define write_numbers (fp, a)
{
   variable size = sizeof (_typeof (a));
   variable num = do_fwrite (a, fp);
   return num * size;
}

private define read_numbers (fp, t, nbytes)
{
   variable size = sizeof (t);
   nbytes /= size;
   return do_fread (t, nbytes, fp);
}

private define write_string (fp, a)
{
   return do_fwrite (a, fp);
}

private define read_string (fp, t, nbytes)
{
   return do_fread (BString_Type, nbytes, fp);
}

private define start_header (fp, id)
{
   variable len = write_numbers (fp, id);
   variable pos = do_ftell (fp);
   len += write_numbers (fp, 0);	       %  temporary

   variable h = struct
     {
	pos, len
     };
   h.pos = pos;
   h.len = len;

   return h;
}

private define end_header (fp, h, num)
{
   do_fseek (fp, h.pos, SEEK_SET);
   () = do_fwrite (num, fp);
   do_fseek (fp, 0, SEEK_END);
   return h.len + num;
}

private define id_to_datatype (id)
{
   variable keys, values;

   keys = assoc_get_keys (Type_Map);
   values = assoc_get_values (Type_Map);
   variable i = where (values == id);
   !if (length (i))
     verror ("Corrupt file?  Unknown type-id (%d)", id);
   return eval (keys[i][0]);
}

private define write_scalars (fp, a)
{
   variable id = get_type_id (typeof (a));
   variable h = start_header (fp, id);
   variable len = write_numbers (fp, a);
   return end_header (fp, h, len);
}

private define read_null (fp, t, nbytes)
{
   return NULL;
}

private define write_null (fp, a)
{
   return 0;
}

private define write_object ();
private define read_object ();

% Array DataBytes: int num_dims, int dims[num_dims], type, Data...
private define write_array (fp, a)
{
   variable dims, num_dims, data_type;
   (dims, num_dims, data_type) = array_info (a);
   variable len;
   variable id = get_type_id (data_type);

   len = write_numbers (fp, num_dims) + write_numbers (fp, dims)
     + write_numbers (fp, id);

   % For now allow numbers or strings
   if (_typeof(a) == String_Type)
     {
	foreach (a)
	  {
	     variable elem = ();
	     len += write_object (fp, elem);
	  }

	return len;
     }

   len += write_numbers (fp, a);

   return len;
}

private define read_array (fp, type, nbytes)
{
   variable num_dims = do_fread (Int_Type, 1, fp);
   variable dims = do_fread (Int_Type, num_dims, fp);
   type = do_fread (Int_Type, 1, fp);
   variable len;
   len = 1;
   foreach (dims)
     len *= ();

   type = id_to_datatype (type);

   variable v;

   if (type == String_Type)
     {
	v = String_Type [len];
	_for (0,len-1,1)
	  {
	     variable i = ();
	     v[i] = read_object (fp, NULL);
	  }
     }
   else v = do_fread (type, len, fp);

   reshape (v, dims);
   return v;
}

% Data Bytes: int num_fields.  String-Object [num_fields], Values[num_fields]
private define write_struct (fp, a)
{
   variable fields = get_struct_field_names (a);
   variable len = write_numbers (fp, typecast (length (fields), Int_Type));
   foreach (fields)
     {
	variable f = ();
	len += write_object (fp, f);
     }

   foreach (fields)
     {
	f = ();
	len += write_object (fp, get_struct_field (a, f));
     }

   return len;
}

private define read_struct (fp, type, nbytes)
{
   variable num_fields = do_fread (Int_Type, 1, fp);
   variable fields = String_Type[num_fields];
   variable i;
   _for (0, num_fields-1, 1)
     {
	i = ();
	fields[i] = read_object (fp, NULL);
     }

   variable s = @Struct_Type (fields);

   %  make sure it is in the cache in case the fields refer to it.
   if (type != _Save_Object_Cache_Type)
     () = cache_object (s);

   _for (0, num_fields-1, 1)
     {
	i = ();
	set_struct_field (s, fields[i], read_object (fp, NULL));
     }

   return s;
}

% Data Bytes: int index
private define write_cached_object (fp, a)
{
   return write_numbers (fp, a.index);
}

private define read_cached_object (fp, type, nbytes)
{
   variable index = read_numbers (fp, Int_Type, nbytes);
   return get_object_from_cache (index);
}

private define add_type (t, w, r, id)
{
   t = string (t);
   Type_Map[t] = id;
   Write_Object_Funs[t] = w;
   Read_Object_Funs [t] = r;
}

add_type (Char_Type,	&write_numbers,	&read_numbers,	1);
add_type (UChar_Type,	&write_numbers,	&read_numbers,	2);
add_type (Short_Type,	&write_numbers,	&read_numbers,	3);
add_type (UShort_Type,	&write_numbers,	&read_numbers,	4);
add_type (Integer_Type,	&write_numbers,	&read_numbers,	5);
add_type (UInteger_Type,&write_numbers,	&read_numbers,	6);
add_type (Long_Type,	&write_numbers,	&read_numbers,	7);
add_type (ULong_Type,	&write_numbers,	&read_numbers,	8);
add_type (Float_Type,	&write_numbers,	&read_numbers,	9);
add_type (Double_Type,	&write_numbers,	&read_numbers,	10);
add_type (String_Type,	&write_string,	&read_string,	11);
add_type (BString_Type,	&write_string,	&read_string,	12);
add_type (Struct_Type,	&write_struct,	&read_struct,	13);
add_type (Array_Type,	&write_array,	&read_array,	14);
add_type (Null_Type,	&write_null,	&read_null,	15);

add_type (_Save_Object_Cache_Type, &write_cached_object, &read_cached_object, 1000);

private define get_write_function (type)
{
   variable key = string (type);
   if (assoc_key_exists (Write_Object_Funs, key))
     return Write_Object_Funs[key];
   verror ("No write method defined for %S", key);
}

private define get_read_function (type)
{
   variable key = string (type);
   if (assoc_key_exists (Read_Object_Funs, key))
     return Read_Object_Funs[key];
   verror ("No read method defined for %S", key);
}

private define write_object (fp, a)
{
   a = cache_object (a);
   variable id = get_type_id (typeof (a));

   variable h = start_header (fp, id);
   variable f = get_write_function (typeof (a));
   variable num = (@f)(fp, a);
   %vmessage ("Done Writing %S", a);
   return end_header (fp, h, num);
}

private define read_object (fp, statusp)
{
   variable type, nbytes;
   variable status = fread (&type, Integer_Type, 1, fp);
   if (status == -1)
     {
	if (statusp == NULL)
	  verror ("No more objects in file");

	@statusp = 0;
	return 0;
     }

   nbytes = do_fread (Integer_Type, 1, fp);
   type = id_to_datatype (type);

   variable f = get_read_function (type);
   variable v = (@f)(fp, type, nbytes);

   % Necessary because String_Type may get written as BString_Type
   if (type != _Save_Object_Cache_Type)
     {
	v = typecast (v, type);
	() = cache_object (v);
     }

   %vmessage ("Read %S", v);
   if (statusp != NULL)
     @statusp = 1;

   return v;
}

public define save_object ()
{
   if (_NARGS < 2)
     usage ("save_object (file, obj1, ...)");

   variable objs = __pop_args (_NARGS - 1);
   variable file = ();

   variable fp = fopen (file, "w+");
   if (fp == NULL)
     verror ("Unable to open %s: %s", file, errno_string (errno));

   create_cache ();

   foreach (objs)
     {
	variable obj = ().value;
	() = write_object (fp, obj);
     }

   delete_cache ();
}

public define load_object ()
{
   if (_NARGS != 1)
     usage ("(var1,...) = load_object (filename);");
   variable file = ();
   variable fp = fopen (file, "r");
   if (fp == NULL)
     verror ("Unable to open %s: %s", file, errno_string (errno));

   create_cache ();
   forever
     {
	variable status;
	variable obj = read_object (fp, &status);
	if (status == 0)
	  break;
	obj;
     }
   delete_cache ();
}

#ifntrue
% Regression test
private define failed (s, a, b)
{
   vmessage ("Failed: %s: wrote: '%S', read '%S'\n", s, a, b);
}

private define test_eqs ();
private define test_eqs (a, b)
{
   if ((typeof (a) != typeof (b))
       or (_typeof (a) != _typeof (b)))
     {
	failed ("typeof", typeof(a), typeof(b));
	verror ("foo");
	return 0;
     }

   if (typeof (a) != Struct_Type)
     {
	if (length (a) != length (b))
	  {
	     failed ("test_eqs length", a, b);
	     return 0;
	  }

	if (length (where (a != b)))
	  {
	     failed ("test_eqs", a, b);
	     return 0;
	  }
	return 1;
     }

   variable fa, fb;
   fa = get_struct_field_names (a);
   fb = get_struct_field_names (b);

   !if (test_eqs (fa, fb))
     {
	failed ("test_eqs: fa, fb");
	return 0;
     }

   if (length (fa) != length (fb))
     return 0;

   foreach (fa)
     {
	variable name = ();
	variable va, vb;
	va = get_struct_field (a, name);
	vb = get_struct_field (b, name);
	if ((typeof (va) == Struct_Type)
	    and (typeof (vb) == Struct_Type))
	  {
	     % void loop
	     continue;
	  }
	!if (test_eqs (va, vb))
	  return 0;
     }

   return 1;
}

private define test_save_object ()
{
   variable x0 = 1278;
   variable x1 = 2.3;
   variable x2 = "foo";
   variable x3 = struct
     {
	a, b, c, d
     };
   variable x4 = [1:10];
   variable x5 = ["a","b","c","d"];

   x3.a = "foo";
   x3.b = PI;
   x3.c = [1:20];
   x3.d = x3;

   variable x6 = typecast ("foo", BString_Type);
   variable x7 = x6;
   save_object ("foo.sv", x0,x1,x2,x3,x4,x5,x6,x7);

   variable y0,y1,y2,y3,y4,y5,y6,y7;

   (y0,y1,y2,y3,y4,y5,y6,y7) = load_object ("foo.sv");

   !if (test_eqs (x0, y0))
     failed ("x0", x0, y0);
   !if (test_eqs (x1, y1))
     failed ("x1", x1, y1);
   !if (test_eqs (x2, y2))
     failed ("x2", x2, y2);

   !if (test_eqs (x3, y3))
     failed ("x3", x3, y3);

   !if (test_eqs (x4, y4))
     failed ("x4", x4, y4);
   !if (test_eqs (x5, y5))
     failed ("x5", x5, y5);

   !if (test_eqs (x6, y6))
     failed ("x5", x6, y6);
   !if (__is_same (y6,y7))
     failed ("__is_same(y6,y7)",y6,y7);

   vmessage ("Regression Test Done");
}

test_save_object ();
#endif
