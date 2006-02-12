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
##      Convert the output of the LZO test program into a nice table.
##      Option '-b' will print the 'official' LZO benchmark.
##
##  Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
##
##---------------------------------------------------------------------------##

$VERSION = '1.01.5';
$PROG = $0;

require 'ctime.pl';

#
# get options
#

while ($_ = $ARGV[ $[ ], /^-/) {
	shift(@ARGV);
	/^--$/ && ($opt_last = 1, last);

	/^-b/ && ($opt_bench++, next);
	/^-f/ && ($opt_force++, next);
	/^-n/ && ($opt_sort_summary_by_name++, next);
	/^-r/ && ($opt_sort_summary_by_ratio++, next);
	/^-s/ && ($opt_summary_only++, next);
	/^-t/ && ($opt_clear_time++, next);

	/^-D(.*)/ && ($opt_debug = $1 ? $1 : 1);
}


$alg = '';
$sep = "+" . ("-" x 72) . "+\n";

$block_size = -1;

$n = 0;
@algs = ();
%average = ();
%total = ();

$bv_n = 0;
@bv = (0, 0, 0.0, 0.0, 0.0);
%bench = ();
$bench_total = -1;

$lzo_version_string = '';
$lzo_version_date = '';


# benchmark reference values for 100.00

%bench_c = ('LZO1B-1',  1577.00,
            'LZO1B-9',   590.50,
            'LZO1C-1',  1532.50,
            'LZO1C-9',   588.50,
            'LZO1F-1',  1654.50,
            'LZO1X-1',  1598.00 );

%bench_d = ('LZO1B-1',  4381.50,
            'LZO1B-9',  4395.00,
            'LZO1C-1',  4403.00,
            'LZO1C-9',  4408.50,
            'LZO1F-1',  4769.50,
            'LZO1X-1',  4907.00 );


# /***********************************************************************
# //
# ************************************************************************/

while (<>) {

	if (/(^|\s)(\d+)\s+block\-size/i) {
		if ($block_size < 0) {
			$block_size = $2;
			&start($block_size);
		} elsif ($block_size != $2) {
			die "$PROG: block-size: $block_size != $2\n";
		}
		next;
	}

	if (/execution\s+time.*\s+(\d+)\s/i) {
		if ($bench_total < 0) {
			$bench_total = $1;
		} elsif ($bench_total != $1) {
			die "$PROG: execution time: $bench_total != $1\n";
		}
		next;
	}

	if (/^\s*LZO\s.*library\s+\(v\s*([\w\.\s]+)\s*\,\s*([^\)]+)\)/) {
		$lzo_version_string = $1;
		$lzo_version_date = $2;
		next;
	}

	if (/^\s*(\S+(\s+\[\S+\])?)\s*(\|.*\|)\s*$/i) {
		if ($1 ne $alg) {
			&footer($1);
			&header($1);
		}
		$line = $3;
		&stats(*line);
		print "$line\n" if (!$opt_bench && !$opt_summary_only);
	}
}
&footer($1);

&bench() if $opt_bench;
&summary() unless $opt_bench;

exit(0);


# /***********************************************************************
# //
# ************************************************************************/

sub stats {
	local (*l) = @_;
	local ($x1, $x2, $x3, $x4, $x5, $x6, $x7, $x8);

	if ($l !~ /^\|\s*(.*)\s+(\d+)\s+(\d+)\s+(\d+)\s+([\d\.]+\s+)?([\d\.]+\s+)?([\d\.]+)\s+([\d\.]+)\s*\|/) {
		die $_;
	}

	$n++;

	$x1 = $1; $x2 = $2; $x3 = $3; $x4 = $4;
	$x5 = ($x2 > 0) ? $x4 * 100.0 / $x2 : 0.0;
	$x6 = ($x2 > 0) ? $x4 *   8.0 / $x2 : 0.0;
	$x7 = $7; $x8 = $8;

	if ($opt_clear_time && !$opt_bench) {
		$x7 = $x8 = 0.0;
	}

	$s[0] += $x2;
	$s[1] += $x3;
	$s[2] += $x4;
	$s[3] += $x5;
	$s[4] += $x6;
	$s[5] += $x7;
	$s[6] += $x8;

	$x1 =~ s/\s+$//;
	$l = sprintf("| %-14s %8d %4d %8d %6.1f %5.2f %9.2f %9.2f |",
					$x1, $x2, $x3, $x4, $x5, $x6, $x7, $x8);
}


