#!/usr/bin/perl

# usage:
#   hist.pl <sym_file_sorted> <pc_samples>
#

use IO::File;

$sym_file = new IO::File "@ARGV[0]", "r";
$pc_file  = new IO::File "@ARGV[1]", "r";
$|++; # flush after every char for update

sub hashValueDescendingNum {
   $results{$b} <=> $results{$a};
}

$i = 0;
while ($line = <$sym_file>) {
    ($sym_addr[$i], $sym_name[$i]) = split(/ /, $line); 
    $i++;
}

$total = $i;
for($i = 0; $i < $total; $i++) {
    $sym_hist[$i] = 0;
    $sym_name[$i] =~ s/\n//g;
}

print "Crunching:\n";

$total_pc = 0;
while ($line = <$pc_file>) {
    $total_pc++;
    for($i = 0; $i < $total; $i++) {
        if ((hex("$line") >= hex("0x" . "$sym_addr[$i]")) && (hex("$line") < hex("0x" . "$sym_addr[$i + 1]"))) {
            $sym_hist[$i]++;
            last;
        }
    }
    #print "$i\n";
    print "*";
}

print "\n\n";

for($i = 0; $i < $total; $i++) {
    if ($sym_hist[$i] > 0) {
        #print "$sym_name[$i]   $sym_hist[$i]\n";
        $results{$sym_name[$i]} = $sym_hist[$i];
    }
}

print "\tCalls \t %CPU \t Function\n";
print "\t-------------------------------------------\n";
foreach $key (sort hashValueDescendingNum (keys(%results))) {
    $val = ($results{$key}/$total_pc)*100;
    print "\t$results{$key} \t $val \t $key\n";
}
