--TEST--
Operator precedence
--FILE--
<?php /* $Id: zend_operators.phpt 242949 2007-09-26 15:44:16Z cvs2svn $ */

var_dump((object)1 instanceof stdClass);
var_dump(! (object)1 instanceof Exception);

?>
--EXPECT--
bool(true)
bool(true)
