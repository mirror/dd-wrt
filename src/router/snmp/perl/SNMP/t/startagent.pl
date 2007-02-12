# common parameters used in SNMP::Session creation and tests
$agent_host = 'localhost';
$agent_port = 7000;
$trap_port = 8000;
$mibdir = '/usr/local/share/snmp/mibs';
$comm = 'v1_private';
$comm2 = 'v2c_private';
$comm3 = 'v3_private';
$sec_name = 'v3_user';
$oid = '.1.3.6.1.2.1.1.1';
$name = 'sysDescr';
$auth_pass = 'test_pass_auth';
$priv_pass = 'test_pass_priv';

# don't use any .conf files other than those specified.
$ENV{'SNMPCONFPATH'} |= "bogus";

# erroneous input to test failure cases
$bad_comm = 'BAD_COMMUNITY';
$bad_name = "badName";
$bad_oid = ".1.3.6.1.2.1.1.1.1.1.1";
$bad_host = 'bad.host.here';
$bad_port = '9999999';
$bad_auth_pass = 'bad_auth_pass';
$bad_priv_pass = 'bad_priv_pass';
$bad_sec_name = 'bad_sec_name';
$bad_version = 7;

local $snmpd_cmd;
local $snmptrapd_cmd;

my $line;

sub snmptest_cleanup {
    sleep 1; # strangely we need to wait for pid files to appear ??
    if ((-e "t/snmpd.pid") && (-r "t/snmpd.pid")) {
        # Making sure that any running agents are killed.
	# warn "killing snmpd:", `cat t/snmpd.pid`, "\n";
	system "kill `cat t/snmpd.pid` > /dev/null 2>&1";
	unlink "t/snmpd.pid";
    }
    if ((-e "t/snmptrapd.pid") && (-r "t/snmptrapd.pid")) {
        # Making sure that any running agents are killed.
	# warn "killing snmptrap:", `cat t/snmptrapd.pid`, "\n";
	system "kill `cat t/snmptrapd.pid` > /dev/null 2>&1";
	unlink "t/snmptrapd.pid";
    }
}
snmptest_cleanup();
#Open the snmptest.cmd file and get the info
if (open(CMD, "<t/snmptest.cmd")) {
  while ($line = <CMD>) {
    if ($line =~ /HOST\s*=>\s*(\S+)/) {
      $agent_host = $1;
    } elsif ($line =~ /MIBDIR\s*=>\s*(\S+)/) {
      $mibdir = $1;
    } elsif ($line =~ /AGENT_PORT\s*=>\s*(\S+)/) {
      $agent_port = $1;
    } elsif ($line =~ /SNMPD\s*=>\s*(\S+)/) {
      $snmpd_cmd = $1;
    } elsif ($line =~ /SNMPTRAPD\s*=>\s*(\S+)/) {
      $snmptrapd_cmd = $1;
    }
  } # end of while
  close CMD;
} else {
  die ("Could not start agent. Couldn't find snmptest.cmd file\n");
}

if ($^O !~ /win32/i) {
  if ($snmpd_cmd) {
    if (-r $snmpd_cmd and -x $snmpd_cmd) {
#      print STDERR "running: $snmpd_cmd -r -l t/snmptest.log -C -c t/snmptest.conf -P t/snmpd.pid $agent_port > /dev/null 2>&1\n";
      system "$snmpd_cmd -r -l t/snmptest.log -C -c t/snmptest.conf -P t/snmpd.pid $agent_port > /dev/null 2>&1";
    } else {
      warn("Couldn't run snmpd\n");
    }
  }
  if ($snmptrapd_cmd) {
    if (-r $snmptrapd_cmd and -x $snmptrapd_cmd) {
      system "$snmptrapd_cmd -u t/snmptrapd.pid -c t/snmptest.conf -C $trap_port > /dev/null 2>&1";
    } else {
      warn("Couldn't run snmptrapd\n");
    }
  }
}

1;

