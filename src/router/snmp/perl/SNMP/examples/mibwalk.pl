# snmpwalk of entire MIB
# stop on error at end of MIB

use SNMP 1.8;
$SNMP::use_sprint_value = 1;
my $host = shift || localhost;
my $comm = shift || public;

$sess = new SNMP::Session(DestHost => $host, Community => $comm);

$var = new SNMP::Varbind([]);

do {
  $val = $sess->getnext($var);
  print "$var->[$SNMP::Varbind::tag_f].$var->[$SNMP::Varbind::iid_f] = ",
        "$var->[$SNMP::Varbind::val_f]\n";
} until ($sess->{ErrorStr});
