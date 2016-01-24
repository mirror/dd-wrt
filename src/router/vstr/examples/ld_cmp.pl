#! /usr/bin/perl -w


use strict;


if (scalar (@ARGV) < 3)
  {
    die " Format: $0 <build-dir1> <build-dir2> <program> [options...]\n";
  }

# beg: conf
my $num_cmp = 4;
my $ld_tmpfile = "ld_cmp_tmp.$$";
# end: conf



sub gen_stats
  {
    my $bdir = shift;

    if (!open(STATS, "> $ld_tmpfile"))
      { die "open($ld_tmpfile): $!\n"; }
    if (!open(OLD_STDERR, ">&STDERR"))
      { die "dup2(OLD, STDERR): $!\n"; }
    if (!open(STDERR, ">&STATS"))
      { die "dup2(STDERR, STATS): $!\n"; }

    $ENV{LD_LIBRARY_PATH} = "../$bdir/src/.libs/";
    for (1..$num_cmp)
      {	system("./ld_stats.sh", @_); }

    if (!open(STDERR, ">&OLD_STDERR"))
      { die "dup2(STATS): $!\n"; }

    close(OLD_STDERR);
    close(STATS);

    if (!open(STATS, "< $ld_tmpfile"))
      { die "open($ld_tmpfile): $!\n"; }
  }

sub proc_stats
  {
    my $bstats = shift;

    $bstats->{startup} = [];
    $bstats->{time_reloc_clock} = [];
    $bstats->{time_reloc_per} = [];
    $bstats->{num_reloc} = [];
    $bstats->{num_cache_reloc} = [];
    $bstats->{time_objs_clock} = [];
    $bstats->{time_objs_per} = [];
    $bstats->{final_num_reloc} = [];
    $bstats->{final_num_cache_reloc} = [];

    while (<STATS>)
      {
	if (/\d+: \s+ $/x) { next; }
	if (/\d+: \s+ runtime \s linker \s statistics:/x) { next; }
	if (/\d+: \s+ total \s startup \s time \s in \s dynamic \s loader: \s
	    (\d+) \s clock \s cycles$/x)
	  {
	    push @{$bstats->{startup}}, $1;
	    next;
	  }
	if (/\d+: \s+ time \s needed \s for \s relocation: \s
	    (\d+) \s clock \s cycles \s \((\d+).(\d+)\%\)/x)
	  {
	    push @{$bstats->{time_reloc_clock}}, $1;
	    push @{$bstats->{time_reloc_per}}, "$2.$3";
	    next;
	  }
	if (/\d+: \s+ number \s of \s relocations: \s
	    (\d+)$/x)
	  {
	    push @{$bstats->{num_reloc}}, $1;
	    next;
	  }
	if (/\d+: \s+ number \s of \s relocations \s from \s cache: \s
	    (\d+)$/x)
	  {
	    push @{$bstats->{num_cache_reloc}}, $1;
	    next;
	  }
	if (/\d+: \s+ time \s needed \s to \s load \s objects: \s
	    (\d+) \s clock \s cycles \s \((\d+).(\d+)\%\)/x)
	  {
	    push @{$bstats->{time_objs_clock}}, $1;
	    push @{$bstats->{time_objs_per}}, "$2.$3";
	    next;
	  }
	if (/\d+: \s+ final \s number \s of \s relocations: \s
	    (\d+)$/x)
	  {
	    push @{$bstats->{final_num_reloc}}, $1;
	    next;
	  }
	if (/\d+: \s+ final \s number \s of \s relocations \s from \s cache: \s
	    (\d+)$/x)
	  {
	    push @{$bstats->{final_num_cache_reloc}}, $1;
	    next;
	  }
	die "No match: $_\n";
      }
  }

my %b1_stats = ();
my %b2_stats = ();
my @bl = [];

$bl[0] = shift @ARGV;
$bl[1] = shift @ARGV;

gen_stats($bl[0], @ARGV);
proc_stats(\%b1_stats);

gen_stats($bl[1], @ARGV);
proc_stats(\%b2_stats);

for my $i (keys %b1_stats)
  {
    print "$i\n";

    for my $j (1..$num_cmp)
      {
	my $off = $j - 1;
	my @vals;

	$vals[0] = ${$b1_stats{$i}}[$off];
	$vals[1] = ${$b2_stats{$i}}[$off];

        print sprintf("%4d.  ", $j);
	print "$bl[0]($vals[0]) - $bl[1]($vals[1]) = ";
	print $vals[0] - $vals[1];
	print "\n";
      }
  }

unlink($ld_tmpfile);
