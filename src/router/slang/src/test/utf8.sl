_debug_info = 1; () = evalfile ("inc.sl");

#ifeval (0 == _slang_utf8_ok)
%#stop
#endif

testing_feature ("utf8");

static define check_sprintf (wc)
{
   variable s1 = sprintf ("%lc", wc);
   variable s2 = eval (sprintf ("\"\\x{%X}\"", wc));
   if (s1 != s2)
     failed ("check_sprintf: %lX", wc);
}

check_sprintf (0xF);
check_sprintf (0xFF);
check_sprintf (0xFFF);
check_sprintf (0xFFFF);
check_sprintf (0xFFFFF);
check_sprintf (0xFFFFFF);
check_sprintf (0xFFFFFFF);
check_sprintf (0x7FFFFFFF);

#iffalse
vmessage ("%s", "\x{F}\n");
vmessage ("%s", "\x{FF}\n");
vmessage ("%s", "\x{FFF}\n");
vmessage ("%s", "\x{FFFF}\n");
vmessage ("%s", "\x{FFFFF}\n");
vmessage ("%s", "\x{FFFFFF}\n");
vmessage ("%s", "\x{FFFFFFF}\n");
vmessage ("%s", "\x{7FFFFFFF}\n");
#endif

if (strlen (char (173)) != 1)
  failed ("strlen (char(173))");

$1 = 0;
try
{
   $1+='­';			       %  2 bytes: 0xC2 an 0xAD
   $1+='\x{AD}';
   $1+='\xAD';
   $1+='\d173';
}
catch AnyError: failed ("To parse 'x'");
if ($1 != 4*173)
  failed ("various character forms");

if ("\u{AD}" != "\xC2\xAD")
{
   failed ("\\u{AD} != \\xC2\\xAD");
}

% If \x{...} is used with 3 or more hex digits, the result is UTF-8,
% regardless of the mode.
if ("\x{0AD}" != "\xC2\xAD")
{
   failed ("\\x{0AD} != \\xC2\\xAD");
}
if ("\x{0AD}" != "\xC2\xAD")
{
   failed ("\\x{00AD} != \\xC2\\xAD");
}

if (strbytelen ("\uAB") != 2)
{
   failed ("\\uAB expected to be 2 bytes");
}

if (_slang_utf8_ok)
{
   if (strbytelen ("\x{AB}") != 2)
     failed ("\\x{AB} expected to be 2 bytes");
}
else
{
   if (strbytelen ("\x{AB}") != 1)
     failed ("\\x{AB} expected to be 1 byte");
}

print ("Ok\n");
exit (0);

