% This file shows how to use the varray-module to treat a file as an
% array of objects.
import ("varray");

% First of all, create an array of doubles
static variable x = [1:1000.0:1.0];

% and write it to disk
static variable file = "varray_example.dat";
static variable fp = fopen (file, "wb");
if (fp == NULL)
{
   () = fprintf (stderr, "failed to open %s\n", file);
   exit (1);
}
if ((-1 == fwrite (x, fp))
    or (-1 == fclose (fp)))
{
   () = fprintf (stderr, "Failed to write x\n");
   exit (1);
}

% Now associate an array with the file
variable y = mmap_array (file, 0, _typeof(x), length(x));

if (length (where (y != x)))
{
   fprintf (stderr, "mmap_array has failed\n");
   exit (1);
}

y = 0;				       %  remove the map

exit (0);

