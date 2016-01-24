#! /usr/bin/perl -w

use strict;

push @INC, "$ENV{SRCDIR}/tst";
require 'vstr_tst_examples.pl';

run_tst("ex_cat");

run_tst("ex_cat", undef, "--mmap");

run_tst("ex_cat", "ex_cat_help", "--help");
run_tst("ex_cat", "ex_cat_version", "--version");

success();
