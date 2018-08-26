\function{list_append}
\synopsis{Append an object to a list}
\usage{list_append (List_Type list, object [,Int_Type nth])}
\description
  The \ifun{list_append} function is like \ifun{list_insert} except
  this function appends the object to the list.  The optional
  argument \exmp{nth} may be used to specify where the object is to be
  appended.  See the documentation on \ifun{list_insert} for more details.
\seealso{list_concat, list_insert, list_join, list_delete, list_pop, list_new, list_reverse}
\done

\function{list_concat}
\synopsis{Concatenate two lists to form a third}
\usage{List_Type = list_concat (List_Type a, List_Type b)}
\description
  This function creates a new list that is formed by concatenating the
  two lists \exmp{a} and \exmp{b} together.  Neither of the input
  lists are modified by this operation.
\seealso{list_join, list_append, list_insert}
\done

\function{list_delete}
\synopsis{Remove an item from a list}
\usage{list_delete (List_Type list, Int_Type nth)}
\description
  This function removes the \exmp{nth} item in the specified list.
  The first item in the list corresponds to a value of \exmp{nth}
  equal to zero.  If \exmp{nth} is negative, then the indexing is with
  respect to the end of the list with the last item corresponding to
  \exmp{nth} equal to -1.
\seealso{list_insert, list_append, list_pop, list_new, list_reverse}
\done

\function{list_insert}
\synopsis{Insert an item into a list}
\usage{list_insert (List_Type list, object [,Int_Type nth])}
\description
  This function may be used to insert an object into the specified
  list.  With just two arguments, the object will be inserted at the
  beginning of the list.  The optional third argument, \exmp{nth}, may
  be used to specify the insertion point.  The first item in the list
  corresponds to a value of \exmp{nth} equal to zero.  If \exmp{nth}
  is negative, then the indexing is with respect to the end of the
  list with the last item given by a value of \exmp{nth} equal to -1.
\notes
  It is important to note that
#v+
    list_insert (list, object, 0);
#v-
  is not the same as
#v+
    list = {object, list}
#v-
  since the latter creates a new list with two items, \exmp{object}
  and the old list.
\seealso{list_append, list_pop, list_delete, list_new, list_reverse}
\done

\function{list_join}
\synopsis{Join the elements of a second list onto the end of the first}
\usage{list_join (List_Type a, List_Type b)}
\description
  This function modifies the list \exmp{a} by appending the elements
  of \exmp{b} to it.
\seealso{list_concat, list_append, list_insert}
\done

\function{list_new}
\synopsis{Create a new list}
\usage{List_Type list_new ()}
\description
  This function creates a new empty \dtype{List_Type} object.  Such a
  list may also be created using the syntax
#v+
     list = {};
#v-
\seealso{list_delete, list_insert, list_append, list_reverse, list_pop}
\done

\function{list_pop}
\synopsis{Extract an item from a list}
\usage{object = list_pop (List_Type list [, Int_Type nth])}
\description
  The \ifun{list_pop} function returns a object from a list deleting
  the item from the list in the process.  If the second argument is
  present, then it may be used to specify the position in the list
  where the item is to be obtained.  If called with a single argument,
  the first item in the list will be used.
\seealso{list_delete, list_insert, list_append, list_reverse, list_new}
\done

\function{list_reverse}
\synopsis{Reverse a list}
\usage{list_reverse (List_Type list)}
\description
  This function may be used to reverse the items in list.
\notes
  This function does not create a new list.  The list passed to the
  function will be reversed upon return from the function.  If it is
  desired to create a separate reversed list, then a separate copy
  should be made, e.g.,
#v+
     rev_list = @list;
     list_reverse (rev_list);
#v-
\seealso{list_new, list_insert, list_append, list_delete, list_pop}
\done

\function{list_to_array}
\synopsis{Convert a list into an array}
\usage{Array_Type list_to_array (List_Type list [,DataType_Type type])}
\description
 The \ifun{list_to_array} function converts a list of objects into an
 array of the same length and returns the result.  The optional
 argument may be used to specify the array's data type.  If no
 \exmp{type} is given, \ifun{list_to_array} tries to find the common
 data type of all list elements. This function will generate an
 exception if the list is empty and no type has been specified, or the
 objects in the list cannot be converted to a common type.
\notes
 A future version of this function may produce an \dtype{Any_Type}
 array for an empty or heterogeneous list.
\seealso{length, typecast, __pop_list, typeof, array_sort}
\done

