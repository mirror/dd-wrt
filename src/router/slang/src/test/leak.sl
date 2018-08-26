% This used to produce a subtle leak.
typedef struct
{
   x, y, z
}
Vector_Type;

static define vector_chs (v)
{
   v = @v;
   v.x = -v.x;
   v.y = -v.y;
   v.z = -v.z;
   return v;
}
static define vector (x, y, z)
{
   variable v = @Vector_Type;
   v.x = x;
   v.y = y;
   v.z = z;
   return v;
}

__add_unary ("-", Vector_Type, &vector_chs, Vector_Type);

static variable Y = vector (4, 5, 6);
static variable X = Vector_Type[3];
X[0] = vector (1,2,3);
X[1] = vector (1,2,3);
X[2] = vector (1,2,3);
Y = -X;
