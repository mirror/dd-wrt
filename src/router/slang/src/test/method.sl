_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("method calls");

static variable S = struct
{
   f, a
};

static variable T = struct
{
   g, b
};

static define method_0 (s)
{
   if ((_NARGS != 1) and (s != S))
     failed ("method_0: %S: %S", _NARGS, s);
}
S.f = &method_0;
S.f();

static define method_1 (s,pi)
{
   if ((_NARGS != 2) and (s != S))
     failed ("method_1");

   if (pi != PI)
     failed ("method_1");
}

S.f = &method_1;
S.f(PI);

static define method_2 (s,null,pi)
{
   if ((_NARGS != 3) and (s != S) and (null != NULL))
     failed ("method_2");

   if (pi != PI)
     failed ("method_2");
}

S.f = &method_2;
S.f(,PI);

static define method_t (s)
{
   return s.a;
}

static define method_g (t, a)
{
   if ((_NARGS != 2) and (t != T))
     failed ("method_g");

   return a;
}
T.g = &method_g;
S.f = &method_t;
S.a = T;

if (T != S.f().g (T))
  failed ("s.f().g T");

if (sin(1) != S.f().g (sin(1)))
  failed ("s.f().g sin(1)");

S.f = T;
if (PI != S.f.g (PI))
  failed ("s.f.g PI");

S.f = Struct_Type[1];
S.f[0] = T;

if (PI != S.f[0].g (PI))
  failed ("s.f[0].g PI");

print ("Ok\n");

exit (0);

