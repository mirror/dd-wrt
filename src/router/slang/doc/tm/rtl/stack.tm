\function{dup}
\synopsis{Duplicate the value at the top of the stack}
\usage{dup ()}
\description
  This function returns an exact duplicate of the object on top of the
  stack.  For some objects such as arrays or structures, it creates a
  new reference to the object.  However, for simple scalar \slang types such
  as strings, integers, and doubles, it creates a new copy of the
  object.
\seealso{pop, typeof}
\done

\function{exch}
\synopsis{Exchange two items on the stack}
\usage{exch ()}
\description
  The \ifun{exch} swaps the two top items on the stack.
\seealso{pop, _stk_reverse, _stk_roll}
\done

\function{pop}
\synopsis{Discard an item from the stack}
\usage{pop ()}
\description
  The \ifun{pop} function removes the top item from the stack.
\seealso{_pop_n, __pop_args}
\done

\function{__pop_args}
\synopsis{Remove n function arguments from the stack}
\usage{args = __pop_args(Integer_Type n)}
\description
  This function, together with the companion function
  \ifun{__push_args}, is useful for creating a function that takes a
  variable number of arguments, as well as passing the arguments of
  one function to another function.

  \ifun{__pop_args} removes the specified number of values from the
  stack and returns them as an array of structures of the corresponding
  length.  Each structure in the array consists of a single
  field called \exmp{value}, which represents the value of the
  argument.
\example
  Consider the following function.  It prints all its arguments to
  \ivar{stdout} separated by spaces:
#v+
    define print_args ()
    {
       variable i;
       variable args = __pop_args (_NARGS);

       for (i = 0; i < _NARGS; i++)
         {
            () = fputs (string (args[i].value), stdout);
            () = fputs (" ", stdout);
         }
       () = fputs ("\n", stdout);
       () = fflush (stdout);
    }
#v-
  Now consider the problem of defining a function called \exmp{ones}
  that returns a multi-dimensional array with all the elements set to
  1.  For example, \exmp{ones(10)} should return a 1-d array of 10
  ones, whereas \exmp{ones(10,20)} should return a 10x20 array.
#v+
    define ones ()
    {
      !if (_NARGS) return 1;
      variable a;

      a = __pop_args (_NARGS);
      return @Array_Type (Integer_Type, [__push_args (a)]) + 1;
    }
#v-
  Here, \ifun{__push_args} was used to push the arguments passed to
  the \exmp{ones} function onto the stack to be used when dereferencing
  \dtype{Array_Type}.
\notes
  This function has been superseded by the \ifun{__pop_list} function,
  which returns the objects as a list instead of an array of structures.
\seealso{__push_args, __pop_list, __push_list, typeof, _pop_n}
\done

\function{__pop_list}
\synopsis{Convert items on the stack to a List_Type}
\usage{List_Type = __pop_list (Int_Type n)}
\description
 This function removes a specified number of items from the stack and
 converts returns them in the form of a list.
\example
#v+
  define print_args ()
  {
     variable list = __pop_list (_NARGS);
     variable i;
     _for i (0, length(list)-1, 1)
        {
           vmessage ("arg[%d]: %S", i, list[i]);
        }
  }
#v-
\seealso{__push_list}
\done

\function{_pop_n}
\synopsis{Remove objects from the stack}
\usage{_pop_n (Integer_Type n);}
\description
  The \ifun{_pop_n} function removes the specified number of objects
  from the top of the stack.
\seealso{_stkdepth, pop}
\done

\function{_print_stack}
\synopsis{Print the values on the stack.}
\usage{_print_stack ()}
\description
  This function dumps out what is currently on the \slang stack.  It does not
  alter the stack and it is usually used for debugging purposes.
\seealso{_stkdepth, string, message}
\done

\function{__push_args}
\synopsis{Move n function arguments onto the stack}
\usage{__push_args (Struct_Type args);}
\description
  This function together with the companion function \ifun{__pop_args}
  is useful for the creation of functions that take a variable number
  of arguments.  See the description of \ifun{__pop_args} for more
  information.
\notes
  This function has been superseded by the \ifun{__push_list} function.
\seealso{__pop_args, __push_list, __pop_list, typeof, _pop_n}
\done

\function{__push_list}
\synopsis{Push the elements of a list to the stack}
\usage{__push_list (List_Type list)}
\description
 This function pushes the elements of a list to the stack.
\example
#v+
 private define list_to_array (list)
 {
    return [__push_list (list)];
 }
#v-
\seealso{__pop_list}
\done

\function{_stkdepth}
\usage{Get the number of objects currently on the stack}
\synopsis{Integer_Type _stkdepth ()}
\description
  The \ifun{_stkdepth} function returns number of items on the stack.
\seealso{_print_stack, _stk_reverse, _stk_roll}
\done

\function{_stk_reverse}
\synopsis{Reverse the order of the objects on the stack}
\usage{_stk_reverse (Integer_Type n)}
\description
   The \ifun{_stk_reverse} function reverses the order of the top
   \exmp{n} items on the stack.
\seealso{_stkdepth, _stk_roll}
\done

\function{_stk_roll}
\synopsis{Roll items on the stack}
\usage{_stk_roll (Integer_Type n)}
\description
  This function may be used to alter the arrangement of objects on the
  stack.  Specifically, if the integer \exmp{n} is positive, the top
  \exmp{n} items on the stack are rotated up.  If
  \exmp{n} is negative, the top \exmp{abs(n)} items on the stack are
  rotated down.
\example
  If the stack looks like:
#v+
    item-0
    item-1
    item-2
    item-3
#v-
  where \exmp{item-0} is at the top of the stack, then
  \exmp{_stk_roll(-3)} will change the stack to:
#v+
    item-2
    item-0
    item-1
    item-3
#v-
\notes
  This function only has an effect if \exmp{abs(n) > 1}.
\seealso{_stkdepth, _stk_reverse, _pop_n, _print_stack}
\done