# /***********************************************************************
# //
# ************************************************************************/

sub header {
	local ($t) = @_;

	$alg = $t;

	# reset stats
	$n = 0;
	@s = (0, 0, 0, 0.0, 0.0, 0.0, 0.0);

	return if $opt_bench;

	print "\n$alg\n\n";
	print $sep;
print <<Header;
| File Name        Length  CxB   ComLen  %Remn  Bits   Com K/s   Dec K/s |
| ---------        ------  ---   ------  -----  ----   -------   ------- |
Header
}


# /***********************************************************************
# //
# ************************************************************************/

sub footer {
	local ($t) = @_;
	local ($bv);
	local ($c, $d);

	return unless $alg;
	die if $n <= 0;
	die if $s[0] <= 0;

	push(@algs,$alg);

	$average{$alg} =
		sprintf("| %-14s %8d %4d %8d %6.1f %5.2f %9.2f %9.2f |\n",
			"Average", $s[0]/$n, $s[1]/$n, $s[2]/$n,
			$s[3]/$n, $s[4]/$n,
			$s[5]/$n, $s[6]/$n );

	$total{$alg} =
		sprintf("| %-14s %8d %4d %8d %6.1f %5.2f %9.2f %9.2f |\n",
			"Total", $s[0], $s[1], $s[2],
			$s[2]/$s[0]*100, $s[2]/$s[0]*8,
			$s[5]/$n, $s[6]/$n );

	if ($opt_bench) {

		$c = $bench_c{$alg};
		$d = $bench_d{$alg};

		($c > 0 && $d > 0) || die "$PROG: invalid benchmark algorithm $alg\n";
		$n == 14 || die "$PROG: invalid benchmark suite\n";
		$s[0] == 3141622 || die "$PROG: invalid benchmark length $s[0]\n";

		$bv = ((($s[5]/$c + $s[6]/$d) / $n) / 2.0) * 100.0;

		$bench{$alg} =
			sprintf("| %-14s %10d %10d %10.2f %10.2f %11.2f |\n",
				"Benchmark", $s[0], $s[2],
				$s[5]/$n, $s[6]/$n , $bv);

		$bv_n++;
		$bv[0] += $s[0];
		$bv[1] += $s[2];
		$bv[2] += $s[5]/$n;
		$bv[3] += $s[6]/$n;
		$bv[4] += $bv;

	} else {

		print $sep;
		print $average{$alg};
		print $total{$alg};
		print $sep, "\n";
	}
}


# /***********************************************************************
# //
# ************************************************************************/

$sort_mode = 0;

sub cmp_by_ratio {
	local ($aa, $bb);

	if ($sort_mode == 0) {
		$aa = $average{$a};
		$bb = $average{$b};
	} elsif ($sort_mode == 1) {
		$aa = $total{$a};
		$bb = $total{$b};
	} else {
		die;
	}

	($aa =~ m%^\s*\|\s+\S+\s+\d+\s+\d+\s+\d+\s+(\S+)%) || die;
	$aa = $1;
	($bb =~ m%^\s*\|\s+\S+\s+\d+\s+\d+\s+\d+\s+(\S+)%) || die;
	$bb = $1;

	# $aa < $bb;
	$aa cmp $bb;
}


# /***********************************************************************
# //
# ************************************************************************/

sub bench {
	local ($l);
	local (@k);

	die if $bv_n <= 0;
	die if $bv[0] <= 0;
	if (!$opt_force) {
		$bench_total >= 1200 || die "$PROG: benchmark execution time $bench_total is too short\n";
	}

	@k = @algs;

	print $sep;
print <<Bench;
| Algorithm          Length     ComLen    Com K/s    Dec K/s   LZO bench |
| ---------          ------     ------    -------    -------   --------- |
Bench

	for (@k) {
		$l = $bench{$_};
		$l =~ s/Benchmark[\s]{5}/sprintf("%-14s",$_)/e;
		print $l;
	}
	print $sep;
	printf("| %-14s %10d %10d %-10s %-10s %11.2f |\n",
		"OVERALL", $bv[0], $bv[1], "", "", $bv[4]/$bv_n);
	print $sep;
}


# /***********************************************************************
# //
# ************************************************************************/

