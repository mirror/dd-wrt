_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("string functions");

% Usage: test (&fun, args, expected_ans);
static define test ()
{
   variable ans = ();
   variable args = __pop_args (_NARGS-2);
   variable fun = ();
   variable result = (@fun) (__push_args(args));
   if (ans != result)
     {
	variable fmt = String_Type[_NARGS-2];
	fmt[*] = "%S";
	fmt = sprintf ("%%S(%s) ==> %%S!=%%S", strjoin (fmt, ","));
	failed (fmt, fun, __push_args(args), result, ans);
     }
}

%test (&str_delete_chars, "foo\xAAbar", "\xAA", "foobar");

static variable s;

s = strcompress (" \t  \tA\n\ntest\t", " \t\n");
if (s != "A test") failed ("strcompress");
s = strcompress ("../afoo/bfoo/cbard/ooohbhar", "/");
if (s != "../afoo/bfoo/cbard/ooohbhar")
  failed ("strcompress 2");

s = " \t hello world\n\t";
if ("hello world" != strtrim (s)) failed ("strtrim");
if ("hello world\n\t" != strtrim_beg (s)) failed ("strtrim_beg");
if (" \t hello world" != strtrim_end (s)) failed ("strtrim_beg");

if ("hello wor" != strtrim (s, " \t\nld")) failed ("strtrim with whitespace");

if ("" != strcat ("", ""))
  failed ("strcat 0");
if ("1" != strcat ("", "1"))
  failed ("strcat 1");

if ("abcdefg" != strcat ("a", "b", "c", "d", "e", "f", "g")) failed ("strcat");
if ("abcdefg" != strcat ("abcdefg")) failed ("strcat 2");

if ((strtok (s)[0] != "hello") 
    or (strtok(s)[1] != "world")
    or (strtok (s, "^a-z")[0] != "hello")
    or (strtok (s, "^a-z")[1] != "world")
    or (2 != length (strtok (s)))
    or (2 != length (strtok (s, "^a-z")))) failed ("strtok");

define test_create_delimited_string ()
{
   variable n = ();
   variable args = __pop_args (_NARGS - 3);
   variable delim = ();
   variable eresult = ();
   variable result;
   
   result = create_delimited_string (delim, __push_args (args), n);
   if (eresult != result)
     failed ("create_delimited_string: expected: %s, got: %s",
	     eresult, result);

   if (n)
     result = strjoin ([__push_args (args)], delim);
   else 
     result = strjoin (String_Type[0], delim);

   if (eresult != result)
     failed ("strjoin: expected: %s, got: %s",
	     eresult, result);
}

	
test_create_delimited_string ("aXXbXXcXXdXXe",
			      "XX",
			      "a", "b", "c", "d", "e",
			      5);


test_create_delimited_string ("", "", "", 1);
test_create_delimited_string ("a", ",", "a", 1);
test_create_delimited_string (",", ",", "", "", 2);
test_create_delimited_string (",,", ",", "", "", "", 3);
test_create_delimited_string ("", "XXX", 0);

static define test_str_delete_chars (str, del_set, ans)
{
   variable s1 = str_delete_chars (str, del_set);
   if (ans != s1)
     failed ("str_delete_chars(%s, %s) --> %s", str, del_set, s1);
}

test_str_delete_chars ("abcdefg", "bdf", "aceg");
test_str_delete_chars ("abcdefg", "^bdf", "bdf");
test_str_delete_chars ("abcdefg", "bdfg", "ace");
test_str_delete_chars ("abcdefg", "ag", "bcdef");
test_str_delete_chars ("abcdefg", "^ag", "ag");
test_str_delete_chars ("abcdefg", "a-z", "");
test_str_delete_chars ("abcdefgABCDEF", "\l"R, "ABCDEF");
test_str_delete_chars ("abcdefgABCDEF", "^\l"R, "abcdefg");


static define test_strtrans (s, from, to, ans)
{
   variable s1 = strtrans (s, from, to);
   if (ans != s1)
     failed ("strtrans(%s, %s, %s) --> %s", s, from, to, s1);
}

