() = evalfile ("./inc.sl");

testing_feature ("lists");

static variable L = list_new ();
list_insert (L, "f0");
if ("f0" != L[0])
  failed ("L[0]");
list_append (L, "f1");
if ("f1" != L[1])
  failed ("L[1]");
list_append (L, "f2", 1);
if ("f2" != L[2])
  failed ("L[2]");
if (length (L) != 3)
  failed ("list length");
list_delete (L, 0);
if (L[0] != "f1")
  failed ("list_delete (L,0)");
list_delete (L, -1);
if (L[0] != "f1")
  failed ("list_delete (L,-1)");
list_delete (L, 0);
if (length (L))
  failed ("list_delete to empty list");

L = 0;

L = {};
L = {L, "100", [1:10]};
if (length (L) != 3)
  failed ("list with an empty list");

if (0 == _eqs(L[0], {})) failed ("list {}");

L = {"200", L, 3.14};
if ((L[0] != "200")
    or (L[2] != 3.14)
    or (L[-1] != 3.14))
  failed ("list {with list elements}");

L = {20, 30, {40, {}, 50}};
if ((L[0] != 20) or (L[1] != 30) or (L[2][0] != 40) or (L[2][2] != 50))
  failed ("list { {} }");

static variable L1 = @L;
if (__is_same (L1, L))
  failed ("@L");
if (0 == _eqs (L1, L))
  failed ("_eqs(@L,L)");

_for (1, 10000, 1)
{
   $1 = ();
   list_append(L, $1);
}

L = {};
_for (1, 100, 1)
{
   $1 = ();
   list_append (L, {$1, "$1"$});
}
$1 = 0;
foreach $2 (L)
{
   $1++;
   if (($2[0] != $1) or ($2[1] != "$1"$))
     failed ("foreach list");
}
if (($1 != length (L)) or ($1 != 100))
  failed ("foreach length");

L = {};
_for $1 (0, 10, 1)
  L = {L,$1};

if (L[1] != 10)
  failed ("L[1]");

private define test_push_pop_list ()
{
   variable list = __pop_list (_NARGS);

   variable d0 = _stkdepth ();
   __push_list (list);
   variable d1 = _stkdepth ();
   if (d1 - d0 != _NARGS)
     failed ("push/pop_list");

   loop (_NARGS)
     {
	variable a = list_pop (list, -1);
	variable b = ();
	if (not __is_same (a, b))
	  failed ("push/pop_list failed sameness test");
     }
}

test_push_pop_list ("A", 1, 4, PI, Array_Type[3]);
test_push_pop_list ();
test_push_pop_list ("A");
test_push_pop_list ({});
test_push_pop_list ("A", {});

private define test_complex_list ()
{
   variable l = {};
   variable n1 = 0;
   variable s0 = "string";
   variable z0 = 1;
#ifexists Complex_Type
   z0 += 2i;
#endif

   variable n = 20;
   loop (n)
     {
	list_append (l, z0);
	list_append (l, s0);
     }

   variable za, sa, zl, sl;

   zl = l[[::2]];
   sl = l[[1::2]];

   za = list_to_array (zl);
   sa = list_to_array (sl);

   if (any(za != z0))
     failed ("list of complex numbers with array indexing");

   if (any (sa != s0))
     failed ("list of strings with array indexing");

   % LHS = array
   l[[::2]] = sa;
   l[[1::2]] = za;

   sa = list_to_array (l[[::2]]);
   za = list_to_array (l[[1::2]]);

   if (any(za != z0))
     failed ("aput complex with array LHS");

   if (any (sa != s0))
     failed ("aput string with array LHS");

   % LHS = list
   l[[::2]] = zl;
   l[[1::2]] = sl;

   sa = list_to_array (l[[::2]]);
   za = list_to_array (l[[1::2]]);

   if (any(za != z0))
     failed ("aput complex with array LHS");

   if (any (sa != s0))
     failed ("aput string with array LHS");
}
test_complex_list ();

private define make_big_list (len)
{
   variable l = {};
   _for (0, len-1, 1)
     {
	variable i = ();
	list_append (l, i);
     }
   return l;
}

private define random_indices (num, cd)
{
   variable i = int (urand () * num);
   variable j = int (urand () * num);
   return i, j;
}

private define forward_indices (num, last_ip)
{
   variable i = (@last_ip + 1) mod num;
   variable j = (i + 1) mod num;
   @last_ip = i;
   return i, j;
}

private define reverse_indices (num, last_ip)
{
   variable i = (num + (@last_ip - 1)) mod num;
   variable j = (num + (i - 1)) mod num;
   @last_ip = i;
   return i, j;
}

private define test_indexing (type, num, nloops, index_fun, cd)
{
   variable l = make_big_list (num);
   loop (nloops)
     {
	variable i, j;
	(i, j) = (@index_fun)(num, cd);
	if (l[i] != i)
	  failed ("%s list indexing", type);

	list_insert (l, -j, j);
	if (l[j] != -j)
	  failed ("%s list indexing with insertion", type);

	if (i < j)
	  {
	     if (l[i] != i)
	       failed ("%s list indexing before insertion", type);
	  }
	else if (i > j)
	  {
	     if (l[i] != i-1)
	       failed ("%s list indexing after insertion", type);
	  }

	list_delete (l, j);

	if (l[i] != i)
	  failed ("%s list indexing with deletion", type);
     }
}
private define run_index_tests ()
{
   variable cd;
   foreach ([1, 100, 127, 128, 129, 255, 256, 257, 1024, 8192, 8193])
     {
	variable num = ();
	test_indexing ("random", num, 10000, &random_indices, NULL);
	cd = -1;
	test_indexing ("forward", num, num, &forward_indices, &cd);
	cd = num+1;
	test_indexing ("reverse", num, num, &reverse_indices, &cd);
     }
}
run_index_tests ();

private define test_append_join ()
{
   variable da = {PI, "foo", 2.1, Array_Type[4]};
   variable ab = {};
   variable ab_concat = {};
   variable ab_join = {};
   variable obj;
   variable N = 31;
   loop (N)
     {
	ab_concat = list_concat (ab, da);
	list_join (ab_join, da);
	foreach obj (da)
	  list_append (ab, obj);
     }
   variable num = N*length(da);
   if (num != length (ab))
     failed ("list_append failed to produce a list of the expected length");
   if (num != length (ab_concat))
     failed ("list_concat failed to produce a list of the expected length");
   if (num != length (ab_join))
     failed ("list_join failed to produce a list of the expected length");

   _for (0, num-1, 1)
     {
	variable i = ();
	ifnot (_eqs (ab[i], ab_concat[i]))
	  failed ("list_concat failure: %S != %S", ab[i], ab_concat[i]);
	ifnot (_eqs (ab[i], ab_join[i]))
	  failed ("list_join failure: %S != %S", ab[i], ab_join[i]);
     }
}
test_append_join ();

print ("Ok\n");

exit (0);

