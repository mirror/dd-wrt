use strict;
use SNMP;

my $target = shift || die "no ping target supplied\n"; # numeric ip address
my $host = shift || 'localhost';
my $community = shift || 'private';

{
        my $sess = new SNMP::Session (DestHost => $host,
				      Community => $community,
				      Retries => 1);

        my $dec = pack("C*",split /\./, $target);
        my $oid = ".1.3.6.1.4.1.9.9.16.1.1.1";
        my $row = "300";

        $sess->set([
                ["$oid.16", $row, 6, "INTEGER"],
                ["$oid.16", $row, 5, "INTEGER"],
                ["$oid.15", $row, "MoNDS", "OCTETSTR"],
                ["$oid.2", $row, 1, "INTEGER"],
                ["$oid.4", $row, 20, "INTEGER"],
                ["$oid.5", $row, 150, "INTEGER"],
                ["$oid.3", $row, $dec, "OCTETSTR"]]);

        $sess->set([["$oid.16", $row, 1, "INTEGER"]]);
        sleep 30;
        my ($sent, $received, $low, $avg, $high, $completed) = $sess->get([
                ["$oid.9", $row], ["$oid.10", $row], ["$oid.11", $row],
                ["$oid.12", $row], ["$oid.13", $row], ["$oid.14", $row]]);

        printf "Packet loss: %d% (%d/%d)\n", (100 * ($sent-$received)) / $sent,
                $received, $sent;
        print "Average delay $avg (low: $low high: $high)\n";
        $sess->set(["$oid.16", $row, 6, "INTEGER"]);
}
