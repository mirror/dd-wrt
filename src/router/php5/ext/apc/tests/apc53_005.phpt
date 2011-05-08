--TEST--
APC: goto (php 5.3)
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

$i = 0;
a: 
$i++;
if($i % 3 == 0) goto b;
echo "$i\n";
b:
if($i < 10) goto a;
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
1
2
4
5
7
8
10
===DONE===
