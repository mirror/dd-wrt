--TEST--
ziparchive::addEmptyDir
--SKIPIF--
<?php
/* $Id: oo_addemptydir.phpt 260091 2008-05-21 09:27:41Z pajoye $ */
if(!extension_loaded('zip')) die('skip');
?>
--FILE--
<?php

$dirname = dirname(__FILE__) . '/';
include $dirname . 'utils.inc';
$file = $dirname . '__tmp_oo_addfile.zip';

copy($dirname . 'test.zip', $file);

$zip = new ZipArchive;
if (!$zip->open($file)) {
	exit('failed');
}

$zip->addEmptyDir('emptydir');
if ($zip->status == ZIPARCHIVE::ER_OK) {
	dump_entries_name($zip);
	$zip->close();
} else {
	echo "failed\n";
}
@unlink($file);
?>
--EXPECTF--
0 bar
1 foobar/
2 foobar/baz
3 entry1.txt
4 emptydir/
