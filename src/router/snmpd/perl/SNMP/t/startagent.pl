# common parameters used in SNMP::Session creation and tests
$agent_host = 'localhost';
$agent_port = 8765;
$trap_port = 8764;
$mibdir = '/usr/local/share/snmp/mibs';
$comm = 'v1_private';
$comm2 = 'v2c_private';
$comm3 = 'v3_private';
$sec_name = 'v3_user';
$oid = '.1.3.6.1.2.1.1.1';
$name = 'sysDescr';
$name_module = 'RFC1213-MIB::sysDescr';
$name_module2 = 'SNMPv2-MIB::sysDescr';
$name_long = '.iso.org.dod.internet.mgmt.mib-2.system.sysDescr';
$name_module_long = 'RFC1213-MIB::.iso.org.dod.internet.mgmt.mib-2.system.sysDescr';
$name_module_long2 = 'SNMPv2-MIB::.iso.org.dod.internet.mgmt.mib-2.system.sysDescr';
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

if ($^O =~ /win32/i) {
  require Win32::Process;
}

sub snmptest_cleanup {
    sleep 1; # strangely we need to wait for pid files to appear ??
    if ((-e "t/snmpd.pid") && (-r "t/snmpd.pid")) {
        #warn "\nStopping agents for test script $0\n";
        # Making sure that any running agents are killed.
	# warn "killing snmpd:", `cat t/snmpd.pid`, "\n";
        if ($^O !~ /win32/i) {
          system "kill `cat t/snmpd.pid` > /dev/null 2>&1";
        }
        else {
          open(H, "<t/snmpd.pid");
          my $pid = (<H>);
          close (H);
          if ($pid > 0) {
            Win32::Process::KillProcess($pid, 0)
          }
        }
	unlink "t/snmpd.pid";
    }
    if ((-e "t/snmptrapd.pid") && (-r "t/snmptrapd.pid")) {
        # Making sure that any running agents are killed.
	# warn "killing snmptrap:", `cat t/snmptrapd.pid`, "\n";
        if ($^O !~ /win32/i) {
          system "kill `cat t/snmptrapd.pid` > /dev/null 2>&1";
        }
        else {
          open(H, "<t/snmptrapd.pid");
          my $pid = (<H>);
          close (H);
          if ($pid > 0) {
            Win32::Process::KillProcess($pid, 0)
          }
        }
          
	unlink "t/snmptrapd.pid";
    }
}
snmptest_cleanup();

#Open the snmptest.cmd file and get the info
if (open(CMD, "<t/snmptest.cmd")) {
  while ($line = <CMD>) {
    if ($line =~ /HOST\s*=>\s*(.*?)\s+$/) {
      $agent_host = $1;
    } elsif ($line =~ /MIBDIR\s*=>\s*(.*?)\s+$/) {
      $mibdir = $1;
    } elsif ($line =~ /AGENT_PORT\s*=>\s*(.*?)\s+$/) {
      $agent_port = $1;
    } elsif ($line =~ /SNMPD\s*=>\s*(.*?)\s+$/) {
      $snmpd_cmd = $1;
    } elsif ($line =~ /SNMPTRAPD\s*=>\s*(.*?)\s+$/) {
      $snmptrapd_cmd = $1;
    }
  } # end of while
  close CMD;
} else {
  die ("Could not start agent. Couldn't find snmptest.cmd file\n");
}

#warn "\nStarting agents for test script $0\n";

if ($^O !~ /win32/i) {
  # Unix
  if ($snmpd_cmd) {
    if (-r $snmpd_cmd and -x $snmpd_cmd) {
      $basedir = `pwd`;
      chomp $basedir;
      system "$snmpd_cmd -r -Lf t/snmptest.log -M+$mibdir -C -c $basedir/t/snmptest.conf -p $basedir/t/snmpd.pid ${agent_host}:${agent_port} > /dev/null 2>&1";
    } else {
      warn("Couldn't run snmpd\n");
    }
  }
  if ($snmptrapd_cmd) {
    if (-r $snmptrapd_cmd and -x $snmptrapd_cmd) {
      system "$snmptrapd_cmd -p t/snmptrapd.pid -M+$mibdir -C -c t/snmptest.conf -C ${agent_host}:${trap_port} > /dev/null 2>&1";
    } else {
      warn("Couldn't run snmptrapd\n");
    }
  }
}
else {
  # Windows
  if ($snmpd_cmd) {
      if (-r $snmpd_cmd) {
        #print STDERR "start \"SNMPD\" /min \"$snmpd_cmd\" -r -Lf t/snmptest.log -C -c t/snmptest.conf -p t/snmpd.pid $agent_port > nul\n";
      system "start \"SNMPD\" /min \"$snmpd_cmd\" -r -Lf t/snmptest.log -C -c t/snmptest.conf -p t/snmpd.pid $agent_port > nul";
    } else {
      warn("Couldn't run snmpd\n");
    }
  }
  if ($snmptrapd_cmd) {
    if (-r $snmptrapd_cmd) {
      #print STDERR "start /min \"SNMPTRAPD\" \"$snmptrapd_cmd\" -Lf t/snmptrapdtest.log -p t/snmptrapd.pid -C -c t/snmptest.conf $trap_port > nul\n";
      system "start /min \"SNMPTRAPD\" \"$snmptrapd_cmd\" -Lf t/snmptrapdtest.log -p t/snmptrapd.pid -C -c t/snmptest.conf $trap_port > nul";
    } else {
      warn("Couldn't run snmptrapd\n");
    }
  }
  sleep 2; # Give programs time to start

} 

1;

