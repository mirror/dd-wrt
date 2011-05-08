--TEST--
Sybase-CT close default connection
--SKIPIF--
<?php require('skipif.inc'); ?>
--FILE--
<?php
/* This file is part of PHP test framework for ext/sybase_ct
 *
 * $Id: test_close_default.phpt 268631 2008-11-09 11:38:17Z thekid $ 
 */

  require('test.inc');

  sybase_connect_ex();
  sybase_close();
  var_dump(sybase_query('select getdate()'));
?>
--EXPECTF--

Warning: sybase_query(): Sybase:  No connection in %s on line %d
bool(false)
