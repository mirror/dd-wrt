--TEST--
InterBase: connect, close and pconnect
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php /* $Id: 002.phpt 158958 2004-05-19 08:56:50Z helly $ */

	require("interbase.inc");
    
	ibase_connect($test_base);
	out_table("test1");
	ibase_close();

	$con = ibase_connect($test_base);
	$pcon1 = ibase_pconnect($test_base);
	$pcon2 = ibase_pconnect($test_base);
	ibase_close($con);
	unset($con);
	ibase_close($pcon1);
	unset($pcon1);

	out_table("test1");

	ibase_close($pcon2);
	unset($pcon2);
?>
--EXPECT--
--- test1 ---
1	test table not created with isql	
---
--- test1 ---
1	test table not created with isql	
---
