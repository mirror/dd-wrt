<?php
/* This file is part of PHP test framework for ext/sybase_ct
 *
 * $Id: skipif.inc 268592 2008-11-08 15:39:01Z felipe $ 
 */

  require 'test.inc';

  if (!extension_loaded('sybase_ct')) die('skip Sybase-CT extension not loaded');
  
  sybase_min_server_severity(11);
  if (!@sybase_connect(HOST, USER, PASSWORD)) die('skip Unable to connect');
?>
