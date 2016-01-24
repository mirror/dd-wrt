#! /usr/bin/perl

my $col     = shift;
my $col_num;

   if ($col eq "speed")     { $col_num = 2; }
elsif ($col eq "mem")       { $col_num = 3; }
elsif ($col eq "unusedmem") { $col_num = 4; }
else { die "Format: $0 speed|mem|unusedmem [filenames...]\n"; }

$col = ucfirst(lc($col));

print <<EOL;

# set data style dots
# set xrange [0:500]
# set yrange [0:0.004]
# replot

set xlabel "Length"
set ylabel "$col"

EOL

my @plots = ();
for (@ARGV)
{
	push(@plots, '"' . $_ . '" using 1:' . $col_num);
}

print "plot " . (join  ", ", @plots) . "\n";

