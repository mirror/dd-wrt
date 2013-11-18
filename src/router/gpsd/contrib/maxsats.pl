#!/usr/bin/perl

# Copyright (c) 2008 Chris Kuethe <chris.kuethe@gmail.com>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

use strict;
use warnings;

my ($tm, $nsr, $nt, $nu, @TL, @UL, $l);
while (<>){
	next unless (/,Y=\w+ (\d+\.\d+) (\d+):(.+:)/);
	$tm = $1;
	$nsr = $2;
	$l = ":$3:";
	$nt = $nu = 0;
	@TL = @UL = ();
	while ($l =~ /(\d+) \w+ \w+ (\d+) ([01]):/g){
		if ($1 <= 32){ # $1 => prn
			if ($2){ # $2 => snr
				push(@TL, $1);
				$nt++;
			}
			if ($3){ # $3 => used
				push(@UL, $1);
				$nu++;
			}
		}
	}
	print "$tm $nsr nu/nt = $nu/$nt T=\[@TL\] U=\[@UL\]\n" if (($nu >= 10));
}
