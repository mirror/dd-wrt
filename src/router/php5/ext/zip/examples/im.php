<?php
/* $Id: im.php,v 1.1 2006/07/24 16:58:58 pajoye Exp $ */
$im = imagecreatefromgif('zip://' . dirname(__FILE__) . '/test_im.zip#pear_item.gif');
imagepng($im, 'a.png');

$z = new ZipArchive();
$z->open(dirname(__FILE__) . '/test_im.zip');
$im_string = $z->getFromName("pear_item.gif");
$im = imagecreatefromstring($im_string);
imagepng($im, 'b.png');

