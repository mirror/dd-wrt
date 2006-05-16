# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

######################### We start with some black magic to print on failure.

# Change 1..1 below to 1..last_test_to_print .
# (It may become useful if the test is moved to ./t subdirectory.)

BEGIN { $| = 1; 

	%tests = ( 
                  "NETSNMP_DS_AGENT_VERBOSE"               => 0,
                  "NETSNMP_DS_AGENT_ROLE"                  => 1,
                  "NETSNMP_DS_AGENT_NO_ROOT_ACCESS"        => 2,
                  "NETSNMP_DS_AGENT_AGENTX_MASTER"         => 3,
                  "NETSNMP_DS_AGENT_QUIT_IMMEDIATELY"      => 4,
                  "NETSNMP_DS_AGENT_DISABLE_PERL"          => 5,
                  "NETSNMP_DS_AGENT_PROGNAME"              => 0,
                  "NETSNMP_DS_AGENT_X_SOCKET"              => 1,
                  "NETSNMP_DS_AGENT_PORTS"                 => 2,
                  "NETSNMP_DS_AGENT_INTERNAL_SECNAME"      => 3,
                  "NETSNMP_DS_AGENT_PERL_INIT_FILE"        => 4,
                  "NETSNMP_DS_AGENT_FLAGS"                 => 0,
                  "NETSNMP_DS_AGENT_USERID"                => 1,
                  "NETSNMP_DS_AGENT_GROUPID"               => 2,
                  "NETSNMP_DS_AGENT_AGENTX_PING_INTERVAL"  => 3,
		  );

	print "1.." . (scalar(keys(%tests)) + 2) . "\n"; 
    }
END {print "not ok 1\n" unless $loaded;}
use NetSNMP::agent::default_store (':all');
$loaded = 1;
print "ok 1\n";

######################### End of black magic.

# Insert your test code below (better if it prints "ok 13"
# (correspondingly "not ok 13") depending on the success of chunk 13
# of the test code):

$c = 2;
foreach my $i (keys(%tests)) {
    my $str = "NetSNMP::agent::default_store::$i";
    my $val = eval $str;
#    print "$i -> $val -> $tests{$i}\n";
    $c++;
    print (($val eq $tests{$i})?"ok $c\n" : "not ok $c\n#  error:  name=$i value_expected=$tests{$i}  value_got=$val \n");
}
