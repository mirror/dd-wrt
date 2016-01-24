#! /usr/bin/perl -w

use strict;
use FileHandle;

my $inc = undef;

if (0) {}
elsif (-x "./configure")
  {
    $inc ="./include";
  }
elsif (-x "../configure")
  {
    $inc ="../include";
  }

if (!defined ($inc))
  {
    STDERR->print("Can't find configure.\n");
    exit (1);
  }

if (!open(IN, "< $inc/vstr-const.h"))
  {
    die "open: $!\n";
  }

$/ = undef;

$_ = <IN>;
while (/^#define\s+(VSTR_[0-9a-zA-Z][0-9a-zA-Z_]*)(\s|\()/gm)
  {
    if ($2 eq '(')
     { print "$1()\n"; }
    else
     { print "$1\n"; }
  }

if (!open(IN, "< $inc/vstr-switch.h"))
  {
    die "open: $!\n";
  }

$_ = <IN>;
while (/^#\s*define\s+(VSTR_COMPILE_[0-9a-zA-Z][0-9a-zA-Z_]*)\s/gm)
  {
    print "$1\n";
  }

