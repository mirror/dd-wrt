--TEST--
APC: anonymous functions (php 5.3)
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

$greet = function($name)
{
    printf("Hello %s\r\n", $name);
};

$greet('World');
$greet('PHP');

?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
Hello World
Hello PHP
===DONE===
