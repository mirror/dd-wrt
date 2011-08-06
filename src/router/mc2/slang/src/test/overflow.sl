() = evalfile ("inc.sl");

testing_feature ("literal integer overflow");

private define check_overflow (str, overflow)
{
   try
     {
	() = eval (str + ";");
	if (overflow)
	  failed ("Expected %s to generate an overflow error", str);
     }
   catch AnyError:
     {
	if (overflow == 0)
	  failed ("Obtained unexpected overflow error for %s", str);
     }
}
if (Int16_Type == Short_Type)
{
   check_overflow ("123456h", 1);
   check_overflow ("32768h", 1);
   check_overflow ("-32768h", 0);
   check_overflow ("65535hu", 0);
   check_overflow ("0xFFFF1hu", 1);
   check_overflow ("0xFFFFh", 0); % no sign overflow checked for hex literals
   check_overflow ("0xFFFFhu", 0);
}
if (Int32_Type == Int_Type)
{
   check_overflow ("-2147483648", 0);
   check_overflow ("2147483648", 1);
   check_overflow ("4294967295U", 0);
   check_overflow ("4294967296U", 1);
   check_overflow ("0xFFFFFFFF", 0);
   check_overflow ("0xFFFFFFFF1", 1);
   check_overflow ("0xFFFFFFFF1U", 1);
   check_overflow ("0xFFFFFFFFU", 0);
}
print ("Ok\n");

exit (0);

