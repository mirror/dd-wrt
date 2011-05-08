--TEST--
zip_entry_read() function
--SKIPIF--
<?php
/* $Id: zip_entry_read.phpt 260091 2008-05-21 09:27:41Z pajoye $ */
if(!extension_loaded('zip')) die('skip');
?>
--FILE--
<?php
$zip    = zip_open(dirname(__FILE__)."/test_procedural.zip");
$entry  = zip_read($zip);
if (!zip_entry_open($zip, $entry, "r")) die("Failure");
echo zip_entry_read($entry);
zip_entry_close($entry);
zip_close($zip);

?>
--EXPECT--
foo
