#!/usr/bin/perl -w
use English;

my @P;
my %N;
my $m=0;
my ($n,$p);
my (%L,%SL);
die "BUG2" if !open(F,'<../src/lib/ndpi_main.c');
while(<F>) {
	next if !/ndpi_init_ptree_ipv/;
	next if !/ndpi_init_ptree_ipv[46]/;
	next if /static void ndpi_init_ptree_ipv/ || /, host_protocol_list/;
	if( /ndpi_str->ip_risk->v[46],\s*(ndpi_[a-z-0-9_]+)/) {
		print STDERR "SKIP '$1' $_";
		$SL{$1} = 1;
		next;
	}
	if(/^\s+ndpi_init_ptree_ipv4\s*\(\s*ndpi_str->protocols->v4,\s*(ndpi_[a-z-0-9_]+)\s*\)/) {
		$L{$1} = 1;
		print STDERR "Found4 $1\n";
		next;
	}
	if(/^\s+ndpi_init_ptree_ipv6\s*\(\s*ndpi_str,\s*ndpi_str->protocols->v6,\s*(ndpi_[a-z-0-9_]+)\s*\)/) {
		$L{$1} = 1;
		print STDERR "Found6 $1\n";
		next;
	}
	die "ndpi_init_ptree_ipv ",$_;
}
close(F);
die "BUG1" if !open(F,'<../src/include/ndpi_protocol_ids.h');

while(<F>) {
	next if !/^\s*NDPI_(CONTENT|SERVICE|PROTOCOL)_(\S+)\s*=\s*(\d+)\s*,/;
	next if $2 eq "HISTORY_SIZE";
	next if $1 eq "PROTOCOL" && $2 eq "SIZE";
	($p,$n) = ($1.'_'.$2,$3);
	if(defined $P[$n]) {
		print STDERR "BUG! $p ($n) redefined $P[$n]. Skip it!\n"
			if !($p eq 'PROTOCOL_UNKNOWN' && !$n);
		next;
	}
	$P[$n]=$p;
	$N{$p}=$n;
	$m = $n if $n > $m;
	print STDERR "Found $p $n\n";
}
close(F);
my (@inclist,@iplist4,@iplist6,@iplist4_l,@iplist6_l);
foreach my $ips (glob('../src/lib/inc_generated/*.c.inc')) {
	#next if $ips =~ /ndpi_crawlers_match|ndpi_amazon_aws_api_gatewy_match|ndpi_icloud_private_relay_match/;
	open(F,'<'.$ips) || die "open $ips $!";
	my $found = 0;
	foreach my $i (grep /^\s*static\s+(ndpi_network6?)\s+([a-zA-Z0-9_]+)\s*\[/,<F>) {
		die "Bad $i" if $i !~ /^\s*static\s+(ndpi_network6?)\s+([a-zA-Z0-9_]+)\s*\[/;
		if(!defined $L{$2}) {
			print "SKIP $1 $2\n";
			next;
		}
		if(defined $SL{$2}) {
			print "SKIP $1 $2\n";
			next;
		}
		if($1 eq 'ndpi_network') {
			push @iplist4,"\&$2\[0\]";
			push @iplist4_l,"\"$ips\"";
			$found = 1;
		} elsif ($1 eq 'ndpi_network6') {
			push @iplist6,"\&$2\[0\]";
			push @iplist6_l,"\"$ips\"";
			$found = 1;
		} else { die "$i"; }
	}
	push @inclist,"\#include \"$ips\"\n" if $found;
}

if(open(F,'>ndpi_network_list_compile.h')) {
	print F "#define NDPI_LAST_IMPLEMENTED_PROTOCOL $m\n\n";
	print F "const char *proto_def[NDPI_LAST_IMPLEMENTED_PROTOCOL+1] = {\n\n";

	for(my $i=0; $i <= $m; $i++) {
		print F "_P(NDPI_$P[$i])",$i == $m ? '':',',"\n";
	}
	print F "\n};\n";
	print F join('',@inclist),"\n";
	print F "static ndpi_network * ip4list[] = {\n";
	print F " ".join(",\n ",@iplist4)."\n};\n";
	print F "static char * ip4list_file[] = {\n";
	print F " ".join(",\n ",@iplist4_l)."\n};\n";
	print F "static ndpi_network6 * ip6list[] = {\n";
	print F " ".join(",\n ",@iplist6)."\n};\n";
	print F "static char * ip6list_file[] = {\n";
	print F " ".join(",\n ",@iplist6_l)."\n};\n";
	close(F);
}

exit(0);
