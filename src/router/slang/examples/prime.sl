#! /usr/bin/env slsh
% This demo counts the number of primes between 2 and some integer

private define usage ()
{
   () = fprintf (stderr, "Usage: %S <integer greater than 2>\n", __argv[0]);
   exit (1);
}

define count_primes (num)
{
   variable size = (num - 1)/2;
   variable nonprimes = Char_Type[size + 1];   %  last one is sentinel
   variable count = 1;
   variable prime = 3;
   variable i = 0;

   do
     {
        count++;
	%()=printf ("%S\n", prime);

	nonprimes [[i:size-1:prime]] = 1;
	variable i_save = i;
	while (i++, nonprimes[i])
	  ;
	prime += 2 * (i - i_save);
     }
   while (i < size);

   return count;
}

private variable Num;

if (__argc != 2)
  usage ();
Num = integer (__argv[1]);
if (Num < 3)
  usage ();

tic ();
()=printf ("\n\n%d primes between 2 and %d in %f seconds.\n",
	   count_primes (Num), Num, toc ());
exit(0);