test_strtrans ("hello world", "^a-zA-Z", "X", "helloXworld");
test_strtrans ("hello world", "^\a"R, "X", "helloXworld");
test_strtrans ("hello", "", "xxxx", "hello");
test_strtrans ("hello", "l", "", "heo");
test_strtrans ("hello", "helo", "abcd", "abccd");
test_strtrans ("hello", "hl", "X", "XeXXo");
test_strtrans ("", "hl", "X", "");
test_strtrans ("hello", "a-z", "A-Z", "HELLO");
test_strtrans ("he\\o"R, "\\\\", "x", "hexxo");
test_strtrans ("hello", "\l"R, "\u"R, "HELLO");
test_strtrans ("hello", "a-mn-z", "A-MN-Z", "HELLO");
test_strtrans ("hello", "a-mn-z", "\\u", "HELLO");
test_strtrans ("hello", "a-mn-z", "\\u\\l", "HELLo");
test_strtrans ("abcdefg", "a-z", "Z-A", "ZYXWVUT");
%test_strtrans ("hejklo", "k-l", "L---", "hejL-o");
test_strtrans ("hejklo", "k-l", "\\u", "hejKLo");
test_strtrans ("hello", "he", "-+", "-+llo");
test_strtrans ("hello", "", "", "hello");
test_strtrans ("hello", "helo", "", "");
test_strtrans ("hello", "o", "", "hell");
test_strtrans ("hello", "hlo", "", "e");
test_strtrans ("", "hlo", "", "");
test_strtrans ("HeLLo", "A-Ze", "", "o");
test_strtrans ("HeLLo", "^A-Z", "", "HLL");
test_strtrans ("HeLLo", "\\l\\u", "aA", "AaAAa");
test_strtrans ("He11o", "\l\u\d"R, "aAx", "Aaxxa");

define test_str_replace (a, b, c, result, n)
{
   variable new;
   variable m;

   (new, m) = strreplace (a, b, c, n);

   if (new != result)
     failed ("strreplace (%s, %s, %s, %d) ==> %s!=%s", a, b, c, n, new, result);
   
   if (n == 1)
     {
	n = str_replace (a, b, c);
	!if (n) a;
	new = ();
	if (new != result)
	  failed ("str_replace (%s, %s, %s) ==> %s!=", a, b, c, new, result);
     }
}

test_str_replace ("a", "b", "x", "a", 1);
test_str_replace ("a", "b", "x", "a", -1);
test_str_replace ("a", "b", "x", "a", -10);
test_str_replace ("a", "b", "x", "a", 10);
test_str_replace ("a", "b", "x", "a", 0);
test_str_replace ("blafoofbarfoobar", "", "xyyy", "blafoofbarfoobar", 0);
test_str_replace ("blafoofbarfoobar", "", "xyyy", "blafoofbarfoobar", 1);
test_str_replace ("blafoofbarfoobar", "", "xyyy", "blafoofbarfoobar", -1);
test_str_replace ("blafoofbarfoobar", "", "xyyy", "blafoofbarfoobar", -10);

test_str_replace ("blafoofbarfoobar", "foo", "XY", "blafoofbarfoobar", 0);
test_str_replace ("blafoofbarfoobar", "foo", "XY", "blaXYfbarfoobar", 1);
test_str_replace ("blafoofbarfoobar", "foo", "XY", "blaXYfbarXYbar", 2);
test_str_replace ("blafoofbarfoobar", "foo", "XY", "blaXYfbarXYbar", 10);
test_str_replace ("blafoofbarfoobar", "foo", "XY", "blafoofbarXYbar", -1);
test_str_replace ("blafoofbarfoobar", "foo", "XY", "blaXYfbarXYbar", -2);
test_str_replace ("blafoofbarfoobar", "r", "", "blafoofbarfoobar", 0);
test_str_replace ("blafoofbarfoobar", "r", "", "blafoofbafoobar", 1);
test_str_replace ("blafoofbarfoobar", "r", "", "blafoofbafooba", 2);
test_str_replace ("blafoofbarfoobar", "r", "", "blafoofbarfooba", -1);
test_str_replace ("blafoofbarfoobar", "r", "", "blafoofbafooba", -2);
test_str_replace ("bla", "bla", "", "", -2);
test_str_replace ("bla", "bla", "foo", "foo", -2);
test_str_replace ("bla", "bla", "foo", "foo", 1);

define test_strcat ()
{
   % This test generates a combined byte-code.  It is used for leak checking
   variable a = "hello";
   variable b = "world";
   loop (20)
     {
	variable c = a + b;
	a = c;
     }
}
test_strcat ();

static define test_str_uncomment_string (s, beg, end, result)
{
   variable r = str_uncomment_string (s, beg, end);
   if (r != result)
     {
	failed ("str_uncomment_string(%s,%s,%s)==>%s!=%s",
		s, beg, end, r, result);
     }
}

test_str_uncomment_string ("Ab(cd)e", "(",")", "Abe");
test_str_uncomment_string ("(Ab(cd)e", "(",")", "e");
test_str_uncomment_string ("(Abcde)", "(",")", "");
test_str_uncomment_string ("(Ab[cde)[def]g", "([",")]", "g");

