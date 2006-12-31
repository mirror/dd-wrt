#!/usr/bin/perl
#
# Find include files.
#
# usage: incl.pl ... [-nostdinc] ... [-I dir] ... file ...
#
# -Idir and -I dir are equivalent, argument order doesn't matter, but
# file may be mis-detected if other options follow.
#
@STD = ("/usr/include","/usr/local/include");
while (@ARGV) {
    $arg = shift @ARGV;
    if ($arg eq "-nostdinc") {
	undef @STD;
    }
    if ($arg =~ /-I/) {
	if ($' ne "") { push(@USR,$'); }
	else { push(@USR,shift @ARGV); }
	next;
    }
    next if $arg =~ /^-/;
    next if $arg =~ /\.h$/ && defined $last;
    $last = $arg;
}
die "no include file specified" unless defined $last;
for (@STD,@USR) {
    next unless defined stat $_."/".$last;
    print $_."/".$last."\n" || die "print STDOUT: $!";
    exit 0;
}
die "$last not found";
