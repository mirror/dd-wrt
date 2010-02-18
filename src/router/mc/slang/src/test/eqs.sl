_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("_eqs");

if (0 == _eqs (1h, 1L))
  failed ("_eqs(1h,1L)");

if (0 == _eqs (1h, 1+0i))
  failed ("_eqs(1h,1+0i)");

if (0 == _eqs ([1h:10h], [1L:10L]))
  failed ("_eqs (arrays of ints)");

static variable A = List_Type[2];
static variable L = {A};
A[0] = L;
if (_eqs (A,L))
  failed ("_eqs(L,A)");
if (0 == _eqs (A[0],L))
  failed ("_eqs(A[0],L)");

print ("Ok\n");

exit (0);

