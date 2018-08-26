#! /usr/bin/env slsh

% This program presents the solution to a problem posed by
% Tom Christiansen <tchrist@mox.perl.com>.  The problem reads:
%
%    Sort an input file that consists of lines like this
%
%        var1=23 other=14 ditto=23 fred=2
%
%    such that each output line is sorted WRT to the number.  Order
%    of output lines does not change.  Resolve collisions using the
%    variable name.   e.g.
%
%        fred=2 other=14 ditto=23 var1=23
%
%    Lines may be up to several kilobytes in length and contain
%    zillions of variables.
%---------------------------------------------------------------------------
%
% The solution presented below works by breaking up the line into an
% array of alternating keywords and values with the keywords as the even
% elements and the values as the odd.  It is about 30% faster than the
% python solution.

private variable Keys, Values;
private define sort_fun (i, j)
{
   variable s, a, b;

   s = Values[i] - Values[j];
   !if (s)
     return strcmp (Keys[i], Keys[j]);
   return s;
}

define slsh_main ()
{
   variable line, len, i, vals;
   foreach line (stdin)
     {
	line = strtok (line, " \t\n=");
	len = length(line)/2;
	if (len == 0)
	  continue;

	% Even elements are keys, odd are values
	Keys = line[[0::2]];
	vals = line[[1::2]];

	Values = atoi (vals);

	i = array_sort ([0:len-1], &sort_fun);

	% There are different ways of writing the result.  Here is a
	% fast way that avoids a loop.
	() = printf ("%s\n", strjoin (Keys[i] + "=" + vals[i], " "));
     }
}