static define test_str_quote_string (str, qlis, quote, result)
{
   variable r = str_quote_string (str, qlis, quote);
   if (r != result)
     {
	failed ("str_quote_string (%s,%s,%d)==>%s!=%s",
		str, qlis, quote, r, result);
     }
}

test_str_quote_string ("hello", "lh", 'X', "XheXlXlo");
#ifeval _slang_utf8_ok
test_str_quote_string ("hel\u{1234}o", "lh\u{1234}", 0x2345, "\u{2345}he\u{2345}l\u{2345}\u{1234}o");
#endif

private variable D, S;

foreach D ([',', 0xAB, 0xABCD])
{
   if ((D > 0xFF) and (_slang_utf8_ok == 0))
     continue;

   S = "foo0,bar1,baz2,,bing4,5,,,8,";
   test_str_replace ((S, ",", sprintf ("%c", D),
		     sprintf ("foo0%sbar1%sbaz2%s%sbing4%s5%s%s%s8%s",
			      char(D),char(D),char(D),char(D),char(D),char(D),
			      char(D),char(D),char(D)),
		      strlen (S)));
   
   (S,) = strreplace (S, ",", sprintf ("%c", D), strlen (S));
   test (&extract_element,(S,0,,D,), "foo0");
   test (&extract_element,(S,1,,D,), "bar1");
   test (&extract_element,(S,3,,D,), "");
   test (&extract_element,(S,4,,D,), "bing4");
   test (&extract_element,(S,5,,D,), "5");
   test (&extract_element,(S,6,,D,), "");
   test (&extract_element,(S,7,,D,), "");
   test (&extract_element,(S,8,,D,), "8");
   test (&extract_element,(S,9,,D,), "");

   test(&is_list_element, (S, "bar1", ,D,), 1+1);
   test(&is_list_element, (S, "goo", ,D,), 0);
   test(&is_list_element, (S, "8", ,D,), 8+1);
   test(&is_list_element, (S, "", ,D,), 3+1);
   
   S = ",1,";
   (S,) = strreplace (S, ",", sprintf ("%c", D), strlen (S));
   test (&extract_element,(S,0,,D,), "");
   test (&extract_element,(S,1,,D,), "1");
   test (&extract_element,(S,2,,D,), "");
   S = "";
   (S,) = strreplace (S, ",", sprintf ("%c", D), strlen (S));
   test (&extract_element,(S,0,,D,), "");
   test (&extract_element,(S,1,,D,), NULL);
   S = ",1";
   (S,) = strreplace (S, ",", sprintf ("%lc", D), strlen (S));
   test (&extract_element,(S,0,,D,), "");
   test (&extract_element,(S,1,,D,), "1");
}

static define test_strncmp (a, b, n, ans)
{
   variable ans1 = strncmp (a, b, n);
   if (ans1 != ans)
     failed ("strncmp (%s,%s,%s)", a, b, n);
}

test_strncmp ("ignore_all", "ign", 3, 0);
test_strncmp ("ign", "ignore_all", 3, 0);

static define test_strchop (s, d, len, nth, nth_val)
{
   variable a = strchop (s, d, 0);
   if (length (a) != len)
     failed ("strchop (%S,%S,0) ==> %S", s,d,a);
   
   if (a[nth] != nth_val)
     failed ("strchop (%S,%S,0)[%d] ==> %S, not %S", s,d,nth,a[nth],nth_val);
}
test_strchop ("{{{\r}}}\r\rX", '\r', 4, 0, "{{{");
test_strchop ("{{{\r}}}\r\rX", '\r', 4, 1, "}}}");
test_strchop ("{{{\r}}}\r\rX", '\r', 4, 2, "");
test_strchop ("{{{\r}}}\r\rX", '\r', 4, 3, "X");

test_strchop ("\r{{{\r}}}\r\rX", '\r', 5, 0, "");
test_strchop ("\r{{{\r}}}\r\rX", '\r', 5, 1, "{{{");
test_strchop ("", '\r', 1, 0, "");
test_strchop ("\r", '\r', 2, 0, "");
test_strchop ("\r", '\r', 2, 1, "");

static define test_substr (fun, s, n, len, ret)
{
   variable ret1 = (@fun) (s, n, len);
   if (ret1 != ret)
     failed ("$fun($s,$n,$len)==>$ret1 not $ret"$);
}
test_substr (&substr, "To be or not to be", 7, 5, "or no");
test_substr (&substr, "", 1, -1, "");
test_substr (&substr, "A", 1, -1, "A");
test_substr (&substr, "A", 1, 0, "");
test_substr (&substr, "A", 2, 1, "");

