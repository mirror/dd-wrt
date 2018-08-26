_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("signals\n");

static variable Signal = 0;

static define handle_hup (sig)
{
   Signal = sig;
}

static define test_signal (sig, func, old_func)
{
   variable old;
   signal (sig, func, &old);
   if (_eqs (old, old_func))
     return;
   failed ("signal ($sig,$func,$old_func), got $old"$);
}

test_signal (SIGHUP, &handle_hup, SIG_DFL);
test_signal (SIGHUP, SIG_APP, &handle_hup);
test_signal (SIGHUP, SIG_DFL, SIG_DFL);
test_signal (SIGHUP, SIG_IGN, SIG_DFL);
test_signal (SIGHUP, &handle_hup, SIG_IGN);

static define test_signal (sig, func)
{
   Signal = 0;
   signal (sig, func);
   kill (getpid (), sig);
   variable count = 10;
   while (count and (Signal == 0))
     {
	sleep (1);
	count--;
     }
   if (Signal != sig)
     failed ("signal %d not caught, count=%d", sig, count);
}

static define sigalrm_handler (sig)
{
   if (sig != SIGALRM)
     failed ("alarm");
   throw MathError;
}

signal (SIGALRM, &sigalrm_handler);
static define test_sigalarm ()
{
   print ("\tPausing for 3 seconds for alarm test");
   alarm (3);
   try
     {
	while (1)
	  {
	     sleep (1);
	     print (".");
	  }
     }
   catch MathError;
   print ("\n");
}

static define sigint_handler (sig)
{
   if (sig != SIGINT)
     failed ("sigint_handler");
   throw UserBreakError;
}

static define test_sigsuspend ()
{
   signal (SIGINT, &sigint_handler);
   print ("\tNow try pressing ^C in next 5 seconds...");
   alarm (5);
   try
     sigsuspend ([SIGINT, SIGTSTP]);
   catch UserBreakError;
   catch MathError;
   print ("\n");
}

static define test_sigprocmask ()
{
   variable oldmask, origmask;
   sigprocmask (SIG_SETMASK, SIGHUP, &origmask);
   sigprocmask (SIG_BLOCK, SIGINT, &oldmask);

   if (orelse
       {length (oldmask) != 1}
       {oldmask[0] != SIGHUP})
     failed ("sigprocmask: expected to see SIGHUP in the mask");

   sigprocmask (SIG_UNBLOCK, SIGHUP, &oldmask);
   if (orelse
       {length (oldmask) != 2}
       {(0 == length (where (oldmask == SIGHUP)))}
       {(0 == length (where (oldmask == SIGINT)))})
     failed ("sigprocmask: expected to see SIGINT and SIGHUP in the mask");

   sigprocmask (SIG_SETMASK, origmask, &oldmask);
   if (orelse
       {length (oldmask) != 1}
       {oldmask[0] != SIGINT})
     failed ("sigprocmask: expected to see SIGINT in the mask");
}

test_signal (SIGHUP, &handle_hup);
test_sigalarm ();
test_sigsuspend ();
test_sigprocmask ();

print ("Ok\n");

exit (0);

