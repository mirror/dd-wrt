#! /usr/bin/perl -w

use strict;
use FileHandle;

# Do a fast check of the main options ?
my $conf_fast_check = 0;

# Do a check of the linker script ?
my $conf_linker_check = 0;

# Do a full check, or group all disable options together
my $conf_full_dbl_check = 0;
my $conf_full_test_check = 0;
my $conf_full_wrap_check = 0;

my $conf_append_output = 0;
my $conf_print_stdout_info = 1;
# Skip valgrind run ... as it's broken on glibc-2.3 atm. with debugging on.
my $conf_valgrind = 0;
my $conf_time = 0;
my $conf_debug = 1;
my $conf_test = 1;
my $conf_wrap = 1;

# Call configurations randomly ?
my $conf_rand = 0;

while (scalar (@ARGV))
{
  my $arg = shift @ARGV;

  if (0) {}

  elsif ("fast" eq $arg)
  { $conf_fast_check = 1; }
  elsif ("nofast" eq $arg)
  { $conf_fast_check = 0; }

  elsif ("linker" eq $arg)
  { $conf_linker_check = 1; }
  elsif ("nolinker" eq $arg)
  { $conf_linker_check = 0; }

  elsif ("dblfull" eq $arg)
  { $conf_full_dbl_check = 1; }
  elsif ("nodblfull" eq $arg)
  { $conf_full_dbl_check = 0; }
  elsif ("tstfull" eq $arg)
  { $conf_full_test_check = 1; }
  elsif ("notstfull" eq $arg)
  { $conf_full_test_check = 0; }
  elsif ("wrapfull" eq $arg)
  { $conf_full_wrap_check = 1; }
  elsif ("nowrapfull" eq $arg)
  { $conf_full_wrap_check = 0; }

  elsif ("append" eq $arg)
  { $conf_append_output = 1; }
  elsif ("noappend" eq $arg)
  { $conf_append_output = 0; }

  elsif ("verbose" eq $arg)
  { $conf_print_stdout_info = 1; }
  elsif ("noverbose" eq $arg)
  { $conf_print_stdout_info = 0; }

  elsif ("valgrind" eq $arg)
  { $conf_valgrind = 1; }
  elsif ("novalgrind" eq $arg)
  { $conf_valgrind = 0; }

  elsif ("time" eq $arg)
  { $conf_time = 1; }
  elsif ("notime" eq $arg)
  { $conf_time = 0; }

  elsif ("test" eq $arg)
  { $conf_test = 1; }
  elsif ("notest" eq $arg)
  { $conf_test = 0; }
  elsif ("tst" eq $arg)
  { $conf_test = 1; }
  elsif ("notst" eq $arg)
  { $conf_test = 0; }

  elsif ("wrap" eq $arg)
  { $conf_wrap = 1; }
  elsif ("nowrap" eq $arg)
  { $conf_wrap = 0; }

  elsif ("debug" eq $arg)
  { $conf_debug = 1; }
  elsif ("nodebug" eq $arg)
  { $conf_debug = 0; }

  elsif ("RAND" eq $arg)
  { $conf_rand = 1; }

  elsif ("ALL" eq $arg)
  {
  	{ $conf_linker_check = 1; }
  	{ $conf_full_dbl_check = 1; }
  	{ $conf_full_test_check = 1; }
  	{ $conf_full_wrap_check = 1; }
  	{ $conf_test = 1; }
  	{ $conf_wrap = 1; }
  	{ $conf_debug = 1; }
  }

  else
  { die "Unknown option $arg\n"; }
}


my @C_ls =     ("--enable-linker-script");
my @C_dbg =    ("--enable-debug");
my @C_np =     ("--enable-noposix-host");

my @C_nin =    ("--enable-tst-noinline");
my @C_natals = ("--enable-tst-noattr-alias");
my @C_natvis = ("--enable-tst-noattr-visibility");

my @C_dbl_g =  ("--with-fmt-float=glibc");
my @C_dbl_h =  ("--with-fmt-float=host");
my @C_dbl_n =  ("--with-fmt-float=none");

my @C_wr_cpy = ("--enable-wrap-memcpy");
my @C_wr_cmp = ("--enable-wrap-memcmp");
my @C_wr_chr = ("--enable-wrap-memchr");
my @C_wr_rchr= ("--enable-wrap-memrchr");
my @C_wr_set = ("--enable-wrap-memset");
my @C_wr_move= ("--enable-wrap-memmove");


$SIG{INT} = sub { exit (1); };

my %remap = ();

