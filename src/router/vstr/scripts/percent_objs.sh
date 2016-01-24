#! /bin/sh

ls -l src/*.o | \
 awk '{print $5 " " $9}' | \
 sort -n | \
 perl -e '
 $/ = undef;
 my $all = <>;
 my @a = (); 
 for (split "\n", $all) { if (/(\d+) src\/(\w*\.o)/) { push @a, $2, $1; } }
 my $num = 0;
 my $c = 0;

 for (@a)
 {
   if ($c) { $num += $_; }
   $c ^= 1;
 }

 $c = 0;
 for (@a)
 {
   print $_ . "\t";
   if ($c) { print sprintf("%.2f%%\n", ($_ * 100) / $num); }
   $c ^= 1;
 }
'
