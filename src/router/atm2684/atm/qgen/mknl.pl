#!/usr/bin/perl
print "# THIS IS A MACHINE-GENERATED FILE. DO NOT EDIT !\n" || die "write: $!";
while ($line = <STDIN>) {
    chop($line);
    next unless $line =~ /^#define\s/;
    while ($line =~ m|/\*| && $line !~ m|\*/|) { $line .= <STDIN>; }
    $line =~ s/\s+/ /g;
    if (!defined($curr) || $line !~ /^#define ${curr}_/) {
	undef $curr;
	for (@ARGV) {
	    ($tmp = $_) =~ tr/a-z/A-Z/;
	    next unless $line =~ /^#define ${tmp}_/;
	    $curr = $tmp;
	    print "\n:$_\n" || die "write: $!";
	    last;
	}
    }
    next unless defined $curr;
    next unless $line =~ m|^#define (\S+) (\S+)( (/\*\s*(.*\S)\s*\*/))?|;
#    if (defined $3) {
#	print "$2=$1 $4\n" || die "write: $!";
#    }
    if (defined $3) {
	print "$2=$2 \\\"$5\\\"\n" || die "write: $!";
    }
    else {
	print "$2=$1\n" || die "write: $!";
    }
}