test_substr (&substrbytes, "To be or not to be", 7, 5, "or no");
test_substr (&substrbytes, "", 1, -1, "");
test_substr (&substrbytes, "A", 1, -1, "A");
test_substr (&substrbytes, "A", 1, 0, "");
test_substr (&substrbytes, "A", 2, 1, "");

static define test_strsub (fun, s, pos, ch, ret)
{
   variable ret1 = (@fun)(s, pos, ch);
   if (ret1 != ret)
     failed ("$fun($s,$pos,$ch) ==> $ret1 not $ret"$);
}

test_strsub (&strsub, "A", 1, 0, "");
test_strsub (&strsub, "AB", 1, 'a', "aB");
test_strsub (&strsub, "AB", 2, 0, "A");

test_strsub (&strbytesub, "A", 1, 0, "");
test_strsub (&strbytesub, "AB", 1, 'a', "aB");
test_strsub (&strbytesub, "AB", 2, 0, "A");

private define test_foreach ()
{
   variable X = "ab\xAA\x{BB}";
   variable utf8_X = {'a', 'b', -0xAA, 0xBB};
   % Note that \x{BB} varies according to the UTF-8 mode
   variable xi;
   foreach xi (X)
     {
	if (typeof (xi) != UChar_Type)
	  failed ("foreach (String_Type) failed to produce UChar_Types");
     }
   foreach (X) using ("bytes")
     {
	xi = ();
	if (typeof (xi) != UChar_Type)
	  failed ("foreach (String_Type) using bytes failed to produce UChar_Types");
     }

   variable i = 0;
   foreach xi (X) using ("chars")
     {
	if (_slang_utf8_ok)
	  {
	     if (xi != utf8_X[i])
	       failed ("foreach (String_Type) using chars failed at i=%d", i);
	  }
	else if (xi != X[i])
	  failed ("foreach (String_Type) using chars failed at i=%d", i);
	i++;
     }
}
test_foreach ();


define test_char (c, s)
{
   variable cs = char (c);
   variable ss = sprintf ("%b", -c);
   if (s != cs)
     failed ("char(%d) ==> %s, not %s as expected", c, cs, s);

   if (s != ss)
     failed ("sprintf using %%b with %d ==> %s, not %s as expected",
	     -c, cs, s);
}
test_char (-0x78, "\x78");
test_char (-0xAB, "\xAB");

#ifexists Double_Type
_for $1 (0, 4000, 10)
{
   () = sprintf ("%f", 10^$1);
   () = sprintf ("%f", -10^$1);
   () = sprintf ("%f", 10^-$1);
   () = sprintf ("%f", -10^-$1);
}
#endif
	 

define test_count_occur (func, s, ch, ans)
{
   variable n = (@func) (s, ch);
   if (ans != n)
     {
	failed ("%S failed on %s: expected %u, got %u", func, s, ans, n);
     }
}

test_count_occur (&count_char_occurances, "", 'A', 0);
test_count_occur (&count_char_occurances, "A", 'A', 1);
test_count_occur (&count_char_occurances, " A", 'A', 1);
test_count_occur (&count_char_occurances, "A ", 'A', 1);
test_count_occur (&count_char_occurances, "A A", 'A', 2);
test_count_occur (&count_char_occurances, "A  A ", 'A', 2);

test_count_occur (&count_byte_occurances, "", 'A', 0);
test_count_occur (&count_byte_occurances, "A", 'A', 1);
test_count_occur (&count_byte_occurances, " A\0", 'A', 1);
test_count_occur (&count_byte_occurances, "A ", 'A', 1);
test_count_occur (&count_byte_occurances, "A \0A", 'A', 2);
test_count_occur (&count_byte_occurances, "A \0 A ", 'A', 2);

if (_slang_utf8_ok)
{
   test_count_occur (&count_char_occurances, "", 0xFF, 0);
   test_count_occur (&count_char_occurances, "\u{00FF}", 0xFF, 1);
   test_count_occur (&count_char_occurances, " \u{00FF}", 0xFF, 1);
   test_count_occur (&count_char_occurances, "\u{00FF} ", 0xFF, 1);
   test_count_occur (&count_char_occurances, "\u{00FF} \u{00FF}", 0xFF, 2);
   test_count_occur (&count_char_occurances, "\u{00FF}  \u{00FF} ", 0xFF, 2);
}

print ("Ok\n");
exit (0);
