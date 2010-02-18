import ("newt");

newtInit ();
%newtCls ();
newtDrawRootText (0, 0, "Root Text");
newtOpenWindow (3,4,20, 20, "First Window");
newtOpenWindow (8, 10,20, 20, "Second Window");

newtRefresh ();
sleep (5);
NewtFinished ();

exit (0);

