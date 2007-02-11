#!/usr/bin/perl
$place = ".1.3.6.1.4.1.2021.255";
$req = $ARGV[1];

if ($ARGV[0] eq "-s") {
  open LOG,">>/tmp/passtest.log";
  print LOG "@ARGV\n";
  close LOG;
  exit 0;
}

if ($ARGV[0] eq "-n") {
  if ($req eq "$place") { $ret = "$place.1";}
  elsif ($req eq "$place.1") { $ret = "$place.2.1";}
  elsif ($req eq "$place.2.1") { $ret = "$place.2.2";}
  elsif ($req eq "$place.2.2") { $ret = "$place.3";}
  elsif ($req eq "$place.3") { $ret = "$place.4";}
  elsif ($req eq "$place.4") { $ret = "$place.5";}
  elsif ($req eq "$place.5") { $ret = "$place.6";}
  else {exit 0;}
}
else {
  if ($req eq "$place") { exit 0;}
  else {$ret = $req;}
}

print "$ret\n";

if ($ret eq "$place.1") { print "string\nlife the universe and everything\n"; exit 0;}
elsif ($ret eq "$place.2.1") { print "integer\n42\n"; exit 0;}
elsif ($ret eq "$place.2.2") { print "objectid\n.1.3.6.1.4.42.42.42\n"; exit 0;}
elsif ($ret eq "$place.3") { print "timeticks\n363136200\n"; exit 0;}
elsif ($ret eq "$place.4") { print "ipaddress\n127.0.0.1\n"; exit 0;}
elsif ($ret eq "$place.5") { print "counter\n42\n"; exit 0;}
elsif ($ret eq "$place.6") { print "gauge\n42\n"; exit 0;}
else { print "string\nack... $ret $req\n"; exit 0; }
