_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("pack and unpack functions");

static variable is_lil_endian = (pack ("j", 0xFF)[0] == 0xFF);

static define test_pack ()
{
   variable str;
   variable fmt, val, args;

   args = __pop_args (_NARGS - 2);
   (fmt, val) = ();

   str = pack (fmt, __push_args (args));
   if (typeof (str) != BString_Type)
     failed ("pack did not return a bstring for format = " + fmt);
   if (str != val)
     failed ("pack returned wrong result for format = "
	     + fmt + ":" + str);
}

variable X = 0x12345678L;
variable S = "\x12\x34\x56\x78";
if (is_lil_endian) S = "\x78\x56\x34\x12";

test_pack (">k", "\x12\x34\x56\x78", X);
test_pack ("<k", "\x78\x56\x34\x12", X);
test_pack ("=k", S, X);

test_pack ("c", "X", 'X');
test_pack ("cc", "XY", 'X', 'Y');
test_pack ("c4", "ABCD", 'A', ['B', 'C'], 'D', 'E');
test_pack ("xx c xx c2 x >j1", "\0\0A\0\0BC\0\xD\xE", 'A', ['B', 'C'], 0x0D0E);

test_pack ("s4", "1234", "123456");
test_pack ("z4", "123\0", "123456");
test_pack ("S4", "1234", "123456");
test_pack ("s10", "1234\0\0\0\0\0\0", "1234");
test_pack ("S10", "1234      ", "1234");

define test_unpack1 (fmt, str, x, type)
{
   variable xx;

   x = typecast (x, type);

   xx = unpack (fmt, str);

   if (length (where(xx != x)))
     failed ("unpack returned wrong result for " + fmt + ":" + string (xx));
}

#ifexists Double_Type
X = 3.14; if (X != unpack ("d", pack ("d", X))) failed ("pack->unpack for d");
X = 3.14f; if (X != unpack ("f", pack ("f", X))) failed ("pack->unpack for f");
#endif

test_unpack1 (">j", "\xAB\xCD"B, 0xABCD, Int16_Type);
test_unpack1 (">k", "\xAB\xCD\xEF\x12"B, 0xABCDEF12L, Int32_Type);
test_unpack1 ("<j", "\xCD\xAB"B, 0xABCD, Int16_Type);
test_unpack1 ("<k", "\x12\xEF\xCD\xAB"B, 0xABCDEF12L, Int32_Type);

define test_unpack2 (fmt, a, type)
{
   test_unpack1 (fmt, pack (fmt, a), a, type);
}

test_unpack2 ("c5", [1,2,3,4,5], Char_Type);
test_unpack2 ("C5", [1,2,3,4,5], UChar_Type);
test_unpack2 ("h5", [1,2,3,4,5], Short_Type);
test_unpack2 ("H5", [1,2,3,4,5], UShort_Type);
test_unpack2 ("i5", [1,2,3,4,5], Int_Type);
test_unpack2 ("I5", [1,2,3,4,5], UInt_Type);
test_unpack2 ("l5", [1,2,3,4,5], Long_Type);
test_unpack2 ("L5", [1,2,3,4,5], ULong_Type);
#ifexists LLong_Type
test_unpack2 ("m5", [1,2,3,4,5], ULLong_Type);
test_unpack2 ("M5", [1,2,3,4,5], ULLong_Type);
#endif
#ifexists Double_Type
test_unpack2 ("f5", [1,2,3,4,5], Float_Type);
test_unpack2 ("d5", [1,2,3,4,5], Double_Type);
#endif

test_unpack1 ("s4", "ABCDEFGHI", "ABCD", String_Type);
test_unpack1 ("S4", "ABCDEFGHI", "ABCD", String_Type);
test_unpack1 ("z4", "ABCDFGHI", "ABCD", String_Type);
test_unpack1 ("s5", "ABCD FGHI", "ABCD ", String_Type);
test_unpack1 ("S5", "ABCD FGHI", "ABCD", String_Type);
test_unpack1 ("S5", "ABCD\0FGHI", "ABCD", BString_Type);
test_unpack1 ("z5", "ABCD\0FGHI", "ABCD", BString_Type);
test_unpack1 ("s5", "ABCD\0FGHI", "ABCD\0", BString_Type);
test_unpack1 ("S5", "          ", "", String_Type);

define test_unpack3 (fmt, a, b)
{
   variable c, d;
   variable s;

   (c, d) = unpack (fmt, pack (fmt, a, b));
   if ((a != c) or (b != d))
     failed ("%s", "unpack failed for $fmt, found ($a!=$c) or ($b!=$d)"$);
}

#ifexists Double_Type
test_unpack3 ("x x h1 x x20 d x", 31h, 41.7);
test_unpack3 ("x x S20 x x20 d x", "FF", 41.7);
test_unpack3 ("x x d0d0d0d0 S20 x x20 d x", "FF", 41.7);
test_unpack3 ("x x0 S20 x x20 d x", "FF", 41.7);
test_unpack3 ("x x0 s5 x x20 d x", "FF\0\0\0", 41.7);
test_unpack3 ("x x0 z5 x x20 f x", "FF", 41.7f);
#endif

print ("Ok\n");
exit (0);

