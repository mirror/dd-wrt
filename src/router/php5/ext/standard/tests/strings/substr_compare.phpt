--TEST--
substr_compare()
--FILE--
<?php

var_dump(substr_compare("abcde", "df", -2) < 0);
var_dump(substr_compare("abcde", "bc", 1, 2));
var_dump(substr_compare("abcde", "bcg", 1, 2));
var_dump(substr_compare("abcde", "BC", 1, 2, true));
var_dump(substr_compare("abcde", "bc", 1, 3));
var_dump(substr_compare("abcde", "cd", 1, 2));
var_dump(substr_compare("abcde", "abc", 5, 1));
var_dump(substr_compare("abcde", "abcdef", -10, 10));
var_dump(substr_compare("abcde", "abc", 0, 0));
var_dump(substr_compare("abcde", -1, 0, NULL, new stdClass));
echo "Test\n";
var_dump(substr_compare("abcde", "abc", 0, -1));
var_dump(substr_compare("abcde", "abc", -1, NULL, -5));
var_dump(substr_compare("abcde", -1, 0, "str", new stdClass));

echo "Done\n";
?>
--EXPECTF--
bool(true)
int(0)
int(0)
int(0)
int(1)
int(-1)

Warning: substr_compare(): The start position cannot exceed initial string length in %s on line %d
bool(false)
int(-1)
int(0)

Warning: substr_compare() expects parameter 5 to be boolean, object given in %s on line %d
bool(false)
Test

Warning: substr_compare(): The length must be greater than or equal to zero in %s on line %d
bool(false)
int(0)

Warning: substr_compare() expects parameter 4 to be long, string given in %s on line %d
bool(false)
Done
