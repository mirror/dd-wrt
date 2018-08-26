\function{__add_binary}
\synopsis{Extend a binary operation to a user defined type}
\usage{__add_binary(op, return_type, binary_funct, lhs_type, rhs_type)}
#v+
   String_Type op;
   Ref_Type binary_funct;
   DataType_Type return_type, lhs_type, rhs_type;
#v-
\description
  The \ifun{__add_binary} function is used to specify a function to be
  called when a binary operation takes place between specified data
  types.  The first parameter indicates the binary operator and must
  be one of the following:
#v+
   "+", "-", "*", "/", "==", "!=", ">", ">=", "<", "<=", "^",
   "or", "and", "&", "|", "xor", "shl", "shr", "mod"
#v-
  The second parameter (\exmp{binary_funct}) specifies the function to
  be called when the binary function takes place between the
  types \exmp{lhs_type} and \exmp{rhs_type}.  The \exmp{return_type}
  parameter stipulates the return values of the function and the data
  type of the result of the binary operation.

  The data type for \exmp{lhs_type} or \exmp{rhs_type} may be left
  unspecified by using \dtype{Any_Type} for either of these values.
  However, at least one of the parameters must correspond to a
  user-defined datatype.
\example
  This example defines a vector data type and extends the "*" operator
  to the new type:
#v+
    typedef struct { x, y, z } Vector_Type;
    define vector (x, y, z)
    {
       variable v = @Vector_Type;
       v.x = x;
       v.y = y;
       v.z = z;
       return v;
    }
    static define vector_scalar_mul (v, a)
    {
       return vector (a*v.x, a*v.y, a*v.z);
    }
    static define scalar_vector_mul (a, v)
    {
       return vector_scalar_mul (v, a);
    }
    static define dotprod (v1,v2)
    {
       return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
    }
    __add_binary ("*", Vector_Type, &scalar_vector_mul, Any_Type, Vector_Type);
    __add_binary ("*", Vector_Type, &scalar_vector_mul, Any_Type, Vector_Type);
    __add_binary ("*", Double_Type, &dotprod, Vector_Type, Vector_Type);
#v-
\seealso{__add_unary, __add_string, __add_destroy}
\done

\function{__add_string}
\synopsis{Specify a string representation for a user-defined type}
\usage{__add_string (DataType_Type user_type, Ref_Type func)}
\description
  The \ifun{__add_string} function specifies a function to be called
  when a string representation is required for the specified
  user-defined datatype.
\example
  Consider the \exmp{Vector_Type} object defined in the example
  for the \ifun{__add_binary} function.
#v+
     static define vector_string (v)
     {
        return sprintf ("[%S,%S,%S]", v.x, v.y, v.z);
     }
     __add_string (Vector_Type, &vector_string);
#v-
  Then
#v+
     v = vector (3, 4, 5);
     vmessage ("v=%S", v);
#v-
  will generate the message:
#v+
     v=[3,4,5]
#v-
\seealso{__add_unary, __add_binary, __add_destroy, __add_typecast}
\done

\function{__add_typecast}
\synopsis{Add a typecast-function for a user-defined type}
\usage{__add_typecast (DataType_Type user_type, DataType_Type totype, Ref_Type func)}
\description
  The \ifun{__add_typecast} function specifies a function to be called
  to typecast the user-defined type to an object of type
  \exmp{totype}.  The function must be defined to take a single
  argument (the user-type to be converted) and must return an object
  of type \exmp{totype}.
\seealso{__add_unary, __add_binary, __add_destroy, __add_string}
\done

\function{__add_unary}
\synopsis{Extend a unary operator to a user-defined type}
\usage{__add_unary (op, return_type, unary_func, user_type)}
#v+
   String_Type op;
   Ref_Type unary_func;
   DataType_Type return_type, user_type;
#v-
\description
  The \ifun{__add_unary} function is used to define the action of an
  unary operation on a user-defined type.  The first parameter
  \exmp{op} must be a valid unary operator
#v+
   "-", "not", "~"
#v-
  or one of the following:
