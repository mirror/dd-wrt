--TEST--
APC: global spaces (php 5.3)
--SKIPIF--
<?php
    require_once(dirname(__FILE__) . '/skipif.inc'); 
    if(version_compare(zend_version(), '2.3.0') < 0) {
		echo "skip\n";
	}
?>
--INI--
apc.enabled=1
apc.enable_cli=1
apc.file_update_protection=0
--FILE--
<?php

require_once(dirname(__FILE__) . '/php_5_3_ns.inc'); 

$a = new Foo\Bar\Baz();
$a->foo();
var_dump(Foo\Bar\sort());
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
array(4) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
  [3]=>
  int(4)
}
array(4) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
  [3]=>
  int(4)
}
string(8) "IT WORKS"
===DONE===
