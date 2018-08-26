\datatype{Assoc_Type}
\synopsis{An associative array or hash type}
\description
  An \dtype{Assoc_Type} object is like an array except that it is
  indexed using strings and not integers.  Unlike an \dtype{Array_Type}
  object, the size of an associative array is not fixed, but grows as
  objects are added to the array.  Another difference is that ordinary
  arrays represent ordered object; however, the ordering of the
  elements of an \var{Assoc_Type} object is unspecified.

  An \dtype{Assoc_Type} object whose elements are of some data-type
  \exmp{d} may be created using using
#v+
    A = Assoc_Type[d];
#v-
  For example,
#v+
    A = Assoc_Type[Int_Type];
#v-
  will create an associative array of integers.  To create an
  associative array capable of storing an arbitrary type, use the form
#v+
    A = Assoc_Type[];
#v-

  An optional parameter may be used to specify a default value for
  array elements.  For example,
#v+
   A = Assoc_Type[Int_Type, -1];
#v-
  creates an integer-valued associative array with a default element
  value of -1.  Then \exmp{A["foo"]} will return -1 if the key
  \exmp{"foo"} does not exist in the array.  Default values are
  available only if the type was specified when the associative array
  was created.

  The following functions may be used with associative arrays:
#v+
    assoc_get_keys
    assoc_get_values
    assoc_key_exists
    assoc_delete_key
#v-
  The \ifun{length} function may be used to obtain the number of
  elements in the array.

  The \var{foreach} construct may be used with associative arrays via
  one of the following forms:
#v+
      foreach k,v (A) {...}
      foreach k (A) using ("keys") { ... }
      foreach v (A) using ("values") { ... }
      foreach k,v (A) using ("keys", "values") { ... }
#v-
  In all the above forms, the loop is over all elements of the array
  such that \exmp{v=A[k]}.
\seealso{List_Type, Array_Type, Struct_Type}
\done

\datatype{List_Type}
\synopsis{A list object}
\description
  An object of type \var{List_Type} represents a list, which is
  defined as an ordered heterogeneous collection of objects.
  A list may be created using, e.g.,
#v+
    empty_list = {};
    list_with_4_items = {[1:10], "three", 9, {1,2,3}};
#v-
  Note that the last item of the list in the last example is also a
  list.  A List_Type object may be manipulated by the following
  functions:
#v+
    list_new
    list_insert
    list_append
    list_delete
    list_reverse
    list_pop
#v-
  A \var{List_Type} object may be indexed using an array syntax with
  the first item on the list given by an index of 0.  The
  \ifun{length} function may be used to obtain the number of elements
  in the list.

  A copy of the list may be created using the @ operator, e.g.,
  \exmp{copy = @list}.

  The \kw{foreach} statement may be used with a \dtype{List_Type}
  object to loop over its elements:
#v+
    foreach elem (list) {....}
#v-
\seealso{Array_Type, Assoc_Type, Struct_Type}
\done

\datatype{String_Type}
\synopsis{A string object}
\description
  An object of type \var{String_Type} represents a string of bytes or
  characters, which in general have different semantics depending upon
  the UTF-8 mode.

  The string obeys byte-semantics when indexed as an
  array.  That is, \exmp{S[0]} will return the first byte of the
  string \exmp{S}.  For character semantics, the nth character in the
  string may be obtained using \ivar{substr} function.

  The \kw{foreach} statement may be used with a \dtype{String_Type}
  object \exmp{S} to loop over its bytes:
#v+
    foreach b (S) {....}
    foreach b (S) using ("bytes") {....}
#v-
  To loop over its characters, the following form may be used:
#v+
    foreach c (S) using ("chars") {...}
#v-
  When UTF-8 mode is not in effect, the byte and character forms will
  produce the same sequence.  Otherwise, the string will be decoded
  to generate the (wide) character sequence.  If the string contains
  an invalid UTF-8 encoded character, successive bytes of the invalid
  sequence will be returned as negative integers.  For example,
  \exmp{"a\\xAB\\x{AB}"} specifies a string composed of the character
  \exmp{a}, a byte \exmp{0xAB}, and the character \exmp{0xAB}.  In
  this case,
#v+
     foreach c ("a\xAB\x{AB}") {...}
#v-
  will produce the integer-valued sequence \exmp{'a', -0xAB, 0xAB}.

\seealso{Array_Type, _slang_utf8_ok}
\done

