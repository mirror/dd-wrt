% Functions that operate on sets in the form of arrays and lists:
% Copyright (C) 2010-2011 John E. Davis
%
% This file is part of the S-Lang Library and may be distributed under the
% terms of the GNU General Public License.  See the file COPYING for
% more information.
%
%  Functions: unique, union, complement, intersection, ismember
private define pop_set_object ()
{
   variable a = ();
   if ((typeof(a) != Array_Type) && (typeof(a) != List_Type))
     a = [a];
   return a;
}

private define list_unique (a)
{
   variable len = length(a);
   variable indices = Int_Type[len];
   variable i, j, k;

   k = 0;
   _for i (0, len-1, 1)
     {
	variable a_i = a[i];
	_for j (0, i-1, 1)
	  {
	     if (_eqs(a_i, a[j]))
	       break;
	  }
	then
	  {
	     indices[k] = i;
	     k++;
	  }
     }
   return indices[[0:k-1]];
}


define unique ()
{
   variable i, j, len;
   variable a;

   if (_NARGS != 1)
     {
	_pop_n (_NARGS);
	usage ("i = unique (a); %% i = indices of unique elements of a");
     }

   a = pop_set_object ();
   if (typeof(a) == List_Type)
     {
	try
	  {
	     a = list_to_array (a);
	  }
	catch AnyError: return list_unique (a);
     }

   len = length(a);
   if (len <= 1)
     return [0:len-1];

   if (length(array_shape(a)) != 1)
     a = _reshape (__tmp(a),[len]);

   try
     {
	i = array_sort(a);
     }
   catch AnyError: return list_unique (a);

   a = a[i];
   if (a[0] == a[-1])		       %  all equal
     return [0];
   j = where (shift(a,-1)!=a);
   % Now, i contains the sorted indices, and j contains the indices into the
   % sorted array.  So, the unique elements are given by a[i][j] where a is
   % the original input array.  It seems amusing that the indices given by
   % [i][j] are also given by i[j].
   return i[__tmp(j)];
}

define union ()
{
   !if (_NARGS)
     usage ("U = union (A, B, ..., C);");

   variable args = {}, obj;
   variable has_list = 0;
   loop (_NARGS)
     {
	has_list += (typeof (obj) == List_Type);
	obj = pop_set_object ();
	list_insert (args, obj);
     }

   variable a = NULL;
   if (has_list == 0)
     {
	try
	  {
	     a = [__push_list (args)];
	  }
	catch AnyError:;
     }

   if (a == NULL)
     {
	a = {};
	foreach obj (args)
	  {
	     if (typeof(obj) == List_Type)
	       {
		  list_join (a, obj);
		  continue;
	       }
	     foreach (obj)
	       {
		  variable x = ();
		  list_append (a, x);
	       }
	  }
     }
   return a[unique (a)];
}

% return indices of a that are not in b
private define list_complement (a, b)
{
   variable lena = length(a), lenb = length(b);
   variable indices = Int_Type[lena];
   variable i, j, k;

   k = 0;
   _for i (0, lena-1, 1)
     {
	variable a_i = a[i];
	_for j (0, lenb-1, 1)
	  {
	     if (_eqs(a_i, b[j]))
	       break;
	  }
	then
	  {
	     indices[k] = i;
	     k++;
	  }
     }
   return indices[[0:k-1]];
}

define complement ()
{
   variable a, b;
   if (_NARGS != 2)
     usage ("\
i = complement (a, b);\n\
%% Returns the indices of the elements of `a' that are not in `b'");

   b = pop_set_object ();
   a = pop_set_object ();

   variable
     lena = length(a),
     lenb = length(b);

   if ((lena == 0) || (lenb == 0))
     return [0:lena-1];

   variable sia, sib;
   try
     {
	if (typeof (a) == List_Type)
	  a = list_to_array (a);
	if (typeof (b) == List_Type)
	  b = list_to_array (b);
	sia = array_sort (a);
	sib = array_sort (b);
     }
   catch AnyError:
     return list_complement (a, b);

   variable
     c = Int_Type [lena], j = 0,
     ia, ib, xa, xb, k;

   ia = 0; ib = 0;
   xb = b[sib[ib]];
   while (ia < lena)
     {
	k = sia[ia];
	xa = a[k];
	if (xa < xb)
	  {
	     c[j] = k;
	     j++;
	     ia++;
	     continue;
	  }
	if (xb == xa)
	  {
	     ia++;
	     continue;
	  }

	while (ib++, (ib < lenb) && (xa > b[sib[ib]]))
	  ;
	if (ib == lenb)
	  {
	     variable n = lena-ia;
	     c[[j:j+n-1]] = sia[[ia:lena-1]];
	     j += n;
	     break;
	  }
	xb = b[sib[ib]];
	if (xa == xb)
	  ia++;
     }
   return c[[0:j-1]];
}

% Return the indices into a of the common elements of both a and b
define intersection ()
{
   if (_NARGS < 2)
     usage ("\
i = intersection (a, b, .., c);\n\
%% Returns the indices of 'a' of the common elements of b,.., c");

   variable b = pop_set_object ();
   loop (_NARGS-1)
     {
	variable a = pop_set_object ();
	variable i = complement (a, __tmp(b));
	i = complement (a, a[i]);
	b = a[i];
     }
   return i;
}

% Returns whether or not a is a member of b.
define ismember ()
{
   if (_NARGS != 2)
     usage ("I = ismember (a, b);\n\
Returns a boolean array indicated whether the corresponding elements of 'a'\n\
are members of 'b'");

   variable a, b;
   (a, b) = ();
   if ((typeof(a) == Array_Type) || (typeof(a) == List_Type))
     {
	variable lena = length (a);
	variable result = Char_Type[lena];
	result[intersection(a,b)] = 1;
	return result;
     }
   return 0 != length (intersection (a, b));
}
