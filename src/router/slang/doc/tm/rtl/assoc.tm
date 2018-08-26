\function{assoc_delete_key}
\synopsis{Delete a key from an Associative Array}
\usage{assoc_delete_key (Assoc_Type a, String_Type k)}
\description
  The \ifun{assoc_delete_key} function deletes a key given by \exmp{k}
  from the associative array \exmp{a}.  If the specified key does not
  exist in \exmp{a}, then this function has no effect.
\seealso{assoc_key_exists, assoc_get_keys}
\done

\function{assoc_get_keys}
\synopsis{Return all the key names of an Associative Array}
\usage{String_Type[] assoc_get_keys (Assoc_Type a)}
\description
  This function returns all the key names of an associative array
  \exmp{a} as an ordinary one dimensional array of strings.  If the
  associative array contains no keys, an empty array will be returned.
\seealso{assoc_get_values, assoc_key_exists, assoc_delete_key, length}
\done

\function{assoc_get_values}
\synopsis{Return all the values of an Associative Array}
\usage{Array_Type assoc_get_keys (Assoc_Type a)}
\description
  This function returns all the values in the associative array
  \exmp{a} as an array of proper type.  If the associative array
  contains no keys, an empty array will be returned.
\example
  Suppose that \exmp{a} is an associative array of type
  \dtype{Integer_Type}, i.e., it was created via
#v+
      variable a = Assoc_Type[Integer_Type];
#v-
  The the following may be used to print the values of the array in
  ascending order:
#v+
      define print_sorted_values (a)
      {
         variable v = assoc_get_values (a);
         variable i = array_sort (v);
         v = v[i];
         foreach (v)
           {
              variable vi = ();
              () = fprintf (stdout, "%d\n", vi);
           }
      }
#v-
\seealso{assoc_get_values, assoc_key_exists, assoc_delete_key, array_sort}
\done

\function{assoc_key_exists}
\synopsis{Check to see whether a key exists in an Associative Array}
\usage{Integer_Type assoc_key_exists (Assoc_Type a, String_Type k)}
\description
  The \ifun{assoc_key_exists} function may be used to determine whether
  or not a specified key \exmp{k} exists in an associative array \exmp{a}.
  It returns 1 if the key exists, or 0 if it does not.
\seealso{assoc_get_keys, assoc_get_values, assoc_delete_key}
\done

