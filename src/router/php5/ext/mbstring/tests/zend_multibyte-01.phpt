--TEST--
zend multibyte (1)
--SKIPIF--
--INI--
zend.multibyte=On
zend.script_encoding=Shift_JIS
mbstring.internal_encoding=Shift_JIS
--FILE--
<?php
	function �\�\�\($����)
	{
		echo $����;
	}

	�\�\�\("�h���~�t�@�\");
?>
--EXPECT--
�h���~�t�@�\
