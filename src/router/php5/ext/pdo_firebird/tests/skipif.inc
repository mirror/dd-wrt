<?php /* $Id: skipif.inc 161049 2004-06-11 01:37:06Z abies $ */

if (!extension_loaded("interbase") || !extension_loaded("pdo_firebird")) print "skip"; 

?>