sub summary {
	local ($l);
	local (@k);

	$sort_mode = 0;
	if ($opt_sort_summary_by_name) {
		@k = sort(@algs);
	} elsif ($opt_sort_summary_by_ratio) {
		@k = sort(cmp_by_ratio @algs);
	} else {
		@k = @algs;
	}

	print "\n\n";
	print "Summary of average values\n\n";
	print $sep;
print <<Summary;
| Algorithm        Length  CxB   ComLen  %Remn  Bits   Com K/s   Dec K/s |
| ---------        ------  ---   ------  -----  ----   -------   ------- |
Summary

	for (@k) {
		$l = $average{$_};
		$l =~ s/Average[\s]{7}/sprintf("%-14s",$_)/e;
		print $l;
	}
	print $sep;



	$sort_mode = 1;
	if ($opt_sort_summary_by_name) {
		@k = sort(@algs);
	} elsif ($opt_sort_summary_by_ratio) {
		@k = sort(cmp_by_ratio @algs);
	} else {
		@k = @algs;
	}

	print "\n\n";
	print "Summary of total values\n\n";
	print $sep;
print <<Summary;
| Algorithm        Length  CxB   ComLen  %Remn  Bits   Com K/s   Dec K/s |
| ---------        ------  ---   ------  -----  ----   -------   ------- |
Summary

	for (@k) {
		$l = $total{$_};
		$l =~ s/Total[\s]{9}/sprintf("%-14s",$_)/e;
		print $l;
	}
	print $sep;


print <<Summary;


Notes:
- CxB is the number of blocks
- K/s is the speed measured in 1000 uncompressed bytes per second
- all averages are calculated from the un-rounded values

Summary
}


# /***********************************************************************
# //
# ************************************************************************/

sub start {
	local ($bs) = @_;
	local ($v, $t, $x);
	local ($u, $uname_m, $uname_s, $uname_r);

	$t = &ctime(time); chop($t);
	$t = sprintf("%-51s |", $t);

	$v='';
	if ($lzo_version_string) {
		$v = $lzo_version_string;
		$v .= ', ' . $lzo_version_date if $lzo_version_date;
		$v = sprintf("%-51s |", $v);
		$v = sprintf("| LZO version      : %s\n", $v);
	}

	if ($bs % 1024 == 0) {
		$x = sprintf("%d (= %d kB)", $bs, $bs / 1024);
	} else {
		$x = sprintf("%d (= %.3f kB)", $bs, $bs / 1024.0);
	}
	$x = sprintf("%-51s |", $x);

	$u='';
	if (1 == 1) {
		$uname_s = `uname -s`; $uname_s =~ s/^\s+//; $uname_s =~ s/\s+$//;
		$uname_r = `uname -r`; $uname_r =~ s/^\s+//; $uname_r =~ s/\s+$//;
		$uname_m = `uname -m`; $uname_m =~ s/^\s+//; $uname_m =~ s/\s+$//;
		if ($uname_s && $uname_m) {
			$u = $uname_s;
			$u .= ' ' . $uname_r if $uname_r;
			$u .= ' ' . $uname_m;
			$u = sprintf("%-51s |", $u);
			$u = sprintf("| Operating system : %s\n", $u);
		}
	}

	if ($opt_bench) {
		print <<Start;

+------------------------------------------------------------------------+
| LZO 'OFFICIAL' BENCHMARK                                               |
| ========================                                               |
| Time of run      : $t
$v$u| Test suite       : Calgary Corpus Suite                                |
| Files in suite   : 14                                                  |
| Context length   : $x
+------------------------------------------------------------------------+


Start
	} else {
		print <<Start;

+------------------------------------------------------------------------+
| DATA COMPRESSION TEST                                                  |
| =====================                                                  |
| Time of run      : $t
$v$u| Context length   : $x
| Timing accuracy  : One part in 100                                     |
+------------------------------------------------------------------------+


Start
	}
}

__END__


### insert something like this after 'Time of run':

| Hardware         : Intel Pentium 133, 64 MB RAM, 256 kB Cache          |
| Operating system : MS-DOS 7.10, HIMEM.SYS 3.95, DOS/4GW 1.97           |
| Compiler         : Watcom C32 10.5                                     |
| Compiler flags   : -mf -5r -oneatx                                     |
| Test suite       : Calgary Corpus Suite                                |
| Files in suite   : 14                                                  |


