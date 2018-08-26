() = evalfile ("inc.sl");

testing_feature ("Comparison Operators");

private variable Self_Called = 0;
define self (x)
{
   Self_Called++;
   return x;
}

define test_is_in (x0, x, x1)
{
   variable is_in = ((x0 < x) and (x < x1));
   ifnot (_eqs (is_in, (x0 < x < x1)))
     failed ("is_in 0");

   ifnot (_eqs (is_in, (x0 + 0 < x - 0 * x1 < x1 < x1+1 <= x1 + 1)))
     failed ("is_in 1");

   ifnot (_eqs (is_in, (x0 - 5 < x - 5 * (2>1) < x1 - 5 <= x1 - 5)))
     failed ("is_in 2");

   Self_Called = 0;
   ifnot (_eqs (is_in, (x0 < self(x) < x1)))
     failed ("is_in 0");
   if (Self_Called != 1)
     {
	failed ("is_in: Self_Called != 1");
     }

   is_in = ((x0 > x) and (x < x1));
   ifnot (_eqs (is_in, (x0 > x < x1)))
     failed ("is_in b0");

   ifnot (_eqs (is_in, (x0 + 0 > x - 0 * x1 < x1 < x1+1 <= x1 + 1)))
     failed ("is_in b1");

   ifnot (_eqs (is_in, (x0 - 5 > x - 5 * (2>1) < x1 - 5 <= x1 - 5)))
     failed ("is_in b2");

   Self_Called = 0;
   ifnot (_eqs (is_in, (x0 > self(x) < x1)))
     failed ("is_in b3");
   if (Self_Called != 1)
     {
	failed ("is_in: Self_Called != 1");
     }
}

test_is_in (1, 2, 3);
test_is_in (1, 1, 3);
test_is_in (1, 3, 3);

test_is_in ([1,1,1], [2,1,3], [1,3,3]);

test_is_in (urand(1000), urand(1000), urand (1000));

print ("Ok\n");

exit (0);

