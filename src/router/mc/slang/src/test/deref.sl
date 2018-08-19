_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("dereferences");

% Tests go here....

static variable x, a, s;

x = &$1;
@x = 3;
if ($1 != 3)
  failed ("@x=3");
(@x)++;
if ($1 != 4)
  failed ("(@x)++");
(@x)--;
if ($1 != 3)
  failed ("(@x)--");

@x += 2;
if ($1 != 5)
  failed ("@x += 2");

@x *=3;
if ($1 != 15)
  failed("@x *= 3");

@x /= 3;
if ($1 != 5)
  failed ("@x /= 3");

@x &= 0x1;
if ($1 != 1)
  failed ("@x &= 0x1");

@x |= 0x10;
if ($1 != 17)
  failed ("@x |= 0x10");

s = struct {x};
s.x = &$1;

@s.x = 3;
if ($1 != 3)
  failed ("@s.x=3");
(@s.x)++;
if ($1 != 4)
  failed ("(@s.x)++");
(@s.x)--;
if ($1 != 3)
  failed ("(@s.x)--");

@s.x += 2;
if ($1 != 5)
  failed ("@s.x += 2");

@s.x *=3;
if ($1 != 15)
  failed("@s.x *= 3");

@s.x /= 3;
if ($1 != 5)
  failed ("@s.x /= 3");

@s.x &= 0x1;
if ($1 != 1)
  failed ("@s.x &= 0x1");

@s.x |= 0x10;
if ($1 != 17)
  failed ("@s.x |= 0x10");

a = Ref_Type[2];
a[1] = &$1;

@a[1] = 3;
if ($1 != 3)
  failed ("@a[1]=3");
(@a[1])++;
if ($1 != 4)
  failed ("(@a[1])++");
(@a[1])--;
if ($1 != 3)
  failed ("(@a[1])--");

@a[1] += 2;
if ($1 != 5)
  failed ("@a[1] += 2");

@a[1] *=3;
if ($1 != 15)
  failed("@a[1] *= 3");

@a[1] /= 3;
if ($1 != 5)
  failed ("@a[1] /= 3");

@a[1] &= 0x1;
if ($1 != 1)
  failed ("@a[1] &= 0x1");

@a[1] |= 0x10;
if ($1 != 17)
  failed ("@a[1] |= 0x10");

static define return_ref (addr)
{
   if (_NARGS != 1)
     failed ("_NARGS in return_ref");

   return addr;
}

$1 = PI;
if ($1 != @return_ref (&$1))
  failed ("@return_ref");
static variable f = &return_ref;

if ($1 != @@f(&$1))
  failed ("@return_ref");

if ($1 != @(@f)(&$1))
  failed ("@return_ref");

print ("Ok\n");

exit (0);