sub print_cc_cflags()
  {
    my $cc = "gcc";       if (defined ($ENV{CC}))     { $cc = $ENV{CC}; }
    my $cf = "<default>"; if (defined ($ENV{CFLAGS})) { $cf = $ENV{CFLAGS}; }

    FOUT->print("CC=\"$cc\"\n");
    FOUT->print("CFLAGS=\"$cf\"\n");
    FERR->print("CC=\"$cc\"\n");
    FERR->print("CFLAGS=\"$cf\"\n");
    if ($conf_print_stdout_info)
    {
      STDOUT->print("CC=\"$cc\"\n");
      STDOUT->print("CFLAGS=\"$cf\"\n");
    }
  }

sub conf
{
  my $p_args = join " ", @_;

  $p_args =~ s/--enable-/-e-/g;
  $p_args =~ s/--with-/-w-/g;

  FOUT->print("\n");
  FOUT->print("\n");
  FOUT->print("==== BEG: $p_args ====\n");
  FOUT->print("\n");
  if ($conf_print_stdout_info)
  {
    STDOUT->print("BEG: $p_args\n");
  }

  FERR->print("\n");
  FERR->print("==== BEG: $p_args ====\n");

  my $c = undef;
  my $v = undef;

  if (0) {}
  elsif (-x "./configure")
    {
      $c =  "./configure";
      $v =  "./scripts/valgrind.sh";
    }
  elsif (-x "../configure")
    {
      $c =  "../configure";
      $v =  "../scripts/valgrind.sh";
    }

  if (!defined ($c))
    {
      STDERR->print("Can't find configure.\n");
      exit (1);
    }

  if (!open(OLD_STDOUT, ">&STDOUT"))
    { die "dup2(OLD, STDOUT): $!\n"; }
  if (!open(OLD_STDERR, ">&STDERR"))
    { die "dup2(OLD, STDERR): $!\n"; }

  if (!open(STDOUT, ">&FOUT"))
    { die "dup2(STDOUT, FOUT): $!\n"; }
  if (!open(STDERR, ">&FERR"))
    { die "dup2(STDERR, FERR): $!\n"; }

  my $ok = 0;

  if (!system($c, @_, "--enable-tst-noassert-loop") &&
      !system("make", "clean") &&
      !system("make", "check") &&
      (!$conf_time || !system("time", "make", "check")))
    {
      # Fear the power of sh...
      if (!open(STDOUT, ">&FERR"))
	{ die "dup2(STDOUT, FERR): $!\n"; }

      if (!$conf_valgrind || system("$v | egrep -C 2 '^=='"))
	{
	  $ok = 1;
	}
    }

  if (!$ok)
    {
      if ($conf_print_stdout_info)
        {
          OLD_STDOUT->print("*" x 35 . " BAD " . "*" x 35 . "\n");
        }
      FOUT->print("*" x 35 . " BAD " . "*" x 35 . "\n");
      FERR->print("*" x 35 . " BAD " . "*" x 35 . "\n");
      sleep(4);
    }

  FOUT->print("==== END: $p_args ====\n");
  FERR->print("==== END: $p_args ====\n");

  if (!open(STDOUT, ">&OLD_STDOUT"))
    { die "dup2(FOUT): $!\n"; }
  if (!open(STDERR, ">&OLD_STDERR"))
    { die "dup2(FERR): $!\n"; }

  close(OLD_STDOUT);
  close(OLD_STDERR);
}

sub t_U
{ # This accounts for the remap hash...
  my @a = @_;

  for (@a)
    {
      if (defined ($remap{$_}))
	{ $_ = $remap{$_}; }
    }

  @a = sort @a;

  my $last = undef;
  for my $i (@a)
    {
      if (defined ($last) && ($last == $i))
	{ return (0); }
      $last = $i;
    }

  return (1);
}

sub tst_conf_X
  { # Pick all combs. of $len each
    our $conf_args = shift;
    our @a = ();
    our $conf_last = $#{$conf_args};

    sub tst_conf__X
      { # Pick all combs. of $len each
	my $len = shift;
	my $ret = shift;
	my $num = shift;
	local @a = @a;

	if (defined ($num))
	  {
	    @a = (@a, $num);

	    if (!t_U(@a))
	      { return; }

	    if ($len == 0)
	      {
		push (@$ret, \@a);
		return;
	      }
	  }
	else
	  {
	    $num = 0;
	  }

	for my $i ($num..$conf_last)
	  {
	    tst_conf__X($len - 1, $ret, $i);
	  }
      }

    my @confs = ();
    my $tlen = $conf_last + 1;
    while ($tlen)
      {
	tst_conf__X($tlen, \@confs);
	--$tlen;
      }

    if ($conf_rand)
      { # perlfaq4: How do I shuffle an array randomly?
         use List::Util 'shuffle';
         @confs = shuffle(@confs);
      }

    my $count = 0;
    for my $val (@confs)
      {
        my $per = ((0.0 + ++$count) / scalar(@confs) * 100.0);

        if ($conf_print_stdout_info)
        {
          STDOUT->print(sprintf("%.2f%% ", $per));
        }

	conf (map(@{$conf_args->[$_]}, @$val));
      }
  }

