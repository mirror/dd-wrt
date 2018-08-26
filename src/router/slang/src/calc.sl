_debug_info = 1;
variable X = Any_Type[1];
X[0] = "foo";

% a print function
_debug_info = 1;
set_float_format ("%.32e");
define p(obj)
{
   () = fprintf (stdout, "%S\n", obj);
   () = fflush (stdout);
}

define print_struct (s)
{
   variable name, value;

   foreach (get_struct_field_names (s))
     {
	name = ();
	value = get_struct_field (s, name);

	() = printf ("s.%s = %S\n", name, value);
     }
}

static variable Static_Variable;

static define test1 ()
{
   loop (_NARGS) p;
}

define test ()
{
   usage ("silly silly silly");
   variable args = __pop_args (_NARGS);
   test1 (__push_args (args));
}

define calc_take_input_hook ()
{
   while (_stkdepth ())
     p ();
}

define print_array (a)
{
   variable num_dims, dims;
   variable nr, nc;
   variable i, j;

   (dims ,num_dims,) = array_info (a);
   if (num_dims > 2)
     {
	p (a);
	return;
     }

   nr = dims [0];
   nc = 0;
   if (num_dims == 2)
     nc = dims[1];

   _for (0, nr - 1, 1)
     {
	i = ();
	!if (nc)
	  {
	     () = printf ("Array[%d] = %S\n", i, a[i]);
	     continue;
	  }
	_for (0, nc - 1, 1)
	  {
	     j = ();
	     () = printf ("\t%S", a[i, j]);
	  }
	() = fputs ("\n", stdout);
     }
}

define read_file (file)
{
   variable line, len;
   variable root, tail, s;
   variable fp;

   fp = fopen (file, "r");
   if (fp == NULL)
     error ("unable to open file");

   root = NULL;
   tail = NULL;
   while (-1 != fgets (&line, fp))
     {
	s = struct { next, value };
	s.value = line;
	s.next = NULL;

	if (root == NULL)
	  root = s;
	else
	  tail.next = s;

	tail = s;
     }
   () = fclose (fp);
   return root;
}

define list_len (list)
{
   variable len = 0;

   foreach (list) using ("next")
     {
	() = ();
	len++;
     }
   return len;
}

% calc.sl--- Init file for calc.  This file must be placed in the default
%  directory for calc and is automatically loaded when calc runs.
%
% This file contains S-Lang code for Newton's method, etc...
%
% Here is a function which computes the root of the equation y = f(x) using
% Newtons method.  The usage is:
%
%   root = newton(s, &f);
%
% where s is a seed value and f is the function whose root is sought.
%
% For example, consider the function my_fun(x) = x^2 - 2 with solution
% x = sqrt(2).  This function may be expressed in S-Lang as:
%
% define my_func(x)
% {
%   return (x * x - 2);
% }
%
% To solve the equation my_fun(x) = 0 using the newton routine below, use
%
%     newton(5.0, &myfun);
%
% Here, I have randomly chosen 5.0 as an initial guess.   In addition,
% I have used the '&' operator to pass the function 'myfun' to the routine.

% Newton's method requires the derivative of a function.  Here is such a
% function called by newton.  Given f(x), it returns df/dx at the point x.
%
% Its usage is:
%
%    derivative(x, &f);

define derivative(x, f)
{
   variable dx;
   dx = 1.0e-4;        % small number

   return ((@f(x + dx) - @f(x - dx))/(2 * dx));
}

% And now the Newton's method:

define newton(x, f)
{
   variable err, max, dx;

   err = 1.0e-6;
   max = 1000;

   while (max)
     {
	--max;
	dx = @f(x) / derivative(x, f);
	if (abs(dx) < err)
	  {
	     return(x);
	  }

	x -= dx;
     }

   message ("\7Root not found.  Try another seed!");
   return(x);
}

%% This is a standard benchmark for interpreters.  It is a heavily
%% recursive routine that returns the nth Fibonacci number.
%% It is defined recursively as:
%%
%%     f_0 = 0, f_1 = 1, .... , f_{n+1} = f_n + f_{n-1}, ...
%%
%%     or {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, ...}
%%

define fib();               % required for recursion

define fib(n)
{
   !if (n) return(0);
   n--;
   !if (n) return(1);

   return fib(n) + fib(n-1);
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   Two routines which illustrate the how to deal with files
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% type out a file to terminal
define type_file(file)
{
   variable fp, n, line;

   fp = fopen(file, "r");
   if (fp == NULL)
     verror ("%s failed to open.", file);

   while (-1 != fgets (&line, fp))
     {
	() = fputs (line, stdout);
     }

   if (-1 == fclose(fp))
     verror ("Error closing %s", file);
}

%
%  Here is a function that prints the number of lines in a file
%

define count_lines1 (file)
{
   variable fp, lines, nchars, num_lines, st;

   fp = fopen (file, "r");
   if (fp == NULL)
     verror ("count_lines1: unable to open %s", file);

   st = stat_file (file);
   if (st == NULL)
     verror ("stat_file failed");

   lines = fgetslines (fp);
   nchars = st.st_size;

   num_lines = length (lines);

   () = fclose (fp);
   vmessage ("%s consists of %d characters and %d lines.\n",
	     file, nchars, num_lines);
}

define count_lines(f)
{
   variable fp, n, nchars, dn, line;

   fp = fopen(f, "r");
   if (fp == NULL) error("Unable to open file!");
   n = 0; nchars = 0;

   while (dn = fgets (&line, fp), dn != -1)
     {
	++n;
	nchars += dn;
     }
   () = fclose(fp);		       %/* ignore return value */

   vmessage ("%s consists of %d characters and %d lines.\n",
	     f, nchars, n);
}

define count_lines2(f)
{
   variable fp, n, nchars, dn, line;

   fp = fopen(f, "r");
   if (fp == NULL) error("Unable to open file!");
   n = 0; nchars = 0;

   foreach (fp)
     {
	nchars += strlen ();
	++n;
     }
   () = fclose(fp);		       %/* ignore return value */

   vmessage ("%s consists of %d characters and %d lines.\n",
	     f, nchars, n);
}

define count_lines3(f)
{
   variable fp, n, nchars, dn, line;

   fp = fopen(f, "r");
   if (fp == NULL) error("Unable to open file!");
   n = 0; nchars = 0;

   n = 1;
   foreach (fp) using ("char")
     {
	variable ch = ();
	if (ch == '\n')
	  n++;
	nchars++;
     }
   () = fclose(fp);		       %/* ignore return value */

   vmessage ("%s consists of %d characters and %d lines.\n",
	     f, nchars, n);
}

% an apropos function
define apropos (what)
{
   variable n = _apropos(what, 0xF);
   variable i, f1, f2, f3;

   if (n) () = printf ("Found %d matches:\n", n);
   else
     {
	() = printf ("No matches.\n");
	return;
     }

   loop (n / 3)
     {
	f1 = (); f2 = (); f3 = ();
	() = printf ("%-26s %-26s %s\n", f1, f2, f3);
     }
   n = n mod 3;
   loop (n)
     {
	f1 = ();
	() = printf ("%-26s ", f1);
     }
   if (n) () = printf("\n");
}

%%% more help (called from calc.c)
define calc_help ()
{
   p("Additional functions:");
   p("  p();     -- displays top element of the stack (discarding it).");
   p("  quit();  -- quit calculator");
   p("  apropos(\"STRING\");  -- lists all objects containing \"STRING\"");
   p("\nExample: p (2.4 * E);    yields 6.52388.\n");
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%    end of calc.sl
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
