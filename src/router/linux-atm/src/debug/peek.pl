#!/usr/bin/perl
$base = 0x220000;
$off = $ARGV[0];
$dsc = $base+hex($off)*4;
$start = ($dsc & ~15)-64;
$| = 1;
printf("Descriptor is at 0x%06x\n",$dsc);
system("hexdump -e'\"%06.6_ax \" 16/1 \"%02x \" \"\\n\"' -s $start /var/tmp/all");
