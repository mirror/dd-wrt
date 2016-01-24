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

if (!open(IN, "< $inc/vstr-extern.h"))
  {
    die "open: $!\n";
  }

$/ = undef;

$_ = <IN>;

# Not valid for everything C, but good enough...

while (/^#define\s+(VSTR_[0-9a-zA-Z][0-9a-zA-Z_]*)\s*\(/gm)
  {
    print "$1()\n";
  }

# Note all macro function are above prototypes...

while (/^extern\s+[\s0-9a-zA-Z_*]*\s+\**(vstr_[0-9a-zA-Z][0-9a-zA-Z_]*)\(/gm)
  {
    print "$1()\n";
  }

