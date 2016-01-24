#! /usr/bin/perl -w

use strict;

my @filters = ("Arrow access of non-pointer \\\(Vstr_iter",
               "Arrow access of non-pointer \\\(Vstr_locale");

my $out = 1;

while (<>)
{
	if (/\w+: [^(]/)
	{
		$out = 1;
		for my $i (@filters)
		{
			if (/$i/)
			{
				$out = 0;
			}
		}
	}
	if ($out)
	{
		print;
	}
}

