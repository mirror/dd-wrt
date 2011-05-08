--TEST--
Sybase-CT connectivity
--SKIPIF--
<?php require('skipif.inc'); ?>
--FILE--
<?php
/* This file is part of PHP test framework for ext/sybase_ct
 *
 * $Id: test_connect.phpt 242949 2007-09-26 15:44:16Z cvs2svn $ 
 */

  require('test.inc');

  $db= sybase_connect_ex();
  var_dump($db);
  sybase_close($db);
?>
--EXPECTF--
resource(%d) of type (sybase-ct link)
