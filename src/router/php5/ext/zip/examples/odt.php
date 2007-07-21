<?php
/* $Id: odt.php,v 1.1 2006/07/24 16:58:58 pajoye Exp $ */
$reader = new XMLReader();

$reader->open('zip://' . dirname(__FILE__) . '/test.odt#meta.xml');
$odt_meta = array();
while ($reader->read()) {
	if ($reader->nodeType == XMLREADER::ELEMENT) {
		$elm = $reader->name;
	} else {
		if ($reader->nodeType == XMLREADER::END_ELEMENT && $reader->name == 'office:meta') {
			break;
		}
		if (!trim($reader->value)) {
			continue;
		}
		$odt_meta[$elm] = $reader->value;
	}
}
print_r($odt_meta);
