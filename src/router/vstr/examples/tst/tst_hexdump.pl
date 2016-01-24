#! /usr/bin/perl -w

use strict;

push @INC, "$ENV{SRCDIR}/tst";
require 'vstr_tst_examples.pl';

run_tst("ex_hexdump");

run_tst("ex_hexdump", undef, "--mmap");

run_tst("ex_hexdump", "ex_hexdump_none", "--none");

run_tst("ex_hexdump", "ex_hexdump_help", "--help");
run_tst("ex_hexdump", "ex_hexdump_version", "--version");

success();
