#d note1 A set may either be an Array_Type or a List_Type object.\__newline__\
  For a homogeneous collection of objects, it is better to\__newline__\
  use an Array_Type. i.e., \exmp{[1,2,3]} instead of \exmp{\{1,2,3\}}.


\function{complement}
\synopsis{Extract the elements of a set that are not contained in other sets.}
\usage{indices = complement (a, b, ..., c)}
\description
  This function computes the elements of the first argument (\exmp{a})
  that are not contained in the sets given by the other arguments
  (\exmp{b,...,c}) and returns them in the form of indices into the
  first argument.
\example
#v+
   a = {"foo", PI, 7};
   b = [1,2,3,PI];
   indices = complement (a, b);
#v-
  Upon return, \exmp{indices} will have the value \exmp{[0, 2]} since
  \exmp{a[0]} and \exmp{a[2]} are not contained in \exmp{b}.
\notes
  \note1
\seealso{intersection, ismember, union, unique}
\done

\function{intersection}
\synopsis{Extract the common elements of two or more sets}
\usage{indices = complement (a, b, ..., c)}
\description
  This function computes the common elements of two or more sets and
  returns them in the form of indices into the first argument.
\example
#v+
   a = {"foo", 7, PI};
   b = {PI, "bar", "foo"};
   indices = intersection (a, b);
#v-
  Upon return, \exmp{indices} will have the value \exmp{[0, 2]} since
  \exmp{a[0]} and \exmp{a[2]} are the common elements of the sets.
\notes
  \note1
\seealso{complement, ismember, union, unique}
\done

\function{ismember}
\synopsis{test to see if the elements of one set are members of another}
\usage{val = ismember (a, b)}
\description
  This function may be used to see which of the elements of the set
  \exmp{a} are members of the set \exmp{b}.  It returns a boolean
  array indicating whether or not the corresponding element of
  \exmp{a} is a member of \exmp{b}.
\notes
  \note1
\seealso{complement, intersection, union, unique}
\done

\function{union}
\synopsis{Form a set of the unique elements of one ore more subsets}
\usage{abc = union (a, b,..., c)}
\description
  This function interprets each of its arguments as a set, then merges
  them together and returns only the unique elements.  The returned
  value may either be an \dtype{Array_Type} or a \dtype{List_Type}
  object.
\notes
  \note1
\seealso{complement, intersection, ismember, unique}
\done

\function{unique}
\synopsis{Get the indices of the unique elements of a set}
\usage{indices = unique (A)}
\description
  This function returns an array of the indices of the unique elements
  of a set.
\notes
  \note1
\seealso{complement, intersection, ismember, union}
\done
