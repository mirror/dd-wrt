#!/usr/bin/perl
#
# Usage:
#
# hosts2ans.pl [-r] [ domain [ host [ email ] ] ] </etc/hosts.atm >zonefile
#
# Where  domain    is the name of the domain to create, e.g. lrc.epfl.ch (if
#		   omitted,  hostname -d  is used)
#	 host	   is the name of the primary DNS server of that domain, e.g.
#		   lrcpcs.epfl.ch (if omitted,  hostname -f  is used)
#	 email	   is the e-mail address of the DNS administrator of that
#		   domain, e.g. root@lrc.epfl.ch (if omitted, root@host is used)
#	 zonefile  is the name of the output file, e.g. lrc.zone
#
# Trailing dots in domain, host, and email are silently removed.
#
# With -r, the reverse mapping (PTR) is created. Otherwise, the forward mapping
# (ATMA) is created.
#
# Example: host2ans.pl lrc.epfl.ch </etc/hosts.atm >/etc/named/lrc.zone

if ($ARGV[0] eq "-r") {
    shift(@ARGV);
    $rev = 1;
}
$master = $tmp = `hostname -f` unless defined($master = $ARGV[1]);
$master =~ s/\n//;
$master =~ s/\.$//;
($domain = $master) =~ s/^[^.]*\.// unless defined($domain = $ARGV[0]);
$domain =~ s/\.$//;
$email = "root\@$master" unless defined($email = $ARGV[2]);
$email =~ s/@/\./;
$email =~ s/\.$//;
print "; ".($rev ? "Reverse mapping" : "Authoritative data")." for $domain".
  "\n\n";
print "@\t\tIN\tSOA\t$master. $email. (\n";
@t = localtime(time);
$t[5] += 1900 if $t[5] < 100; # Perl bug ?
printf("\t\t\t\t%04d%02d%02d%02d\t; Serial\n",$t[5],$t[4],$t[3],$t[2]);
print "\t\t\t\t10800\t\t; Refresh (3h)\n";
print "\t\t\t\t3600\t\t; Retry (1h)\n";
print "\t\t\t\t3600000\t\t; Expire (1000h)\n";
print "\t\t\t\t86400 )\t\t; Minimum (24h)\n";
print "\t\tIN\tNS\t$master.\n";
print "localhost\tIN\tA\t127.0.0.1\n" unless $rev;
while (<STDIN>) {
    chop;
    s/#.*//;
    s/\s+$//;
    s/^\s+//;
    next if /^$/;
    ($addr,$host) = split(/\s+/);
    $addr =~ s/\.//g;
    $host =~ s/\..*//;
    $host =~ s/\.$//;
    if ($rev) {
	$pfx = substr($addr,0,26);
	$tail = substr($addr,26,14);
	if ($pfx ne $origin) {
	    $origin = $pfx;
	    $single = substr($pfx,6,20);
	    print "\$ORIGIN ".join(".",reverse split("",substr($pfx,6,20))).
	      ".".substr($pfx,2,4).".".substr($pfx,0,2).".AESA.ATMA.INT.\n";
	}
	print substr($tail,12,2).".".substr($tail,0,12)."\tIN\tPTR\t$host.".
	  $domain.".\n";
    }
    else {
	print $host.(length($host) < 8 ? "\t" : "")."\tIN\tATMA\t$addr\n";
    }
}
