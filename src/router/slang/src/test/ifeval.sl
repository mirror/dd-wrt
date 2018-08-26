#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
_debug_info = 1; () = evalfile ("inc.sl");
#else
failed("#else");
#endif

#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
testing_feature ("#ifeval");
#else
failed("#else");
#endif

#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
define
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
check_typeof
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
(expr,
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
 type)
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
{
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
   if
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
     (typeof (
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
	      expr)
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
      != type)
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
     failed (
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
"typeof " + string (
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
type) + " found " + string (
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
typeof(
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
expr)));
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
}
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
static variable
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
Silly = [
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
1,
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
2,
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
3,
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
4,
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
5,
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
6];
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
if (length (Silly) != 6)
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
  failed ("Silly Array");
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
Silly = [1:
#else
failed("#else");
#endif
#ifeval variable XXX = [1:3]; XXX = [1,2,3]; length(XXX);
10];
#else
failed("#else");
#endif
#ifeval variable XXX = [1:3]; XXX = [1,2,3]; length(XXX);
if (length (Silly) != 10) failed ("[1:10]");
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
Silly = struct
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
{
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
   a,
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {x = struct { c, d, a}; return 1;} crazy (0,0,0,0);
   b,
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
   c
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
};
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
Silly.a = "hello";
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
define check_bool (
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
i)
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
{
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
   check_typeof (
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
i == i,
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
 Char_Type);
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
}
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
define check_result (
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
i,
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
 j,
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
 k)
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
{
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
   if (
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
k != i + j)
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
     failed (
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
sprintf(
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
"%S + %S != %S",
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
 typeof (
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
i),
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
 typeof(
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
j),
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
 typeof(
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
k)));
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
}
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
check_typeof(
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
'a',
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
 UChar_Type);
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
check_typeof (
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
'a' + 'b',
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
 Integer_Type);
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
check_typeof (
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
1h + 'b',
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
 Integer_Type);
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
if (
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
Integer_Type == Short_Type) check_typeof (
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
1hu + 'b',
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
 UInteger_Type);
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
else check_typeof (
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
1hu + 'b',
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
 Integer_Type);
#else
failed("#else");
#endif

print (
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
       "Ok\n");
exit (
#else
failed("#else");
#endif
#ifeval define crazy (x,y,z,w) {return 1;} crazy (0,0,0,0);
0);
#else
failed("#else");
#endif

failed ("Should not see this!!!");
