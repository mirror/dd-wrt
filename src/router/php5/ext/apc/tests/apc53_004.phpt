--TEST--
APC: closures (php 5.3)
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
function multiplier($n) {
	return function($i) use ($n) {
		return $n * $i;
	};
}

$doubler = multiplier(2);
$tripler = multiplier(3);

echo "double of 9 is ".$doubler(9)."\n";
echo "triple of 4 is ".$tripler(4)."\n";
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
double of 9 is 18
triple of 4 is 12
===DONE===
