_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("Matrix Multiplications");
#ifexists Double_Type

static define dot_prod (a, b)
{
   (a # b)[0];			       %  transpose not needed for 1-d arrays
}

static define sum (a)
{
   variable ones = Double_Type [length (a)] + 1;
   dot_prod (a, ones);
}

if (1+2+3+4+5 != sum([1,2,3,4,5]))
  failed ("sum");

#ifexists Complex_Type
if (1+2i != sum ([1,2i]))
  failed ("sum complex");
#endif

define mult (a, b)
{
   variable dims_a, dims_b;
   variable nr_a, nr_b, nc_a, nc_b;
   variable i, j;
   variable c;

   (dims_a,,) = array_info (a);
   (dims_b,,) = array_info (b);
   nr_a = dims_a[0];
   nc_a = dims_a[1];
   nr_b = dims_b[0];
   nc_b = dims_b[1];

   c = _typeof ([a[0,0]]#[b[0,0]])[nr_a, nc_b];

   for (i = 0; i < nr_a; i++)
     {
	for (j = 0; j < nc_b; j++)
	  c[i,j] = dot_prod (a[i,*], b[*,j]);
     }
   return c;
}

static define arr_cmp (a, b)
{
   variable i = length (where (b != a));
   if (i == 0)
     return 0;

   i = where (b != a);
   a = a[i];
   b = b[i];
   reshape (a, [length(a)]);
   reshape (b, [length(b)]);
   vmessage ("%S != %S\n", a[0], b[0]);
   return 1;
}

static define test (a, b)
{
   if (0 != arr_cmp (mult (a,b), a#b))
     failed ("%S # %S", a, b);
}

variable A, B;

#ifexists Complex_Type
A = [1+2i];
B = [3+4i];
reshape (A, [1, 1]);
reshape (B, [1, 1]);
test (A,B);
#endif

% Test intgers
A = _reshape ([[1:20], [1:20]], [2,20]);
B = _reshape ([[1:20],[1:20],[1:20],[1:20],[1:20]], [5,20]);
B = transpose (B);

foreach $1 ([1:100:10])
{
   __set_innerprod_block_size ($1);
   test (A, B);

   B *= 1f;
   test (A, B);

   B *= 1.0;
   test (A,B);

   A *= 1f;
   test (A,B);

#ifexists Complex_Type
   B += 2i;
   test (A,B);

   A += 3i;
   test (A,B);

   B = Real(B);
   test (A,B);

% Now try an empty array

   if (Complex_Type != _typeof (Complex_Type[0,0,0] # Complex_Type[0]))
     failed ("[]#[]");
#endif
}
% And finally, do a 3-d array:

A = _reshape ([1:2*3*4], [2,3,4]);
B = _reshape ([1:4*5*6], [4,5,6]);
static variable C = A#B;

% C should be a [2,3,5,6] matrix.  Let's check via brute force

static define multiply_3d (a, b, c)
{
   variable i, j, k, l, m;
   variable dims_a, dims_b;

   (dims_a,,) = array_info(a);
   (dims_b,,) = array_info(b);

   _for (0, dims_a[0]-1, 1)
     {
	i = ();
	_for (0, dims_a[1]-1, 1)
	  {
	     j = ();
	     _for (0, dims_b[1]-1, 1)
	       {
		  l = ();
		  _for (0, dims_b[2]-1, 1)
		    {
		       m = ();

		       variable sum = 0;
		       _for (0, dims_b[0]-1, 1)
			 {
			    k = ();
			    sum += a[i,j,k] * b[k, l, m];
			 }
		       if (sum != c[i,j,l,m])
			 failed ("multiply_3d");
		    }
	       }
	  }
     }
}

multiply_3d (A, B, C);

print ("Ok\n");
#else
print ("Not available\n");
#endif
exit (0);

