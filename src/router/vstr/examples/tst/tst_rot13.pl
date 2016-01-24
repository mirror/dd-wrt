#! /usr/bin/perl -w

use strict;

push @INC, "$ENV{SRCDIR}/tst";
require 'vstr_tst_examples.pl';

run_tst("ex_rot13");

run_tst("ex_rot13", undef, "--mmap");

run_tst("ex_rot13", "ex_rot13_help", "--help");
run_tst("ex_rot13", "ex_rot13_version", "--version");

success();
