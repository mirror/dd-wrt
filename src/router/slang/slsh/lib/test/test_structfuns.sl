require ("structfuns");

define slsh_main ()
{
   variable s = struct
     {
	str = "foo",
	a1 = [1:3],
	a2 = _reshape ([1:3*4*5], [3,4*5]),
	a3 = _reshape ([1:3*4*5], [3,4,5]),
     };
   variable i = [1,2];
   variable s1 = struct_filter (s, i; dim=0, copy);
   if ((s1.str != s.str)
       || not _eqs (s1.a1, s.a1[i])
       || not _eqs (s1.a2, s.a2[i,*])
       || not _eqs (s1.a3, s.a3[i,*,*]))
     throw RunTimeError, "filtering on dim=0 failed";

   i = [1,2];
   s1 = struct_filter (s, i; dim=1, copy);
   if ((s1.str != s.str)
       || not _eqs (s1.a1, s.a1)
       || not _eqs (s1.a2, s.a2[*,i])
       || not _eqs (s1.a3, s.a3[*,i,*]))
     throw RunTimeError, "filtering on dim=1 failed";

   i = [1,2];
   s1 = struct_filter (s, i; dim=2, copy);
   if ((s1.str != s.str)
       || not _eqs (s1.a1, s.a1)
       || not _eqs (s1.a2, s.a2)
       || not _eqs (s1.a3, s.a3[*,*,i]))
     throw RunTimeError, "filtering on dim=1 failed";
}