# -------------------------------------------------------------
# -------------------------------------------------------------

# Main

if ($conf_print_stdout_info)
{
  sub tf
    {
      my $val = shift;
      if ($val) { return ("true"); }

      return ("false");
    }

  my $header = " configuration ";

  my $xes_l = (79 - length($header)) / 2;
  my $xes_r = (79 - length($header)) - $xes_l;

  STDOUT->print("-" x $xes_l . $header . "-" x $xes_r . "\n");

  STDOUT->print(sprintf("%16s = %s\n", "fast",     tf($conf_fast_check)));
  STDOUT->print(sprintf("%16s = %s\n", "linker",   tf($conf_linker_check)));
  STDOUT->print(sprintf("%16s = %s\n", "debug",    tf($conf_debug)));

  STDOUT->print(sprintf("%16s = %s\n", "dblfull",  tf($conf_full_dbl_check)));
  STDOUT->print(sprintf("%16s = %s\n", "tstfull",  tf($conf_full_test_check)));
  STDOUT->print(sprintf("%16s = %s\n", "wrapfull", tf($conf_full_wrap_check)));

  STDOUT->print(sprintf("%16s = %s\n", "append",   tf($conf_append_output)));
  # verbose must be on...
  STDOUT->print(sprintf("%16s = %s\n", "valgrind", tf($conf_valgrind)));
  STDOUT->print(sprintf("%16s = %s\n", "time",     tf($conf_time)));
  STDOUT->print(sprintf("%16s = %s\n", "test",     tf($conf_test)));
  STDOUT->print(sprintf("%16s = %s\n", "wrap",     tf($conf_wrap)));
  STDOUT->print(sprintf("%-16s = %s\n", "RANDOMISE",tf($conf_rand)));

  STDOUT->print("-" x 79 . "\n");
  STDOUT->print(" NOTE: That ASSERT()'s are there for the xfail's in the autocheck2.log" . "\n");
  STDOUT->print("_" x 79 . "\n");
}



if ($conf_append_output)
  {
    if (!open(FOUT, ">> autocheck1.log"))
      { die "open(autocheck1.log): $!\n"; }

    if (!open(FERR, ">> autocheck2.log"))
      { die "open(autocheck2.log): $!\n"; }
  }
else
  {
    if (!open(FOUT, "> autocheck1.log"))
      { die "open(autocheck1.log): $!\n"; }

    if (!open(FERR, "> autocheck2.log"))
      { die "open(autocheck2.log): $!\n"; }
  }

print_cc_cflags();

my @confs = ();

if ($conf_debug)
  {
    push(@confs, \@C_dbg);
  }

if ($conf_linker_check)
  {
    push(@confs, \@C_ls);
  }

if ($conf_fast_check)
  {
    tst_conf_X(\@confs);
  }

if (!$conf_full_dbl_check)
  {
    push(@confs, \@C_dbl_g);
  }
else
  {
    $remap{$#confs + 2} = $#confs + 1;
    $remap{$#confs + 3} = $#confs + 1;
    push(@confs, \@C_dbl_g, \@C_dbl_h, \@C_dbl_n);
  }

if ($conf_test)
  { # Group for "turn off" flags...
    my @tmp_no = (\@C_np, \@C_nin, \@C_natals, \@C_natvis);

    if (!$conf_full_test_check)
      {
	push @confs, [ map { @$_ } @tmp_no ];
      }
    else
      {
	push @confs, @tmp_no;
      }
  }

if ($conf_wrap)
  { # Group for "wrap" flags...
    my @tmp_wr = (\@C_wr_cpy, \@C_wr_cmp, \@C_wr_chr, \@C_wr_rchr, \@C_wr_set,
		  \@C_wr_move);

    if (!$conf_full_wrap_check)
      {
	push @confs, [ map { @$_ } @tmp_wr ];
      }
    else
      {
	push @confs, @tmp_wr;
      }
  }

tst_conf_X(\@confs);

exit (0);
