#!/usr/bin/perl -w
use strict;

use English;
use Socket qw(inet_ntoa);

my @NP;

die "open proto" if !open(F,'</proc/net/xt_ndpi/proto');
my $l = <F>;
while(<F>) {
	chomp;
	my @a = split /\s+/;
	next if $#a < 3;
	next if $a[1] eq 'disabled';
#	print hex($a[0]),' ',$a[2],"\n";
	$NP[hex($a[0])] = $a[2];
}
close(F);

my $outf = defined $ARGV[0] ? $ARGV[0] : 'ndpi.bin';

die "open out" if !open(O,'<'.$outf);
binmode O;
my ($i,$n,$r) = (0,0,0);
my $buf;
while(1) {
	$n = sysread(O,$buf,4);
	if($n != 4) {
		last if !$n;
		die "read4 ".$?;
	}
	my ($rt,$np,$cl,$hl) = unpack 'CCCC',$buf;
	if(($rt & 0x3) == 1) {
		$n = sysread(O,$buf,4);
		die "t1 read4" if $n != 4;
		my $tm = unpack 'I',$buf;
		print "TIME $tm\n";
	} elsif(($rt & 0x3) == 0) {
		die "t0 hl 0" if !$hl;
		my $nhost;
		$n = sysread(O,$nhost,$hl);
		die "t0 read host" if $n != $hl;
		$NP[$np + $cl * 256] = $nhost;
#		printf("Proto %d %s\n",$np + $cl * 256,$nhost);
	} elsif(($rt & 0x3) == 2) {
		if($rt & 4) {
			die "ipv6";
		}
		my $len = 48-4 + 24 + $cl + $hl;
		$n = sysread(O,$buf,$len);
#		print "flow $len\n";
		die "t2 read" if $n != $len;
		my ($tm1,$cpi,$cpo,$cbi,$cbo,$tm2,$ifidx,$ofidx,$proto_master,$proto_app,
			$connmark,$ip_s,$ip_d,$ip_snat,$ip_dnat,$sport,$dport,$sport_nat,$dport_nat) =
			unpack 'IIIQQISSSSNNNNNnnnn',$buf;
		my ($nhost,$ncert)=('','');
		if($cl > 0) {
			$ncert = ' C='.substr($buf,44+24,$cl);
		}
		if($hl > 0) {
			$nhost = ' H='.substr($buf,44+24+$cl,$hl);
		}
		$ip_s = inet_ntoa substr($buf,44,4);
		$ip_d = inet_ntoa substr($buf,48,4);
		my $nflag = ($rt >> 3) & 0x7;
		my $nat = '';
		if($nflag != 0) {
			$ip_snat = inet_ntoa substr($buf,52,4);
			$ip_dnat = inet_ntoa substr($buf,56,4);
			if($nflag & 1) {
				$nat .= " SN=$ip_snat,$sport_nat";
			}
			if($nflag & 2) {
				$nat .= " DN=$ip_d,$dport";
				$ip_d = $ip_dnat;
				$dport = $dport_nat;
			}
			if($nflag & 4) {
				$nat .= " UI=$ip_snat,$sport_nat";
			}
		}
		if($proto_master > 0) {
			die "$proto_master" if !defined $NP[$proto_master];
			$proto_master = $NP[$proto_master];
		} else {
			$proto_master = '';
		}
		if($proto_app > 0) {
			die "$proto_app" if !defined $NP[$proto_app];
			$proto_app = $NP[$proto_app];
			if($proto_master) {
				$proto_app = ','.$proto_master;
			}
		} else {
			$proto_app = $proto_master ? $proto_master : 'Unknown';
		}
		my $ioif = $ifidx;
		$ioif .= ','.$ofidx if $ofidx != $ifidx;
		print "$tm1 $tm2 4 $np $ip_s $sport $ip_d $dport $cbi $cbo $cpi $cpo I=$ioif$nat P=$proto_app$nhost$ncert\n";
	} elsif(($rt & 0x3) == 3) {
		my $len = 48-4;
		$n = sysread(O,$buf,$len);
		die "t3 read" if $n != $len;
		my ($tm1,$cpi,$cpo,$cbi,$cbo) =
			unpack 'IIIQQ',$buf;
		print "LOST_TRAFFIC $cpi,$cpo,$cbi,$cbo\n";
	} else {
		die "unknown type ".($rt & 0x3)."\n";
	}

}
close(O);
exit(0);
