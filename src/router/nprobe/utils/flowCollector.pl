#!/usr/bin/perl -w

#
# netFlow collector
#
# (C) 2002-04 - Deri Luca <deri@ntop.org>
#

# This application has been strongly inspired by:
# ftp://ftp.mcs.anl.gov/pub/systems/software/netflowdb-0.1.0.tar.gz
#

#use strict;
use IO::Socket;
use Getopt::Std;
use POSIX qw(strftime);
use vars (qw($opt_P $opt_p $opt_v));

my $num = $#ARGV;


my $debug = 1;
my $numFlow = 0;
my $time = 0;

#########################

if($num == 0) {
    help();
}

getopts('p:P:v');

my $udpPort  = $opt_p || 2055;
my $flowsDir = $opt_P || ".";
my $verbose  = $opt_v || 0;

if($flowsDir =~ /\/$/)    { chop($flowsDir); }
if(!($flowsDir =~ /^\//)) { $flowsDir = "./".$flowsDir; }

#########################

# Open a socket to receive Cisco netflow output packets
my $flows = IO::Socket::INET->new(Proto		=> "udp",
				  LocalAddr	=> "0.0.0.0",
				  LocalPort	=> "($udpPort)")
    || die "cannot open UDP socket listening @ port $udpPort";


print "Saving on '$flowsDir' flows received on port $udpPort\n";

while(1) {
    my ($buf, $len, $dest, $tmpTime );
    $flows->recv($buf, 2048, 0) || die "recv: $!";
    $len = length($buf);

    if($verbose) {        	 	 
	print "[".strftime("%e %b %Y %H:%M:%S", localtime(time))."] - Received $len bytes\n"; 
    }

    $tmpTime = time();

    $tmpTime = $tmpTime - ($tmpTime % 60);

    if($time != $tmpTime) {
	my $outFile = "$flowsDir/$time";

	if($time > 0) {
	    close(OUT);
	    rename("$outFile.flow.temp", "$outFile.flow");
	}

	$time = $tmpTime;
	$outFile = "$flowsDir/$time";

	if($verbose) { print "Saving into file $outFile.flow.temp\n"; }
	open(OUT, "> $outFile.flow.temp");
	binmode OUT,  ":raw";
    }

    printf OUT "%04d", $len;
    print OUT $buf;
}

##############

sub help {
    print "Perl-based flow collector\n\n";
    print "Usage: flowCollector.pl [-v] [-p <port>] [-P <dir>]\n";
    print "       -v         Verbose output\n";
    print "       -p <port>  It specifies the UDP port where flows are received\n";
    print "       -P <dir>   It specified the directory where flows will be stored\n";
    print "\n";
    exit 0;
}
