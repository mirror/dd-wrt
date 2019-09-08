#!/usr/bin/perl -w
use strict;

use English;
use Time::HiRes qw(gettimeofday);

my $outf = defined $ARGV[0] ? $ARGV[0] : 'ndpi.bin';

die "open out" if !open(O,'>'.$outf);
my ($seconds, $microseconds) = gettimeofday;
die "BUG1" if !open(F,'+</proc/net/xt_ndpi/flows');
binmode F;
binmode O;
syswrite(F,"read_all_bin\n");
my (@BUF,@LBUF);
my ($i,$n,$r) = (0,0,0);
while(1) {
	$n = sysread(F,$BUF[$i],65536*4);
	if(defined $n && $n > 0) {
		$LBUF[$i] = $n;
		$i++;
		$r += $n;
	} else {
		last;
	}
}
close(F);
my ($seconds2, $microseconds2) = gettimeofday;

for($n=0; $n < $i; $n++) {
	syswrite(O,$BUF[$n],$LBUF[$n]);

}
close(O);
my $dt = (($seconds2-$seconds)+$microseconds2/1000000) - $microseconds/1000000;
printf STDERR "Read total %d %5.2g msec, speed %5.3g MB/s\n",$r,$dt*1000,$r/$dt/1024/1024;
exit(0);
