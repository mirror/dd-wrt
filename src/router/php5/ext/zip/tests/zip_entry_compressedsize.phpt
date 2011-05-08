--TEST--
zip_entry_compressedsize() function
--SKIPIF--
<?php
/* $Id: zip_entry_compressedsize.phpt 260091 2008-05-21 09:27:41Z pajoye $ */
if(!extension_loaded('zip')) die('skip');
?>
--FILE--
<?php
$zip = zip_open(dirname(__FILE__)."/test_procedural.zip");
if (!is_resource($zip)) die("Failure");
$entries = 0;
while ($entry = zip_read($zip)) {
  echo zip_entry_compressedsize($entry)."\n";
}
zip_close($zip);

?>
--EXPECT--
5
4
0
24
