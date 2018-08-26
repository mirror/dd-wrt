_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("Short Ciruit Operators");

private variable Fired = 0;
private define fun (tf)
{
   Fired++;
   return tf;
}

private define test_sc ()
{
   Fired = 0;
   if (1 && fun(1) && 0 && fun(0) && fun (1))
     failed ("Simple &&");
   if (Fired != 1)
     failed ("&& is not short circuiting");

   Fired = 0;
   if (0 == (0 || fun(1) || 0 || fun(0) || fun (1)))
     failed ("Simple ||");
   if (Fired != 1)
     failed ("|| is not short circuiting");

   Fired = 0;
   if (0 == (fun(0) && fun(0) || 1 || fun(1)))
     failed ("mixed && ||");
   if (Fired != 1)
     failed ("mixed && || did not short circuit: Fired=%d", Fired);

   Fired = 0;
   if (1 && fun(0) || 1 && fun(0) || 0 && fun(0))
     {
	failed ("mixed || && 2");
     }
   if (Fired != 2)
     failed ("mixed || && 2 did not short circuit");

   variable a = [1,2];
   if (length (a) > 5 && a[5] == 3)
     failed ("Simple && case 2 failed");
}

test_sc ();

print ("Ok\n");

exit (0);

