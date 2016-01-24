#! /usr/bin/perl -w

use strict;

push @INC, "$ENV{SRCDIR}/tst";
require 'vstr_tst_examples.pl';

run_tst("ex_nl");

run_tst("ex_nl", undef, "--mmap");

run_tst("ex_nl", "ex_nl_help", "--help");
run_tst("ex_nl", "ex_nl_version", "--version");

success();
