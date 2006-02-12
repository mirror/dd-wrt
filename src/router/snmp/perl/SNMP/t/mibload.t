#!./perl

BEGIN {
    unless(grep /blib/, @INC) {
        chdir 't' if -d 't';
        @INC = '../lib' if -d '../lib';
    }
}

use Test;
BEGIN {plan tests => 7}
use SNMP;

require "t/startagent.pl";

use vars qw($mibdir);

$SNMP::verbose = 0;

my $mib_file = 't/mib.txt';
my $junk_mib_file = 'mib.txt';
my $mibfile1 = "$mibdir/TCP-MIB.txt";
my @mibdir = ("$mibdir");
my $mibfile2 = "$mibdir/IPV6-TCP-MIB.txt";

######################################################################
# See if we can find a mib to use, return of 0 means the file wasn't
# found or isn't readable.

$res = SNMP::setMib($junk_mib_file,1);
ok(defined(!$res));
######################################################################
# Now we give the right name

$res = SNMP::setMib($mib_file,1);
ok(defined($res));
######################################################################
# See if we can find a mib to use

$res = SNMP::setMib($mib_file,0);
ok(defined($res));
######################## 4 ################################
# add a mib dir

$res = SNMP::addMibDirs($mibdir[0]);

SNMP::loadModules("IP-MIB", "IF-MIB", "IANAifType-MIB", "RFC1213-MIB");
#SNMP::unloadModules(RMON-MIB);
#etherStatsDataSource shouldn't be found.
#present only in 1271 & RMON-MIB.
$res = $SNMP::MIB{etherStatsDataSource};

ok(!defined($res));

########################  5  ############################
# add mib file

$res1 = SNMP::addMibFiles($mibfile1);
ok(defined($res1));
$res2 = SNMP::addMibFiles($mibfile2);
ok(defined($res2));

$res = $SNMP::MIB{ipv6TcpConnState}{moduleID};

ok($res =~ /^IPV6-TCP-MIB/);
#################################################