#v+
   "++", "--",
   "abs", "sign", "sqr", "mul2", "_ispos", "_isneg", "_isnonneg",
#v-
  The third parameter, \exmp{unary_func} specifies the function to be
  called to carry out the specified unary operation on the data type
  \exmp{user_type}.  The result of the operation is indicated by the
  value of the \exmp{return_type} parameter and must also be the
  return type of the unary function.
\example
  The example for the \exmp{__add_binary} function defined a
  \exmp{Vector_Type} object.  Here, the unary \exmp{"-"} and
  \exmp{"abs"} operators are
  extended to this type:
#v+
   static define vector_chs (v)
   {
      variable v1 = @Vector_Type;
      v1.x = -v.x;
      v1.y = -v.y;
      v1.z = -v.z;
      return v1;
   }
   static define vector_abs (v)
   {
      return sqrt (v.x*v.x + v.y*v.y + v.z*v.z);
   }
   __add_unary ("-", Vector_Type, &vector_chs, Vector_Type);
   __add_unary ("abs", Double_Type, &vector_abs, Vector_Type);
#v-
\seealso{__add_binary, __add_string, __add_destroy}
\done

\function{get_struct_field}
\synopsis{Get the value associated with a structure field}
\usage{x = get_struct_field (Struct_Type s, String field_name)}
\description
   The \ifun{get_struct_field} function gets the value of the field
   whose name is specified by \exmp{field_name} of the structure \exmp{s}.
\seealso{set_struct_field, get_struct_field_names, array_info}
\done

\function{get_struct_field_names}
\synopsis{Retrieve the field names associated with a structure}
\usage{String_Type[] = get_struct_field_names (Struct_Type s)}
\description
   The \ifun{get_struct_field_names} function returns an array of
   strings whose elements specify the names of the fields of the
   struct \exmp{s}.
\example
   The following example illustrates how the
   \ifun{get_struct_field_names} function may be used in conjunction
   with the \ifun{get_struct_field} function to print the
   value of a structure.
#v+
      define print_struct (s)
      {
         variable name, value;

         foreach (get_struct_field_names (s))
           {
             name = ();
             value = get_struct_field (s, name);
             vmessage ("s.%s = %s\n", name, string (value));
           }
      }
#v-
\seealso{_push_struct_field_values, get_struct_field}
\done

\function{is_struct_type}
\synopsis{Determine whether or not an object is a structure}
\usage{Integer_Type is_struct_type (X)}
\description
  The \ifun{is_struct_type} function returns 1 if the parameter
  refers to a structure or a user-defined type.  If the object is
  neither, 0 will be returned.
\seealso{typeof, _typeof, _is_struct_type}
\done

\function{_push_struct_field_values}
\synopsis{Push the values of a structure's fields onto the stack}
\usage{Integer_Type num = _push_struct_field_values (Struct_Type s)}
\description
  The \ifun{_push_struct_field_values} function pushes the values of
  all the fields of a structure onto the stack, returning the
  number of items pushed.  The fields are pushed such that the last
  field of the structure is pushed first.
\seealso{get_struct_field_names, get_struct_field}
\done

\function{set_struct_field}
\synopsis{Set the value associated with a structure field}
\usage{set_struct_field (s, field_name, field_value)}
#v+
   Struct_Type s;
   String_Type field_name;
   Generic_Type field_value;
#v-
\description
   The \ifun{set_struct_field} function sets the value of the field
   whose name is specified by \exmp{field_name} of the structure
   \exmp{s} to \exmp{field_value}.
\seealso{get_struct_field, get_struct_field_names, set_struct_fields, array_info}
\done

\function{set_struct_fields}
\synopsis{Set the fields of a structure}
\usage{set_struct_fields (Struct_Type s, ...)}
\description
  The \ifun{set_struct_fields} function may be used to set zero or more
  fields of a structure.  The fields are set in the order in which
  they were created when the structure was defined.
\example
#v+
    variable s = struct { name, age, height };
    set_struct_fields (s, "Bill", 13, 64);
#v-
\seealso{set_struct_field, get_struct_field_names}
\done

