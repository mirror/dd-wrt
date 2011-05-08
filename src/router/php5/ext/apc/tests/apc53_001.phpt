--TEST--
APC: classes with namespaces (php 5.3)
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
var_dump($a);
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
object(Foo\Bar\Baz)#1 (3) {
  ["i"]=>
  int(1)
  ["f":protected]=>
  float(3.14)
  ["s":"Foo\Bar\Baz":private]=>
  string(11) "hello world"
}
===DONE===
