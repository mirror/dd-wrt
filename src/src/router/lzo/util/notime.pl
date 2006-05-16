#! /usr/bin/perl
##
## vi:ts=4
##
##---------------------------------------------------------------------------##
##
##  Author:
##      Markus F.X.J. Oberhumer         <markus@oberhumer.com>
##
##  Description:
##      Remove timing values from a table created by table.pl
##
##  Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
##
##---------------------------------------------------------------------------##


while (<>) {

	if (substr($_,52) =~ /^\s+[\d\.]+\s+[\d\.]+\s+\|\s*\n$/i) {
		substr($_,52) = "      0.00      0.00 |\n";
	}
	print;
}

exit(0);