\datatype{Struct_Type}
\synopsis{A structure datatype}
\description
  A \dtype{Struct_Type} object with fields \exmp{f1}, \exmp{f2},...,
  \exmp{fN} may be created using
#v+
    s = struct { f1, f2, ..., fN };
#v-
  The fields may be accessed via the "dot" operator, e.g.,
#v+
     s.f1 = 3;
     if (s12.f1 == 4) s.f1++;
#v-
  By default, all fields will be initialized to \NULL.

  A structure may also be created using the dereference operator (@):
#v+
    s = @Struct_Type ("f1", "f2", ..., "fN");
    s = @Struct_Type ( ["f1", "f2", ..., "fN"] );
#v-
  Functions for manipulating structure fields include:
#v+
     _push_struct_field_values
     get_struct_field
     get_struct_field_names
     set_struct_field
     set_struct_fields
#v-

  The \kw{foreach} loop may be used to loop over elements of a linked
  list.  Suppose that first structure in the list is called
  \exmp{root}, and that the \exmp{child} field is used to form the
  chain.  Then one may walk the list using:
#v+
     foreach s (root) using ("child")
      {
         % s will take on successive values in the list
          .
          .
      }
#v-
  The loop will terminate when the last elements \exmp{child} field is
  NULL.  If no ``linking'' field is specified, the field name will
  default to \exmp{next}.

  User-defined data types are similar to the \var{Struct_Type}.  A
  type, e.g., \exmp{Vector_Type} may be created using:
#v+
    typedef struct { x, y, z } Vector_Type;
#v-
  Objects of this type may be created via the @ operator, e.g.,
#v+
    v = @Vector_Type;
#v-
  It is recommended that this be used in a function for creating such
  types, e.g.,
#v+
    define vector (x, y, z)
    {
       variable v = @Vector_Type;
       v.x = x;
       v.y = y;
       v.z = z;
       return v;
    }
#v-
  The action of the binary and unary operators may be defined for such
  types.  Consider the "+" operator.  First define a function for
  adding two \exmp{Vector_Type} objects:
#v+
    static define vector_add (v1, v2)
    {
       return vector (v1.x+v2.x, v1.y+v2.y, v1.z, v2.z);
    }
#v-
  Then use
#v+
    __add_binary ("+", Vector_Type, &vector_add, Vector_Type, Vector_Type);
#v-
  to indicate that the function is to be called whenever the "+"
  binary operation between two \exmp{Vector_Type} objects takes place,
  e.g.,
#v+
    V1 = vector (1, 2, 3);
    V2 = vector (8, 9, 1);
    V3 = V1 + V2;
#v-
  will assigned the vector (9, 11, 4) to \exmp{V3}.  Similarly, the
  \exmp{"*"} operator between scalars and vectors may be defined using:
#v+
    static define vector_scalar_mul (v, a)
    {
       return vector (a*v.x, a*v.y, a*v.z);
    }
    static define scalar_vector_mul (a, v)
    {
       return vector_scalar_mul (v, a);
    }
    __add_binary ("*", Vector_Type, &scalar_vector_mul, Any_Type, Vector_Type);
    __add_binary ("*", Vector_Type, &vector_scalar_mul, Vector_Type, Any_Type);
#v-
  Related functions include:
#v+
    __add_unary
    __add_string
    __add_destroy
#v-
\seealso{List_Type, Assoc_Type}
\done

\datatype{File_Type}
\synopsis{A type representing a C stdio object}
\description
  An \dtype{File_Type} is the interpreter's representation of a C
  stdio FILE object and is usually created using the \ifun{fopen}
  function, i.e.,
#v+
    fp = fopen ("file.dat", "r");
#v-
  Functions that utilize the \dtype{File_Type} include:
#v+
    fopen
    fclose
    fgets
    fputs
    ferror
    feof
    fflush
    fprintf
    fseek
    ftell
    fread
    fwrite
    fread_bytes
#v-
  The \var{foreach} construct may be used with \dtype{File_Type}
  objects via one of the following forms:
#v+
   foreach line (fp) {...}
   foreach byte (A) using ("char") { ... }   % read bytes
   foreach line (A) using ("line") { ... }   % read lines (default)
   foreach line (A) using ("wsline") { ... } % whitespace stripped from lines
#v-
\seealso{List_Type, Array_Type, Struct_Type}
\done

