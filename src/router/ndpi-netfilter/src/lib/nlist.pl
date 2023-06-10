#!/usr/bin/perl -w

use strict;

my %P;


match_inc('ndpi_content_match.c.inc','host_protocol_list');
foreach my $f (glob('inc_generated/*.c.inc')) {
	next if $f eq 'inc_generated/ndpi_dga_match.c.inc';
#	next if $f eq 'ndpi_content_match.c.inc';
#	next if $f !~ /^ndpi_([a-z0-9_]+)match.c.inc$/;
	next if $f eq 'inc_generated/ndpi_icloud_private_relay_match.c.inc';
	next if $f eq 'inc_generated/ndpi_crawlers_match.c.inc';
	next if $f eq 'inc_generated/ndpi_gambling_match.c.inc';
	match_inc($f,"ndpi_protocol_.*_protocol_list");
}

foreach my $proto (qw(AMAZON_AWS MICROSOFT_AZURE CLOUDFLARE MINING MICROSOFT_365 MS_ONE_DRIVE MS_OUTLOOK SKYPE_TEAMS TOR WHATSAPP ZOOM other)) {
	my $f = 'ndpi_network_list_'.lc($proto).'.yaml';
	if(-f $f) {
		print STDERR "$f exists!\n";
#		next;
	}
	die "create file $f" if !open(F,'>'.$f);
	foreach (sort keys %P) {
		print_proto(*F,$P{$_}) if $_ eq $proto || $proto eq 'other';
	}
	close(F);
	print STDERR "Write $f OK\n";
}

exit(0);

sub match_inc {
my ($fn,$listname) = @_;
my $st = 0;
my $cmmnt = '';
my ($ev1, $ev2) = (0,0);
print STDERR "Start $fn $listname\n";
die "open $fn: ".$! if !open(F,'<'.$fn);
die "create ${fn}.new: ".$! if !open(R,'>'.$fn.'.new');
while(<F>) {
	chomp();

	if(!$st) {
		next if /^#if\s0$/;
		if(/ndpi_network\s+$listname\[/) {
			$ev1++;
			print STDERR "host_protocol_list start\n";
			print R "#if 0\n";
			print R "$_\n";
			$st = 1;
			next;
		}
		print R "$_\n";
		next;
	}
#	print "# ${st}: $_\n" if $st < 3;
	print R "$_\n" if $st != 4;
	if($st == 1) {
		if(/^\s*$/) {
			next;
		}
		if(m,^\s*/\*,) {
			$cmmnt = '';
			if(m,/\*\s*(.*)\s*\*/,) {
				$cmmnt .= $1."\n";
# print "# ${st}: short comment $1\n";
			} else {
				$st = 2;
			}
			next;
		}
		if(/^\s*\{/) { # No comments. Start network address
			if(add_net(\$cmmnt,$_)) {
				$st = 3;
				next;
			}
			next;
		}
		if(/^\s*#if\s+0/) {
				do {
					$_ = <F>;
					print R "$_";
				} while($_ !~ /^\s*#endif/);
			next;
		}
		if(/#ifdef CUSTOM_NDPI_PROTOCOLS/) {
				$_ = <F>;
				die if !/#include/;
				print R "$_";
				$_ = <F>;
				die if !/#endif/;
				print R "$_";
				next;
		}
		die "?: ".$_;
	}
	if($st == 2) { # comment
		if(m,\*/,) {
			$st = 1;
#print "# ${st}: stop comment. Result: $cmmnt \n";
			next;
		}
		s/^\s*(.*)\s*$/$1/;
		$cmmnt .= $_."\n";
#print "# ${st}: part of comment. Result: $_ \n";
		next;
	}
	if($st == 3) {
		if(/\};/) {
			$st = 4;
			$ev2++;
			print STDERR "list end\n";
#			print R "$_\n";
			print R "#endif\n";
			next;
		}
		next;
	}
	if($st == 4) {
		if($ev2 == 1 && /^#endif$/) {
				$ev2 = 2;
				next;
		}
		if(/ndpi_category_match\s+category_match\[\]\s+=\s+{/) {
			print STDERR "ndpi_category_match category_match start\n";
			print R "#ifndef __KERNEL__\n";
			print R "$_\n";
			$st = 5;
			next;
		}
		if(/^static\s+ndpi_protocol_match\s+host_match\[\]\s+=\s+{/) {
				s/^static\s+//;
				print STDERR "Remove static host_match\n";
		}
		print R "$_\n";
		next;
	}
	if($st == 5) {
		if(/\};/) {
			$st = 4;
			$ev2++;
			print STDERR "list end\n";
			print R "#endif\n";
			next;
		}
		next;
	}
}
close(R);
close(F);
die "Missing host_protocol_list\n" if !$ev1;
die "host_protocol_list not closed!\n" if !$ev2;
my $rc = system("diff -q ${fn}.new ${fn}");
if($rc == 256) {
		system("mv ${fn}.new ${fn}");
} else {
		unlink "${fn}.new";
}
}

sub print_proto {
	my ($f,$r) = @_;

	return if defined $r->{dumped};
	print $f "$r->{n}:\n";
	if($#{$r->{s}} >= 0) {
		print	$f "\tsource:\n\t  - ",join("\n\t  - ",@{$r->{s}}),"\n";
	}
	print	$f "\tip:\n\t  - ",join("\n\t  - ",@{$r->{ip}}),"\n";
	$r->{dumped} = 1;
}

sub add_net {
	my ($cmmnt,$str) = @_;
	$str =~ s,/\*.*\*/,,;
	$str =~ s,\s+,,g;
	$str =~ s,^\{(.*)\}.*$,$1,;
#	print "# $str\n";
	my @s = split /,/,$str;
	return 1 if $str eq '0x0,0,0';
	die "Bad string $str" if $#s != 2 || $s[0] !~ /^0x/i;
	die "Bad proto $s[2]" if $s[2] !~ /NDPI_(PROTOCOL|CONTENT|SERVICE)_(.*)$/;
	$s[2] = $2;
	$s[0] =~ s/0x//i;
	if(length($s[0]) > 8) {
		$s[0] = substr($s[0],-8);
	}
	my $ip = join('.',unpack('C4',pack('H8',$s[0])));

	if( !defined $P{$s[2]} ) {
		$P{$s[2]} = {s=>[],ip=>[],n=>$s[2]} if !defined $P{$s[2]};
#		print "# start $s[2]\n";
	}
	my $r = $P{$s[2]};
	if(${$cmmnt}) {
#		print "#Add comment ${$cmmnt}\n";
		push @{$r->{s}}, split /\n/,${$cmmnt};
		${$cmmnt} = '';
	}
	push @{$r->{ip}}, "$ip/$s[1]";
	return 0;
}


#
# vim: set ts=4:
#
